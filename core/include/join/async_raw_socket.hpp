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

#ifndef JOIN_CORE_ASYNC_RAW_SOCKET_HPP
#define JOIN_CORE_ASYNC_RAW_SOCKET_HPP

// libjoin.
#include <join/async_operation.hpp>
#include <join/async_socket.hpp>
#include <join/raw_socket.hpp>
#include <join/function.hpp>
#include <join/utils.hpp>

// C++.
#include <system_error>
#include <utility>
#include <string>

// C.
#include <cstddef>
#include <cstring>

namespace join
{
    /**
     * @brief asynchronous raw socket class.
     */
    template <class Protocol, class Proactor, size_t OpCount>
    class BasicAsyncRawSocket : public BasicAsyncSocket<Protocol, Proactor, OpCount>
    {
    public:
        using Socket = typename Protocol::Socket;
        using Endpoint = typename Protocol::Endpoint;
        using Option = typename Socket::Option;
        using AsyncRead = BasicAsyncRead<Protocol, Proactor>;
        using AsyncWrite = BasicAsyncWrite<Protocol, Proactor>;
        using ReadHandler = typename AsyncRead::Read;
        using ReadFromHandler = typename AsyncRead::ReadFrom;
        using ConnectHandler = typename AsyncWrite::Connect;
        using WriteHandler = typename AsyncWrite::Write;

        /**
         * @brief create the socket instance.
         * @param proactor proactor driving the operations.
         */
        explicit BasicAsyncRawSocket (Proactor& proactor = ProactorThread::proactor ())
        : BasicAsyncSocket<Protocol, Proactor, OpCount> (proactor)
        {
        }

        /**
         * @brief create the socket instance adopting an already opened socket.
         * @param sock socket to adopt.
         * @param proactor proactor driving the operations.
         */
        explicit BasicAsyncRawSocket (Socket&& sock, Proactor& proactor = ProactorThread::proactor ())
        : BasicAsyncSocket<Protocol, Proactor, OpCount> (std::move (sock), proactor)
        {
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicAsyncRawSocket (const BasicAsyncRawSocket& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicAsyncRawSocket& operator= (const BasicAsyncRawSocket& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        BasicAsyncRawSocket (BasicAsyncRawSocket&& other) noexcept
        : BasicAsyncSocket<Protocol, Proactor, OpCount> (std::move (other))
        {
            this->resumeAll ();
        }

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicAsyncRawSocket& operator= (BasicAsyncRawSocket&& other) noexcept
        {
            BasicAsyncSocket<Protocol, Proactor, OpCount>::operator= (std::move (other));

            this->resumeAll ();

            return *this;
        }

        /**
         * @brief destroy the socket instance.
         */
        ~BasicAsyncRawSocket ()
        {
            this->close ();
        }

        /**
         * @brief assign the specified endpoint to the socket.
         * @param endpoint endpoint to assign to the socket.
         * @return 0 on success, -1 on failure.
         */
        int bind (const Endpoint& endpoint) noexcept
        {
            return this->_socket.bind (endpoint);
        }

        /**
         * @brief assign the specified device to the socket.
         * @param device device name.
         * @return 0 on success, -1 on failure.
         */
        int bindToDevice (const std::string& device) noexcept
        {
            return this->_socket.bindToDevice (device);
        }

        /**
         * @brief get the number of readable bytes.
         * @return the number of readable bytes, -1 on failure.
         */
        int canRead () const noexcept
        {
            return this->_socket.canRead ();
        }

        /**
         * @brief start an asynchronous read.
         * @param data buffer used to store the data received, valid until the handler is invoked.
         * @param maxSize maximum number of bytes to read.
         * @param handler handler invoked on completion.
         * @param flush flush the submission queue.
         * @param link link this operation to the next one submitted.
         * @return index of the operation on success, -1 on failure.
         */
        ssize_t asyncRead (char* data, size_t maxSize, ReadHandler handler, bool flush = true,
                           bool link = false) noexcept
        {
            if (JOIN_UNLIKELY (!this->_socket.opened ()))
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            AsyncRead* read = allocateRead ();
            if (JOIN_UNLIKELY (read == nullptr))
            {
                lastError = make_error_code (Errc::OutOfMemory);
                return -1;
            }

            read->readHandler = std::move (handler);
            read->iov.iov_base = data;
            read->iov.iov_len = maxSize;
            read->msg.msg_name = nullptr;
            read->msg.msg_namelen = 0;
            read->msg.msg_iov = &read->iov;
            read->msg.msg_iovlen = 1;
            read->msg.msg_control = nullptr;
            read->msg.msg_controllen = 0;
            read->msg.msg_flags = 0;
            read->op = IoOperation::makeRecvmsg (this->_socket.handle (), &read->msg, 0, this, link);
            read->op.state.store (IoOperation::State::Submitted, std::memory_order_release);

            size_t index = this->_arena.getIndex (read);

            if (this->_proactor->submit (read->op, flush, false) == -1)
            {
                // LCOV_EXCL_START
                this->releaseOp (read);
                return -1;
                // LCOV_EXCL_STOP
            }

            return static_cast<ssize_t> (index);
        }

#ifdef JOIN_HAS_IO_URING
        /**
         * @brief start an asynchronous read into a registered buffer.
         * @param data registered buffer used to store the data received, valid until the handler is invoked.
         * @param maxSize maximum number of bytes to read.
         * @param index index of the registered buffer area the buffer belongs to.
         * @param handler handler invoked on completion.
         * @param flush flush the submission queue.
         * @param link link this operation to the next one submitted.
         * @return index of the operation on success, -1 on failure.
         */
        ssize_t asyncReadFixed (char* data, size_t maxSize, uint16_t index, ReadHandler handler, bool flush = true,
                                bool link = false) noexcept
        {
            if (JOIN_UNLIKELY (!this->_socket.opened ()))
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            AsyncRead* read = allocateRead ();
            if (JOIN_UNLIKELY (read == nullptr))
            {
                lastError = make_error_code (Errc::OutOfMemory);
                return -1;
            }

            read->readHandler = std::move (handler);
            read->op = IoOperation::makeReadFixed (this->_socket.handle (), data, maxSize, index, this, link);
            read->op.state.store (IoOperation::State::Submitted, std::memory_order_release);

            size_t slot = this->_arena.getIndex (read);

            if (this->_proactor->submit (read->op, flush, false) == -1)
            {
                // LCOV_EXCL_START
                this->releaseOp (read);
                return -1;
                // LCOV_EXCL_STOP
            }

            return static_cast<ssize_t> (slot);
        }
#endif

        /**
         * @brief start an asynchronous multishot read, staying armed until cancelled or failed.
         * @param group provided buffer group to receive into, registered on the proactor.
         * @param handler handler invoked on each completion, the buffer being valid during the call only.
         * @param flush flush the submission queue.
         * @return index of the operation on success, -1 on failure.
         */
        ssize_t asyncReadMulti (uint16_t group, ReadHandler handler, bool flush = true) noexcept
        {
            if (JOIN_UNLIKELY (!this->_socket.opened ()))
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            AsyncRead* read = allocateRead ();
            if (JOIN_UNLIKELY (read == nullptr))
            {
                lastError = make_error_code (Errc::OutOfMemory);
                return -1;
            }

            read->readHandler = std::move (handler);
            read->op = IoOperation::makeRecvMulti (this->_socket.handle (), group, 0, this);
            read->op.state.store (IoOperation::State::Submitted, std::memory_order_release);

            size_t index = this->_arena.getIndex (read);

            if (this->_proactor->submit (read->op, flush, false) == -1)
            {
                // LCOV_EXCL_START
                this->releaseOp (read);
                return -1;
                // LCOV_EXCL_STOP
            }

            return static_cast<ssize_t> (index);
        }

        /**
         * @brief start an asynchronous write.
         * @param data data buffer to send, valid until the handler is invoked.
         * @param size number of bytes to write.
         * @param handler handler invoked on completion.
         * @param flush flush the submission queue.
         * @param link link this operation to the next one submitted.
         * @return index of the operation on success, -1 on failure.
         */
        ssize_t asyncWrite (const char* data, size_t size, WriteHandler handler, bool flush = true,
                            bool link = false) noexcept
        {
            if (JOIN_UNLIKELY (!this->_socket.opened ()))
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            AsyncWrite* write = allocateWrite ();
            if (JOIN_UNLIKELY (write == nullptr))
            {
                lastError = make_error_code (Errc::OutOfMemory);
                return -1;
            }

            write->writeHandler = std::move (handler);
            write->iov.iov_base = const_cast<char*> (data);
            write->iov.iov_len = size;
            write->msg.msg_name = nullptr;
            write->msg.msg_namelen = 0;
            write->msg.msg_iov = &write->iov;
            write->msg.msg_iovlen = 1;
            write->msg.msg_control = nullptr;
            write->msg.msg_controllen = 0;
            write->msg.msg_flags = 0;
            write->op = IoOperation::makeSendmsg (this->_socket.handle (), &write->msg, MSG_NOSIGNAL, this, link);
            write->op.state.store (IoOperation::State::Submitted, std::memory_order_release);

            size_t index = this->_arena.getIndex (write);

            if (this->_proactor->submit (write->op, flush, false) == -1)
            {
                // LCOV_EXCL_START
                this->releaseOp (write);
                return -1;
                // LCOV_EXCL_STOP
            }

            return static_cast<ssize_t> (index);
        }

#ifdef JOIN_HAS_IO_URING
        /**
         * @brief start an asynchronous write from a registered buffer.
         * @param data registered buffer to send, valid until the handler is invoked.
         * @param size number of bytes to write.
         * @param index index of the registered buffer area the buffer belongs to.
         * @param handler handler invoked on completion.
         * @param flush flush the submission queue.
         * @param link link this operation to the next one submitted.
         * @return index of the operation on success, -1 on failure.
         */
        ssize_t asyncWriteFixed (const char* data, size_t size, uint16_t index, WriteHandler handler, bool flush = true,
                                 bool link = false) noexcept
        {
            if (JOIN_UNLIKELY (!this->_socket.opened ()))
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            AsyncWrite* write = allocateWrite ();
            if (JOIN_UNLIKELY (write == nullptr))
            {
                lastError = make_error_code (Errc::OutOfMemory);
                return -1;
            }

            write->writeHandler = std::move (handler);
            write->op = IoOperation::makeWriteFixed (this->_socket.handle (), data, size, index, this, link);
            write->op.state.store (IoOperation::State::Submitted, std::memory_order_release);

            size_t slot = this->_arena.getIndex (write);

            if (this->_proactor->submit (write->op, flush, false) == -1)
            {
                // LCOV_EXCL_START
                this->releaseOp (write);
                return -1;
                // LCOV_EXCL_STOP
            }

            return static_cast<ssize_t> (slot);
        }
#endif

        /**
         * @brief cancel the connect operation in flight, if any.
         * @return 0 on success, -1 on failure.
         */
        int cancelConnect () noexcept
        {
            for (auto& slot : this->_ops)
            {
                IoOperation* op = slot.load (std::memory_order_acquire);

                if ((op != nullptr) && (static_cast<IoOperation::Opcode> (op->code) == IoOperation::Opcode::Connect))
                {
                    return this->cancelOp (op);
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
        int setOption (Option option, int value) noexcept
        {
            return this->_socket.setOption (option, value);
        }

    protected:
        /**
         * @brief invoke the completion handler of the given operation, then release it.
         * @param op completed operation.
         * @param code error code reported by the kernel.
         * @param size number of bytes transferred.
         */
        void dispatch (IoOperation& op, const std::error_code& code, size_t size) noexcept override
        {
            IoOperation::Opcode opcode = static_cast<IoOperation::Opcode> (op.code);

            if ((opcode == IoOperation::Opcode::RecvMsg) || (opcode == IoOperation::Opcode::Recv) ||
                (opcode == IoOperation::Opcode::ReadFixed))
            {
                completeRead (reinterpret_cast<AsyncRead*> (&op), code, size);
            }
            else if ((opcode == IoOperation::Opcode::SendMsg) || (opcode == IoOperation::Opcode::WriteFixed))
            {
                completeWrite (reinterpret_cast<AsyncWrite*> (&op), code, size);
            }
            else if (opcode == IoOperation::Opcode::Connect)
            {
                completeConnect (reinterpret_cast<AsyncWrite*> (&op), code);
            }
            else
            {
                BasicAsyncSocket<Protocol, Proactor, OpCount>::dispatch (op, code, size);
            }
        }

        /**
         * @brief invoke the connect completion handler.
         * @param code error code reported by the kernel.
         */
        void completeConnect (AsyncWrite* connect, const std::error_code& code) noexcept
        {
            ConnectHandler handler = std::move (connect->connectHandler);

            this->releaseOp (connect);

            if (code)
            {
                this->_socket.close ();
            }
            else
            {
                this->_socket._state = Socket::Connected;
            }

            if (JOIN_LIKELY (handler))
            {
                handler (code);
            }
        }

        /**
         * @brief invoke the read completion handler.
         * @param read completed read operation.
         * @param code error code reported by the kernel.
         * @param size number of bytes received.
         */
        void completeRead (AsyncRead* read, const std::error_code& code, size_t size) noexcept
        {
            std::error_code result = code;
            const char* data = nullptr;

            switch (static_cast<IoOperation::Opcode> (read->op.code))
            {
                case IoOperation::Opcode::Recv:
                    data = static_cast<const char*> (read->op.data.stream.buf);
                    break;

                case IoOperation::Opcode::ReadFixed:
                    data = static_cast<const char*> (read->op.data.rw.buf);
                    break;

                default:
                    data = static_cast<const char*> (read->msg.msg_iov->iov_base);
                    break;
            }

            if (this->_socket.type () == SOCK_STREAM)
            {
                if (JOIN_UNLIKELY (!result && (size == 0)))
                {
                    result = make_error_code (Errc::ConnectionClosed);
                }
            }
            else if (JOIN_UNLIKELY (!result && (read->msg.msg_flags & MSG_TRUNC)))
            {
                result = make_error_code (Errc::MessageTooLong);
            }

            Endpoint from;

            if (read->msg.msg_name != nullptr)
            {
                from = Endpoint (static_cast<const struct sockaddr*> (read->msg.msg_name), read->msg.msg_namelen);
            }

            if (read->op.more)
            {
                if (read->readFromHandler)
                {
                    read->readFromHandler (result, data, size, from, true);
                }
                else if (JOIN_LIKELY (read->readHandler))
                {
                    read->readHandler (result, data, size, true);
                }

                return;
            }

            ReadHandler handler = std::move (read->readHandler);
            ReadFromHandler fromHandler = std::move (read->readFromHandler);

            this->releaseOp (read);

            if (fromHandler)
            {
                fromHandler (result, data, size, from, false);
            }
            else if (JOIN_LIKELY (handler))
            {
                handler (result, data, size, false);
            }
        }

        /**
         * @brief invoke the write completion handler.
         * @param code error code reported by the kernel.
         * @param size number of bytes sent.
         */
        void completeWrite (AsyncWrite* write, const std::error_code& code, size_t size) noexcept
        {
            WriteHandler handler = std::move (write->writeHandler);

            if (JOIN_LIKELY (!write->op.multishot))
            {
                this->releaseOp (write);
            }

            if (JOIN_LIKELY (handler))
            {
                handler (code, size);
            }
        }

        /**
         * @brief allocate a read operation in the arena.
         * @return allocated read operation, or nullptr if the arena is exhausted.
         */
        AsyncRead* allocateRead () noexcept
        {
            return this->template allocateOp<AsyncRead> ();
        }

        /**
         * @brief allocate a write operation in the arena.
         * @return allocated write operation, or nullptr if the arena is exhausted.
         */
        AsyncWrite* allocateWrite () noexcept
        {
            return this->template allocateOp<AsyncWrite> ();
        }
    };
}

#endif
