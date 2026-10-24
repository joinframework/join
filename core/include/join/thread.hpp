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

#ifndef JOIN_CORE_THREAD_HPP
#define JOIN_CORE_THREAD_HPP

// libjoin.
#include <join/utils.hpp>

// C++.
#include <system_error>
#include <type_traits>
#include <functional>
#include <memory>
#include <atomic>

// C.
#include <sys/syscall.h>
#include <pthread.h>
#include <unistd.h>

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
         * @param core thread core affinity (-1 no pinning).
         * @param prio thread priority (0 = SCHED_OTHER, 1-99 = SCHED_FIFO).
         * @param stackSize thread stack size (0 = default).
         * @param prefault prefault thread stack.
         * @param func callable object to execute in the new thread of execution.
         * @param args... arguments to pass to the callable object to execute.
         * @throw std::system_error if the thread of execution cannot be created.
         */
        template <class Function, class... Args>
        explicit Invoker (int core, int prio, size_t stackSize, bool prefault, Function&& func, Args&&... args)
        : _func (std::bind (std::forward<Function> (func), std::forward<Args> (args)...))
        , _core (core)
        , _priority (prio)
        , _prefault (prefault)
        , _done (false)
        {
            pthread_attr_t attr;
            pthread_attr_init (&attr);

            if (stackSize > 0)
            {
                const int err = pthread_attr_setstacksize (&attr, stackSize);
                if (err != 0)
                {
                    pthread_attr_destroy (&attr);
                    throw std::system_error (err, std::system_category (), "pthread_attr_setstacksize failed");
                }
            }

            const int err = pthread_create (&_handle, &attr, _routine, this);
            pthread_attr_destroy (&attr);
            if (err != 0)
            {
                throw std::system_error (err, std::system_category (), "pthread_create failed");  // LCOV_EXCL_LINE
            }
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
        ~Invoker () = default;

    private:
        /**
         * @brief prefault calling thread stack.
         * @return 0 on success, -1 on failure.
         */
        static int prefault () noexcept;

        /**
         * @brief thread routine.
         * @param context context passed to the new thread of execution.
         * @return thread return statement.
         */
        static void* _routine (void* context);

        /**
         * @brief thread routine.
         * @return thread return statement.
         */
        void* routine ();

        /// user function to execute.
        std::function<void ()> _func;

        /// thread handle.
        pthread_t _handle = {};

        /// thread core affinity.
        std::atomic_int _core{-1};

        /// thread priority.
        std::atomic_int _priority{0};

        /// prefault stack flag.
        bool _prefault = false;

        /// invalid thread id.
        static constexpr pid_t _invalidId = -1;

        /// thread id.
        std::atomic<pid_t> _id{_invalidId};

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
        /// invalid thread id.
        static constexpr pid_t invalidId = Invoker::_invalidId;

        /**
         * @brief default constructor.
         */
        Thread () noexcept = default;

        /**
         * @brief creates a new thread object associated with a thread of execution.
         * @param func callable object to execute in the new thread.
         * @param args... arguments to pass to the new function.
         * @throw std::system_error if the thread of execution cannot be created.
         */
        template <class Function, class... Args,
                  typename = std::enable_if_t<!std::is_arithmetic<std::decay_t<Function>>::value>>
        explicit Thread (Function&& func, Args&&... args)
        : Thread (-1, 0, 0, false, std::forward<Function> (func), std::forward<Args> (args)...)
        {
        }

        /**
         * @brief creates a new thread object associated with a thread of execution.
         * @param core thread core affinity (-1 no pinning).
         * @param prio thread priority (0 = SCHED_OTHER, 1-99 = SCHED_FIFO).
         * @param func callable object to execute in the new thread.
         * @param args... arguments to pass to the new function.
         * @throw std::system_error if the thread of execution cannot be created.
         */
        template <class Function, class... Args,
                  typename = std::enable_if_t<!std::is_arithmetic<std::decay_t<Function>>::value>>
        explicit Thread (int core, int prio, Function&& func, Args&&... args)
        : Thread (core, prio, 0, false, std::forward<Function> (func), std::forward<Args> (args)...)
        {
        }

        /**
         * @brief creates a new thread object associated with a thread of execution.
         * @param core thread core affinity (-1 no pinning).
         * @param prio thread priority (0 = SCHED_OTHER, 1-99 = SCHED_FIFO).
         * @param stackSize thread stack size (0 = default).
         * @param prefault prefault thread stack.
         * @param func callable object to execute in the new thread.
         * @param args... arguments to pass to the new function.
         * @throw std::system_error if the thread of execution cannot be created.
         */
        template <class Function, class... Args>
        explicit Thread (int core, int prio, size_t stackSize, bool prefault, Function&& func, Args&&... args)
        : _invoker (new Invoker (core, prio, stackSize, prefault, std::forward<Function> (func),
                                 std::forward<Args> (args)...))
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
         * @brief set thread affinity.
         * @param core thread core affinity (-1 to disable pinning).
         * @return 0 on success, -1 on failure.
         */
        int affinity (int core);

        /**
         * @brief set thread affinity.
         * @param handle thread handle.
         * @param core thread core affinity (-1 to disable pinning).
         * @return 0 on success, -1 on failure.
         */
        static int affinity (pthread_t handle, int core);

        /**
         * @brief get current thread affinity.
         * @return affinity or -1 if not pinned.
         */
        int affinity () const noexcept;

        /**
         * @brief set thread priority.
         * @param prio thread priority (0 = SCHED_OTHER, 1-99 = SCHED_FIFO).
         * @return 0 on success, -1 on failure.
         */
        int priority (int prio);

        /**
         * @brief set thread priority.
         * @param handle thread handle.
         * @param prio thread priority (0 = SCHED_OTHER, 1-99 = SCHED_FIFO).
         * @return 0 on success, -1 on failure.
         */
        static int priority (pthread_t handle, int prio);

        /**
         * @brief get current thread priority.
         * @return priority.
         */
        int priority () const noexcept;

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
        void join () noexcept;

        /**
         * @brief performs a nonblocking join on the running thread.
         * @return true if thread was joined, false if running thread didn't finished its execution.
         */
        bool tryJoin () noexcept;

        /**
         * @brief cancel the running thread if any.
         */
        void cancel () noexcept;

        /**
         * @brief swap underlying handles of two thread objects.
         * @param other the thread to swap with.
         */
        void swap (Thread& other) noexcept;

        /**
         * @brief get calling thread id.
         * @return calling thread id.
         */
        static pid_t currentId () noexcept
        {
            static thread_local pid_t tid __attribute__ ((tls_model ("initial-exec"))) = 0;
            if (JOIN_UNLIKELY (tid == 0))
            {
                static const int registered = ::pthread_atfork (nullptr, nullptr, [] () {
                    tid = 0;
                });
                static_cast<void> (registered);
                tid = static_cast<pid_t> (::syscall (SYS_gettid));
            }
            return tid;
        }

        /**
         * @brief get thread id.
         * @return thread id or invalidId.
         */
        pid_t id () const noexcept;

        /**
         * @brief get the handle of the thread of execution.
         * @return thread of execution handle.
         */
        pthread_t handle () const noexcept;

    private:
        /// current thread informations.
        std::unique_ptr<Invoker> _invoker;
    };
}

#endif
