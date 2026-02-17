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

// C++.
#include <functional>
#include <algorithm>

// C.
#include <sys/eventfd.h>
#include <unistd.h>
#include <sched.h>
#include <numa.h>

using join::Backoff;
using join::EventHandler;
using join::Reactor;

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : Reactor
// =========================================================================
Reactor::Reactor ()
: _wakeup (eventfd (0, EFD_NONBLOCK | EFD_CLOEXEC))
, _epoll (epoll_create1 (EPOLL_CLOEXEC))
, _commands (_queueSize)
{
    if (_wakeup == -1)
    {
        throw std::system_error (errno, std::system_category (), "eventfd failed");
    }

    if (_epoll == -1)
    {
        int err = errno;
        ::close (_wakeup);
        throw std::system_error (err, std::system_category (), "epoll_create1 failed");
    }

    _deleted.reserve (_deletedReserve);

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

    _dispatcher = Thread ([this] () { eventLoop (); });
}

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : ~Reactor
// =========================================================================
Reactor::~Reactor ()
{
    _running.store (false, std::memory_order_release);
    writeCommand ({CommandType::Stop, nullptr, nullptr, nullptr});
    _dispatcher.join ();

    ::close (_epoll);
    ::close (_wakeup);
}

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : addHandler
// =========================================================================
int Reactor::addHandler (EventHandler* handler, bool sync) noexcept
{
    if (JOIN_UNLIKELY (handler == nullptr))
    {
        lastError = make_error_code (Errc::InvalidParam);
        return -1;
    }

    if (JOIN_UNLIKELY (handler->handle () < 0))
    {
        lastError = std::make_error_code (std::errc::bad_file_descriptor);
        return -1;
    }

    if (JOIN_UNLIKELY (_dispatcher.handle () == pthread_self ()))
    {
        return registerHandler (handler);
    }

    std::atomic <bool> done {false}, *pdone = nullptr;
    std::atomic <int> errc {0}, *perrc = nullptr;

    if (JOIN_UNLIKELY (sync))
    {
        pdone = &done;
        perrc = &errc;
    }

    if (JOIN_UNLIKELY (writeCommand ({CommandType::Add, handler, pdone, perrc}) == -1))
    {
        return -1;
    }

    if (JOIN_UNLIKELY (sync))
    {
        Backoff backoff;
        while (!done.load (std::memory_order_acquire))
        {
            backoff ();
        }

        int err = errc.load (std::memory_order_acquire);
        if (JOIN_UNLIKELY (err != 0))
        {
            lastError = std::make_error_code (static_cast <std::errc> (err));
            return -1;
        }
    }

    return 0;
}

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : delHandler
// =========================================================================
int Reactor::delHandler (EventHandler* handler, bool sync) noexcept
{
    if (JOIN_UNLIKELY (handler == nullptr))
    {
        lastError = make_error_code (Errc::InvalidParam);
        return -1;
    }

    if (JOIN_UNLIKELY (handler->handle () < 0))
    {
        lastError = std::make_error_code (std::errc::bad_file_descriptor);
        return -1;
    }

    if (JOIN_UNLIKELY (_dispatcher.handle () == pthread_self ()))
    {
        return unregisterHandler (handler);
    }

    std::atomic <bool> done {false}, *pdone = nullptr;
    std::atomic <int> errc {0}, *perrc = nullptr;

    if (JOIN_UNLIKELY (sync))
    {
        pdone = &done;
        perrc = &errc;
    }

    if (JOIN_UNLIKELY (writeCommand ({CommandType::Del, handler, pdone, perrc}) == -1))
    {
        return -1;
    }

    if (JOIN_UNLIKELY (sync))
    {
        Backoff backoff;
        while (!done.load (std::memory_order_acquire))
        {
            backoff ();
        }

        int err = errc.load (std::memory_order_acquire);
        if (JOIN_UNLIKELY (err != 0))
        {
            lastError = std::make_error_code (static_cast <std::errc> (err));
            return -1;
        }
    }

    return 0;
}

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : instance
// =========================================================================
Reactor* Reactor::instance () noexcept
{
    static Reactor reactor;
    return &reactor;
}

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : registerHandler
// =========================================================================
int Reactor::registerHandler (EventHandler* handler) noexcept
{
    _deleted.erase (std::remove (_deleted.begin (), _deleted.end (), handler), _deleted.end ());

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
        _deleted.push_back (handler);
        return -1;
    }

    _deleted.push_back (handler);
    return 0;
}

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : writeCommand
// =========================================================================
int Reactor::writeCommand (const Command& cmd) noexcept
{
    if (_commands.push (cmd) == -1)
    {
        return -1;
    }

    uint64_t value = 1;
    [[maybe_unused]] ssize_t bytes = ::write (_wakeup, &value, sizeof (uint64_t));

    return 0;
}

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : processCommand
// =========================================================================
void  Reactor::processCommand (const Command& cmd)
{
    int err = 0;

    switch (cmd.type)
    {
        case CommandType::Add:
            err = registerHandler (cmd.handler);
            break;

        case CommandType::Del:
            err = unregisterHandler (cmd.handler);
            break;

        case CommandType::Stop:
            break;
    }

    if (JOIN_UNLIKELY (cmd.done))
    {
        if (cmd.errc && (err != 0))
        {
            cmd.errc->store (lastError.value (), std::memory_order_release);
        }
        cmd.done->store (true, std::memory_order_release);
    }
}

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : readCommands
// =========================================================================
void Reactor::readCommands ()
{
    uint64_t count;
    [[maybe_unused]] ssize_t bytesRead = ::read (_wakeup, &count, sizeof (count));

    Command cmd;
    while (_commands.tryPop (cmd) == 0)
    {
        processCommand (cmd);
    }
}

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : dispatchEvent
// =========================================================================
void Reactor::dispatchEvent (const epoll_event& event)
{
    EventHandler* handler = static_cast <EventHandler*> (event.data.ptr);

    if (JOIN_LIKELY (isActive (handler)))
    {
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
                readCommands ();
            }
            else
            {
                dispatchEvent (events[i]);
            }
        }

        _deleted.clear ();
    }
}

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : isActive
// =========================================================================
bool Reactor::isActive (EventHandler* handler) const noexcept
{
    return std::find (_deleted.begin (), _deleted.end (), handler) == _deleted.end ();
}
