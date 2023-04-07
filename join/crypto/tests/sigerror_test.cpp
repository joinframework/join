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
#include <join/signature.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::SigErrc;
using join::SigCategory;

/**
 * @brief Test name.
 */
TEST (SigCategory, name)
{
    EXPECT_STREQ (SigCategory ().name (), "libjoin");
}

/**
 * @brief Test message.
 */
TEST (SigCategory, message)
{
    EXPECT_STREQ (SigCategory ().message (0).c_str (), "success");
    EXPECT_STREQ (SigCategory ().message (static_cast <int> (SigErrc::InvalidAlgorithm)).c_str (), "invalid algorithm");
    EXPECT_STREQ (SigCategory ().message (static_cast <int> (SigErrc::InvalidPrivateKey)).c_str (), "invalid private key");
    EXPECT_STREQ (SigCategory ().message (static_cast <int> (SigErrc::InvalidPublicKey)).c_str (), "invalid public key");
    EXPECT_STREQ (SigCategory ().message (static_cast <int> (SigErrc::InvalidSignature)).c_str (), "invalid signature");
}

/**
 * @brief Test default_error_condition.
 */
TEST (SigCategory, default_error_condition)
{
    EXPECT_EQ (SigCategory ().default_error_condition (0).message(), "success");
    EXPECT_EQ (SigCategory ().default_error_condition (1).message(), "invalid algorithm");
    EXPECT_EQ (SigCategory ().default_error_condition (2).message(), "invalid private key");
    EXPECT_EQ (SigCategory ().default_error_condition (3).message(), "invalid public key");
    EXPECT_EQ (SigCategory ().default_error_condition (4).message(), "invalid signature");
}

/**
 * @brief Test equal.
 */
TEST (SigCategory, equal)
{
    std::error_code error1, error2;

    error1 = make_error_code (SigErrc::InvalidAlgorithm);
    error2 = make_error_code (SigErrc::InvalidAlgorithm);
    ASSERT_TRUE (error1 == error2);

    error1 = make_error_code (SigErrc::InvalidAlgorithm);
    error2 = make_error_code (SigErrc::InvalidPrivateKey);
    ASSERT_FALSE (error1 == error2);

    error1 = make_error_code (SigErrc::InvalidAlgorithm);
    ASSERT_TRUE (error1 == SigErrc::InvalidAlgorithm);

    error1 = make_error_code (SigErrc::InvalidAlgorithm);
    ASSERT_FALSE (error1 == SigErrc::InvalidPublicKey);

    error2 = make_error_code (SigErrc::InvalidPublicKey);
    ASSERT_TRUE (SigErrc::InvalidPublicKey == error2);

    error2 = make_error_code (SigErrc::InvalidPublicKey);
    ASSERT_FALSE (SigErrc::InvalidAlgorithm == error2);
}

/**
 * @brief Test different.
 */
TEST (SigCategory, different)
{
    std::error_code error1, error2;

    error1 = make_error_code (SigErrc::InvalidAlgorithm);
    error2 = make_error_code (SigErrc::InvalidAlgorithm);
    ASSERT_FALSE (error1 != error2);

    error1 = make_error_code (SigErrc::InvalidAlgorithm);
    error2 = make_error_code (SigErrc::InvalidPrivateKey);
    ASSERT_TRUE (error1 != error2);

    error1 = make_error_code (SigErrc::InvalidAlgorithm);
    ASSERT_FALSE (error1 != SigErrc::InvalidAlgorithm);

    error1 = make_error_code (SigErrc::InvalidAlgorithm);
    ASSERT_TRUE (error1 != SigErrc::InvalidPublicKey);

    error2 = make_error_code (SigErrc::InvalidPublicKey);
    ASSERT_FALSE (SigErrc::InvalidPublicKey != error2);

    error2 = make_error_code (SigErrc::InvalidPublicKey);
    ASSERT_TRUE (SigErrc::InvalidAlgorithm != error2);
}

/**
 * @brief Test make_error_code.
 */
TEST (SigCategory, make_error_code)
{
    auto code = make_error_code (SigErrc::InvalidSignature);
    EXPECT_EQ (code, SigErrc::InvalidSignature);
    EXPECT_STREQ (code.message ().c_str (), "invalid signature");
}

/**
 * @brief Test make_error_condition.
 */
TEST (SigCategory, make_error_condition)
{
    auto code = make_error_condition (SigErrc::InvalidPrivateKey);
    EXPECT_EQ (code, SigErrc::InvalidPrivateKey);
    EXPECT_STREQ (code.message ().c_str (), "invalid private key");
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
