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
#include <join/function.hpp>
#include <join/proactor.hpp>
#include <join/backoff.hpp>

// C++.
#include <system_error>
#include <utility>
#include <atomic>
#include <memory>
#include <new>

// C.
#include <cstdlib>
#include <cstddef>
#include <cstdint>

namespace join
{
    /**
     * @brief asynchronous stream socket class.
     */
    template <class Protocol, class Engine>
    class BasicAsyncStreamSocket : private CompletionHandler
    {
    public:
        using Socket = BasicStreamSocket<Protocol>;
        using Endpoint = typename Protocol::Endpoint;
        using Option = typename Socket::Option;

        /// handler invoked on connect completion.
        using ConnectHandler = Function<void (const std::error_code&)>;

        /// handler invoked on read completion.
        using ReadHandler = Function<void (const std::error_code&, size_t)>;

        /// handler invoked on write completion.
        using WriteHandler = Function<void (const std::error_code&, size_t)>;

        /**
         * @brief create the socket instance.
         * @param engine engine driving the operations.
         */
        explicit BasicAsyncStreamSocket (Engine& engine = ProactorThread::proactor ())
        : _engine (&engine)
        {
        }

        /**
         * @brief create the socket instance adopting an already connected socket.
         * @param sock socket to adopt.
         * @param engine engine driving the operations.
         */
        explicit BasicAsyncStreamSocket (Socket&& sock, Engine& engine = ProactorThread::proactor ())
        : _socket (std::move (sock))
        , _engine (&engine)
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
        : _socket (std::move (other._socket))
        , _engine (other._engine)
        , _ops (std::move (other._ops))
        , _onConnect (std::move (other._onConnect))
        , _onRead (std::move (other._onRead))
        , _onWrite (std::move (other._onWrite))
        {
            _ops->readOp.handler = this;
            _ops->writeOp.handler = this;
        }

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicAsyncStreamSocket& operator= (BasicAsyncStreamSocket&& other) noexcept
        {
            if (_ops)
            {
                close ();
            }

            _socket = std::move (other._socket);
            _engine = other._engine;
            _ops = std::move (other._ops);
            _onConnect = std::move (other._onConnect);
            _onRead = std::move (other._onRead);
            _onWrite = std::move (other._onWrite);

            _ops->readOp.handler = this;
            _ops->writeOp.handler = this;

            return *this;
        }

        /**
         * @brief destroy the socket instance.
         */
        ~BasicAsyncStreamSocket ()
        {
            if (_ops)
            {
                close ();
            }
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
            cancelRead ();
            cancelWrite ();

            if (!_engine->isProactorThread ())
            {
                Backoff backoff;

                while ((_ops->readState.load (std::memory_order_acquire) != State::Idle) ||
                       (_ops->writeState.load (std::memory_order_acquire) != State::Idle))
                {
                    backoff ();
                }
            }

            _socket.close ();
        }

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
         * @brief start an asynchronous connection to the given endpoint.
         * @param endpoint endpoint to connect to.
         * @param handler handler invoked on completion.
         * @return 0 on success, -1 on failure.
         */
        int asyncConnect (const Endpoint& endpoint, ConnectHandler handler) noexcept
        {
            if (!_socket.opened () && (_socket.open (endpoint.protocol ()) == -1))
            {
                return -1;  // LCOV_EXCL_LINE
            }

            State expected = State::Idle;

            if (!_ops->writeState.compare_exchange_strong (expected, State::Pending, std::memory_order_acq_rel,
                                                           std::memory_order_acquire))
            {
                if ((expected != State::Dispatching) || !_engine->isProactorThread ())
                {
                    lastError = make_error_code (Errc::InUse);
                    return -1;
                }

                _ops->writeState.store (State::Pending, std::memory_order_release);
            }

            _socket._state = Socket::Connecting;
            _socket._remote = endpoint;
            _onConnect = std::move (handler);
            _ops->remote = endpoint;
            _ops->writeOp =
                IoOperation::makeConnect (_socket.handle (), _ops->remote.addr (), _ops->remote.length (), this);

            if (_engine->submit (&_ops->writeOp, true, false) == -1)
            {
                // LCOV_EXCL_START
                _ops->writeState.store (State::Idle, std::memory_order_release);
                _onConnect.reset ();
                _socket.close ();
                return -1;
                // LCOV_EXCL_STOP
            }

            return 0;
        }

        /**
         * @brief start an asynchronous read.
         * @param data buffer used to store the data received, valid until the handler is invoked.
         * @param maxSize maximum number of bytes to read.
         * @param handler handler invoked on completion.
         * @return 0 on success, -1 on failure.
         */
        int asyncRead (char* data, size_t maxSize, ReadHandler handler) noexcept
        {
            if (!_socket.opened ())
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            State expected = State::Idle;

            if (!_ops->readState.compare_exchange_strong (expected, State::Pending, std::memory_order_acq_rel,
                                                          std::memory_order_acquire))
            {
                if ((expected != State::Dispatching) || !_engine->isProactorThread ())
                {
                    lastError = make_error_code (Errc::InUse);
                    return -1;
                }

                _ops->readState.store (State::Pending, std::memory_order_release);
            }

            _onRead = std::move (handler);
            _ops->readOp = IoOperation::makeRecv (_socket.handle (), data, static_cast<uint32_t> (maxSize), 0, this);

            if (_engine->submit (&_ops->readOp, true, false) == -1)
            {
                // LCOV_EXCL_START
                _ops->readState.store (State::Idle, std::memory_order_release);
                _onRead.reset ();
                return -1;
                // LCOV_EXCL_STOP
            }

            return 0;
        }

        /**
         * @brief start an asynchronous write.
         * @param data data buffer to send, valid until the handler is invoked.
         * @param size number of bytes to write.
         * @param handler handler invoked on completion.
         * @return 0 on success, -1 on failure.
         */
        int asyncWrite (const char* data, size_t size, WriteHandler handler) noexcept
        {
            if (!_socket.opened ())
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            State expected = State::Idle;

            if (!_ops->writeState.compare_exchange_strong (expected, State::Pending, std::memory_order_acq_rel,
                                                           std::memory_order_acquire))
            {
                if ((expected != State::Dispatching) || !_engine->isProactorThread ())
                {
                    lastError = make_error_code (Errc::InUse);
                    return -1;
                }

                _ops->writeState.store (State::Pending, std::memory_order_release);
            }

            _onWrite = std::move (handler);
            _ops->writeOp =
                IoOperation::makeSend (_socket.handle (), data, static_cast<uint32_t> (size), MSG_NOSIGNAL, this);

            if (_engine->submit (&_ops->writeOp, true, false) == -1)
            {
                // LCOV_EXCL_START
                _ops->writeState.store (State::Idle, std::memory_order_release);
                _onWrite.reset ();
                return -1;
                // LCOV_EXCL_STOP
            }

            return 0;
        }

        /**
         * @brief cancel the read operation in flight, if any.
         * @return 0 on success, -1 on failure.
         */
        int cancelRead () noexcept
        {
            if (_ops->readState.load (std::memory_order_acquire) == State::Idle)
            {
                return 0;
            }

            if (_engine->cancel (&_ops->readOp, true, true) == -1)
            {
                return (lastError == Errc::OperationFailed) ? 0 : -1;
            }

            return 0;
        }

        /**
         * @brief cancel the connect or write operation in flight, if any.
         * @return 0 on success, -1 on failure.
         */
        int cancelWrite () noexcept
        {
            if (_ops->writeState.load (std::memory_order_acquire) == State::Idle)
            {
                return 0;
            }

            if (_engine->cancel (&_ops->writeOp, true, true) == -1)
            {
                return (lastError == Errc::OperationFailed) ? 0 : -1;
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
         * @brief determine the remote endpoint associated with this socket.
         * @return remote endpoint.
         */
        const Endpoint& remoteEndpoint () const noexcept
        {
            return _socket.remoteEndpoint ();
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
         * @brief check if the socket is connected.
         * @return true if connected, false otherwise.
         */
        bool connected () noexcept
        {
            return _socket.connected ();
        }

        /**
         * @brief check if the socket is connecting.
         * @return true if connecting, false otherwise.
         */
        bool connecting () const noexcept
        {
            return _socket.connecting ();
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
         * @brief get the path maximum transmission unit.
         * @return the path maximum transmission unit, -1 on failure.
         */
        int mtu () const noexcept
        {
            return _socket.mtu ();
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

        /**
         * @brief get the underlying synchronous socket.
         * @return the underlying synchronous socket.
         */
        Socket& socket () noexcept
        {
            return _socket;
        }

        /**
         * @brief get the underlying synchronous socket.
         * @return the underlying synchronous socket.
         */
        const Socket& socket () const noexcept
        {
            return _socket;
        }

    private:
        /**
         * @brief operation slot state.
         */
        enum class State : uint8_t
        {
            Idle,        /**< no operation in flight and no completion handler running. */
            Pending,     /**< an operation is in flight. */
            Dispatching, /**< the completion handler is running. */
        };

        /**
         * @brief operation block, kept at a stable address across moves.
         */
        struct Ops
        {
            /**
             * @brief allocate a block honouring its extended alignment.
             * @param size allocation size in bytes.
             * @return pointer to the allocated storage.
             */
            static void* operator new (size_t size)
            {
                void* mem = ::aligned_alloc (alignof (Ops), size);

                if (mem == nullptr)
                {
                    throw std::bad_alloc ();  // LCOV_EXCL_LINE
                }

                return mem;
            }

            /**
             * @brief release storage allocated by operator new.
             * @param mem storage to release.
             */
            static void operator delete (void* mem) noexcept
            {
                ::free (mem);
            }

            /// read operation slot.
            alignas (64) IoOperation readOp = {};

            /// connect or write operation slot.
            alignas (64) IoOperation writeOp = {};

            /// read slot state.
            alignas (64) std::atomic<State> readState{State::Idle};

            /// connect or write slot state.
            alignas (64) std::atomic<State> writeState{State::Idle};

            /// remote endpoint, read by the kernel until the connect completes.
            Endpoint remote;
        };

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
            bool isRead = (op == &_ops->readOp);
            std::atomic<State>& state = isRead ? _ops->readState : _ops->writeState;

            state.store (State::Dispatching, std::memory_order_release);

            if (isRead)
            {
                ReadHandler handler = std::move (_onRead);

                if (handler)
                {
                    handler ((!code && (size == 0)) ? make_error_code (Errc::ConnectionClosed) : code, size);
                }
            }
            else if (op->code == static_cast<uint8_t> (IoOperation::Opcode::Connect))
            {
                ConnectHandler handler = std::move (_onConnect);

                if (code)
                {
                    _socket.close ();
                }
                else
                {
                    _socket._state = Socket::Connected;
                }

                if (handler)
                {
                    handler (code);
                }
            }
            else
            {
                WriteHandler handler = std::move (_onWrite);

                if (handler)
                {
                    handler (code, size);
                }
            }

            State expected = State::Dispatching;
            state.compare_exchange_strong (expected, State::Idle, std::memory_order_acq_rel, std::memory_order_acquire);
        }

        /// underlying synchronous socket.
        Socket _socket;

        /// engine driving the operations.
        Engine* _engine;

        /// operation block.
        std::unique_ptr<Ops> _ops{new Ops ()};

        /// handler invoked on connect completion.
        ConnectHandler _onConnect;

        /// handler invoked on read completion.
        ReadHandler _onRead;

        /// handler invoked on write completion.
        WriteHandler _onWrite;
    };
}

#endif
