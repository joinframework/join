/**
 * MIT License
 *
 * Copyright (c) 2021 Mathieu Rabine
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

#ifndef __JOIN_ACCEPTOR_HPP__
#define __JOIN_ACCEPTOR_HPP__

// libjoin.
#include <join/socketstream.hpp>

namespace join
{
    /**
     * @brief basic acceptor class.
     */
    template <class Protocol>
    class BasicAcceptor
    {
    public:
        using Endpoint = typename Protocol::Endpoint;
        using Socket   = typename Protocol::Socket;
        using Stream   = typename Protocol::Stream;

        /**
         * @brief create the acceptor instance.
         */
        BasicAcceptor () = default;

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicAcceptor (const BasicAcceptor& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicAcceptor& operator= (const BasicAcceptor& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        BasicAcceptor (BasicAcceptor&& other)
        : _handle (other._handle),
          _protocol (other._protocol)
        {
            other._handle = -1;
            other._protocol = Protocol ();
        }

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicAcceptor& operator= (BasicAcceptor&& other)
        {
            this->close ();

            this->_handle = other._handle;
            this->_protocol = other._protocol;

            other._handle = -1;
            other._protocol = Protocol ();

            return *this;
        }

        /**
         * @brief destroy instance.
         */
        virtual ~BasicAcceptor ()
        {
            if (this->_handle != -1)
            {
                ::close (this->_handle);
            }
        }

        /**
         * @brief open acceptor.
         * @param protocol protocol to use.
         * @return 0 on success, -1 on failure.
         */
        virtual int open (const Protocol& protocol = Protocol ()) noexcept
        {
            if (this->opened ())
            {
                lastError = make_error_code (Errc::InUse);
                return -1;
            }

            this->_handle = ::socket (protocol.family (), protocol.type () | SOCK_CLOEXEC, protocol.protocol ());
            if (this->_handle == -1)
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                this->close ();
                return -1;
            }

            if (protocol.family () == AF_INET6)
            {
                int off = 0;

                if (::setsockopt (this->_handle, IPPROTO_IPV6, IPV6_V6ONLY, &off, sizeof (off)) == -1)
                {
                    lastError = std::make_error_code (static_cast <std::errc> (errno));
                    this->close ();
                    return -1;
                }
            }

            this->_protocol = protocol;

            return 0;
        }

        /**
         * @brief close acceptor.
         */
        virtual void close () noexcept
        {
            if (this->_handle != -1)
            {
                ::close (this->_handle);
                this->_handle = -1;
            }
        }

        /**
         * @brief assigns the specified endpoint to the acceptor.
         * @param endpoint endpoint to assign to the acceptor.
         * @return 0 on success, -1 on failure.
         */
        virtual int bind (const Endpoint& endpoint) noexcept
        {
            if (!this->opened () && this->open (endpoint.protocol ()) == -1)
            {
                return -1;
            }

            if (endpoint.protocol ().family () == AF_UNIX)
            {
                ::unlink (endpoint.device ().c_str ());
            }
            else if (endpoint.protocol ().protocol () == IPPROTO_TCP)
            {
                int on = 1;

                if (::setsockopt (this->_handle, SOL_SOCKET, SO_REUSEADDR, &on, sizeof (on)) == -1)
                {
                    lastError = std::make_error_code (static_cast <std::errc> (errno));
                    this->close ();
                    return -1;
                }
            }

            if (::bind (this->_handle, endpoint.addr (), endpoint.length ()) == -1)
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                this->close ();
                return -1;
            }

            return 0;
        }

        /**
         * @brief marks this acceptor as ready to accept connections.
         * @param max max listen connections.
         * @return 0 on success, -1 on failure.
         */
        virtual int listen (size_t max = SOMAXCONN) noexcept
        {
            if (::listen (this->_handle, max) == -1)
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                this->close ();
                return -1;
            }

            return 0;
        }

        /**
         * @brief determine the local endpoint associated with this socket.
         * @return local endpoint.
         */
        Endpoint localEndpoint () const
        {
            Endpoint endpoint;
            socklen_t addrLen = endpoint.length ();

            if (::getsockname (this->_handle, endpoint.addr (), &addrLen) == -1)
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                return {};
            }

            return endpoint;
        }

        /**
         * @brief check if the socket is opened.
         * @return true if opened, false otherwise.
         */
        bool opened () const noexcept
        {
            return (this->_handle != -1);
        }

        /**
         * @brief get address family.
         * @return address family.
         */
        int family () const noexcept
        {
            return this->_protocol.family ();
        }

        /**
         * @brief get the protocol communication semantic.
         * @return the protocol communication semantic.
         */
        int type () const noexcept
        {
            return this->_protocol.type ();
        }

        /**
         * @brief get acceptor protocol.
         * @return acceptor protocol.
         */
        int protocol () const noexcept
        {
            return this->_protocol.protocol ();
        }

        /**
         * @brief get socket native handle.
         * @return socket native handle.
         */
        int handle () const noexcept
        {
            return this->_handle;
        }

    protected:
        /// socket handle.
        int _handle = -1;

        /// protocol.
        Protocol _protocol;
    };

    /**
     * @brief basic stream acceptor class.
     */
    template <class Protocol>
    class BasicStreamAcceptor : public BasicAcceptor <Protocol>
    {
    public:
        using Endpoint = typename Protocol::Endpoint;
        using Socket   = typename Protocol::Socket;
        using Stream   = typename Protocol::Stream;

        /**
         * @brief create the acceptor instance.
         */
        BasicStreamAcceptor () = default;

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicStreamAcceptor (const BasicStreamAcceptor& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicStreamAcceptor& operator= (const BasicStreamAcceptor& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        BasicStreamAcceptor (BasicStreamAcceptor&& other) = default;

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicStreamAcceptor& operator= (BasicStreamAcceptor&& other) = default;

        /**
         * @brief destroy instance.
         */
        virtual ~BasicStreamAcceptor () = default;

        /**
         * @brief accept new connection and fill in the client object with connection parameters.
         * @note the client socket object is allocated and must be released.
         * @return The client socket object on success, nullptr on failure.
         */
        virtual Socket accept () const
        {
            Endpoint endpoint;
            socklen_t addrLen = endpoint.length ();
            Socket client;

            client._handle = ::accept (this->_handle, endpoint.addr (), &addrLen);
            if (client._handle == -1)
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                return {};
            }

            client._state = Socket::Connected;

            if (client.setMode (Socket::NonBlocking) == -1)
            {
                client.close ();
                return {};
            }

            if (client.protocol () == IPPROTO_TCP && client.setOption (Socket::NoDelay, 1) == -1)
            {
                client.close ();
                return {};
            }

            return client;
        }
    };

    /**
     * @brief basic TLS acceptor class.
     */
    template <class Protocol>
    class BasicTlsAcceptor : public BasicAcceptor <Protocol>
    {
    public:
        using Endpoint = typename Protocol::Endpoint;
        using Socket   = typename Protocol::Socket;
        using Stream   = typename Protocol::Stream;

        /**
         * @brief create the acceptor instance.
         */
        BasicTlsAcceptor ()
        : BasicAcceptor <Protocol> (),
        #if OPENSSL_VERSION_NUMBER < 0x10100000L
          _tlsContext (SSL_CTX_new (SSLv23_method ()), join::crypto::SslCtxDelete ()),
        #else
          _tlsContext (SSL_CTX_new (TLS_method ()), join::crypto::SslCtxDelete ()),
        #endif
          _sessionId (randomize <int> ())
        {
            if (_tlsContext == nullptr)
            {
                throw std::runtime_error ("OpenSSL libraries were not initialized at process start");
            }

            // enable the OpenSSL bug workaround options.
            SSL_CTX_set_options (_tlsContext.get (), SSL_OP_ALL);

        #if OPENSSL_VERSION_NUMBER >= 0x10100000L
            // disallow compression.
            SSL_CTX_set_options (_tlsContext.get (), SSL_OP_NO_COMPRESSION);
        #endif

            // disallow usage of SSLv2, SSLv3, TLSv1 and TLSv1.1 which are considered insecure.
            SSL_CTX_set_options (_tlsContext.get (), SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1);

            // choose the cipher according to the server's preferences.
            SSL_CTX_set_options (_tlsContext.get (), SSL_OP_CIPHER_SERVER_PREFERENCE);

            // setup write mode.
            SSL_CTX_set_mode (_tlsContext.get (), SSL_MODE_ENABLE_PARTIAL_WRITE | SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);

            // automatically renegotiates.
            SSL_CTX_set_mode (_tlsContext.get (), SSL_MODE_AUTO_RETRY);

            // enable SSL session caching.
            SSL_CTX_set_session_id_context (_tlsContext.get (), reinterpret_cast <const unsigned char *> (&_sessionId), sizeof (_sessionId));

            // no verification by default.
            SSL_CTX_set_verify (_tlsContext.get (), SSL_VERIFY_NONE, nullptr);

            // set default TLSv1.2 and below cipher suites.
            SSL_CTX_set_cipher_list (_tlsContext.get (), join::crypto::defaultCipher_.c_str ());

        #if OPENSSL_VERSION_NUMBER >= 0x10101000L
            //  set default TLSv1.3 cipher suites.
            SSL_CTX_set_ciphersuites (_tlsContext.get (), join::crypto::defaultCipher_1_3_.c_str ());

            // disallow client-side renegotiation.
            SSL_CTX_set_options (_tlsContext.get (), SSL_OP_NO_RENEGOTIATION);
        #endif

        #if OPENSSL_VERSION_NUMBER >= 0x30000000L
            // use the default built-in Diffie-Hellman parameters.
            SSL_CTX_set_dh_auto (_tlsContext.get (), 1);

            // Set elliptic curve Diffie-Hellman key.
            SSL_CTX_set1_groups_list (_tlsContext.get (), join::crypto::defaultCurve_.c_str ());
        #else
            // Set Diffie-Hellman key.
            join::crypto::DhKeyPtr dh (getDh2236 ());
            if (dh)
            {
                SSL_CTX_set_tmp_dh (_tlsContext.get (), dh.get ());
            }

            // Set elliptic curve Diffie-Hellman key.
            join::crypto::EcdhKeyPtr ecdh (EC_KEY_new_by_curve_name (NID_X9_62_prime256v1));
            if (ecdh)
            {
                SSL_CTX_set_tmp_ecdh (_tlsContext.get (), ecdh.get ());
            }
        #endif
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicTlsAcceptor (const BasicTlsAcceptor& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicTlsAcceptor& operator= (const BasicTlsAcceptor& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        BasicTlsAcceptor (BasicTlsAcceptor&& other)
        : BasicAcceptor <Protocol> (std::move (other)),
          _tlsContext (std::move (other._tlsContext)),
          _sessionId (other._sessionId)
        {
            other._sessionId = 0;
        }

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicTlsAcceptor& operator= (BasicTlsAcceptor&& other)
        {
            BasicAcceptor <Protocol>::operator= (std::move (other));

            _tlsContext = std::move (other._tlsContext);
            _sessionId = other._sessionId;

            other._sessionId = 0;

            return *this;
        }

        /**
         * @brief accept new connection and fill in the client object with connection parameters.
         * @note the client socket object is allocated and must be released.
         * @return The client socket object on success, nullptr on failure.
         */
        virtual Socket accept () const
        {
            Endpoint endpoint;
            socklen_t addrLen = endpoint.length ();
            Socket client (this->_tlsContext, Socket::ServerMode);

            // accept connection.
            client._handle = ::accept (this->_handle, endpoint.addr (), &addrLen);
            if (client._handle == -1)
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                return {};
            }

            client._state = Socket::Connected;

            // set client socket mode.
            if (client.setMode (Socket::NonBlocking) == -1)
            {
                client.close ();
                return {};
            }

            // set the no delay option.
            if (client.setOption (Socket::NoDelay, 1) == -1)
            {
                client.close ();
                return {};
            }

            //  create an SSL object for the connection.
            client._tlsHandle.reset (SSL_new (client._tlsContext.get ()));
            if (client._tlsHandle == nullptr)
            {
                lastError = make_error_code (Errc::OutOfMemory);
                client.close ();
                return {};
            }

            // set the file descriptor as the input/output facility for the TLS/SSL handle.
            if (SSL_set_fd (client._tlsHandle.get (), client._handle) == 0)
            {
                lastError = make_error_code (Errc::InvalidParam);
                client.close ();
                return {};
            }

            // prepare the object to work in server mode.
            SSL_set_accept_state (client._tlsHandle.get ());

            // save the internal context.
            SSL_set_app_data (client._tlsHandle.get (), &client);

        #ifdef DEBUG
            // set info callback.
            SSL_set_info_callback (client._tlsHandle.get (), Socket::infoWrapper);
        #endif

            client._tlsState = Socket::Encrypted;

            return client;
        }

        /**
         * @brief set the certificate and the private key.
         * @param cert certificate path.
         * @param key private key path.
         * @return 0 on success, -1 on failure.
         */
        int setCertificate (const std::string& cert, const std::string& key = "")
        {
            if (SSL_CTX_use_certificate_file (this->_tlsContext.get (), cert.c_str (), SSL_FILETYPE_PEM) == 0)
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            if (key.size ())
            {
                if (SSL_CTX_use_PrivateKey_file (this->_tlsContext.get (), key.c_str(), SSL_FILETYPE_PEM) == 0)
                {
                    lastError = make_error_code (Errc::InvalidParam);
                    return -1;
                }
            }

            // check the consistency of the private key and the certificate.
            if (SSL_CTX_check_private_key (this->_tlsContext.get ()) == 0)
            {
                lastError = make_error_code (Errc::InvalidParam);
                this->close ();
                return -1;
            }

            return 0;
        }

        /**
         * @brief Set the location of the trusted CA certificate.
         * @param caFile path of the trusted CA certificate file.
         * @return 0 on success, -1 on failure.
         */
        int setCaCertificate (const std::string& caFile)
        {
            join::crypto::StackOfX509NamePtr certNames (SSL_load_client_CA_file (caFile.c_str ()));
            if (certNames == nullptr)
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            SSL_CTX_load_verify_locations (this->_tlsContext.get (), caFile.c_str (), nullptr);
            SSL_CTX_set_client_CA_list (this->_tlsContext.get (), certNames.release ());

            return 0;
        }

        /**
         * @brief Enable/Disable the verification of the peer certificate.
         * @param verify Enable peer verification if set to true, false otherwise.
         * @param depth The maximum certificate verification depth (default: no limit).
         */
        void setVerify (bool verify, int depth = -1)
        {
            if (verify)
            {
                // SSL_VERIFY_PEER will lead the client to verify the server certificate.
                // If the verification process fails, the TLS/SSL handshake is immediately terminated.
                SSL_CTX_set_verify (this->_tlsContext.get (), SSL_VERIFY_PEER, Socket::verifyWrapper);
                SSL_CTX_set_verify_depth (this->_tlsContext.get (), depth);
            }
            else
            {
                // SSL_VERIFY_NONE will lead the client to continue the handshake regardless of the verification result.
                SSL_CTX_set_verify (this->_tlsContext.get (), SSL_VERIFY_NONE, nullptr);
            }
        }

        /**
         * @brief set the cipher list (TLSv1.2 and below).
         * @param cipher the cipher list.
         * @return 0 on success, -1 on failure.
         */
        int setCipher (const std::string& cipher)
        {
            if (SSL_CTX_set_cipher_list (this->_tlsContext.get (), cipher.c_str ()) == 0)
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            return 0;
        }

    #if OPENSSL_VERSION_NUMBER >= 0x10101000L
        /**
         * @brief set the cipher list (TLSv1.3).
         * @param cipher the cipher list.
         * @return 0 on success, -1 on failure.
         */
        int setCipher_1_3 (const std::string &cipher)
        {
            if (SSL_CTX_set_ciphersuites (this->_tlsContext.get (), cipher.c_str ()) == 0)
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            return 0;
        }
    #endif

    #if OPENSSL_VERSION_NUMBER >= 0x30000000L
        /**
         * @brief set curve list.
         * @param curves curve list.
         * @return 0 on success, -1 on failure.
         */
        int setCurve (const std::string &curves)
        {
            if (SSL_CTX_set1_groups_list (this->_tlsContext.get (), curves.c_str ()) == 0)
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            return 0;
        }
    #else

    protected:
        /**
         * @brief generate openssl Diffie-Hellman parameters.
         * @note random Diffie-Hellman parameters generated using the command "openssl dhparam -C 2236".
         * @return Diffie-Hellman parameters.
         */
        static DH* getDh2236 ()
        {
            static unsigned char dhp_2236[] = {
                0x0C, 0xA5, 0x51, 0x2B, 0x8F, 0xF7, 0xA8, 0x74, 0x4D, 0x52,
                0xD7, 0xED, 0x97, 0x83, 0xA4, 0xD2, 0x8B, 0xF3, 0xE7, 0x92,
                0xF0, 0x27, 0x1B, 0xA0, 0x80, 0x83, 0x19, 0xDD, 0x02, 0xEF,
                0xA3, 0xE6, 0x13, 0x0A, 0x47, 0xE6, 0xF1, 0x3B, 0xC1, 0x5F,
                0x63, 0xC4, 0x03, 0xBA, 0xAC, 0xAA, 0xA3, 0x44, 0xC2, 0x03,
                0x6D, 0x62, 0x33, 0xAA, 0xF9, 0xA2, 0x5A, 0x98, 0xC2, 0xC0,
                0x71, 0x6F, 0xB0, 0x93, 0x6A, 0x26, 0x92, 0x90, 0x95, 0xEA,
                0xE8, 0x5F, 0x81, 0x50, 0x57, 0xB3, 0xB7, 0xE6, 0x3A, 0x3A,
                0x90, 0x15, 0x01, 0x2F, 0xC7, 0x8F, 0xAA, 0x0C, 0xAE, 0xC0,
                0xFF, 0x3A, 0xA7, 0x26, 0x5C, 0x87, 0xC2, 0x00, 0x68, 0xCA,
                0x02, 0x06, 0x50, 0x44, 0xEE, 0x75, 0xE7, 0xFF, 0x16, 0xD1,
                0x0F, 0x64, 0x51, 0x97, 0x52, 0x54, 0x69, 0xF0, 0x31, 0x81,
                0x4D, 0xEB, 0xF5, 0xA8, 0xB3, 0x7B, 0x48, 0x60, 0xBD, 0xC7,
                0xC9, 0x6E, 0x97, 0x86, 0x9B, 0xE6, 0x66, 0x4E, 0x1D, 0xE5,
                0x6F, 0xBA, 0xC5, 0x3D, 0xFD, 0x3F, 0x34, 0x69, 0x6F, 0xC0,
                0xFA, 0x8D, 0x42, 0x73, 0xA2, 0x49, 0xDE, 0xB6, 0x8D, 0x71,
                0x15, 0xFC, 0xB4, 0x18, 0x31, 0x5A, 0x24, 0xD0, 0x5E, 0xA8,
                0xE0, 0xD8, 0x1C, 0xF8, 0x0F, 0x1F, 0x59, 0x22, 0x5A, 0x07,
                0x75, 0x06, 0x98, 0x58, 0xE1, 0xF6, 0xA5, 0x53, 0xFD, 0x66,
                0x1E, 0x8F, 0x41, 0x63, 0x61, 0xA1, 0x79, 0x0D, 0x3B, 0xA7,
                0xF4, 0xBD, 0x72, 0xEB, 0xE1, 0xDC, 0xE2, 0xC9, 0x9B, 0x41,
                0xF6, 0x33, 0x3F, 0x9F, 0x0C, 0x33, 0x7B, 0xF2, 0x90, 0x68,
                0x28, 0xD3, 0x5A, 0xC1, 0x5C, 0xDE, 0x15, 0x11, 0xF4, 0xDD,
                0xCB, 0x09, 0x78, 0x63, 0x3B, 0xB6, 0xE8, 0xEE, 0x9A, 0x48,
                0xE9, 0x79, 0x80, 0x3F, 0x34, 0x8D, 0xB9, 0x24, 0x8D, 0x94,
                0x88, 0xA9, 0x75, 0xA5, 0x19, 0x05, 0x8D, 0x77, 0x20, 0xAF,
                0xC2, 0xC9, 0x7B, 0xD2, 0x51, 0xEE, 0x17, 0x22, 0xAC, 0x33,
                0xA8, 0xA6, 0x1B, 0x8B, 0xE3, 0x79, 0xF3, 0xE8, 0x3B, 0x6B
            };

            static unsigned char dhg_2236[] = {
                0x02
            };

            DH *dh = DH_new ();
            if (dh == nullptr)
            {
                return nullptr;
            }

            BIGNUM *p = BN_bin2bn (dhp_2236, sizeof (dhp_2236), nullptr);
            BIGNUM *g = BN_bin2bn (dhg_2236, sizeof (dhg_2236), nullptr);
            if (p == nullptr || g == nullptr || !DH_set0_pqg (dh, p, nullptr, g))
            {
                DH_free (dh);
                BN_free (p);
                BN_free (g);
                return nullptr;
            }

            return dh;
        }
    #endif

    protected:
        /// SSL/TLS context.
        join::crypto::SslCtxPtr _tlsContext;

        /// SSL session id.
        int _sessionId = 0;
    };
}

#endif
