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

#ifndef JOIN_FABRIC_DNS_HPP
#define JOIN_FABRIC_DNS_HPP

// libjoin.
#include <join/resolver.hpp>
#include <join/socket.hpp>

namespace join
{
    /**
     * @brief standard DNS transport
     */
    class Dns
    {
    public:
        using Socket = Udp::Socket;
        using Endpoint = Udp::Endpoint;
        using Resolver = BasicDnsResolver<Dns>;

        /**
         * @brief default constructor.
         */
        Dns () noexcept = default;

        /**
         * @brief destroy instance.
         */
        ~Dns () noexcept
        {
            close ();
        }

        /**
         * @brief create the DNS over UDP transport.
         * @param handler event handler to register to the reactor.
         * @param interface network interface to bind the socket to.
         * @param server remote DNS server address.
         * @param port remote DNS server port (default: 53).
         * @param reactor reactor instance.
         * @return 0 on success, -1 on failure.
         */
        int create (EventHandler* handler, const std::string& interface, const IpAddress& server,
                    uint16_t port = defaultPort, Reactor* reactor = nullptr)
        {
            _handler = handler;
            _interface = interface;
            _server = server;
            _port = port;
            _reactor = reactor ? reactor : ReactorThread::reactor ();

            return reconnect ();
        }

        /**
         * @brief close the DNS over UDP transport.
         */
        void close () noexcept
        {
            if (_reactor && _socket.handle () != -1)
            {
                _reactor->delHandler (_socket.handle ());
            }

            _socket.close ();
        }

        /**
         * @brief reconnect to the remote DNS server.
         * @return 0 on success, -1 on failure.
         */
        int reconnect ()
        {
            close ();

            if ((_socket.bind ({IpAddress (_server.family ()), 0}) == -1) || (_socket.bindToDevice (_interface) == -1))
            {
                _socket.close ();
                return -1;
            }

            if (_socket.connect ({_server, _port}) == -1)
            {
                _socket.close ();
                return -1;
            }

            return _reactor->addHandler (_socket.handle (), _handler);
        }

        /**
         * @brief read DNS message from UDP stream.
         * @param buffer destination buffer.
         * @param maxSize maximum number of bytes to read.
         * @return number of bytes read, or -1 on error.
         */
        int read (uint8_t* buffer, size_t maxSize) noexcept
        {
            return _socket.read (reinterpret_cast<char*> (buffer), maxSize);
        }

        /**
         * @brief write DNS message to UDP stream.
         * @param buffer source buffer.
         * @param size number of bytes to write.
         * @return number of bytes written, -1 on error.
         */
        int write (const uint8_t* buffer, size_t size) noexcept
        {
            return _socket.write (reinterpret_cast<const char*> (buffer), size);
        }

        /**
         * @brief determine if the socket is connected.
         * @return true if connected, false otherwise.
         */
        bool connected () noexcept
        {
            return _socket.connected ();
        }

        /**
         * @brief get the IP address family.
         * @return the IP address family.
         */
        int family () const noexcept
        {
            return _socket.family ();
        }

        /**
         * @brief determine the local endpoint associated with this transport.
         * @return local endpoint.
         */
        Endpoint localEndpoint () const noexcept
        {
            return _socket.localEndpoint ();
        }

        /**
         * @brief determine the remote endpoint associated with this transport.
         * @return remote endpoint.
         */
        Endpoint remoteEndpoint () const noexcept
        {
            return _socket.remoteEndpoint ();
        }

        /// default port.
        static constexpr uint16_t defaultPort = 53;

        /// max message size.
        static constexpr size_t maxMsgSize = 8192;

    private:
        /// event handler registered in the reactor.
        EventHandler* _handler = nullptr;

        /// network interface to bind the socket to.
        std::string _interface;

        /// remote DNS server address.
        IpAddress _server;

        /// remote DNS server port.
        uint16_t _port = defaultPort;

        /// event loop reactor.
        Reactor* _reactor = nullptr;

        /// UDP socket.
        Socket _socket;
    };
}

#endif
