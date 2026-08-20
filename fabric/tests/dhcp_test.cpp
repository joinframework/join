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
// libjoin.
#include <join/condition.hpp>
#include <join/error.hpp>
#include <join/dhcp.hpp>

// Libraries.
#include <gtest/gtest.h>

// C++.
#include <system_error>

using join::lastError;
using join::Errc;
using join::IpAddress;
using join::MacAddress;
using join::IpList;
using join::Mutex;
using join::ScopedLock;
using join::Condition;
using join::Raw;
using join::DhcpOption;
using join::DhcpPacket;
using join::DhcpMessage;
using join::Dhcp;

/**
 * @brief Class used to test the DHCP API.
 */
class DhcpTest : public ::testing::Test, public Dhcp::Server
{
public:
    /**
     * @brief set up test case.
     */
    static void SetUpTestCase ()
    {
        [[maybe_unused]] int result;

        result = std::system (("ip link add " + _device + " type dummy").c_str ());
        result = std::system (("ip link set " + _device + " address " + _mac).c_str ());
        result = std::system (("ip addr add " + _server + "/24 brd 192.168.24.255 dev " + _device).c_str ());
        result = std::system (("ip link set " + _device + " up arp on multicast on").c_str ());

        result = std::system (("ip link add " + _bare + " type dummy").c_str ());
        result = std::system (("ip link set " + _bare + " up").c_str ());
    }

    /**
     * @brief tear down test case.
     */
    static void TearDownTestCase ()
    {
        [[maybe_unused]] int result;

        result = std::system (("ip link set dev " + _device + " down").c_str ());
        result = std::system (("ip link del " + _device).c_str ());

        result = std::system (("ip link set dev " + _bare + " down").c_str ());
        result = std::system (("ip link del " + _bare).c_str ());
    }

    /**
     * @brief create the DhcpTest instance.
     */
    DhcpTest ()
    : Dhcp::Server (_device)
    {
    }

protected:
    /**
     * @brief how the server answers.
     */
    enum Behaviour
    {
        Normal,    /**< answer the way a server should. */
        Refuse,    /**< refuse every request. */
        Silent,    /**< refuse every request without giving a reason. */
        WrongType, /**< answer a discover with a type the client does not expect. */
        NoType,    /**< answer a discover without a message type at all. */
    };

    /**
     * @brief answer a discover with an offer.
     * @param request message received.
     */
    void onDiscover (const DhcpPacket& request) override
    {
        _secs = request.secs;
        received (DhcpMessage::Discover);

        if (_behaviour == WrongType)
        {
            ack (request, _lease, settings ());
            return;
        }

        if (_behaviour == NoType)
        {
            DhcpPacket out;
            out.op = DhcpMessage::BootReply;
            out.id = request.id;
            out.hardware = request.hardware;
            out.src = hardware ();
            out.dest = request.src;
            out.your = _lease;

            transmit (out, _server, _lease);
            return;
        }

        offer (request, _lease, settings ());
    }

    /**
     * @brief answer a request with an acknowledgement, or refuse it.
     * @param request message received.
     */
    void onRequest (const DhcpPacket& request) override
    {
        received (DhcpMessage::Request);

        const IpAddress* wants = request.options.getIf<IpAddress> (DhcpOption::RequestedIpAddress);
        IpAddress address = (wants != nullptr) ? *wants : request.client;

        if (_behaviour == Silent)
        {
            nak (request);
            return;
        }

        if ((_behaviour == Refuse) || (address != IpAddress (_lease)))
        {
            nak (request, "address not available");
            return;
        }

        ack (request, address, settings ());
    }

    /**
     * @brief note that a release was received.
     * @param request message received.
     */
    void onRelease ([[maybe_unused]] const DhcpPacket& request) override
    {
        received (DhcpMessage::Release);
    }

    /**
     * @brief note that a decline was received.
     * @param request message received.
     */
    void onDecline ([[maybe_unused]] const DhcpPacket& request) override
    {
        received (DhcpMessage::Decline);
    }

    /**
     * @brief answer an inform with the parameters, without a lease.
     * @param request message received.
     */
    void onInform (const DhcpPacket& request) override
    {
        received (DhcpMessage::Inform);

        DhcpOption options = settings ();
        options.erase (DhcpOption::IpAddressLeaseTime);
        options.erase (DhcpOption::RenewalTimeValue);
        options.erase (DhcpOption::RebindingTimeValue);

        ack (request, IpAddress::ipv4Wildcard, options);
    }

    /**
     * @brief get the options the server advertises.
     * @return the options.
     */
    static DhcpOption settings ()
    {
        DhcpOption options;

        options.insert (DhcpOption::IpAddressLeaseTime, 86400);
        options.insert (DhcpOption::RenewalTimeValue, 43200);
        options.insert (DhcpOption::RebindingTimeValue, 75600);
        options.insert (DhcpOption::SubnetMask, "255.255.255.0");
        options.insert (DhcpOption::BroadcastAddress, "192.168.24.255");
        options.insert (DhcpOption::Router, IpList{"192.168.24.254"});
        options.insert (DhcpOption::DomainNameServer, IpList{"192.168.24.249"});
        options.insert (DhcpOption::DomainName, "foo.local");
        options.insert (DhcpOption::InterfaceMtu, 1500);

        return options;
    }

    /**
     * @brief check that a message carries the settings the server advertises.
     * @param packet message to check.
     */
    static void checkSettings (const DhcpPacket& packet)
    {
        ASSERT_EQ (*packet.options.getIf<IpAddress> (DhcpOption::ServerIdentifier), _server);
        ASSERT_EQ (*packet.options.getIf<uint32_t> (DhcpOption::IpAddressLeaseTime), 86400u);
        ASSERT_EQ (*packet.options.getIf<uint32_t> (DhcpOption::RenewalTimeValue), 43200u);
        ASSERT_EQ (*packet.options.getIf<uint32_t> (DhcpOption::RebindingTimeValue), 75600u);
        ASSERT_EQ (*packet.options.getIf<IpAddress> (DhcpOption::SubnetMask), "255.255.255.0");
        ASSERT_EQ (*packet.options.getIf<IpAddress> (DhcpOption::BroadcastAddress), "192.168.24.255");
        ASSERT_EQ (*packet.options.getIf<IpList> (DhcpOption::Router), IpList{"192.168.24.254"});
        ASSERT_EQ (*packet.options.getIf<IpList> (DhcpOption::DomainNameServer), IpList{"192.168.24.249"});
        ASSERT_EQ (*packet.options.getIf<std::string> (DhcpOption::DomainName), "foo.local");
        ASSERT_EQ (*packet.options.getIf<uint16_t> (DhcpOption::InterfaceMtu), 1500);
    }

    /**
     * @brief build a message the way a client sends one.
     * @param type message type.
     * @return the message.
     */
    static DhcpPacket requestOf (uint8_t type)
    {
        DhcpPacket request;

        request.op = DhcpMessage::BootRequest;
        request.id = 0x11223344;
        request.hardware = _mac;
        request.src = _mac;
        request.dest = MacAddress::broadcast;
        request.options.insert (DhcpOption::DhcpMessageType, type);

        return request;
    }

    /**
     * @brief frame a message the way the wire carries it.
     * @param packet message to frame.
     * @return the frame.
     */
    static std::string frameOf (const DhcpPacket& packet)
    {
        DhcpMessage message;
        std::stringstream data;
        message.serialize (packet, data);

        const std::string payload = data.str ();
        std::string wire (sizeof (Frame) + payload.size (), '\0');
        ::memcpy (&wire[sizeof (Frame)], payload.data (), payload.size ());

        Frame* frame = reinterpret_cast<Frame*> (&wire[0]);
        const uint16_t datagram = static_cast<uint16_t> (sizeof (frame->udp) + payload.size ());

        ::memcpy (frame->eth.h_source, packet.src.addr (), ETH_ALEN);
        ::memcpy (frame->eth.h_dest, packet.dest.addr (), ETH_ALEN);
        frame->eth.h_proto = htons (ETH_P_IP);

        frame->ip.version = IPVERSION;
        frame->ip.ihl = sizeof (frame->ip) >> 2;
        frame->ip.tot_len = htons (static_cast<uint16_t> (sizeof (frame->ip) + datagram));
        frame->ip.ttl = IPDEFTTL;
        frame->ip.protocol = IPPROTO_UDP;
        ::memcpy (&frame->ip.saddr, IpAddress (_server).addr (), sizeof (frame->ip.saddr));
        ::memcpy (&frame->ip.daddr, IpAddress::ipv4Broadcast.addr (), sizeof (frame->ip.daddr));
        frame->ip.check = 0;
        frame->ip.check = Dhcp::Socket::checksum (reinterpret_cast<const uint16_t*> (&frame->ip), sizeof (frame->ip));

        const bool boot = (packet.op == DhcpMessage::BootRequest);

        frame->udp.source = htons (boot ? Dhcp::clientPort : Dhcp::serverPort);
        frame->udp.dest = htons (boot ? Dhcp::serverPort : Dhcp::clientPort);
        frame->udp.len = htons (datagram);
        frame->udp.check = 0;
        frame->udp.check = udpChecksum (*frame, payload.data (), payload.size ());

        return wire;
    }

    /**
     * @brief put raw bytes on the wire, bypassing the library.
     * @param wire bytes to write.
     */
    static void inject (const std::string& wire)
    {
        Dhcp::Socket socket;

        ASSERT_NE (socket.bind (_device), -1) << lastError.message ();
        ASSERT_NE (socket.write (wire.data (), wire.size ()), -1) << lastError.message ();
    }

    /**
     * @brief record the type of the last message the server received.
     * @param type message type.
     */
    void received (uint8_t type)
    {
        ScopedLock<Mutex> lock (_mutex);
        _last = type;
        _cond.broadcast ();
    }

    /**
     * @brief wait for the server to receive a message of the given type.
     * @param type message type.
     * @return true if it was received, false if it timed out.
     */
    bool awaits (uint8_t type)
    {
        ScopedLock<Mutex> lock (_mutex);
        return _cond.timedWait (lock, std::chrono::seconds (2), [this, type] {
            return _last == type;
        });
    }

    /// interface both peers are bound to.
    static const std::string _device;

    /// interface that owns no address.
    static const std::string _bare;

    /// hardware address of that interface.
    static const std::string _mac;

    /// address of the server.
    static const std::string _server;

    /// address the server hands out.
    static const std::string _lease;

    /// how the server answers.
    Behaviour _behaviour = Normal;

    /// elapsed time the last discover reported.
    uint16_t _secs = 0;

    /// type of the last message received.
    uint8_t _last = 0;

    /// last message notification.
    Condition _cond;

    /// last message protection mutex.
    Mutex _mutex;
};

const std::string DhcpTest::_device = "dhcp0";
const std::string DhcpTest::_bare = "dhcp1";
const std::string DhcpTest::_mac = "4e:ed:ed:ee:59:db";
const std::string DhcpTest::_server = "192.168.24.100";
const std::string DhcpTest::_lease = "192.168.24.110";

/**
 * @brief A server that answers nothing, for the cases the fixture cannot host.
 */
class Mute : public Dhcp::Server
{
public:
    using Dhcp::Server::Server;
    using Dhcp::Server::offer;

protected:
    void onDiscover ([[maybe_unused]] const DhcpPacket& request) override
    {
    }

    void onRequest ([[maybe_unused]] const DhcpPacket& request) override
    {
    }

    void onRelease ([[maybe_unused]] const DhcpPacket& request) override
    {
    }

    void onDecline ([[maybe_unused]] const DhcpPacket& request) override
    {
    }

    void onInform ([[maybe_unused]] const DhcpPacket& request) override
    {
    }
};

/**
 * @brief Test create method.
 */
TEST_F (DhcpTest, create)
{
    ASSERT_THROW (Dhcp::Client ("foo0"), std::system_error);

    ASSERT_THROW (Dhcp::Client (_device, 100), std::system_error);

    ASSERT_NO_THROW (Dhcp::Client (_device, 1500, "foobar"));
}

/**
 * @brief Test interface method.
 */
TEST_F (DhcpTest, interface)
{
    Dhcp::Client client (_device);

    ASSERT_EQ (client.interface (), _device);
    ASSERT_EQ (interface (), _device);
}

/**
 * @brief Test hardware method.
 */
TEST_F (DhcpTest, hardware)
{
    Dhcp::Client client (_device);

    ASSERT_EQ (client.hardware (), _mac);
    ASSERT_EQ (hardware (), _mac);
}

/**
 * @brief Test compose method.
 */
TEST_F (DhcpTest, compose)
{
    Dhcp::Client client (_device, 1500, "foobar");

    DhcpPacket request = client.compose (DhcpMessage::Request);

    ASSERT_EQ (request.op, DhcpMessage::BootRequest);
    ASSERT_EQ (request.hardware, _mac);
    ASSERT_EQ (request.src, _mac);
    ASSERT_EQ (request.dest, MacAddress::broadcast);
    ASSERT_EQ (request.client, IpAddress::ipv4Wildcard);

    ASSERT_EQ (request.secs, 0);

    ASSERT_EQ (*request.options.getIf<uint8_t> (DhcpOption::DhcpMessageType), DhcpMessage::Request);
    ASSERT_EQ (*request.options.getIf<MacAddress> (DhcpOption::ClientIdentifier), _mac);
    ASSERT_EQ (*request.options.getIf<std::string> (DhcpOption::HostName), "foobar");
    ASSERT_EQ (*request.options.getIf<uint16_t> (DhcpOption::MaximumDhcpMessageSize), 1500);

    Dhcp::Client bare (_device);
    DhcpPacket plain = bare.compose (DhcpMessage::Discover);

    ASSERT_FALSE (plain.options.contains (DhcpOption::HostName));
    ASSERT_FALSE (plain.options.contains (DhcpOption::MaximumDhcpMessageSize));
}

/**
 * @brief Test exchange method.
 */
TEST_F (DhcpTest, exchange)
{
    Dhcp::Client client (_device);

    DhcpPacket request = client.compose (DhcpMessage::Discover);
    request.options.insert (DhcpOption::RequestedIpAddress, _lease);

    const uint32_t id = request.id;

    DhcpPacket::Ptr answer = client.exchange (request, IpAddress::ipv4Broadcast, DhcpMessage::Offer);
    ASSERT_NE (answer, nullptr) << lastError.message ();
    ASSERT_EQ (answer->id, id);
    ASSERT_EQ (answer->your, _lease);
    ASSERT_EQ (_secs, 0);

    request.secs = 4;

    answer = client.exchange (request, IpAddress::ipv4Broadcast, DhcpMessage::Offer);
    ASSERT_NE (answer, nullptr) << lastError.message ();
    ASSERT_EQ (answer->id, id);
    ASSERT_EQ (_secs, 4);

    DhcpPacket lonely = client.compose (DhcpMessage::Discover);
    ASSERT_EQ (client.exchange (lonely, IpAddress::ipv4Broadcast, DhcpMessage::Offer, std::chrono::milliseconds (0)),
               nullptr);
    ASSERT_EQ (lastError, Errc::TimedOut) << lastError.message ();
}

/**
 * @brief Test discover method.
 */
TEST_F (DhcpTest, discover)
{
    Dhcp::Client client (_device, 1500, "foobar");

    DhcpPacket::Ptr answer = client.discover (_lease);
    ASSERT_NE (answer, nullptr) << lastError.message ();

    ASSERT_EQ (answer->op, DhcpMessage::BootReply);
    ASSERT_EQ (answer->your, _lease);
    ASSERT_EQ (answer->server, _server);
    ASSERT_EQ (answer->client, IpAddress::ipv4Wildcard);
    ASSERT_EQ (*answer->options.getIf<uint8_t> (DhcpOption::DhcpMessageType), DhcpMessage::Offer);
    checkSettings (*answer);

    answer = client.discover ();
    ASSERT_NE (answer, nullptr) << lastError.message ();
    ASSERT_EQ (answer->your, _lease);

    DhcpPacket filled = client.compose (DhcpMessage::Discover);
    filled.client = _lease;

    answer = client.exchange (filled, IpAddress::ipv4Broadcast, DhcpMessage::Offer);
    ASSERT_NE (answer, nullptr) << lastError.message ();
    ASSERT_EQ (answer->client, IpAddress::ipv4Wildcard);

    _behaviour = WrongType;
    ASSERT_EQ (client.discover (_lease), nullptr);
    ASSERT_EQ (lastError, Errc::MessageUnknown) << lastError.message ();

    _behaviour = NoType;
    ASSERT_EQ (client.discover (_lease), nullptr);
    ASSERT_EQ (lastError, Errc::MessageUnknown) << lastError.message ();

    _behaviour = Normal;
}

/**
 * @brief Test request method.
 */
TEST_F (DhcpTest, request)
{
    Dhcp::Client client (_device);

    DhcpPacket::Ptr answer = client.request (_lease, _server);
    ASSERT_NE (answer, nullptr) << lastError.message ();

    ASSERT_EQ (answer->your, _lease);
    ASSERT_EQ (*answer->options.getIf<uint8_t> (DhcpOption::DhcpMessageType), DhcpMessage::Ack);
    checkSettings (*answer);

    ASSERT_EQ (client.request ("192.168.24.111", _server), nullptr);
    ASSERT_EQ (lastError, Errc::ConnectionRefused) << lastError.message ();
}

/**
 * @brief Test reason method.
 */
TEST_F (DhcpTest, reason)
{
    Dhcp::Client client (_device);

    ASSERT_NE (client.request (_lease, _server), nullptr) << lastError.message ();
    ASSERT_TRUE (client.reason ().empty ());

    _behaviour = Refuse;

    ASSERT_EQ (client.request (_lease, _server), nullptr);
    ASSERT_EQ (client.reason (), "address not available");

    _behaviour = Silent;

    ASSERT_EQ (client.request (_lease, _server), nullptr);
    ASSERT_TRUE (client.reason ().empty ());
}

/**
 * @brief Test renew method.
 */
TEST_F (DhcpTest, renew)
{
    Dhcp::Client client (_device);

    DhcpPacket::Ptr answer = client.renew (_lease, _server);
    ASSERT_NE (answer, nullptr) << lastError.message ();

    ASSERT_EQ (answer->your, _lease);
    ASSERT_EQ (answer->client, _lease);
    ASSERT_EQ (*answer->options.getIf<uint8_t> (DhcpOption::DhcpMessageType), DhcpMessage::Ack);
    checkSettings (*answer);

    ASSERT_TRUE (awaits (DhcpMessage::Request));

    ASSERT_EQ (client.renew (_lease, "192.168.24.217", std::chrono::milliseconds (20)), nullptr);
    ASSERT_EQ (lastError, std::errc::no_such_device_or_address) << lastError.message ();
}

/**
 * @brief Test inform method.
 */
TEST_F (DhcpTest, inform)
{
    Dhcp::Client client (_device);

    DhcpPacket::Ptr answer = client.inform (_lease);
    ASSERT_NE (answer, nullptr) << lastError.message ();

    ASSERT_EQ (*answer->options.getIf<uint8_t> (DhcpOption::DhcpMessageType), DhcpMessage::Ack);
    ASSERT_EQ (*answer->options.getIf<IpAddress> (DhcpOption::SubnetMask), "255.255.255.0");
    ASSERT_EQ (*answer->options.getIf<std::string> (DhcpOption::DomainName), "foo.local");

    ASSERT_FALSE (answer->options.contains (DhcpOption::IpAddressLeaseTime));
    ASSERT_EQ (answer->your, IpAddress::ipv4Wildcard);
    ASSERT_EQ (answer->client, _lease);

    ASSERT_TRUE (awaits (DhcpMessage::Inform));
}

/**
 * @brief Test release method.
 */
TEST_F (DhcpTest, release)
{
    Dhcp::Client client (_device);

    ASSERT_EQ (client.release (_lease, _server), 0) << lastError.message ();
    ASSERT_TRUE (awaits (DhcpMessage::Release));

    ASSERT_EQ (client.release (_lease, "192.168.24.217", std::chrono::milliseconds (20)), -1);
    ASSERT_EQ (lastError, std::errc::no_such_device_or_address) << lastError.message ();
}

/**
 * @brief Test decline method.
 */
TEST_F (DhcpTest, decline)
{
    Dhcp::Client client (_device);

    ASSERT_EQ (client.decline (_lease, _server, "already in use"), 0) << lastError.message ();
    ASSERT_TRUE (awaits (DhcpMessage::Decline));

    ASSERT_EQ (client.decline (_lease, _server), 0) << lastError.message ();
}

/**
 * @brief Test offer method.
 */
TEST_F (DhcpTest, offer)
{
    Dhcp::Client client (_device);

    DhcpPacket request = requestOf (DhcpMessage::Discover);
    ASSERT_EQ (offer (request, _lease, settings ()), 0) << lastError.message ();

    request.flags = DhcpMessage::BroadcastFlag;
    ASSERT_EQ (offer (request, _lease), 0) << lastError.message ();

    request.flags = 0;
    ASSERT_EQ (offer (request, IpAddress::ipv4Wildcard), 0) << lastError.message ();

    ASSERT_NE (client.discover (_lease), nullptr) << lastError.message ();

    Mute mute (_bare);
    ASSERT_EQ (mute.offer (request, _lease), -1);
    ASSERT_EQ (lastError, std::errc::address_not_available) << lastError.message ();
}

/**
 * @brief Test ack method.
 */
TEST_F (DhcpTest, ack)
{
    Dhcp::Client client (_device);

    DhcpPacket request = requestOf (DhcpMessage::Request);
    ASSERT_EQ (ack (request, _lease, settings ()), 0) << lastError.message ();

    DhcpPacket::Ptr answer = client.request (_lease, _server);
    ASSERT_NE (answer, nullptr) << lastError.message ();
    ASSERT_EQ (*answer->options.getIf<uint8_t> (DhcpOption::DhcpMessageType), DhcpMessage::Ack);
}

/**
 * @brief Test nak method.
 */
TEST_F (DhcpTest, nak)
{
    Dhcp::Client client (_device);

    DhcpPacket request = requestOf (DhcpMessage::Request);

    ASSERT_EQ (nak (request), 0) << lastError.message ();
    ASSERT_EQ (nak (request, "address not available"), 0) << lastError.message ();

    _behaviour = Refuse;

    ASSERT_EQ (client.request (_lease, _server), nullptr);
    ASSERT_EQ (lastError, Errc::ConnectionRefused) << lastError.message ();

    _behaviour = Normal;
}

/**
 * @brief Test receive method.
 */
TEST_F (DhcpTest, receive)
{
    DhcpPacket packet = requestOf (DhcpMessage::Ack);
    packet.op = DhcpMessage::BootReply;
    packet.dest = _mac;
    packet.your = _lease;

    const std::string valid = frameOf (packet);
    ASSERT_NE (receive (valid.data (), valid.size ()), nullptr) << lastError.message ();

    ASSERT_EQ (receive (valid.data (), sizeof (Frame)), nullptr);

    std::string wire = valid;
    reinterpret_cast<Frame*> (&wire[0])->eth.h_proto = htons (ETH_P_ARP);
    ASSERT_EQ (receive (wire.data (), wire.size ()), nullptr);

    wire = valid;
    reinterpret_cast<Frame*> (&wire[0])->ip.check ^= 0xffff;
    ASSERT_EQ (receive (wire.data (), wire.size ()), nullptr);

    wire = valid;
    reinterpret_cast<Frame*> (&wire[0])->udp.dest = htons (53);
    ASSERT_EQ (receive (wire.data (), wire.size ()), nullptr);

    wire = valid;
    reinterpret_cast<Frame*> (&wire[0])->udp.len = htons (2000);
    ASSERT_EQ (receive (wire.data (), wire.size ()), nullptr);

    wire = valid;
    reinterpret_cast<Frame*> (&wire[0])->udp.check ^= 0xffff;
    ASSERT_EQ (receive (wire.data (), wire.size ()), nullptr);

    wire = valid;
    wire[sizeof (Frame) + DhcpMessage::headerSize - 1] ^= 0xff;
    reinterpret_cast<Frame*> (&wire[0])->udp.check = 0;
    ASSERT_EQ (receive (wire.data (), wire.size ()), nullptr);
}

/**
 * @brief Test onMessage method.
 */
TEST_F (DhcpTest, onMessage)
{
    Dhcp::Client client (_device), listener (_device);

    ASSERT_NE (client.discover (_lease), nullptr) << lastError.message ();

    inject (std::string (64, '\0'));

    DhcpPacket reply = requestOf (DhcpMessage::Offer);
    reply.op = DhcpMessage::BootReply;
    reply.dest = _mac;
    inject (frameOf (reply));

    DhcpPacket bare = requestOf (DhcpMessage::Discover);
    bare.options.erase (DhcpOption::DhcpMessageType);
    inject (frameOf (bare));

    inject (frameOf (requestOf (DhcpMessage::Offer)));

    ASSERT_EQ (client.release (_lease, _server), 0) << lastError.message ();
    ASSERT_TRUE (awaits (DhcpMessage::Release));
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
