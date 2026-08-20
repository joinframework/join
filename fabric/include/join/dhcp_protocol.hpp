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

#ifndef JOIN_FABRIC_DHCP_PROTOCOL_HPP
#define JOIN_FABRIC_DHCP_PROTOCOL_HPP

// libjoin.
#include <join/socket.hpp>

namespace join
{
    template <class Protocol>
    class BasicDhcp;

    template <class Protocol>
    class BasicDhcpClient;

    template <class Protocol>
    class BasicDhcpServer;

    /**
     * @brief dynamic host configuration protocol class.
     */
    class Dhcp
    {
    public:
        using Transport = Raw;
        using Endpoint = typename Transport::Endpoint;
        using Socket = typename Transport::Socket;
        using Client = BasicDhcpClient<Dhcp>;
        using Server = BasicDhcpServer<Dhcp>;

        /**
         * @brief construct the DHCP protocol instance.
         */
        constexpr Dhcp () noexcept = default;

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

        /// port a client listens on.
        static constexpr uint16_t clientPort = 68;

        /// port a server listens on.
        static constexpr uint16_t serverPort = 67;

        /// maximum DHCP message size.
        static constexpr size_t maxMsgSize = 1472;

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
    constexpr bool operator== (const Dhcp& a, const Dhcp& b) noexcept
    {
        return a.family () == b.family ();
    }

    /**
     * @brief check if not equals.
     * @param a protocol to check.
     * @param b protocol to check.
     * @return true if not equals.
     */
    constexpr bool operator!= (const Dhcp& a, const Dhcp& b) noexcept
    {
        return !(a == b);
    }
}

#endif
