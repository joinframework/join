/**
 * MIT License
 *
 * Copyright (c) 2026 Mathieu Rabine
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
#include <join/view.hpp>

// Libraries.
#include <gtest/gtest.h>

// C++.
#include <type_traits>
#include <compare>
#include <cmath>

using join::View;

/**
 * @brief test layout.
 */
TEST (View, layout)
{
    EXPECT_EQ (sizeof (View<int>), sizeof (int*) + sizeof (uint32_t));
    EXPECT_TRUE ((std::is_trivially_copyable_v<View<int>>));
}

/**
 * @brief test create.
 */
TEST (View, create)
{
    static constexpr int values[] = {1, 2, 3};
    static_assert (View<const int> ().empty ());
    static_assert (View<const int> (values, 3)[2] == 3);
    static_assert (View<const int> (values, 3) == View<const int> (values, 3));
    static_assert ((View<const int> (values, 2) <=> View<const int> (values, 3)) < 0);

    View<const int> view (values, 3);
    EXPECT_EQ (view.data (), values);
    EXPECT_EQ (view.size (), 3);
}

/**
 * @brief test data.
 */
TEST (View, data)
{
    int values[] = {1, 2, 3};
    EXPECT_EQ (View<int> ().data (), nullptr);
    EXPECT_EQ (View<int> (values, 3).data (), values);
}

/**
 * @brief test size.
 */
TEST (View, size)
{
    int values[] = {1, 2, 3};
    EXPECT_EQ (View<int> ().size (), 0);
    EXPECT_EQ (View<int> (values, 2).size (), 2);
}

/**
 * @brief test empty.
 */
TEST (View, empty)
{
    int values[] = {1, 2, 3};
    EXPECT_TRUE (View<int> ().empty ());
    EXPECT_TRUE (View<int> (values, 0).empty ());
    EXPECT_FALSE (View<int> (values, 3).empty ());
}

/**
 * @brief test begin.
 */
TEST (View, begin)
{
    int values[] = {1, 2, 3};
    View<int> view (values, 3);
    EXPECT_EQ (view.begin (), values);
    EXPECT_EQ (*view.begin (), 1);
}

/**
 * @brief test end.
 */
TEST (View, end)
{
    int values[] = {1, 2, 3};
    View<int> view (values, 3);
    EXPECT_EQ (view.end (), values + 3);

    int sum = 0;
    for (int value : view)
    {
        sum += value;
    }
    EXPECT_EQ (sum, 6);
}

/**
 * @brief test operator[].
 */
TEST (View, at)
{
    int values[] = {1, 2, 3};
    View<int> view (values, 3);
    EXPECT_EQ (view[0], 1);
    EXPECT_EQ (view[2], 3);
    view[1] = 5;
    EXPECT_EQ (values[1], 5);
}

/**
 * @brief test operator==.
 */
TEST (View, equal)
{
    int lhs[] = {1, 2, 3};
    int rhs[] = {1, 2, 3};
    EXPECT_TRUE (View<int> (lhs, 3) == View<int> (rhs, 3));
    EXPECT_FALSE (View<int> (lhs, 3) == View<int> (rhs, 2));
    rhs[2] = 4;
    EXPECT_FALSE (View<int> (lhs, 3) == View<int> (rhs, 3));
    EXPECT_TRUE (View<int> () == View<int> ());
    EXPECT_TRUE (View<int> () == View<int> (lhs, 0));

    double nan[] = {std::nan ("")};
    EXPECT_FALSE (View<double> (nan, 1) == View<double> (nan, 1));

    EXPECT_TRUE (View<const char> () == View<const char> ("", 0));
    EXPECT_TRUE (View<const char> ("abc", 3) == View<const char> ("abcd", 3));
    EXPECT_FALSE (View<const char> ("abc", 3) == View<const char> ("abd", 3));
    EXPECT_FALSE (View<const char> ("abc", 3) == View<const char> ("ab", 2));

    char text[] = "abc";
    EXPECT_TRUE (View<char> (text, 3) == View<char> (text, 3));
    EXPECT_FALSE (View<char> (text, 3) == View<char> (text, 2));
}

/**
 * @brief test operator<=>.
 */
TEST (View, compare)
{
    int lhs[] = {1, 2, 3};
    int rhs[] = {1, 2, 4};
    EXPECT_EQ (View<int> (lhs, 3) <=> View<int> (rhs, 3), std::partial_ordering::less);
    EXPECT_EQ (View<int> (rhs, 3) <=> View<int> (lhs, 3), std::partial_ordering::greater);
    EXPECT_EQ (View<int> (lhs, 2) <=> View<int> (rhs, 2), std::partial_ordering::equivalent);
    EXPECT_EQ (View<int> (lhs, 2) <=> View<int> (lhs, 3), std::partial_ordering::less);

    double nan[] = {std::nan ("")};
    EXPECT_EQ (View<double> (nan, 1) <=> View<double> (nan, 1), std::partial_ordering::unordered);

    EXPECT_EQ (View<const char> ("abc", 3) <=> View<const char> ("abd", 3), std::partial_ordering::less);
    EXPECT_EQ (View<const char> ("abd", 3) <=> View<const char> ("abc", 3), std::partial_ordering::greater);
    EXPECT_EQ (View<const char> ("abc", 3) <=> View<const char> ("abcd", 3), std::partial_ordering::equivalent);
    EXPECT_EQ (View<const char> ("ab", 2) <=> View<const char> ("abc", 3), std::partial_ordering::less);
    EXPECT_EQ (View<const char> ("\xC3", 1) <=> View<const char> ("a", 1), std::partial_ordering::greater);

    char high[] = "\xC3";
    char low[] = "a";
    EXPECT_EQ (View<char> (high, 1) <=> View<char> (low, 1), std::partial_ordering::greater);
    EXPECT_EQ (View<char> (low, 1) <=> View<char> (high, 1), std::partial_ordering::less);
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
