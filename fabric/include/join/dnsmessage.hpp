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

#ifndef JOIN_FABRIC_DNSMESSAGE_HPP
#define JOIN_FABRIC_DNSMESSAGE_HPP

// libjoin.
#include <join/ipaddress.hpp>
#include <join/utils.hpp>
#include <join/error.hpp>

// C++.
#include <unordered_set>
#include <system_error>
#include <sstream>
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
         * @brief serialize a DNS packet into a byte stream.
         * @param packet DNS packet to serialize.
         * @param data output stream.
         * @return 0 on success, -1 on error.
         */
        int serialize (const DnsPacket& packet, std::stringstream& data) const
        {
            uint16_t id = htons (packet.id);
            data.write (reinterpret_cast<const char*> (&id), sizeof (id));

            uint16_t flags = htons (packet.flags);
            data.write (reinterpret_cast<const char*> (&flags), sizeof (flags));

            uint16_t qcount = htons (static_cast<uint16_t> (packet.questions.size ()));
            data.write (reinterpret_cast<const char*> (&qcount), sizeof (qcount));

            uint16_t ancount = htons (static_cast<uint16_t> (packet.answers.size ()));
            data.write (reinterpret_cast<const char*> (&ancount), sizeof (ancount));

            uint16_t nscount = htons (static_cast<uint16_t> (packet.authorities.size ()));
            data.write (reinterpret_cast<const char*> (&nscount), sizeof (nscount));

            uint16_t arcount = htons (static_cast<uint16_t> (packet.additionals.size ()));
            data.write (reinterpret_cast<const char*> (&arcount), sizeof (arcount));

            for (auto const& question : packet.questions)
            {
                if (encodeQuestion (question, data) == -1)
                {
                    return -1;  // LCOV_EXCL_LINE
                }
            }

            for (auto const& answer : packet.answers)
            {
                if (encodeResource (answer, data) == -1)
                {
                    return -1;  // LCOV_EXCL_LINE
                }
            }

            for (auto const& authority : packet.authorities)
            {
                if (encodeResource (authority, data) == -1)
                {
                    return -1;  // LCOV_EXCL_LINE
                }
            }

            for (auto const& additional : packet.additionals)
            {
                if (encodeResource (additional, data) == -1)
                {
                    return -1;  // LCOV_EXCL_LINE
                }
            }

            return 0;
        }

        /**
         * @brief deserialize a DNS packet from a byte stream.
         * @param packet DNS packet to fill.
         * @param data input stream.
         * @return 0 on success, -1 on error.
         */
        int deserialize (DnsPacket& packet, std::stringstream& data) const
        {
            data.read (reinterpret_cast<char*> (&packet.id), sizeof (packet.id));
            packet.id = ntohs (packet.id);

            data.read (reinterpret_cast<char*> (&packet.flags), sizeof (packet.flags));
            packet.flags = ntohs (packet.flags);

            uint16_t qcount = 0;
            data.read (reinterpret_cast<char*> (&qcount), sizeof (qcount));
            qcount = ntohs (qcount);

            uint16_t ancount = 0;
            data.read (reinterpret_cast<char*> (&ancount), sizeof (ancount));
            ancount = ntohs (ancount);

            uint16_t nscount = 0;
            data.read (reinterpret_cast<char*> (&nscount), sizeof (nscount));
            nscount = ntohs (nscount);

            uint16_t arcount = 0;
            data.read (reinterpret_cast<char*> (&arcount), sizeof (arcount));
            arcount = ntohs (arcount);

            packet.questions.clear ();
            for (uint16_t i = 0; i < qcount; ++i)
            {
                QuestionRecord question;
                if (decodeQuestion (question, data) == -1)
                {
                    return -1;
                }
                packet.questions.emplace_back (std::move (question));
            }

            packet.answers.clear ();
            for (uint16_t i = 0; i < ancount; ++i)
            {
                ResourceRecord answer;
                if (decodeResource (answer, data) == -1)
                {
                    return -1;  // LCOV_EXCL_LINE
                }
                packet.answers.emplace_back (std::move (answer));
            }

            packet.authorities.clear ();
            for (uint16_t i = 0; i < nscount; ++i)
            {
                ResourceRecord authority;
                if (decodeResource (authority, data) == -1)
                {
                    return -1;  // LCOV_EXCL_LINE
                }
                packet.authorities.emplace_back (std::move (authority));
            }

            packet.additionals.clear ();
            for (uint16_t i = 0; i < arcount; ++i)
            {
                ResourceRecord additional;
                if (decodeResource (additional, data) == -1)
                {
                    return -1;  // LCOV_EXCL_LINE
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
         * @brief encode a DNS name into a byte stream.
         * @param name DNS name to encode.
         * @param data output stream.
         * @return 0 on success, -1 on error.
         */
        int encodeName (const std::string& name, std::stringstream& data) const
        {
            std::istringstream iss (name);

            for (std::string token; std::getline (iss, token, '.');)
            {
                data << static_cast<uint8_t> (token.size ());
                data << token;
            }

            data << '\0';

            return 0;
        }

        /**
         * @brief decode a DNS name from a byte stream.
         * @param name decoded DNS name.
         * @param data input stream.
         * @param depth recursion depth.
         * @return 0 on success, -1 on error.
         */
        int decodeName (std::string& name, std::stringstream& data, int depth = 0) const
        {
            if (depth > 10)
            {
                return -1;
            }

            for (;;)
            {
                auto pos = data.tellg ();

                uint16_t offset = 0;
                data.read (reinterpret_cast<char*> (&offset), sizeof (offset));
                offset = ntohs (offset);

                if (offset & 0xC000)
                {
                    pos = data.tellg ();
                    data.seekg (offset & 0x3FFF);
                    if (decodeName (name, data, depth + 1) == -1)
                    {
                        return -1;
                    }
                    data.seekg (pos);
                    break;
                }
                else
                {
                    data.seekg (pos);

                    uint8_t size = 0;
                    data.read (reinterpret_cast<char*> (&size), sizeof (size));

                    if (size == 0)
                    {
                        if (!name.empty () && name.back () == '.')
                        {
                            name.pop_back ();
                        }
                        break;
                    }

                    name.resize (name.size () + size);
                    data.read (&name[name.size () - size], size);
                    name += '.';
                }
            }

            return 0;
        }

        /**
         * @brief encode a mail address into a byte stream.
         * @param mail mail address to encode.
         * @param data output stream.
         * @return 0 on success, -1 on error.
         */
        int encodeMail (const std::string& mail, std::stringstream& data) const
        {
            std::string encodedMail = mail;
            size_t atPos = encodedMail.find ('@');

            if (atPos != std::string::npos)
            {
                encodedMail.replace (atPos, 1, ".");
            }

            encodeName (encodedMail, data);

            return 0;
        }

        /**
         * @brief decode a mail address from a byte stream.
         * @param mail decoded mail address.
         * @param data input stream.
         * @return 0 on success, -1 on error.
         */
        int decodeMail (std::string& mail, std::stringstream& data) const
        {
            if (decodeName (mail, data) == -1)
            {
                return -1;  // LCOV_EXCL_LINE
            }

            auto pos = mail.find ('.');
            if (pos != std::string::npos)
            {
                mail[pos] = '@';
            }

            return 0;
        }

        /**
         * @brief encode a question record into a byte stream.
         * @param question question record to encode.
         * @param data output stream.
         * @return 0 on success, -1 on error.
         */
        int encodeQuestion (const QuestionRecord& question, std::stringstream& data) const
        {
            encodeName (question.host, data);

            uint16_t type = htons (question.type);
            data.write (reinterpret_cast<const char*> (&type), sizeof (type));

            uint16_t dnsclass = htons (question.dnsclass);
            data.write (reinterpret_cast<const char*> (&dnsclass), sizeof (dnsclass));

            return 0;
        }

        /**
         * @brief decode a question record from a byte stream.
         * @param question question record to fill.
         * @param data input stream.
         * @return 0 on success, -1 on error.
         */
        int decodeQuestion (QuestionRecord& question, std::stringstream& data) const
        {
            if (decodeName (question.host, data) == -1)
            {
                return -1;
            }

            data.read (reinterpret_cast<char*> (&question.type), sizeof (question.type));
            question.type = ntohs (question.type);

            data.read (reinterpret_cast<char*> (&question.dnsclass), sizeof (question.dnsclass));
            question.dnsclass = ntohs (question.dnsclass);

            return 0;
        }

        /**
         * @brief encode a resource record into a byte stream.
         * @param resource resource record to encode.
         * @param data output stream.
         * @return 0 on success, -1 on error.
         */
        int encodeResource (const ResourceRecord& resource, std::stringstream& data) const
        {
            encodeName (resource.host, data);

            uint16_t type = htons (resource.type);
            data.write (reinterpret_cast<const char*> (&type), sizeof (type));

            uint16_t dnsclass = htons (resource.dnsclass);
            data.write (reinterpret_cast<const char*> (&dnsclass), sizeof (dnsclass));

            uint32_t ttl = htonl (resource.ttl);
            data.write (reinterpret_cast<const char*> (&ttl), sizeof (ttl));

            uint16_t dataLen = 0;
            auto dataLenPos = data.tellp ();
            data.write (reinterpret_cast<const char*> (&dataLen), sizeof (dataLen));

            auto dataBegPos = data.tellp ();

            if (resource.type == RecordType::A)
            {
                data.write (reinterpret_cast<const char*> (resource.addr.addr ()), sizeof (in_addr));
            }
            else if (resource.type == RecordType::AAAA)
            {
                data.write (reinterpret_cast<const char*> (resource.addr.addr ()), sizeof (in6_addr));
            }
            else if (resource.type == RecordType::NS)
            {
                encodeName (resource.name, data);
            }
            else if (resource.type == RecordType::CNAME)
            {
                encodeName (resource.name, data);
            }
            else if (resource.type == RecordType::PTR)
            {
                encodeName (resource.name, data);
            }
            else if (resource.type == RecordType::MX)
            {
                uint16_t mxpref = htons (resource.mxpref);
                data.write (reinterpret_cast<const char*> (&mxpref), sizeof (mxpref));
                encodeName (resource.name, data);
            }
            else if (resource.type == RecordType::SOA)
            {
                encodeName (resource.name, data);
                encodeMail (resource.mail, data);

                uint32_t serial = htonl (resource.serial);
                data.write (reinterpret_cast<const char*> (&serial), sizeof (serial));

                uint32_t refresh = htonl (resource.refresh);
                data.write (reinterpret_cast<const char*> (&refresh), sizeof (refresh));

                uint32_t retry = htonl (resource.retry);
                data.write (reinterpret_cast<const char*> (&retry), sizeof (retry));

                uint32_t expire = htonl (resource.expire);
                data.write (reinterpret_cast<const char*> (&expire), sizeof (expire));

                uint32_t minimum = htonl (resource.minimum);
                data.write (reinterpret_cast<const char*> (&minimum), sizeof (minimum));
            }
            else if (resource.type == RecordType::TXT)
            {
                for (auto const& txt : resource.txts)
                {
                    uint8_t size = static_cast<uint8_t> (txt.size ());
                    data.write (reinterpret_cast<const char*> (&size), sizeof (size));
                    data.write (txt.data (), size);
                }
            }
            else if (resource.type == RecordType::SRV)
            {
                uint16_t priority = htons (resource.priority);
                data.write (reinterpret_cast<const char*> (&priority), sizeof (priority));

                uint16_t weight = htons (resource.weight);
                data.write (reinterpret_cast<const char*> (&weight), sizeof (weight));

                uint16_t port = htons (resource.port);
                data.write (reinterpret_cast<const char*> (&port), sizeof (port));

                encodeName (resource.name, data);
            }

            auto dataEndPos = data.tellp ();
            dataLen = static_cast<uint16_t> (dataEndPos - dataBegPos);

            data.seekp (dataLenPos);
            dataLen = htons (dataLen);
            data.write (reinterpret_cast<const char*> (&dataLen), sizeof (dataLen));
            data.seekp (dataEndPos);

            return 0;
        }

        /**
         * @brief decode a resource record from a byte stream.
         * @param resource resource record to fill.
         * @param data input stream.
         * @return 0 on success, -1 on error.
         */
        int decodeResource (ResourceRecord& resource, std::stringstream& data) const
        {
            if (decodeName (resource.host, data) == -1)
            {
                return -1;  // LCOV_EXCL_LINE
            }

            data.read (reinterpret_cast<char*> (&resource.type), sizeof (resource.type));
            resource.type = ntohs (resource.type);

            data.read (reinterpret_cast<char*> (&resource.dnsclass), sizeof (resource.dnsclass));
            resource.dnsclass = ntohs (resource.dnsclass);

            data.read (reinterpret_cast<char*> (&resource.ttl), sizeof (resource.ttl));
            resource.ttl = ntohl (resource.ttl);

            uint16_t dataLen = 0;
            data.read (reinterpret_cast<char*> (&dataLen), sizeof (dataLen));
            dataLen = ntohs (dataLen);

            auto dataBegPos = data.tellg ();

            if (resource.type == RecordType::A)
            {
                struct in_addr addr;
                data.read (reinterpret_cast<char*> (&addr), sizeof (addr));
                resource.addr = IpAddress (&addr, sizeof (struct in_addr));
            }
            else if (resource.type == RecordType::AAAA)
            {
                struct in6_addr addr;
                data.read (reinterpret_cast<char*> (&addr), sizeof (addr));
                resource.addr = IpAddress (&addr, sizeof (struct in6_addr));
            }
            else if (resource.type == RecordType::NS)
            {
                if (decodeName (resource.name, data) == -1)
                {
                    return -1;  // LCOV_EXCL_LINE
                }
            }
            else if (resource.type == RecordType::CNAME)
            {
                if (decodeName (resource.name, data) == -1)
                {
                    return -1;  // LCOV_EXCL_LINE
                }
            }
            else if (resource.type == RecordType::PTR)
            {
                if (decodeName (resource.name, data) == -1)
                {
                    return -1;  // LCOV_EXCL_LINE
                }
            }
            else if (resource.type == RecordType::MX)
            {
                data.read (reinterpret_cast<char*> (&resource.mxpref), sizeof (resource.mxpref));
                resource.mxpref = ntohs (resource.mxpref);

                if (decodeName (resource.name, data) == -1)
                {
                    return -1;  // LCOV_EXCL_LINE
                }
            }
            else if (resource.type == RecordType::SOA)
            {
                if (decodeName (resource.name, data) == -1)
                {
                    return -1;  // LCOV_EXCL_LINE
                }

                decodeMail (resource.mail, data);

                data.read (reinterpret_cast<char*> (&resource.serial), sizeof (resource.serial));
                resource.serial = ntohl (resource.serial);

                data.read (reinterpret_cast<char*> (&resource.refresh), sizeof (resource.refresh));
                resource.refresh = ntohl (resource.refresh);

                data.read (reinterpret_cast<char*> (&resource.retry), sizeof (resource.retry));
                resource.retry = ntohl (resource.retry);

                data.read (reinterpret_cast<char*> (&resource.expire), sizeof (resource.expire));
                resource.expire = ntohl (resource.expire);

                data.read (reinterpret_cast<char*> (&resource.minimum), sizeof (resource.minimum));
                resource.minimum = ntohl (resource.minimum);
            }
            else if (resource.type == RecordType::TXT)
            {
                while (data.tellg () != -1 && (data.tellg () - dataBegPos < dataLen))
                {
                    uint8_t size = 0;
                    if (!data.read (reinterpret_cast<char*> (&size), sizeof (size)))
                    {
                        return -1;  // LCOV_EXCL_LINE
                    }

                    std::string txt;
                    txt.resize (size);
                    if (!data.read (&txt[0], size))
                    {
                        return -1;  // LCOV_EXCL_LINE
                    }

                    resource.txts.emplace_back (std::move (txt));
                }
            }
            else if (resource.type == RecordType::SRV)
            {
                data.read (reinterpret_cast<char*> (&resource.priority), sizeof (resource.priority));
                resource.priority = ntohs (resource.priority);

                data.read (reinterpret_cast<char*> (&resource.weight), sizeof (resource.weight));
                resource.weight = ntohs (resource.weight);

                data.read (reinterpret_cast<char*> (&resource.port), sizeof (resource.port));
                resource.port = ntohs (resource.port);

                if (decodeName (resource.name, data) == -1)
                {
                    return -1;  // LCOV_EXCL_LINE
                }
            }
            else
            {
                data.seekg (dataBegPos + static_cast<std::streamoff> (dataLen));
            }

            return 0;
        }
    };
}

#endif
