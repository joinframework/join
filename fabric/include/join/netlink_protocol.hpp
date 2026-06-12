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

#ifndef JOIN_FABRIC_NETLINK_PROTOCOL_HPP
#define JOIN_FABRIC_NETLINK_PROTOCOL_HPP

// libjoin.
#include <join/netlink_endpoint.hpp>
#include <join/protocol.hpp>

namespace join
{
    /**
     * @brief netlink protocol class.
     */
    class Netlink
    {
    public:
        using Endpoint = BasicNetlinkEndpoint<Netlink>;
        using Socket = BasicDatagramSocket<Netlink>;

        /**
         * @brief construct the netlink protocol instance by default.
         * @param proto protocol type.
         */
        constexpr Netlink (int proto = NETLINK_ROUTE) noexcept
        : _proto (proto)
        {
        }

        /**
         * @brief get protocol suitable for netlink route.
         * @return a netlink route protocol.
         */
        static inline Netlink& rt () noexcept
        {
            static Netlink route (NETLINK_ROUTE);
            return route;
        }

        /**
         * @brief get protocol suitable for netlink netfilter.
         * @return a netlink route protocol.
         */
        static inline Netlink& nf () noexcept
        {
            static Netlink netfilter (NETLINK_NETFILTER);
            return netfilter;
        }

        /**
         * @brief get the protocol address family.
         * @return the protocol address family.
         */
        constexpr int family () const noexcept
        {
            return AF_NETLINK;
        }

        /**
         * @brief get the protocol communication semantic.
         * @return the protocol communication semantic.
         */
        constexpr int type () const noexcept
        {
            return SOCK_RAW;
        }

        /**
         * @brief get the protocol type.
         * @return the protocol type.
         */
        constexpr int protocol () const noexcept
        {
            return _proto;
        }

    private:
        /// protocol.
        int _proto;
    };

    /**
     * @brief check if equals.
     * @param a protocol to check.
     * @param b protocol to check.
     * @return true if equals.
     */
    constexpr bool operator== (const Netlink& a, const Netlink& b) noexcept
    {
        return a.protocol () == b.protocol ();
    }

    /**
     * @brief check if not equals.
     * @param a protocol to check.
     * @param b protocol to check.
     * @return true if not equals.
     */
    constexpr bool operator!= (const Netlink& a, const Netlink& b) noexcept
    {
        return !(a == b);
    }
}

#endif
