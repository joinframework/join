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

#ifndef __JOIN_SOCKET_HPP__
#define __JOIN_SOCKET_HPP__

// libjoin.
#include <join/resolver.hpp>
#include <join/observer.hpp>
#include <join/openssl.hpp>
#include <join/utils.hpp>

// Libraries.
#include <openssl/err.h>

// C++.
#include <type_traits>
#include <iostream>

// C.
#include <netinet/tcp.h>
#include <linux/icmp.h>
#include <sys/ioctl.h>
#include <fnmatch.h>
#include <cassert>
#include <fcntl.h>

namespace join
{
    /**
     * @brief basic socket class.
     */
    template <class Protocol>
    class BasicSocket
    {
    public:
        using Observer = BasicObserver <BasicSocket <Protocol>>;
        using Ptr      = std::unique_ptr <BasicSocket <Protocol>>;
        using Endpoint = typename Protocol::Endpoint;

        /**
         * @brief socket modes.
         */
        enum Mode
        {
            Blocking,               /**< the socket will block */
            NonBlocking,            /**< the socket will not block */
        };

        /**
         * @brief socket options.
         */
        enum Option
        {
            NoDelay,                /**< set the TCP_NODELAY option to disable/enable the nagle's algorithm. */
            KeepAlive,              /**< set the SO_KEEPALIVE option. */
            KeepIdle,               /**< set the keepalive idle timeout option. */
            KeepIntvl,              /**< set the keepalive probe interval option. */
            KeepCount,              /**< set the keepalive probe count option. */
            SndBuffer,              /**< set the socket send buffer size at the OS level. */
            RcvBuffer,              /**< set the socket receive buffer size at the OS level. */
            TimeStamp,              /**< enable or disable the receiving of the SO_TIMESTAMP control message. */
            ReuseAddr,              /**< allow reuse of local addresses. */
            ReusePort,              /**< permits multiple sockets to be bound to an identical socket address. */
            Broadcast,              /**< allow datagram sockets to send packets to a broadcast address. */
            Ttl,                    /**< set the time-to-live value of outgoing packets. */
            MulticastLoop,          /**< determines whether multicast packets should be looped back to the local sockets. */
            MulticastTtl,           /**< set the time-to-live value of outgoing multicast packets. */
            PathMtuDiscover,        /**< set the Path MTU Discovery setting for a socket. */
            RcvError,               /**< enable extended reliable error message passing. */
            AuxData,                /**< enable extended metadata message passing. */
        };

        /**
         * @brief socket states.
         */
        enum State
        {
            Connecting,             /**< socket is connecting. */
            Connected,              /**< socket is connected. */
            Disconnecting,          /**< socket is disconnecting.*/
            Disconnected,           /**< socket is disconnected.*/
            Closed,                 /**< socket is closed. */
        };

        /**
         * @brief default constructor.
         */
        BasicSocket ()
        : BasicSocket (Mode::NonBlocking)
        {
        }

        /**
         * @brief create socket instance specifying the mode.
         * @param mode blocking mode.
         */
        BasicSocket (Mode mode)
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
        BasicSocket (BasicSocket&& other)
        : _state (other._state),
          _mode (other._mode),
          _handle (other._handle),
          _protocol (other._protocol)
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
        BasicSocket& operator= (BasicSocket&& other)
        {
            this->close ();

            this->_state = other._state;
            this->_mode = other._mode;
            this->_handle = other._handle;
            this->_protocol = other._protocol;

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
            if (this->_handle != -1)
            {
                ::close (this->_handle);
            }
        }

        /**
         * @brief open socket using the given protocol.
         * @param protocol protocol to use.
         * @return 0 on success, -1 on failure.
         */
        virtual int open (const Protocol& protocol = Protocol ()) noexcept
        {
            if (this->_state != State::Closed)
            {
                lastError = make_error_code (Errc::InUse);
                return -1;
            }

            if (this->_mode == Mode::NonBlocking)
                this->_handle = ::socket (protocol.family (), protocol.type () | SOCK_NONBLOCK, protocol.protocol ());
            else
                this->_handle = ::socket (protocol.family (), protocol.type (), protocol.protocol ());

            if (this->_handle == -1)
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                this->close ();
                return -1;
            }

            this->_state = State::Disconnected;
            this->_protocol = protocol;

            return 0;
        }

        /**
         * @brief close the socket.
         */
        virtual void close () noexcept
        {
            if (this->_state != State::Closed)
            {
                ::close (this->_handle);
                this->_state = State::Closed;
                this->_handle = -1;
            }
        }

        /**
         * @brief assigns the specified endpoint to the socket.
         * @param endpoint endpoint to assign to the socket.
         * @return 0 on success, -1 on failure.
         */
        virtual int bind (const Endpoint& endpoint) noexcept
        {
            if (this->_state == State::Connected)
            {
                lastError = make_error_code (Errc::InUse);
                return -1;
            }

            if ((this->_state == State::Closed) && (this->open (endpoint.protocol ()) == -1))
            {
                return -1;
            }

            if ((endpoint.protocol ().family () == AF_INET6) || (endpoint.protocol ().family () == AF_INET))
            {
                if (setOption (Option::ReuseAddr, 1) == -1)
                {
                    return -1;
                }
            }
            else if (endpoint.protocol ().family () == AF_UNIX)
            {
                ::unlink (endpoint.device ().c_str ());
            }

            if (::bind (this->_handle, endpoint.addr (), endpoint.length ()) == -1)
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                return -1;
            }

            return 0;
        }

        /**
         * @brief get the number of readable bytes.
         * @return the number of readable bytes, -1 on failure.
         */
        virtual int canRead () const noexcept
        {
            if (this->_state == State::Closed)
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            int available = 0;

            // check if data can be read in the socket internal buffer.
            if (::ioctl (this->_handle, FIONREAD, &available) == -1)
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                return -1;
            }

            return available;
        }

        /**
         * @brief block until new data is available for reading.
         * @param timeout timeout in milliseconds.
         * @return true if there is new data available for reading, false otherwise.
         */
        virtual bool waitReadyRead (int timeout = 0) const noexcept
        {
            if (this->_state == State::Closed)
            {
                lastError = make_error_code (Errc::OperationFailed);
                return false;
            }

            return (this->wait (true, false, timeout) == 0);
        }

        /**
         * @brief read data.
         * @param data buffer used to store the data received.
         * @param maxSize maximum number of bytes to read.
         * @return the number of bytes received, -1 on failure.
         */
        virtual int read (char *data, unsigned long maxSize) noexcept
        {
            if (this->_state == State::Closed)
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            struct iovec iov;
            iov.iov_base = data;
            iov.iov_len = maxSize;

            struct msghdr message;
            message.msg_name = nullptr;
            message.msg_namelen = 0;
            message.msg_iov = &iov;
            message.msg_iovlen = 1;
            message.msg_control = nullptr;
            message.msg_controllen = 0;

            int size = ::recvmsg (this->_handle, &message, 0);
            if (size < 1)
            {
                if (size == -1)
                {
                    lastError = std::make_error_code (static_cast <std::errc> (errno));
                }
                else
                {
                    lastError = make_error_code (Errc::ConnectionClosed);
                }
                return -1;
            }

            return size;
        }

        /**
         * @brief block until at least one byte can be written.
         * @param timeout timeout in milliseconds.
         * @return true if data can be written, false otherwise.
         */
        virtual bool waitReadyWrite (int timeout = 0) const noexcept
        {
            if (this->_state == State::Closed)
            {
                lastError = make_error_code (Errc::OperationFailed);
                return false;
            }

            return (this->wait (false, true, timeout) == 0);
        }

        /**
         * @brief write data.
         * @param data data buffer to send.
         * @param maxSize maximum number of bytes to write.
         * @return the number of bytes written, -1 on failure.
         */
        virtual int write (const char *data, unsigned long maxSize) noexcept
        {
            if (this->_state == State::Closed)
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            struct iovec iov;
            iov.iov_base = const_cast <char *> (data);
            iov.iov_len = maxSize;

            struct msghdr message;
            message.msg_name = nullptr;
            message.msg_namelen = 0;
            message.msg_iov = &iov;
            message.msg_iovlen = 1;
            message.msg_control = nullptr;
            message.msg_controllen = 0;

            int result = ::sendmsg (this->_handle, &message, 0);
            if (result == -1)
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                return -1;
            }

            return result;
        }

        /**
         * @brief set the socket to the non-blocking or blocking mode.
         * @param mode blocking mode.
         * @return 0 on success, -1 on failure.
         */
        int setMode (Mode mode) noexcept
        {
            // save socket mode.
            this->_mode = mode;

            if (this->_state == State::Closed)
            {
                // socket is closed.
                // don't return an error because the mode will be set on next call to connect().
                return 0;
            }

            int oldFlags, newFlags;

            oldFlags = ::fcntl (this->_handle, F_GETFL, 0);
            if (oldFlags == -1)
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                return -1;
            }

            if (mode == Mode::NonBlocking)
            {
                newFlags = oldFlags | O_NONBLOCK;
            }
            else
            {
                newFlags = oldFlags & ~O_NONBLOCK;
            }

            if (newFlags != oldFlags)
            {
                if (::fcntl (this->_handle, F_SETFL, newFlags) == -1)
                {
                    lastError = std::make_error_code (static_cast <std::errc> (errno));
                    return -1;
                }
            }
            return 0;
        }

        /**
         * @brief set the given option to the given value.
         * @param option socket option.
         * @param value option value.
         * @return 0 on success, -1 on failure.
         */
        virtual int setOption (Option option, int value) noexcept
        {
            if (this->_state == State::Closed)
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            int optlevel, optname;

            switch (option)
            {
                case Option::KeepAlive:
                    optlevel = SOL_SOCKET;
                    optname = SO_KEEPALIVE;
                    break;

                case Option::SndBuffer:
                    optlevel = SOL_SOCKET;
                    optname = SO_SNDBUF;
                    break;

                case Option::RcvBuffer:
                    optlevel = SOL_SOCKET;
                    optname = SO_RCVBUF;
                    break;

                case Option::TimeStamp:
                    optlevel = SOL_SOCKET;
                    optname = SO_TIMESTAMP;
                    break;

                case Option::ReuseAddr:
                    optlevel = SOL_SOCKET;
                    optname = SO_REUSEADDR;
                    break;

                case Option::ReusePort:
                    optlevel = SOL_SOCKET;
                    optname = SO_REUSEPORT;
                    break;

                case Option::Broadcast:
                    optlevel = SOL_SOCKET;
                    optname = SO_BROADCAST;
                    break;

                case Option::AuxData:
                    optlevel = SOL_PACKET;
                    optname = PACKET_AUXDATA;
                    break;

                default:
                    lastError = make_error_code (Errc::InvalidParam);
                    return -1;
            }

            int result = ::setsockopt (this->_handle, optlevel, optname, &value, sizeof (value));
            if (result == -1)
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                return -1;
            }

            return 0;
        }

        /**
         * @brief determine the local endpoint associated with this socket.
         * @return local endpoint.
         */
        Endpoint localEndpoint () const
        {
            Endpoint endpoint;
            socklen_t addrLen = endpoint.length ();

            if (::getsockname (this->_handle, endpoint.addr (), &addrLen) == -1)
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                return {};
            }

            return endpoint;
        }

        /**
         * @brief check if the socket is opened.
         * @return true if opened, false otherwise.
         */
        bool opened () const noexcept
        {
            return (this->_state != State::Closed);
        }

        /**
         * @brief check if the socket is secure.
         * @return true if encrypted, false otherwise.
         */
        virtual bool encrypted () const noexcept
        {
            return false;
        }

        /**
         * @brief get socket address family.
         * @return socket address family.
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
         * @brief get socket protocol.
         * @return socket protocol.
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

        /**
         * @brief get standard 1s complement checksum.
         * @param data data pointer.
         * @param len data len.
         * @param current Current sum.
         * @return checksum.
         */
        static uint16_t checksum (const uint16_t* data, size_t len, uint16_t current = 0)
        {
            uint32_t sum = current;

            while (len > 1)
            {
                sum += *data++;
                len -= 2;
            }

            if (len == 1)
            {
                if (BYTE_ORDER == LITTLE_ENDIAN)
                    sum += *reinterpret_cast <const uint8_t *> (data);
                else
                    sum += *reinterpret_cast <const uint8_t *> (data) << 8;
            }

            sum  = (sum >> 16) + (sum & 0xffff);
            sum += (sum >> 16);

            return static_cast <uint16_t> (~sum);
        }

    protected:
        /**
         * @brief wait for the socket handle to become ready.
         * @param wantRead set to true if want read
         * @param wantWrite set to true if want write.
         * @param timeout timeout in milliseconds.
         * @return 0 on success, -1 on failure.
         */
        int wait (bool wantRead, bool wantWrite, int timeout) const noexcept
        {
            timeval time, *ptime = nullptr;

            if (timeout > 0)
            {
                time.tv_sec = timeout / 1000;
                time.tv_usec = (timeout % 1000) * 1000;
                ptime = &time;
            }

            fd_set rfds, *prfds = nullptr;
            fd_set wfds, *pwfds = nullptr;

            if (wantRead)
            {
                FD_ZERO (&rfds);
                FD_SET (this->_handle, &rfds);
                prfds = &rfds;
            }

            if (wantWrite)
            {
                FD_ZERO (&wfds);
                FD_SET (this->_handle, &wfds);
                pwfds = &wfds;
            }

            int nset = ::select (this->_handle + 1, prfds, pwfds, 0, ptime);
            if (nset != 1)
            {
                if (nset == -1)
                {
                    lastError = std::make_error_code (static_cast <std::errc> (errno));
                }
                else
                {
                    lastError = make_error_code (Errc::TimedOut);
                }

                return -1;
            }

            return 0;
        }

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
    constexpr bool operator< (const BasicSocket <Protocol>& a, const BasicSocket <Protocol>& b) noexcept
    {
        return a.handle () < b.handle ();
    }

    /**
     * @brief basic datagram socket class.
     */
    template <class Protocol>
    class BasicDatagramSocket : public BasicSocket <Protocol>
    {
    public:
        using Observer = BasicObserver <BasicDatagramSocket <Protocol>>;
        using Ptr      = std::unique_ptr <BasicDatagramSocket <Protocol>>;
        using Mode     = typename BasicSocket <Protocol>::Mode;
        using Option   = typename BasicSocket <Protocol>::Option;
        using State    = typename BasicSocket <Protocol>::State;
        using Endpoint = typename Protocol::Endpoint;

        /**
         * @brief Default constructor.
         */
        BasicDatagramSocket (int ttl = 60)
        : BasicDatagramSocket (Mode::NonBlocking, ttl)
        {
        }

        /**
         * @brief Create instance specifying the mode.
         * @param mode Set the socket blocking mode.
         */
        BasicDatagramSocket (Mode mode, int ttl = 60)
        : BasicSocket <Protocol> (mode),
          ttl_ (ttl)
        {
        }

        /**
         * @brief Copy constructor.
         * @param other Other object to copy.
         */
        BasicDatagramSocket (const BasicDatagramSocket &other) = delete;

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
        BasicDatagramSocket (BasicDatagramSocket&& other)
        : BasicSocket <Protocol> (std::move (other)),
          ttl_ (other.ttl_)
        {
            other.ttl_ = 60;
        }

        /**
         * @brief Move assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicDatagramSocket& operator= (BasicDatagramSocket&& other)
        {
            BasicSocket <Protocol>::operator= (std::move (other));

            ttl_ = other.ttl_;

            other.ttl_ = 60;

            return *this;
        }

        /**
         * @brief Destroy the instance.
         */
        virtual ~BasicDatagramSocket () = default;

        /**
         * @brief open socket using the given protocol.
         * @param protocol protocol to use.
         * @return 0 on success, -1 on failure.
         */
        virtual int open (const Protocol& protocol = Protocol ()) noexcept override
        {
            int result = BasicSocket <Protocol>::open (protocol);
            if (result == -1)
            {
                return -1;
            }

            int off = 0;

            if ((protocol.protocol () == IPPROTO_UDP) || (protocol.protocol () == IPPROTO_TCP))
            {
                if ((protocol.family () == AF_INET6) && (::setsockopt (this->_handle, IPPROTO_IPV6, IPV6_V6ONLY, &off, sizeof (off)) == -1))
                {
                    lastError = std::make_error_code (static_cast <std::errc> (errno));
                    this->close ();
                    return -1;
                }
            }

            if ((protocol.protocol () == IPPROTO_ICMPV6) || (protocol.protocol () == IPPROTO_ICMP))
            {
                if ((protocol.family () == AF_INET) && (::setsockopt (this->_handle, IPPROTO_IP, IP_HDRINCL, &off, sizeof (off)) == -1))
                {
                    lastError = std::make_error_code (static_cast <std::errc> (errno));
                    this->close ();
                    return -1;
                }

                if (setOption (Option::MulticastTtl, this->ttl_) == -1)
                {
                    this->close ();
                    return -1;
                }

                if (setOption (Option::Ttl, this->ttl_) == -1)
                {
                    this->close ();
                    return -1;
                }
            }

            return 0;
        }

        /**
         * @brief assigns the specified interface to the socket.
         * @param interface Interface name.
         * @return 0 on success, -1 on failure.
         */
        /*virtual int bind (const std::string& interface) noexcept
        {
            if (this->_state == State::Connected)
            {
                lastError = make_error_code (Errc::InUse);
                return -1;
            }

            if ((this->_state == State::Closed) && (this->open () == -1))
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            if ((this->_protocol.family () == AF_INET6) || (this->_protocol.family () == AF_INET))
            {
                // allow reuse of local addresses.
                if (setOption (Option::ReuseAddr, 1) == -1)
                {
                    return -1;
                }
            }

            // assigns the specified interface to the socket.
            int result = setsockopt (this->_handle, SOL_SOCKET, SO_BINDTODEVICE, interface.c_str (), interface.size ());
            if (result == -1)
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                return -1;
            }

            return 0;
        }*/

        /**
         * @brief make a connection to the given endpoint.
         * @param endpoint endpoint to connect to.
         * @return 0 on success, -1 on failure.
         */
        virtual int connect (const Endpoint& endpoint)
        {
            if ((this->_state != State::Closed) && (this->_state != State::Disconnected))
            {
                lastError = make_error_code (Errc::InUse);
                return -1;
            }

            if ((this->_state == State::Closed) && (this->open (endpoint.protocol ()) == -1))
            {
                return -1;
            }

            int result = ::connect (this->_handle, endpoint.addr (), endpoint.length ());
            this->_state = State::Connecting;

            if (result == -1)
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                if (lastError != std::errc::operation_in_progress)
                {
                    this->close ();
                }
                return -1;
            }

            this->_state = State::Connected;

            return 0;
        }

        /**
         * @brief shutdown the connection.
         * @return 0 on success, -1 on failure.
         */
        virtual int disconnect ()
        {
            if (this->_state == State::Connected)
            {
                struct sockaddr_storage nullAddr;
                ::memset (&nullAddr, 0, sizeof (nullAddr));

                nullAddr.ss_family = AF_UNSPEC;

                int result = ::connect (this->_handle, reinterpret_cast <struct sockaddr*> (&nullAddr), sizeof (struct sockaddr_storage));
                if (result == -1)
                {
                    if (errno != EAFNOSUPPORT)
                    {
                        lastError = std::make_error_code (static_cast <std::errc> (errno));
                        return -1;
                    }
                }

                this->_state = State::Disconnected;
            }

            return 0;
        }

        /**
         * @brief read data.
         * @param data buffer used to store the data received.
         * @param maxSize maximum number of bytes to read.
         * @return the number of bytes received, -1 on failure.
         */
        virtual int read (char *data, unsigned long maxSize) noexcept override
        {
            if ((this->_state != State::Connected) && (this->_state != State::Disconnecting))
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            return BasicSocket <Protocol>::read (data, maxSize);
        }

        /**
         * @brief read data on the socket.
         * @param data buffer used to store the data received.
         * @param maxSize maximum number of bytes to read.
         * @param endpoint endpoint from where data are coming (optional).
         * @return The number of bytes received, -1 on failure.
         */
        virtual int readFrom (char* data, unsigned long maxSize, Endpoint* endpoint = nullptr) noexcept
        {
            if (this->_state == State::Closed)
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            Endpoint from;
            socklen_t addrLen = from.length ();

            int size = ::recvfrom (this->_handle, data, maxSize, 0, from.addr (), &addrLen);
            if (size < 1)
            {
                if (size == -1)
                {
                    lastError = std::make_error_code (static_cast <std::errc> (errno));
                }
                else
                {
                    lastError = make_error_code (Errc::ConnectionClosed);
                    this->_state = State::Disconnected;
                }

                return -1;
            }

            if (endpoint != nullptr)
            {
                *endpoint = from;
            }

            return size;
        }

        /**
         * @brief write data.
         * @param data data buffer to send.
         * @param maxSize maximum number of bytes to write.
         * @return the number of bytes written, -1 on failure.
         */
        virtual int write (const char *data, unsigned long maxSize) noexcept override
        {
            if (this->_state != State::Connected)
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            return BasicSocket <Protocol>::write (data, maxSize);
        }

        /**
         * @brief write data on the socket.
         * @param data data buffer to send.
         * @param maxSize maximum number of bytes to write.
         * @param endpoint endpoint where to write the data.
         * @return the number of bytes written, -1 on failure.
         */
        virtual int writeTo (const char* data, unsigned long maxSize, const Endpoint& endpoint) noexcept
        {
            if ((this->_state == State::Closed) && (this->open (endpoint.protocol ()) == -1))
            {
                return -1;
            }

            int result = ::sendto (this->_handle, data, maxSize, 0, endpoint.addr (), endpoint.length ());
            if (result < 0)
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                return -1;
            }

            return result;
        }

        /**
         * @brief set the given option to the given value.
         * @param option socket option.
         * @param value option value.
         * @return 0 on success, -1 on failure.
         */
        virtual int setOption (Option option, int value) noexcept override
        {
            if (this->_state == State::Closed)
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            int optlevel, optname;

            switch (option)
            {
                case Option::Ttl:
                    if (this->family () == AF_INET6)
                    {
                        optlevel = IPPROTO_IPV6;
                        optname = IPV6_UNICAST_HOPS;
                    }
                    else
                    {
                        optlevel = IPPROTO_IP;
                        optname = IP_TTL;
                    }
                    break;

                case Option::MulticastLoop:
                    if (this->family () == AF_INET6)
                    {
                        optlevel = IPPROTO_IPV6;
                        optname = IPV6_MULTICAST_LOOP;
                    }
                    else
                    {
                        optlevel = IPPROTO_IP;
                        optname = IP_MULTICAST_LOOP;
                    }
                    break;

                case Option::MulticastTtl:
                    if (this->family () == AF_INET6)
                    {
                        optlevel = IPPROTO_IPV6;
                        optname = IPV6_MULTICAST_HOPS;
                    }
                    else
                    {
                        optlevel = IPPROTO_IP;
                        optname = IP_MULTICAST_TTL;
                    }
                    break;

                case Option::PathMtuDiscover:
                    if (this->family () == AF_INET6)
                    {
                        optlevel = IPPROTO_IPV6;
                        optname = IPV6_MTU_DISCOVER;
                    }
                    else
                    {
                        optlevel = IPPROTO_IP;
                        optname = IP_MTU_DISCOVER;
                    }
                    break;

                case Option::RcvError:
                    if (this->family () == AF_INET6)
                    {
                        optlevel = IPPROTO_IPV6;
                        optname = IPV6_RECVERR;
                    }
                    else
                    {
                        optlevel = IPPROTO_IP;
                        optname = IP_RECVERR;
                    }
                    break;

                default:
                    return BasicSocket<Protocol>::setOption (option, value);
            }

            int result = ::setsockopt (this->_handle, optlevel, optname, &value, sizeof (value));
            if (result == -1)
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                return -1;
            }

            return 0;
        }

        /**
         * @brief determine the remote endpoint associated with this socket.
         * @return remote endpoint.
         */
        Endpoint remoteEndpoint () const
        {
            Endpoint endpoint;
            socklen_t addrLen = endpoint.length ();

            if (::getpeername (this->_handle, endpoint.addr (), &addrLen) == -1)
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                return {};
            }

            return endpoint;
        }

        /**
         * @brief check if the socket is connected.
         * @return true if connected, false otherwise.
         */
        virtual bool connected () noexcept
        {
            return (this->_state == State::Connected);
        }

        /**
         * @brief get socket mtu.
         * @return mtu on success, -1 on failure.
         */
        int mtu () const
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
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                return -1;
            }

            return value;
        }

        /**
         * @brief Returns the Time-To-Live value.
         * @return The Time-To-Live value.
         */
        int ttl () const
        {
            return this->ttl_;
        }

    protected:
        /// packet time to live.
        int ttl_ = 60;
    };

    /**
     * @brief compare if socket handle is inferior.
     * @param a socket handle to compare.
     * @param b socket handle to compare to.
     * @return true if inferior.
     */
    template <class Protocol>
    constexpr bool operator< (const BasicDatagramSocket <Protocol>& a, const BasicDatagramSocket <Protocol>& b) noexcept
    {
        return a.handle () < b.handle ();
    }

    /**
     * @brief basic stream socket class.
     */
    template <class Protocol>
    class BasicStreamSocket : public BasicDatagramSocket <Protocol>
    {
    public:
        using Observer = BasicObserver <BasicStreamSocket <Protocol>>;
        using Ptr      = std::unique_ptr <BasicStreamSocket <Protocol>>;
        using Mode     = typename BasicDatagramSocket <Protocol>::Mode;
        using Option   = typename BasicDatagramSocket <Protocol>::Option;
        using State    = typename BasicDatagramSocket <Protocol>::State;
        using Endpoint = typename Protocol::Endpoint;

        /**
         * @brief default constructor.
         */
        BasicStreamSocket ()
        : BasicStreamSocket (Mode::NonBlocking)
        {
        }

        /**
         * @brief create instance specifying the mode.
         * @param mode Set the socket blocking mode.
         */
        BasicStreamSocket (Mode mode)
        : BasicDatagramSocket <Protocol> (mode)
        {
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicStreamSocket (const BasicStreamSocket &other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicStreamSocket& operator= (const BasicStreamSocket& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        BasicStreamSocket (BasicStreamSocket&& other)
        : BasicDatagramSocket <Protocol> (std::move (other))
        {
        }

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicStreamSocket& operator= (BasicStreamSocket&& other)
        {
            BasicDatagramSocket <Protocol>::operator= (std::move (other));

            return *this;
        }

        /**
         * @brief destroy the instance.
         */
        virtual ~BasicStreamSocket () = default;

        /**
         * @brief block until connected.
         * @param timeout timeout in milliseconds.
         * @return true if connected, false otherwise.
         */
        virtual bool waitConnected (int timeout = 0)
        {
            if (this->_state != State::Connected)
            {
                if (this->_state != State::Connecting)
                {
                    lastError = make_error_code (Errc::OperationFailed);
                    return false;
                }

                if (!this->waitReadyWrite (timeout))
                {
                    return false;
                }

                return connected ();
            }

            return true;
        }

        /**
         * @brief shutdown the connection.
         * @return 0 on success, -1 on failure.
         */
        virtual int disconnect () override
        {
            if (this->_state == State::Connected)
            {
                ::shutdown (this->_handle, SHUT_WR);
                this->_state = State::Disconnecting;
            }

            if (this->_state == State::Disconnecting)
            {
                char buffer[4096];
                // closing before reading can make the client
                // not see all of our output.
                // we have to do a "lingering close"
                for (;;)
                {
                    int result = this->read (buffer, sizeof (buffer));
                    if (result <= 0)
                    {
                        if ((result == -1) && (lastError == Errc::TemporaryError))
                        {
                            return -1;
                        }

                        break;
                    }
                }

                ::shutdown (this->_handle, SHUT_RD);
                this->_state = State::Disconnected;
            }

            this->close ();

            return 0;
        }

        /**
         * @brief wait until the connection as been shut down.
         * @param timeout timeout in milliseconds.
         * return true if the connection as been shut down, false otherwise.
         */
        virtual bool waitDisconnected (int timeout = 0)
        {
            if ((this->_state != State::Disconnected) && (this->_state != State::Closed))
            {
                if (this->_state != State::Disconnecting)
                {
                    lastError = make_error_code (Errc::OperationFailed);
                    return false;
                }

                auto start = std::chrono::steady_clock::now ();
                int elapsed = 0;

                while ((lastError == Errc::TemporaryError) && (elapsed <= timeout))
                {
                    if (!this->waitReadyRead (timeout - elapsed))
                    {
                        return false;
                    }

                    if (this->disconnect () == 0)
                    {
                        return true;
                    }

                    if (timeout)
                    {
                        elapsed = std::chrono::duration_cast <std::chrono::milliseconds> (std::chrono::steady_clock::now () - start).count ();
                    }
                }

                return false;
            }

            return true;
        }

        /**
         * @brief read data until size is reached or an error occurred.
         * @param data buffer used to store the data received.
         * @param size number of bytes to read.
         * @param timeout timeout in milliseconds.
         * @return 0 on success, -1 on failure.
         */
        int readExactly (char *data, unsigned long size, int timeout = 0)
        {
            unsigned long numRead = 0;

            while (numRead < size)
            {
                int result = this->read (data + numRead, size - numRead);
                if (result == -1)
                {
                    if (lastError == Errc::TemporaryError)
                    {
                        if (this->waitReadyRead (timeout))
                            continue;
                    }

                    return -1;
                }

                numRead += result;
            }

            return 0;
        }

        /**
         * @brief write data until size is reached or an error occurred.
         * @param data data buffer to send.
         * @param size number of bytes to write.
         * @param timeout timeout in milliseconds.
         * @return 0 on success, -1 on failure.
         */
        int writeExactly (const char *data, unsigned long size, int timeout = 0)
        {
            unsigned long numWrite = 0;

            while (numWrite < size)
            {
                int result = this->write (data + numWrite, size - numWrite);
                if (result == -1)
                {
                    if (lastError == Errc::TemporaryError)
                    {
                        if (this->waitReadyWrite (timeout))
                            continue;
                    }

                    return -1;
                }

                numWrite += result;
            }

            return 0;
        }

        /**
         * @brief set the given option to the given value.
         * @param option socket option.
         * @param value option value.
         * @return 0 on success, -1 on failure.
         */
        virtual int setOption (Option option, int value) noexcept override
        {
            if (this->_state == State::Closed)
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            int optlevel, optname;

            switch (option)
            {
                case Option::NoDelay:
                    optlevel = IPPROTO_TCP;
                    optname = TCP_NODELAY;
                    break;

                case Option::KeepIdle:
                    optlevel = IPPROTO_TCP;
                    optname = TCP_KEEPIDLE;
                    break;

                case Option::KeepIntvl:
                    optlevel = IPPROTO_TCP;
                    optname = TCP_KEEPINTVL;
                    break;

                case Option::KeepCount:
                    optlevel = IPPROTO_TCP;
                    optname = TCP_KEEPCNT;
                    break;

                default:
                    return BasicDatagramSocket <Protocol>::setOption (option, value);
            }

            int result = ::setsockopt (this->_handle, optlevel, optname, &value, sizeof (value));
            if (result == -1)
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                return -1;
            }

            return 0;
        }

        /**
         * @brief check if the socket is connecting.
         * @return true if connecting, false otherwise.
         */
        virtual bool connecting () const noexcept
        {
            return (this->_state == State::Connecting);
        }

        /**
         * @brief check if the socket is connected.
         * @return true if connected, false otherwise.
         */
        virtual bool connected () noexcept override
        {
            if (this->_state == State::Connected)
            {
                return true;
            }
            else if (this->_state != State::Connecting)
            {
                lastError = make_error_code (Errc::OperationFailed);
                return false;
            }

            int optval;
            socklen_t optlen = sizeof (optval);

            int result = ::getsockopt (this->_handle, SOL_SOCKET, SO_ERROR, &optval, &optlen);
            if (result == -1)
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                return false;
            }

            if (optval != 0)
            {
                lastError = std::make_error_code (static_cast <std::errc> (optval));
                return false;
            }

            this->_state = State::Connected;

            return true;
        }

        /// friendship with basic stream acceptor
        friend class BasicStreamAcceptor <Protocol>;
    };

    /**
     * @brief compare if socket handle is inferior.
     * @param a socket handle to compare.
     * @param b socket handle to compare to.
     * @return true if inferior.
     */
    template <class Protocol>
    constexpr bool operator< (const BasicStreamSocket <Protocol>& a, const BasicStreamSocket <Protocol>& b) noexcept
    {
        return a.handle () < b.handle ();
    }

    /**
     * @brief TLS error codes.
     */
    enum class TlsErrc
    {
        TlsCloseNotifyAlert = 1,    /**< A close notify alert was received. */
        TlsProtocolError            /**< A failure in the TLS library occurred, usually a protocol error. */
    };

    /**
     * @brief TLS error category.
     */
    class TlsCategory : public std::error_category
    {
    public:
        /**
         * @brief get digest error category name.
         * @return digest error category name.
         */
        virtual const char* name () const noexcept
        {
            return "join";
        }

        /**
         * @brief translate digest error code to human readable error string.
         * @param code error code.
         * @return human readable error string.
         */
        virtual std::string message (int code) const
        {
            switch (static_cast <TlsErrc> (code))
            {
                case TlsErrc::TlsCloseNotifyAlert:
                    return "TLS close notify alert received";
                case TlsErrc::TlsProtocolError:
                    return "TLS protocol error";
                default:
                    return "success";
            }
        }
    };

    /**
     * @brief get error category.
     * @return the created std::error_category object.
     */
    const std::error_category& getTlsCategory ()
    {
        static TlsCategory instance;
        return instance;
    }

    /**
     * @brief create an std::error_code object.
     * @param code error code number.
     * @return the created std::error_code object.
     */
    std::error_code make_error_code (TlsErrc code)
    {
        return std::error_code (static_cast <int> (code), getTlsCategory ());
    }

    /**
     * @brief create an std::error_condition object.
     * @param code error code number.
     * @return the created std::error_condition object.
     */
    std::error_condition make_error_condition (TlsErrc code)
    {
        return std::error_condition (static_cast <int> (code), getTlsCategory ());
    }

    /**
     * @brief basic TLS socket class.
     */
    template <class Protocol>
    class BasicTlsSocket : public BasicStreamSocket <Protocol>
    {
    public:
        using Observer = BasicObserver <BasicTlsSocket <Protocol>>;
        using Ptr      = std::unique_ptr <BasicTlsSocket <Protocol>>;
        using Mode     = typename BasicStreamSocket <Protocol>::Mode;
        using Option   = typename BasicStreamSocket <Protocol>::Option;
        using State    = typename BasicStreamSocket <Protocol>::State;
        using Endpoint = typename Protocol::Endpoint;

        /**
         * @brief TLS mode.
         */
        enum TlsMode
        {
            ClientMode,             /**< TLS client mode. */
            ServerMode,             /**< TLS server mode. */
        };

        /**
         * @brief default constructor.
         */
        BasicTlsSocket ()
        : BasicTlsSocket (Mode::NonBlocking)
        {
        }

        /**
         * @brief create instance specifying the mode.
         * @param mode Set the socket blocking mode.
         */
        BasicTlsSocket (Mode mode)
        : BasicStreamSocket <Protocol> (mode),
        #if OPENSSL_VERSION_NUMBER < 0x10100000L
          _tlsContext (SSL_CTX_new (SSLv23_method ()), join::crypto::SslCtxDelete ())
        #else
          _tlsContext (SSL_CTX_new (TLS_method ()), join::crypto::SslCtxDelete ())
        #endif
        {
            if (_tlsContext == nullptr)
            {
                throw std::runtime_error ("OpenSSL libraries were not initialized at process start");
            }

            // enable the OpenSSL bug workaround options.
            SSL_CTX_set_options (_tlsContext.get (), SSL_OP_ALL);

        #if OPENSSL_VERSION_NUMBER >= 0x10100000L
            // disallow compression.
            SSL_CTX_set_options (_tlsContext.get (), SSL_OP_NO_COMPRESSION);
        #endif

            // disallow usage of SSLv2, SSLv3, TLSv1 and TLSv1.1 which are considered insecure.
            SSL_CTX_set_options (_tlsContext.get (), SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1);

            // setup write mode.
            SSL_CTX_set_mode (_tlsContext.get (), SSL_MODE_ENABLE_PARTIAL_WRITE | SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);

            // automatically renegotiates.
            SSL_CTX_set_mode (_tlsContext.get (), SSL_MODE_AUTO_RETRY);

            // set session cache mode to client by default.
            SSL_CTX_set_session_cache_mode (_tlsContext.get (), SSL_SESS_CACHE_CLIENT);

            // no verification by default.
            SSL_CTX_set_verify (_tlsContext.get (), SSL_VERIFY_NONE, nullptr);

            // set default TLSv1.2 and below cipher suites.
            SSL_CTX_set_cipher_list (_tlsContext.get (), join::crypto::defaultCipher_.c_str ());

        #if OPENSSL_VERSION_NUMBER >= 0x10101000L
            //  set default TLSv1.3 cipher suites.
            SSL_CTX_set_ciphersuites (_tlsContext.get (), join::crypto::defaultCipher_1_3_.c_str ());
        #endif
        }

        /**
         * @brief create instance specifying TLS context and TLS mode.
         * @param tlsContext TLS context.
         * @param tlsMode TLS mode.
         */
        BasicTlsSocket (const join::crypto::SslCtxPtr& tlsContext, TlsMode tlsMode)
        : BasicTlsSocket (Mode::NonBlocking, tlsContext, tlsMode)
        {
        }

        /**
         * @brief Create socket instance specifying the socket mode, TLS context and TLS mode.
         * @param mode Set the socket blocking mode.
         * @param tlsContext TLS context.
         * @param tlsMode TLS mode.
         */
        BasicTlsSocket (Mode mode, const join::crypto::SslCtxPtr& tlsContext, TlsMode tlsMode)
        : BasicStreamSocket <Protocol> (mode),
          _tlsContext (tlsContext),
          _tlsMode (tlsMode)
        {
            if (_tlsContext == nullptr)
            {
                throw std::invalid_argument ("OpenSSL context is invalid");
            }
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicTlsSocket (const BasicTlsSocket &other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicTlsSocket& operator= (const BasicTlsSocket& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        BasicTlsSocket (BasicTlsSocket&& other)
        : BasicStreamSocket <Protocol> (std::move (other)),
          _tlsContext (std::move (other._tlsContext)),
          _tlsHandle (std::move (other._tlsHandle)),
          _tlsMode (other._tlsMode),
          _tlsState (other._tlsState)
        {
            SSL_set_app_data (this->_tlsHandle.get (), this);

            other._tlsMode = TlsMode::ClientMode;
            other._tlsState = TlsState::NonEncrypted;
        }

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicTlsSocket& operator= (BasicTlsSocket&& other)
        {
            BasicStreamSocket <Protocol>::operator= (std::move (other));

            this->_tlsContext = std::move (other._tlsContext);
            this->_tlsHandle = std::move (other._tlsHandle);
            this->_tlsMode = other._tlsMode;
            this->_tlsState = other._tlsState;

            SSL_set_app_data (this->_tlsHandle.get (), this);

            other._tlsMode = TlsMode::ClientMode;
            other._tlsState = TlsState::NonEncrypted;

            return *this;
        }

        /**
         * @brief destroy the instance.
         */
        virtual ~BasicTlsSocket () = default;

        /**
         * @brief make an encrypted connection to the given endpoint.
         * @param endpoint endpoint to connect to.
         * @return 0 on success, -1 on failure.
         */
        int connectEncrypted (const Endpoint& endpoint)
        {
            if (BasicStreamSocket <Protocol>::connect (endpoint) == -1)
            {
                return -1;
            }

            return this->startEncryption ();
        }

        /**
         * @brief start socket encryption (perform TLS handshake).
         * @return 0 on success, -1 on failure.
         */
        int startEncryption ()
        {
            if (this->_state != State::Connected)
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            if (this->encrypted () == false)
            {
                if (_tlsHandle == nullptr)
                {
                    this->_tlsHandle.reset (SSL_new (this->_tlsContext.get ()));
                    if (this->_tlsHandle == nullptr)
                    {
                        lastError = make_error_code (Errc::UnknownError);
                        return -1;
                    }

                    if (SSL_set_fd (this->_tlsHandle.get (), this->_handle) != 1)
                    {
                        lastError = make_error_code (Errc::InvalidParam);
                        this->_tlsHandle.reset ();
                        return -1;
                    }

                    // prepare the object to work in client or server mode.
                    if (this->_tlsMode == TlsMode::ClientMode)
                    {
                        SSL_set_connect_state (this->_tlsHandle.get ());
                    }
                    else
                    {
                        SSL_set_accept_state (this->_tlsHandle.get ());
                    }

                    // Save the internal context.
                    SSL_set_app_data (this->_tlsHandle.get (), this);

                #ifdef DEBUG
                    // Set info callback.
                    SSL_set_info_callback (this->_tlsHandle.get (), infoWrapper);
                #endif
                }

                return startHandshake ();
            }

            return 0;
        }

        /**
         * @brief wait until TLS handshake is performed or timeout occur (non blocking socket).
         * @param timeout timeout in milliseconds (0: infinite).
         * return true on success, false otherwise.
         */
        bool waitEncrypted (int timeout = 0)
        {
            if (this->encrypted () == false)
            {
                if ((this->_state != State::Connecting) && (this->_state != State::Connected))
                {
                    lastError = make_error_code (Errc::OperationFailed);
                    return -1;
                }

                if (this->_state == State::Connecting)
                {
                    if (!this->waitConnected (timeout))
                    {
                        return false;
                    }

                    if (this->startEncryption () == 0)
                    {
                        return true;
                    }
                }

                while ((lastError == Errc::TemporaryError) && (SSL_want_read  (this->_tlsHandle.get ()) || SSL_want_write (this->_tlsHandle.get ())))
                {
                    if (this->wait (SSL_want_read (this->_tlsHandle.get ()), SSL_want_write (this->_tlsHandle.get ()), timeout) == -1)
                    {
                        return false;
                    }

                    if (this->startHandshake () == 0)
                    {
                        return true;
                    }
                }

                return false;
            }

            return true;
        }

        /**
         * @brief shutdown the connection.
         * @return 0 on success, -1 on failure.
         */
        virtual int disconnect () override
        {
            if (this->encrypted ())
            {
                // check if the close_notify alert was already sent.
                if ((SSL_get_shutdown (this->_tlsHandle.get ()) & SSL_SENT_SHUTDOWN) == false)
                {
                    // send the close_notify alert to the peer.
                    int result = SSL_shutdown (this->_tlsHandle.get ());
                    if (result < 0)
                    {
                        // shutdown was not successful.
                        switch (SSL_get_error (this->_tlsHandle.get (), result))
                        {
                            case SSL_ERROR_WANT_READ:
                            case SSL_ERROR_WANT_WRITE:
                                // SSL_shutdown want read or want write.
                                lastError = make_error_code (Errc::TemporaryError);
                                break;
                            case SSL_ERROR_SYSCALL:
                                // an error occurred at the socket level.
                                switch (errno)
                                {
                                    case 0:
                                    case ECONNRESET:
                                    case EPIPE:
                                        lastError = make_error_code (Errc::ConnectionClosed);
                                        this->_tlsState = TlsState::NonEncrypted;
                                        this->_state = State::Disconnected;
                                        break;
                                    default:
                                        lastError = std::make_error_code (static_cast <std::errc> (errno));
                                        break;
                                }
                                break;
                            default:
                                // SSL protocol error.
                            #ifdef DEBUG
                                std::cout << ERR_reason_error_string (ERR_get_error ()) << std::endl;
                            #endif
                                lastError = make_error_code (TlsErrc::TlsProtocolError);
                                break;
                        }

                        return -1;
                    }
                    else if (result == 1)
                    {
                        // shutdown was successfully completed.
                        // close_notify alert was sent and the peer's close_notify alert was received.
                        this->_tlsState = TlsState::NonEncrypted;
                    }
                    else
                    {
                        // shutdown is not yet finished.
                        // the close_notify was sent but the peer did not send it back yet.
                        // SSL_read must be called to do a bidirectional shutdown.
                    }
                }
            }

            return BasicStreamSocket <Protocol>::disconnect ();
        }

        /**
         * @brief close the socket handle.
         */
        virtual void close () noexcept override
        {
            BasicStreamSocket <Protocol>::close ();
            this->_tlsState = TlsState::NonEncrypted;
            this->_tlsHandle.reset ();
        }

        /**
         * @brief block until new data is available for reading.
         * @param timeout timeout in milliseconds (0: infinite).
         * @return true if there is new data available for reading, false otherwise.
         */
        virtual bool waitReadyRead (int timeout = 0) const noexcept override
        {
            if (this->encrypted () && (SSL_want_read (this->_tlsHandle.get ()) || SSL_want_write (this->_tlsHandle.get ())))
            {
                return (this->wait (SSL_want_read (this->_tlsHandle.get ()), SSL_want_write (this->_tlsHandle.get ()), timeout) == 0);
            }

            return BasicStreamSocket <Protocol>::waitReadyRead (timeout);
        }

        /**
         * @brief get the number of readable bytes.
         * @return The number of readable bytes, -1 on failure.
         */
        virtual int canRead () const noexcept override
        {
            if (this->encrypted ())
            {
                return SSL_pending (this->_tlsHandle.get ());
            }

            return BasicStreamSocket <Protocol>::canRead ();
        }

        /**
         * @brief read data on the socket.
         * @param data buffer used to store the data received.
         * @param maxSize maximum number of bytes to read.
         * @return the number of bytes received, -1 on failure.
         */
        virtual int read (char *data, unsigned long maxSize) noexcept override
        {
            if (this->encrypted ())
            {
                // read data.
                int result = SSL_read (this->_tlsHandle.get (), data, int (maxSize));
                if (result < 1)
                {
                    switch (SSL_get_error (this->_tlsHandle.get (), result))
                    {
                        case SSL_ERROR_WANT_READ:
                        case SSL_ERROR_WANT_WRITE:
                        case SSL_ERROR_WANT_X509_LOOKUP:
                            // SSL_read want read, want write or want lookup.
                            lastError = make_error_code (Errc::TemporaryError);
                            break;
                        case SSL_ERROR_ZERO_RETURN:
                            // a close notify alert was received.
                            // we have to answer by sending a close notify alert too.
                            lastError = make_error_code (TlsErrc::TlsCloseNotifyAlert);
                            if (SSL_get_shutdown (this->_tlsHandle.get ()) & SSL_SENT_SHUTDOWN)
                            {
                                this->_tlsState = TlsState::NonEncrypted;
                            }
                            break;
                        case SSL_ERROR_SYSCALL:
                            // an error occurred at the socket level.
                            switch (errno)
                            {
                                case 0:
                                case ECONNRESET:
                                case EPIPE:
                                    lastError = make_error_code (Errc::ConnectionClosed);
                                    this->_tlsState = TlsState::NonEncrypted;
                                    this->_state = State::Disconnected;
                                    break;
                                default:
                                    lastError = std::make_error_code (static_cast <std::errc> (errno));
                                    break;
                            }
                            break;
                        default:
                            // SSL protocol error.
                        #ifdef DEBUG
                            std::cout << ERR_reason_error_string (ERR_get_error ()) << std::endl;
                        #endif
                            lastError = make_error_code (TlsErrc::TlsProtocolError);
                            break;
                    }

                    return -1;
                }

                return result;
            }

            return BasicStreamSocket <Protocol>::read (data, maxSize);
        }

        /**
         * @brief block until until at least one byte can be written on the socket.
         * @param timeout timeout in milliseconds (0: infinite).
         * @return true if data can be written on the socket, false otherwise.
         */
        virtual bool waitReadyWrite (int timeout = 0) const noexcept override
        {
            if (this->encrypted () && (SSL_want_read (this->_tlsHandle.get ()) || SSL_want_write (this->_tlsHandle.get ())))
            {
                return (this->wait (SSL_want_read (this->_tlsHandle.get ()), SSL_want_write (this->_tlsHandle.get ()), timeout) == 0);
            }

            return BasicStreamSocket <Protocol>::waitReadyWrite (timeout);
        }

        /**
         * @brief write data on the socket.
         * @param data data buffer to send.
         * @param maxSize maximum number of bytes to write.
         * @return the number of bytes written, -1 on failure.
         */
        virtual int write (const char *data, unsigned long maxSize) noexcept override
        {
            if (this->encrypted ())
            {
                // write data.
                int result = SSL_write (this->_tlsHandle.get (), data, int (maxSize));
                if (result < 1)
                {
                    switch (SSL_get_error (this->_tlsHandle.get (), result))
                    {
                        case SSL_ERROR_WANT_READ:
                        case SSL_ERROR_WANT_WRITE:
                        case SSL_ERROR_WANT_X509_LOOKUP:
                            // SSL_write want read, want write or want lookup.
                            lastError = make_error_code (Errc::TemporaryError);
                            break;
                        case SSL_ERROR_ZERO_RETURN:
                            // a close notify alert was received.
                            // we have to answer by sending a close notify alert too.
                            lastError = make_error_code (TlsErrc::TlsCloseNotifyAlert);
                            if (SSL_get_shutdown (this->_tlsHandle.get ()) & SSL_SENT_SHUTDOWN)
                            {
                                this->_tlsState = TlsState::NonEncrypted;
                            }
                            break;
                        case SSL_ERROR_SYSCALL:
                            // an error occurred at the socket level.
                            switch (errno)
                            {
                                case 0:
                                case ECONNRESET:
                                case EPIPE:
                                    lastError = make_error_code (Errc::ConnectionClosed);
                                    this->_tlsState = TlsState::NonEncrypted;
                                    this->_state = State::Disconnected;
                                    break;
                                default:
                                    lastError = std::make_error_code (static_cast <std::errc> (errno));
                                    break;
                            }
                            break;
                        default:
                            // SSL protocol error.
                        #ifdef DEBUG
                            std::cout << ERR_reason_error_string (ERR_get_error ()) << std::endl;
                        #endif
                            lastError = make_error_code (TlsErrc::TlsProtocolError);
                            break;
                    }

                    return -1;
                }

                return result;
            }

            return BasicStreamSocket <Protocol>::write (data, maxSize);
        }

        /**
         * @brief check if the socket is secure.
         * @return true if the socket is secure, false otherwise.
         */
        virtual bool encrypted () const noexcept override
        {
            return (this->_tlsState == TlsState::Encrypted);
        }

        /**
         * @brief set the certificate and the private key.
         * @param cert certificate path.
         * @param key private key path.
         * @return 0 on success, -1 on failure.
         */
        int setCertificate (const std::string& cert, const std::string& key = "")
        {
            if (((this->_tlsHandle) ? SSL_use_certificate_file (this->_tlsHandle.get (), cert.c_str (), SSL_FILETYPE_PEM)
                                    : SSL_CTX_use_certificate_file (this->_tlsContext.get (), cert.c_str (), SSL_FILETYPE_PEM)) == 0)
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            if (key.size ())
            {
                if (((this->_tlsHandle) ? SSL_use_PrivateKey_file (this->_tlsHandle.get (), key.c_str (), SSL_FILETYPE_PEM)
                                        : SSL_CTX_use_PrivateKey_file (this->_tlsContext.get (), key.c_str (), SSL_FILETYPE_PEM)) == 0)
                {
                    lastError = make_error_code (Errc::InvalidParam);
                    return -1;
                }
            }

            if (((this->_tlsHandle) ? SSL_check_private_key (this->_tlsHandle.get ())
                                    : SSL_CTX_check_private_key (this->_tlsContext.get ())) == 0)
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            return 0;
        }

        /**
         * @brief set the location of the trusted CA certificate.
         * @param caFile path of the trusted CA certificate file.
         * @return 0 on success, -1 on failure.
         */
        int setCaCertificate (const std::string& caFile)
        {
            if (SSL_CTX_load_verify_locations (this->_tlsContext.get (), caFile.c_str (), nullptr) == 0)
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            return 0;
        }

        /**
         * @brief Enable/Disable the verification of the peer certificate.
         * @param verify Enable peer verification if set to true, false otherwise.
         * @param depth The maximum certificate verification depth (default: no limit).
         */
        void setVerify (bool verify, int depth = -1)
        {
            if (verify == true)
            {
                if (this->_tlsHandle)
                {
                    SSL_set_verify (this->_tlsHandle.get (), SSL_VERIFY_PEER, verifyWrapper);
                    SSL_set_verify_depth (this->_tlsHandle.get (), depth);
                }
                else
                {
                    SSL_CTX_set_verify (this->_tlsContext.get (), SSL_VERIFY_PEER, verifyWrapper);
                    SSL_CTX_set_verify_depth (this->_tlsContext.get (), depth);
                }
            }
            else
            {
                if (this->_tlsHandle)
                {
                    SSL_set_verify (this->_tlsHandle.get (), SSL_VERIFY_NONE, nullptr);
                }
                else
                {
                    SSL_CTX_set_verify (this->_tlsContext.get (), SSL_VERIFY_NONE, nullptr);
                }
            }
        }

        /**
         * @brief set the cipher list (TLSv1.2 and below).
         * @param cipher the cipher list.
         * @return 0 on success, -1 on failure.
         */
        int setCipher (const std::string &cipher)
        {
            if (((this->_tlsHandle) ? SSL_set_cipher_list (this->_tlsHandle.get (), cipher.c_str ())
                                    : SSL_CTX_set_cipher_list (this->_tlsContext.get (), cipher.c_str ())) == 0)
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            return 0;
        }

    #if OPENSSL_VERSION_NUMBER >= 0x10101000L
        /**
         * @brief set the cipher list (TLSv1.3).
         * @param cipher the cipher list.
         * @return 0 on success, -1 on failure.
         */
        int setCipher_1_3 (const std::string &cipher)
        {
            if (((this->_tlsHandle) ? SSL_set_ciphersuites (this->_tlsHandle.get (), cipher.c_str ())
                                    : SSL_CTX_set_ciphersuites (this->_tlsContext.get (), cipher.c_str ())) == 0)
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            return 0;
        }
    #endif

    protected:
        /**
         * @brief TLS state.
         */
        enum TlsState
        {
            Encrypted,          /**< Socket is encrypted */
            NonEncrypted,       /**< Socket is not encrypted */
        };

        /**
         * @brief Start SSL handshake.
         * @return 0 on success, -1 on failure.
         */
        int startHandshake ()
        {
            // start the SSL handshake.
            int result = SSL_do_handshake (this->_tlsHandle.get ());
            if (result < 1)
            {
                switch (SSL_get_error (this->_tlsHandle.get (), result))
                {
                    case SSL_ERROR_WANT_READ:
                    case SSL_ERROR_WANT_WRITE:
                    case SSL_ERROR_WANT_X509_LOOKUP:
                        // SSL_do_handshake want read or want write.
                        lastError = make_error_code (Errc::TemporaryError);
                        break;
                    case SSL_ERROR_ZERO_RETURN:
                        // a close notify alert was received.
                        // we have to answer by sending a close notify alert too.
                        lastError = make_error_code (TlsErrc::TlsCloseNotifyAlert);
                        break;
                    case SSL_ERROR_SYSCALL:
                        // an error occurred at the socket level.
                        switch (errno)
                        {
                            case 0:
                            case ECONNRESET:
                            case EPIPE:
                                lastError = make_error_code (Errc::ConnectionClosed);
                                this->_state = State::Disconnected;
                                break;
                            default:
                                lastError = std::make_error_code (static_cast <std::errc> (errno));
                                break;
                        }
                        break;
                    default:
                        // SSL protocol error.
                    #ifdef DEBUG
                        std::cout << ERR_reason_error_string (ERR_get_error ()) << std::endl;
                    #endif
                        lastError = make_error_code (TlsErrc::TlsProtocolError);
                        break;
                }

                return -1;
            }

            this->_tlsState = TlsState::Encrypted;

            return 0;
        }

        /**
         * @brief c style callback wrapper for the state information callback.
         * @param ssl SSL objects created from context during connection.
         * @param where information about which context the callback function was called.
         * @param ret error condition.
         */
        static void infoWrapper (const SSL *ssl, int where, int ret)
        {
            assert (ssl);
            static_cast <BasicTlsSocket <Protocol>*> (SSL_get_app_data (ssl))->infoCallback (where, ret);
        }

        /**
         * @brief state information callback.
         * @param where information about which context the callback function was called.
         * @param ret error condition.
         */
        void infoCallback (int where, int ret) const
        {
            if (where & SSL_CB_ALERT)
            {
                std::cout << "SSL/TLS Alert ";
                (where & SSL_CB_READ) ? std::cout << "[read] " : std::cout << "[write] ";
                std::cout << SSL_alert_type_string_long (ret) << ":";
                std::cout << SSL_alert_desc_string_long (ret);
                std::cout << std::endl;
            }
            else if (where & SSL_CB_LOOP)
            {
                std::cout << "SSL/TLS State ";
                (SSL_in_connect_init (this->_tlsHandle.get ())) ? std::cout << "[connect] " : (SSL_in_accept_init (this->_tlsHandle.get ())) ? std::cout << "[accept] " : std::cout << "[undefined] ";
                std::cout << SSL_state_string_long (this->_tlsHandle.get ());
                std::cout << std::endl;
            }
            else if (where & SSL_CB_HANDSHAKE_START)
            {
                std::cout << "SSL/TLS Handshake [Start] "<< SSL_state_string_long (this->_tlsHandle.get ()) << std::endl;
            }
            else if (where & SSL_CB_HANDSHAKE_DONE)
            {
                std::cout << "SSL/TLS Handshake [Done] "<< SSL_state_string_long (this->_tlsHandle.get ()) << std::endl;
                std::cout << SSL_CTX_sess_number (this->_tlsContext.get ()) << " items in the session cache"<< std::endl;
                std::cout << SSL_CTX_sess_connect (this->_tlsContext.get ()) << " client connects"<< std::endl;
                std::cout << SSL_CTX_sess_connect_good (this->_tlsContext.get ()) << " client connects that finished"<< std::endl;
                std::cout << SSL_CTX_sess_connect_renegotiate (this->_tlsContext.get ()) << " client renegotiations requested"<< std::endl;
                std::cout << SSL_CTX_sess_accept (this->_tlsContext.get ()) << " server connects"<< std::endl;
                std::cout << SSL_CTX_sess_accept_good (this->_tlsContext.get ()) << " server connects that finished"<< std::endl;
                std::cout << SSL_CTX_sess_accept_renegotiate (this->_tlsContext.get ()) << " server renegotiations requested"<< std::endl;
                std::cout << SSL_CTX_sess_hits (this->_tlsContext.get ()) << " session cache hits"<< std::endl;
                std::cout << SSL_CTX_sess_cb_hits (this->_tlsContext.get ()) << " external session cache hits"<< std::endl;
                std::cout << SSL_CTX_sess_misses (this->_tlsContext.get ()) << " session cache misses"<< std::endl;
                std::cout << SSL_CTX_sess_timeouts (this->_tlsContext.get ()) << " session cache timeouts"<< std::endl;
                std::cout << "negotiated " << SSL_get_cipher (this->_tlsHandle.get ()) << " cipher suite" << std::endl;
            }
        }

        /**
         * @brief c style callback wrapper for the Trusted CA certificates verification callback.
         * @param preverifiedindicates, whether the verification of the certificate in question was passed or not.
         * @param context pointer to the complete context used for the certificate chain verification.
         * @return when verified successfully, the callback should return 1, 0 otherwise.
         */
        static int verifyWrapper (int preverified, X509_STORE_CTX *context)
        {
            SSL* ssl = static_cast <SSL*> (X509_STORE_CTX_get_ex_data (context, SSL_get_ex_data_X509_STORE_CTX_idx ()));

            assert (ssl);
            return static_cast <BasicTlsSocket <Protocol>*> (SSL_get_app_data (ssl))->verifyCallback (preverified, context);
        }

        /**
         * @brief trusted CA certificates verification callback.
         * @param preverified indicates, whether the verification of the certificate in question was passed or not.
         * @param context pointer to the complete context used for the certificate chain verification.
         * @return when verified successfully, the callback should return 1, 0 otherwise.
         */
        int verifyCallback (int preverified, X509_STORE_CTX *context) const
        {
            int maxDepth = SSL_get_verify_depth (this->_tlsHandle.get ());
            int dpth = X509_STORE_CTX_get_error_depth (context);

            std::cout << "verification started at depth="<< dpth << std::endl;

            // catch a too long certificate chain.
            if ((maxDepth > 0) && (dpth > maxDepth))
            {
                preverified = 0;
                X509_STORE_CTX_set_error (context, X509_V_ERR_CERT_CHAIN_TOO_LONG);
            }

            if (!preverified)
            {
                std::cout << "verification failed at depth=" << dpth << " err=" << X509_STORE_CTX_get_error (context) << std::endl;
                return 0;
            }

            // check the certificate host name.
            if (!verifyCert (context))
            {
                std::cout << "rejected by CERT at depth=" << dpth << std::endl;
                return 0;
            }

            // check the revocation list.
            if (!verifyCrl (context))
            {
                std::cout << "rejected by CRL at depth=" << dpth << std::endl;
                return 0;
            }

            // check ocsp.
            if (!verifyOcsp (context))
            {
                std::cout << "rejected by OCSP at depth=" << dpth << std::endl;
                return 0;
            }

            std::cout << "certificate accepted at depth=" << dpth << std::endl;

            return 1;
        }

        /**
         * @brief verify certificate validity.
         * @param context pointer to the complete context used for the certificate chain verification.
         * @return when verified successfully, the callback should return 1, 0 otherwise.
         */
        int verifyCert (X509_STORE_CTX *context) const
        {
            int depth = X509_STORE_CTX_get_error_depth (context);
            X509* cert = X509_STORE_CTX_get_current_cert (context);

            char buf[256];
            X509_NAME_oneline (X509_get_subject_name (cert), buf, sizeof (buf));
            std::cout << "subject=" << buf << std::endl;

            // check the certificate host name
            if (depth == 0)
            {
                // confirm a match between the hostname and the hostnames listed in the certificate.
                if (!checkHostName (cert))
                {
                    std::cout << "no match for hostname in the certificate" << std::endl;
                    return 0;
                }
            }

            return 1;
        }

        /**
         * @brief confirm a match between the hostname contacted and the hostnames listed in the certificate.
         * @param certificate the server certificate.
         * @return true if an alternative name matched the server hostname.
         */
        bool checkHostName (X509 *certificate) const
        {
            bool match = false;

            // get alternative names.
            /*join::crypto::StackOfGeneralNamePtr altnames (reinterpret_cast <STACK_OF(GENERAL_NAME)*> (X509_get_ext_d2i (certificate, NID_subject_alt_name, 0, 0)));
            if (altnames)
            {
                for (int i = 0; (i < sk_GENERAL_NAME_num (altnames.get ())) && !match; ++i)
                {
                    // get a handle to alternative name.
                    GENERAL_NAME *current_name = sk_GENERAL_NAME_value (altnames.get (), i);

                    if (current_name->type == GEN_DNS)
                    {
                        // get data and length.
                        const char *host = reinterpret_cast <const char *> (ASN1_STRING_get0_data (current_name->d.ia5));
                        size_t len = size_t (ASN1_STRING_length (current_name->d.ia5));
                        std::string pattern (host, host + len), serverName (name);

                        // strip off trailing dots.
                        if (pattern.back () == '.')
                        {
                            pattern.pop_back ();
                        }

                        if (serverName.back () == '.')
                        {
                            serverName.pop_back ();
                        }

                        // compare to pattern.
                        if (fnmatch (pattern.c_str (), serverName.c_str (), 0) == 0)
                        {
                            // an alternative name matched the server hostname.
                            match = true;
                        }
                    }
                }
            }*/

            return match;
        }

        /**
         * @brief verify certificate revocation using CRL.
         * @param context pointer to the complete context used for the certificate chain verification.
         * @return when verified successfully, the callback should return 1, 0 otherwise.
         */
        int verifyCrl (X509_STORE_CTX *context) const
        {
            return 1;
        }

        /**
         * @brief verify certificate revocation using OCSP.
         * @param context pointer to the complete context used for the certificate chain verification.
         * @return when verified successfully, the callback should return 1, 0 otherwise.
         */
        int verifyOcsp (X509_STORE_CTX *context) const
        {
            return 1;
        }

        /// TLS context.
        join::crypto::SslCtxPtr _tlsContext;

        /// TLS handle.
        join::crypto::SslPtr _tlsHandle;

        /// TLS mode
        TlsMode _tlsMode = TlsMode::ClientMode;

        /// TLS state.
        TlsState _tlsState = TlsState::NonEncrypted;

        /// friendship with basic TLS acceptor
        friend class BasicTlsAcceptor <Protocol>;
    };

    /**
     * @brief compare if socket handle is inferior.
     * @param a socket handle to compare.
     * @param b socket handle to compare to.
     * @return true if inferior.
     */
    template <class Protocol>
    constexpr bool operator< (const BasicTlsSocket <Protocol>& a, const BasicTlsSocket <Protocol>& b) noexcept
    {
        return a.handle () < b.handle ();
    }
}

namespace std
{
    /// TLS error code specialization.
    template <> struct is_error_condition_enum <join::TlsErrc> : public true_type {};
}

#endif
