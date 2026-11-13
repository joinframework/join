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
#include <join/pack.hpp>

// libraries.
#include <gtest/gtest.h>

// C++.
#include <string_view>
#include <sstream>
#include <string>

using join::Value;
using join::PackReader;
using join::PackWriter;

/**
 * @brief convert hexadecimal digits to bytes.
 * @param digits hexadecimal digits.
 * @return bytes.
 */
static std::string hex (std::string_view digits)
{
    std::string bytes;
    for (size_t i = 0; i + 1 < digits.size (); i += 2)
    {
        bytes += static_cast<char> (std::stoi (std::string (digits.substr (i, 2)), nullptr, 16));
    }
    return bytes;
}

/**
 * @brief Test MessagePack round trip.
 */
TEST (Pack, roundtrip)
{
    Value value;
    PackReader reader;

    std::stringstream stream;
    PackWriter writer (stream);

    stream.clear ();
    stream.str (hex ("91c0"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("91c0"));

    stream.clear ();
    stream.str (hex ("91c3"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("91c3"));

    stream.clear ();
    stream.str (hex ("91c2"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("91c2"));

    stream.clear ();
    stream.str (hex ("9100"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("9100"));

    stream.clear ();
    stream.str (hex ("91a3666f6f"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("91a3666f6f"));

    stream.clear ();
    stream.str (hex ("90"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("90"));

    stream.clear ();
    stream.str (hex ("80"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("80"));

    stream.clear ();
    stream.str (hex ("920001"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("920001"));

    stream.clear ();
    stream.str (hex ("81a3666f6fa3626172"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("81a3666f6fa3626172"));

    stream.clear ();
    stream.str (hex ("82a161c0a3666f6fa3626172"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("82a161c0a3666f6fa3626172"));

    stream.clear ();
    stream.str (hex ("91ff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("91ff"));

    stream.clear ();
    stream.str (hex ("91d280000000"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("91d280000000"));

    stream.clear ();
    stream.str (hex ("91d3eeddef0b82167eeb"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("91d3eeddef0b82167eeb"));

    stream.clear ();
    stream.str (hex ("91d38000000000000000"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("91d38000000000000000"));

    stream.clear ();
    stream.str (hex ("9101"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("9101"));

    stream.clear ();
    stream.str (hex ("91ce7fffffff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("91ce7fffffff"));

    stream.clear ();
    stream.str (hex ("91ceffffffff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("91ceffffffff"));

    stream.clear ();
    stream.str (hex ("91cf112210f47de98115"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("91cf112210f47de98115"));

    stream.clear ();
    stream.str (hex ("91cf7fffffffffffffff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("91cf7fffffffffffffff"));

    stream.clear ();
    stream.str (hex ("91cfffffffffffffffff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("91cfffffffffffffffff"));

    stream.clear ();
    stream.str (hex ("91ca00000000"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("91ca00000000"));

    stream.clear ();
    stream.str (hex ("91ca80000000"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("91ca80000000"));

    stream.clear ();
    stream.str (hex ("91cb3ff3c083126e978d"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("91cb3ff3c083126e978d"));

    stream.clear ();
    stream.str (hex ("91cbbff3c083126e978d"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("91cbbff3c083126e978d"));

    stream.clear ();
    stream.str (hex ("91cb0000000000000001"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("91cb0000000000000001"));

    stream.clear ();
    stream.str (hex ("91cb000fffffffffffff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("91cb000fffffffffffff"));

    stream.clear ();
    stream.str (hex ("91cb0010000000000000"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("91cb0010000000000000"));

    stream.clear ();
    stream.str (hex ("91cb7fefffffffffffff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("91cb7fefffffffffffff"));

    stream.clear ();
    stream.str (hex ("91c403010203"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("91c403010203"));

    stream.clear ();
    stream.str (hex ("810102"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("810102"));

    stream.clear ();
    stream.str (hex ("91ca7f800000"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("91ca7f800000"));

    stream.clear ();
    stream.str (hex ("91caff800000"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("91caff800000"));

    stream.clear ();
    stream.str (hex ("91ca7fc00000"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("91ca7fc00000"));

    stream.clear ();
    stream.str (hex ("91caffc00000"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("91caffc00000"));
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
