/**
 * MIT License
 *
 * Copyright (c) 2021 Mathieu Rabine
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

#ifndef __JOIN_OBSERVER_HPP__
#define __JOIN_OBSERVER_HPP__

// libjoin.
#include <join/error.hpp>
#include <join/protocol.hpp>

// C++.
#include <condition_variable>
#include <functional>
#include <memory>
#include <thread>
#include <mutex>

// C.
#include <sys/eventfd.h>
#include <sys/epoll.h>

namespace join
{
namespace net
{
    /**
     * @brief basic observer class.
     */
    template <class Observable>
    class BasicObserver : public Observable
    {
    public:
        using Observable::handle;

        /**
         * @brief default constructor.
         */
        BasicObserver ()
        {
            eventdesc_ = eventfd (0, EFD_NONBLOCK | EFD_CLOEXEC);
            if (eventdesc_ == -1)
            {
                throw std::system_error (errno, std::generic_category ());
            }

            epolldesc_ = epoll_create1 (EPOLL_CLOEXEC);
            if (epolldesc_ == -1)
            {
                throw std::system_error (errno, std::generic_category ());
            }
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicObserver (const BasicObserver &other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicObserver& operator= (const BasicObserver& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        BasicObserver (BasicObserver&& other) = delete;

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicObserver& operator= (BasicObserver&& other) = delete;

        /**
         * @brief destroy the instance.
         */
        virtual ~BasicObserver ()
        {
            // start/stop must not occur when queue is processed.
            std::unique_lock <std::recursive_mutex> lock (epollmx_);

            // check if thread was running.
            if (finished_ == false)
            {
                uint64_t value = 1;

                // notify event in order to stop reception thread.
                [[maybe_unused]] ssize_t bytes = ::write (eventdesc_, &value, sizeof (uint64_t));

                // wait for end of reception thread.
                epollend_.wait (lock, [this] () {
                    return finished_;
                });
            }

            // close descriptors.
            ::close (eventdesc_);
            ::close (epolldesc_);
        }

        /**
         * @brief start reception thread.
         * @return 0 on success, -1 on failure.
         */
        int start ()
        {
            // start/stop must not occur when queue is processed.
            std::lock_guard <std::recursive_mutex> lock (epollmx_);

            // check if already running.
            if (finished_ == false)
            {
                lastError = make_error_code (Errc::InUse);
                return -1;
            }

            struct epoll_event ev;
            ev.events = EPOLLIN | EPOLLRDHUP;
            ev.data.fd = handle ();

            // add descriptor to epoll instance.
            if (epoll_ctl (epolldesc_, EPOLL_CTL_ADD, ev.data.fd, &ev) == -1)
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                return -1;
            }

            try
            {
                // start reception thread.
                std::thread (std::bind (&BasicObserver::waitReception, this)).detach ();
                finished_ = false;
            }
            catch (const std::system_error& err)
            {
                lastError = err.code ();
                return -1;
            }

            return 0;
        }

        /**
         * @brief stop reception thread.
         * @return 0 on success, -1 on failure.
         */
        int stop ()
        {
            // start/stop must not occur when queue is processed.
            std::lock_guard <std::recursive_mutex> lock (epollmx_);

            // check if already stopped.
            if (finished_ == true)
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            // remove descriptor from epoll instance.
            if (epoll_ctl (epolldesc_, EPOLL_CTL_DEL, handle (), nullptr) == -1)
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                return -1;
            }

            uint64_t value = 1;

            // notify event in order to stop reception thread.
            [[maybe_unused]] ssize_t bytes = ::write (eventdesc_, &value, sizeof (uint64_t));

            return 0;
        }

    protected:
        /**
         * @brief method called on receive.
         */
        virtual void onReceive () = 0;

        /**
         * @brief method called on error.
         */
        virtual void onError () {}

        /**
         * @brief method called on close.
         */
        virtual void onClose () {}

    private:
        /**
         * @brief reception thread routine.
         */
        void waitReception ()
        {
            std::unique_lock <std::recursive_mutex> lock (epollmx_);

            fd_set setfd;
            FD_ZERO (&setfd);
            FD_SET (eventdesc_, &setfd);
            FD_SET (epolldesc_, &setfd);
            int maxdesc = std::max (eventdesc_, epolldesc_);

            for (;;)
            {
                fd_set descset = setfd;

                lock.unlock ();

                int nset = ::select (maxdesc + 1, &descset, nullptr, nullptr, nullptr);
                if (nset > 0)
                {
                    if (FD_ISSET (eventdesc_, &descset))
                    {
                        lock.lock ();

                        uint64_t value;
                        [[maybe_unused]] ssize_t bytes = ::read (eventdesc_, &value, sizeof (uint64_t));

                        break;
                    }

                    if (FD_ISSET (epolldesc_, &descset))
                    {
                        lock.lock ();

                        struct epoll_event ev;

                        // read event now that we are protected.
                        nset = epoll_wait (epolldesc_, &ev, 1, 0);
                        if (nset == 1)
                        {
                            if (ev.events & EPOLLERR)
                            {
                                onError ();
                            }
                            else if ((ev.events & EPOLLRDHUP) || (ev.events & EPOLLHUP))
                            {
                                onClose ();
                            }
                            else if (ev.events & EPOLLIN)
                            {
                                onReceive ();
                            }
                        }

                        continue;
                    }
                }

                lock.lock ();
            }

            // set reception thread status to not running.
            finished_ = true;

            // signal end of reception thread.
            // instance could be destroyed immediately after lock is released.
            // nothing should be done out of the lock scope.
            epollend_.notify_one ();
        }

        /// event descriptor.
        int eventdesc_ = -1;

        /// epoll file descriptor.
        int epolldesc_ = -1;

        /// epoll protection mutex.
        std::recursive_mutex epollmx_;

        /// epoll end event.
        std::condition_variable_any epollend_;

        /// epoll status.
        bool finished_ = true;
    };
}
}

#endif