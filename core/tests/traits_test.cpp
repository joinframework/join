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
#include <join/traits.hpp>

// Libraries.
#include <gtest/gtest.h>

/**
 * @brief test match trait.
 */
TEST (Traits, match)
{
    auto match = std::is_same_v<int, join::match_t<int8_t&&, int, double, std::string, bool, std::nullptr_t>>;
    EXPECT_TRUE (match);
    match = std::is_same_v<int, join::match_t<uint8_t&&, int, double, std::string, bool, std::nullptr_t>>;
    EXPECT_TRUE (match);
    match = std::is_same_v<int, join::match_t<int16_t&&, int, double, std::string, bool, std::nullptr_t>>;
    EXPECT_TRUE (match);
    match = std::is_same_v<int, join::match_t<uint16_t&&, int, double, std::string, bool, std::nullptr_t>>;
    EXPECT_TRUE (match);
    match = std::is_same_v<int, join::match_t<int32_t&&, int, double, std::string, bool, std::nullptr_t>>;
    EXPECT_TRUE (match);

    match = std::is_same_v<double, join::match_t<float&&, int, double, std::string, bool, std::nullptr_t>>;
    EXPECT_TRUE (match);
    match = std::is_same_v<double, join::match_t<double&&, int, double, std::string, bool, std::nullptr_t>>;
    EXPECT_TRUE (match);

    match = std::is_same_v<std::string, join::match_t<std::string&&, int, double, std::string, bool, std::nullptr_t>>;
    EXPECT_TRUE (match);

    match = std::is_same_v<bool, join::match_t<bool&&, int, double, std::string, bool, std::nullptr_t>>;
    EXPECT_TRUE (match);

    match =
        std::is_same_v<std::nullptr_t, join::match_t<std::nullptr_t&&, int, double, std::string, bool, std::nullptr_t>>;
    EXPECT_TRUE (match);
}

/**
 * @brief test find_index_v trait.
 */
TEST (Traits, find_index_v)
{
    auto index = join::find_index_v<int, int, double, std::string, bool, std::nullptr_t>;
    EXPECT_EQ (index, 0);

    index = join::find_index_v<double, int, double, std::string, bool, std::nullptr_t>;
    EXPECT_EQ (index, 1);

    index = join::find_index_v<std::string, int, double, std::string, bool, std::nullptr_t>;
    EXPECT_EQ (index, 2);

    index = join::find_index_v<bool, int, double, std::string, bool, std::nullptr_t>;
    EXPECT_EQ (index, 3);

    index = join::find_index_v<std::nullptr_t, int, double, std::string, bool, std::nullptr_t>;
    EXPECT_EQ (index, 4);
}

/**
 * @brief test count_v trait.
 */
TEST (Traits, count_v)
{
    auto num = join::count_v<int, double, std::string, bool, std::nullptr_t>;
    EXPECT_EQ (num, 0);

    num = join::count_v<int, int, std::string, bool, std::nullptr_t>;
    EXPECT_EQ (num, 1);

    num = join::count_v<int, int, int, bool, std::nullptr_t>;
    EXPECT_EQ (num, 2);

    num = join::count_v<int, int, int, int, std::nullptr_t>;
    EXPECT_EQ (num, 3);

    num = join::count_v<int, int, int, int, int>;
    EXPECT_EQ (num, 4);
}

/**
 * @brief test is_unique_v trait.
 */
TEST (Traits, is_unique_v)
{
    auto unique = join::is_unique_v<int, double, std::string, bool, std::nullptr_t>;
    EXPECT_FALSE (unique);

    unique = join::is_unique_v<int, int, std::string, bool, std::nullptr_t>;
    EXPECT_TRUE (unique);

    unique = join::is_unique_v<int, int, int, bool, std::nullptr_t>;
    EXPECT_FALSE (unique);

    unique = join::is_unique_v<int, int, int, int, std::nullptr_t>;
    EXPECT_FALSE (unique);

    unique = join::is_unique_v<int, int, int, int, int>;
    EXPECT_FALSE (unique);
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
