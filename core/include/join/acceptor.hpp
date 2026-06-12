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
            this->close ();

            this->_handle = other._handle;
            this->_protocol = other._protocol;

            other._handle = -1;
            other._protocol = Protocol ();

            return *this;
        }

        /**
         * @brief destroy instance.
         */
        virtual ~BasicStreamAcceptor ()
        {
            this->close ();
        }

        /**
         * @brief create acceptor
         * @param endpoint endpoint to assign to the acceptor.
         * @return 0 on success, -1 on failure.
         */
        virtual int create (const Endpoint& endpoint) noexcept
        {
            if (this->opened ())
            {
                lastError = make_error_code (Errc::InUse);
                return -1;
            }

            this->_handle = ::socket (endpoint.protocol ().family (), endpoint.protocol ().type () | SOCK_CLOEXEC,
                                      endpoint.protocol ().protocol ());
            if (this->_handle == -1)
            {
                lastError = std::error_code (errno, std::generic_category ());
                this->close ();
                return -1;
            }

            if (endpoint.protocol ().family () == AF_INET6)
            {
                int off = 0;

                if (::setsockopt (this->_handle, IPPROTO_IPV6, IPV6_V6ONLY, &off, sizeof (off)) == -1)
                {
                    lastError = std::error_code (errno, std::generic_category ());
                    this->close ();
                    return -1;
                }
            }

            if (endpoint.protocol ().family () == AF_UNIX)
            {
                ::unlink (endpoint.device ().c_str ());
            }
            else if (endpoint.protocol ().protocol () == IPPROTO_TCP)
            {
                int on = 1;

                if (::setsockopt (this->_handle, SOL_SOCKET, SO_REUSEADDR, &on, sizeof (on)) == -1)
                {
                    lastError = std::error_code (errno, std::generic_category ());
                    this->close ();
                    return -1;
                }
            }

            if ((::bind (this->_handle, endpoint.addr (), endpoint.length ()) == -1) ||
                (::listen (this->_handle, SOMAXCONN) == -1))
            {
                lastError = std::error_code (errno, std::generic_category ());
                this->close ();
                return -1;
            }

            this->_protocol = endpoint.protocol ();

            return 0;
        }

        /**
         * @brief close acceptor.
         */
        virtual void close () noexcept
        {
            if (this->_handle != -1)
            {
                ::close (this->_handle);
                this->_handle = -1;
            }

            this->_protocol = Protocol ();
        }

        /**
         * @brief accept new connection and fill in the client object with connection parameters.
         * @return the accepted client socket object.
         */
        virtual Socket accept () const
        {
            struct sockaddr_storage sa;
            socklen_t sa_len = sizeof (struct sockaddr_storage);
            Socket sock;

            sock._handle = ::accept (this->_handle, reinterpret_cast<struct sockaddr*> (&sa), &sa_len);
            if (sock._handle == -1)
            {
                lastError = std::error_code (errno, std::generic_category ());
                return sock;
            }

            sock._remote = Endpoint (reinterpret_cast<struct sockaddr*> (&sa), sa_len);
            sock._state = Socket::Connected;

            if (sock.protocol () == IPPROTO_TCP)
            {
                sock.setOption (Socket::NoDelay, 1);
            }
            sock.setMode (Socket::NonBlocking);

            return sock;
        }

        /**
         * @brief accept new connection and fill in the client object with connection parameters.
         * @return The client stream object on success, nullptr on failure.
         */
        virtual Stream acceptStream () const
        {
            Stream stream;
            stream.socket () = this->accept ();
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

            if (::getsockname (this->_handle, reinterpret_cast<struct sockaddr*> (&sa), &sa_len) == -1)
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
            return (this->_handle != -1);
        }

        /**
         * @brief get address family.
         * @return address family.
         */
        int family () const noexcept
        {
            return this->_protocol.family ();
        }

        /**
         * @brief get the protocol communication semantic.
         * @return the protocol communication semantic.
         */
        int type () const noexcept
        {
            return this->_protocol.type ();
        }

        /**
         * @brief get acceptor protocol.
         * @return acceptor protocol.
         */
        int protocol () const noexcept
        {
            return this->_protocol.protocol ();
        }

        /**
         * @brief get socket native handle.
         * @return socket native handle.
         */
        int handle () const noexcept
        {
            return this->_handle;
        }

    protected:
        /// socket handle.
        int _handle = -1;

        /// protocol.
        Protocol _protocol;
    };
}

#endif
