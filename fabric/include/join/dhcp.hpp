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
#ifndef JOIN_FABRIC_DHCP_HPP
#define JOIN_FABRIC_DHCP_HPP

// libjoin.
#include <join/dhcp_protocol.hpp>
#include <join/dhcp_message.hpp>
#include <join/condition.hpp>
#include <join/reactor.hpp>
#include <join/utils.hpp>
#include <join/error.hpp>
#include <join/arp.hpp>

// C++.
#include <unordered_map>
#include <system_error>
#include <chrono>
#include <string>
#include <vector>

// C.
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <net/if.h>

namespace join
{
    /**
     * @brief carries DHCP messages over a packet socket.
     */
    template <class Protocol>
    class BasicDhcp : public EventHandler
    {
    public:
        using Socket = typename Protocol::Socket;

        /**
         * @brief create the BasicDhcp instance.
         */
        BasicDhcp () = delete;

        /**
         * @brief create the instance bound to the given interface.
         * @param interface interface name.
         * @param reactor reactor instance.
         * @throw std::system_error if the interface is unknown or the socket could not be bound to it.
         */
        explicit BasicDhcp (const std::string& interface, Reactor& reactor = ReactorThread::reactor ())
        : _buffer (std::make_unique<char[]> (sizeof (Frame) + Protocol::maxMsgSize))
        , _interface (interface)
        , _hardware (MacAddress::address (interface))
        , _reactor (reactor)
        {
            if (::if_nametoindex (_interface.c_str ()) == 0)
            {
                throw std::system_error (errno, std::system_category (), "dhcp interface lookup failed");
            }

            if (_socket.bind (_interface) == -1 || _socket.setOption (Socket::Broadcast, 1) == -1)
            {
                throw std::system_error (lastError, "dhcp socket setup failed");  // LCOV_EXCL_LINE
            }

            _reactor.addHandler (_socket.handle (), this);
        }

        /**
         * @brief create instance by copy.
         * @param other other object to copy.
         */
        BasicDhcp (const BasicDhcp& other) = delete;

        /**
         * @brief assign instance by copy.
         * @param other other object to copy.
         * @return a reference of the current object.
         */
        BasicDhcp& operator= (const BasicDhcp& other) = delete;

        /**
         * @brief create instance by move.
         * @param other other object to move.
         */
        BasicDhcp (BasicDhcp&& other) = delete;

        /**
         * @brief assign instance by move.
         * @param other other object to move.
         * @return a reference of the current object.
         */
        BasicDhcp& operator= (BasicDhcp&& other) = delete;

        /**
         * @brief destroy the instance.
         */
        virtual ~BasicDhcp ()
        {
            _reactor.delHandler (_socket.handle ());
        }

        /**
         * @brief get the name of the interface the instance is bound to.
         * @return the interface name.
         */
        const std::string& interface () const noexcept
        {
            return _interface;
        }

        /**
         * @brief get the hardware address of the interface the instance is bound to.
         * @return the hardware address.
         */
        const MacAddress& hardware () const noexcept
        {
            return _hardware;
        }

    protected:
        /**
         * @brief link, internet and transport headers a DHCP message is framed with.
         */
        struct __attribute__ ((packed)) Frame
        {
            struct ethhdr eth;
            struct iphdr ip;
            struct udphdr udp;
        };

        /**
         * @brief header the UDP checksum is computed over, RFC 768.
         */
        struct __attribute__ ((packed)) Pseudo
        {
            uint32_t source;
            uint32_t destination;
            uint8_t zero;
            uint8_t protocol;
            uint16_t length;
        };

        /**
         * @brief method called when a DHCP message is received.
         * @param packet message received.
         */
        virtual void onMessage (DhcpPacket::Ptr packet) = 0;

        /**
         * @brief method called when data are ready to be read.
         * @param fd file descriptor.
         */
        void onReadable ([[maybe_unused]] int fd) override final
        {
            ssize_t size = _socket.read (_buffer.get (), sizeof (Frame) + Protocol::maxMsgSize);
            if (size <= 0)
            {
                return;  // LCOV_EXCL_LINE
            }

            DhcpPacket::Ptr packet = receive (_buffer.get (), static_cast<size_t> (size));
            if (packet != nullptr)
            {
                onMessage (std::move (packet));
            }
        }

        /**
         * @brief compute the checksum of a UDP datagram, RFC 768.
         * @param frame frame carrying the datagram, its checksum field must be zero.
         * @param payload datagram payload.
         * @param size payload size.
         * @return the checksum.
         */
        static uint16_t udpChecksum (const Frame& frame, const char* payload, size_t size)
        {
            Pseudo pseudo = {};
            pseudo.source = frame.ip.saddr;
            pseudo.destination = frame.ip.daddr;
            pseudo.protocol = IPPROTO_UDP;
            pseudo.length = frame.udp.len;

            std::vector<uint8_t> scratch (sizeof (pseudo) + sizeof (frame.udp) + size, 0);
            ::memcpy (scratch.data (), &pseudo, sizeof (pseudo));
            ::memcpy (scratch.data () + sizeof (pseudo), &frame.udp, sizeof (frame.udp));
            ::memcpy (scratch.data () + sizeof (pseudo) + sizeof (frame.udp), payload, size);

            uint16_t sum = join::checksum (reinterpret_cast<const uint16_t*> (scratch.data ()), scratch.size ());

            return sum ? sum : 0xffff;
        }

        /**
         * @brief frame a message and write it on the wire.
         * @param packet message to send.
         * @param source source address.
         * @param destination destination address.
         * @return 0 on success, -1 on failure.
         */
        int transmit (const DhcpPacket& packet, const IpAddress& source, const IpAddress& destination)
        {
            std::stringstream data;
            if (_message.serialize (packet, data) == -1)
            {
                return -1;  // LCOV_EXCL_LINE
            }

            const std::string payload = data.str ();

            if (payload.size () > Protocol::maxMsgSize)
            {
                // LCOV_EXCL_START
                lastError = make_error_code (Errc::MessageTooLong);
                return -1;
                // LCOV_EXCL_STOP
            }

            const size_t size = sizeof (Frame) + payload.size ();

            std::vector<char> buffer (size, 0);
            Frame* frame = reinterpret_cast<Frame*> (buffer.data ());
            ::memcpy (buffer.data () + sizeof (Frame), payload.data (), payload.size ());

            const bool boot = (packet.op == DhcpMessage::BootRequest);
            const uint16_t datagram = static_cast<uint16_t> (sizeof (frame->udp) + payload.size ());

            frame->udp.source = htons (boot ? Protocol::clientPort : Protocol::serverPort);
            frame->udp.dest = htons (boot ? Protocol::serverPort : Protocol::clientPort);
            frame->udp.len = htons (datagram);
            frame->udp.check = 0;

            frame->ip.version = IPVERSION;
            frame->ip.ihl = sizeof (frame->ip) >> 2;
            frame->ip.tos = IPTOS_CLASS_CS6 | IPTOS_ECN_NOT_ECT;
            frame->ip.tot_len = htons (static_cast<uint16_t> (sizeof (frame->ip) + datagram));
            frame->ip.frag_off = htons (IP_DF);
            frame->ip.ttl = IPDEFTTL;
            frame->ip.protocol = IPPROTO_UDP;
            ::memcpy (&frame->ip.saddr, source.addr (), sizeof (frame->ip.saddr));
            ::memcpy (&frame->ip.daddr, destination.addr (), sizeof (frame->ip.daddr));
            frame->ip.check = 0;
            frame->ip.check = join::checksum (reinterpret_cast<const uint16_t*> (&frame->ip), sizeof (frame->ip));

            frame->udp.check = udpChecksum (*frame, payload.data (), payload.size ());

            ::memcpy (frame->eth.h_dest, packet.dest.addr (), ETH_ALEN);
            ::memcpy (frame->eth.h_source, packet.src.addr (), ETH_ALEN);
            frame->eth.h_proto = htons (ETH_P_IP);

            return (_socket.write (buffer.data (), size) == -1) ? -1 : 0;
        }

        /**
         * @brief decode a frame received on the wire.
         * @param data frame received.
         * @param size frame size.
         * @return the message decoded, nullptr if the frame does not carry one.
         */
        DhcpPacket::Ptr receive (const char* data, size_t size) const
        {
            if (size < sizeof (Frame) + DhcpMessage::headerSize)
            {
                return nullptr;
            }

            Frame frame;
            ::memcpy (&frame, data, sizeof (frame));

            if ((frame.eth.h_proto != htons (ETH_P_IP)) || (frame.ip.version != IPVERSION) ||
                (frame.ip.ihl != (sizeof (frame.ip) >> 2)) || (frame.ip.protocol != IPPROTO_UDP))
            {
                return nullptr;
            }

            struct iphdr header = frame.ip;
            const uint16_t check = header.check;
            header.check = 0;

            if (check != join::checksum (reinterpret_cast<const uint16_t*> (&header), sizeof (header)))
            {
                return nullptr;
            }

            if (((frame.udp.source != htons (Protocol::serverPort)) ||
                 (frame.udp.dest != htons (Protocol::clientPort))) &&
                ((frame.udp.source != htons (Protocol::clientPort)) ||
                 (frame.udp.dest != htons (Protocol::serverPort))))
            {
                return nullptr;
            }

            const size_t datagram = ntohs (frame.udp.len);
            const size_t available = size - sizeof (frame.eth) - sizeof (frame.ip);

            if ((datagram < sizeof (frame.udp)) || (datagram > available))
            {
                return nullptr;
            }

            const char* payload = data + sizeof (Frame);
            const size_t payloadSize = datagram - sizeof (frame.udp);

            if (frame.udp.check)
            {
                Frame probe = frame;
                probe.udp.check = 0;

                if (frame.udp.check != udpChecksum (probe, payload, payloadSize))
                {
                    return nullptr;
                }
            }

            std::stringstream stream;
            stream.rdbuf ()->pubsetbuf (const_cast<char*> (payload), payloadSize);

            DhcpPacket::Ptr packet = std::make_unique<DhcpPacket> ();
            if (_message.deserialize (*packet, stream) == -1)
            {
                return nullptr;
            }

            packet->src = MacAddress (frame.eth.h_source, ETH_ALEN);
            packet->dest = MacAddress (frame.eth.h_dest, ETH_ALEN);

            return packet;
        }

        /// underlying socket.
        Socket _socket;

        /// DHCP message codec.
        DhcpMessage _message;

        /// receive buffer.
        std::unique_ptr<char[]> _buffer;

        /// interface name.
        const std::string _interface;

        /// hardware address of the interface.
        const MacAddress _hardware;

        /// event loop reactor.
        Reactor& _reactor;
    };

    /**
     * @brief DHCP client.
     */
    template <class Protocol>
    class BasicDhcpClient : public BasicDhcp<Protocol>
    {
    public:
        using BasicDhcp<Protocol>::hardware;
        using BasicDhcp<Protocol>::interface;
        using BasicDhcp<Protocol>::transmit;

        /**
         * @brief create the BasicDhcpClient instance.
         */
        BasicDhcpClient () = delete;

        /**
         * @brief create the instance bound to the given interface.
         * @param interface interface name.
         * @param maxSize biggest message the server may send back, zero to leave it unspecified.
         * @param hostname host name to advertise, empty to advertise none.
         * @param reactor reactor instance.
         * @throw std::system_error if the interface is unknown, the socket could not be bound to it,
         *                          or the maximum message size is below what RFC 2132 allows.
         */
        explicit BasicDhcpClient (const std::string& interface, uint16_t maxSize = 0, const std::string& hostname = {},
                                  Reactor& reactor = ReactorThread::reactor ())
        : BasicDhcp<Protocol> (interface, reactor)
        , _hostname (hostname)
        , _maxSize (maxSize)
        {
            if (maxSize && !DhcpOption::isValid (DhcpOption::MaximumDhcpMessageSize, maxSize))
            {
                throw std::system_error (make_error_code (Errc::InvalidParam),
                                         "dhcp maximum message size is too small");
            }
        }

        /**
         * @brief destroy the instance.
         */
        virtual ~BasicDhcpClient () = default;

        /**
         * @brief build a message carrying what every message a client sends has in common.
         * @param type message type.
         * @return the message.
         */
        DhcpPacket compose (uint8_t type) const
        {
            DhcpPacket packet;

            packet.op = DhcpMessage::BootRequest;
            packet.id = randomize<uint32_t> ();
            packet.hardware = hardware ();
            packet.src = hardware ();
            packet.dest = MacAddress::broadcast;

            packet.options.insert (DhcpOption::DhcpMessageType, type);
            packet.options.insert (DhcpOption::ClientIdentifier, hardware ());

            if (!_hostname.empty ())
            {
                packet.options.insert (DhcpOption::HostName, _hostname);
            }

            if (_maxSize)
            {
                packet.options.insert (DhcpOption::MaximumDhcpMessageSize, _maxSize);
            }

            return packet;
        }

        /**
         * @brief send a message and wait for its answer.
         * @param request message to send.
         * @param destination destination address.
         * @param expected message type the answer must carry.
         * @param timeout answer timeout.
         * @return the answer received, nullptr on failure.
         */
        DhcpPacket::Ptr exchange (DhcpPacket& request, const IpAddress& destination, uint8_t expected,
                                  std::chrono::milliseconds timeout = std::chrono::seconds (1))
        {
            ScopedLock<Mutex> lock (_syncMutex);

            _reason.clear ();

            auto inserted = _pending.emplace (request.id, std::make_unique<PendingRequest> ());
            if (!inserted.second)
            {
                // LCOV_EXCL_START
                lastError = make_error_code (Errc::InUse);
                return nullptr;
                // LCOV_EXCL_STOP
            }

            PendingRequest* pending = inserted.first->second.get ();

            if (transmit (request, request.client, destination) == -1)
            {
                // LCOV_EXCL_START
                _pending.erase (request.id);
                return nullptr;
                // LCOV_EXCL_STOP
            }

            if (!pending->cond.timedWait (lock, timeout, [pending] {
                    return pending->answer != nullptr;
                }))
            {
                _pending.erase (request.id);
                lastError = make_error_code (Errc::TimedOut);
                return nullptr;
            }

            DhcpPacket::Ptr answer = std::move (pending->answer);
            _pending.erase (request.id);

            const uint8_t* type = answer->options.getIf<uint8_t> (DhcpOption::DhcpMessageType);
            if (type == nullptr)
            {
                lastError = make_error_code (Errc::MessageUnknown);
                return nullptr;
            }

            if (*type == DhcpMessage::Nak)
            {
                const std::string* message = answer->options.getIf<std::string> (DhcpOption::Message);
                if (message != nullptr)
                {
                    _reason = *message;
                }

                lastError = make_error_code (Errc::ConnectionRefused);
                return nullptr;
            }

            if (*type != expected)
            {
                lastError = make_error_code (Errc::MessageUnknown);
                return nullptr;
            }

            return answer;
        }

        /**
         * @brief get the reason the server gave for the last refusal.
         * @return the reason, empty if the server gave none or if nothing was refused.
         */
        std::string reason () const
        {
            ScopedLock<Mutex> lock (_syncMutex);

            return _reason;
        }

        /**
         * @brief broadcast a DISCOVER message and wait for an OFFER.
         * @param wants address the client would like to get, wildcard to let the server choose.
         * @param timeout answer timeout.
         * @return the offer received, nullptr on failure.
         */
        DhcpPacket::Ptr discover (const IpAddress& wants = IpAddress::ipv4Wildcard,
                                  std::chrono::milliseconds timeout = std::chrono::seconds (1))
        {
            DhcpPacket out = compose (DhcpMessage::Discover);

            out.options.insert (DhcpOption::ParameterRequestList, _defaultParams);

            if (!wants.isWildcard ())
            {
                out.options.insert (DhcpOption::RequestedIpAddress, wants);
            }

            return exchange (out, IpAddress::ipv4Broadcast, DhcpMessage::Offer, timeout);
        }

        /**
         * @brief broadcast a REQUEST message and wait for an ACK.
         * @param wants address the client asks to be assigned.
         * @param server address of the server the offer came from.
         * @param timeout answer timeout.
         * @return the acknowledgement received, nullptr on failure.
         */
        DhcpPacket::Ptr request (const IpAddress& wants, const IpAddress& server,
                                 std::chrono::milliseconds timeout = std::chrono::seconds (1))
        {
            DhcpPacket out = compose (DhcpMessage::Request);

            out.options.insert (DhcpOption::ParameterRequestList, _defaultParams);
            out.options.insert (DhcpOption::RequestedIpAddress, wants);
            out.options.insert (DhcpOption::ServerIdentifier, server);

            return exchange (out, IpAddress::ipv4Broadcast, DhcpMessage::Ack, timeout);
        }

        /**
         * @brief send a REQUEST message to the server holding the lease and wait for an ACK.
         * @param client address the client currently owns.
         * @param server address of the server holding the lease.
         * @param timeout answer timeout.
         * @return the acknowledgement received, nullptr on failure.
         */
        DhcpPacket::Ptr renew (const IpAddress& client, const IpAddress& server,
                               std::chrono::milliseconds timeout = std::chrono::seconds (1))
        {
            MacAddress mac = Arp::get (interface (), server, timeout);
            if (mac.isWildcard ())
            {
                return nullptr;
            }

            DhcpPacket out = compose (DhcpMessage::Request);

            out.dest = mac;
            out.client = client;
            out.options.insert (DhcpOption::ParameterRequestList, _defaultParams);

            return exchange (out, server, DhcpMessage::Ack, timeout);
        }

        /**
         * @brief ask a server for the parameters of an externally configured address.
         * @param client address the client already owns.
         * @param timeout answer timeout.
         * @return the acknowledgement received, nullptr on failure.
         */
        DhcpPacket::Ptr inform (const IpAddress& client, std::chrono::milliseconds timeout = std::chrono::seconds (1))
        {
            DhcpPacket out = compose (DhcpMessage::Inform);

            out.client = client;
            out.options.insert (DhcpOption::ParameterRequestList, _defaultParams);

            return exchange (out, IpAddress::ipv4Broadcast, DhcpMessage::Ack, timeout);
        }

        /**
         * @brief give a lease back to the server holding it.
         * @param client address the client currently owns.
         * @param server address of the server holding the lease.
         * @param timeout time allowed to resolve the server hardware address.
         * @return 0 on success, -1 on failure.
         */
        int release (const IpAddress& client, const IpAddress& server,
                     std::chrono::milliseconds timeout = std::chrono::seconds (1))
        {
            MacAddress mac = Arp::get (interface (), server, timeout);
            if (mac.isWildcard ())
            {
                return -1;
            }

            DhcpPacket out = compose (DhcpMessage::Release);

            out.dest = mac;
            out.client = client;
            out.options.insert (DhcpOption::ServerIdentifier, server);

            return transmit (out, client, server);
        }

        /**
         * @brief tell the server that the address it offered is already in use.
         * @param address address the client refuses.
         * @param server address of the server that offered it.
         * @param message human readable reason, empty to give none.
         * @return 0 on success, -1 on failure.
         */
        int decline (const IpAddress& address, const IpAddress& server, const std::string& message = {})
        {
            DhcpPacket out = compose (DhcpMessage::Decline);

            out.options.insert (DhcpOption::RequestedIpAddress, address);
            out.options.insert (DhcpOption::ServerIdentifier, server);

            if (!message.empty ())
            {
                out.options.insert (DhcpOption::Message, message);
            }

            return transmit (out, IpAddress::ipv4Wildcard, IpAddress::ipv4Broadcast);
        }

    protected:
        /**
         * @brief hand a received message to the request waiting for it.
         * @param packet message received.
         */
        void onMessage (DhcpPacket::Ptr packet) override final
        {
            if (packet->op != DhcpMessage::BootReply)
            {
                return;
            }

            ScopedLock<Mutex> lock (_syncMutex);

            auto it = _pending.find (packet->id);
            if (it != _pending.end ())
            {
                it->second->answer = std::move (packet);
                it->second->cond.signal ();
            }
        }

        /**
         * @brief message waiting for its answer.
         */
        struct PendingRequest
        {
            /// answer notification.
            Condition cond;

            /// answer received.
            DhcpPacket::Ptr answer;
        };

        /// options a client asks for by default.
        static const ByteList _defaultParams;

        /// messages waiting for their answer, indexed by transaction identifier.
        std::unordered_map<uint32_t, std::unique_ptr<PendingRequest>> _pending;

        /// mutex for synchronous operations.
        mutable Mutex _syncMutex;

        /// reason the server gave for the last refusal.
        std::string _reason;

        /// host name to advertise.
        const std::string _hostname;

        /// biggest message the server may send back.
        const uint16_t _maxSize;
    };

    template <class Protocol>
    const ByteList BasicDhcpClient<Protocol>::_defaultParams = {DhcpOption::SubnetMask,       DhcpOption::Router,
                                                                DhcpOption::DomainNameServer, DhcpOption::DomainName,
                                                                DhcpOption::BroadcastAddress, DhcpOption::InterfaceMtu};

    /**
     * @brief DHCP server.
     */
    template <class Protocol>
    class BasicDhcpServer : public BasicDhcp<Protocol>
    {
    public:
        using BasicDhcp<Protocol>::hardware;
        using BasicDhcp<Protocol>::interface;
        using BasicDhcp<Protocol>::transmit;

        /**
         * @brief create the BasicDhcpServer instance.
         */
        BasicDhcpServer () = delete;

        /**
         * @brief create the instance bound to the given interface.
         * @param interface interface name.
         * @param reactor reactor instance.
         * @throw std::system_error if the interface is unknown or the socket could not be bound to it.
         */
        explicit BasicDhcpServer (const std::string& interface, Reactor& reactor = ReactorThread::reactor ())
        : BasicDhcp<Protocol> (interface, reactor)
        {
        }

        /**
         * @brief destroy the instance.
         */
        virtual ~BasicDhcpServer () = default;

        /**
         * @brief answer a DISCOVER message with an OFFER.
         * @param request message being answered.
         * @param address address offered to the client.
         * @param options options to advertise, the message type and the server identifier are added.
         * @return 0 on success, -1 on failure.
         */
        int offer (const DhcpPacket& request, const IpAddress& address, const DhcpOption& options = {})
        {
            return reply (request, DhcpMessage::Offer, address, options);
        }

        /**
         * @brief answer a REQUEST message with an ACK.
         * @param request message being answered.
         * @param address address assigned to the client.
         * @param options options to advertise, the message type and the server identifier are added.
         * @return 0 on success, -1 on failure.
         */
        int ack (const DhcpPacket& request, const IpAddress& address, const DhcpOption& options = {})
        {
            return reply (request, DhcpMessage::Ack, address, options);
        }

        /**
         * @brief refuse a REQUEST message with a NAK.
         * @param request message being refused.
         * @param message human readable reason, empty to give none.
         * @return 0 on success, -1 on failure.
         */
        int nak (const DhcpPacket& request, const std::string& message = {})
        {
            DhcpOption options;

            if (!message.empty ())
            {
                options.insert (DhcpOption::Message, message);
            }

            return reply (request, DhcpMessage::Nak, IpAddress::ipv4Wildcard, options);
        }

    protected:
        /**
         * @brief method called when a DISCOVER message is received.
         * @param request message received.
         */
        virtual void onDiscover (const DhcpPacket& request) = 0;

        /**
         * @brief method called when a REQUEST message is received.
         * @param request message received.
         */
        virtual void onRequest (const DhcpPacket& request) = 0;

        /**
         * @brief method called when a RELEASE message is received.
         * @param request message received.
         */
        virtual void onRelease (const DhcpPacket& request) = 0;

        /**
         * @brief method called when a DECLINE message is received.
         * @param request message received.
         */
        virtual void onDecline (const DhcpPacket& request) = 0;

        /**
         * @brief method called when an INFORM message is received.
         * @param request message received.
         */
        virtual void onInform (const DhcpPacket& request) = 0;

        /**
         * @brief dispatch a received message to the handler for its type.
         * @param packet message received.
         */
        void onMessage (DhcpPacket::Ptr packet) override final
        {
            if (packet->op != DhcpMessage::BootRequest)
            {
                return;
            }

            const uint8_t* type = packet->options.getIf<uint8_t> (DhcpOption::DhcpMessageType);
            if (type == nullptr)
            {
                return;
            }

            switch (*type)
            {
                case DhcpMessage::Discover:
                    onDiscover (*packet);
                    break;

                case DhcpMessage::Request:
                    onRequest (*packet);
                    break;

                case DhcpMessage::Release:
                    onRelease (*packet);
                    break;

                case DhcpMessage::Decline:
                    onDecline (*packet);
                    break;

                case DhcpMessage::Inform:
                    onInform (*packet);
                    break;

                default:
                    break;
            }
        }

        /**
         * @brief answer a message received from a client.
         * @param request message being answered.
         * @param type message type of the answer.
         * @param address address assigned to the client.
         * @param options options to advertise.
         * @return 0 on success, -1 on failure.
         */
        int reply (const DhcpPacket& request, uint8_t type, const IpAddress& address, const DhcpOption& options)
        {
            const IpAddress server = IpAddress::ipv4Address (interface ());
            if (server.isWildcard ())
            {
                lastError = std::make_error_code (std::errc::address_not_available);
                return -1;
            }

            DhcpPacket out;
            out.op = DhcpMessage::BootReply;
            out.id = request.id;
            out.flags = request.flags;
            out.hardware = request.hardware;

            if (type == DhcpMessage::Ack)
            {
                out.client = request.client;
            }
            out.your = address;
            out.server = server;
            out.gateway = request.gateway;
            out.src = hardware ();

            out.options.insert (DhcpOption::DhcpMessageType, type);
            out.options.insert (DhcpOption::ServerIdentifier, server);
            out.options.insert (options.begin (), options.end ());

            const bool broadcast =
                (type == DhcpMessage::Nak) || (request.client.isWildcard () &&
                                               ((request.flags & DhcpMessage::BroadcastFlag) || address.isWildcard ()));

            out.dest = broadcast ? MacAddress::broadcast : request.hardware;

            const IpAddress& unicast = request.client.isWildcard () ? address : request.client;

            return transmit (out, server, broadcast ? IpAddress::ipv4Broadcast : unicast);
        }
    };
}

#endif
