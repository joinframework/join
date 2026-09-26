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

#ifndef JOIN_FABRIC_NDP_PROTOCOL_HPP
#define JOIN_FABRIC_NDP_PROTOCOL_HPP

// libjoin.
#include <join/socket.hpp>

namespace join
{
    template <class Protocol>
    class BasicNdp;

    template <class Protocol>
    class BasicNdpClient;

    template <class Protocol>
    class BasicNdpServer;

    /**
     * @brief neighbor discovery protocol class.
     */
    class Ndp
    {
    public:
        using Transport = Icmp;
        using Endpoint = typename Transport::Endpoint;
        using Socket = typename Transport::Socket;
        using Client = BasicNdpClient<Ndp>;
        using Server = BasicNdpServer<Ndp>;

        /**
         * @brief construct the NDP protocol instance.
         */
        constexpr Ndp () noexcept
        : _transport (AF_INET6)
        {
        }

        /**
         * @brief get the protocol IP address family.
         * @return the protocol IP address family.
         */
        constexpr int family () const noexcept
        {
            return _transport.family ();
        }

        /**
         * @brief get the protocol communication semantic.
         * @return the protocol communication semantic.
         */
        constexpr int type () const noexcept
        {
            return _transport.type ();
        }

        /**
         * @brief get the protocol type.
         * @return the protocol type.
         */
        constexpr int protocol () const noexcept
        {
            return _transport.protocol ();
        }

        /// hop limit every neighbor discovery message is sent and received with, RFC 4861 section 6.1.
        static constexpr int hopLimit = 255;

        /// maximum NDP message size, the biggest payload an IPv6 packet can carry.
        static constexpr size_t maxMsgSize = 65535;

    private:
        /// underlying transport protocol.
        Transport _transport;
    };

    /**
     * @brief check if equals.
     * @param a protocol to check.
     * @param b protocol to check.
     * @return true if equals.
     */
    constexpr bool operator== (const Ndp& a, const Ndp& b) noexcept
    {
        return a.family () == b.family ();
    }

    /**
     * @brief check if not equals.
     * @param a protocol to check.
     * @param b protocol to check.
     * @return true if not equals.
     */
    constexpr bool operator!= (const Ndp& a, const Ndp& b) noexcept
    {
        return !(a == b);
    }
}

#endif
