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

#ifndef JOIN_FABRIC_DHCP_MESSAGE_HPP
#define JOIN_FABRIC_DHCP_MESSAGE_HPP

// libjoin.
#include <join/dhcp_option.hpp>
#include <join/utils.hpp>

// C++.
#include <vector>

// C.
#include <net/ethernet.h>
#include <netinet/in.h>
#include <cstring>

namespace join
{
    /**
     * @brief DHCP message.
     */
    struct DhcpPacket
    {
        /// link layer source address, set by the transport.
        MacAddress src;

        /// link layer destination address, set by the transport.
        MacAddress dest;

        /// client hardware address, carried by the message itself.
        MacAddress hardware;

        /// client IP address, set when the client already owns a lease.
        IpAddress client{AF_INET};

        /// IP address the server assigns to the client.
        IpAddress your{AF_INET};

        /// IP address of the next server to use at boot time.
        IpAddress server{AF_INET};

        /// IP address of the relay agent the message went through.
        IpAddress gateway{AF_INET};

        /// transaction identifier.
        uint32_t id = 0;

        /// seconds elapsed since the client started the acquisition.
        uint16_t secs = 0;

        /// message flags.
        uint16_t flags = 0;

        /// operation code.
        uint8_t op = 0;

        /// message options.
        DhcpOption options;
    };

    /**
     * @brief DHCP message codec.
     */
    class DhcpMessage
    {
    public:
        /**
         * @brief DHCP message types, RFC 2132 section 9.6.
         */
        enum MessageType : uint8_t
        {
            Discover = 1, /**< a client looking for a server. */
            Offer = 2,    /**< a server proposing an address. */
            Request = 3,  /**< a client asking for an address. */
            Decline = 4,  /**< a client refusing an address already in use. */
            Ack = 5,      /**< a server confirming an address. */
            Nak = 6,      /**< a server refusing a request. */
            Release = 7,  /**< a client giving an address back. */
            Inform = 8,   /**< a client asking for parameters only. */
        };

        /**
         * @brief DHCP operation codes.
         */
        enum Operation : uint8_t
        {
            BootRequest = 1, /**< message sent by a client. */
            BootReply = 2,   /**< message sent by a server. */
        };

        /**
         * @brief message flags.
         */
        enum Flag : uint16_t
        {
            BroadcastFlag = 0x8000, /**< the client asks to be answered by broadcast. */
        };

        /**
         * @brief option overload values.
         */
        enum Overload : uint8_t
        {
            FileOverload = 1,  /**< the file field carries options. */
            SnameOverload = 2, /**< the sname field carries options. */
        };

        /**
         * @brief serialize a DHCP message into a buffer.
         * @param packet DHCP message to serialize.
         * @param data buffer to serialize the message into.
         * @param maxSize buffer size.
         * @return the message size, -1 on failure.
         */
        ssize_t serialize (const DhcpPacket& packet, char* data, size_t maxSize) const
        {
            if ((packet.op != BootRequest) && (packet.op != BootReply))
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            char* cur = data;
            const char* end = data + maxSize;
            bool ok = true;

            uint8_t head[4] = {packet.op, _ethernet, ETH_ALEN, 0};
            ok &= writeBytes (cur, end, head, sizeof (head));

            uint32_t id = htonl (packet.id);
            ok &= writeBytes (cur, end, &id, sizeof (id));

            uint16_t secs = htons (packet.secs);
            ok &= writeBytes (cur, end, &secs, sizeof (secs));

            uint16_t flags = htons (packet.flags);
            ok &= writeBytes (cur, end, &flags, sizeof (flags));

            ok &= writeAddress (cur, end, packet.client);
            ok &= writeAddress (cur, end, packet.your);
            ok &= writeAddress (cur, end, packet.server);
            ok &= writeAddress (cur, end, packet.gateway);

            char chaddr[_chaddrSize] = {};
            ::memcpy (chaddr, packet.hardware.addr (), ETH_ALEN);
            ok &= writeBytes (cur, end, chaddr, sizeof (chaddr));

            char padding[_snameSize + _fileSize] = {};
            ok &= writeBytes (cur, end, padding, sizeof (padding));

            uint32_t cookie = htonl (magicCookie);
            ok &= writeBytes (cur, end, &cookie, sizeof (cookie));

            if (!ok || (serialize (packet.options, cur, end) == -1))
            {
                return -1;
            }

            const size_t written = static_cast<size_t> (cur - data);
            if (written < minMsgSize)
            {
                const char pad[minMsgSize] = {};
                if (!writeBytes (cur, end, pad, minMsgSize - written))
                {
                    return -1;
                }
            }

            return cur - data;
        }

        /**
         * @brief deserialize a DHCP message from a buffer.
         * @param packet DHCP message to deserialize into.
         * @param data buffer holding the message to deserialize.
         * @param size message size.
         * @return 0 on success, -1 on failure.
         */
        int deserialize (DhcpPacket& packet, const char* data, size_t size) const
        {
            packet.options.clear ();

            const char* cur = data;
            const char* end = data + size;

            uint8_t head[4] = {};
            if (!readBytes (cur, end, head, sizeof (head)))
            {
                return -1;
            }

            packet.op = head[0];
            if (((packet.op != BootRequest) && (packet.op != BootReply)) || (head[1] != _ethernet) ||
                (head[2] != ETH_ALEN))
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            uint32_t id = 0;
            uint16_t secs = 0, flags = 0;

            if (!readBytes (cur, end, &id, sizeof (id)) || !readBytes (cur, end, &secs, sizeof (secs)) ||
                !readBytes (cur, end, &flags, sizeof (flags)))
            {
                return -1;
            }

            packet.id = ntohl (id);
            packet.secs = ntohs (secs);
            packet.flags = ntohs (flags);

            if (!readAddress (cur, end, packet.client) || !readAddress (cur, end, packet.your) ||
                !readAddress (cur, end, packet.server) || !readAddress (cur, end, packet.gateway))
            {
                return -1;
            }

            uint8_t chaddr[_chaddrSize] = {};
            if (!readBytes (cur, end, chaddr, sizeof (chaddr)))
            {
                return -1;
            }

            packet.hardware = MacAddress (chaddr, ETH_ALEN);

            char sname[_snameSize] = {}, file[_fileSize] = {};
            uint32_t cookie = 0;

            if (!readBytes (cur, end, sname, sizeof (sname)) || !readBytes (cur, end, file, sizeof (file)) ||
                !readBytes (cur, end, &cookie, sizeof (cookie)))
            {
                return -1;
            }

            if (ntohl (cookie) != magicCookie)
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            if (deserialize (packet.options, cur, end) == -1)
            {
                return -1;
            }

            const uint8_t* overload = packet.options.getIf<uint8_t> (DhcpOption::OptionOverload);
            if (overload != nullptr)
            {
                if ((*overload & FileOverload) && (deserialize (packet.options, file, file + sizeof (file)) == -1))
                {
                    return -1;
                }

                if ((*overload & SnameOverload) && (deserialize (packet.options, sname, sname + sizeof (sname)) == -1))
                {
                    return -1;
                }
            }

            return 0;
        }

        /// magic cookie introducing the option field.
        static constexpr uint32_t magicCookie = 0x63825363;

        /// size of the fixed part of a DHCP message, magic cookie included.
        static constexpr size_t headerSize = 240;

        /// smallest message a BOOTP implementation accepts, RFC 951 and RFC 1542 section 2.1.
        static constexpr size_t minMsgSize = 300;

        /// biggest payload a single option can carry, its length field is one octet.
        static constexpr size_t maxOptionSize = 255;

    protected:
        /**
         * @brief serialize an option list into a buffer.
         * @param options options to serialize.
         * @param cur current position in the buffer, advanced on success.
         * @param end end of the buffer.
         * @return 0 on success, -1 on failure.
         */
        int serialize (const DhcpOption& options, char*& cur, const char* end) const;

        /**
         * @brief deserialize an option list from a buffer.
         * @param options option list to deserialize into.
         * @param cur beginning of the options.
         * @param end end of the options.
         * @return 0 on success, -1 on failure.
         */
        int deserialize (DhcpOption& options, const char* cur, const char* end) const;

        /**
         * @brief write an option header into a buffer.
         * @param cur current position in the buffer, advanced on success.
         * @param end end of the buffer.
         * @param code option code.
         * @param size option payload size.
         * @return true on success, false if the payload does not fit in an option or the header in the buffer.
         */
        static bool writeHead (char*& cur, const char* end, uint8_t code, size_t size);

        /**
         * @brief write an IPv4 address into a buffer.
         * @param cur current position in the buffer, advanced on success.
         * @param end end of the buffer.
         * @param address address to write.
         * @return true on success, false if the buffer is too small.
         */
        static bool writeAddress (char*& cur, const char* end, const IpAddress& address)
        {
            struct in_addr addr = {};

            if (address.family () == AF_INET)
            {
                ::memcpy (&addr, address.addr (), sizeof (addr));
            }

            return writeBytes (cur, end, &addr, sizeof (addr));
        }

        /**
         * @brief read an IPv4 address from a buffer.
         * @param cur current position in the buffer, advanced on success.
         * @param end end of the buffer.
         * @param address address read.
         * @return true on success, false on failure.
         */
        static bool readAddress (const char*& cur, const char* end, IpAddress& address)
        {
            struct in_addr addr = {};

            if (!readBytes (cur, end, &addr, sizeof (addr)))
            {
                return false;
            }

            address = IpAddress (&addr, sizeof (addr));

            return true;
        }

        /// hardware address type of an Ethernet link.
        static constexpr uint8_t _ethernet = 1;

        /// size of the client hardware address field.
        static constexpr size_t _chaddrSize = 16;

        /// size of the server name field.
        static constexpr size_t _snameSize = 64;

        /// size of the boot file name field.
        static constexpr size_t _fileSize = 128;
    };
}

#endif
