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
#include <join/backoff.hpp>
#include <join/memory.hpp>
#include <join/socket.hpp>
#include <join/utils.hpp>

// C++.
#include <system_error>
#include <utility>
#include <atomic>
#include <array>
#include <new>

// C.
#include <sys/socket.h>
#include <poll.h>

namespace join
{
    /**
     * @brief basic asynchronous socket class.
     */
    template <class Protocol, class Proactor, size_t OpCount>
    class BasicAsyncSocket : public CompletionHandler
    {
    public:
        using Socket = typename Protocol::Socket;
        using Endpoint = typename Protocol::Endpoint;
        using AsyncWait = BasicAsyncWait<Protocol, Proactor>;
        using WaitHandler = typename AsyncWait::Wait;

        /// number of operations in flight.
        static constexpr size_t _opCount = OpCount;

        static_assert (OpCount > 0, "a socket needs at least one operation slot");

        /// size of an operation slot.
        static constexpr size_t _opSize = nextPow2 (AsyncOp<Protocol, Proactor>::maxSize);

        using OpArena = LocalMem::Allocator<_opCount, _opSize>;

    protected:
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
         * @brief move constructor.
         * @param other other object to move.
         */
        BasicAsyncSocket (BasicAsyncSocket&& other) noexcept
        : _proactor (other._proactor)
        {
            other.suspendAll ();

            _arena = std::move (other._arena);

            for (size_t i = 0; i < other._ops.size (); ++i)
            {
                _ops[i].store (other._ops[i].exchange (nullptr, std::memory_order_acq_rel), std::memory_order_release);
            }

            _socket = std::move (other._socket);
        }

    public:
        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicAsyncSocket& operator= (const BasicAsyncSocket& other) = delete;

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicAsyncSocket& operator= (BasicAsyncSocket&& other) noexcept
        {
            close ();

            _proactor = other._proactor;

            other.suspendAll ();

            _arena = std::move (other._arena);

            for (size_t i = 0; i < other._ops.size (); ++i)
            {
                _ops[i].store (other._ops[i].exchange (nullptr, std::memory_order_acq_rel), std::memory_order_release);
            }

            _socket = std::move (other._socket);

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

        /**
         * @brief start an asynchronous wait for the socket to become ready in one direction.
         * @param wantRead wait until the socket is readable.
         * @param wantWrite wait until the socket is writable.
         * @param handler handler invoked on completion.
         * @param flush flush the submission queue.
         * @return index of the operation on success, -1 on failure.
         */
        ssize_t asyncWait (bool wantRead, bool wantWrite, WaitHandler handler, bool flush = true) noexcept
        {
            if (JOIN_UNLIKELY (!_socket.opened ()))
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            if (JOIN_UNLIKELY (wantRead == wantWrite))
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            AsyncWait* wait = allocateOp<AsyncWait> ();
            if (JOIN_UNLIKELY (wait == nullptr))
            {
                lastError = make_error_code (Errc::OutOfMemory);
                return -1;
            }

            wait->waitHandler = std::move (handler);
            wait->op = IoOperation::makePoll (_socket.handle (), wantRead ? POLLIN : POLLOUT, this);
            wait->op.state.store (IoOperation::State::Submitted, std::memory_order_release);

            size_t index = _arena.getIndex (wait);

            if (_proactor->submit (wait->op, flush, false) == -1)
            {
                // LCOV_EXCL_START
                releaseOp (wait);
                return -1;
                // LCOV_EXCL_STOP
            }

            return static_cast<ssize_t> (index);
        }

        /**
         * @brief start an asynchronous multishot wait, staying armed until cancelled or failed.
         * @param wantRead wait until the socket is readable.
         * @param wantWrite wait until the socket is writable.
         * @param handler handler invoked on each completion, it must drain the socket or be invoked again at once.
         * @param flush flush the submission queue.
         * @return index of the operation on success, -1 on failure.
         */
        ssize_t asyncWaitMulti (bool wantRead, bool wantWrite, WaitHandler handler, bool flush = true) noexcept
        {
            if (JOIN_UNLIKELY (!_socket.opened ()))
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            if (JOIN_UNLIKELY (wantRead == wantWrite))
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            AsyncWait* wait = allocateOp<AsyncWait> ();
            if (JOIN_UNLIKELY (wait == nullptr))
            {
                lastError = make_error_code (Errc::OutOfMemory);
                return -1;
            }

            wait->waitHandler = std::move (handler);
            wait->op = IoOperation::makePollMulti (_socket.handle (), wantRead ? POLLIN : POLLOUT, this);
            wait->op.state.store (IoOperation::State::Submitted, std::memory_order_release);

            size_t index = _arena.getIndex (wait);

            if (_proactor->submit (wait->op, flush, false) == -1)
            {
                // LCOV_EXCL_START
                releaseOp (wait);
                return -1;
                // LCOV_EXCL_STOP
            }

            return static_cast<ssize_t> (index);
        }

        /**
         * @brief cancel the operation stored at the given index, if in flight.
         * @param index index of the operation to cancel.
         * @return 0 on success, -1 on failure.
         */
        int cancel (size_t index) noexcept
        {
            if (JOIN_UNLIKELY (index >= _ops.size ()))
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            return cancelOp (_ops[index].load (std::memory_order_acquire));
        }

#ifdef JOIN_HAS_IO_URING
        /**
         * @brief flush the pending submissions of the proactor driving this socket.
         * @return 0 on success, -1 on failure.
         */
        int flush () noexcept
        {
            return _proactor->flush (false);
        }

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
        void onComplete (IoOperation& op, int result) override
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
        void onCancel (IoOperation& op, [[maybe_unused]] int result) override
        {
            dispatch (op, make_error_code (std::errc::operation_canceled), 0);
        }

        /**
         * @brief invoke the completion handler of the given operation, then release it.
         * @param op completed operation.
         * @param code error code reported by the kernel.
         * @param size number of bytes transferred, or ready events.
         */
        virtual void dispatch (IoOperation& op, const std::error_code& code, size_t size) noexcept
        {
            completeWait (reinterpret_cast<AsyncWait*> (&op), code, size);
        }

        /**
         * @brief invoke the wait completion handler.
         * @param wait completed wait operation.
         * @param code error code reported by the kernel.
         * @param revents ready events.
         */
        void completeWait (AsyncWait* wait, const std::error_code& code, size_t revents) noexcept
        {
            std::error_code result = code;

            if (!result)
            {
                bool ready = (revents & wait->op.data.poll.events) != 0;

                if (JOIN_UNLIKELY (revents & POLLERR))
                {
                    int err = 0;
                    socklen_t len = sizeof (err);
                    ::getsockopt (_socket.handle (), SOL_SOCKET, SO_ERROR, &err, &len);

                    if (JOIN_LIKELY (err != 0))
                    {
                        result = std::error_code (err, std::generic_category ());
                    }
                    else
                    {
                        result = make_error_code (Errc::OperationFailed);  // LCOV_EXCL_LINE
                    }
                }
                else if (JOIN_UNLIKELY ((revents & POLLHUP) && !ready))
                {
                    result = make_error_code (Errc::ConnectionClosed);
                }
            }

            if (wait->op.more && !result && !(revents & POLLHUP))
            {
                if (JOIN_LIKELY (wait->waitHandler))
                {
                    wait->waitHandler (result, true);
                }

                return;
            }

            WaitHandler handler = std::move (wait->waitHandler);

            if (wait->op.more)
            {
                cancelOp (&wait->op);
            }
            else
            {
                releaseOp (wait);
            }

            if (handler)
            {
                handler (result, false);
            }
        }

        /**
         * @brief allocate an operation in the arena.
         * @return allocated operation, or nullptr if the arena is exhausted.
         */
        template <class Op>
        Op* allocateOp () noexcept
        {
            static_assert (sizeof (Op) <= _opSize, "operation larger than an arena slot");

            void* chunk = _arena.allocate (sizeof (Op));
            if (JOIN_UNLIKELY (chunk == nullptr))
            {
                return nullptr;
            }

            Op* operation = new (chunk) Op ();
            _ops[_arena.getIndex (chunk)].store (&operation->op, std::memory_order_release);

            return operation;
        }

        /**
         * @brief return an operation to the arena.
         * @param operation operation to release.
         */
        template <class Op>
        void releaseOp (Op* operation) noexcept
        {
            const uint32_t index = _arena.getIndex (operation);

            operation->~Op ();
            _arena.deallocate (operation);
            _ops[index].store (nullptr, std::memory_order_release);
        }

        /**
         * @brief suspend every operation in flight.
         */
        void suspendAll () noexcept
        {
            for (auto& slot : _ops)
            {
                _proactor->suspend (slot.load (std::memory_order_acquire));
            }
        }

        /**
         * @brief resume every suspended operation, redirecting it to this handler.
         */
        void resumeAll () noexcept
        {
            for (auto& slot : _ops)
            {
                _proactor->resume (slot.load (std::memory_order_acquire), this);
            }
        }

        /**
         * @brief cancel every operation in flight.
         */
        void cancelAll () noexcept
        {
            for (auto& slot : _ops)
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
            for (auto& slot : _ops)
            {
                IoOperation* op = slot.load (std::memory_order_acquire);
                if ((op != nullptr) && pending (*op))
                {
                    return true;
                }
            }

            return false;
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

            if (_proactor->cancel (*op, true, true) == -1)
            {
                return (lastError == Errc::OperationFailed) ? 0 : -1;
            }

            return 0;
        }

        /**
         * @brief check if an operation is in flight.
         * @param op operation to check.
         * @return true if the operation is in flight, false otherwise.
         */
        bool inFlight (const IoOperation& op) const noexcept
        {
            return op.state.load (std::memory_order_acquire) == IoOperation::State::Submitted;
        }

        /**
         * @brief check if an operation is in flight or completing.
         * @param op operation to check.
         * @return true if the operation is in flight or completing, false otherwise.
         */
        bool pending (const IoOperation& op) const noexcept
        {
            IoOperation::State state = op.state.load (std::memory_order_acquire);

            return (state == IoOperation::State::Submitted) || (state == IoOperation::State::Busy);
        }

        /// proactor driving the operations.
        Proactor* _proactor;

        /// underlying synchronous socket.
        Socket _socket;

        /// operations arena.
        OpArena _arena;

        /// operations in flight.
        std::array<std::atomic<IoOperation*>, _opCount> _ops{};
    };
}

#endif
