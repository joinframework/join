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
    ASSERT_STREQ (HttpCategory ().message (static_cast <int> (HttpErrc::BadRequest)).c_str (), "bad request");
    ASSERT_STREQ (HttpCategory ().message (static_cast <int> (HttpErrc::Unauthorized)).c_str (), "unauthorized");
    ASSERT_STREQ (HttpCategory ().message (static_cast <int> (HttpErrc::Forbidden)).c_str (),  "forbidden");
    ASSERT_STREQ (HttpCategory ().message (static_cast <int> (HttpErrc::NotFound)).c_str (),  "not found");
    ASSERT_STREQ (HttpCategory ().message (static_cast <int> (HttpErrc::ForbiddenMethod)).c_str (), "method not allowed");
    ASSERT_STREQ (HttpCategory ().message (static_cast <int> (HttpErrc::LengthRequired)).c_str (), "length required");
    ASSERT_STREQ (HttpCategory ().message (static_cast <int> (HttpErrc::PayloadTooLarge)).c_str (),  "payload too large");
    ASSERT_STREQ (HttpCategory ().message (static_cast <int> (HttpErrc::UriTooLong)).c_str (),  "URI too long");
    ASSERT_STREQ (HttpCategory ().message (static_cast <int> (HttpErrc::HeaderTooLarge)).c_str (), "request header too large");
    ASSERT_STREQ (HttpCategory ().message (static_cast <int> (HttpErrc::ServerError)).c_str (), "internal server error");
    ASSERT_STREQ (HttpCategory ().message (static_cast <int> (HttpErrc::NotImplemented)).c_str (),  "not implemented");
    ASSERT_STREQ (HttpCategory ().message (static_cast <int> (HttpErrc::BadGateway)).c_str (),  "bad gateway");
}

/**
 * @brief Test default_error_condition.
 */
TEST (HttpCategory, default_error_condition)
{
    ASSERT_EQ (HttpCategory ().default_error_condition (0).message(), "success");
    ASSERT_EQ (HttpCategory ().default_error_condition (400).message(), "bad request");
    ASSERT_EQ (HttpCategory ().default_error_condition (401).message(), "unauthorized");
    ASSERT_EQ (HttpCategory ().default_error_condition (403).message(), "forbidden");
    ASSERT_EQ (HttpCategory ().default_error_condition (404).message(), "not found");
    ASSERT_EQ (HttpCategory ().default_error_condition (405).message(), "method not allowed");
    ASSERT_EQ (HttpCategory ().default_error_condition (411).message(), "length required");
    ASSERT_EQ (HttpCategory ().default_error_condition (413).message(), "payload too large");
    ASSERT_EQ (HttpCategory ().default_error_condition (414).message(), "URI too long");
    ASSERT_EQ (HttpCategory ().default_error_condition (494).message(), "request header too large");
    ASSERT_EQ (HttpCategory ().default_error_condition (500).message(), "internal server error");
    ASSERT_EQ (HttpCategory ().default_error_condition (501).message(), "not implemented");
    ASSERT_EQ (HttpCategory ().default_error_condition (502).message(), "bad gateway");
}

/**
 * @brief Test equal.
 */
TEST (HttpCategory, equal)
{
    std::error_code error1, error2;

    error1 = make_error_code (HttpErrc::BadRequest);
    error2 = make_error_code (HttpErrc::BadRequest);
    ASSERT_TRUE (error1 == error2);

    error2 = make_error_code (HttpErrc::ForbiddenMethod);
    ASSERT_FALSE (error1 == error2);

    error1 = make_error_code (HttpErrc::ForbiddenMethod);
    ASSERT_TRUE (error1 == error2);

    error2 = make_error_code (HttpErrc::HeaderTooLarge);
    ASSERT_FALSE (error1 == error2);

    error1 = make_error_code (HttpErrc::HeaderTooLarge);
    ASSERT_TRUE (error1 == error2);

    error1 = make_error_code (HttpErrc::BadRequest);
    ASSERT_TRUE (error1 == HttpErrc::BadRequest);

    error1 = make_error_code (HttpErrc::ForbiddenMethod);
    ASSERT_FALSE (error1 == HttpErrc::BadRequest);

    error2 = make_error_code (HttpErrc::ForbiddenMethod);
    ASSERT_TRUE (HttpErrc::ForbiddenMethod == error2);

    error2 = make_error_code (HttpErrc::HeaderTooLarge);
    ASSERT_FALSE (HttpErrc::ForbiddenMethod == error2);
}

/**
 * @brief Test different.
 */
TEST (HttpCategory, different)
{
    std::error_code error1, error2;

    error1 = make_error_code (HttpErrc::BadRequest);
    error2 = make_error_code (HttpErrc::BadRequest);
    ASSERT_FALSE (error1 != error2);

    error2 = make_error_code (HttpErrc::ForbiddenMethod);
    ASSERT_TRUE (error1 != error2);

    error1 = make_error_code (HttpErrc::ForbiddenMethod);
    ASSERT_FALSE (error1 != error2);

    error2 = make_error_code (HttpErrc::HeaderTooLarge);
    ASSERT_TRUE (error1 != error2);

    error1 = make_error_code (HttpErrc::HeaderTooLarge);
    ASSERT_FALSE (error1 != error2);

    error1 = make_error_code (HttpErrc::BadRequest);
    ASSERT_FALSE (error1 != HttpErrc::BadRequest);

    error1 = make_error_code (HttpErrc::ForbiddenMethod);
    ASSERT_TRUE (error1 != HttpErrc::BadRequest);

    error2 = make_error_code (HttpErrc::ForbiddenMethod);
    ASSERT_FALSE (HttpErrc::ForbiddenMethod != error2);

    error2 = make_error_code (HttpErrc::HeaderTooLarge);
    ASSERT_TRUE (HttpErrc::ForbiddenMethod != error2);
}

/**
 * @brief Test make_error_code.
 */
TEST (HttpCategory, make_error_code)
{
    auto code = make_error_code (HttpErrc::BadRequest);
    ASSERT_EQ (code, HttpErrc::BadRequest);
    ASSERT_STREQ (code.message ().c_str (), "bad request");
}

/**
 * @brief Test make_error_condition.
 */
TEST (HttpCategory, make_error_condition)
{
    auto code = make_error_condition (HttpErrc::ForbiddenMethod);
    ASSERT_EQ (code, HttpErrc::ForbiddenMethod);
    ASSERT_STREQ (code.message ().c_str (), "method not allowed");
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
