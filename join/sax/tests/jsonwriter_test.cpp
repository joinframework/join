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

// C++.
#include <sstream>

using join::sax::JsonWriter;

/**
 * @brief Test setNull method.
 */
TEST (JsonWriter, setNull)
{
    std::stringstream stream;
    JsonWriter jsonWriter (stream);

    stream.str ("");
    EXPECT_EQ (jsonWriter.setNull (), 0);
    EXPECT_EQ (stream.str (), "null");
}

/**
 * @brief Test setBool method.
 */
TEST (JsonWriter, setBool)
{
    std::stringstream stream;
    JsonWriter jsonWriter (stream);

    stream.str ("");
    EXPECT_EQ (jsonWriter.setBool (true), 0);
    EXPECT_EQ (stream.str (),"true");

    stream.str ("");
    EXPECT_EQ (jsonWriter.setBool (false), 0);
    EXPECT_EQ (stream.str (), "false");
}

/**
 * @brief Test setInt method.
 */
TEST (JsonWriter, setInt)
{
    std::stringstream stream;
    JsonWriter jsonWriter (stream);

    stream.str ("");
    EXPECT_EQ (jsonWriter.setInt (std::numeric_limits <int32_t>::min ()), 0);
    EXPECT_EQ (stream.str (), "-2147483648");

    stream.str ("");
    EXPECT_EQ (jsonWriter.setInt (-32768), 0);
    EXPECT_EQ (stream.str (), "-32768");

    stream.str ("");
    EXPECT_EQ (jsonWriter.setInt (1234567890), 0);
    EXPECT_EQ (stream.str (), "1234567890");
}

/**
 * @brief Test setUint method.
 */
TEST (JsonWriter, setUint)
{
    std::stringstream stream;
    JsonWriter jsonWriter (stream);

    stream.str ("");
    EXPECT_EQ (jsonWriter.setUint (0), 0);
    EXPECT_EQ (stream.str (), "0");

    stream.str ("");
    EXPECT_EQ (jsonWriter.setUint (1234567890), 0);
    EXPECT_EQ (stream.str (), "1234567890");
}

/**
 * @brief Test setInt64 method.
 */
TEST (JsonWriter, setInt64)
{
    std::stringstream stream;
    JsonWriter jsonWriter (stream);

    stream.str ("");
    EXPECT_EQ (jsonWriter.setInt64 (std::numeric_limits <int64_t>::min ()), 0);
    EXPECT_EQ (stream.str (), "-9223372036854775808");

    stream.str ("");
    EXPECT_EQ (jsonWriter.setInt64 (-32768), 0);
    EXPECT_EQ (stream.str (), "-32768");

    stream.str ("");
    EXPECT_EQ (jsonWriter.setInt64 (1234567890), 0);
    EXPECT_EQ (stream.str (), "1234567890");
}

/**
 * @brief Test setUint64 method.
 */
TEST (JsonWriter, setUint64)
{
    std::stringstream stream;
    JsonWriter jsonWriter (stream);

    stream.str ("");
    EXPECT_EQ (jsonWriter.setUint64 (0), 0);
    EXPECT_EQ (stream.str (), "0");

    stream.str ("");
    EXPECT_EQ (jsonWriter.setUint64 (1234567890), 0);
    EXPECT_EQ (stream.str (), "1234567890");
}

/**
 * @brief Test setDouble method.
 */
TEST (JsonWriter, setDouble)
{
    std::stringstream stream;
    JsonWriter jsonWriter (stream);

    stream.str ("");
    EXPECT_EQ (jsonWriter.setDouble (98.6), 0);
    EXPECT_EQ (stream.str (), "98.6");

    stream.str ("");
    EXPECT_EQ (jsonWriter.setDouble (1.0), 0);
    EXPECT_EQ (stream.str (), "1.0");

    stream.str ("");
    EXPECT_EQ (jsonWriter.setDouble (-9876.54321), 0);
    EXPECT_EQ (stream.str (), "-9876.54321");

    stream.str ("");
    EXPECT_EQ (jsonWriter.setDouble (1.23456789e-13), 0);
    EXPECT_EQ (stream.str (), "1.23456789e-13");

    stream.str ("");
    EXPECT_EQ (jsonWriter.setDouble (-std::numeric_limits <double>::infinity ()), 0);
    EXPECT_EQ (stream.str (), "-Inf");

    stream.str ("");
    EXPECT_EQ (jsonWriter.setDouble (std::numeric_limits <double>::infinity ()), 0);
    EXPECT_EQ (stream.str (), "Inf");

    stream.str ("");
    EXPECT_EQ (jsonWriter.setDouble (-std::numeric_limits <double>::quiet_NaN ()), 0);
    EXPECT_EQ (stream.str (), "-NaN");

    stream.str ("");
    EXPECT_EQ (jsonWriter.setDouble (std::numeric_limits <double>::quiet_NaN ()), 0);
    EXPECT_EQ (stream.str (), "NaN");
}

/**
 * @brief Test setString method.
 */
TEST (JsonWriter, setString)
{
    std::stringstream stream;
    JsonWriter jsonWriter (stream);

    stream.str ("");
    EXPECT_EQ (jsonWriter.setString ("foo"), 0);
    EXPECT_EQ (stream.str (), "\"foo\"");

    stream.str ("");
    EXPECT_EQ (jsonWriter.setString ("\"\\\b\f\n\r\t\x19"), 0);
    EXPECT_EQ (stream.str (), "\"\\\"\\\\\\b\\f\\n\\r\\t\\u0019\"");
}

/**
 * @brief Test startArray method.
 */
TEST (JsonWriter, startArray)
{
    std::stringstream stream;
    JsonWriter jsonWriter (stream, 2);

    stream.str ("");
    EXPECT_EQ (jsonWriter.startArray (), 0);
    EXPECT_EQ (jsonWriter.stopArray (), 0);
    EXPECT_EQ (stream.str (), "[]");

    stream.str ("");
    EXPECT_EQ (jsonWriter.startArray (2), 0);
    EXPECT_EQ (jsonWriter.setInt (1), 0);
    EXPECT_EQ (jsonWriter.setInt (2), 0);
    EXPECT_EQ (jsonWriter.stopArray (), 0);
    EXPECT_EQ (stream.str (), "[\n  1,\n  2\n]");
}

/**
 * @brief Test startObject method.
 */
TEST (JsonWriter, startObject)
{
    std::stringstream stream;
    JsonWriter jsonWriter (stream, 2);

    stream.str ("");
    EXPECT_EQ (jsonWriter.startObject (), 0);
    EXPECT_EQ (jsonWriter.stopObject (), 0);
    EXPECT_EQ (stream.str (), "{}");

    stream.str ("");
    EXPECT_EQ (jsonWriter.startObject (2), 0);
    EXPECT_EQ (jsonWriter.setKey ("foo"), 0);
    EXPECT_EQ (jsonWriter.setString ("bar"), 0);
    EXPECT_EQ (jsonWriter.setKey ("fuzz"), 0);
    EXPECT_EQ (jsonWriter.setString ("bazz"), 0);
    EXPECT_EQ (jsonWriter.stopObject (), 0);
    EXPECT_EQ (stream.str (), "{\n  \"foo\": \"bar\",\n  \"fuzz\": \"bazz\"\n}");
}

/**
 * @brief Test setKey method.
 */
TEST (JsonWriter, setKey)
{
    std::stringstream stream;
    JsonWriter jsonWriter (stream);

    stream.str ("");
    EXPECT_EQ (jsonWriter.setKey ("foo"), 0);
    EXPECT_EQ (stream.str (), "\"foo\":");

    stream.str ("");
    EXPECT_EQ (jsonWriter.setKey ("\"\\\b\f\n\r\t\x19"), 0);
    EXPECT_EQ (stream.str (), "\"\\\"\\\\\\b\\f\\n\\r\\t\\u0019\":");
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
