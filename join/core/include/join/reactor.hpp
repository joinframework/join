/**
 * MIT License
 *
 * Copyright (c) 2022 Mathieu Rabine
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

#ifndef __JOIN_REACTOR_HPP__
#define __JOIN_REACTOR_HPP__

// libjoin.
#include <join/condition.hpp>

// C++.
#include <vector>

// C.
#include <sys/epoll.h>

namespace join
{
    /**
     *  @brief Event handler interface class.
     */
    class EventHandler
    {
    public:
        /**
         * @brief method called when data are ready to be read on handle.
         */
        virtual void onReceive () = 0;

        /**
         * @brief method called when handle is closed.
         */
        virtual void onClose () { /*do nothing*/ }

        /**
         * @brief method called when an error occured on handle.
         */
        virtual void onError () { /*do nothing*/ }

        /**
         * @brief get native handle.
         * @return native handle.
         */
        virtual int handle () const = 0;
    };

    /**
     *  @brief Reactor class.
     */
    class Reactor
    {
    public:
        /**
         * @brief create the Reactor instance.
         * @return Reactor instance pointer.
         */
        static Reactor* instance ();

        /**
         * @brief add handler to reactor.
         * @param handler handler pointer.
         * @return 0 on success, -1 on failure.
         */
        int addHandler (EventHandler* handler);

        /**
         * @brief delete handler from reactor.
         * @param handler handler pointer.
         * @return 0 on success, -1 on failure.
         */
        int delHandler (EventHandler* handler);

    protected:
        /**
         * @brief default constructor.
         */
        Reactor ();

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        Reactor (const Reactor& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        Reactor (Reactor&& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to copy.
         * @return current object.
         */
        Reactor& operator= (const Reactor& other) = delete;

        /**
         * @brief move assignment operator.
         * @param other other object to move.
         * @return current object.
         */
        Reactor& operator= (Reactor&& other) = delete;

        /**
         * @brief destroy instance.
         */
        ~Reactor ();

        /**
         * @brief dispatch events received.
         */
        void dispatch ();

        /// event descriptor.
        int _event = -1;

        /// epoll file descriptor.
        int _epoll = -1;

        /// epoll protection mutex.
        Mutex _mutex;

        /// epoll end event.
        Condition _end;

        /// status.
        bool _running = false;

        /// number of handlers.
        int _num = 0;
    };
}

#endif
