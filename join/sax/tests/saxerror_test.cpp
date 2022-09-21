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
#include <join/sax.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::SaxErrc;
using join::SaxCategory;

/**
 * @brief Test name.
 */
TEST (SaxCategory, name)
{
    EXPECT_STREQ (SaxCategory ().name (), "libjoin");
}

/**
 * @brief Test message.
 */
TEST (SaxCategory, message)
{
    EXPECT_STREQ (SaxCategory ().message (0).c_str (), "success");
    EXPECT_STREQ (SaxCategory ().message (static_cast <int> (SaxErrc::StackOverflow)).c_str (), "stack overflow");
    EXPECT_STREQ (SaxCategory ().message (static_cast <int> (SaxErrc::InvalidParent)).c_str (), "parent not an array nor an object");
    EXPECT_STREQ (SaxCategory ().message (static_cast <int> (SaxErrc::InvalidValue)).c_str (), "value is invalid");
    EXPECT_STREQ (SaxCategory ().message (static_cast <int> (SaxErrc::ExtraData)).c_str (), "extra data detected");
}

/**
 * @brief Test default_error_condition.
 */
TEST (SaxCategory, default_error_condition)
{
    EXPECT_EQ (SaxCategory ().default_error_condition (0).message(), "success");
    EXPECT_EQ (SaxCategory ().default_error_condition (1).message(), "stack overflow");
    EXPECT_EQ (SaxCategory ().default_error_condition (2).message(), "parent not an array nor an object");
    EXPECT_EQ (SaxCategory ().default_error_condition (3).message(), "value is invalid");
    EXPECT_EQ (SaxCategory ().default_error_condition (4).message(), "extra data detected");
}

/**
 * @brief Test equal.
 */
TEST (SaxCategory, equal)
{
    std::error_code error1, error2;

    error1 = make_error_code (SaxErrc::StackOverflow);
    error2 = make_error_code (SaxErrc::StackOverflow);
    ASSERT_TRUE (error1 == error2);

    error1 = make_error_code (SaxErrc::StackOverflow);
    error2 = make_error_code (SaxErrc::InvalidParent);
    ASSERT_FALSE (error1 == error2);

    error1 = make_error_code (SaxErrc::StackOverflow);
    ASSERT_TRUE (error1 == SaxErrc::StackOverflow);

    error1 = make_error_code (SaxErrc::StackOverflow);
    ASSERT_FALSE (error1 == SaxErrc::InvalidValue);

    error2 = make_error_code (SaxErrc::InvalidValue);
    ASSERT_TRUE (SaxErrc::InvalidValue == error2);

    error2 = make_error_code (SaxErrc::InvalidValue);
    ASSERT_FALSE (SaxErrc::StackOverflow == error2);
}

/**
 * @brief Test different.
 */
TEST (SaxCategory, different)
{
    std::error_code error1, error2;

    error1 = make_error_code (SaxErrc::StackOverflow);
    error2 = make_error_code (SaxErrc::StackOverflow);
    ASSERT_FALSE (error1 != error2);

    error1 = make_error_code (SaxErrc::StackOverflow);
    error2 = make_error_code (SaxErrc::InvalidParent);
    ASSERT_TRUE (error1 != error2);

    error1 = make_error_code (SaxErrc::StackOverflow);
    ASSERT_FALSE (error1 != SaxErrc::StackOverflow);

    error1 = make_error_code (SaxErrc::StackOverflow);
    ASSERT_TRUE (error1 != SaxErrc::InvalidValue);

    error2 = make_error_code (SaxErrc::InvalidValue);
    ASSERT_FALSE (SaxErrc::InvalidValue != error2);

    error2 = make_error_code (SaxErrc::InvalidValue);
    ASSERT_TRUE (SaxErrc::StackOverflow != error2);
}

/**
 * @brief Test make_error_code.
 */
TEST (SaxCategory, make_error_code)
{
    auto code = make_error_code (SaxErrc::ExtraData);
    EXPECT_EQ (code, SaxErrc::ExtraData);
    EXPECT_STREQ (code.message ().c_str (), "extra data detected");
}

/**
 * @brief Test make_error_condition.
 */
TEST (SaxCategory, make_error_condition)
{
    auto code = make_error_condition (SaxErrc::InvalidParent);
    EXPECT_EQ (code, SaxErrc::InvalidParent);
    EXPECT_STREQ (code.message ().c_str (), "parent not an array nor an object");
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}


