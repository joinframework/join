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

// C++.
#include <mutex>

/**
 * @brief test identity trait.
 */
TEST (Traits, identity)
{
    auto same = std::is_same <int, join::identity_t <int>>::value;
    EXPECT_TRUE (same);

    same = std::is_same <int, join::identity_t <double>>::value;
    EXPECT_FALSE (same);
}

/**
 * @brief test match trait.
 */
TEST (Traits, match)
{
    auto match = std::is_same <int, join::match_t <int8_t&&, int, double, std::string, bool, std::nullptr_t>>::value;
    EXPECT_TRUE (match);
    match = std::is_same <int, join::match_t <uint8_t&&, int, double, std::string, bool, std::nullptr_t>>::value;
    EXPECT_TRUE (match);
    match = std::is_same <int, join::match_t <int16_t&&, int, double, std::string, bool, std::nullptr_t>>::value;
    EXPECT_TRUE (match);
    match = std::is_same <int, join::match_t <uint16_t&&, int, double, std::string, bool, std::nullptr_t>>::value;
    EXPECT_TRUE (match);
    match = std::is_same <int, join::match_t <int32_t&&, int, double, std::string, bool, std::nullptr_t>>::value;
    EXPECT_TRUE (match);

    match = std::is_same <double, join::match_t <float&&, int, double, std::string, bool, std::nullptr_t>>::value;
    EXPECT_TRUE (match);
    match = std::is_same <double, join::match_t <double&&, int, double, std::string, bool, std::nullptr_t>>::value;
    EXPECT_TRUE (match);

    match = std::is_same <std::string, join::match_t <std::string&&, int, double, std::string, bool, std::nullptr_t>>::value;
    EXPECT_TRUE (match);

    match = std::is_same <bool, join::match_t <bool&&, int, double, std::string, bool, std::nullptr_t>>::value;
    EXPECT_TRUE (match);

    match = std::is_same <std::nullptr_t, join::match_t <std::nullptr_t&&, int, double, std::string, bool, std::nullptr_t>>::value;
    EXPECT_TRUE (match);
}

/**
 * @brief test find_index trait.
 */
TEST (Traits, find_index)
{
    auto index = join::find_index <int, int, double, std::string, bool, std::nullptr_t>::value;
    EXPECT_EQ (index, 0);

    index = join::find_index <double, int, double, std::string, bool, std::nullptr_t>::value;
    EXPECT_EQ (index, 1);

    index = join::find_index <std::string, int, double, std::string, bool, std::nullptr_t>::value;
    EXPECT_EQ (index, 2);

    index = join::find_index <bool, int, double, std::string, bool, std::nullptr_t>::value;
    EXPECT_EQ (index, 3);

    index = join::find_index <std::nullptr_t, int, double, std::string, bool, std::nullptr_t>::value;
    EXPECT_EQ (index, 4);
}

/**
 * @brief test find_element trait.
 */
TEST (Traits, find_element)
{
    auto same = std::is_same <int, join::find_element_t <0, int, double, std::string, bool, std::nullptr_t>>::value;
    EXPECT_TRUE (same);
    same = std::is_same <int, join::find_element_t <1, int, double, std::string, bool, std::nullptr_t>>::value;
    EXPECT_FALSE (same);
    same = std::is_same <int, join::find_element_t <2, int, double, std::string, bool, std::nullptr_t>>::value;
    EXPECT_FALSE (same);
    same = std::is_same <int, join::find_element_t <3, int, double, std::string, bool, std::nullptr_t>>::value;
    EXPECT_FALSE (same);
    same = std::is_same <int, join::find_element_t <4, int, double, std::string, bool, std::nullptr_t>>::value;
    EXPECT_FALSE (same);

    same = std::is_same <double, join::find_element_t <0, int, double, std::string, bool, std::nullptr_t>>::value;
    EXPECT_FALSE (same);
    same = std::is_same <double, join::find_element_t <1, int, double, std::string, bool, std::nullptr_t>>::value;
    EXPECT_TRUE (same);
    same = std::is_same <double, join::find_element_t <2, int, double, std::string, bool, std::nullptr_t>>::value;
    EXPECT_FALSE (same);
    same = std::is_same <double, join::find_element_t <3, int, double, std::string, bool, std::nullptr_t>>::value;
    EXPECT_FALSE (same);
    same = std::is_same <double, join::find_element_t <4, int, double, std::string, bool, std::nullptr_t>>::value;
    EXPECT_FALSE (same);

    same = std::is_same <std::string, join::find_element_t <0, int, double, std::string, bool, std::nullptr_t>>::value;
    EXPECT_FALSE (same);
    same = std::is_same <std::string, join::find_element_t <1, int, double, std::string, bool, std::nullptr_t>>::value;
    EXPECT_FALSE (same);
    same = std::is_same <std::string, join::find_element_t <2, int, double, std::string, bool, std::nullptr_t>>::value;
    EXPECT_TRUE (same);
    same = std::is_same <std::string, join::find_element_t <3, int, double, std::string, bool, std::nullptr_t>>::value;
    EXPECT_FALSE (same);
    same = std::is_same <std::string, join::find_element_t <4, int, double, std::string, bool, std::nullptr_t>>::value;
    EXPECT_FALSE (same);

    same = std::is_same <bool, join::find_element_t <0, int, double, std::string, bool, std::nullptr_t>>::value;
    EXPECT_FALSE (same);
    same = std::is_same <bool, join::find_element_t <1, int, double, std::string, bool, std::nullptr_t>>::value;
    EXPECT_FALSE (same);
    same = std::is_same <bool, join::find_element_t <2, int, double, std::string, bool, std::nullptr_t>>::value;
    EXPECT_FALSE (same);
    same = std::is_same <bool, join::find_element_t <3, int, double, std::string, bool, std::nullptr_t>>::value;
    EXPECT_TRUE (same);
    same = std::is_same <bool, join::find_element_t <4, int, double, std::string, bool, std::nullptr_t>>::value;
    EXPECT_FALSE (same);

    same = std::is_same <std::nullptr_t, join::find_element_t <0, int, double, std::string, bool, std::nullptr_t>>::value;
    EXPECT_FALSE (same);
    same = std::is_same <std::nullptr_t, join::find_element_t <1, int, double, std::string, bool, std::nullptr_t>>::value;
    EXPECT_FALSE (same);
    same = std::is_same <std::nullptr_t, join::find_element_t <2, int, double, std::string, bool, std::nullptr_t>>::value;
    EXPECT_FALSE (same);
    same = std::is_same <std::nullptr_t, join::find_element_t <3, int, double, std::string, bool, std::nullptr_t>>::value;
    EXPECT_FALSE (same);
    same = std::is_same <std::nullptr_t, join::find_element_t <4, int, double, std::string, bool, std::nullptr_t>>::value;
    EXPECT_TRUE (same);
}

/**
 * @brief test is_alternative trait.
 */
TEST (Traits, is_alternative)
{
    auto valid = join::is_alternative <int, int, double, std::string, bool, std::nullptr_t>::value;
    EXPECT_TRUE (valid);

    valid = join::is_alternative <double, int, double, std::string, bool, std::nullptr_t>::value;
    EXPECT_TRUE (valid);

    valid = join::is_alternative <std::string, int, double, std::string, bool, std::nullptr_t>::value;
    EXPECT_TRUE (valid);

    valid = join::is_alternative <bool, int, double, std::string, bool, std::nullptr_t>::value;
    EXPECT_TRUE (valid);

    valid = join::is_alternative <std::nullptr_t, int, double, std::string, bool, std::nullptr_t>::value;
    EXPECT_TRUE (valid);

    valid = join::is_alternative <int64_t, int, double, std::string, bool, std::nullptr_t>::value;
    EXPECT_FALSE (valid);

    valid = join::is_alternative <float, int, double, std::string, bool, std::nullptr_t>::value;
    EXPECT_FALSE (valid);

    valid = join::is_alternative <std::fstream, int, double, std::string, bool, std::nullptr_t>::value;
    EXPECT_FALSE (valid);

    valid = join::is_alternative <void, int, double, std::string, bool, std::nullptr_t>::value;
    EXPECT_FALSE (valid);
}

/**
 * @brief test is_index trait.
 */
TEST (Traits, is_index)
{
    auto valid = join::is_index <0, int, double, std::string, bool, std::nullptr_t>::value;
    EXPECT_TRUE (valid);

    valid = join::is_index <1, int, double, std::string, bool, std::nullptr_t>::value;
    EXPECT_TRUE (valid);

    valid = join::is_index <2, int, double, std::string, bool, std::nullptr_t>::value;
    EXPECT_TRUE (valid);

    valid = join::is_index <3, int, double, std::string, bool, std::nullptr_t>::value;
    EXPECT_TRUE (valid);

    valid = join::is_index <4, int, double, std::string, bool, std::nullptr_t>::value;
    EXPECT_TRUE (valid);

    valid = join::is_index <5, int, double, std::string, bool, std::nullptr_t>::value;
    EXPECT_FALSE (valid);

    valid = join::is_index <6, int, double, std::string, bool, std::nullptr_t>::value;
    EXPECT_FALSE (valid);
}

/**
 * @brief test count trait.
 */
TEST (Traits, count)
{
    auto num = join::count <int, double, std::string, bool, std::nullptr_t>::value;
    EXPECT_EQ (num, 0);

    num = join::count <int, int, std::string, bool, std::nullptr_t>::value;
    EXPECT_EQ (num, 1);

    num = join::count <int, int, int, bool, std::nullptr_t>::value;
    EXPECT_EQ (num, 2);

    num = join::count <int, int, int, int, std::nullptr_t>::value;
    EXPECT_EQ (num, 3);

    num = join::count <int, int, int, int, int>::value;
    EXPECT_EQ (num, 4);
}

/**
 * @brief test unique trait.
 */
TEST (Traits, unique)
{
    auto unique = join::is_unique <int, double, std::string, bool, std::nullptr_t>::value;
    EXPECT_FALSE (unique);

    unique = join::is_unique <int, int, std::string, bool, std::nullptr_t>::value;
    EXPECT_TRUE (unique);

    unique = join::is_unique <int, int, int, bool, std::nullptr_t>::value;
    EXPECT_FALSE (unique);

    unique = join::is_unique <int, int, int, int, std::nullptr_t>::value;
    EXPECT_FALSE (unique);

    unique = join::is_unique <int, int, int, int, int>::value;
    EXPECT_FALSE (unique);
}

/**
 * @brief test all trait.
 */
TEST (Traits, all)
{
    auto all = join::all <true, true, true>::value;
    EXPECT_TRUE (all);

    all = join::all <true, false, true>::value;
    EXPECT_FALSE (all);

    all = join::all <false, false, false>::value;
    EXPECT_FALSE (all);
}

/**
 * @brief test are_copy_constructible trait.
 */
TEST (Traits, are_copy_constructible)
{
    auto can = join::are_copy_constructible <int, double, std::string, bool, std::nullptr_t>::value;
    EXPECT_TRUE (can);

    can = join::are_copy_constructible <std::mutex, int, double, std::string, bool, std::nullptr_t>::value;
    EXPECT_FALSE (can);
}

/**
 * @brief test are_move_constructible trait.
 */
TEST (Traits, are_move_constructible)
{
    auto can = join::are_move_constructible <int, double, std::string, bool, std::nullptr_t>::value;
    EXPECT_TRUE (can);

    can = join::are_move_constructible <std::mutex, int, double, std::string, bool, std::nullptr_t>::value;
    EXPECT_FALSE (can);
}

/**
 * @brief test are_copy_assignable trait.
 */
TEST (Traits, are_copy_assignable)
{
    auto can = join::are_copy_assignable <int, double, std::string, bool, std::nullptr_t>::value;
    EXPECT_TRUE (can);

    can = join::are_copy_assignable <std::mutex, int, double, std::string, bool, std::nullptr_t>::value;
    EXPECT_FALSE (can);
}

/**
 * @brief test are_move_assignable trait.
 */
TEST (Traits, are_move_assignable)
{
    auto can = join::are_move_assignable <int, double, std::string, bool, std::nullptr_t>::value;
    EXPECT_TRUE (can);

    can = join::are_move_assignable <std::mutex, int, double, std::string, bool, std::nullptr_t>::value;
    EXPECT_FALSE (can);
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
