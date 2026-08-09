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
#include <join/http_protocol.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::Http;
using join::Https;

/**
 * @brief test the family method.
 */
TEST (Http, family)
{
    ASSERT_EQ (Http ().family (), AF_INET);
    ASSERT_EQ (Http::v4 ().family (), AF_INET);
    ASSERT_EQ (Http::v6 ().family (), AF_INET6);
}

/**
 * @brief test the type method.
 */
TEST (Http, type)
{
    ASSERT_EQ (Http ().type (), SOCK_STREAM);
    ASSERT_EQ (Http::v4 ().type (), SOCK_STREAM);
    ASSERT_EQ (Http::v6 ().type (), SOCK_STREAM);
}

/**
 * @brief test the protocol method.
 */
TEST (Http, protocol)
{
    ASSERT_EQ (Http ().protocol (), IPPROTO_TCP);
    ASSERT_EQ (Http::v4 ().protocol (), IPPROTO_TCP);
    ASSERT_EQ (Http::v6 ().protocol (), IPPROTO_TCP);
}

/**
 * @brief test the equal method.
 */
TEST (Http, equal)
{
    ASSERT_EQ (Http::v4 (), Http::v4 ());
    ASSERT_NE (Http::v4 (), Http::v6 ());
    ASSERT_EQ (Http::v6 (), Http::v6 ());
    ASSERT_NE (Http::v6 (), Http::v4 ());
}

/**
 * @brief test the family method.
 */
TEST (Https, family)
{
    ASSERT_EQ (Https ().family (), AF_INET);
    ASSERT_EQ (Https::v4 ().family (), AF_INET);
    ASSERT_EQ (Https::v6 ().family (), AF_INET6);
}

/**
 * @brief test the equal method.
 */
TEST (Https, equal)
{
    ASSERT_EQ (Https::v4 (), Https::v4 ());
    ASSERT_NE (Https::v4 (), Https::v6 ());
    ASSERT_EQ (Https::v6 (), Https::v6 ());
    ASSERT_NE (Https::v6 (), Https::v4 ());
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
