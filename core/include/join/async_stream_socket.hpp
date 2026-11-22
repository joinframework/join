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

#ifndef JOIN_CORE_ASYNC_STREAM_SOCKET_HPP
#define JOIN_CORE_ASYNC_STREAM_SOCKET_HPP

// libjoin.
#include <join/async_raw_socket.hpp>
#include <join/stream_socket.hpp>

// C++.
#include <system_error>
#include <utility>

// C.
#include <sys/socket.h>
#include <cstddef>

namespace join
{
    /**
     * @brief asynchronous stream socket class.
     */
    template <class Protocol, class ProactorType, size_t OpCount>
    class BasicAsyncStreamSocket : public BasicAsyncRawSocket<Protocol, ProactorType, OpCount>
    {
    public:
        using Socket = typename Protocol::Socket;
        using Endpoint = typename Protocol::Endpoint;
        using AsyncRead = BasicAsyncRead<Protocol, ProactorType>;
        using AsyncWrite = BasicAsyncWrite<Protocol, ProactorType>;
        using ConnectHandler = typename AsyncWrite::Connect;
        using ReadHandler = typename AsyncRead::Read;
        using WriteHandler = typename AsyncWrite::Write;

        /**
         * @brief create the socket instance.
         * @param proactor proactor driving the operations.
         */
        explicit BasicAsyncStreamSocket (ProactorType& proactor = ProactorThread::proactor ())
        : BasicAsyncRawSocket<Protocol, ProactorType, OpCount> (proactor)
        {
        }

        /**
         * @brief create the socket instance adopting an already connected socket.
         * @param sock socket to adopt.
         * @param proactor proactor driving the operations.
         */
        explicit BasicAsyncStreamSocket (Socket&& sock, ProactorType& proactor = ProactorThread::proactor ())
        : BasicAsyncRawSocket<Protocol, ProactorType, OpCount> (std::move (sock), proactor)
        {
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicAsyncStreamSocket (const BasicAsyncStreamSocket& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicAsyncStreamSocket& operator= (const BasicAsyncStreamSocket& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        BasicAsyncStreamSocket (BasicAsyncStreamSocket&& other) noexcept = default;

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicAsyncStreamSocket& operator= (BasicAsyncStreamSocket&& other) noexcept = default;

        /**
         * @brief start an asynchronous connection to the given endpoint.
         * @param handler handler invoked on completion.
         * @param endpoint endpoint to connect to.
         * @return 0 on success, -1 on failure.
         */
        int asyncConnect (ConnectHandler handler, const Endpoint& endpoint) noexcept
        {
            if (JOIN_UNLIKELY (!this->_allocator.hasBackend ()))
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            if (this->_socket.connected () || this->_socket.connecting ())
            {
                lastError = make_error_code (Errc::InUse);
                return -1;
            }

            if (!this->_socket.opened () && (this->_socket.open (endpoint.protocol ()) == -1))
            {
                return -1;  // LCOV_EXCL_LINE
            }

            AsyncWrite* connect = this->allocateWrite ();
            if (JOIN_UNLIKELY (connect == nullptr))
            {
                // LCOV_EXCL_START
                lastError = make_error_code (Errc::OutOfMemory);
                return -1;
                // LCOV_EXCL_STOP
            }

            this->_socket._state = Socket::Connecting;
            this->_socket._remote = endpoint;
            connect->connectHandler = std::move (handler);
            connect->op = IoOperation::makeConnect (this->_socket.handle (), this->_socket._remote.addr (),
                                                    this->_socket._remote.length (), this);
            connect->op.state.store (IoOperation::State::Submitted, std::memory_order_release);

            if (this->_proactor->submit (connect->op, true, false) == -1)
            {
                // LCOV_EXCL_START
                this->releaseOperation (connect);
                this->_socket.close ();
                return -1;
                // LCOV_EXCL_STOP
            }

            return 0;
        }

        /**
         * @brief start an asynchronous read.
         * @param handler handler invoked on completion.
         * @param data buffer used to store the data received, valid until the handler is invoked.
         * @param maxSize maximum number of bytes to read.
         * @param flush flush the submission queue.
         * @param link link this operation to the next one submitted.
         * @return index of the operation on success, -1 on failure.
         */
        ssize_t asyncRead (ReadHandler handler, char* data, size_t maxSize, bool flush = true,
                           bool link = false) noexcept
        {
            if (JOIN_UNLIKELY (!this->_socket.opened ()))
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            AsyncRead* read = this->allocateRead ();
            if (JOIN_UNLIKELY (read == nullptr))
            {
                lastError = make_error_code (Errc::OutOfMemory);
                return -1;
            }

            read->readHandler = std::move (handler);
            read->op = IoOperation::makeRecv (this->_socket.handle (), data, maxSize, 0, this, link);
            read->op.state.store (IoOperation::State::Submitted, std::memory_order_release);

            size_t index = this->_allocator.getIndex (read);

            if (this->_proactor->submit (read->op, flush, false) == -1)
            {
                // LCOV_EXCL_START
                this->releaseOperation (read);
                return -1;
                // LCOV_EXCL_STOP
            }

            return static_cast<ssize_t> (index);
        }

        /**
         * @brief start an asynchronous write, without copy if the data belong to the proactor fixed buffers.
         * @param handler handler invoked on completion.
         * @param data data buffer to send, valid until the handler is invoked.
         * @param size number of bytes to write.
         * @param flush flush the submission queue.
         * @param link link this operation to the next one submitted.
         * @return index of the operation on success, -1 on failure.
         */
        ssize_t asyncWrite (WriteHandler handler, const char* data, size_t size, bool flush = true,
                            bool link = false) noexcept
        {
            if (JOIN_UNLIKELY (!this->_socket.opened ()))
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            AsyncWrite* write = this->allocateWrite ();
            if (JOIN_UNLIKELY (write == nullptr))
            {
                lastError = make_error_code (Errc::OutOfMemory);
                return -1;
            }

            write->writeHandler = std::move (handler);

            if constexpr (has_zero_copy_v<Protocol> && has_fixed_buffer_v<ProactorType>)
            {
                if (this->_proactor->fixedBuffer ().owns (data, size))
                {
                    write->op = IoOperation::makeSendZc (this->_socket.handle (), data, size, MSG_NOSIGNAL, this, link);
                }
                else
                {
                    write->op = IoOperation::makeSend (this->_socket.handle (), data, size, MSG_NOSIGNAL, this, link);
                }
            }
            else
            {
                write->op = IoOperation::makeSend (this->_socket.handle (), data, size, MSG_NOSIGNAL, this, link);
            }

            write->op.state.store (IoOperation::State::Submitted, std::memory_order_release);

            size_t index = this->_allocator.getIndex (write);

            if (this->_proactor->submit (write->op, flush, false) == -1)
            {
                // LCOV_EXCL_START
                this->releaseOperation (write);
                return -1;
                // LCOV_EXCL_STOP
            }

            return static_cast<ssize_t> (index);
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
        bool connected () noexcept
        {
            return this->_socket.connected ();
        }

        /**
         * @brief check if the socket is connecting.
         * @return true if connecting, false otherwise.
         */
        bool connecting () const noexcept
        {
            return this->_socket.connecting ();
        }

        /**
         * @brief get the path maximum transmission unit.
         * @return the path maximum transmission unit, -1 on failure.
         */
        int mtu () const noexcept
        {
            return this->_socket.mtu ();
        }
    };
}

#endif
