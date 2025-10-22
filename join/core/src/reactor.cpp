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

// C++.
#include <functional>
#include <thread>

// C.
#include <sys/eventfd.h>
#include <unistd.h>

using join::ScopedLock;
using join::EventHandler;
using join::Reactor;

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : Reactor
// =========================================================================
Reactor::Reactor ()
: _eventfd (eventfd (0, EFD_NONBLOCK | EFD_CLOEXEC)),
  _epoll (epoll_create1 (EPOLL_CLOEXEC))
{
    struct epoll_event ev {};
    ev.events = EPOLLIN;
    ev.data.ptr = nullptr;
    epoll_ctl (_epoll, EPOLL_CTL_ADD, _eventfd, &ev);
}

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : ~Reactor
// =========================================================================
Reactor::~Reactor ()
{
    ScopedLock <RecursiveMutex> lock (_mutex);

    if (_running)
    {
        uint64_t value = 1;
        [[maybe_unused]] ssize_t bytes = ::write (_eventfd, &value, sizeof (uint64_t));
        _threadStatus.wait (lock, [this] () { return !_running; });
    }

    ::close (_epoll);
    ::close (_eventfd);
}

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : addHandler
// =========================================================================
int Reactor::addHandler (EventHandler* handler)
{
    if (handler == nullptr)
    {
        lastError = make_error_code (Errc::InvalidParam);
        return -1;
    }

    struct epoll_event ev {};
    ev.events = EPOLLIN | EPOLLRDHUP;
    ev.data.ptr = handler;

    if (epoll_ctl (_epoll, EPOLL_CTL_ADD, handler->handle (), &ev) == -1)
    {
        lastError = std::make_error_code (static_cast <std::errc> (errno));
        return -1;
    }

    {
        ScopedLock <RecursiveMutex> lock (_mutex);

        if (++_num == 1)
        {
            // first handler, start dispatcher thread.
            std::thread th (std::bind (&Reactor::dispatch, this));
            _threadId = th.get_id ();
            th.detach ();
        }

        // wait until dispatcher is running,
        // unless we're the dispatcher thread itself.
        _threadStatus.wait (lock, [this] () {
            return (std::this_thread::get_id () == _threadId) || _running; 
        });
    }

    return 0;
}

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : delHandler
// =========================================================================
int Reactor::delHandler (EventHandler* handler)
{
    if (handler == nullptr)
    {
        lastError = make_error_code (Errc::InvalidParam);
        return -1;
    }

    if (epoll_ctl (_epoll, EPOLL_CTL_DEL, handler->handle (), nullptr) == -1)
    {
        lastError = std::make_error_code (static_cast <std::errc> (errno));
        return -1;
    }

    {
        ScopedLock <RecursiveMutex> lock (_mutex);

        if (--_num == 0)
        {
            // last handler, stop dispatcher thread.
            uint64_t value = 1;
            [[maybe_unused]] ssize_t bytes = ::write (_eventfd, &value, sizeof (uint64_t));

            // wait until dispatcher has stopped,
            // unless we're the dispatcher thread itself.
            _threadStatus.wait (lock, [this] () {
                return (std::this_thread::get_id () == _threadId) || !_running;
            });
        }
    }

    return 0;
}

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : instance
// =========================================================================
Reactor* Reactor::instance ()
{
    static Reactor reactor;
    return &reactor;
}

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : dispatch
// =========================================================================
void Reactor::dispatch ()
{
    {
        ScopedLock <RecursiveMutex> lock (_mutex);
        _running = true;
        _threadStatus.broadcast ();
    }

    std::vector <struct epoll_event> ev (256);
    bool stop = false;

    while (!stop)
    {
        int nset = epoll_wait (_epoll, ev.data (), ev.size (), -1);

        ScopedLock <RecursiveMutex> lock (_mutex);

        for (int n = 0; n < nset; ++n)
        {
            if (ev[n].data.ptr == nullptr)
            {
                uint64_t value;
                [[maybe_unused]] ssize_t bytes = ::read (_eventfd, &value, sizeof (uint64_t));
                stop = true;
                break;
            }

            if (ev[n].events & EPOLLERR)
            {
                 reinterpret_cast <EventHandler*> (ev[n].data.ptr)->onError ();
            }
            else if ((ev[n].events & EPOLLRDHUP) || (ev[n].events & EPOLLHUP))
            {
                 reinterpret_cast <EventHandler*> (ev[n].data.ptr)->onClose ();
            }
            else if (ev[n].events & EPOLLIN)
            {
                 reinterpret_cast <EventHandler*> (ev[n].data.ptr)->onReceive ();
            }
        }

        if (nset == static_cast <int> (ev.size ()))
        {
            ev.resize (ev.size () * 2);
        }
    }

    {
        ScopedLock <RecursiveMutex> lock (_mutex);
        _running = false;
        _threadStatus.broadcast ();
    }
}
