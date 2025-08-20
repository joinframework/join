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
}

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : ~Reactor
// =========================================================================
Reactor::~Reactor ()
{
    ScopedLock lock (_mutex);

    if (_running)
    {
        uint64_t value = 1;
        [[maybe_unused]] ssize_t bytes = ::write (_eventfd, &value, sizeof (uint64_t));
        _end.wait (lock, [this] () { return !_running; });
    }

    ::close (_eventfd);
    ::close (_epoll);
}

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : addHandler
// =========================================================================
int Reactor::addHandler (EventHandler* handler)
{
    ScopedLock lock (_mutex);

    if (handler == nullptr)
    {
        lastError = make_error_code (Errc::InvalidParam);
        return -1;
    }

    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLRDHUP;
    ev.data.ptr = handler;

    if (epoll_ctl (_epoll, EPOLL_CTL_ADD, handler->handle (), &ev) == -1)
    {
        lastError = std::make_error_code (static_cast <std::errc> (errno));
        return -1;
    }

    if (++_num == 1)
    {
        std::thread (std::bind (&Reactor::dispatch, this)).detach ();
        _running = true;
    }

    return 0;
}

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : delHandler
// =========================================================================
int Reactor::delHandler (EventHandler* handler)
{
    ScopedLock lock (_mutex);

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

    if (--_num == 0)
    {
        uint64_t value = 1;
        [[maybe_unused]] ssize_t bytes = ::write (_eventfd, &value, sizeof (uint64_t));
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
    _mutex.lock ();

    std::vector <struct epoll_event> ev (32);

    fd_set setfd;
    FD_ZERO (&setfd);
    FD_SET (_eventfd, &setfd);
    FD_SET (_epoll, &setfd);
    int max = std::max (_eventfd, _epoll);

    for (;;)
    {
        fd_set descset = setfd;

        _mutex.unlock ();
        int nset = ::select (max + 1, &descset, nullptr, nullptr, nullptr);
        _mutex.lock ();

        if (nset > 0)
        {
            if (FD_ISSET (_eventfd, &descset))
            {
                uint64_t value;
                [[maybe_unused]] ssize_t bytes = ::read (_eventfd, &value, sizeof (uint64_t));
                break;
            }

            if (FD_ISSET (_epoll, &descset))
            {
                nset = epoll_wait (_epoll, ev.data (), ev.size (), 0);
                for (int n = 0; n < nset; ++n)
                {
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
            }
        }
    }

    _running = false;
    _end.signal ();

    _mutex.unlock ();
}
