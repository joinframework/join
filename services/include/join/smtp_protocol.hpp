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

#ifndef JOIN_SERVICES_SMTP_PROTOCOL_HPP
#define JOIN_SERVICES_SMTP_PROTOCOL_HPP

// libjoin.
#include <join/tls_protocol.hpp>

namespace join
{
    template <class Protocol>
    class BasicSmtpClient;

    template <class Protocol>
    class BasicSmtpSecureClient;

    /**
     * @brief SMTP protocol class (SMTP over TCP, upgradable to TLS using STARTTLS).
     */
    class Smtp
    {
    public:
        using Transport = Tcp;
        using Endpoint = typename Transport::Endpoint;
        using Socket = BasicTlsWrapper<Smtp>;
        using Stream = BasicTlsStream<Smtp>;
        using Client = BasicSmtpClient<Smtp>;

        /**
         * @brief construct the SMTP protocol instance.
         * @param family IP address family.
         */
        constexpr explicit Smtp (int family = AF_INET) noexcept
        : _transport (family)
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
            return _transport.family ();
        }

        /// default SMTP port.
        static constexpr uint16_t defaultPort = 25;

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
     * @brief SMTPS protocol class (SMTP over TLS).
     */
    class Smtps
    {
    public:
        using Transport = Tcp;
        using Endpoint = typename Transport::Endpoint;
        using Socket = BasicTlsWrapper<Smtps>;
        using Stream = BasicTlsStream<Smtps>;
        using Client = BasicSmtpSecureClient<Smtps>;

        /**
         * @brief construct the SMTPS protocol instance.
         * @param family IP address family.
         */
        constexpr explicit Smtps (int family = AF_INET) noexcept
        : _transport (family)
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
            return _transport.family ();
        }

        /// default SMTPS port.
        static constexpr uint16_t defaultPort = 465;

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
