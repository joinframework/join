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

#ifndef JOIN_CORE_ASYNC_DATAGRAM_SOCKET_HPP
#define JOIN_CORE_ASYNC_DATAGRAM_SOCKET_HPP

// libjoin.
#include <join/datagram_socket.hpp>
#include <join/async_socket.hpp>

// C++.
#include <system_error>
#include <utility>

namespace join
{
    /**
     * @brief asynchronous datagram socket class.
     */
    template <class Protocol, class Engine>
    class BasicAsyncDatagramSocket : public BasicAsyncSocket<Protocol, Engine>
    {
    public:
        using Socket = BasicDatagramSocket<Protocol>;
        using Endpoint = typename Protocol::Endpoint;

        /// handler invoked on read completion.
        using ReadHandler = typename BasicAsyncSocket<Protocol, Engine>::ReadHandler;

        /// handler invoked on write completion.
        using WriteHandler = typename BasicAsyncSocket<Protocol, Engine>::WriteHandler;

        /**
         * @brief create the socket instance.
         * @param engine engine driving the operations.
         */
        explicit BasicAsyncDatagramSocket (Engine& engine = ProactorThread::proactor ())
        : BasicAsyncSocket<Protocol, Engine> (engine)
        {
        }

        /**
         * @brief create the socket instance specifying the time to live.
         * @param ttl packet time to live.
         * @param engine engine driving the operations.
         */
        explicit BasicAsyncDatagramSocket (int ttl, Engine& engine = ProactorThread::proactor ())
        : BasicAsyncSocket<Protocol, Engine> (Socket (ttl), engine)
        {
        }

        /**
         * @brief create the socket instance adopting an already opened socket.
         * @param sock socket to adopt.
         * @param engine engine driving the operations.
         */
        explicit BasicAsyncDatagramSocket (Socket&& sock, Engine& engine = ProactorThread::proactor ())
        : BasicAsyncSocket<Protocol, Engine> (std::move (sock), engine)
        {
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicAsyncDatagramSocket (const BasicAsyncDatagramSocket& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicAsyncDatagramSocket& operator= (const BasicAsyncDatagramSocket& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        BasicAsyncDatagramSocket (BasicAsyncDatagramSocket&& other) = delete;

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicAsyncDatagramSocket& operator= (BasicAsyncDatagramSocket&& other) = delete;

        /**
         * @brief destroy the socket instance.
         */
        ~BasicAsyncDatagramSocket ()
        {
            this->close ();
        }

        /**
         * @brief assign the default remote endpoint for this socket.
         * @param endpoint endpoint to assign.
         * @return 0 on success, -1 on failure.
         */
        int connect (const Endpoint& endpoint) noexcept
        {
            return this->_socket.connect (endpoint);
        }

        /**
         * @brief remove the default remote endpoint.
         * @return 0 on success, -1 on failure.
         */
        int disconnect () noexcept
        {
            return this->_socket.disconnect ();
        }

        /**
         * @brief start an asynchronous read, reporting the endpoint the data are coming from.
         * @param data buffer used to store the data received, valid until the handler is invoked.
         * @param maxSize maximum number of bytes to read.
         * @param endpoint endpoint from where data are coming, valid until the handler is invoked.
         * @param handler handler invoked on completion.
         * @return 0 on success, -1 on failure.
         */
        int asyncReadFrom (char* data, size_t maxSize, Endpoint& endpoint, ReadHandler handler) noexcept
        {
            if (JOIN_UNLIKELY (!this->_socket.opened ()))
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            AsyncOperation::State expected = AsyncOperation::Idle;

            if (!this->_ops->read.state.compare_exchange_strong (expected, AsyncOperation::Pending,
                                                                 std::memory_order_acquire, std::memory_order_acquire))
            {
                if ((expected != AsyncOperation::Dispatching) || !this->_engine->isProactorThread ())
                {
                    lastError = make_error_code (Errc::InUse);
                    return -1;
                }

                this->_ops->read.state.store (AsyncOperation::Pending, std::memory_order_release);
            }

            this->_onRead = std::move (handler);
            this->_ops->readIov.iov_base = data;
            this->_ops->readIov.iov_len = maxSize;
            this->_ops->readMsg.msg_name = endpoint.addr ();
            this->_ops->readMsg.msg_namelen = sizeof (struct sockaddr_storage);
            this->_ops->readMsg.msg_iov = &this->_ops->readIov;
            this->_ops->readMsg.msg_iovlen = 1;
            this->_ops->readMsg.msg_control = nullptr;
            this->_ops->readMsg.msg_controllen = 0;
            this->_ops->readMsg.msg_flags = 0;
            this->_ops->read.op = IoOperation::makeRecvmsg (this->_socket.handle (), &this->_ops->readMsg, 0, this);

            if (this->_engine->submit (&this->_ops->read.op, true, false) == -1)
            {
                // LCOV_EXCL_START
                this->_ops->read.state.store (AsyncOperation::Idle, std::memory_order_release);
                this->_onRead.reset ();
                return -1;
                // LCOV_EXCL_STOP
            }

            return 0;
        }

        /**
         * @brief start an asynchronous write to the given endpoint.
         * @param data data buffer to send, valid until the handler is invoked.
         * @param size number of bytes to write.
         * @param endpoint endpoint where to write the data, valid until the handler is invoked.
         * @param handler handler invoked on completion.
         * @return 0 on success, -1 on failure.
         */
        int asyncWriteTo (const char* data, size_t size, Endpoint& endpoint, WriteHandler handler) noexcept
        {
            if (!this->_socket.opened () && (this->_socket.open (endpoint.protocol ()) == -1))
            {
                return -1;  // LCOV_EXCL_LINE
            }

            AsyncOperation::State expected = AsyncOperation::Idle;

            if (!this->_ops->write.state.compare_exchange_strong (expected, AsyncOperation::Pending,
                                                                  std::memory_order_acquire, std::memory_order_acquire))
            {
                if ((expected != AsyncOperation::Dispatching) || !this->_engine->isProactorThread ())
                {
                    lastError = make_error_code (Errc::InUse);
                    return -1;
                }

                this->_ops->write.state.store (AsyncOperation::Pending, std::memory_order_release);
            }

            this->_onWrite = std::move (handler);
            this->_ops->writeIov.iov_base = const_cast<char*> (data);
            this->_ops->writeIov.iov_len = size;
            this->_ops->writeMsg.msg_name = endpoint.addr ();
            this->_ops->writeMsg.msg_namelen = endpoint.length ();
            this->_ops->writeMsg.msg_iov = &this->_ops->writeIov;
            this->_ops->writeMsg.msg_iovlen = 1;
            this->_ops->writeMsg.msg_control = nullptr;
            this->_ops->writeMsg.msg_controllen = 0;
            this->_ops->writeMsg.msg_flags = 0;
            this->_ops->write.op =
                IoOperation::makeSendmsg (this->_socket.handle (), &this->_ops->writeMsg, MSG_NOSIGNAL, this);

            if (this->_engine->submit (&this->_ops->write.op, true, false) == -1)
            {
                // LCOV_EXCL_START
                this->_ops->write.state.store (AsyncOperation::Idle, std::memory_order_release);
                this->_onWrite.reset ();
                return -1;
                // LCOV_EXCL_STOP
            }

            return 0;
        }

        /**
         * @brief determine the remote endpoint associated with this socket.
         * @return remote endpoint.
         */
        const Endpoint& remoteEndpoint () const noexcept
        {
            return this->_socket.remoteEndpoint ();
        }

        /**
         * @brief check if the socket is connected.
         * @return true if connected, false otherwise.
         */
        bool connected () const noexcept
        {
            return this->_socket.connected ();
        }

        /**
         * @brief get socket mtu.
         * @return mtu on success, -1 on failure.
         */
        int mtu () const noexcept
        {
            return this->_socket.mtu ();
        }

        /**
         * @brief returns the Time-To-Live value.
         * @return the Time-To-Live value.
         */
        int ttl () const noexcept
        {
            return this->_socket.ttl ();
        }
    };
}

#endif
