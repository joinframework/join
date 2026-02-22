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

#ifndef __JOIN_DNSCLIENT_HPP__
#define __JOIN_DNSCLIENT_HPP__

// libjoin.
#include <join/reactor.hpp>
#include <join/socket.hpp>
#include <join/utils.hpp>

// C++.
#include <type_traits>
#include <functional>
#include <sstream>
#include <chrono>
#include <bitset>
#include <string>
#include <vector>
#include <set>

// C.
#include <resolv.h>
#include <netdb.h>

namespace join
{
    /// forward declarations.
    class Mdns;
    class Dns;
    class Dot;

    /**
     * @brief specialization trait for DNS, DoT.
     */
    template <typename Transport>
    struct is_multicast_dns : std::false_type {};

    /**
     * @brief specialization trait for mDNS.
     */
    template <>
    struct is_multicast_dns <Mdns> : std::true_type {};

    /// list of alias.
    using AliasList = std::set <std::string>;

    /// list of name servers.
    using ServerList = std::set <std::string>;

    /// list of mail exchangers.
    using ExchangerList = std::set <std::string>;

    /**
     * @brief question record.
     */
    struct QuestionRecord
    {
        std::string host;                           /**< host name. */
        uint16_t type = 0;                          /**< resource record type. */
        uint16_t dnsclass = 0;                      /**< DNS class. */
    };

    /**
     * @brief resource record.
     */
    struct ResourceRecord : public QuestionRecord
    {
        uint32_t ttl = 0;                           /**< record TTL. */
        IpAddress addr;                             /**< address. */
        std::string name;                           /**< canonical, server or mail exchanger name. */
        std::string mail;                           /**< server mail. */
        uint32_t serial = 0;                        /**< serial number. */
        uint32_t refresh = 0;                       /**< refresh interval. */
        uint32_t retry = 0;                         /**< retry interval. */
        uint32_t expire = 0;                        /**< upper limit before zone is no longer authoritative. */
        uint32_t minimum = 0;                       /**< minimum TTL. */
        uint16_t mxpref = 0;                        /**< mail exchange preference. */
    };

    /**
     * @brief DNS packet.
     */
    struct DnsPacket
    {
        IpAddress src;                              /**< source IP address.*/
        IpAddress dest;                             /**< destination IP address.*/
        uint16_t port = 0;                          /**< port.*/
        std::vector <QuestionRecord> questions;     /**< question records. */
        std::vector <ResourceRecord> answers;       /**< answer records. */
        std::vector <ResourceRecord> authorities;   /**< authority records. */
        std::vector <ResourceRecord> additionals;   /**< additional records. */
    };

    /**
     * @brief basic domain name resolution class.
     */
    template <typename Transport>
    class BasicDnsClient : public EventHandler
    {
    public:
        /**
         * @brief DNS record types.
         */
        enum RecordType : uint16_t
        {
            A = 1,                                  /**< IPv4 host address. */
            NS = 2,                                 /**< Authoritative name server. */
            CNAME = 5,                              /**< Canonical name for an alias. */
            SOA = 6,                                /**< Start of a zone of authority. */
            PTR = 12,                               /**< Domain name pointer. */
            MX = 15,                                /**< Mail exchange. */
            AAAA = 28,                              /**< IPv6 host address. */
            ANY = 255,                              /**< Any record type. */
        };

        /**
         * @brief DNS record classes.
         */
        enum RecordClass : uint16_t
        {
            IN = 1,                                 /**< Internet. */
        };

        /**
         * @brief create the Resolver instance binded to the given interface.
         * @param interface interface to use.
         * @param args additional arguments (see. policy::create).
         */
        template <typename... Args>
        BasicDnsClient (const std::string& interface = "", Args&&... args)
    #ifdef DEBUG
        : _onSuccess (defaultOnSuccess)
        , _onFailure (defaultOnFailure)
        , _interface (interface)
    #else
        : _interface (interface)
    #endif
        , _buffer (std::make_unique <uint8_t[]> (4096))
        {
            if (Transport::create (_socket, _interface, std::forward <Args> (args)...) == -1)
            {
                throw std::system_error (lastError);
            }

            Reactor::instance ()->addHandler (this);
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicDnsClient (const BasicDnsClient& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to copy.
         * @return a reference to the current object.
         */
        BasicDnsClient& operator= (const BasicDnsClient& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        BasicDnsClient (BasicDnsClient&& other) = delete;

        /**
         * @brief move assignment operator.
         * @param other other object to move.
         * @return a reference to the current object.
         */
        BasicDnsClient& operator= (BasicDnsClient&& other) = delete;

        /**
         * @brief destroy instance.
         */
        ~BasicDnsClient ()
        {
            Reactor::instance ()->delHandler (this);
            _socket.close ();
        }

        /**
         * @brief get IP address of the currently configured name servers.
         * @return a list of configured name servers.
         */
        template <typename T = Transport, typename std::enable_if <!is_multicast_dns <T>::value, int>::type = 0>
        static IpAddressList nameServers ()
        {
            IpAddressList addressList;

            struct __res_state res;
            if (res_ninit (&res) == 0)
            {
                for (int i = 0; i < res.nscount; ++i)
                {
                    if (res.nsaddr_list[i].sin_family == AF_INET)
                    {
                        addressList.emplace_back (&res.nsaddr_list[i].sin_addr, sizeof (struct in_addr));
                    }
                    else if (res._u._ext.nsaddrs[i] != nullptr && res._u._ext.nsaddrs[i]->sin6_family == AF_INET6)
                    {
                        addressList.emplace_back (&res._u._ext.nsaddrs[i]->sin6_addr, sizeof (struct in6_addr));
                    }
                }
                res_nclose (&res);
            }

            return addressList;
        }

//         /**
//          * @brief send probe query
//          * @param host hostname to probe.
//          * @param family address family.
//          * @param address IP address to probe.
//          * @param timeout timeout in milliseconds (default: 5000).
//          * @return DNS packet containing any responses received
//          */
//         template <typename T = Transport, typename std::enable_if <is_multicast_dns <T>::value, int>::type = 0>
//         bool probe (const std::string& host, int family, const IpAddress& address, std::chrono::milliseconds timeout = std::chrono::milliseconds (5000))
//         {
//             DnsPacket packet;
//             packet.src = IpAddress (family);
//             packet.dest = T::multicastAddress (family);
//             packet.port = T::defaultPort;

//             QuestionRecord question;
//             question.host = host;
//             question.type = RecordType::ANY;
//             question.dnsclass = RecordClass::IN;
//             packet.questions.push_back (question);

//             ResourceRecord authority;
//             authority.host = host;
//             authority.type = (family == AF_INET6) ? RecordType::AAAA : RecordType::A;
//             authority.dnsclass = RecordClass::IN;
//             authority.ttl = 120;
//             authority.addr = address;
//             packet.authorities.push_back (authority);

//             if (lookup (packet, timeout) == -1)
//             {
//                 return false;
//             }

//             for (const auto& answer : packet.answers)
//             {
//                 if (answer.host == host)
//                 {
//                     return false;
//                 }
//             }

//             return true;
//         }

//         template <typename T = Transport, typename std::enable_if <is_multicast_dns <T>::value, int>::type = 0>
//         static bool probe (const std::string& hostname, int family, const IpAddress& ipaddr, const std::string& interface, std::chrono::milliseconds timeout = std::chrono::milliseconds (5000))
//         {
//             return BasicDnsClient <T> (interface).probe (hostname, family, ipaddr, timeout);
//         }

//         /**
//          * @brief resolve host name and return all IP address found.
//          * @param host host name to resolve.
//          * @param family address family.
//          * @param timeout timeout in milliseconds (default: 5000).
//          * @return the resolved IP address list.
//          */
//         template <typename T = Transport, typename std::enable_if <is_multicast_dns <T>::value, int>::type = 0>
//         IpAddressList resolveAllHost (const std::string& host, int family, std::chrono::milliseconds timeout = std::chrono::milliseconds (5000))
//         {
//             return resolveAllHostImpl (host, family, T::multicastAddress (family), T::defaultPort, timeout);
//         }

//         /**
//          * @brief resolve host name and return all IP address found.
//          * @param host host name to resolve.
//          * @param family address family.
//          * @param interface interface name (e.g., "eth0").
//          * @param timeout timeout in milliseconds (default: 5000).
//          * @return the resolved IP address list.
//          */
//         template <typename T = Transport, typename std::enable_if <is_multicast_dns <T>::value, int>::type = 0>
//         static IpAddressList resolveAllHost (const std::string& host, int family, const std::string& interface, std::chrono::milliseconds timeout = std::chrono::milliseconds (5000))
//         {
//             return BasicDnsClient <T> (interface).resolveAllHost (host, family, timeout);
//         }

//         /**
//          * @brief resolve host name and return all IP address found.
//          * @param host host name to resolve.
//          * @param family address family.
//          * @param server server address.
//          * @param port port.
//          * @param timeout timeout in milliseconds (default: 5000).
//          * @return the resolved IP address list.
//          */
//         template <typename T = Transport, typename std::enable_if <!is_multicast_dns <T>::value, int>::type = 0>
//         IpAddressList resolveAllHost (const std::string& host, int family, const IpAddress& server, uint16_t port = T::defaultPort, std::chrono::milliseconds timeout = std::chrono::milliseconds (5000))
//         {
//             return resolveAllHostImpl (host, family, server, port, timeout);
//         }

//         /**
//          * @brief resolve host name and return all IP address found.
//          * @param host host name to resolve.
//          * @param family address family.
//          * @return the resolved IP address list.
//          */
//         template <typename T = Transport, typename std::enable_if <!is_multicast_dns <T>::value, int>::type = 0>
//         static IpAddressList resolveAllHost (const std::string& host, int family)
//         {
//             for (auto const& server : nameServers ())
//             {
//                 IpAddressList addresses = BasicDnsClient <T> ().resolveAllHost (host, family, server);
//                 if (!addresses.empty ())
//                 {
//                     return addresses;
//                 }
//             }

//             return {};
//         }

//         /**
//          * @brief resolve host name and return all IP address found.
//          * @param host host name to resolve.
//          * @param timeout timeout in milliseconds (default: 5000).
//          * @return the resolved IP address list.
//          */
//         template <typename T = Transport, typename std::enable_if <is_multicast_dns <T>::value, int>::type = 0>
//         IpAddressList resolveAllHost (const std::string& host, std::chrono::milliseconds timeout = std::chrono::milliseconds (5000))
//         {
//             IpAddressList addresses;

//             for (auto const& family : { AF_INET6, AF_INET })
//             {
//                 IpAddressList tmp = resolveAllHost (host, family, timeout);
//                 addresses.insert (addresses.end (), tmp.begin (), tmp.end ());
//             }

//             return addresses;
//         }

//         /**
//          * @brief resolve host name and return all IP address found.
//          * @param host host name to resolve.
//          * @param interface interface name.
//          * @param timeout timeout in milliseconds (default: 5000).
//          * @return the resolved IP address list.
//          */
//         template <typename T = Transport, typename std::enable_if <is_multicast_dns <T>::value, int>::type = 0>
//         static IpAddressList resolveAllHost (const std::string& host, const std::string& interface, std::chrono::milliseconds timeout = std::chrono::milliseconds (5000))
//         {
//             return BasicDnsClient <T> (interface).resolveAllHost (host, timeout);
//         }

//         /**
//          * @brief resolve host name and return all IP address found.
//          * @param host host name to resolve.
//          * @param server server address.
//          * @param port port.
//          * @param timeout timeout in milliseconds (default: 5000).
//          * @return the resolved IP address list.
//          */
//         template <typename T = Transport, typename std::enable_if <!is_multicast_dns <T>::value, int>::type = 0>
//         IpAddressList resolveAllHost (const std::string& host, const IpAddress& server, uint16_t port = T::defaultPort, std::chrono::milliseconds timeout = std::chrono::milliseconds (5000))
//         {
//             IpAddressList addresses;

//             for (auto const& family : { AF_INET6, AF_INET })
//             {
//                 IpAddressList tmp = resolveAllHost (host, family, server, port, timeout);
//                 addresses.insert (addresses.end (), tmp.begin (), tmp.end ());
//             }

//             return addresses;
//         }

//         /**
//          * @brief resolve host name and return all IP address found.
//          * @param host host name to resolve.
//          * @return the resolved IP address list.
//          */
//         template <typename T = Transport, typename std::enable_if <!is_multicast_dns <T>::value, int>::type = 0>
//         static IpAddressList resolveAllHost (const std::string& host)
//         {
//             for (auto const& server : nameServers ())
//             {
//                 IpAddressList addresses = BasicDnsClient <T> ().resolveAllHost (host, server);
//                 if (!addresses.empty ())
//                 {
//                     return addresses;
//                 }
//             }

//             return {};
//         }

//         /**
//          * @brief resolve host name using address family.
//          * @param host host name to resolve.
//          * @param family address family.
//          * @param timeout timeout in milliseconds (default: 5000).
//          * @return the first resolved IP address found matching address family.
//          */
//         template <typename T = Transport, typename std::enable_if <is_multicast_dns <T>::value, int>::type = 0>
//         IpAddress resolveHost (const std::string& host, int family, std::chrono::milliseconds timeout = std::chrono::milliseconds (5000))
//         {
//             for (auto const& address : resolveAllHost (host, family, T::multicastAddress (family), timeout))
//             {
//                 return address;
//             }

//             return IpAddress (family);
//         }

//         /**
//          * @brief resolve host name using address family.
//          * @param host host name to resolve.
//          * @param family address family.
//          * @param interface interface name.
//          * @param timeout timeout in milliseconds (default: 5000).
//          * @return the first resolved IP address found matching address family.
//          */
//         template <typename T = Transport, typename std::enable_if <is_multicast_dns <T>::value, int>::type = 0>
//         static IpAddress resolveHost (const std::string& host, int family, const std::string& interface, std::chrono::milliseconds timeout = std::chrono::milliseconds (5000))
//         {
//             return BasicDnsClient <T> (interface).resolveHost (host, family, timeout);
//         }

//         /**
//          * @brief resolve host name using address family.
//          * @param host host name to resolve.
//          * @param family address family.
//          * @param server server address.
//          * @param port port.
//          * @param timeout timeout in milliseconds (default: 5000).
//          * @return the first resolved IP address found matching address family.
//          */
//         template <typename T = Transport, typename std::enable_if <!is_multicast_dns <T>::value, int>::type = 0>
//         IpAddress resolveHost (const std::string& host, int family, const IpAddress& server, uint16_t port = T::defaultPort, std::chrono::milliseconds timeout = std::chrono::milliseconds (5000))
//         {
//             for (auto const& address : resolveAllHost (host, family, server, port, timeout))
//             {
//                 return address;
//             }

//             return IpAddress (family);
//         }

//         /**
//          * @brief resolve host name using address family.
//          * @param host host name to resolve.
//          * @param family address family.
//          * @return the first resolved IP address found matching address family.
//          */
//         template <typename T = Transport, typename std::enable_if <!is_multicast_dns <T>::value, int>::type = 0>
//         static IpAddress resolveHost (const std::string& host, int family)
//         {
//             for (auto const& address : BasicDnsClient <T> ().resolveAllHost (host, family))
//             {
//                 return address;
//             }

//             return IpAddress (family);
//         }

//         /**
//          * @brief resolve host name.
//          * @param host host name to resolve.
//          * @param port port.
//          * @param timeout timeout in milliseconds (default: 5000).
//          * @return the first resolved IP address found matching address family.
//          */
//         template <typename T = Transport, typename std::enable_if <is_multicast_dns <T>::value, int>::type = 0>
//         IpAddress resolveHost (const std::string& host, std::chrono::milliseconds timeout = std::chrono::milliseconds (5000))
//         {
//             for (auto const& family : { AF_INET6, AF_INET })
//             {
//                 for (auto const& address : resolveAllHost (host, family, timeout))
//                 {
//                     return address;
//                 }
//             }

//             return {};
//         }

//         /**
//          * @brief resolve host name.
//          * @param host host name to resolve.
//          * @param interface interface name.
//          * @param timeout timeout in milliseconds (default: 5000).
//          * @return the first resolved IP address found.
//          */
//         template <typename T = Transport, typename std::enable_if <is_multicast_dns <T>::value, int>::type = 0>
//         static IpAddress resolveHost (const std::string& host, const std::string& interface, std::chrono::milliseconds timeout = std::chrono::milliseconds (5000))
//         {
//             return BasicDnsClient <T> (interface).resolveHost (host, timeout);
//         }

//         /**
//          * @brief resolve host name.
//          * @param host host name to resolve.
//          * @param server server address.
//          * @param port port.
//          * @param timeout timeout in milliseconds (default: 5000).
//          * @return the first resolved IP address found matching address family.
//          */
//         template <typename T = Transport, typename std::enable_if <!is_multicast_dns <T>::value, int>::type = 0>
//         IpAddress resolveHost (const std::string& host, const IpAddress& server, uint16_t port = T::defaultPort, std::chrono::milliseconds timeout = std::chrono::milliseconds (5000))
//         {
//             for (auto const& address : resolveAllHost (host, server, port, timeout))
//             {
//                 return address;
//             }

//             return {};
//         }

//         /**
//          * @brief resolve host name.
//          * @param host host name to resolve.
//          * @return the first resolved IP address found.
//          */
//         template <typename T = Transport, typename std::enable_if <!is_multicast_dns <T>::value, int>::type = 0>
//         static IpAddress resolveHost (const std::string& host)
//         {
//             for (auto const& address : resolveAllHost (host))
//             {
//                 return address;
//             }

//             return {};
//         }

//         /**
//          * @brief resolve all host address.
//          * @param address host address to resolve.
//          * @param server server address.
//          * @param port port.
//          * @param timeout timeout in milliseconds (default: 5000).
//          * @return the resolved alias list.
//          */
//         template <typename T = Transport, typename std::enable_if <!is_multicast_dns <T>::value, int>::type = 0>
//         AliasList resolveAllAddress(const IpAddress& address, const IpAddress& server, uint16_t port = T::defaultPort, std::chrono::milliseconds timeout = std::chrono::milliseconds (5000))
//         {
//             AliasList aliases;

//             DnsPacket packet;
//             packet.src = IpAddress (server.family ());
//             packet.dest = server;
//             packet.port = port;

//             QuestionRecord question;
//             question.host = address.toArpa ();
//             question.type = RecordType::PTR;
//             question.dnsclass = RecordClass::IN;

//             packet.questions.push_back (question);

//             if (lookup (packet, timeout) == -1)
//             {
//                 return aliases;
//             }

//             for (auto const& answer : packet.answers)
//             {
//                 if (!answer.name.empty ())
//                 {
//                     aliases.insert (answer.name);
//                 }
//             }

//             return aliases;
//         }

//         /**
//          * @brief resolve all host address.
//          * @param address host address to resolve.
//          * @return the resolved alias list.
//          */
//         template <typename T = Transport, typename std::enable_if <!is_multicast_dns <T>::value, int>::type = 0>
//         static AliasList resolveAllAddress (const IpAddress& address)
//         {
//             for (auto const& server : nameServers ())
//             {
//                 AliasList aliases = BasicDnsClient <T> ().resolveAllAddress (address, server);
//                 if (!aliases.empty ())
//                 {
//                     return aliases;
//                 }
//             }

//             return {};
//         }

//         /**
//          * @brief resolve host address.
//          * @param address host address to resolve.
//          * @param server server address.
//          * @param port port.
//          * @param timeout timeout in milliseconds (default: 5000).
//          * @return the first resolved alias.
//          */
//         template <typename T = Transport, typename std::enable_if <!is_multicast_dns <T>::value, int>::type = 0>
//         std::string resolveAddress (const IpAddress& address, const IpAddress& server, uint16_t port = T::defaultPort, std::chrono::milliseconds timeout = std::chrono::milliseconds (5000))
//         {
//             for (auto const& alias : resolveAllAddress (address, server, port, timeout))
//             {
//                 return alias;
//             }

//             return {};
//         }

//         /**
//          * @brief resolve host address.
//          * @param host host address to resolve.
//          * @return the first resolved alias.
//          */
//         template <typename T = Transport, typename std::enable_if <!is_multicast_dns <T>::value, int>::type = 0>
//         static std::string resolveAddress (const IpAddress& address)
//         {
//             for (auto const& alias : resolveAllAddress (address))
//             {
//                 return alias;
//             }

//             return {};
//         }

//         /**
//          * @brief resolve all host name server.
//          * @param host host name to resolve.
//          * @param server server address.
//          * @param port port.
//          * @param timeout timeout in milliseconds (default: 5000).
//          * @return the resolved name server list.
//          */
//         template <typename T = Transport, typename std::enable_if <!is_multicast_dns <T>::value, int>::type = 0>
//         ServerList resolveAllNameServer (const std::string& host, const IpAddress& server, uint16_t port = T::defaultPort, std::chrono::milliseconds timeout = std::chrono::milliseconds (5000))
//         {
//             ServerList names;

//             DnsPacket packet;
//             packet.src = IpAddress (server.family ());
//             packet.dest = server;
//             packet.port = port;

//             QuestionRecord question;
//             question.host = host;
//             question.type = RecordType::NS;
//             question.dnsclass = RecordClass::IN;

//             packet.questions.push_back (question);

//             if (lookup (packet, timeout) == -1)
//             {
//                 return names;
//             }

//             for (auto const& answer : packet.answers)
//             {
//                 if (!answer.name.empty ())
//                 {
//                     names.insert (answer.name);
//                 }
//             }

//             return names;
//         }

//         /**
//          * @brief resolve all host name server.
//          * @param host host name to resolve.
//          * @return the resolved name server list.
//          */
//         template <typename T = Transport, typename std::enable_if <!is_multicast_dns <T>::value, int>::type = 0>
//         static ServerList resolveAllNameServer (const std::string& host)
//         {
//             for (auto const& server : nameServers ())
//             {
//                 ServerList names = BasicDnsClient <T> ().resolveAllNameServer (host, server);
//                 if (!names.empty ())
//                 {
//                     return names;
//                 }
//             }

//             return {};
//         }

//         /**
//          * @brief resolve host name server.
//          * @param host host name to resolve.
//          * @param server server address.
//          * @param port port.
//          * @param timeout timeout in milliseconds (default: 5000).
//          * @return the first resolved name server.
//          */
//         template <typename T = Transport, typename std::enable_if <!is_multicast_dns <T>::value, int>::type = 0>
//         std::string resolveNameServer (const std::string& host, const IpAddress& server, uint16_t port = T::defaultPort, std::chrono::milliseconds timeout = std::chrono::milliseconds (5000))
//         {
//             for (auto const& name : resolveAllNameServer (host, server, port, timeout))
//             {
//                 return name;
//             }

//             return {};
//         }

//         /**
//          * @brief resolve host name server.
//          * @param host host name to resolve.
//          * @return the first resolved name server.
//          */
//         template <typename T = Transport, typename std::enable_if <!is_multicast_dns <T>::value, int>::type = 0>
//         static std::string resolveNameServer (const std::string& host)
//         {
//             for (auto const& name : resolveAllNameServer (host))
//             {
//                 return name;
//             }

//             return {};
//         }

//         /**
//          * @brief resolve host start of authority name server.
//          * @param host host name to resolve.
//          * @param server server address.
//          * @param port port.
//          * @param timeout timeout in milliseconds (default: 5000).
//          * @return the start of authority name server.
//          */
//         template <typename T = Transport, typename std::enable_if <!is_multicast_dns <T>::value, int>::type = 0>
//         std::string resolveAuthority (const std::string& host, const IpAddress& server, uint16_t port = T::defaultPort, std::chrono::milliseconds timeout = std::chrono::milliseconds (5000))
//         {
//             DnsPacket packet;
//             packet.src = IpAddress (server.family ());
//             packet.dest = server;
//             packet.port = port;

//             QuestionRecord question;
//             question.host = host;
//             question.type = RecordType::SOA;
//             question.dnsclass = RecordClass::IN;

//             packet.questions.push_back (question);

//             if (lookup (packet, timeout) == 0)
//             {
//                 for (auto const& answer : packet.answers)
//                 {
//                     if (!answer.name.empty ())
//                     {
//                         return answer.name;
//                     }
//                 }
//             }

//             return {};
//         }

//         /**
//          * @brief resolve host start of authority name server.
//          * @param host host name to resolve.
//          * @return the start of authority name server.
//          */
//         template <typename T = Transport, typename std::enable_if <!is_multicast_dns <T>::value, int>::type = 0>
//         static std::string resolveAuthority (const std::string& host)
//         {
//             for (auto const& server : nameServers ())
//             {
//                 std::string name = BasicDnsClient <T> ().resolveAuthority (host, server);
//                 if (!name.empty ())
//                 {
//                     return name;
//                 }
//             }

//             return {};
//         }

//         /**
//          * @brief resolve all host mail exchanger.
//          * @param host host name to resolve.
//          * @param server server address.
//          * @param port port.
//          * @param timeout timeout in milliseconds (default: 5000).
//          * @return the resolved mail exchanger list.
//          */
//         template <typename T = Transport, typename std::enable_if <!is_multicast_dns <T>::value, int>::type = 0>
//         ExchangerList resolveAllMailExchanger (const std::string& host, const IpAddress& server, uint16_t port = T::defaultPort, std::chrono::milliseconds timeout = std::chrono::milliseconds (5000))
//         {
//             ExchangerList exchangers;

//             DnsPacket packet;
//             packet.src = IpAddress (server.family ());
//             packet.dest = server;
//             packet.port = port;

//             QuestionRecord question;
//             question.host = host;
//             question.type = RecordType::MX;
//             question.dnsclass = RecordClass::IN;

//             packet.questions.push_back (question);

//             if (lookup (packet, timeout) == -1)
//             {
//                 return exchangers;
//             }

//             for (auto const& answer : packet.answers)
//             {
//                 if (!answer.name.empty ())
//                 {
//                     exchangers.insert (answer.name);
//                 }
//             }

//             return exchangers;
//         }

//         /**
//          * @brief resolve all host mail exchanger.
//          * @param host host name to resolve.
//          * @return the resolved mail exchanger list.
//          */
//         template <typename T = Transport, typename std::enable_if <!is_multicast_dns <T>::value, int>::type = 0>
//         static ExchangerList resolveAllMailExchanger (const std::string& host)
//         {
//             for (auto const& server : nameServers ())
//             {
//                 ExchangerList aliases = BasicDnsClient <T> ().resolveAllMailExchanger (host, server);
//                 if (!aliases.empty ())
//                 {
//                     return aliases;
//                 }
//             }

//             return {};
//         }

//         /**
//          * @brief resolve host mail exchanger.
//          * @param host host name to resolve.
//          * @param server server address.
//          * @param port port.
//          * @param timeout timeout in milliseconds (default: 5000).
//          * @return the first resolved mail exchanger.
//          */
//         template <typename T = Transport, typename std::enable_if <!is_multicast_dns <T>::value, int>::type = 0>
//         std::string resolveMailExchanger (const std::string& host, const IpAddress& server, uint16_t port = T::defaultPort, std::chrono::milliseconds timeout = std::chrono::milliseconds (5000))
//         {
//             for (auto const& exchanger : resolveAllMailExchanger (host, server, port, timeout))
//             {
//                 return exchanger;
//             }

//             return {};
//         }

//         /**
//          * @brief resolve host mail exchanger.
//          * @param host host name to resolve.
//          * @return the first resolved mail exchanger.
//          */
//         template <typename T = Transport, typename std::enable_if <!is_multicast_dns <T>::value, int>::type = 0>
//         static std::string resolveMailExchanger (const std::string& host)
//         {
//             for (auto const& exchanger : resolveAllMailExchanger (host))
//             {
//                 return exchanger;
//             }

//             return {};
//         }

//         /**
//          * @brief resolve service name.
//          * @param service service name to resolve (ex. "http", "ftp", "ssh" etc...).
//          * @return the port resolved.
//          */
//         template <typename T = Transport, typename std::enable_if <!is_multicast_dns <T>::value, int>::type = 0>
//         static uint16_t resolveService (const std::string& service)
//         {
//             struct servent entry, *res;
//             char buffer[1024];

//             int status = getservbyname_r (service.c_str (), nullptr, &entry, buffer, sizeof buffer, &res);
//             if ((status == 0) && (res != nullptr))
//             {
//                 return ntohs (entry.s_port);
//             }

//             return 0;
//         }

//     protected:
//         /**
//          * @brief send the DNS request.
//          * @param packet DNS packet to send.
//          * @param timeout timeout in milliseconds.
//          * @return 0 on success, -1 on failure.
//          */
//         int lookup (DnsPacket& packet, std::chrono::milliseconds timeout)
//         {
//             if (this->create (packet.src, packet.dest, packet.port, _interface) == -1)
//             {
//                 this->close();
//                 notify (_onFailure, packet);
//                 return -1;
//             }

//             std::stringstream data;
//             uint16_t reqid = join::randomize <uint16_t> ();
//             setHeader (reqid, 1 << 8, packet.questions.size (), 0, 0, 0, data);

//             for (auto const& question : packet.questions)
//             {
//                 encodeQuestion (question.host, question.type, question.dnsclass, data);
//             }

//             if (this->write (data.str ().c_str (), data.str ().length ()) == -1)
//             {
//                 this->close();
//                 notify (_onFailure, packet);
//                 return -1;
//             }

//             uint16_t resid = 0, flags = 0, qcount = 0, ancount = 0, nscount = 0, arcount = 0;
//             auto elapsed = std::chrono::milliseconds::zero ();
//             std::unique_ptr <char []> buf;
            
//             for (;;)
//             {
//                 auto beg = std::chrono::high_resolution_clock::now ();

//                 if ((timeout <= elapsed) || !this->waitReadyRead ((timeout - elapsed).count ()))
//                 {
//                     lastError = std::make_error_code (std::errc::timed_out);
//                     this->close();
//                     notify (_onFailure, packet);
//                     return -1;
//                 }

//                 buf = std::make_unique <char []> (this->canRead ());
//                 if (buf == nullptr)
//                 {
//                     lastError = std::make_error_code (std::errc::not_enough_memory);
//                     this->close();
//                     notify (_onFailure, packet);
//                     return -1;
//                 }

//                 int size = this->read (buf.get (), this->canRead ());
//                 if (size < 0)
//                 {
//                     this->close();
//                     notify (_onFailure, packet);
//                     return -1;
//                 }

//                 data.rdbuf ()->pubsetbuf (buf.get (), size);
//                 getHeader (resid, flags, qcount, ancount, nscount, arcount, data);

//                 if (resid != reqid || !std::bitset <16> (flags).test (15))
//                 {
//                     elapsed += std::chrono::duration_cast <std::chrono::milliseconds> (std::chrono::high_resolution_clock::now () - beg);
//                     continue;
//                 }

//                 int error = std::bitset <4> (flags).to_ulong ();
//                 if (error)
//                 {
//                     lastError = parseError (error);
//                     this->close();
//                     notify (_onFailure, packet);
//                     return -1;
//                 }

//                 break;
//             }

//             packet.questions.clear ();

//             for (uint16_t i = 0; i < qcount; ++i)
//             {
//                 packet.questions.emplace_back (decodeQuestion (data));
//             }

//             for (uint16_t i = 0; i < ancount; ++i)
//             {
//                 packet.answers.emplace_back (decodeAnswer (data));
//             }

//             for (uint16_t i = 0; i < nscount; ++i)
//             {
//                 packet.authorities.emplace_back (decodeAnswer (data));
//             }

//             for (uint16_t i = 0; i < arcount; ++i)
//             {
//                 packet.additionals.emplace_back (decodeAnswer (data));
//             }

//             this->close();
//             notify (_onSuccess, packet);

//             return 0;
//         }

//         /**
//          * @brief resolve host name and return all IP address found.
//          * @param host host name to resolve.
//          * @param family address family.
//          * @param server server address.
//          * @param port port.
//          * @param timeout timeout in milliseconds.
//          * @return the resolved IP address list.
//          */
//         IpAddressList resolveAllHostImpl (const std::string& host, int family, const IpAddress& server, uint16_t port, std::chrono::milliseconds timeout)
//         {
//             IpAddressList addresses;

//             DnsPacket packet;
//             packet.src = IpAddress (server.family ());
//             packet.dest = server;
//             packet.port = port;

//             QuestionRecord question;
//             question.host = host;
//             question.type = (family == AF_INET6) ? RecordType::AAAA : RecordType::A;
//             question.dnsclass = RecordClass::IN;

//             packet.questions.push_back (question);

//             if (lookup (packet, timeout) == -1)
//             {
//                 return addresses;
//             }

//             for (auto const& answer : packet.answers)
//             {
//                 if (!answer.addr.isWildcard ())
//                 {
//                     addresses.push_back (answer.addr);
//                 }
//             }

//             return addresses;
//         }

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
                OUT_ENUM (AAAA);
                OUT_ENUM (ANY);
            }

            return "UNKNOWN";
        }

        /**
         * @brief get record class name.
         * @param recordType record class.
         * @return record class name.
         */
        static std::string className (uint16_t recordClass)
        {
            switch (recordClass)
            {
                OUT_ENUM (IN);
            }

            return "UNKNOWN";
        }

        /**
         * @brief get native handle.
         * @return native handle.
         */
        int handle () const noexcept override
        {
            return _socket.handle ();
        }

        /// notification callback definition.
        using DnsNotify = std::function <void (const DnsPacket&)>;

        /// callback called when a lookup sequence succeed.
        DnsNotify _onSuccess;

        /// callback called when a lookup sequence failed.
        DnsNotify _onFailure;

    protected:
        void onReceive () override
        {
        }

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
        void setHeader (uint16_t id, uint16_t flags, uint16_t qcount, uint16_t ancount, uint16_t nscount, uint16_t arcount, std::stringstream& data)
        {
            id = htons (id);
            data.write (reinterpret_cast <char *> (&id), sizeof (id));

            flags = htons (flags);
            data.write (reinterpret_cast <char *> (&flags), sizeof (flags));

            qcount = htons (qcount);
            data.write (reinterpret_cast <char *> (&qcount), sizeof (qcount));

            ancount = htons (ancount);
            data.write (reinterpret_cast <char *> (&ancount), sizeof (ancount));

            nscount = htons (nscount);
            data.write (reinterpret_cast <char *> (&nscount), sizeof (nscount));

            arcount = htons (arcount);
            data.write (reinterpret_cast <char *> (&arcount), sizeof (arcount));
        }

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
        void getHeader (uint16_t& id, uint16_t& flags, uint16_t& qcount, uint16_t& ancount, uint16_t& nscount, uint16_t& arcount, std::stringstream& data)
        {
            data.read (reinterpret_cast <char *> (&id), sizeof (id));
            id = ntohs (id);

            data.read (reinterpret_cast <char *> (&flags), sizeof (flags));
            flags = ntohs (flags);

            data.read (reinterpret_cast <char *> (&qcount), sizeof (qcount));
            qcount = ntohs (qcount);

            data.read (reinterpret_cast <char *> (&ancount), sizeof (ancount));
            ancount = ntohs (ancount);

            data.read (reinterpret_cast <char *> (&nscount), sizeof (nscount));
            nscount = ntohs (nscount);

            data.read (reinterpret_cast <char *> (&arcount), sizeof (arcount));
            arcount = ntohs (arcount);
        }

        /**
         * @brief encode name.
         * @param host host name to encode.
         * @param data data stream where to store encoded name.
         */
        static void encodeName (const std::string& host, std::stringstream& data)
        {
            std::istringstream iss (host);

            for (std::string token; std::getline (iss, token, '.');)
            {
                data << static_cast <uint8_t> (token.size ());
                data << token;
            }

            data << '\0';
        }

        /**
         * @brief decode name.
         * @param data stream where the encoded name is stored.
         * @return decoded name.
         */
        static std::string decodeName (std::stringstream& data)
        {
            std::string decoded;

            for (;;)
            {
                auto pos = data.tellg ();

                uint16_t offset = 0;
                data.read (reinterpret_cast <char *> (&offset), sizeof (offset));
                offset = ntohs (offset);

                if (offset & 0xC000)
                {
                    pos = data.tellg ();
                    data.seekg (std::bitset <14> (offset).to_ulong ());
                    decoded += decodeName (data);
                    data.seekg (pos);
                    break;
                }
                else
                {
                    data.seekg (pos);

                    uint8_t size = 0;
                    data.read (reinterpret_cast <char *> (&size), sizeof (size));

                    if (size == 0)
                    {
                        if (decoded.back () == '.')
                        {
                            decoded.pop_back ();
                        }
                        break;
                    }

                    decoded.resize (decoded.size () + size);
                    data.read (&decoded[decoded.size () - size], size);
                    decoded += '.';
                }
            }

            return decoded;
        }

        /**
         * @brief decode mail.
         * @param data stream where the encoded mail is stored.
         * @return decoded mail.
         */
        static std::string decodeMail (std::stringstream& data)
        {
            std::string mail = decodeName (data);
            std::size_t pos = 0;

            while ((pos = mail.find (".", pos)) != std::string::npos)
            {
                if (pos > 1 && mail[pos-1] == '/')
                {
                    mail.replace (pos-1, 2, ".");
                }
                else
                {
                    mail.replace (pos, 1, "@");
                    break;
                }
            }

            return mail;
        }

        /**
         * @brief decode question record.
         * @param host host name.
         * @param type record type.
         * @param dnsclass record class.
         * @param data data stream where to store encoded question.
         */
        static void encodeQuestion (const std::string& host, uint16_t type, uint16_t dnsclass, std::stringstream& data)
        {
            encodeName (host, data);

            type = htons (type);
            data.write (reinterpret_cast <char *> (&type), sizeof (type));

            dnsclass = htons (dnsclass);
            data.write (reinterpret_cast <char *> (&dnsclass), sizeof (dnsclass));
        }

        /**
         * @brief decode question record.
         * @param data stream where the encoded mail is stored.
         * @return decoded question record.
         */
        static QuestionRecord decodeQuestion (std::stringstream& data)
        {
            QuestionRecord question;

            question.host = decodeName (data);

            data.read (reinterpret_cast <char *> (&question.type), sizeof (question.type));
            question.type = ntohs (question.type);

            data.read (reinterpret_cast <char *> (&question.dnsclass), sizeof (question.dnsclass));
            question.dnsclass = ntohs (question.dnsclass);

            return question;
        }

        /**
         * @brief decode answer record.
         * @param data stream where the encoded mail is stored.
         * @return decoded answer record.
         */
        static ResourceRecord decodeAnswer (std::stringstream& data)
        {
            ResourceRecord answer;

            answer.host = decodeName (data);

            data.read (reinterpret_cast <char *> (&answer.type), sizeof (answer.type));
            answer.type = ntohs (answer.type);

            data.read (reinterpret_cast <char *> (&answer.dnsclass), sizeof (answer.dnsclass));
            answer.dnsclass = ntohs (answer.dnsclass);

            data.read (reinterpret_cast <char *> (&answer.ttl), sizeof (answer.ttl));
            answer.ttl = ntohl (answer.ttl);

            uint16_t dataLen = 0;
            data.read (reinterpret_cast <char *> (&dataLen), sizeof (dataLen));
            dataLen = ntohs (dataLen);

            if (answer.type == RecordType::A)
            {
                struct in_addr addr;
                data.read (reinterpret_cast <char *> (&addr), sizeof (addr));
                answer.addr = IpAddress (&addr, sizeof (struct in_addr));
            }
            else if (answer.type == RecordType::NS)
            {
                answer.name = decodeName (data);
            }
            else if (answer.type == RecordType::CNAME)
            {
                answer.name = decodeName (data);
            }
            else if (answer.type == RecordType::SOA)
            {
                answer.name = decodeName (data);

                answer.mail = decodeMail (data);

                data.read (reinterpret_cast <char *> (&answer.serial), sizeof (answer.serial));
                answer.serial = ntohl (answer.serial);

                data.read (reinterpret_cast <char *> (&answer.refresh), sizeof (answer.refresh));
                answer.refresh = ntohl (answer.refresh);

                data.read (reinterpret_cast <char *> (&answer.retry), sizeof (answer.retry));
                answer.retry = ntohl (answer.retry);

                data.read (reinterpret_cast <char *> (&answer.expire), sizeof (answer.expire));
                answer.expire = ntohl (answer.expire);

                data.read (reinterpret_cast <char *> (&answer.minimum), sizeof (answer.minimum));
                answer.minimum = ntohl (answer.minimum);
            }
            else if (answer.type == RecordType::PTR)
            {
                answer.name = decodeName (data);
            }
            else if (answer.type == RecordType::MX)
            {
                data.read (reinterpret_cast <char *> (&answer.mxpref), sizeof (answer.mxpref));
                answer.mxpref = ntohs (answer.mxpref);

                answer.name = decodeName (data);
            }
            else if (answer.type == RecordType::AAAA)
            {
                struct in6_addr addr;
                data.read (reinterpret_cast <char *> (&addr), sizeof (addr));
                answer.addr = IpAddress (&addr, sizeof (struct in6_addr));
            }

            return answer;
        }

        /**
         * @brief convert DNS error to system error code.
         * @param error DNS error number.
         * @return system error.
         */
        static std::error_code parseError (int error)
        {
            std::error_code code;

            switch (error)
            {
                case 0:
                case 3:
                    code = make_error_code (Errc::NotFound);
                    break;
                case 1:
                case 4:
                    code = make_error_code (Errc::InvalidParam);
                    break;
                case 2:
                    code = make_error_code (Errc::OperationFailed);
                    break;
                case 5:
                    code = make_error_code (Errc::PermissionDenied);
                    break;
                default:
                    code = make_error_code (Errc::UnknownError);
                    break;
            }

            return code;
        }

    #ifdef DEBUG
        /**
         * @brief default callback called when a lookup sequence succeed.
         * @param packet DNS packet.
         */
        static void defaultOnSuccess (const DnsPacket& packet)
        {
            std::cout << std::endl;
            std::cout << "SERVER: " << packet.dest << "#" << packet.port << std::endl;

            std::cout << std::endl;
            std::cout << ";; QUESTION SECTION: " << std::endl;
            for (auto const& question : packet.questions)
            {
                std::cout << question.host;
                std::cout << "  " << typeName (question.type);
                std::cout << "  " << className (question.dnsclass);
                std::cout << std::endl;
            }

            std::cout << std::endl;
            std::cout << ";; ANSWER SECTION: " << std::endl;
            for (auto const& answer : packet.answers)
            {
                std::cout << answer.host;
                std::cout << "  " << typeName (answer.type);
                std::cout << "  " << className (answer.dnsclass);
                std::cout << "  " << answer.ttl;
                if (answer.type == RecordType::A)
                {
                    std::cout << "  " << answer.addr;
                }
                else if (answer.type == RecordType::NS)
                {
                    std::cout << "  " << answer.name;
                }
                else if (answer.type == RecordType::CNAME)
                {
                    std::cout << "  " << answer.name;
                }
                else if (answer.type == RecordType::SOA)
                {
                    std::cout << "  " << answer.name;
                    std::cout << "  " << answer.mail;
                    std::cout << "  " << answer.serial;
                    std::cout << "  " << answer.refresh;
                    std::cout << "  " << answer.retry;
                    std::cout << "  " << answer.expire;
                    std::cout << "  " << answer.minimum;
                }
                else if (answer.type == RecordType::PTR)
                {
                    std::cout << "  " << answer.name;
                }
                else if (answer.type == RecordType::MX)
                {
                    std::cout << "  " << answer.mxpref;
                    std::cout << "  " << answer.name;
                }
                else if (answer.type == RecordType::AAAA)
                {
                    std::cout << "  " << answer.addr;
                }
                std::cout << std::endl;
            }
        }

        /**
         * @brief default callback called when a lookup sequence failed.
         * @param packet DNS packet.
         */
        static void defaultOnFailure (const DnsPacket& packet)
        {
            std::cout << std::endl;
            std::cout << "SERVER: " << packet.dest << "#" << packet.port << std::endl;

            std::cout << std::endl;
            std::cout << ";; QUESTION SECTION: " << std::endl;
            for (auto const& question : packet.questions)
            {
                std::cout << question.host;
                std::cout << "  " << typeName (question.type);
                std::cout << "  " << className (question.dnsclass);
                std::cout << std::endl;
            }

            std::cout << std::endl;
            std::cout << lastError.message () << std::endl;
        }
    #endif

        /**
         * @brief safe way to notify DNS events.
         * @param function function to call.
         * @param packet DNS packet.
         */
        void notify (const DnsNotify& function, const DnsPacket& packet)
        {
            if (function)
            {
                function (packet);
            }
        }

        /// interface name.
        std::string _interface;

        ///
        std::unique_ptr <uint8_t[]> _buffer;

        ///
        typename Transport::Socket _socket;
    };

    /**
     * @brief multicast DNS transport
     */
    class Mdns
    {
    public:
        using Client = BasicDnsClient <Mdns>;
        using Socket = Udp::Socket;

        /**
         * @brief get multicast address for the given address family.
         * @param family Address family.
         * @return multicast IP address.
         */
        static IpAddress multicastAddress (int family)
        {
            return (family == AF_INET6) ? "ff02::fb" : "224.0.0.251";
        }

        /**
         * @brief create and initialize the socket.
         * @param socket socket object to create and initialize.
         * @param interface network interface name to bind to.
         * @param family IP address family.
         * @return 0 on success, -1 on failure
         */
        static int create (Socket& socket, const std::string& interface, int family)
        {
            IpAddress maddress = multicastAddress (family);

            if ((socket.bind (IpAddress (family)) == -1) || (socket.bindToDevice (interface) == -1))
            {
                socket.close ();
                return -1;
            }

            if (socket.setOption (Socket::Option::ReusePort, 1) == -1)
            {
                socket.close ();
                return -1;
            }

            if (family == AF_INET6)
            {
                struct ipv6_mreq mreq;
                ::memset (&mreq, 0, sizeof (mreq));
                ::memcpy (&mreq.ipv6mr_multiaddr, maddress.addr (), maddress.length ());
                if (setsockopt (socket.handle (), IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, &mreq, sizeof (mreq)) == -1)
                {
                    lastError = std::make_error_code (static_cast <std::errc> (errno));
                    socket.close ();
                    return -1;
                }
            }
            else
            {
                struct ip_mreq mreq;
                ::memset (&mreq, 0, sizeof (mreq));
                ::memcpy (&mreq.imr_multiaddr, maddress.addr (), maddress.length ());
                if (setsockopt (socket.handle (), IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof (mreq)) == -1)
                {
                    lastError = std::make_error_code (static_cast <std::errc> (errno));
                    socket.close ();
                    return -1;
                }
            }

            if (socket.connect ({maddress, defaultPort}) == -1)
            {
                socket.close ();
                return -1;
            }

            return 0;
        }

        /// default port for mDNS.
        static constexpr uint16_t defaultPort = 5353;
    };

    /**
     * @brief standard DNS transport
     */
    class Dns
    {
    public:
        using Client = BasicDnsClient <Dns>;
        using Socket = Udp::Socket;

        /**
         * @brief create and initialize the socket.
         * @param socket socket object to create and initialize.
         * @param interface network interface name to bind to.
         * @param server remote server address.
         * @param port remote server port.
         * @return 0 on success, -1 on failure
         */
        static int create (Socket& socket, const std::string& interface, const IpAddress& server, uint16_t port = defaultPort)
        {
            if ((socket.bind (IpAddress (server.family ())) == -1) || (socket.bindToDevice (interface) == -1))
            {
                socket.close ();
                return -1;
            }

            if (socket.connect ({server, port}) == -1)
            {
                socket.close ();
                return -1;
            }

            return 0;
        }

        /// default port for DNS.
        static constexpr uint16_t defaultPort = 53;
    };

    /**
     * @brief DNS over TLS transport
     */
    class Dot
    {
    public:
        using Client = BasicDnsClient <Dot>;
        using Socket = Tls::Socket;

        /**
         * @brief create and initialize the socket.
         * @param socket socket object to create and initialize.
         * @param interface network interface name to bind to.
         * @param server remote server address.
         * @param port remote server port.
         * @return 0 on success, -1 on failure
         */
        static int create (Socket& socket, const std::string& interface, const IpAddress& server, uint16_t port = defaultPort)
        {
            if ((socket.bind (IpAddress (server.family ())) == -1) || (socket.bindToDevice (interface) == -1))
            {
                socket.close ();
                return -1;
            }

            if (socket.connectEncrypted ({server, port}) == -1)
            {
                if (lastError != Errc::TemporaryError)
                {
                    socket.close ();
                    return -1;
                }

                if (!socket.waitEncrypted ())
                {
                    socket.close ();
                    return -1;
                }
            }

            return 0;
        }

        /// default port for DoT.
        static constexpr uint16_t defaultPort = 853;
    };
}

#endif
