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

#ifndef JOIN_FABRIC_RESOLVER_HPP
#define JOIN_FABRIC_RESOLVER_HPP

// libjoin.
#include <join/dnsmessage.hpp>
#include <join/condition.hpp>
#include <join/socket.hpp>

// C++.
#include <unordered_map>
#include <chrono>
#include <memory>

// C.
#include <arpa/nameser.h>
#include <netinet/in.h>
#include <resolv.h>
#include <netdb.h>

namespace join
{
    /**
     * @brief basic DNS resolver over datagram socket.
     */
    template <class Protocol>
    class BasicDatagramResolver : public Protocol::Socket
    {
    public:
        using Socket = typename Protocol::Socket;
        using Endpoint = typename Protocol::Endpoint;
        using State = typename Socket::State;

        /**
         * @brief construct the resolver instance.
         * @param server remote DNS server hostname or IP address.
         * @param port remote DNS server port.
         * @param reactor reactor instance.
         */
        explicit BasicDatagramResolver (const std::string& server = {}, uint16_t port = Protocol::defaultPort,
                                        Reactor* reactor = nullptr)
        : Socket ()
#ifdef DEBUG
        , _onSuccess (defaultOnSuccess)
        , _onFailure (defaultOnFailure)
#else
        , _onSuccess (nullptr)
        , _onFailure (nullptr)
#endif
        , _server (server)
        , _port (port)
        , _reactor (reactor ? reactor : ReactorThread::reactor ())
        , _buffer (std::make_unique<char[]> (Protocol::maxMsgSize))
        {
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicDatagramResolver (const BasicDatagramResolver& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to copy.
         * @return a reference to the current object.
         */
        BasicDatagramResolver& operator= (const BasicDatagramResolver& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        BasicDatagramResolver (BasicDatagramResolver&& other) = delete;

        /**
         * @brief move assignment operator.
         * @param other other object to move.
         * @return a reference to the current object.
         */
        BasicDatagramResolver& operator= (BasicDatagramResolver&& other) = delete;

        /**
         * @brief destroy instance.
         */
        virtual ~BasicDatagramResolver () noexcept = default;

        /**
         * @brief make a connection to the given endpoint.
         * @param endpoint endpoint to connect to.
         * @return 0 on success, -1 on failure.
         */
        virtual int connect (const Endpoint& endpoint) override
        {
            if (Socket::connect (endpoint) == -1)
            {
                return -1;
            }

            _server = endpoint.hostname ();
            if (_server.empty ())
            {
                _server = endpoint.ip ().toString ();
            }
            _port = endpoint.port ();

            this->_reactor->addHandler (this->handle (), this);

            return 0;
        }

        /**
         * @brief shutdown the connection.
         * @return 0 on success, -1 on failure.
         */
        virtual int disconnect () override
        {
            this->_reactor->delHandler (this->_handle);

            if (Socket::disconnect () == -1)
            {
                return -1;
            }

            return 0;
        }

        /**
         * @brief resolve host name and return all IP addresses found.
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
            question.type = (family == AF_INET6) ? DnsMessage::RecordType::AAAA : DnsMessage::RecordType::A;
            question.dnsclass = DnsMessage::RecordClass::IN;
            packet.questions.push_back (question);

            if (query (packet, timeout) == -1)
            {
                return {};
            }

            IpAddressList addresses;

            for (auto const& answer : packet.answers)
            {
                if (!answer.addr.isWildcard () && (answer.type == question.type))
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
                    BasicDatagramResolver<Protocol> (server.toString ()).resolveAllAddress (host, family);
                if (!addresses.empty ())
                {
                    return addresses;
                }
            }

            return {};
        }

        /**
         * @brief resolve host name and return all IP addresses found.
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
                IpAddressList addresses = BasicDatagramResolver<Protocol> (server.toString ()).resolveAllAddress (host);
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
            question.type = DnsMessage::RecordType::PTR;
            question.dnsclass = DnsMessage::RecordClass::IN;
            packet.questions.push_back (question);

            if (query (packet, timeout) == -1)
            {
                return {};
            }

            AliasList aliases;

            for (auto const& answer : packet.answers)
            {
                if (!answer.name.empty () && (answer.type == DnsMessage::RecordType::PTR))
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
                AliasList aliases = BasicDatagramResolver<Protocol> (server.toString ()).resolveAllName (address);
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
            question.type = DnsMessage::RecordType::NS;
            question.dnsclass = DnsMessage::RecordClass::IN;
            packet.questions.push_back (question);

            if (query (packet, timeout) == -1)
            {
                return {};
            }

            ServerList servers;

            for (auto const& answer : packet.answers)
            {
                if (!answer.name.empty () && (answer.type == DnsMessage::RecordType::NS))
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
                ServerList servers = BasicDatagramResolver<Protocol> (server.toString ()).resolveAllNameServer (host);
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
            question.type = DnsMessage::RecordType::SOA;
            question.dnsclass = DnsMessage::RecordClass::IN;
            packet.questions.push_back (question);

            if (query (packet, timeout) == -1)
            {
                return {};
            }

            for (auto const& answer : packet.answers)
            {
                if (!answer.name.empty () && (answer.type == DnsMessage::RecordType::SOA))
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
                std::string authority = BasicDatagramResolver<Protocol> (server.toString ()).resolveAuthority (host);
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
            question.type = DnsMessage::RecordType::MX;
            question.dnsclass = DnsMessage::RecordClass::IN;
            packet.questions.push_back (question);

            if (query (packet, timeout) == -1)
            {
                return {};
            }

            ExchangerList exchangers;
            for (auto const& answer : packet.answers)
            {
                if (!answer.name.empty () && (answer.type == DnsMessage::RecordType::MX))
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
                    BasicDatagramResolver<Protocol> (server.toString ()).resolveAllMailExchanger (host);
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
         * @brief get IP address of the currently configured name servers.
         * @return a list of configured name servers.
         */
        static IpAddressList nameServers () noexcept
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

    protected:
        /**
         * @brief check if client must reconnect.
         * @return true if reconnection is required.
         */
        bool needReconnection () noexcept
        {
            return !this->connected ();
        }

        /**
         * @brief reconnect to the remote DNS server.
         * @param endpoint endpoint to connect to.
         * @param timeout timeout in milliseconds.
         * @return 0 on success, -1 on failure.
         */
        virtual int reconnect (const Endpoint& endpoint, [[maybe_unused]] std::chrono::milliseconds timeout)
        {
            if (this->disconnect () == -1)
            {
                this->close ();
                return -1;
            }

            if (this->connect (endpoint) == -1)
            {
                this->close ();
                return -1;
            }

            return 0;
        }

        /**
         * @brief serialize and send a DNS query, waiting for a response.
         * @param packet DNS packet to send, filled with the response on success.
         * @param timeout query timeout.
         * @return 0 on success, -1 on error.
         */
        int query (DnsPacket& packet, std::chrono::milliseconds timeout)
        {
            if (this->_remote.ip ().isWildcard ())
            {
                IpAddress ip = IpAddress::isIpAddress (_server) ? IpAddress (_server)
                                                                : Dns::Resolver::lookupAddress (_server, AF_INET);

                if (ip.isWildcard ())
                {
                    lastError = make_error_code (Errc::InvalidParam);
                    notify (_onFailure, packet);
                    return -1;
                }

                this->_remote.ip (ip);
                this->_remote.port (_port);
            }

            packet.dest = this->_remote.ip ();
            packet.port = this->_remote.port ();

            if (this->needReconnection ())
            {
                Endpoint endpoint{packet.dest, packet.port};
                endpoint.hostname (_server);

                if (this->reconnect (endpoint, timeout) == -1)
                {
                    notify (_onFailure, packet);
                    return -1;
                }
            }

            packet.src = this->localEndpoint ().ip ();

            std::stringstream data;
            if (_message.serialize (packet, data) == -1)
            {
                lastError = make_error_code (Errc::InvalidParam);
                notify (_onFailure, packet);
                return -1;
            }

            std::string buffer = data.str ();
            if (buffer.size () > Protocol::maxMsgSize)
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

            if (this->write (buffer.data (), buffer.size ()) == -1)
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
            int size = this->read (_buffer.get (), Protocol::maxMsgSize);
            if (size < 12)
            {
                return;
            }

            std::stringstream data;
            data.rdbuf ()->pubsetbuf (_buffer.get (), size);

            DnsPacket packet;
            _message.deserialize (packet, data);
            packet.src = this->localEndpoint ().ip ();
            packet.dest = this->remoteEndpoint ().ip ();
            packet.port = this->remoteEndpoint ().port ();

            if (packet.flags & 0x8000)
            {
                ScopedLock<Mutex> lock (_syncMutex);

                auto it = _pending.find (packet.id);
                if (it != _pending.end ())
                {
                    it->second->packet = packet;
                    it->second->ec = DnsMessage::decodeError (packet.flags & 0x000F);
                    if ((packet.flags & 0x0200) && it->second->ec == std::error_code{})
                    {
                        it->second->ec = make_error_code (Errc::MessageTooLong);
                    }
                    it->second->cond.signal ();
                }
            }
        }

        /**
         * @brief method called when handle is closed.
         * @param fd file descriptor.
         */
        void onClose ([[maybe_unused]] int fd) override final
        {
            this->disconnect ();
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
                std::cout << "  " << DnsMessage::typeName (question.type);
                std::cout << "  " << DnsMessage::className (question.dnsclass);
                std::cout << std::endl;
            }

            std::cout << std::endl;
            std::cout << ";; ANSWER SECTION: " << std::endl;
            for (auto const& answer : packet.answers)
            {
                std::cout << answer.host;
                std::cout << "  " << DnsMessage::typeName (answer.type);
                std::cout << "  " << DnsMessage::className (answer.dnsclass);
                std::cout << "  " << answer.ttl;
                if (answer.type == DnsMessage::RecordType::A)
                {
                    std::cout << "  " << answer.addr;
                }
                else if (answer.type == DnsMessage::RecordType::NS)
                {
                    std::cout << "  " << answer.name;
                }
                else if (answer.type == DnsMessage::RecordType::CNAME)
                {
                    std::cout << "  " << answer.name;
                }
                else if (answer.type == DnsMessage::RecordType::SOA)
                {
                    std::cout << "  " << answer.name;
                    std::cout << "  " << answer.mail;
                    std::cout << "  " << answer.serial;
                    std::cout << "  " << answer.refresh;
                    std::cout << "  " << answer.retry;
                    std::cout << "  " << answer.expire;
                    std::cout << "  " << answer.minimum;
                }
                else if (answer.type == DnsMessage::RecordType::PTR)
                {
                    std::cout << "  " << answer.name;
                }
                else if (answer.type == DnsMessage::RecordType::MX)
                {
                    std::cout << "  " << answer.mxpref;
                    std::cout << "  " << answer.name;
                }
                else if (answer.type == DnsMessage::RecordType::AAAA)
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
                std::cout << "  " << DnsMessage::typeName (question.type);
                std::cout << "  " << DnsMessage::className (question.dnsclass);
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
        void notify (const DnsNotify& func, const DnsPacket& packet) const noexcept
        {
            if (func)
            {
                func (packet);
            }
        }

        /// DNS message codec.
        DnsMessage _message;

        /// remote DNS server.
        std::string _server;

        /// remote DNS server port.
        uint16_t _port;

        /// event loop reactor.
        Reactor* _reactor;

        /// reception buffer.
        std::unique_ptr<char[]> _buffer;

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

    /**
     * @brief basic DNS resolver over TLS socket (DNS over TLS).
     */
    template <class Protocol>
    class BasicTlsResolver : public BasicDatagramResolver<Protocol>
    {
    public:
        using Socket = typename Protocol::Socket;
        using Endpoint = typename Protocol::Endpoint;
        using State = typename Socket::State;

        /**
         * @brief construct the DoT resolver instance.
         * @param server remote DNS server address.
         * @param port remote DNS server port.
         * @param reactor reactor instance.
         */
        explicit BasicTlsResolver (const std::string& server = {}, uint16_t port = Protocol::defaultPort,
                                   Reactor* reactor = nullptr)
        : BasicDatagramResolver<Protocol> (server, port, reactor)
        {
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicTlsResolver (const BasicTlsResolver& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to copy.
         * @return a reference to the current object.
         */
        BasicTlsResolver& operator= (const BasicTlsResolver& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        BasicTlsResolver (BasicTlsResolver&& other) = delete;

        /**
         * @brief move assignment operator.
         * @param other other object to move.
         * @return a reference to the current object.
         */
        BasicTlsResolver& operator= (BasicTlsResolver&& other) = delete;

        /**
         * @brief destroy instance.
         */
        virtual ~BasicTlsResolver () noexcept = default;

        /**
         * @brief make a connection to the given endpoint.
         * @param endpoint endpoint to connect to.
         * @return 0 on success, -1 on failure.
         */
        virtual int connect (const Endpoint& endpoint) override
        {
            if (Socket::connect (endpoint) == -1)
            {
                return -1;
            }

            this->_server = this->_remote.hostname ();
            if (this->_server.empty ())
            {
                this->_server = this->_remote.ip ().toString ();
            }
            this->_port = this->_remote.port ();

            return 0;
        }

        /**
         * @brief make an encrypted connection to the given endpoint.
         * @param endpoint endpoint to connect to.
         * @return 0 on success, -1 on failure.
         */
        virtual int connectEncrypted (const Endpoint& endpoint) override
        {
            if (Socket::connectEncrypted (endpoint) == -1)
            {
                return -1;
            }

            this->_reactor->addHandler (this->handle (), this);

            return 0;
        }

        /**
         * @brief wait until TLS handshake is performed or timeout occur (non blocking socket).
         * @param timeout timeout in milliseconds (0: infinite).
         * return true on success, false otherwise.
         */
        virtual bool waitEncrypted (int timeout = 0) override
        {
            if (!Socket::waitEncrypted (timeout))
            {
                return false;
            }

            this->_reactor->addHandler (this->handle (), this);

            return true;
        }

        /**
         * @brief block until connected.
         * @param timeout timeout in milliseconds.
         * @return true if connected, false otherwise.
         */
        virtual bool waitConnected (int timeout = 0) override
        {
            if (!Socket::waitConnected (timeout))
            {
                return false;
            }

            this->_server = this->_remote.hostname ();
            if (this->_server.empty ())
            {
                this->_server = this->_remote.ip ().toString ();
            }
            this->_port = this->_remote.port ();

            return true;
        }

        /**
         * @brief close the TLS connection and reset framing state.
         */
        virtual void close () noexcept override
        {
            Socket::close ();
            _size = 0;
            _offset = 0;
        }

        /**
         * @brief read a framed DoT message (2-byte length prefix).
         * @param data destination buffer.
         * @param maxSize maximum number of bytes to read.
         * @return number of bytes read, or -1 on error.
         */
        virtual int read (char* data, unsigned long maxSize) noexcept override
        {
            if (_offset < _headerSize)
            {
                int nread = Socket::read (data + _offset, _headerSize - _offset);
                if (nread == -1)
                {
                    if (lastError != Errc::TemporaryError)
                    {
                        _offset = 0;
                        _size = 0;
                    }
                    return -1;
                }

                _offset += static_cast<size_t> (nread);

                if (_offset < _headerSize)
                {
                    lastError = make_error_code (Errc::TemporaryError);
                    return -1;
                }

                _size = ntohs (*reinterpret_cast<uint16_t*> (data));

                if (_size > maxSize)
                {
                    lastError = make_error_code (Errc::MessageTooLong);
                    _offset = 0;
                    _size = 0;
                    return -1;
                }
            }

            int nread = Socket::read (data + (_offset - _headerSize), _size - (_offset - _headerSize));
            if (nread == -1)
            {
                if (lastError != Errc::TemporaryError)
                {
                    _offset = 0;
                    _size = 0;
                }
                return -1;
            }

            _offset += static_cast<size_t> (nread);

            if (_offset < (_size + _headerSize))
            {
                lastError = make_error_code (Errc::TemporaryError);
                return -1;
            }

            int msgLen = static_cast<int> (_size);
            _offset = 0;
            _size = 0;

            return msgLen;
        }

        /**
         * @brief write a framed DoT message (2-byte length prefix).
         * @param data source buffer.
         * @param size number of bytes to write.
         * @return number of bytes written, or -1 on error.
         */
        virtual int write (const char* data, unsigned long size) noexcept override
        {
            uint16_t msgLength = htons (static_cast<uint16_t> (size));
            const char* p = reinterpret_cast<const char*> (&msgLength);
            unsigned long remaining = sizeof (msgLength);

            while (remaining > 0)
            {
                int result = Socket::write (p, remaining);
                if (result == -1)
                {
                    if (lastError == Errc::TemporaryError)
                    {
                        if (this->waitReadyWrite ())
                            continue;
                    }
                    return -1;
                }
                p += result;
                remaining -= result;
            }

            p = data;
            remaining = size;

            while (remaining > 0)
            {
                int result = Socket::write (p, remaining);
                if (result == -1)
                {
                    if (lastError == Errc::TemporaryError)
                    {
                        if (this->waitReadyWrite ())
                            continue;
                    }
                    return -1;
                }
                p += result;
                remaining -= result;
            }

            return static_cast<int> (size);
        }

        /**
         * @brief resolve host name using system name servers and return all IP addresses found.
         * @param host host name to resolve.
         * @param family address family.
         * @return the resolved IP address list, empty on error.
         */
        static IpAddressList lookupAllAddress (const std::string& host, int family) = delete;

        /**
         * @brief resolve host name using system name servers and return all IP addresses found.
         * @param host host name to resolve.
         * @return the resolved IP address list, empty on error.
         */
        static IpAddressList lookupAllAddress (const std::string& host) = delete;

        /**
         * @brief resolve host name using system name servers.
         * @param host host name to resolve.
         * @param family address family.
         * @return the first resolved IP address found, wildcard address on error.
         */
        static IpAddress lookupAddress (const std::string& host, int family) = delete;

        /**
         * @brief resolve host name using system name servers.
         * @param host host name to resolve.
         * @return the first resolved IP address found, wildcard address on error.
         */
        static IpAddress lookupAddress (const std::string& host) = delete;

        /**
         * @brief resolve all host address.
         * @param address host address to resolve.
         * @return the resolved alias list.
         */
        static AliasList lookupAllName (const IpAddress& address) = delete;

        /**
         * @brief resolve host address.
         * @param address host address to resolve.
         * @return the first resolved alias.
         */
        static std::string lookupName (const IpAddress& address) = delete;

        /**
         * @brief resolve all host name server.
         * @param host host name to resolve.
         * @return the resolved name server list.
         */
        static ServerList lookupAllNameServer (const std::string& host) = delete;

        /**
         * @brief resolve host name server.
         * @param host host name to resolve.
         * @return the first resolved name server.
         */
        static std::string lookupNameServer (const std::string& host) = delete;

        /**
         * @brief resolve host start of authority name server.
         * @param host host name to resolve.
         * @return the start of authority name server.
         */
        static std::string lookupAuthority (const std::string& host) = delete;

        /**
         * @brief resolve all host mail exchanger.
         * @param host host name to resolve.
         * @return the resolved mail exchanger list.
         */
        static ExchangerList lookupAllMailExchanger (const std::string& host) = delete;

        /**
         * @brief resolve host mail exchanger.
         * @param host host name to resolve.
         * @return the first resolved mail exchanger.
         */
        static std::string lookupMailExchanger (const std::string& host) = delete;

    private:
        /**
         * @brief reconnect to the remote DNS server.
         * @param endpoint endpoint to connect to.
         * @param timeout timeout in milliseconds.
         * @return 0 on success, -1 on failure.
         */
        virtual int reconnect (const Endpoint& endpoint, std::chrono::milliseconds timeout) override
        {
            if (this->disconnect () == -1)
            {
                if (lastError != Errc::TemporaryError)
                {
                    this->close ();
                    return -1;
                }

                if (!this->waitDisconnected (timeout.count ()))
                {
                    this->close ();
                    return -1;
                }
            }

            this->setAlpnProtocols ({"dot"});

            if (this->connectEncrypted (endpoint) == -1)
            {
                if (lastError != Errc::TemporaryError)
                {
                    this->close ();
                    return -1;
                }

                if (!this->waitEncrypted (timeout.count ()))
                {
                    this->close ();
                    return -1;
                }
            }

            return 0;
        }

        /// DOT framing header size.
        static constexpr size_t _headerSize = 2;

        /// total expected payload size.
        size_t _size = 0;

        /// current position in the buffer.
        size_t _offset = 0;
    };
}

#endif
