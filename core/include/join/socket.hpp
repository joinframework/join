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

#ifndef JOIN_CORE_SOCKET_HPP
#define JOIN_CORE_SOCKET_HPP

// libjoin.
#include <join/protocol.hpp>
#include <join/endpoint.hpp>
#include <join/utils.hpp>
#include <join/error.hpp>

// C++.
#include <algorithm>
#include <chrono>
#include <memory>
#include <string>

// C.
#include <linux/icmp.h>
#include <fcntl.h>
#include <poll.h>

namespace join
{
    /**
     * @brief basic socket class.
     */
    template <class Protocol>
    class BasicSocket
    {
        /// friendship with basic asynchronous raw socket
        template <class P, class E>
        friend class BasicAsyncRawSocket;

    public:
        using Ptr = std::unique_ptr<BasicSocket<Protocol>>;
        using Endpoint = typename Protocol::Endpoint;
        using TimePoint = std::chrono::steady_clock::time_point;

        /**
         * @brief socket modes.
         */
        enum Mode
        {
            Blocking,    /**< the socket will block */
            NonBlocking, /**< the socket will not block */
        };

        /**
         * @brief socket states.
         */
        enum State
        {
            Connecting,    /**< socket is connecting. */
            Connected,     /**< socket is connected. */
            Disconnecting, /**< socket is disconnecting.*/
            Disconnected,  /**< socket is disconnected.*/
            Closed,        /**< socket is closed. */
        };

        /**
         * @brief default constructor.
         */
        BasicSocket () noexcept
        : BasicSocket (Mode::NonBlocking)
        {
        }

        /**
         * @brief create socket instance specifying the mode.
         * @param mode blocking mode.
         */
        explicit BasicSocket (Mode mode) noexcept
        : _mode (mode)
        {
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicSocket (const BasicSocket& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        BasicSocket& operator= (const BasicSocket& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        BasicSocket (BasicSocket&& other) noexcept
        : _state (other._state)
        , _mode (other._mode)
        , _handle (other._handle)
        , _protocol (other._protocol)
        {
            other._state = State::Closed;
            other._mode = Mode::NonBlocking;
            other._handle = -1;
            other._protocol = Protocol ();
        }

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        BasicSocket& operator= (BasicSocket&& other) noexcept
        {
            close ();

            _state = other._state;
            _mode = other._mode;
            _handle = other._handle;
            _protocol = other._protocol;

            other._state = State::Closed;
            other._mode = Mode::NonBlocking;
            other._handle = -1;
            other._protocol = Protocol ();

            return *this;
        }

        /**
         * @brief destroy the socket instance.
         */
        virtual ~BasicSocket ()
        {
            if (_handle != -1)
            {
                ::close (_handle);
            }
        }

        /**
         * @brief open socket using the given protocol.
         * @param protocol protocol to use.
         * @return 0 on success, -1 on failure.
         */
        virtual int open (const Protocol& protocol = Protocol ()) noexcept
        {
            if (_state != State::Closed)
            {
                lastError = make_error_code (Errc::InUse);
                return -1;
            }

            if (_mode == Mode::NonBlocking)
            {
                _handle = ::socket (protocol.family (), protocol.type () | SOCK_NONBLOCK, protocol.protocol ());
            }
            else
            {
                _handle = ::socket (protocol.family (), protocol.type (), protocol.protocol ());
            }

            if (_handle == -1)
            {
                lastError = std::error_code (errno, std::generic_category ());
                close ();
                return -1;
            }

            _state = State::Disconnected;
            _protocol = protocol;

            return 0;
        }

        /**
         * @brief close the socket.
         */
        virtual void close () noexcept
        {
            if (_state != State::Closed)
            {
                ::close (_handle);
                _state = State::Closed;
                _handle = -1;
            }
        }

        /**
         * @brief assigns the specified endpoint to the socket.
         * @param endpoint endpoint to assign to the socket.
         * @return 0 on success, -1 on failure.
         */
        virtual int bind (const Endpoint& endpoint) noexcept
        {
            if ((_state == State::Closed) && (open (endpoint.protocol ()) == -1))
            {
                return -1;
            }

            if (::bind (_handle, endpoint.addr (), endpoint.length ()) == -1)
            {
                lastError = std::error_code (errno, std::generic_category ());
                return -1;
            }

            return 0;
        }

        /**
         * @brief block until new data is available for reading.
         * @return true if there is new data available for reading, false otherwise.
         */
        bool waitReadyRead () const noexcept
        {
            return (waitUntil (true, false, TimePoint::max ()) == 0);
        }

        /**
         * @brief block until new data is available for reading, giving up after the given duration.
         * @param timeout maximum time to wait.
         * @return true if there is new data available for reading, false otherwise.
         */
        bool waitReadyRead (std::chrono::nanoseconds timeout) const noexcept
        {
            return (waitUntil (true, false, std::chrono::steady_clock::now () + timeout) == 0);
        }

        /**
         * @brief block until new data is available for reading, giving up at the given time point.
         * @param deadline time point at which to give up, max to wait indefinitely.
         * @return true if there is new data available for reading, false otherwise.
         */
        bool waitReadyRead (TimePoint deadline) const noexcept
        {
            return (waitUntil (true, false, deadline) == 0);
        }

        /**
         * @brief block until at least one byte can be written.
         * @return true if data can be written, false otherwise.
         */
        bool waitReadyWrite () const noexcept
        {
            return (waitUntil (false, true, TimePoint::max ()) == 0);
        }

        /**
         * @brief block until at least one byte can be written, giving up after the given duration.
         * @param timeout maximum time to wait.
         * @return true if data can be written, false otherwise.
         */
        bool waitReadyWrite (std::chrono::nanoseconds timeout) const noexcept
        {
            return (waitUntil (false, true, std::chrono::steady_clock::now () + timeout) == 0);
        }

        /**
         * @brief block until at least one byte can be written, giving up at the given time point.
         * @param deadline time point at which to give up, max to wait indefinitely.
         * @return true if data can be written, false otherwise.
         */
        bool waitReadyWrite (TimePoint deadline) const noexcept
        {
            return (waitUntil (false, true, deadline) == 0);
        }

        /**
         * @brief set the socket to the non-blocking or blocking mode.
         * @param mode blocking mode.
         */
        void setMode (Mode mode) noexcept
        {
            _mode = mode;

            if (_state != State::Closed)
            {
                int flags = ::fcntl (_handle, F_GETFL, 0);

                if (_mode == Mode::NonBlocking)
                {
                    flags = flags | O_NONBLOCK;
                }
                else
                {
                    flags = flags & ~O_NONBLOCK;
                }

                ::fcntl (_handle, F_SETFL, flags);
            }
        }

        /**
         * @brief determine the local endpoint associated with this socket.
         * @return local endpoint.
         */
        Endpoint localEndpoint () const noexcept
        {
            struct sockaddr_storage sa;
            socklen_t sa_len = sizeof (struct sockaddr_storage);

            if (::getsockname (_handle, reinterpret_cast<struct sockaddr*> (&sa), &sa_len) == -1)
            {
                return {};
            }

            return Endpoint (reinterpret_cast<struct sockaddr*> (&sa), sa_len);
        }

        /**
         * @brief check if the socket is opened.
         * @return true if opened, false otherwise.
         */
        bool opened () const noexcept
        {
            return (_state != State::Closed);
        }

        /**
         * @brief get socket address family.
         * @return socket address family.
         */
        int family () const noexcept
        {
            return _protocol.family ();
        }

        /**
         * @brief get the protocol communication semantic.
         * @return the protocol communication semantic.
         */
        int type () const noexcept
        {
            return _protocol.type ();
        }

        /**
         * @brief get socket protocol.
         * @return socket protocol.
         */
        int protocol () const noexcept
        {
            return _protocol.protocol ();
        }

        /**
         * @brief get the blocking mode of the socket.
         * @return the blocking mode of the socket.
         */
        Mode mode () const noexcept
        {
            return _mode;
        }

        /**
         * @brief get socket native handle.
         * @return socket native handle.
         */
        int handle () const noexcept
        {
            return _handle;
        }

        /**
         * @brief wait for the socket handle to become ready.
         * @param wantRead set to true if want read
         * @param wantWrite set to true if want write.
         * @return 0 on success, -1 on failure.
         */
        int wait (bool wantRead, bool wantWrite) const noexcept
        {
            return waitUntil (wantRead, wantWrite, TimePoint::max ());
        }

        /**
         * @brief wait for the socket handle to become ready, giving up after the given duration.
         * @param wantRead set to true if want read
         * @param wantWrite set to true if want write.
         * @param timeout maximum time to wait.
         * @return 0 on success, -1 on failure.
         */
        int waitFor (bool wantRead, bool wantWrite, std::chrono::nanoseconds timeout) const noexcept
        {
            return waitUntil (wantRead, wantWrite, std::chrono::steady_clock::now () + timeout);
        }

        /**
         * @brief wait for the socket handle to become ready, giving up at the given time point.
         * @param wantRead set to true if want read
         * @param wantWrite set to true if want write.
         * @param deadline time point at which to give up, max to wait indefinitely.
         * @return 0 on success, -1 on failure.
         */
        int waitUntil (bool wantRead, bool wantWrite, TimePoint deadline) const noexcept
        {
            struct pollfd handle;
            handle.fd = _handle;
            handle.events = 0;
            handle.revents = 0;

            if (wantRead)
            {
                handle.events |= POLLIN;
            }

            if (wantWrite)
            {
                handle.events |= POLLOUT;
            }

            struct timespec ts = {};
            const struct timespec* remaining = nullptr;

            if (deadline != TimePoint::max ())
            {
                ts = toTimespec (std::max (deadline - std::chrono::steady_clock::now (), TimePoint::duration::zero ()));
                remaining = &ts;
            }

            int nset = (handle.fd > -1) ? ::ppoll (&handle, 1, remaining, nullptr) : -1;
            if (nset != 1)
            {
                if (nset == -1)
                {
                    if (handle.fd == -1)
                    {
                        errno = EBADF;
                    }
                    lastError = std::error_code (errno, std::generic_category ());
                }
                else
                {
                    lastError = make_error_code (Errc::TimedOut);
                }

                return -1;
            }

            return 0;
        }

    protected:
        /// socket state.
        State _state = State::Closed;

        /// socket mode.
        Mode _mode = Mode::NonBlocking;

        /// socket handle.
        int _handle = -1;

        /// protocol.
        Protocol _protocol;
    };

    /**
     * @brief compare if handle is inferior.
     * @param a socket handle to compare.
     * @param b socket handle to compare to.
     * @return true if inferior.
     */
    template <class Protocol>
    inline bool operator< (const BasicSocket<Protocol>& a, const BasicSocket<Protocol>& b) noexcept
    {
        return a.handle () < b.handle ();
    }
}

#endif
