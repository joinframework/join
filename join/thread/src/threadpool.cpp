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
#include <join/threadpool.hpp>

using join::WorkerThread;
using join::ThreadPool;

// =========================================================================
//   CLASS     : WorkerThread
//   METHOD    : WorkerThread
// =========================================================================
WorkerThread::WorkerThread (ThreadPool& pool)
: _pool (std::addressof (pool)),
  _thread ([this] () {work ();})
{
}

// =========================================================================
//   CLASS     : WorkerThread
//   METHOD    : ~WorkerThread
// =========================================================================
WorkerThread::~WorkerThread ()
{
    _thread.join ();
}

// =========================================================================
//   CLASS     : WorkerThread
//   METHOD    : work
// =========================================================================
void WorkerThread::work ()
{
    for (;;)
    {
        std::function <void ()> func;
        {
            ScopedLock lock (_pool->_mutex);
            _pool->_condition.wait (lock, [&] () {return _pool->_stop || !_pool->_jobs.empty ();});
            if (_pool->_stop && _pool->_jobs.empty ())
            {
                return;
            }
            func = std::move (_pool->_jobs.front ());
            _pool->_jobs.pop_front ();
        }
        func ();
    }
}

// =========================================================================
//   CLASS     : ThreadPool
//   METHOD    : ThreadPool
// =========================================================================
ThreadPool::ThreadPool (int workers)
: _stop (false)
{
    for (int nworkers = 0; nworkers < workers; ++nworkers)
    {
        _workers.emplace_back (new WorkerThread (*this));
    }
}

// =========================================================================
//   CLASS     : ThreadPool
//   METHOD    : ~ThreadPool
// =========================================================================
ThreadPool::~ThreadPool ()
{
    _stop = true;
    _condition.broadcast ();
    _workers.clear ();
}

// =========================================================================
//   CLASS     : ThreadPool
//   METHOD    : size
// =========================================================================
size_t ThreadPool::size ()
{
    ScopedLock lock (_mutex);
    return _workers.size ();
}
