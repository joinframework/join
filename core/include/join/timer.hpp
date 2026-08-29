/**
 * MIT License
 *
 * Copyright (c) 2025 Mathieu Rabine
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef JOIN_CORE_TIMER_HPP
#define JOIN_CORE_TIMER_HPP

// libjoin.
#include <join/proactor.hpp>
#include <join/function.hpp>
#include <join/backoff.hpp>
#include <join/clock.hpp>

// C++.
#include <chrono>
#include <atomic>
#include <memory>
#include <new>

// C.
#include <sys/timerfd.h>
#include <unistd.h>
#include <cstdlib>

namespace join
{
    /**
     * @brief base timer class.
     */
    template <class ClockPolicy>
    class BasicTimer : protected CompletionHandler
    {
    public:
        /**
         * @brief timer state.
         */
        enum class State
        {
            Idle,     /**< no callback armed. */
            Armed,    /**< a callback is armed. */
            Invoking, /**< the callback is being invoked. */
            Closed,   /**< the timerfd read operation is no longer in flight. */
        };

        /**
         * @brief create instance.
         * @param proactor completion dispatcher.
         */
        explicit BasicTimer (Proactor& proactor = ProactorThread::proactor ())
        : _handle (timerfd_create (ClockPolicy::type (), TFD_NONBLOCK | TFD_CLOEXEC))
        , _proactor (proactor)
        {
            if (_handle == -1)
            {
                throw std::system_error (errno, std::system_category (), "timerfd_create failed");  // LCOV_EXCL_LINE
            }

            _ops->op = IoOperation::makeRead (_handle, &_ops->expirations,
                                              static_cast<uint32_t> (sizeof (_ops->expirations)), this);

            if (_proactor.submit (&_ops->op, true, true) == -1)
            {
                // LCOV_EXCL_START
                close (_handle);
                throw std::system_error (lastError, "timerfd submit failed");
                // LCOV_EXCL_STOP
            }
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicTimer (const BasicTimer& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicTimer& operator= (const BasicTimer& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        BasicTimer (BasicTimer&& other) = delete;

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicTimer& operator= (BasicTimer&& other) = delete;

        /**
         * @brief destroy instance.
         */
        ~BasicTimer () noexcept
        {
            cancel ();

            Backoff backoff;
            while (_state.load (std::memory_order_acquire) != State::Closed)
            {
                _proactor.cancel (&_ops->op, true, true);
                backoff ();
            }

            close (_handle);
        }

        /**
         * @brief arm the timer as a one-shot timer.
         * @param duration timeout duration before timer expires.
         * @param callback function to call when timer expires, captures limited to 32 bytes.
         */
        template <class Rep, class Period, typename Func>
        void setOneShot (std::chrono::duration<Rep, Period> duration, Func&& callback)
        {
            cancel ();

            auto ns = std::chrono::duration_cast<std::chrono::nanoseconds> (duration);
            _callback = std::forward<Func> (callback);
            _oneShot = true;
            _ns = std::chrono::nanoseconds::zero ();
            _state.store (State::Armed, std::memory_order_release);

            auto ts = toTimerSpec (ns);
            timerfd_settime (_handle, 0, &ts, nullptr);
        }

        /**
         * @brief arm the timer as a one-shot timer with absolute time.
         * @param timePoint absolute time when timer should expire.
         * @param callback function to call when timer expires, captures limited to 32 bytes.
         */
        template <class Clock, class Duration, typename Func>
        void setOneShot (std::chrono::time_point<Clock, Duration> timePoint, Func&& callback)
        {
            static_assert (
                (std::is_same<ClockPolicy, RealTime>::value && std::is_same<Clock, std::chrono::system_clock>::value) ||
                    (std::is_same<ClockPolicy, Monotonic>::value &&
                     std::is_same<Clock, std::chrono::steady_clock>::value),
                "Clock type mismatch timer policy");

            cancel ();

            auto elapsed = timePoint.time_since_epoch ();
            auto ns = std::chrono::duration_cast<std::chrono::nanoseconds> (elapsed);
            _callback = std::forward<Func> (callback);
            _oneShot = true;
            _ns = std::chrono::nanoseconds::zero ();
            _state.store (State::Armed, std::memory_order_release);

            auto ts = toTimerSpec (ns);
            timerfd_settime (_handle, TFD_TIMER_ABSTIME, &ts, nullptr);
        }

        /**
         * @brief arm the timer as a periodic timer.
         * @param duration interval duration between timer expirations.
         * @param callback function to call on each timer expiration, captures limited to 32 bytes.
         */
        template <class Rep, class Period, typename Func>
        void setInterval (std::chrono::duration<Rep, Period> duration, Func&& callback)
        {
            cancel ();

            auto ns = std::chrono::duration_cast<std::chrono::nanoseconds> (duration);
            _callback = std::forward<Func> (callback);
            _oneShot = false;
            _ns = ns;
            _state.store (State::Armed, std::memory_order_release);

            auto ts = toTimerSpec (ns, true);
            timerfd_settime (_handle, 0, &ts, nullptr);
        }

        /**
         * @brief cancel the timer.
         */
        void cancel () noexcept
        {
            _oneShot = true;
            _ns = std::chrono::nanoseconds::zero ();

            struct itimerspec ts = {};
            timerfd_settime (_handle, 0, &ts, nullptr);

            if (JOIN_UNLIKELY (_proactor.isProactorThread ()))
            {
                _state.store (State::Idle, std::memory_order_release);
                return;
            }

            Backoff backoff;
            State expected = State::Armed;

            while (!_state.compare_exchange_strong (expected, State::Idle, std::memory_order_acq_rel,
                                                    std::memory_order_acquire))
            {
                if (expected != State::Invoking)
                {
                    return;
                }

                backoff ();
                expected = State::Armed;
            }

            _callback = nullptr;
        }

        /**
         * @brief check if timer is running.
         * @return true if timer is active.
         */
        bool active () const noexcept
        {
            struct itimerspec ts = {};
            timerfd_gettime (_handle, &ts);
            const bool hasValue = (ts.it_value.tv_sec != 0 || ts.it_value.tv_nsec != 0);
            const bool hasInterval = (ts.it_interval.tv_sec != 0 || ts.it_interval.tv_nsec != 0);
            return hasValue || hasInterval;
        }

        /**
         * @brief get the remaining time until expiration.
         * @return remaining duration.
         */
        std::chrono::nanoseconds remaining () const noexcept
        {
            struct itimerspec ts = {};
            timerfd_gettime (_handle, &ts);
            return std::chrono::seconds (ts.it_value.tv_sec) + std::chrono::nanoseconds (ts.it_value.tv_nsec);
        }

        /**
         * @brief get the interval of the running periodic timer.
         * @return interval duration in nanoseconds, zero if one-shot or inactive.
         */
        std::chrono::nanoseconds interval () const noexcept
        {
            return _ns;
        }

        /**
         * @brief check if timer is a one-shot timer.
         * @return true if timer is a one-shot timer.
         */
        bool oneShot () const noexcept
        {
            return _oneShot;
        }

        /**
         * @brief get the timer type.
         * @return the timer type.
         */
        static constexpr int type () noexcept
        {
            return ClockPolicy::type ();
        }

    private:
        /**
         * @brief method called when the timerfd read completes.
         * @param op completed operation.
         * @param result bytes read, or negative errno.
         */
        void onComplete ([[maybe_unused]] IoOperation* op, int result) override
        {
            uint64_t expirations = _ops->expirations;

            if (JOIN_UNLIKELY (result != static_cast<int> (sizeof (_ops->expirations)) && result != -EAGAIN &&
                               result != -EINTR))
            {
                // LCOV_EXCL_START
                _state.store (State::Closed, std::memory_order_release);
                return;
                // LCOV_EXCL_STOP
            }

            if (JOIN_UNLIKELY (_proactor.submit (&_ops->op) == -1))
            {
                // LCOV_EXCL_START
                _state.store (State::Closed, std::memory_order_release);
                return;
                // LCOV_EXCL_STOP
            }

            if (JOIN_UNLIKELY (result != static_cast<int> (sizeof (_ops->expirations))))
            {
                return;  // LCOV_EXCL_LINE
            }

            State expected = State::Armed;

            if (JOIN_LIKELY (_state.compare_exchange_strong (expected, State::Invoking, std::memory_order_acq_rel,
                                                             std::memory_order_acquire)))
            {
                for (uint64_t i = 0; i < expirations; ++i)
                {
                    _callback ();
                }

                expected = State::Invoking;
                _state.compare_exchange_strong (expected, State::Armed, std::memory_order_acq_rel,
                                                std::memory_order_acquire);
            }
        }

        /**
         * @brief method called when the timerfd read is cancelled.
         * @param op cancelled operation.
         * @param result negative errno.
         */
        void onCancel ([[maybe_unused]] IoOperation* op, [[maybe_unused]] int result) override
        {
            _state.store (State::Closed, std::memory_order_release);
        }

        /**
         * @brief convert nsec to itimerspec.
         * @param ns value to convert.
         * @param periodic specify if periodic.
         * @return itimerspec.
         */
        static constexpr itimerspec toTimerSpec (std::chrono::nanoseconds ns, bool periodic = false) noexcept
        {
            itimerspec ts{};
            if (periodic)
            {
                ts.it_interval.tv_sec = ns.count () / _nsPerSec;
                ts.it_interval.tv_nsec = ns.count () % _nsPerSec;
            }
            if (ns.count () < 1)
            {
                ns = std::chrono::nanoseconds (1);
            }
            ts.it_value.tv_sec = ns.count () / _nsPerSec;
            ts.it_value.tv_nsec = ns.count () % _nsPerSec;
            return ts;
        }

        /**
         * @brief operation block, kept at its extended alignment.
         */
        struct Ops
        {
            /**
             * @brief allocate a block honouring its extended alignment.
             * @param size allocation size in bytes.
             * @return pointer to the allocated storage.
             */
            static void* operator new (size_t size)
            {
                void* mem = ::aligned_alloc (alignof (Ops), size);

                if (mem == nullptr)
                {
                    throw std::bad_alloc ();  // LCOV_EXCL_LINE
                }

                return mem;
            }

            /**
             * @brief release storage allocated by operator new.
             * @param mem storage to release.
             */
            static void operator delete (void* mem) noexcept
            {
                ::free (mem);
            }

            /// timerfd read operation.
            IoOperation op = {};

            /// expiration count, written by the kernel until the read completes.
            uint64_t expirations = 0;
        };

        /// ns per sec.
        static constexpr uint64_t _nsPerSec = 1000000000ULL;

        /// operation block.
        const std::unique_ptr<Ops> _ops{new Ops ()};

        /// timer state.
        std::atomic<State> _state{State::Idle};

        /// callback function
        Function<void ()> _callback;

        /// interval.
        std::chrono::nanoseconds _ns{};

        /// timer type
        bool _oneShot = true;

        /// timer handle.
        int _handle = -1;

        /// completion dispatcher.
        Proactor& _proactor;
    };
}

#endif
