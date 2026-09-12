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
    public:
        using Socket = BasicStreamSocket<Protocol>;
        using Endpoint = typename Protocol::Endpoint;
        using AsyncConnect = BasicAsyncConnect<Protocol, Proactor>;
        using ConnectHandler = typename AsyncConnect::Handler;

        /**
         * @brief create the socket instance.
         * @param proactor proactor driving the operations.
         */
        explicit BasicAsyncStreamSocket (Proactor& proactor = ProactorThread::proactor ())
        : BasicAsyncSocket<Protocol, Proactor> (proactor)
        {
        }

        /**
         * @brief create the socket instance adopting an already connected socket.
         * @param sock socket to adopt.
         * @param proactor proactor driving the operations.
         */
        explicit BasicAsyncStreamSocket (Socket&& sock, Proactor& proactor = ProactorThread::proactor ())
        : BasicAsyncSocket<Protocol, Proactor> (std::move (sock), proactor)
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
        BasicAsyncStreamSocket (BasicAsyncStreamSocket&& other) noexcept = default;

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicAsyncStreamSocket& operator= (BasicAsyncStreamSocket&& other) noexcept = default;

        /**
         * @brief start an asynchronous connection to the given endpoint.
         * @param endpoint endpoint to connect to.
         * @param handler handler invoked on completion.
         * @return 0 on success, -1 on failure.
         */
        int asyncConnect (const Endpoint& endpoint, ConnectHandler handler) noexcept
        {
            if (JOIN_UNLIKELY (!this->_writeArena.hasBackend ()))
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

            AsyncConnect* connect = this->allocateConnect ();
            if (JOIN_UNLIKELY (connect == nullptr))
            {
                lastError = make_error_code (Errc::InUse);
                return -1;
            }

            this->_socket._state = Socket::Connecting;
            this->_socket._remote = endpoint;
            connect->handler = std::move (handler);
            connect->op = IoOperation::makeConnect (this->_socket.handle (), this->_socket._remote.addr (),
                                                    this->_socket._remote.length (), this);
            connect->op.state.store (IoOperation::State::Submitted, std::memory_order_release);

            if (this->_proactor->submit (&connect->op, true, false) == -1)
            {
                // LCOV_EXCL_START
                this->releaseConnect (connect);
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
    };
}

#endif
