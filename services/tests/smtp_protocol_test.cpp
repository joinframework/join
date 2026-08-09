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
#include <join/smtp_protocol.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::Smtp;
using join::Smtps;

/**
 * @brief test the family method.
 */
TEST (Smtp, family)
{
    ASSERT_EQ (Smtp ().family (), AF_INET);
    ASSERT_EQ (Smtp::v4 ().family (), AF_INET);
    ASSERT_EQ (Smtp::v6 ().family (), AF_INET6);
}

/**
 * @brief test the equal method.
 */
TEST (Smtp, equal)
{
    ASSERT_EQ (Smtp::v4 (), Smtp::v4 ());
    ASSERT_NE (Smtp::v4 (), Smtp::v6 ());
    ASSERT_EQ (Smtp::v6 (), Smtp::v6 ());
    ASSERT_NE (Smtp::v6 (), Smtp::v4 ());
}

/**
 * @brief test the default port.
 */
TEST (Smtp, defaultPort)
{
    ASSERT_EQ (Smtp::defaultPort, 25);
}

/**
 * @brief test the family method.
 */
TEST (Smtps, family)
{
    ASSERT_EQ (Smtps ().family (), AF_INET);
    ASSERT_EQ (Smtps::v4 ().family (), AF_INET);
    ASSERT_EQ (Smtps::v6 ().family (), AF_INET6);
}

/**
 * @brief test the equal method.
 */
TEST (Smtps, equal)
{
    ASSERT_EQ (Smtps::v4 (), Smtps::v4 ());
    ASSERT_NE (Smtps::v4 (), Smtps::v6 ());
    ASSERT_EQ (Smtps::v6 (), Smtps::v6 ());
    ASSERT_NE (Smtps::v6 (), Smtps::v4 ());
}

/**
 * @brief test the default port.
 */
TEST (Smtps, defaultPort)
{
    ASSERT_EQ (Smtps::defaultPort, 465);
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
