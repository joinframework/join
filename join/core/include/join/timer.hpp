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
    /**
     * @brief timer class.
     */
    class Timer : protected EventHandler
    {
    public:
        /**
         * @brief create instance.
         */
        Timer ()
        : _handle (timerfd_create (CLOCK_MONOTONIC, TFD_NONBLOCK))
        {
            Reactor::instance ()->addHandler (this);
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        Timer (const Timer& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        Timer& operator= (const Timer& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        Timer (Timer&& other) noexcept
        : _handle (other._handle)
        , _callback (std::move (other._callback))
        , _oneShot (other._oneShot)
        , _ns (other._ns)
        {
            Reactor::instance ()->delHandler (&other);

            other._handle = -1;
            other._callback = nullptr;
            other._oneShot = true;
            other._ns = 0;

            Reactor::instance ()->addHandler (this);
        }

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        Timer& operator= (Timer&& other) noexcept
        {
            Reactor::instance ()->delHandler (this);
            cancel ();
            close (_handle);

            _handle = other._handle;
            _callback = std::move (other._callback);
            _oneShot = other._oneShot;
            _ns = other._ns;

            Reactor::instance ()->delHandler (&other);

            other._handle = -1;
            other._callback = nullptr;
            other._oneShot = true;
            other._ns = 0;

            Reactor::instance ()->addHandler (this);

            return *this;
        }

        /**
         * @brief destroy instance.
         */
        virtual ~Timer ()
        {
            Reactor::instance ()->delHandler (this);
            cancel ();
            close (_handle);
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
            _ns = 0;

            struct itimerspec ts {};
            ts.it_value.tv_sec = ns.count () / NS_PER_SEC;
            ts.it_value.tv_nsec = ns.count () % NS_PER_SEC;
            timerfd_settime (_handle, 0, &ts, nullptr);
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
            _ns = ns.count ();

            struct itimerspec ts {};
            ts.it_value.tv_sec = ns.count () / NS_PER_SEC;
            ts.it_value.tv_nsec = ns.count () % NS_PER_SEC;
            ts.it_interval.tv_sec = ns.count () / NS_PER_SEC;
            ts.it_interval.tv_nsec = ns.count () % NS_PER_SEC;
            timerfd_settime (_handle, 0, &ts, nullptr);
        }

        /**
         * @brief cancel the timer.
         */
        void cancel ()
        {
            _callback = nullptr;
            _oneShot = true;
            _ns = 0;

            struct itimerspec ts {};
            timerfd_settime (_handle, 0, &ts, nullptr);
        }

        /**
         * @brief get the interval of the running periodic timer.
         * @return interval duration in nanoseconds, zero if one-shot or inactive.
         */
        std::chrono::nanoseconds interval () const
        {
            return std::chrono::nanoseconds (_ns);
        }

        /**
         * @brief check if timer is running.
         * @return true if timer is active.
         */
        bool isActive () const
        {
            struct itimerspec ts {};
            timerfd_gettime (_handle, &ts);
            const bool hasValue = (ts.it_value.tv_sec != 0 || ts.it_value.tv_nsec != 0);
            const bool hasInterval = (ts.it_interval.tv_sec != 0 || ts.it_interval.tv_nsec != 0);
            return hasValue || hasInterval;
        }

        /**
         * @brief check if timer is a one-shot timer.
         * @return true if timer is a one-shot timer.
         */
        bool isOneShot () const
        {
            return _oneShot;
        }

protected:
        /**
         * @brief get native handle.
         * @return native handle.
         */
        virtual int handle () const noexcept override
        {
            return _handle;
        }

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

    private:
        /// ns per sec.
        static constexpr uint64_t NS_PER_SEC = 1000000000ULL;

        /// timer handle.
        int _handle = -1;

        /// callback function
        std::function <void ()> _callback;

        /// timer type
        bool _oneShot = true;

        /// interval.
        uint64_t _ns = 0;
    };
}
