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
#include <join/digest.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::DigestErrc;
using join::DigestCategory;

/**
 * @brief Test name.
 */
TEST (DigestCategory, name)
{
    EXPECT_STREQ (DigestCategory ().name (), "libjoin");
}

/**
 * @brief Test message.
 */
TEST (DigestCategory, message)
{
    EXPECT_STREQ (DigestCategory ().message (0).c_str (), "success");
    EXPECT_STREQ (DigestCategory ().message (static_cast <int> (DigestErrc::InvalidAlgorithm)).c_str (), "invalid algorithm");
    EXPECT_STREQ (DigestCategory ().message (static_cast <int> (DigestErrc::InvalidKey)).c_str (), "invalid key");
    EXPECT_STREQ (DigestCategory ().message (static_cast <int> (DigestErrc::InvalidSignature)).c_str (), "invalid signature");
}

/**
 * @brief Test default_error_condition.
 */
TEST (DigestCategory, default_error_condition)
{
    EXPECT_EQ (DigestCategory ().default_error_condition (0).message(), "success");
    EXPECT_EQ (DigestCategory ().default_error_condition (1).message(), "invalid algorithm");
    EXPECT_EQ (DigestCategory ().default_error_condition (2).message(), "invalid key");
    EXPECT_EQ (DigestCategory ().default_error_condition (3).message(), "invalid signature");
}

/**
 * @brief Test equal.
 */
TEST (DigestCategory, equal)
{
    std::error_code error1, error2;

    error1 = make_error_code (DigestErrc::InvalidAlgorithm);
    error2 = make_error_code (DigestErrc::InvalidAlgorithm);
    ASSERT_TRUE (error1 == error2);

    error1 = make_error_code (DigestErrc::InvalidAlgorithm);
    error2 = make_error_code (DigestErrc::InvalidKey);
    ASSERT_FALSE (error1 == error2);

    error1 = make_error_code (DigestErrc::InvalidAlgorithm);
    ASSERT_TRUE (error1 == DigestErrc::InvalidAlgorithm);

    error1 = make_error_code (DigestErrc::InvalidAlgorithm);
    ASSERT_FALSE (error1 == DigestErrc::InvalidKey);

    error2 = make_error_code (DigestErrc::InvalidKey);
    ASSERT_TRUE (DigestErrc::InvalidKey == error2);

    error2 = make_error_code (DigestErrc::InvalidKey);
    ASSERT_FALSE (DigestErrc::InvalidAlgorithm == error2);
}

/**
 * @brief Test different.
 */
TEST (DigestCategory, different)
{
    std::error_code error1, error2;

    error1 = make_error_code (DigestErrc::InvalidAlgorithm);
    error2 = make_error_code (DigestErrc::InvalidAlgorithm);
    ASSERT_FALSE (error1 != error2);

    error1 = make_error_code (DigestErrc::InvalidAlgorithm);
    error2 = make_error_code (DigestErrc::InvalidKey);
    ASSERT_TRUE (error1 != error2);

    error1 = make_error_code (DigestErrc::InvalidAlgorithm);
    ASSERT_FALSE (error1 != DigestErrc::InvalidAlgorithm);

    error1 = make_error_code (DigestErrc::InvalidAlgorithm);
    ASSERT_TRUE (error1 != DigestErrc::InvalidKey);

    error2 = make_error_code (DigestErrc::InvalidKey);
    ASSERT_FALSE (DigestErrc::InvalidKey != error2);

    error2 = make_error_code (DigestErrc::InvalidKey);
    ASSERT_TRUE (DigestErrc::InvalidAlgorithm != error2);
}

/**
 * @brief Test make_error_code.
 */
TEST (DigestCategory, make_error_code)
{
    auto code = make_error_code (DigestErrc::InvalidSignature);
    EXPECT_EQ (code, DigestErrc::InvalidSignature);
    EXPECT_STREQ (code.message ().c_str (), "invalid signature");
}

/**
 * @brief Test make_error_condition.
 */
TEST (DigestCategory, make_error_condition)
{
    auto code = make_error_condition (DigestErrc::InvalidKey);
    EXPECT_EQ (code, DigestErrc::InvalidKey);
    EXPECT_STREQ (code.message ().c_str (), "invalid key");
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    join::initializeOpenSSL ();
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
