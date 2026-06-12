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
#include <join/dot_protocol.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::Dot;

/**
 * @brief test the family method.
 */
TEST (Dot, family)
{
    ASSERT_EQ (Dot ().family (), AF_INET);
    ASSERT_EQ (Dot::v4 ().family (), AF_INET);
    ASSERT_EQ (Dot::v6 ().family (), AF_INET6);
}

/**
 * @brief test the type method.
 */
TEST (Dot, type)
{
    ASSERT_EQ (Dot ().type (), SOCK_STREAM);
    ASSERT_EQ (Dot::v4 ().type (), SOCK_STREAM);
    ASSERT_EQ (Dot::v6 ().type (), SOCK_STREAM);
}

/**
 * @brief test the protocol method.
 */
TEST (Dot, protocol)
{
    ASSERT_EQ (Dot ().protocol (), IPPROTO_TCP);
    ASSERT_EQ (Dot::v4 ().protocol (), IPPROTO_TCP);
    ASSERT_EQ (Dot::v6 ().protocol (), IPPROTO_TCP);
}

/**
 * @brief test the equal method.
 */
TEST (Dot, equal)
{
    ASSERT_EQ (Dot::v4 (), Dot::v4 ());
    ASSERT_NE (Dot::v4 (), Dot::v6 ());
    ASSERT_EQ (Dot::v6 (), Dot::v6 ());
    ASSERT_NE (Dot::v6 (), Dot::v4 ());
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
