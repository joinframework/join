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

#ifndef __JOIN_THREAD_HPP__
#define __JOIN_THREAD_HPP__

// C++.
#include <system_error>
#include <functional>
#include <memory>
#include <thread>
#include <atomic>

// C.
#include <pthread.h>

namespace join
{
    /**
     * @brief thread invoker class.
     */
    class Invoker
    {
    public:
        /**
         * @brief default constructor.
         */
        Invoker () = delete;

    private:
        /**
         * @brief creates a new thread of execution.
         * @param func callable object to execute in the new thread of execution.
         * @param args... arguments to pass to the callable object to execute.
         */
        template <class Function, class... Args>
        explicit Invoker (Function&& func, Args&&... args)
        : _func (std::bind (std::forward <Function> (func), std::forward <Args> (args)...)),
          _done (false)
        {
            pthread_attr_init (&_attr);
            pthread_create (&_handle, &_attr, _routine, this);
        }

    public:
        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        Invoker (const Invoker& other) = delete;

        /**
         * @brief copy assignment.
         * @param other other object to copy.
         * @return a reference to the current object.
         */
        Invoker& operator= (const Invoker& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        Invoker (Invoker&& other) = delete;

        /**
         * @brief move assignment.
         * @param other other object to move.
         * @return a reference to the current object.
         */
        Invoker& operator= (Invoker&& other) = delete;

        /**
         * @brief destoyer.
         */
        ~Invoker ();

    private:
        /**
         * @brief get the handle of the thread of execution.
         * @retunr thread of execution handle.
         */
        pthread_t handle ();

        /**
         * @brief thread routine.
         * @param context context passed to the new thread of execution.
         * @return thread return statement.
         */
        static void * _routine (void * context);

        /**
         * @brief thread routine.
         * @return thread return statement.
         */
        void * routine (void);

        /// user function to execute.
        std::function <void ()> _func;

        /// thread attributes.
        pthread_attr_t _attr;

        /// thread handle.
        pthread_t _handle;

        /// completed flag.
        std::atomic_bool _done;

        /// friendship with the thread class.
        friend class Thread;
    };

    /**
     * @brief thread class.
     */
    class Thread
    {
    public:
        /**
         * @brief default constructor.
         */
        Thread () noexcept = default;

        /**
         * @brief creates a new thread object associated with a thread of execution.
         * @param func callable object to execute in the new thread.
         * @param args... arguments to pass to the new function.
         */
        template <class Function, class... Args>
        explicit Thread (Function&& func, Args&&... args)
        : _invoker (new Invoker (std::forward <Function> (func), std::forward <Args> (args)...))
        {
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        Thread (const Thread& other) = delete;

        /**
         * @brief copy assignment.
         * @param other other object to copy.
         * @return a reference to the current object.
         */
        Thread& operator= (const Thread& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        Thread (Thread&& other) noexcept;

        /**
         * @brief move assignment.
         * @param other other object to move.
         * @return a reference to the current object.
         */
        Thread& operator= (Thread&& other) noexcept;

        /**
         * @brief destroys the thread object.
         */
        ~Thread ();

        /**
         * @brief check if thread is joinable.
         * @return true if joinable.
         */
        bool joinable () const noexcept;

        /**
         * @brief check if the thread is running.
         * @return true if running, false otherwise.
         */
        bool running () const noexcept;

        /**
         * @brief block the current thread until the running thread finishes its execution.
         */
        void join ();

        /**
         * @brief performs a nonblocking join on the running thread.
         * @return true if thread was joined, false if running thread didn't finished its execution.
         */
        bool tryJoin ();

        /**
         * @brief cancel the running thread if any.
         */
        void cancel ();

        /**
         * @brief swap underlying handles of two thread objects.
         * @param other the thread to swap with.
         */
        void swap (Thread& other);

    private:
        /// current thread informations.
        std::unique_ptr <Invoker> _invoker;
    };
}

#endif
