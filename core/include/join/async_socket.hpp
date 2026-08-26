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
#include <join/io_operation.hpp>
#include <join/function.hpp>
#include <join/utils.hpp>
#include <join/proactor.hpp>
#include <join/backoff.hpp>
#include <join/socket.hpp>

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
     * @brief asynchronous operation.
     */
    struct AsyncOperation
    {
        /**
         * @brief caller side operation state.
         */
        enum State : uint8_t
        {
            Idle,        /**< no operation in flight and no completion handler running. */
            Pending,     /**< an operation is in flight. */
            Dispatching, /**< the completion handler is running. */
        };

        /// operation.
        IoOperation op = {};

        /// caller side operation state.
        alignas (64) std::atomic<State> state{Idle};
    };

    /**
     * @brief basic asynchronous socket class.
     */
    template <class Protocol, class Engine>
    class BasicAsyncSocket : protected CompletionHandler
    {
    public:
        using Socket = typename Protocol::Socket;
        using Endpoint = typename Protocol::Endpoint;
        using Option = typename Socket::Option;

        /// handler invoked on read completion.
        using ReadHandler = Function<void (const std::error_code&, size_t)>;

        /// handler invoked on write completion.
        using WriteHandler = Function<void (const std::error_code&, size_t)>;

        /**
         * @brief create the socket instance.
         * @param engine engine driving the operations.
         */
        explicit BasicAsyncSocket (Engine& engine = ProactorThread::proactor ())
        : _engine (&engine)
        {
        }

        /**
         * @brief create the socket instance adopting an already opened socket.
         * @param sock socket to adopt.
         * @param engine engine driving the operations.
         */
        explicit BasicAsyncSocket (Socket&& sock, Engine& engine = ProactorThread::proactor ())
        : _socket (std::move (sock))
        , _engine (&engine)
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
        , _engine (other._engine)
        , _ops (std::move (other._ops))
        , _onRead (std::move (other._onRead))
        , _onWrite (std::move (other._onWrite))
        {
            if (_ops)
            {
                _ops->read.op.handler = this;
                _ops->write.op.handler = this;
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

            _socket = std::move (other._socket);
            _engine = other._engine;
            _ops = std::move (other._ops);
            _onRead = std::move (other._onRead);
            _onWrite = std::move (other._onWrite);

            if (_ops)
            {
                _ops->read.op.handler = this;
                _ops->write.op.handler = this;
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

            if (_ops && !_engine->isProactorThread ())
            {
                Backoff backoff;

                while ((_ops->read.state.load (std::memory_order_acquire) != AsyncOperation::Idle) ||
                       (_ops->write.state.load (std::memory_order_acquire) != AsyncOperation::Idle))
                {
                    backoff ();
                }
            }

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
            if (JOIN_UNLIKELY (!_ops || !_socket.opened ()))
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            AsyncOperation::State expected = AsyncOperation::Idle;

            if (!_ops->read.state.compare_exchange_strong (expected, AsyncOperation::Pending, std::memory_order_acquire,
                                                           std::memory_order_acquire))
            {
                if ((expected != AsyncOperation::Dispatching) || !_engine->isProactorThread ())
                {
                    lastError = make_error_code (Errc::InUse);
                    return -1;
                }

                _ops->read.state.store (AsyncOperation::Pending, std::memory_order_release);
            }

            _onRead = std::move (handler);

            _ops->readIov.iov_base = data;
            _ops->readIov.iov_len = maxSize;

            _ops->readMsg.msg_name = nullptr;
            _ops->readMsg.msg_namelen = 0;
            _ops->readMsg.msg_iov = &_ops->readIov;
            _ops->readMsg.msg_iovlen = 1;
            _ops->readMsg.msg_control = nullptr;
            _ops->readMsg.msg_controllen = 0;
            _ops->readMsg.msg_flags = 0;

            _ops->read.op = IoOperation::makeRecvmsg (_socket.handle (), &_ops->readMsg, 0, this);

            if (_engine->submit (&_ops->read.op, true, false) == -1)
            {
                // LCOV_EXCL_START
                _ops->read.state.store (AsyncOperation::Idle, std::memory_order_release);
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
            if (JOIN_UNLIKELY (!_ops || !_socket.opened ()))
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            AsyncOperation::State expected = AsyncOperation::Idle;

            if (!_ops->write.state.compare_exchange_strong (expected, AsyncOperation::Pending,
                                                            std::memory_order_acquire, std::memory_order_acquire))
            {
                if ((expected != AsyncOperation::Dispatching) || !_engine->isProactorThread ())
                {
                    lastError = make_error_code (Errc::InUse);
                    return -1;
                }

                _ops->write.state.store (AsyncOperation::Pending, std::memory_order_release);
            }

            _onWrite = std::move (handler);

            _ops->writeIov.iov_base = const_cast<char*> (data);
            _ops->writeIov.iov_len = size;

            _ops->writeMsg.msg_name = nullptr;
            _ops->writeMsg.msg_namelen = 0;
            _ops->writeMsg.msg_iov = &_ops->writeIov;
            _ops->writeMsg.msg_iovlen = 1;
            _ops->writeMsg.msg_control = nullptr;
            _ops->writeMsg.msg_controllen = 0;
            _ops->writeMsg.msg_flags = 0;

            _ops->write.op = IoOperation::makeSendmsg (_socket.handle (), &_ops->writeMsg, MSG_NOSIGNAL, this);

            if (_engine->submit (&_ops->write.op, true, false) == -1)
            {
                // LCOV_EXCL_START
                _ops->write.state.store (AsyncOperation::Idle, std::memory_order_release);
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
            if (!_ops || (_ops->read.state.load (std::memory_order_acquire) == AsyncOperation::Idle))
            {
                return 0;
            }

            if (_engine->cancel (&_ops->read.op, true, true) == -1)
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
            if (!_ops || (_ops->write.state.load (std::memory_order_acquire) == AsyncOperation::Idle))
            {
                return 0;
            }

            if (_engine->cancel (&_ops->write.op, true, true) == -1)
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
            AsyncOperation read;

            /// read message header, read by the kernel until the read completes.
            msghdr readMsg = {};

            /// read scatter gather entry, read by the kernel until the read completes.
            iovec readIov = {};

            /// connect or write operation slot.
            AsyncOperation write;

            /// write message header, read by the kernel until the write completes.
            msghdr writeMsg = {};

            /// write scatter gather entry, read by the kernel until the write completes.
            iovec writeIov = {};
        };

        /// underlying synchronous socket.
        Socket _socket;

        /// engine driving the operations.
        Engine* _engine;

        /// operation block.
        std::unique_ptr<Ops> _ops{new Ops ()};

        /// handler invoked on read completion.
        ReadHandler _onRead;

        /// handler invoked on write completion.
        WriteHandler _onWrite;

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
            if (op == &_ops->read.op)
            {
                _ops->read.state.store (AsyncOperation::Dispatching, std::memory_order_release);

                ReadHandler handler = std::move (_onRead);
                std::error_code result = code;

                if (JOIN_LIKELY (!result))
                {
                    if (JOIN_UNLIKELY (size == 0))
                    {
                        result = make_error_code (Errc::ConnectionClosed);
                    }
                    else if (JOIN_UNLIKELY (_ops->readMsg.msg_flags & MSG_TRUNC))
                    {
                        result = make_error_code (Errc::MessageTooLong);
                    }
                }

                if (JOIN_LIKELY (handler))
                {
                    handler (result, size);
                }

                AsyncOperation::State expected = AsyncOperation::Dispatching;
                _ops->read.state.compare_exchange_strong (expected, AsyncOperation::Idle, std::memory_order_release,
                                                          std::memory_order_relaxed);
                return;
            }

            _ops->write.state.store (AsyncOperation::Dispatching, std::memory_order_release);

            WriteHandler handler = std::move (_onWrite);

            if (JOIN_LIKELY (handler))
            {
                handler (code, size);
            }

            AsyncOperation::State expected = AsyncOperation::Dispatching;
            _ops->write.state.compare_exchange_strong (expected, AsyncOperation::Idle, std::memory_order_release,
                                                       std::memory_order_relaxed);
        }
    };
}

#endif
