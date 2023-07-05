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
 * @brief test the resolveHost method.
 */
TEST (Resolver, resolveHost)
{
    IpAddress address = Resolver::resolveHost ("");
    EXPECT_TRUE (address.isWildcard ());

    address = Resolver::resolveHost ("localhost");
    EXPECT_TRUE (address.isLoopBack ());

    address = Resolver::resolveHost ("ip6-localhost", AF_INET6);
    EXPECT_TRUE (address.isIpv6Address ());
    EXPECT_TRUE (address.isLoopBack ());

    address = Resolver::resolveHost ("localhost", AF_INET);
    EXPECT_TRUE (address.isIpv4Address ());
    EXPECT_TRUE (address.isLoopBack ());
}

/**
 * @brief test the resolveAllHost method.
 */
TEST (Resolver, resolveAllHost)
{
    IpAddressList addressList = Resolver::resolveAllHost ("");
    EXPECT_EQ (addressList.size (), 0);
    
    addressList = Resolver::resolveAllHost ("localhost");
    EXPECT_GT (addressList.size (), 0);
}

/**
 * @brief test the resolveAddress method.
 */
TEST (Resolver, resolveAddress)
{
    std::string name = Resolver::resolveAddress ("192.168.24.32");
    EXPECT_EQ (name, "192.168.24.32");

    name = Resolver::resolveAddress ("127.0.0.1");
    EXPECT_NE (name.find ("localhost"), std::string::npos);

    name = Resolver::resolveAddress ("::1");
    EXPECT_NE (name.find ("localhost"), std::string::npos);
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
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
