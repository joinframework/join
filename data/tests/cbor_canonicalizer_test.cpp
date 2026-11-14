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
#include <join/value.hpp>
#include <join/cbor.hpp>

// libraries.
#include <gtest/gtest.h>

// C++.
#include <sstream>
#include <string>

using join::CborCanonicalizer;
using join::CborReader;
using join::Value;
using join::Object;
using join::SaxErrc;

/**
 * @brief Test members method.
 */
TEST (CborCanonicalizer, members)
{
    std::stringstream stream;
    CborCanonicalizer cborWriter (stream);

    Value strings;
    strings["aa"] = 1;
    strings["z"] = 2;
    strings["b"] = 3;
    auto sorted = cborWriter.members (strings.getObject ());
    ASSERT_EQ (sorted.size (), 3U);
    EXPECT_EQ (sorted[0].get ().first, "b");
    EXPECT_EQ (sorted[1].get ().first, "z");
    EXPECT_EQ (sorted[2].get ().first, "aa");

    Value integers;
    integers[100] = 1;
    integers[-1] = 2;
    integers[10] = 3;
    integers[0] = 4;
    sorted = cborWriter.members (integers.getObject ());
    ASSERT_EQ (sorted.size (), 4U);
    EXPECT_EQ (sorted[0].get ().first, 0);
    EXPECT_EQ (sorted[1].get ().first, 10);
    EXPECT_EQ (sorted[2].get ().first, 100);
    EXPECT_EQ (sorted[3].get ().first, -1);

    Value key;
    key.pushBack (1);
    Value mixed;
    mixed[1.5] = 1;
    mixed[true] = 2;
    mixed.insert (key, 3);
    mixed["a"] = 4;
    mixed[-1] = 5;
    mixed[1] = 6;
    sorted = cborWriter.members (mixed.getObject ());
    ASSERT_EQ (sorted.size (), 6U);
    EXPECT_EQ (sorted[0].get ().first, 1);
    EXPECT_EQ (sorted[1].get ().first, -1);
    EXPECT_EQ (sorted[2].get ().first, "a");
    EXPECT_EQ (sorted[3].get ().first, key);
    EXPECT_EQ (sorted[4].get ().first, true);
    EXPECT_EQ (sorted[5].get ().first, 1.5);

    Value value;
    value["b"] = 1;
    value["a"] = 2;
    ASSERT_EQ (value.serialize (cborWriter), 0) << join::lastError.message ();
    EXPECT_EQ (stream.str (), std::string ({'\xA2', '\x61', 'a', '\x02', '\x61', 'b', '\x01'}));
    EXPECT_EQ (value.getObject ()[0].first, "b");
    EXPECT_EQ (value.getObject ()[1].first, "a");

    Value nested;
    nested["x"]["b"] = 1;
    nested["x"]["a"] = 2;
    stream.str ("");
    ASSERT_EQ (nested.serialize (cborWriter), 0) << join::lastError.message ();
    EXPECT_EQ (stream.str (), std::string ({'\xA1', '\x61', 'x', '\xA2', '\x61', 'a', '\x02', '\x61', 'b', '\x01'}));
}

/**
 * @brief Test stopObject method.
 */
TEST (CborCanonicalizer, stopObject)
{
    std::stringstream stream;
    CborCanonicalizer cborWriter (stream);

    EXPECT_EQ (cborWriter.stopObject (), -1);
    EXPECT_EQ (join::lastError, SaxErrc::InvalidParent);

    EXPECT_EQ (cborWriter.startObject (0), 0);
    EXPECT_EQ (cborWriter.stopObject (), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xA0'}));

    Value duplicate;
    std::stringstream in (std::string ({'\xA3', '\x61', 'a', '\x01', '\x61', 'b', '\x02', '\x61', 'a', '\x03'}));
    ASSERT_EQ (duplicate.deserialize<CborReader> (in), 0) << join::lastError.message ();
    stream.str ("");
    ASSERT_EQ (duplicate.serialize (cborWriter), -1);
    EXPECT_EQ (join::lastError, SaxErrc::DuplicateKey);
    EXPECT_TRUE (stream.str ().empty ());

    Value nested;
    std::stringstream nestedIn (std::string ({'\xA1', '\x61', 'x', '\xA2', '\x61', 'a', '\x01', '\x61', 'a', '\x02'}));
    ASSERT_EQ (nested.deserialize<CborReader> (nestedIn), 0) << join::lastError.message ();
    ASSERT_EQ (nested.serialize (cborWriter), -1);
    EXPECT_EQ (join::lastError, SaxErrc::DuplicateKey);
    EXPECT_TRUE (stream.str ().empty ());

    Value value;
    value["b"] = 1;
    value["a"] = 2;
    ASSERT_EQ (value.serialize (cborWriter), 0) << join::lastError.message ();
    EXPECT_EQ (stream.str (), std::string ({'\xA2', '\x61', 'a', '\x02', '\x61', 'b', '\x01'}));
}

/**
 * @brief Test flush method.
 */
TEST (CborCanonicalizer, flush)
{
    std::stringstream stream;
    CborCanonicalizer cborWriter (stream);

    EXPECT_EQ (cborWriter.startObject (1), 0);
    EXPECT_EQ (cborWriter.setKey ("a"), 0);
    EXPECT_EQ (cborWriter.setInt (1), 0);
    EXPECT_EQ (stream.str (), "");
    cborWriter.flush ();
    EXPECT_EQ (stream.str (), std::string ({'\xA1', '\x61', 'a', '\x01'}));
    EXPECT_EQ (cborWriter.stopObject (), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xA1', '\x61', 'a', '\x01'}));
}

/**
 * @brief Test reset method.
 */
TEST (CborCanonicalizer, reset)
{
    std::stringstream stream;
    CborCanonicalizer cborWriter (stream);

    EXPECT_EQ (cborWriter.startObject (1), 0);
    EXPECT_EQ (cborWriter.setKey ("a"), 0);
    cborWriter.reset ();
    EXPECT_EQ (stream.str (), "");
    EXPECT_EQ (cborWriter.setInt (2), 0);
    EXPECT_EQ (stream.str (), std::string ({'\x02'}));
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
