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

#ifndef JOIN_CORE_ASYNCACCEPTOR_HPP
#define JOIN_CORE_ASYNCACCEPTOR_HPP

// libjoin.
#include <join/acceptor.hpp>
#include <join/proactor.hpp>

namespace join
{
    /**
     * @brief asynchronous stream acceptor class.
     */
    template <class Protocol>
    class BasicAsyncStreamAcceptor : public BasicAcceptor<Protocol>, public CompletionHandler
    {
    public:
        using Endpoint = typename Protocol::Endpoint;
        using AsyncSocket = typename Protocol::AsyncSocket;
        using Stream = typename Protocol::Stream;
        using AcceptCb = void (*) (AsyncSocket&&, void*);

        /**
         * @brief create the asynchronous acceptor instance.
         * @param proactor event loop proactor.
         */
        explicit BasicAsyncStreamAcceptor (Proactor& proactor = ProactorThread::proactor ())
        : BasicAcceptor<Protocol> ()
        , _proactor (proactor)
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
         * @brief destroy instance.
         */
        virtual ~BasicAsyncStreamAcceptor () = default;

        /**
         * @brief create acceptor
         * @param endpoint endpoint to assign to the acceptor.
         * @return 0 on success, -1 on failure.
         */
        int create (const Endpoint& endpoint) noexcept override
        {
            if (BasicAcceptor<Protocol>::create (endpoint) == -1)
            {
                return -1;
            }

            if (::fcntl (this->_handle, F_SETFL, ::fcntl (this->_handle, F_GETFL) | O_NONBLOCK) == -1)
            {
                // LCOV_EXCL_START
                lastError = std::error_code (errno, std::generic_category ());
                this->close ();
                return -1;
                // LCOV_EXCL_STOP
            }

            return 0;
        }

        /**
         * @brief close acceptor.
         */
        void close () noexcept override
        {
            cancelAccept ();
            BasicAcceptor<Protocol>::close ();
        }

        /**
         * @brief submit an asynchronous accept on the listening socket.
         * @param callback completion callback.
         * @param ctx opaque user context forwarded verbatim to the callback.
         * @return 0 on successful submission, -1 on error.
         */
        int accept (AcceptCb callback, void* ctx = nullptr) noexcept
        {
            if (JOIN_UNLIKELY (!callback))
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            if (JOIN_UNLIKELY (_op.state != IoOperation::State::Idle))
            {
                lastError = make_error_code (Errc::InUse);
                return -1;
            }

            _peerAddr = {};
            _peerAddrLen = sizeof (_peerAddr);
            _callback = callback;
            _ctx = ctx;
            _op = IoOperation::makeAccept (this->_handle, reinterpret_cast<sockaddr*> (&_peerAddr), &_peerAddrLen,
                                           SOCK_NONBLOCK | SOCK_CLOEXEC, this);

            if (JOIN_UNLIKELY (_proactor.submit (&_op) == -1))
            {
                _callback = nullptr;
                _ctx = nullptr;
                _op = IoOperation{};
                return -1;
            }

            return 0;
        }

        /**
         * @brief cancel a pending asynchronous accept, if any.
         * @return 0 on success or if no operation was pending, -1 on error.
         */
        int cancelAccept () noexcept
        {
            if (_op.state == IoOperation::State::Idle)
            {
                return 0;
            }

            return _proactor.cancel (&_op);
        }

    protected:
        /**
         * @brief method called when an operation completes successfully.
         * @param op completed operation.
         * @param result number of bytes transferred, or operation-specific value.
         */
        void onComplete ([[maybe_unused]] IoOperation* op, int result) override
        {
            AcceptCb cb = _callback;
            void* ctx = _ctx;
            _callback = nullptr;
            _ctx = nullptr;

            AsyncSocket sock;

            if (JOIN_LIKELY (result >= 0))
            {
                sock._handle = result;
                sock._remote = Endpoint (reinterpret_cast<sockaddr*> (&_peerAddr), _peerAddrLen);
                sock._state = AsyncSocket::Connected;

                if (sock.protocol () == IPPROTO_TCP)
                {
                    sock.setOption (AsyncSocket::NoDelay, 1);
                }
            }
            else
            {
                lastError = std::error_code (-result, std::generic_category ());  // LCOV_EXCL_LINE
            }

            if (cb)
            {
                cb (std::move (sock), ctx);
            }
        }

        /**
         * @brief method called when an operation is cancelled.
         * @param op cancelled operation.
         * @param result -ECANCELED, or other negative errno on failure.
         */
        void onCancel ([[maybe_unused]] IoOperation* op, [[maybe_unused]] int result) override
        {
            AcceptCb cb = _callback;
            void* ctx = _ctx;
            _callback = nullptr;
            _ctx = nullptr;

            if (cb)
            {
                cb (AsyncSocket{}, ctx);
            }
        }

        /// proactor.
        Proactor& _proactor;

        /// In-flight IoOperation.
        IoOperation _op{};

        /// Storage for the peer address.
        sockaddr_storage _peerAddr{};

        /// Length of the peer address.
        socklen_t _peerAddrLen = 0;

        /// Pending completion callback.
        AcceptCb _callback = nullptr;

        /// Opaque user context forwarded verbatim to the callback.
        void* _ctx = nullptr;
    };
}

#endif
