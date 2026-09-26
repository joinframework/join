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

#ifndef JOIN_FABRIC_NDP_MESSAGE_HPP
#define JOIN_FABRIC_NDP_MESSAGE_HPP

// libjoin.
#include <join/mac_address.hpp>
#include <join/utils.hpp>
#include <join/ip_address.hpp>
#include <join/error.hpp>

// C++.
#include <sstream>
#include <vector>

// C.
#include <netinet/icmp6.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <cstring>

namespace join
{
    /**
     * @brief prefix carried by a router advertisement, RFC 4861 section 4.6.2.
     */
    struct NdpPrefix
    {
        /// prefix.
        IpAddress prefix{AF_INET6};

        /// number of leading bits of the prefix that are valid.
        uint8_t length = 64;

        /// prefix flags.
        uint8_t flags = ND_OPT_PI_FLAG_ONLINK | ND_OPT_PI_FLAG_AUTO;

        /// time in seconds the prefix is valid for on-link determination.
        uint32_t valid = 2592000;

        /// time in seconds addresses generated from the prefix remain preferred.
        uint32_t preferred = 604800;
    };

    /**
     * @brief recursive DNS servers carried by a router advertisement, RFC 8106 section 5.1.
     */
    struct NdpRdnss
    {
        /// time in seconds the servers may be used for.
        uint32_t lifetime = 1800;

        /// server addresses.
        std::vector<IpAddress> servers;
    };

    /**
     * @brief router solicitation, RFC 4861 section 4.1.
     */
    struct RouterSolicitation
    {
        /// address the solicitation came from, set by the transport.
        IpAddress src{AF_INET6};

        /// link layer address of the sender, wildcard if not carried.
        MacAddress link;
    };

    /**
     * @brief router advertisement, RFC 4861 section 4.2.
     */
    struct RouterAdvertisement
    {
        /// address the advertisement came from, set by the transport.
        IpAddress src{AF_INET6};

        /// hop limit hosts should use, zero if unspecified.
        uint8_t hopLimit = 0;

        /// router flags.
        uint8_t flags = 0;

        /// lifetime in seconds of the router as a default router, zero if it is not one.
        uint16_t lifetime = 0;

        /// time in milliseconds a neighbor is assumed reachable, zero if unspecified.
        uint32_t reachable = 0;

        /// time in milliseconds between retransmitted neighbor solicitations, zero if unspecified.
        uint32_t retransmit = 0;

        /// link layer address of the router, wildcard if not carried.
        MacAddress link;

        /// link MTU, zero if not carried.
        uint32_t mtu = 0;

        /// prefixes.
        std::vector<NdpPrefix> prefixes;

        /// recursive DNS servers, one entry per option.
        std::vector<NdpRdnss> rdnss;
    };

    /**
     * @brief NDP message codec.
     */
    class NdpMessage
    {
    public:
        /**
         * @brief NDP message types, RFC 4861 section 4.
         */
        enum MessageType : uint8_t
        {
            RouterSolicit = ND_ROUTER_SOLICIT, /**< a host looking for routers. */
            RouterAdvert = ND_ROUTER_ADVERT,   /**< a router advertising its presence. */
        };

        /**
         * @brief NDP option types.
         */
        enum OptionType : uint8_t
        {
            SourceLinkAddress = ND_OPT_SOURCE_LINKADDR,    /**< link layer address of the sender. */
            PrefixInformation = ND_OPT_PREFIX_INFORMATION, /**< on-link and autoconfiguration prefix. */
            Mtu = ND_OPT_MTU,                              /**< link MTU. */
            RecursiveDnsServer = 25,                       /**< recursive DNS servers, RFC 8106. */
        };

        /**
         * @brief serialize a router solicitation into a byte stream.
         * @param packet router solicitation to serialize.
         * @param data byte stream to serialize the message into.
         * @return 0 on success, -1 on failure.
         */
        int serialize (const RouterSolicitation& packet, std::stringstream& data) const
        {
            struct nd_router_solicit rs = {};
            rs.nd_rs_type = RouterSolicit;
            data.write (reinterpret_cast<const char*> (&rs), sizeof (rs));
            writeLinkAddress (data, packet.link);
            return 0;
        }

        /**
         * @brief serialize a router advertisement into a byte stream.
         * @param packet router advertisement to serialize.
         * @param data byte stream to serialize the message into.
         * @return 0 on success, -1 on failure.
         */
        int serialize (const RouterAdvertisement& packet, std::stringstream& data) const
        {
            return serialize (packet, data, packet.link);
        }

        /**
         * @brief serialize a router advertisement into a byte stream with the given link layer address.
         * @param packet router advertisement to serialize.
         * @param data byte stream to serialize the message into.
         * @param link link layer address to carry instead of the one of the advertisement.
         * @return 0 on success, -1 on failure.
         */
        int serialize (const RouterAdvertisement& packet, std::stringstream& data, const MacAddress& link) const
        {
            for (auto const& prefix : packet.prefixes)
            {
                if ((prefix.prefix.family () != AF_INET6) || (prefix.length > 128))
                {
                    lastError = make_error_code (Errc::InvalidParam);
                    return -1;
                }
            }

            for (auto const& rdnss : packet.rdnss)
            {
                if (rdnss.servers.empty ())
                {
                    lastError = make_error_code (Errc::InvalidParam);
                    return -1;
                }

                if (rdnss.servers.size () > _maxDnsServers)
                {
                    lastError = make_error_code (Errc::MessageTooLong);
                    return -1;
                }

                for (auto const& server : rdnss.servers)
                {
                    if (server.family () != AF_INET6)
                    {
                        lastError = make_error_code (Errc::InvalidParam);
                        return -1;
                    }
                }
            }

            struct nd_router_advert ra = {};
            ra.nd_ra_type = RouterAdvert;
            ra.nd_ra_curhoplimit = packet.hopLimit;
            ra.nd_ra_flags_reserved = packet.flags;
            ra.nd_ra_router_lifetime = htons (packet.lifetime);
            ra.nd_ra_reachable = htonl (packet.reachable);
            ra.nd_ra_retransmit = htonl (packet.retransmit);
            data.write (reinterpret_cast<const char*> (&ra), sizeof (ra));

            writeLinkAddress (data, link);

            if (packet.mtu)
            {
                struct nd_opt_mtu mtu = {};
                mtu.nd_opt_mtu_type = Mtu;
                mtu.nd_opt_mtu_len = sizeof (mtu) >> 3;
                mtu.nd_opt_mtu_mtu = htonl (packet.mtu);
                data.write (reinterpret_cast<const char*> (&mtu), sizeof (mtu));
            }

            for (auto const& prefix : packet.prefixes)
            {
                IpAddress masked = prefix.prefix & IpAddress (prefix.length, AF_INET6);

                struct nd_opt_prefix_info pi = {};
                pi.nd_opt_pi_type = PrefixInformation;
                pi.nd_opt_pi_len = sizeof (pi) >> 3;
                pi.nd_opt_pi_prefix_len = prefix.length;
                pi.nd_opt_pi_flags_reserved = prefix.flags;
                pi.nd_opt_pi_valid_time = htonl (prefix.valid);
                pi.nd_opt_pi_preferred_time = htonl (prefix.preferred);
                ::memcpy (&pi.nd_opt_pi_prefix, masked.addr (), sizeof (pi.nd_opt_pi_prefix));
                data.write (reinterpret_cast<const char*> (&pi), sizeof (pi));
            }

            for (auto const& rdnss : packet.rdnss)
            {
                RdnssHeader header = {};
                header.type = RecursiveDnsServer;
                header.len = static_cast<uint8_t> (1 + (rdnss.servers.size () * 2));
                header.lifetime = htonl (rdnss.lifetime);
                data.write (reinterpret_cast<const char*> (&header), sizeof (header));

                for (auto const& server : rdnss.servers)
                {
                    data.write (reinterpret_cast<const char*> (server.addr ()), sizeof (struct in6_addr));
                }
            }

            return 0;
        }

        /**
         * @brief deserialize a router solicitation from a byte stream.
         * @param packet router solicitation to deserialize into.
         * @param data byte stream holding the message to deserialize.
         * @return 0 on success, -1 on failure.
         */
        int deserialize (RouterSolicitation& packet, std::stringstream& data) const
        {
            struct nd_router_solicit rs = {};
            if (!extract (data, &rs, sizeof (rs)))
            {
                return -1;
            }

            if ((rs.nd_rs_type != RouterSolicit) || (rs.nd_rs_code != 0))
            {
                lastError = make_error_code (Errc::MessageUnknown);
                return -1;
            }

            packet.link = MacAddress ();

            return readOptions (data, [&packet] (const uint8_t* option, size_t size) {
                if ((option[0] == SourceLinkAddress) && (size >= _optionHeaderSize + ETH_ALEN))
                {
                    packet.link = MacAddress (option + _optionHeaderSize, ETH_ALEN);
                }
            });
        }

        /**
         * @brief deserialize a router advertisement from a byte stream.
         * @param packet router advertisement to deserialize into.
         * @param data byte stream holding the message to deserialize.
         * @return 0 on success, -1 on failure.
         */
        int deserialize (RouterAdvertisement& packet, std::stringstream& data) const
        {
            struct nd_router_advert ra = {};
            if (!extract (data, &ra, sizeof (ra)))
            {
                return -1;
            }

            if ((ra.nd_ra_type != RouterAdvert) || (ra.nd_ra_code != 0))
            {
                lastError = make_error_code (Errc::MessageUnknown);
                return -1;
            }

            packet.hopLimit = ra.nd_ra_curhoplimit;
            packet.flags = ra.nd_ra_flags_reserved;
            packet.lifetime = ntohs (ra.nd_ra_router_lifetime);
            packet.reachable = ntohl (ra.nd_ra_reachable);
            packet.retransmit = ntohl (ra.nd_ra_retransmit);
            packet.link = MacAddress ();
            packet.mtu = 0;
            packet.prefixes.clear ();
            packet.rdnss.clear ();

            return readOptions (data, [&packet] (const uint8_t* option, size_t size) {
                switch (option[0])
                {
                    case SourceLinkAddress:
                        if (size >= _optionHeaderSize + ETH_ALEN)
                        {
                            packet.link = MacAddress (option + _optionHeaderSize, ETH_ALEN);
                        }
                        break;

                    case Mtu:
                        if (size == sizeof (struct nd_opt_mtu))
                        {
                            struct nd_opt_mtu mtu;
                            ::memcpy (&mtu, option, sizeof (mtu));
                            packet.mtu = ntohl (mtu.nd_opt_mtu_mtu);
                        }
                        break;

                    case PrefixInformation:
                        if (size == sizeof (struct nd_opt_prefix_info))
                        {
                            struct nd_opt_prefix_info pi;
                            ::memcpy (&pi, option, sizeof (pi));

                            if (pi.nd_opt_pi_prefix_len <= 128)
                            {
                                NdpPrefix prefix;
                                prefix.prefix = IpAddress (&pi.nd_opt_pi_prefix, sizeof (pi.nd_opt_pi_prefix));
                                prefix.length = pi.nd_opt_pi_prefix_len;
                                prefix.flags = pi.nd_opt_pi_flags_reserved;
                                prefix.valid = ntohl (pi.nd_opt_pi_valid_time);
                                prefix.preferred = ntohl (pi.nd_opt_pi_preferred_time);
                                packet.prefixes.push_back (std::move (prefix));
                            }
                        }
                        break;

                    case RecursiveDnsServer:
                        if ((size > sizeof (RdnssHeader)) &&
                            (((size - sizeof (RdnssHeader)) % sizeof (struct in6_addr)) == 0))
                        {
                            RdnssHeader header;
                            ::memcpy (&header, option, sizeof (header));

                            NdpRdnss rdnss;
                            rdnss.lifetime = ntohl (header.lifetime);
                            rdnss.servers.reserve ((size - sizeof (RdnssHeader)) / sizeof (struct in6_addr));

                            for (size_t offset = sizeof (RdnssHeader); offset < size;
                                 offset += sizeof (struct in6_addr))
                            {
                                rdnss.servers.emplace_back (option + offset, sizeof (struct in6_addr));
                            }

                            packet.rdnss.push_back (std::move (rdnss));
                        }
                        break;

                    default:
                        break;
                }
            });
        }

    protected:
        /**
         * @brief recursive DNS server option header, RFC 8106 section 5.1.
         */
        struct __attribute__ ((packed)) RdnssHeader
        {
            uint8_t type;
            uint8_t len;
            uint16_t reserved;
            uint32_t lifetime;
        };

        /**
         * @brief write a link layer address option into a byte stream.
         * @param data byte stream to write to.
         * @param link link layer address, nothing is written if wildcard.
         */
        static void writeLinkAddress (std::ostream& data, const MacAddress& link)
        {
            if (link.isWildcard ())
            {
                return;
            }

            uint8_t option[_optionHeaderSize + ETH_ALEN] = {SourceLinkAddress, 1};
            ::memcpy (option + _optionHeaderSize, link.addr (), ETH_ALEN);
            data.write (reinterpret_cast<const char*> (option), sizeof (option));
        }

        /**
         * @brief walk through the options of a message, RFC 4861 section 4.6.
         * @param data byte stream holding the options.
         * @param handler function called with each option, header included, and its size.
         * @return 0 on success, -1 if an option is malformed.
         */
        template <typename Handler>
        static int readOptions (std::stringstream& data, Handler&& handler)
        {
            uint8_t option[_maxOptionSize];

            for (;;)
            {
                data.read (reinterpret_cast<char*> (option), _optionHeaderSize);
                if (data.gcount () == 0)
                {
                    return 0;
                }

                if ((static_cast<size_t> (data.gcount ()) != _optionHeaderSize) || (option[1] == 0))
                {
                    lastError = make_error_code (Errc::InvalidParam);
                    return -1;
                }

                const size_t size = static_cast<size_t> (option[1]) << 3;
                if (!extract (data, option + _optionHeaderSize, size - _optionHeaderSize))
                {
                    return -1;
                }

                handler (option, size);
            }
        }

        /// size of an option type and length fields.
        static constexpr size_t _optionHeaderSize = 2;

        /// biggest option, its length field counts units of 8 octets.
        static constexpr size_t _maxOptionSize = 255 << 3;

        /// most recursive DNS servers a single option can carry.
        static constexpr size_t _maxDnsServers = 127;
    };
}

#endif
