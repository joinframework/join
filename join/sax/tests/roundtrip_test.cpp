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
#include <join/json.hpp>

// libraries.
#include <gtest/gtest.h>

using join::sax::Array;
using join::sax::Member;
using join::sax::Object;
using join::sax::Value;

using join::sax::SaxErrc;
using join::sax::JsonErrc;
using join::sax::JsonWriter;
using join::sax::JsonReader;

/**
 * @brief Test JSON round trip.
 */
TEST (JsonReader, roundtrip)
{
    Value value;
    JsonReader reader (value);

    std::stringstream stream;
    JsonWriter writer (stream);

    stream.clear ();
    stream.str ("[null]");
    ASSERT_NE (reader.deserialize (stream), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (writer.serialize (value), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), "[null]");

    stream.clear ();
    stream.str ("[true]");
    ASSERT_NE (reader.deserialize (stream), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (writer.serialize (value), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), "[true]");

    stream.clear ();
    stream.str ("[false]");
    ASSERT_NE (reader.deserialize (stream), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (writer.serialize (value), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), "[false]");

    stream.clear ();
    stream.str ("[0]");
    ASSERT_NE (reader.deserialize (stream), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (writer.serialize (value), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), "[0]");

    stream.clear ();
    stream.str ("[\"foo\"]");
    ASSERT_NE (reader.deserialize (stream), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (writer.serialize (value), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), "[\"foo\"]");

    stream.clear ();
    stream.str ("[]");
    ASSERT_NE (reader.deserialize (stream), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (writer.serialize (value), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), "[]");

    stream.clear ();
    stream.str ("{}");
    ASSERT_NE (reader.deserialize (stream), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (writer.serialize (value), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), "{}");

    stream.clear ();
    stream.str ("[0,1]");
    ASSERT_NE (reader.deserialize (stream), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (writer.serialize (value), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), "[0,1]");

    stream.clear ();
    stream.str ("{\"foo\":\"bar\"}");
    ASSERT_NE (reader.deserialize (stream), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (writer.serialize (value), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), "{\"foo\":\"bar\"}");

    stream.clear ();
    stream.str ("{\"a\":null,\"foo\":\"bar\"}");
    ASSERT_NE (reader.deserialize (stream), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (writer.serialize (value), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), "{\"a\":null,\"foo\":\"bar\"}");

    stream.clear ();
    stream.str ("[-1]");
    ASSERT_NE (reader.deserialize (stream), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (writer.serialize (value), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), "[-1]");

    stream.clear ();
    stream.str ("[-2147483648]");
    ASSERT_NE (reader.deserialize (stream), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (writer.serialize (value), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), "[-2147483648]");

    stream.clear ();
    stream.str ("[-1234567890123456789]");
    ASSERT_NE (reader.deserialize (stream), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (writer.serialize (value), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), "[-1234567890123456789]");

    stream.clear ();
    stream.str ("[-9223372036854775808]");
    ASSERT_NE (reader.deserialize (stream), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (writer.serialize (value), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), "[-9223372036854775808]");

    stream.clear ();
    stream.str ("[1]");
    ASSERT_NE (reader.deserialize (stream), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (writer.serialize (value), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), "[1]");

    stream.clear ();
    stream.str ("[2147483647]");
    ASSERT_NE (reader.deserialize (stream), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (writer.serialize (value), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), "[2147483647]");

    stream.clear ();
    stream.str ("[4294967295]");
    ASSERT_NE (reader.deserialize (stream), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (writer.serialize (value), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), "[4294967295]");

    stream.clear ();
    stream.str ("[1234567890123456789]");
    ASSERT_NE (reader.deserialize (stream), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (writer.serialize (value), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), "[1234567890123456789]");

    stream.clear ();
    stream.str ("[9223372036854775807]");
    ASSERT_NE (reader.deserialize (stream), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (writer.serialize (value), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), "[9223372036854775807]");

    stream.clear ();
    stream.str ("[0.0]");
    ASSERT_NE (reader.deserialize (stream), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (writer.serialize (value), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), "[0.0]");

    stream.clear ();
    stream.str ("[-0.0]");
    ASSERT_NE (reader.deserialize (stream), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (writer.serialize (value), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), "[-0.0]");

    stream.clear ();
    stream.str ("[1.2345]");
    ASSERT_NE (reader.deserialize (stream), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (writer.serialize (value), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), "[1.2345]");

    stream.clear ();
    stream.str ("[-1.2345]");
    ASSERT_NE (reader.deserialize (stream), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (writer.serialize (value), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), "[-1.2345]");

    stream.clear ();
    stream.str ("[5e-324]");
    ASSERT_NE (reader.deserialize (stream), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (writer.serialize (value), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), "[5e-324]");

    stream.clear ();
    stream.str ("[2.225073858507201e-308]");
    ASSERT_NE (reader.deserialize (stream), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (writer.serialize (value), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), "[2.225073858507201e-308]");

    stream.clear ();
    stream.str ("[2.2250738585072014e-308]");
    ASSERT_NE (reader.deserialize (stream), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (writer.serialize (value), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), "[2.2250738585072014e-308]");

    stream.clear ();
    stream.str ("[1.7976931348623157e308]");
    ASSERT_NE (reader.deserialize (stream), -1) << join::lastError.message ();
    stream.str ("");
    stream.clear ();
    ASSERT_NE (writer.serialize (value), -1) << join::lastError.message ();
    EXPECT_EQ (stream.str (), "[1.7976931348623157e308]");
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}

