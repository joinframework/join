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

// C.
#include <cstddef>

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
         * @brief move constructor, no operation may be in flight on the moved object.
         * @param other other object to move.
         */
        BasicAsyncStreamSocket (BasicAsyncStreamSocket&& other) noexcept
        : _socket (std::move (other._socket))
        , _engine (other._engine)
        , _onConnect (std::move (other._onConnect))
        , _onRead (std::move (other._onRead))
        , _onWrite (std::move (other._onWrite))
        {
        }

        /**
         * @brief move assignment operator, no operation may be in flight on either object.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicAsyncStreamSocket& operator= (BasicAsyncStreamSocket&& other) noexcept
        {
            _socket = std::move (other._socket);
            _engine = other._engine;
            _onConnect = std::move (other._onConnect);
            _onRead = std::move (other._onRead);
            _onWrite = std::move (other._onWrite);

            return *this;
        }

        /**
         * @brief destroy the socket instance.
         */
        ~BasicAsyncStreamSocket ()
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
            cancelRead ();
            cancelWrite ();
            waitIdle ();
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
            if (_writeOp.state != IoOperation::State::Idle)
            {
                // LCOV_EXCL_START
                lastError = make_error_code (Errc::InUse);
                return -1;
                // LCOV_EXCL_STOP
            }

            if (!_socket.opened () && (_socket.open (endpoint.protocol ()) == -1))
            {
                return -1;  // LCOV_EXCL_LINE
            }

            _socket._state = Socket::Connecting;
            _socket._remote = endpoint;
            _onConnect = std::move (handler);
            _writeOp =
                IoOperation::makeConnect (_socket.handle (), _socket._remote.addr (), _socket._remote.length (), this);

            _writing.store (true, std::memory_order_release);

            if (_engine->submit (&_writeOp, true, !_engine->isProactorThread ()) == -1)
            {
                // LCOV_EXCL_START
                _writing.store (false, std::memory_order_release);
                _onConnect.reset ();
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
            if (_readOp.state != IoOperation::State::Idle)
            {
                lastError = make_error_code (Errc::InUse);
                return -1;
            }

            if (!_socket.opened ())
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            _onRead = std::move (handler);
            _readOp = IoOperation::makeRecv (_socket.handle (), data, static_cast<uint32_t> (maxSize), 0, this);

            _reading.store (true, std::memory_order_release);

            if (_engine->submit (&_readOp, true, !_engine->isProactorThread ()) == -1)
            {
                // LCOV_EXCL_START
                _reading.store (false, std::memory_order_release);
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
            if (_writeOp.state != IoOperation::State::Idle)
            {
                // LCOV_EXCL_START
                lastError = make_error_code (Errc::InUse);
                return -1;
                // LCOV_EXCL_STOP
            }

            if (!_socket.opened ())
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            _onWrite = std::move (handler);
            _writeOp =
                IoOperation::makeSend (_socket.handle (), data, static_cast<uint32_t> (size), MSG_NOSIGNAL, this);

            _writing.store (true, std::memory_order_release);

            if (_engine->submit (&_writeOp, true, !_engine->isProactorThread ()) == -1)
            {
                // LCOV_EXCL_START
                _writing.store (false, std::memory_order_release);
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
            if (_readOp.state != IoOperation::State::Submitted)
            {
                return 0;
            }

            return _engine->cancel (&_readOp, true, !_engine->isProactorThread ());
        }

        /**
         * @brief cancel the connect or write operation in flight, if any.
         * @return 0 on success, -1 on failure.
         */
        int cancelWrite () noexcept
        {
            if (_writeOp.state != IoOperation::State::Submitted)
            {
                return 0;
            }

            return _engine->cancel (&_writeOp, true, !_engine->isProactorThread ());  // LCOV_EXCL_LINE
        }

        /**
         * @brief check if a read operation is in flight.
         * @return true if a read operation is in flight, false otherwise.
         */
        bool readPending () const noexcept
        {
            return _reading.load (std::memory_order_acquire);
        }

        /**
         * @brief check if a connect or write operation is in flight.
         * @return true if a connect or write operation is in flight, false otherwise.
         */
        bool writePending () const noexcept
        {
            return _writing.load (std::memory_order_acquire);
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
            bool isRead = (op == &_readOp);

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

                if (!code)
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

            if (isRead)
            {
                _reading.store (_readOp.state != IoOperation::State::Idle, std::memory_order_release);
            }
            else
            {
                _writing.store (_writeOp.state != IoOperation::State::Idle, std::memory_order_release);
            }
        }

        /**
         * @brief block until both operation slots are idle.
         */
        void waitIdle () noexcept
        {
            if (_engine->isProactorThread ())
            {
                return;
            }

            Backoff backoff;

            while (_reading.load (std::memory_order_acquire) || _writing.load (std::memory_order_acquire))
            {
                backoff ();
            }
        }

        /// underlying synchronous socket.
        Socket _socket;

        /// engine driving the operations.
        Engine* _engine;

        /// true while a read operation or its completion handler is in flight.
        alignas (64) std::atomic<bool> _reading{false};

        /// true while a connect or write operation or its completion handler is in flight.
        std::atomic<bool> _writing{false};

        /// read operation slot.
        IoOperation _readOp = {};

        /// connect or write operation slot.
        IoOperation _writeOp = {};

        /// handler invoked on connect completion.
        ConnectHandler _onConnect;

        /// handler invoked on read completion.
        ReadHandler _onRead;

        /// handler invoked on write completion.
        WriteHandler _onWrite;
    };
}

#endif
