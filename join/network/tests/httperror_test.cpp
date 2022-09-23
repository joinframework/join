/**
 * MIT License
 *
 * Copyright (c) 2022 Mathieu Rabine
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
#include <join/httpmessage.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::HttpErrc;
using join::HttpCategory;

/**
 * @brief Test name.
 */
TEST (HttpCategory, name)
{
    ASSERT_STREQ (HttpCategory ().name (), "libjoin");
}

/**
 * @brief Test message.
 */
TEST (HttpCategory, message)
{
    ASSERT_STREQ (HttpCategory ().message (0).c_str (), "success");
    ASSERT_STREQ (HttpCategory ().message (static_cast <int> (HttpErrc::InvalidRequest)).c_str (), "invalid HTTP request");
    ASSERT_STREQ (HttpCategory ().message (static_cast <int> (HttpErrc::InvalidResponse)).c_str (), "invalid HTTP response");
    ASSERT_STREQ (HttpCategory ().message (static_cast <int> (HttpErrc::InvalidMethod)).c_str (),  "invalid HTTP method");
    ASSERT_STREQ (HttpCategory ().message (static_cast <int> (HttpErrc::InvalidHeader)).c_str (),  "invalid HTTP header");
}

/**
 * @brief Test default_error_condition.
 */
TEST (HttpCategory, default_error_condition)
{
    ASSERT_EQ (HttpCategory ().default_error_condition (0).message(), "success");
    ASSERT_EQ (HttpCategory ().default_error_condition (1).message(), "invalid HTTP request");
    ASSERT_EQ (HttpCategory ().default_error_condition (2).message(), "invalid HTTP response");
    ASSERT_EQ (HttpCategory ().default_error_condition (3).message(), "invalid HTTP method");
    ASSERT_EQ (HttpCategory ().default_error_condition (4).message(), "invalid HTTP header");
}

/**
 * @brief Test equal.
 */
TEST (HttpCategory, equal)
{
    std::error_code error1, error2;

    error1 = make_error_code (HttpErrc::InvalidRequest);
    error2 = make_error_code (HttpErrc::InvalidRequest);
    ASSERT_TRUE (error1 == error2);

    error2 = make_error_code (HttpErrc::InvalidMethod);
    ASSERT_FALSE (error1 == error2);

    error1 = make_error_code (HttpErrc::InvalidMethod);
    ASSERT_TRUE (error1 == error2);

    error2 = make_error_code (HttpErrc::InvalidHeader);
    ASSERT_FALSE (error1 == error2);

    error1 = make_error_code (HttpErrc::InvalidHeader);
    ASSERT_TRUE (error1 == error2);

    error1 = make_error_code (HttpErrc::InvalidRequest);
    ASSERT_TRUE (error1 == HttpErrc::InvalidRequest);

    error1 = make_error_code (HttpErrc::InvalidMethod);
    ASSERT_FALSE (error1 == HttpErrc::InvalidRequest);

    error2 = make_error_code (HttpErrc::InvalidMethod);
    ASSERT_TRUE (HttpErrc::InvalidMethod == error2);

    error2 = make_error_code (HttpErrc::InvalidHeader);
    ASSERT_FALSE (HttpErrc::InvalidMethod == error2);
}

/**
 * @brief Test different.
 */
TEST (HttpCategory, different)
{
    std::error_code error1, error2;

    error1 = make_error_code (HttpErrc::InvalidRequest);
    error2 = make_error_code (HttpErrc::InvalidRequest);
    ASSERT_FALSE (error1 != error2);

    error2 = make_error_code (HttpErrc::InvalidMethod);
    ASSERT_TRUE (error1 != error2);

    error1 = make_error_code (HttpErrc::InvalidMethod);
    ASSERT_FALSE (error1 != error2);

    error2 = make_error_code (HttpErrc::InvalidHeader);
    ASSERT_TRUE (error1 != error2);

    error1 = make_error_code (HttpErrc::InvalidHeader);
    ASSERT_FALSE (error1 != error2);

    error1 = make_error_code (HttpErrc::InvalidRequest);
    ASSERT_FALSE (error1 != HttpErrc::InvalidRequest);

    error1 = make_error_code (HttpErrc::InvalidMethod);
    ASSERT_TRUE (error1 != HttpErrc::InvalidRequest);

    error2 = make_error_code (HttpErrc::InvalidMethod);
    ASSERT_FALSE (HttpErrc::InvalidMethod != error2);

    error2 = make_error_code (HttpErrc::InvalidHeader);
    ASSERT_TRUE (HttpErrc::InvalidMethod != error2);
}

/**
 * @brief Test make_error_code.
 */
TEST (HttpCategory, make_error_code)
{
    auto code = make_error_code (HttpErrc::InvalidRequest);
    ASSERT_EQ (code, HttpErrc::InvalidRequest);
    ASSERT_STREQ (code.message ().c_str (), "invalid HTTP request");
}

/**
 * @brief Test make_error_condition.
 */
TEST (HttpCategory, make_error_condition)
{
    auto code = make_error_condition (HttpErrc::InvalidMethod);
    ASSERT_EQ (code, HttpErrc::InvalidMethod);
    ASSERT_STREQ (code.message ().c_str (), "invalid HTTP method");
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
