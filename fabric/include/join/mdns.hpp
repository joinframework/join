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

#ifndef JOIN_FABRIC_MDNS_HPP
#define JOIN_FABRIC_MDNS_HPP

// libjoin.
#include <join/nameserver.hpp>
#include <join/socket.hpp>

// C.
// #include <linux/filter.h>

namespace join
{
    /**
     * @brief multicast DNS transport
     */
    class Mdns
    {
    public:
        using Socket = Udp::Socket;
        using Endpoint = Udp::Endpoint;
        using Peer = BasicDnsPeer<Mdns>;

        /**
         * @brief default constructor.
         */
        Mdns () noexcept = default;

        /**
         * @brief create the DNS over UDP multicast transport.
         * @param handler event handler to register to the reactor.
         * @param interface network interface to bind the socket to.
         * @param family IP address family (AF_INET or AF_INET6).
         * @param reactor reactor instance.
         * @return 0 on success, -1 on failure.
         */
        int create (EventHandler* handler, const std::string& interface, int family, Reactor* reactor = nullptr)
        {
            _reactor = reactor ? reactor : ReactorThread::reactor ();

            if (_socket.open (family == AF_INET6 ? Udp::v6 () : Udp::v4 ()) == -1)
            {
                return -1;
            }

            if (_socket.setOption (Socket::ReusePort, 1) == -1)
            {
                _socket.close ();
                return -1;
            }

            if ((_socket.bind ({IpAddress (family), defaultPort}) == -1) || (_socket.bindToDevice (interface) == -1))
            {
                _socket.close ();
                return -1;
            }

            unsigned int ifindex = ::if_nametoindex (interface.c_str ());
            if (ifindex == 0)
            {
                lastError = std::make_error_code (static_cast<std::errc> (errno));
                _socket.close ();
                return -1;
            }

            IpAddress maddress = multicastAddress (family);

            if (family == AF_INET6)
            {
                ipv6_mreq mreq{};
                ::memcpy (&mreq.ipv6mr_multiaddr, maddress.addr (), maddress.length ());
                mreq.ipv6mr_interface = ifindex;
                if (setsockopt (_socket.handle (), IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, &mreq, sizeof (mreq)) == -1)
                {
                    lastError = std::make_error_code (static_cast<std::errc> (errno));
                    _socket.close ();
                    return -1;
                }
            }
            else
            {
                ip_mreqn mreq{};
                ::memcpy (&mreq.imr_multiaddr, maddress.addr (), maddress.length ());
                mreq.imr_ifindex = static_cast<int> (ifindex);
                if (setsockopt (_socket.handle (), IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof (mreq)) == -1)
                {
                    lastError = std::make_error_code (static_cast<std::errc> (errno));
                    _socket.close ();
                    return -1;
                }
            }

            if (_socket.setOption (Socket::MulticastLoop, 0) == -1)
            {
                _socket.close ();
                return -1;
            }

            // static struct sock_filter code[] = {
            //     {0x28, 0, 0, 0x0000000c},   {0x15, 0, 6, ETH_P_IP},    {0x30, 0, 0, 0x00000017},
            //     {0x15, 0, 10, IPPROTO_UDP}, {0x28, 0, 0, 0x00000014},  {0x15, 2, 0, defaultPort},
            //     {0x28, 0, 0, 0x00000016},   {0x15, 0, 7, defaultPort}, {0x15, 0, 6, ETH_P_IPV6},
            //     {0x30, 0, 0, 0x00000014},   {0x15, 0, 4, IPPROTO_UDP}, {0x28, 0, 0, 0x00000036},
            //     {0x15, 2, 0, defaultPort},  {0x28, 0, 0, 0x00000038},  {0x15, 0, 1, defaultPort},
            //     {0x6, 0, 0, 0x00040000},    {0x06, 0, 0, 0x00000000},
            // };

            // struct sock_fprog bpf = {
            //     .len = sizeof (code) / sizeof (struct sock_filter),
            //     .filter = code,
            // };

            // if (setsockopt (_socket.handle (), SOL_SOCKET, SO_ATTACH_FILTER, &bpf, sizeof (bpf)) == -1)
            // {
            //     lastError = std::make_error_code (static_cast<std::errc> (errno));
            //     _socket.close ();
            //     return -1;
            // }

            return _reactor->addHandler (_socket.handle (), handler);
        }

        /**
         * @brief close the DNS over UDP multicast transport.
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
         * @brief read DNS message from UDP multicast stream.
         * @param buffer destination buffer.
         * @param maxSize maximum number of bytes to read.
         * @param endpoint endpoint from where data are coming (optional).
         * @return number of bytes received, -1 on error.
         */
        int read (uint8_t* buffer, size_t maxSize, Endpoint* endpoint) noexcept
        {
            return _socket.readFrom (reinterpret_cast<char*> (buffer), maxSize, endpoint);
        }

        /**
         * @brief write DNS message to UDP multicast stream.
         * @param buffer source buffer.
         * @param size number of bytes to write.
         * @param endpoint endpoint where to write the data.
         * @return number of bytes written, -1 on error.
         */
        int write (const uint8_t* buffer, size_t size, const Endpoint& endpoint) noexcept
        {
            return _socket.writeTo (reinterpret_cast<const char*> (buffer), size, endpoint);
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
         * @brief get multicast address for the given address family.
         * @param family IP address family.
         * @return multicast IP address.
         */
        static IpAddress multicastAddress (int family) noexcept
        {
            return (family == AF_INET6) ? "ff02::fb" : "224.0.0.251";
        }

        /// default port.
        static constexpr uint16_t defaultPort = 5353;

        /// max message size.
        static constexpr size_t maxMsgSize = 8192;

    private:
        /// socket.
        Socket _socket;

        /// event loop reactor.
        Reactor* _reactor = nullptr;
    };
}

#endif
