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
#include <sys/mman.h>
#include <signal.h>
#include <unistd.h>

using join::Invoker;
using join::Thread;

constexpr pid_t Thread::invalidId;

// =========================================================================
//   CLASS     : Invoker
//   METHOD    : _routine
// =========================================================================
void* Invoker::_routine (void* context)
{
    Invoker* self = static_cast<Invoker*> (context);

    self->_id.store (Thread::currentId (), std::memory_order_release);

    if (self->_core.load (std::memory_order_relaxed) != -1)
    {
        Thread::affinity (pthread_self (), self->_core.load (std::memory_order_relaxed));
    }

    if (self->_prefault)
    {
        prefault ();
    }

    if (self->_priority.load (std::memory_order_relaxed) > 0)
    {
        Thread::priority (pthread_self (), self->_priority.load (std::memory_order_relaxed));
    }

    return self->routine ();
}

// =========================================================================
//   CLASS     : Invoker
//   METHOD    : routine
// =========================================================================
void* Invoker::routine ()
{
    if (_func)
    {
        _func ();
    }

    _done.store (true, std::memory_order_release);

    return nullptr;
}

// =========================================================================
//   CLASS     : Invoker
//   METHOD    : prefault
// =========================================================================
int Invoker::prefault () noexcept
{
    pthread_attr_t attr;
    const int err = pthread_getattr_np (pthread_self (), &attr);
    if (err != 0)
    {
        // LCOV_EXCL_START
        lastError = std::error_code (err, std::generic_category ());
        return -1;
        // LCOV_EXCL_STOP
    }

    void* addr = nullptr;
    size_t size = 0;
    pthread_attr_getstack (&attr, &addr, &size);
    pthread_attr_destroy (&attr);

    if (::madvise (addr, size, MADV_POPULATE_WRITE) == -1)
    {
        // LCOV_EXCL_START
        lastError = std::error_code (errno, std::generic_category ());
        return -1;
        // LCOV_EXCL_STOP
    }

    return 0;
}

// =========================================================================
//   CLASS     : Thread
//   METHOD    : Thread
// =========================================================================
Thread::Thread (Thread&& other) noexcept
: _invoker (std::move (other._invoker))
{
}

// =========================================================================
//   CLASS     : Thread
//   METHOD    : Thread
// =========================================================================
Thread& Thread::operator= (Thread&& other) noexcept
{
    cancel ();
    _invoker = std::move (other._invoker);
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

    _invoker->_core.store ((core < 0) ? -1 : core, std::memory_order_relaxed);

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
    return _invoker ? _invoker->_core.load (std::memory_order_relaxed) : -1;
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

    _invoker->_priority.store (prio, std::memory_order_relaxed);

    return 0;
}

// =========================================================================
//   CLASS     : Thread
//   METHOD    : priority
// =========================================================================
int Thread::priority (pthread_t handle, int prio)
{
    struct sched_param param{};

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
    return _invoker ? _invoker->_priority.load (std::memory_order_relaxed) : 0;
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
}

// =========================================================================
//   CLASS     : Thread
//   METHOD    : id
// =========================================================================
pid_t Thread::id () const noexcept
{
    return joinable () ? _invoker->_id.load (std::memory_order_acquire) : invalidId;
}

// =========================================================================
//   CLASS     : Thread
//   METHOD    : handle
// =========================================================================
pthread_t Thread::handle () const noexcept
{
    return joinable () ? _invoker->_handle : pthread_t{};
}
