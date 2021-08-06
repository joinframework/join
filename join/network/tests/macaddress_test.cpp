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
#include <join/macaddress.hpp>

// Libraries.
#include <gtest/gtest.h>

// C.
#include <linux/if_arp.h>
#include <climits>

using join::net::MacAddress;

/**
 * @brief Test default construction.
 */
TEST (MacAddress, defaultConstruct)
{
    MacAddress mac;
    EXPECT_STREQ (mac.toString ().c_str (), "00:00:00:00:00:00");
}

/**
 * @brief Test copy construction.
 */
TEST (MacAddress, copyConstruct)
{
    MacAddress tmp ("50:7b:9d:13:82:df");

    MacAddress mac = tmp;
    EXPECT_STREQ (MacAddress (mac).toString ().c_str (), "50:7b:9d:13:82:df");
}

/**
 * @brief Test move construction.
 */
TEST (MacAddress, moveConstruct)
{
    MacAddress tmp ("02:42:64:2f:6a:d0");

    MacAddress mac = std::move (tmp);
    EXPECT_STREQ (mac.toString ().c_str (), "02:42:64:2f:6a:d0");
}

/**
 * @brief Test array construction.
 */
TEST (MacAddress, arrayConstruct)
{
    uint8_t tmp[] = {0x4c, 0x34, 0x88, 0x25, 0x41, 0xee};

    MacAddress mac1 (tmp);
    EXPECT_STREQ (mac1.toString ().c_str (), "4c:34:88:25:41:ee");

    MacAddress mac2 (tmp, 3);
    EXPECT_STREQ (mac2.toString ().c_str (), "4c:34:88:00:00:00");
}

/**
 * @brief Test initialization list construction.
 */
TEST (MacAddress, initListConstruct)
{
    MacAddress mac = {0x4c, 0x34, 0x88, 0x25, 0x41, 0xee};
    EXPECT_STREQ (mac.toString ().c_str (), "4c:34:88:25:41:ee");
}

/**
 * @brief Test sockaddr construction.
 */
TEST (MacAddress, sockaddrConstruct)
{
    uint8_t tmp[] = {0x50, 0x7b, 0x9d, 0x13, 0x82, 0xdf};

    struct sockaddr hwaddr;
    hwaddr.sa_family = ARPHRD_ETHER;
    memcpy (hwaddr.sa_data, tmp, sizeof (tmp));

    MacAddress mac (hwaddr);
    EXPECT_STREQ (mac.toString ().c_str (), "50:7b:9d:13:82:df");
}

/**
 * @brief Test string construction.
 */
TEST (MacAddress, stringConstruct)
{
    MacAddress mac = "00:00:00:00:00:00";
    EXPECT_STREQ (mac.toString ().c_str (), "00:00:00:00:00:00");

    mac = MacAddress ("4c:34:88:25:41:ee");
    EXPECT_STREQ (mac.toString ().c_str (), "4c:34:88:25:41:ee");

    mac = MacAddress ("4C:34:88:25:41:EE");
    EXPECT_STREQ (mac.toString ().c_str (), "4c:34:88:25:41:ee");

    EXPECT_THROW (mac = MacAddress ("xx:xx:xx:xx:xx:xx"), std::invalid_argument);
    EXPECT_THROW (mac = MacAddress ("XX:XX:XX:XX:XX:XX"), std::invalid_argument);

    EXPECT_THROW (mac = MacAddress ("foo"), std::invalid_argument);

    mac = MacAddress (std::string ("00:00:00:00:00:00"));
    EXPECT_STREQ (mac.toString ().c_str (), "00:00:00:00:00:00");

    mac = MacAddress (std::string ("4c:34:88:25:41:ee"));
    EXPECT_STREQ (mac.toString ().c_str (), "4c:34:88:25:41:ee");

    mac = MacAddress (std::string ("4C:34:88:25:41:EE"));
    EXPECT_STREQ (mac.toString ().c_str (), "4c:34:88:25:41:ee");

    EXPECT_THROW (mac = MacAddress (std::string ("xx:xx:xx:xx:xx:xx")), std::invalid_argument);
    EXPECT_THROW (mac = MacAddress (std::string ("XX:XX:XX:XX:XX:XX")), std::invalid_argument);

    EXPECT_THROW (mac = MacAddress (std::string ("foo")), std::invalid_argument);
}

/**
 * @brief Test family method.
 */
TEST (MacAddress, family)
{
    MacAddress mac;
    ASSERT_EQ (mac.family (), ARPHRD_ETHER);
}

/**
 * @brief Test addr method.
 */
TEST (MacAddress, addr)
{
    MacAddress mac;
    EXPECT_NE (mac.addr (), nullptr);
}

/**
 * @brief Test length method.
 */
TEST (MacAddress, length)
{
    MacAddress mac;
    EXPECT_EQ (mac.length (), IFHWADDRLEN);
}

/**
 * @brief Test isWildcard method.
 */
TEST (MacAddress, isWildcard)
{
    MacAddress mac;
    ASSERT_TRUE (mac.isWildcard ());

    mac = MacAddress ("4c:34:88:25:41:ee");
    ASSERT_FALSE (mac.isWildcard ());

    mac = MacAddress ("00:00:00:00:00:00");
    ASSERT_TRUE (mac.isWildcard ());
}

/**
 * @brief Test isBroadcast method.
 */
TEST (MacAddress, isBroadcast)
{
    MacAddress mac;
    ASSERT_FALSE (mac.isBroadcast ());

    mac = MacAddress ("4c:34:88:25:41:ee");
    ASSERT_FALSE (mac.isBroadcast ());

    mac = MacAddress ("ff:ff:ff:ff:ff:ff");
    ASSERT_TRUE (mac.isBroadcast ());
}

/**
 * @brief Test isMacAddress method.
 */
TEST (MacAddress, isMacAddress)
{
    ASSERT_TRUE (MacAddress::isMacAddress ("00:00:00:00:00:00"));
    ASSERT_TRUE (MacAddress::isMacAddress ("4c:34:88:25:41:ee"));
    ASSERT_TRUE (MacAddress::isMacAddress ("4C:34:88:25:41:EE"));

    ASSERT_FALSE (MacAddress::isMacAddress ("foo.bar"));
    ASSERT_FALSE (MacAddress::isMacAddress ("4C:34:88:25:41.bar"));
}

/**
 * @brief Test toString method.
 */
TEST (MacAddress, toString)
{
    MacAddress mac = "00:00:00:00:00:00";
    EXPECT_STREQ (mac.toString ().c_str (), "00:00:00:00:00:00");

    mac = "4c:34:88:25:41:ee";
    EXPECT_STREQ (mac.toString (std::nouppercase).c_str (), "4c:34:88:25:41:ee");
    EXPECT_STREQ (mac.toString (std::uppercase).c_str (), "4C:34:88:25:41:EE");

    mac = "02:42:64:2f:6a:d0";
    EXPECT_STREQ (mac.toString (std::nouppercase).c_str (), "02:42:64:2f:6a:d0");
    EXPECT_STREQ (mac.toString (std::uppercase).c_str (), "02:42:64:2F:6A:D0");

    mac = "50:7b:9d:13:82:df";
    EXPECT_STREQ (mac.toString (std::nouppercase).c_str (), "50:7b:9d:13:82:df");
    EXPECT_STREQ (mac.toString (std::uppercase).c_str (), "50:7B:9D:13:82:DF");
}

/**
 * @brief Test toIpv6 method.
 */
TEST (MacAddress, toIpv6)
{
    MacAddress mac = "00:14:3e:48:d4:5b";
    EXPECT_TRUE (mac.toIpv6 ("2001:db8::", 32).isGlobal());
    EXPECT_EQ (mac.toIpv6 ("2001:db8::", 32).toString (), "2001:db8::214:3eff:fe48:d45b");
}

/**
 * @brief Test toLinkLocalIpv6 method.
 */
TEST (MacAddress, toLinkLocalIpv6)
{
    MacAddress mac = "e0:3f:49:45:9d:7b";
    EXPECT_TRUE (mac.toLinkLocalIpv6 ().isLinkLocal ());
    EXPECT_EQ (mac.toLinkLocalIpv6 ().toString (), "fe80::e23f:49ff:fe45:9d7b");
}

/**
 * @brief Test toUniqueLocalIpv6 method.
 */
TEST (MacAddress, toUniqueLocalIpv6)
{
    MacAddress mac = "e0:3f:49:45:9d:7b";
    EXPECT_TRUE (mac.toUniqueLocalIpv6 ().isUniqueLocal ());
}

/**
 * @brief Test clear method.
 */
TEST (MacAddress, clear)
{
    MacAddress mac = "e0:3f:49:45:9d:7b";
    EXPECT_FALSE (mac.isWildcard ());

    mac.clear ();
    EXPECT_TRUE (mac.isWildcard ());
}

/**
 * @brief Test begin method.
 */
TEST (MacAddress, begin)
{
    MacAddress mac = "4c:34:88:25:41:ee";

    MacAddress::iterator beg = mac.begin ();
    EXPECT_EQ (*beg, 0x4c);

    MacAddress::const_iterator cbeg = mac.begin ();
    EXPECT_EQ (*cbeg, 0x4c);
}

/**
 * @brief Test cbegin method.
 */
TEST (MacAddress, cbegin)
{
    MacAddress mac = "4c:34:88:25:41:ee";

    auto beg = mac.cbegin ();
    EXPECT_EQ (*beg, 0x4c);
}

/**
 * @brief Test end method.
 */
TEST (MacAddress, end)
{
    MacAddress mac = "4c:34:88:25:41:ee";

    MacAddress::iterator end = mac.end ();
    EXPECT_EQ (*(--end), 0xee);

    MacAddress::const_iterator cend = mac.cend ();
    EXPECT_EQ (*(--cend), 0xee);
}

/**
 * @brief Test cend method.
 */
TEST (MacAddress, cend)
{
    MacAddress mac = "4c:34:88:25:41:ee";

    auto end = mac.cend ();
    EXPECT_EQ (*(--end), 0xee);
}

/**
 * @brief Test copy assignment method.
 */
TEST (MacAddress, copyAssign)
{
    MacAddress tmp ("50:7b:9d:13:82:df"), mac;

    mac = tmp;
    EXPECT_STREQ (mac.toString ().c_str (), "50:7b:9d:13:82:df");
}

/**
 * @brief Test move assignment method.
 */
TEST (MacAddress, moveAssign)
{
    MacAddress tmp ("50:7b:9d:13:82:df"), mac;

    mac = std::move (tmp);
    EXPECT_STREQ (mac.toString ().c_str (), "50:7b:9d:13:82:df");
}

/**
 * @brief Test array assignment method.
 */
TEST (MacAddress, arrayAssign)
{
    MacAddress mac;
    uint8_t tmp[] = {0x4c, 0x34, 0x88, 0x25, 0x41, 0xee};

    mac = tmp;
    EXPECT_STREQ (mac.toString ().c_str (), "4c:34:88:25:41:ee");
}

/**
 * @brief Test array assignment method.
 */
TEST (MacAddress, initListAssign)
{
    MacAddress mac;

    mac = {};
    EXPECT_STREQ (mac.toString ().c_str (), "00:00:00:00:00:00");

    mac = {0x50, 0x7b, 0x9d};
    EXPECT_STREQ (mac.toString ().c_str (), "50:7b:9d:00:00:00");

    mac = {0x50, 0x7b, 0x9d, 0x13, 0x82, 0xdf};
    EXPECT_STREQ (mac.toString ().c_str (), "50:7b:9d:13:82:df");

    EXPECT_THROW ((mac = {0x50, 0x7b, 0x9d, 0x13, 0x82, 0xdf, 0xff}), std::invalid_argument);
}

/**
 * @brief Test sockaddr assignment method.
 */
TEST (MacAddress, sockaddrAssign)
{
    MacAddress mac;
    struct sockaddr hwaddr;
    uint8_t hw[] = {0x50, 0x7b, 0x9d, 0x13, 0x82, 0xdf};

    hwaddr.sa_family = ARPHRD_ETHER;
    memcpy (hwaddr.sa_data, hw, 6);

    mac = hwaddr;
    EXPECT_STREQ (mac.toString ().c_str (), "50:7b:9d:13:82:df");
}

/**
 * @brief Test add assignment method.
 */
TEST (MacAddress, addAssign)
{
    MacAddress mac;

    mac += 255;
    EXPECT_STREQ (mac.toString ().c_str (), "00:00:00:00:00:ff");

    mac += 65535;
    EXPECT_STREQ (mac.toString ().c_str (), "00:00:00:01:00:fe");
}

/**
 * @brief Test pre-increment method.
 */
TEST (MacAddress, preIncrement)
{
    MacAddress mac = "02:42:64:2f:6a:de";
    EXPECT_STREQ ((++mac).toString ().c_str (), "02:42:64:2f:6a:df");
    EXPECT_STREQ ((++mac).toString ().c_str (), "02:42:64:2f:6a:e0");

    mac = "02:42:64:2f:6a:fe";
    EXPECT_STREQ ((++mac).toString ().c_str (), "02:42:64:2f:6a:ff");
    EXPECT_STREQ ((++mac).toString ().c_str (), "02:42:64:2f:6b:00");

    mac = "02:42:64:2f:fe:ff";
    EXPECT_STREQ ((++mac).toString ().c_str (), "02:42:64:2f:ff:00");
    EXPECT_STREQ ((++mac).toString ().c_str (), "02:42:64:2f:ff:01");

    mac = "02:42:64:fe:ff:ff";
    EXPECT_STREQ ((++mac).toString ().c_str (), "02:42:64:ff:00:00");
    EXPECT_STREQ ((++mac).toString ().c_str (), "02:42:64:ff:00:01");

    mac = "02:42:fe:ff:ff:ff";
    EXPECT_STREQ ((++mac).toString ().c_str (), "02:42:ff:00:00:00");
    EXPECT_STREQ ((++mac).toString ().c_str (), "02:42:ff:00:00:01");

    mac = "02:fe:ff:ff:ff:ff";
    EXPECT_STREQ ((++mac).toString ().c_str (), "02:ff:00:00:00:00");
    EXPECT_STREQ ((++mac).toString ().c_str (), "02:ff:00:00:00:01");

    mac = "fe:ff:ff:ff:ff:ff";
    EXPECT_STREQ ((++mac).toString ().c_str (), "ff:00:00:00:00:00");
    EXPECT_STREQ ((++mac).toString ().c_str (), "ff:00:00:00:00:01");
}

/**
 * @brief Test post-increment method.
 */
TEST (MacAddress, postIncrement)
{
    MacAddress mac = "02:42:64:2f:6a:de";
    EXPECT_STREQ ((mac++).toString ().c_str (), "02:42:64:2f:6a:de");
    EXPECT_STREQ ((mac++).toString ().c_str (), "02:42:64:2f:6a:df");
    EXPECT_STREQ ((mac++).toString ().c_str (), "02:42:64:2f:6a:e0");

    mac = "02:42:64:2f:6a:fe";
    EXPECT_STREQ ((mac++).toString ().c_str (), "02:42:64:2f:6a:fe");
    EXPECT_STREQ ((mac++).toString ().c_str (), "02:42:64:2f:6a:ff");
    EXPECT_STREQ ((mac++).toString ().c_str (), "02:42:64:2f:6b:00");

    mac = "02:42:64:2f:fe:ff";
    EXPECT_STREQ ((mac++).toString ().c_str (), "02:42:64:2f:fe:ff");
    EXPECT_STREQ ((mac++).toString ().c_str (), "02:42:64:2f:ff:00");
    EXPECT_STREQ ((mac++).toString ().c_str (), "02:42:64:2f:ff:01");

    mac = "02:42:64:fe:ff:ff";
    EXPECT_STREQ ((mac++).toString ().c_str (), "02:42:64:fe:ff:ff");
    EXPECT_STREQ ((mac++).toString ().c_str (), "02:42:64:ff:00:00");
    EXPECT_STREQ ((mac++).toString ().c_str (), "02:42:64:ff:00:01");

    mac = "02:42:fe:ff:ff:ff";
    EXPECT_STREQ ((mac++).toString ().c_str (), "02:42:fe:ff:ff:ff");
    EXPECT_STREQ ((mac++).toString ().c_str (), "02:42:ff:00:00:00");
    EXPECT_STREQ ((mac++).toString ().c_str (), "02:42:ff:00:00:01");

    mac = "02:fe:ff:ff:ff:ff";
    EXPECT_STREQ ((mac++).toString ().c_str (), "02:fe:ff:ff:ff:ff");
    EXPECT_STREQ ((mac++).toString ().c_str (), "02:ff:00:00:00:00");
    EXPECT_STREQ ((mac++).toString ().c_str (), "02:ff:00:00:00:01");

    mac = "fe:ff:ff:ff:ff:ff";
    EXPECT_STREQ ((mac++).toString ().c_str (), "fe:ff:ff:ff:ff:ff");
    EXPECT_STREQ ((mac++).toString ().c_str (), "ff:00:00:00:00:00");
    EXPECT_STREQ ((mac++).toString ().c_str (), "ff:00:00:00:00:01");
}

/**
 * @brief Test element access method.
 */
TEST (MacAddress, getElement)
{
    MacAddress mac ("50:7b:9d:13:82:df");

    EXPECT_EQ (mac[0], 0x50);
    EXPECT_EQ (mac[1], 0x7b);
    EXPECT_EQ (mac[2], 0x9d);
    EXPECT_EQ (mac[3], 0x13);
    EXPECT_EQ (mac[4], 0x82);
    EXPECT_EQ (mac[5], 0xdf);

    EXPECT_EQ (mac[0], 80);
    EXPECT_EQ (mac[1], 123);
    EXPECT_EQ (mac[2], 157);
    EXPECT_EQ (mac[3], 19);
    EXPECT_EQ (mac[4], 130);
    EXPECT_EQ (mac[5], 223);

    EXPECT_NO_THROW (mac[0] = 0x00);
    EXPECT_NO_THROW (mac[1] = 0x0a);
    EXPECT_NO_THROW (mac[2] = 0xd4);
    EXPECT_NO_THROW (mac[3] = 0x7f);
    EXPECT_NO_THROW (mac[4] = 0x04);
    EXPECT_NO_THROW (mac[5] = 0xff);

    EXPECT_STREQ (mac.toString ().c_str (), "00:0a:d4:7f:04:ff");

    EXPECT_NO_THROW (mac[0] = 255);
    EXPECT_NO_THROW (mac[1] = 4);
    EXPECT_NO_THROW (mac[2] = 127);
    EXPECT_NO_THROW (mac[3] = 212);
    EXPECT_NO_THROW (mac[4] = 10);
    EXPECT_NO_THROW (mac[5] = 0);

    EXPECT_STREQ (mac.toString ().c_str (), "ff:04:7f:d4:0a:00");

    EXPECT_THROW (mac[CHAR_MAX] = 0x00, std::invalid_argument);
    EXPECT_THROW (mac[SHRT_MAX] = 0x00, std::invalid_argument);
    EXPECT_THROW (mac[INT_MAX] = 0x00, std::invalid_argument);
}

/**
 * @brief not operators.
 */
TEST (MacAddress, notOperation)
{
    MacAddress mac, result;

    mac = "02:42:64:2f:6a:d0";
    result = ~mac;
    EXPECT_STREQ (result.toString ().c_str (), "fd:bd:9b:d0:95:2f");
}

/**
 * @brief Test add method.
 */
TEST (MacAddress, add)
{
    MacAddress mac1, mac2;

    mac1 = "fd:bd:9b:d0:95:2f";
    mac2 = mac1 + 1;
    EXPECT_STREQ (mac1.toString ().c_str (), "fd:bd:9b:d0:95:2f");
    EXPECT_STREQ (mac2.toString ().c_str (), "fd:bd:9b:d0:95:30");

    mac1 = "02:42:64:2f:6a:d0";
    mac2 = 1 + mac1;
    EXPECT_STREQ (mac1.toString ().c_str (), "02:42:64:2f:6a:d0");
    EXPECT_STREQ (mac2.toString ().c_str (), "02:42:64:2f:6a:d1");
}

/**
 * @brief Test is equal method.
 */
TEST (MacAddress, equal)
{
    MacAddress mac1, mac2;

    mac1 = "02:42:64:2f:6a:d0";
    mac2 = "02:42:64:2f:6a:d0";
    EXPECT_TRUE (mac1 == mac2);

    mac1 = "02:42:64:2f:6a:d0";
    mac2 = "50:7b:9d:13:82:df";
    EXPECT_FALSE (mac1 == mac2);

    mac1 = "50:7b:9d:13:82:df";
    mac2 = "4c:34:88:25:41:ee";
    EXPECT_FALSE (mac1 == mac2);

    mac1 = "02:42:64:2f:6a:d0";
    EXPECT_TRUE (mac1 == "02:42:64:2f:6a:d0");

    mac1 = "02:42:64:2f:6a:d0";
    EXPECT_FALSE (mac1 == "50:7b:9d:13:82:df");

    mac1 = "50:7b:9d:13:82:df";
    EXPECT_FALSE (mac1 == "4c:34:88:25:41:ee");

    mac2 = "02:42:64:2f:6a:d0";
    EXPECT_TRUE ("02:42:64:2f:6a:d0" == mac2);

    mac2 = "50:7b:9d:13:82:df";
    EXPECT_FALSE ("02:42:64:2f:6a:d0" == mac2);

    mac2 = "4c:34:88:25:41:ee";
    EXPECT_FALSE ("50:7b:9d:13:82:df" == mac2);
}

/**
 * @brief Test is different method.
 */
TEST (MacAddress, different)
{
    MacAddress mac1, mac2;

    mac1 = "02:42:64:2f:6a:d0";
    mac2 = "02:42:64:2f:6a:d0";
    EXPECT_FALSE (mac1 != mac2);

    mac1 = "02:42:64:2f:6a:d0";
    mac2 = "50:7b:9d:13:82:df";
    EXPECT_TRUE (mac1 != mac2);

    mac1 = "50:7b:9d:13:82:df";
    mac2 = "4c:34:88:25:41:ee";
    EXPECT_TRUE (mac1 != mac2);

    mac1 = "02:42:64:2f:6a:d0";
    EXPECT_FALSE (mac1 != "02:42:64:2f:6a:d0");

    mac1 = "02:42:64:2f:6a:d0";
    EXPECT_TRUE (mac1 != "50:7b:9d:13:82:df");

    mac1 = "50:7b:9d:13:82:df";
    EXPECT_TRUE (mac1 != "4c:34:88:25:41:ee");

    mac2 = "02:42:64:2f:6a:d0";
    EXPECT_FALSE ("02:42:64:2f:6a:d0" != mac2);

    mac2 = "50:7b:9d:13:82:df";
    EXPECT_TRUE ("02:42:64:2f:6a:d0" != mac2);

    mac2 = "4c:34:88:25:41:ee";
    EXPECT_TRUE ("50:7b:9d:13:82:df" != mac2);
}

/**
 * @brief Test is lower method.
 */
TEST (MacAddress, lower)
{
    MacAddress mac1, mac2;

    mac1 = "02:42:64:2f:6a:d0";
    mac2 = "02:42:64:2f:6a:d0";
    EXPECT_FALSE (mac1 < mac2);

    mac1 = "02:42:64:2f:6a:d0";
    mac2 = "50:7b:9d:13:82:df";
    EXPECT_TRUE (mac1 < mac2);

    mac1 = "50:7b:9d:13:82:df";
    mac2 = "4c:34:88:25:41:ee";
    EXPECT_FALSE (mac1 < mac2);

    mac1 = "02:42:64:2f:6a:d0";
    EXPECT_FALSE (mac1 < "02:42:64:2f:6a:d0");

    mac1 = "02:42:64:2f:6a:d0";
    EXPECT_TRUE (mac1 < "50:7b:9d:13:82:df");

    mac1 = "50:7b:9d:13:82:df";
    EXPECT_FALSE (mac1 < "4c:34:88:25:41:ee");

    mac2 = "02:42:64:2f:6a:d0";
    EXPECT_FALSE ("02:42:64:2f:6a:d0" < mac2);

    mac2 = "50:7b:9d:13:82:df";
    EXPECT_TRUE ("02:42:64:2f:6a:d0" < mac2);

    mac2 = "4c:34:88:25:41:ee";
    EXPECT_FALSE ("50:7b:9d:13:82:df" < mac2);
}

/**
 * @brief Test is lower or equal method.
 */
TEST (MacAddress, lowerOrEqual)
{
    MacAddress mac1, mac2;

    mac1 = "02:42:64:2f:6a:d0";
    mac2 = "02:42:64:2f:6a:d0";
    EXPECT_TRUE (mac1 <= mac2);

    mac1 = "02:42:64:2f:6a:d0";
    mac2 = "50:7b:9d:13:82:df";
    EXPECT_TRUE (mac1 <= mac2);

    mac1 = "50:7b:9d:13:82:df";
    mac2 = "4c:34:88:25:41:ee";
    EXPECT_FALSE (mac1 <= mac2);

    mac1 = "02:42:64:2f:6a:d0";
    EXPECT_TRUE (mac1 <= "02:42:64:2f:6a:d0");

    mac1 = "02:42:64:2f:6a:d0";
    EXPECT_TRUE (mac1 <= "50:7b:9d:13:82:df");

    mac1 = "50:7b:9d:13:82:df";
    EXPECT_FALSE (mac1 <= "4c:34:88:25:41:ee");

    mac2 = "02:42:64:2f:6a:d0";
    EXPECT_TRUE ("02:42:64:2f:6a:d0" <= mac2);

    mac2 = "50:7b:9d:13:82:df";
    EXPECT_TRUE ("02:42:64:2f:6a:d0" <= mac2);

    mac2 = "4c:34:88:25:41:ee";
    EXPECT_FALSE ("50:7b:9d:13:82:df" <= mac2);
}

/**
 * @brief Test is greater method.
 */
TEST (MacAddress, greater)
{
    MacAddress mac1, mac2;

    mac1 = "02:42:64:2f:6a:d0";
    mac2 = "02:42:64:2f:6a:d0";
    EXPECT_FALSE (mac1 > mac2);

    mac1 = "02:42:64:2f:6a:d0";
    mac2 = "50:7b:9d:13:82:df";
    EXPECT_FALSE (mac1 > mac2);

    mac1 = "50:7b:9d:13:82:df";
    mac2 = "4c:34:88:25:41:ee";
    EXPECT_TRUE (mac1 > mac2);

    mac1 = "02:42:64:2f:6a:d0";
    EXPECT_FALSE (mac1 > "02:42:64:2f:6a:d0");

    mac1 = "02:42:64:2f:6a:d0";
    EXPECT_FALSE (mac1 > "50:7b:9d:13:82:df");

    mac1 = "50:7b:9d:13:82:df";
    EXPECT_TRUE (mac1 > "4c:34:88:25:41:ee");

    mac2 = "02:42:64:2f:6a:d0";
    EXPECT_FALSE ("02:42:64:2f:6a:d0" > mac2);

    mac2 = "50:7b:9d:13:82:df";
    EXPECT_FALSE ("02:42:64:2f:6a:d0" > mac2);

    mac2 = "4c:34:88:25:41:ee";
    EXPECT_TRUE ("50:7b:9d:13:82:df" > mac2);
}

/**
 * @brief Test is greater or equal method.
 */
TEST (MacAddress, greaterOrEqual)
{
    MacAddress mac1, mac2;

    mac1 = "02:42:64:2f:6a:d0";
    mac2 = "02:42:64:2f:6a:d0";
    EXPECT_TRUE (mac1 >= mac2);

    mac1 = "02:42:64:2f:6a:d0";
    mac2 = "50:7b:9d:13:82:df";
    EXPECT_FALSE (mac1 >= mac2);

    mac1 = "50:7b:9d:13:82:df";
    mac2 = "4c:34:88:25:41:ee";
    EXPECT_TRUE (mac1 >= mac2);

    mac1 = "02:42:64:2f:6a:d0";
    EXPECT_TRUE (mac1 >= "02:42:64:2f:6a:d0");

    mac1 = "02:42:64:2f:6a:d0";
    EXPECT_FALSE (mac1 >= "50:7b:9d:13:82:df");

    mac1 = "50:7b:9d:13:82:df";
    EXPECT_TRUE (mac1 >= "4c:34:88:25:41:ee");

    mac2 = "02:42:64:2f:6a:d0";
    EXPECT_TRUE ("02:42:64:2f:6a:d0" >= mac2);

    mac2 = "50:7b:9d:13:82:df";
    EXPECT_FALSE ("02:42:64:2f:6a:d0" >= mac2);

    mac2 = "4c:34:88:25:41:ee";
    EXPECT_TRUE ("50:7b:9d:13:82:df" >= mac2);
}

/**
 * @brief and operators.
 */
TEST (MacAddress, and)
{
    MacAddress mac1, mac2, result;

    mac1 = "02:42:64:2f:6a:d0";
    mac2 = "ff:ff:ff:ff:ff:00";
    result = mac1 & mac2;
    EXPECT_STREQ (mac1.toString ().c_str (), "02:42:64:2f:6a:d0");
    EXPECT_STREQ (mac2.toString ().c_str (), "ff:ff:ff:ff:ff:00");
    EXPECT_STREQ (result.toString ().c_str (), "02:42:64:2f:6a:00");

    mac1 = "02:42:64:2f:6a:d0";
    result = mac1 & "ff:ff:ff:ff:ff:00";
    EXPECT_STREQ (mac1.toString ().c_str (), "02:42:64:2f:6a:d0");
    EXPECT_STREQ (result.toString ().c_str (), "02:42:64:2f:6a:00");

    mac2 = "ff:ff:ff:ff:ff:00";
    result = "02:42:64:2f:6a:d0" & mac2;
    EXPECT_STREQ (mac2.toString ().c_str (), "ff:ff:ff:ff:ff:00");
    EXPECT_STREQ (result.toString ().c_str (), "02:42:64:2f:6a:00");
}

/**
 * @brief or operators.
 */
TEST (MacAddress, or)
{
    MacAddress mac1, mac2, result;

    mac1 = "02:42:64:2f:6a:d0";
    mac2 = "ff:ff:ff:ff:ff:00";
    result = mac1 | mac2;
    EXPECT_STREQ (mac1.toString ().c_str (), "02:42:64:2f:6a:d0");
    EXPECT_STREQ (mac2.toString ().c_str (), "ff:ff:ff:ff:ff:00");
    EXPECT_STREQ (result.toString ().c_str (), "ff:ff:ff:ff:ff:d0");

    mac1 = "02:42:64:2f:6a:d0";
    result = mac1 | "ff:ff:ff:ff:ff:00";
    EXPECT_STREQ (mac1.toString ().c_str (), "02:42:64:2f:6a:d0");
    EXPECT_STREQ (result.toString ().c_str (), "ff:ff:ff:ff:ff:d0");

    mac2 = "ff:ff:ff:ff:ff:00";
    result = "02:42:64:2f:6a:d0" | mac2;
    EXPECT_STREQ (mac2.toString ().c_str (), "ff:ff:ff:ff:ff:00");
    EXPECT_STREQ (result.toString ().c_str (), "ff:ff:ff:ff:ff:d0");
}

/**
 * @brief xor operators.
 */
TEST (MacAddress, xor)
{
    MacAddress mac1, mac2, result;

    mac1 = "02:42:64:2f:6a:d0";
    mac2 = "ff:ff:ff:ff:ff:00";
    result = mac1 ^ mac2;
    EXPECT_STREQ (mac1.toString ().c_str (), "02:42:64:2f:6a:d0");
    EXPECT_STREQ (mac2.toString ().c_str (), "ff:ff:ff:ff:ff:00");
    EXPECT_STREQ (result.toString ().c_str (), "fd:bd:9b:d0:95:d0");

    mac1 = "02:42:64:2f:6a:d0";
    result = mac1 ^ "ff:ff:ff:ff:ff:00";
    EXPECT_STREQ (mac1.toString ().c_str (), "02:42:64:2f:6a:d0");
    EXPECT_STREQ (result.toString ().c_str (), "fd:bd:9b:d0:95:d0");

    mac2 = "ff:ff:ff:ff:ff:00";
    result = "02:42:64:2f:6a:d0" ^ mac2;
    EXPECT_STREQ (mac2.toString ().c_str (), "ff:ff:ff:ff:ff:00");
    EXPECT_STREQ (result.toString ().c_str (), "fd:bd:9b:d0:95:d0");
}

/**
 * @brief test the serialize method.
 */
TEST (MacAddress, serialize)
{
    MacAddress mac = "50:7b:9d:13:82:df";

    std::stringstream stream;
    EXPECT_NO_THROW (stream << mac);
    EXPECT_STREQ (stream.str ().c_str (), "50:7b:9d:13:82:df");
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
   testing::InitGoogleTest (&argc, argv);
   return RUN_ALL_TESTS ();
}
