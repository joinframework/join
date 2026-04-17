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
#include <join/condition.hpp>
#include <join/dnsbase.hpp>

// C++.
#include <unordered_map>
#include <chrono>

namespace join
{
    /**
     * @brief basic DNS name server class.
     */
    template <typename TransportPolicy>
    class BasicDnsNameServer : public BasicDns<TransportPolicy>
    {
    public:
        using Endpoint = typename BasicDns<TransportPolicy>::Endpoint;
        using RecordType = typename BasicDns<TransportPolicy>::RecordType;
        using RecordClass = typename BasicDns<TransportPolicy>::RecordClass;

        /**
         * @brief create the nameserver instance binded to the given interface.
         * @param interface interface to use.
         * @param args additional arguments (see. transport policies ::create).
         */
        template <typename... Args>
        explicit BasicDnsNameServer (const std::string& interface = "", Args&&... args)
        {
            if (this->_transport.create (this, interface, std::forward<Args> (args)...) == -1)
            {
                throw std::system_error (lastError);  // LCOV_EXCL_LINE
            }
        }

        /**
         * @brief destroy instance.
         */
        virtual ~BasicDnsNameServer () noexcept
        {
            this->_transport.close (this);
        }

        /**
         * @brief reply to a DNS query.
         * @return 0 on success, -1 on error.
         */
        // int reply (const DnsPacket& response)
        // {
        //     return -1;
        // }

        /**
         * @brief method called when a DNS query is received.
         * @param packet parsed DNS query received.
         */
        virtual void onQuery (const DnsPacket& packet) = 0;

    protected:
        /**
         * @brief serialize and send a DNS packet without waiting for a response.
         * @param packet DNS packet to send.
         * @return 0 on success, -1 on error.
         */
        int send (const DnsPacket& packet)
        {
            std::stringstream data;
            if (this->serialize (packet, data) == -1)
            {
                return -1;
            }

            std::string buffer = data.str ();
            if (buffer.size () > TransportPolicy::maxMsgSize)
            {
                return -1;
            }

            Endpoint endpoint = {packet.dest, packet.port};
            return this->_transport.write (reinterpret_cast<const uint8_t*> (buffer.data ()), buffer.size (), endpoint);
        }

        /**
         * @brief method called when data are ready to be read on handle.
         * @param fd file descriptor.
         */
        virtual void onReceive ([[maybe_unused]] int fd) override
        {
            Endpoint from{};
            int size = this->_transport.read (this->_buffer.get (), TransportPolicy::maxMsgSize, &from);
            if (size < 12)
            {
                return;
            }

            std::stringstream data;
            data.rdbuf ()->pubsetbuf (reinterpret_cast<char*> (this->_buffer.get ()), size);

            DnsPacket packet;
            this->deserialize (packet, data);
            packet.src = from.ip ();
            packet.port = from.port ();

            if ((packet.flags & 0x8000) == 0)
            {
                onQuery (packet);
            }
        }
    };

    /**
     * @brief basic DNS peer class.
     */
    template <typename TransportPolicy>
    class BasicDnsPeer : public BasicDnsNameServer<TransportPolicy>
    {
    public:
        using Endpoint = typename BasicDnsNameServer<TransportPolicy>::Endpoint;
        using RecordType = typename BasicDnsNameServer<TransportPolicy>::RecordType;
        using RecordClass = typename BasicDnsNameServer<TransportPolicy>::RecordClass;

        /**
         * @brief create the nameserver instance binded to the given interface.
         * @param interface interface to use.
         * @param args additional arguments (see. transport policies ::create).
         */
        template <typename... Args>
        explicit BasicDnsPeer (const std::string& interface = "", Args&&... args)
        : BasicDnsNameServer<TransportPolicy> (interface, std::forward<Args> (args)...)
        {
        }

        /**
         * @brief destroy instance.
         */
        ~BasicDnsPeer () noexcept = default;

        /**
         * @brief send a probe query to check if a name is available on the network.
         * @param name host name to probe.
         * @param addr IP address to probe.
         * @return 0 on success, -1 on error.
         */
        int probe (const std::string& name, const IpAddress& addr)
        {
            int family = this->_transport.family ();

            DnsPacket packet{};
            packet.id = 0;
            packet.flags = 0;
            packet.dest = TransportPolicy::multicastAddress (family);
            packet.port = TransportPolicy::defaultPort;

            QuestionRecord question;
            question.host = name;
            question.type = RecordType::ANY;
            question.dnsclass = RecordClass::IN | 0x8000;
            packet.questions.push_back (question);

            ResourceRecord record;
            record.host = name;
            record.type = (addr.family () == AF_INET6) ? RecordType::AAAA : RecordType::A;
            record.dnsclass = RecordClass::IN;
            record.ttl = 0;
            record.addr = addr;
            packet.authorities.push_back (record);

            return this->sendAsync (packet);
        }

        /**
         * @brief announce a name and address on the network.
         * @param name host name to announce.
         * @param addr IP address to announce.
         * @return 0 on success, -1 on error.
         */
        int announce (const std::string& name, const IpAddress& addr)
        {
            int family = this->_transport.family ();

            DnsPacket packet{};
            packet.id = 0;
            packet.flags = 0x8400;
            packet.dest = TransportPolicy::multicastAddress (family);
            packet.port = TransportPolicy::defaultPort;

            ResourceRecord record;
            record.host = name;
            record.type = (addr.family () == AF_INET6) ? RecordType::AAAA : RecordType::A;
            record.dnsclass = RecordClass::IN;
            record.ttl = 4500;
            record.addr = addr;
            packet.answers.push_back (record);

            return this->sendAsync (packet);
        }

        /**
         * @brief browse for services on the network.
         * @param service service type to browse (ex: "_http._tcp.local").
         * @return 0 on success, -1 on error.
         */
        int browse (const std::string& service)
        {
            int family = this->_transport.family ();

            DnsPacket packet{};
            packet.id = 0;
            packet.flags = 0;
            packet.dest = TransportPolicy::multicastAddress (family);
            packet.port = TransportPolicy::defaultPort;

            QuestionRecord question;
            question.host = service;
            question.type = RecordType::PTR;
            question.dnsclass = RecordClass::IN;
            packet.questions.push_back (question);

            return this->send (packet);
        }

        /**
         * @brief resolve a host name to a list of IP addresses.
         * @param name host name to resolve.
         * @param type record type to query (default: A).
         * @param timeout query timeout (default: 5 seconds).
         * @return list of IP addresses, empty on error.
         */
        IpAddressList resolve (const std::string& name, RecordType type = RecordType::A,
                               std::chrono::milliseconds timeout = std::chrono::seconds (5))
        {
            DnsPacket packet{};
            packet.id = 0;
            packet.flags = 0;
            packet.dest = TransportPolicy::multicastAddress (this->_transport.family ());
            packet.port = TransportPolicy::defaultPort;

            QuestionRecord question;
            question.host = name;
            question.type = type;
            question.dnsclass = RecordClass::IN;
            packet.questions.push_back (question);

            if (this->query (packet, timeout) == -1)
            {
                return {};
            }

            IpAddressList addresses;

            for (auto const& answer : packet.answers)
            {
                if (!answer.addr.isWildcard ())
                {
                    addresses.push_back (answer.addr);
                }
            }

            return addresses;
        }

        /**
         * @brief resolve an IP address to a host name.
         * @param addr IP address to resolve.
         * @param timeout query timeout (default: 5 seconds).
         * @return host name, empty string on error.
         */
        std::string reverse (const IpAddress& addr, std::chrono::milliseconds timeout = std::chrono::seconds (5))
        {
            int family = this->_transport.family ();

            DnsPacket packet{};
            packet.id = 0;
            packet.flags = 0;
            packet.dest = TransportPolicy::multicastAddress (family);
            packet.port = TransportPolicy::defaultPort;

            QuestionRecord question;
            question.host = addr.toArpa ();
            question.type = RecordType::PTR;
            question.dnsclass = RecordClass::IN;
            packet.questions.push_back (question);

            if (this->sendSync (packet, timeout) == -1)
            {
                return {};
            }

            for (auto const& answer : packet.answers)
            {
                if (!answer.name.empty ())
                {
                    return answer.name;
                }
            }

            return {};
        }

        /**
         * @brief method called when a DNS query is received.
         * @param packet parsed DNS query received.
         */
        virtual void onQuery (const DnsPacket& packet) override = 0;

        /**
         * @brief method called when a DNS annoucement is received.
         * @param packet parsed DNS annoucement received.
         * @param partial partial data received, more data are coming.
         */
        virtual void onAnnouncement (const DnsPacket& packet, bool partial) = 0;

    private:
        /**
         * @brief serialize and send a DNS query, waiting for a response.
         * @param packet DNS packet to send, filled with the response on success.
         * @param timeout query timeout.
         * @return 0 on success, -1 on error.
         */
        int query (DnsPacket& packet, std::chrono::milliseconds timeout)
        {
            std::stringstream data;
            if (this->serialize (packet, data) == -1)
            {
                return -1;
            }

            std::string buffer = data.str ();
            if (buffer.size () > TransportPolicy::maxMsgSize)
            {
                return -1;
            }

            ScopedLock<Mutex> lock (_syncMutex);

            auto inserted = _pending.emplace (packet.id, std::make_unique<PendingRequest> ());
            if (!inserted.second)
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            Endpoint endpoint = {packet.dest, packet.port};
            if (this->_transport.write (reinterpret_cast<const uint8_t*> (buffer.data ()), buffer.size (), endpoint) ==
                -1)
            {
                _pending.erase (inserted.first);
                return -1;
            }

            if (!inserted.first->second->cond.timedWait (lock, timeout))
            {
                _pending.erase (inserted.first);
                lastError = make_error_code (Errc::TimedOut);
                return -1;
            }

            auto pendingReq = std::move (inserted.first->second);
            _pending.erase (inserted.first);

            if (pendingReq->ec)
            {
                lastError = pendingReq->ec;
                return -1;
            }

            packet = std::move (pendingReq->packet);

            return 0;
        }

        /**
         * @brief method called when data are ready to be read on handle.
         * @param fd file descriptor.
         */
        void onReceive ([[maybe_unused]] int fd) override final
        {
            Endpoint from{};
            int size = this->_transport.read (this->_buffer.get (), TransportPolicy::maxMsgSize, &from);
            if (size < 12)
            {
                return;
            }

            std::stringstream data;
            data.rdbuf ()->pubsetbuf (reinterpret_cast<char*> (this->_buffer.get ()), size);

            DnsPacket packet;
            this->deserialize (packet, data);
            packet.src = from.ip ();
            packet.port = from.port ();

            if (packet.flags & 0x8000)
            {
                bool found = false;

                {
                    ScopedLock<Mutex> lock (_syncMutex);

                    auto it = _pending.find (packet.id);
                    if (it != _pending.end ())
                    {
                        found = true;
                        it->second->packet = packet;
                        it->second->ec = this->decodeError (packet.flags & 0x000F);
                        if ((packet.flags & 0x0200) && it->second->ec == std::error_code{})
                        {
                            it->second->ec = make_error_code (Errc::MessageTooLong);
                        }
                        it->second->cond.signal ();
                    }
                }

                if (!found)
                {
                    onAnnouncement (packet, packet.flags & 0x0200);
                }
            }
            else
            {
                onQuery (packet);
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
