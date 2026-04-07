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
#include <join/dns.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::Dns;
using join::IpAddress;
using join::IpAddressList;
using join::AliasList;
using join::ServerList;
using join::ExchangerList;

using namespace std::chrono_literals;

/**
 * @brief test the nameServers method.
 */
TEST (DnsTest, nameServers)
{
    IpAddressList servers = Dns::Resolver::nameServers ();
    EXPECT_GT (servers.size (), 0);
}

/**
 * @brief test the resolveAllAddress method.
 */
TEST (DnsTest, resolveAllAddress)
{
    IpAddressList servers = Dns::Resolver::nameServers ();
    ASSERT_GT (servers.size (), 0);

    IpAddressList addresses = Dns::Resolver ("", servers.front ()).resolveAllAddress ("", AF_INET);
    EXPECT_EQ (addresses.size (), 0);

    addresses = Dns::Resolver::lookupAllAddress ("", AF_INET6);
    EXPECT_EQ (addresses.size (), 0);

    addresses = Dns::Resolver ("", servers.front ()).resolveAllAddress ("");
    EXPECT_EQ (addresses.size (), 0);

    addresses = Dns::Resolver::lookupAllAddress ("");
    EXPECT_EQ (addresses.size (), 0);

    addresses = Dns::Resolver ("", servers.front ()).resolveAllAddress ("localhost", AF_INET);
    EXPECT_GT (addresses.size (), 0);

    addresses = Dns::Resolver::lookupAllAddress ("localhost", AF_INET6);
    EXPECT_GT (addresses.size (), 0);

    addresses = Dns::Resolver ("", servers.front ()).resolveAllAddress ("localhost");
    EXPECT_GT (addresses.size (), 0);

    addresses = Dns::Resolver::lookupAllAddress ("localhost");
    EXPECT_GT (addresses.size (), 0);

    addresses = Dns::Resolver ("foo", servers.front ()).resolveAllAddress ("localhost", AF_INET);
    EXPECT_EQ (addresses.size (), 0);

    addresses = Dns::Resolver ("foo", servers.front ()).resolveAllAddress ("localhost");
    EXPECT_EQ (addresses.size (), 0);

    addresses = Dns::Resolver ("", "255.255.255.255").resolveAllAddress ("localhost", AF_INET);
    EXPECT_EQ (addresses.size (), 0);

    addresses = Dns::Resolver ("", "255.255.255.255").resolveAllAddress ("localhost");
    EXPECT_EQ (addresses.size (), 0);

    addresses = Dns::Resolver ("", "8.8.8.8", 53).resolveAllAddress ("joinframework.net", AF_INET, 1ms);
    EXPECT_EQ (addresses.size (), 0);

    addresses = Dns::Resolver ("", "8.8.8.8", 53).resolveAllAddress ("joinframework.net", 1ms);
    EXPECT_EQ (addresses.size (), 0);

    addresses = Dns::Resolver::lookupAllAddress ("www.netflix.com");
    EXPECT_GT (addresses.size (), 0);

    addresses = Dns::Resolver ("", servers.front ()).resolveAllAddress ("www.google.com");
    EXPECT_GT (addresses.size (), 0);

    addresses = Dns::Resolver::lookupAllAddress ("www.amazon.com");
    EXPECT_GT (addresses.size (), 0);
}

/**
 * @brief test the resolveAddress method.
 */
TEST (DnsTest, resolveAddress)
{
    IpAddressList servers = Dns::Resolver::nameServers ();
    ASSERT_GT (servers.size (), 0);

    IpAddress address = Dns::Resolver ("", servers.front ()).resolveAddress ("", AF_INET);
    EXPECT_TRUE (address.isWildcard ());

    address = Dns::Resolver::lookupAddress ("", AF_INET6);
    EXPECT_TRUE (address.isWildcard ());

    address = Dns::Resolver ("", servers.front ()).resolveAddress ("");
    EXPECT_TRUE (address.isWildcard ());

    address = Dns::Resolver::lookupAddress ("");
    EXPECT_TRUE (address.isWildcard ());

    address = Dns::Resolver ("", servers.front ()).resolveAddress ("localhost", AF_INET);
    EXPECT_TRUE (address.isIpv4Address ());
    EXPECT_TRUE (address.isLoopBack ());

    address = Dns::Resolver::lookupAddress ("localhost", AF_INET6);
    EXPECT_TRUE (address.isIpv6Address ());
    EXPECT_TRUE (address.isLoopBack ());

    address = Dns::Resolver ("", servers.front ()).resolveAddress ("localhost");
    EXPECT_TRUE (address.isLoopBack ());

    address = Dns::Resolver::lookupAddress ("localhost");
    EXPECT_TRUE (address.isLoopBack ());

    address = Dns::Resolver ("foo", servers.front ()).resolveAddress ("localhost", AF_INET);
    EXPECT_TRUE (address.isWildcard ());

    address = Dns::Resolver ("foo", servers.front ()).resolveAddress ("localhost");
    EXPECT_TRUE (address.isWildcard ());

    address = Dns::Resolver ("", "255.255.255.255").resolveAddress ("localhost", AF_INET);
    EXPECT_TRUE (address.isWildcard ());

    address = Dns::Resolver ("", "255.255.255.255").resolveAddress ("localhost");
    EXPECT_TRUE (address.isWildcard ());

    address = Dns::Resolver ("", "8.8.8.8", 53).resolveAddress ("joinframework.net", AF_INET, 1ms);
    EXPECT_TRUE (address.isWildcard ());

    address = Dns::Resolver ("", "8.8.8.8", 53).resolveAddress ("joinframework.net", 1ms);
    EXPECT_TRUE (address.isWildcard ());

    address = Dns::Resolver::lookupAddress ("www.netflix.com");
    EXPECT_FALSE (address.isWildcard ());

    address = Dns::Resolver ("", servers.front ()).resolveAddress ("www.google.com");
    EXPECT_FALSE (address.isWildcard ());

    address = Dns::Resolver::lookupAddress ("www.amazon.com");
    EXPECT_FALSE (address.isWildcard ());
}

/**
 * @brief test the resolveAllName method.
 */
TEST (DnsTest, resolveAllName)
{
    IpAddressList servers = Dns::Resolver::nameServers ();
    ASSERT_GT (servers.size (), 0);

    AliasList aliases = Dns::Resolver ("", servers.front ()).lookupAllName ("192.168.24.32");
    EXPECT_EQ (aliases.size (), 0);

    aliases = Dns::Resolver::lookupAllName ("192.168.24.32");
    EXPECT_EQ (aliases.size (), 0);

    aliases = Dns::Resolver ("", servers.front ()).resolveAllName ("127.0.0.1");
    EXPECT_GT (aliases.size (), 0);

    aliases = Dns::Resolver::lookupAllName ("127.0.0.1");
    EXPECT_GT (aliases.size (), 0);

    aliases = Dns::Resolver ("", servers.front ()).resolveAllName ("::1");
    EXPECT_GT (aliases.size (), 0);

    aliases = Dns::Resolver::lookupAllName ("::1");
    EXPECT_GT (aliases.size (), 0);

    aliases = Dns::Resolver ("foo", servers.front ()).resolveAllName ("127.0.0.1");
    EXPECT_EQ (aliases.size (), 0);

    aliases = Dns::Resolver ("foo", servers.front ()).resolveAllName ("::1");
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
TEST (DnsTest, resolveName)
{
    IpAddressList servers = Dns::Resolver::nameServers ();
    ASSERT_GT (servers.size (), 0);

    std::string alias = Dns::Resolver ("", servers.front ()).resolveName ("192.168.24.32");
    EXPECT_TRUE (alias.empty ());

    alias = Dns::Resolver::lookupName ("192.168.24.32");
    EXPECT_TRUE (alias.empty ());

    alias = Dns::Resolver ("", servers.front ()).resolveName ("127.0.0.1");
    EXPECT_FALSE (alias.empty ());

    alias = Dns::Resolver::lookupName ("127.0.0.1");
    EXPECT_FALSE (alias.empty ());

    alias = Dns::Resolver ("", servers.front ()).resolveName ("::1");
    EXPECT_FALSE (alias.empty ());

    alias = Dns::Resolver::lookupName ("::1");
    EXPECT_FALSE (alias.empty ());

    alias = Dns::Resolver ("foo", servers.front ()).resolveName ("127.0.0.1");
    EXPECT_TRUE (alias.empty ());

    alias = Dns::Resolver ("foo", servers.front ()).resolveName ("::1");
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
TEST (DnsTest, resolveAllNameServer)
{
    IpAddressList servers = Dns::Resolver::nameServers ();
    ASSERT_GT (servers.size (), 0);

    ServerList names = Dns::Resolver ("", servers.front ()).resolveAllNameServer ("");
    EXPECT_GT (names.size (), 0);

    names = Dns::Resolver::lookupAllNameServer ("");
    EXPECT_GT (names.size (), 0);

    names = Dns::Resolver ("", servers.front ()).resolveAllNameServer ("localhost");
    EXPECT_EQ (names.size (), 0);

    names = Dns::Resolver::lookupAllNameServer ("localhost");
    EXPECT_EQ (names.size (), 0);

    names = Dns::Resolver ("foo", servers.front ()).resolveAllNameServer ("localhost");
    EXPECT_EQ (names.size (), 0);

    names = Dns::Resolver ("", "255.255.255.255").resolveAllNameServer ("localhost");
    EXPECT_EQ (names.size (), 0);

    names = Dns::Resolver ("", "8.8.8.8", 53).resolveAllNameServer ("joinframework.net", 1ms);
    EXPECT_EQ (names.size (), 0);

    names = Dns::Resolver::lookupAllNameServer ("netflix.com");
    EXPECT_GT (names.size (), 0);

    names = Dns::Resolver ("", servers.front ()).resolveAllNameServer ("google.com");
    EXPECT_GT (names.size (), 0);

    names = Dns::Resolver ("", Dns::Resolver::lookupAddress ("a.gtld-servers.net")).resolveAllNameServer ("google.com");
    EXPECT_EQ (names.size (), 0);

    names = Dns::Resolver::lookupAllNameServer ("amazon.com");
    EXPECT_GT (names.size (), 0);
}

/**
 * @brief test the resolveNameServer method.
 */
TEST (DnsTest, resolveNameServer)
{
    IpAddressList servers = Dns::Resolver::nameServers ();
    ASSERT_GT (servers.size (), 0);

    std::string name = Dns::Resolver ("", servers.front ()).resolveNameServer ("");
    EXPECT_FALSE (name.empty ());

    name = Dns::Resolver::lookupNameServer ("");
    EXPECT_FALSE (name.empty ());

    name = Dns::Resolver ("", servers.front ()).resolveNameServer ("localhost");
    EXPECT_TRUE (name.empty ());

    name = Dns::Resolver::lookupNameServer ("localhost");
    EXPECT_TRUE (name.empty ());

    name = Dns::Resolver ("foo", servers.front ()).resolveNameServer ("localhost");
    EXPECT_TRUE (name.empty ());

    name = Dns::Resolver ("", "255.255.255.255").resolveNameServer ("localhost");
    EXPECT_TRUE (name.empty ());

    name = Dns::Resolver ("", "8.8.8.8", 53).resolveNameServer ("joinframework.net", 1ms);
    EXPECT_TRUE (name.empty ());

    name = Dns::Resolver::lookupNameServer ("netflix.com");
    EXPECT_FALSE (name.empty ());

    name = Dns::Resolver ("", servers.front ()).resolveNameServer ("google.com");
    EXPECT_FALSE (name.empty ());

    name = Dns::Resolver ("", Dns::Resolver::lookupAddress ("a.gtld-servers.net")).resolveNameServer ("google.com");
    EXPECT_TRUE (name.empty ());

    name = Dns::Resolver::lookupNameServer ("amazon.com");
    EXPECT_FALSE (name.empty ());
}

/**
 * @brief test the resolveAuthority method.
 */
TEST (DnsTest, resolveAuthority)
{
    IpAddressList servers = Dns::Resolver::nameServers ();
    ASSERT_GT (servers.size (), 0);

    std::string name = Dns::Resolver ("", servers.front ()).resolveAuthority ("");
    EXPECT_FALSE (name.empty ());

    name = Dns::Resolver::lookupAuthority ("");
    EXPECT_FALSE (name.empty ());

    name = Dns::Resolver ("", servers.front ()).resolveAuthority ("localhost");
    EXPECT_TRUE (name.empty ());

    name = Dns::Resolver::lookupAuthority ("localhost");
    EXPECT_TRUE (name.empty ());

    name = Dns::Resolver ("foo", servers.front ()).resolveAuthority ("localhost");
    EXPECT_TRUE (name.empty ());

    name = Dns::Resolver ("", "255.255.255.255").resolveAuthority ("localhost");
    EXPECT_TRUE (name.empty ());

    name = Dns::Resolver ("", "8.8.8.8", 53).resolveAuthority ("joinframework.net", 1ms);
    EXPECT_TRUE (name.empty ());

    name = Dns::Resolver::lookupAuthority ("netflix.com");
    EXPECT_FALSE (name.empty ());

    name = Dns::Resolver ("", servers.front ()).resolveAuthority ("google.com");
    EXPECT_FALSE (name.empty ());

    name = Dns::Resolver::lookupAuthority ("amazon.com");
    EXPECT_FALSE (name.empty ());
}

/**
 * @brief test the resolveAllMailExchanger method.
 */
TEST (DnsTest, resolveAllMailExchanger)
{
    IpAddressList servers = Dns::Resolver::nameServers ();
    ASSERT_GT (servers.size (), 0);

    ExchangerList exchangers = Dns::Resolver ("", servers.front ()).resolveAllMailExchanger ("");
    EXPECT_EQ (exchangers.size (), 0);

    exchangers = Dns::Resolver::lookupAllMailExchanger ("");
    EXPECT_EQ (exchangers.size (), 0);

    exchangers = Dns::Resolver ("", servers.front ()).resolveAllMailExchanger ("localhost");
    EXPECT_EQ (exchangers.size (), 0);

    exchangers = Dns::Resolver::lookupAllMailExchanger ("localhost");
    EXPECT_EQ (exchangers.size (), 0);

    exchangers = Dns::Resolver ("foo", servers.front ()).resolveAllMailExchanger ("localhost");
    EXPECT_EQ (exchangers.size (), 0);

    exchangers = Dns::Resolver ("", "255.255.255.255").resolveAllMailExchanger ("localhost");
    EXPECT_EQ (exchangers.size (), 0);

    exchangers = Dns::Resolver ("", "8.8.8.8", 53).resolveAllMailExchanger ("joinframework.net", 1ms);
    EXPECT_EQ (exchangers.size (), 0);

    exchangers = Dns::Resolver::lookupAllMailExchanger ("netflix.com");
    EXPECT_GT (exchangers.size (), 0);

    exchangers = Dns::Resolver ("", servers.front ()).resolveAllMailExchanger ("google.com");
    EXPECT_GT (exchangers.size (), 0);

    exchangers = Dns::Resolver::lookupAllMailExchanger ("amazon.com");
    EXPECT_GT (exchangers.size (), 0);
}

/**
 * @brief test the resolveMailExchanger method.
 */
TEST (DnsTest, resolveMailExchanger)
{
    IpAddressList servers = Dns::Resolver::nameServers ();
    ASSERT_GT (servers.size (), 0);

    std::string exchanger = Dns::Resolver ("", servers.front ()).resolveMailExchanger ("");
    EXPECT_TRUE (exchanger.empty ());

    exchanger = Dns::Resolver::lookupMailExchanger ("");
    EXPECT_TRUE (exchanger.empty ());

    exchanger = Dns::Resolver ("", servers.front ()).resolveMailExchanger ("localhost");
    EXPECT_TRUE (exchanger.empty ());

    exchanger = Dns::Resolver::lookupMailExchanger ("localhost");
    EXPECT_TRUE (exchanger.empty ());

    exchanger = Dns::Resolver ("foo", servers.front ()).resolveMailExchanger ("localhost");
    EXPECT_TRUE (exchanger.empty ());

    exchanger = Dns::Resolver ("", "255.255.255.255").resolveMailExchanger ("localhost");
    EXPECT_TRUE (exchanger.empty ());

    exchanger = Dns::Resolver ("", "8.8.8.8", 53).resolveMailExchanger ("joinframework.net", 1ms);
    EXPECT_TRUE (exchanger.empty ());

    exchanger = Dns::Resolver::lookupMailExchanger ("netflix.com");
    EXPECT_FALSE (exchanger.empty ());

    exchanger = Dns::Resolver ("", servers.front ()).resolveMailExchanger ("google.com");
    EXPECT_FALSE (exchanger.empty ());

    exchanger = Dns::Resolver::lookupMailExchanger ("amazon.com");
    EXPECT_FALSE (exchanger.empty ());
}

/**
 * @brief test the resolveService method.
 */
TEST (DnsTest, resolveService)
{
    EXPECT_EQ (Dns::Resolver::resolveService ("ssh"), 22);
    EXPECT_EQ (Dns::Resolver::resolveService ("smtp"), 25);
    EXPECT_EQ (Dns::Resolver::resolveService ("smtps"), 465);
    EXPECT_EQ (Dns::Resolver::resolveService ("http"), 80);
    EXPECT_EQ (Dns::Resolver::resolveService ("https"), 443);
}

/**
 * @brief test the typeName method.
 */
TEST (DnsTest, typeName)
{
    EXPECT_EQ (Dns::Resolver::typeName (0), "UNKNOWN");
    EXPECT_EQ (Dns::Resolver::typeName (Dns::Resolver::A), "A");
    EXPECT_EQ (Dns::Resolver::typeName (Dns::Resolver::NS), "NS");
    EXPECT_EQ (Dns::Resolver::typeName (Dns::Resolver::CNAME), "CNAME");
    EXPECT_EQ (Dns::Resolver::typeName (Dns::Resolver::SOA), "SOA");
    EXPECT_EQ (Dns::Resolver::typeName (Dns::Resolver::PTR), "PTR");
    EXPECT_EQ (Dns::Resolver::typeName (Dns::Resolver::MX), "MX");
    EXPECT_EQ (Dns::Resolver::typeName (Dns::Resolver::AAAA), "AAAA");
    EXPECT_EQ (Dns::Resolver::typeName (Dns::Resolver::SRV), "SRV");
    EXPECT_EQ (Dns::Resolver::typeName (Dns::Resolver::ANY), "ANY");
}

/**
 * @brief test the className method.
 */
TEST (DnsTest, className)
{
    EXPECT_EQ (Dns::Resolver::className (0), "UNKNOWN");
    EXPECT_EQ (Dns::Resolver::className (Dns::Resolver::IN), "IN");
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
