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

#ifndef JOIN_CORE_ASYNC_OPERATION_HPP
#define JOIN_CORE_ASYNC_OPERATION_HPP

// libjoin.
#include <join/io_operation.hpp>
#include <join/function.hpp>
#include <join/proactor.hpp>
#include <join/backoff.hpp>
#include <join/socket.hpp>
#include <join/error.hpp>
#include <join/utils.hpp>

// C++.
#include <system_error>
#include <utility>
#include <atomic>
#include <new>

// C.
#include <cstdlib>
#include <cstddef>
#include <cstdint>

namespace join
{
    /**
     * @brief basic asynchronous operation class.
     */
    template <class Protocol>
    class BasicAsyncOperation : public CompletionHandler
    {
        /// friendship with basic asynchronous socket
        template <class P, class E>
        friend class BasicAsyncSocket;

        /// friendship with basic asynchronous stream socket
        template <class P, class E>
        friend class BasicAsyncStreamSocket;

        /// friendship with basic asynchronous datagram socket
        template <class P, class E>
        friend class BasicAsyncDatagramSocket;

        /// friendship with basic asynchronous stream acceptor
        template <class P, class E>
        friend class BasicAsyncStreamAcceptor;

    public:
        using Endpoint = typename Protocol::Endpoint;

        /**
         * @brief caller side operation state.
         */
        enum State : uint8_t
        {
            Idle,        /**< no operation in flight and no completion handler running. */
            Pending,     /**< an operation is in flight. */
            Dispatching, /**< the completion handler is running. */
            Closing,     /**< the socket is closing, no operation may be reserved. */
        };

        /**
         * @brief allocate an operation honouring its extended alignment.
         * @param size allocation size in bytes.
         * @return pointer to the allocated storage.
         */
        static void* operator new (size_t size)
        {
            void* mem = ::aligned_alloc (alignof (BasicAsyncOperation), size);

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

        /**
         * @brief reserve the operation slot for a new operation.
         * @param engine engine driving the operation.
         * @return 0 on success, -1 on failure.
         */
        template <class Engine>
        int reserve (Engine& engine) noexcept
        {
            State expected = Idle;

            if (!state.compare_exchange_strong (expected, Pending, std::memory_order_acquire,
                                                std::memory_order_acquire))
            {
                if ((expected != Dispatching) || !engine.isProactorThread ())
                {
                    lastError = make_error_code (Errc::InUse);
                    return -1;
                }

                state.store (Pending, std::memory_order_release);
            }

            return 0;
        }

        /**
         * @brief release the operation slot.
         */
        void release () noexcept
        {
            state.store (Idle, std::memory_order_release);
        }

        /**
         * @brief wait for the operation in flight to end, then retain the slot.
         * @param cancel callable cancelling the operation in flight.
         */
        template <class Cancel>
        void drain (Cancel cancel) noexcept
        {
            Backoff backoff;
            State expected = Idle;

            while (!state.compare_exchange_strong (expected, Closing, std::memory_order_acq_rel,
                                                   std::memory_order_acquire))
            {
                if (expected == Pending)
                {
                    cancel ();
                }

                backoff ();
                expected = Idle;
            }
        }

        /**
         * @brief run the completion handler, publishing the dispatch state around it.
         * @param invoke callable running the completion handler.
         */
        template <class Fn>
        void dispatch (Fn&& invoke) noexcept
        {
            state.store (Dispatching, std::memory_order_release);

            invoke ();

            State expected = Dispatching;
            state.compare_exchange_strong (expected, Idle, std::memory_order_release, std::memory_order_relaxed);
        }

    protected:
        /// operation.
        IoOperation op = {};

        /// caller side operation state.
        alignas (64) std::atomic<State> state{Idle};
    };

    /**
     * @brief basic asynchronous read operation class.
     */
    template <class Protocol>
    class BasicAsyncRead : public BasicAsyncOperation<Protocol>
    {
        /// friendship with basic asynchronous socket
        template <class P, class E>
        friend class BasicAsyncSocket;

        /// friendship with basic asynchronous stream socket
        template <class P, class E>
        friend class BasicAsyncStreamSocket;

        /// friendship with basic asynchronous datagram socket
        template <class P, class E>
        friend class BasicAsyncDatagramSocket;

    public:
        /// handler invoked on read completion.
        using Handler = Function<void (const std::error_code&, size_t)>;

    protected:
        /**
         * @brief method called when the read completes.
         * @param op completed operation.
         * @param result number of bytes received, or negative errno.
         */
        void onComplete ([[maybe_unused]] IoOperation* op, int result) override
        {
            complete ((result < 0) ? std::error_code (-result, std::generic_category ()) : std::error_code (),
                      (result > 0) ? static_cast<size_t> (result) : 0);
        }

        /**
         * @brief method called when the read is cancelled.
         * @param op cancelled operation.
         * @param result negative errno.
         */
        void onCancel ([[maybe_unused]] IoOperation* op, [[maybe_unused]] int result) override
        {
            complete (make_error_code (std::errc::operation_canceled), 0);
        }

        /**
         * @brief invoke the completion handler.
         * @param code error code reported by the kernel.
         * @param size number of bytes received.
         */
        void complete (const std::error_code& code, size_t size) noexcept
        {
            this->dispatch ([this, &code, size] () {
                Handler h = std::move (handler);
                std::error_code result = code;

                if (stream)
                {
                    if (JOIN_UNLIKELY (!result && (size == 0)))
                    {
                        result = make_error_code (Errc::ConnectionClosed);
                    }
                }
                else if (JOIN_UNLIKELY (!result && (msg.msg_flags & MSG_TRUNC)))
                {
                    result = make_error_code (Errc::MessageTooLong);
                }

                if (JOIN_LIKELY (h))
                {
                    h (result, size);
                }
            });
        }

        /// handler invoked on completion.
        Handler handler;

        /// message header, written by the kernel until the read completes.
        msghdr msg = {};

        /// scatter gather entry, written by the kernel until the read completes.
        iovec iov = {};

        /// report an empty read as a closed connection.
        bool stream = false;
    };

    /**
     * @brief basic asynchronous write operation class.
     */
    template <class Protocol>
    class BasicAsyncWrite : public BasicAsyncOperation<Protocol>
    {
        /// friendship with basic asynchronous socket
        template <class P, class E>
        friend class BasicAsyncSocket;

        /// friendship with basic asynchronous stream socket
        template <class P, class E>
        friend class BasicAsyncStreamSocket;

        /// friendship with basic asynchronous datagram socket
        template <class P, class E>
        friend class BasicAsyncDatagramSocket;

    public:
        using Endpoint = typename BasicAsyncOperation<Protocol>::Endpoint;
        using Socket = BasicSocket<Protocol>;

        /// handler invoked on write completion.
        using Handler = Function<void (const std::error_code&, size_t)>;

        /// handler invoked on connect completion.
        using ConnectHandler = Function<void (const std::error_code&)>;

    protected:
        /**
         * @brief method called when the write completes.
         * @param op completed operation.
         * @param result number of bytes sent, or negative errno.
         */
        void onComplete (IoOperation* op, int result) override
        {
            complete (op, (result < 0) ? std::error_code (-result, std::generic_category ()) : std::error_code (),
                      (result > 0) ? static_cast<size_t> (result) : 0);
        }

        /**
         * @brief method called when the write is cancelled.
         * @param op cancelled operation.
         * @param result negative errno.
         */
        void onCancel (IoOperation* op, [[maybe_unused]] int result) override
        {
            complete (op, make_error_code (std::errc::operation_canceled), 0);
        }

        /**
         * @brief invoke the completion handler.
         * @param op completed or cancelled operation.
         * @param code error code to report.
         * @param size number of bytes sent.
         */
        void complete (IoOperation* op, const std::error_code& code, size_t size) noexcept
        {
            if (JOIN_UNLIKELY (op->code == static_cast<uint8_t> (IoOperation::Opcode::Connect)))
            {
                this->dispatch ([this, &code] () {
                    ConnectHandler h = std::move (connectHandler);

                    if (code)
                    {
                        socket->close ();
                    }
                    else
                    {
                        socket->_state = Socket::Connected;
                    }

                    if (JOIN_LIKELY (h))
                    {
                        h (code);
                    }
                });

                return;
            }

            this->dispatch ([this, &code, size] () {
                Handler h = std::move (handler);

                if (JOIN_LIKELY (h))
                {
                    h (code, size);
                }
            });
        }

        /// handler invoked on completion.
        Handler handler;

        /// handler invoked on connect completion.
        ConnectHandler connectHandler;

        /// message header, read by the kernel until the write completes.
        msghdr msg = {};

        /// scatter gather entry, read by the kernel until the write completes.
        iovec iov = {};

        /// remote endpoint, read by the kernel until the operation completes.
        Endpoint remote;

        /// socket owning this operation slot.
        Socket* socket = nullptr;
    };
}

#endif
