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
#include <join/utf16.hpp>

// Libraries.
#include <gtest/gtest.h>

/**
 * @brief Test isHighSurrogate.
 */
TEST (Utf16, isHighSurrogate)
{
    EXPECT_FALSE (join::utf16::isHighSurrogate (0xD7FF));
    EXPECT_TRUE (join::utf16::isHighSurrogate (0xD800));
    EXPECT_TRUE (join::utf16::isHighSurrogate (0xDBFF));
    EXPECT_FALSE (join::utf16::isHighSurrogate (0xDC00));
    static_assert (join::utf16::isHighSurrogate (0xD800), "isHighSurrogate must be usable at compile time");
}

/**
 * @brief Test isLowSurrogate.
 */
TEST (Utf16, isLowSurrogate)
{
    EXPECT_FALSE (join::utf16::isLowSurrogate (0xDBFF));
    EXPECT_TRUE (join::utf16::isLowSurrogate (0xDC00));
    EXPECT_TRUE (join::utf16::isLowSurrogate (0xDFFF));
    EXPECT_FALSE (join::utf16::isLowSurrogate (0xE000));
    static_assert (join::utf16::isLowSurrogate (0xDC00), "isLowSurrogate must be usable at compile time");
}

/**
 * @brief Test isSurrogate.
 */
TEST (Utf16, isSurrogate)
{
    EXPECT_FALSE (join::utf16::isSurrogate (0xD7FF));
    EXPECT_TRUE (join::utf16::isSurrogate (0xD800));
    EXPECT_TRUE (join::utf16::isSurrogate (0xDFFF));
    EXPECT_FALSE (join::utf16::isSurrogate (0xE000));
    static_assert (join::utf16::isSurrogate (0xDFFF), "isSurrogate must be usable at compile time");
}

/**
 * @brief Test highSurrogate.
 */
TEST (Utf16, highSurrogate)
{
    EXPECT_EQ (join::utf16::highSurrogate (0x10000), 0xD800U);
    EXPECT_EQ (join::utf16::highSurrogate (0x1F602), 0xD83DU);
    EXPECT_EQ (join::utf16::highSurrogate (0x10FFFF), 0xDBFFU);
    static_assert (join::utf16::highSurrogate (0x1D11E) == 0xD834, "highSurrogate must be usable at compile time");
}

/**
 * @brief Test lowSurrogate.
 */
TEST (Utf16, lowSurrogate)
{
    EXPECT_EQ (join::utf16::lowSurrogate (0x10000), 0xDC00U);
    EXPECT_EQ (join::utf16::lowSurrogate (0x1F602), 0xDE02U);
    EXPECT_EQ (join::utf16::lowSurrogate (0x10FFFF), 0xDFFFU);
    static_assert (join::utf16::lowSurrogate (0x1D11E) == 0xDD1E, "lowSurrogate must be usable at compile time");
}

/**
 * @brief Test combine.
 */
TEST (Utf16, combine)
{
    EXPECT_EQ (join::utf16::combine (0xD800, 0xDC00), 0x10000U);
    EXPECT_EQ (join::utf16::combine (0xD834, 0xDD1E), 0x1D11EU);
    EXPECT_EQ (join::utf16::combine (0xDBFF, 0xDFFF), 0x10FFFFU);
    static_assert (join::utf16::combine (0xD83D, 0xDE02) == 0x1F602, "combine must be usable at compile time");
}

/**
 * @brief Test decode.
 */
TEST (Utf16, decode)
{
    char16_t bmp[] = {0x20AC};
    const char16_t* cur = bmp;
    uint32_t codepoint = 0;
    ASSERT_EQ (join::utf16::decode (cur, bmp + 1, codepoint), 0);
    EXPECT_EQ (codepoint, 0x20ACU);
    EXPECT_EQ (cur, bmp + 1);

    char16_t pair[] = {0xD834, 0xDD1E};
    cur = pair;
    ASSERT_EQ (join::utf16::decode (cur, pair + 2, codepoint), 0);
    EXPECT_EQ (codepoint, 0x1D11EU);
    EXPECT_EQ (cur, pair + 2);

    char16_t lowFirst[] = {0xDC00, 0xDC00};
    cur = lowFirst;
    EXPECT_EQ (join::utf16::decode (cur, lowFirst + 2, codepoint), -1);
    EXPECT_EQ (cur, lowFirst);

    char16_t highAlone[] = {0xD800};
    cur = highAlone;
    EXPECT_EQ (join::utf16::decode (cur, highAlone + 1, codepoint), -1);
    EXPECT_EQ (cur, highAlone);

    char16_t highHigh[] = {0xD800, 0xD800};
    cur = highHigh;
    EXPECT_EQ (join::utf16::decode (cur, highHigh + 2, codepoint), -1);

    char16_t highBmp[] = {0xD800, 0x0041};
    cur = highBmp;
    EXPECT_EQ (join::utf16::decode (cur, highBmp + 2, codepoint), -1);
}

/**
 * @brief Test encode.
 */
TEST (Utf16, encode)
{
    char16_t buf[2];

    ASSERT_EQ (join::utf16::encode (0xFFFF, buf), 1U);
    EXPECT_EQ (buf[0], 0xFFFF);

    ASSERT_EQ (join::utf16::encode (0x10000, buf), 2U);
    EXPECT_EQ (buf[0], 0xD800);
    EXPECT_EQ (buf[1], 0xDC00);

    ASSERT_EQ (join::utf16::encode (0x10FFFF, buf), 2U);
    EXPECT_EQ (buf[0], 0xDBFF);
    EXPECT_EQ (buf[1], 0xDFFF);

    for (uint32_t codepoint = 0; codepoint <= 0x10FFFF; ++codepoint)
    {
        if (join::utf16::isSurrogate (codepoint))
        {
            continue;
        }
        size_t n = join::utf16::encode (codepoint, buf);
        const char16_t* cur = buf;
        uint32_t decoded = 0;
        ASSERT_EQ (join::utf16::decode (cur, buf + n, decoded), 0);
        ASSERT_EQ (decoded, codepoint);
        ASSERT_EQ (cur, buf + n);
    }
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
