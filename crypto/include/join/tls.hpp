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

#ifndef JOIN_CRYPTO_TLS_HPP
#define JOIN_CRYPTO_TLS_HPP

// libjoin.
#include <join/datagram_socket.hpp>
#include <join/stream_socket.hpp>
#include <join/tls_protocol.hpp>
#include <join/tls_context.hpp>
#include <join/tls_error.hpp>
#include <join/error.hpp>

// C++.
#include <iostream>
#include <utility>
#include <chrono>
#include <string>

// C.
#include <fnmatch.h>
#include <cassert>

namespace join
{
    /**
     * @brief basic TLS/DTLS decorator.
     */
    template <class Protocol>
    class BasicTls
    {
    public:
        using UnderlyingSocket = typename Protocol::Transport::Socket;
        using Mode = typename UnderlyingSocket::Mode;
        using Option = typename UnderlyingSocket::Option;
        using State = typename UnderlyingSocket::State;
        using Endpoint = typename Protocol::Endpoint;

        /**
         * @brief create a TLS decorator with an internally created socket.
         * @param ctx TLS context.
         * @param mode blocking mode.
         */
        explicit BasicTls (TlsContext ctx, Mode mode = Mode::NonBlocking) noexcept
        : BasicTls (UnderlyingSocket{mode}, ctx)
        {
        }

        /**
         * @brief create a TLS decorator taking ownership of the given socket, without context.
         * @param socket underlying socket.
         */
        explicit BasicTls (UnderlyingSocket&& socket) noexcept
        : _socket (std::move (socket))
        {
        }

        /**
         * @brief create a TLS decorator taking ownership of the given socket.
         * @param socket underlying socket.
         * @param ctx TLS context.
         */
        BasicTls (UnderlyingSocket&& socket, TlsContext ctx) noexcept
        : _socket (std::move (socket))
        , _ctx (ctx)
        {
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicTls (const BasicTls& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicTls& operator= (const BasicTls& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        BasicTls (BasicTls&& other) noexcept
        : _socket (std::move (other._socket))
        , _ctx (std::move (other._ctx))
        , _ssl (std::move (other._ssl))
        {
            if (_ssl)
            {
                SSL_set_app_data (_ssl.get (), this);
            }
        }

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicTls& operator= (BasicTls&& other) noexcept
        {
            _socket = std::move (other._socket);
            _ctx = std::move (other._ctx);
            _ssl = std::move (other._ssl);

            if (_ssl)
            {
                SSL_set_app_data (_ssl.get (), this);
            }

            return *this;
        }

        /**
         * @brief destroy the instance.
         */
        virtual ~BasicTls () = default;

        /**
         * @brief open the underlying socket using the given protocol.
         * @param protocol protocol to use.
         * @return 0 on success, -1 on failure.
         */
        int open (const Protocol& protocol = Protocol ()) noexcept
        {
            return _socket.open (typename Protocol::Transport (protocol.family ()));
        }

        /**
         * @brief check if the underlying socket is opened.
         * @return true if opened.
         */
        bool opened () const noexcept
        {
            return _socket.opened ();
        }

        /**
         * @brief assigns the specified endpoint to the underlying socket.
         * @param ep endpoint to assign to the underlying socket.
         * @return 0 on success, -1 on failure.
         */
        int bind (const Endpoint& ep) noexcept
        {
            return _socket.bind (ep);
        }

        /**
         * @brief assigns the specified device to the underlying socket.
         * @param dev device name.
         * @return 0 on success, -1 on failure.
         */
        int bindToDevice (const std::string& dev) noexcept
        {
            return _socket.bindToDevice (dev);
        }

        /**
         * @brief connect the underlying socket to the remote endpoint.
         * @param ep endpoint to connect to.
         * @return 0 on success, -1 on failure.
         * @note transport connection only, the TLS handshake is performed separately.
         */
        int connect (const Endpoint& ep) noexcept
        {
            return _socket.connect (ep);
        }

        /**
         * @brief check if the underlying socket is connected.
         * @return true if connected.
         */
        bool connected () noexcept
        {
            return _socket.connected ();
        }

        /**
         * @brief set the TLS layer up without negotiating immediately.
         * @return 0 on success, -1 on failure.
         */
        int deferHandshake ()
        {
            if (!_ctx.handle ())
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            if (!_ssl)
            {
                if (_socket.type () == SOCK_DGRAM)
                {
                    if (!_socket.opened ())
                    {
                        lastError = make_error_code (Errc::OperationFailed);
                        return -1;
                    }

                    if (!_ctx.isServer () && !_socket.connected ())
                    {
                        lastError = make_error_code (Errc::OperationFailed);
                        return -1;
                    }
                }

                if (_socket.type () == SOCK_STREAM)
                {
                    if (!_socket.connected ())
                    {
                        lastError = make_error_code (Errc::OperationFailed);
                        return -1;
                    }
                }

                _ssl.reset (SSL_new (_ctx.handle ()));
                if (!_ssl)
                {
                    // LCOV_EXCL_START
                    lastError = make_error_code (Errc::OutOfMemory);
                    return -1;
                    // LCOV_EXCL_STOP
                }

                if (_socket.type () == SOCK_DGRAM)
                {
                    BIO* bio = BIO_new_dgram (_socket.handle (), BIO_NOCLOSE);
                    if (!bio)
                    {
                        lastError = make_error_code (Errc::OutOfMemory);
                        _ssl.reset ();
                        return -1;
                    }

                    if (_socket.connected ())
                    {
                        BIO_ctrl (bio, BIO_CTRL_DGRAM_SET_CONNECTED, 0,
                                  const_cast<struct sockaddr*> (_socket.remoteEndpoint ().addr ()));
                    }

                    SSL_set_bio (_ssl.get (), bio, bio);
                    SSL_set_read_ahead (_ssl.get (), 1);
                }
                else
                {
                    if (SSL_set_fd (_ssl.get (), _socket.handle ()) == 0)
                    {
                        lastError = make_error_code (Errc::InvalidParam);
                        _ssl.reset ();
                        return -1;
                    }
                }

                if (SSL_is_server (_ssl.get ()))
                {
                    SSL_set_accept_state (_ssl.get ());
                }
                else
                {
                    const std::string& host = _socket.remoteEndpoint ().hostname ();
                    if (!host.empty () && SSL_set_tlsext_host_name (_ssl.get (), host.c_str ()) != 1)
                    {
                        lastError = make_error_code (Errc::InvalidParam);
                        _ssl.reset ();
                        return -1;
                    }

                    SSL_set_connect_state (_ssl.get ());
                }

                SSL_set_app_data (_ssl.get (), this);
#ifdef DEBUG
                SSL_set_info_callback (_ssl.get (), infoWrapper);
#endif

                if (_ctx.verify ())
                {
                    SSL_set_verify (_ssl.get (), SSL_VERIFY_PEER, verifyWrapper);
                    SSL_set_verify_depth (_ssl.get (), _ctx.depth ());
                }
                else
                {
                    SSL_set_verify (_ssl.get (), SSL_VERIFY_NONE, nullptr);
                }
            }

            return 0;
        }

        /**
         * @brief perform the TLS handshake.
         * @return 0 on success, -1 on failure.
         */
        int handshake ()
        {
            if (encrypted ())
            {
                return 0;
            }

            if (deferHandshake () == -1)
            {
                return -1;
            }

            int result = SSL_do_handshake (_ssl.get ());
            if (result < 1)
            {
                int ret = handleTlsError (result);
                if (lastError != make_error_code (Errc::TemporaryError))
                {
                    _ssl.reset ();
                }
                return ret;
            }

            return 0;
        }

        /**
         * @brief block until TLS handshake is finished.
         * @param timeout timeout duration (zero: infinite).
         * @return true if TLS handshake is finished.
         */
        virtual bool waitHandshake (std::chrono::nanoseconds timeout)
        {
            if (handshake () == 0)
            {
                return true;
            }

            const bool isDtls = (_socket.type () == SOCK_DGRAM);

            while (lastError == make_error_code (Errc::TemporaryError))
            {
                bool wantRead = SSL_want_read (_ssl.get ());
                bool wantWrite = SSL_want_write (_ssl.get ());

                if (!wantRead && !wantWrite)
                {
                    break;  // LCOV_EXCL_LINE
                }

                std::chrono::nanoseconds activeTimeout = timeout;

                if (isDtls)
                {
                    struct timeval dtlsTimeout;
                    if (DTLSv1_get_timeout (_ssl.get (), &dtlsTimeout))
                    {
                        auto dtlsDuration = std::chrono::duration_cast<std::chrono::nanoseconds> (
                            std::chrono::seconds (dtlsTimeout.tv_sec) +
                            std::chrono::microseconds (dtlsTimeout.tv_usec));
                        if ((timeout <= std::chrono::nanoseconds::zero ()) || (dtlsDuration < timeout))
                        {
                            activeTimeout = dtlsDuration;
                        }
                    }
                }

                int waitResult = _socket.wait (wantRead, wantWrite, activeTimeout);
                if (waitResult == -1)
                {
                    if (isDtls && (lastError == make_error_code (Errc::TimedOut)))
                    {
                        int ret = DTLSv1_handle_timeout (_ssl.get ());
                        if (ret < 0)
                        {
                            lastError = make_error_code (TlsErrc::TlsProtocolError);
                            return false;
                        }
                        else if (ret == 0)
                        {
                            continue;
                        }

                        lastError = make_error_code (Errc::TemporaryError);
                        if (handshake () == 0)
                        {
                            return true;
                        }

                        continue;
                    }

                    return false;
                }

                if (handshake () == 0)
                {
                    return true;
                }
            }

            return false;
        }

        /**
         * @brief check if the stream is encrypted.
         * @return true if encrypted.
         */
        bool encrypted () const noexcept
        {
            return _ssl != nullptr && SSL_is_init_finished (_ssl.get ());
        }

        /**
         * @brief Perform the TLS shutdown.
         * @return 0 on success, -1 on failure.
         */
        int shutdown () noexcept
        {
            if (!_ssl)
            {
                return 0;
            }

            if ((SSL_get_shutdown (_ssl.get ()) & SSL_SENT_SHUTDOWN) == 0)
            {
                int result = SSL_shutdown (_ssl.get ());
                if (result < 0)
                {
                    return handleTlsError (result);
                }
            }

            if (_socket.type () == SOCK_DGRAM)
            {
                if ((SSL_get_shutdown (_ssl.get ()) & SSL_RECEIVED_SHUTDOWN) == 0)
                {
                    int result = SSL_shutdown (_ssl.get ());
                    if (result < 0)
                    {
                        handleTlsError (result);
                        if (lastError != Errc::ConnectionClosed && lastError != TlsErrc::TlsCloseNotifyAlert)
                        {
                            return -1;
                        }
                    }
                }
            }

            _ssl.reset ();
            return 0;
        }

        /**
         * @brief block until TLS shutdown is finished.
         * @param timeout timeout duration (zero: infinite).
         * @return true if TLS shutdown is finished.
         */
        bool waitShutdown (std::chrono::nanoseconds timeout) noexcept
        {
            if (!_ssl)
            {
                return true;
            }

            if (shutdown () == 0)
            {
                return true;
            }

            while (lastError == make_error_code (Errc::TemporaryError))
            {
                bool wantRead = SSL_want_read (_ssl.get ());
                bool wantWrite = SSL_want_write (_ssl.get ());

                if (!wantRead && !wantWrite)
                {
                    break;  // LCOV_EXCL_LINE
                }

                if (_socket.wait (wantRead, wantWrite, timeout) == -1)
                {
                    return false;
                }

                if (shutdown () == 0)
                {
                    return true;
                }
            }

            return false;
        }

        /**
         * @brief disconnect the underlying socket from the remote endpoint.
         * @return 0 on success, -1 on failure.
         */
        int disconnect () noexcept
        {
            return _socket.disconnect ();
        }

        /**
         * @brief close the socket handle.
         */
        void close () noexcept
        {
            _ssl.reset ();
            _socket.close ();
        }

        /**
         * @brief block until new data is available for reading.
         * @param timeout timeout duration (zero: infinite).
         * @return true if there is new data available for reading, false otherwise.
         */
        bool waitReadyRead (std::chrono::nanoseconds timeout = std::chrono::nanoseconds::zero ()) const noexcept
        {
            if (_ssl)
            {
                bool wantRead = SSL_want_read (_ssl.get ());
                bool wantWrite = SSL_want_write (_ssl.get ());

                if (wantRead || wantWrite)
                {
                    return (_socket.wait (wantRead, wantWrite, timeout) == 0);
                }
            }

            return _socket.waitReadyRead (timeout);
        }

        /**
         * @brief read data from the TLS stream.
         * @param buf buffer to read into.
         * @param len maximum number of bytes to read.
         * @return number of bytes read on success, -1 on failure.
         */
        ssize_t read (char* buf, size_t len) noexcept
        {
            if (_ssl)
            {
                int nread = SSL_read (_ssl.get (), buf, static_cast<int> (len));
                if (nread < 1)
                {
                    return handleTlsError (nread);
                }

                return nread;
            }

            return _socket.read (buf, len);
        }

        /**
         * @brief read data until size is reached or an error occurred.
         * @param data buffer used to store the data received.
         * @param size number of bytes to read.
         * @param timeout timeout duration (zero: infinite).
         * @return 0 on success, -1 on failure.
         */
        int readExactly (char* data, size_t size, std::chrono::nanoseconds timeout = std::chrono::nanoseconds::zero ())
        {
            size_t numRead = 0;

            while (numRead < size)
            {
                ssize_t result = read (data + numRead, size - numRead);
                if (result == -1)
                {
                    if (lastError == Errc::TemporaryError)
                    {
                        if (waitReadyRead (timeout))
                        {
                            continue;
                        }
                    }

                    return -1;
                }

                numRead += result;
            }

            return 0;
        }

        /**
         * @brief block until at least one byte can be written on the socket.
         * @param timeout timeout duration (zero: infinite).
         * @return true if data can be written on the socket, false otherwise.
         */
        bool waitReadyWrite (std::chrono::nanoseconds timeout = std::chrono::nanoseconds::zero ()) const noexcept
        {
            if (_ssl)
            {
                bool wantRead = SSL_want_read (_ssl.get ());
                bool wantWrite = SSL_want_write (_ssl.get ());

                if (wantRead || wantWrite)
                {
                    return (_socket.wait (wantRead, wantWrite, timeout) == 0);
                }
            }

            return _socket.waitReadyWrite (timeout);
        }

        /**
         * @brief write data to the TLS stream.
         * @param buf buffer to write from.
         * @param len number of bytes to write.
         * @return number of bytes written on success, -1 on failure.
         */
        ssize_t write (const char* buf, size_t len) noexcept
        {
            if (_ssl)
            {
                int nwritten = SSL_write (_ssl.get (), buf, static_cast<int> (len));
                if (nwritten < 1)
                {
                    return handleTlsError (nwritten);
                }

                return nwritten;
            }

            return _socket.write (buf, len);
        }

        /**
         * @brief write data until size is reached or an error occurred.
         * @param data data buffer to send.
         * @param size number of bytes to write.
         * @param timeout timeout duration (zero: infinite).
         * @return 0 on success, -1 on failure.
         */
        int writeExactly (const char* data, size_t size,
                          std::chrono::nanoseconds timeout = std::chrono::nanoseconds::zero ())
        {
            size_t numWrite = 0;

            while (numWrite < size)
            {
                ssize_t result = write (data + numWrite, size - numWrite);
                if (result == -1)
                {
                    if (lastError == Errc::TemporaryError)
                    {
                        if (waitReadyWrite (timeout))
                        {
                            continue;
                        }
                    }

                    return -1;
                }

                numWrite += result;
            }

            return 0;
        }

        /**
         * @brief set the mode of the underlying socket.
         * @param mode socket mode.
         */
        void setMode (Mode mode) noexcept
        {
            _socket.setMode (mode);
        }

        /**
         * @brief set an option for the underlying socket.
         * @param opt socket option.
         * @param val option value.
         * @return 0 on success, -1 on failure.
         */
        int setOption (Option opt, int val) noexcept
        {
            return _socket.setOption (opt, val);
        }

        /**
         * @brief get the underlying socket handle.
         * @return socket handle.
         */
        int handle () const noexcept
        {
            return _socket.handle ();
        }

        /**
         * @brief get the underlying socket address family.
         * @return socket address family.
         */
        int family () const noexcept
        {
            return _socket.family ();
        }

        /**
         * @brief get the underlying socket type.
         * @return socket type.
         */
        int type () const noexcept
        {
            return _socket.type ();
        }

        /**
         * @brief get the underlying protocol.
         * @return protocol.
         */
        int protocol () const noexcept
        {
            return _socket.protocol ();
        }

        /**
         * @brief get the maximum transmission unit.
         * @return MTU.
         */
        int mtu () const noexcept
        {
            return _socket.mtu ();
        }

        /**
         * @brief get the local endpoint.
         * @return local endpoint.
         */
        Endpoint localEndpoint () const noexcept
        {
            return _socket.localEndpoint ();
        }

        /**
         * @brief get the remote endpoint.
         * @return remote endpoint.
         * @note returns by value, copying the endpoint hostname may allocate.
         */
        Endpoint remoteEndpoint () const
        {
            return _socket.remoteEndpoint ();
        }

    protected:
        /**
         * @brief handle TLS error.
         * @param result result returned by a previous call of the openssl API.
         * @return -1 is always returned.
         */
        int handleTlsError (int result) noexcept
        {
            switch (SSL_get_error (_ssl.get (), result))
            {
                case SSL_ERROR_WANT_READ:
                case SSL_ERROR_WANT_WRITE:
                case SSL_ERROR_WANT_X509_LOOKUP:
                    // want read, want write or want lookup.
                    lastError = make_error_code (Errc::TemporaryError);
                    break;

                case SSL_ERROR_ZERO_RETURN:
                    // a close notify alert was received.
                    // we have to answer by sending a close notify alert too.
                    lastError = make_error_code (TlsErrc::TlsCloseNotifyAlert);
                    break;

                case SSL_ERROR_SYSCALL:
                    // an error occurred at the socket level.
                    if (errno == 0 || errno == ECONNRESET || errno == EPIPE)
                    {
                        lastError = make_error_code (Errc::ConnectionClosed);
                    }
                    else
                    {
                        lastError = std::error_code (errno, std::generic_category ());
                    }
                    break;

                default:
                    // SSL protocol error.
#ifdef DEBUG
                    std::cout << ERR_reason_error_string (ERR_get_error ()) << std::endl;
#endif
                    lastError = make_error_code (TlsErrc::TlsProtocolError);
                    break;
            }

            return -1;
        }

        /**
         * @brief c style info callback wrapper.
         * @param ssl SSL object.
         * @param where context flags.
         * @param ret error condition.
         */
        static void infoWrapper (const SSL* ssl, int where, int ret) noexcept
        {
            assert (ssl);
            static_cast<BasicTls<Protocol>*> (SSL_get_app_data (ssl))->infoCallback (where, ret);
        }

        /**
         * @brief SSL state info callback.
         * @param where context flags.
         * @param ret error condition.
         */
        void infoCallback (int where, int ret) const noexcept
        {
            if (where & SSL_CB_ALERT)
            {
                std::cout << "SSL/TLS Alert ";
                (where & SSL_CB_READ) ? std::cout << "[read] " : std::cout << "[write] ";
                std::cout << SSL_alert_type_string_long (ret) << ":";
                std::cout << SSL_alert_desc_string_long (ret);
                std::cout << std::endl;
            }
            else if (where & SSL_CB_LOOP)
            {
                std::cout << "SSL/TLS State ";
                (SSL_in_connect_init (_ssl.get ()))  ? std::cout << "[connect] "
                : (SSL_in_accept_init (_ssl.get ())) ? std::cout << "[accept] "
                                                     : std::cout << "[undefined] ";
                std::cout << SSL_state_string_long (_ssl.get ());
                std::cout << std::endl;
            }
            else if (where & SSL_CB_HANDSHAKE_START)
            {
                std::cout << "SSL/TLS Handshake [Start] " << SSL_state_string_long (_ssl.get ()) << std::endl;
            }
            else if (where & SSL_CB_HANDSHAKE_DONE)
            {
                std::cout << "SSL/TLS Handshake [Done] " << SSL_state_string_long (_ssl.get ()) << std::endl;
                std::cout << SSL_CTX_sess_number (_ctx.handle ()) << " items in the session cache" << std::endl;
                std::cout << SSL_CTX_sess_connect (_ctx.handle ()) << " client connects" << std::endl;
                std::cout << SSL_CTX_sess_connect_good (_ctx.handle ()) << " client connects that finished"
                          << std::endl;
                std::cout << SSL_CTX_sess_connect_renegotiate (_ctx.handle ()) << " client renegotiations requested"
                          << std::endl;
                std::cout << SSL_CTX_sess_accept (_ctx.handle ()) << " server connects" << std::endl;
                std::cout << SSL_CTX_sess_accept_good (_ctx.handle ()) << " server connects that finished" << std::endl;
                std::cout << SSL_CTX_sess_accept_renegotiate (_ctx.handle ()) << " server renegotiations requested"
                          << std::endl;
                std::cout << SSL_CTX_sess_hits (_ctx.handle ()) << " session cache hits" << std::endl;
                std::cout << SSL_CTX_sess_cb_hits (_ctx.handle ()) << " external session cache hits" << std::endl;
                std::cout << SSL_CTX_sess_misses (_ctx.handle ()) << " session cache misses" << std::endl;
                std::cout << SSL_CTX_sess_timeouts (_ctx.handle ()) << " session cache timeouts" << std::endl;
                std::cout << "negotiated " << SSL_get_cipher (_ssl.get ()) << " cipher suite" << std::endl;
            }
        }

        /**
         * @brief c style verify callback wrapper.
         * @param preverified pre-verification status.
         * @param x509Ctx X509 store context.
         * @return 1 if verified, 0 otherwise.
         */
        static int verifyWrapper (int preverified, X509_STORE_CTX* x509Ctx) noexcept
        {
            SSL* ssl = static_cast<SSL*> (X509_STORE_CTX_get_ex_data (x509Ctx, SSL_get_ex_data_X509_STORE_CTX_idx ()));
            assert (ssl);
            return static_cast<BasicTls<Protocol>*> (SSL_get_app_data (ssl))->verifyCallback (preverified, x509Ctx);
        }

        /**
         * @brief verify peer certificate.
         * @param preverified pre-verification status.
         * @param context X509 store context.
         * @return 1 if verified, 0 otherwise.
         */
        int verifyCallback (int preverified, X509_STORE_CTX* context) noexcept
        {
            int maxDepth = SSL_get_verify_depth (_ssl.get ());
            int dpth = X509_STORE_CTX_get_error_depth (context);

#ifdef DEBUG
            std::cout << "verification started at depth=" << dpth << std::endl;
#endif

            // catch a too long certificate chain.
            if ((maxDepth >= 0) && (dpth > maxDepth))
            {
                preverified = 0;
                X509_STORE_CTX_set_error (context, X509_V_ERR_CERT_CHAIN_TOO_LONG);
            }

            if (!preverified)
            {
#ifdef DEBUG
                std::cout << "verification failed at depth=" << dpth << " - "
                          << X509_verify_cert_error_string (X509_STORE_CTX_get_error (context)) << std::endl;
#endif
                return 0;
            }

            // check the certificate host name.
            if (!verifyCert (context))
            {
#ifdef DEBUG
                std::cout << "rejected by CERT at depth=" << dpth << std::endl;
#endif
                return 0;
            }

            // check the revocation list.
            /*if (!verifyCrl (context))
            {
            #ifdef DEBUG
                std::cout << "rejected by CRL at depth=" << dpth << std::endl;
            #endif
                return 0;
            }*/

            // check ocsp.
            /*if (!verifyOcsp (context))
            {
            #ifdef DEBUG
                std::cout << "rejected by OCSP at depth=" << dpth << std::endl;
            #endif
                return 0;
            }*/

#ifdef DEBUG
            std::cout << "certificate accepted at depth=" << dpth << std::endl;
#endif

            return 1;
        }

        /**
         * @brief verify certificate validity.
         * @param context pointer to the complete context used for the certificate chain verification.
         * @return when verified successfully, the callback should return 1, 0 otherwise.
         */
        int verifyCert (X509_STORE_CTX* context) const
        {
            int depth = X509_STORE_CTX_get_error_depth (context);
            X509* cert = X509_STORE_CTX_get_current_cert (context);

            char buf[256];
            X509_NAME_oneline (X509_get_subject_name (cert), buf, sizeof (buf));
#ifdef DEBUG
            std::cout << "subject=" << buf << std::endl;
#endif

            // check the certificate host name
            if (depth == 0)
            {
                // confirm a match between the hostname and the hostnames listed in the certificate.
                if (!checkHostname (cert))
                {
#ifdef DEBUG
                    std::cout << "no match for hostname in the certificate" << std::endl;
#endif
                    return 0;
                }
            }

            return 1;
        }

        /**
         * @brief check certificate hostname against remote endpoint.
         * @param certificate X509 certificate.
         * @return true if hostname matches.
         */
        bool checkHostname (X509* certificate) const noexcept
        {
            bool match = false;

            // get remote hostname name.
            std::string serverName (_socket.remoteEndpoint ().hostname ());

            // strip off trailing dots.
            if (!serverName.empty () && serverName.back () == '.')
            {
                serverName.pop_back ();
            }

            // get alternative names.
            join::StackOfGeneralNamePtr altnames (reinterpret_cast<STACK_OF (GENERAL_NAME)*> (
                X509_get_ext_d2i (certificate, NID_subject_alt_name, 0, 0)));
            if (altnames)
            {
                for (int i = 0; (i < sk_GENERAL_NAME_num (altnames.get ())) && !match; ++i)
                {
                    // get a handle to alternative name.
                    GENERAL_NAME* current_name = sk_GENERAL_NAME_value (altnames.get (), i);

                    if (current_name->type == GEN_DNS)
                    {
                        // get data and length.
                        const char* host = reinterpret_cast<const char*> (ASN1_STRING_get0_data (current_name->d.ia5));
                        size_t len = size_t (ASN1_STRING_length (current_name->d.ia5));
                        std::string pattern (host, host + len);

                        // strip off trailing dots.
                        if (!pattern.empty () && pattern.back () == '.')
                        {
                            pattern.pop_back ();
                        }

                        // compare to pattern.
                        if (fnmatch (pattern.c_str (), serverName.c_str (), 0) == 0)
                        {
                            // an alternative name matched the server hostname.
                            match = true;
                        }
                    }
                }
            }

            return match;
        }

        /**
         * @brief verify certificate revocation using CRL.
         * @param context pointer to the complete context used for the certificate chain verification.
         * @return when verified successfully, the callback should return 1, 0 otherwise.
         */
        /*int verifyCrl ([[maybe_unused]]X509_STORE_CTX *context) const
        {
            return 1;
        }*/

        /**
         * @brief verify certificate revocation using OCSP.
         * @param context pointer to the complete context used for the certificate chain verification.
         * @return when verified successfully, the callback should return 1, 0 otherwise.
         */
        /*int verifyOcsp ([[maybe_unused]]X509_STORE_CTX *context) const
        {
            return 1;
        }*/

        /// underlying socket.
        UnderlyingSocket _socket;

        /// TLS context.
        TlsContext _ctx;

        /// TLS handle.
        SslPtr _ssl;
    };

    /**
     * @brief compare two TLS decorators based on their underlying socket handle.
     * @param a first TLS decorator.
     * @param b second TLS decorator.
     * @return true if the handle of a is less than the handle of b, false otherwise.
     */
    template <class Protocol>
    inline bool operator< (const BasicTls<Protocol>& a, const BasicTls<Protocol>& b) noexcept
    {
        return a.handle () < b.handle ();
    }
}

#endif
