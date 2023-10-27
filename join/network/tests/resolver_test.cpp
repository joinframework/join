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
#include <join/socket.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::IpAddress;
using join::IpAddressList;
using join::AliasList;
using join::ServerList;
using join::ExchangerList;
using join::Resolver;

/**
 * @brief test the nameServers method.
 */
TEST (Resolver, nameServers)
{
    IpAddressList servers = Resolver::nameServers ();
    EXPECT_GT (servers.size (), 0);
}

/**
 * @brief test the resolveAllHost method.
 */
TEST (Resolver, resolveAllHost)
{
    IpAddressList servers = Resolver::nameServers ();
    ASSERT_GT (servers.size (), 0);

    IpAddressList addresses = Resolver ().resolveAllHost ("", AF_INET, servers.front ());
    EXPECT_EQ (addresses.size (), 0);

    addresses = Resolver::resolveAllHost ("", AF_INET6);
    EXPECT_EQ (addresses.size (), 0);

    addresses = Resolver ().resolveAllHost ("", servers.front ());
    EXPECT_EQ (addresses.size (), 0);

    addresses = Resolver::resolveAllHost ("");
    EXPECT_EQ (addresses.size (), 0);

    addresses = Resolver ().resolveAllHost ("localhost", AF_INET, servers.front ());
    EXPECT_GT (addresses.size (), 0);

    addresses = Resolver::resolveAllHost ("localhost", AF_INET6);
    EXPECT_GT (addresses.size (), 0);

    addresses = Resolver ().resolveAllHost ("localhost", servers.front ());
    EXPECT_GT (addresses.size (), 0);

    addresses = Resolver::resolveAllHost ("localhost");
    EXPECT_GT (addresses.size (), 0);

    addresses = Resolver ("foo").resolveAllHost ("localhost", AF_INET, servers.front ());
    EXPECT_EQ (addresses.size (), 0);

    addresses = Resolver ("foo").resolveAllHost ("localhost", servers.front ());
    EXPECT_EQ (addresses.size (), 0);

    addresses = Resolver ().resolveAllHost ("localhost", AF_INET, "255.255.255.255");
    EXPECT_EQ (addresses.size (), 0);

    addresses = Resolver ().resolveAllHost ("localhost", "255.255.255.255");
    EXPECT_EQ (addresses.size (), 0);

    addresses = Resolver ().resolveAllHost ("joinframework.net", AF_INET, "8.8.8.8", Resolver::dnsPort, 1);
    EXPECT_EQ (addresses.size (), 0);

    addresses = Resolver ().resolveAllHost ("joinframework.net", "8.8.8.8", Resolver::dnsPort, 1);
    EXPECT_EQ (addresses.size (), 0);

    addresses = Resolver::resolveAllHost ("www.netflix.com");
    EXPECT_GT (addresses.size (), 0);

    addresses = Resolver ().resolveAllHost ("www.google.com", servers.front ());
    EXPECT_GT (addresses.size (), 0);

    addresses = Resolver::resolveAllHost ("www.amazon.com");
    EXPECT_GT (addresses.size (), 0);
}

/**
 * @brief test the resolveHost method.
 */
TEST (Resolver, resolveHost)
{
    IpAddressList servers = Resolver::nameServers ();
    ASSERT_GT (servers.size (), 0);

    IpAddress address = Resolver ().resolveHost ("", AF_INET, servers.front ());
    EXPECT_TRUE (address.isWildcard ());

    address = Resolver::resolveHost ("", AF_INET6);
    EXPECT_TRUE (address.isWildcard ());

    address = Resolver ().resolveHost ("", servers.front ());
    EXPECT_TRUE (address.isWildcard ());

    address = Resolver::resolveHost ("");
    EXPECT_TRUE (address.isWildcard ());

    address = Resolver ().resolveHost ("localhost", AF_INET, servers.front ());
    EXPECT_TRUE (address.isIpv4Address ());
    EXPECT_TRUE (address.isLoopBack ());

    address = Resolver::resolveHost ("localhost", AF_INET6);
    EXPECT_TRUE (address.isIpv6Address ());
    EXPECT_TRUE (address.isLoopBack ());

    address = Resolver ().resolveHost ("localhost", servers.front ());
    EXPECT_TRUE (address.isLoopBack ());

    address = Resolver::resolveHost ("localhost");
    EXPECT_TRUE (address.isLoopBack ());

    address = Resolver ("foo").resolveHost ("localhost", AF_INET, servers.front ());
    EXPECT_TRUE (address.isWildcard ());

    address = Resolver ("foo").resolveHost ("localhost", servers.front ());
    EXPECT_TRUE (address.isWildcard ());

    address = Resolver ().resolveHost ("localhost", AF_INET, "255.255.255.255");
    EXPECT_TRUE (address.isWildcard ());

    address = Resolver ().resolveHost ("localhost", "255.255.255.255");
    EXPECT_TRUE (address.isWildcard ());

    address = Resolver ().resolveHost ("joinframework.net", AF_INET, "8.8.8.8", Resolver::dnsPort, 1);
    EXPECT_TRUE (address.isWildcard ());

    address = Resolver ().resolveHost ("joinframework.net", "8.8.8.8", Resolver::dnsPort, 1);
    EXPECT_TRUE (address.isWildcard ());

    address = Resolver::resolveHost ("www.netflix.com");
    EXPECT_FALSE (address.isWildcard ());

    address = Resolver ().resolveHost ("www.google.com", servers.front ());
    EXPECT_FALSE (address.isWildcard ());

    address = Resolver::resolveHost ("www.amazon.com");
    EXPECT_FALSE (address.isWildcard ());
}

/**
 * @brief test the resolveAllAddress method.
 */
TEST (Resolver, resolveAllAddress)
{
    IpAddressList servers = Resolver::nameServers ();
    ASSERT_GT (servers.size (), 0);

    AliasList aliases = Resolver ().resolveAllAddress ("192.168.24.32", servers.front ());
    EXPECT_EQ (aliases.size (), 0);

    aliases = Resolver::resolveAllAddress ("192.168.24.32");
    EXPECT_EQ (aliases.size (), 0);

    aliases = Resolver ().resolveAllAddress ("127.0.0.2", servers.front ());
    EXPECT_GT (aliases.size (), 0);

    aliases = Resolver::resolveAllAddress ("127.0.0.2");
    EXPECT_GT (aliases.size (), 0);

    aliases = Resolver ().resolveAllAddress ("::1", servers.front ());
    EXPECT_GT (aliases.size (), 0);

    aliases = Resolver::resolveAllAddress ("::1");
    EXPECT_GT (aliases.size (), 0);

    aliases = Resolver ("foo").resolveAllAddress ("127.0.0.2", servers.front ());
    EXPECT_EQ (aliases.size (), 0);

    aliases = Resolver ("foo").resolveAllAddress ("::1", servers.front ());
    EXPECT_EQ (aliases.size (), 0);

    aliases = Resolver ().resolveAllAddress ("127.0.0.2", "255.255.255.255");
    EXPECT_EQ (aliases.size (), 0);

    aliases = Resolver ().resolveAllAddress ("::1", "255.255.255.255");
    EXPECT_EQ (aliases.size (), 0);

    aliases = Resolver ().resolveAllAddress (Resolver::resolveHost ("joinframework.net", AF_INET), "8.8.8.8", Resolver::dnsPort, 1);
    EXPECT_EQ (aliases.size (), 0);

    aliases = Resolver ().resolveAllAddress (Resolver::resolveHost ("joinframework.net", AF_INET6), "8.8.8.8", Resolver::dnsPort, 1);
    EXPECT_EQ (aliases.size (), 0);
}

/**
 * @brief test the resolveAddress method.
 */
TEST (Resolver, resolveAddress)
{
    IpAddressList servers = Resolver::nameServers ();
    ASSERT_GT (servers.size (), 0);

    std::string alias = Resolver ().resolveAddress ("192.168.24.32", servers.front ());
    EXPECT_TRUE (alias.empty ());

    alias = Resolver::resolveAddress ("192.168.24.32");
    EXPECT_TRUE (alias.empty ());

    alias = Resolver ().resolveAddress ("127.0.0.2", servers.front ());
    EXPECT_FALSE (alias.empty ());

    alias = Resolver::resolveAddress ("127.0.0.2");
    EXPECT_FALSE (alias.empty ());

    alias = Resolver ().resolveAddress ("::1", servers.front ());
    EXPECT_FALSE (alias.empty ());

    alias = Resolver::resolveAddress ("::1");
    EXPECT_FALSE (alias.empty ());

    alias = Resolver ("foo").resolveAddress ("127.0.0.2", servers.front ());
    EXPECT_TRUE (alias.empty ());

    alias = Resolver ("foo").resolveAddress ("::1", servers.front ());
    EXPECT_TRUE (alias.empty ());

    alias = Resolver ().resolveAddress ("127.0.0.2", "255.255.255.255");
    EXPECT_TRUE (alias.empty ());

    alias = Resolver ().resolveAddress ("::1", "255.255.255.255");
    EXPECT_TRUE (alias.empty ());

    alias = Resolver ().resolveAddress (Resolver::resolveHost ("joinframework.net", AF_INET), "8.8.8.8", Resolver::dnsPort, 1);
    EXPECT_TRUE (alias.empty ());

    alias = Resolver ().resolveAddress (Resolver::resolveHost ("joinframework.net", AF_INET6), "8.8.8.8", Resolver::dnsPort, 1);
    EXPECT_TRUE (alias.empty ());
}

/**
 * @brief test the resolveAllNameServer method.
 */
TEST (Resolver, resolveAllNameServer)
{
    IpAddressList servers = Resolver::nameServers ();
    ASSERT_GT (servers.size (), 0);

    ServerList names = Resolver ().resolveAllNameServer ("", servers.front ());
    EXPECT_GT (names.size (), 0);

    names = Resolver::resolveAllNameServer ("");
    EXPECT_GT (names.size (), 0);

    names = Resolver ().resolveAllNameServer ("localhost", servers.front ());
    EXPECT_EQ (names.size (), 0);

    names = Resolver::resolveAllNameServer ("localhost");
    EXPECT_EQ (names.size (), 0);

    names = Resolver ("foo").resolveAllNameServer ("localhost", servers.front ());
    EXPECT_EQ (names.size (), 0);

    names = Resolver ().resolveAllNameServer ("localhost", "255.255.255.255");
    EXPECT_EQ (names.size (), 0);

    names = Resolver ().resolveAllNameServer ("joinframework.net", "8.8.8.8", Resolver::dnsPort, 1);
    EXPECT_EQ (names.size (), 0);

    names = Resolver::resolveAllNameServer ("netflix.com");
    EXPECT_GT (names.size (), 0);

    names = Resolver ().resolveAllNameServer ("google.com", servers.front ());
    EXPECT_GT (names.size (), 0);

    names = Resolver ().resolveAllNameServer ("google.com", Resolver::resolveHost ("a.gtld-servers.net"));
    EXPECT_EQ (names.size (), 0);

    names = Resolver::resolveAllNameServer ("amazon.com");
    EXPECT_GT (names.size (), 0);
}

/**
 * @brief test the resolveNameServer method.
 */
TEST (Resolver, resolveNameServer)
{
    IpAddressList servers = Resolver::nameServers ();
    ASSERT_GT (servers.size (), 0);

    std::string name = Resolver ().resolveNameServer ("", servers.front ());
    EXPECT_FALSE (name.empty ());

    name = Resolver::resolveNameServer ("");
    EXPECT_FALSE (name.empty ());

    name = Resolver ().resolveNameServer ("localhost", servers.front ());
    EXPECT_TRUE (name.empty ());

    name = Resolver::resolveNameServer ("localhost");
    EXPECT_TRUE (name.empty ());

    name = Resolver ("foo").resolveNameServer ("localhost", servers.front ());
    EXPECT_TRUE (name.empty ());

    name = Resolver ().resolveNameServer ("localhost", "255.255.255.255");
    EXPECT_TRUE (name.empty ());

    name = Resolver ().resolveNameServer ("joinframework.net", "8.8.8.8", Resolver::dnsPort, 1);
    EXPECT_TRUE (name.empty ());

    name = Resolver::resolveNameServer ("netflix.com");
    EXPECT_FALSE (name.empty ());

    name = Resolver ().resolveNameServer ("google.com", servers.front ());
    EXPECT_FALSE (name.empty ());

    name = Resolver ().resolveNameServer ("google.com", Resolver::resolveHost ("a.gtld-servers.net"));
    EXPECT_TRUE (name.empty ());

    name = Resolver::resolveNameServer ("amazon.com");
    EXPECT_FALSE (name.empty ());
}

/**
 * @brief test the resolveAuthority method.
 */
TEST (Resolver, resolveAuthority)
{
    IpAddressList servers = Resolver::nameServers ();
    ASSERT_GT (servers.size (), 0);

    std::string name = Resolver ().resolveAuthority ("", servers.front ());
    EXPECT_FALSE (name.empty ());

    name = Resolver::resolveAuthority ("");
    EXPECT_FALSE (name.empty ());

    name = Resolver ().resolveAuthority ("localhost", servers.front ());
    EXPECT_TRUE (name.empty ());

    name = Resolver::resolveAuthority ("localhost");
    EXPECT_TRUE (name.empty ());

    name = Resolver ("foo").resolveAuthority ("localhost", servers.front ());
    EXPECT_TRUE (name.empty ());

    name = Resolver ().resolveAuthority ("localhost", "255.255.255.255");
    EXPECT_TRUE (name.empty ());

    name = Resolver ().resolveAuthority ("joinframework.net", "8.8.8.8", Resolver::dnsPort, 1);
    EXPECT_TRUE (name.empty ());

    name = Resolver::resolveAuthority ("netflix.com");
    EXPECT_FALSE (name.empty ());

    name = Resolver ().resolveAuthority ("google.com", servers.front ());
    EXPECT_FALSE (name.empty ());

    name = Resolver::resolveAuthority ("amazon.com");
    EXPECT_FALSE (name.empty ());
}

/**
 * @brief test the resolveAllMailExchanger method.
 */
TEST (Resolver, resolveAllMailExchanger)
{
    IpAddressList servers = Resolver::nameServers ();
    ASSERT_GT (servers.size (), 0);

    ExchangerList exchangers = Resolver ().resolveAllMailExchanger ("", servers.front ());
    EXPECT_EQ (exchangers.size (), 0);

    exchangers = Resolver::resolveAllMailExchanger ("");
    EXPECT_EQ (exchangers.size (), 0);

    exchangers = Resolver ().resolveAllMailExchanger ("localhost", servers.front ());
    EXPECT_EQ (exchangers.size (), 0);

    exchangers = Resolver::resolveAllMailExchanger ("localhost");
    EXPECT_EQ (exchangers.size (), 0);

    exchangers = Resolver ("foo").resolveAllMailExchanger ("localhost", servers.front ());
    EXPECT_EQ (exchangers.size (), 0);

    exchangers = Resolver ().resolveAllMailExchanger ("localhost", "255.255.255.255");
    EXPECT_EQ (exchangers.size (), 0);

    exchangers = Resolver ().resolveAllMailExchanger ("joinframework.net", "8.8.8.8", Resolver::dnsPort, 1);
    EXPECT_EQ (exchangers.size (), 0);

    exchangers = Resolver::resolveAllMailExchanger ("netflix.com");
    EXPECT_GT (exchangers.size (), 0);

    exchangers = Resolver ().resolveAllMailExchanger ("google.com", servers.front ());
    EXPECT_GT (exchangers.size (), 0);

    exchangers = Resolver::resolveAllMailExchanger ("amazon.com");
    EXPECT_GT (exchangers.size (), 0);
}

/**
 * @brief test the resolveMailExchanger method.
 */
TEST (Resolver, resolveMailExchanger)
{
    IpAddressList servers = Resolver::nameServers ();
    ASSERT_GT (servers.size (), 0);

    std::string exchanger = Resolver ().resolveMailExchanger ("", servers.front ());
    EXPECT_TRUE (exchanger.empty ());

    exchanger = Resolver::resolveMailExchanger ("");
    EXPECT_TRUE (exchanger.empty ());

    exchanger = Resolver ().resolveMailExchanger ("localhost", servers.front ());
    EXPECT_TRUE (exchanger.empty ());

    exchanger = Resolver::resolveMailExchanger ("localhost");
    EXPECT_TRUE (exchanger.empty ());

    exchanger = Resolver ("foo").resolveMailExchanger ("localhost", servers.front ());
    EXPECT_TRUE (exchanger.empty ());

    exchanger = Resolver ().resolveMailExchanger ("localhost", "255.255.255.255");
    EXPECT_TRUE (exchanger.empty ());

    exchanger = Resolver ().resolveMailExchanger ("joinframework.net", "8.8.8.8", Resolver::dnsPort, 1);
    EXPECT_TRUE (exchanger.empty ());

    exchanger = Resolver::resolveMailExchanger ("netflix.com");
    EXPECT_FALSE (exchanger.empty ());

    exchanger = Resolver ().resolveMailExchanger ("google.com", servers.front ());
    EXPECT_FALSE (exchanger.empty ());

    exchanger = Resolver::resolveMailExchanger ("amazon.com");
    EXPECT_FALSE (exchanger.empty ());
}

/**
 * @brief test the resolveService method.
 */
TEST (Resolver, resolveService)
{
    EXPECT_EQ (Resolver::resolveService ("ssh"), 22);
    EXPECT_EQ (Resolver::resolveService ("smtp"), 25);
    EXPECT_EQ (Resolver::resolveService ("smtps"), 465);
    EXPECT_EQ (Resolver::resolveService ("http"), 80);
    EXPECT_EQ (Resolver::resolveService ("https"), 443);
}

/**
 * @brief test the typeName method.
 */
TEST (Resolver, typeName)
{
    EXPECT_EQ (Resolver::typeName (0), "UNKNOWN");
    EXPECT_EQ (Resolver::typeName (Resolver::A), "A");
    EXPECT_EQ (Resolver::typeName (Resolver::NS), "NS");
    EXPECT_EQ (Resolver::typeName (Resolver::CNAME), "CNAME");
    EXPECT_EQ (Resolver::typeName (Resolver::SOA), "SOA");
    EXPECT_EQ (Resolver::typeName (Resolver::PTR), "PTR");
    EXPECT_EQ (Resolver::typeName (Resolver::MX), "MX");
    EXPECT_EQ (Resolver::typeName (Resolver::AAAA), "AAAA");
}

/**
 * @brief test the className method.
 */
TEST (Resolver, className)
{
    EXPECT_EQ (Resolver::className (0), "UNKNOWN");
    EXPECT_EQ (Resolver::className (Resolver::IN), "IN");
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
