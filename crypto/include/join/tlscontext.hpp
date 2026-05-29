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

#ifndef JOIN_CORE_TLS_CONTEXT_HPP
#define JOIN_CORE_TLS_CONTEXT_HPP

// libjoin.
// #include <join/tlserror.hpp>
#include <join/openssl.hpp>

namespace join
{
    /**
     * @brief TLS/DTLS context.
     */
    class TlsContext
    {
    public:
        /**
         * @brief TLS/DTLS role.
         */
        enum class Role
        {
            TlsClient,  /**< TLS client. */
            TlsServer,  /**< TLS server. */
            DtlsClient, /**< DTLS client. */
            DtlsServer, /**< DTLS server. */
        };

        /**
         * @brief create TLS/DTLS context for the given role.
         * @param role client or server, TLS or DTLS.
         * @throw std::runtime_error if SSL_CTX_new fails.
         */
        explicit TlsContext (Role role = Role::TlsClient);

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        TlsContext (const TlsContext&) = default;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        TlsContext& operator= (const TlsContext&) = default;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        TlsContext (TlsContext&&) = default;

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        TlsContext& operator= (TlsContext&&) = default;

        /**
         * @brief destroy instance.
         */
        ~TlsContext () = default;

        /**
         * @brief set the certificate and the private key.
         * @param cert certificate path.
         * @param key private key path.
         * @return 0 on success, -1 on failure.
         */
        int setCertificate (const std::string& cert, const std::string& key = "") noexcept;

        /**
         * @brief set the location of the trusted CA certificates.
         * @param caPath path of the trusted CA certificates.
         * @return 0 on success, -1 on failure.
         */
        int setCaPath (const std::string& caPath);

        /**
         * @brief set the location of the trusted CA certificate file.
         * @param caFile path of the trusted CA certificate file.
         * @return 0 on success, -1 on failure.
         */
        int setCaFile (const std::string& caFile);

        /**
         * @brief enable or disable peer certificate verification.
         * @param verify true to enable peer verification.
         * @param depth maximum certificate chain depth (-1 = no limit).
         */
        void setVerify (bool verify, int depth = -1) noexcept;

        /**
         * @brief set the cipher list (TLSv1.2 and below).
         * @param cipher the cipher list.
         * @return 0 on success, -1 on failure.
         */
        int setCipher (const std::string& cipher) noexcept;

        /**
         * @brief set the cipher list (TLSv1.3).
         * @param cipher the cipher list.
         * @return 0 on success, -1 on failure.
         */
        int setCipher_1_3 (const std::string& cipher) noexcept;

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
        /**
         * @brief set elliptic curve list (OpenSSL 3+).
         * @param curves curve list string.
         * @return 0 on success, -1 on failure.
         */
        int setCurve (const std::string& curves);
#endif

        /**
         * @brief set the ALPN protocols list.
         * @param protocols list of protocol names (e.g. {"h2", "http/1.1"}).
         * @return 0 on success, -1 on failure.
         */
        int setAlpnProtocols (const std::vector<std::string>& protocols);

        /**
         * @brief get the native SSL_CTX handle.
         * @return SSL_CTX*.
         */
        SSL_CTX* handle () const noexcept;

        /**
         * @brief check if peer verification is enabled.
         * @return true if enabled.
         */
        bool verify () const noexcept;

        /**
         * @brief get the maximum certificate chain depth.
         * @return depth.
         */
        int depth () const noexcept;

        /**
         * @brief check if the role is a server role.
         * @return true if the role is a server role, false otherwise.
         */
        bool isServer () const noexcept;

    private:
        /**
         * @brief get the SSL_METHOD for the given role.
         * @param role client or server, TLS or DTLS.
         * @return the SSL_METHOD for the given role.
         */
        static const SSL_METHOD* method (Role role) noexcept;

#if OPENSSL_VERSION_NUMBER < 0x30000000L
        /**
         * @brief generate openssl Diffie-Hellman parameters.
         * @note random Diffie-Hellman parameters generated using the command "openssl dhparam -C 2236".
         * @return Diffie-Hellman parameters.
         */
        static DH* getDh2236 ();
#endif

        /// TLS role.
        Role _role;

        /// peer verification enabled.
        bool _verify = false;

        /// maximum certificate chain depth.
        int _depth = -1;

        /// OpenSSL context.
        SslCtxPtr _ctx;
    };
}

#endif
