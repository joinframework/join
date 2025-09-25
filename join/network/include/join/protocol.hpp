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
#include <linux/netlink.h>
#include <net/ethernet.h>

namespace join
{
    template <class Protocol> class BasicSocket;
    template <class Protocol> class BasicDatagramSocket;
    template <class Protocol> class BasicStreamSocket;
    template <class Protocol> class BasicTlsSocket;

    template <class Protocol> class BasicSocketStream;
    template <class Protocol> class BasicTlsStream;

    template <class Protocol> class BasicStreamAcceptor;
    template <class Protocol> class BasicTlsAcceptor;

    template <class Protocol> class BasicHttpClient;
    template <class Protocol> class BasicHttpSecureClient;

    template <class Protocol> class BasicHttpWorker;
    template <class Protocol> class BasicHttpServer;
    template <class Protocol> class BasicHttpSecureServer;

    template <class Protocol> class BasicSmtpClient;
    template <class Protocol> class BasicSmtpSecureClient;

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
     * @brief netlink protocol class.
     */
    class NetLink
    {
    public:
        using Endpoint = BasicNetLinkEndpoint <NetLink>;
        using Socket   = BasicDatagramSocket <NetLink>;

        /**
         * @brief construct the netlink protocol instance by default.
         * @param proto protocol type.
         */
        constexpr NetLink (int proto = NETLINK_ROUTE) noexcept
        : _proto (proto)
        {
        }

        /**
         * @brief get protocol suitable for netlink route.
         * @return a netlink route protocol.
         */
        static inline NetLink& rt () noexcept
        {
            static NetLink rt (NETLINK_ROUTE);
            return rt;
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
     * @brief UDP protocol class.
     */
    class Udp
    {
    public:
        using Endpoint = BasicInternetEndpoint <Udp>;
        using Socket   = BasicDatagramSocket <Udp>;

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
     * @brief check if equals.
     * @param a protocol to check.
     * @param b protocol to check.
     * @return true if equals.
     */
    constexpr bool operator== (const Tcp& a, const Tcp& b) noexcept
    {
        return a.family () == b.family ();
    }

    /**
     * @brief check if not equals.
     * @param a protocol to check.
     * @param b protocol to check.
     * @return true if not equals.
     */
    constexpr bool operator!= (const Tcp& a, const Tcp& b) noexcept
    {
        return !(a == b);
    }

    /**
     * @brief SSL/TLS protocol class.
     */
    class Tls
    {
    public:
        using Endpoint = BasicInternetEndpoint <Tls>;
        using Socket   = BasicTlsSocket <Tls>;
        using Stream   = BasicTlsStream <Tls>;
        using Acceptor = BasicTlsAcceptor <Tls>;

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
     * @brief check if equals.
     * @param a protocol to check.
     * @param b protocol to check.
     * @return true if equals.
     */
    constexpr bool operator== (const Tls& a, const Tls& b) noexcept
    {
        return a.family () == b.family ();
    }

    /**
     * @brief check if not equals.
     * @param a protocol to check.
     * @param b protocol to check.
     * @return true if not equals.
     */
    constexpr bool operator!= (const Tls& a, const Tls& b) noexcept
    {
        return !(a == b);
    }

    /**
     * @brief HTTP protocol class.
     */
    class Http
    {
    public:
        using Endpoint = BasicInternetEndpoint <Http>;
        using Socket   = BasicStreamSocket <Http>;
        using Stream   = BasicSocketStream <Http>;
        using Acceptor = BasicStreamAcceptor <Http>;
        using Client   = BasicHttpClient <Http>;
        using Worker   = BasicHttpWorker <Http>;
        using Server   = BasicHttpServer <Http>;

        /**
         * @brief create the HTTP protocol  instance.
         * @param family IP address family.
         */
        constexpr Http (int family = AF_INET) noexcept
        : _family (family)
        {
        }

        /**
         * @brief get protocol suitable for IPv4 address family.
         * @return an IPv4 address family suitable protocol.
         */
        static inline Http& v4 () noexcept
        {
            static Http httpv4 (AF_INET);
            return httpv4;
        }

        /**
         * @brief get protocol suitable for IPv6 address family.
         * @return an IPv6 address family suitable protocol.
         */
        static inline Http& v6 () noexcept
        {
            static Http httpv6 (AF_INET6);
            return httpv6;
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
     * @brief check if equals.
     * @param a protocol to check.
     * @param b protocol to check.
     * @return true if equals.
     */
    constexpr bool operator== (const Http& a, const Http& b) noexcept
    {
        return a.family () == b.family ();
    }

    /**
     * @brief check if not equals.
     * @param a protocol to check.
     * @param b protocol to check.
     * @return true if not equals.
     */
    constexpr bool operator!= (const Http& a, const Http& b) noexcept
    {
        return !(a == b);
    }

    /**
     * @brief HTTPS protocol class.
     */
    class Https
    {
    public:
        using Endpoint = BasicInternetEndpoint <Https>;
        using Socket   = BasicTlsSocket <Https>;
        using Stream   = BasicTlsStream <Https>;
        using Acceptor = BasicTlsAcceptor <Https>;
        using Client   = BasicHttpSecureClient <Https>;
        using Worker   = BasicHttpWorker <Https>;
        using Server   = BasicHttpSecureServer <Https>;

        /**
         * @brief create the HTTPS protocol  instance.
         * @param family IP address family.
         */
        constexpr Https (int family = AF_INET) noexcept
        : _family (family)
        {
        }

        /**
         * @brief get protocol suitable for IPv4 address family.
         * @return an IPv4 address family suitable protocol.
         */
        static inline Https& v4 () noexcept
        {
            static Https httpsv4 (AF_INET);
            return httpsv4;
        }

        /**
         * @brief get protocol suitable for IPv6 address family.
         * @return an IPv6 address family suitable protocol.
         */
        static inline Https& v6 () noexcept
        {
            static Https httpsv6 (AF_INET6);
            return httpsv6;
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
     * @brief check if equals.
     * @param a protocol to check.
     * @param b protocol to check.
     * @return true if equals.
     */
    constexpr bool operator== (const Https& a, const Https& b) noexcept
    {
        return a.family () == b.family ();
    }

    /**
     * @brief check if not equals.
     * @param a protocol to check.
     * @param b protocol to check.
     * @return true if not equals.
     */
    constexpr bool operator!= (const Https& a, const Https& b) noexcept
    {
        return !(a == b);
    }

    /**
     * @brief SMTP protocol class.
     */
    class Smtp
    {
    public:
        using Endpoint = BasicInternetEndpoint <Smtp>;
        using Socket   = BasicTlsSocket <Smtp>;
        using Stream   = BasicTlsStream <Smtp>;
        using Client   = BasicSmtpClient <Smtp>;

        /**
         * @brief create the SMTP protocol  instance.
         * @param family IP address family.
         */
        constexpr Smtp (int family = AF_INET) noexcept
        : _family (family)
        {
        }

        /**
         * @brief get protocol suitable for IPv4 address family.
         * @return an IPv4 address family suitable protocol.
         */
        static inline Smtp& v4 () noexcept
        {
            static Smtp smtpv4 (AF_INET);
            return smtpv4;
        }

        /**
         * @brief get protocol suitable for IPv6 address family.
         * @return an IPv6 address family suitable protocol.
         */
        static inline Smtp& v6 () noexcept
        {
            static Smtp smtpv6 (AF_INET6);
            return smtpv6;
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
     * @brief check if equals.
     * @param a protocol to check.
     * @param b protocol to check.
     * @return true if equals.
     */
    constexpr bool operator== (const Smtp& a, const Smtp& b) noexcept
    {
        return a.family () == b.family ();
    }

    /**
     * @brief check if not equals.
     * @param a protocol to check.
     * @param b protocol to check.
     * @return true if not equals.
     */
    constexpr bool operator!= (const Smtp& a, const Smtp& b) noexcept
    {
        return !(a == b);
    }

    /**
     * @brief SMTPS protocol class.
     */
    class Smtps
    {
    public:
        using Endpoint = BasicInternetEndpoint <Smtps>;
        using Socket   = BasicTlsSocket <Smtps>;
        using Stream   = BasicTlsStream <Smtps>;
        using Client   = BasicSmtpSecureClient <Smtps>;

        /**
         * @brief create the SMTPS protocol  instance.
         * @param family IP address family.
         */
        constexpr Smtps (int family = AF_INET) noexcept
        : _family (family)
        {
        }

        /**
         * @brief get protocol suitable for IPv4 address family.
         * @return an IPv4 address family suitable protocol.
         */
        static inline Smtps& v4 () noexcept
        {
            static Smtps smtpsv4 (AF_INET);
            return smtpsv4;
        }

        /**
         * @brief get protocol suitable for IPv6 address family.
         * @return an IPv6 address family suitable protocol.
         */
        static inline Smtps& v6 () noexcept
        {
            static Smtps smtpsv6 (AF_INET6);
            return smtpsv6;
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
     * @brief check if equals.
     * @param a protocol to check.
     * @param b protocol to check.
     * @return true if equals.
     */
    constexpr bool operator== (const Smtps& a, const Smtps& b) noexcept
    {
        return a.family () == b.family ();
    }

    /**
     * @brief check if not equals.
     * @param a protocol to check.
     * @param b protocol to check.
     * @return true if not equals.
     */
    constexpr bool operator!= (const Smtps& a, const Smtps& b) noexcept
    {
        return !(a == b);
    }
}

#endif
