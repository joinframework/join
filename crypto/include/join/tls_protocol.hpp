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

#ifndef JOIN_CRYPTO_TLS_PROTOCOL_HPP
#define JOIN_CRYPTO_TLS_PROTOCOL_HPP

// libjoin.
#include <join/protocol.hpp>

namespace join
{
    template <class Protocol>
    class BasicTlsWrapper;

    template <class Protocol>
    class BasicDtlsWrapper;

    template <class Protocol>
    class BasicTlsStream;

    /**
     * @brief TLS protocol class (TLS over TCP).
     */
    class Tls
    {
    public:
        using Transport = Tcp;
        using Endpoint = typename Transport::Endpoint;
        using Socket = BasicTlsWrapper<Tls>;
        using Stream = BasicTlsStream<Tls>;

        /**
         * @brief create the tls protocol instance.
         * @param family IP address family.
         */
        constexpr explicit Tls (int family = AF_INET) noexcept
        : _transport (family)
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
            return _transport.family ();
        }

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
     * @brief DTLS protocol class (DTLS over UDP).
     */
    class Dtls
    {
    public:
        using Transport = Udp;
        using Endpoint = typename Transport::Endpoint;
        using Socket = BasicDtlsWrapper<Dtls>;

        /**
         * @brief create the dtls protocol instance.
         * @param family IP address family.
         */
        constexpr explicit Dtls (int family = AF_INET) noexcept
        : _transport (family)
        {
        }

        /**
         * @brief get protocol suitable for IPv4 address family.
         * @return an IPv4 address family suitable protocol.
         */
        static inline Dtls& v4 () noexcept
        {
            static Dtls dtlsv4 (AF_INET);
            return dtlsv4;
        }

        /**
         * @brief get protocol suitable for IPv6 address family.
         * @return an IPv6 address family suitable protocol.
         */
        static inline Dtls& v6 () noexcept
        {
            static Dtls dtlsv6 (AF_INET6);
            return dtlsv6;
        }

        /**
         * @brief get the protocol IP address family.
         * @return the protocol IP address family.
         */
        constexpr int family () const noexcept
        {
            return _transport.family ();
        }

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
    constexpr bool operator== (const Dtls& a, const Dtls& b) noexcept
    {
        return a.family () == b.family ();
    }

    /**
     * @brief check if not equals.
     * @param a protocol to check.
     * @param b protocol to check.
     * @return true if not equals.
     */
    constexpr bool operator!= (const Dtls& a, const Dtls& b) noexcept
    {
        return !(a == b);
    }
}

#endif
