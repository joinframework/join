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

// C.
#include <signal.h>

using join::Invoker;
using join::Thread;

// =========================================================================
//   CLASS     : Invoker
//   METHOD    : ~Invoker
// =========================================================================
Invoker::~Invoker ()
{
    pthread_attr_destroy (&_attr);
}

// =========================================================================
//   CLASS     : Invoker
//   METHOD    : handle
// =========================================================================
pthread_t Invoker::handle ()
{
    return _handle;
}

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
    _done = true;
    return nullptr;
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
    return (joinable () && !_invoker->_done);
}

// =========================================================================
//   CLASS     : Thread
//   METHOD    : join
// =========================================================================
void Thread::join ()
{
    if (joinable ())
    {
        pthread_join (_invoker->handle (), nullptr);
        _invoker.reset ();
    }
}

// =========================================================================
//   CLASS     : Thread
//   METHOD    : tryJoin
// =========================================================================
bool Thread::tryJoin ()
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
void Thread::cancel ()
{
    if (running ())
    {
        pthread_cancel (_invoker->handle ());
    }
    join ();
}

// =========================================================================
//   CLASS     : Thread
//   METHOD    : swap
// =========================================================================
void Thread::swap (Thread& other)
{
    std::swap (_invoker, other._invoker);
}
