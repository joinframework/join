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
#include <join/probe.hpp>

// libraries.
#include <gtest/gtest.h>

// C++.
#include <sstream>

using join::StringProbe;

/**
 * @brief create test.
 */
TEST (StringProbe, create)
{
    char msg[] = "hello world";

    StringProbe view1 (msg);
    ASSERT_NE (view1.peek (), std::char_traits<char>::eof ());

    StringProbe view2 (msg, strlen (msg));
    ASSERT_NE (view2.peek (), std::char_traits<char>::eof ());

    StringProbe view3 (msg, msg + strlen (msg));
    ASSERT_NE (view3.peek (), std::char_traits<char>::eof ());
}

/**
 * @brief peek test.
 */
TEST (StringProbe, peek)
{
    StringProbe probe ("hello world");

    ASSERT_EQ (probe.peek (), 'h');
    ASSERT_EQ (probe.get (), 'h');
    ASSERT_EQ (probe.peek (), 'e');
    ASSERT_EQ (probe.get (), 'e');
    ASSERT_EQ (probe.peek (), 'l');
    ASSERT_EQ (probe.get (), 'l');
    ASSERT_EQ (probe.peek (), 'l');
    ASSERT_EQ (probe.get (), 'l');
    ASSERT_EQ (probe.peek (), 'o');
    ASSERT_EQ (probe.get (), 'o');
    ASSERT_EQ (probe.peek (), ' ');
    ASSERT_EQ (probe.get (), ' ');
    ASSERT_EQ (probe.peek (), 'w');
    ASSERT_EQ (probe.get (), 'w');
    ASSERT_EQ (probe.peek (), 'o');
    ASSERT_EQ (probe.get (), 'o');
    ASSERT_EQ (probe.peek (), 'r');
    ASSERT_EQ (probe.get (), 'r');
    ASSERT_EQ (probe.peek (), 'l');
    ASSERT_EQ (probe.get (), 'l');
    ASSERT_EQ (probe.peek (), 'd');
    ASSERT_EQ (probe.get (), 'd');
    ASSERT_EQ (probe.peek (), std::char_traits<char>::eof ());
}

/**
 * @brief get test.
 */
TEST (StringProbe, get)
{
    StringProbe probe ("hello world");

    ASSERT_EQ (probe.get (), 'h');
    ASSERT_EQ (probe.get (), 'e');
    ASSERT_EQ (probe.get (), 'l');
    ASSERT_EQ (probe.get (), 'l');
    ASSERT_EQ (probe.get (), 'o');
    ASSERT_EQ (probe.get (), ' ');
    ASSERT_EQ (probe.get (), 'w');
    ASSERT_EQ (probe.get (), 'o');
    ASSERT_EQ (probe.get (), 'r');
    ASSERT_EQ (probe.get (), 'l');
    ASSERT_EQ (probe.get (), 'd');
    ASSERT_EQ (probe.get (), std::char_traits<char>::eof ());
}

/**
 * @brief getIf test.
 */
TEST (StringProbe, getIf)
{
    StringProbe probe ("hello world");

    ASSERT_FALSE (probe.getIf ('X'));
    ASSERT_FALSE (probe.getIf ('x'));
    ASSERT_FALSE (probe.getIf ('H'));
    ASSERT_TRUE (probe.getIf ('h'));
    ASSERT_FALSE (probe.getIf ('E'));
    ASSERT_TRUE (probe.getIf ('e'));
}

/**
 * @brief getIfNoCase test.
 */
TEST (StringProbe, getIfNoCase)
{
    StringProbe probe ("hello world");

    ASSERT_FALSE (probe.getIfNoCase ('x'));
    ASSERT_FALSE (probe.getIfNoCase ('X'));
    ASSERT_TRUE (probe.getIfNoCase ('h'));
    ASSERT_TRUE (probe.getIfNoCase ('E'));
    ASSERT_TRUE (probe.getIfNoCase ('l'));
    ASSERT_TRUE (probe.getIfNoCase ('L'));
}

/**
 * @brief read test.
 */
TEST (StringProbe, read)
{
    StringProbe probe ("hello world");
    char buf[8] = {};

    ASSERT_EQ (probe.read (buf, 5), 5);
    ASSERT_EQ (probe.read (buf, 8), 6);
    ASSERT_EQ (probe.read (buf, 8), 0);
}

/**
 * @brief readUntilEscaped test.
 */
TEST (StringProbe, readUntilEscaped)
{
    StringProbe probe ("hello\"world");
    std::string out;

    probe.readUntilEscaped (out);
    ASSERT_EQ (out, "hello");
    ASSERT_EQ (probe.peek (), '"');
}

/**
 * @brief skipWhitespaces test.
 */
TEST (StringProbe, skipWhitespaces)
{
    StringProbe probe ("   hello");

    probe.skipWhitespaces ();
    ASSERT_EQ (probe.peek (), 'h');
}

/**
 * @brief skipWhitespacesAndComments test.
 */
TEST (StringProbe, skipWhitespacesAndComments)
{
    StringProbe probe ("   // comment\nhello");
    ASSERT_EQ (probe.skipWhitespacesAndComments (), 0);
    ASSERT_EQ (probe.peek (), 'h');

    probe = "   /* comment */hello";
    ASSERT_EQ (probe.skipWhitespacesAndComments (), 0);
    ASSERT_EQ (probe.peek (), 'h');

    probe = "// comment";
    ASSERT_EQ (probe.skipWhitespacesAndComments (), 0);
    ASSERT_EQ (probe.peek (), std::char_traits<char>::eof ());

    probe = "/* comment */";
    ASSERT_EQ (probe.skipWhitespacesAndComments (), 0);
    ASSERT_EQ (probe.peek (), std::char_traits<char>::eof ());

    probe = "/";
    ASSERT_EQ (probe.skipWhitespacesAndComments (), -1);

    probe = "/*";
    ASSERT_EQ (probe.skipWhitespacesAndComments (), -1);

    probe = "/!";
    ASSERT_EQ (probe.skipWhitespacesAndComments (), -1);
}

/**
 * @brief tell test.
 */
TEST (StringProbe, tell)
{
    StringProbe probe ("hello world");
    auto beg = probe.tell ();

    ASSERT_EQ (probe.get (), 'h');
    ASSERT_EQ (probe.tell (), beg + 1);
    ASSERT_EQ (probe.get (), 'e');
    ASSERT_EQ (probe.tell (), beg + 2);
    ASSERT_EQ (probe.get (), 'l');
    ASSERT_EQ (probe.tell (), beg + 3);
    ASSERT_EQ (probe.get (), 'l');
    ASSERT_EQ (probe.tell (), beg + 4);
    ASSERT_EQ (probe.get (), 'o');
    ASSERT_EQ (probe.tell (), beg + 5);
    ASSERT_EQ (probe.get (), ' ');
    ASSERT_EQ (probe.tell (), beg + 6);
    ASSERT_EQ (probe.get (), 'w');
    ASSERT_EQ (probe.tell (), beg + 7);
    ASSERT_EQ (probe.get (), 'o');
    ASSERT_EQ (probe.tell (), beg + 8);
    ASSERT_EQ (probe.get (), 'r');
    ASSERT_EQ (probe.tell (), beg + 9);
    ASSERT_EQ (probe.get (), 'l');
    ASSERT_EQ (probe.tell (), beg + 10);
    ASSERT_EQ (probe.get (), 'd');
    ASSERT_EQ (probe.tell (), beg + 11);
}

/**
 * @brief seek test.
 */
TEST (StringProbe, seek)
{
    StringProbe probe ("hello world");

    ASSERT_EQ (probe.get (), 'h');
    probe.seek (probe.tell () - 10000);
    ASSERT_EQ (probe.get (), 'h');
    ASSERT_EQ (probe.get (), 'e');
    ASSERT_EQ (probe.get (), 'l');
    probe.seek (probe.tell () - 2);
    ASSERT_EQ (probe.get (), 'e');
    ASSERT_EQ (probe.get (), 'l');
    probe.seek (probe.tell () + 10000);
    ASSERT_EQ (probe.get (), std::char_traits<char>::eof ());
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
