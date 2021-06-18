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
#include <join/socket.hpp>

namespace join
{
namespace net
{
    /**
     * @brief basic stream acceptor class.
     */
    template <class Protocol>
    class BasicStreamAcceptor
    {
    public:
        using Observer = BasicObserver <BasicStreamAcceptor <Protocol>>;
        using Socket   = BasicStreamSocket <Protocol>;
        using Endpoint = typename Protocol::Endpoint;

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
        BasicStreamAcceptor (BasicStreamAcceptor&& other)
        : handle_ (other.handle_),
          protocol_ (other.protocol_)
        {
            other.handle_ = -1;
            other.protocol_ = Protocol ();
        }

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicStreamAcceptor& operator= (BasicStreamAcceptor&& other)
        {
            this->close ();

            this->handle_ = other.handle_;
            this->protocol_ = other.protocol_;

            other.handle_ = -1;
            other.protocol_ = Protocol ();

            return *this;
        }

        /**
         * @brief destroy instance.
         */
        virtual ~BasicStreamAcceptor ()
        {
            this->close ();
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

            this->handle_ = ::socket (protocol.family (), protocol.type () | SOCK_CLOEXEC, protocol.protocol ());
            if (this->handle_ == -1)
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                this->close ();
                return -1;
            }

            if (protocol.family () == AF_INET6)
            {
                int off = 0;

                if (::setsockopt (this->handle_, IPPROTO_IPV6, IPV6_V6ONLY, &off, sizeof (off)) == -1)
                {
                    lastError = std::make_error_code (static_cast <std::errc> (errno));
                    this->close ();
                    return -1;
                }
            }

            this->protocol_ = protocol;

            return 0;
        }

        /**
         * @brief close acceptor.
         * @return 0 on success, -1 on failure.
         */
        virtual int close () noexcept
        {
            if (this->handle_ != -1)
            {
                if (::close (this->handle_) == -1)
                {
                    lastError = std::make_error_code (static_cast <std::errc> (errno));
                    return -1;
                }

                this->handle_ = -1;
            }

            return 0;
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

                if (::setsockopt (this->handle_, SOL_SOCKET, SO_REUSEADDR, &on, sizeof (on)) == -1)
                {
                    lastError = std::make_error_code (static_cast <std::errc> (errno));
                    this->close ();
                    return -1;
                }
            }

            if (::bind (this->handle_, endpoint.addr (), endpoint.length ()) == -1)
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
            if (!this->opened ())
            {
                return -1;
            }

            if (::listen (this->handle_, max) == -1)
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                this->close ();
                return -1;
            }

            return 0;
        }

        /**
         * @brief accept new connection and fill in the client object with connection parameters.
         * @note the client socket object is allocated and must be released.
         * @return The client socket object on success, nullptr on failure.
         */
        virtual Socket accept () const
        {
            if (!this->opened ())
            {
                lastError = make_error_code (Errc::OperationFailed);
                return {};
            }

            Endpoint endpoint;
            socklen_t addrLen = endpoint.length ();
            Socket client;

            client.handle_ = ::accept (this->handle_, endpoint.addr (), &addrLen);
            if (client.handle_ == -1)
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                return {};
            }

            client.state_ = Socket::Connected;

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

        /**
         * @brief determine the local endpoint associated with this socket.
         * @return local endpoint.
         */
        Endpoint localEndpoint () const
        {
            Endpoint endpoint;
            socklen_t addrLen = endpoint.length ();

            if (::getsockname (this->handle_, endpoint.addr (), &addrLen) == -1)
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
            return (this->handle_ != -1);
        }

        /**
         * @brief get address family.
         * @return address family.
         */
        int family () const noexcept
        {
            return this->protocol_.family ();
        }

        /**
         * @brief get the protocol communication semantic.
         * @return the protocol communication semantic.
         */
        int type () const noexcept
        {
            return this->protocol_.type ();
        }

        /**
         * @brief get acceptor protocol.
         * @return acceptor protocol.
         */
        int protocol () const noexcept
        {
            return this->protocol_.protocol ();
        }

        /**
         * @brief get socket native handle.
         * @return socket native handle.
         */
        int handle () const noexcept
        {
            return this->handle_;
        }

    protected:
        /// socket handle.
        int handle_ = -1;

        /// protocol.
        Protocol protocol_;
    };

    /**
     * @brief basic TLS acceptor class.
     */
    template <class Protocol>
    class BasicTlsAcceptor : public BasicStreamAcceptor <Protocol>
    {
    public:
        using Observer = BasicObserver <BasicTlsAcceptor <Protocol>>;
        using Socket   = BasicTlsSocket <Protocol>;
        using Endpoint = typename Protocol::Endpoint;

        /**
         * @brief create the acceptor instance.
         */
        BasicTlsAcceptor ()
        : BasicStreamAcceptor <Protocol> (),
        #if OPENSSL_VERSION_NUMBER < 0x10100000L
          tlsContext_ (SSL_CTX_new (SSLv23_method ()), join::crypto::SslCtxDelete ()),
        #else
          tlsContext_ (SSL_CTX_new (TLS_method ()), join::crypto::SslCtxDelete ()),
        #endif
          sessionId_ (randomize <int> ())
        {
            if (tlsContext_ == nullptr)
            {
                throw std::runtime_error ("OpenSSL libraries were not initialized at process start");
            }

            // enable the OpenSSL bug workaround options.
            SSL_CTX_set_options (tlsContext_.get (), SSL_OP_ALL);

        #if OPENSSL_VERSION_NUMBER >= 0x10100000L
            // disallow compression.
            SSL_CTX_set_options (tlsContext_.get (), SSL_OP_NO_COMPRESSION);
        #endif

            // disallow usage of SSLv2, SSLv3, TLSv1 and TLSv1.1 which are considered insecure.
            SSL_CTX_set_options (tlsContext_.get (), SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1);

            // choose the cipher according to the server's preferences.
            SSL_CTX_set_options (tlsContext_.get (), SSL_OP_CIPHER_SERVER_PREFERENCE);

            // setup write mode.
            SSL_CTX_set_mode (tlsContext_.get (), SSL_MODE_ENABLE_PARTIAL_WRITE | SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);

            // automatically renegotiates.
            SSL_CTX_set_mode (tlsContext_.get (), SSL_MODE_AUTO_RETRY);

            // enable SSL session caching.
            SSL_CTX_set_session_id_context (tlsContext_.get (), reinterpret_cast <const unsigned char *> (&sessionId_), sizeof (sessionId_));

            // no verification by default.
            SSL_CTX_set_verify (tlsContext_.get (), SSL_VERIFY_NONE, nullptr);

            // set default TLSv1.2 and below cipher suites.
            SSL_CTX_set_cipher_list (tlsContext_.get (), join::crypto::defaultCipher_.c_str ());

        #if OPENSSL_VERSION_NUMBER >= 0x10101000L
            //  set default TLSv1.3 cipher suites.
            SSL_CTX_set_ciphersuites (tlsContext_.get (), join::crypto::defaultCipher_1_3_.c_str ());

            // disallow clent-side renegotiation.
            SSL_CTX_set_options (tlsContext_.get (), SSL_OP_NO_RENEGOTIATION);
        #endif

            // Set Diffie-Hellman key.
            join::crypto::DhKeyPtr dh (getDh2236 ());
            if (dh)
            {
                SSL_CTX_set_tmp_dh (tlsContext_.get (), dh.get ());
            }

            // Set elliptic curve Diffie-Hellman key.
            join::crypto::EcdhKeyPtr ecdh (EC_KEY_new_by_curve_name (NID_X9_62_prime256v1));
            if (ecdh)
            {
                SSL_CTX_set_tmp_ecdh (tlsContext_.get (), ecdh.get ());
            }
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
        : BasicStreamAcceptor <Protocol> (std::move (other)),
          tlsContext_ (std::move (other.tlsContext_)),
          sessionId_ (other.sessionId_)
        {
            other.sessionId_ = 0;
        }

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicTlsAcceptor& operator= (BasicTlsAcceptor&& other)
        {
            BasicStreamAcceptor <Protocol>::operator= (std::move (other));

            tlsContext_ = std::move (other.tlsContext_);
            sessionId_ = other.sessionId_;

            other.sessionId_ = 0;

            return *this;
        }

        /**
         * @brief accept new connection and fill in the client object with connection parameters.
         * @note the client socket object is allocated and must be released.
         * @return The client socket object on success, nullptr on failure.
         */
        virtual Socket accept () const override
        {
            if (!this->opened ())
            {
                lastError = make_error_code (Errc::OperationFailed);
                return {};
            }

            Endpoint endpoint;
            socklen_t addrLen = endpoint.length ();
            Socket client (this->tlsContext_, Socket::ServerMode);

            client.handle_ = ::accept (this->handle_, endpoint.addr (), &addrLen);
            if (client.handle_ == -1)
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                return {};
            }

            client.state_ = Socket::Connected;

            if (client.setMode (Socket::NonBlocking) == -1)
            {
                client.close ();
                return {};
            }

            if (client.setOption (Socket::NoDelay, 1) == -1)
            {
                client.close ();
                return {};
            }

            client.tlsHandle_.reset (SSL_new (client.tlsContext_.get ()));
            if (client.tlsHandle_ == nullptr)
            {
                lastError = make_error_code (Errc::OutOfMemory);
                client.close ();
                return {};
            }

            if (SSL_set_fd (client.tlsHandle_.get (), client.handle_) == 0)
            {
                lastError = make_error_code (Errc::InvalidParam);
                client.close ();
                return {};
            }

            SSL_set_app_data (client.tlsHandle_.get (), &client);  
            SSL_set_accept_state (client.tlsHandle_.get ());

        #ifdef DEBUG
            // Set info callback.
            SSL_set_info_callback (client.tlsHandle_.get (), Socket::infoWrapper);
        #endif // DEBUG

            client.tlsState_ = Socket::Encrypted;

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
            if (SSL_CTX_use_certificate_file (this->tlsContext_.get (), cert.c_str (), SSL_FILETYPE_PEM) == 0)
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            if (key.size ())
            {
                if (SSL_CTX_use_PrivateKey_file (this->tlsContext_.get (), key.c_str(), SSL_FILETYPE_PEM) == 0)
                {
                    lastError = make_error_code (Errc::InvalidParam);
                    return -1;
                }
            }

            // check the consistency of the private key and the certificate.
            if (SSL_CTX_check_private_key (this->tlsContext_.get ()) == 0)
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

            if (SSL_CTX_load_verify_locations (this->tlsContext_.get (), caFile.c_str (), nullptr) == 0)
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            SSL_CTX_set_client_CA_list (this->tlsContext_.get (), certNames.release ());

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
                SSL_CTX_set_verify (this->tlsContext_.get (), SSL_VERIFY_PEER, Socket::verifyWrapper);
                SSL_CTX_set_verify_depth (this->tlsContext_.get (), depth);
            }
            else
            {
                // SSL_VERIFY_NONE will lead the client to continue the handshake regardless of the verification result.
                SSL_CTX_set_verify (this->tlsContext_.get (), SSL_VERIFY_NONE, nullptr);
            }
        }

        /**
         * @brief set the cipher list (TLSv1.2 and below).
         * @param cipher the cipher list.
         * @return 0 on success, -1 on failure.
         */
        int setCipher (const std::string& cipher)
        {
            if (SSL_CTX_set_cipher_list (this->tlsContext_.get (), cipher.c_str ()) == 0)
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
            if (SSL_CTX_set_ciphersuites (this->tlsContext_.get (), cipher.c_str ()) == 0)
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            return 0;
        }
    #endif // OPENSSL_VERSION_NUMBER >= 0x10101000L

        /**
         * @brief create a random number for message id.
         * @rerturn random number for message id.
         */
        template <typename Type>
        std::enable_if_t <std::numeric_limits <Type>::is_integer, Type>
        static randomize ()
        {
            std::random_device rnd;
            std::uniform_int_distribution <Type> dist {};
            return dist (rnd);
        }

    protected:
        /**
         * @brief generate openssl Diffie-Hellman parameters.
         * @note random Diffie-Hellman parameters generated using the command "openssl dhparam -C 2236".
         * @return Diffie-Hellman parameters.
         */
        static DH* getDh2236 ()
        {
            static unsigned char dhp_2236[] = {
                0x0C, 0xE0, 0x86, 0x60, 0xA9, 0x7C, 0x2D, 0x02, 0xF5, 0x58,
                0x08, 0x0C, 0x92, 0x1D, 0x07, 0xC6, 0xF1, 0xBF, 0x66, 0xBA,
                0x9B, 0xDB, 0x0D, 0x3F, 0x06, 0x1E, 0x9F, 0x33, 0x9F, 0xC9,
                0x37, 0x89, 0xD4, 0x9E, 0x33, 0x4B, 0x37, 0x0B, 0xC2, 0x96,
                0x30, 0xA9, 0x17, 0x15, 0xA5, 0xF2, 0x33, 0x1E, 0x1E, 0xFB,
                0xE8, 0xBF, 0x23, 0xCD, 0xEC, 0xBA, 0x95, 0x12, 0xBB, 0xA1,
                0x15, 0x5E, 0x4D, 0x1A, 0xA3, 0x6F, 0xA3, 0x64, 0x65, 0x6A,
                0xF2, 0x9F, 0x2F, 0xFB, 0xE8, 0xC5, 0xD7, 0x38, 0xA2, 0xF0,
                0x32, 0x5F, 0x87, 0x73, 0x37, 0x26, 0x9B, 0x88, 0xF3, 0x5A,
                0x2C, 0x8C, 0x1E, 0x33, 0x84, 0x5D, 0x05, 0xEC, 0x92, 0x47,
                0x04, 0xDA, 0xEC, 0x33, 0x89, 0x33, 0x57, 0x50, 0x1D, 0x76,
                0x86, 0x5D, 0x67, 0x35, 0x72, 0x50, 0x83, 0x56, 0x99, 0x58,
                0xA2, 0x3E, 0x06, 0xB9, 0x49, 0xD6, 0xA6, 0x4A, 0x92, 0xE5,
                0x32, 0xAB, 0x1C, 0x76, 0x1E, 0xDC, 0x41, 0x1A, 0xBA, 0x0B,
                0xF9, 0x12, 0x0A, 0xFD, 0x34, 0x0C, 0xFD, 0xD8, 0x5F, 0x85,
                0x03, 0x1B, 0xBE, 0x12, 0xAE, 0x5A, 0x5E, 0xEA, 0xB9, 0x1E,
                0x93, 0x82, 0x7F, 0x65, 0x10, 0x8E, 0x33, 0x11, 0x73, 0x23,
                0x3C, 0x8C, 0x22, 0x4D, 0xBA, 0xFD, 0x62, 0xAD, 0x0B, 0x6B,
                0x84, 0x79, 0x0E, 0xFC, 0x92, 0x49, 0x16, 0x0D, 0x52, 0x29,
                0x95, 0x61, 0x83, 0x50, 0xF2, 0xD8, 0xD0, 0x57, 0x3D, 0x00,
                0xE4, 0x38, 0xB0, 0x17, 0x93, 0xE5, 0x70, 0x39, 0x77, 0xAE,
                0x96, 0x25, 0x2E, 0x97, 0xDC, 0x37, 0xC4, 0x21, 0x34, 0xBC,
                0x8E, 0xF5, 0xD9, 0xC7, 0x9D, 0x92, 0xBF, 0xE1, 0xAD, 0x45,
                0x61, 0x3C, 0xD6, 0xAC, 0x9E, 0x8A, 0xBC, 0xCD, 0x0C, 0xE3,
                0x7C, 0x7A, 0x99, 0xE5, 0x7A, 0x10, 0xD8, 0xF1, 0xAC, 0x6B,
                0x72, 0x58, 0xB9, 0xBD, 0x2C, 0x1C, 0xAC, 0xBA, 0xFA, 0x65,
                0x5B, 0xCF, 0x5D, 0x0B, 0x2F, 0xE8, 0x69, 0xA3, 0xD2, 0x52,
                0xAB, 0x17, 0x65, 0xBC, 0x72, 0x35, 0x6D, 0x84, 0x5B, 0x9B
            };
            static unsigned char dhg_2236[] = {
                0x02
            };
            DH *dh = DH_new();
            BIGNUM *p, *g;

            if (dh == NULL)
                return NULL;
            p = BN_bin2bn(dhp_2236, sizeof(dhp_2236), NULL);
            g = BN_bin2bn(dhg_2236, sizeof(dhg_2236), NULL);
            if (p == NULL || g == NULL
                    || !DH_set0_pqg(dh, p, NULL, g)) {
                DH_free(dh);
                BN_free(p);
                BN_free(g);
                return NULL;
            }
            return dh;
        }

        /// SSL/TLS context.
        join::crypto::SslCtxPtr tlsContext_;

        /// SSL session id.
        int sessionId_ = 0;
    };
}
}

#endif