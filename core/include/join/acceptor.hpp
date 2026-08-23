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

#ifndef JOIN_CORE_ACCEPTOR_HPP
#define JOIN_CORE_ACCEPTOR_HPP

// libjoin.
#include <join/socket_stream.hpp>

namespace join
{
    /**
     * @brief basic stream acceptor class.
     */
    template <class Protocol>
    class BasicStreamAcceptor
    {
    public:
        using Endpoint = typename Protocol::Endpoint;
        using Socket = typename Protocol::Socket;
        using Stream = typename Protocol::Stream;

        /**
         * @brief create the acceptor instance.
         */
        BasicStreamAcceptor () = default;

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicStreamAcceptor (const BasicStreamAcceptor& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicStreamAcceptor& operator= (const BasicStreamAcceptor& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        BasicStreamAcceptor (BasicStreamAcceptor&& other)
        : _handle (other._handle)
        , _protocol (other._protocol)
        {
            other._handle = -1;
            other._protocol = Protocol ();
        }

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicStreamAcceptor& operator= (BasicStreamAcceptor&& other)
        {
            close ();

            _handle = other._handle;
            _protocol = other._protocol;

            other._handle = -1;
            other._protocol = Protocol ();

            return *this;
        }

        /**
         * @brief destroy instance.
         */
        ~BasicStreamAcceptor ()
        {
            close ();
        }

        /**
         * @brief create acceptor
         * @param endpoint endpoint to assign to the acceptor.
         * @param flags acceptor socket creation flags.
         * @return 0 on success, -1 on failure.
         */
        int create (const Endpoint& endpoint, int flags = SOCK_CLOEXEC) noexcept
        {
            if (opened ())
            {
                lastError = make_error_code (Errc::InUse);
                return -1;
            }

            _handle = ::socket (endpoint.protocol ().family (), endpoint.protocol ().type () | flags,
                                endpoint.protocol ().protocol ());
            if (_handle == -1)
            {
                // LCOV_EXCL_START
                lastError = std::error_code (errno, std::generic_category ());
                close ();
                return -1;
                // LCOV_EXCL_STOP
            }

            if (endpoint.protocol ().family () == AF_INET6)
            {
                int off = 0;

                if (::setsockopt (_handle, IPPROTO_IPV6, IPV6_V6ONLY, &off, sizeof (off)) == -1)
                {
                    // LCOV_EXCL_START
                    lastError = std::error_code (errno, std::generic_category ());
                    close ();
                    return -1;
                    // LCOV_EXCL_STOP
                }
            }

            if (endpoint.protocol ().family () == AF_UNIX)
            {
                ::unlink (endpoint.device ().c_str ());
            }
            else if (endpoint.protocol ().protocol () == IPPROTO_TCP)
            {
                int on = 1;

                if (::setsockopt (_handle, SOL_SOCKET, SO_REUSEADDR, &on, sizeof (on)) == -1)
                {
                    // LCOV_EXCL_START
                    lastError = std::error_code (errno, std::generic_category ());
                    close ();
                    return -1;
                    // LCOV_EXCL_STOP
                }
            }

            if ((::bind (_handle, endpoint.addr (), endpoint.length ()) == -1) || (::listen (_handle, SOMAXCONN) == -1))
            {
                lastError = std::error_code (errno, std::generic_category ());
                close ();
                return -1;
            }

            _protocol = endpoint.protocol ();

            return 0;
        }

        /**
         * @brief close acceptor.
         */
        void close () noexcept
        {
            if (_handle != -1)
            {
                ::close (_handle);
                _handle = -1;
            }

            _protocol = Protocol ();
        }

        /**
         * @brief accept new connection and fill in the client object with connection parameters.
         * @param flags accepted socket creation flags.
         * @return the accepted client socket object.
         */
        Socket accept (int flags = SOCK_NONBLOCK | SOCK_CLOEXEC) const
        {
            struct sockaddr_storage sa;
            socklen_t sa_len = sizeof (struct sockaddr_storage);

            int fd = ::accept4 (_handle, reinterpret_cast<struct sockaddr*> (&sa), &sa_len, flags);
            if (fd == -1)
            {
                // LCOV_EXCL_START
                lastError = std::error_code (errno, std::generic_category ());
                return {};
                // LCOV_EXCL_STOP
            }

            return Socket (fd, Endpoint (reinterpret_cast<struct sockaddr*> (&sa), sa_len),
                           (flags & SOCK_NONBLOCK) ? Socket::NonBlocking : Socket::Blocking);
        }

        /**
         * @brief accept new connection and fill in the client object with connection parameters.
         * @return The client stream object on success, nullptr on failure.
         */
        Stream acceptStream () const
        {
            Stream stream;
            stream.socket () = accept ();
            return stream;
        }

        /**
         * @brief determine the local endpoint associated with this socket.
         * @return local endpoint.
         */
        Endpoint localEndpoint () const
        {
            struct sockaddr_storage sa;
            socklen_t sa_len = sizeof (struct sockaddr_storage);

            if (::getsockname (_handle, reinterpret_cast<struct sockaddr*> (&sa), &sa_len) == -1)
            {
                lastError = std::error_code (errno, std::generic_category ());
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
            return (_handle != -1);
        }

        /**
         * @brief get address family.
         * @return address family.
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
         * @brief get acceptor protocol.
         * @return acceptor protocol.
         */
        int protocol () const noexcept
        {
            return _protocol.protocol ();
        }

        /**
         * @brief get socket native handle.
         * @return socket native handle.
         */
        int handle () const noexcept
        {
            return _handle;
        }

    private:
        /// socket handle.
        int _handle = -1;

        /// protocol.
        Protocol _protocol;
    };
}

#endif
