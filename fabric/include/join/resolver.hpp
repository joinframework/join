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

#ifndef JOIN_FABRIC_RESOLVER_HPP
#define JOIN_FABRIC_RESOLVER_HPP

// libjoin.
#include <join/condition.hpp>
#include <join/dnsbase.hpp>

// C++.
#include <chrono>

// C.
#include <arpa/nameser.h>
#include <netinet/in.h>
#include <resolv.h>
#include <netdb.h>

namespace join
{
    /**
     * @brief .
     */
    template <typename TransportPolicy>
    class BasicDnsResolver : public BasicDns<TransportPolicy>
    {
    public:
        using Endpoint = typename BasicDns<TransportPolicy>::Endpoint;
        using RecordType = typename BasicDns<TransportPolicy>::RecordType;
        using RecordClass = typename BasicDns<TransportPolicy>::RecordClass;

        /**
         * @brief create the resolver instance binded to the given interface.
         * @param interface interface to use.
         * @param args additional arguments (see. transport policies ::create).
         */
        template <typename... Args>
        explicit BasicDnsResolver (const std::string& interface = "", Args&&... args)
        : BasicDns<TransportPolicy> ()
#ifdef DEBUG
        , _onSuccess (defaultOnSuccess)
        , _onFailure (defaultOnFailure)
#else
        , _onSuccess (nullptr)
        , _onFailure (nullptr)
#endif
        {
            if (this->_transport.create (this, interface, std::forward<Args> (args)...) == -1)
            {
                throw std::system_error (lastError);
            }
        }

        /**
         * @brief create instance by copy.
         */
        BasicDnsResolver (const BasicDnsResolver&) = delete;

        /**
         * @brief create instance by move.
         */
        BasicDnsResolver& operator= (const BasicDnsResolver&) = delete;

        /**
         * @brief assign instance by copy.
         */
        BasicDnsResolver (BasicDnsResolver&&) = delete;

        /**
         * @brief assign instance by move.
         */
        BasicDnsResolver& operator= (BasicDnsResolver&&) = delete;

        /**
         * @brief destroy instance.
         */
        virtual ~BasicDnsResolver () noexcept
        {
            this->_transport.close ();
        }

        /**
         * @brief get IP address of the currently configured name servers.
         * @return a list of configured name servers.
         */
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
                    // LCOV_EXCL_START: requires specific host IPv6 configuration.
                    else if (res._u._ext.nsaddrs[i] != nullptr && res._u._ext.nsaddrs[i]->sin6_family == AF_INET6)
                    {
                        addressList.emplace_back (&res._u._ext.nsaddrs[i]->sin6_addr, sizeof (struct in6_addr));
                    }
                    // LCOV_EXCL_STOP
                }
                res_nclose (&res);
            }

            return addressList;
        }

        /**
         * @brief resolve host name and return all IP address found.
         * @param host host name to resolve.
         * @param family address family.
         * @param timeout timeout in milliseconds (default: 5000).
         * @return the resolved IP address list.
         */
        IpAddressList resolveAllAddress (const std::string& host, int family,
                                         std::chrono::milliseconds timeout = std::chrono::seconds (5))
        {
            if (host.empty ())
            {
                return {};
            }

            DnsPacket packet{};
            packet.id = join::randomize<uint16_t> ();
            packet.flags = 1 << 8;

            QuestionRecord question;
            question.host = host;
            RecordType expected = (family == AF_INET6) ? RecordType::AAAA : RecordType::A;
            question.type = expected;
            question.dnsclass = RecordClass::IN;

            packet.questions.push_back (question);

            if (this->query (packet, timeout) == -1)
            {
                return {};
            }

            IpAddressList addresses;

            for (auto const& answer : packet.answers)
            {
                if (!answer.addr.isWildcard () && (answer.type == expected))
                {
                    addresses.push_back (answer.addr);
                }
            }

            return addresses;
        }

        /**
         * @brief resolve host name using system name servers and return all IP addresses found.
         * @param host host name to resolve.
         * @param family address family.
         * @return the resolved IP address list, empty on error.
         */
        static IpAddressList lookupAllAddress (const std::string& host, int family)
        {
            for (auto const& server : nameServers ())
            {
                IpAddressList addresses =
                    BasicDnsResolver<TransportPolicy> ("", server).resolveAllAddress (host, family);
                if (!addresses.empty ())
                {
                    return addresses;
                }
            }

            return {};
        }

        /**
         * @brief resolve host name and return all IP address found.
         * @param host host name to resolve.
         * @param timeout timeout in milliseconds (default: 5000).
         * @return the resolved IP address list.
         */
        IpAddressList resolveAllAddress (const std::string& host,
                                         std::chrono::milliseconds timeout = std::chrono::seconds (5))
        {
            IpAddressList addresses;

            for (auto const& family : {AF_INET, AF_INET6})
            {
                IpAddressList tmp = resolveAllAddress (host, family, timeout);
                addresses.insert (addresses.end (), tmp.begin (), tmp.end ());
            }

            return addresses;
        }

        /**
         * @brief resolve host name using system name servers and return all IP addresses found.
         * @param host host name to resolve.
         * @return the resolved IP address list, empty on error.
         */
        static IpAddressList lookupAllAddress (const std::string& host)
        {
            for (auto const& server : nameServers ())
            {
                IpAddressList addresses = BasicDnsResolver<TransportPolicy> ("", server).resolveAllAddress (host);
                if (!addresses.empty ())
                {
                    return addresses;
                }
            }

            return {};
        }

        /**
         * @brief resolve host name using address family.
         * @param host host name to resolve.
         * @param family address family.
         * @param timeout timeout in milliseconds (default: 5000).
         * @return the first resolved IP address found matching address family.
         */
        IpAddress resolveAddress (const std::string& host, int family,
                                  std::chrono::milliseconds timeout = std::chrono::seconds (5))
        {
            for (auto const& address : resolveAllAddress (host, family, timeout))
            {
                return address;
            }

            return IpAddress (family);
        }

        /**
         * @brief resolve host name using system name servers.
         * @param host host name to resolve.
         * @param family address family.
         * @return the first resolved IP address found, wildcard address on error.
         */
        static IpAddress lookupAddress (const std::string& host, int family)
        {
            for (auto const& address : lookupAllAddress (host, family))
            {
                return address;
            }

            return IpAddress (family);
        }

        /**
         * @brief resolve host name.
         * @param host host name to resolve.
         * @param timeout timeout in milliseconds (default: 5000).
         * @return the first resolved IP address found.
         */
        IpAddress resolveAddress (const std::string& host, std::chrono::milliseconds timeout = std::chrono::seconds (5))
        {
            for (auto const& address : resolveAllAddress (host, timeout))
            {
                return address;
            }

            return {};
        }

        /**
         * @brief resolve host name using system name servers.
         * @param host host name to resolve.
         * @return the first resolved IP address found, wildcard address on error.
         */
        static IpAddress lookupAddress (const std::string& host)
        {
            for (auto const& address : lookupAllAddress (host))
            {
                return address;
            }

            return {};
        }

        /**
         * @brief resolve all host address.
         * @param address host address to resolve.
         * @param timeout timeout in milliseconds (default: 5000).
         * @return the resolved alias list.
         */
        AliasList resolveAllName (const IpAddress& address,
                                  std::chrono::milliseconds timeout = std::chrono::seconds (5))
        {
            if (address.isWildcard ())
            {
                return {};
            }

            DnsPacket packet{};
            packet.id = join::randomize<uint16_t> ();
            packet.flags = 1 << 8;

            QuestionRecord question;
            question.host = address.toArpa ();
            question.type = RecordType::PTR;
            question.dnsclass = RecordClass::IN;

            packet.questions.push_back (question);

            if (this->query (packet, timeout) == -1)
            {
                return {};
            }

            AliasList aliases;

            for (auto const& answer : packet.answers)
            {
                if (!answer.name.empty () && (answer.type == RecordType::PTR))
                {
                    aliases.insert (answer.name);
                }
            }

            return aliases;
        }

        /**
         * @brief resolve all host address.
         * @param address host address to resolve.
         * @return the resolved alias list.
         */
        static AliasList lookupAllName (const IpAddress& address)
        {
            for (auto const& server : nameServers ())
            {
                AliasList aliases = BasicDnsResolver<TransportPolicy> ("", server).resolveAllName (address);
                if (!aliases.empty ())
                {
                    return aliases;
                }
            }

            return {};
        }

        /**
         * @brief resolve host address.
         * @param address host address to resolve.
         * @param timeout timeout in milliseconds (default: 5000).
         * @return the first resolved alias.
         */
        std::string resolveName (const IpAddress& address, std::chrono::milliseconds timeout = std::chrono::seconds (5))
        {
            for (auto const& alias : resolveAllName (address, timeout))
            {
                return alias;
            }

            return {};
        }

        /**
         * @brief resolve host address.
         * @param address host address to resolve.
         * @return the first resolved alias.
         */
        static std::string lookupName (const IpAddress& address)
        {
            for (auto const& alias : lookupAllName (address))
            {
                return alias;
            }

            return {};
        }

        /**
         * @brief resolve all host name server.
         * @param host host name to resolve.
         * @param timeout timeout in milliseconds (default: 5000).
         * @return the resolved name server list.
         */
        ServerList resolveAllNameServer (const std::string& host,
                                         std::chrono::milliseconds timeout = std::chrono::seconds (5))
        {
            if (host.empty ())
            {
                return {};
            }

            DnsPacket packet{};
            packet.id = join::randomize<uint16_t> ();
            packet.flags = 1 << 8;

            QuestionRecord question;
            question.host = host;
            question.type = RecordType::NS;
            question.dnsclass = RecordClass::IN;
            packet.questions.push_back (question);

            if (this->query (packet, timeout) == -1)
            {
                return {};
            }

            ServerList servers;

            for (auto const& answer : packet.answers)
            {
                if (!answer.name.empty () && (answer.type == RecordType::NS))
                {
                    servers.insert (answer.name);
                }
            }

            return servers;
        }

        /**
         * @brief resolve all host name server.
         * @param host host name to resolve.
         * @return the resolved name server list.
         */
        static ServerList lookupAllNameServer (const std::string& host)
        {
            for (auto const& server : nameServers ())
            {
                ServerList servers = BasicDnsResolver<TransportPolicy> ("", server).resolveAllNameServer (host);
                if (!servers.empty ())
                {
                    return servers;
                }
            }

            return {};
        }

        /**
         * @brief resolve host name server.
         * @param host host name to resolve.
         * @param timeout timeout in milliseconds (default: 5000).
         * @return the first resolved name server.
         */
        std::string resolveNameServer (const std::string& host,
                                       std::chrono::milliseconds timeout = std::chrono::seconds (5))
        {
            for (auto const& server : resolveAllNameServer (host, timeout))
            {
                return server;
            }

            return {};
        }

        /**
         * @brief resolve host name server.
         * @param host host name to resolve.
         * @return the first resolved name server.
         */
        static std::string lookupNameServer (const std::string& host)
        {
            for (auto const& server : lookupAllNameServer (host))
            {
                return server;
            }

            return {};
        }

        /**
         * @brief resolve host start of authority name server.
         * @param host host name to resolve.
         * @param timeout timeout in milliseconds (default: 5000).
         * @return the start of authority name server.
         */
        std::string resolveAuthority (const std::string& host,
                                      std::chrono::milliseconds timeout = std::chrono::seconds (5))
        {
            if (host.empty ())
            {
                return {};
            }

            DnsPacket packet{};
            packet.id = join::randomize<uint16_t> ();
            packet.flags = 1 << 8;

            QuestionRecord question;
            question.host = host;
            question.type = RecordType::SOA;
            question.dnsclass = RecordClass::IN;
            packet.questions.push_back (question);

            if (this->query (packet, timeout) == -1)
            {
                return {};
            }

            for (auto const& answer : packet.answers)
            {
                if (!answer.name.empty () && (answer.type == RecordType::SOA))
                {
                    return answer.name;
                }
            }

            return {};
        }

        /**
         * @brief resolve host start of authority name server.
         * @param host host name to resolve.
         * @return the start of authority name server.
         */
        static std::string lookupAuthority (const std::string& host)
        {
            for (auto const& server : nameServers ())
            {
                std::string authority = BasicDnsResolver<TransportPolicy> ("", server).resolveAuthority (host);
                if (!authority.empty ())
                {
                    return authority;
                }
            }

            return {};
        }

        /**
         * @brief resolve all host mail exchanger.
         * @param host host name to resolve.
         * @param timeout timeout in milliseconds (default: 5000).
         * @return the resolved mail exchanger list.
         */
        ExchangerList resolveAllMailExchanger (const std::string& host,
                                               std::chrono::milliseconds timeout = std::chrono::seconds (5))
        {
            if (host.empty ())
            {
                return {};
            }

            DnsPacket packet{};
            packet.id = join::randomize<uint16_t> ();
            packet.flags = 1 << 8;

            QuestionRecord question;
            question.host = host;
            question.type = RecordType::MX;
            question.dnsclass = RecordClass::IN;
            packet.questions.push_back (question);

            if (this->query (packet, timeout) == -1)
            {
                return {};
            }

            ExchangerList exchangers;

            for (auto const& answer : packet.answers)
            {
                if (!answer.name.empty () && (answer.type == RecordType::MX))
                {
                    exchangers.insert (answer.name);
                }
            }

            return exchangers;
        }

        /**
         * @brief resolve all host mail exchanger.
         * @param host host name to resolve.
         * @return the resolved mail exchanger list.
         */
        static ExchangerList lookupAllMailExchanger (const std::string& host)
        {
            for (auto const& server : nameServers ())
            {
                ExchangerList exchangers =
                    BasicDnsResolver<TransportPolicy> ("", server).resolveAllMailExchanger (host);
                if (!exchangers.empty ())
                {
                    return exchangers;
                }
            }

            return {};
        }

        /**
         * @brief resolve host mail exchanger.
         * @param host host name to resolve.
         * @param timeout timeout in milliseconds (default: 5000).
         * @return the first resolved mail exchanger.
         */
        std::string resolveMailExchanger (const std::string& host,
                                          std::chrono::milliseconds timeout = std::chrono::seconds (5))
        {
            for (auto const& exchanger : resolveAllMailExchanger (host, timeout))
            {
                return exchanger;
            }

            return {};
        }

        /**
         * @brief resolve host mail exchanger.
         * @param host host name to resolve.
         * @return the first resolved mail exchanger.
         */
        static std::string lookupMailExchanger (const std::string& host)
        {
            for (auto const& exchanger : lookupAllMailExchanger (host))
            {
                return exchanger;
            }

            return {};
        }

        /**
         * @brief resolve service name.
         * @param service service name to resolve (ex. "http", "ftp", "ssh" etc...).
         * @return the port resolved.
         */
        static uint16_t resolveService (const std::string& service) noexcept
        {
            struct servent entry, *res;
            char buffer[1024];

            int status = getservbyname_r (service.c_str (), nullptr, &entry, buffer, sizeof buffer, &res);
            if ((status == 0) && (res != nullptr))
            {
                return ntohs (entry.s_port);
            }

            return 0;
        }

        /// notification callback definition.
        using DnsNotify = std::function<void (const DnsPacket&)>;

        /// callback called when a lookup sequence succeed.
        DnsNotify _onSuccess;

        /// callback called when a lookup sequence failed.
        DnsNotify _onFailure;

    private:
        /**
         * @brief serialize and send a DNS query, waiting for a response.
         * @param packet DNS packet to send, filled with the response on success.
         * @param timeout query timeout.
         * @return 0 on success, -1 on error.
         */
        int query (DnsPacket& packet, std::chrono::milliseconds timeout)
        {
            packet.src = this->_transport.localEndpoint ().ip ();
            packet.dest = this->_transport.remoteEndpoint ().ip ();
            packet.port = this->_transport.remoteEndpoint ().port ();

            std::stringstream data;
            if (this->serialize (packet, data) == -1)
            {
                lastError = make_error_code (Errc::InvalidParam);
                notify (_onFailure, packet);
                return -1;
            }

            std::string buffer = data.str ();
            if (buffer.size () > TransportPolicy::maxMsgSize)
            {
                lastError = make_error_code (Errc::MessageTooLong);
                notify (_onFailure, packet);
                return -1;
            }

            ScopedLock<Mutex> lock (_syncMutex);

            auto inserted = _pending.emplace (packet.id, std::make_unique<PendingRequest> ());
            if (!inserted.second)
            {
                lastError = make_error_code (Errc::OperationFailed);
                notify (_onFailure, packet);
                return -1;
            }

            if (this->_transport.write (reinterpret_cast<const uint8_t*> (buffer.data ()), buffer.size ()) == -1)
            {
                _pending.erase (inserted.first);
                notify (_onFailure, packet);
                return -1;
            }

            if (!inserted.first->second->cond.timedWait (lock, timeout))
            {
                _pending.erase (inserted.first);
                lastError = make_error_code (Errc::TimedOut);
                notify (_onFailure, packet);
                return -1;
            }

            auto pendingReq = std::move (inserted.first->second);
            _pending.erase (inserted.first);

            if (pendingReq->ec)
            {
                lastError = pendingReq->ec;
                notify (_onFailure, packet);
                return -1;
            }

            packet = std::move (pendingReq->packet);
            notify (_onSuccess, packet);

            return 0;
        }

        /**
         * @brief method called when data are ready to be read on handle.
         * @param fd file descriptor.
         */
        void onReceive ([[maybe_unused]] int fd) override final
        {
            int size = this->_transport.read (this->_buffer.get (), TransportPolicy::maxMsgSize);
            if (size >= 12)
            {
                std::stringstream data;
                data.rdbuf ()->pubsetbuf (reinterpret_cast<char*> (this->_buffer.get ()), size);

                DnsPacket packet;
                this->deserialize (packet, data);
                packet.src = this->_transport.localEndpoint ().ip ();
                packet.dest = this->_transport.remoteEndpoint ().ip ();
                packet.port = this->_transport.remoteEndpoint ().port ();

                if (packet.flags & 0x8000)
                {
                    ScopedLock<Mutex> lock (_syncMutex);

                    auto it = _pending.find (packet.id);
                    if (it != _pending.end ())
                    {
                        it->second->packet = packet;
                        it->second->ec = this->decodeError (packet.flags & 0x000F);
                        if ((packet.flags & 0x0200) && it->second->ec == std::error_code{})
                        {
                            it->second->ec = make_error_code (Errc::MessageTooLong);
                        }
                        it->second->cond.signal ();
                    }
                }
            }
        }

#ifdef DEBUG
        /*
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
                std::cout << "  " << BasicDns<TransportPolicy>::typeName (question.type);
                std::cout << "  " << BasicDns<TransportPolicy>::className (question.dnsclass);
                std::cout << std::endl;
            }

            std::cout << std::endl;
            std::cout << ";; ANSWER SECTION: " << std::endl;
            for (auto const& answer : packet.answers)
            {
                std::cout << answer.host;
                std::cout << "  " << BasicDns<TransportPolicy>::typeName (answer.type);
                std::cout << "  " << BasicDns<TransportPolicy>::className (answer.dnsclass);
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

        /*
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
                std::cout << "  " << BasicDns<TransportPolicy>::typeName (question.type);
                std::cout << "  " << BasicDns<TransportPolicy>::className (question.dnsclass);
                std::cout << std::endl;
            }

            std::cout << std::endl;
            std::cout << lastError.message () << std::endl;
        }
#endif

        /**
         * @brief safe way to notify DNS events.
         * @param func function to call.
         * @param packet DNS packet.
         */
        void notify (const DnsNotify& func, const DnsPacket& packet) const
        {
            if (func)
            {
                func (packet);
            }
        }

        /// pending synchronous request.
        struct PendingRequest
        {
            Condition cond;
            DnsPacket packet;
            std::error_code ec;
        };

        /// synchronous requests indexed by sequence number.
        std::unordered_map<uint16_t, std::unique_ptr<PendingRequest>> _pending;

        /// protection mutex.
        Mutex _syncMutex;
    };
}

#endif
