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

#ifndef JOIN_FABRIC_DNS_MESSAGE_HPP
#define JOIN_FABRIC_DNS_MESSAGE_HPP

// libjoin.
#include <join/ip_address.hpp>
#include <join/utils.hpp>
#include <join/error.hpp>

// C++.
#include <unordered_set>
#include <system_error>
#include <vector>
#include <string>

// C.
#include <cstdint>

namespace join
{
    /// list of aliases.
    using AliasList = std::unordered_set<std::string>;

    /// list of name servers.
    using ServerList = std::unordered_set<std::string>;

    /// list of mail exchangers.
    using ExchangerList = std::unordered_set<std::string>;

    /**
     * @brief question record.
     */
    struct QuestionRecord
    {
        std::string host;      /**< host name. */
        uint16_t type = 0;     /**< resource record type. */
        uint16_t dnsclass = 0; /**< DNS class. */
    };

    /**
     * @brief resource record.
     */
    struct ResourceRecord : public QuestionRecord
    {
        uint32_t ttl = 0;              /**< record TTL. */
        IpAddress addr;                /**< address. */
        std::string name;              /**< canonical, server or mail exchanger name. */
        uint16_t priority = 0;         /**< SRV priority. */
        uint16_t weight = 0;           /**< SRV weight. */
        uint16_t port = 0;             /**< SRV port. */
        std::vector<std::string> txts; /**< TXT records. */
        std::string mail;              /**< server mail. */
        uint32_t serial = 0;           /**< serial number. */
        uint32_t refresh = 0;          /**< refresh interval. */
        uint32_t retry = 0;            /**< retry interval. */
        uint32_t expire = 0;           /**< upper limit before zone is no longer authoritative. */
        uint32_t minimum = 0;          /**< minimum TTL. */
        uint16_t mxpref = 0;           /**< mail exchange preference. */
    };

    /**
     * @brief DNS packet.
     */
    struct DnsPacket
    {
        uint16_t id = 0;                         /**< transaction ID. */
        uint16_t flags = 0;                      /**< transaction flags. */
        IpAddress src;                           /**< source IP address. */
        IpAddress dest;                          /**< destination IP address. */
        uint16_t port = 0;                       /**< port. */
        std::vector<QuestionRecord> questions;   /**< question records. */
        std::vector<ResourceRecord> answers;     /**< answer records. */
        std::vector<ResourceRecord> authorities; /**< authority records. */
        std::vector<ResourceRecord> additionals; /**< additional records. */
    };

    /**
     * @brief DNS message codec.
     */
    class DnsMessage
    {
    public:
        /**
         * @brief DNS record types.
         */
        enum RecordType : uint16_t
        {
            A = 1,     /**< IPv4 host address. */
            NS = 2,    /**< Authoritative name server. */
            CNAME = 5, /**< Canonical name for an alias. */
            SOA = 6,   /**< Start of a zone of authority. */
            PTR = 12,  /**< Domain name pointer. */
            MX = 15,   /**< Mail exchange. */
            TXT = 16,  /**< Text strings. */
            AAAA = 28, /**< IPv6 host address. */
            SRV = 33,  /**< Service locator. */
            ANY = 255, /**< Any record type. */
        };

        /**
         * @brief DNS record classes.
         */
        enum RecordClass : uint16_t
        {
            IN = 1, /**< Internet. */
        };

        /**
         * @brief create the DnsMessage instance.
         */
        DnsMessage () noexcept = default;

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        DnsMessage (const DnsMessage& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to copy.
         * @return a reference to the current object.
         */
        DnsMessage& operator= (const DnsMessage& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        DnsMessage (DnsMessage&& other) = delete;

        /**
         * @brief move assignment operator.
         * @param other other object to move.
         * @return a reference to the current object.
         */
        DnsMessage& operator= (DnsMessage&& other) = delete;

        /**
         * @brief destroy instance.
         */
        ~DnsMessage () noexcept = default;

        /**
         * @brief serialize a DNS packet into a buffer.
         * @param packet DNS packet to serialize.
         * @param data buffer to serialize the packet into.
         * @param maxSize buffer size.
         * @return the packet size, -1 on error.
         */
        ssize_t serialize (const DnsPacket& packet, char* data, size_t maxSize) const
        {
            char* cur = data;
            const char* end = data + maxSize;
            bool ok = true;

            uint16_t id = htons (packet.id);
            ok &= writeBytes (cur, end, &id, sizeof (id));

            uint16_t flags = htons (packet.flags);
            ok &= writeBytes (cur, end, &flags, sizeof (flags));

            uint16_t qcount = htons (static_cast<uint16_t> (packet.questions.size ()));
            ok &= writeBytes (cur, end, &qcount, sizeof (qcount));

            uint16_t ancount = htons (static_cast<uint16_t> (packet.answers.size ()));
            ok &= writeBytes (cur, end, &ancount, sizeof (ancount));

            uint16_t nscount = htons (static_cast<uint16_t> (packet.authorities.size ()));
            ok &= writeBytes (cur, end, &nscount, sizeof (nscount));

            uint16_t arcount = htons (static_cast<uint16_t> (packet.additionals.size ()));
            ok &= writeBytes (cur, end, &arcount, sizeof (arcount));

            for (auto const& question : packet.questions)
            {
                ok &= encodeQuestion (question, cur, end);
            }

            for (auto const& answer : packet.answers)
            {
                ok &= encodeResource (answer, cur, end);
            }

            for (auto const& authority : packet.authorities)
            {
                ok &= encodeResource (authority, cur, end);
            }

            for (auto const& additional : packet.additionals)
            {
                ok &= encodeResource (additional, cur, end);
            }

            return ok ? cur - data : -1;
        }

        /**
         * @brief deserialize a DNS packet from a buffer.
         * @param packet DNS packet to fill.
         * @param data buffer holding the packet to deserialize.
         * @param size packet size.
         * @return 0 on success, -1 on error.
         */
        int deserialize (DnsPacket& packet, const char* data, size_t size) const
        {
            const char* cur = data;
            const char* end = data + size;
            bool ok = true;

            uint16_t qcount = 0, ancount = 0, nscount = 0, arcount = 0;

            ok &= readBytes (cur, end, &packet.id, sizeof (packet.id));
            ok &= readBytes (cur, end, &packet.flags, sizeof (packet.flags));
            ok &= readBytes (cur, end, &qcount, sizeof (qcount));
            ok &= readBytes (cur, end, &ancount, sizeof (ancount));
            ok &= readBytes (cur, end, &nscount, sizeof (nscount));
            ok &= readBytes (cur, end, &arcount, sizeof (arcount));

            if (!ok)
            {
                return -1;
            }

            packet.id = ntohs (packet.id);
            packet.flags = ntohs (packet.flags);
            qcount = ntohs (qcount);
            ancount = ntohs (ancount);
            nscount = ntohs (nscount);
            arcount = ntohs (arcount);

            packet.questions.clear ();
            for (uint16_t i = 0; i < qcount; ++i)
            {
                QuestionRecord question;
                if (decodeQuestion (question, cur, data, end) == -1)
                {
                    return -1;
                }
                packet.questions.emplace_back (std::move (question));
            }

            packet.answers.clear ();
            for (uint16_t i = 0; i < ancount; ++i)
            {
                ResourceRecord answer;
                if (decodeResource (answer, cur, data, end) == -1)
                {
                    return -1;
                }
                packet.answers.emplace_back (std::move (answer));
            }

            packet.authorities.clear ();
            for (uint16_t i = 0; i < nscount; ++i)
            {
                ResourceRecord authority;
                if (decodeResource (authority, cur, data, end) == -1)
                {
                    return -1;
                }
                packet.authorities.emplace_back (std::move (authority));
            }

            packet.additionals.clear ();
            for (uint16_t i = 0; i < arcount; ++i)
            {
                ResourceRecord additional;
                if (decodeResource (additional, cur, data, end) == -1)
                {
                    return -1;
                }
                packet.additionals.emplace_back (std::move (additional));
            }

            return 0;
        }

        /**
         * @brief convert DNS error to system error code.
         * @param error DNS error number.
         * @return system error.
         */
        static std::error_code decodeError (uint16_t error) noexcept
        {
            switch (error)
            {
                case 0:
                    return {};
                case 1:
                case 4:
                    return make_error_code (Errc::InvalidParam);
                case 2:
                    return make_error_code (Errc::OperationFailed);
                case 3:
                    return make_error_code (Errc::NotFound);
                case 5:
                    return make_error_code (Errc::PermissionDenied);
                default:
                    return make_error_code (Errc::UnknownError);
            }
        }

        /**
         * @brief get record type name.
         * @param recordType record type.
         * @return record type name.
         */
        static std::string typeName (uint16_t recordType)
        {
            switch (recordType)
            {
                OUT_ENUM (A);
                OUT_ENUM (NS);
                OUT_ENUM (CNAME);
                OUT_ENUM (SOA);
                OUT_ENUM (PTR);
                OUT_ENUM (MX);
                OUT_ENUM (TXT);
                OUT_ENUM (AAAA);
                OUT_ENUM (SRV);
                OUT_ENUM (ANY);
            }

            return "UNKNOWN";
        }

        /**
         * @brief get record class name.
         * @param recordClass record class.
         * @return record class name.
         */
        static std::string className (uint16_t recordClass)
        {
            switch (recordClass & 0x7FFF)
            {
                OUT_ENUM (IN);
            }

            return "UNKNOWN";
        }

    private:
        /**
         * @brief encode a DNS name into a buffer.
         * @param name DNS name to encode.
         * @param cur current position in the buffer, advanced on success.
         * @param end end of the buffer.
         * @return true on success, false if the buffer is too small.
         */
        bool encodeName (const std::string& name, char*& cur, const char* end) const
        {
            bool ok = true;

            for (size_t pos = 0; pos < name.size ();)
            {
                size_t dot = name.find ('.', pos);
                if (dot == std::string::npos)
                {
                    dot = name.size ();
                }

                uint8_t len = static_cast<uint8_t> (dot - pos);
                ok &= writeBytes (cur, end, &len, sizeof (len));
                ok &= writeBytes (cur, end, name.data () + pos, len);

                pos = dot + 1;
            }

            uint8_t root = 0;
            ok &= writeBytes (cur, end, &root, sizeof (root));

            return ok;
        }

        /**
         * @brief decode a DNS name from a buffer.
         * @param name decoded DNS name.
         * @param cur current position in the buffer, advanced on success.
         * @param begin beginning of the packet, compression pointers are relative to it.
         * @param end end of the buffer.
         * @param depth recursion depth.
         * @return 0 on success, -1 on error.
         */
        int decodeName (std::string& name, const char*& cur, const char* begin, const char* end, int depth = 0) const
        {
            if (depth > 10)
            {
                return -1;
            }

            for (;;)
            {
                uint8_t first = 0;
                if (!readBytes (cur, end, &first, sizeof (first)))
                {
                    return -1;
                }

                if ((first & 0xC0) == 0xC0)
                {
                    uint8_t second = 0;
                    if (!readBytes (cur, end, &second, sizeof (second)))
                    {
                        return -1;
                    }

                    const size_t offset = ((first & 0x3F) << 8) | second;
                    if (offset >= static_cast<size_t> (end - begin))
                    {
                        lastError = make_error_code (Errc::InvalidParam);
                        return -1;
                    }

                    const char* target = begin + offset;
                    if (decodeName (name, target, begin, end, depth + 1) == -1)
                    {
                        return -1;
                    }
                    break;
                }

                if (first == 0)
                {
                    if (!name.empty () && name.back () == '.')
                    {
                        name.pop_back ();
                    }
                    break;
                }

                name.reserve (name.size () + first + 1);
                name.resize (name.size () + first);
                if (!readBytes (cur, end, &name[name.size () - first], first))
                {
                    return -1;
                }
                name += '.';
            }

            return 0;
        }

        /**
         * @brief encode a mail address into a buffer.
         * @param mail mail address to encode.
         * @param cur current position in the buffer, advanced on success.
         * @param end end of the buffer.
         * @return true on success, false if the buffer is too small.
         */
        bool encodeMail (const std::string& mail, char*& cur, const char* end) const
        {
            std::string encodedMail = mail;
            size_t atPos = encodedMail.find ('@');

            if (atPos != std::string::npos)
            {
                encodedMail.replace (atPos, 1, ".");
            }

            return encodeName (encodedMail, cur, end);
        }

        /**
         * @brief decode a mail address from a buffer.
         * @param mail decoded mail address.
         * @param cur current position in the buffer, advanced on success.
         * @param begin beginning of the packet.
         * @param end end of the buffer.
         * @return 0 on success, -1 on error.
         */
        int decodeMail (std::string& mail, const char*& cur, const char* begin, const char* end) const
        {
            if (decodeName (mail, cur, begin, end) == -1)
            {
                return -1;
            }

            auto pos = mail.find ('.');
            if (pos != std::string::npos)
            {
                mail[pos] = '@';
            }

            return 0;
        }

        /**
         * @brief encode a question record into a buffer.
         * @param question question record to encode.
         * @param cur current position in the buffer, advanced on success.
         * @param end end of the buffer.
         * @return true on success, false if the buffer is too small.
         */
        bool encodeQuestion (const QuestionRecord& question, char*& cur, const char* end) const
        {
            bool ok = encodeName (question.host, cur, end);

            uint16_t type = htons (question.type);
            ok &= writeBytes (cur, end, &type, sizeof (type));

            uint16_t dnsclass = htons (question.dnsclass);
            ok &= writeBytes (cur, end, &dnsclass, sizeof (dnsclass));

            return ok;
        }

        /**
         * @brief decode a question record from a buffer.
         * @param question question record to fill.
         * @param cur current position in the buffer, advanced on success.
         * @param begin beginning of the packet.
         * @param end end of the buffer.
         * @return 0 on success, -1 on error.
         */
        int decodeQuestion (QuestionRecord& question, const char*& cur, const char* begin, const char* end) const
        {
            if (decodeName (question.host, cur, begin, end) == -1)
            {
                return -1;
            }

            bool ok = true;
            ok &= readBytes (cur, end, &question.type, sizeof (question.type));
            ok &= readBytes (cur, end, &question.dnsclass, sizeof (question.dnsclass));

            if (!ok)
            {
                return -1;
            }

            question.type = ntohs (question.type);
            question.dnsclass = ntohs (question.dnsclass);

            return 0;
        }

        /**
         * @brief encode a resource record into a buffer.
         * @param resource resource record to encode.
         * @param cur current position in the buffer, advanced on success.
         * @param end end of the buffer.
         * @return true on success, false if the buffer is too small.
         */
        bool encodeResource (const ResourceRecord& resource, char*& cur, const char* end) const
        {
            bool ok = encodeName (resource.host, cur, end);

            uint16_t type = htons (resource.type);
            ok &= writeBytes (cur, end, &type, sizeof (type));

            uint16_t dnsclass = htons (resource.dnsclass);
            ok &= writeBytes (cur, end, &dnsclass, sizeof (dnsclass));

            uint32_t ttl = htonl (resource.ttl);
            ok &= writeBytes (cur, end, &ttl, sizeof (ttl));

            uint16_t dataLen = 0;
            char* length = cur;
            ok &= writeBytes (cur, end, &dataLen, sizeof (dataLen));

            char* rdata = cur;

            if (resource.type == RecordType::A)
            {
                ok &= writeBytes (cur, end, resource.addr.addr (), sizeof (in_addr));
            }
            else if (resource.type == RecordType::AAAA)
            {
                ok &= writeBytes (cur, end, resource.addr.addr (), sizeof (in6_addr));
            }
            else if (resource.type == RecordType::NS)
            {
                ok &= encodeName (resource.name, cur, end);
            }
            else if (resource.type == RecordType::CNAME)
            {
                ok &= encodeName (resource.name, cur, end);
            }
            else if (resource.type == RecordType::PTR)
            {
                ok &= encodeName (resource.name, cur, end);
            }
            else if (resource.type == RecordType::MX)
            {
                uint16_t mxpref = htons (resource.mxpref);
                ok &= writeBytes (cur, end, &mxpref, sizeof (mxpref));
                ok &= encodeName (resource.name, cur, end);
            }
            else if (resource.type == RecordType::SOA)
            {
                ok &= encodeName (resource.name, cur, end);
                ok &= encodeMail (resource.mail, cur, end);

                uint32_t serial = htonl (resource.serial);
                ok &= writeBytes (cur, end, &serial, sizeof (serial));

                uint32_t refresh = htonl (resource.refresh);
                ok &= writeBytes (cur, end, &refresh, sizeof (refresh));

                uint32_t retry = htonl (resource.retry);
                ok &= writeBytes (cur, end, &retry, sizeof (retry));

                uint32_t expire = htonl (resource.expire);
                ok &= writeBytes (cur, end, &expire, sizeof (expire));

                uint32_t minimum = htonl (resource.minimum);
                ok &= writeBytes (cur, end, &minimum, sizeof (minimum));
            }
            else if (resource.type == RecordType::TXT)
            {
                for (auto const& txt : resource.txts)
                {
                    uint8_t size = static_cast<uint8_t> (txt.size ());
                    ok &= writeBytes (cur, end, &size, sizeof (size));
                    ok &= writeBytes (cur, end, txt.data (), size);
                }
            }
            else if (resource.type == RecordType::SRV)
            {
                uint16_t priority = htons (resource.priority);
                ok &= writeBytes (cur, end, &priority, sizeof (priority));

                uint16_t weight = htons (resource.weight);
                ok &= writeBytes (cur, end, &weight, sizeof (weight));

                uint16_t port = htons (resource.port);
                ok &= writeBytes (cur, end, &port, sizeof (port));

                ok &= encodeName (resource.name, cur, end);
            }

            if (ok)
            {
                dataLen = htons (static_cast<uint16_t> (cur - rdata));
                ::memcpy (length, &dataLen, sizeof (dataLen));
            }

            return ok;
        }

        /**
         * @brief decode a resource record from a buffer.
         * @param resource resource record to fill.
         * @param cur current position in the buffer, advanced on success.
         * @param begin beginning of the packet.
         * @param end end of the buffer.
         * @return 0 on success, -1 on error.
         */
        int decodeResource (ResourceRecord& resource, const char*& cur, const char* begin, const char* end) const
        {
            if (decodeName (resource.host, cur, begin, end) == -1)
            {
                return -1;
            }

            uint16_t dataLen = 0;

            bool ok = true;
            ok &= readBytes (cur, end, &resource.type, sizeof (resource.type));
            ok &= readBytes (cur, end, &resource.dnsclass, sizeof (resource.dnsclass));
            ok &= readBytes (cur, end, &resource.ttl, sizeof (resource.ttl));
            ok &= readBytes (cur, end, &dataLen, sizeof (dataLen));

            if (!ok)
            {
                return -1;
            }

            resource.type = ntohs (resource.type);
            resource.dnsclass = ntohs (resource.dnsclass);
            resource.ttl = ntohl (resource.ttl);
            dataLen = ntohs (dataLen);

            if (dataLen > static_cast<size_t> (end - cur))
            {
                lastError = make_error_code (Errc::MessageTooLong);
                return -1;
            }

            const char* rdataEnd = cur + dataLen;

            if (resource.type == RecordType::A)
            {
                struct in_addr addr;
                if (!readBytes (cur, end, &addr, sizeof (addr)))
                {
                    return -1;
                }
                resource.addr = IpAddress (&addr, sizeof (struct in_addr));
            }
            else if (resource.type == RecordType::AAAA)
            {
                struct in6_addr addr;
                if (!readBytes (cur, end, &addr, sizeof (addr)))
                {
                    return -1;
                }
                resource.addr = IpAddress (&addr, sizeof (struct in6_addr));
            }
            else if (resource.type == RecordType::NS)
            {
                if (decodeName (resource.name, cur, begin, end) == -1)
                {
                    return -1;
                }
            }
            else if (resource.type == RecordType::CNAME)
            {
                if (decodeName (resource.name, cur, begin, end) == -1)
                {
                    return -1;
                }
            }
            else if (resource.type == RecordType::PTR)
            {
                if (decodeName (resource.name, cur, begin, end) == -1)
                {
                    return -1;
                }
            }
            else if (resource.type == RecordType::MX)
            {
                if (!readBytes (cur, end, &resource.mxpref, sizeof (resource.mxpref)) ||
                    (decodeName (resource.name, cur, begin, end) == -1))
                {
                    return -1;
                }

                resource.mxpref = ntohs (resource.mxpref);
            }
            else if (resource.type == RecordType::SOA)
            {
                if ((decodeName (resource.name, cur, begin, end) == -1) ||
                    (decodeMail (resource.mail, cur, begin, end) == -1))
                {
                    return -1;
                }

                ok &= readBytes (cur, end, &resource.serial, sizeof (resource.serial));
                ok &= readBytes (cur, end, &resource.refresh, sizeof (resource.refresh));
                ok &= readBytes (cur, end, &resource.retry, sizeof (resource.retry));
                ok &= readBytes (cur, end, &resource.expire, sizeof (resource.expire));
                ok &= readBytes (cur, end, &resource.minimum, sizeof (resource.minimum));

                if (!ok)
                {
                    return -1;
                }

                resource.serial = ntohl (resource.serial);
                resource.refresh = ntohl (resource.refresh);
                resource.retry = ntohl (resource.retry);
                resource.expire = ntohl (resource.expire);
                resource.minimum = ntohl (resource.minimum);
            }
            else if (resource.type == RecordType::TXT)
            {
                while (cur < rdataEnd)
                {
                    uint8_t size = 0;
                    if (!readBytes (cur, end, &size, sizeof (size)))
                    {
                        return -1;  // LCOV_EXCL_LINE
                    }

                    std::string txt;
                    txt.resize (size);
                    if (!readBytes (cur, end, &txt[0], size))
                    {
                        return -1;
                    }

                    resource.txts.emplace_back (std::move (txt));
                }
            }
            else if (resource.type == RecordType::SRV)
            {
                ok &= readBytes (cur, end, &resource.priority, sizeof (resource.priority));
                ok &= readBytes (cur, end, &resource.weight, sizeof (resource.weight));
                ok &= readBytes (cur, end, &resource.port, sizeof (resource.port));

                if (!ok || (decodeName (resource.name, cur, begin, end) == -1))
                {
                    return -1;
                }

                resource.priority = ntohs (resource.priority);
                resource.weight = ntohs (resource.weight);
                resource.port = ntohs (resource.port);
            }
            else
            {
                cur = rdataEnd;
            }

            return 0;
        }
    };
}

#endif
