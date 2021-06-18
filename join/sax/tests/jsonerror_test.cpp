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
#include <join/json.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::sax::JsonErrc;
using join::sax::JsonCategory;

/**
 * @brief Test name.
 */
TEST (JsonCategory, name)
{
    EXPECT_STREQ (JsonCategory ().name (), "join");
}

/**
 * @brief Test message.
 */
TEST (JsonCategory, message)
{
    EXPECT_STREQ (JsonCategory ().message (0).c_str (), "success");
    EXPECT_STREQ (JsonCategory ().message (static_cast <int> (JsonErrc::InvalidComment)).c_str (), "comment is invalid");
    EXPECT_STREQ (JsonCategory ().message (static_cast <int> (JsonErrc::InvalidEscaping)).c_str (), "character escaping is invalid");
    EXPECT_STREQ (JsonCategory ().message (static_cast <int> (JsonErrc::InvalidEncoding)).c_str (), "character encoding is invalid");
    EXPECT_STREQ (JsonCategory ().message (static_cast <int> (JsonErrc::IllegalCharacter)).c_str (), "illegal character");
    EXPECT_STREQ (JsonCategory ().message (static_cast <int> (JsonErrc::MissingCurlyBracket)).c_str (), "missing curly bracket");
    EXPECT_STREQ (JsonCategory ().message (static_cast <int> (JsonErrc::MissingSquareBracket)).c_str (), "missing square bracket");
    EXPECT_STREQ (JsonCategory ().message (static_cast <int> (JsonErrc::MissingQuote)).c_str (), "missing quote");
    EXPECT_STREQ (JsonCategory ().message (static_cast <int> (JsonErrc::MissingColon)).c_str (), "missing colon");
    EXPECT_STREQ (JsonCategory ().message (static_cast <int> (JsonErrc::MissingComma)).c_str (), "missing comma");
    EXPECT_STREQ (JsonCategory ().message (static_cast <int> (JsonErrc::EndOfFile)).c_str (), "end of file");
}

/**
 * @brief Test default_error_condition.
 */
TEST (JsonCategory, default_error_condition)
{
    EXPECT_EQ (JsonCategory ().default_error_condition (0).message(), "success");
    EXPECT_EQ (JsonCategory ().default_error_condition (1).message(), "comment is invalid");
    EXPECT_EQ (JsonCategory ().default_error_condition (2).message(), "character escaping is invalid");
    EXPECT_EQ (JsonCategory ().default_error_condition (3).message(), "character encoding is invalid");
    EXPECT_EQ (JsonCategory ().default_error_condition (4).message(), "illegal character");
    EXPECT_EQ (JsonCategory ().default_error_condition (5).message(), "missing curly bracket");
    EXPECT_EQ (JsonCategory ().default_error_condition (6).message(), "missing square bracket");
    EXPECT_EQ (JsonCategory ().default_error_condition (7).message(), "missing quote");
    EXPECT_EQ (JsonCategory ().default_error_condition (8).message(), "missing colon");
    EXPECT_EQ (JsonCategory ().default_error_condition (9).message(), "missing comma");
    EXPECT_EQ (JsonCategory ().default_error_condition (10).message(), "end of file");
}

/**
 * @brief Test equal.
 */
TEST (JsonCategory, equal)
{
    std::error_code error1, error2;

    error1 = make_error_code (JsonErrc::InvalidComment);
    error2 = make_error_code (JsonErrc::InvalidComment);
    ASSERT_TRUE (error1 == error2);

    error1 = make_error_code (JsonErrc::InvalidComment);
    error2 = make_error_code (JsonErrc::InvalidEscaping);
    ASSERT_FALSE (error1 == error2);

    error1 = make_error_code (JsonErrc::InvalidComment);
    ASSERT_TRUE (error1 == JsonErrc::InvalidComment);

    error1 = make_error_code (JsonErrc::InvalidComment);
    ASSERT_FALSE (error1 == JsonErrc::MissingComma);

    error2 = make_error_code (JsonErrc::MissingComma);
    ASSERT_TRUE (JsonErrc::MissingComma == error2);

    error2 = make_error_code (JsonErrc::MissingComma);
    ASSERT_FALSE (JsonErrc::InvalidComment == error2);
}

/**
 * @brief Test different.
 */
TEST (JsonCategory, different)
{
    std::error_code error1, error2;

    error1 = make_error_code (JsonErrc::InvalidComment);
    error2 = make_error_code (JsonErrc::InvalidComment);
    ASSERT_FALSE (error1 != error2);

    error1 = make_error_code (JsonErrc::InvalidComment);
    error2 = make_error_code (JsonErrc::InvalidEscaping);
    ASSERT_TRUE (error1 != error2);

    error1 = make_error_code (JsonErrc::InvalidComment);
    ASSERT_FALSE (error1 != JsonErrc::InvalidComment);

    error1 = make_error_code (JsonErrc::InvalidComment);
    ASSERT_TRUE (error1 != JsonErrc::MissingComma);

    error2 = make_error_code (JsonErrc::MissingComma);
    ASSERT_FALSE (JsonErrc::MissingComma != error2);

    error2 = make_error_code (JsonErrc::MissingComma);
    ASSERT_TRUE (JsonErrc::InvalidComment != error2);
}

/**
 * @brief Test make_error_code.
 */
TEST (JsonCategory, make_error_code)
{
    auto code = make_error_code (JsonErrc::IllegalCharacter);
    EXPECT_EQ (code, JsonErrc::IllegalCharacter);
    EXPECT_STREQ (code.message ().c_str (), "illegal character");
}

/**
 * @brief Test make_error_condition.
 */
TEST (JsonCategory, make_error_condition)
{
    auto code = make_error_condition (JsonErrc::MissingQuote);
    EXPECT_EQ (code, JsonErrc::MissingQuote);
    EXPECT_STREQ (code.message ().c_str (), "missing quote");
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
