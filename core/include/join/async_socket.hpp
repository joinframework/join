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
#include <join/proactor.hpp>
#include <join/backoff.hpp>
#include <join/memory.hpp>
#include <join/socket.hpp>

// C++.
#include <system_error>
#include <utility>

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
            other.suspendAll ();

            _socket = std::move (other._socket);
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

            other.suspendAll ();

            _socket = std::move (other._socket);

            return *this;
        }

        /**
         * @brief destroy the socket instance.
         */
        ~BasicAsyncSocket () = default;

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
            Backoff backoff;

            do
            {
                cancelAll ();

                backoff ();
            }
            while (!_proactor->isProactorThread () && pendingAny ());

            _socket.close ();
        }

        /**
         * @brief register a provided buffer ring on the proactor driving this socket.
         * @param group buffer group id.
         * @param arena arena owning the buffers, reserved until the ring is unregistered, must outlive the proactor.
         * @return 0 on success, -1 on failure.
         * @throw std::system_error if the descriptor ring cannot be mapped.
         */
        template <size_t Count, size_t Size>
        int registerBufferRing (uint16_t group, LocalMem::Allocator<Count, Size>& arena)
        {
            return _proactor->registerBufferRing (group, arena);
        }

        /**
         * @brief unregister a provided buffer ring from the proactor driving this socket.
         * @param group buffer group id.
         * @return 0 on success, -1 on failure.
         */
        int unregisterBufferRing (uint16_t group)
        {
            return _proactor->unregisterBufferRing (group);
        }

#ifdef JOIN_HAS_IO_URING
        /**
         * @brief register the arena chunks as fixed buffers on the proactor driving this socket.
         * @param arena arena to register.
         * @return 0 on success, -1 on failure.
         */
        template <size_t Count, size_t... Sizes>
        int registerFixedBuffers (LocalMem::Allocator<Count, Sizes...>& arena) noexcept
        {
            return _proactor->registerFixedBuffers (arena);
        }

        /**
         * @brief unregister the fixed buffers from the proactor driving this socket.
         * @return 0 on success, -1 on failure.
         */
        int unregisterFixedBuffers () noexcept
        {
            return _proactor->unregisterFixedBuffers ();
        }
#endif

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
        void onComplete (IoOperation& op, int result) override
        {
            std::error_code code =
                (result < 0) ? std::error_code (-result, std::generic_category ()) : std::error_code ();

            dispatch (op, code, (result > 0) ? static_cast<size_t> (result) : 0);
        }

        /**
         * @brief method called when an operation is cancelled.
         * @param op cancelled operation.
         * @param result negative errno.
         */
        void onCancel (IoOperation& op, [[maybe_unused]] int result) override
        {
            dispatch (op, make_error_code (std::errc::operation_canceled), 0);
        }

        /**
         * @brief invoke the completion handler of the given operation, then release it.
         * @param op completed operation.
         * @param code error code reported by the kernel.
         * @param size number of bytes transferred.
         */
        virtual void dispatch (IoOperation& op, const std::error_code& code, size_t size) noexcept = 0;

        /**
         * @brief suspend every operation in flight.
         */
        virtual void suspendAll () noexcept = 0;

        /**
         * @brief resume every suspended operation, redirecting it to this handler.
         */
        virtual void resumeAll () noexcept = 0;

        /**
         * @brief cancel every operation in flight.
         */
        virtual void cancelAll () noexcept = 0;

        /**
         * @brief check if at least one operation is in flight or completing.
         * @return true if at least one operation is in flight or completing, false otherwise.
         */
        virtual bool pendingAny () const noexcept = 0;

        /**
         * @brief cancel the given operation, if in flight.
         * @param op operation to cancel.
         * @return 0 on success, -1 on failure.
         */
        int cancelOp (IoOperation* op) noexcept
        {
            if ((op == nullptr) || !inFlight (*op))
            {
                return 0;
            }

            if (_proactor->cancel (*op, true, true) == -1)
            {
                return (lastError == Errc::OperationFailed) ? 0 : -1;
            }

            return 0;
        }

        /// proactor driving the operations.
        Proactor* _proactor;

        /// underlying synchronous socket.
        Socket _socket;
    };
}

#endif
