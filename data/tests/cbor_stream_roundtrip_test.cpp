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
using join::CborStreamWriter;

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
 * @brief Test CBOR stream round trip.
 */
TEST (CborStream, roundtrip)
{
    Value value;
    CborReader reader;

    std::stringstream stream;
    CborStreamWriter writer (stream);

    stream.clear ();
    stream.str (hex ("9ff6ff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("9ff6ff"));

    stream.clear ();
    stream.str (hex ("9ff5ff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("9ff5ff"));

    stream.clear ();
    stream.str (hex ("9ff4ff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("9ff4ff"));

    stream.clear ();
    stream.str (hex ("9f00ff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("9f00ff"));

    stream.clear ();
    stream.str (hex ("9f63666f6fff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("9f63666f6fff"));

    stream.clear ();
    stream.str (hex ("9fff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("9fff"));

    stream.clear ();
    stream.str (hex ("bfff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("bfff"));

    stream.clear ();
    stream.str (hex ("9f0001ff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("9f0001ff"));

    stream.clear ();
    stream.str (hex ("bf63666f6f63626172ff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("bf63666f6f63626172ff"));

    stream.clear ();
    stream.str (hex ("bf6161f663666f6f63626172ff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("bf6161f663666f6f63626172ff"));

    stream.clear ();
    stream.str (hex ("9f20ff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("9f20ff"));

    stream.clear ();
    stream.str (hex ("9f3a7fffffffff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("9f3a7fffffffff"));

    stream.clear ();
    stream.str (hex ("9f3b112210f47de98114ff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("9f3b112210f47de98114ff"));

    stream.clear ();
    stream.str (hex ("9f3b7fffffffffffffffff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("9f3b7fffffffffffffffff"));

    stream.clear ();
    stream.str (hex ("9f01ff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("9f01ff"));

    stream.clear ();
    stream.str (hex ("9f1a7fffffffff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("9f1a7fffffffff"));

    stream.clear ();
    stream.str (hex ("9f1affffffffff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("9f1affffffffff"));

    stream.clear ();
    stream.str (hex ("9f1b112210f47de98115ff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("9f1b112210f47de98115ff"));

    stream.clear ();
    stream.str (hex ("9f1b7fffffffffffffffff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("9f1b7fffffffffffffffff"));

    stream.clear ();
    stream.str (hex ("9f1bffffffffffffffffff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("9f1bffffffffffffffffff"));

    stream.clear ();
    stream.str (hex ("9ffa00000000ff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("9ffa00000000ff"));

    stream.clear ();
    stream.str (hex ("9ffa80000000ff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("9ffa80000000ff"));

    stream.clear ();
    stream.str (hex ("9ffb3ff3c083126e978dff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("9ffb3ff3c083126e978dff"));

    stream.clear ();
    stream.str (hex ("9ffbbff3c083126e978dff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("9ffbbff3c083126e978dff"));

    stream.clear ();
    stream.str (hex ("9ffb0000000000000001ff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("9ffb0000000000000001ff"));

    stream.clear ();
    stream.str (hex ("9ffb000fffffffffffffff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("9ffb000fffffffffffffff"));

    stream.clear ();
    stream.str (hex ("9ffb0010000000000000ff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("9ffb0010000000000000ff"));

    stream.clear ();
    stream.str (hex ("9ffb7fefffffffffffffff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("9ffb7fefffffffffffffff"));

    stream.clear ();
    stream.str (hex ("9f43010203ff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("9f43010203ff"));

    stream.clear ();
    stream.str (hex ("bf0102ff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("bf0102ff"));

    stream.clear ();
    stream.str (hex ("9ffa7f800000ff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("9ffa7f800000ff"));

    stream.clear ();
    stream.str (hex ("9ffaff800000ff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("9ffaff800000ff"));

    stream.clear ();
    stream.str (hex ("9ffa7fc00000ff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("9ffa7fc00000ff"));

    stream.clear ();
    stream.str (hex ("9ffaffc00000ff"));
    ASSERT_NE (reader.deserialize (stream, value), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), hex ("9ffaffc00000ff"));
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
