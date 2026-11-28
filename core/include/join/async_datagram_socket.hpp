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
#include <join/async_raw_socket.hpp>
#include <join/datagram_socket.hpp>

// C++.
#include <system_error>
#include <utility>
#include <chrono>
#include <limits>

namespace join
{
    /**
     * @brief asynchronous datagram socket class.
     */
    template <class Protocol, class ProactorType, class Allocator>
    class BasicAsyncDatagramSocket final : public BasicAsyncRawSocket<Protocol, ProactorType, Allocator>
    {
    public:
        using Socket = typename Protocol::Socket;
        using Endpoint = typename Protocol::Endpoint;
        using TimePoint = typename ProactorType::TimePoint;
        using AsyncRead = BasicAsyncRead<Protocol, ProactorType>;
        using ReadHandler = typename AsyncRead::Read;
        using ReadFromHandler = typename AsyncRead::ReadFrom;
        using AsyncWrite = BasicAsyncWrite<Protocol, ProactorType>;
        using WriteHandler = typename AsyncWrite::Write;

        /**
         * @brief create the socket instance.
         * @param proactor proactor driving the operations.
         * @throw std::system_error if mmap fails.
         * @throw std::bad_alloc if the allocator cannot be created.
         */
        explicit BasicAsyncDatagramSocket (ProactorType& proactor = ProactorThread::proactor ())
        : BasicAsyncRawSocket<Protocol, ProactorType, Allocator> (proactor)
        {
        }

        /**
         * @brief create the socket instance specifying the time to live.
         * @param ttl packet time to live.
         * @param proactor proactor driving the operations.
         * @throw std::system_error if mmap fails.
         * @throw std::bad_alloc if the allocator cannot be created.
         */
        explicit BasicAsyncDatagramSocket (int ttl, ProactorType& proactor = ProactorThread::proactor ())
        : BasicAsyncRawSocket<Protocol, ProactorType, Allocator> (Socket (ttl), proactor)
        {
        }

        /**
         * @brief create the socket instance adopting an already opened socket.
         * @param sock socket to adopt.
         * @param proactor proactor driving the operations.
         * @throw std::system_error if mmap fails.
         * @throw std::bad_alloc if the allocator cannot be created.
         */
        explicit BasicAsyncDatagramSocket (Socket&& sock, ProactorType& proactor = ProactorThread::proactor ())
        : BasicAsyncRawSocket<Protocol, ProactorType, Allocator> (std::move (sock), proactor)
        {
        }

        /**
         * @brief create instance with a caller allocator.
         * @param allocator caller allocator.
         * @param proactor proactor driving the operations.
         */
        explicit BasicAsyncDatagramSocket (Allocator& allocator, ProactorType& proactor = ProactorThread::proactor ())
        : BasicAsyncRawSocket<Protocol, ProactorType, Allocator> (allocator, proactor)
        {
        }

        /**
         * @brief create instance with a time to live and a caller allocator.
         * @param ttl packet time to live.
         * @param allocator caller allocator.
         * @param proactor proactor driving the operations.
         */
        BasicAsyncDatagramSocket (int ttl, Allocator& allocator, ProactorType& proactor = ProactorThread::proactor ())
        : BasicAsyncRawSocket<Protocol, ProactorType, Allocator> (Socket (ttl), allocator, proactor)
        {
        }

        /**
         * @brief adopt a socket with a caller allocator.
         * @param sock socket to adopt.
         * @param allocator caller allocator.
         * @param proactor proactor driving the operations.
         */
        BasicAsyncDatagramSocket (Socket&& sock, Allocator& allocator,
                                  ProactorType& proactor = ProactorThread::proactor ())
        : BasicAsyncRawSocket<Protocol, ProactorType, Allocator> (std::move (sock), allocator, proactor)
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
        BasicAsyncDatagramSocket (BasicAsyncDatagramSocket&& other) noexcept
        : BasicAsyncRawSocket<Protocol, ProactorType, Allocator> (std::move (other), false)
        {
            this->resumeAll ();
        }

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

            if (JOIN_UNLIKELY (maxSize > std::numeric_limits<uint32_t>::max ()))
            {
                lastError = std::make_error_code (std::errc::message_size);
                return -1;
            }

            AsyncRead* read = this->allocateRead ();
            if (JOIN_UNLIKELY (read == nullptr))
            {
                lastError = make_error_code (Errc::OutOfMemory);
                return -1;
            }

            read->readHandler = std::move (handler);
            read->op = IoOperation::makeRecv (this->descriptor ().handle (), data, maxSize, MSG_TRUNC, this, link);
            read->op.state.store (IoOperation::State::Submitted, std::memory_order_release);

            size_t index = this->_allocator->getIndex (read);

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
         * @brief start an asynchronous read, reporting the endpoint the data are coming from.
         * @param handler handler invoked on completion.
         * @param data buffer used to store the data received, valid until the handler is invoked.
         * @param maxSize maximum number of bytes to read.
         * @param endpoint endpoint from where data are coming, valid until the handler is invoked.
         * @param control buffer used to store the control messages received, valid until the handler is invoked.
         * @param controlSize size of the control buffer, zero to receive no control message.
         * @param flush flush the submission queue.
         * @param link link this operation to the next one submitted.
         * @return index of the operation on success, -1 on failure.
         */
        ssize_t asyncReadFrom (ReadFromHandler handler, char* data, size_t maxSize, Endpoint& endpoint,
                               char* control = nullptr, size_t controlSize = 0, bool flush = true,
                               bool link = false) noexcept
        {
            return asyncReadFrom (std::move (handler), data, maxSize, endpoint, TimePoint::max (), control, controlSize,
                                  flush, link);
        }

        /**
         * @brief start an asynchronous read reporting the endpoint, giving up after the given duration.
         * @param handler handler invoked on completion.
         * @param data buffer used to store the data received, valid until the handler is invoked.
         * @param maxSize maximum number of bytes to read.
         * @param endpoint endpoint from where data are coming, valid until the handler is invoked.
         * @param timeout maximum time granted to the operation.
         * @param control buffer used to store the control messages received, valid until the handler is invoked.
         * @param controlSize size of the control buffer, zero to receive no control message.
         * @param flush flush the submission queue.
         * @return index of the operation on success, -1 on failure.
         */
        ssize_t asyncReadFrom (ReadFromHandler handler, char* data, size_t maxSize, Endpoint& endpoint,
                               std::chrono::nanoseconds timeout, char* control = nullptr, size_t controlSize = 0,
                               bool flush = true) noexcept
        {
            return asyncReadFrom (std::move (handler), data, maxSize, endpoint,
                                  std::chrono::steady_clock::now () + timeout, control, controlSize, flush);
        }

        /**
         * @brief start an asynchronous read reporting the endpoint, giving up at the given time point.
         * @param handler handler invoked on completion.
         * @param data buffer used to store the data received, valid until the handler is invoked.
         * @param maxSize maximum number of bytes to read.
         * @param endpoint endpoint from where data are coming, valid until the handler is invoked.
         * @param deadline time point at which to give up, max to wait indefinitely.
         * @param control buffer used to store the control messages received, valid until the handler is invoked.
         * @param controlSize size of the control buffer, zero to receive no control message.
         * @param flush flush the submission queue.
         * @param link link this operation to the next one submitted.
         * @return index of the operation on success, -1 on failure.
         */
        ssize_t asyncReadFrom (ReadFromHandler handler, char* data, size_t maxSize, Endpoint& endpoint,
                               TimePoint deadline, char* control = nullptr, size_t controlSize = 0, bool flush = true,
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

            read->readFromHandler = std::move (handler);
            read->iov.iov_base = data;
            read->iov.iov_len = maxSize;
            read->msg.msg_name = endpoint.addr ();
            read->msg.msg_namelen = sizeof (struct sockaddr_storage);
            read->msg.msg_iov = &read->iov;
            read->msg.msg_iovlen = 1;
            read->msg.msg_control = control;
            read->msg.msg_controllen = controlSize;
            read->msg.msg_flags = 0;
            read->op = IoOperation::makeRecvmsg (this->descriptor ().handle (), &read->msg, 0, this, link);
            read->op.state.store (IoOperation::State::Submitted, std::memory_order_release);

            size_t index = this->_allocator->getIndex (read);

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
         * @brief start an asynchronous multishot read, reporting the endpoint the data are coming from.
         * @param handler handler invoked on completion.
         * @param group provided buffer group to receive into, registered on the proactor.
         * @param controlSize room reserved in each buffer for the control messages, zero to receive none.
         * @param flush flush the submission queue.
         * @return index of the operation on success, -1 on failure.
         */
        ssize_t asyncReadFromMulti (ReadFromHandler handler, uint16_t group, size_t controlSize = 0,
                                    bool flush = true) noexcept
        {
            if (JOIN_UNLIKELY (!this->_socket.opened ()))
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            if (JOIN_UNLIKELY (controlSize > std::numeric_limits<uint16_t>::max ()))
            {
                lastError = std::make_error_code (std::errc::message_size);
                return -1;
            }

            AsyncRead* read = this->allocateRead ();
            if (JOIN_UNLIKELY (read == nullptr))
            {
                lastError = make_error_code (Errc::OutOfMemory);
                return -1;
            }

            read->readFromHandler = std::move (handler);
            read->msg.msg_name = nullptr;
            read->msg.msg_namelen = sizeof (struct sockaddr_storage);
            read->msg.msg_iov = &read->iov;
            read->msg.msg_iovlen = 1;
            read->msg.msg_control = nullptr;
            read->msg.msg_controllen = controlSize;
            read->msg.msg_flags = 0;
            read->op = IoOperation::makeRecvmsgMulti (this->descriptor ().handle (), group, &read->msg, 0, this);
            read->op.state.store (IoOperation::State::Submitted, std::memory_order_release);

            size_t index = this->_allocator->getIndex (read);

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

            if (JOIN_UNLIKELY (size > std::numeric_limits<uint32_t>::max ()))
            {
                lastError = std::make_error_code (std::errc::message_size);
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

            size_t index = this->_allocator->getIndex (write);

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
         * @brief start an asynchronous write to the given endpoint.
         * @param handler handler invoked on completion.
         * @param data data buffer to send, valid until the handler is invoked.
         * @param size number of bytes to write.
         * @param endpoint endpoint where to write the data, valid until the handler is invoked.
         * @param control control messages to send, valid until the handler is invoked.
         * @param controlSize size of the control messages, zero to send none.
         * @param flush flush the submission queue.
         * @param link link this operation to the next one submitted.
         * @return index of the operation on success, -1 on failure.
         */
        ssize_t asyncWriteTo (WriteHandler handler, const char* data, size_t size, Endpoint& endpoint,
                              const char* control = nullptr, size_t controlSize = 0, bool flush = true,
                              bool link = false) noexcept
        {
            return asyncWriteTo (std::move (handler), data, size, endpoint, TimePoint::max (), control, controlSize,
                                 flush, link);
        }

        /**
         * @brief start an asynchronous write to the given endpoint, giving up after the given duration.
         * @param handler handler invoked on completion.
         * @param data data buffer to send, valid until the handler is invoked.
         * @param size number of bytes to write.
         * @param endpoint endpoint where to write the data, valid until the handler is invoked.
         * @param timeout maximum time granted to the operation.
         * @param control control messages to send, valid until the handler is invoked.
         * @param controlSize size of the control messages, zero to send none.
         * @param flush flush the submission queue.
         * @return index of the operation on success, -1 on failure.
         */
        ssize_t asyncWriteTo (WriteHandler handler, const char* data, size_t size, Endpoint& endpoint,
                              std::chrono::nanoseconds timeout, const char* control = nullptr, size_t controlSize = 0,
                              bool flush = true) noexcept
        {
            return asyncWriteTo (std::move (handler), data, size, endpoint, std::chrono::steady_clock::now () + timeout,
                                 control, controlSize, flush);
        }

        /**
         * @brief start an asynchronous write to the given endpoint, giving up at the given time point.
         * @param handler handler invoked on completion.
         * @param data data buffer to send, valid until the handler is invoked.
         * @param size number of bytes to write.
         * @param endpoint endpoint where to write the data, valid until the handler is invoked.
         * @param deadline time point at which to give up, max to wait indefinitely.
         * @param control control messages to send, valid until the handler is invoked.
         * @param controlSize size of the control messages, zero to send none.
         * @param flush flush the submission queue.
         * @param link link this operation to the next one submitted.
         * @return index of the operation on success, -1 on failure.
         */
        ssize_t asyncWriteTo (WriteHandler handler, const char* data, size_t size, Endpoint& endpoint,
                              TimePoint deadline, const char* control = nullptr, size_t controlSize = 0,
                              bool flush = true, bool link = false) noexcept
        {
            if (!this->_socket.opened () && (this->_socket.open (endpoint.protocol ()) == -1))
            {
                return -1;  // LCOV_EXCL_LINE
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
            write->msg.msg_name = endpoint.addr ();
            write->msg.msg_namelen = endpoint.length ();
            write->msg.msg_iov = &write->iov;
            write->msg.msg_iovlen = 1;
            write->msg.msg_control = const_cast<char*> (control);
            write->msg.msg_controllen = controlSize;
            write->msg.msg_flags = 0;
            write->op = IoOperation::makeSendmsg (this->descriptor ().handle (), &write->msg, MSG_NOSIGNAL, this, link);
            write->op.state.store (IoOperation::State::Submitted, std::memory_order_release);

            size_t index = this->_allocator->getIndex (write);

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
