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
#include <join/stream_socket.hpp>
#include <join/async_socket.hpp>

// C++.
#include <system_error>
#include <utility>

namespace join
{
    /**
     * @brief asynchronous stream socket class.
     */
    template <class Protocol, class Proactor>
    class BasicAsyncStreamSocket : public BasicAsyncSocket<Protocol, Proactor>
    {
        /// friendship with basic asynchronous accept operation
        friend class BasicAsyncAccept<Protocol, Proactor>;

        /// friendship with basic asynchronous stream acceptor
        friend class BasicAsyncStreamAcceptor<Protocol, Proactor>;

    public:
        using Socket = BasicStreamSocket<Protocol>;
        using Endpoint = typename Protocol::Endpoint;
        using AsyncOperation = BasicAsyncOperation<Protocol, Proactor>;
        using AsyncAccept = BasicAsyncAccept<Protocol, Proactor>;
        using AsyncConnect = BasicAsyncConnect<Protocol, Proactor>;
        using AsyncRead = BasicAsyncRead<Protocol, Proactor>;
        using AsyncWrite = BasicAsyncWrite<Protocol, Proactor>;
        using State = typename AsyncOperation::State;
        using ConnectHandler = typename AsyncConnect::Handler;
        using ReadHandler = typename AsyncRead::Handler;
        using WriteHandler = typename AsyncWrite::Handler;

        /**
         * @brief create the socket instance.
         * @param proactor proactor driving the operations.
         */
        explicit BasicAsyncStreamSocket (Proactor& proactor = ProactorThread::proactor ())
        : BasicAsyncSocket<Protocol, Proactor> (proactor)
        {
            this->_readOp->_stream = true;
            _connectOp->_socket = &this->_socket;
        }

        /**
         * @brief create the socket instance adopting an already connected socket.
         * @param sock socket to adopt.
         * @param proactor proactor driving the operations.
         */
        explicit BasicAsyncStreamSocket (Socket&& sock, Proactor& proactor = ProactorThread::proactor ())
        : BasicAsyncSocket<Protocol, Proactor> (std::move (sock), proactor)
        {
            this->_readOp->_stream = true;
            _connectOp->_socket = &this->_socket;
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
        : BasicAsyncSocket<Protocol, Proactor> (std::move (other))
        , _connectOp (std::move (other._connectOp))
        , _pendingAccept (other._pendingAccept)
        {
            if (_connectOp != nullptr)
            {
                _connectOp->_socket = &this->_socket;
            }

            other._pendingAccept = nullptr;

            if (_pendingAccept != nullptr)
            {
                _pendingAccept->_peer = this;
            }
        }

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicAsyncStreamSocket& operator= (BasicAsyncStreamSocket&& other) noexcept
        {
            BasicAsyncSocket<Protocol, Proactor>::operator= (std::move (other));

            _connectOp = std::move (other._connectOp);

            if (_connectOp != nullptr)
            {
                _connectOp->_socket = &this->_socket;
            }

            _pendingAccept = other._pendingAccept;
            other._pendingAccept = nullptr;

            if (_pendingAccept != nullptr)
            {
                _pendingAccept->_peer = this;
            }

            return *this;
        }

        /**
         * @brief close the socket, cancelling the operations in flight.
         */
        void close () noexcept
        {
            cancelConnect ();

            if (this->_proactor->isProactorThread ())
            {
                BasicAsyncSocket<Protocol, Proactor>::close ();
                return;
            }

            if (_connectOp != nullptr)
            {
                _connectOp->drain ([this] () {
                    cancelConnect ();
                });
            }

            BasicAsyncSocket<Protocol, Proactor>::close ();

            if (_connectOp != nullptr)
            {
                _connectOp->release ();
            }
        }

        /**
         * @brief destroy the socket instance.
         */
        ~BasicAsyncStreamSocket ()
        {
            this->close ();
        }

        /**
         * @brief start an asynchronous connection to the given endpoint.
         * @param endpoint endpoint to connect to.
         * @param handler handler invoked on completion.
         * @return 0 on success, -1 on failure.
         */
        int asyncConnect (const Endpoint& endpoint, ConnectHandler handler) noexcept
        {
            if (JOIN_UNLIKELY (_connectOp == nullptr))
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

            if (this->_connectOp->reserve (*this->_proactor) == -1)
            {
                return -1;
            }

            this->_socket._state = Socket::Connecting;
            this->_socket._remote = endpoint;
            _connectOp->_handler = std::move (handler);
            this->_connectOp->_op = IoOperation::makeConnect (this->_socket.handle (), this->_socket._remote.addr (),
                                                              this->_socket._remote.length (), this->_connectOp.get ());

            if (this->_proactor->submit (&this->_connectOp->_op, true, false) == -1)
            {
                // LCOV_EXCL_START
                this->_connectOp->release ();
                _connectOp->_handler.reset ();
                this->_socket.close ();
                return -1;
                // LCOV_EXCL_STOP
            }

            return 0;
        }

        /**
         * @brief cancel the connect operation in flight, if any.
         * @return 0 on success, -1 on failure.
         */
        int cancelConnect () noexcept
        {
            if ((_connectOp == nullptr) ||
                (_connectOp->_state.load (std::memory_order_acquire) != AsyncOperation::Pending))
            {
                return 0;
            }

            if (this->_proactor->cancel (&_connectOp->_op, true, true) == -1)
            {
                return (lastError == Errc::OperationFailed) ? 0 : -1;
            }

            return 0;
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

    protected:
        /// connect operation.
        std::unique_ptr<AsyncConnect> _connectOp{new AsyncConnect ()};

        /// acceptation operation this socket is the target of, if any.
        AsyncAccept* _pendingAccept = nullptr;
    };
}

#endif
