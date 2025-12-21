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
#include <join/socket.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::TlsErrc;
using join::TlsCategory;

/**
 * @brief Test name.
 */
TEST (TlsCategory, name)
{
    EXPECT_STREQ (TlsCategory ().name (), "libjoin");
}

/**
 * @brief Test message.
 */
TEST (TlsCategory, message)
{
    EXPECT_STREQ (TlsCategory ().message (0).c_str (), "success");
    EXPECT_STREQ (TlsCategory ().message (static_cast <int> (TlsErrc::TlsCloseNotifyAlert)).c_str (), "TLS close notify alert received");
    EXPECT_STREQ (TlsCategory ().message (static_cast <int> (TlsErrc::TlsProtocolError)).c_str (), "TLS protocol error");
}

/**
 * @brief Test default_error_condition.
 */
TEST (TlsCategory, default_error_condition)
{
    EXPECT_EQ (TlsCategory ().default_error_condition (0).message(), "success");
    EXPECT_EQ (TlsCategory ().default_error_condition (1).message(), "TLS close notify alert received");
    EXPECT_EQ (TlsCategory ().default_error_condition (2).message(), "TLS protocol error");
}

/**
 * @brief Test equal.
 */
TEST (TlsCategory, equal)
{
    std::error_code error1, error2;

    error1 = make_error_code (TlsErrc::TlsCloseNotifyAlert);
    error2 = make_error_code (TlsErrc::TlsCloseNotifyAlert);
    ASSERT_TRUE (error1 == error2);

    error2 = make_error_code (TlsErrc::TlsProtocolError);
    ASSERT_FALSE (error1 == error2);

    error1 = make_error_code (TlsErrc::TlsProtocolError);
    ASSERT_TRUE (error1 == error2);

    error1 = make_error_code (TlsErrc::TlsCloseNotifyAlert);
    ASSERT_TRUE (error1 == TlsErrc::TlsCloseNotifyAlert);

    error1 = make_error_code (TlsErrc::TlsCloseNotifyAlert);
    ASSERT_FALSE (error1 == TlsErrc::TlsProtocolError);

    error2 = make_error_code (TlsErrc::TlsProtocolError);
    ASSERT_TRUE (TlsErrc::TlsProtocolError == error2);

    error2 = make_error_code (TlsErrc::TlsCloseNotifyAlert);
    ASSERT_FALSE (TlsErrc::TlsProtocolError == error2);
}

/**
 * @brief Test different.
 */
TEST (TlsCategory, different)
{
    std::error_code error1, error2;

    error1 = make_error_code (TlsErrc::TlsCloseNotifyAlert);
    error2 = make_error_code (TlsErrc::TlsCloseNotifyAlert);
    ASSERT_FALSE (error1 != error2);

    error2 = make_error_code (TlsErrc::TlsProtocolError);
    ASSERT_TRUE (error1 != error2);

    error1 = make_error_code (TlsErrc::TlsProtocolError);
    ASSERT_FALSE (error1 != error2);

    error1 = make_error_code (TlsErrc::TlsCloseNotifyAlert);
    ASSERT_FALSE (error1 != TlsErrc::TlsCloseNotifyAlert);

    error1 = make_error_code (TlsErrc::TlsCloseNotifyAlert);
    ASSERT_TRUE (error1 != TlsErrc::TlsProtocolError);

    error2 = make_error_code (TlsErrc::TlsProtocolError);
    ASSERT_FALSE (TlsErrc::TlsProtocolError != error2);

    error2 = make_error_code (TlsErrc::TlsCloseNotifyAlert);
    ASSERT_TRUE (TlsErrc::TlsProtocolError != error2);
}

/**
 * @brief Test make_error_code.
 */
TEST (TlsCategory, make_error_code)
{
    auto code = make_error_code (TlsErrc::TlsCloseNotifyAlert);
    EXPECT_EQ (code, TlsErrc::TlsCloseNotifyAlert);
    EXPECT_STREQ (code.message ().c_str (), "TLS close notify alert received");
}

/**
 * @brief Test make_error_condition.
 */
TEST (TlsCategory, make_error_condition)
{
    auto code = make_error_condition (TlsErrc::TlsProtocolError);
    EXPECT_EQ (code, TlsErrc::TlsProtocolError);
    EXPECT_STREQ (code.message ().c_str (), "TLS protocol error");
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
