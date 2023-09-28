/**
 * MIT License
 *
 * Copyright (c) 2023 Mathieu Rabine
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

#ifndef __JOIN_RESOLVER_HPP__
#define __JOIN_RESOLVER_HPP__

// libjoin.
#include <join/macaddress.hpp>

// C++.
#include <functional>
#include <set>

namespace join
{
    /// list of alias.
    using AliasList = std::set <std::string>;

    /**
     *  @brief question record.
     */
    struct QuestionRecord
    {
        std::string host;                           /**< host name. */
        uint16_t type = 0;                          /**< resource record type. */
        uint16_t dnsclass = 0;                      /**< DNS class. */
    };

    /**
     *  @brief answer record.
     */
    struct AnswerRecord : public QuestionRecord
    {
        uint32_t ttl = 0;                           /**< record TTL. */
        IpAddress addr;                             /**< address. */
        std::string cname;                          /**< canonical name. */
        uint16_t mxpref = 0;                        /**< mail exchange preference. */
        std::string mxname;                         /**< mail exchange name. */
    };

    /**
     *  @brief name server record.
     */
    struct NameServerRecord : public QuestionRecord
    {
        uint32_t ttl = 0;                           /**< record TTL. */
        std::string ns;                             /**< server name. */
        std::string mail;                           /**< server mail. */
        uint32_t serial = 0;                        /**< serial number. */
        uint32_t refresh = 0;                       /**< refresh interval. */
        uint32_t retry = 0;                         /**< retry interval. */
        uint32_t expire = 0;                        /**< upper limit before zone is no longer authoritative. */
        uint32_t minimum = 0;                       /**< minimum TTL. */
    };

    /**
     *  @brief additional record.
     */
    struct AdditionalRecord : public QuestionRecord
    {
        uint32_t ttl = 0;                           /**< record TTL. */
        IpAddress addr;                             /**< address. */
        std::string cname;                          /**< canonical name. */
        uint16_t mxpref = 0;                        /**< mail exchange preference. */
        std::string mxname;                         /**< mail exchange name. */
    };

    /**
     *  @brief DNS packet.
     */
    struct DnsPacket
    {
        IpAddress src;                              /**< source IP address.*/
        IpAddress dest;                             /**< destination IP address.*/
        uint16_t port = 0;                          /**< port.*/
        std::vector <QuestionRecord> questions;     /**< question records. */
        std::vector <AnswerRecord> answers;         /**< answer records. */
        std::vector <NameServerRecord> servers;     /**< name server records. */
        std::vector <AdditionalRecord> additionals; /**< additional records. */
    };

    /**
     * @brief basic domain name resolution class.
     */
    class Resolver
    {
    public:
        /**
         * @brief DNS record types.
         */
        enum RecordType : uint16_t
        {
            A = 1,                          /**< IPv4 host address. */
            NS = 2,                         /**< Authoritative name server. */
            CNAME = 5,                      /**< Canonical name for an alias. */
            SOA = 6,                        /**< Start of a zone of authority. */
            PTR = 12,                       /**< Domain name pointer. */
            MX = 15,                        /**< Mail exchange. */
            AAAA = 28,                      /**< IPv6 host address. */
        };

        /**
         * @brief DNS record classes.
         */
        enum RecordClass : uint16_t
        {
            IN = 1,                         /**< Internet. */
        };

        /**
         * @brief default constructor.
         */
        Resolver ();

        /**
         * @brief create the Resolver instance binded to the given interface.
         * @param interface interface to use.
         */
        Resolver (const std::string& interface);

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        Resolver (const Resolver& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to copy.
         * @return a reference to the current object.
         */
        Resolver& operator= (const Resolver& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        Resolver (Resolver&& other) = delete;

        /**
         * @brief move assignment operator.
         * @param other other object to move.
         * @return a reference to the current object.
         */
        Resolver& operator= (Resolver&& other) = delete;

        /**
         * @brief destroy instance.
         */
        virtual ~Resolver () = default;

        /**
         * @brief get IP address of the currently configured name servers.
         * @return a list of configured name servers.
         */
        static IpAddressList nameServers ();

        /**
         * @brief resolve host name and return all IP address found.
         * @param host host name to resolve.
         * @param family address family.
         * @param server server address.
         * @param port server port.
         * @param timeout timeout in milliseconds (default: 5000).
         * @return the resolved IP address list.
         */
        IpAddressList resolveAllHost (const std::string& host, int family, const IpAddress& server, uint16_t port = dnsPort, int timeout = 5000);

        /**
         * @brief resolve host name and return all IP address found.
         * @param host host name to resolve.
         * @param family address family.
         * @return the resolved IP address list.
         */
        static IpAddressList resolveAllHost (const std::string& host, int family);

        /**
         * @brief resolve host name and return all IP address found.
         * @param host host name to resolve.
         * @param server server address.
         * @param port server port.
         * @param timeout timeout in milliseconds (default: 5000).
         * @return the resolved IP address list.
         */
        IpAddressList resolveAllHost (const std::string& host, const IpAddress& server, uint16_t port = dnsPort, int timeout = 5000);

        /**
         * @brief resolve host name and return all IP address found.
         * @param host host name to resolve.
         * @return the resolved IP address list.
         */
        static IpAddressList resolveAllHost (const std::string& host);

        /**
         * @brief resolve host name using address family.
         * @param host host name to resolve.
         * @param family address family.
         * @param server server address.
         * @param port server port.
         * @param timeout timeout in milliseconds (default: 5000).
         * @return the first resolved IP address found matching address family.
         */
        IpAddress resolveHost (const std::string& host, int family, const IpAddress& server, uint16_t port = dnsPort, int timeout = 5000);

        /**
         * @brief resolve host name using address family.
         * @param host host name to resolve.
         * @param family address family.
         * @return the first resolved IP address found matching address family.
         */
        static IpAddress resolveHost (const std::string& host, int family);

        /**
         * @brief resolve host name.
         * @param host host name to resolve.
         * @param server server address.
         * @param port server port.
         * @param timeout timeout in milliseconds (default: 5000).
         * @return the first resolved IP address found.
         */
        IpAddress resolveHost (const std::string& host, const IpAddress& server, uint16_t port = dnsPort, int timeout = 5000);

        /**
         * @brief resolve host name.
         * @param host host name to resolve.
         * @return the first resolved IP address found.
         */
        static IpAddress resolveHost (const std::string& host);

        /**
         * @brief resolve all host address.
         * @param address host address to resolve.
         * @param server server address.
         * @param port server port.
         * @param timeout timeout in milliseconds (default: 5000).
         * @return the resolved alias list.
         */
        AliasList resolveAllAddress (const IpAddress& address, const IpAddress& server, uint16_t port = dnsPort, int timeout = 5000);

        /**
         * @brief resolve all host address.
         * @param address host address to resolve.
         * @return the resolved alias list.
         */
        static AliasList resolveAllAddress (const IpAddress& address);

        /**
         * @brief resolve host address.
         * @param address host address to resolve.
         * @param server server address.
         * @param port server port.
         * @param timeout timeout in milliseconds (default: 5000).
         * @return the first resolved alias.
         */
        std::string resolveAddress (const IpAddress& address, const IpAddress& server, uint16_t port = dnsPort, int timeout = 5000);

        /**
         * @brief resolve host address.
         * @param host host address to resolve.
         * @return the first resolved alias.
         */
        static std::string resolveAddress (const IpAddress& address);

        /**
         * @brief resolve service name.
         * @param service service name to resolve (ex. "http", "ftp", "ssh" etc...).
         * @return the port resolved.
         */
        static uint16_t resolveService (const std::string& service);

        /**
         * @brief get record type name.
         * @param recordType record type.
         * @return record type name.
         */
        static std::string typeName (uint16_t recordType);

        /**
         * @brief get record class name.
         * @param recordType record class.
         * @return record class name.
         */
        static std::string className (uint16_t recordClass);

        /// default DNS port.
        static constexpr uint16_t dnsPort = 53;

        /// notification callback definition.
        using DnsNotify = std::function <void (const DnsPacket&)>;

        /// callback called when a lookup sequence succeed.
        DnsNotify _onSuccess;

        /// callback called when a lookup sequence failed.
        DnsNotify _onFailure;

    protected:
        /**
         * @brief send the DNS request.
         * @param packet DNS packet to send.
         * @param timeout timeout in milliseconds.
         * @return 0 on success, -1 on failure.
         */
        int lookup (DnsPacket& packet, int timeout);

        /**
         * @brief set DNS header.
         * @param id request id.
         * @param flags flags.
         * @param qcount question record count.
         * @param ancount answer record count.
         * @param nscount name server record count.
         * @param arcount additional record count.
         * @param data data stream where to write header.
         */
        void setHeader (uint16_t id, uint16_t flags, uint16_t qcount, uint16_t ancount, uint16_t nscount, uint16_t arcount, std::stringstream& data);

        /**
         * @brief get DNS header.
         * @param id request id.
         * @param flags flags.
         * @param qcount question record count.
         * @param ancount answer record count.
         * @param nscount name server record count.
         * @param arcount additional record count.
         * @param data data stream where to read header.
         */
        void getHeader (uint16_t& id, uint16_t& flags, uint16_t& qcount, uint16_t& ancount, uint16_t& nscount, uint16_t& arcount, std::stringstream& data);

        /**
         * @brief encode name.
         * @param host host name to encode.
         * @param data data stream where to store encoded name.
         */
        static void encodeName (const std::string& host, std::stringstream& data);

        /**
         * @brief decode name.
         * @param data stream where the encoded name is stored.
         * @return decoded name.
         */
        static std::string decodeName (std::stringstream& data);

        /**
         * @brief decode mail.
         * @param data stream where the encoded mail is stored.
         * @return decoded mail.
         */
        static std::string decodeMail (std::stringstream& data);

        /**
         * @brief decode question record.
         * @param host host name.
         * @param type record type.
         * @param dnsclass record class.
         * @param data data stream where to store encoded question.
         */
        static void encodeQuestion (const std::string& host, uint16_t type, uint16_t dnsclass, std::stringstream& data);

        /**
         * @brief decode question record.
         * @param data stream where the encoded mail is stored.
         * @return decoded question record.
         */
        static QuestionRecord decodeQuestion (std::stringstream& data);

        /**
         * @brief decode answer record.
         * @param data stream where the encoded mail is stored.
         * @return decoded answer record.
         */
        static AnswerRecord decodeAnswer (std::stringstream& data);

        /**
         * @brief decode name server record.
         * @param data stream where the encoded mail is stored.
         * @return decoded  name server record.
         */
        static NameServerRecord decodeNameServer (std::stringstream& data);

        /**
         * @brief decode additional record.
         * @param data stream where the encoded mail is stored.
         * @return decoded additional record.
         */
        static AdditionalRecord decodeAdditional (std::stringstream& data);

        /**
         * @brief convert DNS error to system error code.
         * @param error DNS error number.
         * @return system error.
         */
        static std::error_code parseError (int error);

    #ifdef DEBUG
        /*
        * @brief default callback called when a lookup sequence succeed.
        * @param packet DNS packet.
        */
        static void defaultOnSuccess (const DnsPacket& packet);

        /*
        * @brief default callback called when a lookup sequence failed.
        * @param packet DNS packet.
        */
        static void defaultOnFailure (const DnsPacket& packet);
    #endif

        /**
         * @brief safe way to notify DNS events.
         * @param function function to call.
         * @param packet DNS packet.
         */
        void notify (const DnsNotify& function, const DnsPacket& packet);

        /// interface name.
        std::string _interface;
    };
}

#endif
