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

using join::StringStreamView;

/**
 * @brief create test.
 */
TEST (StreamView, create)
{
    std::stringstream msg ("hello world");
    StringStreamView view (msg);

    ASSERT_NE (view.peek (), std::char_traits <char>::eof ());
}

/**
 * @brief peek test.
 */
TEST (StreamView, peek)
{
    std::stringstream msg ("hello world");
    StringStreamView view (msg);

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
TEST (StreamView, get)
{
    std::stringstream msg ("hello world");
    StringStreamView view (msg);

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
TEST (StreamView, getIf)
{
    std::stringstream msg ("hello world");
    StringStreamView view (msg);

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
TEST (StreamView, getIfNoCase)
{
    std::stringstream msg ("hello world");
    StringStreamView view (msg);

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
TEST (StreamView, read)
{
    std::stringstream msg ("hello world");
    StringStreamView view (msg);
    char buf[8] = {};

    ASSERT_EQ (view.read (buf, 5), 5);
    ASSERT_EQ (view.read (buf, 8), 6);
    ASSERT_EQ (view.read (buf, 8), 0);
}

/**
 * @brief readUntilEscaped test.
 */
TEST (StreamView, readUntilEscaped)
{
    std::stringstream msg ("hello\"world");
    StringStreamView view (msg);
    std::string out;

    view.readUntilEscaped (out);
    ASSERT_EQ (out, "hello");
    ASSERT_EQ (view.peek (), '"');
}

/**
 * @brief skipWhitespaces test.
 */
TEST (StreamView, skipWhitespaces)
{
    std::stringstream msg ("   hello");
    StringStreamView view (msg);
    std::string out;

    view.skipWhitespaces ();
    ASSERT_EQ (view.peek (), 'h');
}

/**
 * @brief skipWhitespacesAndComments test.
 */
TEST (StreamView, skipWhitespacesAndComments)
{
    std::stringstream msg ("   // comment\nhello");
    StringStreamView view1 (msg);
    ASSERT_EQ (view1.skipWhitespacesAndComments (), 0);
    ASSERT_EQ (view1.peek (), 'h');

    msg.str ("   /* comment */hello");
    msg.clear ();
    StringStreamView view2 (msg);
    ASSERT_EQ (view2.skipWhitespacesAndComments (), 0);
    ASSERT_EQ (view2.peek (), 'h');

    msg.str ("// comment");
    msg.clear ();
    StringStreamView view3 (msg);
    ASSERT_EQ (view3.skipWhitespacesAndComments (), 0);
    ASSERT_EQ (view3.peek (), std::char_traits <char>::eof ());

    msg.str ("/* comment */");
    msg.clear ();
    StringStreamView view4 (msg);
    ASSERT_EQ (view4.skipWhitespacesAndComments (), 0);
    ASSERT_EQ (view4.peek (), std::char_traits <char>::eof ());

    msg.str ("/");
    msg.clear ();
    StringStreamView view5 (msg);
    ASSERT_EQ (view5.skipWhitespacesAndComments (), -1);

    msg.str ("/*");
    msg.clear ();
    StringStreamView view6 (msg);
    ASSERT_EQ (view6.skipWhitespacesAndComments (), -1);

    msg.str ("/!");
    msg.clear ();
    StringStreamView view7 (msg);
    ASSERT_EQ (view7.skipWhitespacesAndComments (), -1);
}

/**
 * @brief tell test.
 */
TEST (StreamView, tell)
{
    std::stringstream msg ("hello world");
    StringStreamView view (msg);
    auto beg = view.tell ();

    ASSERT_EQ (view.get (), 'h');
    ASSERT_EQ (view.tell (), beg + std::streamoff (1));
    ASSERT_EQ (view.get (), 'e');
    ASSERT_EQ (view.tell (), beg + std::streamoff (2));
    ASSERT_EQ (view.get (), 'l');
    ASSERT_EQ (view.tell (), beg + std::streamoff (3));
    ASSERT_EQ (view.get (), 'l');
    ASSERT_EQ (view.tell (), beg + std::streamoff (4));
    ASSERT_EQ (view.get (), 'o');
    ASSERT_EQ (view.tell (), beg + std::streamoff (5));
    ASSERT_EQ (view.get (), ' ');
    ASSERT_EQ (view.tell (), beg + std::streamoff (6));
    ASSERT_EQ (view.get (), 'w');
    ASSERT_EQ (view.tell (), beg + std::streamoff (7));
    ASSERT_EQ (view.get (), 'o');
    ASSERT_EQ (view.tell (), beg + std::streamoff (8));
    ASSERT_EQ (view.get (), 'r');
    ASSERT_EQ (view.tell (), beg + std::streamoff (9));
    ASSERT_EQ (view.get (), 'l');
    ASSERT_EQ (view.tell (), beg + std::streamoff (10));
    ASSERT_EQ (view.get (), 'd');
    ASSERT_EQ (view.tell (), beg + std::streamoff (11));
}

/**
 * @brief seek test.
 */
TEST (StreamView, seek)
{
    std::stringstream msg ("hello world");
    StringStreamView view (msg);

    ASSERT_EQ (view.get (), 'h');
    ASSERT_EQ (view.get (), 'e');
    ASSERT_EQ (view.get (), 'l');
    view.seek (view.tell () - std::streamoff (2));
    ASSERT_EQ (view.get (), 'e');
    ASSERT_EQ (view.get (), 'l');
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
