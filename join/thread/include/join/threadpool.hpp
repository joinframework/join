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

// libjoin.
#include <join/condition.hpp>
#include <join/thread.hpp>

// C++.
#include <functional>
#include <thread>
#include <memory>
#include <atomic>
#include <vector>
#include <deque>

namespace join
{
    /// forward declaration.
    class ThreadPool;

    /**
     * @brief worker thread.
     */
    class Worker
    {
    public:
        /**
         * @brief create worker thread.
         * @param pool thread pool.
         */
        Worker (ThreadPool& pool);

        /**
         * @brief destroy worker thread.
         */
        ~Worker ();

    private:
        /**
         * @brief worker thread routine.
         */
        void work ();

        /// thread pool.
        ThreadPool& _pool;

        /// thread.
        Thread _thread;
    };

    /**
     * @brief thread pool.
     */
    class ThreadPool
    {
    public:
        /**
         * @brief create thread pool.
         * @param workers number of worker threads.
         */
        ThreadPool (size_t workers = std::thread::hardware_concurrency () + 1);

        /**
         * @brief destroy thread pool.
         */
        ~ThreadPool ();

        /**
         * @brief push a job to the work queue.
         */
        template <class Function, class... Args>
        void push (Function&& func, Args&&... args)
        {
            ScopedLock lock (_mutex);
            _jobs.emplace_back (std::bind (std::forward <Function> (func), std::forward <Args> (args)...));
            _condition.signal ();
        }

    private:
        /// worker threads.
        std::vector <std::unique_ptr <Worker>> _workers;

        /// condition shared with worker threads.
        Condition _condition;

        /// condition protection mutex.
        Mutex _mutex;

        /// gracefully stop all threads.
        std::atomic <bool> _stop;

        /// jobs queue.
        std::deque <std::function <void ()>> _jobs;

        /// friendship with worker.
        friend class Worker;
    };

    /**
     * @brief parrallel for each loop.
     * @param first first iterator.
     * @param last last iterator.
     * @param function function to execute in parallel.
     */
    template <class InputIt, class Func>
    static void parallelForEach (InputIt first, InputIt last, Func function)
    {
        ThreadPool pool;

        for (; first != last; ++first)
        {
            pool.push (function, std::ref (*first));
        }
    }
}
