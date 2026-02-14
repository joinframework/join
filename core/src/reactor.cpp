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
    _deleted.reserve (_deletedReserve);

    struct epoll_event ev {};
    ev.events = EPOLLIN;
    ev.data.ptr = nullptr;
    epoll_ctl (_epoll, EPOLL_CTL_ADD, _wakeup, &ev);

    _dispatcher = Thread (std::bind (&Reactor::eventLoop, this));
}

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : ~Reactor
// =========================================================================
Reactor::~Reactor ()
{
    if (_commands.push ({ CommandType::Stop, nullptr, nullptr }) == 0)
    {
        wakeDispatcher ();
    }

    _dispatcher.join ();

    ::close (_epoll);
    ::close (_wakeup);
}

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : addHandler
// =========================================================================
int Reactor::addHandler (EventHandler* handler) noexcept
{
    if (handler == nullptr)
    {
        lastError = make_error_code (Errc::InvalidParam);
        return -1;
    }

    if (handler->handle () < 0)
    {
        lastError = std::make_error_code (std::errc::bad_file_descriptor);
        return -1;
    }

    if (_commands.push ({ CommandType::Add, handler, nullptr }) == -1)
    {
        return -1;
    }

    wakeDispatcher ();

    return 0;
}

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : delHandler
// =========================================================================
int Reactor::delHandler (EventHandler* handler) noexcept
{
    if (handler == nullptr)
    {
        lastError = make_error_code (Errc::InvalidParam);
        return -1;
    }

    if (handler->handle () < 0)
    {
        lastError = std::make_error_code (std::errc::bad_file_descriptor);
        return -1;
    }

    if (_dispatcher.id () == pthread_self ())
    {
        unregisterHandler (handler);
        return 0;
    }

    std::atomic <bool> done {false};

    if (_commands.push ({ CommandType::Del, handler, &done }) == -1)
    {
        return -1;
    }

    wakeDispatcher ();

    Backoff backoff;
    while (!done.load (std::memory_order_acquire))
    {
        backoff ();
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
void Reactor::registerHandler (EventHandler* handler)
{
    _deleted.erase (std::remove (_deleted.begin (), _deleted.end (), handler), _deleted.end ());
    struct epoll_event ev {};
    ev.events = EPOLLIN | EPOLLRDHUP;
    ev.data.ptr = handler;
    epoll_ctl (_epoll, EPOLL_CTL_ADD, handler->handle (), &ev);
}

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : unregisterHandler
// =========================================================================
void Reactor::unregisterHandler (EventHandler* handler)
{
    epoll_ctl (_epoll, EPOLL_CTL_DEL, handler->handle (), nullptr);
    _deleted.push_back (handler);
}

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : processCommands
// =========================================================================
bool Reactor::processCommands ()
{
    uint64_t count;
    [[maybe_unused]] ssize_t bytesRead = ::read (_wakeup, &count, sizeof (count));
    Command cmd;

    while (_commands.tryPop (cmd) == 0)
    {
        switch (cmd.type)
        {
            case CommandType::Add:
                registerHandler (cmd.handler);
                break;

            case CommandType::Del:
                unregisterHandler (cmd.handler);
                break;

            case CommandType::Stop:
                return true;
        }

        if (cmd.done)
        {
            cmd.done->store (true, std::memory_order_release);
        }
    }

    return false;
}

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : dispatchEvent
// =========================================================================
void Reactor::dispatchEvent (const epoll_event& event)
{
    EventHandler* handler = static_cast <EventHandler*> (event.data.ptr);

    if (isActive (handler))
    {
        if (event.events & EPOLLERR)
        {
            handler->onError ();
        }
        else if (event.events & (EPOLLRDHUP | EPOLLHUP))
        {
            handler->onClose ();
        }
        else if (event.events & EPOLLIN)
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

    for (;;)
    {
        int eventCount = epoll_wait (_epoll, events.data (), events.size (), -1);

        for (int i = 0; i < eventCount; ++i)
        {
            if (events[i].data.ptr == nullptr)
            {
                if (processCommands ())
                {
                    return;
                }
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
//   METHOD    : wakeDispatcher
// =========================================================================
void Reactor::wakeDispatcher () noexcept
{
    uint64_t value = 1;
    [[maybe_unused]] ssize_t bytes = ::write (_wakeup, &value, sizeof (uint64_t));
}

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : isActive
// =========================================================================
bool Reactor::isActive (EventHandler* handler) const noexcept
{
    return std::find (_deleted.begin (), _deleted.end (), handler) == _deleted.end ();
}
