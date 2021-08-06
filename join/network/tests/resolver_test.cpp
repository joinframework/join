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

using join::net::IpAddress;
using join::net::IpAddressList;
using join::net::Udp;
using join::net::Icmp;
using join::net::Tcp;

/**
 * @brief test the nameServers method.
 */
TEST (Resolver, nameServers)
{
    IpAddressList servers = Udp::Resolver::nameServers ();
    EXPECT_GT (servers.size (), 0);

    servers = Icmp::Resolver::nameServers ();
    EXPECT_GT (servers.size (), 0);

    servers = Tcp::Resolver::nameServers ();
    EXPECT_GT (servers.size (), 0);
}

/**
 * @brief test the resolve method.
 */
TEST (Resolver, resolve)
{
    Udp::Endpoint endpoint = Udp::Resolver::resolve ("localhost");
    EXPECT_EQ (endpoint.ip (), "127.0.0.1");
    EXPECT_EQ (endpoint.port (), 0);

    endpoint = Udp::Resolver::resolve ("https://localhost");
    EXPECT_EQ (endpoint.ip (), "127.0.0.1");
    EXPECT_EQ (endpoint.port (), 443);

    endpoint = Udp::Resolver::resolve ("https://localhost:5000");
    EXPECT_EQ (endpoint.ip (), "127.0.0.1");
    EXPECT_EQ (endpoint.port (), 5000);

    endpoint = Udp::Resolver::resolve ("https://192.168.0.1:5000");
    EXPECT_EQ (endpoint.ip (), "192.168.0.1");
    EXPECT_EQ (endpoint.port (), 5000);

    endpoint = Udp::Resolver::resolve ("localhost", AF_INET);
    EXPECT_EQ (endpoint.ip (), "127.0.0.1");
    EXPECT_EQ (endpoint.port (), 0);

    endpoint = Udp::Resolver::resolve ("http://localhost", AF_INET6);
    EXPECT_TRUE (endpoint.ip ().isWildcard ());
    EXPECT_EQ (endpoint.port (), 0);

    endpoint = Udp::Resolver::resolve ("https://localhost", AF_INET);
    EXPECT_EQ (endpoint.ip (), "127.0.0.1");
    EXPECT_EQ (endpoint.port (), 443);

    endpoint = Udp::Resolver::resolve ("https://localhost:5000", AF_INET);
    EXPECT_EQ (endpoint.ip (), "127.0.0.1");
    EXPECT_EQ (endpoint.port (), 5000);
}

/**
 * @brief test the resolveHost method.
 */
TEST (Resolver, resolveHost)
{
    IpAddress address = Udp::Resolver::resolveHost ("localhost");
    EXPECT_TRUE (address.isLoopBack ());

    address = Udp::Resolver::resolveHost ("localhost", AF_INET6);
    EXPECT_TRUE (address.isWildcard ());

    address = Udp::Resolver::resolveHost ("localhost", AF_INET);
    EXPECT_TRUE (address.isIpv4Address ());
    EXPECT_TRUE (address.isLoopBack ());

    address = Icmp::Resolver::resolveHost ("localhost");
    EXPECT_TRUE (address.isLoopBack ());

    address = Icmp::Resolver::resolveHost ("localhost", AF_INET6);
    EXPECT_TRUE (address.isWildcard ());

    address = Icmp::Resolver::resolveHost ("localhost", AF_INET);
    EXPECT_TRUE (address.isIpv4Address ());
    EXPECT_TRUE (address.isLoopBack ());

    address = Tcp::Resolver::resolveHost ("localhost");
    EXPECT_TRUE (address.isLoopBack ());

    address = Tcp::Resolver::resolveHost ("localhost", AF_INET6);
    EXPECT_TRUE (address.isWildcard ());

    address = Tcp::Resolver::resolveHost ("localhost", AF_INET);
    EXPECT_TRUE (address.isIpv4Address ());
    EXPECT_TRUE (address.isLoopBack ());
}

/**
 * @brief test the resolveAllHost method.
 */
TEST (Resolver, resolveAllHost)
{
    IpAddressList addressList = Udp::Resolver::resolveAllHost ("localhost");
    EXPECT_GT (addressList.size (), 0);

    addressList = Icmp::Resolver::resolveAllHost ("localhost");
    EXPECT_GT (addressList.size (), 0);

    addressList = Tcp::Resolver::resolveAllHost ("localhost");
    EXPECT_GT (addressList.size (), 0);
}

/**
 * @brief test the resolveAddress method.
 */
TEST (Resolver, resolveAddress)
{
    std::string name = Udp::Resolver::resolveAddress ("127.0.0.1");
    EXPECT_EQ (name, "localhost");

    name = Icmp::Resolver::resolveAddress ("127.0.0.1");
    EXPECT_EQ (name, "localhost");

    name = Tcp::Resolver::resolveAddress ("127.0.0.1");
    EXPECT_EQ (name, "localhost");
}

/**
 * @brief test the resolveService method.
 */
TEST (Resolver, resolveService)
{
    EXPECT_EQ (Udp::Resolver::resolveService ("ssh"), 22);
    EXPECT_EQ (Udp::Resolver::resolveService ("smtp"), 25);
    EXPECT_EQ (Udp::Resolver::resolveService ("http"), 80);
    EXPECT_EQ (Udp::Resolver::resolveService ("https"), 443);

    EXPECT_EQ (Icmp::Resolver::resolveService ("ssh"), 22);
    EXPECT_EQ (Icmp::Resolver::resolveService ("smtp"), 25);
    EXPECT_EQ (Icmp::Resolver::resolveService ("http"), 80);
    EXPECT_EQ (Icmp::Resolver::resolveService ("https"), 443);

    EXPECT_EQ (Tcp::Resolver::resolveService ("ssh"), 22);
    EXPECT_EQ (Tcp::Resolver::resolveService ("smtp"), 25);
    EXPECT_EQ (Tcp::Resolver::resolveService ("http"), 80);
    EXPECT_EQ (Tcp::Resolver::resolveService ("https"), 443);
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
