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
#include <join/backoff.hpp>
#include <join/socket.hpp>
#include <join/utils.hpp>

// C++.
#include <system_error>
#include <utility>
#include <atomic>
#include <memory>
#include <string>
#include <new>

// C.
#include <cstdlib>
#include <cstddef>
#include <cstdint>

namespace join
{
    /**
     * @brief basic asynchronous socket class.
     */
    template <class Protocol, class Proactor>
    class BasicAsyncSocket
    {
    public:
        using Socket = typename Protocol::Socket;
        using Endpoint = typename Protocol::Endpoint;
        using Option = typename Socket::Option;
        using AsyncOperation = BasicAsyncOperation<Protocol, Proactor>;
        using AsyncRead = BasicAsyncRead<Protocol, Proactor>;
        using AsyncWrite = BasicAsyncWrite<Protocol, Proactor>;
        using State = typename AsyncOperation::State;
        using ReadHandler = typename AsyncRead::Handler;
        using WriteHandler = typename AsyncWrite::Handler;

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
        : _socket (std::move (sock))
        , _proactor (&proactor)
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
        : _socket (std::move (other._socket))
        , _proactor (other._proactor)
        , _readOp (std::move (other._readOp))
        , _writeOp (std::move (other._writeOp))
        {
        }

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicAsyncSocket& operator= (BasicAsyncSocket&& other) noexcept
        {
            close ();

            _socket = std::move (other._socket);
            _proactor = other._proactor;

            _readOp = std::move (other._readOp);
            _writeOp = std::move (other._writeOp);

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

            if (_proactor->isProactorThread ())
            {
                _socket.close ();
                return;
            }

            if (_readOp != nullptr)
            {
                _readOp->drain ([this] () {
                    cancelRead ();
                });
            }

            if (_writeOp != nullptr)
            {
                _writeOp->drain ([this] () {
                    cancelWrite ();
                });
            }

            _socket.close ();

            if (_readOp != nullptr)
            {
                _readOp->release ();
            }

            if (_writeOp != nullptr)
            {
                _writeOp->release ();
            }
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
            if (JOIN_UNLIKELY (_readOp == nullptr))
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            if (JOIN_UNLIKELY (!_socket.opened ()))
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            if (_readOp->reserve (*_proactor) == -1)
            {
                return -1;
            }

            _readOp->_handler = std::move (handler);
            _readOp->_iov.iov_base = data;
            _readOp->_iov.iov_len = maxSize;
            _readOp->_msg.msg_name = nullptr;
            _readOp->_msg.msg_namelen = 0;
            _readOp->_msg.msg_iov = &_readOp->_iov;
            _readOp->_msg.msg_iovlen = 1;
            _readOp->_msg.msg_control = nullptr;
            _readOp->_msg.msg_controllen = 0;
            _readOp->_msg.msg_flags = 0;
            _readOp->_op = IoOperation::makeRecvmsg (_socket.handle (), &_readOp->_msg, 0, _readOp.get ());

            if (_proactor->submit (&_readOp->_op, true, false) == -1)
            {
                // LCOV_EXCL_START
                _readOp->release ();
                _readOp->_handler.reset ();
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
            if (JOIN_UNLIKELY (_writeOp == nullptr))
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            if (JOIN_UNLIKELY (!_socket.opened ()))
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            if (_writeOp->reserve (*_proactor) == -1)
            {
                return -1;
            }

            _writeOp->_handler = std::move (handler);
            _writeOp->_iov.iov_base = const_cast<char*> (data);
            _writeOp->_iov.iov_len = size;
            _writeOp->_msg.msg_name = nullptr;
            _writeOp->_msg.msg_namelen = 0;
            _writeOp->_msg.msg_iov = &_writeOp->_iov;
            _writeOp->_msg.msg_iovlen = 1;
            _writeOp->_msg.msg_control = nullptr;
            _writeOp->_msg.msg_controllen = 0;
            _writeOp->_msg.msg_flags = 0;
            _writeOp->_op =
                IoOperation::makeSendmsg (_socket.handle (), &_writeOp->_msg, MSG_NOSIGNAL, _writeOp.get ());

            if (_proactor->submit (&_writeOp->_op, true, false) == -1)
            {
                // LCOV_EXCL_START
                _writeOp->release ();
                _writeOp->_handler.reset ();
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
            if ((_readOp == nullptr) || (_readOp->_state.load (std::memory_order_acquire) != AsyncOperation::Pending))
            {
                return 0;
            }

            if (_proactor->cancel (&_readOp->_op, true, true) == -1)
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
            if ((_writeOp == nullptr) || (_writeOp->_state.load (std::memory_order_acquire) != AsyncOperation::Pending))
            {
                return 0;
            }

            if (_proactor->cancel (&_writeOp->_op, true, true) == -1)
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
        /// underlying synchronous socket.
        Socket _socket;

        /// proactor driving the operations.
        Proactor* _proactor;

        /// read operation.
        std::unique_ptr<AsyncRead> _readOp{new AsyncRead ()};

        /// write operation.
        std::unique_ptr<AsyncWrite> _writeOp{new AsyncWrite ()};
    };
}

#endif
