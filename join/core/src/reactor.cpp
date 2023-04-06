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
//   METHOD    : instance
// =========================================================================
Reactor* Reactor::instance ()
{
    static Reactor reactor;
    return &reactor;
}

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : addHandler
// =========================================================================
int Reactor::addHandler (EventHandler* handler)
{
    ScopedLock lk (_mutex);

    if (!handler)
    {
        lastError = make_error_code (Errc::InvalidParam);
        return -1;
    }

    if (_event == -1)
    {
        _event = eventfd (0, EFD_NONBLOCK | EFD_CLOEXEC);
        if (_event == -1)
        {
            lastError = std::make_error_code (static_cast <std::errc> (errno));
            return -1;
        }
    }

    if (_epoll == -1)
    {
        _epoll = epoll_create1 (EPOLL_CLOEXEC);
        if (_epoll == -1)
        {
            lastError = std::make_error_code (static_cast <std::errc> (errno));
            return -1;
        }
    }

    auto status = _handlers.emplace (handler->handle (), handler);
    if (status.second == false)
    {
        lastError = make_error_code (Errc::InUse);
        return -1;
    }

    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLRDHUP;
    ev.data.fd = handler->handle ();

    if (epoll_ctl (_epoll, EPOLL_CTL_ADD, ev.data.fd, &ev) == -1)
    {
        lastError = std::make_error_code (static_cast <std::errc> (errno));
        return -1;
    }

    if (_running == false)
    {
        try
        {
            std::thread (std::bind (&Reactor::dispatch, this)).detach ();
            _running = true;
        }
        catch (const std::system_error& err)
        {
            lastError = err.code ();
            return -1;
        }
    }

    return 0;
}

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : delHandler
// =========================================================================
int Reactor::delHandler (EventHandler* handler)
{
    ScopedLock lk (_mutex);

    if (!handler)
    {
        lastError = make_error_code (Errc::InvalidParam);
        return -1;
    }

    if (_handlers.erase (handler->handle ()) == 0)
    {
        lastError = make_error_code (Errc::InvalidParam);
        return -1;
    }

    if (epoll_ctl (_epoll, EPOLL_CTL_DEL, handler->handle (), nullptr) == -1)
    {
        lastError = std::make_error_code (static_cast <std::errc> (errno));
        return -1;
    }

    if (_handlers.empty ())
    {
        uint64_t value = 1;
        [[maybe_unused]] ssize_t bytes = ::write (_event, &value, sizeof (uint64_t));
    }

    return 0;
}

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : Reactor
// =========================================================================
Reactor::Reactor ()
: _ev (32)
{
}

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : ~Reactor
// =========================================================================
Reactor::~Reactor ()
{
    ScopedLock lk (_mutex);

    if (_running)
    {
        uint64_t value = 1;
        [[maybe_unused]] ssize_t bytes = ::write (_event, &value, sizeof (uint64_t));

        _end.wait (lk, [this] () {
            return !_running;
        });
    }

    _handlers.clear ();

    ::close (_event);
    ::close (_epoll);
}

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : dispatch
// =========================================================================
void Reactor::dispatch ()
{
    _mutex.lock ();

    fd_set setfd;
    FD_ZERO (&setfd);
    FD_SET (_event, &setfd);
    FD_SET (_epoll, &setfd);
    int maxdesc = std::max (_event, _epoll);

    for (;;)
    {
        fd_set descset = setfd;

        _mutex.unlock ();
        int nset = ::select (maxdesc + 1, &descset, nullptr, nullptr, nullptr);
        _mutex.lock ();

        if (nset > 0)
        {
            if (FD_ISSET (_event, &descset))
            {
                uint64_t value;
                [[maybe_unused]] ssize_t bytes = ::read (_event, &value, sizeof (uint64_t));
                break;
            }

            if (FD_ISSET (_epoll, &descset))
            {
                nset = epoll_wait (_epoll, _ev.data (), _ev.size (), 0);
                for (int n = 0; n < nset; ++n)
                {
                    auto it = _handlers.find (_ev[n].data.fd);
                    if (it != _handlers.end ())
                    {
                        if (_ev[n].events & EPOLLERR)
                        {
                            it->second->onError ();
                        }
                        else if ((_ev[n].events & EPOLLRDHUP) || (_ev[n].events & EPOLLHUP))
                        {
                            it->second->onClose ();
                        }
                        else if (_ev[n].events & EPOLLIN)
                        {
                            it->second->onReceive ();
                        }
                    }
                }
            }
        }
    }

    _running = false;
    _end.signal ();

    _mutex.unlock ();
}
