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

/**
 * @brief test the nameServers method.
 */
TEST (Resolver, nameServers)
{
    IpAddressList servers = Udp::Resolver::nameServers ();
    ASSERT_GT (servers.size (), 0);

    servers = Icmp::Resolver::nameServers ();
    ASSERT_GT (servers.size (), 0);
}

/**
 * @brief test the resolve method.
 */
TEST (Resolver, resolve)
{
    Udp::Endpoint endpoint = Udp::Resolver::resolve ("localhost");
    ASSERT_EQ (endpoint.ip (), "127.0.0.1");
    ASSERT_EQ (endpoint.port (), 0);

    endpoint = Udp::Resolver::resolve ("https://localhost");
    ASSERT_EQ (endpoint.ip (), "127.0.0.1");
    ASSERT_EQ (endpoint.port (), 443);

    endpoint = Udp::Resolver::resolve ("https://localhost:5000");
    ASSERT_EQ (endpoint.ip (), "127.0.0.1");
    ASSERT_EQ (endpoint.port (), 5000);

    endpoint = Udp::Resolver::resolve ("https://192.168.0.1:5000");
    ASSERT_EQ (endpoint.ip (), "192.168.0.1");
    ASSERT_EQ (endpoint.port (), 5000);

    endpoint = Udp::Resolver::resolve ("https://[::1]:5000");
    ASSERT_EQ (endpoint.ip (), "::1");
    ASSERT_EQ (endpoint.port (), 5000);

    endpoint = Udp::Resolver::resolve ("localhost", AF_INET);
    ASSERT_EQ (endpoint.ip (), "127.0.0.1");
    ASSERT_EQ (endpoint.port (), 0);

    endpoint = Udp::Resolver::resolve ("http://localhost", AF_INET6);
    ASSERT_TRUE (endpoint.ip ().isWildcard ());
    ASSERT_EQ (endpoint.port (), 0);

    endpoint = Udp::Resolver::resolve ("https://localhost", AF_INET);
    ASSERT_EQ (endpoint.ip (), "127.0.0.1");
    ASSERT_EQ (endpoint.port (), 443);

    endpoint = Udp::Resolver::resolve ("https://localhost:5000", AF_INET);
    ASSERT_EQ (endpoint.ip (), "127.0.0.1");
    ASSERT_EQ (endpoint.port (), 5000);

    endpoint = Udp::Resolver::resolve ("ip6-localhost", AF_INET6);
    ASSERT_EQ (endpoint.ip (), "::1");
    ASSERT_EQ (endpoint.port (), 0);

    endpoint = Udp::Resolver::resolve ("http://ip6-localhost", AF_INET6);
    ASSERT_EQ (endpoint.ip (), "::1");
    ASSERT_EQ (endpoint.port (), 80);

    endpoint = Udp::Resolver::resolve ("http://ip6-localhost:5001", AF_INET6);
    ASSERT_EQ (endpoint.ip (), "::1");
    ASSERT_EQ (endpoint.port (), 5001);

    endpoint = Udp::Resolver::resolve ("https://ip6-localhost", AF_INET);
    ASSERT_TRUE (endpoint.ip ().isWildcard ());
    ASSERT_EQ (endpoint.port (), 0);
}

/**
 * @brief test the resolveHost method.
 */
TEST (Resolver, resolveHost)
{
    IpAddress address = Udp::Resolver::resolveHost ("localhost");
    ASSERT_TRUE (address.isLoopBack ());

    address = Udp::Resolver::resolveHost ("localhost", AF_INET6);
    ASSERT_TRUE (address.isWildcard ());

    address = Udp::Resolver::resolveHost ("localhost", AF_INET);
    ASSERT_TRUE (address.isIpv4Address ());
    ASSERT_TRUE (address.isLoopBack ());
    
    address = Udp::Resolver::resolveHost ("ip6-localhost", AF_INET6);
    ASSERT_TRUE (address.isIpv6Address ());
    ASSERT_TRUE (address.isLoopBack ());

    address = Udp::Resolver::resolveHost ("ip6-localhost", AF_INET);
    ASSERT_TRUE (address.isWildcard ());

    address = Icmp::Resolver::resolveHost ("localhost");
    ASSERT_TRUE (address.isLoopBack ());

    address = Icmp::Resolver::resolveHost ("localhost", AF_INET6);
    ASSERT_TRUE (address.isWildcard ());

    address = Icmp::Resolver::resolveHost ("localhost", AF_INET);
    ASSERT_TRUE (address.isIpv4Address ());
    ASSERT_TRUE (address.isLoopBack ());
    
    address = Icmp::Resolver::resolveHost ("ip6-localhost", AF_INET6);
    ASSERT_TRUE (address.isIpv6Address ());
    ASSERT_TRUE (address.isLoopBack ());

    address = Icmp::Resolver::resolveHost ("ip6-localhost", AF_INET);
    ASSERT_TRUE (address.isWildcard ());
}

/**
 * @brief test the resolveAllHost method.
 */
TEST (Resolver, resolveAllHost)
{
    IpAddressList addressList = Udp::Resolver::resolveAllHost ("localhost");
    ASSERT_GT (addressList.size (), 0);

    addressList = Icmp::Resolver::resolveAllHost ("localhost");
    ASSERT_GT (addressList.size (), 0);
}

/**
 * @brief test the resolveAddress method.
 */
TEST (Resolver, resolveAddress)
{
    std::string name = Udp::Resolver::resolveAddress ("::1");
    ASSERT_EQ (name, "ip6-localhost");

    name = Udp::Resolver::resolveAddress ("127.0.0.1");
    ASSERT_EQ (name, "localhost");

    name = Icmp::Resolver::resolveAddress ("::1");
    ASSERT_EQ (name, "ip6-localhost");

    name = Icmp::Resolver::resolveAddress ("127.0.0.1");
    ASSERT_EQ (name, "localhost");
}

/**
 * @brief test the resolveService method.
 */
TEST (Resolver, resolveService)
{
    ASSERT_EQ (Udp::Resolver::resolveService ("ssh"), 22);
    ASSERT_EQ (Udp::Resolver::resolveService ("smtp"), 25);
    ASSERT_EQ (Udp::Resolver::resolveService ("http"), 80);
    ASSERT_EQ (Udp::Resolver::resolveService ("https"), 443);

    ASSERT_EQ (Icmp::Resolver::resolveService ("ssh"), 22);
    ASSERT_EQ (Icmp::Resolver::resolveService ("smtp"), 25);
    ASSERT_EQ (Icmp::Resolver::resolveService ("http"), 80);
    ASSERT_EQ (Icmp::Resolver::resolveService ("https"), 443);
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}