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
#include <chrono>

// C.
#include <sys/socket.h>
#include <cstddef>

namespace join
{
    /**
     * @brief asynchronous stream socket class.
     */
    template <class Protocol, class ProactorType, size_t OpCount>
    class BasicAsyncStreamSocket final : public BasicAsyncRawSocket<Protocol, ProactorType, OpCount>
    {
    public:
        using Socket = typename Protocol::Socket;
        using Endpoint = typename Protocol::Endpoint;
        using TimePoint = typename ProactorType::TimePoint;
        using AsyncConnect = BasicAsyncConnect<Protocol, ProactorType>;
        using ConnectHandler = typename AsyncConnect::Connect;
        using AsyncDisconnect = BasicAsyncDisconnect<Protocol, ProactorType>;
        using DisconnectHandler = typename AsyncDisconnect::Disconnect;
        using AsyncRead = BasicAsyncRead<Protocol, ProactorType>;
        using ReadHandler = typename AsyncRead::Read;
        using AsyncWrite = BasicAsyncWrite<Protocol, ProactorType>;
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
        BasicAsyncStreamSocket (BasicAsyncStreamSocket&& other) noexcept
        : BasicAsyncRawSocket<Protocol, ProactorType, OpCount> (std::move (other), false)
        {
            this->resumeAll ();
        }

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicAsyncStreamSocket& operator= (BasicAsyncStreamSocket&& other) noexcept = default;

        /**
         * @brief destroy the socket instance.
         */
        ~BasicAsyncStreamSocket ()
        {
            this->close ();
        }

        /**
         * @brief start an asynchronous connection to the given endpoint.
         * @param handler handler invoked on completion.
         * @param endpoint endpoint to connect to.
         * @return 0 on success, -1 on failure.
         */
        int asyncConnect (ConnectHandler handler, const Endpoint& endpoint) noexcept
        {
            return asyncConnect (std::move (handler), endpoint, TimePoint::max ());
        }

        /**
         * @brief start an asynchronous connection to the given endpoint, giving up after the given duration.
         * @param handler handler invoked on completion.
         * @param endpoint endpoint to connect to.
         * @param timeout maximum time granted to the operation.
         * @return 0 on success, -1 on failure.
         */
        int asyncConnect (ConnectHandler handler, const Endpoint& endpoint, std::chrono::nanoseconds timeout) noexcept
        {
            return asyncConnect (std::move (handler), endpoint, std::chrono::steady_clock::now () + timeout);
        }

        /**
         * @brief start an asynchronous connection to the given endpoint, giving up at the given time point.
         * @param handler handler invoked on completion.
         * @param endpoint endpoint to connect to.
         * @param deadline time point at which to give up, max to wait indefinitely.
         * @return 0 on success, -1 on failure.
         */
        int asyncConnect (ConnectHandler handler, const Endpoint& endpoint, TimePoint deadline) noexcept
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

            AsyncConnect* connect = this->template allocateOperation<AsyncConnect> ();
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
            connect->op = IoOperation::makeConnect (this->descriptor ().handle (), this->_socket._remote.addr (),
                                                    this->_socket._remote.length (), this);
            connect->op.state.store (IoOperation::State::Submitted, std::memory_order_release);

            if (this->_proactor->submit (connect->op, deadline, true, false) == -1)
            {
                // LCOV_EXCL_START
                this->releaseOperation (connect);
                this->cancelAll ();
                this->_socket.close ();
                return -1;
                // LCOV_EXCL_STOP
            }

            return 0;
        }

        /**
         * @brief start an asynchronous lingering close, discarding the data received until the end of stream.
         * @param handler handler invoked on completion.
         * @return 0 on success, -1 on failure.
         */
        int asyncDisconnect (DisconnectHandler handler) noexcept
        {
            return asyncDisconnect (std::move (handler), TimePoint::max ());
        }

        /**
         * @brief start an asynchronous lingering close, giving up after the given duration.
         * @param handler handler invoked on completion.
         * @param timeout maximum time granted to the operation.
         * @return 0 on success, -1 on failure.
         */
        int asyncDisconnect (DisconnectHandler handler, std::chrono::nanoseconds timeout) noexcept
        {
            return asyncDisconnect (std::move (handler), std::chrono::steady_clock::now () + timeout);
        }

        /**
         * @brief start an asynchronous lingering close, giving up at the given time point.
         * @param handler handler invoked on completion.
         * @param deadline time point at which to give up, max to wait indefinitely.
         * @return 0 on success, -1 on failure.
         */
        int asyncDisconnect (DisconnectHandler handler, TimePoint deadline) noexcept
        {
            if (JOIN_UNLIKELY ((this->_socket._state != Socket::Connected) &&
                               (this->_socket._state != Socket::Disconnecting)))
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            if (JOIN_UNLIKELY (this->pendingAny ()))
            {
                lastError = make_error_code (Errc::InUse);
                return -1;
            }

            AsyncDisconnect* disconnect = this->template allocateOperation<AsyncDisconnect> ();
            if (JOIN_UNLIKELY (disconnect == nullptr))
            {
                // LCOV_EXCL_START
                lastError = make_error_code (Errc::OutOfMemory);
                return -1;
                // LCOV_EXCL_STOP
            }

            if (this->_socket._state == Socket::Connected)
            {
                ::shutdown (this->_socket.handle (), SHUT_WR);
                this->_socket._state = Socket::Disconnecting;
            }

            disconnect->disconnectHandler = std::move (handler);
            disconnect->deadline = deadline;
            disconnect->op = IoOperation::makeRead (this->descriptor ().handle (), disconnect->buffer,
                                                    sizeof (disconnect->buffer), this);
            disconnect->op.state.store (IoOperation::State::Submitted, std::memory_order_release);

            if (this->_proactor->submit (disconnect->op, deadline, true, false) == -1)
            {
                // LCOV_EXCL_START
                this->releaseOperation (disconnect);
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
            return asyncRead (std::move (handler), data, maxSize, TimePoint::max (), flush, link);
        }

        /**
         * @brief start an asynchronous read, giving up after the given duration.
         * @param handler handler invoked on completion.
         * @param data buffer used to store the data received, valid until the handler is invoked.
         * @param maxSize maximum number of bytes to read.
         * @param timeout maximum time granted to the operation.
         * @param flush flush the submission queue.
         * @return index of the operation on success, -1 on failure.
         */
        ssize_t asyncRead (ReadHandler handler, char* data, size_t maxSize, std::chrono::nanoseconds timeout,
                           bool flush = true) noexcept
        {
            return asyncRead (std::move (handler), data, maxSize, std::chrono::steady_clock::now () + timeout, flush);
        }

        /**
         * @brief start an asynchronous read, giving up at the given time point.
         * @param handler handler invoked on completion.
         * @param data buffer used to store the data received, valid until the handler is invoked.
         * @param maxSize maximum number of bytes to read.
         * @param deadline time point at which to give up, max to wait indefinitely.
         * @param flush flush the submission queue.
         * @param link link this operation to the next one submitted.
         * @return index of the operation on success, -1 on failure.
         */
        ssize_t asyncRead (ReadHandler handler, char* data, size_t maxSize, TimePoint deadline, bool flush = true,
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
            read->op = IoOperation::makeRecv (this->descriptor ().handle (), data, maxSize, 0, this, link);
            read->op.state.store (IoOperation::State::Submitted, std::memory_order_release);

            size_t index = this->_allocator.getIndex (read);

            if (this->_proactor->submit (read->op, deadline, flush, false) == -1)
            {
                // LCOV_EXCL_START
                this->releaseOperation (read);
                return -1;
                // LCOV_EXCL_STOP
            }

            return static_cast<ssize_t> (index);
        }

        /**
         * @brief read exactly size bytes.
         * @param handler handler invoked on completion.
         * @param data buffer used to store the data received, valid until the handler is invoked.
         * @param size number of bytes to read.
         * @param flush flush the submission queue.
         * @param link link this operation to the next one submitted.
         * @return index of the operation on success, -1 on failure.
         */
        ssize_t asyncReadExactly (ReadHandler handler, char* data, size_t size, bool flush = true,
                                  bool link = false) noexcept
        {
            return asyncReadExactly (std::move (handler), data, size, TimePoint::max (), flush, link);
        }

        /**
         * @brief read exactly size bytes, giving up after a duration.
         * @param handler handler invoked on completion.
         * @param data buffer used to store the data received, valid until the handler is invoked.
         * @param size number of bytes to read.
         * @param timeout maximum time granted to the operation.
         * @param flush flush the submission queue.
         * @return index of the operation on success, -1 on failure.
         */
        ssize_t asyncReadExactly (ReadHandler handler, char* data, size_t size, std::chrono::nanoseconds timeout,
                                  bool flush = true) noexcept
        {
            return asyncReadExactly (std::move (handler), data, size, std::chrono::steady_clock::now () + timeout,
                                     flush);
        }

        /**
         * @brief read exactly size bytes, giving up at a time point.
         * @param handler handler invoked on completion.
         * @param data buffer used to store the data received, valid until the handler is invoked.
         * @param size number of bytes to read.
         * @param deadline time point at which to give up, max to wait indefinitely.
         * @param flush flush the submission queue.
         * @param link link this operation to the next one submitted.
         * @return index of the operation on success, -1 on failure.
         */
        ssize_t asyncReadExactly (ReadHandler handler, char* data, size_t size, TimePoint deadline, bool flush = true,
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
            read->iov.iov_base = data;
            read->iov.iov_len = size;
            read->deadline = deadline;
            read->op = IoOperation::makeRecv (this->descriptor ().handle (), data, size, MSG_WAITALL, this, link);
            read->op.state.store (IoOperation::State::Submitted, std::memory_order_release);

            size_t index = this->_allocator.getIndex (read);

            if (this->_proactor->submit (read->op, deadline, flush, false) == -1)
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
            return asyncWrite (std::move (handler), data, size, TimePoint::max (), flush, link);
        }

        /**
         * @brief start an asynchronous write, giving up after the given duration.
         * @param handler handler invoked on completion.
         * @param data data buffer to send, valid until the handler is invoked.
         * @param size number of bytes to write.
         * @param timeout maximum time granted to the operation.
         * @param flush flush the submission queue.
         * @return index of the operation on success, -1 on failure.
         */
        ssize_t asyncWrite (WriteHandler handler, const char* data, size_t size, std::chrono::nanoseconds timeout,
                            bool flush = true) noexcept
        {
            return asyncWrite (std::move (handler), data, size, std::chrono::steady_clock::now () + timeout, flush);
        }

        /**
         * @brief start an asynchronous write, giving up at the given time point.
         * @param handler handler invoked on completion.
         * @param data data buffer to send, valid until the handler is invoked.
         * @param size number of bytes to write.
         * @param deadline time point at which to give up, max to wait indefinitely.
         * @param flush flush the submission queue.
         * @param link link this operation to the next one submitted.
         * @return index of the operation on success, -1 on failure.
         */
        ssize_t asyncWrite (WriteHandler handler, const char* data, size_t size, TimePoint deadline, bool flush = true,
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
                    write->op =
                        IoOperation::makeSendZc (this->descriptor ().handle (), data, size, MSG_NOSIGNAL, this, link);
                }
                else
                {
                    write->op =
                        IoOperation::makeSend (this->descriptor ().handle (), data, size, MSG_NOSIGNAL, this, link);
                }
            }
            else
            {
                write->op = IoOperation::makeSend (this->descriptor ().handle (), data, size, MSG_NOSIGNAL, this, link);
            }

            write->op.state.store (IoOperation::State::Submitted, std::memory_order_release);

            size_t index = this->_allocator.getIndex (write);

            if (this->_proactor->submit (write->op, deadline, flush, false) == -1)
            {
                // LCOV_EXCL_START
                this->releaseOperation (write);
                return -1;
                // LCOV_EXCL_STOP
            }

            return static_cast<ssize_t> (index);
        }

        /**
         * @brief write exactly size bytes.
         * @param handler handler invoked on completion.
         * @param data data buffer to send, valid until the handler is invoked.
         * @param size number of bytes to write.
         * @param flush flush the submission queue.
         * @param link link this operation to the next one submitted.
         * @return index of the operation on success, -1 on failure.
         */
        ssize_t asyncWriteExactly (WriteHandler handler, const char* data, size_t size, bool flush = true,
                                   bool link = false) noexcept
        {
            return asyncWriteExactly (std::move (handler), data, size, TimePoint::max (), flush, link);
        }

        /**
         * @brief write exactly size bytes, giving up after a duration.
         * @param handler handler invoked on completion.
         * @param data data buffer to send, valid until the handler is invoked.
         * @param size number of bytes to write.
         * @param timeout maximum time granted to the operation.
         * @param flush flush the submission queue.
         * @return index of the operation on success, -1 on failure.
         */
        ssize_t asyncWriteExactly (WriteHandler handler, const char* data, size_t size,
                                   std::chrono::nanoseconds timeout, bool flush = true) noexcept
        {
            return asyncWriteExactly (std::move (handler), data, size, std::chrono::steady_clock::now () + timeout,
                                      flush);
        }

        /**
         * @brief write exactly size bytes, giving up at a time point.
         * @param handler handler invoked on completion.
         * @param data data buffer to send, valid until the handler is invoked.
         * @param size number of bytes to write.
         * @param deadline time point at which to give up, max to wait indefinitely.
         * @param flush flush the submission queue.
         * @param link link this operation to the next one submitted.
         * @return index of the operation on success, -1 on failure.
         */
        ssize_t asyncWriteExactly (WriteHandler handler, const char* data, size_t size, TimePoint deadline,
                                   bool flush = true, bool link = false) noexcept
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
            write->iov.iov_base = const_cast<char*> (data);
            write->iov.iov_len = size;
            write->deadline = deadline;

            if constexpr (has_zero_copy_v<Protocol> && has_fixed_buffer_v<ProactorType>)
            {
                if (this->_proactor->fixedBuffer ().owns (data, size))
                {
                    write->op = IoOperation::makeSendZc (this->descriptor ().handle (), data, size,
                                                         MSG_NOSIGNAL | MSG_WAITALL, this, link);
                }
                else
                {
                    write->op = IoOperation::makeSend (this->descriptor ().handle (), data, size,
                                                       MSG_NOSIGNAL | MSG_WAITALL, this, link);
                }
            }
            else
            {
                write->op = IoOperation::makeSend (this->descriptor ().handle (), data, size,
                                                   MSG_NOSIGNAL | MSG_WAITALL, this, link);
            }

            write->op.state.store (IoOperation::State::Submitted, std::memory_order_release);

            size_t index = this->_allocator.getIndex (write);

            if (this->_proactor->submit (write->op, deadline, flush, false) == -1)
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
