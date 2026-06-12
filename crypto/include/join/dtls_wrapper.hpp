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

#ifndef JOIN_CRYPTO_DTLS_WRAPPER_HPP
#define JOIN_CRYPTO_DTLS_WRAPPER_HPP

// libjoin.
#include <join/tls.hpp>

namespace join
{
    /**
     * @brief DTLS decorator for datagram sockets.
     */
    template <class Protocol>
    class BasicDtlsWrapper : public BasicTls<Protocol>
    {
    public:
        using Endpoint = typename BasicTls<Protocol>::Endpoint;

        /// inherit the base constructors.
        using BasicTls<Protocol>::BasicTls;

        /**
         * @brief read data on the socket.
         * @param buf buffer used to store the data received.
         * @param len maximum number of bytes to read.
         * @param endpoint endpoint from where data are coming (optional).
         * @return The number of bytes received, -1 on failure.
         */
        int readFrom (char* buf, unsigned long len, Endpoint* endpoint = nullptr) noexcept
        {
            if (this->encrypted ())
            {
                int nread = SSL_read (this->_ssl.get (), buf, static_cast<int> (len));
                if (nread < 1)
                {
                    return this->handleTlsError (nread);
                }

                if (endpoint != nullptr)
                {
                    BIO* rbio = SSL_get_rbio (this->_ssl.get ());
                    if (!rbio)
                    {
                        lastError = make_error_code (Errc::OperationFailed);
                        return -1;
                    }

                    struct sockaddr_storage sa;
                    socklen_t sa_len = sizeof (struct sockaddr_storage);
                    if (BIO_dgram_get_peer (rbio, &sa) <= 0)
                    {
                        lastError = make_error_code (Errc::OperationFailed);
                        return -1;
                    }

                    *endpoint = Endpoint (reinterpret_cast<struct sockaddr*> (&sa), sa_len);
                }

                return nread;
            }

            return this->_socket.readFrom (buf, len, endpoint);
        }

        /**
         * @brief write data on the socket.
         * @param buf buffer to write from.
         * @param len number of bytes to write.
         * @param endpoint endpoint where to write the data.
         * @return the number of bytes written, -1 on failure.
         */
        int writeTo (const char* buf, unsigned long len, const Endpoint& endpoint) noexcept
        {
            if (this->encrypted ())
            {
                BIO* wbio = SSL_get_wbio (this->_ssl.get ());
                if (!wbio)
                {
                    lastError = make_error_code (Errc::OperationFailed);
                    return -1;
                }

                struct sockaddr_storage sa;
                socklen_t sa_len = sizeof (struct sockaddr_storage);
                if (BIO_dgram_get_peer (wbio, &sa) <= 0)
                {
                    lastError = make_error_code (Errc::OperationFailed);
                    return -1;
                }

                Endpoint remote (reinterpret_cast<struct sockaddr*> (&sa), sa_len);
                if (remote != endpoint)
                {
                    lastError = make_error_code (Errc::InvalidParam);
                    return -1;
                }

                int nwritten = SSL_write (this->_ssl.get (), buf, static_cast<int> (len));
                if (nwritten < 1)
                {
                    return this->handleTlsError (nwritten);
                }

                return nwritten;
            }

            return this->_socket.writeTo (buf, len, endpoint);
        }

        /**
         * @brief returns the Time-To-Live value.
         * @return The Time-To-Live value.
         */
        int ttl () const noexcept
        {
            return this->_socket.ttl ();
        }
    };

    /**
     * @brief compare two DTLS decorators based on their underlying socket handle.
     * @param a first DTLS decorator.
     * @param b second DTLS decorator.
     * @return true if the handle of a is less than the handle of b, false otherwise.
     */
    template <class Protocol>
    inline bool operator< (const BasicDtlsWrapper<Protocol>& a, const BasicDtlsWrapper<Protocol>& b) noexcept
    {
        return a.handle () < b.handle ();
    }
}

#endif
