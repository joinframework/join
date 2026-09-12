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
#include <join/socket.hpp>
#include <join/utils.hpp>

// C++.
#include <system_error>
#include <utility>
#include <memory>
#include <string>

// C.
#include <cstddef>

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
        using AsyncConnect = BasicAsyncConnect<Protocol, Proactor>;
        using ReadHandler = typename AsyncRead::Handler;
        using WriteHandler = typename AsyncWrite::Handler;
        using ConnectHandler = typename AsyncConnect::Handler;

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
            if (other._readOp != nullptr)
            {
                _proactor->suspend (&other._readOp->op);
            }

            if (other._writeOp != nullptr)
            {
                _proactor->suspend (&other._writeOp->op);
            }

            if (other._connectOp != nullptr)
            {
                _proactor->suspend (&other._connectOp->op);
            }

            _socket = std::move (other._socket);
            _readOp = std::move (other._readOp);
            _writeOp = std::move (other._writeOp);
            _connectOp = std::move (other._connectOp);

            if (_readOp != nullptr)
            {
                _proactor->resume (&_readOp->op, this);
            }

            if (_writeOp != nullptr)
            {
                _proactor->resume (&_writeOp->op, this);
            }

            if (_connectOp != nullptr)
            {
                _proactor->resume (&_connectOp->op, this);
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

            if (other._readOp != nullptr)
            {
                _proactor->suspend (&other._readOp->op);
            }

            if (other._writeOp != nullptr)
            {
                _proactor->suspend (&other._writeOp->op);
            }

            if (other._connectOp != nullptr)
            {
                _proactor->suspend (&other._connectOp->op);
            }

            _socket = std::move (other._socket);
            _readOp = std::move (other._readOp);
            _writeOp = std::move (other._writeOp);
            _connectOp = std::move (other._connectOp);

            if (_readOp != nullptr)
            {
                _proactor->resume (&_readOp->op, this);
            }

            if (_writeOp != nullptr)
            {
                _proactor->resume (&_writeOp->op, this);
            }

            if (_connectOp != nullptr)
            {
                _proactor->resume (&_connectOp->op, this);
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
            cancelRead ();
            cancelWrite ();
            cancelConnect ();

            _socket.close ();
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
            if (JOIN_UNLIKELY (!_socket.opened ()))
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            if (JOIN_UNLIKELY (!armable (_readOp->op)))
            {
                lastError = make_error_code (Errc::InUse);
                return -1;
            }

            _readOp->handler = std::move (handler);
            _readOp->iov.iov_base = data;
            _readOp->iov.iov_len = maxSize;
            _readOp->msg.msg_name = nullptr;
            _readOp->msg.msg_namelen = 0;
            _readOp->msg.msg_iov = &_readOp->iov;
            _readOp->msg.msg_iovlen = 1;
            _readOp->msg.msg_control = nullptr;
            _readOp->msg.msg_controllen = 0;
            _readOp->msg.msg_flags = 0;
            _readOp->op = IoOperation::makeRecvmsg (_socket.handle (), &_readOp->msg, 0, this);

            if (_proactor->submit (&_readOp->op, true, false) == -1)
            {
                // LCOV_EXCL_START
                _readOp->handler.reset ();
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
            if (JOIN_UNLIKELY (!_socket.opened ()))
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            if (JOIN_UNLIKELY (!armable (_writeOp->op)))
            {
                lastError = make_error_code (Errc::InUse);
                return -1;
            }

            _writeOp->handler = std::move (handler);
            _writeOp->iov.iov_base = const_cast<char*> (data);
            _writeOp->iov.iov_len = size;
            _writeOp->msg.msg_name = nullptr;
            _writeOp->msg.msg_namelen = 0;
            _writeOp->msg.msg_iov = &_writeOp->iov;
            _writeOp->msg.msg_iovlen = 1;
            _writeOp->msg.msg_control = nullptr;
            _writeOp->msg.msg_controllen = 0;
            _writeOp->msg.msg_flags = 0;
            _writeOp->op = IoOperation::makeSendmsg (_socket.handle (), &_writeOp->msg, MSG_NOSIGNAL, this);

            if (_proactor->submit (&_writeOp->op, true, false) == -1)
            {
                // LCOV_EXCL_START
                _writeOp->handler.reset ();
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
            if (_readOp == nullptr)
            {
                return 0;
            }

            if (_proactor->cancel (&_readOp->op, true, true) == -1)
            {
                return (lastError == Errc::OperationFailed) ? 0 : -1;
            }

            return 0;
        }

        /**
         * @brief cancel the write operation in flight, if any.
         * @return 0 on success, -1 on failure.
         */
        int cancelWrite () noexcept
        {
            if (_writeOp == nullptr)
            {
                return 0;
            }

            if (_proactor->cancel (&_writeOp->op, true, true) == -1)
            {
                return (lastError == Errc::OperationFailed) ? 0 : -1;
            }

            return 0;
        }

        /**
         * @brief cancel the connect operation in flight, if any.
         * @return 0 on success, -1 on failure.
         */
        int cancelConnect () noexcept
        {
            if (_connectOp == nullptr)
            {
                return 0;
            }

            if (_proactor->cancel (&_connectOp->op, true, true) == -1)
            {
                return (lastError == Errc::OperationFailed) ? 0 : -1;
            }

            return 0;
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

            if (op == &_readOp->op)
            {
                completeRead (code, (result > 0) ? static_cast<size_t> (result) : 0);
            }
            else if (op == &_writeOp->op)
            {
                completeWrite (code, (result > 0) ? static_cast<size_t> (result) : 0);
            }
            else
            {
                completeConnect (code);
            }
        }

        /**
         * @brief method called when an operation is cancelled.
         * @param op cancelled operation.
         * @param result negative errno.
         */
        void onCancel (IoOperation* op, [[maybe_unused]] int result) override
        {
            std::error_code code = make_error_code (std::errc::operation_canceled);

            if (op == &_readOp->op)
            {
                completeRead (code, 0);
            }
            else if (op == &_writeOp->op)
            {
                completeWrite (code, 0);
            }
            else
            {
                completeConnect (code);
            }
        }

        /**
         * @brief invoke the read completion handler.
         * @param code error code reported by the kernel.
         * @param size number of bytes received.
         */
        void completeRead (const std::error_code& code, size_t size) noexcept
        {
            ReadHandler handler = std::move (_readOp->handler);
            std::error_code result = code;

            if (_readOp->stream)
            {
                if (JOIN_UNLIKELY (!result && (size == 0)))
                {
                    result = make_error_code (Errc::ConnectionClosed);
                }
            }
            else if (JOIN_UNLIKELY (!result && (_readOp->msg.msg_flags & MSG_TRUNC)))
            {
                result = make_error_code (Errc::MessageTooLong);
            }

            if (JOIN_LIKELY (handler))
            {
                handler (result, size);
            }
        }

        /**
         * @brief invoke the write completion handler.
         * @param code error code reported by the kernel.
         * @param size number of bytes sent.
         */
        void completeWrite (const std::error_code& code, size_t size) noexcept
        {
            WriteHandler handler = std::move (_writeOp->handler);

            if (JOIN_LIKELY (handler))
            {
                handler (code, size);
            }
        }

        /**
         * @brief invoke the connect completion handler.
         * @param code error code reported by the kernel.
         */
        void completeConnect (const std::error_code& code) noexcept
        {
            ConnectHandler handler = std::move (_connectOp->handler);

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
         * @brief check if an operation can be armed.
         * @param op operation to check.
         * @return true if the operation can be armed, false otherwise.
         */
        bool armable (const IoOperation& op) const noexcept
        {
            IoOperation::State state = op.state.load (std::memory_order_acquire);

            return (state == IoOperation::State::Idle) || (state == IoOperation::State::Busy);
        }

        /// proactor driving the operations.
        Proactor* _proactor;

        /// underlying synchronous socket.
        Socket _socket;

        /// read operation.
        std::unique_ptr<AsyncRead> _readOp{new AsyncRead ()};

        /// write operation.
        std::unique_ptr<AsyncWrite> _writeOp{new AsyncWrite ()};

        /// connect operation.
        std::unique_ptr<AsyncConnect> _connectOp{new AsyncConnect ()};
    };
}

#endif
