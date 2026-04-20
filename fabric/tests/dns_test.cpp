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
#include <join/resolver.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::Dns;
using join::IpAddress;
using join::IpAddressList;
using join::AliasList;
using join::ServerList;
using join::ExchangerList;

using namespace std::chrono_literals;

class DnsTest : public ::testing::Test
{
protected:
    void SetUp () override
    {
        _servers = Dns::Resolver::nameServers ();
        ASSERT_GT (_servers.size (), 0);

        _resolver = std::make_unique<Dns::Resolver> ("", _servers.front ());
    }

    IpAddressList _servers;
    std::unique_ptr<Dns::Resolver> _resolver;
};

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

    addresses = Dns::Resolver ("foo", _servers.front ()).resolveAllAddress ("localhost", AF_INET);
    EXPECT_EQ (addresses.size (), 0);

    addresses = Dns::Resolver ("", "255.255.255.255").resolveAllAddress ("localhost", AF_INET);
    EXPECT_EQ (addresses.size (), 0);

    addresses = Dns::Resolver ("", "8.8.8.8", 53).resolveAllAddress ("joinframework.net", AF_INET, 1ms);
    EXPECT_EQ (addresses.size (), 0);

    addresses = Dns::Resolver ("", "8.8.8.8", 53).resolveAllAddress ("joinframework.net", 1ms);
    EXPECT_EQ (addresses.size (), 0);

    addresses = Dns::Resolver::lookupAllAddress ("www.netflix.com");
    EXPECT_GT (addresses.size (), 0);

    addresses = _resolver->resolveAllAddress ("www.google.com");
    EXPECT_GT (addresses.size (), 0);

    addresses = Dns::Resolver::lookupAllAddress ("www.amazon.com");
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
    EXPECT_TRUE (address.isLoopBack ());

    address = Dns::Resolver::lookupAddress ("localhost", AF_INET6);
    EXPECT_TRUE (address.isIpv6Address ());
    EXPECT_TRUE (address.isLoopBack ());

    address = _resolver->resolveAddress ("localhost");
    EXPECT_TRUE (address.isLoopBack ());

    address = Dns::Resolver::lookupAddress ("localhost");
    EXPECT_TRUE (address.isLoopBack ());

    address = Dns::Resolver ("foo", _servers.front ()).resolveAddress ("localhost", AF_INET);
    EXPECT_TRUE (address.isWildcard ());

    address = Dns::Resolver ("", "255.255.255.255").resolveAddress ("localhost", AF_INET);
    EXPECT_TRUE (address.isWildcard ());

    address = Dns::Resolver ("", "8.8.8.8", 53).resolveAddress ("joinframework.net", AF_INET, 1ms);
    EXPECT_TRUE (address.isWildcard ());

    address = Dns::Resolver ("", "8.8.8.8", 53).resolveAddress ("joinframework.net", 1ms);
    EXPECT_TRUE (address.isWildcard ());

    address = Dns::Resolver::lookupAddress ("www.netflix.com");
    EXPECT_FALSE (address.isWildcard ());

    address = _resolver->resolveAddress ("www.google.com");
    EXPECT_FALSE (address.isWildcard ());

    address = Dns::Resolver::lookupAddress ("www.amazon.com");
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

    aliases = _resolver->resolveAllName ("192.168.24.32");
    EXPECT_EQ (aliases.size (), 0);

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

    aliases = Dns::Resolver ("foo", _servers.front ()).resolveAllName ("127.0.0.1");
    EXPECT_EQ (aliases.size (), 0);

    aliases = Dns::Resolver ("foo", _servers.front ()).resolveAllName ("::1");
    EXPECT_EQ (aliases.size (), 0);

    aliases = Dns::Resolver ("", "255.255.255.255").resolveAllName ("127.0.0.1");
    EXPECT_EQ (aliases.size (), 0);

    aliases = Dns::Resolver ("", "255.255.255.255").resolveAllName ("::1");
    EXPECT_EQ (aliases.size (), 0);

    aliases = Dns::Resolver ("", "8.8.8.8", 53)
                  .resolveAllName (Dns::Resolver::lookupAddress ("joinframework.net", AF_INET), 1ms);
    EXPECT_EQ (aliases.size (), 0);

    aliases = Dns::Resolver ("", "8.8.8.8", 53)
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

    alias = _resolver->resolveName ("192.168.24.32");
    EXPECT_TRUE (alias.empty ());

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

    alias = Dns::Resolver ("foo", _servers.front ()).resolveName ("127.0.0.1");
    EXPECT_TRUE (alias.empty ());

    alias = Dns::Resolver ("foo", _servers.front ()).resolveName ("::1");
    EXPECT_TRUE (alias.empty ());

    alias = Dns::Resolver ("", "255.255.255.255").resolveName ("127.0.0.1");
    EXPECT_TRUE (alias.empty ());

    alias = Dns::Resolver ("", "255.255.255.255").resolveName ("::1");
    EXPECT_TRUE (alias.empty ());

    alias = Dns::Resolver ("", "8.8.8.8", 53)
                .resolveName (Dns::Resolver::lookupAddress ("joinframework.net", AF_INET), 1ms);
    EXPECT_TRUE (alias.empty ());

    alias = Dns::Resolver ("", "8.8.8.8", 53)
                .resolveName (Dns::Resolver::lookupAddress ("joinframework.net", AF_INET6), 1ms);
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

    names = _resolver->resolveAllNameServer ("localhost");
    EXPECT_EQ (names.size (), 0);

    names = Dns::Resolver::lookupAllNameServer ("localhost");
    EXPECT_EQ (names.size (), 0);

    names = Dns::Resolver ("foo", _servers.front ()).resolveAllNameServer ("localhost");
    EXPECT_EQ (names.size (), 0);

    names = Dns::Resolver ("", "255.255.255.255").resolveAllNameServer ("localhost");
    EXPECT_EQ (names.size (), 0);

    names = Dns::Resolver ("", "8.8.8.8", 53).resolveAllNameServer ("joinframework.net", 1ms);
    EXPECT_EQ (names.size (), 0);

    names = Dns::Resolver::lookupAllNameServer ("netflix.com");
    EXPECT_GT (names.size (), 0);

    names = _resolver->resolveAllNameServer ("google.com");
    EXPECT_GT (names.size (), 0);

    names = Dns::Resolver ("", Dns::Resolver::lookupAddress ("a.gtld-servers.net")).resolveAllNameServer ("google.com");
    EXPECT_EQ (names.size (), 0);

    names = Dns::Resolver::lookupAllNameServer ("amazon.com");
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

    name = _resolver->resolveNameServer ("localhost");
    EXPECT_TRUE (name.empty ());

    name = Dns::Resolver::lookupNameServer ("localhost");
    EXPECT_TRUE (name.empty ());

    name = Dns::Resolver ("foo", _servers.front ()).resolveNameServer ("localhost");
    EXPECT_TRUE (name.empty ());

    name = Dns::Resolver ("", "255.255.255.255").resolveNameServer ("localhost");
    EXPECT_TRUE (name.empty ());

    name = Dns::Resolver ("", "8.8.8.8", 53).resolveNameServer ("joinframework.net", 1ms);
    EXPECT_TRUE (name.empty ());

    name = Dns::Resolver::lookupNameServer ("netflix.com");
    EXPECT_FALSE (name.empty ());

    name = _resolver->resolveNameServer ("google.com");
    EXPECT_FALSE (name.empty ());

    name = Dns::Resolver ("", Dns::Resolver::lookupAddress ("a.gtld-servers.net")).resolveNameServer ("google.com");
    EXPECT_TRUE (name.empty ());

    name = Dns::Resolver::lookupNameServer ("amazon.com");
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

    name = _resolver->resolveAuthority ("localhost");
    EXPECT_TRUE (name.empty ());

    name = Dns::Resolver::lookupAuthority ("localhost");
    EXPECT_TRUE (name.empty ());

    name = Dns::Resolver ("foo", _servers.front ()).resolveAuthority ("localhost");
    EXPECT_TRUE (name.empty ());

    name = Dns::Resolver ("", "255.255.255.255").resolveAuthority ("localhost");
    EXPECT_TRUE (name.empty ());

    name = Dns::Resolver ("", "8.8.8.8", 53).resolveAuthority ("joinframework.net", 1ms);
    EXPECT_TRUE (name.empty ());

    name = Dns::Resolver::lookupAuthority ("netflix.com");
    EXPECT_FALSE (name.empty ());

    name = _resolver->resolveAuthority ("google.com");
    EXPECT_FALSE (name.empty ());

    name = Dns::Resolver::lookupAuthority ("amazon.com");
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

    exchangers = _resolver->resolveAllMailExchanger ("localhost");
    EXPECT_EQ (exchangers.size (), 0);

    exchangers = Dns::Resolver::lookupAllMailExchanger ("localhost");
    EXPECT_EQ (exchangers.size (), 0);

    exchangers = Dns::Resolver ("foo", _servers.front ()).resolveAllMailExchanger ("localhost");
    EXPECT_EQ (exchangers.size (), 0);

    exchangers = Dns::Resolver ("", "255.255.255.255").resolveAllMailExchanger ("localhost");
    EXPECT_EQ (exchangers.size (), 0);

    exchangers = Dns::Resolver ("", "8.8.8.8", 53).resolveAllMailExchanger ("joinframework.net", 1ms);
    EXPECT_EQ (exchangers.size (), 0);

    exchangers = Dns::Resolver::lookupAllMailExchanger ("netflix.com");
    EXPECT_GT (exchangers.size (), 0);

    exchangers = _resolver->resolveAllMailExchanger ("google.com");
    EXPECT_GT (exchangers.size (), 0);

    exchangers = Dns::Resolver::lookupAllMailExchanger ("amazon.com");
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

    exchanger = _resolver->resolveMailExchanger ("localhost");
    EXPECT_TRUE (exchanger.empty ());

    exchanger = Dns::Resolver::lookupMailExchanger ("localhost");
    EXPECT_TRUE (exchanger.empty ());

    exchanger = Dns::Resolver ("foo", _servers.front ()).resolveMailExchanger ("localhost");
    EXPECT_TRUE (exchanger.empty ());

    exchanger = Dns::Resolver ("", "255.255.255.255").resolveMailExchanger ("localhost");
    EXPECT_TRUE (exchanger.empty ());

    exchanger = Dns::Resolver ("", "8.8.8.8", 53).resolveMailExchanger ("joinframework.net", 1ms);
    EXPECT_TRUE (exchanger.empty ());

    exchanger = Dns::Resolver::lookupMailExchanger ("netflix.com");
    EXPECT_FALSE (exchanger.empty ());

    exchanger = _resolver->resolveMailExchanger ("google.com");
    EXPECT_FALSE (exchanger.empty ());

    exchanger = Dns::Resolver::lookupMailExchanger ("amazon.com");
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
