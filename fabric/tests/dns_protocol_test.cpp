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
#include <join/dns.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::Dns;

/**
 * @brief test the family method.
 */
TEST (Dns, family)
{
    ASSERT_EQ (Dns ().family (), AF_INET);
    ASSERT_EQ (Dns::v4 ().family (), AF_INET);
    ASSERT_EQ (Dns::v6 ().family (), AF_INET6);
}

/**
 * @brief test the type method.
 */
TEST (Dns, type)
{
    ASSERT_EQ (Dns ().type (), SOCK_DGRAM);
    ASSERT_EQ (Dns::v4 ().type (), SOCK_DGRAM);
    ASSERT_EQ (Dns::v6 ().type (), SOCK_DGRAM);
}

/**
 * @brief test the protocol method.
 */
TEST (Dns, protocol)
{
    ASSERT_EQ (Dns ().protocol (), IPPROTO_UDP);
    ASSERT_EQ (Dns::v4 ().protocol (), IPPROTO_UDP);
    ASSERT_EQ (Dns::v6 ().protocol (), IPPROTO_UDP);
}

/**
 * @brief test the equal method.
 */
TEST (Dns, equal)
{
    ASSERT_EQ (Dns::v4 (), Dns::v4 ());
    ASSERT_NE (Dns::v4 (), Dns::v6 ());
    ASSERT_EQ (Dns::v6 (), Dns::v6 ());
    ASSERT_NE (Dns::v6 (), Dns::v4 ());
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
