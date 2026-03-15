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
#include <join/error.hpp>
#include <join/arp.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::lastError;
using join::IpAddress;
using join::MacAddress;
using join::Errc;
using join::Arp;

/**
 * @brief Class used to test the ARP API.
 */
class ArpTest : public ::testing::Test
{
public:
    /**
     * @brief set up test case.
     */
    static void SetUpTestCase ()
    {
        [[maybe_unused]] int result;

        // create bridge.
        result = std::system ("brctl addbr br0");
        result = std::system ("ip link set br0 address 4e:ed:ed:ee:59:da");
        result = std::system ("ip addr add 192.168.16.100/24 brd 192.168.16.255 dev br0");
        result = std::system ("ip link set br0 up");

        // create veth interface.
        result = std::system ("ip netns add red");
        result = std::system ("ip link add veth0 type veth peer name eth0 netns red");
        result = std::system ("ip link set veth0 up arp on multicast on");
        result = std::system ("ip -n red link set eth0 address 4e:ed:ed:ee:59:db");
        result = std::system ("ip -n red addr add 192.168.16.200/24 brd 192.168.16.255 dev eth0");
        result = std::system ("ip -n red link set eth0 up arp on multicast on");
        result = std::system ("brctl addif br0 veth0");
    }

    /**
     * @brief tear down test case.
     */
    static void TearDownTestCase ()
    {
        [[maybe_unused]] int result;

        result = std::system ("brctl delif br0 veth0");
        result = std::system ("ip link set br0 down");
        result = std::system ("brctl delbr br0");

        result = std::system ("ip link set dev veth0 down");
        result = std::system ("ip link del veth0");
        result = std::system ("ip netns del red");
    }
};

/**
 * @brief Test get method.
 */
TEST_F (ArpTest, get)
{
    ASSERT_TRUE (Arp::get ("br0", IpAddress (AF_INET6)).isWildcard ());
    ASSERT_EQ (lastError, Errc::InvalidParam) << lastError.message ();

    ASSERT_TRUE (Arp::get ("foo0", "192.168.16.200").isWildcard ());
    ASSERT_EQ (lastError, std::errc::no_such_device) << lastError.message ();

    ASSERT_TRUE (Arp::get ("br0", "192.168.16.217", std::chrono::milliseconds (20)).isWildcard ());
    ASSERT_EQ (lastError, std::errc::no_such_device_or_address) << lastError.message ();

    ASSERT_EQ (Arp::get ("eth0", IpAddress::ipv4Address ("eth0")), MacAddress::address ("eth0"))
        << lastError.message ();

    ASSERT_EQ (Arp::get ("br0", "192.168.16.200"), "4e:ed:ed:ee:59:db") << lastError.message ();
}

/**
 * @brief Test request method.
 */
TEST_F (ArpTest, request)
{
    ASSERT_TRUE (Arp::request ("br0", IpAddress (AF_INET6)).isWildcard ());
    ASSERT_EQ (lastError, Errc::InvalidParam) << lastError.message ();

    ASSERT_TRUE (Arp::request ("foo0", "192.168.16.200").isWildcard ());
    ASSERT_EQ (lastError, std::errc::no_such_device) << lastError.message ();

    ASSERT_TRUE (Arp::request ("br0", "192.168.16.217", std::chrono::milliseconds (20)).isWildcard ());
    ASSERT_EQ (lastError, std::errc::no_such_device_or_address) << lastError.message ();

    ASSERT_EQ (Arp::request ("br0", "192.168.16.200"), "4e:ed:ed:ee:59:db") << lastError.message ();
}

/**
 * @brief Test add method.
 */
TEST_F (ArpTest, add)
{
    ASSERT_EQ (Arp::add ("br0", "4e:ed:ed:ee:59:dd", IpAddress (AF_INET6)), -1);
    ASSERT_EQ (lastError, Errc::InvalidParam) << lastError.message ();

    ASSERT_EQ (Arp::add ("foo0", "4e:ed:ed:ee:59:dd", "192.168.16.201"), -1);
    ASSERT_EQ (lastError, std::errc::no_such_device) << lastError.message ();

    ASSERT_EQ (Arp::add ("br0", "4e:ed:ed:ee:59:dd", "192.168.16.201"), 0) << lastError.message ();
}

/**
 * @brief Test add method.
 */
TEST_F (ArpTest, remove)
{
    ASSERT_EQ (Arp::remove ("br0", IpAddress (AF_INET6)), -1);
    ASSERT_EQ (lastError, Errc::InvalidParam) << lastError.message ();

    ASSERT_EQ (Arp::remove ("foo0", "192.168.16.200"), -1);
    ASSERT_EQ (lastError, std::errc::no_such_device) << lastError.message ();

    ASSERT_EQ (Arp::add ("br0", "4e:ed:ed:ee:59:dc", "192.168.16.210"), 0) << lastError.message ();
    ASSERT_EQ (Arp::cache ("br0", "192.168.16.210"), "4e:ed:ed:ee:59:dc") << lastError.message ();

    ASSERT_EQ (Arp::remove ("br0", "192.168.16.210"), 0) << lastError.message ();
    ASSERT_TRUE (Arp::cache ("br0", "192.168.16.210").isWildcard ());
}

/**
 * @brief Test cache method.
 */
TEST_F (ArpTest, cache)
{
    ASSERT_TRUE (Arp::cache ("br0", IpAddress (AF_INET6)).isWildcard ());
    ASSERT_EQ (lastError, Errc::InvalidParam) << lastError.message ();

    ASSERT_TRUE (Arp::cache ("foo0", "192.168.16.200").isWildcard ());
    ASSERT_EQ (lastError, std::errc::no_such_device) << lastError.message ();

    ASSERT_TRUE (Arp::cache ("br0", "192.168.16.200").isWildcard ());
    ASSERT_EQ (lastError, std::errc::no_such_device_or_address) << lastError.message ();

    ASSERT_EQ (Arp::add ("br0", "4e:ed:ed:ee:59:db", "192.168.16.200"), 0) << lastError.message ();
    ASSERT_EQ (Arp::cache ("br0", "192.168.16.200"), "4e:ed:ed:ee:59:db") << lastError.message ();

    std::system ("ip neigh add 192.168.16.210 dev br0 nud failed");
    ASSERT_TRUE (Arp::cache ("br0", "192.168.16.210").isWildcard ());
    ASSERT_EQ (lastError, std::errc::no_such_device_or_address);
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
