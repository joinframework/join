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

#ifndef JOIN_FABRIC_MDNS_PROTOCOL_HPP
#define JOIN_FABRIC_MDNS_PROTOCOL_HPP

// libjoin.
#include <join/protocol.hpp>

namespace join
{
    template <class Protocol>
    class BasicDatagramPeer;

    /**
     * @brief Multicast DNS protocol class
     */
    class Mdns
    {
    public:
        using Endpoint = BasicInternetEndpoint<Mdns>;
        using Socket = BasicDatagramSocket<Mdns>;
        using Peer = BasicDatagramPeer<Mdns>;

        /**
         * @brief construct the mDNS protocol instance.
         * @param family IP address family.
         */
        constexpr explicit Mdns (int family = AF_INET) noexcept
        : _family (family)
        {
        }

        /**
         * @brief get protocol suitable for IPv4 address family.
         * @return an IPv4 address family suitable protocol.
         */
        static inline Mdns& v4 () noexcept
        {
            static Mdns mdnsv4 (AF_INET);
            return mdnsv4;
        }

        /**
         * @brief get protocol suitable for IPv6 address family.
         * @return an IPv6 address family suitable protocol.
         */
        static inline Mdns& v6 () noexcept
        {
            static Mdns mdnsv6 (AF_INET6);
            return mdnsv6;
        }

        /**
         * @brief get the protocol IP address family.
         * @return the protocol IP address family.
         */
        constexpr int family () const noexcept
        {
            return _family;
        }

        /**
         * @brief get the protocol communication semantic.
         * @return the protocol communication semantic.
         */
        constexpr int type () const noexcept
        {
            return SOCK_DGRAM;
        }

        /**
         * @brief get the protocol type.
         * @return the protocol type.
         */
        constexpr int protocol () const noexcept
        {
            return IPPROTO_UDP;
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

        /// default DNS port.
        static constexpr uint16_t defaultPort = 5353;

        /// maximum DNS message size.
        static constexpr size_t maxMsgSize = 8192;

    private:
        /// IP address family.
        int _family;
    };

    /**
     * @brief check if equals.
     * @param a protocol to check.
     * @param b protocol to check.
     * @return true if equals.
     */
    constexpr bool operator== (const Mdns& a, const Mdns& b) noexcept
    {
        return a.family () == b.family ();
    }

    /**
     * @brief check if not equals.
     * @param a protocol to check.
     * @param b protocol to check.
     * @return true if not equals.
     */
    constexpr bool operator!= (const Mdns& a, const Mdns& b) noexcept
    {
        return !(a == b);
    }
}

#endif
