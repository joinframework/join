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

// C++.
#include <sstream>
#include <memory>
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
        /// pointer to a DHCP message.
        using Ptr = std::unique_ptr<DhcpPacket>;

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
         * @brief serialize a DHCP message into a byte stream.
         * @param packet DHCP message to serialize.
         * @param data byte stream to serialize the message into.
         * @return 0 on success, -1 on failure.
         */
        int serialize (const DhcpPacket& packet, std::stringstream& data) const
        {
            if ((packet.op != BootRequest) && (packet.op != BootReply))
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            const std::streampos start = data.tellp ();

            uint8_t head[4] = {packet.op, _ethernet, ETH_ALEN, 0};
            data.write (reinterpret_cast<const char*> (head), sizeof (head));

            uint32_t id = htonl (packet.id);
            data.write (reinterpret_cast<const char*> (&id), sizeof (id));

            uint16_t secs = htons (packet.secs);
            data.write (reinterpret_cast<const char*> (&secs), sizeof (secs));

            uint16_t flags = htons (packet.flags);
            data.write (reinterpret_cast<const char*> (&flags), sizeof (flags));

            writeAddress (data, packet.client);
            writeAddress (data, packet.your);
            writeAddress (data, packet.server);
            writeAddress (data, packet.gateway);

            char chaddr[_chaddrSize] = {};
            ::memcpy (chaddr, packet.hardware.addr (), ETH_ALEN);
            data.write (chaddr, sizeof (chaddr));

            char padding[_snameSize + _fileSize] = {};
            data.write (padding, sizeof (padding));

            uint32_t cookie = htonl (magicCookie);
            data.write (reinterpret_cast<const char*> (&cookie), sizeof (cookie));

            if (serialize (packet.options, data) == -1)
            {
                return -1;
            }

            const std::streamoff written = data.tellp () - start;
            if (written < static_cast<std::streamoff> (minMsgSize))
            {
                const std::string pad (static_cast<size_t> (minMsgSize - written), static_cast<char> (DhcpOption::Pad));
                data.write (pad.data (), pad.size ());
            }

            return 0;
        }

        /**
         * @brief deserialize a DHCP message from a byte stream.
         * @param packet DHCP message to deserialize into.
         * @param data byte stream holding the message to deserialize.
         * @return 0 on success, -1 on failure.
         */
        int deserialize (DhcpPacket& packet, std::stringstream& data) const
        {
            packet.options.clear ();

            uint8_t head[4] = {};
            if (!extract (data, head, sizeof (head)))
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

            if (!extract (data, &id, sizeof (id)) || !extract (data, &secs, sizeof (secs)) ||
                !extract (data, &flags, sizeof (flags)))
            {
                return -1;
            }

            packet.id = ntohl (id);
            packet.secs = ntohs (secs);
            packet.flags = ntohs (flags);

            if (!readAddress (data, packet.client) || !readAddress (data, packet.your) ||
                !readAddress (data, packet.server) || !readAddress (data, packet.gateway))
            {
                return -1;
            }

            uint8_t chaddr[_chaddrSize] = {};
            if (!extract (data, chaddr, sizeof (chaddr)))
            {
                return -1;
            }

            packet.hardware = MacAddress (chaddr, ETH_ALEN);

            char sname[_snameSize] = {}, file[_fileSize] = {};
            uint32_t cookie = 0;

            if (!extract (data, sname, sizeof (sname)) || !extract (data, file, sizeof (file)) ||
                !extract (data, &cookie, sizeof (cookie)))
            {
                return -1;
            }

            if (ntohl (cookie) != magicCookie)
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            if (deserialize (packet.options, data) == -1)
            {
                return -1;
            }

            const uint8_t* overload = packet.options.getIf<uint8_t> (DhcpOption::OptionOverload);
            if (overload != nullptr)
            {
                if ((*overload & FileOverload) && !deserialize (packet.options, file, sizeof (file)))
                {
                    return -1;
                }

                if ((*overload & SnameOverload) && !deserialize (packet.options, sname, sizeof (sname)))
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
         * @brief serialize an option list into a byte stream.
         * @param options options to serialize.
         * @param data byte stream to serialize the options into.
         * @return 0 on success, -1 on failure.
         */
        int serialize (const DhcpOption& options, std::stringstream& data) const;

        /**
         * @brief deserialize an option list from a byte stream.
         * @param options option list to deserialize into.
         * @param data byte stream holding the options to deserialize.
         * @return 0 on success, -1 on failure.
         */
        int deserialize (DhcpOption& options, std::stringstream& data) const;

        /**
         * @brief deserialize an option list from an overloaded field.
         * @param options option list to deserialize into.
         * @param field field holding the options to deserialize.
         * @param size field size.
         * @return true on success, false on failure.
         */
        bool deserialize (DhcpOption& options, const char* field, size_t size) const
        {
            std::stringstream data;
            data.rdbuf ()->pubsetbuf (const_cast<char*> (field), size);
            return (deserialize (options, data) != -1);
        }

        /**
         * @brief write an option header into a byte stream.
         * @param data byte stream to write to.
         * @param code option code.
         * @param size option payload size.
         * @return true if the payload fits in an option, false otherwise.
         */
        static bool writeHead (std::ostream& data, uint8_t code, size_t size);

        /**
         * @brief write an IPv4 address into a byte stream.
         * @param data byte stream to write to.
         * @param address address to write.
         */
        static void writeAddress (std::ostream& data, const IpAddress& address)
        {
            struct in_addr addr = {};

            if (address.family () == AF_INET)
            {
                ::memcpy (&addr, address.addr (), sizeof (addr));
            }

            data.write (reinterpret_cast<const char*> (&addr), sizeof (addr));
        }

        /**
         * @brief read an IPv4 address from a byte stream.
         * @param data byte stream to read from.
         * @param address address read.
         * @return true on success, false on failure.
         */
        static bool readAddress (std::istream& data, IpAddress& address)
        {
            struct in_addr addr = {};

            if (!extract (data, &addr, sizeof (addr)))
            {
                return false;
            }

            address = IpAddress (&addr, sizeof (addr));

            return true;
        }

        /**
         * @brief read a fixed amount of bytes from a byte stream.
         * @param data byte stream to read from.
         * @param out buffer to read into.
         * @param size number of bytes to read.
         * @return true if every byte was read, false otherwise.
         */
        static bool extract (std::istream& data, void* out, size_t size)
        {
            data.read (reinterpret_cast<char*> (out), size);

            if (data.fail ())
            {
                lastError = make_error_code (Errc::MessageTooLong);
                return false;
            }

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
