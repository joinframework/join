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

// libjoin.
#include <join/reactor.hpp>
#include <join/backoff.hpp>
#include <join/utils.hpp>

// C++.
#include <array>

// C.
#include <sys/eventfd.h>
#include <unistd.h>

using join::Backoff;
using join::EventHandler;
using join::Reactor;
using join::ReactorThread;

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : Reactor
// =========================================================================
Reactor::Reactor ()
: _wakeup (eventfd (0, EFD_NONBLOCK | EFD_CLOEXEC)),
  _epoll (epoll_create1 (EPOLL_CLOEXEC))
{
    if ((_wakeup == -1) || (_epoll == -1))
    {
        int err = errno;
        ::close (_wakeup);
        ::close (_epoll);
        throw std::system_error (err, std::system_category (), "eventfd / epoll failed");
    }

    struct epoll_event ev {};
    ev.events = EPOLLIN;
    ev.data.ptr = nullptr;

    if (epoll_ctl (_epoll, EPOLL_CTL_ADD, _wakeup, &ev) == -1)
    {
        int err = errno;
        ::close (_epoll);
        ::close (_wakeup);
        throw std::system_error (err, std::system_category (), "epoll_ctl failed");
    }
}

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : ~Reactor
// =========================================================================
Reactor::~Reactor ()
{
    stop ();

    ::close (_epoll);
    ::close (_wakeup);
}

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : addHandler
// =========================================================================
int Reactor::addHandler (EventHandler* handler) noexcept
{
    if (JOIN_UNLIKELY (handler == nullptr))
    {
        lastError = make_error_code (Errc::InvalidParam);
        return -1;
    }

    if (JOIN_UNLIKELY (registerHandler (handler) == -1))
    {
        return -1;
    }

    return 0;
}

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : delHandler
// =========================================================================
int Reactor::delHandler (EventHandler* handler) noexcept
{
    if (JOIN_UNLIKELY (handler == nullptr))
    {
        lastError = make_error_code (Errc::InvalidParam);
        return -1;
    }

    if (JOIN_UNLIKELY (unregisterHandler (handler) == -1))
    {
        return -1;
    }

    return 0;
}

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : run
// =========================================================================
void Reactor::run ()
{
    _threadId.store (pthread_self (), std::memory_order_release);

    _running.store (true, std::memory_order_release);
    eventLoop ();

    _threadId.store (0, std::memory_order_release);
}

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : stop
// =========================================================================
void Reactor::stop () noexcept
{
    _running.store (false, std::memory_order_release);

    if (_threadId.load (std::memory_order_acquire) == pthread_self ())
    {
        return;
    }

    uint64_t value = 1;
    [[maybe_unused]] ssize_t bytes = ::write (_wakeup, &value, sizeof (uint64_t));

    Backoff backoff;
    while (_threadId.load (std::memory_order_acquire) != 0)
    {
        backoff ();
    }
}

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : registerHandler
// =========================================================================
int Reactor::registerHandler (EventHandler* handler) noexcept
{
    struct epoll_event ev {};
    ev.events = EPOLLIN | EPOLLRDHUP;
    ev.data.ptr = handler;

    if (JOIN_UNLIKELY (epoll_ctl (_epoll, EPOLL_CTL_ADD, handler->handle (), &ev) == -1))
    {
        lastError = std::make_error_code (static_cast <std::errc> (errno));
        return -1;
    }

    return 0;
}

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : unregisterHandler
// =========================================================================
int Reactor::unregisterHandler (EventHandler* handler) noexcept
{
    if (JOIN_UNLIKELY (epoll_ctl (_epoll, EPOLL_CTL_DEL, handler->handle (), nullptr) == -1))
    {
        lastError = std::make_error_code (static_cast <std::errc> (errno));
        return -1;
    }

    return 0;
}

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : dispatchEvent
// =========================================================================
void Reactor::dispatchEvent (const epoll_event& event)
{
    EventHandler* handler = static_cast <EventHandler*> (event.data.ptr);

    if (JOIN_UNLIKELY (event.events & EPOLLERR))
    {
        handler->onError ();
    }
    else if (JOIN_UNLIKELY (event.events & (EPOLLRDHUP | EPOLLHUP)))
    {
        handler->onClose ();
    }
    else if (JOIN_LIKELY (event.events & EPOLLIN))
    {
        handler->onReceive ();
    }
}

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : eventLoop
// =========================================================================
void Reactor::eventLoop ()
{
    std::array <epoll_event, _maxEvents> events;

    while (_running.load (std::memory_order_acquire))
    {
        int eventCount = epoll_wait (_epoll, events.data (), events.size (), -1);

        for (int i = 0; i < eventCount; ++i)
        {
            if (JOIN_UNLIKELY (events[i].data.ptr == nullptr))
            {
                uint64_t value;
                [[maybe_unused]] ssize_t bytes = ::read (_wakeup, &value, sizeof (uint64_t));
                break;
            }
            else
            {
                dispatchEvent (events[i]);
            }
        }
    }
}

// =========================================================================
//   CLASS     : ReactorThread
//   METHOD    : reactor
// =========================================================================
Reactor* ReactorThread::reactor ()
{
    return &instance ()._reactor;
}

// =========================================================================
//   CLASS     : ReactorThread
//   METHOD    : affinity
// =========================================================================
int ReactorThread::affinity (int core)
{
    return instance ()._dispatcher.affinity (core);
}

// =========================================================================
//   CLASS     : ReactorThread
//   METHOD    : affinity
// =========================================================================
int ReactorThread::affinity ()
{
    return instance ()._dispatcher.affinity ();
}

// =========================================================================
//   CLASS     : ReactorThread
//   METHOD    : priority
// =========================================================================
int ReactorThread::priority (int prio)
{
    return instance ()._dispatcher.priority (prio);
}

// =========================================================================
//   CLASS     : ReactorThread
//   METHOD    : priority
// =========================================================================
int ReactorThread::priority ()
{
    return instance ()._dispatcher.priority ();
}

// =========================================================================
//   CLASS     : ReactorThread
//   METHOD    : handle
// =========================================================================
pthread_t ReactorThread::handle ()
{
    return instance ()._dispatcher.handle ();
}

// =========================================================================
//   CLASS     : ReactorThread
//   METHOD    : instance
// =========================================================================
ReactorThread& ReactorThread::instance ()
{
    static ReactorThread reactorThread;
    return reactorThread;
}

// =========================================================================
//   CLASS     : ReactorThread
//   METHOD    : ReactorThread
// =========================================================================
ReactorThread::ReactorThread ()
{
    _dispatcher = Thread ([this] () {_reactor.run ();});
}

// =========================================================================
//   CLASS     : ReactorThread
//   METHOD    : ~ReactorThread
// =========================================================================
ReactorThread::~ReactorThread ()
{
    _reactor.stop ();
    _dispatcher.join ();
}
