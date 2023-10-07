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

#ifndef __JOIN_THREADPOOL_HPP__
#define __JOIN_THREADPOOL_HPP__

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
     * @brief worker thread class.
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
        ThreadPool* _pool = nullptr;

        /// thread.
        Thread _thread;
    };

    /**
     * @brief thread pool class.
     */
    class ThreadPool
    {
    public:
        /**
         * @brief create thread pool.
         * @param workers number of worker threads.
         */
        ThreadPool (int workers = std::max (int (std::thread::hardware_concurrency ()), 1));

        /**
         * @brief destroy thread pool.
         */
        ~ThreadPool ();

        /**
         * @brief return thread pool size.
         */
        int size ();

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
     * @brief determine the number of threads and tasks per thread to run and execute them.
     * @param first first iterator.
     * @param last last iterator.
     * @param function function to run in parallel.
     */
    template <class InputIt, class Func>
    void dispatch (InputIt first, InputIt last, Func function)
    {
        // determine number of threads and task per thread to run.
        int concurrency = std::max (int (std::thread::hardware_concurrency ()), 1);
        int count       = std::distance (first, last);
        int elements    = count / concurrency;
        int rest        = count % concurrency;

        std::vector <int> tasks (concurrency, elements);
        for (int i = 0; i < rest; ++i)
        {
            tasks[i]++;
        }

        // determine the real thread pool size (concurrency minus 1 as we are a thread).
        std::vector <std::thread> pool;
        int nth = concurrency - 1;
        pool.reserve (nth);

        // create threads.
        auto beg = first, end = first;
        for (int i = 0; i < nth; ++i)
        {
            std::advance (end, tasks[i]);
            pool.emplace_back (function, beg, end);
            beg = end;
        }

        // we are a thread so we can help.
        function (beg, last);

        // wait for threads to terminate.
        for (auto& thread : pool)
        {
            thread.join ();
        }
    }

    /**
     * @brief parrallel for each loop.
     * @param first first iterator.
     * @param last last iterator.
     * @param function function to execute in parallel.
     */
    template <class InputIt, class Func>
    void parallelForEach (InputIt first, InputIt last, Func function)
    {
        // task executed by threads.
        auto task = [&function] (InputIt beg, InputIt end)
        {
            for (; beg != end; ++beg)
            {
                function (*beg);
            }
        };

        // execute tasks.
        dispatch (first, last, task);
    }
}

#endif
