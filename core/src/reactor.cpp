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

// C.
#include <sys/eventfd.h>
#include <unistd.h>

using join::EventHandler;
using join::Reactor;

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : Reactor
// =========================================================================
Reactor::Reactor ()
: _eventfd (eventfd (0, EFD_NONBLOCK | EFD_CLOEXEC))
, _epoll (epoll_create1 (EPOLL_CLOEXEC))
, _cmdQueue (1024)
{
    struct epoll_event ev {};
    ev.events = EPOLLIN;
    ev.data.ptr = nullptr;
    epoll_ctl (_epoll, EPOLL_CTL_ADD, _eventfd, &ev);

    _thread = Thread (std::bind (&Reactor::dispatch, this));
}

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : ~Reactor
// =========================================================================
Reactor::~Reactor ()
{
    if (_cmdQueue.push ({Command::Type::Stop, nullptr}) == 0)
    {
        notify ();
    }

    _thread.join ();

    ::close (_epoll);
    ::close (_eventfd);
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

    if (_cmdQueue.push ({Command::Type::Add, handler}) == -1)
    {
        return -1;
    }

    return notify ();
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

    if (_cmdQueue.push ({Command::Type::Del, handler}) == -1)
    {
        return -1;
    }

    return notify ();
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
//   METHOD    : dispatch
// =========================================================================
void Reactor::dispatch ()
{
    std::array <struct epoll_event, 1024> ev;

    for (;;)
    {
        int nset = epoll_wait (_epoll, ev.data (), ev.size (), -1);

        for (int n = 0; n < nset; ++n)
        {
            if (ev[n].data.ptr == nullptr)
            {
                uint64_t value;
                [[maybe_unused]] ssize_t bytes = ::read (_eventfd, &value, sizeof (uint64_t));

                Command cmd;
                while (_cmdQueue.tryPop (cmd) == 0)
                {
                    switch (cmd.type)
                    {
                        case Command::Type::Add:
                        {
                            struct epoll_event ev {};
                            ev.events = EPOLLIN | EPOLLRDHUP;
                            ev.data.ptr = cmd.handler;
                            epoll_ctl (_epoll, EPOLL_CTL_ADD, cmd.handler->handle (), &ev);
                            break;
                        }

                        case Command::Type::Del:
                        {
                            epoll_ctl (_epoll, EPOLL_CTL_DEL, cmd.handler->handle (), nullptr);
                            break;
                        }

                        case Command::Type::Stop:
                        {
                            return;
                        }
                    }
                }
            }
            else
            {
                try
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
                catch (...)
                {
                    // ignore exceptions from user handlers
                }
            }
        }
    }
}

// =========================================================================
//   CLASS     : Reactor
//   METHOD    : notify
// =========================================================================
int Reactor::notify () noexcept
{
    uint64_t value = 1;
    ssize_t bytes = ::write (_eventfd, &value, sizeof (uint64_t));

    if (bytes != sizeof (uint64_t))
    {
        lastError = std::make_error_code (static_cast <std::errc> (errno));
        return -1;
    }

    return 0;
}
