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
#include <join/ipaddress.hpp>

// Libraries.
#include <gtest/gtest.h>

// C++.
#include <sstream>

using join::IpAddress;

/**
 * @brief Test default construction.
 */
TEST (IpAddress, defaultConstruct)
{
    IpAddress ip;
    ASSERT_EQ (ip.family (), AF_INET);
    ASSERT_STREQ (ip.toString ().c_str (), "0.0.0.0");
}

/**
 * @brief Test family construction.
 */
TEST (IpAddress, familyConstruct)
{
    ASSERT_THROW (IpAddress (AF_UNSPEC), std::invalid_argument);

    IpAddress ip4 (AF_INET);
    ASSERT_EQ (ip4.family (), AF_INET);
    ASSERT_STREQ (ip4.toString ().c_str (), "0.0.0.0");

    IpAddress ip6 (AF_INET6);
    ASSERT_EQ (ip6.family (), AF_INET6);
    ASSERT_STREQ (ip6.toString ().c_str (), "::");
}

/**
 * @brief Test copy construction.
 */
TEST (IpAddress, copyConstruct)
{
    IpAddress ip = "0.0.0.0";

    IpAddress ip4 (ip);
    ASSERT_EQ (ip4.family (), AF_INET);
    ASSERT_STREQ (ip4.toString ().c_str (), "0.0.0.0");

    ip = "::";

    IpAddress ip6 (ip);
    ASSERT_EQ (ip6.family (), AF_INET6);
    ASSERT_STREQ (ip6.toString ().c_str (), "::");
}

/**
 * @brief Test move construction.
 */
TEST (IpAddress, moveConstruct)
{
    IpAddress ip = "0.0.0.0";

    IpAddress ip4 (std::move (ip));
    ASSERT_EQ (ip4.family (), AF_INET);
    ASSERT_STREQ (ip4.toString ().c_str (), "0.0.0.0");

    ip = "::";

    IpAddress ip6 (std::move (ip));
    ASSERT_EQ (ip6.family (), AF_INET6);
    ASSERT_STREQ (ip6.toString ().c_str (), "::");
}

/**
 * @brief Test sockaddrConstruct construction.
 */
TEST (IpAddress, sockaddrConstruct)
{
    struct sockaddr_storage sa;
    memset (&sa, 0, sizeof (sa));

    ASSERT_THROW (IpAddress (*reinterpret_cast <struct sockaddr*> (&sa)), std::invalid_argument);

    struct sockaddr_storage sa4;
    memset (&sa4, 0, sizeof (sa4));

    reinterpret_cast <struct sockaddr_in*> (&sa4)->sin_family = AF_INET;
    IpAddress ip4 (*reinterpret_cast <struct sockaddr*> (&sa4));
    ASSERT_EQ (ip4.family (), AF_INET);
    ASSERT_STREQ (ip4.toString ().c_str (), "0.0.0.0");

    struct sockaddr_storage sa6;
    memset (&sa6, 0, sizeof (sa6));

    reinterpret_cast <struct sockaddr_in6*> (&sa6)->sin6_family = AF_INET6;
    IpAddress ip6 (*reinterpret_cast <struct sockaddr*> (&sa6));
    ASSERT_EQ (ip6.family (), AF_INET6);
    ASSERT_STREQ (ip6.toString ().c_str (), "::");
}

/**
 * @brief Test addr construction.
 */
TEST (IpAddress, addrConstruct)
{
    char sa[1];
    memset (&sa, 0, sizeof (sa));

    ASSERT_THROW (IpAddress (&sa, sizeof (sa)), std::invalid_argument);
    ASSERT_THROW (IpAddress (&sa, sizeof (sa), 0), std::invalid_argument);

    struct in_addr sa4;
    memset (&sa4, 0, sizeof (sa4));

    IpAddress ip4 (&sa4, sizeof (sa4));
    ASSERT_EQ (ip4.family (), AF_INET);
    ASSERT_STREQ (ip4.toString ().c_str (), "0.0.0.0");

    IpAddress scopedIp4 (&sa4, sizeof (sa4), 0);
    ASSERT_EQ (scopedIp4.family (), AF_INET);
    ASSERT_STREQ (scopedIp4.toString ().c_str (), "0.0.0.0");

    struct in6_addr sa6;
    memset (&sa6, 0, sizeof (sa6));

    IpAddress ip6 (&sa6, sizeof (sa6));
    ASSERT_EQ (ip6.family (), AF_INET6);
    ASSERT_STREQ (ip6.toString ().c_str (), "::");

    IpAddress scopedIp6 (&sa6, sizeof (sa6), 0);
    ASSERT_EQ (scopedIp6.family (), AF_INET6);
    ASSERT_STREQ (scopedIp6.toString ().c_str (), "::");
}

/**
 * @brief Test string construction.
 */
TEST (IpAddress, stringConstruct)
{
    IpAddress ip ("", AF_INET);
    ASSERT_EQ (ip.family (), AF_INET);
    ASSERT_STREQ (ip.toString ().c_str (), "0.0.0.0");

    ip = IpAddress (nullptr, AF_INET);
    ASSERT_EQ (ip.family (), AF_INET);
    ASSERT_STREQ (ip.toString ().c_str (), "0.0.0.0");

    ip = IpAddress ("0.0.0.0");
    ASSERT_EQ (ip.family (), AF_INET);
    ASSERT_STREQ (ip.toString ().c_str (), "0.0.0.0");

    ip = IpAddress ("0.0.0.0", AF_INET);
    ASSERT_EQ (ip.family (), AF_INET);
    ASSERT_STREQ (ip.toString ().c_str (), "0.0.0.0");

    ip = IpAddress ("0.0.0.0", AF_INET6);
    ASSERT_EQ (ip.family (), AF_INET6);
    ASSERT_STREQ (ip.toString ().c_str (), "::ffff:0.0.0.0");

    ip = IpAddress (std::string ("0.0.0.0"));
    ASSERT_EQ (ip.family (), AF_INET);
    ASSERT_STREQ (ip.toString ().c_str (), "0.0.0.0");

    ip = IpAddress (std::string ("0.0.0.0"), AF_INET);
    ASSERT_EQ (ip.family (), AF_INET);
    ASSERT_STREQ (ip.toString ().c_str (), "0.0.0.0");

    ip = IpAddress (std::string ("0.0.0.0"), AF_INET6);
    ASSERT_EQ (ip.family (), AF_INET6);
    ASSERT_STREQ (ip.toString ().c_str (), "::ffff:0.0.0.0");

    ip = IpAddress ("");
    ASSERT_EQ (ip.family (), AF_INET6);
    ASSERT_STREQ (ip.toString ().c_str (), "::");

    ip = IpAddress ("", AF_INET6);
    ASSERT_EQ (ip.family (), AF_INET6);
    ASSERT_STREQ (ip.toString ().c_str (), "::");

    ip = IpAddress (nullptr);
    ASSERT_EQ (ip.family (), AF_INET6);
    ASSERT_STREQ (ip.toString ().c_str (), "::");

    ip = IpAddress (nullptr, AF_INET6);
    ASSERT_EQ (ip.family (), AF_INET6);
    ASSERT_STREQ (ip.toString ().c_str (), "::");

    ip = IpAddress ("::");
    ASSERT_EQ (ip.family (), AF_INET6);
    ASSERT_STREQ (ip.toString ().c_str (), "::");

    ASSERT_THROW (ip = IpAddress ("::", AF_INET), std::invalid_argument);

    ip = IpAddress ("0:0:0:0:0:0:0:0");
    ASSERT_EQ (ip.family (), AF_INET6);
    ASSERT_STREQ (ip.toString ().c_str (), "::");

    ASSERT_THROW (ip = IpAddress ("0:0:0:0:0:0:0:0", AF_INET), std::invalid_argument);

    ip = IpAddress ("0::0");
    ASSERT_EQ (ip.family (), AF_INET6);
    ASSERT_STREQ (ip.toString ().c_str (), "::");

    ASSERT_THROW (ip = IpAddress ("0::0", AF_INET), std::invalid_argument);

    ip = IpAddress ("::", AF_INET6);
    ASSERT_EQ (ip.family (), AF_INET6);
    ASSERT_STREQ (ip.toString ().c_str (), "::");

    ip = IpAddress (std::string ("::"));
    ASSERT_EQ (ip.family (), AF_INET6);
    ASSERT_STREQ (ip.toString ().c_str (), "::");

    ASSERT_THROW (ip = IpAddress (std::string ("::"), AF_INET), std::invalid_argument);

    ip = IpAddress (std::string ("::"), AF_INET6);
    ASSERT_EQ (ip.family (), AF_INET6);
    ASSERT_STREQ (ip.toString ().c_str (), "::");

    ip = IpAddress ("192.168.14.31");
    ASSERT_EQ (ip.family (), AF_INET);
    ASSERT_STREQ (ip.toString ().c_str (), "192.168.14.31");

    ip = IpAddress ("192.168.14.31", AF_INET);
    ASSERT_EQ (ip.family (), AF_INET);
    ASSERT_STREQ (ip.toString ().c_str (), "192.168.14.31");

    ip = IpAddress ("192.168.14.31", AF_INET6);
    ASSERT_EQ (ip.family (), AF_INET6);
    ASSERT_STREQ (ip.toString ().c_str (), "::ffff:192.168.14.31");

    ip = IpAddress (std::string ("192.168.14.31"));
    ASSERT_EQ (ip.family (), AF_INET);
    ASSERT_STREQ (ip.toString ().c_str (), "192.168.14.31");

    ip = IpAddress (std::string ("192.168.14.31"), AF_INET);
    ASSERT_EQ (ip.family (), AF_INET);
    ASSERT_STREQ (ip.toString ().c_str (), "192.168.14.31");

    ip = IpAddress (std::string ("192.168.14.31"), AF_INET6);
    ASSERT_EQ (ip.family (), AF_INET6);
    ASSERT_STREQ (ip.toString ().c_str (), "::ffff:192.168.14.31");

    ip = IpAddress ("fe80::57f3:baa4:fc3a:890a");
    ASSERT_EQ (ip.family (), AF_INET6);
    ASSERT_STREQ (ip.toString ().c_str (), "fe80::57f3:baa4:fc3a:890a");

    ASSERT_THROW (ip = IpAddress ("fe80::57f3:baa4:fc3a:890a", AF_INET), std::invalid_argument);

    ip = IpAddress ("fe80::57f3:baa4:fc3a:890a", AF_INET6);
    ASSERT_EQ (ip.family (), AF_INET6);
    ASSERT_STREQ (ip.toString ().c_str (), "fe80::57f3:baa4:fc3a:890a");

    ip = IpAddress (std::string ("fe80::57f3:baa4:fc3a:890a"));
    ASSERT_EQ (ip.family (), AF_INET6);
    ASSERT_STREQ (ip.toString ().c_str (), "fe80::57f3:baa4:fc3a:890a");

    ASSERT_THROW (ip = IpAddress (std::string ("fe80::57f3:baa4:fc3a:890a"), AF_INET), std::invalid_argument);

    ip = IpAddress (std::string ("fe80::57f3:baa4:fc3a:890a"), AF_INET6);
    ASSERT_EQ (ip.family (), AF_INET6);
    ASSERT_STREQ (ip.toString ().c_str (), "fe80::57f3:baa4:fc3a:890a");
}

/**
 * @brief Test prefix construction.
 */
TEST (IpAddress, prefixConstruct)
{
    ASSERT_THROW (IpAddress (0, AF_UNSPEC), std::invalid_argument);

    ASSERT_THROW (IpAddress (40, AF_INET), std::invalid_argument);
    ASSERT_THROW (IpAddress (-8, AF_INET), std::invalid_argument);

    IpAddress mask = IpAddress (32, AF_INET);
    ASSERT_EQ (mask.family (), AF_INET);
    ASSERT_STREQ (mask.toString ().c_str (), "255.255.255.255");

    mask = IpAddress (24, AF_INET);
    ASSERT_EQ (mask.family (), AF_INET);
    ASSERT_STREQ (mask.toString ().c_str (), "255.255.255.0");

    mask = IpAddress (16, AF_INET);
    ASSERT_EQ (mask.family (), AF_INET);
    ASSERT_STREQ (mask.toString ().c_str (), "255.255.0.0");

    mask = IpAddress (8, AF_INET);
    ASSERT_EQ (mask.family (), AF_INET);
    ASSERT_STREQ (mask.toString ().c_str (), "255.0.0.0");

    mask = IpAddress (0, AF_INET);
    ASSERT_EQ (mask.family (), AF_INET);
    ASSERT_STREQ (mask.toString ().c_str (), "0.0.0.0");

    ASSERT_THROW (IpAddress (136, AF_INET6), std::invalid_argument);
    ASSERT_THROW (IpAddress (-128, AF_INET), std::invalid_argument);

    mask = IpAddress (128, AF_INET6);
    ASSERT_EQ (mask.family (), AF_INET6);
    ASSERT_STREQ (mask.toString ().c_str (), "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff");

    mask = IpAddress (120, AF_INET6);
    ASSERT_EQ (mask.family (), AF_INET6);
    ASSERT_STREQ (mask.toString ().c_str (), "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ff00");

    mask = IpAddress (112, AF_INET6);
    ASSERT_EQ (mask.family (), AF_INET6);
    ASSERT_STREQ (mask.toString ().c_str (), "ffff:ffff:ffff:ffff:ffff:ffff:ffff:0");

    mask = IpAddress (32, AF_INET6);
    ASSERT_EQ (mask.family (), AF_INET6);
    ASSERT_STREQ (mask.toString ().c_str (), "ffff:ffff::");

    mask = IpAddress (8, AF_INET6);
    ASSERT_EQ (mask.family (), AF_INET6);
    ASSERT_STREQ (mask.toString ().c_str (), "ff00::");

    mask = IpAddress (0, AF_INET6);
    ASSERT_EQ (mask.family (), AF_INET6);
    ASSERT_STREQ (mask.toString ().c_str (), "::");
}

/**
 * @brief Test copy assignment method.
 */
TEST (IpAddress, copyAssign)
{
    IpAddress ip4 ("0.0.0.0"), ip;
    ip = ip4;

    ASSERT_EQ (ip.family (), AF_INET);
    ASSERT_NE (ip.addr (), nullptr);
    ASSERT_EQ (ip.length (), sizeof (struct in_addr));
    ASSERT_EQ (ip.scope (), 0);
    ASSERT_TRUE (ip.isWildcard ());
    ASSERT_FALSE (ip.isLoopBack ());
    ASSERT_FALSE (ip.isLinkLocal ());
    ASSERT_FALSE (ip.isSiteLocal ());
    ASSERT_FALSE (ip.isUnicast ());
    ASSERT_FALSE (ip.isBroadcast ());
    ASSERT_FALSE (ip.isMulticast ());
    ASSERT_TRUE (ip.isIpv4Mapped ());
    ASSERT_STREQ (ip.toString ().c_str (), "0.0.0.0");

    IpAddress ip6 ("::");
    ip = ip6;

    ASSERT_EQ (ip.family (), AF_INET6);
    ASSERT_NE (ip.addr (), nullptr);
    ASSERT_EQ (ip.length (), sizeof (struct in6_addr));
    ASSERT_EQ (ip.scope (), 0);
    ASSERT_TRUE (ip.isWildcard ());
    ASSERT_FALSE (ip.isLoopBack ());
    ASSERT_FALSE (ip.isLinkLocal ());
    ASSERT_FALSE (ip.isSiteLocal ());
    ASSERT_FALSE (ip.isUnicast ());
    ASSERT_FALSE (ip.isBroadcast ());
    ASSERT_FALSE (ip.isMulticast ());
    ASSERT_FALSE (ip.isIpv4Mapped ());
    ASSERT_STREQ (ip.toString ().c_str (), "::");
}

/**
 * @brief Test move assignment method.
 */
TEST (IpAddress, moveAssign)
{
    IpAddress ip4 ("0.0.0.0"), ip;

    ip = std::move (ip4);

    ASSERT_EQ (ip.family (), AF_INET);
    ASSERT_NE (ip.addr (), nullptr);
    ASSERT_EQ (ip.length (), sizeof (struct in_addr));
    ASSERT_EQ (ip.scope (), 0);
    ASSERT_TRUE (ip.isWildcard ());
    ASSERT_FALSE (ip.isLoopBack ());
    ASSERT_FALSE (ip.isLinkLocal ());
    ASSERT_FALSE (ip.isSiteLocal ());
    ASSERT_FALSE (ip.isUnicast ());
    ASSERT_FALSE (ip.isBroadcast ());
    ASSERT_FALSE (ip.isMulticast ());
    ASSERT_TRUE (ip.isIpv4Mapped ());
    ASSERT_STREQ (ip.toString ().c_str (), "0.0.0.0");

    IpAddress ip6 ("::");

    ip = std::move (ip6);

    ASSERT_EQ (ip.family (), AF_INET6);
    ASSERT_NE (ip.addr (), nullptr);
    ASSERT_EQ (ip.length (), sizeof (struct in6_addr));
    ASSERT_EQ (ip.scope (), 0);
    ASSERT_TRUE (ip.isWildcard ());
    ASSERT_FALSE (ip.isLoopBack ());
    ASSERT_FALSE (ip.isLinkLocal ());
    ASSERT_FALSE (ip.isSiteLocal ());
    ASSERT_FALSE (ip.isUnicast ());
    ASSERT_FALSE (ip.isBroadcast ());
    ASSERT_FALSE (ip.isMulticast ());
    ASSERT_FALSE (ip.isIpv4Mapped ());
    ASSERT_STREQ (ip.toString ().c_str (), "::");
}

/**
 * @brief Test sockaddr assignment method.
 */
TEST (IpAddress, sockaddrAssign)
{
    IpAddress ip4;
    struct sockaddr_in sa4;
    memset (&sa4, 0, sizeof (struct sockaddr_in));
    sa4.sin_family = AF_INET;

    ip4 = *reinterpret_cast <struct sockaddr*> (&sa4);
    ASSERT_EQ (ip4.family (), AF_INET);
    ASSERT_NE (ip4.addr (), nullptr);
    ASSERT_EQ (ip4.length (), sizeof (struct in_addr));
    ASSERT_EQ (ip4.scope (), 0);
    ASSERT_TRUE (ip4.isWildcard ());
    ASSERT_FALSE (ip4.isLoopBack ());
    ASSERT_FALSE (ip4.isLinkLocal ());
    ASSERT_FALSE (ip4.isSiteLocal ());
    ASSERT_FALSE (ip4.isUnicast ());
    ASSERT_FALSE (ip4.isBroadcast ());
    ASSERT_FALSE (ip4.isMulticast ());
    ASSERT_TRUE (ip4.isIpv4Mapped ());
    ASSERT_STREQ (ip4.toString ().c_str (), "0.0.0.0");

    IpAddress ip6;
    struct sockaddr_in6 sa6;
    memset (&sa6, 0, sizeof (struct sockaddr_in6));
    sa6.sin6_family = AF_INET6;

    ip6 = *reinterpret_cast <struct sockaddr*> (&sa6);
    ASSERT_EQ (ip6.family (), AF_INET6);
    ASSERT_NE (ip6.addr (), nullptr);
    ASSERT_EQ (ip6.length (), sizeof (struct in6_addr));
    ASSERT_EQ (ip6.scope (), 0);
    ASSERT_TRUE (ip6.isWildcard ());
    ASSERT_FALSE (ip6.isLoopBack ());
    ASSERT_FALSE (ip6.isLinkLocal ());
    ASSERT_FALSE (ip6.isSiteLocal ());
    ASSERT_FALSE (ip6.isUnicast ());
    ASSERT_FALSE (ip6.isBroadcast ());
    ASSERT_FALSE (ip6.isMulticast ());
    ASSERT_FALSE (ip6.isIpv4Mapped ());
    ASSERT_STREQ (ip6.toString ().c_str (), "::");
}

/**
 * @brief Test family method.
 */
TEST (IpAddress, family)
{
    IpAddress ip = "0.0.0.0";
    ASSERT_EQ (ip.family (), AF_INET);

    ip = "::ffff:0.0.0.0";
    ASSERT_EQ (ip.family (), AF_INET6);

    ip = "::";
    ASSERT_EQ (ip.family (), AF_INET6);

    ip = "0:0:0:0:0:0:0:0";
    ASSERT_EQ (ip.family (), AF_INET6);

    ip = "0::0";
    ASSERT_EQ (ip.family (), AF_INET6);

    ip = "127.0.0.1";
    ASSERT_EQ (ip.family (), AF_INET);

    ip = "::127.0.0.1";
    ASSERT_EQ (ip.family (), AF_INET6);

    ip = "::ffff:127.0.0.1";
    ASSERT_EQ (ip.family (), AF_INET6);

    ip = "::1";
    ASSERT_EQ (ip.family (), AF_INET6);

    ip = "10.41.45.2";
    ASSERT_EQ (ip.family (), AF_INET);

    ip = "::10.41.45.2";
    ASSERT_EQ (ip.family (), AF_INET6);

    ip = "::ffff:10.41.45.2";
    ASSERT_EQ (ip.family (), AF_INET6);

    ip = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_EQ (ip.family (), AF_INET6);
}

/**
 * @brief Test addr method.
 */
TEST (IpAddress, addr)
{
    IpAddress ip = "0.0.0.0";
    ASSERT_NE (ip.addr (), nullptr);

    ip = "::";
    ASSERT_NE (ip.addr (), nullptr);
}

/**
 * @brief Test length method.
 */
TEST (IpAddress, length)
{
    IpAddress ip = "0.0.0.0";
    ASSERT_EQ (ip.length (), sizeof (struct in_addr));

    ip = "::ffff:0.0.0.0";
    ASSERT_EQ (ip.length (), sizeof (struct in6_addr));

    ip = "::";
    ASSERT_EQ (ip.length (), sizeof (struct in6_addr));

    ip = "0:0:0:0:0:0:0:0";
    ASSERT_EQ (ip.length (), sizeof (struct in6_addr));

    ip = "0::0";
    ASSERT_EQ (ip.length (), sizeof (struct in6_addr));

    ip = "127.0.0.1";
    ASSERT_EQ (ip.length (), sizeof (struct in_addr));

    ip = "::127.0.0.1";
    ASSERT_EQ (ip.length (), sizeof (struct in6_addr));

    ip = "::ffff:127.0.0.1";
    ASSERT_EQ (ip.length (), sizeof (struct in6_addr));

    ip = "::1";
    ASSERT_EQ (ip.length (), sizeof (struct in6_addr));

    ip = "10.41.45.2";
    ASSERT_EQ (ip.length (), sizeof (struct in_addr));

    ip = "::10.41.45.2";
    ASSERT_EQ (ip.length (), sizeof (struct in6_addr));

    ip = "::ffff:10.41.45.2";
    ASSERT_EQ (ip.length (), sizeof (struct in6_addr));

    ip = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_EQ (ip.length (), sizeof (struct in6_addr));
}

/**
 * @brief Test scope method.
 */
TEST (IpAddress, scope)
{
    IpAddress ip = "0.0.0.0";
    ASSERT_EQ (ip.scope (), 0);

    ip = "127.0.0.1";
    ASSERT_EQ (ip.scope (), 0);

    ip = "10.41.45.2";
    ASSERT_EQ (ip.scope (), 0);

    ip = "fe80::1234%1";
    ASSERT_EQ (ip.scope (), 1);

    ip = "ff02::5678%5";
    ASSERT_EQ (ip.scope (), 5);

    ip = "ff08::9abc%10";
    ASSERT_EQ (ip.scope (), 10);

    ip = "fe80::1234%lo";
    ASSERT_EQ (ip.scope (), 1);
}

/**
 * @brief Test prefix method.
 */
TEST (IpAddress, prefix)
{
    IpAddress ip ("0.0.0.0");
    ASSERT_EQ (ip.prefix (), 0);

    ip = "255.0.0.0";
    ASSERT_EQ (ip.prefix (), 8);

    ip = "255.255.0.0";
    ASSERT_EQ (ip.prefix (), 16);

    ip = "255.255.255.0";
    ASSERT_EQ (ip.prefix (), 24);

    ip = "255.255.255.255";
    ASSERT_EQ (ip.prefix (), 32);

    ip = "::";
    ASSERT_EQ (ip.prefix (), 0);

    ip = "ff00::";
    ASSERT_EQ (ip.prefix (), 8);

    ip = "ffff::";
    ASSERT_EQ (ip.prefix (), 16);

    ip = "ffff:ff00::";
    ASSERT_EQ (ip.prefix (), 24);

    ip = "ffff:ffff::";
    ASSERT_EQ (ip.prefix (), 32);

    ip = "ffff:ffff:ff00::";
    ASSERT_EQ (ip.prefix (), 40);

    ip = "ffff:ffff:ffff::";
    ASSERT_EQ (ip.prefix (), 48);

    ip = "ffff:ffff:ffff:ff00::";
    ASSERT_EQ (ip.prefix (), 56);

    ip = "ffff:ffff:ffff:ffff::";
    ASSERT_EQ (ip.prefix (), 64);

    ip = "ffff:ffff:ffff:ffff:ff00::";
    ASSERT_EQ (ip.prefix (), 72);

    ip = "ffff:ffff:ffff:ffff:ffff::";
    ASSERT_EQ (ip.prefix (), 80);

    ip = "ffff:ffff:ffff:ffff:ffff:ff00::";
    ASSERT_EQ (ip.prefix (), 88);

    ip = "ffff:ffff:ffff:ffff:ffff:ffff::";
    ASSERT_EQ (ip.prefix (), 96);

    ip = "ffff:ffff:ffff:ffff:ffff:ffff:ff00::";
    ASSERT_EQ (ip.prefix (), 104);

    ip = "ffff:ffff:ffff:ffff:ffff:ffff:ffff::";
    ASSERT_EQ (ip.prefix (), 112);

    ip = "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ff00";
    ASSERT_EQ (ip.prefix (), 120);


    ip = "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff";
    ASSERT_EQ (ip.prefix (), 128);
}

/**
 * @brief Test isWildcard method.
 */
TEST (IpAddress, isWildcard)
{
    IpAddress ip = "0.0.0.0";
    ASSERT_TRUE (ip.isWildcard ());

    ip = "127.0.0.1";
    ASSERT_FALSE (ip.isWildcard ());

    ip = "10.41.45.2";
    ASSERT_FALSE (ip.isWildcard ());

    ip = "::127.0.0.1";
    ASSERT_FALSE (ip.isWildcard ());

    ip = "::10.41.45.2";
    ASSERT_FALSE (ip.isWildcard ());

    ip = "::ffff:0.0.0.0";
    ASSERT_FALSE (ip.isWildcard ());

    ip = "::ffff:127.0.0.1";
    ASSERT_FALSE (ip.isWildcard ());

    ip = "::ffff:10.41.45.2";
    ASSERT_FALSE (ip.isWildcard ());

    ip = "::";
    ASSERT_TRUE (ip.isWildcard ());

    ip = "0:0:0:0:0:0:0:0";
    ASSERT_TRUE (ip.isWildcard ());

    ip = "0::0";
    ASSERT_TRUE (ip.isWildcard ());

    ip = "::1";
    ASSERT_FALSE (ip.isWildcard ());

    ip = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_FALSE (ip.isWildcard ());

    ip = "fe80::1234%1";
    ASSERT_FALSE (ip.isWildcard ());
}

/**
 * @brief Test isLoopBack method.
 */
TEST (IpAddress, isLoopBack)
{
    IpAddress ip = "0.0.0.0";
    ASSERT_FALSE (ip.isLoopBack ());

    ip = "127.0.0.1";
    ASSERT_TRUE (ip.isLoopBack ());

    ip = "10.41.45.2";
    ASSERT_FALSE (ip.isLoopBack ());

    ip = "::ffff:0.0.0.0";
    ASSERT_FALSE (ip.isLoopBack ());

    ip = "::127.0.0.1";
    ASSERT_FALSE (ip.isLoopBack ());

    ip = "::10.41.45.2";
    ASSERT_FALSE (ip.isLoopBack ());

    ip = "::ffff:127.0.0.1";
    ASSERT_FALSE (ip.isLoopBack ());

    ip = "::ffff:10.41.45.2";
    ASSERT_FALSE (ip.isLoopBack ());

    ip = "::";
    ASSERT_FALSE (ip.isLoopBack ());

    ip = "0:0:0:0:0:0:0:0";
    ASSERT_FALSE (ip.isLoopBack ());

    ip = "0::0";
    ASSERT_FALSE (ip.isLoopBack ());

    ip = "::1";
    ASSERT_TRUE (ip.isLoopBack ());

    ip = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_FALSE (ip.isLoopBack ());

    ip = "fe80::1234%1";
    ASSERT_FALSE (ip.isLoopBack ());
}

/**
 * @brief Test isLinkLocal method.
 */
TEST (IpAddress, isLinkLocal)
{
    IpAddress ip = "169.254.0.1";
    ASSERT_TRUE (ip.isLinkLocal ());

    ip = "127.0.0.1";
    ASSERT_FALSE (ip.isLinkLocal ());

    ip = "192.168.1.51";
    ASSERT_FALSE (ip.isLinkLocal ());

    ip = "10.41.51.18";
    ASSERT_FALSE (ip.isLinkLocal ());

    ip = "::169.254.0.1";
    ASSERT_FALSE (ip.isLinkLocal ());

    ip = "::127.0.0.1";
    ASSERT_FALSE (ip.isLinkLocal ());

    ip = "::192.168.1.51";
    ASSERT_FALSE (ip.isLinkLocal ());

    ip = "::10.41.51.18";
    ASSERT_FALSE (ip.isLinkLocal ());

    ip = "::ffff:169.254.0.1";
    ASSERT_FALSE (ip.isLinkLocal ());

    ip = "::ffff:127.0.0.1";
    ASSERT_FALSE (ip.isLinkLocal ());

    ip = "::ffff:192.168.1.51";
    ASSERT_FALSE (ip.isLinkLocal ());

    ip = "::ffff:10.41.51.18";
    ASSERT_FALSE (ip.isLinkLocal ());

    ip = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_TRUE (ip.isLinkLocal ());

    ip = "::1";
    ASSERT_FALSE (ip.isLinkLocal ());

    ip = "fec0::1234:5678:9ab";
    ASSERT_FALSE (ip.isLinkLocal ());

    ip = "2001:db8:1234:5678::1";
    ASSERT_FALSE (ip.isLinkLocal ());
}

/**
 * @brief Test isSiteLocal method.
 */
TEST (IpAddress, isSiteLocal)
{
    IpAddress ip = "192.168.7.2";
    ASSERT_TRUE (ip.isSiteLocal ());

    ip = "172.16.1.13";
    ASSERT_TRUE (ip.isSiteLocal ());

    ip = "10.41.51.18";
    ASSERT_TRUE (ip.isSiteLocal ());

    ip = "127.0.0.1";
    ASSERT_FALSE (ip.isSiteLocal ());

    ip = "91.121.158.49";
    ASSERT_FALSE (ip.isSiteLocal ());

    ip = "::192.168.7.2";
    ASSERT_FALSE (ip.isSiteLocal ());

    ip = "::172.16.1.13";
    ASSERT_FALSE (ip.isSiteLocal ());

    ip = "::10.41.51.18";
    ASSERT_FALSE (ip.isSiteLocal ());

    ip = "::127.0.0.1";
    ASSERT_FALSE (ip.isSiteLocal ());

    ip = "::91.121.158.49";
    ASSERT_FALSE (ip.isSiteLocal ());

    ip = "::ffff:192.168.7.2";
    ASSERT_FALSE (ip.isSiteLocal ());

    ip = "::ffff:172.16.1.13";
    ASSERT_FALSE (ip.isSiteLocal ());

    ip = "::ffff:10.41.51.18";
    ASSERT_FALSE (ip.isSiteLocal ());

    ip = "::ffff:127.0.0.1";
    ASSERT_FALSE (ip.isSiteLocal ());

    ip = "::ffff:91.121.158.49";
    ASSERT_FALSE (ip.isSiteLocal ());

    ip = "fec0::1234:5678:9ab";
    ASSERT_TRUE (ip.isSiteLocal ());

    ip = "::1";
    ASSERT_FALSE (ip.isSiteLocal ());

    ip = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_FALSE (ip.isSiteLocal ());

    ip = "2001:db8:1234:5678::1";
    ASSERT_FALSE (ip.isSiteLocal ());
}

/**
 * @brief Test isUnicast method.
 */
TEST (IpAddress, isUnicast)
{
    IpAddress ip = "0.0.0.0";
    ASSERT_FALSE (ip.isUnicast ());

    ip = "224.125.3.12";
    ASSERT_FALSE (ip.isUnicast ());

    ip = "255.255.255.255";
    ASSERT_FALSE (ip.isUnicast ());

    ip = "127.0.0.1";
    ASSERT_TRUE (ip.isUnicast ());

    ip = "192.168.7.2";
    ASSERT_TRUE (ip.isUnicast ());

    ip = "10.41.51.18";
    ASSERT_TRUE (ip.isUnicast ());

    ip = "91.121.158.49";
    ASSERT_TRUE (ip.isUnicast ());

    ip = "::";
    ASSERT_FALSE (ip.isUnicast ());

    ip = "0:0:0:0:0:0:0:0";
    ASSERT_FALSE (ip.isUnicast ());

    ip = "0::0";
    ASSERT_FALSE (ip.isUnicast ());

    ip = "ff05::1";
    ASSERT_FALSE (ip.isUnicast ());

    ip = "::1";
    ASSERT_TRUE (ip.isUnicast ());

    ip = "fec0::1234:5678:9ab";
    ASSERT_TRUE (ip.isUnicast ());

    ip = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_TRUE (ip.isUnicast ());

    ip = "2001:db8:1234:5678::1";
    ASSERT_TRUE (ip.isUnicast ());
}

/**
 * @brief Test isBroadcast method.
 */
TEST (IpAddress, isBroadcast)
{
    IpAddress ip = "0.0.0.0";
    ASSERT_FALSE (ip.isBroadcast ());

    ip = "127.0.0.1";
    ASSERT_FALSE (ip.isBroadcast ());

    ip = "192.168.7.2";
    ASSERT_FALSE (ip.isBroadcast ());

    ip = "10.41.51.18";
    ASSERT_FALSE (ip.isBroadcast ());

    ip = "91.121.158.49";
    ASSERT_FALSE (ip.isBroadcast ());

    ip = "224.125.3.12";
    ASSERT_FALSE (ip.isBroadcast ());

    ip = "255.255.255.255";
    ASSERT_TRUE (ip.isBroadcast ());

    ip = "::127.0.0.1";
    ASSERT_FALSE (ip.isBroadcast ());

    ip = "::192.168.7.2";
    ASSERT_FALSE (ip.isBroadcast ());

    ip = "::10.41.51.18";
    ASSERT_FALSE (ip.isBroadcast ());

    ip = "::91.121.158.49";
    ASSERT_FALSE (ip.isBroadcast ());

    ip = "::224.125.3.12";
    ASSERT_FALSE (ip.isBroadcast ());

    ip = "::ffff:0.0.0.0";
    ASSERT_FALSE (ip.isBroadcast ());

    ip = "::ffff:127.0.0.1";
    ASSERT_FALSE (ip.isBroadcast ());

    ip = "::ffff:192.168.7.2";
    ASSERT_FALSE (ip.isBroadcast ());

    ip = "::ffff:10.41.51.18";
    ASSERT_FALSE (ip.isBroadcast ());

    ip = "::ffff:91.121.158.49";
    ASSERT_FALSE (ip.isBroadcast ());

    ip = "::ffff:224.125.3.12";
    ASSERT_FALSE (ip.isBroadcast ());

    ip = "::ffff:255.255.255.255";
    ASSERT_FALSE (ip.isBroadcast ());

    ip = "::";
    ASSERT_FALSE (ip.isBroadcast ());

    ip = "0:0:0:0:0:0:0:0";
    ASSERT_FALSE (ip.isBroadcast ());

    ip = "0::0";
    ASSERT_FALSE (ip.isBroadcast ());

    ip = "ff05::1";
    ASSERT_FALSE (ip.isBroadcast ());

    ip = "::1";
    ASSERT_FALSE (ip.isBroadcast ());

    ip = "fec0::1234:5678:9ab";
    ASSERT_FALSE (ip.isBroadcast ());

    ip = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_FALSE (ip.isBroadcast ());

    ip = "2001:db8:1234:5678::1";
    ASSERT_FALSE (ip.isBroadcast ());
}

/**
 * @brief Test isMulticast method.
 */
TEST (IpAddress, isMulticast)
{
    IpAddress ip = "0.0.0.0";
    ASSERT_FALSE (ip.isMulticast ());

    ip = "127.0.0.1";
    ASSERT_FALSE (ip.isMulticast ());

    ip = "192.168.7.2";
    ASSERT_FALSE (ip.isMulticast ());

    ip = "10.41.51.18";
    ASSERT_FALSE (ip.isMulticast ());

    ip = "91.121.158.49";
    ASSERT_FALSE (ip.isMulticast ());

    ip = "224.125.3.12";
    ASSERT_TRUE (ip.isMulticast ());

    ip = "::127.0.0.1";
    ASSERT_FALSE (ip.isMulticast ());

    ip = "::192.168.7.2";
    ASSERT_FALSE (ip.isMulticast ());

    ip = "::10.41.51.18";
    ASSERT_FALSE (ip.isMulticast ());

    ip = "::91.121.158.49";
    ASSERT_FALSE (ip.isMulticast ());

    ip = "::224.125.3.12";
    ASSERT_FALSE (ip.isMulticast ());

    ip = "::ffff:0.0.0.0";
    ASSERT_FALSE (ip.isMulticast ());

    ip = "::ffff:127.0.0.1";
    ASSERT_FALSE (ip.isMulticast ());

    ip = "::ffff:192.168.7.2";
    ASSERT_FALSE (ip.isMulticast ());

    ip = "::ffff:10.41.51.18";
    ASSERT_FALSE (ip.isMulticast ());

    ip = "::ffff:91.121.158.49";
    ASSERT_FALSE (ip.isMulticast ());

    ip = "::ffff:224.125.3.12";
    ASSERT_FALSE (ip.isMulticast ());

    ip = "::";
    ASSERT_FALSE (ip.isMulticast ());

    ip = "0:0:0:0:0:0:0:0";
    ASSERT_FALSE (ip.isMulticast ());

    ip = "0::0";
    ASSERT_FALSE (ip.isMulticast ());

    ip = "::1";
    ASSERT_FALSE (ip.isMulticast ());

    ip = "fec0::1234:5678:9ab";
    ASSERT_FALSE (ip.isMulticast ());

    ip = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_FALSE (ip.isMulticast ());

    ip = "2001:db8:1234:5678::1";
    ASSERT_FALSE (ip.isMulticast ());

    ip = "ff05::1";
    ASSERT_TRUE (ip.isMulticast ());
}

/**
 * @brief Test isUniqueLocal method.
 */
TEST (IpAddress, isUniqueLocal)
{
    IpAddress ip = "192.168.7.2";
    ASSERT_TRUE (ip.isUniqueLocal ());

    ip = "172.16.1.13";
    ASSERT_TRUE (ip.isUniqueLocal ());

    ip = "10.41.51.18";
    ASSERT_TRUE (ip.isUniqueLocal ());

    ip = "127.0.0.1";
    ASSERT_FALSE (ip.isUniqueLocal ());

    ip = "91.121.158.49";
    ASSERT_FALSE (ip.isUniqueLocal ());

    ip = "::192.168.7.2";
    ASSERT_FALSE (ip.isUniqueLocal ());

    ip = "::172.16.1.13";
    ASSERT_FALSE (ip.isUniqueLocal ());

    ip = "::10.41.51.18";
    ASSERT_FALSE (ip.isUniqueLocal ());

    ip = "::127.0.0.1";
    ASSERT_FALSE (ip.isUniqueLocal ());

    ip = "::91.121.158.49";
    ASSERT_FALSE (ip.isUniqueLocal ());

    ip = "::ffff:192.168.7.2";
    ASSERT_FALSE (ip.isUniqueLocal ());

    ip = "::ffff:172.16.1.13";
    ASSERT_FALSE (ip.isUniqueLocal ());

    ip = "::ffff:10.41.51.18";
    ASSERT_FALSE (ip.isUniqueLocal ());

    ip = "::ffff:127.0.0.1";
    ASSERT_FALSE (ip.isUniqueLocal ());

    ip = "::ffff:91.121.158.49";
    ASSERT_FALSE (ip.isUniqueLocal ());

    ip = "fec0::1234:5678:9ab";
    ASSERT_FALSE (ip.isUniqueLocal ());

    ip = "::1";
    ASSERT_FALSE (ip.isUniqueLocal ());

    ip = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_FALSE (ip.isUniqueLocal ());

    ip = "fd59:e975:e10a::1";
    ASSERT_TRUE (ip.isUniqueLocal ());

    ip = "2001:db8:1234:5678::1";
    ASSERT_FALSE (ip.isUniqueLocal ());
}

/**
 * @brief Test isGlobal method.
 */
TEST (IpAddress, isGlobal)
{
    IpAddress ip = "192.168.7.2";
    ASSERT_FALSE (ip.isGlobal ());

    ip = "172.16.1.13";
    ASSERT_FALSE (ip.isGlobal ());

    ip = "10.41.51.18";
    ASSERT_FALSE (ip.isGlobal ());

    ip = "127.0.0.1";
    ASSERT_FALSE (ip.isGlobal ());

    ip = "91.121.158.49";
    ASSERT_TRUE (ip.isGlobal ());

    ip = "fec0::1234:5678:9ab";
    ASSERT_FALSE (ip.isGlobal ());

    ip = "::1";
    ASSERT_FALSE (ip.isGlobal ());

    ip = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_FALSE (ip.isGlobal ());

    ip = "fd59:e975:e10a::1";
    ASSERT_FALSE (ip.isGlobal ());

    ip = "2001:db8:1234:5678::1";
    ASSERT_TRUE (ip.isGlobal ());
}

/**
 * @brief Test isIpv4Address method.
 */
TEST (IpAddress, isIpv4Address)
{
    IpAddress ip = "0.0.0.0";
    ASSERT_TRUE (ip.isIpv4Address ());

    ip = "127.0.0.1";
    ASSERT_TRUE (ip.isIpv4Address ());

    ip = "192.168.7.2";
    ASSERT_TRUE (ip.isIpv4Address ());

    ip = "10.41.51.18";
    ASSERT_TRUE (ip.isIpv4Address ());

    ip = "91.121.158.49";
    ASSERT_TRUE (ip.isIpv4Address ());

    ip = "224.125.3.12";
    ASSERT_TRUE (ip.isIpv4Address ());

    ip = "255.255.255.255";
    ASSERT_TRUE (ip.isIpv4Address ());

    ip = "::";
    ASSERT_FALSE (ip.isIpv4Address ());

    ip = "0:0:0:0:0:0:0:0";
    ASSERT_FALSE (ip.isIpv4Address ());

    ip = "0::0";
    ASSERT_FALSE (ip.isIpv4Address ());

    ip = "::1";
    ASSERT_FALSE (ip.isIpv4Address ());

    ip = "fec0::1234:5678:9ab";
    ASSERT_FALSE (ip.isIpv4Address ());

    ip = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_FALSE (ip.isIpv4Address ());

    ip = "2001:db8:1234:5678::1";
    ASSERT_FALSE (ip.isIpv4Address ());

    ip = "ff05::1";
    ASSERT_FALSE (ip.isIpv4Address ());

    ip = "::ffff:128.144.52.38";
    ASSERT_FALSE (ip.isIpv4Address ());
}

/**
 * @brief Test isIpv6Address method.
 */
TEST (IpAddress, isIpv6Address)
{
    IpAddress ip = "0.0.0.0";
    ASSERT_FALSE (ip.isIpv6Address ());

    ip = "127.0.0.1";
    ASSERT_FALSE (ip.isIpv6Address ());

    ip = "192.168.7.2";
    ASSERT_FALSE (ip.isIpv6Address ());

    ip = "10.41.51.18";
    ASSERT_FALSE (ip.isIpv6Address ());

    ip = "91.121.158.49";
    ASSERT_FALSE (ip.isIpv6Address ());

    ip = "224.125.3.12";
    ASSERT_FALSE (ip.isIpv6Address ());

    ip = "255.255.255.255";
    ASSERT_FALSE (ip.isIpv6Address ());

    ip = "::";
    ASSERT_TRUE (ip.isIpv6Address ());

    ip = "0:0:0:0:0:0:0:0";
    ASSERT_TRUE (ip.isIpv6Address ());

    ip = "0::0";
    ASSERT_TRUE (ip.isIpv6Address ());

    ip = "::1";
    ASSERT_TRUE (ip.isIpv6Address ());

    ip = "fec0::1234:5678:9ab";
    ASSERT_TRUE (ip.isIpv6Address ());

    ip = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_TRUE (ip.isIpv6Address ());

    ip = "2001:db8:1234:5678::1";
    ASSERT_TRUE (ip.isIpv6Address ());

    ip = "ff05::1";
    ASSERT_TRUE (ip.isIpv6Address ());

    ip = "::ffff:128.144.52.38";
    ASSERT_TRUE (ip.isIpv6Address ());
}

/**
 * @brief Test isIpv4Compat method.
 */
TEST (IpAddress, isIpv4Compat)
{
    IpAddress ip = "0.0.0.0";
    ASSERT_TRUE (ip.isIpv4Compat ());

    ip = "127.0.0.1";
    ASSERT_TRUE (ip.isIpv4Compat ());

    ip = "192.168.7.2";
    ASSERT_TRUE (ip.isIpv4Compat ());

    ip = "10.41.51.18";
    ASSERT_TRUE (ip.isIpv4Compat ());

    ip = "91.121.158.49";
    ASSERT_TRUE (ip.isIpv4Compat ());

    ip = "224.125.3.12";
    ASSERT_TRUE (ip.isIpv4Compat ());

    ip = "255.255.255.255";
    ASSERT_TRUE (ip.isIpv4Compat ());

    ip = "::";
    ASSERT_FALSE (ip.isIpv4Compat ());

    ip = "0:0:0:0:0:0:0:0";
    ASSERT_FALSE (ip.isIpv4Compat ());

    ip = "0::0";
    ASSERT_FALSE (ip.isIpv4Compat ());

    ip = "::1";
    ASSERT_FALSE (ip.isIpv4Compat ());

    ip = "fec0::1234:5678:9ab";
    ASSERT_FALSE (ip.isIpv4Compat ());

    ip = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_FALSE (ip.isIpv4Compat ());

    ip = "2001:db8:1234:5678::1";
    ASSERT_FALSE (ip.isIpv4Compat ());

    ip = "ff05::1";
    ASSERT_FALSE (ip.isIpv4Compat ());

    ip = "::ffff:128.144.52.38";
    ASSERT_FALSE (ip.isIpv4Compat ());

    ip = "::128.144.52.38";
    ASSERT_TRUE (ip.isIpv4Compat ());
}

/**
 * @brief Test isIpv4Mapped method.
 */
TEST (IpAddress, isIpv4Mapped)
{
    IpAddress ip = "0.0.0.0";
    ASSERT_TRUE (ip.isIpv4Mapped ());

    ip = "127.0.0.1";
    ASSERT_TRUE (ip.isIpv4Mapped ());

    ip = "192.168.7.2";
    ASSERT_TRUE (ip.isIpv4Mapped ());

    ip = "10.41.51.18";
    ASSERT_TRUE (ip.isIpv4Mapped ());

    ip = "91.121.158.49";
    ASSERT_TRUE (ip.isIpv4Mapped ());

    ip = "224.125.3.12";
    ASSERT_TRUE (ip.isIpv4Mapped ());

    ip = "255.255.255.255";
    ASSERT_TRUE (ip.isIpv4Mapped ());

    ip = "::";
    ASSERT_FALSE (ip.isIpv4Mapped ());

    ip = "0:0:0:0:0:0:0:0";
    ASSERT_FALSE (ip.isIpv4Mapped ());

    ip = "0::0";
    ASSERT_FALSE (ip.isIpv4Mapped ());

    ip = "::1";
    ASSERT_FALSE (ip.isIpv4Mapped ());

    ip = "fec0::1234:5678:9ab";
    ASSERT_FALSE (ip.isIpv4Mapped ());

    ip = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_FALSE (ip.isIpv4Mapped ());

    ip = "2001:db8:1234:5678::1";
    ASSERT_FALSE (ip.isIpv4Mapped ());

    ip = "ff05::1";
    ASSERT_FALSE (ip.isIpv4Mapped ());

    ip = "::128.144.52.38";
    ASSERT_FALSE (ip.isIpv4Mapped ());

    ip = "::ffff:128.144.52.38";
    ASSERT_TRUE (ip.isIpv4Mapped ());
}

/**
 * @brief Test isIpAddress method.
 */
TEST (IpAddress, isIpAddress)
{
    ASSERT_TRUE (IpAddress::isIpAddress ("0.0.0.0"));
    ASSERT_TRUE (IpAddress::isIpAddress ("127.0.0.1"));
    ASSERT_TRUE (IpAddress::isIpAddress ("10.41.51.18"));

    ASSERT_TRUE (IpAddress::isIpAddress ("::"));
    ASSERT_TRUE (IpAddress::isIpAddress ("::1"));
    ASSERT_TRUE (IpAddress::isIpAddress ("2001:db8:1234:5678::1"));

    ASSERT_FALSE (IpAddress::isIpAddress ("foo.bar"));
    ASSERT_FALSE (IpAddress::isIpAddress ("192.bar"));
}

/**
 * @brief Test toIpv4 method.
 */
TEST (IpAddress, toIpv4)
{
    IpAddress ip = "0.0.0.0";
    IpAddress ipv4 = ip.toIpv4 ();
    ASSERT_STREQ (ipv4.toString ().c_str (), "0.0.0.0");

    ip = "127.0.0.1";
    ipv4 = ip.toIpv4 ();
    ASSERT_STREQ (ipv4.toString ().c_str (), "127.0.0.1");

    ip = "10.41.45.2";
    ipv4 = ip.toIpv4 ();
    ASSERT_STREQ (ipv4.toString ().c_str (), "10.41.45.2");

    ip = "::127.0.0.1";
    ipv4 = ip.toIpv4 ();
    ASSERT_STREQ (ipv4.toString ().c_str (), "127.0.0.1");

    ip = "::10.41.45.2";
    ipv4 = ip.toIpv4 ();
    ASSERT_STREQ (ipv4.toString ().c_str (), "10.41.45.2");

    ip = "::ffff:0.0.0.0";
    ipv4 = ip.toIpv4 ();
    ASSERT_STREQ (ipv4.toString ().c_str (), "0.0.0.0");

    ip = "::ffff:127.0.0.1";
    ipv4 = ip.toIpv4 ();
    ASSERT_STREQ (ipv4.toString ().c_str (), "127.0.0.1");

    ip = "::ffff:10.41.45.2";
    ipv4 = ip.toIpv4 ();
    ASSERT_STREQ (ipv4.toString ().c_str (), "10.41.45.2");

    ip = "::";
    ipv4 = ip.toIpv4 ();
    ASSERT_STREQ (ipv4.toString ().c_str (), "::");

    ip = "0:0:0:0:0:0:0:0";
    ipv4 = ip.toIpv4 ();
    ASSERT_STREQ (ipv4.toString ().c_str (), "::");

    ip = "0::0";
    ipv4 = ip.toIpv4 ();
    ASSERT_STREQ (ipv4.toString ().c_str (), "::");

    ip = "::1";
    ipv4 = ip.toIpv4 ();
    ASSERT_STREQ (ipv4.toString ().c_str (), "::1");

    ip = "fe80::57f3:baa4:fc3a:890a";
    ipv4 = ip.toIpv4 ();
    ASSERT_STREQ (ipv4.toString ().c_str (), "fe80::57f3:baa4:fc3a:890a");
}

/**
 * @brief Test toIpv6 method.
 */
TEST (IpAddress, toIpv6)
{
    IpAddress ip   = "0.0.0.0";
    IpAddress ipv6 = ip.toIpv6 ();
    ASSERT_STREQ (ipv6.toString ().c_str (), "::ffff:0.0.0.0");

    ip   = "127.0.0.1";
    ipv6 = ip.toIpv6 ();
    ASSERT_STREQ (ipv6.toString ().c_str (), "::ffff:127.0.0.1");

    ip   = "10.41.45.2";
    ipv6 = ip.toIpv6 ();
    ASSERT_STREQ (ipv6.toString ().c_str (), "::ffff:10.41.45.2");

    ip   = "::";
    ipv6 = ip.toIpv6 ();
    ASSERT_STREQ (ipv6.toString ().c_str (), "::");

    ip   = "0:0:0:0:0:0:0:0";
    ipv6 = ip.toIpv6 ();
    ASSERT_STREQ (ipv6.toString ().c_str (), "::");

    ip   = "0::0";
    ipv6 = ip.toIpv6 ();
    ASSERT_STREQ (ipv6.toString ().c_str (), "::");

    ip   = "::1";
    ipv6 = ip.toIpv6 ();
    ASSERT_STREQ (ipv6.toString ().c_str (), "::1");

    ip   = "fe80::57f3:baa4:fc3a:890a";
    ipv6 = ip.toIpv6 ();
    ASSERT_STREQ (ipv6.toString ().c_str (), "fe80::57f3:baa4:fc3a:890a");
}

/**
 * @brief Test toString method.
 */
TEST (IpAddress, toString)
{
    IpAddress ip = "0.0.0.0";
    ASSERT_STREQ (ip.toString ().c_str (), "0.0.0.0");

    ip = "127.0.0.1";
    ASSERT_STREQ (ip.toString ().c_str (), "127.0.0.1");

    ip = "10.41.45.2";
    ASSERT_STREQ (ip.toString ().c_str (), "10.41.45.2");

    ip = "::";
    ASSERT_STREQ (ip.toString ().c_str (), "::");

    ip   = "0:0:0:0:0:0:0:0";
    ASSERT_STREQ (ip.toString ().c_str (), "::");

    ip   = "0::0";
    ASSERT_STREQ (ip.toString ().c_str (), "::");

    ip = "::1";
    ASSERT_STREQ (ip.toString ().c_str (), "::1");

    ip = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_STREQ (ip.toString ().c_str (), "fe80::57f3:baa4:fc3a:890a");

    ip = "fe80::57f3:baa4:fc3a:890a%lo";
    ASSERT_STREQ (ip.toString ().c_str (), "fe80::57f3:baa4:fc3a:890a%lo");

    ip = "fe80::57f3:baa4:fc3a:890a%8";
    ASSERT_STREQ (ip.toString ().c_str (), "fe80::57f3:baa4:fc3a:890a%8");
}

/**
 * @brief Test toArpa method.
 */
TEST (IpAddress, toArpa)
{
    IpAddress addr ("127.0.0.1");
    ASSERT_STREQ (addr.toArpa ().c_str (), "1.0.0.127.in-addr.arpa");

    addr = "10.41.45.2";
    ASSERT_STREQ (addr.toArpa ().c_str (), "2.45.41.10.in-addr.arpa");

    addr = "::1";
    ASSERT_STREQ (addr.toArpa ().c_str (), "1.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.ip6.arpa");

    addr = "2001:db8::567:89ab";
    ASSERT_STREQ (addr.toArpa ().c_str (), "b.a.9.8.7.6.5.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.8.b.d.0.1.0.0.2.ip6.arpa");
}

/**
 * @brief Test clear method.
 */
TEST (IpAddress, clear)
{
    IpAddress ip = "127.0.0.1";
    ASSERT_FALSE (ip.isWildcard());
    ip.clear ();
    ASSERT_TRUE (ip.isWildcard());

    ip = "192.168.7.2";
    ASSERT_FALSE (ip.isWildcard());
    ip.clear ();
    ASSERT_TRUE (ip.isWildcard());

    ip = "10.41.51.18";
    ASSERT_FALSE (ip.isWildcard());
    ip.clear ();
    ASSERT_TRUE (ip.isWildcard());

    ip = "91.121.158.49";
    ASSERT_FALSE (ip.isWildcard());
    ip.clear ();
    ASSERT_TRUE (ip.isWildcard());

    ip = "224.125.3.12";
    ASSERT_FALSE (ip.isWildcard());
    ip.clear ();
    ASSERT_TRUE (ip.isWildcard());

    ip = "255.255.255.255";
    ASSERT_FALSE (ip.isWildcard());
    ip.clear ();
    ASSERT_TRUE (ip.isWildcard());

    ip = "::1";
    ASSERT_FALSE (ip.isWildcard());
    ip.clear ();
    ASSERT_TRUE (ip.isWildcard());

    ip = "fec0::1234:5678:9ab";
    ASSERT_FALSE (ip.isWildcard());
    ip.clear ();
    ASSERT_TRUE (ip.isWildcard());

    ip = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_FALSE (ip.isWildcard());
    ip.clear ();
    ASSERT_TRUE (ip.isWildcard());

    ip = "2001:db8:1234:5678::1";
    ASSERT_FALSE (ip.isWildcard());
    ip.clear ();
    ASSERT_TRUE (ip.isWildcard());

    ip = "ff05::1";
    ASSERT_FALSE (ip.isWildcard());
    ip.clear ();
    ASSERT_TRUE (ip.isWildcard());

    ip = "::128.144.52.38";
    ASSERT_FALSE (ip.isWildcard());
    ip.clear ();
    ASSERT_TRUE (ip.isWildcard());

    ip = "::ffff:128.144.52.38";
    ASSERT_FALSE (ip.isWildcard());
    ip.clear ();
    ASSERT_TRUE (ip.isWildcard());
}

/**
 * @brief Test at method.
 */
TEST (IpAddress, at)
{
    IpAddress ip4 (AF_INET);
    ip4[0] = 10;
    ip4[1] = 41;
    ip4[2] = 45;
    ip4[3] = 2;

    ASSERT_EQ (ip4[0], 10);
    ASSERT_EQ (ip4[1], 41);
    ASSERT_EQ (ip4[2], 45);
    ASSERT_EQ (ip4[3], 2);
    ASSERT_THROW (ip4[4], std::out_of_range);

    ASSERT_EQ (((const IpAddress*)&ip4)->operator[] (0), 10);
    ASSERT_EQ (((const IpAddress*)&ip4)->operator[] (1), 41);
    ASSERT_EQ (((const IpAddress*)&ip4)->operator[] (2), 45);
    ASSERT_EQ (((const IpAddress*)&ip4)->operator[] (3), 2);
    ASSERT_THROW (((const IpAddress*)&ip4)->operator[] (4), std::out_of_range);

    IpAddress ip6 (AF_INET6);
    ip6[0] = 0xfe;
    ip6[1] = 0x80;
    ip6[8] = 0x57;
    ip6[9] = 0xf3;
    ip6[10] = 0xba;
    ip6[11] = 0xa4;
    ip6[12] = 0xfc;
    ip6[13] = 0x3a;
    ip6[14] = 0x89;
    ip6[15] = 0x0a;

    ASSERT_EQ (ip6[0], 0xfe);
    ASSERT_EQ (ip6[1], 0x80);
    ASSERT_EQ (ip6[8], 0x57);
    ASSERT_EQ (ip6[9], 0xf3);
    ASSERT_EQ (ip6[10], 0xba);
    ASSERT_EQ (ip6[11], 0xa4);
    ASSERT_EQ (ip6[12], 0xfc);
    ASSERT_EQ (ip6[13], 0x3a);
    ASSERT_EQ (ip6[14], 0x89);
    ASSERT_EQ (ip6[15], 0x0a);
    ASSERT_THROW (ip6[16], std::out_of_range);

    ASSERT_EQ (((const IpAddress*)&ip6)->operator[] (0), 0xfe);
    ASSERT_EQ (((const IpAddress*)&ip6)->operator[] (1), 0x80);
    ASSERT_EQ (((const IpAddress*)&ip6)->operator[] (8), 0x57);
    ASSERT_EQ (((const IpAddress*)&ip6)->operator[] (9), 0xf3);
    ASSERT_EQ (((const IpAddress*)&ip6)->operator[] (10), 0xba);
    ASSERT_EQ (((const IpAddress*)&ip6)->operator[] (11), 0xa4);
    ASSERT_EQ (((const IpAddress*)&ip6)->operator[] (12), 0xfc);
    ASSERT_EQ (((const IpAddress*)&ip6)->operator[] (13), 0x3a);
    ASSERT_EQ (((const IpAddress*)&ip6)->operator[] (14), 0x89);
    ASSERT_EQ (((const IpAddress*)&ip6)->operator[] (15), 0x0a);
    ASSERT_THROW (((const IpAddress*)&ip6)->operator[] (16), std::out_of_range);
}

/**
 * @brief Test is equal method.
 */
TEST (IpAddress, equal)
{
    IpAddress ip1, ip2;

    ip1 = "192.168.13.31";
    ip2 = "192.168.13.31";
    ASSERT_TRUE (ip1 == ip2);

    ip1 = "192.168.13.31";
    ip2 = "10.41.45.2";
    ASSERT_FALSE (ip1 == ip2);

    ip1 = "10.41.45.2";
    ip2 = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_FALSE (ip1 == ip2);

    ip1 = "192.168.13.31";
    ASSERT_TRUE (ip1 == "192.168.13.31");

    ip1 = "192.168.13.31";
    ASSERT_FALSE (ip1 == "10.41.45.2");

    ip1 = "10.41.45.2";
    ASSERT_FALSE (ip1 == "fe80::57f3:baa4:fc3a:890a");

    ip2 = "192.168.13.31";
    ASSERT_TRUE ("192.168.13.31" == ip2);

    ip2 = "10.41.45.2";
    ASSERT_FALSE ("192.168.13.31" == ip2);

    ip2 = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_FALSE ("10.41.45.2" == ip2);

    ip1 = "2001:db8:1234:5678::1";
    ip2 = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_FALSE (ip1 == ip2);

    ip1 = "2001:db8:1234:5678::1";
    ip2 = "2001:db8:1234:5678::1";
    ASSERT_TRUE (ip1 == ip2);

    ip1 = "2001:db8:1234:5678::1";
    ASSERT_FALSE (ip1 == "fe80::57f3:baa4:fc3a:890a");

    ip1 = "2001:db8:1234:5678::1";
    ASSERT_TRUE (ip1 == "2001:db8:1234:5678::1");

    ip2 = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_FALSE ("2001:db8:1234:5678::1" == ip2);

    ip2 = "2001:db8:1234:5678::1";
    ASSERT_TRUE ("2001:db8:1234:5678::1" == ip2);

    ip1 = "fe80::57f3:baa4:fc3a:890a%lo";
    ASSERT_TRUE (ip1 == "fe80::57f3:baa4:fc3a:890a%lo");

    ip2 = "fe80::57f3:baa4:fc3a:890a%eth0";
    ASSERT_FALSE (ip1 == ip2);
}

/**
 * @brief Test is different method.
 */
TEST (IpAddress, different)
{
    IpAddress ip1, ip2;

    ip1 = "192.168.13.31";
    ip2 = "192.168.13.31";
    ASSERT_FALSE (ip1 != ip2);

    ip1 = "192.168.13.31";
    ip2 = "10.41.45.2";
    ASSERT_TRUE (ip1 != ip2);

    ip1 = "10.41.45.2";
    ip2 = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_TRUE (ip1 != ip2);

    ip1 = "192.168.13.31";
    ASSERT_FALSE (ip1 != "192.168.13.31");

    ip1 = "192.168.13.31";
    ASSERT_TRUE (ip1 != "10.41.45.2");

    ip1 = "10.41.45.2";
    ASSERT_TRUE (ip1 != "fe80::57f3:baa4:fc3a:890a");

    ip2 = "192.168.13.31";
    ASSERT_FALSE ("192.168.13.31" != ip2);

    ip2 = "10.41.45.2";
    ASSERT_TRUE ("192.168.13.31" != ip2);

    ip2 = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_TRUE ("10.41.45.2" != ip2);

    ip1 = "2001:db8:1234:5678::1";
    ip2 = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_TRUE (ip1 != ip2);

    ip1 = "2001:db8:1234:5678::1";
    ip2 = "2001:db8:1234:5678::1";
    ASSERT_FALSE (ip1 != ip2);

    ip1 = "2001:db8:1234:5678::1";
    ASSERT_TRUE (ip1 != "fe80::57f3:baa4:fc3a:890a");

    ip1 = "2001:db8:1234:5678::1";
    ASSERT_FALSE (ip1 != "2001:db8:1234:5678::1");

    ip2 = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_TRUE ("2001:db8:1234:5678::1" != ip2);

    ip2 = "2001:db8:1234:5678::1";
    ASSERT_FALSE ("2001:db8:1234:5678::1" != ip2);

    ip1 = "fe80::57f3:baa4:fc3a:890a%lo";
    ASSERT_FALSE (ip1 != "fe80::57f3:baa4:fc3a:890a%lo");

    ip2 = "fe80::57f3:baa4:fc3a:890a%eth0";
    ASSERT_TRUE (ip1 != ip2);
}

/**
 * @brief Test is lower method.
 */
TEST (IpAddress, lower)
{
    IpAddress ip1, ip2;

    ip1 = "192.168.13.31";
    ip2 = "192.168.13.31";
    ASSERT_FALSE (ip1 < ip2);

    ip1 = "192.168.13.31";
    ip2 = "192.168.13.32";
    ASSERT_TRUE (ip1 < ip2);

    ip1 = "192.168.13.31";
    ip2 = "10.41.45.2";
    ASSERT_FALSE (ip1 < ip2);

    ip1 = "192.168.13.31";
    ASSERT_FALSE (ip1 < "192.168.13.31");

    ip1 = "192.168.13.31";
    ASSERT_TRUE (ip1 < "192.168.13.32");

    ip1 = "192.168.13.31";
    ASSERT_FALSE (ip1 < "10.41.45.2");

    ip2 = "192.168.13.31";
    ASSERT_FALSE ("192.168.13.31" < ip2);

    ip2 = "192.168.13.32";
    ASSERT_TRUE ("192.168.13.31" < ip2);

    ip2 = "10.41.45.2";
    ASSERT_FALSE ("192.168.13.31" < ip2);

    ip1 = "10.41.45.2";
    ip2 = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_TRUE (ip1 < ip2);

    ip1 = "fe80::57f3:baa4:fc3a:890a";
    ip2 = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_FALSE (ip1 < ip2);

    ip1 = "2001:db8:1234:5678::1";
    ip2 = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_TRUE (ip1 < ip2);

    ip1 = "2001:db8:1234:5678::1";
    ip2 = "2001:db8:1234:5678::1";
    ASSERT_FALSE (ip1 < ip2);

    ip1 = "10.41.45.2";
    ASSERT_TRUE (ip1 < "fe80::57f3:baa4:fc3a:890a");

    ip1 = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_FALSE (ip1 < "fe80::57f3:baa4:fc3a:890a");

    ip1 = "2001:db8:1234:5678::1";
    ASSERT_TRUE (ip1 < "fe80::57f3:baa4:fc3a:890a");

    ip1 = "2001:db8:1234:5678::1";
    ASSERT_FALSE (ip1 < "2001:db8:1234:5678::1");

    ip2 = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_TRUE ("10.41.45.2" < ip2);

    ip2 = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_FALSE ("fe80::57f3:baa4:fc3a:890a" < ip2);

    ip2 = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_TRUE ("2001:db8:1234:5678::1" < ip2);

    ip2 = "2001:db8:1234:5678::1";
    ASSERT_FALSE ("2001:db8:1234:5678::1" < ip2);

    ip1 = "fe80::57f3:baa4:fc3a:890a%lo";
    ASSERT_FALSE (ip1 < "fe80::57f3:baa4:fc3a:890a%lo");

    ip2 = "fe80::57f3:baa4:fc3a:890a%eth0";
    ASSERT_TRUE (ip1 < ip2);
}

/**
 * @brief Test is lower or equal method.
 */
TEST (IpAddress, lowerOrEqual)
{
    IpAddress ip1, ip2;

    ip1 = "192.168.13.31";
    ip2 = "192.168.13.31";
    ASSERT_TRUE (ip1 <= ip2);

    ip1 = "192.168.13.31";
    ip2 = "192.168.13.32";
    ASSERT_TRUE (ip1 <= ip2);

    ip1 = "192.168.13.31";
    ip2 = "10.41.45.2";
    ASSERT_FALSE (ip1 <= ip2);

    ip1 = "192.168.13.31";
    ASSERT_TRUE (ip1 <= "192.168.13.31");

    ip1 = "192.168.13.31";
    ASSERT_TRUE (ip1 <= "192.168.13.32");

    ip1 = "192.168.13.31";
    ASSERT_FALSE (ip1 <= "10.41.45.2");

    ip2 = "192.168.13.31";
    ASSERT_TRUE ("192.168.13.31" <= ip2);

    ip2 = "192.168.13.32";
    ASSERT_TRUE ("192.168.13.31" <= ip2);

    ip2 = "10.41.45.2";
    ASSERT_FALSE ("192.168.13.31" <= ip2);

    ip1 = "10.41.45.2";
    ip2 = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_TRUE (ip1 <= ip2);

    ip1 = "fe80::57f3:baa4:fc3a:890a";
    ip2 = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_TRUE (ip1 <= ip2);

    ip1 = "2001:db8:1234:5678::1";
    ip2 = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_TRUE (ip1 <= ip2);

    ip1 = "2001:db8:1234:5678::1";
    ip2 = "2001:db8:1234:5678::1";
    ASSERT_TRUE (ip1 <= ip2);

    ip1 = "10.41.45.2";
    ASSERT_TRUE (ip1 <= "fe80::57f3:baa4:fc3a:890a");

    ip1 = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_TRUE (ip1 <= "fe80::57f3:baa4:fc3a:890a");

    ip1 = "2001:db8:1234:5678::1";
    ASSERT_TRUE (ip1 <= "fe80::57f3:baa4:fc3a:890a");

    ip1 = "2001:db8:1234:5678::1";
    ip2 = "2001:db8:1234:5678::1";
    ASSERT_TRUE (ip1 <= "2001:db8:1234:5678::1");

    ip2 = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_TRUE ("10.41.45.2" <= ip2);

    ip2 = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_TRUE ("fe80::57f3:baa4:fc3a:890a" <= ip2);

    ip2 = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_TRUE ("2001:db8:1234:5678::1" <= ip2);

    ip2 = "2001:db8:1234:5678::1";
    ASSERT_TRUE ("2001:db8:1234:5678::1" <= ip2);

    ip1 = "fe80::57f3:baa4:fc3a:890a%lo";
    ASSERT_TRUE (ip1 <= "fe80::57f3:baa4:fc3a:890a%lo");

    ip2 = "fe80::57f3:baa4:fc3a:890a%eth0";
    ASSERT_TRUE (ip1 <= ip2);
}

/**
 * @brief Test is greater method.
 */
TEST (IpAddress, greater)
{
    IpAddress ip1, ip2;

    ip1 = "192.168.13.31";
    ip2 = "192.168.13.31";
    ASSERT_FALSE (ip1 > ip2);

    ip1 = "192.168.13.31";
    ip2 = "192.168.13.32";
    ASSERT_FALSE (ip1 > ip2);

    ip1 = "192.168.13.31";
    ip2 = "10.41.45.2";
    ASSERT_TRUE (ip1 > ip2);

    ip1 = "192.168.13.31";
    ASSERT_FALSE (ip1 > "192.168.13.31");

    ip1 = "192.168.13.31";
    ASSERT_FALSE (ip1 > "192.168.13.32");

    ip1 = "192.168.13.31";
    ASSERT_TRUE (ip1 > "10.41.45.2");

    ip2 = "192.168.13.31";
    ASSERT_FALSE ("192.168.13.31" > ip2);

    ip2 = "192.168.13.32";
    ASSERT_FALSE ("192.168.13.31" > ip2);

    ip2 = "10.41.45.2";
    ASSERT_TRUE ("192.168.13.31" > ip2);

    ip1 = "10.41.45.2";
    ip2 = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_FALSE (ip1 > ip2);

    ip1 = "fe80::57f3:baa4:fc3a:890a";
    ip2 = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_FALSE (ip1 > ip2);

    ip1 = "2001:db8:1234:5678::1";
    ip2 = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_FALSE (ip1 > ip2);

    ip1 = "2001:db8:1234:5678::1";
    ip2 = "2001:db8:1234:5678::1";
    ASSERT_FALSE (ip1 > ip2);

    ip1 = "10.41.45.2";
    ASSERT_FALSE (ip1 > "fe80::57f3:baa4:fc3a:890a");

    ip1 = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_FALSE (ip1 > "fe80::57f3:baa4:fc3a:890a");

    ip1 = "2001:db8:1234:5678::1";
    ASSERT_FALSE (ip1 > "fe80::57f3:baa4:fc3a:890a");

    ip1 = "2001:db8:1234:5678::1";
    ASSERT_FALSE (ip1 > "2001:db8:1234:5678::1");

    ip2 = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_FALSE ("10.41.45.2" > ip2);

    ip2 = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_FALSE ("fe80::57f3:baa4:fc3a:890a" > ip2);

    ip2 = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_FALSE ("2001:db8:1234:5678::1" > ip2);

    ip2 = "2001:db8:1234:5678::1";
    ASSERT_FALSE ("2001:db8:1234:5678::1" > ip2);

    ip1 = "fe80::57f3:baa4:fc3a:890a%lo";
    ASSERT_FALSE (ip1 > "fe80::57f3:baa4:fc3a:890a%lo");

    ip2 = "fe80::57f3:baa4:fc3a:890a%eth0";
    ASSERT_FALSE (ip1 > ip2);
}

/**
 * @brief Test is greater or equal method.
 */
TEST (IpAddress, greaterOrEqual)
{
    IpAddress ip1, ip2;

    ip1 = "192.168.13.31";
    ip2 = "192.168.13.31";
    ASSERT_TRUE (ip1 >= ip2);

    ip1 = "192.168.13.31";
    ip2 = "192.168.13.32";
    ASSERT_FALSE (ip1 >= ip2);

    ip1 = "192.168.13.31";
    ip2 = "10.41.45.2";
    ASSERT_TRUE (ip1 >= ip2);

    ip1 = "192.168.13.31";
    ASSERT_TRUE (ip1 >= "192.168.13.31");

    ip1 = "192.168.13.31";
    ASSERT_FALSE (ip1 >= "192.168.13.32");

    ip1 = "192.168.13.31";
    ASSERT_TRUE (ip1 >= "10.41.45.2");

    ip2 = "192.168.13.31";
    ASSERT_TRUE ("192.168.13.31" >= ip2);

    ip2 = "192.168.13.32";
    ASSERT_FALSE ("192.168.13.31" >= ip2);

    ip2 = "10.41.45.2";
    ASSERT_TRUE ("192.168.13.31" >= ip2);

    ip1 = "10.41.45.2";
    ip2 = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_FALSE (ip1 >= ip2);

    ip1 = "fe80::57f3:baa4:fc3a:890a";
    ip2 = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_TRUE (ip1 >= ip2);

    ip1 = "2001:db8:1234:5678::1";
    ip2 = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_FALSE (ip1 >= ip2);

    ip1 = "2001:db8:1234:5678::1";
    ip2 = "2001:db8:1234:5678::1";
    ASSERT_TRUE (ip1 >= ip2);

    ip1 = "10.41.45.2";
    ASSERT_FALSE (ip1 >= "fe80::57f3:baa4:fc3a:890a");

    ip1 = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_TRUE (ip1 >= "fe80::57f3:baa4:fc3a:890a");

    ip1 = "2001:db8:1234:5678::1";
    ASSERT_FALSE (ip1 >= "fe80::57f3:baa4:fc3a:890a");

    ip1 = "2001:db8:1234:5678::1";
    ASSERT_TRUE (ip1 >= "2001:db8:1234:5678::1");

    ip2 = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_FALSE ("10.41.45.2" >= ip2);

    ip2 = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_TRUE ("fe80::57f3:baa4:fc3a:890a" >= ip2);

    ip2 = "fe80::57f3:baa4:fc3a:890a";
    ASSERT_FALSE ("2001:db8:1234:5678::1" >= ip2);

    ip2 = "2001:db8:1234:5678::1";
    ASSERT_TRUE ("2001:db8:1234:5678::1" >= ip2);

    ip1 = "fe80::57f3:baa4:fc3a:890a%lo";
    ASSERT_TRUE (ip1 >= "fe80::57f3:baa4:fc3a:890a%lo");

    ip2 = "fe80::57f3:baa4:fc3a:890a%eth0";
    ASSERT_FALSE (ip1 >= ip2);
}

/**
 * @brief and operators.
 */
TEST (IpAddress, and)
{
    IpAddress ip1, ip2, result;

    ip1 = "192.168.13.31";
    ip2 = IpAddress (24, AF_INET);
    result = ip1 & ip2;
    ASSERT_STREQ (result.toString ().c_str (), "192.168.13.0");

    result = ip1 & "255.255.255.0";
    ASSERT_STREQ (result.toString ().c_str (), "192.168.13.0");

    result = "255.255.255.0" & ip1;
    ASSERT_STREQ (result.toString ().c_str (), "192.168.13.0");

    ip1 = "2001:db8:abcd:12::1";
    ip2 = IpAddress (64, AF_INET6);
    result = ip1 & ip2;
    ASSERT_STREQ (result.toString ().c_str (), "2001:db8:abcd:12::");

    result = ip1 & "ffff:ffff:ffff:ffff::";
    ASSERT_STREQ (result.toString ().c_str (), "2001:db8:abcd:12::");

    result = "ffff:ffff:ffff:ffff::" & ip1;
    ASSERT_STREQ (result.toString ().c_str (), "2001:db8:abcd:12::");

    ip1 = "192.168.13.31";
    ip2 = "2001:db8:abcd:12::1";
    ASSERT_THROW (ip1 & ip2, std::invalid_argument);
}

/**
 * @brief or operators.
 */
TEST (IpAddress, or)
{
    IpAddress ip1, ip2, result;

    ip1 = "192.168.13.31";
    ip2 = IpAddress (24, AF_INET);
    result = ip1 | ip2;
    ASSERT_STREQ (result.toString ().c_str (), "255.255.255.31");

    result = ip1 | "255.255.255.0";
    ASSERT_STREQ (result.toString ().c_str (), "255.255.255.31");

    result = "255.255.255.0" | ip1;
    ASSERT_STREQ (result.toString ().c_str (), "255.255.255.31");

    ip1 = "2001:db8:abcd:12::1";
    ip2 = IpAddress (64, AF_INET6);
    result = ip1 | ip2;
    ASSERT_STREQ (result.toString ().c_str (), "ffff:ffff:ffff:ffff::1");

    result = ip1 | "ffff:ffff:ffff:ffff::";
    ASSERT_STREQ (result.toString ().c_str (), "ffff:ffff:ffff:ffff::1");

    result = "ffff:ffff:ffff:ffff::" | ip1;
    ASSERT_STREQ (result.toString ().c_str (), "ffff:ffff:ffff:ffff::1");

    ip1 = "192.168.13.31";
    ip2 = "2001:db8:abcd:12::1";
    ASSERT_THROW (ip1 | ip2, std::invalid_argument);
}

/**
 * @brief xor operators.
 */
TEST (IpAddress, xor)
{
    IpAddress ip1, ip2, result;

    ip1 = "192.168.13.31";
    ip2 = IpAddress (24, AF_INET);
    result = ip1 ^ ip2;
    ASSERT_STREQ (result.toString ().c_str (), "63.87.242.31");

    result = ip1 ^ "255.255.255.0";
    ASSERT_STREQ (result.toString ().c_str (), "63.87.242.31");

    result = "255.255.255.0" ^ ip1;
    ASSERT_STREQ (result.toString ().c_str (), "63.87.242.31");

    ip1 = "2001:db8:abcd:12::1";
    ip2 = IpAddress (64, AF_INET6);
    result = ip1 ^ ip2;
    ASSERT_STREQ (result.toString ().c_str (), "dffe:f247:5432:ffed::1");

    result = ip1 ^ "ffff:ffff:ffff:ffff::";
    ASSERT_STREQ (result.toString ().c_str (), "dffe:f247:5432:ffed::1");

    result = "ffff:ffff:ffff:ffff::" ^ ip1;
    ASSERT_STREQ (result.toString ().c_str (), "dffe:f247:5432:ffed::1");

    ip1 = "192.168.13.31";
    ip2 = "2001:db8:abcd:12::1";
    ASSERT_THROW (ip1 ^ ip2, std::invalid_argument);
}

/**
 * @brief not operators.
 */
TEST (IpAddress, not)
{
    IpAddress ip, result;

    ip = "192.168.13.31";
    result = ~ip;
    ASSERT_STREQ (result.toString ().c_str (), "63.87.242.224");

    ip = "2001:db8:abcd:12::1";
    result = ~ip;
    ASSERT_STREQ (result.toString ().c_str (), "dffe:f247:5432:ffed:ffff:ffff:ffff:fffe");
}

/**
 * @brief test the serialize method.
 */
TEST (IpAddress, serialize)
{
    IpAddress ip = "2001:db8:1234:5678::1";

    std::stringstream stream;
    ASSERT_NO_THROW (stream << ip);
    ASSERT_STREQ (stream.str ().c_str (), "2001:db8:1234:5678::1");
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
