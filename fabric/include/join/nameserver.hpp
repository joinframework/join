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

#ifndef JOIN_FABRIC_NAMESERVER_HPP
#define JOIN_FABRIC_NAMESERVER_HPP

// libjoin.
#include <join/dnsmessage.hpp>
#include <join/condition.hpp>
#include <join/socket.hpp>

namespace join
{
    /**
     * @brief basic DNS name server over datagram socket.
     */
    template <typename Protocol>
    class BasicDatagramNameServer : public Protocol::Socket
    {
    public:
        using Socket = typename Protocol::Socket;
        using Endpoint = typename Protocol::Endpoint;

        /**
         * @brief construct the name server instance.
         * @param reactor event loop reactor.
         */
        explicit BasicDatagramNameServer (Reactor* reactor = nullptr)
        : Socket ()
        , _reactor (reactor ? reactor : ReactorThread::reactor ())
        , _buffer (std::make_unique<char[]> (Protocol::maxMsgSize))
        {
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicDatagramNameServer (const BasicDatagramNameServer& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to copy.
         * @return a reference to the current object.
         */
        BasicDatagramNameServer& operator= (const BasicDatagramNameServer& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        BasicDatagramNameServer (BasicDatagramNameServer&& other) = delete;

        /**
         * @brief move assignment operator.
         * @param other other object to move.
         * @return a reference to the current object.
         */
        BasicDatagramNameServer& operator= (BasicDatagramNameServer&& other) = delete;

        /**
         * @brief destroy instance.
         */
        virtual ~BasicDatagramNameServer () noexcept = default;

        /**
         * @brief bind the socket to the given endpoint and register with the reactor.
         * @param endpoint endpoint to bind to.
         * @return 0 on success, -1 on failure.
         */
        virtual int bind (const Endpoint& endpoint) noexcept override
        {
            if (Socket::bind (endpoint) == -1)
            {
                return -1;  // LCOV_EXCL_LINE
            }

            _reactor->addHandler (this->handle (), this);

            return 0;
        }

        /**
         * @brief close the socket and unregister from the reactor.
         */
        virtual void close () noexcept override
        {
            _reactor->delHandler (this->handle ());
            Socket::close ();
        }

        /**
         * @brief reply to a DNS query.
         * @param query original query packet.
         * @param answers answer records.
         * @param authorities authority records.
         * @param additionals additional records.
         * @param rcode response code (default: 0 = no error).
         * @return 0 on success, -1 on error.
         */
        int reply (const DnsPacket& query, const std::vector<ResourceRecord>& answers = {},
                   const std::vector<ResourceRecord>& authorities = {},
                   const std::vector<ResourceRecord>& additionals = {}, uint16_t rcode = 0)
        {
            DnsPacket packet{};
            packet.id = query.id;
            packet.flags = (uint16_t (1) << 15) | (query.flags & 0x7800) | (uint16_t (1) << 10) | (rcode & 0x000F);
            packet.dest = query.src;
            packet.port = query.port;

            packet.questions = query.questions;
            packet.answers = answers;
            packet.authorities = authorities;
            packet.additionals = additionals;

            return send (packet);
        }

        /**
         * @brief method called when a DNS query is received.
         * @param packet parsed DNS query received.
         */
        virtual void onQuery (const DnsPacket& packet) = 0;

    protected:
        /**
         * @brief method called when data are ready to be read on handle.
         * @param fd file descriptor.
         */
        virtual void onReadable ([[maybe_unused]] int fd) override
        {
            Endpoint from;
            int size = this->readFrom (_buffer.get (), Protocol::maxMsgSize, &from);
            if (size >= int (_headerSize))
            {
                std::stringstream data;
                data.rdbuf ()->pubsetbuf (_buffer.get (), size);

                DnsPacket packet;
                _message.deserialize (packet, data);
                packet.src = from.ip ();
                packet.dest = this->localEndpoint ().ip ();
                packet.port = from.port ();

                if ((packet.flags & 0x8000) == 0)
                {
                    this->onQuery (packet);
                }
            }
        }

        /**
         * @brief serialize and send a DNS packet.
         * @param packet DNS packet to send.
         * @return 0 on success, -1 on error.
         */
        int send (DnsPacket& packet)
        {
            std::stringstream data;
            if (_message.serialize (packet, data) == -1)
            {
                // LCOV_EXCL_START
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
                // LCOV_EXCL_STOP
            }

            std::string buffer = data.str ();
            if (buffer.size () > Protocol::maxMsgSize)
            {
                // LCOV_EXCL_START
                lastError = make_error_code (Errc::MessageTooLong);
                return -1;
                // LCOV_EXCL_STOP
            }

            if (this->writeTo (buffer.data (), buffer.size (), {packet.dest, packet.port}) == -1)
            {
                return -1;  // LCOV_EXCL_LINE
            }

            return 0;
        };

        /// DNS message header size.
        static constexpr size_t _headerSize = 12;

        /// DNS message codec.
        DnsMessage _message;

        /// event loop reactor.
        Reactor* _reactor;

        /// reception buffer.
        std::unique_ptr<char[]> _buffer;
    };

    /**
     * @brief mDNS peer.
     */
    template <typename Protocol>
    class BasicDatagramPeer : public BasicDatagramNameServer<Protocol>
    {
    public:
        using Socket = typename BasicDatagramNameServer<Protocol>::Socket;
        using Endpoint = typename BasicDatagramNameServer<Protocol>::Endpoint;

        /// DNS notification callback type.
        using DnsNotify = std::function<void (const DnsPacket&)>;

        /// callback called when a lookup sequence succeed.
        DnsNotify onSuccess;

        /// callback called when a lookup sequence failed.
        DnsNotify onFailure;

        /**
         * @brief construct the mDNS peer instance.
         * @param ifindex interface index.
         * @param reactor event loop reactor.
         */
        explicit BasicDatagramPeer (unsigned int ifindex, Reactor* reactor = nullptr)
        : BasicDatagramNameServer<Protocol> (reactor)
#ifdef DEBUG
        , onSuccess (defaultOnSuccess)
        , onFailure (defaultOnFailure)
#else
        , onSuccess (nullptr)
        , onFailure (nullptr)
#endif
        , _ifindex (ifindex)
        {
        }

        /**
         * @brief construct the mDNS peer instance.
         * @param interface interface name.
         * @param reactor event loop reactor.
         */
        explicit BasicDatagramPeer (const std::string& interface, Reactor* reactor = nullptr)
        : BasicDatagramPeer<Protocol> (if_nametoindex (interface.c_str ()), reactor)
        {
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicDatagramPeer (const BasicDatagramPeer& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to copy.
         * @return a reference to the current object.
         */
        BasicDatagramPeer& operator= (const BasicDatagramPeer& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        BasicDatagramPeer (BasicDatagramPeer&& other) = delete;

        /**
         * @brief move assignment operator.
         * @param other other object to move.
         * @return a reference to the current object.
         */
        BasicDatagramPeer& operator= (BasicDatagramPeer&& other) = delete;

        /**
         * @brief destroy instance.
         */
        virtual ~BasicDatagramPeer () noexcept = default;

        using BasicDatagramNameServer<Protocol>::bind;

        /**
         * @brief bind the socket to specified address family.
         * @param family address family.
         * @return 0 on success, -1 on failure.
         */
        int bind (int family) noexcept
        {
            IpAddress maddress = Protocol::multicastAddress (family);
            Endpoint endpoint{IpAddress (family), Protocol::defaultPort};

            if ((this->_state == Socket::State::Closed) && (this->open (endpoint.protocol ()) == -1))
            {
                return -1;  // LCOV_EXCL_LINE
            }

            if (this->setOption (Socket::ReusePort, 1) == -1)
            {
                // LCOV_EXCL_START
                this->close ();
                return -1;
                // LCOV_EXCL_STOP
            }

            if (Socket::bind (endpoint) == -1)
            {
                // LCOV_EXCL_START
                this->close ();
                return -1;
                // LCOV_EXCL_STOP
            }

            if (endpoint.protocol ().family () == AF_INET6)
            {
                ipv6_mreq mreq{};
                ::memcpy (&mreq.ipv6mr_multiaddr, maddress.addr (), maddress.length ());
                mreq.ipv6mr_interface = _ifindex;
                if (::setsockopt (this->handle (), IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, &mreq, sizeof (mreq)) == -1)
                {
                    // LCOV_EXCL_START
                    lastError = std::error_code (errno, std::generic_category ());
                    this->close ();
                    return -1;
                    // LCOV_EXCL_STOP
                }
                if (::setsockopt (this->handle (), IPPROTO_IPV6, IPV6_MULTICAST_IF, &_ifindex, sizeof (_ifindex)) == -1)
                {
                    // LCOV_EXCL_START
                    lastError = std::error_code (errno, std::generic_category ());
                    this->close ();
                    return -1;
                    // LCOV_EXCL_STOP
                }
            }
            else
            {
                // LCOV_EXCL_START: IPv4 multicast not supported by github action containers.
                ip_mreqn mreq{};
                ::memcpy (&mreq.imr_multiaddr, maddress.addr (), maddress.length ());
                mreq.imr_ifindex = static_cast<int> (_ifindex);
                if (::setsockopt (this->handle (), IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof (mreq)) == -1)
                {
                    lastError = std::error_code (errno, std::generic_category ());
                    this->close ();
                    return -1;
                }
                if (::setsockopt (this->handle (), IPPROTO_IP, IP_MULTICAST_IF, &mreq, sizeof (mreq)) == -1)
                {
                    lastError = std::error_code (errno, std::generic_category ());
                    this->close ();
                    return -1;
                }
                // LCOV_EXCL_STOP
            }

#ifndef DEBUG
            if (this->setOption (Socket::MulticastLoop, 0) == -1)
            {
                // LCOV_EXCL_START
                this->close ();
                return -1;
                // LCOV_EXCL_STOP
            }
#endif

            this->_reactor->addHandler (this->handle (), this);

            return 0;
        }

        /**
         * @brief probe the local network for the presence of a service.
         * @param records resource records to query for.
         * @return 0 on success, -1 on error.
         */
        int probe (const std::vector<ResourceRecord>& records)
        {
            if (records.empty ())
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            DnsPacket packet{};
            packet.id = 0;
            packet.flags = 0;
            IpAddress mcast = Protocol::multicastAddress (this->family ());
            packet.dest = IpAddress (mcast.addr (), mcast.length (), _ifindex);
            packet.port = Protocol::defaultPort;

            for (auto const& record : records)
            {
                QuestionRecord question;
                question.host = record.host;
                question.type = DnsMessage::RecordType::ANY;
                question.dnsclass = DnsMessage::RecordClass::IN | 0x8000;
                packet.questions.push_back (question);

                packet.authorities.push_back (record);
            }

            return this->send (packet);
        }

        /**
         * @brief announce the presence of a service on the local network.
         * @param records resource records to announce.
         * @return 0 on success, -1 on error.
         */
        int announce (const std::vector<ResourceRecord>& records)
        {
            if (records.empty ())
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            DnsPacket packet{};
            packet.id = 0;
            packet.flags = (uint16_t (1) << 15) | (uint16_t (1) << 10);
            IpAddress mcast = Protocol::multicastAddress (this->family ());
            packet.dest = IpAddress (mcast.addr (), mcast.length (), _ifindex);
            packet.port = Protocol::defaultPort;

            for (auto const& record : records)
            {
                packet.answers.push_back (record);
            }

            return this->send (packet);
        }

        /**
         * @brief send a goodbye message.
         * @param records resource records to send in goodbye message.
         * @return 0 on success, -1 on error.
         */
        int goodbye (const std::vector<ResourceRecord>& records)
        {
            if (records.empty ())
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            DnsPacket packet{};
            packet.id = 0;
            packet.flags = (uint16_t (1) << 15) | (uint16_t (1) << 10);
            IpAddress mcast = Protocol::multicastAddress (this->family ());
            packet.dest = IpAddress (mcast.addr (), mcast.length (), _ifindex);
            packet.port = Protocol::defaultPort;

            for (auto const& record : records)
            {
                ResourceRecord goodbye = record;
                goodbye.ttl = 0;
                packet.answers.push_back (goodbye);
            }

            return this->send (packet);
        }

        /**
         * @brief browse for services on the local network.
         * @param serviceType service type to browse for (e.g. "_http._tcp.local").
         * @return 0 on success, -1 on error.
         */
        int browse (const std::string& serviceType)
        {
            if (serviceType.empty ())
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            DnsPacket packet{};
            packet.id = 0;
            packet.flags = 0;
            IpAddress mcast = Protocol::multicastAddress (this->family ());
            packet.dest = IpAddress (mcast.addr (), mcast.length (), _ifindex);
            packet.port = Protocol::defaultPort;

            QuestionRecord question;
            question.host = serviceType;
            question.type = DnsMessage::RecordType::PTR;
            question.dnsclass = DnsMessage::RecordClass::IN;
            packet.questions.push_back (question);

            return this->send (packet);
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
         * @brief method called when a DNS query is received.
         * @param packet parsed DNS query received.
         */
        virtual void onAnnouncement (const DnsPacket& packet) = 0;

    protected:
        /**
         * @brief method called when data are ready to be read on handle.
         * @param fd file descriptor.
         */
        void onReadable ([[maybe_unused]] int fd) override final
        {
            Endpoint from;
            int size = this->readFrom (this->_buffer.get (), Protocol::maxMsgSize, &from);
            if (size >= int (this->_headerSize))
            {
                std::stringstream data;
                data.rdbuf ()->pubsetbuf (this->_buffer.get (), size);

                DnsPacket packet;
                this->_message.deserialize (packet, data);
                IpAddress mcast = Protocol::multicastAddress (this->family ());
                packet.src = from.ip ();
                packet.dest = IpAddress (mcast.addr (), mcast.length (), _ifindex);
                packet.port = from.port ();

                if ((packet.flags & 0x8000) == 0)
                {
                    bool unicast = false;
                    for (auto const& q : packet.questions)
                    {
                        if (q.dnsclass & 0x8000)
                        {
                            unicast = true;
                            break;
                        }
                    }

                    if (!unicast)
                    {
                        packet.src = IpAddress (mcast.addr (), mcast.length (), _ifindex);
                    }

                    this->onQuery (packet);
                    return;
                }

                {
                    ScopedLock<Mutex> lock (_syncMutex);

                    auto it = _pending.find (packet.id);
                    if (it != _pending.end ())
                    {
                        it->second->packet = packet;
                        it->second->ec = DnsMessage::decodeError (packet.flags & 0x000F);
                        it->second->cond.signal ();
                        return;
                    }
                }

                onAnnouncement (packet);
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
            std::cout << "PEER: " << packet.dest << "#" << packet.port << std::endl;

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
                else if (answer.type == DnsMessage::RecordType::PTR)
                {
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
            std::cout << "PEER: " << packet.dest << "#" << packet.port << std::endl;

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

        /**
         * @brief serialize and send a DNS query, waiting for a response.
         * @param packet DNS packet to send, filled with the response on success
         * @param timeout query timeout.
         * @return 0 on success, -1 on error.
         */
        int query (DnsPacket& packet, std::chrono::milliseconds timeout)
        {
            IpAddress mcast = Protocol::multicastAddress (this->family ());
            packet.dest = IpAddress (mcast.addr (), mcast.length (), _ifindex);
            packet.port = Protocol::defaultPort;

            std::stringstream data;
            if (this->_message.serialize (packet, data) == -1)
            {
                // LCOV_EXCL_START
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
                // LCOV_EXCL_STOP
            }

            std::string buffer = data.str ();
            if (buffer.size () > Protocol::maxMsgSize)
            {
                // LCOV_EXCL_START
                lastError = make_error_code (Errc::MessageTooLong);
                return -1;
                // LCOV_EXCL_STOP
            }

            ScopedLock<Mutex> lock (_syncMutex);

            auto inserted = _pending.emplace (packet.id, std::make_unique<PendingRequest> ());
            if (!inserted.second)
            {
                // LCOV_EXCL_START
                lastError = make_error_code (Errc::OperationFailed);
                notify (onFailure, packet);
                return -1;
                // LCOV_EXCL_STOP
            }

            if (this->writeTo (buffer.data (), buffer.size (), {packet.dest, packet.port}) == -1)
            {
                // LCOV_EXCL_START
                _pending.erase (inserted.first);
                notify (onFailure, packet);
                return -1;
                // LCOV_EXCL_STOP
            }

            if (!inserted.first->second->cond.timedWait (lock, timeout))
            {
                // LCOV_EXCL_START
                _pending.erase (inserted.first);
                lastError = make_error_code (Errc::TimedOut);
                notify (onFailure, packet);
                return -1;
                // LCOV_EXCL_STOP
            }

            auto pendingReq = std::move (inserted.first->second);
            _pending.erase (inserted.first);

            if (pendingReq->ec)
            {
                // LCOV_EXCL_START
                lastError = pendingReq->ec;
                notify (onFailure, packet);
                return -1;
                // LCOV_EXCL_STOP
            }

            packet = std::move (pendingReq->packet);
            notify (onSuccess, packet);

            return 0;
        }

        /// interface index.
        unsigned int _ifindex;

        /// pending synchronous request.
        struct PendingRequest
        {
            Condition cond;     /**< condition variable to signal response reception. */
            DnsPacket packet;   /**< received response packet. */
            std::error_code ec; /**< error code from the response. */
        };

        /// synchronous requests indexed by sequence number.
        std::unordered_map<uint16_t, std::unique_ptr<PendingRequest>> _pending;

        /// protection mutex.
        Mutex _syncMutex;
    };
}

#endif
