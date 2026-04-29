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

// libjoin.
#include <join/nameserver.hpp>
#include <join/resolver.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::lastError;
using join::Dns;
using join::IpAddress;
using join::IpAddressList;
using join::AliasList;
using join::ServerList;
using join::ExchangerList;
using join::DnsPacket;
using join::ResourceRecord;
using join::DnsMessage;

using namespace std::chrono_literals;

/**
 * @brief DNS test class.
 */
class DnsTest : public ::testing::Test, public Dns::NameServer
{
protected:
    /**
     * @brief set up the test fixture.
     */
    void SetUp () override
    {
        ASSERT_EQ (this->bind (Dns::Endpoint (IpAddress::ipv4Wildcard, _dnsPort)), 0) << lastError.message ();

        _resolver = std::make_unique<Dns::Resolver> ("127.0.0.1", _dnsPort);
        ASSERT_NE (_resolver, nullptr);
    }

    /**
     * @brief tear down the test fixture.
     */
    void TearDown () override
    {
        EXPECT_EQ (_resolver->disconnect (), 0) << lastError.message ();
        this->close ();
    }

    /**
     * @brief handle a DNS query.
     * @param query the DNS query.
     */
    void onQuery (const DnsPacket& query) override
    {
        if (query.questions.empty ())
        {
            this->reply (query, {}, {}, {}, 1);
            return;
        }

        std::vector<ResourceRecord> answers;

        for (const auto& question : query.questions)
        {
            if (question.host.empty ())
            {
                this->reply (query, {}, {}, {}, 1);
                return;
            }

            switch (question.type)
            {
                case DnsMessage::RecordType::A:
                    {
                        ResourceRecord rr;
                        rr.host = question.host;
                        rr.type = DnsMessage::RecordType::A;
                        rr.dnsclass = DnsMessage::RecordClass::IN;
                        rr.ttl = 60;
                        rr.addr = _fakeIPv4;
                        answers.push_back (rr);
                        break;
                    }

                case DnsMessage::RecordType::AAAA:
                    {
                        ResourceRecord rr;
                        rr.host = question.host;
                        rr.type = DnsMessage::RecordType::AAAA;
                        rr.dnsclass = DnsMessage::RecordClass::IN;
                        rr.ttl = 60;
                        rr.addr = _fakeIPv6;
                        answers.push_back (rr);
                        break;
                    }

                case DnsMessage::RecordType::PTR:
                    {
                        ResourceRecord rr;
                        rr.host = question.host;
                        rr.type = DnsMessage::RecordType::PTR;
                        rr.dnsclass = DnsMessage::RecordClass::IN;
                        rr.ttl = 60;
                        rr.name = _fakePTR;
                        answers.push_back (rr);
                        break;
                    }

                case DnsMessage::RecordType::NS:
                    {
                        ResourceRecord rr;
                        rr.host = question.host;
                        rr.type = DnsMessage::RecordType::NS;
                        rr.dnsclass = DnsMessage::RecordClass::IN;
                        rr.ttl = 60;
                        rr.name = _fakeNS;
                        answers.push_back (rr);
                        break;
                    }

                case DnsMessage::RecordType::SOA:
                    {
                        ResourceRecord rr;
                        rr.host = question.host;
                        rr.type = DnsMessage::RecordType::SOA;
                        rr.dnsclass = DnsMessage::RecordClass::IN;
                        rr.ttl = 60;
                        rr.name = _fakeSOA;
                        rr.mail = "admin@fake.local";
                        rr.serial = 1;
                        rr.refresh = 3600;
                        rr.retry = 600;
                        rr.expire = 86400;
                        rr.minimum = 60;
                        answers.push_back (rr);
                        break;
                    }

                case DnsMessage::RecordType::MX:
                    {
                        ResourceRecord rr;
                        rr.host = question.host;
                        rr.type = DnsMessage::RecordType::MX;
                        rr.dnsclass = DnsMessage::RecordClass::IN;
                        rr.ttl = 60;
                        rr.mxpref = 10;
                        rr.name = _fakeMX;
                        answers.push_back (rr);
                        break;
                    }

                default:
                    this->reply (query, {}, {}, {}, 4);
                    return;
            }
        }

        this->reply (query, answers);
    }

    /// DNS resolver instance.
    std::unique_ptr<Dns::Resolver> _resolver;

    /// DNS server port.
    static const uint16_t _dnsPort;

    /// fake IPv4 address.
    static const IpAddress _fakeIPv4;

    /// fake IPv6 address.
    static const IpAddress _fakeIPv6;

    /// fake PTR record.
    static constexpr const char* _fakePTR = "fake.local";

    /// fake NS record.
    static constexpr const char* _fakeNS = "ns.fake.local";

    /// fake SOA record.
    static constexpr const char* _fakeSOA = "soa.fake.local";

    /// fake MX record.
    static constexpr const char* _fakeMX = "mail.fake.local";
};

const uint16_t DnsTest::_dnsPort = 5353;
const IpAddress DnsTest::_fakeIPv4 ("1.2.3.4");
const IpAddress DnsTest::_fakeIPv6 ("::1:2:3:4");

/**
 * @brief test the resolveAllAddress method.
 */
TEST_F (DnsTest, resolveAllAddress)
{
    IpAddressList addresses = _resolver->resolveAllAddress ("", AF_INET);
    EXPECT_EQ (addresses.size (), 0);

    addresses = Dns::Resolver::lookupAllAddress ("", AF_INET6);
    EXPECT_EQ (addresses.size (), 0);

    addresses = _resolver->resolveAllAddress ("");
    EXPECT_EQ (addresses.size (), 0);

    addresses = Dns::Resolver::lookupAllAddress ("");
    EXPECT_EQ (addresses.size (), 0);

    addresses = _resolver->resolveAllAddress ("localhost", AF_INET);
    EXPECT_GT (addresses.size (), 0);

    addresses = Dns::Resolver::lookupAllAddress ("localhost", AF_INET6);
    EXPECT_GT (addresses.size (), 0);

    addresses = _resolver->resolveAllAddress ("localhost");
    EXPECT_GT (addresses.size (), 0);

    addresses = Dns::Resolver::lookupAllAddress ("localhost");
    EXPECT_GT (addresses.size (), 0);

    addresses = Dns::Resolver ("255.255.255.255").resolveAllAddress ("localhost", AF_INET);
    EXPECT_EQ (addresses.size (), 0);

    addresses = Dns::Resolver ("8.8.8.8", 53).resolveAllAddress ("joinframework.net", AF_INET, 1ms);
    EXPECT_EQ (addresses.size (), 0);

    addresses = Dns::Resolver ("8.8.8.8", 53).resolveAllAddress ("joinframework.net", 1ms);
    EXPECT_EQ (addresses.size (), 0);

    addresses = Dns::Resolver::lookupAllAddress ("google.com");
    EXPECT_GT (addresses.size (), 0);

    addresses = _resolver->resolveAllAddress ("google.com");
    EXPECT_GT (addresses.size (), 0);

    addresses = Dns::Resolver::lookupAllAddress ("google.com");
    EXPECT_GT (addresses.size (), 0);
}

/**
 * @brief test the resolveAddress method.
 */
TEST_F (DnsTest, resolveAddress)
{
    IpAddress address = _resolver->resolveAddress ("", AF_INET);
    EXPECT_TRUE (address.isWildcard ());

    address = Dns::Resolver::lookupAddress ("", AF_INET6);
    EXPECT_TRUE (address.isWildcard ());

    address = _resolver->resolveAddress ("");
    EXPECT_TRUE (address.isWildcard ());

    address = Dns::Resolver::lookupAddress ("");
    EXPECT_TRUE (address.isWildcard ());

    address = _resolver->resolveAddress ("localhost", AF_INET);
    EXPECT_TRUE (address.isIpv4Address ());

    address = Dns::Resolver::lookupAddress ("localhost", AF_INET6);
    EXPECT_TRUE (address.isIpv6Address ());

    address = Dns::Resolver::lookupAddress ("localhost");
    EXPECT_TRUE (address.isLoopBack ());

    address = Dns::Resolver ("255.255.255.255").resolveAddress ("localhost", AF_INET);
    EXPECT_TRUE (address.isWildcard ());

    address = Dns::Resolver ("8.8.8.8", 53).resolveAddress ("joinframework.net", AF_INET, 1ms);
    EXPECT_TRUE (address.isWildcard ());

    address = Dns::Resolver ("8.8.8.8", 53).resolveAddress ("joinframework.net", 1ms);
    EXPECT_TRUE (address.isWildcard ());

    address = Dns::Resolver::lookupAddress ("google.com");
    EXPECT_FALSE (address.isWildcard ());

    address = _resolver->resolveAddress ("google.com");
    EXPECT_FALSE (address.isWildcard ());

    address = Dns::Resolver::lookupAddress ("google.com");
    EXPECT_FALSE (address.isWildcard ());
}

/**
 * @brief test the resolveAllName method.
 */
TEST_F (DnsTest, resolveAllName)
{
    AliasList aliases = _resolver->resolveAllName ("0.0.0.0");
    EXPECT_EQ (aliases.size (), 0);

    aliases = Dns::Resolver::lookupAllName ("0.0.0.0");
    EXPECT_EQ (aliases.size (), 0);

    aliases = _resolver->resolveAllName ("::");
    EXPECT_EQ (aliases.size (), 0);

    aliases = Dns::Resolver::lookupAllName ("::");
    EXPECT_EQ (aliases.size (), 0);

    // aliases = _resolver->resolveAllName ("192.168.24.32");
    // EXPECT_EQ (aliases.size (), 0);

    aliases = Dns::Resolver::lookupAllName ("192.168.24.32");
    EXPECT_EQ (aliases.size (), 0);

    aliases = _resolver->resolveAllName ("127.0.0.1");
    EXPECT_GT (aliases.size (), 0);

    aliases = Dns::Resolver::lookupAllName ("127.0.0.1");
    EXPECT_GT (aliases.size (), 0);

    aliases = _resolver->resolveAllName ("::1");
    EXPECT_GT (aliases.size (), 0);

    aliases = Dns::Resolver::lookupAllName ("::1");
    EXPECT_GT (aliases.size (), 0);

    aliases = Dns::Resolver ("255.255.255.255").resolveAllName ("127.0.0.1");
    EXPECT_EQ (aliases.size (), 0);

    aliases = Dns::Resolver ("255.255.255.255").resolveAllName ("::1");
    EXPECT_EQ (aliases.size (), 0);

    aliases =
        Dns::Resolver ("8.8.8.8", 53).resolveAllName (Dns::Resolver::lookupAddress ("joinframework.net", AF_INET), 1ms);
    EXPECT_EQ (aliases.size (), 0);

    aliases = Dns::Resolver ("8.8.8.8", 53)
                  .resolveAllName (Dns::Resolver::lookupAddress ("joinframework.net", AF_INET6), 1ms);
    EXPECT_EQ (aliases.size (), 0);
}

/**
 * @brief test the resolveName method.
 */
TEST_F (DnsTest, resolveName)
{
    std::string alias = _resolver->resolveName ("0.0.0.0");
    EXPECT_TRUE (alias.empty ());

    alias = Dns::Resolver::lookupName ("0.0.0.0");
    EXPECT_TRUE (alias.empty ());

    alias = _resolver->resolveName ("::");
    EXPECT_TRUE (alias.empty ());

    alias = Dns::Resolver::lookupName ("::");
    EXPECT_TRUE (alias.empty ());

    // alias = _resolver->resolveName ("192.168.24.32");
    // EXPECT_TRUE (alias.empty ());

    alias = Dns::Resolver::lookupName ("192.168.24.32");
    EXPECT_TRUE (alias.empty ());

    alias = _resolver->resolveName ("127.0.0.1");
    EXPECT_FALSE (alias.empty ());

    alias = Dns::Resolver::lookupName ("127.0.0.1");
    EXPECT_FALSE (alias.empty ());

    alias = _resolver->resolveName ("::1");
    EXPECT_FALSE (alias.empty ());

    alias = Dns::Resolver::lookupName ("::1");
    EXPECT_FALSE (alias.empty ());

    alias = Dns::Resolver ("255.255.255.255").resolveName ("127.0.0.1");
    EXPECT_TRUE (alias.empty ());

    alias = Dns::Resolver ("255.255.255.255").resolveName ("::1");
    EXPECT_TRUE (alias.empty ());

    alias =
        Dns::Resolver ("8.8.8.8", 53).resolveName (Dns::Resolver::lookupAddress ("joinframework.net", AF_INET), 1ms);
    EXPECT_TRUE (alias.empty ());

    alias =
        Dns::Resolver ("8.8.8.8", 53).resolveName (Dns::Resolver::lookupAddress ("joinframework.net", AF_INET6), 1ms);
    EXPECT_TRUE (alias.empty ());
}

/**
 * @brief test the resolveAllNameServer method.
 */
TEST_F (DnsTest, resolveAllNameServer)
{
    ServerList names = _resolver->resolveAllNameServer ("");
    EXPECT_EQ (names.size (), 0);

    names = Dns::Resolver::lookupAllNameServer ("");
    EXPECT_EQ (names.size (), 0);

    // names = _resolver->resolveAllNameServer ("localhost");
    // EXPECT_EQ (names.size (), 0);

    names = Dns::Resolver::lookupAllNameServer ("localhost");
    EXPECT_EQ (names.size (), 0);

    names = Dns::Resolver ("255.255.255.255").resolveAllNameServer ("localhost");
    EXPECT_EQ (names.size (), 0);

    names = Dns::Resolver ("8.8.8.8", 53).resolveAllNameServer ("joinframework.net", 1ms);
    EXPECT_EQ (names.size (), 0);

    names = Dns::Resolver::lookupAllNameServer ("google.com");
    EXPECT_GT (names.size (), 0);

    names = _resolver->resolveAllNameServer ("google.com");
    EXPECT_GT (names.size (), 0);

    names = Dns::Resolver (Dns::Resolver::lookupAddress ("a.gtld-servers.net").toString ())
                .resolveAllNameServer ("google.com");
    EXPECT_EQ (names.size (), 0);

    names = Dns::Resolver::lookupAllNameServer ("google.com");
    EXPECT_GT (names.size (), 0);
}

/**
 * @brief test the resolveNameServer method.
 */
TEST_F (DnsTest, resolveNameServer)
{
    std::string name = _resolver->resolveNameServer ("");
    EXPECT_TRUE (name.empty ());

    name = Dns::Resolver::lookupNameServer ("");
    EXPECT_TRUE (name.empty ());

    // name = _resolver->resolveNameServer ("localhost");
    // EXPECT_TRUE (name.empty ());

    name = Dns::Resolver::lookupNameServer ("localhost");
    EXPECT_TRUE (name.empty ());

    name = Dns::Resolver ("255.255.255.255").resolveNameServer ("localhost");
    EXPECT_TRUE (name.empty ());

    name = Dns::Resolver ("8.8.8.8", 53).resolveNameServer ("joinframework.net", 1ms);
    EXPECT_TRUE (name.empty ());

    name = Dns::Resolver::lookupNameServer ("google.com");
    EXPECT_FALSE (name.empty ());

    name = _resolver->resolveNameServer ("google.com");
    EXPECT_FALSE (name.empty ());

    name = Dns::Resolver (Dns::Resolver::lookupAddress ("a.gtld-servers.net").toString ())
               .resolveNameServer ("google.com");
    EXPECT_TRUE (name.empty ());

    name = Dns::Resolver::lookupNameServer ("google.com");
    EXPECT_FALSE (name.empty ());
}

/**
 * @brief test the resolveAuthority method.
 */
TEST_F (DnsTest, resolveAuthority)
{
    std::string name = _resolver->resolveAuthority ("");
    EXPECT_TRUE (name.empty ());

    name = Dns::Resolver::lookupAuthority ("");
    EXPECT_TRUE (name.empty ());

    // name = _resolver->resolveAuthority ("localhost");
    // EXPECT_TRUE (name.empty ());

    name = Dns::Resolver::lookupAuthority ("localhost");
    EXPECT_TRUE (name.empty ());

    name = Dns::Resolver ("255.255.255.255").resolveAuthority ("localhost");
    EXPECT_TRUE (name.empty ());

    name = Dns::Resolver ("8.8.8.8", 53).resolveAuthority ("joinframework.net", 1ms);
    EXPECT_TRUE (name.empty ());

    name = Dns::Resolver::lookupAuthority ("google.com");
    EXPECT_FALSE (name.empty ());

    name = _resolver->resolveAuthority ("google.com");
    EXPECT_FALSE (name.empty ());

    name = Dns::Resolver::lookupAuthority ("google.com");
    EXPECT_FALSE (name.empty ());
}

/**
 * @brief test the resolveAllMailExchanger method.
 */
TEST_F (DnsTest, resolveAllMailExchanger)
{
    ExchangerList exchangers = _resolver->resolveAllMailExchanger ("");
    EXPECT_EQ (exchangers.size (), 0);

    exchangers = Dns::Resolver::lookupAllMailExchanger ("");
    EXPECT_EQ (exchangers.size (), 0);

    // exchangers = _resolver->resolveAllMailExchanger ("localhost");
    // EXPECT_EQ (exchangers.size (), 0);

    exchangers = Dns::Resolver::lookupAllMailExchanger ("localhost");
    EXPECT_EQ (exchangers.size (), 0);

    exchangers = Dns::Resolver ("255.255.255.255").resolveAllMailExchanger ("localhost");
    EXPECT_EQ (exchangers.size (), 0);

    exchangers = Dns::Resolver ("8.8.8.8", 53).resolveAllMailExchanger ("joinframework.net", 1ms);
    EXPECT_EQ (exchangers.size (), 0);

    exchangers = Dns::Resolver::lookupAllMailExchanger ("google.com");
    EXPECT_GT (exchangers.size (), 0);

    exchangers = _resolver->resolveAllMailExchanger ("google.com");
    EXPECT_GT (exchangers.size (), 0);

    exchangers = Dns::Resolver::lookupAllMailExchanger ("google.com");
    EXPECT_GT (exchangers.size (), 0);
}

/**
 * @brief test the resolveMailExchanger method.
 */
TEST_F (DnsTest, resolveMailExchanger)
{
    std::string exchanger = _resolver->resolveMailExchanger ("");
    EXPECT_TRUE (exchanger.empty ());

    exchanger = Dns::Resolver::lookupMailExchanger ("");
    EXPECT_TRUE (exchanger.empty ());

    // exchanger = _resolver->resolveMailExchanger ("localhost");
    // EXPECT_TRUE (exchanger.empty ());

    exchanger = Dns::Resolver::lookupMailExchanger ("localhost");
    EXPECT_TRUE (exchanger.empty ());

    exchanger = Dns::Resolver ("255.255.255.255").resolveMailExchanger ("localhost");
    EXPECT_TRUE (exchanger.empty ());

    exchanger = Dns::Resolver ("8.8.8.8", 53).resolveMailExchanger ("joinframework.net", 1ms);
    EXPECT_TRUE (exchanger.empty ());

    exchanger = Dns::Resolver::lookupMailExchanger ("google.com");
    EXPECT_FALSE (exchanger.empty ());

    exchanger = _resolver->resolveMailExchanger ("google.com");
    EXPECT_FALSE (exchanger.empty ());

    exchanger = Dns::Resolver::lookupMailExchanger ("google.com");
    EXPECT_FALSE (exchanger.empty ());
}

/**
 * @brief test the resolveService method.
 */
TEST_F (DnsTest, resolveService)
{
    EXPECT_EQ (Dns::Resolver::resolveService ("foo"), 0);
    EXPECT_EQ (Dns::Resolver::resolveService ("ssh"), 22);
    EXPECT_EQ (Dns::Resolver::resolveService ("smtp"), 25);
    EXPECT_EQ (Dns::Resolver::resolveService ("smtps"), 465);
    EXPECT_EQ (Dns::Resolver::resolveService ("http"), 80);
    EXPECT_EQ (Dns::Resolver::resolveService ("https"), 443);
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
