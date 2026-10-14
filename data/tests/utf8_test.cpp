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
#include <join/utf8.hpp>
#include <join/utf16.hpp>

// Libraries.
#include <gtest/gtest.h>

// C++.
#include <string>

/**
 * @brief Test decode.
 */
TEST (Utf8, decode)
{
    struct Valid
    {
        std::string in;
        uint32_t codepoint;
    };

    for (auto const& v :
         {Valid{std::string (1, '\0'), 0x00}, Valid{"\x7F", 0x7F}, Valid{"\xC2\x80", 0x80}, Valid{"\xDF\xBF", 0x7FF},
          Valid{"\xE0\xA0\x80", 0x800}, Valid{"\xED\x9F\xBF", 0xD7FF}, Valid{"\xEE\x80\x80", 0xE000},
          Valid{"\xEF\xBF\xBF", 0xFFFF}, Valid{"\xF0\x90\x80\x80", 0x10000}, Valid{"\xF4\x8F\xBF\xBF", 0x10FFFF}})
    {
        const char* cur = v.in.data ();
        uint32_t codepoint = 0;
        ASSERT_EQ (join::utf8::decode (cur, v.in.data () + v.in.size (), codepoint), 0);
        EXPECT_EQ (codepoint, v.codepoint);
        EXPECT_EQ (cur, v.in.data () + v.in.size ());
    }

    for (auto const& in : {"\x80", "\xBF", "\xC0\x80", "\xC1\xBF", "\xC2\x7F", "\xC2\xC0", "\xE0\x9F\xBF",
                           "\xED\xA0\x80", "\xED\xBF\xBF", "\xE1\x80\x7F", "\xF0\x8F\xBF\xBF", "\xF4\x90\x80\x80",
                           "\xF1\x80\x80\xC0", "\xF5\x80\x80\x80", "\xFF", "\xC2", "\xE1\x80", "\xF1\x80\x80"})
    {
        std::string str (in);
        const char* cur = str.data ();
        uint32_t codepoint = 0;
        EXPECT_EQ (join::utf8::decode (cur, str.data () + str.size (), codepoint), -1);
        EXPECT_EQ (cur, str.data ());
    }
}

/**
 * @brief Test encode.
 */
TEST (Utf8, encode)
{
    char buf[4];

    ASSERT_EQ (join::utf8::encode (0x24, buf), 1U);
    EXPECT_EQ (std::string (buf, 1), "\x24");

    ASSERT_EQ (join::utf8::encode (0xA2, buf), 2U);
    EXPECT_EQ (std::string (buf, 2), "\xC2\xA2");

    ASSERT_EQ (join::utf8::encode (0x20AC, buf), 3U);
    EXPECT_EQ (std::string (buf, 3), "\xE2\x82\xAC");

    ASSERT_EQ (join::utf8::encode (0x1D11E, buf), 4U);
    EXPECT_EQ (std::string (buf, 4), "\xF0\x9D\x84\x9E");

    for (uint32_t codepoint = 0; codepoint <= 0x10FFFF; ++codepoint)
    {
        if (join::utf16::isSurrogate (codepoint))
        {
            continue;
        }
        size_t n = join::utf8::encode (codepoint, buf);
        const char* cur = buf;
        uint32_t decoded = 0;
        ASSERT_EQ (join::utf8::decode (cur, buf + n, decoded), 0);
        ASSERT_EQ (decoded, codepoint);
        ASSERT_EQ (cur, buf + n);
    }
}

/**
 * @brief Test validate.
 */
TEST (Utf8, validate)
{
    std::string empty;
    EXPECT_EQ (join::utf8::validate (empty.data (), empty.data ()), 0);

    std::string ascii ("The quick brown fox jumps over the lazy dog");
    EXPECT_EQ (join::utf8::validate (ascii.data (), ascii.data () + ascii.size ()), 0);

    std::string mixed ("ascii prefix \xE2\x82\xAC followed by ascii \xF0\x9F\x98\x82 and \xC3\xB6 suffix");
    EXPECT_EQ (join::utf8::validate (mixed.data (), mixed.data () + mixed.size ()), 0);

    std::string truncated ("ascii prefix \xE2\x82");
    EXPECT_EQ (join::utf8::validate (truncated.data (), truncated.data () + truncated.size ()), -1);

    std::string surrogate ("ascii prefix \xED\xA0\x80 suffix");
    EXPECT_EQ (join::utf8::validate (surrogate.data (), surrogate.data () + surrogate.size ()), -1);

    std::string overlong ("\xC0\xAF");
    EXPECT_EQ (join::utf8::validate (overlong.data (), overlong.data () + overlong.size ()), -1);
}

/**
 * @brief Test compareUtf16Order.
 */
TEST (Utf8, compareUtf16Order)
{
    auto cmp = [] (const std::string& a, const std::string& b) {
        return join::utf8::compareUtf16Order (a.data (), a.data () + a.size (), b.data (), b.data () + b.size ());
    };

    EXPECT_EQ (cmp ("", ""), 0);
    EXPECT_EQ (cmp ("abc", "abc"), 0);
    EXPECT_LT (cmp ("ab", "abc"), 0);
    EXPECT_GT (cmp ("abc", "ab"), 0);
    EXPECT_LT (cmp ("a", "b"), 0);
    EXPECT_LT (cmp ("\xC3\xB6", "\xE2\x82\xAC"), 0);
    EXPECT_LT (cmp ("\xED\x9F\xBF", "\xF0\x9F\x98\x82"), 0);
    EXPECT_LT (cmp ("\xF0\x9F\x98\x82", "\xEE\x80\x80"), 0);
    EXPECT_GT (cmp ("\xEF\xBF\xBF", "\xF4\x8F\xBF\xBF"), 0);
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
