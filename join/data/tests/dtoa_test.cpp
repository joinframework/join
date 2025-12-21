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
#include <join/dtoa.hpp>

// libraries.
#include <gtest/gtest.h>

/**
 * @brief digitsCount test.
 */
TEST (dtoa, digitsCount)
{
    EXPECT_EQ (join::details::digitsCount (1), 1);
    EXPECT_EQ (join::details::digitsCount (11), 2);
    EXPECT_EQ (join::details::digitsCount (111), 3);
    EXPECT_EQ (join::details::digitsCount (1111), 4);
    EXPECT_EQ (join::details::digitsCount (11111), 5);
    EXPECT_EQ (join::details::digitsCount (111111), 6);
    EXPECT_EQ (join::details::digitsCount (1111111), 7);
    EXPECT_EQ (join::details::digitsCount (11111111), 8);
    EXPECT_EQ (join::details::digitsCount (111111111), 9);
    EXPECT_EQ (join::details::digitsCount (1111111111), 10);
}

/**
 * @brief dtoa test.
 */
TEST (dtoa, dtoa)
{
    char beg [25] = {};

    char* end = join::dtoa (beg, 0.0);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "0.0");

    end = join::dtoa (beg, -0.0);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "-0.0");

    end = join::dtoa (beg, 0.1);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "0.1");

    end = join::dtoa (beg, 0.12);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "0.12");

    end = join::dtoa (beg, 0.123);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "0.123");

    end = join::dtoa (beg, 0.1234);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "0.1234");

    end = join::dtoa (beg, 1.0);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "1.0");

    end = join::dtoa (beg, 1.1234);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "1.1234");

    end = join::dtoa (beg, 1.5);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "1.5");

    end = join::dtoa (beg, -1.5);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "-1.5");

    end = join::dtoa (beg, 3.1416);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "3.1416");

    end = join::dtoa (beg, 1E10);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "10000000000.0");

    end = join::dtoa (beg, 1e10);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "10000000000.0");

    end = join::dtoa (beg, 1E+10);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "10000000000.0");

    end = join::dtoa (beg, 1E-10);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "1e-10");

    end = join::dtoa (beg, -1E10);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "-10000000000.0");

    end = join::dtoa (beg, -1e10);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "-10000000000.0");

    end = join::dtoa (beg, -1E+10);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "-10000000000.0");

    end = join::dtoa (beg, -1E-10);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "-1e-10");

    end = join::dtoa (beg, 1.234E+10);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "12340000000.0");

    end = join::dtoa (beg, 1.234E-10);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "1.234e-10");

    end = join::dtoa (beg, 1.79769e+308);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "1.79769e308");

    end = join::dtoa (beg, 2.22507e-308);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "2.22507e-308");

    end = join::dtoa (beg, -1.79769e+308);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "-1.79769e308");

    end = join::dtoa (beg, -2.22507e-308);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "-2.22507e-308");

    end = join::dtoa (beg, -4.9406564584124654e-324);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "-5e-324");

    end = join::dtoa (beg, 2.2250738585072009e-308);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "2.225073858507201e-308");

    end = join::dtoa (beg, 2.2250738585072014e-308);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "2.2250738585072014e-308");

    end = join::dtoa (beg, 1.7976931348623157e+308);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "1.7976931348623157e308");

    end = join::dtoa (beg, 18446744073709551616.0);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "18446744073709552000.0");

    end = join::dtoa (beg, -9223372036854775809.0);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "-9223372036854776000.0");

    end = join::dtoa (beg, 0.9868011474609375);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "0.9868011474609375");

    end = join::dtoa (beg, 123e34);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "1.23e36");

    end = join::dtoa (beg, 45913141877270640000.0);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "45913141877270640000.0");

    end = join::dtoa (beg, 2.2250738585072011e-308);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "2.225073858507201e-308");

    end = join::dtoa (beg, 72057594037927928.0);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "72057594037927930.0");

    end = join::dtoa (beg, 72057594037927936.0);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "72057594037927940.0");

    end = join::dtoa (beg, 72057594037927936.0);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "72057594037927940.0");

    end = join::dtoa (beg, 9223372036854774784.0);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "9223372036854775000.0");

    end = join::dtoa (beg, 9223372036854775808.0);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "9223372036854776000.0");

    end = join::dtoa (beg, 10141204801825834086073718800384.0);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "1.0141204801825834e31");

    end = join::dtoa (beg, 10141204801825835211973625643008.0);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "1.0141204801825835e31");

    end = join::dtoa (beg, 5708990770823838890407843763683279797179383808.0);
    ASSERT_NE (end, nullptr);
    EXPECT_STREQ (std::string (beg, end - beg).c_str (), "5.708990770823839e45");
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
