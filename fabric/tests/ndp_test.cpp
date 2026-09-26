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
#include <join/ndp.hpp>

// Libraries.
#include <gtest/gtest.h>

// C++.
#include <system_error>

// C.
#include <netinet/ip6.h>
#include <net/ethernet.h>

using join::lastError;
using join::Errc;
using join::IpAddress;
using join::MacAddress;
using join::Mutex;
using join::ScopedLock;
using join::Condition;
using join::Icmp;
using join::Raw;
using join::NdpPrefix;
using join::NdpRdnss;
using join::NdpMessage;
using join::RouterSolicitation;
using join::RouterAdvertisement;
using join::Ndp;

/**
 * @brief messages received by a listener.
 */
template <class Message>
class Inbox
{
public:
    /**
     * @brief record a message.
     * @param message message received.
     */
    void push (const Message& message)
    {
        ScopedLock<Mutex> lock (_mutex);
        _messages.push_back (message);
        _cond.broadcast ();
    }

    /**
     * @brief wait for the given number of messages to be received.
     * @param count number of messages.
     * @param timeout wait timeout.
     * @return true if they were received, false if it timed out.
     */
    bool awaits (size_t count, std::chrono::milliseconds timeout = std::chrono::seconds (5))
    {
        ScopedLock<Mutex> lock (_mutex);
        return _cond.timedWait (lock, timeout, [this, count] {
            return _messages.size () >= count;
        });
    }

    /**
     * @brief get the messages received.
     * @return the messages received.
     */
    std::vector<Message> messages ()
    {
        ScopedLock<Mutex> lock (_mutex);
        return _messages;
    }

private:
    /// messages received.
    std::vector<Message> _messages;

    /// message notification.
    Condition _cond;

    /// messages protection mutex.
    Mutex _mutex;
};

/**
 * @brief Class used to test the neighbor discovery protocol API.
 */
class NdpTest : public ::testing::Test
{
public:
    /**
     * @brief set up test case.
     */
    static void SetUpTestCase ()
    {
        [[maybe_unused]] int result;

        result = std::system (("ip link add " + _device + " address " + _mac + " type dummy").c_str ());
        result = std::system (("sysctl -qw net.ipv6.conf." + _device + ".accept_dad=0").c_str ());
        result = std::system (("sysctl -qw net.ipv6.conf." + _device + ".accept_ra=0").c_str ());
        result = std::system (("ip -6 addr add " + _router + "/64 dev " + _device + " nodad").c_str ());
        result = std::system (("ip -6 addr add " + _global + "/64 dev " + _device + " nodad").c_str ());
        result = std::system (("ip link set " + _device + " mtu " + std::to_string (_mtu)).c_str ());
        result = std::system (("ip link set " + _device + " up arp on multicast on").c_str ());
    }

    /**
     * @brief tear down test case.
     */
    static void TearDownTestCase ()
    {
        [[maybe_unused]] int result;

        result = std::system (("ip link set dev " + _device + " down").c_str ());
        result = std::system (("ip link del " + _device).c_str ());
    }

    /**
     * @brief set up the test fixture, the router answers every solicitation to the solicitor.
     */
    void SetUp () override
    {
        _answer = _server.addSolicitationListener ([this] (const RouterSolicitation& solicitation) {
            _solicitations.push (solicitation);
            _server.advertise (settings (), solicitation.src);
        });
    }

    /**
     * @brief tear down the test fixture.
     */
    void TearDown () override
    {
        _server.removeSolicitationListener (_answer);
    }

protected:
    /**
     * @brief get the advertisement the router sends.
     * @return the advertisement.
     */
    static RouterAdvertisement settings ()
    {
        RouterAdvertisement advert;

        advert.hopLimit = 64;
        advert.flags = ND_RA_FLAG_OTHER;
        advert.lifetime = 1800;
        advert.mtu = 1480;

        NdpPrefix prefix;
        prefix.prefix = "2a01:e34:ed42:a680::";
        prefix.length = 64;
        prefix.valid = 86400;
        prefix.preferred = 86400;
        advert.prefixes.push_back (prefix);

        NdpRdnss rdnss;
        rdnss.lifetime = 3600;
        rdnss.servers = {"fe80::211:32ff:fe04:b5f1"};
        advert.rdnss.push_back (rdnss);

        return advert;
    }

    /**
     * @brief check that an advertisement carries what the router sends.
     * @param advert advertisement to check.
     */
    static void checkSettings (const RouterAdvertisement& advert)
    {
        ASSERT_EQ (advert.src, IpAddress (_router + "%" + _device));
        ASSERT_EQ (advert.link, MacAddress (_mac));
        ASSERT_EQ (advert.hopLimit, 64);
        ASSERT_EQ (advert.flags, ND_RA_FLAG_OTHER);
        ASSERT_EQ (advert.lifetime, 1800);
        ASSERT_EQ (advert.reachable, 0u);
        ASSERT_EQ (advert.retransmit, 0u);
        ASSERT_EQ (advert.mtu, 1480u);
        ASSERT_EQ (advert.prefixes.size (), 1u);
        ASSERT_EQ (advert.prefixes[0].prefix, IpAddress ("2a01:e34:ed42:a680::"));
        ASSERT_EQ (advert.prefixes[0].length, 64);
        ASSERT_EQ (advert.prefixes[0].flags, ND_OPT_PI_FLAG_ONLINK | ND_OPT_PI_FLAG_AUTO);
        ASSERT_EQ (advert.prefixes[0].valid, 86400u);
        ASSERT_EQ (advert.prefixes[0].preferred, 86400u);
        ASSERT_EQ (advert.rdnss.size (), 1u);
        ASSERT_EQ (advert.rdnss[0].lifetime, 3600u);
        ASSERT_EQ (advert.rdnss[0].servers, std::vector<IpAddress>{"fe80::211:32ff:fe04:b5f1"});
    }

    /**
     * @brief put raw bytes on the link, bypassing the library.
     * @param wire message to write.
     * @param destination destination address.
     * @param hop hop limit to send the message with.
     * @param source source address, wildcard to let the stack choose.
     */
    static void inject (const std::string& wire, const IpAddress& destination, int hop = Ndp::hopLimit,
                        const IpAddress& source = IpAddress::ipv6Wildcard)
    {
        Icmp::Socket socket (hop);
        unsigned int index = ::if_nametoindex (_device.c_str ());

        ASSERT_NE (socket.open (Icmp::v6 ()), -1) << lastError.message ();
        ASSERT_NE (socket.bindToDevice (_device), -1) << lastError.message ();
        ASSERT_NE (::setsockopt (socket.handle (), IPPROTO_IPV6, IPV6_MULTICAST_IF, &index, sizeof (index)), -1);
        if (!source.isWildcard ())
        {
            ASSERT_NE (socket.bind (Icmp::Endpoint (source)), -1) << lastError.message ();
        }
        Icmp::Endpoint to (IpAddress (destination.addr (), destination.length (), index));
        ASSERT_NE (socket.writeTo (wire.data (), wire.size (), to), -1) << lastError.message ();
    }

    /**
     * @brief serialize an advertisement.
     * @param advert advertisement to serialize.
     * @return the serialized advertisement.
     */
    static std::string wireOf (const RouterAdvertisement& advert)
    {
        NdpMessage message;
        std::stringstream data;
        message.serialize (advert, data);
        return data.str ();
    }

    /// interface name.
    static const std::string _device;

    /// hardware address of the interface.
    static const std::string _mac;

    /// link-local address of the interface.
    static const std::string _router;

    /// global address of the interface.
    static const std::string _global;

    /// MTU of the interface.
    static const int _mtu;

    /// timeout.
    static const std::chrono::milliseconds timeout;

    /// solicitations received by the router.
    Inbox<RouterSolicitation> _solicitations;

    /// router, destroyed before the solicitations its listener records.
    Ndp::Server _server{_device};

    /// identifier of the listener answering the solicitations.
    ssize_t _answer = 0;
};

const std::string NdpTest::_device = "ndp0";
const std::string NdpTest::_mac = "4e:ed:ed:ee:59:db";
const std::string NdpTest::_router = "fe80::4ced:edff:feee:59db";
const std::string NdpTest::_global = "2a01:e34:ed42:a680::1";
const int NdpTest::_mtu = 9000;
const std::chrono::milliseconds NdpTest::timeout = std::chrono::seconds (5);

/**
 * @brief test the construction of a client and a server.
 */
TEST_F (NdpTest, create)
{
    ASSERT_THROW (Ndp::Client ("foo0"), std::system_error);
    ASSERT_THROW (Ndp::Server ("foo0"), std::system_error);

    Ndp::Client client (_device);
    ASSERT_EQ (client.interface (), _device);
    ASSERT_EQ (client.index (), ::if_nametoindex (_device.c_str ()));
    ASSERT_EQ (client.hardware (), MacAddress (_mac));

    ASSERT_EQ (_server.interface (), _device);
    ASSERT_EQ (_server.index (), ::if_nametoindex (_device.c_str ()));
    ASSERT_EQ (_server.hardware (), MacAddress (_mac));
}

/**
 * @brief test the solicit method.
 */
TEST_F (NdpTest, solicit)
{
    Inbox<RouterAdvertisement> adverts;
    Ndp::Client client (_device);

    client.addAdvertisementListener ([&adverts] (const RouterAdvertisement& advert) {
        adverts.push (advert);
    });

    ASSERT_EQ (client.solicit (), 0) << lastError.message ();
    ASSERT_TRUE (adverts.awaits (1));
    checkSettings (adverts.messages ()[0]);

    ASSERT_TRUE (_solicitations.awaits (1));
    ASSERT_EQ (_solicitations.messages ()[0].src, IpAddress (_router + "%" + _device));
    ASSERT_EQ (_solicitations.messages ()[0].link, MacAddress (_mac));
}

/**
 * @brief test the solicit method waiting for the answer.
 */
TEST_F (NdpTest, solicitSync)
{
    Inbox<RouterAdvertisement> adverts;
    Ndp::Client client (_device);

    client.addAdvertisementListener ([&adverts] (const RouterAdvertisement& advert) {
        adverts.push (advert);
    });

    RouterAdvertisement advert;
    ASSERT_EQ (client.solicit (advert, timeout), 0) << lastError.message ();
    checkSettings (advert);
    ASSERT_EQ (adverts.messages ().size (), 1u);
    checkSettings (adverts.messages ()[0]);

    _server.removeSolicitationListener (_answer);

    RouterAdvertisement none;
    ASSERT_EQ (client.solicit (none, std::chrono::milliseconds (200)), -1);
    ASSERT_EQ (lastError, Errc::TimedOut) << lastError.message ();
    ASSERT_TRUE (none.prefixes.empty ());
    ASSERT_EQ (adverts.messages ().size (), 1u);
}

/**
 * @brief test the solicit method called from a listener.
 */
TEST_F (NdpTest, solicitFromListener)
{
    Inbox<RouterAdvertisement> adverts;
    Ndp::Client client (_device);

    int sync = 0, async = -1;
    std::error_code code;

    client.addAdvertisementListener ([&] (const RouterAdvertisement& advert) {
        if (adverts.messages ().empty ())
        {
            RouterAdvertisement unused;
            sync = client.solicit (unused);
            code = lastError;
            async = client.solicit ();
        }
        adverts.push (advert);
    });

    ASSERT_EQ (_server.advertise (settings ()), 0) << lastError.message ();
    ASSERT_TRUE (adverts.awaits (2));
    ASSERT_EQ (sync, -1);
    ASSERT_EQ (code, std::errc::resource_deadlock_would_occur) << code.message ();
    ASSERT_EQ (async, 0);
}

/**
 * @brief test the listener registration.
 */
TEST_F (NdpTest, listeners)
{
    Inbox<RouterAdvertisement> first, second;
    Ndp::Client client (_device);

    ssize_t id = client.addAdvertisementListener ([&first] (const RouterAdvertisement& advert) {
        first.push (advert);
    });

    ssize_t self = 0;
    self = client.addAdvertisementListener ([&client, &second, &self] (const RouterAdvertisement& advert) {
        second.push (advert);
        client.removeAdvertisementListener (self);
    });

    ASSERT_NE (id, -1) << lastError.message ();
    ASSERT_NE (self, -1) << lastError.message ();
    ASSERT_NE (id, self);
    ASSERT_EQ (_server.advertise (settings ()), 0) << lastError.message ();
    ASSERT_TRUE (first.awaits (1));
    ASSERT_TRUE (second.awaits (1));

    ASSERT_EQ (client.removeAdvertisementListener (id), 0) << lastError.message ();

    ASSERT_EQ (_server.advertise (settings ()), 0) << lastError.message ();
    ASSERT_FALSE (first.awaits (2, std::chrono::milliseconds (200)));
    ASSERT_FALSE (second.awaits (2, std::chrono::milliseconds (200)));
}

/**
 * @brief test the advertise method.
 */
TEST_F (NdpTest, advertise)
{
    Inbox<RouterAdvertisement> adverts;
    Ndp::Client client (_device);

    client.addAdvertisementListener ([&adverts] (const RouterAdvertisement& advert) {
        adverts.push (advert);
    });

    RouterAdvertisement bad = settings ();
    bad.prefixes[0].prefix = "192.168.24.0";
    ASSERT_EQ (_server.advertise (bad), -1);
    ASSERT_EQ (lastError, Errc::InvalidParam) << lastError.message ();

    bad.link = "4e:ed:ed:ee:59:dd";
    ASSERT_EQ (_server.advertise (bad), -1);
    ASSERT_EQ (lastError, Errc::InvalidParam) << lastError.message ();

    ASSERT_EQ (_server.advertise (settings (), "192.168.24.255"), -1);
    ASSERT_EQ (lastError, Errc::InvalidParam) << lastError.message ();

    ASSERT_EQ (_server.advertise (settings ()), 0) << lastError.message ();
    ASSERT_TRUE (adverts.awaits (1));
    checkSettings (adverts.messages ()[0]);

    RouterAdvertisement other = settings ();
    other.link = "4e:ed:ed:ee:59:dd";
    ASSERT_EQ (_server.advertise (other), 0) << lastError.message ();
    ASSERT_TRUE (adverts.awaits (2));
    ASSERT_EQ (adverts.messages ()[1].link, MacAddress ("4e:ed:ed:ee:59:dd"));
}

/**
 * @brief test messages bigger than an ethernet frame.
 */
TEST_F (NdpTest, jumbo)
{
    Inbox<RouterAdvertisement> adverts;
    Ndp::Client client (_device);

    client.addAdvertisementListener ([&adverts] (const RouterAdvertisement& advert) {
        adverts.push (advert);
    });

    RouterAdvertisement big = settings ();
    big.prefixes.assign (250, big.prefixes[0]);
    ASSERT_GT (wireOf (big).size (), size_t (_mtu / 2));
    ASSERT_EQ (_server.advertise (big), 0) << lastError.message ();
    ASSERT_TRUE (adverts.awaits (1));
    ASSERT_EQ (adverts.messages ()[0].prefixes.size (), 250u);

    RouterAdvertisement huge = settings ();
    huge.prefixes.assign (300, huge.prefixes[0]);
    ASSERT_GT (wireOf (huge).size (), size_t (_mtu));
    ASSERT_EQ (_server.advertise (huge), -1);
    ASSERT_EQ (lastError, std::errc::message_size) << lastError.message ();
}

/**
 * @brief test that the messages the checks reject are dropped.
 */
TEST_F (NdpTest, drop)
{
    Inbox<RouterAdvertisement> adverts;
    Ndp::Client client (_device);

    client.addAdvertisementListener ([&adverts] (const RouterAdvertisement& advert) {
        adverts.push (advert);
    });

    inject (wireOf (settings ()), IpAddress::ipv6AllNodes, 64);
    inject (std::string ("\x85\x00\x00\x00\x00\x00\x00\x00", 8), IpAddress::ipv6Routers, 64);

    inject (wireOf (settings ()), IpAddress::ipv6AllNodes, Ndp::hopLimit, _global);

    inject (std::string ("\x86\x00\x00\x00\x00\x00\x00\x00"
                         "\x00\x00\x00\x00\x00\x00\x00\x00"
                         "\x05\x00\x00\x00\x00\x00\x05\xdc",
                         24),
            IpAddress::ipv6AllNodes);
    inject (std::string ("\x85\x00\x00\x00\x00\x00\x00\x00\x01", 9), IpAddress::ipv6Routers);

    inject (wireOf (settings ()), IpAddress::ipv6AllNodes);
    inject (std::string ("\x85\x00\x00\x00\x00\x00\x00\x00", 8), IpAddress::ipv6Routers);

    ASSERT_TRUE (_solicitations.awaits (1));
    ASSERT_TRUE (adverts.awaits (2));
    ASSERT_FALSE (adverts.awaits (3, std::chrono::milliseconds (200)));

    ASSERT_EQ (_solicitations.messages ().size (), 1u);
    ASSERT_TRUE (_solicitations.messages ()[0].link.isWildcard ());
}

/**
 * @brief Class used to test the messages only a link peer can forge.
 */
class NdpLinkTest : public ::testing::Test
{
public:
    /**
     * @brief set up test case.
     */
    static void SetUpTestCase ()
    {
        [[maybe_unused]] int result;

        result = std::system (("ip link add " + _sender + " type veth peer name " + _receiver).c_str ());

        for (auto const& device : {_sender, _receiver})
        {
            result = std::system (("sysctl -qw net.ipv6.conf." + device + ".accept_dad=0").c_str ());
            result = std::system (("sysctl -qw net.ipv6.conf." + device + ".accept_ra=0").c_str ());
            result = std::system (("ip link set " + device + " up").c_str ());
        }
    }

    /**
     * @brief tear down test case.
     */
    static void TearDownTestCase ()
    {
        [[maybe_unused]] int result;

        result = std::system (("ip link del " + _sender).c_str ());
    }

    /**
     * @brief set up the test fixture.
     */
    void SetUp () override
    {
        _listener = _server.addSolicitationListener ([this] (const RouterSolicitation& solicitation) {
            _solicitations.push (solicitation);
        });
    }

    /**
     * @brief tear down the test fixture.
     */
    void TearDown () override
    {
        _server.removeSolicitationListener (_listener);
    }

protected:
    /**
     * @brief header the ICMPv6 checksum is computed over, RFC 8200 section 8.1.
     */
    struct __attribute__ ((packed)) Pseudo
    {
        struct in6_addr source;
        struct in6_addr destination;
        uint32_t length;
        uint8_t zero[3];
        uint8_t next;
    };

    /**
     * @brief frame a router solicitation sent from the unspecified address.
     * @param link carry the link layer address of the sender.
     * @return the frame.
     */
    static std::string solicitationOf (bool link)
    {
        const MacAddress mac = MacAddress::address (_sender);

        std::string icmp (sizeof (struct nd_router_solicit), '\0');
        icmp[0] = static_cast<char> (ND_ROUTER_SOLICIT);

        if (link)
        {
            icmp += std::string ("\x01\x01", 2);
            icmp += std::string (reinterpret_cast<const char*> (mac.addr ()), ETH_ALEN);
        }

        Pseudo pseudo = {};
        ::memcpy (&pseudo.destination, IpAddress::ipv6Routers.addr (), sizeof (pseudo.destination));
        pseudo.length = htonl (static_cast<uint32_t> (icmp.size ()));
        pseudo.next = IPPROTO_ICMPV6;

        std::string sum (reinterpret_cast<const char*> (&pseudo), sizeof (pseudo));
        sum += icmp;
        uint16_t check = join::checksum (reinterpret_cast<const uint16_t*> (sum.data ()), sum.size ());
        ::memcpy (&icmp[2], &check, sizeof (check));

        struct ip6_hdr ip = {};
        ip.ip6_flow = htonl (6 << 28);
        ip.ip6_plen = htons (static_cast<uint16_t> (icmp.size ()));
        ip.ip6_nxt = IPPROTO_ICMPV6;
        ip.ip6_hlim = Ndp::hopLimit;
        ::memcpy (&ip.ip6_dst, IpAddress::ipv6Routers.addr (), sizeof (ip.ip6_dst));

        struct ether_header eth = {};
        const uint8_t routers[ETH_ALEN] = {0x33, 0x33, 0x00, 0x00, 0x00, 0x02};
        ::memcpy (eth.ether_dhost, routers, ETH_ALEN);
        ::memcpy (eth.ether_shost, mac.addr (), ETH_ALEN);
        eth.ether_type = htons (ETHERTYPE_IPV6);

        std::string frame (reinterpret_cast<const char*> (&eth), sizeof (eth));
        frame += std::string (reinterpret_cast<const char*> (&ip), sizeof (ip));
        frame += icmp;

        return frame;
    }

    /**
     * @brief put a frame on the wire, bypassing the IPv6 stack.
     * @param frame frame to write.
     */
    static void inject (const std::string& frame)
    {
        Raw::Socket socket;

        ASSERT_NE (socket.bind (_sender), -1) << lastError.message ();
        ASSERT_NE (socket.write (frame.data (), frame.size ()), -1) << lastError.message ();
    }

    /// interface the frames are written on.
    static const std::string _sender;

    /// interface the router listens on.
    static const std::string _receiver;

    /// solicitations received by the router.
    Inbox<RouterSolicitation> _solicitations;

    /// router, destroyed before the solicitations its listener records.
    Ndp::Server _server{_receiver};

    /// identifier of the listener recording the solicitations.
    ssize_t _listener = 0;
};

const std::string NdpLinkTest::_sender = "ndpa";
const std::string NdpLinkTest::_receiver = "ndpb";

/**
 * @brief test the solicitations sent from the unspecified address.
 */
TEST_F (NdpLinkTest, unspecifiedSource)
{
    inject (solicitationOf (true));
    inject (solicitationOf (false));

    ASSERT_TRUE (_solicitations.awaits (1));
    ASSERT_FALSE (_solicitations.awaits (2, std::chrono::milliseconds (200)));

    ASSERT_TRUE (_solicitations.messages ()[0].src.isWildcard ());
    ASSERT_TRUE (_solicitations.messages ()[0].link.isWildcard ());
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
