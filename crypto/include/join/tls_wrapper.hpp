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

#ifndef JOIN_CRYPTO_TLS_WRAPPER_HPP
#define JOIN_CRYPTO_TLS_WRAPPER_HPP

// libjoin.
#include <join/tls.hpp>

namespace join
{
    /**
     * @brief TLS decorator for stream sockets.
     */
    template <class Protocol>
    class BasicTlsWrapper : public BasicTls<Protocol>
    {
    public:
        using Endpoint = typename BasicTls<Protocol>::Endpoint;
        using TimePoint = typename BasicTls<Protocol>::TimePoint;

        /// inherit the base constructors.
        using BasicTls<Protocol>::BasicTls;

        /**
         * @brief check if the underlying socket is connecting.
         * @return true if connecting.
         */
        bool connecting () const noexcept
        {
            return this->_socket.connecting ();
        }

        /**
         * @brief block until the underlying socket is connected.
         * @return true if connected, false otherwise.
         */
        bool waitConnected ()
        {
            return this->_socket.waitConnected ();
        }

        /**
         * @brief block until the underlying socket is connected, giving up after the given duration.
         * @param timeout maximum time to wait.
         * @return true if connected, false otherwise.
         */
        bool waitConnected (std::chrono::nanoseconds timeout)
        {
            return this->_socket.waitConnected (timeout);
        }

        /**
         * @brief block until the underlying socket is connected, giving up at the given time point.
         * @param deadline time point at which to give up, max to wait indefinitely.
         * @return true if connected, false otherwise.
         */
        bool waitConnected (TimePoint deadline)
        {
            return this->_socket.waitConnected (deadline);
        }

        /**
         * @brief block until the underlying socket is disconnected.
         * @return true if disconnected, false otherwise.
         */
        bool waitDisconnected ()
        {
            return this->_socket.waitDisconnected ();
        }

        /**
         * @brief block until the underlying socket is disconnected, giving up after the given duration.
         * @param timeout maximum time to wait.
         * @return true if disconnected, false otherwise.
         */
        bool waitDisconnected (std::chrono::nanoseconds timeout)
        {
            return this->_socket.waitDisconnected (timeout);
        }

        /**
         * @brief block until the underlying socket is disconnected, giving up at the given time point.
         * @param deadline time point at which to give up, max to wait indefinitely.
         * @return true if disconnected, false otherwise.
         */
        bool waitDisconnected (TimePoint deadline)
        {
            return this->_socket.waitDisconnected (deadline);
        }

        /// keep the base overloads visible, the override below hides them.
        using BasicTls<Protocol>::waitHandshake;

        /**
         * @brief block until TLS handshake is finished, giving up at the given time point.
         * @param deadline time point at which to give up, max to wait indefinitely.
         * @return true if TLS handshake is finished.
         * @note waits for the transport connection first, then runs the common handshake.
         */
        bool waitHandshake (TimePoint deadline) override
        {
            if (!this->_socket.waitConnected (deadline))
            {
                return false;
            }

            return BasicTls<Protocol>::waitHandshake (deadline);
        }

        /**
         * @brief get the number of readable bytes.
         * @return the number of readable bytes, -1 on failure.
         * @note once TLS is armed, reports the decrypted bytes buffered by OpenSSL.
         */
        int canRead () const noexcept
        {
            if (this->_ssl)
            {
                return SSL_pending (this->_ssl.get ());
            }

            return this->_socket.canRead ();
        }
    };

    /**
     * @brief compare two TLS decorators based on their underlying socket handle.
     * @param a first TLS decorator.
     * @param b second TLS decorator.
     * @return true if the handle of a is less than the handle of b, false otherwise.
     */
    template <class Protocol>
    inline bool operator< (const BasicTlsWrapper<Protocol>& a, const BasicTlsWrapper<Protocol>& b) noexcept
    {
        return a.handle () < b.handle ();
    }
}

#endif
