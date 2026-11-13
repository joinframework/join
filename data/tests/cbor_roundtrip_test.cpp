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
#include <join/cbor.hpp>

// libraries.
#include <gtest/gtest.h>

// C++.
#include <string_view>
#include <sstream>
#include <string>

using join::Value;
using join::CborReader;
using join::CborWriter;

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
 * @brief Test CBOR round trip.
 */
TEST (Cbor, roundtrip)
{
    Value value;
    CborReader reader;

    std::stringstream stream;
    CborWriter writer (stream);

    stream.clear ();
    stream.str (hex ("81f6"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("81f6"));

    stream.clear ();
    stream.str (hex ("81f5"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("81f5"));

    stream.clear ();
    stream.str (hex ("81f4"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("81f4"));

    stream.clear ();
    stream.str (hex ("8100"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("8100"));

    stream.clear ();
    stream.str (hex ("8163666f6f"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("8163666f6f"));

    stream.clear ();
    stream.str (hex ("80"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("80"));

    stream.clear ();
    stream.str (hex ("a0"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("a0"));

    stream.clear ();
    stream.str (hex ("820001"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("820001"));

    stream.clear ();
    stream.str (hex ("a163666f6f63626172"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("a163666f6f63626172"));

    stream.clear ();
    stream.str (hex ("a26161f663666f6f63626172"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("a26161f663666f6f63626172"));

    stream.clear ();
    stream.str (hex ("8120"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("8120"));

    stream.clear ();
    stream.str (hex ("813a7fffffff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("813a7fffffff"));

    stream.clear ();
    stream.str (hex ("813b112210f47de98114"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("813b112210f47de98114"));

    stream.clear ();
    stream.str (hex ("813b7fffffffffffffff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("813b7fffffffffffffff"));

    stream.clear ();
    stream.str (hex ("8101"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("8101"));

    stream.clear ();
    stream.str (hex ("811a7fffffff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("811a7fffffff"));

    stream.clear ();
    stream.str (hex ("811affffffff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("811affffffff"));

    stream.clear ();
    stream.str (hex ("811b112210f47de98115"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("811b112210f47de98115"));

    stream.clear ();
    stream.str (hex ("811b7fffffffffffffff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("811b7fffffffffffffff"));

    stream.clear ();
    stream.str (hex ("811bffffffffffffffff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("811bffffffffffffffff"));

    stream.clear ();
    stream.str (hex ("81fa00000000"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("81fa00000000"));

    stream.clear ();
    stream.str (hex ("81fa80000000"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("81fa80000000"));

    stream.clear ();
    stream.str (hex ("81fb3ff3c083126e978d"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("81fb3ff3c083126e978d"));

    stream.clear ();
    stream.str (hex ("81fbbff3c083126e978d"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("81fbbff3c083126e978d"));

    stream.clear ();
    stream.str (hex ("81fb0000000000000001"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("81fb0000000000000001"));

    stream.clear ();
    stream.str (hex ("81fb000fffffffffffff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("81fb000fffffffffffff"));

    stream.clear ();
    stream.str (hex ("81fb0010000000000000"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("81fb0010000000000000"));

    stream.clear ();
    stream.str (hex ("81fb7fefffffffffffff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("81fb7fefffffffffffff"));

    stream.clear ();
    stream.str (hex ("8143010203"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("8143010203"));

    stream.clear ();
    stream.str (hex ("a10102"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("a10102"));

    stream.clear ();
    stream.str (hex ("81fa7f800000"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("81fa7f800000"));

    stream.clear ();
    stream.str (hex ("81faff800000"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("81faff800000"));

    stream.clear ();
    stream.str (hex ("81fa7fc00000"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("81fa7fc00000"));

    stream.clear ();
    stream.str (hex ("81faffc00000"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("81faffc00000"));
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
