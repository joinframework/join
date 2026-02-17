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
#include <join/thread.hpp>
#include <join/error.hpp>

// C.
#include <signal.h>

using join::Invoker;
using join::Thread;

// =========================================================================
//   CLASS     : Invoker
//   METHOD    : _routine
// =========================================================================
void * Invoker::_routine (void * context)
{
    return (static_cast <Invoker *> (context))->routine ();
}

// =========================================================================
//   CLASS     : Invoker
//   METHOD    : routine
// =========================================================================
void * Invoker::routine (void)
{
    _func ();
    _done.store (true, std::memory_order_release);
    return nullptr;
}

// =========================================================================
//   CLASS     : Thread
//   METHOD    : Thread
// =========================================================================
Thread::Thread (Thread&& other) noexcept
: _invoker (std::move (other._invoker))
, _core (other._core)
, _priority (other._priority)
{
    other._core = -1;
    other._priority = 0;
}

// =========================================================================
//   CLASS     : Thread
//   METHOD    : Thread
// =========================================================================
Thread& Thread::operator= (Thread&& other) noexcept
{
    cancel ();

    _invoker = std::move (other._invoker);
    _core = other._core;
    _priority = other._priority;

    other._core = -1;
    other._priority = 0;

    return *this;
}

// =========================================================================
//   CLASS     : Thread
//   METHOD    : ~Thread
// =========================================================================
Thread::~Thread ()
{
    cancel ();
}

// =========================================================================
//   CLASS     : Thread
//   METHOD    : affinity
// =========================================================================
int Thread::affinity (int core)
{
    if (!joinable ())
    {
        lastError = std::make_error_code (std::errc::no_such_process);
        return -1;
    }

    if (affinity (_invoker->_handle, core) == -1)
    {
        return -1;
    }

    _core = (core < 0) ? -1 : core;

    return 0;
}

// =========================================================================
//   CLASS     : Thread
//   METHOD    : affinity
// =========================================================================
int Thread::affinity (pthread_t handle, int core)
{
    cpu_set_t cpuset;
    CPU_ZERO (&cpuset);

    if (core < 0)
    {
        int ncpu = sysconf (_SC_NPROCESSORS_ONLN);
        for (int i = 0; i < ncpu; ++i)
        {
            CPU_SET (i, &cpuset);
        }
    }
    else
    {
        CPU_SET (core, &cpuset);
    }

    int err = pthread_setaffinity_np (handle, sizeof (cpu_set_t), &cpuset);
    if (err != 0)
    {
        lastError = std::error_code (err, std::generic_category ());
        return -1;
    }

    return 0;
}

// =========================================================================
//   CLASS     : Thread
//   METHOD    : affinity
// =========================================================================
int Thread::affinity () const noexcept
{
    return _core;
}

// =========================================================================
//   CLASS     : Thread
//   METHOD    : priority
// =========================================================================
int Thread::priority (int prio)
{
    if (!joinable ())
    {
        lastError = std::make_error_code (std::errc::no_such_process);
        return -1;
    }

    if (priority (_invoker->_handle, prio) == -1)
    {
        return -1;
    }

    _priority = prio;

    return 0;
}


// =========================================================================
//   CLASS     : Thread
//   METHOD    : priority
// =========================================================================
int Thread::priority (pthread_t handle, int prio)
{
    struct sched_param param {};
    param.sched_priority = prio;

    if (prio == 0)
    {
        int err = pthread_setschedparam (handle, SCHED_OTHER, &param);
        if (err != 0)
        {
            lastError = std::error_code (err, std::generic_category ());
            return -1;
        }
    }
    else
    {
        int err = pthread_setschedparam (handle, SCHED_FIFO, &param);
        if (err != 0)
        {
            lastError = std::error_code (err, std::generic_category ());
            return -1;
        }
    }

    return 0;
}

// =========================================================================
//   CLASS     : Thread
//   METHOD    : priority
// =========================================================================
int Thread::priority () const noexcept
{
    return _priority;
}

// =========================================================================
//   CLASS     : Thread
//   METHOD    : joinable
// =========================================================================
bool Thread::joinable () const noexcept
{
    return (_invoker != nullptr);
}

// =========================================================================
//   CLASS     : Thread
//   METHOD    : running
// =========================================================================
bool Thread::running () const noexcept
{
    return (joinable () && !_invoker->_done.load (std::memory_order_acquire));
}

// =========================================================================
//   CLASS     : Thread
//   METHOD    : join
// =========================================================================
void Thread::join () noexcept
{
    if (joinable ())
    {
        pthread_join (_invoker->_handle, nullptr);
        _invoker.reset ();
    }
}

// =========================================================================
//   CLASS     : Thread
//   METHOD    : tryJoin
// =========================================================================
bool Thread::tryJoin () noexcept 
{
    if (running ())
    {
        return false;
    }
    join ();
    return true;
}

// =========================================================================
//   CLASS     : Thread
//   METHOD    : cancel
// =========================================================================
void Thread::cancel () noexcept
{
    if (running ())
    {
        pthread_cancel (_invoker->_handle);
    }
    join ();
}

// =========================================================================
//   CLASS     : Thread
//   METHOD    : swap
// =========================================================================
void Thread::swap (Thread& other) noexcept
{
    std::swap (_invoker, other._invoker);
    std::swap (_core, other._core);
    std::swap (_priority, other._priority);
}

// =========================================================================
//   CLASS     : Thread
//   METHOD    : handle
// =========================================================================
pthread_t Thread::handle () const noexcept
{
    return joinable () ? _invoker->_handle : pthread_t {};
}
