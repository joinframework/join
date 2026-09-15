/**
 * MIT License
 *
 * Copyright (c) 2026 Mathieu Rabine
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

#ifndef JOIN_CORE_RAW_SOCKET_HPP
#define JOIN_CORE_RAW_SOCKET_HPP

// libjoin.
#include <join/socket.hpp>

// C.
#include <netinet/tcp.h>
#include <sys/ioctl.h>

namespace join
{
    /**
     * @brief basic raw socket class.
     */
    template <class Protocol>
    class BasicRawSocket : public BasicSocket<Protocol>
    {
    public:
        using Ptr = std::unique_ptr<BasicRawSocket<Protocol>>;
        using Mode = typename BasicSocket<Protocol>::Mode;
        using State = typename BasicSocket<Protocol>::State;
        using Endpoint = typename Protocol::Endpoint;

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
         * @brief default constructor.
         */
        BasicRawSocket () noexcept
        : BasicRawSocket (Mode::NonBlocking)
        {
        }

        /**
         * @brief create socket instance specifying the mode.
         * @param mode blocking mode.
         */
        explicit BasicRawSocket (Mode mode) noexcept
        : BasicSocket<Protocol> (mode)
        {
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicRawSocket (const BasicRawSocket& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        BasicRawSocket& operator= (const BasicRawSocket& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        BasicRawSocket (BasicRawSocket&& other) noexcept = default;

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        BasicRawSocket& operator= (BasicRawSocket&& other) noexcept = default;

        /**
         * @brief destroy the socket instance.
         */
        virtual ~BasicRawSocket () = default;

        /**
         * @brief assigns the specified endpoint to the socket.
         * @param endpoint endpoint to assign to the socket.
         * @return 0 on success, -1 on failure.
         */
        int bind (const Endpoint& endpoint) noexcept override
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
                setOption (Option::ReuseAddr, 1);
            }
            else if (endpoint.protocol ().family () == AF_UNIX)
            {
                ::unlink (endpoint.device ().c_str ());
            }

            return BasicSocket<Protocol>::bind (endpoint);
        }

        /**
         * @brief assigns the specified device to the socket.
         * @param device device name.
         * @return 0 on success, -1 on failure.
         */
        int bindToDevice (const std::string& device) noexcept
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

            if ((this->family () == AF_INET6) || (this->family () == AF_INET))
            {
                setOption (Option::ReuseAddr, 1);
            }

            int result = ::setsockopt (this->_handle, SOL_SOCKET, SO_BINDTODEVICE, device.c_str (), device.size ());
            if (result == -1)
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
        ssize_t canRead () const noexcept
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
         * @brief read data.
         * @param data buffer used to store the data received.
         * @param maxSize maximum number of bytes to read.
         * @return the number of bytes received, -1 on failure.
         */
        ssize_t read (char* data, size_t maxSize) noexcept
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

            ssize_t size = ::recvmsg (this->_handle, &message, 0);
            if (size == -1)
            {
                lastError = std::error_code (errno, std::generic_category ());
                return -1;
            }

            if (message.msg_flags & MSG_TRUNC)
            {
                lastError = make_error_code (Errc::MessageTooLong);
                return -1;
            }

            return size;
        }

        /**
         * @brief write data.
         * @param data data buffer to send.
         * @param maxSize maximum number of bytes to write.
         * @return the number of bytes written, -1 on failure.
         */
        ssize_t write (const char* data, size_t maxSize) noexcept
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

            ssize_t result = ::sendmsg (this->_handle, &message, 0);
            if (result == -1)
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
    };
}

#endif
