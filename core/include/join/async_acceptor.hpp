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

namespace join
{
    /**
     * @brief asynchronous stream acceptor class.
     */
    template <class Protocol, class Engine>
    class BasicAsyncStreamAcceptor : private CompletionHandler
    {
    public:
        using Acceptor = BasicStreamAcceptor<Protocol>;
        using Endpoint = typename Protocol::Endpoint;
        using Socket = typename Protocol::Socket;
        using AsyncSocket = BasicAsyncStreamSocket<Protocol, Engine>;

        /// handler invoked on acceptation completion.
        using AcceptHandler = Function<void (const std::error_code&, AsyncSocket&&)>;

        /**
         * @brief create the acceptor instance.
         * @param engine engine driving the operations.
         */
        explicit BasicAsyncStreamAcceptor (Engine& engine = ProactorThread::proactor ())
        : _engine (engine)
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
            waitIdle ();
            _acceptor.close ();
        }

        /**
         * @brief start an asynchronous acceptation.
         * @param handler handler invoked on completion.
         * @param flags accepted socket creation flags.
         * @return 0 on success, -1 on failure.
         */
        int asyncAccept (AcceptHandler handler, int flags = SOCK_NONBLOCK | SOCK_CLOEXEC) noexcept
        {
            if (_acceptOp.state != IoOperation::State::Idle)
            {
                lastError = make_error_code (Errc::InUse);
                return -1;
            }

            if (!_acceptor.opened ())
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            _salen = sizeof (struct sockaddr_storage);
            _onAccept = std::move (handler);
            _acceptOp = IoOperation::makeAccept (_acceptor.handle (), reinterpret_cast<struct sockaddr*> (&_sa),
                                                 &_salen, flags | SOCK_NONBLOCK, this);

            _accepting.store (true, std::memory_order_release);

            if (_engine.submit (&_acceptOp, true, !_engine.isProactorThread ()) == -1)
            {
                // LCOV_EXCL_START
                _accepting.store (false, std::memory_order_release);
                _onAccept.reset ();
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
            if (_acceptOp.state != IoOperation::State::Submitted)
            {
                return 0;
            }

            return _engine.cancel (&_acceptOp, true, !_engine.isProactorThread ());
        }

        /**
         * @brief check if an acceptation is in flight.
         * @return true if an acceptation is in flight, false otherwise.
         */
        bool acceptPending () const noexcept
        {
            return _accepting.load (std::memory_order_acquire);
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

        /**
         * @brief get the underlying synchronous acceptor.
         * @return the underlying synchronous acceptor.
         */
        Acceptor& acceptor () noexcept
        {
            return _acceptor;
        }

        /**
         * @brief get the underlying synchronous acceptor.
         * @return the underlying synchronous acceptor.
         */
        const Acceptor& acceptor () const noexcept
        {
            return _acceptor;
        }

    private:
        /**
         * @brief method called when the acceptation completes.
         * @param op completed operation.
         * @param result accepted file descriptor, or negative errno.
         */
        void onComplete ([[maybe_unused]] IoOperation* op, int result) override
        {
            dispatch (result);

            _accepting.store (_acceptOp.state != IoOperation::State::Idle, std::memory_order_release);
        }

        /**
         * @brief invoke the handler owning the acceptation slot.
         * @param result accepted file descriptor, or negative errno.
         */
        void dispatch (int result) noexcept
        {
            AcceptHandler handler = std::move (_onAccept);

            if (!handler)
            {
                if (result > -1)
                {
                    ::close (result);
                }
                return;
            }

            if (result < 0)
            {
                handler (std::error_code (-result, std::generic_category ()), AsyncSocket (_engine));
            }
            else
            {
                handler (std::error_code (),
                         AsyncSocket (Socket (result, Endpoint (reinterpret_cast<struct sockaddr*> (&_sa), _salen)),
                                      _engine));
            }
        }

        /**
         * @brief method called when the acceptation is cancelled.
         * @param op cancelled operation.
         * @param result negative errno.
         */
        void onCancel ([[maybe_unused]] IoOperation* op, [[maybe_unused]] int result) override
        {
            AcceptHandler handler = std::move (_onAccept);

            if (handler)
            {
                handler (make_error_code (std::errc::operation_canceled), AsyncSocket (_engine));
            }

            _accepting.store (_acceptOp.state != IoOperation::State::Idle, std::memory_order_release);
        }

        /**
         * @brief block until the acceptation slot is idle.
         */
        void waitIdle () noexcept
        {
            if (_engine.isProactorThread ())
            {
                return;
            }

            Backoff backoff;

            while (_accepting.load (std::memory_order_acquire))
            {
                backoff ();
            }
        }

        /// underlying synchronous acceptor.
        Acceptor _acceptor;

        /// engine driving the operations.
        Engine& _engine;

        /// true while an acceptation or its completion handler is in flight.
        alignas (64) std::atomic<bool> _accepting{false};

        /// acceptation operation slot.
        IoOperation _acceptOp = {};

        /// handler invoked on acceptation completion.
        AcceptHandler _onAccept;

        /// peer address filled in by the kernel.
        struct sockaddr_storage _sa = {};

        /// peer address length.
        socklen_t _salen = sizeof (struct sockaddr_storage);
    };
}

#endif
