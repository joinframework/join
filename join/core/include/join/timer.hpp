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

#ifndef __JOIN_TIMER_HPP__
#define __JOIN_TIMER_HPP__

 // libjoin.
#include <join/reactor.hpp>

// C++
#include <functional>
#include <stdexcept>
#include <chrono>

// C.
#include <sys/timerfd.h>
#include <unistd.h>

namespace join
{
    class RealTime;
    class Steady;

    /**
     * @brief base timer class.
     */
    template <class ClockPolicy>
    class BasicTimer : protected EventHandler
    {
    public:
        /**
         * @brief create instance.
         */
        BasicTimer ()
        : _handle (timerfd_create (_policy.type (), TFD_NONBLOCK))
        {
            Reactor::instance ()->addHandler (this);
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
        BasicTimer (BasicTimer&& other) noexcept
        : _callback (std::move (other._callback))
        , _ns (other._ns)
        , _oneShot (other._oneShot)
        , _handle (other._handle)
        {
            Reactor::instance ()->delHandler (&other);

            other._callback = nullptr;
            other._ns = std::chrono::nanoseconds::zero ();
            other._oneShot = true;
            other._handle = -1;

            Reactor::instance ()->addHandler (this);
        }

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicTimer& operator= (BasicTimer&& other) noexcept
        {
            Reactor::instance ()->delHandler (this);
            cancel ();
            if (_handle != -1)
            {
                close (_handle);
            }

            _callback = std::move (other._callback);
            _ns = other._ns;
            _oneShot = other._oneShot;
            _handle = other._handle;

            Reactor::instance ()->delHandler (&other);

            other._callback = nullptr;
            other._ns = std::chrono::nanoseconds::zero ();
            other._oneShot = true;
            other._handle = -1;

            Reactor::instance ()->addHandler (this);

            return *this;
        }

        /**
         * @brief destroy instance.
         */
        virtual ~BasicTimer ()
        {
            Reactor::instance ()->delHandler (this);
            cancel ();
            if (_handle != -1)
            {
                close (_handle);
            }
        }

        /**
         * @brief arm the timer as a one-shot timer.
         * @param duration timeout duration before timer expires.
         * @param callback function to call when timer expires.
         */
        template <class Rep, class Period, typename Func>
        void setOneShot (std::chrono::duration <Rep, Period> duration, Func&& callback)
        {
            auto ns = std::chrono::duration_cast <std::chrono::nanoseconds> (duration);
            _callback = std::forward <Func> (callback);
            _oneShot = true;
            _ns = std::chrono::nanoseconds::zero ();

            auto ts = toTimerSpec (ns);
            timerfd_settime (_handle, 0, &ts, nullptr);
        }

        /**
         * @brief arm the timer as a one-shot timer with absolute time.
         * @param timePoint absolute time when timer should expire.
         * @param callback function to call when timer expires.
         */
        template <class Clock, class Duration, typename Func>
        void setOneShot (std::chrono::time_point <Clock, Duration> timePoint, Func&& callback)
        {
            static_assert(
                (std::is_same <ClockPolicy, RealTime>::value && std::is_same <Clock, std::chrono::system_clock>::value) ||
                (std::is_same <ClockPolicy, Steady>::value && std::is_same <Clock, std::chrono::steady_clock>::value),
                "Clock type mismatch timer policy"
            );

            auto elapsed = timePoint.time_since_epoch ();
            auto ns = std::chrono::duration_cast <std::chrono::nanoseconds> (elapsed);
            _callback = std::forward <Func> (callback);
            _oneShot = true;
            _ns = std::chrono::nanoseconds::zero ();

            auto ts = toTimerSpec (ns);
            timerfd_settime (_handle, TFD_TIMER_ABSTIME, &ts, nullptr);
        }

        /**
         * @brief arm the timer as a periodic timer.
         * @param duration interval duration between timer expirations.
         * @param callback function to call on each timer expiration.
         */
        template <class Rep, class Period, typename Func>
        void setInterval (std::chrono::duration <Rep, Period> duration, Func&& callback)
        {
            auto ns = std::chrono::duration_cast <std::chrono::nanoseconds> (duration);
            _callback = std::forward <Func> (callback);
            _oneShot = false;
            _ns = ns;

            auto ts = toTimerSpec (ns, true);
            timerfd_settime (_handle, 0, &ts, nullptr);
        }

        /**
         * @brief cancel the timer.
         */
        void cancel ()
        {
            _callback = nullptr;
            _oneShot = true;
            _ns = std::chrono::nanoseconds::zero ();

            struct itimerspec ts {};
            timerfd_settime (_handle, 0, &ts, nullptr);
        }

        /**
         * @brief check if timer is running.
         * @return true if timer is active.
         */
        bool active () const
        {
            struct itimerspec ts {};
            timerfd_gettime (_handle, &ts);
            const bool hasValue = (ts.it_value.tv_sec != 0 || ts.it_value.tv_nsec != 0);
            const bool hasInterval = (ts.it_interval.tv_sec != 0 || ts.it_interval.tv_nsec != 0);
            return hasValue || hasInterval;
        }

        /**
         * @brief get the remaining time until expiration.
         * @return remaining duration.
         */
        std::chrono::nanoseconds remaining () const
        {
            struct itimerspec ts {};
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
        int type () const noexcept
        {
            return _policy.type ();
        }

    protected:
        /**
         * @brief method called when data are ready to be read on handle.
         */
        virtual void onReceive () override
        {
            uint64_t expirations;
            ssize_t result = read (_handle, &expirations, sizeof (expirations));
            if (result == sizeof (expirations) && _callback)
            {
                for (uint64_t i = 0; i < expirations; ++i)
                {
                    _callback ();
                }
            }
        }

        /**
         * @brief convert nsec to itimerspec.
         * @param ns value to convert.
         * @param periodic specify if periodic.
         * @return itimerspec.
         */
        static itimerspec toTimerSpec (std::chrono::nanoseconds ns, bool periodic = false) noexcept
        {
            itimerspec ts {};
            ts.it_value.tv_sec = ns.count () / NS_PER_SEC;
            ts.it_value.tv_nsec = ns.count () % NS_PER_SEC;
            if (periodic)
            {
                ts.it_interval.tv_sec = ts.it_value.tv_sec;
                ts.it_interval.tv_nsec = ts.it_value.tv_nsec;
            }
            return ts;
        }

        /**
         * @brief get native handle.
         * @return native handle.
         */
        virtual int handle () const noexcept override
        {
            return _handle;
        }

    private:
        /// ns per sec.
        static constexpr uint64_t NS_PER_SEC = 1000000000ULL;

        /// clock policy.
        ClockPolicy _policy;

        /// callback function
        std::function <void ()> _callback;

        /// interval.
        std::chrono::nanoseconds _ns {};

        /// timer type
        bool _oneShot = true;

        /// timer handle.
        int _handle = -1;
    };

    /**
     * @brief system clock policy class.
     */
    class RealTime
    {
    public:
        using Timer = BasicTimer <RealTime>;

        /**
         * @brief construct the timer policy instance by default.
         */
        constexpr RealTime () noexcept = default;

        /**
         * @brief get timer type.
         * @return the timer type.
         */
        constexpr int type () const noexcept
        {
            return CLOCK_REALTIME;
        }
    };

    /**
     * @brief monotonic clock policy class.
     */
    class Steady
    {
    public:
        using Timer = BasicTimer <Steady>;

        /**
         * @brief construct the timer policy instance by default.
         */
        constexpr Steady () noexcept = default;

        /**
         * @brief get timer type.
         * @return the timer type.
         */
        constexpr int type () const noexcept
        {
            return CLOCK_MONOTONIC;
        }
    };
}

#endif
