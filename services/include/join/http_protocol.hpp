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

#ifndef JOIN_SERVICES_HTTP_PROTOCOL_HPP
#define JOIN_SERVICES_HTTP_PROTOCOL_HPP

// libjoin.
#include <join/tls_protocol.hpp>

namespace join
{
    template <class Protocol>
    class BasicHttpClient;

    template <class Protocol>
    class BasicHttpSecureClient;

    template <class Protocol>
    class BasicHttpWorker;

    template <class Protocol>
    class BasicHttpServer;

    template <class Protocol>
    class BasicHttpSecureServer;

    /**
     * @brief HTTP protocol class (HTTP over TCP).
     */
    class Http
    {
    public:
        using Endpoint = BasicInternetEndpoint<Http>;
        using Socket = BasicStreamSocket<Http>;
        using Stream = BasicSocketStream<Http>;
        using Acceptor = BasicStreamAcceptor<Http>;
        using Client = BasicHttpClient<Http>;
        using Worker = BasicHttpWorker<Http>;
        using Server = BasicHttpServer<Http>;

        /**
         * @brief construct the HTTP protocol instance.
         * @param family IP address family.
         */
        constexpr explicit Http (int family = AF_INET) noexcept
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

        /// default HTTP port.
        static constexpr uint16_t defaultPort = 80;

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
     * @brief HTTPS protocol class (HTTP over TLS).
     */
    class Https
    {
    public:
        using Transport = Tcp;
        using Endpoint = typename Transport::Endpoint;
        using Socket = BasicTlsWrapper<Https>;
        using Stream = BasicTlsStream<Https>;
        using Acceptor = typename Transport::Acceptor;
        using Client = BasicHttpSecureClient<Https>;
        using Worker = BasicHttpWorker<Https>;
        using Server = BasicHttpSecureServer<Https>;

        /**
         * @brief construct the HTTPS protocol instance.
         * @param family IP address family.
         */
        constexpr explicit Https (int family = AF_INET) noexcept
        : _transport (family)
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
            return _transport.family ();
        }

        /// default HTTPS port.
        static constexpr uint16_t defaultPort = 443;

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
}

#endif
