/**
 * MIT License
 *
 * Copyright (c) 2021 Mathieu Rabine
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
    EXPECT_GT (servers.size (), 0);

    IpAddressList addressList = Resolver ().resolveAllHost ("", AF_INET, servers.front ());
    EXPECT_EQ (addressList.size (), 0);

    addressList = Resolver::resolveAllHost ("", AF_INET6);
    EXPECT_EQ (addressList.size (), 0);

    addressList = Resolver ().resolveAllHost ("", servers.front ());
    EXPECT_EQ (addressList.size (), 0);

    addressList = Resolver::resolveAllHost ("");
    EXPECT_EQ (addressList.size (), 0);

    addressList = Resolver ().resolveAllHost ("localhost", AF_INET, servers.front ());
    EXPECT_GT (addressList.size (), 0);

    addressList = Resolver::resolveAllHost ("localhost", AF_INET6);
    EXPECT_GT (addressList.size (), 0);

    addressList = Resolver ().resolveAllHost ("localhost", servers.front ());
    EXPECT_GT (addressList.size (), 0);

    addressList = Resolver::resolveAllHost ("localhost");
    EXPECT_GT (addressList.size (), 0);
}

/**
 * @brief test the resolveHost method.
 */
TEST (Resolver, resolveHost)
{
    IpAddressList servers = Resolver::nameServers ();
    EXPECT_GT (servers.size (), 0);

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
}

/**
 * @brief test the resolveAllAddress method.
 */
TEST (Resolver, resolveAllAddress)
{
    IpAddressList servers = Resolver::nameServers ();
    EXPECT_GT (servers.size (), 0);

    AliasList aliasList = Resolver ().resolveAllAddress ("192.168.24.32", servers.front ());
    EXPECT_EQ (aliasList.size (), 0);

    aliasList = Resolver::resolveAllAddress ("192.168.24.32");
    EXPECT_EQ (aliasList.size (), 0);

    aliasList = Resolver ().resolveAllAddress ("127.0.0.2", servers.front ());
    EXPECT_GT (aliasList.size (), 0);

    aliasList = Resolver::resolveAllAddress ("127.0.0.2");
    EXPECT_GT (aliasList.size (), 0);

    aliasList = Resolver ().resolveAllAddress ("::1", servers.front ());
    EXPECT_GT (aliasList.size (), 0);

    aliasList = Resolver::resolveAllAddress ("::1");
    EXPECT_GT (aliasList.size (), 0);
}

/**
 * @brief test the resolveAddress method.
 */
TEST (Resolver, resolveAddress)
{
    IpAddressList servers = Resolver::nameServers ();
    EXPECT_GT (servers.size (), 0);

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
}

/**
 * @brief test the resolveService method.
 */
TEST (Resolver, resolveService)
{
    EXPECT_EQ (Resolver::resolveService ("ssh"), 22);
    EXPECT_EQ (Resolver::resolveService ("smtp"), 25);
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
