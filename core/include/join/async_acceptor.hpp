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
#include <atomic>
#include <memory>

// C.
#include <cstdlib>
#include <cstdint>
#include <cerrno>

namespace join
{
    /**
     * @brief asynchronous stream acceptor class.
     */
    template <class Protocol, class Proactor>
    class BasicAsyncStreamAcceptor : private CompletionHandler
    {
    public:
        using Acceptor = BasicStreamAcceptor<Protocol>;
        using Endpoint = typename Protocol::Endpoint;
        using Socket = typename Protocol::Socket;
        using AsyncSocket = BasicAsyncStreamSocket<Protocol, Proactor>;
        using AsyncOperation = BasicAsyncOperation<Protocol, Proactor>;
        using AsyncAccept = BasicAsyncAccept<Protocol, Proactor>;
        using State = typename AsyncOperation::State;
        using AcceptHandler = typename AsyncAccept::Handler;

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
            cancelAccept ();

            if (_proactor->isProactorThread ())
            {
                _acceptor.close ();
                return;
            }

            _acceptOp->drain ([this] () {
                cancelAccept ();
            });

            _acceptor.close ();

            _acceptOp->release ();
        }

        /**
         * @brief start an asynchronous acceptation.
         * @param peer closed socket receiving the accepted connection, valid until the handler is invoked.
         * @param handler handler invoked on completion.
         * @param flags accepted socket creation flags.
         * @return 0 on success, -1 on failure.
         */
        int asyncAccept (AsyncSocket& peer, AcceptHandler handler, int flags = SOCK_NONBLOCK | SOCK_CLOEXEC) noexcept
        {
            if (JOIN_UNLIKELY (!_acceptor.opened ()))
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            if (JOIN_UNLIKELY (peer.opened ()))
            {
                lastError = make_error_code (Errc::InUse);
                return -1;
            }

            if (_acceptOp->reserve (*_proactor) == -1)
            {
                return -1;
            }

            _acceptOp->_remoteLen = sizeof (struct sockaddr_storage);
            _acceptOp->_handler = std::move (handler);
            _acceptOp->_peer = &peer;
            _acceptOp->_op = IoOperation::makeAccept (_acceptor.handle (), _acceptOp->_remote.addr (),
                                                      &_acceptOp->_remoteLen, flags | SOCK_NONBLOCK, this);

            if (_proactor->submit (&_acceptOp->_op, true, false) == -1)
            {
                // LCOV_EXCL_START
                _acceptOp->release ();
                _acceptOp->_handler.reset ();
                _acceptOp->_peer = nullptr;
                return -1;
                // LCOV_EXCL_STOP
            }

            return 0;
        }

        /**
         * @brief cancel the acceptation in flight, if any.
         * @return 0 on success, -1 on failure.
         */
        int cancelAccept () noexcept
        {
            if (_acceptOp->_state.load (std::memory_order_acquire) != AsyncOperation::Pending)
            {
                return 0;
            }

            if (_proactor->cancel (&_acceptOp->_op, true, true) == -1)
            {
                return (lastError == Errc::OperationFailed) ? 0 : -1;
            }

            return 0;
        }

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

    private:
        /**
         * @brief method called when the acceptation completes.
         * @param op completed operation.
         * @param result accepted file descriptor, or negative errno.
         */
        void onComplete (IoOperation* op, int result) override
        {
            dispatch (op, result);
        }

        /**
         * @brief method called when the acceptation is cancelled.
         * @param op cancelled operation.
         * @param result negative errno.
         */
        void onCancel (IoOperation* op, [[maybe_unused]] int result) override
        {
            dispatch (op, -ECANCELED);
        }

        /**
         * @brief invoke the handler owning the acceptation slot.
         * @param op completed or cancelled operation.
         * @param result accepted file descriptor, or negative errno.
         */
        void dispatch (IoOperation* op, int result) noexcept
        {
            if (op == &_acceptOp->_op)
            {
                _acceptOp->dispatch ([this, result] () {
                    AcceptHandler handler = std::move (_acceptOp->_handler);
                    AsyncSocket* peer = _acceptOp->_peer;
                    _acceptOp->_peer = nullptr;
                    if (JOIN_UNLIKELY (result < 0))
                    {
                        if (JOIN_LIKELY (handler))
                        {
                            handler (std::error_code (-result, std::generic_category ()));
                        }
                    }
                    else
                    {
                        peer->_socket = Socket (result, _acceptOp->_remote);
                        if (JOIN_LIKELY (handler))
                        {
                            handler (std::error_code ());
                        }
                    }
                });
            }
            else
            {
                // do nothing.
            }
        }

        /// underlying synchronous acceptor.
        Acceptor _acceptor;

        /// proactor driving the operations.
        Proactor* _proactor;

        /// accept operation.
        std::unique_ptr<AsyncAccept> _acceptOp{new AsyncAccept ()};
    };
}

#endif
