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

using join::StringStreamProbe;

/**
 * @brief create test.
 */
TEST (StreamProbe, create)
{
    std::stringstream msg ("hello world");
    StringStreamProbe probe (msg);

    ASSERT_NE (probe.peek (), std::char_traits<char>::eof ());
}

/**
 * @brief peek test.
 */
TEST (StreamProbe, peek)
{
    std::stringstream msg ("hello world");
    StringStreamProbe probe (msg);

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
TEST (StreamProbe, get)
{
    std::stringstream msg ("hello world");
    StringStreamProbe probe (msg);

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
TEST (StreamProbe, getIf)
{
    std::stringstream msg ("hello world");
    StringStreamProbe probe (msg);

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
TEST (StreamProbe, getIfNoCase)
{
    std::stringstream msg ("hello world");
    StringStreamProbe probe (msg);

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
TEST (StreamProbe, read)
{
    std::stringstream msg ("hello world");
    StringStreamProbe probe (msg);
    char buf[8] = {};

    ASSERT_EQ (probe.read (buf, 5), 5);
    ASSERT_EQ (probe.read (buf, 8), 6);
    ASSERT_EQ (probe.read (buf, 8), 0);
}

/**
 * @brief readUntilEscaped test.
 */
TEST (StreamProbe, readUntilEscaped)
{
    std::stringstream msg ("hello\"world");
    StringStreamProbe probe (msg);
    std::string out;

    probe.readUntilEscaped (out);
    ASSERT_EQ (out, "hello");
    ASSERT_EQ (probe.peek (), '"');
}

/**
 * @brief skipWhitespaces test.
 */
TEST (StreamProbe, skipWhitespaces)
{
    std::stringstream msg ("   hello");
    StringStreamProbe probe (msg);
    std::string out;

    probe.skipWhitespaces ();
    ASSERT_EQ (probe.peek (), 'h');
}

/**
 * @brief skipWhitespacesAndComments test.
 */
TEST (StreamProbe, skipWhitespacesAndComments)
{
    std::stringstream msg ("   // comment\nhello");
    StringStreamProbe view1 (msg);
    ASSERT_EQ (view1.skipWhitespacesAndComments (), 0);
    ASSERT_EQ (view1.peek (), 'h');

    msg.str ("   /* comment */hello");
    msg.clear ();
    StringStreamProbe view2 (msg);
    ASSERT_EQ (view2.skipWhitespacesAndComments (), 0);
    ASSERT_EQ (view2.peek (), 'h');

    msg.str ("// comment");
    msg.clear ();
    StringStreamProbe view3 (msg);
    ASSERT_EQ (view3.skipWhitespacesAndComments (), 0);
    ASSERT_EQ (view3.peek (), std::char_traits<char>::eof ());

    msg.str ("/* comment */");
    msg.clear ();
    StringStreamProbe view4 (msg);
    ASSERT_EQ (view4.skipWhitespacesAndComments (), 0);
    ASSERT_EQ (view4.peek (), std::char_traits<char>::eof ());

    msg.str ("/");
    msg.clear ();
    StringStreamProbe view5 (msg);
    ASSERT_EQ (view5.skipWhitespacesAndComments (), -1);

    msg.str ("/*");
    msg.clear ();
    StringStreamProbe view6 (msg);
    ASSERT_EQ (view6.skipWhitespacesAndComments (), -1);

    msg.str ("/!");
    msg.clear ();
    StringStreamProbe view7 (msg);
    ASSERT_EQ (view7.skipWhitespacesAndComments (), -1);
}

/**
 * @brief tell test.
 */
TEST (StreamProbe, tell)
{
    std::stringstream msg ("hello world");
    StringStreamProbe probe (msg);
    auto beg = probe.tell ();

    ASSERT_EQ (probe.get (), 'h');
    ASSERT_EQ (probe.tell (), beg + std::streamoff (1));
    ASSERT_EQ (probe.get (), 'e');
    ASSERT_EQ (probe.tell (), beg + std::streamoff (2));
    ASSERT_EQ (probe.get (), 'l');
    ASSERT_EQ (probe.tell (), beg + std::streamoff (3));
    ASSERT_EQ (probe.get (), 'l');
    ASSERT_EQ (probe.tell (), beg + std::streamoff (4));
    ASSERT_EQ (probe.get (), 'o');
    ASSERT_EQ (probe.tell (), beg + std::streamoff (5));
    ASSERT_EQ (probe.get (), ' ');
    ASSERT_EQ (probe.tell (), beg + std::streamoff (6));
    ASSERT_EQ (probe.get (), 'w');
    ASSERT_EQ (probe.tell (), beg + std::streamoff (7));
    ASSERT_EQ (probe.get (), 'o');
    ASSERT_EQ (probe.tell (), beg + std::streamoff (8));
    ASSERT_EQ (probe.get (), 'r');
    ASSERT_EQ (probe.tell (), beg + std::streamoff (9));
    ASSERT_EQ (probe.get (), 'l');
    ASSERT_EQ (probe.tell (), beg + std::streamoff (10));
    ASSERT_EQ (probe.get (), 'd');
    ASSERT_EQ (probe.tell (), beg + std::streamoff (11));
}

/**
 * @brief seek test.
 */
TEST (StreamProbe, seek)
{
    std::stringstream msg ("hello world");
    StringStreamProbe probe (msg);

    ASSERT_EQ (probe.get (), 'h');
    ASSERT_EQ (probe.get (), 'e');
    ASSERT_EQ (probe.get (), 'l');
    probe.seek (probe.tell () - std::streamoff (2));
    ASSERT_EQ (probe.get (), 'e');
    ASSERT_EQ (probe.get (), 'l');
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
