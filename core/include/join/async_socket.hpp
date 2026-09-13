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

#ifndef JOIN_CORE_ASYNC_SOCKET_HPP
#define JOIN_CORE_ASYNC_SOCKET_HPP

// libjoin.
#include <join/async_operation.hpp>
#include <join/proactor.hpp>
#include <join/function.hpp>
#include <join/backoff.hpp>
#include <join/memory.hpp>
#include <join/socket.hpp>
#include <join/utils.hpp>

// C++.
#include <system_error>
#include <utility>
#include <memory>
#include <string>
#include <atomic>
#include <array>
#include <new>

// C.
#include <cstddef>
#include <cstring>

namespace join
{
    /**
     * @brief basic asynchronous socket class.
     */
    template <class Protocol, class Proactor>
    class BasicAsyncSocket : public CompletionHandler
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

        /// number of concurrent operations per direction.
        static constexpr size_t _opCount = 8;

        /// size of a read operation slot.
        static constexpr size_t _readSize = nextPow2 (sizeof (AsyncRead));

        /// size of a write operation slot.
        static constexpr size_t _writeSize = nextPow2 (sizeof (AsyncWrite));

        using ReadArena = LocalMem::Allocator<_opCount, _readSize>;
        using WriteArena = LocalMem::Allocator<_opCount, _writeSize>;

        /**
         * @brief create the socket instance.
         * @param proactor proactor driving the operations.
         */
        explicit BasicAsyncSocket (Proactor& proactor = ProactorThread::proactor ())
        : _proactor (&proactor)
        {
        }

        /**
         * @brief create the socket instance adopting an already opened socket.
         * @param sock socket to adopt.
         * @param proactor proactor driving the operations.
         */
        explicit BasicAsyncSocket (Socket&& sock, Proactor& proactor = ProactorThread::proactor ())
        : _proactor (&proactor)
        , _socket (std::move (sock))
        {
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicAsyncSocket (const BasicAsyncSocket& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicAsyncSocket& operator= (const BasicAsyncSocket& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        BasicAsyncSocket (BasicAsyncSocket&& other) noexcept
        : _proactor (other._proactor)
        {
            for (auto& slot : other._readOps)
            {
                _proactor->suspend (slot.load (std::memory_order_acquire));
            }

            for (auto& slot : other._writeOps)
            {
                _proactor->suspend (slot.load (std::memory_order_acquire));
            }

            _socket = std::move (other._socket);
            _readArena = std::move (other._readArena);
            _writeArena = std::move (other._writeArena);

            for (size_t i = 0; i < other._readOps.size (); ++i)
            {
                _readOps[i].store (other._readOps[i].exchange (nullptr, std::memory_order_acq_rel),
                                   std::memory_order_release);
                _writeOps[i].store (other._writeOps[i].exchange (nullptr, std::memory_order_acq_rel),
                                    std::memory_order_release);
            }

            for (auto& slot : _readOps)
            {
                _proactor->resume (slot.load (std::memory_order_acquire), this);
            }

            for (auto& slot : _writeOps)
            {
                _proactor->resume (slot.load (std::memory_order_acquire), this);
            }
        }

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicAsyncSocket& operator= (BasicAsyncSocket&& other) noexcept
        {
            close ();

            _proactor = other._proactor;

            for (auto& slot : other._readOps)
            {
                _proactor->suspend (slot.load (std::memory_order_acquire));
            }

            for (auto& slot : other._writeOps)
            {
                _proactor->suspend (slot.load (std::memory_order_acquire));
            }

            _socket = std::move (other._socket);
            _readArena = std::move (other._readArena);
            _writeArena = std::move (other._writeArena);

            for (size_t i = 0; i < other._readOps.size (); ++i)
            {
                _readOps[i].store (other._readOps[i].exchange (nullptr, std::memory_order_acq_rel),
                                   std::memory_order_release);
                _writeOps[i].store (other._writeOps[i].exchange (nullptr, std::memory_order_acq_rel),
                                    std::memory_order_release);
            }

            for (auto& slot : _readOps)
            {
                _proactor->resume (slot.load (std::memory_order_acquire), this);
            }

            for (auto& slot : _writeOps)
            {
                _proactor->resume (slot.load (std::memory_order_acquire), this);
            }
            return *this;
        }

        /**
         * @brief destroy the socket instance.
         */
        ~BasicAsyncSocket ()
        {
            close ();
        }

        /**
         * @brief open socket using the given protocol.
         * @param protocol protocol to use.
         * @return 0 on success, -1 on failure.
         */
        int open (const Protocol& protocol = Protocol ()) noexcept
        {
            return _socket.open (protocol);
        }

        /**
         * @brief close the socket, cancelling the operations in flight.
         */
        void close () noexcept
        {
            Backoff backoff;

            do
            {
                cancelAll ();

                backoff ();
            }
            while (!_proactor->isProactorThread () && pendingAny ());

            _socket.close ();
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
            if (JOIN_UNLIKELY (!_socket.opened ()))
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
            read->op = IoOperation::makeRecvmsg (_socket.handle (), &read->msg, 0, this, link);
            read->op.state.store (IoOperation::State::Submitted, std::memory_order_release);

            size_t index = _readArena.getIndex (read);

            if (_proactor->submit (&read->op, flush, false) == -1)
            {
                // LCOV_EXCL_START
                releaseRead (read);
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
            if (JOIN_UNLIKELY (!_socket.opened ()))
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
            read->op = IoOperation::makeReadFixed (_socket.handle (), data, maxSize, index, this, link);
            read->op.state.store (IoOperation::State::Submitted, std::memory_order_release);

            size_t slot = _readArena.getIndex (read);

            if (_proactor->submit (&read->op, flush, false) == -1)
            {
                // LCOV_EXCL_START
                releaseRead (read);
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
            if (JOIN_UNLIKELY (!_socket.opened ()))
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
            read->op = IoOperation::makeRecvMulti (_socket.handle (), group, 0, this);
            read->op.state.store (IoOperation::State::Submitted, std::memory_order_release);

            size_t index = _readArena.getIndex (read);

            if (_proactor->submit (&read->op, flush, false) == -1)
            {
                // LCOV_EXCL_START
                releaseRead (read);
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
            if (JOIN_UNLIKELY (!_socket.opened ()))
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
            write->op = IoOperation::makeSendmsg (_socket.handle (), &write->msg, MSG_NOSIGNAL, this, link);
            write->op.state.store (IoOperation::State::Submitted, std::memory_order_release);

            size_t index = _writeArena.getIndex (write);

            if (_proactor->submit (&write->op, flush, false) == -1)
            {
                // LCOV_EXCL_START
                releaseWrite (write);
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
            if (JOIN_UNLIKELY (!_socket.opened ()))
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
            write->op = IoOperation::makeWriteFixed (_socket.handle (), data, size, index, this, link);
            write->op.state.store (IoOperation::State::Submitted, std::memory_order_release);

            size_t slot = _writeArena.getIndex (write);

            if (_proactor->submit (&write->op, flush, false) == -1)
            {
                // LCOV_EXCL_START
                releaseWrite (write);
                return -1;
                // LCOV_EXCL_STOP
            }

            return static_cast<ssize_t> (slot);
        }
#endif

        /**
         * @brief cancel the read operation stored at the given index, if in flight.
         * @param index index of the operation to cancel.
         * @return 0 on success, -1 on failure.
         */
        int cancelRead (size_t index) noexcept
        {
            if (JOIN_UNLIKELY (index >= _readOps.size ()))
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            return cancelOp (_readOps[index].load (std::memory_order_acquire));
        }

        /**
         * @brief cancel the write operation stored at the given index, if in flight.
         * @param index index of the operation to cancel.
         * @return 0 on success, -1 on failure.
         */
        int cancelWrite (size_t index) noexcept
        {
            if (JOIN_UNLIKELY (index >= _writeOps.size ()))
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            return cancelOp (_writeOps[index].load (std::memory_order_acquire));
        }

        /**
         * @brief cancel the connect operation in flight, if any.
         * @return 0 on success, -1 on failure.
         */
        int cancelConnect () noexcept
        {
            for (auto& slot : _writeOps)
            {
                IoOperation* op = slot.load (std::memory_order_acquire);

                if ((op != nullptr) && (static_cast<IoOperation::Opcode> (op->code) == IoOperation::Opcode::Connect))
                {
                    return cancelOp (op);
                }
            }

            return 0;
        }

        /**
         * @brief register a provided buffer ring on the proactor driving this socket.
         * @param group buffer group id.
         * @param arena arena owning the buffers, reserved until the ring is unregistered, must outlive the proactor.
         * @return 0 on success, -1 on failure.
         * @throw std::system_error if the descriptor ring cannot be mapped.
         */
        template <size_t Count, size_t Size>
        int registerBufferRing (uint16_t group, LocalMem::Allocator<Count, Size>& arena)
        {
            return _proactor->registerBufferRing (group, arena);
        }

        /**
         * @brief unregister a provided buffer ring from the proactor driving this socket.
         * @param group buffer group id.
         * @return 0 on success, -1 on failure.
         */
        int unregisterBufferRing (uint16_t group)
        {
            return _proactor->unregisterBufferRing (group);
        }

#ifdef JOIN_HAS_IO_URING
        /**
         * @brief register the arena chunks as fixed buffers on the proactor driving this socket.
         * @param arena arena to register.
         * @return 0 on success, -1 on failure.
         */
        template <size_t Count, size_t... Sizes>
        int registerFixedBuffers (LocalMem::Allocator<Count, Sizes...>& arena) noexcept
        {
            return _proactor->registerFixedBuffers (arena);
        }

        /**
         * @brief unregister the fixed buffers from the proactor driving this socket.
         * @return 0 on success, -1 on failure.
         */
        int unregisterFixedBuffers () noexcept
        {
            return _proactor->unregisterFixedBuffers ();
        }
#endif

        /**
         * @brief assign the specified endpoint to the socket.
         * @param endpoint endpoint to assign to the socket.
         * @return 0 on success, -1 on failure.
         */
        int bind (const Endpoint& endpoint) noexcept
        {
            return _socket.bind (endpoint);
        }

        /**
         * @brief assign the specified device to the socket.
         * @param device device name.
         * @return 0 on success, -1 on failure.
         */
        int bindToDevice (const std::string& device) noexcept
        {
            return _socket.bindToDevice (device);
        }

        /**
         * @brief set the given option to the given value.
         * @param option socket option.
         * @param value option value.
         * @return 0 on success, -1 on failure.
         */
        int setOption (Option option, int value) noexcept
        {
            return _socket.setOption (option, value);
        }

        /**
         * @brief determine the local endpoint associated with this socket.
         * @return local endpoint.
         */
        Endpoint localEndpoint () const noexcept
        {
            return _socket.localEndpoint ();
        }

        /**
         * @brief check if the socket is opened.
         * @return true if opened, false otherwise.
         */
        bool opened () const noexcept
        {
            return _socket.opened ();
        }

        /**
         * @brief get the number of readable bytes.
         * @return the number of readable bytes, -1 on failure.
         */
        int canRead () const noexcept
        {
            return _socket.canRead ();
        }

        /**
         * @brief get address family.
         * @return address family.
         */
        int family () const noexcept
        {
            return _socket.family ();
        }

        /**
         * @brief get the protocol communication semantic.
         * @return the protocol communication semantic.
         */
        int type () const noexcept
        {
            return _socket.type ();
        }

        /**
         * @brief get socket protocol.
         * @return socket protocol.
         */
        int protocol () const noexcept
        {
            return _socket.protocol ();
        }

        /**
         * @brief get socket native handle.
         * @return socket native handle.
         */
        int handle () const noexcept
        {
            return _socket.handle ();
        }

    protected:
        /**
         * @brief method called when an operation completes.
         * @param op completed operation.
         * @param result number of bytes transferred, or negative errno.
         */
        void onComplete (IoOperation* op, int result) override
        {
            std::error_code code =
                (result < 0) ? std::error_code (-result, std::generic_category ()) : std::error_code ();

            dispatch (op, code, (result > 0) ? static_cast<size_t> (result) : 0);
        }

        /**
         * @brief method called when an operation is cancelled.
         * @param op cancelled operation.
         * @param result negative errno.
         */
        void onCancel (IoOperation* op, [[maybe_unused]] int result) override
        {
            dispatch (op, make_error_code (std::errc::operation_canceled), 0);
        }

        /**
         * @brief invoke the completion handler of the given operation, then release it.
         * @param op completed operation.
         * @param code error code reported by the kernel.
         * @param size number of bytes transferred.
         */
        void dispatch (IoOperation* op, const std::error_code& code, size_t size) noexcept
        {
            IoOperation::Opcode opcode = static_cast<IoOperation::Opcode> (op->code);

            if ((opcode == IoOperation::Opcode::RecvMsg) || (opcode == IoOperation::Opcode::Recv) ||
                (opcode == IoOperation::Opcode::ReadFixed))
            {
                completeRead (reinterpret_cast<AsyncRead*> (op), code, size);
            }
            else if ((opcode == IoOperation::Opcode::SendMsg) || (opcode == IoOperation::Opcode::WriteFixed))
            {
                completeWrite (reinterpret_cast<AsyncWrite*> (op), code, size);
            }
            else
            {
                completeConnect (reinterpret_cast<AsyncWrite*> (op), code);
            }
        }

        /**
         * @brief invoke the connect completion handler.
         * @param code error code reported by the kernel.
         */
        void completeConnect (AsyncWrite* connect, const std::error_code& code) noexcept
        {
            ConnectHandler handler = std::move (connect->connectHandler);

            releaseWrite (connect);

            if (code)
            {
                _socket.close ();
            }
            else
            {
                _socket._state = Socket::Connected;
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

            if (_socket.type () == SOCK_STREAM)
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

            releaseRead (read);

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
                releaseWrite (write);
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
            void* chunk = _readArena.allocate (sizeof (AsyncRead));
            if (JOIN_UNLIKELY (chunk == nullptr))
            {
                return nullptr;
            }

            AsyncRead* read = new (chunk) AsyncRead ();
            _readOps[_readArena.getIndex (chunk)].store (&read->op, std::memory_order_release);

            return read;
        }

        /**
         * @brief allocate a write operation in the arena.
         * @return allocated write operation, or nullptr if the arena is exhausted.
         */
        AsyncWrite* allocateWrite () noexcept
        {
            void* chunk = _writeArena.allocate (sizeof (AsyncWrite));
            if (JOIN_UNLIKELY (chunk == nullptr))
            {
                return nullptr;
            }

            AsyncWrite* write = new (chunk) AsyncWrite ();
            _writeOps[_writeArena.getIndex (chunk)].store (&write->op, std::memory_order_release);

            return write;
        }

        /**
         * @brief return a read operation to the arena.
         * @param read read operation to release.
         */
        void releaseRead (AsyncRead* read) noexcept
        {
            _readOps[_readArena.getIndex (read)].store (nullptr, std::memory_order_release);
            read->~AsyncRead ();
            _readArena.deallocate (read);
        }

        /**
         * @brief return a write operation to the arena.
         * @param write write operation to release.
         */
        void releaseWrite (AsyncWrite* write) noexcept
        {
            _writeOps[_writeArena.getIndex (write)].store (nullptr, std::memory_order_release);
            write->~AsyncWrite ();
            _writeArena.deallocate (write);
        }

        /**
         * @brief cancel the given operation, if in flight.
         * @param op operation to cancel.
         * @return 0 on success, -1 on failure.
         */
        int cancelOp (IoOperation* op) noexcept
        {
            if ((op == nullptr) || !inFlight (*op))
            {
                return 0;
            }

            if (_proactor->cancel (op, true, true) == -1)
            {
                return (lastError == Errc::OperationFailed) ? 0 : -1;
            }

            return 0;
        }

        /**
         * @brief cancel every operation in flight.
         */
        void cancelAll () noexcept
        {
            for (auto& slot : _readOps)
            {
                cancelOp (slot.load (std::memory_order_acquire));
            }

            for (auto& slot : _writeOps)
            {
                cancelOp (slot.load (std::memory_order_acquire));
            }
        }

        /**
         * @brief check if at least one operation is in flight or completing.
         * @return true if at least one operation is in flight or completing, false otherwise.
         */
        bool pendingAny () const noexcept
        {
            for (auto& slot : _readOps)
            {
                IoOperation* op = slot.load (std::memory_order_acquire);
                if ((op != nullptr) && pending (*op))
                {
                    return true;
                }
            }

            for (auto& slot : _writeOps)
            {
                IoOperation* op = slot.load (std::memory_order_acquire);
                if ((op != nullptr) && pending (*op))
                {
                    return true;
                }
            }

            return false;
        }

        /// proactor driving the operations.
        Proactor* _proactor;

        /// underlying synchronous socket.
        Socket _socket;

        /// read operations arena.
        ReadArena _readArena;

        /// write operations arena.
        WriteArena _writeArena;

        /// read operations in flight.
        std::array<std::atomic<IoOperation*>, _opCount> _readOps{};

        /// write and connect operations in flight.
        std::array<std::atomic<IoOperation*>, _opCount> _writeOps{};
    };
}

#endif
