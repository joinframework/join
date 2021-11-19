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

using join::IpAddress;
using join::MacAddress;
using join::Arp;

using join::lastError;

/**
 * @brief Test mac method.
 */
TEST (ArpProtocol, mac)
{
    ASSERT_TRUE (Arp ("bar0").mac ("192.168.16.120").isWildcard ());
    ASSERT_TRUE (Arp::mac ("192.168.16.120", "bar0").isWildcard ());

    ASSERT_TRUE (Arp ("eth0").mac (IpAddress (AF_INET6)).isWildcard ());
    ASSERT_TRUE (Arp::mac (IpAddress (AF_INET6), "eth0").isWildcard ());

    ASSERT_TRUE (Arp ("eth0").mac ("192.168.16.120").isWildcard ());
    ASSERT_TRUE (Arp::mac ("192.168.16.120", "eth0").isWildcard ());

    ASSERT_FALSE (Arp ("eth0").mac (IpAddress::ipv4Address ("eth0")).isWildcard ()) << lastError.message ();
    ASSERT_FALSE (Arp::mac (IpAddress::ipv4Address ("eth0"), "eth0").isWildcard ()) << lastError.message ();
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
