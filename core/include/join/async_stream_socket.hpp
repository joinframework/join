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
#include <memory>

namespace join
{
    /**
     * @brief asynchronous stream socket class.
     */
    template <class Protocol, class Engine>
    class BasicAsyncStreamSocket : public BasicAsyncSocket<Protocol, Engine>
    {
    public:
        using Socket = BasicStreamSocket<Protocol>;
        using Endpoint = typename Protocol::Endpoint;

        /// handler invoked on connect completion.
        using ConnectHandler = Function<void (const std::error_code&)>;

        /// handler invoked on read completion.
        using ReadHandler = typename BasicAsyncSocket<Protocol, Engine>::ReadHandler;

        /// handler invoked on write completion.
        using WriteHandler = typename BasicAsyncSocket<Protocol, Engine>::WriteHandler;

        /**
         * @brief create the socket instance.
         * @param engine engine driving the operations.
         */
        explicit BasicAsyncStreamSocket (Engine& engine = ProactorThread::proactor ())
        : BasicAsyncSocket<Protocol, Engine> (engine)
        {
        }

        /**
         * @brief create the socket instance adopting an already connected socket.
         * @param sock socket to adopt.
         * @param engine engine driving the operations.
         */
        explicit BasicAsyncStreamSocket (Socket&& sock, Engine& engine = ProactorThread::proactor ())
        : BasicAsyncSocket<Protocol, Engine> (std::move (sock), engine)
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
        : BasicAsyncSocket<Protocol, Engine> (std::move (other))
        , _remote (std::move (other._remote))
        , _onConnect (std::move (other._onConnect))
        {
        }

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicAsyncStreamSocket& operator= (BasicAsyncStreamSocket&& other) noexcept
        {
            this->close ();

            BasicAsyncSocket<Protocol, Engine>::operator= (std::move (other));

            _remote = std::move (other._remote);
            _onConnect = std::move (other._onConnect);

            return *this;
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
            if (!this->_ops || !_remote)
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            if (!this->_socket.opened () && (this->_socket.open (endpoint.protocol ()) == -1))
            {
                return -1;  // LCOV_EXCL_LINE
            }

            AsyncOperation::State expected = AsyncOperation::Idle;

            if (!this->_ops->write.state.compare_exchange_strong (expected, AsyncOperation::Pending,
                                                                  std::memory_order_acquire, std::memory_order_acquire))
            {
                if ((expected != AsyncOperation::Dispatching) || !this->_engine->isProactorThread ())
                {
                    lastError = make_error_code (Errc::InUse);
                    return -1;
                }

                this->_ops->write.state.store (AsyncOperation::Pending, std::memory_order_release);
            }

            this->_socket._state = Socket::Connecting;
            this->_socket._remote = endpoint;
            _onConnect = std::move (handler);
            *_remote = endpoint;
            this->_ops->write.op =
                IoOperation::makeConnect (this->_socket.handle (), _remote->addr (), _remote->length (), this);

            if (this->_engine->submit (&this->_ops->write.op, true, false) == -1)
            {
                // LCOV_EXCL_START
                this->_ops->write.state.store (AsyncOperation::Idle, std::memory_order_release);
                _onConnect.reset ();
                this->_socket.close ();
                return -1;
                // LCOV_EXCL_STOP
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
        /**
         * @brief method called when an operation completes.
         * @param op completed operation.
         * @param result number of bytes transferred, or operation specific value.
         */
        void onComplete (IoOperation* op, int result) override
        {
            dispatch (op, (result < 0) ? std::error_code (-result, std::generic_category ()) : std::error_code (),
                      (result > 0) ? static_cast<size_t> (result) : 0);
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
         * @brief invoke the handler owning the given operation slot.
         * @param op completed or cancelled operation.
         * @param code error code to report.
         * @param size number of bytes transferred.
         */
        void dispatch (IoOperation* op, const std::error_code& code, size_t size) noexcept
        {
            if (op == &this->_ops->read.op)
            {
                this->_ops->read.state.store (AsyncOperation::Dispatching, std::memory_order_release);

                ReadHandler handler = std::move (this->_onRead);
                std::error_code result = code;

                if (JOIN_UNLIKELY (!result && (size == 0)))
                {
                    result = make_error_code (Errc::ConnectionClosed);
                }

                if (JOIN_LIKELY (handler))
                {
                    handler (result, size);
                }

                AsyncOperation::State expected = AsyncOperation::Dispatching;
                this->_ops->read.state.compare_exchange_strong (expected, AsyncOperation::Idle,
                                                                std::memory_order_release, std::memory_order_relaxed);
                return;
            }

            this->_ops->write.state.store (AsyncOperation::Dispatching, std::memory_order_release);

            if (JOIN_UNLIKELY (op->code == static_cast<uint8_t> (IoOperation::Opcode::Connect)))
            {
                ConnectHandler handler = std::move (_onConnect);

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
            else
            {
                WriteHandler handler = std::move (this->_onWrite);

                if (JOIN_LIKELY (handler))
                {
                    handler (code, size);
                }
            }

            AsyncOperation::State expected = AsyncOperation::Dispatching;
            this->_ops->write.state.compare_exchange_strong (expected, AsyncOperation::Idle, std::memory_order_release,
                                                             std::memory_order_relaxed);
        }

        /// remote endpoint.
        std::unique_ptr<Endpoint> _remote{new Endpoint ()};

        /// handler invoked on connect completion.
        ConnectHandler _onConnect;
    };
}

#endif
