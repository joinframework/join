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

#ifndef JOIN_CORE_ASYNC_ACCEPTOR_HPP
#define JOIN_CORE_ASYNC_ACCEPTOR_HPP

// libjoin.
#include <join/async_stream_socket.hpp>
#include <join/acceptor.hpp>
#include <join/function.hpp>
#include <join/proactor.hpp>
#include <join/backoff.hpp>

// C++.
#include <system_error>
#include <utility>

namespace join
{
    /**
     * @brief asynchronous stream acceptor class.
     */
    template <class Protocol, class Proactor>
    class BasicAsyncStreamAcceptor : public CompletionHandler
    {
    public:
        using Acceptor = BasicStreamAcceptor<Protocol>;
        using Endpoint = typename Protocol::Endpoint;
        using Socket = typename Protocol::Socket;
        using AsyncAccept = BasicAsyncAccept<Protocol, Proactor>;
        using AcceptHandler = typename AsyncAccept::Accept;

        /**
         * @brief create the acceptor instance.
         * @param proactor proactor driving the operations.
         */
        explicit BasicAsyncStreamAcceptor (Proactor& proactor = ProactorThread::proactor ())
        : _proactor (&proactor)
        {
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicAsyncStreamAcceptor (const BasicAsyncStreamAcceptor& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicAsyncStreamAcceptor& operator= (const BasicAsyncStreamAcceptor& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        BasicAsyncStreamAcceptor (BasicAsyncStreamAcceptor&& other) = delete;

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicAsyncStreamAcceptor& operator= (BasicAsyncStreamAcceptor&& other) = delete;

        /**
         * @brief destroy the acceptor instance.
         */
        ~BasicAsyncStreamAcceptor ()
        {
            close ();
        }

        /**
         * @brief create acceptor.
         * @param endpoint endpoint to assign to the acceptor.
         * @param flags acceptor socket creation flags.
         * @return 0 on success, -1 on failure.
         */
        int create (const Endpoint& endpoint, int flags = SOCK_CLOEXEC | SOCK_NONBLOCK) noexcept
        {
            return _acceptor.create (endpoint, flags | SOCK_NONBLOCK);
        }

        /**
         * @brief close acceptor, cancelling the acceptation in flight.
         */
        void close () noexcept
        {
            Backoff backoff;

            do
            {
                cancel ();

                backoff ();
            }
            while (!_proactor->isProactorThread () && pending (_acceptOp.op));

            _acceptor.close ();
        }

        /**
         * @brief start an asynchronous acceptation.
         * @param handler handler invoked on completion.
         * @param flags accepted socket creation flags.
         * @param flush flush the submission queue.
         * @return 0 on success, -1 on failure.
         */
        ssize_t asyncAccept (AcceptHandler handler, int flags = SOCK_NONBLOCK | SOCK_CLOEXEC,
                             bool flush = true) noexcept
        {
            if (JOIN_UNLIKELY (!_acceptor.opened ()))
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            if (JOIN_UNLIKELY (!armOp ()))
            {
                lastError = make_error_code (Errc::InUse);
                return -1;
            }

            _acceptOp.remoteLen = sizeof (struct sockaddr_storage);
            _acceptOp.acceptHandler = std::move (handler);
            _acceptOp.op = IoOperation::makeAccept (_acceptor.handle (), _acceptOp.remote.addr (), &_acceptOp.remoteLen,
                                                    flags | SOCK_NONBLOCK, this);

            if (_proactor->submit (_acceptOp.op, flush, false) == -1)
            {
                // LCOV_EXCL_START
                _acceptOp.acceptHandler.reset ();
                disarmOp (IoOperation::State::Submitted);
                return -1;
                // LCOV_EXCL_STOP
            }

            return 0;
        }

        /**
         * @brief start an asynchronous multishot acceptation, staying armed until cancelled or failed.
         * @param handler handler invoked on each acceptation, the last call reporting more as false.
         * @param flags accepted socket creation flags.
         * @param flush flush the submission queue.
         * @return 0 on success, -1 on failure.
         */
        ssize_t asyncAcceptMulti (AcceptHandler handler, int flags = SOCK_NONBLOCK | SOCK_CLOEXEC,
                                  bool flush = true) noexcept
        {
            if (JOIN_UNLIKELY (!_acceptor.opened ()))
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            if (JOIN_UNLIKELY (!armOp ()))
            {
                lastError = make_error_code (Errc::InUse);
                return -1;
            }

            _acceptOp.remote = Endpoint ();
            _acceptOp.acceptHandler = std::move (handler);
            _acceptOp.op = IoOperation::makeAcceptMulti (_acceptor.handle (), flags | SOCK_NONBLOCK, this);

            if (_proactor->submit (_acceptOp.op, flush, false) == -1)
            {
                // LCOV_EXCL_START
                _acceptOp.acceptHandler.reset ();
                disarmOp (IoOperation::State::Submitted);
                return -1;
                // LCOV_EXCL_STOP
            }

            return 0;
        }

        /**
         * @brief cancel the acceptation in flight, if any.
         * @return 0 on success, -1 on failure.
         */
        int cancel () noexcept
        {
            if (!inFlight (_acceptOp.op))
            {
                return 0;
            }

            if (_proactor->cancel (_acceptOp.op, true, true) == -1)
            {
                return (lastError == Errc::OperationFailed) ? 0 : -1;
            }

            return 0;
        }

#ifdef JOIN_HAS_IO_URING
        /**
         * @brief flush the pending submissions of the proactor driving this acceptor.
         * @return 0 on success, -1 on failure.
         */
        int flush () noexcept
        {
            return _proactor->flush (false);
        }
#endif

        /**
         * @brief determine the local endpoint associated with this acceptor.
         * @return local endpoint.
         */
        Endpoint localEndpoint () const
        {
            return _acceptor.localEndpoint ();
        }

        /**
         * @brief check if the acceptor is opened.
         * @return true if opened, false otherwise.
         */
        bool opened () const noexcept
        {
            return _acceptor.opened ();
        }

        /**
         * @brief get address family.
         * @return address family.
         */
        int family () const noexcept
        {
            return _acceptor.family ();
        }

        /**
         * @brief get the acceptor communication semantic.
         * @return the acceptor communication semantic.
         */
        int type () const noexcept
        {
            return _acceptor.type ();
        }

        /**
         * @brief get acceptor protocol.
         * @return acceptor protocol.
         */
        int protocol () const noexcept
        {
            return _acceptor.protocol ();
        }

        /**
         * @brief get acceptor native handle.
         * @return acceptor native handle.
         */
        int handle () const noexcept
        {
            return _acceptor.handle ();
        }

    protected:
        /**
         * @brief method called when an operation completes.
         * @param op completed operation.
         * @param result accepted file descriptor, or negative errno.
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
         * @brief invoke the completion handler of the given operation.
         * @param op completed operation.
         * @param code error code reported by the kernel.
         * @param size accepted file descriptor, 0 when the operation failed or was cancelled.
         */
        virtual void dispatch (IoOperation& op, const std::error_code& code, size_t size) noexcept
        {
            completeAccept (reinterpret_cast<AsyncAccept*> (&op), code, size);
        }

        /**
         * @brief invoke the accept completion handler.
         * @param accept completed accept operation.
         * @param code error code reported by the kernel.
         * @param handle accepted file descriptor.
         */
        void completeAccept (AsyncAccept* accept, const std::error_code& code, size_t handle) noexcept
        {
            Socket sock = code ? Socket () : Socket (static_cast<int> (handle), accept->remote);

            if (accept->op.more)
            {
                if (JOIN_LIKELY (accept->acceptHandler))
                {
                    accept->acceptHandler (std::move (sock), code, true);
                }

                return;
            }

            AcceptHandler handler = std::move (accept->acceptHandler);

            if (JOIN_LIKELY (handler))
            {
                handler (std::move (sock), code, false);
            }

            disarmOp (IoOperation::State::Busy);
        }

        /**
         * @brief reserve the accept operation for submission.
         * @return true if the operation was reserved, false if already in flight.
         */
        bool armOp () noexcept
        {
            IoOperation::State expected = IoOperation::State::Idle;

            return _acceptOp.op.state.compare_exchange_strong (expected, IoOperation::State::Submitted,
                                                               std::memory_order_acquire, std::memory_order_relaxed) ||
                   (expected == IoOperation::State::Busy);
        }

        /**
         * @brief release the accept operation reservation.
         * @param expected state the operation is expected to be in.
         */
        void disarmOp (IoOperation::State expected) noexcept
        {
            _acceptOp.op.state.compare_exchange_strong (expected, IoOperation::State::Idle, std::memory_order_release,
                                                        std::memory_order_relaxed);
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

    private:
        /// proactor driving the operations.
        Proactor* _proactor;

        /// underlying synchronous acceptor.
        Acceptor _acceptor;

        /// accept operation.
        AsyncAccept _acceptOp;
    };
}

#endif
