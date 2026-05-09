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
#include <join/nameserver.hpp>

// Libraries.
#include <gtest/gtest.h>

// C++.
#include <fstream>

using join::lastError;
using join::Errc;
using join::Mdns;
using join::IpAddress;
using join::IpAddressList;
using join::AliasList;
using join::DnsPacket;
using join::ResourceRecord;
using join::DnsMessage;
using join::Mutex;
using join::Condition;
using join::ScopedLock;

using namespace std::chrono_literals;

/**
 * @brief mDNS peer acting as an announcer.
 */
class MdnsAnnouncer : public Mdns::Peer
{
public:
    /**
     * @brief construct the announcer instance.
     */
    MdnsAnnouncer ()
    : Mdns::Peer (_iface)
    {
    }

    /**
     * @brief handle a mDNS query by replying with matching local records.
     * @param query the mDNS query received.
     */
    void onQuery (const DnsPacket& query) override
    {
        std::vector<ResourceRecord> answers;

        for (auto const& question : query.questions)
        {
            if (question.host != _host && question.host != _service && question.host != _serviceType &&
                question.host != _hostIp4.toArpa () && question.host != _hostIp6.toArpa ())
            {
                continue;
            }

            for (auto const& kv : _records)
            {
                const ResourceRecord& record = kv.second;
                if (record.host == question.host &&
                    (question.type == DnsMessage::RecordType::ANY || record.type == question.type))
                {
                    answers.push_back (record);
                }
            }
        }

        if (!answers.empty ())
        {
            this->reply (query, answers);
        }
    }

    /**
     * @brief handle a mDNS announcement.
     * @param packet the mDNS announcement received.
     */
    void onAnnouncement ([[maybe_unused]] const DnsPacket& packet) override
    {
        // ignored by the announcer.
    }

    /// local resource records indexed by "host/type".
    std::map<std::string, ResourceRecord> _records;

    /// network interface to use.
    static const std::string _iface;

    /// local hostname.
    static const std::string _host;

    /// local IPv4 address.
    static const IpAddress _hostIp4;

    /// local IPv6 address.
    static const IpAddress _hostIp6;

    /// service instance name.
    static const std::string _service;

    /// service type.
    static const std::string _serviceType;
};

const std::string MdnsAnnouncer::_iface = "veth0";
const std::string MdnsAnnouncer::_host = "mytest.local";
const IpAddress MdnsAnnouncer::_hostIp4 = "192.168.10.1";
const IpAddress MdnsAnnouncer::_hostIp6 = "fd00:10::1";
const std::string MdnsAnnouncer::_service = "MyTest._foobar._tcp.local";
const std::string MdnsAnnouncer::_serviceType = "_foobar._tcp.local";

/**
 * @brief mDNS peer acting as a resolver (collects unsolicited announcements).
 */
class MdnsResolver : public Mdns::Peer
{
public:
    /**
     * @brief construct the resolver instance.
     */
    MdnsResolver ()
    : Mdns::Peer (_iface)
    {
    }

    /**
     * @brief handle a mDNS query (ignored by the resolver).
     * @param query the mDNS query received.
     */
    void onQuery ([[maybe_unused]] const DnsPacket& query) override
    {
        // ignored by the resolver.
    }

    /**
     * @brief handle a mDNS announcement by storing received records.
     * @param packet the mDNS announcement received.
     */
    void onAnnouncement (const DnsPacket& packet) override
    {
        ScopedLock<Mutex> lock (_mutex);
        for (auto const& answer : packet.answers)
        {
            if (answer.host == MdnsAnnouncer::_host || answer.host == MdnsAnnouncer::_service ||
                (answer.host == MdnsAnnouncer::_serviceType && answer.name == MdnsAnnouncer::_service))
            {
                _received.push_back (answer);
            }
        }
        _cond.signal ();
    }

    /**
     * @brief wait until a record of the given type is received or timeout expires.
     * @param type record type to wait for.
     * @param timeout maximum time to wait.
     * @return true if the record was received, false on timeout.
     */
    bool waitForRecord (uint16_t type, std::chrono::milliseconds timeout = 1000ms)
    {
        ScopedLock<Mutex> lock (_mutex);
        return _cond.timedWait (lock, timeout, [&] {
            for (auto const& r : _received)
            {
                if (r.type == type)
                {
                    return true;
                }
            }
            return false;
        });
    }

    /// network interface to use.
    static const std::string _iface;

    /// records received via onAnnouncement.
    std::vector<ResourceRecord> _received;

    /// protection mutex.
    Mutex _mutex;

    /// condition variable to signal record reception.
    Condition _cond;
};

const std::string MdnsResolver::_iface = "veth1";

/**
 * @brief mDNS test class.
 */
class MdnsTest : public ::testing::Test
{
protected:
    /**
     * @brief set up the test suite.
     */
    static void SetUpTestSuite ()
    {
        [[maybe_unused]] int result;

        result = ::system ("ip link del veth0 2>/dev/null");
        result = ::system (
            "ip link add veth0 address 06:d8:b9:37:c2:76 type veth peer name veth1 address 52:37:c5:9d:1d:b6");

        result = ::system ("sysctl -w net.ipv6.conf.veth0.accept_dad=0");
        result = ::system ("ip addr add 192.168.10.1/24 dev veth0");
        result = ::system ("ip link set veth0 multicast on");
        result = ::system ("ip link set veth0 up");

        result = ::system ("sysctl -w net.ipv6.conf.veth1.accept_dad=0");
        result = ::system ("ip addr add 192.168.10.2/24 dev veth1");
        result = ::system ("ip link set veth1 multicast on");
        result = ::system ("ip link set veth1 up");

        result = ::system ("ip neigh add 192.168.10.2 lladdr 52:37:c5:9d:1d:b6 dev veth0");
        result = ::system ("ip neigh add fe80::5037:c5ff:fe9d:1db6 lladdr 52:37:c5:9d:1d:b6 dev veth0");

        result = ::system ("ip neigh add 192.168.10.1 lladdr 06:d8:b9:37:c2:76 dev veth1");
        result = ::system ("ip neigh add fe80::4d8:b9ff:fe37:c276 lladdr 06:d8:b9:37:c2:76 dev veth1");

        result = ::system ("sysctl -w net.ipv6.conf.veth1.accept_ra=0");
        result = ::system ("sysctl -w net.ipv4.conf.veth0.rp_filter=0");
        result = ::system ("sysctl -w net.ipv4.conf.veth1.rp_filter=0");

        result = ::system ("modprobe ip_mr 2>/dev/null");
        result = ::system ("echo 1 > /proc/sys/net/ipv4/conf/veth0/mc_forwarding 2>/dev/null");
        result = ::system ("echo 1 > /proc/sys/net/ipv4/conf/veth1/mc_forwarding 2>/dev/null");
        result = ::system ("ip route add 224.0.0.0/4 dev veth0");
        result = ::system ("ip route add 224.0.0.0/4 dev veth1");

        // check IPv4 multicast availability after full setup.
        {
            MdnsAnnouncer peer;
            if (peer.bind (AF_INET) == 0)
            {
                std::ifstream igmp ("/proc/net/igmp");
                std::string line;
                while (std::getline (igmp, line))
                {
                    if (line.find ("FB0000E0") != std::string::npos)
                    {
                        _ipv4MulticastAvailable = true;
                        break;
                    }
                }
                peer.close ();
            }
        }
    }

    /**
     * @brief tear down the test suite.
     */
    static void TearDownTestSuite ()
    {
        [[maybe_unused]] int result;

        result = ::system ("ip link del veth0 2>/dev/null");
    }

    /**
     * @brief set up the test fixture.
     */
    void SetUp () override
    {
        // IPv6 records
        _resolver6._received.clear ();

        ResourceRecord aaaa;
        aaaa.host = MdnsAnnouncer::_host;
        aaaa.type = DnsMessage::RecordType::AAAA;
        aaaa.dnsclass = DnsMessage::RecordClass::IN;
        aaaa.ttl = 120;
        aaaa.addr = MdnsAnnouncer::_hostIp6;
        _announcer6._records[aaaa.host + "/AAAA"] = aaaa;

        ResourceRecord ptr_arpa6;
        ptr_arpa6.host = MdnsAnnouncer::_hostIp6.toArpa ();
        ptr_arpa6.type = DnsMessage::RecordType::PTR;
        ptr_arpa6.dnsclass = DnsMessage::RecordClass::IN;
        ptr_arpa6.ttl = 120;
        ptr_arpa6.name = MdnsAnnouncer::_host;
        _announcer6._records[ptr_arpa6.host + "/PTR"] = ptr_arpa6;

        ResourceRecord ptr6;
        ptr6.host = MdnsAnnouncer::_serviceType;
        ptr6.type = DnsMessage::RecordType::PTR;
        ptr6.dnsclass = DnsMessage::RecordClass::IN;
        ptr6.ttl = 120;
        ptr6.name = MdnsAnnouncer::_service;
        _announcer6._records[ptr6.host + "/PTR"] = ptr6;

        ResourceRecord srv6;
        srv6.host = MdnsAnnouncer::_service;
        srv6.type = DnsMessage::RecordType::SRV;
        srv6.dnsclass = DnsMessage::RecordClass::IN;
        srv6.ttl = 120;
        srv6.priority = 0;
        srv6.weight = 0;
        srv6.port = 80;
        srv6.name = MdnsAnnouncer::_host;
        _announcer6._records[srv6.host + "/SRV"] = srv6;

        ResourceRecord txt6;
        txt6.host = MdnsAnnouncer::_service;
        txt6.type = DnsMessage::RecordType::TXT;
        txt6.dnsclass = DnsMessage::RecordClass::IN;
        txt6.ttl = 120;
        txt6.txts = {"path=/", "version=1.0"};
        _announcer6._records[txt6.host + "/TXT"] = txt6;

        ASSERT_EQ (_announcer6.bind (AF_INET6), 0) << lastError.message ();
        ASSERT_EQ (_resolver6.bind (AF_INET6), 0) << lastError.message ();

        // IPv4 records
        _resolver4._received.clear ();

        ResourceRecord a;
        a.host = MdnsAnnouncer::_host;
        a.type = DnsMessage::RecordType::A;
        a.dnsclass = DnsMessage::RecordClass::IN;
        a.ttl = 120;
        a.addr = MdnsAnnouncer::_hostIp4;
        _announcer4._records[a.host + "/A"] = a;

        ResourceRecord ptr_arpa4;
        ptr_arpa4.host = MdnsAnnouncer::_hostIp4.toArpa ();
        ptr_arpa4.type = DnsMessage::RecordType::PTR;
        ptr_arpa4.dnsclass = DnsMessage::RecordClass::IN;
        ptr_arpa4.ttl = 120;
        ptr_arpa4.name = MdnsAnnouncer::_host;
        _announcer4._records[ptr_arpa4.host + "/PTR"] = ptr_arpa4;

        ResourceRecord ptr4;
        ptr4.host = MdnsAnnouncer::_serviceType;
        ptr4.type = DnsMessage::RecordType::PTR;
        ptr4.dnsclass = DnsMessage::RecordClass::IN;
        ptr4.ttl = 120;
        ptr4.name = MdnsAnnouncer::_service;
        _announcer4._records[ptr4.host + "/PTR"] = ptr4;

        ResourceRecord srv4;
        srv4.host = MdnsAnnouncer::_service;
        srv4.type = DnsMessage::RecordType::SRV;
        srv4.dnsclass = DnsMessage::RecordClass::IN;
        srv4.ttl = 120;
        srv4.priority = 0;
        srv4.weight = 0;
        srv4.port = 80;
        srv4.name = MdnsAnnouncer::_host;
        _announcer4._records[srv4.host + "/SRV"] = srv4;

        ResourceRecord txt4;
        txt4.host = MdnsAnnouncer::_service;
        txt4.type = DnsMessage::RecordType::TXT;
        txt4.dnsclass = DnsMessage::RecordClass::IN;
        txt4.ttl = 120;
        txt4.txts = {"path=/", "version=1.0"};
        _announcer4._records[txt4.host + "/TXT"] = txt4;

        ASSERT_EQ (_announcer4.bind (AF_INET), 0) << lastError.message ();
        ASSERT_EQ (_resolver4.bind (AF_INET), 0) << lastError.message ();
    }

    /**
     * @brief tear down the test fixture.
     */
    void TearDown () override
    {
        _announcer6.close ();
        _resolver6.close ();

        _announcer4.close ();
        _resolver4.close ();
    }

    /// mDNS IPv6 announcer instance.
    MdnsAnnouncer _announcer6;

    /// mDNS IPv6 resolver instance.
    MdnsResolver _resolver6;

    /// mDNS IPv4 announcer instance.
    MdnsAnnouncer _announcer4;

    /// mDNS IPv4 resolver instance.
    MdnsResolver _resolver4;

    /// whether IPv4 multicast is available on this system.
    static bool _ipv4MulticastAvailable;
};

bool MdnsTest::_ipv4MulticastAvailable = false;

/**
 * @brief test the probe method.
 */
TEST_F (MdnsTest, probe)
{
    std::vector<ResourceRecord> records6;
    for (auto const& kv : _announcer6._records)
    {
        records6.push_back (kv.second);
    }
    EXPECT_EQ (_announcer6.probe (records6), 0) << lastError.message ();

    std::vector<ResourceRecord> records4;
    for (auto const& kv : _announcer4._records)
    {
        records4.push_back (kv.second);
    }
    EXPECT_EQ (_announcer4.probe (records4), 0) << lastError.message ();

    std::vector<ResourceRecord> empty;
    EXPECT_EQ (_announcer6.probe (empty), -1);
    EXPECT_EQ (lastError, make_error_code (Errc::InvalidParam));
    EXPECT_EQ (_announcer4.probe (empty), -1);
    EXPECT_EQ (lastError, make_error_code (Errc::InvalidParam));
}

/**
 * @brief test the announce method.
 */
TEST_F (MdnsTest, announce)
{
    std::vector<ResourceRecord> records6;
    for (auto const& kv : _announcer6._records)
    {
        records6.push_back (kv.second);
    }
    EXPECT_EQ (_announcer6.announce (records6), 0) << lastError.message ();
    EXPECT_TRUE (_resolver6.waitForRecord (DnsMessage::RecordType::AAAA));

    std::vector<ResourceRecord> records4;
    for (auto const& kv : _announcer4._records)
    {
        records4.push_back (kv.second);
    }
    EXPECT_EQ (_announcer4.announce (records4), 0) << lastError.message ();
    if (_ipv4MulticastAvailable)
    {
        EXPECT_TRUE (_resolver4.waitForRecord (DnsMessage::RecordType::A));
    }

    std::vector<ResourceRecord> empty;
    EXPECT_EQ (_announcer6.announce (empty), -1);
    EXPECT_EQ (lastError, make_error_code (Errc::InvalidParam));
    EXPECT_EQ (_announcer4.announce (empty), -1);
    EXPECT_EQ (lastError, make_error_code (Errc::InvalidParam));
}

/**
 * @brief test the goodbye method.
 */
TEST_F (MdnsTest, goodbye)
{
    std::vector<ResourceRecord> records6;
    for (auto const& kv : _announcer6._records)
    {
        records6.push_back (kv.second);
    }
    EXPECT_EQ (_announcer6.goodbye (records6), 0) << lastError.message ();
    EXPECT_TRUE (_resolver6.waitForRecord (DnsMessage::RecordType::AAAA));
    for (auto const& r : _resolver6._received)
    {
        if (r.type == DnsMessage::RecordType::AAAA)
        {
            EXPECT_EQ (r.ttl, 0u);
        }
    }

    std::vector<ResourceRecord> records4;
    for (auto const& kv : _announcer4._records)
    {
        records4.push_back (kv.second);
    }
    EXPECT_EQ (_announcer4.goodbye (records4), 0) << lastError.message ();
    if (_ipv4MulticastAvailable)
    {
        EXPECT_TRUE (_resolver4.waitForRecord (DnsMessage::RecordType::A));
        for (auto const& r : _resolver4._received)
        {
            if (r.type == DnsMessage::RecordType::A)
            {
                EXPECT_EQ (r.ttl, 0u);
            }
        }
    }

    std::vector<ResourceRecord> empty;
    EXPECT_EQ (_announcer6.goodbye (empty), -1);
    EXPECT_EQ (lastError, make_error_code (Errc::InvalidParam));
    EXPECT_EQ (_announcer4.goodbye (empty), -1);
    EXPECT_EQ (lastError, make_error_code (Errc::InvalidParam));
}

/**
 * @brief test the browse method.
 */
TEST_F (MdnsTest, browse)
{
    EXPECT_EQ (_resolver6.browse (MdnsAnnouncer::_serviceType), 0) << lastError.message ();
    EXPECT_TRUE (_resolver6.waitForRecord (DnsMessage::RecordType::PTR));
    bool found6 = false;
    for (auto const& r : _resolver6._received)
    {
        if (r.type == DnsMessage::RecordType::PTR && r.name == MdnsAnnouncer::_service)
        {
            found6 = true;
            break;
        }
    }
    EXPECT_TRUE (found6);

    EXPECT_EQ (_resolver4.browse (MdnsAnnouncer::_serviceType), 0) << lastError.message ();
    if (_ipv4MulticastAvailable)
    {
        EXPECT_TRUE (_resolver4.waitForRecord (DnsMessage::RecordType::PTR));
        bool found4 = false;
        for (auto const& r : _resolver4._received)
        {
            if (r.type == DnsMessage::RecordType::PTR && r.name == MdnsAnnouncer::_service)
            {
                found4 = true;
                break;
            }
        }
        EXPECT_TRUE (found4);
    }

    EXPECT_EQ (_resolver6.browse (""), -1);
    EXPECT_EQ (lastError, make_error_code (Errc::InvalidParam));

    EXPECT_EQ (_resolver4.browse (""), -1);
    EXPECT_EQ (lastError, make_error_code (Errc::InvalidParam));
}

/**
 * @brief test the resolveAddress method.
 */
TEST_F (MdnsTest, resolveAddress)
{
    IpAddress addr = _resolver6.resolveAddress ("", AF_INET6, 500ms);
    EXPECT_TRUE (addr.isWildcard ());

    addr = _resolver6.resolveAddress (MdnsAnnouncer::_host, AF_INET6, 500ms);
    EXPECT_FALSE (addr.isWildcard ());
    EXPECT_EQ (addr, MdnsAnnouncer::_hostIp6);

    addr = _resolver6.resolveAddress ("unknown.local", AF_INET6, 500ms);
    EXPECT_TRUE (addr.isWildcard ());
    EXPECT_EQ (lastError, make_error_code (Errc::TimedOut));

    if (_ipv4MulticastAvailable)
    {
        addr = _resolver4.resolveAddress ("", AF_INET, 500ms);
        EXPECT_TRUE (addr.isWildcard ());

        addr = _resolver4.resolveAddress (MdnsAnnouncer::_host, AF_INET, 500ms);
        EXPECT_FALSE (addr.isWildcard ());
        EXPECT_EQ (addr, MdnsAnnouncer::_hostIp4);

        addr = _resolver4.resolveAddress ("unknown.local", AF_INET, 500ms);
        EXPECT_TRUE (addr.isWildcard ());
        EXPECT_EQ (lastError, make_error_code (Errc::TimedOut));
    }
}

/**
 * @brief test the resolveAllAddress method.
 */
TEST_F (MdnsTest, resolveAllAddress)
{
    IpAddressList addrs = _resolver6.resolveAllAddress ("", AF_INET6, 500ms);
    EXPECT_EQ (addrs.size (), 0);

    addrs = _resolver6.resolveAllAddress (MdnsAnnouncer::_host, AF_INET6, 500ms);
    ASSERT_GT (addrs.size (), 0);
    EXPECT_EQ (addrs.front (), MdnsAnnouncer::_hostIp6);

    addrs = _resolver6.resolveAllAddress ("unknown.local", AF_INET6, 500ms);
    EXPECT_EQ (addrs.size (), 0);

    if (_ipv4MulticastAvailable)
    {
        addrs = _resolver4.resolveAllAddress ("", AF_INET, 500ms);
        EXPECT_EQ (addrs.size (), 0);

        addrs = _resolver4.resolveAllAddress (MdnsAnnouncer::_host, AF_INET, 500ms);
        ASSERT_GT (addrs.size (), 0);
        EXPECT_EQ (addrs.front (), MdnsAnnouncer::_hostIp4);

        addrs = _resolver4.resolveAllAddress ("unknown.local", AF_INET, 500ms);
        EXPECT_EQ (addrs.size (), 0);
    }
}

/**
 * @brief test the resolveName method.
 */
TEST_F (MdnsTest, resolveName)
{
    std::string name = _resolver6.resolveName (IpAddress ("::"), 500ms);
    EXPECT_TRUE (name.empty ());

    name = _resolver6.resolveName (MdnsAnnouncer::_hostIp6, 500ms);
    EXPECT_FALSE (name.empty ());
    EXPECT_EQ (name, MdnsAnnouncer::_host);

    name = _resolver6.resolveName (IpAddress ("fd00::1"), 500ms);
    EXPECT_TRUE (name.empty ());
    EXPECT_EQ (lastError, make_error_code (Errc::TimedOut));

    if (_ipv4MulticastAvailable)
    {
        name = _resolver4.resolveName (IpAddress ("0.0.0.0"), 500ms);
        EXPECT_TRUE (name.empty ());

        name = _resolver4.resolveName (MdnsAnnouncer::_hostIp4, 500ms);
        EXPECT_FALSE (name.empty ());
        EXPECT_EQ (name, MdnsAnnouncer::_host);

        name = _resolver4.resolveName (IpAddress ("192.168.1.99"), 500ms);
        EXPECT_TRUE (name.empty ());
        EXPECT_EQ (lastError, make_error_code (Errc::TimedOut));
    }
}

/**
 * @brief test the resolveAllName method.
 */
TEST_F (MdnsTest, resolveAllName)
{
    AliasList aliases = _resolver6.resolveAllName (IpAddress ("::"), 500ms);
    EXPECT_EQ (aliases.size (), 0);

    aliases = _resolver6.resolveAllName (MdnsAnnouncer::_hostIp6, 500ms);
    ASSERT_GT (aliases.size (), 0);
    EXPECT_NE (aliases.find (MdnsAnnouncer::_host), aliases.end ());

    aliases = _resolver6.resolveAllName (IpAddress ("fd00::1"), 500ms);
    EXPECT_EQ (aliases.size (), 0);
    EXPECT_EQ (lastError, make_error_code (Errc::TimedOut));

    if (_ipv4MulticastAvailable)
    {
        aliases = _resolver4.resolveAllName (IpAddress ("0.0.0.0"), 500ms);
        EXPECT_EQ (aliases.size (), 0);

        aliases = _resolver4.resolveAllName (MdnsAnnouncer::_hostIp4, 500ms);
        ASSERT_GT (aliases.size (), 0);
        EXPECT_NE (aliases.find (MdnsAnnouncer::_host), aliases.end ());

        aliases = _resolver4.resolveAllName (IpAddress ("192.168.1.99"), 500ms);
        EXPECT_EQ (aliases.size (), 0);
        EXPECT_EQ (lastError, make_error_code (Errc::TimedOut));
    }
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
