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

// C++.
#include <sstream>

using join::StringView;

/**
 * @brief create test.
 */
TEST (StringView, create)
{
    char msg[] = "hello world";

    StringView view1 (msg);
    ASSERT_NE (view1.peek (), std::char_traits <char>::eof ());

    StringView view2 (msg, strlen (msg));
    ASSERT_NE (view2.peek (), std::char_traits <char>::eof ());

    StringView view3 (msg, msg + strlen (msg));
    ASSERT_NE (view3.peek (), std::char_traits <char>::eof ());
}

/**
 * @brief peek test.
 */
TEST (StringView, peek)
{
    StringView view ("hello world");

    ASSERT_EQ (view.peek (), 'h');
    ASSERT_EQ (view.get (), 'h');
    ASSERT_EQ (view.peek (), 'e');
    ASSERT_EQ (view.get (), 'e');
    ASSERT_EQ (view.peek (), 'l');
    ASSERT_EQ (view.get (), 'l');
    ASSERT_EQ (view.peek (), 'l');
    ASSERT_EQ (view.get (), 'l');
    ASSERT_EQ (view.peek (), 'o');
    ASSERT_EQ (view.get (), 'o');
    ASSERT_EQ (view.peek (), ' ');
    ASSERT_EQ (view.get (), ' ');
    ASSERT_EQ (view.peek (), 'w');
    ASSERT_EQ (view.get (), 'w');
    ASSERT_EQ (view.peek (), 'o');
    ASSERT_EQ (view.get (), 'o');
    ASSERT_EQ (view.peek (), 'r');
    ASSERT_EQ (view.get (), 'r');
    ASSERT_EQ (view.peek (), 'l');
    ASSERT_EQ (view.get (), 'l');
    ASSERT_EQ (view.peek (), 'd');
    ASSERT_EQ (view.get (), 'd');
    ASSERT_EQ (view.peek (), std::char_traits <char>::eof ());
}

/**
 * @brief get test.
 */
TEST (StringView, get)
{
    StringView view ("hello world");

    ASSERT_EQ (view.get (), 'h');
    ASSERT_EQ (view.get (), 'e');
    ASSERT_EQ (view.get (), 'l');
    ASSERT_EQ (view.get (), 'l');
    ASSERT_EQ (view.get (), 'o');
    ASSERT_EQ (view.get (), ' ');
    ASSERT_EQ (view.get (), 'w');
    ASSERT_EQ (view.get (), 'o');
    ASSERT_EQ (view.get (), 'r');
    ASSERT_EQ (view.get (), 'l');
    ASSERT_EQ (view.get (), 'd');
    ASSERT_EQ (view.get (), std::char_traits <char>::eof ());
}

/**
 * @brief getIf test.
 */
TEST (StringView, getIf)
{
    StringView view ("hello world");

    ASSERT_FALSE (view.getIf ('X'));
    ASSERT_FALSE (view.getIf ('x'));
    ASSERT_FALSE (view.getIf ('H'));
    ASSERT_TRUE  (view.getIf ('h'));
    ASSERT_FALSE (view.getIf ('E'));
    ASSERT_TRUE  (view.getIf ('e'));
}

/**
 * @brief getIfNoCase test.
 */
TEST (StringView, getIfNoCase)
{
    StringView view ("hello world");

    ASSERT_FALSE (view.getIfNoCase ('x'));
    ASSERT_FALSE (view.getIfNoCase ('X'));
    ASSERT_TRUE  (view.getIfNoCase ('h'));
    ASSERT_TRUE  (view.getIfNoCase ('E'));
    ASSERT_TRUE  (view.getIfNoCase ('l'));
    ASSERT_TRUE  (view.getIfNoCase ('L'));
}

/**
 * @brief read test.
 */
TEST (StringView, read)
{
    StringView view ("hello world");
    char buf[8] = {};

    ASSERT_EQ (view.read (buf, 5), 5);
    ASSERT_EQ (view.read (buf, 8), 6);
    ASSERT_EQ (view.read (buf, 8), 0);
}

/**
 * @brief readUntil test.
 */
TEST (StringView, readUntil)
{
    StringView view ("hello world");
    std::string buf;

    ASSERT_EQ (view.readUntil (buf, 'w'), 6);
    ASSERT_EQ (buf, "hello ");

    buf.clear ();
    ASSERT_EQ (view.readUntil (buf, [] (char c) { return c == 'r'; }), 2);
    ASSERT_EQ (buf, "wo");

    ASSERT_EQ (view.readUntil (buf, 'z'), 3);
    ASSERT_EQ (buf, "world");
}

/**
 * @brief consumeUntil test.
 */
TEST (StringView, consumeUntil)
{
    StringView view ("hello world");

    ASSERT_EQ (view.consumeUntil ('o'), 4);
    ASSERT_EQ (view.peek (), 'o');

    ASSERT_EQ (view.consumeUntil ([] (char c) { return c == 'r'; }), 4);
    ASSERT_EQ (view.peek (), 'r');

    ASSERT_EQ (view.consumeUntil ('z'), 3);
    ASSERT_EQ (view.peek (), std::char_traits <char>::eof ());
}

/**
 * @brief tell test.
 */
TEST (StringView, tell)
{
    StringView view ("hello world");
    auto beg = view.tell ();

    ASSERT_EQ (view.get (), 'h');
    ASSERT_EQ (view.tell (), beg + 1);
    ASSERT_EQ (view.get (), 'e');
    ASSERT_EQ (view.tell (), beg + 2);
    ASSERT_EQ (view.get (), 'l');
    ASSERT_EQ (view.tell (), beg + 3);
    ASSERT_EQ (view.get (), 'l');
    ASSERT_EQ (view.tell (), beg + 4);
    ASSERT_EQ (view.get (), 'o');
    ASSERT_EQ (view.tell (), beg + 5);
    ASSERT_EQ (view.get (), ' ');
    ASSERT_EQ (view.tell (), beg + 6);
    ASSERT_EQ (view.get (), 'w');
    ASSERT_EQ (view.tell (), beg + 7);
    ASSERT_EQ (view.get (), 'o');
    ASSERT_EQ (view.tell (), beg + 8);
    ASSERT_EQ (view.get (), 'r');
    ASSERT_EQ (view.tell (), beg + 9);
    ASSERT_EQ (view.get (), 'l');
    ASSERT_EQ (view.tell (), beg + 10);
    ASSERT_EQ (view.get (), 'd');
    ASSERT_EQ (view.tell (), beg + 11);
}

/**
 * @brief seek test.
 */
TEST (StringView, seek)
{
    StringView view ("hello world");

    ASSERT_EQ (view.get (), 'h');
    view.seek (view.tell () - 10000);
    ASSERT_EQ (view.get (), 'h');
    ASSERT_EQ (view.get (), 'e');
    ASSERT_EQ (view.get (), 'l');
    view.seek (view.tell () - 2);
    ASSERT_EQ (view.get (), 'e');
    ASSERT_EQ (view.get (), 'l');
    view.seek (view.tell () + 10000);
    ASSERT_EQ (view.get (), std::char_traits <char>::eof ());
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
