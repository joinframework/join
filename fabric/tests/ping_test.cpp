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
#include <join/thread.hpp>
#include <join/error.hpp>
#include <join/ping.hpp>

// Libraries.
#include <gtest/gtest.h>

// C.
#include <netinet/icmp6.h>
#include <linux/icmp.h>

using join::lastError;
using join::Thread;
using join::IpAddress;
using join::Icmp;
using join::Errc;
using join::PingStats;
using join::Ping;

/**
 * @brief Class used to test the ICMP echo client API.
 */
class PingTest : public ::testing::Test
{
public:
    /**
     * @brief set up test case.
     */
    static void SetUpTestCase ()
    {
        [[maybe_unused]] int result;

        result = std::system ("ip netns add hop1");
        result = std::system ("ip netns add hop2");

        result = std::system ("ip link add veth0 type veth peer name eth0 netns hop1");
        result = std::system ("ip addr add 10.10.0.1/24 dev veth0");
        result = std::system ("ip link set veth0 up");
        result = std::system ("ip -n hop1 addr add 10.10.0.2/24 dev eth0");
        result = std::system ("ip -n hop1 link set eth0 up");

        result = std::system ("ip -n hop1 link add veth1 type veth peer name eth1 netns hop2");
        result = std::system ("ip -n hop1 addr add 10.10.1.1/24 dev veth1");
        result = std::system ("ip -n hop1 link set veth1 up");
        result = std::system ("ip -n hop2 addr add 10.10.1.2/24 dev eth1");
        result = std::system ("ip -n hop2 link set eth1 up");

        result = std::system ("ip -n hop1 link set veth1 mtu 1280");
        result = std::system ("ip -n hop2 link set eth1 mtu 1280");

        result = std::system ("ip -6 addr add fd00:10::1/64 nodad dev veth0");
        result = std::system ("ip -n hop1 -6 addr add fd00:10::2/64 nodad dev eth0");
        result = std::system ("ip -n hop1 -6 addr add fd00:11::1/64 nodad dev veth1");
        result = std::system ("ip -n hop2 -6 addr add fd00:11::2/64 nodad dev eth1");

        result = std::system ("ip netns exec hop1 sysctl -q -w net.ipv4.ip_forward=1");
        result = std::system ("ip netns exec hop1 sysctl -q -w net.ipv6.conf.all.forwarding=1");
        result = std::system ("ip route add 10.10.1.0/24 via 10.10.0.2");
        result = std::system ("ip -6 route add fd00:11::/64 via fd00:10::2");
        result = std::system ("ip -n hop2 route add 10.10.0.0/24 via 10.10.1.1");
        result = std::system ("ip -n hop2 -6 route add fd00:10::/64 via fd00:11::1");
    }

    /**
     * @brief tear down test case.
     */
    static void TearDownTestCase ()
    {
        [[maybe_unused]] int result;

        result = std::system ("ip -6 route del fd00:11::/64 via fd00:10::2");
        result = std::system ("ip route del 10.10.1.0/24 via 10.10.0.2");
        result = std::system ("ip link set veth0 down");
        result = std::system ("ip link del veth0");
        result = std::system ("ip netns del hop1");
        result = std::system ("ip netns del hop2");
    }

protected:
    /**
     * @brief make the far host ignore echo requests.
     * @param ignore set to discard the echo requests, clear to answer them.
     */
    static void ignoreEcho (int ignore)
    {
        [[maybe_unused]] int result = std::system (
            ("ip netns exec hop2 sysctl -q -w net.ipv4.icmp_echo_ignore_all=" + std::to_string (ignore)).c_str ());
    }

    /**
     * @brief set up test.
     */
    void SetUp () override
    {
        Icmp::Socket socket;

        if (socket.open (Icmp::v4 ()) == -1)
        {
            GTEST_SKIP () << "ICMP socket unavailable, CAP_NET_RAW is required: " << lastError.message ();
        }

        socket.close ();
    }

    static const std::string _host4;

    static const std::string _host6;

    static const std::string _far;

    static const std::string _far6;

    static const std::string _badSource;

    static const std::string _badDevice;

    static const int _badTtl;

    static const int _farMtu;

    static const int _farSize;

    static const int _headerSize;


    static const int _maxPacketSize;

    static const int _size;

    static const int _count;

    static const int _concurrentCount;

    static const std::chrono::milliseconds _timeout;

    static const std::chrono::milliseconds _interval;

    static const std::chrono::milliseconds _concurrentTimeout;

    static const std::chrono::milliseconds _lostTimeout;
};

const std::string PingTest::_host4 = "127.0.0.1";
const std::string PingTest::_host6 = "::1";
const std::string PingTest::_far = "10.10.1.2";
const std::string PingTest::_far6 = "fd00:11::2";
const std::string PingTest::_badSource = "1.2.3.4";
const std::string PingTest::_badDevice = "nosuchdev";
const int PingTest::_badTtl = 256;
const int PingTest::_farMtu = 1280;
const int PingTest::_farSize = 1300;
const int PingTest::_headerSize = 28;
const int PingTest::_maxPacketSize = 65535;
const int PingTest::_size = 56;
const int PingTest::_count = 3;
const int PingTest::_concurrentCount = 20;
const std::chrono::milliseconds PingTest::_timeout = std::chrono::seconds (5);
const std::chrono::milliseconds PingTest::_interval = std::chrono::milliseconds (100);
const std::chrono::milliseconds PingTest::_concurrentTimeout = std::chrono::seconds (1);
const std::chrono::milliseconds PingTest::_lostTimeout = std::chrono::milliseconds (500);

/**
 * @brief Test ping method.
 */
TEST_F (PingTest, ping)
{
    PingStats result;

    Ping client ("lo");

    ASSERT_EQ (client.ping (IpAddress (AF_INET), _host4, _size, _count, _interval), _count) << lastError.message ();
    ASSERT_EQ (client.ping (IpAddress (AF_INET6), _host6, _size, _count, _interval), _count) << lastError.message ();

    ASSERT_EQ (client.ping (IpAddress (AF_INET6), _host4, _size, _count, _interval), 0);
    ASSERT_EQ (client.ping (IpAddress (AF_INET), _host6, _size, _count, _interval), 0);

    ASSERT_EQ (client.ping (IpAddress (AF_INET), "unknown.invalid", _size, 1, _interval), 0);
    ASSERT_EQ (lastError, Errc::NotFound);

    ASSERT_EQ (client.ping (IpAddress (AF_INET), _host4, 8, _count, _interval), _count) << lastError.message ();
    ASSERT_EQ (client.ping (IpAddress (AF_INET6), _host6, 8, _count, _interval), _count) << lastError.message ();

    Ping device (_badDevice);

    device.onFailure = [&result] (const PingStats& stats) {
        result = stats;
    };

    ASSERT_EQ (device.ping (IpAddress (AF_INET), _host4, _size, 1, _interval), 0);
    ASSERT_NE (result.error ().find ("bind to device"), std::string::npos) << result.error ();

    Ping router;

    router.onFailure = [&result] (const PingStats& stats) {
        result = stats;
    };

    ASSERT_EQ (router.ping (IpAddress (_badSource), _host4, _size, 1, _interval), 0);
    ASSERT_NE (result.error ().find ("ping: bind:"), std::string::npos) << result.error ();

    ASSERT_EQ (router.ping (IpAddress (_host4), _far, _size, 1, _interval), 0);
    ASSERT_NE (result.error ().find ("connect"), std::string::npos) << result.error ();

    Ping expired (_badTtl);

    expired.onFailure = [&result] (const PingStats& stats) {
        result = stats;
    };

    ASSERT_EQ (expired.ping (IpAddress (AF_INET), _host4, _size, _count, _interval), 0);
    ASSERT_NE (result.error ().find ("time to live"), std::string::npos) << result.error ();
    ASSERT_EQ (result.sent (), 0U);

    ignoreEcho (1);

    ASSERT_EQ (router.ping (IpAddress (AF_INET), _far, _size, 1, _interval, IP_PMTUDISC_DONT, _lostTimeout), 0);
    ASSERT_EQ (lastError, Errc::TimedOut);

    ignoreEcho (0);
}

/**
 * @brief Test ping4 method.
 */
TEST_F (PingTest, ping4)
{
    ASSERT_EQ (Ping::ping4 (_host4, _size, _count, _interval), _count) << lastError.message ();
    ASSERT_EQ (Ping::ping4 (_host6, _size, _count, _interval), 0);
}

/**
 * @brief Test ping6 method.
 */
TEST_F (PingTest, ping6)
{
    ASSERT_EQ (Ping::ping6 (_host6, _size, _count, _interval), _count) << lastError.message ();
    ASSERT_EQ (Ping::ping6 (_host4, _size, _count, _interval), 0);
}

/**
 * @brief Test pathMtu method.
 */
TEST_F (PingTest, pathMtu)
{
    Ping client ("lo");

    ASSERT_EQ (client.pathMtu (IpAddress (AF_INET), _host4, _timeout), _maxPacketSize) << lastError.message ();
    ASSERT_GT (client.pathMtu (IpAddress (AF_INET6), _host6, _timeout), _maxPacketSize) << lastError.message ();

    ASSERT_EQ (client.pathMtu (IpAddress (AF_INET6), _host4, _timeout), -1);
    ASSERT_EQ (client.pathMtu (IpAddress (AF_INET), _host6, _timeout), -1);

    Ping router;

    ASSERT_EQ (router.pathMtu (IpAddress (AF_INET), _far, _timeout), _farMtu) << lastError.message ();

    Ping expired (_badTtl);

    ASSERT_EQ (expired.pathMtu (IpAddress (AF_INET), _host4, _timeout), -1);

    ignoreEcho (1);

    ASSERT_EQ (router.pathMtu (IpAddress (AF_INET), _far, _lostTimeout), -1);

    ignoreEcho (0);
}

/**
 * @brief Test trace method.
 */
TEST_F (PingTest, trace)
{
    std::vector<IpAddress> hops;

    Ping client ("lo");

    ASSERT_EQ (client.trace (IpAddress (AF_INET), _host4, 5, 1, _size), 1) << lastError.message ();
    ASSERT_EQ (client.trace (IpAddress (AF_INET6), _host6, 5, 1, _size), 1) << lastError.message ();

    ASSERT_EQ (client.trace (IpAddress (AF_INET6), _host4, 5, 1, _size), -1);

    Ping router;

    router.onHop = [&hops] (const PingStats& stats) {
        hops.push_back (stats.from ());
    };

    ASSERT_EQ (router.trace (IpAddress (AF_INET), _far, 5, 1, _size), 2) << lastError.message ();

    ASSERT_EQ (hops.size (), 2U);
    ASSERT_EQ (hops[0], IpAddress ("10.10.0.2"));
    ASSERT_EQ (hops[1], IpAddress (_far));

    hops.clear ();

    ASSERT_EQ (router.trace (IpAddress (AF_INET6), _far6, 5, 1, _size), 2) << lastError.message ();

    ASSERT_EQ (hops.size (), 2U);
    ASSERT_EQ (hops[0], IpAddress ("fd00:10::2"));
    ASSERT_EQ (hops[1], IpAddress (_far6));

    PingStats result;

    Ping shrink;

    shrink.onStop = [&result] (const PingStats& stats) {
        result = stats;
    };

    ASSERT_EQ (shrink.trace (IpAddress (AF_INET), _far, 5, 1, _farSize, IP_PMTUDISC_DO), 2) << lastError.message ();
    ASSERT_EQ (result.size (), _farMtu - _headerSize);

    ignoreEcho (1);

    ASSERT_EQ (router.trace (IpAddress (AF_INET), _far, 3, 1, _size, IP_PMTUDISC_DONT, _lostTimeout), -1);

    ignoreEcho (0);
}

/**
 * @brief Test message method.
 */
TEST_F (PingTest, message)
{
    ASSERT_STREQ (Ping::message (AF_INET, ICMP_DEST_UNREACH, ICMP_NET_UNREACH), "destination net unreachable");
    ASSERT_STREQ (Ping::message (AF_INET, ICMP_DEST_UNREACH, ICMP_HOST_UNREACH), "destination host unreachable");
    ASSERT_STREQ (Ping::message (AF_INET, ICMP_DEST_UNREACH, ICMP_PROT_UNREACH), "destination protocol unreachable");
    ASSERT_STREQ (Ping::message (AF_INET, ICMP_DEST_UNREACH, ICMP_PORT_UNREACH), "destination port unreachable");
    ASSERT_STREQ (Ping::message (AF_INET, ICMP_DEST_UNREACH, ICMP_FRAG_NEEDED),
                  "fragmentation needed and don't fragment set");
    ASSERT_STREQ (Ping::message (AF_INET, ICMP_DEST_UNREACH, ICMP_SR_FAILED), "source route failed");
    ASSERT_STREQ (Ping::message (AF_INET, ICMP_DEST_UNREACH, ICMP_NET_UNKNOWN), "destination net unknown");
    ASSERT_STREQ (Ping::message (AF_INET, ICMP_DEST_UNREACH, ICMP_HOST_UNKNOWN), "destination host unknown");
    ASSERT_STREQ (Ping::message (AF_INET, ICMP_DEST_UNREACH, ICMP_HOST_ISOLATED), "source host isolated");
    ASSERT_STREQ (Ping::message (AF_INET, ICMP_DEST_UNREACH, ICMP_NET_ANO), "destination net prohibited");
    ASSERT_STREQ (Ping::message (AF_INET, ICMP_DEST_UNREACH, ICMP_HOST_ANO), "destination host prohibited");
    ASSERT_STREQ (Ping::message (AF_INET, ICMP_DEST_UNREACH, ICMP_NET_UNR_TOS),
                  "destination net unreachable for type of service");
    ASSERT_STREQ (Ping::message (AF_INET, ICMP_DEST_UNREACH, ICMP_HOST_UNR_TOS),
                  "destination host unreachable for type of service");
    ASSERT_STREQ (Ping::message (AF_INET, ICMP_DEST_UNREACH, ICMP_PKT_FILTERED), "packet filtered");
    ASSERT_STREQ (Ping::message (AF_INET, ICMP_DEST_UNREACH, ICMP_PREC_VIOLATION), "precedence violation");
    ASSERT_STREQ (Ping::message (AF_INET, ICMP_DEST_UNREACH, ICMP_PREC_CUTOFF), "precedence cutoff");
    ASSERT_STREQ (Ping::message (AF_INET, ICMP_DEST_UNREACH, 0xff), "destination unreachable");

    ASSERT_STREQ (Ping::message (AF_INET, ICMP_TIME_EXCEEDED, ICMP_EXC_TTL), "time to live exceeded");
    ASSERT_STREQ (Ping::message (AF_INET, ICMP_TIME_EXCEEDED, ICMP_EXC_FRAGTIME), "fragment reassembly time exceeded");
    ASSERT_STREQ (Ping::message (AF_INET, ICMP_TIME_EXCEEDED, 0xff), "time exceeded");

    ASSERT_STREQ (Ping::message (AF_INET, ICMP_PARAMETERPROB, 0), "parameter problem");
    ASSERT_STREQ (Ping::message (AF_INET, ICMP_REDIRECT, 0), "redirect");
    ASSERT_STREQ (Ping::message (AF_INET, 0xff, 0), "unknown error");

    ASSERT_STREQ (Ping::message (AF_INET6, ICMP6_DST_UNREACH, ICMP6_DST_UNREACH_NOROUTE),
                  "destination unreachable: no route");
    ASSERT_STREQ (Ping::message (AF_INET6, ICMP6_DST_UNREACH, ICMP6_DST_UNREACH_ADMIN),
                  "destination unreachable: administratively prohibited");
    ASSERT_STREQ (Ping::message (AF_INET6, ICMP6_DST_UNREACH, ICMP6_DST_UNREACH_BEYONDSCOPE),
                  "destination unreachable: beyond scope of source address");
    ASSERT_STREQ (Ping::message (AF_INET6, ICMP6_DST_UNREACH, ICMP6_DST_UNREACH_ADDR),
                  "destination unreachable: address unreachable");
    ASSERT_STREQ (Ping::message (AF_INET6, ICMP6_DST_UNREACH, ICMP6_DST_UNREACH_NOPORT),
                  "destination unreachable: port unreachable");
    ASSERT_STREQ (Ping::message (AF_INET6, ICMP6_DST_UNREACH, 0xff), "destination unreachable");

    ASSERT_STREQ (Ping::message (AF_INET6, ICMP6_PACKET_TOO_BIG, 0), "packet too big");

    ASSERT_STREQ (Ping::message (AF_INET6, ICMP6_TIME_EXCEEDED, ICMP6_TIME_EXCEED_TRANSIT), "time exceeded: hop limit");
    ASSERT_STREQ (Ping::message (AF_INET6, ICMP6_TIME_EXCEEDED, ICMP6_TIME_EXCEED_REASSEMBLY),
                  "time exceeded: defragmentation failure");
    ASSERT_STREQ (Ping::message (AF_INET6, ICMP6_TIME_EXCEEDED, 0xff), "time exceeded");

    ASSERT_STREQ (Ping::message (AF_INET6, ICMP6_PARAM_PROB, ICMP6_PARAMPROB_HEADER),
                  "parameter problem: wrong header field");
    ASSERT_STREQ (Ping::message (AF_INET6, ICMP6_PARAM_PROB, ICMP6_PARAMPROB_NEXTHEADER),
                  "parameter problem: unknown header");
    ASSERT_STREQ (Ping::message (AF_INET6, ICMP6_PARAM_PROB, ICMP6_PARAMPROB_OPTION),
                  "parameter problem: unknown option");
    ASSERT_STREQ (Ping::message (AF_INET6, ICMP6_PARAM_PROB, 0xff), "parameter problem");

    ASSERT_STREQ (Ping::message (AF_INET6, 0xff, 0), "unknown error");
}

/**
 * @brief Test callbacks.
 */
TEST_F (PingTest, callbacks)
{
    int started = 0, succeeded = 0, failed = 0, stopped = 0;

    Ping client ("lo");

    client.onStart = [&started] (const PingStats&) {
        ++started;
    };

    client.onSuccess = [&succeeded] (const PingStats&) {
        ++succeeded;
    };

    client.onFailure = [&failed] (const PingStats&) {
        ++failed;
    };

    client.onStop = [&stopped] (const PingStats&) {
        ++stopped;
    };

    ASSERT_EQ (client.ping (IpAddress (AF_INET), _host4, _size, _count, _interval), _count) << lastError.message ();

    ASSERT_EQ (started, 1);
    ASSERT_EQ (succeeded, _count);
    ASSERT_EQ (failed, 0);
    ASSERT_EQ (stopped, 1);
}

/**
 * @brief Test concurrent use of a single instance.
 */
TEST_F (PingTest, concurrent)
{
    Ping client ("lo");

    int first = 0, second = 0;

    Thread v4 ([&client, &first] () {
        first = client.ping (IpAddress (AF_INET), _host4, _size, _concurrentCount, std::chrono::milliseconds (0),
                             IP_PMTUDISC_DONT, _concurrentTimeout);
    });

    Thread v6 ([&client, &second] () {
        second = client.ping (IpAddress (AF_INET6), _host6, _size, _concurrentCount, std::chrono::milliseconds (0),
                              IP_PMTUDISC_DONT, _concurrentTimeout);
    });

    v4.join ();
    v6.join ();

    ASSERT_EQ (first, _concurrentCount) << lastError.message ();
    ASSERT_EQ (second, _concurrentCount) << lastError.message ();
}

/**
 * @brief Test statistics.
 */
TEST_F (PingTest, stats)
{
    PingStats result;

    Ping client ("lo");

    client.onStop = [&result] (const PingStats& stats) {
        result = stats;
    };

    ASSERT_EQ (client.ping (IpAddress (AF_INET), _host4, _size, _count, _interval), _count) << lastError.message ();

    ASSERT_EQ (result.device (), "lo");
    ASSERT_EQ (result.host (), _host4);
    ASSERT_EQ (result.address (), IpAddress (_host4));
    ASSERT_EQ (result.from (), IpAddress (_host4));
    ASSERT_EQ (result.size (), _size);
    ASSERT_EQ (result.interval (), _interval);
    ASSERT_EQ (result.sent (), static_cast<uint32_t> (_count));
    ASSERT_EQ (result.received (), static_cast<uint32_t> (_count));
    ASSERT_DOUBLE_EQ (result.loss (), 0.0);
    ASSERT_FALSE (result.expired ());
    ASSERT_TRUE (result.error ().empty ());
    ASSERT_FALSE (result.code ());

    ASSERT_LE (result.min (), result.avg ());
    ASSERT_LE (result.avg (), result.max ());
    ASSERT_GE (result.total (), result.max ());

    result.reset ();

    ASSERT_EQ (result.sent (), 0U);
    ASSERT_EQ (result.received (), 0U);
    ASSERT_EQ (result.min ().count (), 0);
    ASSERT_EQ (result.max ().count (), 0);
    ASSERT_EQ (result.avg ().count (), 0);
    ASSERT_EQ (result.mdev ().count (), 0);
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
