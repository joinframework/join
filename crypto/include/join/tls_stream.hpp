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

#ifndef JOIN_CRYPTO_TLS_STREAM_HPP
#define JOIN_CRYPTO_TLS_STREAM_HPP

// libjoin.
#include <join/socket_stream.hpp>
#include <join/tls_wrapper.hpp>

// C++.
#include <utility>
#include <string>

namespace join
{
    /**
     * @brief TLS stream class.
     */
    template <class Protocol>
    class BasicTlsStream : public BasicSocketStream<Protocol>
    {
    public:
        using Endpoint = typename Protocol::Endpoint;
        using Socket = typename Protocol::Socket;
        using Transport = typename Protocol::Transport::Socket;
        using Mode = typename Socket::Mode;

        /**
         * @brief default constructor.
         */
        BasicTlsStream ()
        : BasicTlsStream (TlsContext{})
        {
        }

        /**
         * @brief construct the TLS stream instance using the given context.
         * @param ctx TLS context to use.
         * @param mode socket blocking mode.
         */
        explicit BasicTlsStream (TlsContext ctx, Mode mode = Mode::NonBlocking)
        : BasicSocketStream<Protocol> (Socket{std::move (ctx), mode})
        {
        }

        /**
         * @brief construct the TLS stream by moving an already wrapped TLS socket in.
         * @param socket TLS socket to move in.
         */
        explicit BasicTlsStream (Socket&& socket)
        : BasicSocketStream<Protocol> (std::move (socket))
        {
        }

        /**
         * @brief construct the TLS stream by wrapping an already connected transport socket.
         * @param socket connected transport socket to move in (e.g. as returned by accept ()).
         * @param ctx TLS context to use for the encryption layer.
         */
        explicit BasicTlsStream (Transport&& socket, TlsContext ctx = TlsContext{})
        : BasicSocketStream<Protocol> (Socket{std::move (socket), std::move (ctx)})
        {
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicTlsStream (const BasicTlsStream& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        BasicTlsStream& operator= (const BasicTlsStream& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        BasicTlsStream (BasicTlsStream&& other)
        : BasicSocketStream<Protocol> (std::move (other))
        {
        }

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        BasicTlsStream& operator= (BasicTlsStream&& other)
        {
            BasicSocketStream<Protocol>::operator= (std::move (other));

            return *this;
        }

        /**
         * @brief destroy the TLS stream instance.
         */
        virtual ~BasicTlsStream () = default;

        /**
         * @brief perform the TLS handshake.
         */
        void handshake ()
        {
            if (this->_sockbuf.socket ().handshake () == -1)
            {
                if (lastError == Errc::TemporaryError)
                {
                    if (this->_sockbuf.socket ().waitHandshake (this->timeout ()))
                    {
                        return;
                    }
                }

                this->setstate (std::ios_base::failbit);
            }
        }

        /**
         * @brief perform the TLS shutdown (send a close_notify alert).
         */
        void shutdown ()
        {
            if (this->_sockbuf.socket ().shutdown () == -1)
            {
                if (lastError == Errc::TemporaryError)
                {
                    if (this->_sockbuf.socket ().waitShutdown (this->timeout ()))
                    {
                        return;
                    }
                }

                this->setstate (std::ios_base::failbit);
            }
        }

        /**
         * @brief check if the stream is encrypted.
         * @return true if the stream is encrypted, false otherwise.
         */
        bool encrypted ()
        {
            return this->_sockbuf.socket ().encrypted ();
        }
    };
}

#endif
