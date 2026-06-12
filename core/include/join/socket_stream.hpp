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

#ifndef JOIN_CORE_SOCKET_STREAM_HPP
#define JOIN_CORE_SOCKET_STREAM_HPP

// libjoin.
#include <join/stream_socket.hpp>

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
        using Socket = typename Protocol::Socket;

        /**
         * @brief default constructor.
         */
        BasicSocketStreambuf ()
        : _buf (std::make_unique<char[]> (2 * _bufsize))
        , _socket (Socket::Mode::NonBlocking)
        {
        }

        /**
         * @brief construct the socket stream buffer by moving an existing socket in.
         * @param socket socket to move in.
         */
        explicit BasicSocketStreambuf (Socket&& socket)
        : _buf (std::make_unique<char[]> (2 * _bufsize))
        , _socket (std::move (socket))
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
        : std::streambuf (std::move (other))
        , _buf (std::move (other._buf))
        , _timeout (other._timeout)
        , _socket (std::move (other._socket))
        {
        }

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        BasicSocketStreambuf& operator= (BasicSocketStreambuf&& other)
        {
            close ();

            std::streambuf::operator= (std::move (other));
            _buf = std::move (other._buf);
            _timeout = other._timeout;
            _socket = std::move (other._socket);

            return *this;
        }

        /**
         * @brief destroy the socket stream buffer instance.
         */
        virtual ~BasicSocketStreambuf ()
        {
            if (_socket.connected ())
            {
                overflow (traits_type::eof ());
            }
        }

        /**
         * @brief assigns the specified endpoint to the socket.
         * @param endpoint endpoint to assign to the socket.
         * @return this on success, nullptr on failure.
         */
        BasicSocketStreambuf* bind (const Endpoint& endpoint)
        {
            if (_socket.bind (endpoint) == -1)
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
            if (_socket.connect (endpoint) == -1)
            {
                if (lastError != Errc::TemporaryError)
                {
                    return nullptr;
                }

                if (!_socket.waitConnected (_timeout))
                {
                    _socket.close ();
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
            if (_socket.connected () && (overflow (traits_type::eof ()) == traits_type::eof ()))
            {
                return nullptr;
            }

            if (_socket.disconnect () == -1)
            {
                if (lastError != Errc::TemporaryError)
                {
                    return nullptr;
                }

                if (!_socket.waitDisconnected (_timeout))
                {
                    return nullptr;
                }
            }

            return this;
        }

        /**
         * @brief close the connection.
         * @return this on success, nullptr on failure.
         */
        void close ()
        {
            _socket.close ();
        }

        /**
         * @brief set the socket timeout.
         * @param ms timeout in milliseconds.
         */
        void timeout (int ms)
        {
            _timeout = ms;
        }

        /**
         * @brief get the current timeout in milliseconds.
         * @return the current timeout.
         */
        int timeout () const
        {
            return _timeout;
        }

        /**
         * @brief get the nested socket.
         * @return the nested socket.
         */
        Socket& socket ()
        {
            return _socket;
        }

    protected:
        /**
         * @brief reads characters from the associated input sequence to the get area.
         * @return the value of the character pointed to by the get pointer after the call on success, EOF otherwise.
         */
        virtual int_type underflow () override
        {
            if (!_socket.connected ())
            {
                lastError = make_error_code (Errc::ConnectionClosed);
                return traits_type::eof ();
            }

            if (eback () == nullptr)
            {
                setg (_buf.get (), _buf.get (), _buf.get ());
            }

            if (gptr () == egptr ())
            {
                for (;;)
                {
                    int nread = _socket.read (eback (), _bufsize);
                    if (nread == -1)
                    {
                        if (lastError == Errc::TemporaryError)
                        {
                            if (_socket.waitReadyRead (_timeout))
                            {
                                continue;
                            }
                        }
                        _socket.close ();
                        return traits_type::eof ();
                    }

                    setg (eback (), eback (), eback () + nread);
                    break;
                }
            }

            return traits_type::to_int_type (*gptr ());
        }

        /**
         * @brief writes characters to the associated output sequence from the put area.
         * @param c the character to store in the put area.
         * @return EOF on failure, some other value on success.
         */
        virtual int_type overflow (int_type c = traits_type::eof ()) override
        {
            if (!_socket.connected ())
            {
                lastError = make_error_code (Errc::ConnectionClosed);
                return traits_type::eof ();
            }

            if (pbase () == nullptr)
            {
                setp (_buf.get () + _bufsize, _buf.get () + (2 * _bufsize));
            }

            if ((pptr () == epptr ()) || (c == traits_type::eof ()))
            {
                std::streamsize pending = pptr () - pbase ();
                if (pending)
                {
                    if (_socket.writeExactly (pbase (), pending, _timeout) == -1)
                    {
                        _socket.close ();
                        return traits_type::eof ();
                    }
                }

                setp (pbase (), pbase () + _bufsize);

                if (c == traits_type::eof ())
                {
                    return traits_type::not_eof (c);
                }
            }

            return sputc (traits_type::to_char_type (c));
        }

        /**
         * @brief synchronizes the buffers with the associated character sequence.
         * @return EOF on failure, some other value on success.
         */
        virtual int_type sync () override
        {
            if (!_socket.connected () || (overflow () == traits_type::eof ()))
            {
                return -1;
            }
            return 0;
        }

        /// internal buffer size.
        static const std::streamsize _bufsize = 4096;

        /// internal buffer.
        std::unique_ptr<char[]> _buf;

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
        using SocketStreambuf = BasicSocketStreambuf<Protocol>;
        using Endpoint = typename Protocol::Endpoint;
        using Socket = typename Protocol::Socket;

        /**
         * @brief default constructor.
         */
        BasicSocketStream ()
        : std::iostream (&_sockbuf)
        {
        }

        /**
         * @brief construct the socket stream by moving an existing socket in.
         * @param socket socket to move in.
         */
        explicit BasicSocketStream (Socket&& socket)
        : std::iostream (&_sockbuf)
        , _sockbuf (std::move (socket))
        {
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
        BasicSocketStream& operator= (const BasicSocketStream& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        BasicSocketStream (BasicSocketStream&& other)
        : std::iostream (std::move (other))
        , _sockbuf (std::move (other._sockbuf))
        {
            set_rdbuf (&_sockbuf);
        }

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        BasicSocketStream& operator= (BasicSocketStream&& other)
        {
            std::iostream::operator= (std::move (other));
            _sockbuf = std::move (other._sockbuf);
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
            if (_sockbuf.bind (endpoint) == nullptr)
            {
                setstate (std::ios_base::failbit);
            }
        }

        /**
         * @brief make a connection to the given endpoint.
         * @param endpoint endpoint to connect to.
         * @throw std::ios_base::failure.
         */
        virtual void connect (const Endpoint& endpoint)
        {
            if (_sockbuf.connect (endpoint) == nullptr)
            {
                setstate (std::ios_base::failbit);
            }
        }

        /**
         * @brief shutdown the connection.
         * @throw std::ios_base::failure.
         */
        virtual void disconnect ()
        {
            if (_sockbuf.disconnect () == nullptr)
            {
                setstate (std::ios_base::failbit);
            }
        }

        /**
         * @brief close the connection.
         * @throw std::ios_base::failure.
         */
        virtual void close ()
        {
            _sockbuf.close ();
        }

        /**
         * @brief determine the local endpoint associated with this socket.
         * @return local endpoint.
         */
        Endpoint localEndpoint ()
        {
            return _sockbuf.socket ().localEndpoint ();
        }

        /**
         * @brief determine the remote endpoint associated with this socket.
         * @return remote endpoint.
         */
        Endpoint remoteEndpoint ()
        {
            return _sockbuf.socket ().remoteEndpoint ();
        }

        /**
         * @brief check if the socket is opened.
         * @return true if opened, false otherwise.
         */
        bool opened ()
        {
            return _sockbuf.socket ().opened ();
        }

        /**
         * @brief check if the socket is connected.
         * @return true if connected, false otherwise.
         */
        bool connected ()
        {
            return _sockbuf.socket ().connected ();
        }

        /**
         * @brief set the socket timeout.
         * @param ms timeout in milliseconds.
         */
        void timeout (int ms)
        {
            _sockbuf.timeout (ms);
        }

        /**
         * @brief get the current timeout in milliseconds.
         * @return the current timeout.
         */
        int timeout () const
        {
            return _sockbuf.timeout ();
        }

        /**
         * @brief get the nested socket.
         * @return the nested socket.
         */
        Socket& socket ()
        {
            return _sockbuf.socket ();
        }

    protected:
        /// associated stream buffer.
        SocketStreambuf _sockbuf;
    };
}

#endif
