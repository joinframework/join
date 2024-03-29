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
#include <join/protocol.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::UnixDgram;
using join::UnixStream;
using join::Raw;
using join::Udp;
using join::Icmp;
using join::Tcp;
using join::Tls;
using join::Http;
using join::Https;
using join::Smtp;
using join::Smtps;

/**
 * @brief test the family method.
 */
TEST (Protocol, family)
{
    ASSERT_EQ (UnixDgram ().family (), AF_UNIX);
    ASSERT_EQ (UnixStream ().family (), AF_UNIX);
    ASSERT_EQ (Raw ().family (), AF_PACKET);
    ASSERT_EQ (Udp ().family (), AF_INET);
    ASSERT_EQ (Udp::v6 ().family (), AF_INET6);
    ASSERT_EQ (Udp::v4 ().family (), AF_INET);
    ASSERT_EQ (Icmp ().family (), AF_INET);
    ASSERT_EQ (Icmp::v6 ().family (), AF_INET6);
    ASSERT_EQ (Icmp::v4 ().family (), AF_INET);
    ASSERT_EQ (Tcp ().family (), AF_INET);
    ASSERT_EQ (Tcp::v6 ().family (), AF_INET6);
    ASSERT_EQ (Tcp::v4 ().family (), AF_INET);
    ASSERT_EQ (Tls ().family (), AF_INET);
    ASSERT_EQ (Tls::v6 ().family (), AF_INET6);
    ASSERT_EQ (Tls::v4 ().family (), AF_INET);
    ASSERT_EQ (Http ().family (), AF_INET);
    ASSERT_EQ (Http::v6 ().family (), AF_INET6);
    ASSERT_EQ (Http::v4 ().family (), AF_INET);
    ASSERT_EQ (Https ().family (), AF_INET);
    ASSERT_EQ (Https::v6 ().family (), AF_INET6);
    ASSERT_EQ (Https::v4 ().family (), AF_INET);
    ASSERT_EQ (Smtp ().family (), AF_INET);
    ASSERT_EQ (Smtp::v6 ().family (), AF_INET6);
    ASSERT_EQ (Smtp::v4 ().family (), AF_INET);
    ASSERT_EQ (Smtps ().family (), AF_INET);
    ASSERT_EQ (Smtps::v6 ().family (), AF_INET6);
    ASSERT_EQ (Smtps::v4 ().family (), AF_INET);
}

/**
 * @brief test the type method.
 */
TEST (Protocol, type)
{
    ASSERT_EQ (UnixDgram ().type (), SOCK_DGRAM);
    ASSERT_EQ (UnixStream ().type (), SOCK_STREAM);
    ASSERT_EQ (Raw ().type (), SOCK_RAW);
    ASSERT_EQ (Udp ().type (), SOCK_DGRAM);
    ASSERT_EQ (Icmp ().type (), SOCK_RAW);
    ASSERT_EQ (Tcp ().type (), SOCK_STREAM);
    ASSERT_EQ (Tls ().type (), SOCK_STREAM);
    ASSERT_EQ (Http ().type (), SOCK_STREAM);
    ASSERT_EQ (Https ().type (), SOCK_STREAM);
    ASSERT_EQ (Smtp ().type (), SOCK_STREAM);
    ASSERT_EQ (Smtps ().type (), SOCK_STREAM);
}

/**
 * @brief test the protocol method.
 */
TEST (Protocol, protocol)
{
    ASSERT_EQ (UnixDgram ().protocol (), 0);
    ASSERT_EQ (UnixStream ().protocol (), 0);
    ASSERT_EQ (Raw ().protocol (), ::htons (ETH_P_ALL));
    ASSERT_EQ (Udp ().protocol (), IPPROTO_UDP);
    ASSERT_EQ (Icmp::v6 ().protocol (), IPPROTO_ICMPV6);
    ASSERT_EQ (Icmp::v4 ().protocol (), IPPROTO_ICMP);
    ASSERT_EQ (Tcp ().protocol (), IPPROTO_TCP);
    ASSERT_EQ (Tls ().protocol (), IPPROTO_TCP);
    ASSERT_EQ (Http ().protocol (), IPPROTO_TCP);
    ASSERT_EQ (Https ().protocol (), IPPROTO_TCP);
    ASSERT_EQ (Smtp ().protocol (), IPPROTO_TCP);
    ASSERT_EQ (Smtps ().protocol (), IPPROTO_TCP);
}

/**
 * @brief equal method.
 */
TEST (Protocol, equal)
{
    ASSERT_EQ (Udp::v4 (), Udp::v4 ());
    ASSERT_NE (Udp::v4 (), Udp::v6 ());
    ASSERT_EQ (Udp::v6 (), Udp::v6 ());
    ASSERT_NE (Udp::v6 (), Udp::v4 ());

    ASSERT_EQ (Icmp::v4 (), Icmp::v4 ());
    ASSERT_NE (Icmp::v4 (), Icmp::v6 ());
    ASSERT_EQ (Icmp::v6 (), Icmp::v6 ());
    ASSERT_NE (Icmp::v6 (), Icmp::v4 ());

    ASSERT_EQ (Tcp::v4 (), Tcp::v4 ());
    ASSERT_NE (Tcp::v4 (), Tcp::v6 ());
    ASSERT_EQ (Tcp::v6 (), Tcp::v6 ());
    ASSERT_NE (Tcp::v6 (), Tcp::v4 ());

    ASSERT_EQ (Tls::v4 (), Tls::v4 ());
    ASSERT_NE (Tls::v4 (), Tls::v6 ());
    ASSERT_EQ (Tls::v6 (), Tls::v6 ());
    ASSERT_NE (Tls::v6 (), Tls::v4 ());

    ASSERT_EQ (Http::v4 (), Http::v4 ());
    ASSERT_NE (Http::v4 (), Http::v6 ());
    ASSERT_EQ (Http::v6 (), Http::v6 ());
    ASSERT_NE (Http::v6 (), Http::v4 ());

    ASSERT_EQ (Https::v4 (), Https::v4 ());
    ASSERT_NE (Https::v4 (), Https::v6 ());
    ASSERT_EQ (Https::v6 (), Https::v6 ());
    ASSERT_NE (Https::v6 (), Https::v4 ());

    ASSERT_EQ (Smtp::v4 (), Smtp::v4 ());
    ASSERT_NE (Smtp::v4 (), Smtp::v6 ());
    ASSERT_EQ (Smtp::v6 (), Smtp::v6 ());
    ASSERT_NE (Smtp::v6 (), Smtp::v4 ());

    ASSERT_EQ (Smtps::v4 (), Smtps::v4 ());
    ASSERT_NE (Smtps::v4 (), Smtps::v6 ());
    ASSERT_EQ (Smtps::v6 (), Smtps::v6 ());
    ASSERT_NE (Smtps::v6 (), Smtps::v4 ());
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
