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
#include <join/view.hpp>
 
// libraries.
#include <gtest/gtest.h>

using join::View;

/**
 * @brief create test.
 */
TEST (View, create)
{
    char msg[] = "hello world";

    View view1 (msg);
    ASSERT_NE (view1.data (), nullptr);
    ASSERT_EQ (view1.size (), 11);

    View view2 (msg, strlen (msg));
    ASSERT_NE (view2.data (), nullptr);
    ASSERT_EQ (view2.size (), 11);

    View view3 (msg, msg + strlen (msg));
    ASSERT_NE (view3.data (), nullptr);
    ASSERT_EQ (view3.size (), 11);
}

/**
 * @brief assign test.
 */
TEST (View, assign)
{
    View view = View ("other");
    ASSERT_NE (view.data (), nullptr);
    ASSERT_EQ (view.size (), 5);

    view = View ("hello world");
    ASSERT_NE (view.data (), nullptr);
    ASSERT_EQ (view.size (), 11);
}

/**
 * @brief data test.
 */
TEST (View, data)
{
    View view ("hello world");
    ASSERT_NE (view.data (), nullptr);
    ASSERT_EQ (*view.data (), 'h');
}

/**
 * @brief size test.
 */
TEST (View, size)
{
    View view ("hello world");
    ASSERT_EQ (view.size (), 11);
}

/**
 * @brief peek test.
 */
TEST (View, peek)
{
    View view ("hello world");
    ASSERT_EQ (view.size (), 11);
    ASSERT_EQ (view.peek (), 'h');
    ASSERT_EQ (view.get (), 'h');
    ASSERT_EQ (view.size (), 10);
    ASSERT_EQ (view.peek (), 'e');
    ASSERT_EQ (view.get (), 'e');
    ASSERT_EQ (view.size (), 9);
    ASSERT_EQ (view.peek (), 'l');
    ASSERT_EQ (view.get (), 'l');
    ASSERT_EQ (view.size (), 8);
    ASSERT_EQ (view.peek (), 'l');
    ASSERT_EQ (view.get (), 'l');
    ASSERT_EQ (view.size (), 7);
    ASSERT_EQ (view.peek (), 'o');
    ASSERT_EQ (view.get (), 'o');
    ASSERT_EQ (view.size (), 6);
    ASSERT_EQ (view.peek (), ' ');
    ASSERT_EQ (view.get (), ' ');
    ASSERT_EQ (view.size (), 5);
    ASSERT_EQ (view.peek (), 'w');
    ASSERT_EQ (view.get (), 'w');
    ASSERT_EQ (view.size (), 4);
    ASSERT_EQ (view.peek (), 'o');
    ASSERT_EQ (view.get (), 'o');
    ASSERT_EQ (view.size (), 3);
    ASSERT_EQ (view.peek (), 'r');
    ASSERT_EQ (view.get (), 'r');
    ASSERT_EQ (view.size (), 2);
    ASSERT_EQ (view.peek (), 'l');
    ASSERT_EQ (view.get (), 'l');
    ASSERT_EQ (view.size (), 1);
    ASSERT_EQ (view.peek (), 'd');
    ASSERT_EQ (view.get (), 'd');
    ASSERT_EQ (view.size (), 0);
    ASSERT_EQ (view.peek (), std::char_traits <char>::eof ());
    ASSERT_EQ (view.get (), std::char_traits <char>::eof ());
}

/**
 * @brief get test.
 */
TEST (View, get)
{
    View view ("hello world");
    ASSERT_EQ (view.size (), 11);
    ASSERT_EQ (view.get (), 'h');
    ASSERT_EQ (view.size (), 10);
    ASSERT_EQ (view.get (), 'e');
    ASSERT_EQ (view.size (), 9);
    ASSERT_EQ (view.get (), 'l');
    ASSERT_EQ (view.size (), 8);
    ASSERT_EQ (view.get (), 'l');
    ASSERT_EQ (view.size (), 7);
    ASSERT_EQ (view.get (), 'o');
    ASSERT_EQ (view.size (), 6);
    ASSERT_EQ (view.get (), ' ');
    ASSERT_EQ (view.size (), 5);
    ASSERT_EQ (view.get (), 'w');
    ASSERT_EQ (view.size (), 4);
    ASSERT_EQ (view.get (), 'o');
    ASSERT_EQ (view.size (), 3);
    ASSERT_EQ (view.get (), 'r');
    ASSERT_EQ (view.size (), 2);
    ASSERT_EQ (view.get (), 'l');
    ASSERT_EQ (view.size (), 1);
    ASSERT_EQ (view.get (), 'd');
    ASSERT_EQ (view.size (), 0);
    ASSERT_EQ (view.get (), std::char_traits <char>::eof ());
}

/**
 * @brief getIf test.
 */
TEST (View, getIf)
{
    View view ("hello world");
    ASSERT_EQ (view.size (), 11);
    ASSERT_FALSE (view.getIf ('x'));
    ASSERT_EQ (view.size (), 11);
    ASSERT_TRUE (view.getIf ('h'));
    ASSERT_EQ (view.size (), 10);
    ASSERT_EQ (view.peek (), 'e');
}

/**
 * @brief removePrefix test.
 */
TEST (View, removePrefix)
{
    View view ("hello world");
    ASSERT_EQ (view.size (), 11);
    view.removePrefix (6);
    ASSERT_EQ (view.peek (), 'w');
    ASSERT_EQ (view.size (), 5);
}

/**
 * @brief at test.
 */
TEST (View, at)
{
    View view ("hello world");
    ASSERT_EQ (view[0], 'h');
    ASSERT_EQ (view[1], 'e');
    ASSERT_EQ (view[2], 'l');
    ASSERT_EQ (view[3], 'l');
    ASSERT_EQ (view[4], 'o');
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
