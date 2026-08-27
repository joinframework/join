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

#ifndef JOIN_CORE_STREAM_SOCKET_HPP
#define JOIN_CORE_STREAM_SOCKET_HPP

// libjoin.
#include <join/socket.hpp>

// C++.
#include <chrono>

// C.
#include <sys/socket.h>

namespace join
{
    /**
     * @brief basic stream socket class.
     */
    template <class Protocol>
    class BasicStreamSocket final : public BasicSocket<Protocol>
    {
        /// friendship with basic asynchronous stream socket
        template <class P, class E>
        friend class BasicAsyncStreamSocket;

    public:
        using Ptr = std::unique_ptr<BasicStreamSocket<Protocol>>;
        using Mode = typename BasicSocket<Protocol>::Mode;
        using Option = typename BasicSocket<Protocol>::Option;
        using State = typename BasicSocket<Protocol>::State;
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
        explicit BasicStreamSocket (Mode mode)
        : BasicSocket<Protocol> (mode)
        {
        }

        /**
         * @brief create the socket instance adopting an accepted file descriptor.
         * @param fd accepted file descriptor.
         * @param remote endpoint of the connected peer, as reported by the accept.
         * @param mode blocking mode the file descriptor was accepted with.
         */
        explicit BasicStreamSocket (int fd, const Endpoint& remote, Mode mode = Mode::NonBlocking)
        : BasicSocket<Protocol> (mode)
        {
            this->_handle = fd;
            this->_state = State::Connected;
            this->_protocol = remote.protocol ();
            _remote = remote;

            if (this->protocol () == IPPROTO_TCP)
            {
                setOption (Option::NoDelay, 1);
            }
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
        BasicStreamSocket (BasicStreamSocket&& other) noexcept
        : BasicSocket<Protocol> (std::move (other))
        , _remote (std::move (other._remote))
        {
        }

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicStreamSocket& operator= (BasicStreamSocket&& other) noexcept
        {
            BasicSocket<Protocol>::operator= (std::move (other));

            _remote = std::move (other._remote);

            return *this;
        }

        /**
         * @brief destroy the instance.
         */
        ~BasicStreamSocket () = default;

        /**
         * @brief make a connection to the given endpoint.
         * @param endpoint endpoint to connect to.
         * @return 0 on success, -1 on failure.
         */
        int connect (const Endpoint& endpoint) noexcept
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
            _remote = endpoint;

            if (result == -1)
            {
                lastError = std::error_code (errno, std::generic_category ());
                if (lastError != std::errc::operation_in_progress)
                {
                    close ();
                }
                return -1;
            }

            this->_state = State::Connected;

            return 0;
        }

        /**
         * @brief block until connected.
         * @param timeout timeout in milliseconds.
         * @return true if connected, false otherwise.
         */
        bool waitConnected (int timeout = 0)
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
        int disconnect () noexcept
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
        bool waitDisconnected (int timeout = 0)
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

                    if (disconnect () == 0)
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
         * @brief close the socket handle.
         */
        void close () noexcept override
        {
            BasicSocket<Protocol>::close ();
            _remote = {};
        }

        /**
         * @brief read data.
         * @param data buffer used to store the data received.
         * @param maxSize maximum number of bytes to read.
         * @return the number of bytes received, -1 on failure.
         */
        ssize_t read (char* data, size_t maxSize) noexcept
        {
            ssize_t size = BasicSocket<Protocol>::read (data, maxSize);
            if (size == 0)
            {
                lastError = make_error_code (Errc::ConnectionClosed);
                return -1;
            }

            return size;
        }

        /**
         * @brief read data until size is reached or an error occurred.
         * @param data buffer used to store the data received.
         * @param size number of bytes to read.
         * @param timeout timeout in milliseconds.
         * @return 0 on success, -1 on failure.
         */
        int readExactly (char* data, size_t size, int timeout = 0) noexcept
        {
            size_t numRead = 0;

            while (numRead < size)
            {
                ssize_t result = this->read (data + numRead, size - numRead);
                if (result == -1)
                {
                    if (lastError == Errc::TemporaryError)
                    {
                        if (this->waitReadyRead (timeout))
                        {
                            continue;
                        }
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
        int writeExactly (const char* data, size_t size, int timeout = 0) noexcept
        {
            size_t numWrite = 0;

            while (numWrite < size)
            {
                ssize_t result = this->write (data + numWrite, size - numWrite);
                if (result == -1)
                {
                    if (lastError == Errc::TemporaryError)
                    {
                        if (this->waitReadyWrite (timeout))
                        {
                            continue;
                        }
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
        int setOption (Option option, int value) noexcept override
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
            return _remote;
        }

        /**
         * @brief check if the socket is connecting.
         * @return true if connecting, false otherwise.
         */
        bool connecting () const noexcept
        {
            return (this->_state == State::Connecting);
        }

        /**
         * @brief check if the socket is connected.
         * @return true if connected, false otherwise.
         * @note this method probes SO_ERROR and updates the internal state when
         *       a pending connection completes, it is therefore not const.
         */
        bool connected () noexcept
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

    protected:
        /// remote endpoint.
        Endpoint _remote;
    };

    /**
     * @brief compare if socket handle is inferior.
     * @param a socket handle to compare.
     * @param b socket handle to compare to.
     * @return true if inferior.
     */
    template <class Protocol>
    inline bool operator< (const BasicStreamSocket<Protocol>& a, const BasicStreamSocket<Protocol>& b) noexcept
    {
        return a.handle () < b.handle ();
    }
}

#endif
