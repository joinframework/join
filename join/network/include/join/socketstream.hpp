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

#ifndef __JOIN_SOCKETSTREAM_HPP__
#define __JOIN_SOCKETSTREAM_HPP__

// libjoin.
#include <join/socket.hpp>

// C++.
#include <streambuf>
#include <utility>
#include <memory>
#include <chrono>

namespace join
{
    /**
     * @brief socket stream buffer class.
     */
    template <class Protocol>
    class BasicSocketStreambuf : public std::streambuf
    {
    public:
        using Endpoint = typename Protocol::Endpoint;
        using Socket   = typename Protocol::Socket;

        /**
         * @brief default constructor.
         */
        BasicSocketStreambuf ()
        : _socket (Socket::Mode::NonBlocking)
        {
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicSocketStreambuf (const BasicSocketStreambuf& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        BasicSocketStreambuf& operator= (const BasicSocketStreambuf& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        BasicSocketStreambuf (BasicSocketStreambuf&& other)
        : std::streambuf (std::move (other)),
          _allocated (other._allocated),
          _buf (other._buf),
          _gsize (other._gsize),
          _psize (other._psize),
          _timeout (other._timeout),
          _socket (std::move (other._socket))
        {
            other._allocated = false;
            other._buf = nullptr;
            other._gsize = BUFSIZ / 2;
            other._psize = BUFSIZ / 2;
            other._timeout = 30000;
        }

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        BasicSocketStreambuf& operator= (BasicSocketStreambuf&& other)
        {
            this->close ();

            std::streambuf::operator= (std::move (other));

            this->_allocated = other._allocated;
            this->_buf = other._buf;
            this->_gsize = other._gsize;
            this->_psize = other._psize;
            this->_timeout = other._timeout;
            this->_socket = std::move (other._socket);

            other._allocated = false;
            other._buf = nullptr;
            other._gsize = BUFSIZ / 2;
            other._psize = BUFSIZ / 2;
            other._timeout = 30000;

            return *this;
        }

        /**
         * @brief destroy the socket stream buffer instance.
         */
        virtual ~BasicSocketStreambuf ()
        {
            if (this->_buf)
            {
                this->overflow (traits_type::eof ());
                this->freeBuffer ();
            }
        }

        /**
         * @brief assigns the specified endpoint to the socket.
         * @param endpoint endpoint to assign to the socket.
         * @return this on success, nullptr on failure.
         */
        BasicSocketStreambuf* bind (const Endpoint& endpoint)
        {
            if (this->_socket.bind (endpoint) == -1)
            {
                return nullptr;
            }

            return this;
        }

        /**
         * @brief make a connection to the given endpoint.
         * @param endpoint endpoint to connect to.
         * @return this on success, nullptr on failure.
         */
        BasicSocketStreambuf* connect (const Endpoint& endpoint)
        {
            this->allocateBuffer ();

            if (this->_socket.connect (endpoint) == -1)
            {
                if (lastError != Errc::TemporaryError)
                {
                    return nullptr;
                }

                if (!this->_socket.waitConnected (this->_timeout))
                {
                    this->_socket.close ();
                    return nullptr;
                }
            }

            return this;
        }

        /**
         * @brief shutdown the connection.
         * @return this on success, nullptr on failure.
         */
        BasicSocketStreambuf* disconnect ()
        {
            if (this->_buf && (this->overflow (traits_type::eof ()) == traits_type::eof ()))
            {
                return nullptr;
            }

            if (this->_socket.disconnect () == -1)
            {
                if (lastError != Errc::TemporaryError)
                {
                    return nullptr;
                }

                if (!this->_socket.waitDisconnected (this->_timeout))
                {
                    return nullptr;
                }
            }

            this->freeBuffer ();

            return this;
        }

        /**
         * @brief close the connection.
         * @return this on success, nullptr on failure.
         */
        void close ()
        {
            this->_socket.close ();
            this->freeBuffer ();
        }

        /**
         * @brief set the socket timeout.
         * @param ms timeout in milliseconds.
         */
        void timeout (int ms)
        {
            this->_timeout = ms;
        }

        /**
         * @brief get the current timeout in milliseconds.
         * @return the current timeout.
         */
        int timeout () const
        {
            return this->_timeout;
        }

        /**
         * @brief get the nested socket.
         * @return the nested socket.
         */
        Socket& socket ()
        {
            return this->_socket;
        }

    protected:
        /**
         * @brief reads characters from the associated input sequence to the get area.
         * @return the value of the character pointed to by the get pointer after the call on success, EOF otherwise.
         */
        virtual int_type underflow () override
        {
            if (!this->_socket.connected ())
            {
                lastError = make_error_code (Errc::ConnectionClosed);
                return traits_type::eof ();
            }

            if (this->eback () == nullptr)
            {
                this->setg (this->_buf, this->_buf, this->_buf);
            }

            if (this->gptr () == this->egptr ())
            {
                for (;;)
                {
                    int nread = this->_socket.read (this->eback (), this->_gsize);
                    if (nread == -1)
                    {
                        if (lastError == Errc::TemporaryError)
                        {
                            if (this->_socket.waitReadyRead (this->_timeout))
                            {
                                continue;
                            }
                        }
                        this->_socket.close();
                        return traits_type::eof ();
                    }

                    this->setg (this->eback (), this->eback (), this->eback () + nread);
                    break;
                }
            }

            return traits_type::to_int_type (*this->gptr ());
        }

        /**
         * @brief puts a character back into the input sequence.
         * @param c character to put back or EOF if only back out is requested.
         * @return EOF on failure, some other value on success.
         */
        virtual int_type pbackfail (int_type c = traits_type::eof ()) override
        {
            if (this->eback () < this->gptr ())
            {
                this->gbump (-1);

                if (traits_type::eq_int_type (c, traits_type::eof ()))
                {
                    return traits_type::not_eof (c);
                }

                if (!traits_type::eq (traits_type::to_char_type (c), this->gptr ()[-1]))
                {
                    *this->gptr () = traits_type::to_char_type (c);
                }

                return c;
            }

            return traits_type::eof ();
        }

        /**
         * @brief writes characters to the associated output sequence from the put area.
         * @param c the character to store in the put area.
         * @return EOF on failure, some other value on success.
         */
        virtual int_type overflow (int_type c = traits_type::eof ()) override
        {
            if (!this->_socket.connected ())
            {
                lastError = make_error_code (Errc::ConnectionClosed);
                return traits_type::eof ();
            }

            if (this->pbase () == nullptr)
            {
                this->setp (this->_buf + this->_gsize, this->_buf + this->_gsize + this->_psize);
            }

            if ((this->pptr () < this->epptr ()) && (c != traits_type::eof ()))
            {
                return this->sputc (traits_type::to_char_type (c));
            }

            std::streamsize pending = this->pptr () - this->pbase ();

            if (pending && this->_socket.writeExactly (this->pbase (), pending, this->_timeout) == -1)
            {
                this->_socket.close ();
                return traits_type::eof ();
            }

            this->setp (this->pbase (), this->pbase () + this->_psize);

            if (c == traits_type::eof ())
            {
                return traits_type::not_eof (c);
            }

            return this->sputc (traits_type::to_char_type (c));
        }

        /**
         * @brief synchronizes the buffers with the associated character sequence.
         * @return EOF on failure, some other value on success.
         */
        virtual int_type sync () override
        {
            if (this->_buf)
            {
                return this->overflow (traits_type::eof ());
            }

            return traits_type::not_eof (traits_type::eof ());
        }

        /**
         * @brief sets the position indicator of the input and/or output sequence relative to some other position.
         * @param off relative position to set the position indicator to.
         * @param dir defines base position to apply the relative offset to.
         * @param which defines which of the input and/or output sequences to affect.
         * @return .
         */
        virtual pos_type seekoff (off_type off, std::ios_base::seekdir way, std::ios_base::openmode mode = std::ios_base::in) override
        {
            if (!this->_socket.connected () || (mode == std::ios_base::out))
            {
                return pos_type (off_type (-1));
            }

            if (way == std::ios_base::beg)
            {
                if ((off < 0) || (off > (this->egptr() - this->eback ())))
                    return pos_type (off_type (-1));

                this->setg (this->eback (), this->eback () + off, this->egptr ());
            }
            else if (way == std::ios_base::end)
            {
                if ((off > 0) || (-off > (this->egptr () - this->eback ())))
                    return pos_type (off_type (-1));

                this->setg (this->eback (), this->egptr () + off, this->egptr ());
            }
            else
            {
                if (((off < 0) && (-off > (this->egptr () - this->eback ()))) ||
                    ((off > 0) && ( off > (this->egptr () - this->eback ()))))
                    return pos_type (off_type (-1));

                this->gbump (off);
            }

            return this->gptr () - this->eback ();
        }

        /**
         * @brief repositions stream if possible.
         * @param pos stream position.
         * @param which defines whether the input sequences, the output sequence, or both are affected.
         * @return the resultant offset converted to pos_type on success or pos_type (off_type (-1)) on failure.
         */
        virtual pos_type seekpos (pos_type pos, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out) override
        {
            return this->seekoff (off_type (pos), std::ios_base::beg, mode);
        }

        /**
         * @brief replaces the buffer with user-defined array.
         * @param s pointer to the user-provided buffer.
         * @param n the number of elements in the user-provided buffer.
         * @return this on success, nullptr on failure..
         */
        virtual std::streambuf* setbuf (char* s, std::streamsize n) override
        {
            if (!this->_socket.connected ())
            {
                if ((s == nullptr) && (n == 0))
                {
                    this->_gsize = 1;
                    this->_psize = 1;
                    this->_buf = s;
                }
                else
                {
                    auto d = std::ldiv (n, 2);
                    this->_gsize = d.quot + d.rem;
                    this->_psize = d.quot;
                    this->_buf = s;
                }
            }

            return this;
        }

        /**
         * @brief allocate internal buffer.
         */
        void allocateBuffer ()
        {
            if (!this->_buf)
            {
                this->_buf = new char [this->_gsize + this->_psize];
                this->_allocated = true;
            }
        }

        /**
         * @brief free internal buffer.
         */
        void freeBuffer ()
        {
            if (this->_allocated)
            {
                delete [] this->_buf;
                this->_allocated = false;
                this->_buf = nullptr;
                this->setg (nullptr, nullptr, nullptr);
                this->setp (nullptr, nullptr);
            }
        }

        /// internal buffer status.
        bool _allocated = false;

        /// internal buffer.
        char* _buf = nullptr;

        /// internal buffer get area size.
        std::streamsize _gsize = BUFSIZ / 2;

        /// internal buffer put area size.
        std::streamsize _psize = BUFSIZ / 2;

        /// timeout.
        int _timeout = 30000;

        /// internal socket.
        Socket _socket;
    };

    /**
     * @brief socket stream class.
     */
    template <class Protocol>
    class BasicSocketStream : public std::iostream
    {
    public:
        using SocketStreambuf = BasicSocketStreambuf <Protocol>;
        using Endpoint        = typename Protocol::Endpoint;
        using Socket          = typename Protocol::Socket;

        /**
         * @brief default constructor.
         */
        BasicSocketStream ()
        : std::iostream (&_sockbuf)
        {
            this->setf (std::ios_base::unitbuf);
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicSocketStream (const BasicSocketStream& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        BasicSocketStream& operator=(const BasicSocketStream& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        BasicSocketStream (BasicSocketStream&& other)
        : std::iostream (std::move (other)),
          _sockbuf (std::move (other._sockbuf))
        {
            this->set_rdbuf (&this->_sockbuf);
        }

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        BasicSocketStream& operator=(BasicSocketStream&& other)
        {
            std::iostream::operator= (std::move (other));
            this->_sockbuf = std::move (other._sockbuf);
            return *this;
        }

        /**
         * @brief destroy the socket stream instance.
         */
        virtual ~BasicSocketStream () = default;

        /**
         * @brief assigns the specified endpoint to the socket.
         * @param endpoint endpoint to assign to the socket.
         * @throw std::ios_base::failure.
         */
        virtual void bind (const Endpoint& endpoint)
        {
            if (this->_sockbuf.bind (endpoint) == nullptr)
            {
                this->setstate (std::ios_base::failbit);
            }
        }

        /**
         * @brief make a connection to the given endpoint.
         * @param endpoint endpoint to connect to.
         * @throw std::ios_base::failure.
         */
        virtual void connect (const Endpoint& endpoint)
        {
            if (this->_sockbuf.connect (endpoint) == nullptr)
            {
                this->setstate (std::ios_base::failbit);
            }
        }

        /**
         * @brief shutdown the connection.
         * @throw std::ios_base::failure.
         */
        virtual void disconnect ()
        {
            if (this->_sockbuf.disconnect () == nullptr)
            {
                this->setstate (std::ios_base::failbit);
            }
        }

        /**
         * @brief close the connection.
         * @throw std::ios_base::failure.
         */
        virtual void close ()
        {
            this->_sockbuf.close ();
        }

        /**
         * @brief determine the local endpoint associated with this socket.
         * @return local endpoint.
         */
        Endpoint localEndpoint ()
        {
            return this->_sockbuf.socket ().localEndpoint ();
        }

        /**
         * @brief determine the remote endpoint associated with this socket.
         * @return remote endpoint.
         */
        const Endpoint& remoteEndpoint ()
        {
            return this->_sockbuf.socket ().remoteEndpoint ();
        }

        /**
         * @brief check if the socket is opened.
         * @return true if opened, false otherwise.
         */
        bool opened ()
        {
            return this->_sockbuf.socket ().opened ();
        }

        /**
         * @brief check if the socket is connected.
         * @return true if connected, false otherwise.
         */
        bool connected ()
        {
            return this->_sockbuf.socket ().connected ();
        }

        /**
         * @brief check if the socket is secure.
         * @return true if the socket is secure, false otherwise.
         */
        bool encrypted ()
        {
            return this->_sockbuf.socket ().encrypted ();
        }

        /**
         * @brief set the socket timeout.
         * @param ms timeout in milliseconds.
         */
        void timeout (int ms)
        {
            this->_sockbuf.timeout (ms);
        }

        /**
         * @brief get the current timeout in milliseconds.
         * @return the current timeout.
         */
        int timeout () const
        {
            return this->_sockbuf.timeout ();
        }

        /**
         * @brief get the nested socket.
         * @return the nested socket.
         */
        Socket& socket ()
        {
            return this->_sockbuf.socket ();
        }

    protected:
        /// associated stream buffer.
        SocketStreambuf _sockbuf;
    };

    /**
     * @brief TLS stream class.
     */
    template <class Protocol>
    class BasicTlsStream : public BasicSocketStream <Protocol>
    {
    public:
        using SocketStreambuf = BasicSocketStreambuf <Protocol>;
        using Endpoint        = typename Protocol::Endpoint;
        using Socket          = typename Protocol::Socket;

        /**
         * @brief default constructor.
         */
        BasicTlsStream ()
        : BasicSocketStream <Protocol> ()
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
        BasicTlsStream& operator=(const BasicTlsStream& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        BasicTlsStream (BasicTlsStream&& other)
        : BasicSocketStream <Protocol> (std::move (other))
        {
        }

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        BasicTlsStream& operator=(BasicTlsStream&& other)
        {
            BasicSocketStream <Protocol>::operator= (std::move (other));

            return *this;
        }

        /**
         * @brief destroy the TLS stream instance.
         */
        virtual ~BasicTlsStream () = default;

        /**
         * @brief start socket encryption (perform TLS handshake).
         * @return 0 on success, -1 on failure.
         */
        void startEncryption ()
        {
            if (this->_sockbuf.socket ().startEncryption () == -1)
            {
                if (lastError == Errc::TemporaryError)
                {
                    if (this->_sockbuf.socket ().waitEncrypted (this->timeout ()))
                        return;
                }

                this->setstate (std::ios_base::failbit);
            }
        }

        /**
         * @brief make an encrypted connection to the given endpoint.
         * @param endpoint endpoint to connect to.
         * @throw std::ios_base::failure.
         */
        void connectEncrypted (const Endpoint& endpoint)
        {
            if (this->_sockbuf.connect (endpoint) == nullptr)
            {
                this->setstate (std::ios_base::failbit);
            }
            else
            {
                this->startEncryption ();
                if (this->fail ())
                {
                    this->close ();
                }
            }
        }

        /**
         * @brief set the certificate and the private key.
         * @param cert certificate path.
         * @param key private key path.
         * @return 0 on success, -1 on failure.
         */
        int setCertificate (const std::string& cert, const std::string& key = "")
        {
            return this->_sockbuf.socket ().setCertificate (cert, key);
        }

        /**
         * @brief set the location of the trusted CA certificates.
         * @param caPath path of the trusted CA certificates.
         * @return 0 on success, -1 on failure.
         */
        int setCaPath (const std::string& caPath)
        {
            return this->_sockbuf.socket ().setCaPath (caPath);
        }

        /**
         * @brief set the location of the trusted CA certificate file.
         * @param caFile path of the trusted CA certificate file.
         * @return 0 on success, -1 on failure.
         */
        int setCaFile (const std::string& caFile)
        {
            return this->_sockbuf.socket ().setCaFile (caFile);
        }

        /**
         * @brief Enable/Disable the verification of the peer certificate.
         * @param verify Enable peer verification if set to true, false otherwise.
         * @param depth The maximum certificate verification depth (default: no limit).
         */
        void setVerify (bool verify, int depth = -1)
        {
            return this->_sockbuf.socket ().setVerify (verify, depth);
        }

        /**
         * @brief set the cipher list (TLSv1.2 and below).
         * @param cipher the cipher list.
         * @return 0 on success, -1 on failure.
         */
        int setCipher (const std::string &cipher)
        {
            return this->_sockbuf.socket ().setCipher (cipher);
        }

    #if OPENSSL_VERSION_NUMBER >= 0x10101000L
        /**
         * @brief set the cipher list (TLSv1.3).
         * @param cipher the cipher list.
         * @return 0 on success, -1 on failure.
         */
        int setCipher_1_3 (const std::string &cipher)
        {
            return this->_sockbuf.socket ().setCipher_1_3 (cipher);
        }
    #endif
    };
}

#endif
