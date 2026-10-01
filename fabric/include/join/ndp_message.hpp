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
     * @brief neighbor solicitation, RFC 4861 section 4.3.
     */
    struct NeighborSolicitation
    {
        /// address the solicitation came from, set by the transport.
        IpAddress src{AF_INET6};

        /// address of the solicited neighbor.
        IpAddress target{AF_INET6};

        /// link layer address of the sender, wildcard if not carried.
        MacAddress link;
    };

    /**
     * @brief neighbor advertisement, RFC 4861 section 4.4.
     */
    struct NeighborAdvertisement
    {
        /// address the advertisement came from, set by the transport.
        IpAddress src{AF_INET6};

        /// advertisement flags, in network byte order as ND_NA_FLAG_* are defined.
        uint32_t flags = 0;

        /// address the advertisement is about.
        IpAddress target{AF_INET6};

        /// link layer address of the target, wildcard if not carried.
        MacAddress link;
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
            RouterSolicit = ND_ROUTER_SOLICIT,     /**< a host looking for routers. */
            RouterAdvert = ND_ROUTER_ADVERT,       /**< a router advertising its presence. */
            NeighborSolicit = ND_NEIGHBOR_SOLICIT, /**< a node resolving a neighbor or checking its reachability. */
            NeighborAdvert = ND_NEIGHBOR_ADVERT,   /**< a node answering a solicitation or announcing a change. */
        };

        /**
         * @brief NDP option types.
         */
        enum OptionType : uint8_t
        {
            SourceLinkAddress = ND_OPT_SOURCE_LINKADDR,    /**< link layer address of the sender. */
            TargetLinkAddress = ND_OPT_TARGET_LINKADDR,    /**< link layer address of the target. */
            PrefixInformation = ND_OPT_PREFIX_INFORMATION, /**< on-link and autoconfiguration prefix. */
            Mtu = ND_OPT_MTU,                              /**< link MTU. */
            RecursiveDnsServer = 25,                       /**< recursive DNS servers, RFC 8106. */
        };

        /**
         * @brief serialize a router solicitation into a buffer.
         * @param packet router solicitation to serialize.
         * @param data buffer to serialize the message into.
         * @param maxSize buffer size.
         * @return the message size, -1 on failure.
         */
        ssize_t serialize (const RouterSolicitation& packet, char* data, size_t maxSize) const
        {
            char* cur = data;
            const char* end = data + maxSize;
            bool ok = true;

            struct nd_router_solicit rs = {};
            rs.nd_rs_type = RouterSolicit;
            ok &= writeBytes (cur, end, &rs, sizeof (rs));
            ok &= writeLinkAddress (cur, end, packet.link);

            return ok ? cur - data : -1;
        }

        /**
         * @brief serialize a router advertisement into a buffer.
         * @param packet router advertisement to serialize.
         * @param data buffer to serialize the message into.
         * @param maxSize buffer size.
         * @return the message size, -1 on failure.
         */
        ssize_t serialize (const RouterAdvertisement& packet, char* data, size_t maxSize) const
        {
            return serialize (packet, data, maxSize, packet.link);
        }

        /**
         * @brief serialize a router advertisement into a buffer with the given link layer address.
         * @param packet router advertisement to serialize.
         * @param data buffer to serialize the message into.
         * @param maxSize buffer size.
         * @param link link layer address to carry instead of the one of the advertisement.
         * @return the message size, -1 on failure.
         */
        ssize_t serialize (const RouterAdvertisement& packet, char* data, size_t maxSize, const MacAddress& link) const
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

            char* cur = data;
            const char* end = data + maxSize;
            bool ok = true;

            struct nd_router_advert ra = {};
            ra.nd_ra_type = RouterAdvert;
            ra.nd_ra_curhoplimit = packet.hopLimit;
            ra.nd_ra_flags_reserved = packet.flags;
            ra.nd_ra_router_lifetime = htons (packet.lifetime);
            ra.nd_ra_reachable = htonl (packet.reachable);
            ra.nd_ra_retransmit = htonl (packet.retransmit);
            ok &= writeBytes (cur, end, &ra, sizeof (ra));

            ok &= writeLinkAddress (cur, end, link);

            if (packet.mtu)
            {
                struct nd_opt_mtu mtu = {};
                mtu.nd_opt_mtu_type = Mtu;
                mtu.nd_opt_mtu_len = sizeof (mtu) >> 3;
                mtu.nd_opt_mtu_mtu = htonl (packet.mtu);
                ok &= writeBytes (cur, end, &mtu, sizeof (mtu));
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
                ok &= writeBytes (cur, end, &pi, sizeof (pi));
            }

            for (auto const& rdnss : packet.rdnss)
            {
                RdnssHeader header = {};
                header.type = RecursiveDnsServer;
                header.len = static_cast<uint8_t> (1 + (rdnss.servers.size () * 2));
                header.lifetime = htonl (rdnss.lifetime);
                ok &= writeBytes (cur, end, &header, sizeof (header));

                for (auto const& server : rdnss.servers)
                {
                    ok &= writeBytes (cur, end, server.addr (), sizeof (struct in6_addr));
                }
            }

            return ok ? cur - data : -1;
        }

        /**
         * @brief serialize a neighbor solicitation into a buffer.
         * @param packet neighbor solicitation to serialize.
         * @param data buffer to serialize the message into.
         * @param maxSize buffer size.
         * @return the message size, -1 on failure.
         */
        ssize_t serialize (const NeighborSolicitation& packet, char* data, size_t maxSize) const
        {
            if ((packet.target.family () != AF_INET6) || packet.target.isWildcard () || packet.target.isMulticast ())
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            char* cur = data;
            const char* end = data + maxSize;
            bool ok = true;

            struct nd_neighbor_solicit ns = {};
            ns.nd_ns_type = NeighborSolicit;
            ::memcpy (&ns.nd_ns_target, packet.target.addr (), sizeof (ns.nd_ns_target));
            ok &= writeBytes (cur, end, &ns, sizeof (ns));
            ok &= writeLinkAddress (cur, end, packet.link);

            return ok ? cur - data : -1;
        }

        /**
         * @brief serialize a neighbor advertisement into a buffer.
         * @param packet neighbor advertisement to serialize.
         * @param data buffer to serialize the message into.
         * @param maxSize buffer size.
         * @return the message size, -1 on failure.
         */
        ssize_t serialize (const NeighborAdvertisement& packet, char* data, size_t maxSize) const
        {
            return serialize (packet, data, maxSize, packet.link);
        }

        /**
         * @brief serialize a neighbor advertisement into a buffer with the given link layer address.
         * @param packet neighbor advertisement to serialize.
         * @param data buffer to serialize the message into.
         * @param maxSize buffer size.
         * @param link link layer address to carry instead of the one of the advertisement.
         * @return the message size, -1 on failure.
         */
        ssize_t serialize (const NeighborAdvertisement& packet, char* data, size_t maxSize,
                           const MacAddress& link) const
        {
            if ((packet.target.family () != AF_INET6) || packet.target.isWildcard () || packet.target.isMulticast ())
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            char* cur = data;
            const char* end = data + maxSize;
            bool ok = true;

            struct nd_neighbor_advert na = {};
            na.nd_na_type = NeighborAdvert;
            na.nd_na_flags_reserved = packet.flags;
            ::memcpy (&na.nd_na_target, packet.target.addr (), sizeof (na.nd_na_target));
            ok &= writeBytes (cur, end, &na, sizeof (na));
            ok &= writeLinkAddress (cur, end, link, TargetLinkAddress);

            return ok ? cur - data : -1;
        }

        /**
         * @brief deserialize a router solicitation from a buffer.
         * @param packet router solicitation to deserialize into.
         * @param data buffer holding the message to deserialize.
         * @param size message size.
         * @return 0 on success, -1 on failure.
         */
        int deserialize (RouterSolicitation& packet, const char* data, size_t size) const
        {
            const char* cur = data;
            const char* end = data + size;

            struct nd_router_solicit rs = {};
            if (!readBytes (cur, end, &rs, sizeof (rs)))
            {
                return -1;
            }

            if ((rs.nd_rs_type != RouterSolicit) || (rs.nd_rs_code != 0))
            {
                lastError = make_error_code (Errc::MessageUnknown);
                return -1;
            }

            packet.link = MacAddress ();

            return readOptions (cur, end, [&packet] (const uint8_t* option, size_t size) {
                if ((option[0] == SourceLinkAddress) && (size >= _optionHeaderSize + ETH_ALEN))
                {
                    packet.link = MacAddress (option + _optionHeaderSize, ETH_ALEN);
                }
            });
        }

        /**
         * @brief deserialize a router advertisement from a buffer.
         * @param packet router advertisement to deserialize into.
         * @param data buffer holding the message to deserialize.
         * @param size message size.
         * @return 0 on success, -1 on failure.
         */
        int deserialize (RouterAdvertisement& packet, const char* data, size_t size) const
        {
            const char* cur = data;
            const char* end = data + size;

            struct nd_router_advert ra = {};
            if (!readBytes (cur, end, &ra, sizeof (ra)))
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

            return readOptions (cur, end, [&packet] (const uint8_t* option, size_t size) {
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

        /**
         * @brief deserialize a neighbor solicitation from a buffer.
         * @param packet neighbor solicitation to deserialize into.
         * @param data buffer holding the message to deserialize.
         * @param size message size.
         * @return 0 on success, -1 on failure.
         */
        int deserialize (NeighborSolicitation& packet, const char* data, size_t size) const
        {
            const char* cur = data;
            const char* end = data + size;

            struct nd_neighbor_solicit ns = {};
            if (!readBytes (cur, end, &ns, sizeof (ns)))
            {
                return -1;
            }

            if ((ns.nd_ns_type != NeighborSolicit) || (ns.nd_ns_code != 0))
            {
                lastError = make_error_code (Errc::MessageUnknown);
                return -1;
            }

            packet.target = IpAddress (&ns.nd_ns_target, sizeof (ns.nd_ns_target));
            if (packet.target.isMulticast ())
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            packet.link = MacAddress ();

            return readOptions (cur, end, [&packet] (const uint8_t* option, size_t size) {
                if ((option[0] == SourceLinkAddress) && (size >= _optionHeaderSize + ETH_ALEN))
                {
                    packet.link = MacAddress (option + _optionHeaderSize, ETH_ALEN);
                }
            });
        }

        /**
         * @brief deserialize a neighbor advertisement from a buffer.
         * @param packet neighbor advertisement to deserialize into.
         * @param data buffer holding the message to deserialize.
         * @param size message size.
         * @return 0 on success, -1 on failure.
         */
        int deserialize (NeighborAdvertisement& packet, const char* data, size_t size) const
        {
            const char* cur = data;
            const char* end = data + size;

            struct nd_neighbor_advert na = {};
            if (!readBytes (cur, end, &na, sizeof (na)))
            {
                return -1;
            }

            if ((na.nd_na_type != NeighborAdvert) || (na.nd_na_code != 0))
            {
                lastError = make_error_code (Errc::MessageUnknown);
                return -1;
            }

            packet.target = IpAddress (&na.nd_na_target, sizeof (na.nd_na_target));
            if (packet.target.isMulticast ())
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            packet.flags = na.nd_na_flags_reserved;
            packet.link = MacAddress ();

            return readOptions (cur, end, [&packet] (const uint8_t* option, size_t size) {
                if ((option[0] == TargetLinkAddress) && (size >= _optionHeaderSize + ETH_ALEN))
                {
                    packet.link = MacAddress (option + _optionHeaderSize, ETH_ALEN);
                }
            });
        }

    protected:
        /**
         * @brief recursive DNS server option header, RFC 8106 section 5.1.
         */
        struct __attribute__ ((packed)) RdnssHeader
        {
            /// option type.
            uint8_t type;

            /// option length in units of 8 octets.
            uint8_t len;

            /// reserved.
            uint16_t reserved;

            /// time in seconds the servers may be used for.
            uint32_t lifetime;
        };

        /**
         * @brief write a link layer address option into a buffer.
         * @param cur current position in the buffer, advanced on success.
         * @param end end of the buffer.
         * @param link link layer address, nothing is written if wildcard.
         * @param type option type, source or target link layer address.
         * @return true on success, false if the buffer is too small.
         */
        static bool writeLinkAddress (char*& cur, const char* end, const MacAddress& link,
                                      OptionType type = SourceLinkAddress)
        {
            if (link.isWildcard ())
            {
                return true;
            }

            uint8_t option[_optionHeaderSize + ETH_ALEN] = {type, 1};
            ::memcpy (option + _optionHeaderSize, link.addr (), ETH_ALEN);

            return writeBytes (cur, end, option, sizeof (option));
        }

        /**
         * @brief walk through the options of a message, RFC 4861 section 4.6.
         * @param cur beginning of the options.
         * @param end end of the options.
         * @param handler function called with each option, header included, and its size.
         * @return 0 on success, -1 if an option is malformed.
         */
        template <typename Handler>
        static int readOptions (const char* cur, const char* end, Handler&& handler)
        {
            uint8_t option[_maxOptionSize];

            for (;;)
            {
                if (cur == end)
                {
                    return 0;
                }

                if (!readBytes (cur, end, option, _optionHeaderSize) || (option[1] == 0))
                {
                    lastError = make_error_code (Errc::InvalidParam);
                    return -1;
                }

                const size_t size = static_cast<size_t> (option[1]) << 3;
                if (!readBytes (cur, end, option + _optionHeaderSize, size - _optionHeaderSize))
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
