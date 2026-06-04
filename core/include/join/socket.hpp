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

// Libraries.
#include <openssl/err.h>

// C++.
#include <type_traits>
#include <iostream>

// C.
#include <netinet/tcp.h>
#include <linux/icmp.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fnmatch.h>
#include <cassert>
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
    public:
        using Ptr = std::unique_ptr<BasicSocket<Protocol>>;
        using Proto = Protocol;
        using Endpoint = typename Protocol::Endpoint;

        /**
         * @brief socket modes.
         */
        enum Mode
        {
            Blocking,    /**< the socket will block */
            NonBlocking, /**< the socket will not block */
        };

        /**
         * @brief socket options.
         */
        enum Option
        {
            NoDelay,         /**< set the TCP_NODELAY option to disable/enable the nagle's algorithm. */
            KeepAlive,       /**< set the SO_KEEPALIVE option. */
            KeepIdle,        /**< set the keepalive idle timeout option. */
            KeepIntvl,       /**< set the keepalive probe interval option. */
            KeepCount,       /**< set the keepalive probe count option. */
            SndBuffer,       /**< set the socket send buffer size at the OS level. */
            RcvBuffer,       /**< set the socket receive buffer size at the OS level. */
            TimeStamp,       /**< enable or disable the receiving of the SO_TIMESTAMP control message. */
            ReuseAddr,       /**< allow reuse of local addresses. */
            ReusePort,       /**< permits multiple sockets to be bound to an identical socket address. */
            Broadcast,       /**< allow datagram sockets to send packets to a broadcast address. */
            Ttl,             /**< set the time-to-live value of outgoing packets. */
            MulticastLoop,   /**< determines whether multicast packets should be looped back to the local sockets. */
            MulticastTtl,    /**< set the time-to-live value of outgoing multicast packets. */
            PathMtuDiscover, /**< set the Path MTU Discovery setting for a socket. */
            RcvError,        /**< enable extended reliable error message passing. */
            AuxData,         /**< enable extended metadata message passing. */
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
                lastError = std::error_code (errno, std::generic_category ());
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
            if ((this->_state == State::Closed) && (this->open (endpoint.protocol ()) == -1))
            {
                return -1;
            }

            if (endpoint.protocol ().family () == AF_PACKET)
            {
                if (reinterpret_cast<const struct sockaddr_ll*> (endpoint.addr ())->sll_ifindex == 0)
                {
                    lastError = std::make_error_code (std::errc::no_such_device);
                    return -1;
                }
            }
            else if ((endpoint.protocol ().family () == AF_INET6) || (endpoint.protocol ().family () == AF_INET))
            {
                this->setOption (Option::ReuseAddr, 1);
            }
            else if (endpoint.protocol ().family () == AF_UNIX)
            {
                ::unlink (endpoint.device ().c_str ());
            }

            if (::bind (this->_handle, endpoint.addr (), endpoint.length ()) == -1)
            {
                lastError = std::error_code (errno, std::generic_category ());
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
            int available = 0;

            // check if data can be read in the socket internal buffer.
            if (::ioctl (this->_handle, FIONREAD, &available) == -1)
            {
                lastError = std::error_code (errno, std::generic_category ());
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
            return (this->wait (true, false, timeout) == 0);
        }

        /**
         * @brief read data.
         * @param data buffer used to store the data received.
         * @param maxSize maximum number of bytes to read.
         * @return the number of bytes received, -1 on failure.
         */
        virtual int read (char* data, unsigned long maxSize) noexcept
        {
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
                    lastError = std::error_code (errno, std::generic_category ());
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
            return (this->wait (false, true, timeout) == 0);
        }

        /**
         * @brief write data.
         * @param data data buffer to send.
         * @param maxSize maximum number of bytes to write.
         * @return the number of bytes written, -1 on failure.
         */
        virtual int write (const char* data, unsigned long maxSize) noexcept
        {
            struct iovec iov;
            iov.iov_base = const_cast<char*> (data);
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
                lastError = std::error_code (errno, std::generic_category ());
                return -1;
            }

            return result;
        }

        /**
         * @brief set the socket to the non-blocking or blocking mode.
         * @param mode blocking mode.
         */
        void setMode (Mode mode) noexcept
        {
            this->_mode = mode;

            if (this->_state != State::Closed)
            {
                int flags = ::fcntl (this->_handle, F_GETFL, 0);

                if (this->_mode == Mode::NonBlocking)
                {
                    flags = flags | O_NONBLOCK;
                }
                else
                {
                    flags = flags & ~O_NONBLOCK;
                }

                ::fcntl (this->_handle, F_SETFL, flags);
            }
        }

        /**
         * @brief set the given option to the given value.
         * @param option socket option.
         * @param value option value.
         * @return 0 on success, -1 on failure.
         */
        virtual int setOption (Option option, int value) noexcept
        {
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
                lastError = std::error_code (errno, std::generic_category ());
                return -1;
            }

            return 0;
        }

        /**
         * @brief determine the local endpoint associated with this socket.
         * @return local endpoint.
         */
        Endpoint localEndpoint () const noexcept
        {
            struct sockaddr_storage sa;
            socklen_t sa_len = sizeof (struct sockaddr_storage);

            if (::getsockname (this->_handle, reinterpret_cast<struct sockaddr*> (&sa), &sa_len) == -1)
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
#if __BYTE_ORDER == __LITTLE_ENDIAN
                sum += *reinterpret_cast<const uint8_t*> (data);
#else
                sum += *reinterpret_cast<const uint8_t*> (data) << 8;
#endif
            }

            sum = (sum >> 16) + (sum & 0xffff);
            sum += (sum >> 16);

            return static_cast<uint16_t> (~sum);
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
            struct pollfd handle = {.fd = this->_handle, .events = 0, .revents = 0};

            if (wantRead)
            {
                handle.events |= POLLIN;
            }

            if (wantWrite)
            {
                handle.events |= POLLOUT;
            }

            int nset = (handle.fd > -1) ? ::poll (&handle, 1, timeout == 0 ? -1 : timeout) : -1;
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

        /// socket state.
        State _state = State::Closed;

        /// socket mode.
        Mode _mode = Mode::NonBlocking;

        /// socket handle.
        int _handle = -1;

        /// protocol.
        Protocol _protocol;

        /// friendship with TLS wrapper.
        template <class Socket>
        friend class TlsWrapper;
    };

    /**
     * @brief compare if handle is inferior.
     * @param a socket handle to compare.
     * @param b socket handle to compare to.
     * @return true if inferior.
     */
    template <class Protocol>
    constexpr bool operator< (const BasicSocket<Protocol>& a, const BasicSocket<Protocol>& b) noexcept
    {
        return a.handle () < b.handle ();
    }

    /**
     * @brief basic datagram socket class.
     */
    template <class Protocol>
    class BasicDatagramSocket : public BasicSocket<Protocol>
    {
    public:
        using Ptr = std::unique_ptr<BasicDatagramSocket<Protocol>>;
        using Proto = Protocol;
        using Mode = typename BasicSocket<Protocol>::Mode;
        using Option = typename BasicSocket<Protocol>::Option;
        using State = typename BasicSocket<Protocol>::State;
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
        BasicDatagramSocket (BasicDatagramSocket&& other)
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
        BasicDatagramSocket& operator= (BasicDatagramSocket&& other)
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
        virtual ~BasicDatagramSocket () = default;

        /**
         * @brief open socket using the given protocol.
         * @param protocol protocol to use.
         * @return 0 on success, -1 on failure.
         */
        virtual int open (const Protocol& protocol = Protocol ()) noexcept override
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
                    this->close ();
                    return -1;
                }
            }

            if ((protocol.protocol () == IPPROTO_ICMPV6) || (protocol.protocol () == IPPROTO_ICMP))
            {
                if ((protocol.family () == AF_INET) &&
                    (::setsockopt (this->_handle, IPPROTO_IP, IP_HDRINCL, &off, sizeof (off)) == -1))
                {
                    lastError = std::error_code (errno, std::generic_category ());
                    this->close ();
                    return -1;
                }

                this->setOption (Option::MulticastTtl, this->_ttl);
                this->setOption (Option::Ttl, this->_ttl);
            }

            return 0;
        }

        /**
         * @brief assigns the specified device to the socket.
         * @param device device name.
         * @return 0 on success, -1 on failure.
         */
        virtual int bindToDevice (const std::string& device) noexcept
        {
            if (this->_state == State::Closed)
            {
                lastError = make_error_code (Errc::ConnectionClosed);
                return -1;
            }

            if (this->_state == State::Connected)
            {
                lastError = make_error_code (Errc::InUse);
                return -1;
            }

            if ((this->_protocol.family () == AF_INET6) || (this->_protocol.family () == AF_INET))
            {
                this->setOption (Option::ReuseAddr, 1);
            }

            int result = setsockopt (this->_handle, SOL_SOCKET, SO_BINDTODEVICE, device.c_str (), device.size ());
            if (result == -1)
            {
                lastError = std::error_code (errno, std::generic_category ());
                return -1;
            }

            return 0;
        }

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
            this->_remote = endpoint;

            if (result == -1)
            {
                lastError = std::error_code (errno, std::generic_category ());
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
                this->_remote = {};
            }

            return 0;
        }

        /**
         * @brief close the socket handle.
         */
        virtual void close () noexcept override
        {
            BasicSocket<Protocol>::close ();
            this->_remote = {};
        }

        /**
         * @brief read data.
         * @param data buffer used to store the data received.
         * @param maxSize maximum number of bytes to read.
         * @return the number of bytes received, -1 on failure.
         */
        virtual int read (char* data, unsigned long maxSize) noexcept override
        {
            return BasicSocket<Protocol>::read (data, maxSize);
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
            struct sockaddr_storage sa;
            socklen_t sa_len = sizeof (struct sockaddr_storage);

            int size = ::recvfrom (this->_handle, data, maxSize, 0, reinterpret_cast<struct sockaddr*> (&sa), &sa_len);
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

            if (endpoint != nullptr)
            {
                *endpoint = Endpoint (reinterpret_cast<struct sockaddr*> (&sa), sa_len);
            }

            return size;
        }

        /**
         * @brief write data.
         * @param data data buffer to send.
         * @param maxSize maximum number of bytes to write.
         * @return the number of bytes written, -1 on failure.
         */
        virtual int write (const char* data, unsigned long maxSize) noexcept override
        {
            return BasicSocket<Protocol>::write (data, maxSize);
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
                lastError = std::error_code (errno, std::generic_category ());
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
                lastError = std::error_code (errno, std::generic_category ());
                return -1;
            }

            return 0;
        }

        /**
         * @brief determine the remote endpoint associated with this socket.
         * @return remote endpoint.
         */
        const Endpoint& remoteEndpoint () const noexcept
        {
            return this->_remote;
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
            return this->_ttl;
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
    constexpr bool operator< (const BasicDatagramSocket<Protocol>& a, const BasicDatagramSocket<Protocol>& b) noexcept
    {
        return a.handle () < b.handle ();
    }

    /**
     * @brief basic stream socket class.
     */
    template <class Protocol>
    class BasicStreamSocket : public BasicDatagramSocket<Protocol>
    {
    public:
        using Ptr = std::unique_ptr<BasicStreamSocket<Protocol>>;
        using Proto = Protocol;
        using Mode = typename BasicDatagramSocket<Protocol>::Mode;
        using Option = typename BasicDatagramSocket<Protocol>::Option;
        using State = typename BasicDatagramSocket<Protocol>::State;
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
        : BasicDatagramSocket<Protocol> (mode)
        {
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicStreamSocket (const BasicStreamSocket& other) = delete;

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
        : BasicDatagramSocket<Protocol> (std::move (other))
        {
        }

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicStreamSocket& operator= (BasicStreamSocket&& other)
        {
            BasicDatagramSocket<Protocol>::operator= (std::move (other));

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
                        elapsed = std::chrono::duration_cast<std::chrono::milliseconds> (
                                      std::chrono::steady_clock::now () - start)
                                      .count ();
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
        int readExactly (char* data, unsigned long size, int timeout = 0)
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
         * @brief read data until size is reached or an error occurred.
         * @param data buffer used to store the data received.
         * @param size number of bytes to read.
         * @param timeout timeout in milliseconds.
         * @return 0 on success, -1 on failure.
         */
        int readExactly (std::string& data, unsigned long size, int timeout = 0)
        {
            data.resize (size);
            return readExactly (&data[0], size, timeout);
        }

        /**
         * @brief write data until size is reached or an error occurred.
         * @param data data buffer to send.
         * @param size number of bytes to write.
         * @param timeout timeout in milliseconds.
         * @return 0 on success, -1 on failure.
         */
        int writeExactly (const char* data, unsigned long size, int timeout = 0)
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
                    return BasicDatagramSocket<Protocol>::setOption (option, value);
            }

            int result = ::setsockopt (this->_handle, optlevel, optname, &value, sizeof (value));
            if (result == -1)
            {
                lastError = std::error_code (errno, std::generic_category ());
                return -1;
            }

            return 0;
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
                return false;
            }

            int optval;
            socklen_t optlen = sizeof (optval);

            int result = ::getsockopt (this->_handle, SOL_SOCKET, SO_ERROR, &optval, &optlen);
            if ((result == -1) || (optval != 0))
            {
                return false;
            }

            this->_state = State::Connected;

            return true;
        }

        /// friendship with basic stream acceptor
        friend class BasicStreamAcceptor<Protocol>;
    };

    /**
     * @brief compare if socket handle is inferior.
     * @param a socket handle to compare.
     * @param b socket handle to compare to.
     * @return true if inferior.
     */
    template <class Protocol>
    constexpr bool operator< (const BasicStreamSocket<Protocol>& a, const BasicStreamSocket<Protocol>& b) noexcept
    {
        return a.handle () < b.handle ();
    }
}

#endif
