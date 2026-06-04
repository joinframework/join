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
#include <join/netlink.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::Netlink;

/**
 * @brief test the addr method.
 */
TEST (NetlinkEndpoint, addr)
{
    Netlink::Endpoint netlinkEndpoint;
    ASSERT_NE (netlinkEndpoint.addr (), nullptr);
}

/**
 * @brief test the length method.
 */
TEST (NetlinkEndpoint, length)
{
    Netlink::Endpoint netlinkEndpoint;
    ASSERT_EQ (netlinkEndpoint.length (), sizeof (struct sockaddr_nl));
}

/**
 * @brief test the device method.
 */
TEST (NetlinkEndpoint, device)
{
    Netlink::Endpoint netlinkEndpoint;
    ASSERT_EQ (netlinkEndpoint.device (), "");
}

/**
 * @brief test the protocol method.
 */
TEST (NetlinkEndpoint, protocol)
{
    ASSERT_EQ (Netlink::Endpoint ().protocol (), Netlink::rt ());
    ASSERT_EQ (Netlink::Endpoint (Netlink::rt (), RTMGRP_LINK).protocol (), Netlink::rt ());
    ASSERT_NE (Netlink::Endpoint (Netlink::rt (), RTMGRP_LINK).protocol (), Netlink::nf ());
    ASSERT_EQ (Netlink::Endpoint (Netlink::nf (), NFNLGRP_NONE).protocol (), Netlink::nf ());
    ASSERT_EQ (Netlink::Endpoint (RTMGRP_LINK).protocol (), Netlink::rt ());
    ASSERT_NE (Netlink::Endpoint (RTMGRP_LINK).protocol (), Netlink::nf ());
}

/**
 * @brief test the equal method.
 */
TEST (NetlinkEndpoint, equal)
{
    ASSERT_EQ (Netlink::Endpoint (RTMGRP_LINK), Netlink::Endpoint (RTMGRP_LINK));
    ASSERT_NE (Netlink::Endpoint (RTMGRP_LINK), Netlink::Endpoint (RTMGRP_IPV4_IFADDR));
    ASSERT_EQ (Netlink::Endpoint (RTMGRP_IPV4_IFADDR), Netlink::Endpoint (RTMGRP_IPV4_IFADDR));
    ASSERT_NE (Netlink::Endpoint (RTMGRP_IPV4_IFADDR), Netlink::Endpoint (RTMGRP_LINK));
}

/**
 * @brief test the serialize method.
 */
TEST (NetlinkEndpoint, serialize)
{
    std::stringstream stream;
    Netlink::Endpoint netlinkEndpoint (RTMGRP_LINK);
    ASSERT_NO_THROW (stream << netlinkEndpoint);
    std::stringstream ss;
    ss << "pid=" << getpid () << ",groups=" << uint32_t (RTMGRP_LINK);
    ASSERT_EQ (stream.str (), ss.str ());
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
