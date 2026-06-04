/**
 * MIT License
 *
 * Copyright (c) 2026 Mathieu Rabine
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
#include <join/mdns.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::Mdns;

/**
 * @brief test the family method.
 */
TEST (Mdns, family)
{
    ASSERT_EQ (Mdns ().family (), AF_INET);
    ASSERT_EQ (Mdns::v4 ().family (), AF_INET);
    ASSERT_EQ (Mdns::v6 ().family (), AF_INET6);
}

/**
 * @brief test the type method.
 */
TEST (Mdns, type)
{
    ASSERT_EQ (Mdns ().type (), SOCK_DGRAM);
    ASSERT_EQ (Mdns::v4 ().type (), SOCK_DGRAM);
    ASSERT_EQ (Mdns::v6 ().type (), SOCK_DGRAM);
}

/**
 * @brief test the protocol method.
 */
TEST (Mdns, protocol)
{
    ASSERT_EQ (Mdns ().protocol (), IPPROTO_UDP);
    ASSERT_EQ (Mdns::v4 ().protocol (), IPPROTO_UDP);
    ASSERT_EQ (Mdns::v6 ().protocol (), IPPROTO_UDP);
}

/**
 * @brief test the equal method.
 */
TEST (Mdns, equal)
{
    ASSERT_EQ (Mdns::v4 (), Mdns::v4 ());
    ASSERT_NE (Mdns::v4 (), Mdns::v6 ());
    ASSERT_EQ (Mdns::v6 (), Mdns::v6 ());
    ASSERT_NE (Mdns::v6 (), Mdns::v4 ());
}

/**
 * @brief test the multicast address method.
 */
TEST (Mdns, multicastAddress)
{
    ASSERT_EQ (Mdns::multicastAddress (AF_INET), "224.0.0.251");
    ASSERT_EQ (Mdns::multicastAddress (AF_INET6), "ff02::fb");
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
