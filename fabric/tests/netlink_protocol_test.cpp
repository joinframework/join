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
#include <join/netlink_protocol.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::Netlink;

/**
 * @brief test the family method.
 */
TEST (Netlink, family)
{
    ASSERT_EQ (Netlink ().family (), AF_NETLINK);
    ASSERT_EQ (Netlink::rt ().family (), AF_NETLINK);
    ASSERT_EQ (Netlink::nf ().family (), AF_NETLINK);
}

/**
 * @brief test the type method.
 */
TEST (Netlink, type)
{
    ASSERT_EQ (Netlink ().type (), SOCK_RAW);
    ASSERT_EQ (Netlink::rt ().type (), SOCK_RAW);
    ASSERT_EQ (Netlink::nf ().type (), SOCK_RAW);
}

/**
 * @brief test the protocol method.
 */
TEST (Netlink, protocol)
{
    ASSERT_EQ (Netlink ().protocol (), NETLINK_ROUTE);
    ASSERT_EQ (Netlink::rt ().protocol (), NETLINK_ROUTE);
    ASSERT_EQ (Netlink::nf ().protocol (), NETLINK_NETFILTER);
}

/**
 * @brief test the equal method.
 */
TEST (Netlink, equal)
{
    ASSERT_EQ (Netlink::rt (), Netlink::rt ());
    ASSERT_NE (Netlink::rt (), Netlink::nf ());
    ASSERT_EQ (Netlink::nf (), Netlink::nf ());
    ASSERT_NE (Netlink::nf (), Netlink::rt ());
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
