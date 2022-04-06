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

#ifndef __JOIN_PROTOCOL_HPP__
#define __JOIN_PROTOCOL_HPP__

// libjoin.
#include <join/endpoint.hpp>

// C.
#include <net/ethernet.h>

namespace join
{
    template <class Protocol> class BasicSocket;
    template <class Protocol> class BasicDatagramSocket;
    template <class Protocol> class BasicStreamSocket;
    template <class Protocol> class BasicTlsSocket;

    template <class Protocol> class BasicSocketStreambuf;
    template <class Protocol> class BasicSocketStream;
    template <class Protocol> class BasicTlsStream;

    template <class Protocol> class BasicStreamAcceptor;
    template <class Protocol> class BasicTlsAcceptor;

    template <class Protocol> class BasicResolver;

    /**
     * @brief unix datagram protocol class.
     */
    class UnixDgram
    {
    public:
        using Endpoint = BasicUnixEndpoint <UnixDgram>;
        using Socket   = BasicDatagramSocket <UnixDgram>;

        /**
         * @brief construct the unix datagram protocol instance by default.
         */
        constexpr UnixDgram () noexcept = default;

        /**
         * @brief get the protocol ip address family.
         * @return the protocol ip address family.
         */
        constexpr int family () const noexcept
        {
            return AF_UNIX;
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
            return 0;
        }
    };

    /**
     * @brief unix stream protocol class.
     */
    class UnixStream
    {
    public:
        using Endpoint = BasicUnixEndpoint <UnixStream>;
        using Socket   = BasicStreamSocket <UnixStream>;
        using Stream   = BasicSocketStream <UnixStream>;
        using Acceptor = BasicStreamAcceptor <UnixStream>;

        /**
         * @brief construct the unix stream protocol instance by default.
         */
        constexpr UnixStream () noexcept = default;

        /**
         * @brief get the protocol ip address family.
         * @return the protocol ip address family.
         */
        constexpr int family () const noexcept
        {
            return AF_UNIX;
        }

        /**
         * @brief get the protocol communication semantic.
         * @return the protocol communication semantic.
         */
        constexpr int type () const noexcept
        {
            return SOCK_STREAM;
        }

        /**
         * @brief get the protocol type.
         * @return the protocol type.
         */
        constexpr int protocol () const noexcept
        {
            return 0;
        }
    };

    /**
     * @brief RAW protocol class.
     */
    class Raw
    {
    public:
        using Endpoint = BasicLinkLayerEndpoint <Raw>;
        using Socket   = BasicSocket <Raw>;

        /**
         * @brief default constructor.
         */
        constexpr Raw () noexcept = default;

        /**
         * @brief get the protocol ip address family.
         * @return the protocol ip address family.
         */
        constexpr int family () const noexcept
        {
            return AF_PACKET;
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
            if (BYTE_ORDER == LITTLE_ENDIAN)
            {
                return (ETH_P_ALL >> 8) | (ETH_P_ALL << 8);
            }
            return ETH_P_ALL;
        }
    };

    /**
     * @brief UDP protocol class.
     */
    class Udp
    {
    public:
        using Endpoint = BasicInternetEndpoint <Udp>;
        using Socket   = BasicDatagramSocket <Udp>;
        using Resolver = BasicResolver <Udp>;

        /**
         * @brief construct the udp protocol instance.
         * @param family IP address family.
         */
        constexpr Udp (int family = AF_INET) noexcept
        : _family (family)
        {
        }

        /**
         * @brief get protocol suitable for IPv4 address family.
         * @return an IPv4 address family suitable protocol.
         */
        static inline Udp& v4 () noexcept
        {
            static Udp udpv4 (AF_INET);
            return udpv4;
        }

        /**
         * @brief get protocol suitable for IPv6 address family.
         * @return an IPv6 address family suitable protocol.
         */
        static inline Udp& v6 () noexcept
        {
            static Udp udpv6 (AF_INET6);
            return udpv6;
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
    constexpr bool operator== (const Udp& a, const Udp& b) noexcept
    {
        return a.family () == b.family ();
    }

    /**
     * @brief check if not equals.
     * @param a protocol to check.
     * @param b protocol to check.
     * @return true if not equals.
     */
    constexpr bool operator!= (const Udp& a, const Udp& b) noexcept
    {
        return !(a == b);
    }

    /**
     * @brief ICMP protocol class.
     */
    class Icmp
    {
    public:
        using Endpoint = BasicInternetEndpoint <Icmp>;
        using Socket   = BasicDatagramSocket <Icmp>;
        using Resolver = BasicResolver <Icmp>;

        /**
         * @brief create the icmp protocol instance.
         * @param family IP address family.
         */
        constexpr Icmp (int family = AF_INET) noexcept
        : _family (family)
        {
        }

        /**
         * @brief get protocol suitable for IPv4 address family.
         * @return an IPv4 address family suitable protocol.
         */
        static inline Icmp& v4 () noexcept
        {
            static Icmp icmpv4 (AF_INET);
            return icmpv4;
        }

        /**
         * @brief get protocol suitable for IPv6 address family.
         * @return an IPv6 address family suitable protocol.
         */
        static inline Icmp& v6 () noexcept
        {
            static Icmp icmpv6 (AF_INET6);
            return icmpv6;
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
            return SOCK_RAW;
        }

        /**
         * @brief get the protocol type.
         * @return the protocol type.
         */
        constexpr int protocol () const noexcept
        {
            if (family () == AF_INET6)
            {
                return int (IPPROTO_ICMPV6);
            }

            return IPPROTO_ICMP;
        }

    private:
        /// IP address family.
        int _family;
    };

    /**
     * @brief check if equal.
     * @param a protocol to compare.
     * @param b protocol to compare to.
     * @return true if equal.
     */
    constexpr bool operator== (const Icmp& a, const Icmp& b) noexcept
    {
        return a.family () == b.family ();
    }

    /**
     * @brief check if not equal.
     * @param a protocol to compare.
     * @param b protocol to compare to.
     * @return true if not equal.
     */
    constexpr bool operator!= (const Icmp& a, const Icmp& b) noexcept
    {
        return !(a == b);
    }

    /**
     * @brief TCP protocol class.
     */
    class Tcp
    {
    public:
        using Endpoint = BasicInternetEndpoint <Tcp>;
        using Socket   = BasicStreamSocket <Tcp>;
        using Stream   = BasicSocketStream <Tcp>;
        using Acceptor = BasicStreamAcceptor <Tcp>;
        using Resolver = BasicResolver <Tcp>;

        /**
         * @brief create the tcp protocol  instance.
         * @param family IP address family.
         */
        constexpr Tcp (int family = AF_INET) noexcept
        : _family (family)
        {
        }

        /**
         * @brief get protocol suitable for IPv4 address family.
         * @return an IPv4 address family suitable protocol.
         */
        static inline Tcp& v4 () noexcept
        {
            static Tcp tcpv4 (AF_INET);
            return tcpv4;
        }

        /**
         * @brief get protocol suitable for IPv6 address family.
         * @return an IPv6 address family suitable protocol.
         */
        static inline Tcp& v6 () noexcept
        {
            static Tcp tcpv6 (AF_INET6);
            return tcpv6;
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
            return SOCK_STREAM;
        }

        /**
         * @brief get the protocol type.
         * @return the protocol type.
         */
        constexpr int protocol () const noexcept
        {
            return IPPROTO_TCP;
        }

    private:
        /// IP address family.
        int _family;
    };

    /**
     * @brief Check if equals.
     * @param a protocol to check.
     * @param b protocol to check.
     * @return true if equals.
     */
    constexpr bool operator== (const Tcp& a, const Tcp& b) noexcept
    {
        return a.family () == b.family ();
    }

    /**
     * @brief Check if not equals.
     * @param a protocol to check.
     * @param b protocol to check.
     * @return true if not equals.
     */
    constexpr bool operator!= (const Tcp& a, const Tcp& b) noexcept
    {
        return !(a == b);
    }

    /**
     * @brief Tls protocol class.
     */
    class Tls
    {
    public:
        using Endpoint = BasicInternetEndpoint <Tls>;
        using Socket   = BasicTlsSocket <Tls>;
        using Stream   = BasicTlsStream <Tls>;
        using Acceptor = BasicTlsAcceptor <Tls>;
        using Resolver = BasicResolver <Tls>;

        /**
         * @brief create the tcp protocol  instance.
         * @param family IP address family.
         */
        constexpr Tls (int family = AF_INET) noexcept
        : _family (family)
        {
        }

        /**
         * @brief get protocol suitable for IPv4 address family.
         * @return an IPv4 address family suitable protocol.
         */
        static inline Tls& v4 () noexcept
        {
            static Tls tlsv4 (AF_INET);
            return tlsv4;
        }

        /**
         * @brief get protocol suitable for IPv6 address family.
         * @return an IPv6 address family suitable protocol.
         */
        static inline Tls& v6 () noexcept
        {
            static Tls tlsv6 (AF_INET6);
            return tlsv6;
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
            return SOCK_STREAM;
        }

        /**
         * @brief get the protocol type.
         * @return the protocol type.
         */
        constexpr int protocol () const noexcept
        {
            return IPPROTO_TCP;
        }

    private:
        /// IP address family.
        int _family;
    };

    /**
     * @brief Check if equals.
     * @param a protocol to check.
     * @param b protocol to check.
     * @return true if equals.
     */
    constexpr bool operator== (const Tls& a, const Tls& b) noexcept
    {
        return a.family () == b.family ();
    }

    /**
     * @brief Check if not equals.
     * @param a protocol to check.
     * @param b protocol to check.
     * @return true if not equals.
     */
    constexpr bool operator!= (const Tls& a, const Tls& b) noexcept
    {
        return !(a == b);
    }
}

#endif
