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
    template <class Protocol, class Proactor>
    class BasicAsyncDatagramSocket : public BasicAsyncSocket<Protocol, Proactor>
    {
    public:
        using Socket = BasicDatagramSocket<Protocol>;
        using Endpoint = typename Protocol::Endpoint;
        using AsyncOperation = BasicAsyncOperation<Protocol, Proactor>;
        using AsyncRead = BasicAsyncRead<Protocol, Proactor>;
        using AsyncWrite = BasicAsyncWrite<Protocol, Proactor>;
        using State = typename AsyncOperation::State;
        using ReadHandler = typename AsyncRead::Handler;
        using WriteHandler = typename AsyncWrite::Handler;

        /**
         * @brief create the socket instance.
         * @param proactor proactor driving the operations.
         */
        explicit BasicAsyncDatagramSocket (Proactor& proactor = ProactorThread::proactor ())
        : BasicAsyncSocket<Protocol, Proactor> (proactor)
        {
        }

        /**
         * @brief create the socket instance specifying the time to live.
         * @param ttl packet time to live.
         * @param proactor proactor driving the operations.
         */
        explicit BasicAsyncDatagramSocket (int ttl, Proactor& proactor = ProactorThread::proactor ())
        : BasicAsyncSocket<Protocol, Proactor> (Socket (ttl), proactor)
        {
        }

        /**
         * @brief create the socket instance adopting an already opened socket.
         * @param sock socket to adopt.
         * @param proactor proactor driving the operations.
         */
        explicit BasicAsyncDatagramSocket (Socket&& sock, Proactor& proactor = ProactorThread::proactor ())
        : BasicAsyncSocket<Protocol, Proactor> (std::move (sock), proactor)
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
        BasicAsyncDatagramSocket (BasicAsyncDatagramSocket&& other) noexcept = default;

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicAsyncDatagramSocket& operator= (BasicAsyncDatagramSocket&& other) noexcept = default;

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
            if (JOIN_UNLIKELY (this->_readOp == nullptr))
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            if (JOIN_UNLIKELY (!this->_socket.opened ()))
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            if (this->_readOp->reserve (*this->_proactor) == -1)
            {
                return -1;
            }

            this->_readOp->_handler = std::move (handler);
            this->_readOp->_iov.iov_base = data;
            this->_readOp->_iov.iov_len = maxSize;
            this->_readOp->_msg.msg_name = endpoint.addr ();
            this->_readOp->_msg.msg_namelen = sizeof (struct sockaddr_storage);
            this->_readOp->_msg.msg_iov = &this->_readOp->_iov;
            this->_readOp->_msg.msg_iovlen = 1;
            this->_readOp->_msg.msg_control = nullptr;
            this->_readOp->_msg.msg_controllen = 0;
            this->_readOp->_msg.msg_flags = 0;
            this->_readOp->_op = IoOperation::makeRecvmsg (this->_socket.handle (), &this->_readOp->_msg, 0, this);

            if (this->_proactor->submit (&this->_readOp->_op, true, false) == -1)
            {
                // LCOV_EXCL_START
                this->_readOp->release ();
                this->_readOp->_handler.reset ();
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
            if (JOIN_UNLIKELY (this->_writeOp == nullptr))
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            if (!this->_socket.opened () && (this->_socket.open (endpoint.protocol ()) == -1))
            {
                return -1;  // LCOV_EXCL_LINE
            }

            if (this->_writeOp->reserve (*this->_proactor) == -1)
            {
                return -1;
            }

            this->_writeOp->_handler = std::move (handler);
            this->_writeOp->_iov.iov_base = const_cast<char*> (data);
            this->_writeOp->_iov.iov_len = size;
            this->_writeOp->_msg.msg_name = endpoint.addr ();
            this->_writeOp->_msg.msg_namelen = endpoint.length ();
            this->_writeOp->_msg.msg_iov = &this->_writeOp->_iov;
            this->_writeOp->_msg.msg_iovlen = 1;
            this->_writeOp->_msg.msg_control = nullptr;
            this->_writeOp->_msg.msg_controllen = 0;
            this->_writeOp->_msg.msg_flags = 0;
            this->_writeOp->_op =
                IoOperation::makeSendmsg (this->_socket.handle (), &this->_writeOp->_msg, MSG_NOSIGNAL, this);

            if (this->_proactor->submit (&this->_writeOp->_op, true, false) == -1)
            {
                // LCOV_EXCL_START
                this->_writeOp->release ();
                this->_writeOp->_handler.reset ();
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
