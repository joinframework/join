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

#ifndef JOIN_FABRIC_DOT_HPP
#define JOIN_FABRIC_DOT_HPP

// libjoin.
#include <join/resolver.hpp>
#include <join/socket.hpp>

namespace join
{
    /**
     * @brief DNS over TLS transport
     */
    class Dot
    {
    public:
        using Socket = Tls::Socket;
        using Endpoint = Tls::Endpoint;
        using Resolver = BasicDnsResolver<Dot>;

        /**
         * @brief default constructor.
         */
        Dot () noexcept = default;

        /**
         * @brief create the DNS over TLS transport and establish a secure connection.
         * @param handler event handler registered in the reactor.
         * @param interface network interface to bind the socket to.
         * @param server remote DNS server address.
         * @param port remote DNS server port (default: 853).
         * @param reactor reactor instance (optional).
         * @return 0 on success, -1 on failure.
         */
        int create (EventHandler* handler, const std::string& interface, const IpAddress& server,
                    uint16_t port = defaultPort, Reactor* reactor = nullptr)
        {
            _reactor = reactor ? reactor : ReactorThread::reactor ();

            if ((_socket.bind ({IpAddress (server.family ()), 0}) == -1) || (_socket.bindToDevice (interface) == -1))
            {
                _socket.close ();
                return -1;
            }

            if (_socket.connectEncrypted ({server, port}) == -1)
            {
                if (lastError != Errc::TemporaryError)
                {
                    _socket.close ();
                    return -1;
                }

                if (!_socket.waitEncrypted ())
                {
                    _socket.close ();
                    return -1;
                }
            }

            return _reactor->addHandler (_socket.handle (), handler);
        }

        /**
         * @brief close the DNS over TLS transport.
         * @param handler event handler to unregister from the reactor.
         */
        void close ([[maybe_unused]] EventHandler* handler) noexcept
        {
            if (_reactor && _socket.handle () != -1)
            {
                _reactor->delHandler (_socket.handle ());
            }

            _socket.close ();
        }

        /**
         * @brief read DNS message from TLS stream.
         * @param buffer destination buffer.
         * @param maxSize maximum number of bytes to read.
         * @return number of bytes read, or -1 on error.
         */
        int read (uint8_t* buffer, size_t maxSize)
        {
            if (_offset < _headerSize)
            {
                int nread = _socket.read (reinterpret_cast<char*> (buffer) + _offset, _headerSize - _offset);
                if (nread == -1)
                {
                    if (lastError != Errc::TemporaryError)
                    {
                        _offset = 0;
                        _size = 0;
                    }
                    return -1;
                }

                _offset += static_cast<size_t> (nread);

                if (_offset < _headerSize)
                {
                    lastError = make_error_code (Errc::TemporaryError);
                    return -1;
                }

                _size = ntohs (*reinterpret_cast<uint16_t*> (buffer));

                if (_size > maxSize)
                {
                    lastError = make_error_code (Errc::MessageTooLong);
                    _offset = 0;
                    _size = 0;
                    return -1;
                }
            }

            int nread = _socket.read (reinterpret_cast<char*> (buffer) + (_offset - _headerSize),
                                      _size - (_offset - _headerSize));
            if (nread == -1)
            {
                if (lastError != Errc::TemporaryError)
                {
                    _offset = 0;
                    _size = 0;
                }
                return -1;
            }

            _offset += static_cast<size_t> (nread);

            if (_offset < (_size + _headerSize))
            {
                lastError = make_error_code (Errc::TemporaryError);
                return -1;
            }

            int msgLen = static_cast<int> (_size);
            _offset = 0;
            _size = 0;

            return msgLen;
        }

        /**
         * @brief write DNS message to TLS stream.
         * @param buffer source buffer.
         * @param size number of bytes to write.
         * @return number of bytes written, -1 on error.
         */
        int write (const uint8_t* buffer, size_t size) noexcept
        {
            uint16_t msgLength = htons (static_cast<uint16_t> (size));

            if (_socket.writeExactly (reinterpret_cast<const char*> (&msgLength), sizeof (msgLength)) == -1)
            {
                return -1;
            }

            if (_socket.writeExactly (reinterpret_cast<const char*> (buffer), size) == -1)
            {
                return -1;
            }

            return static_cast<int> (size);
        }

        /**
         * @brief get the IP address family.
         * @return the IP address family.
         */
        int family () const
        {
            return _socket.family ();
        }

        /**
         * @brief determine the local endpoint associated with this transport.
         * @return local endpoint.
         */
        Endpoint localEndpoint () const
        {
            return _socket.localEndpoint ();
        }

        /**
         * @brief determine the remote endpoint associated with this transport.
         * @return remote endpoint.
         */
        Endpoint remoteEndpoint () const
        {
            return _socket.remoteEndpoint ();
        }

        /// default port.
        static constexpr uint16_t defaultPort = 853;

        /// max message size.
        static constexpr size_t maxMsgSize = 16384;

    private:
        /// .
        static constexpr size_t _headerSize = 2;

        // total expected payload size.
        size_t _size = 0;

        /// current position in the buffer.
        size_t _offset = 0;

        /// socket.
        Socket _socket;

        /// event loop reactor.
        Reactor* _reactor = nullptr;
    };
}

#endif
