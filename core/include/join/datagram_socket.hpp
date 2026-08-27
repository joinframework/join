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

#ifndef JOIN_CORE_DATAGRAM_SOCKET_HPP
#define JOIN_CORE_DATAGRAM_SOCKET_HPP

// libjoin.
#include <join/socket.hpp>

// C.
#include <cstring>

namespace join
{
    /**
     * @brief basic datagram socket class.
     */
    template <class Protocol>
    class BasicDatagramSocket final : public BasicSocket<Protocol>
    {
        /// friendship with basic asynchronous datagram socket
        template <class P, class E>
        friend class BasicAsyncDatagramSocket;

    public:
        using Ptr = std::unique_ptr<BasicDatagramSocket<Protocol>>;
        using Mode = typename BasicSocket<Protocol>::Mode;
        using Option = typename BasicSocket<Protocol>::Option;
        using State = typename BasicSocket<Protocol>::State;
        using Endpoint = typename Protocol::Endpoint;

        /**
         * @brief Default constructor.
         */
        BasicDatagramSocket ()
        : BasicDatagramSocket (Mode::NonBlocking)
        {
        }

        /**
         * @brief Create instance specifying the time to live.
         * @param ttl packet time to live.
         */
        explicit BasicDatagramSocket (int ttl)
        : BasicDatagramSocket (Mode::NonBlocking, ttl)
        {
        }

        /**
         * @brief Create instance specifying the mode.
         * @param mode Set the socket blocking mode.
         * @param ttl packet time to live.
         */
        explicit BasicDatagramSocket (Mode mode, int ttl = 60)
        : BasicSocket<Protocol> (mode)
        , _ttl (ttl)
        {
        }

        /**
         * @brief Copy constructor.
         * @param other Other object to copy.
         */
        BasicDatagramSocket (const BasicDatagramSocket& other) = delete;

        /**
         * @brief Copy assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicDatagramSocket& operator= (const BasicDatagramSocket& other) = delete;

        /**
         * @brief Move constructor.
         * @param other Other object to move.
         */
        BasicDatagramSocket (BasicDatagramSocket&& other) noexcept
        : BasicSocket<Protocol> (std::move (other))
        , _remote (std::move (other._remote))
        , _ttl (other._ttl)
        {
            other._ttl = 60;
        }

        /**
         * @brief Move assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicDatagramSocket& operator= (BasicDatagramSocket&& other) noexcept
        {
            BasicSocket<Protocol>::operator= (std::move (other));

            _remote = std::move (other._remote);
            _ttl = other._ttl;

            other._ttl = 60;

            return *this;
        }

        /**
         * @brief Destroy the instance.
         */
        ~BasicDatagramSocket () = default;

        /**
         * @brief open socket using the given protocol.
         * @param protocol protocol to use.
         * @return 0 on success, -1 on failure.
         */
        int open (const Protocol& protocol = Protocol ()) noexcept override
        {
            int result = BasicSocket<Protocol>::open (protocol);
            if (result == -1)
            {
                return -1;
            }

            int off = 0;

            if ((protocol.protocol () == IPPROTO_UDP) || (protocol.protocol () == IPPROTO_TCP))
            {
                if ((protocol.family () == AF_INET6) &&
                    (::setsockopt (this->_handle, IPPROTO_IPV6, IPV6_V6ONLY, &off, sizeof (off)) == -1))
                {
                    lastError = std::error_code (errno, std::generic_category ());
                    close ();
                    return -1;
                }
            }

            if ((protocol.protocol () == IPPROTO_ICMPV6) || (protocol.protocol () == IPPROTO_ICMP))
            {
                if ((protocol.family () == AF_INET) &&
                    (::setsockopt (this->_handle, IPPROTO_IP, IP_HDRINCL, &off, sizeof (off)) == -1))
                {
                    lastError = std::error_code (errno, std::generic_category ());
                    close ();
                    return -1;
                }

                this->setOption (Option::MulticastTtl, _ttl);
                this->setOption (Option::Ttl, _ttl);
            }

            return 0;
        }

        /**
         * @brief assign the default remote endpoint for this socket.
         * @param endpoint endpoint to assign.
         * @return 0 on success, -1 on failure.
         * @note for a datagram socket ::connect only sets the default peer, it
         *       does not perform any handshake, so there is no connecting state.
         */
        int connect (const Endpoint& endpoint) noexcept
        {
            if ((this->_state != State::Closed) && (this->_state != State::Disconnected))
            {
                lastError = make_error_code (Errc::InUse);
                return -1;
            }

            if ((this->_state == State::Closed) && (open (endpoint.protocol ()) == -1))
            {
                return -1;
            }

            if (::connect (this->_handle, endpoint.addr (), endpoint.length ()) == -1)
            {
                lastError = std::error_code (errno, std::generic_category ());
                close ();
                return -1;
            }

            _remote = endpoint;
            this->_state = State::Connected;

            return 0;
        }

        /**
         * @brief remove the default remote endpoint.
         * @return 0 on success, -1 on failure.
         */
        int disconnect () noexcept
        {
            if (this->_state == State::Connected)
            {
                struct sockaddr_storage nullAddr;
                ::memset (&nullAddr, 0, sizeof (nullAddr));

                nullAddr.ss_family = AF_UNSPEC;

                int result = ::connect (this->_handle, reinterpret_cast<struct sockaddr*> (&nullAddr),
                                        sizeof (struct sockaddr_storage));
                if (result == -1)
                {
                    if (errno != EAFNOSUPPORT)
                    {
                        lastError = std::error_code (errno, std::generic_category ());
                        return -1;
                    }
                }

                this->_state = State::Disconnected;
                _remote = {};
            }

            return 0;
        }

        /**
         * @brief close the socket handle.
         */
        void close () noexcept override
        {
            BasicSocket<Protocol>::close ();
            _remote = {};
        }

        /**
         * @brief read data on the socket.
         * @param data buffer used to store the data received.
         * @param maxSize maximum number of bytes to read.
         * @param endpoint endpoint from where data are coming (optional).
         * @return The number of bytes received, -1 on failure.
         */
        int readFrom (char* data, unsigned long maxSize, Endpoint* endpoint = nullptr) noexcept
        {
            struct sockaddr_storage sa;

            struct iovec iov;
            iov.iov_base = data;
            iov.iov_len = maxSize;

            struct msghdr message;
            message.msg_name = &sa;
            message.msg_namelen = sizeof (struct sockaddr_storage);
            message.msg_iov = &iov;
            message.msg_iovlen = 1;
            message.msg_control = nullptr;
            message.msg_controllen = 0;

            int size = ::recvmsg (this->_handle, &message, 0);
            if (size < 1)
            {
                if (size == -1)
                {
                    lastError = std::error_code (errno, std::generic_category ());
                }
                else
                {
                    lastError = make_error_code (Errc::ConnectionClosed);
                    this->_state = State::Disconnected;
                }

                return -1;
            }

            if (message.msg_flags & MSG_TRUNC)
            {
                lastError = make_error_code (Errc::MessageTooLong);
                return -1;
            }

            if (endpoint != nullptr)
            {
                *endpoint = Endpoint (reinterpret_cast<struct sockaddr*> (&sa), message.msg_namelen);
            }

            return size;
        }

        /**
         * @brief write data on the socket.
         * @param data data buffer to send.
         * @param maxSize maximum number of bytes to write.
         * @param endpoint endpoint where to write the data.
         * @return the number of bytes written, -1 on failure.
         */
        int writeTo (const char* data, unsigned long maxSize, const Endpoint& endpoint) noexcept
        {
            if ((this->_state == State::Closed) && (open (endpoint.protocol ()) == -1))
            {
                return -1;
            }

            int result = ::sendto (this->_handle, data, maxSize, 0, endpoint.addr (), endpoint.length ());
            if (result < 0)
            {
                lastError = std::error_code (errno, std::generic_category ());
                return -1;
            }

            return result;
        }

        /**
         * @brief determine the remote endpoint associated with this socket.
         * @return remote endpoint.
         */
        const Endpoint& remoteEndpoint () const noexcept
        {
            return _remote;
        }

        /**
         * @brief check if the socket is connected.
         * @return true if connected, false otherwise.
         */
        bool connected () const noexcept
        {
            return (this->_state == State::Connected);
        }

        /**
         * @brief get socket mtu.
         * @return mtu on success, -1 on failure.
         */
        int mtu () const noexcept
        {
            if (this->_state == State::Closed)
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            int result = -1, value = -1;
            socklen_t valueLen = sizeof (value);

            if (this->_protocol.family () == AF_INET6)
            {
                result = ::getsockopt (this->_handle, IPPROTO_IPV6, IPV6_MTU, &value, &valueLen);
            }
            else if (this->_protocol.family () == AF_INET)
            {
                result = ::getsockopt (this->_handle, IPPROTO_IP, IP_MTU, &value, &valueLen);
            }
            else
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            if (result == -1)
            {
                lastError = std::error_code (errno, std::generic_category ());
                return -1;
            }

            return value;
        }

        /**
         * @brief returns the Time-To-Live value.
         * @return The Time-To-Live value.
         */
        int ttl () const noexcept
        {
            return _ttl;
        }

    protected:
        /// remote endpoint.
        Endpoint _remote;

        /// packet time to live.
        int _ttl = 60;
    };

    /**
     * @brief compare if socket handle is inferior.
     * @param a socket handle to compare.
     * @param b socket handle to compare to.
     * @return true if inferior.
     */
    template <class Protocol>
    inline bool operator< (const BasicDatagramSocket<Protocol>& a, const BasicDatagramSocket<Protocol>& b) noexcept
    {
        return a.handle () < b.handle ();
    }
}

#endif
