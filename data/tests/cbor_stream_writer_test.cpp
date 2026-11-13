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
#include <limits>
#include <vector>
#include <span>
#include <bit>

// C.
#include <cmath>

using join::CborStreamWriter;
using join::Value;
using join::Object;
using join::SaxErrc;

/**
 * @brief Test setNull method.
 */
TEST (CborStreamWriter, setNull)
{
    std::stringstream stream;
    CborStreamWriter cborStreamWriter (stream);

    stream.str ("");
    EXPECT_EQ (cborStreamWriter.setNull (), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xF6'}));
}

/**
 * @brief Test setBool method.
 */
TEST (CborStreamWriter, setBool)
{
    std::stringstream stream;
    CborStreamWriter cborStreamWriter (stream);

    stream.str ("");
    EXPECT_EQ (cborStreamWriter.setBool (true), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xF5'}));

    stream.str ("");
    EXPECT_EQ (cborStreamWriter.setBool (false), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xF4'}));
}

/**
 * @brief Test setInt method.
 */
TEST (CborStreamWriter, setInt)
{
    std::stringstream stream;
    CborStreamWriter cborStreamWriter (stream);

    auto write = [&] (int32_t value) {
        stream.str ("");
        EXPECT_EQ (cborStreamWriter.setInt (value), 0);
        return stream.str ();
    };

    EXPECT_EQ (write (0), std::string ({'\x00'}));
    EXPECT_EQ (write (23), std::string ({'\x17'}));
    EXPECT_EQ (write (24), std::string ({'\x18', '\x18'}));
    EXPECT_EQ (write (std::numeric_limits<int32_t>::max ()), std::string ({'\x1A', '\x7F', '\xFF', '\xFF', '\xFF'}));
    EXPECT_EQ (write (-1), std::string ({'\x20'}));
    EXPECT_EQ (write (-24), std::string ({'\x37'}));
    EXPECT_EQ (write (-25), std::string ({'\x38', '\x18'}));
    EXPECT_EQ (write (-256), std::string ({'\x38', '\xFF'}));
    EXPECT_EQ (write (-257), std::string ({'\x39', '\x01', '\x00'}));
    EXPECT_EQ (write (-1000), std::string ({'\x39', '\x03', '\xE7'}));
    EXPECT_EQ (write (-65537), std::string ({'\x3A', '\x00', '\x01', '\x00', '\x00'}));
    EXPECT_EQ (write (std::numeric_limits<int32_t>::min ()), std::string ({'\x3A', '\x7F', '\xFF', '\xFF', '\xFF'}));
}

/**
 * @brief Test setUint method.
 */
TEST (CborStreamWriter, setUint)
{
    std::stringstream stream;
    CborStreamWriter cborStreamWriter (stream);

    auto write = [&] (uint32_t value) {
        stream.str ("");
        EXPECT_EQ (cborStreamWriter.setUint (value), 0);
        return stream.str ();
    };

    EXPECT_EQ (write (0), std::string ({'\x00'}));
    EXPECT_EQ (write (23), std::string ({'\x17'}));
    EXPECT_EQ (write (24), std::string ({'\x18', '\x18'}));
    EXPECT_EQ (write (255), std::string ({'\x18', '\xFF'}));
    EXPECT_EQ (write (256), std::string ({'\x19', '\x01', '\x00'}));
    EXPECT_EQ (write (1000), std::string ({'\x19', '\x03', '\xE8'}));
    EXPECT_EQ (write (65535), std::string ({'\x19', '\xFF', '\xFF'}));
    EXPECT_EQ (write (65536), std::string ({'\x1A', '\x00', '\x01', '\x00', '\x00'}));
    EXPECT_EQ (write (std::numeric_limits<uint32_t>::max ()), std::string ({'\x1A', '\xFF', '\xFF', '\xFF', '\xFF'}));
}

/**
 * @brief Test setInt64 method.
 */
TEST (CborStreamWriter, setInt64)
{
    std::stringstream stream;
    CborStreamWriter cborStreamWriter (stream);

    auto write = [&] (int64_t value) {
        stream.str ("");
        EXPECT_EQ (cborStreamWriter.setInt64 (value), 0);
        return stream.str ();
    };

    EXPECT_EQ (write (-1), std::string ({'\x20'}));
    EXPECT_EQ (write (-4294967296LL), std::string ({'\x3A', '\xFF', '\xFF', '\xFF', '\xFF'}));
    EXPECT_EQ (write (-4294967297LL),
               std::string ({'\x3B', '\x00', '\x00', '\x00', '\x01', '\x00', '\x00', '\x00', '\x00'}));
    EXPECT_EQ (write (std::numeric_limits<int64_t>::min ()),
               std::string ({'\x3B', '\x7F', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF'}));
    EXPECT_EQ (write (4294967296LL),
               std::string ({'\x1B', '\x00', '\x00', '\x00', '\x01', '\x00', '\x00', '\x00', '\x00'}));
    EXPECT_EQ (write (std::numeric_limits<int64_t>::max ()),
               std::string ({'\x1B', '\x7F', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF'}));
}

/**
 * @brief Test setUint64 method.
 */
TEST (CborStreamWriter, setUint64)
{
    std::stringstream stream;
    CborStreamWriter cborStreamWriter (stream);

    auto write = [&] (uint64_t value) {
        stream.str ("");
        EXPECT_EQ (cborStreamWriter.setUint64 (value), 0);
        return stream.str ();
    };

    EXPECT_EQ (write (0), std::string ({'\x00'}));
    EXPECT_EQ (write (4294967295ULL), std::string ({'\x1A', '\xFF', '\xFF', '\xFF', '\xFF'}));
    EXPECT_EQ (write (4294967296ULL),
               std::string ({'\x1B', '\x00', '\x00', '\x00', '\x01', '\x00', '\x00', '\x00', '\x00'}));
    EXPECT_EQ (write (std::numeric_limits<uint64_t>::max ()),
               std::string ({'\x1B', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF', '\xFF'}));
}

/**
 * @brief Test setDouble method.
 */
TEST (CborStreamWriter, setDouble)
{
    std::stringstream stream;
    CborStreamWriter cborStreamWriter (stream);

    auto write = [&] (double value) {
        stream.str ("");
        EXPECT_EQ (cborStreamWriter.setDouble (value), 0);
        return stream.str ();
    };

    EXPECT_EQ (write (0.0), std::string ({'\xFA', '\x00', '\x00', '\x00', '\x00'}));
    EXPECT_EQ (write (-0.0), std::string ({'\xFA', '\x80', '\x00', '\x00', '\x00'}));
    EXPECT_EQ (write (1.5), std::string ({'\xFA', '\x3F', '\xC0', '\x00', '\x00'}));
    EXPECT_EQ (write (1.1), std::string ({'\xFB', '\x3F', '\xF1', '\x99', '\x99', '\x99', '\x99', '\x99', '\x9A'}));
    EXPECT_EQ (write (std::numeric_limits<double>::infinity ()),
               std::string ({'\xFA', '\x7F', '\x80', '\x00', '\x00'}));
    EXPECT_EQ (write (-std::numeric_limits<double>::infinity ()),
               std::string ({'\xFA', '\xFF', '\x80', '\x00', '\x00'}));
    EXPECT_EQ (write (std::numeric_limits<double>::quiet_NaN ()),
               std::string ({'\xFA', '\x7F', '\xC0', '\x00', '\x00'}));
    EXPECT_EQ (write (std::bit_cast<double> (uint64_t (0x7ff8000000000001))),
               std::string ({'\xFB', '\x7F', '\xF8', '\x00', '\x00', '\x00', '\x00', '\x00', '\x01'}));
    EXPECT_EQ (write (std::numeric_limits<float>::max ()), std::string ({'\xFA', '\x7F', '\x7F', '\xFF', '\xFF'}));
    EXPECT_EQ (write (std::nextafter (static_cast<double> (std::numeric_limits<float>::max ()),
                                      std::numeric_limits<double>::infinity ())),
               std::string ({'\xFB', '\x47', '\xEF', '\xFF', '\xFF', '\xE0', '\x00', '\x00', '\x01'}));
    EXPECT_EQ (write (std::numeric_limits<float>::denorm_min ()),
               std::string ({'\xFA', '\x00', '\x00', '\x00', '\x01'}));
    EXPECT_EQ (write (std::numeric_limits<double>::denorm_min ()),
               std::string ({'\xFB', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x01'}));
    EXPECT_EQ (write (-std::numeric_limits<double>::quiet_NaN ()),
               std::string ({'\xFA', '\xFF', '\xC0', '\x00', '\x00'}));
    EXPECT_EQ (write (std::bit_cast<double> (uint64_t (0x7ff4000000000000))),
               std::string ({'\xFB', '\x7F', '\xF4', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00'}));
}

/**
 * @brief Test setString method.
 */
TEST (CborStreamWriter, setString)
{
    std::stringstream stream;
    CborStreamWriter cborStreamWriter (stream);

    stream.str ("");
    EXPECT_EQ (cborStreamWriter.setString (""), 0);
    EXPECT_EQ (stream.str (), std::string ({'\x60'}));

    stream.str ("");
    EXPECT_EQ (cborStreamWriter.setString (std::string_view ()), 0);
    EXPECT_EQ (stream.str (), std::string ({'\x60'}));

    stream.str ("");
    EXPECT_EQ (cborStreamWriter.setString ("IETF"), 0);
    EXPECT_EQ (stream.str (), std::string ({'\x64', 'I', 'E', 'T', 'F'}));

    stream.str ("");
    EXPECT_EQ (cborStreamWriter.setString (std::string (23, 'x')), 0);
    EXPECT_EQ (stream.str (), std::string ({'\x77'}) + std::string (23, 'x'));

    stream.str ("");
    EXPECT_EQ (cborStreamWriter.setString (std::string (24, 'x')), 0);
    EXPECT_EQ (stream.str (), std::string ({'\x78', '\x18'}) + std::string (24, 'x'));

    stream.str ("");
    EXPECT_EQ (cborStreamWriter.setString (std::string (256, 'x')), 0);
    EXPECT_EQ (stream.str (), std::string ({'\x79', '\x01', '\x00'}) + std::string (256, 'x'));

    stream.str ("");
    EXPECT_EQ (cborStreamWriter.setString (std::string (65536, 'x')), 0);
    EXPECT_EQ (stream.str (), std::string ({'\x7A', '\x00', '\x01', '\x00', '\x00'}) + std::string (65536, 'x'));
}

/**
 * @brief Test setBinary method.
 */
TEST (CborStreamWriter, setBinary)
{
    std::stringstream stream;
    CborStreamWriter cborStreamWriter (stream);
    const std::vector<uint8_t> data (65536, 0x78);
    const uint8_t bytes[] = {0x01, 0x02, 0x03, 0x04};

    stream.str ("");
    EXPECT_EQ (cborStreamWriter.setBinary (std::span<const uint8_t> ()), 0);
    EXPECT_EQ (stream.str (), std::string ({'\x40'}));

    stream.str ("");
    EXPECT_EQ (cborStreamWriter.setBinary (bytes), 0);
    EXPECT_EQ (stream.str (), std::string ({'\x44', '\x01', '\x02', '\x03', '\x04'}));

    stream.str ("");
    EXPECT_EQ (cborStreamWriter.setBinary (bytes, -1), 0);
    EXPECT_EQ (stream.str (), std::string ({'\x44', '\x01', '\x02', '\x03', '\x04'}));

    stream.str ("");
    EXPECT_EQ (cborStreamWriter.setBinary (std::span<const uint8_t> (data.data (), 256)), 0);
    EXPECT_EQ (stream.str (), std::string ({'\x59', '\x01', '\x00'}) + std::string (256, 'x'));

    stream.str ("");
    EXPECT_EQ (cborStreamWriter.setBinary (data), 0);
    EXPECT_EQ (stream.str (), std::string ({'\x5A', '\x00', '\x01', '\x00', '\x00'}) + std::string (65536, 'x'));
}

/**
 * @brief Test startArray method.
 */
TEST (CborStreamWriter, startArray)
{
    std::stringstream stream;
    CborStreamWriter cborStreamWriter (stream);

    auto array = [&] (uint32_t size) {
        stream.str ("");
        EXPECT_EQ (cborStreamWriter.startArray (size), 0);
        EXPECT_EQ (cborStreamWriter.stopArray (), 0);
        return stream.str ();
    };

    EXPECT_EQ (array (0), std::string ({'\x9F', '\xFF'}));
    EXPECT_EQ (array (23), std::string ({'\x9F', '\xFF'}));
    EXPECT_EQ (array (24), std::string ({'\x9F', '\xFF'}));
    EXPECT_EQ (array (256), std::string ({'\x9F', '\xFF'}));
    EXPECT_EQ (array (65536), std::string ({'\x9F', '\xFF'}));
}

/**
 * @brief Test stopArray method.
 */
TEST (CborStreamWriter, stopArray)
{
    std::stringstream stream;
    CborStreamWriter cborStreamWriter (stream);

    EXPECT_EQ (cborStreamWriter.stopArray (), -1);
    EXPECT_EQ (join::lastError, SaxErrc::InvalidParent);

    EXPECT_EQ (cborStreamWriter.startArray (1), 0);
    EXPECT_EQ (cborStreamWriter.setNull (), 0);
    EXPECT_EQ (cborStreamWriter.stopArray (), 0);
    EXPECT_EQ (stream.str (), std::string ({'\x9F', '\xF6', '\xFF'}));
}

/**
 * @brief Test startObject method.
 */
TEST (CborStreamWriter, startObject)
{
    std::stringstream stream;
    CborStreamWriter cborStreamWriter (stream);

    auto object = [&] (uint32_t size) {
        stream.str ("");
        EXPECT_EQ (cborStreamWriter.startObject (size), 0);
        EXPECT_EQ (cborStreamWriter.stopObject (), 0);
        return stream.str ();
    };

    EXPECT_EQ (object (0), std::string ({'\xBF', '\xFF'}));
    EXPECT_EQ (object (23), std::string ({'\xBF', '\xFF'}));
    EXPECT_EQ (object (24), std::string ({'\xBF', '\xFF'}));
    EXPECT_EQ (object (256), std::string ({'\xBF', '\xFF'}));
    EXPECT_EQ (object (65536), std::string ({'\xBF', '\xFF'}));
}

/**
 * @brief Test members method.
 */
TEST (CborStreamWriter, members)
{
    std::stringstream stream;
    CborStreamWriter cborStreamWriter (stream);
    Value members;
    members["b"] = 1;
    members["a"] = 2;
    const Object& object = members.getObject ();
    EXPECT_EQ (&cborStreamWriter.members (object), &object);
}

/**
 * @brief Test setKey method.
 */
TEST (CborStreamWriter, setKey)
{
    std::stringstream stream;
    CborStreamWriter cborStreamWriter (stream);

    auto key = [&] (auto... value) {
        stream.str ("");
        cborStreamWriter.reset ();
        cborStreamWriter.startObject (1);
        int result = cborStreamWriter.setKey (value...);
        cborStreamWriter.flush ();
        return result;
    };

    EXPECT_EQ (cborStreamWriter.setKey (nullptr), -1);
    EXPECT_EQ (join::lastError, SaxErrc::InvalidParent);
    EXPECT_EQ (cborStreamWriter.setKey (true), -1);
    EXPECT_EQ (join::lastError, SaxErrc::InvalidParent);
    EXPECT_EQ (cborStreamWriter.setKey (int32_t (-2)), -1);
    EXPECT_EQ (join::lastError, SaxErrc::InvalidParent);
    EXPECT_EQ (cborStreamWriter.setKey (uint32_t (3)), -1);
    EXPECT_EQ (join::lastError, SaxErrc::InvalidParent);
    EXPECT_EQ (cborStreamWriter.setKey (int64_t (-4)), -1);
    EXPECT_EQ (join::lastError, SaxErrc::InvalidParent);
    EXPECT_EQ (cborStreamWriter.setKey (uint64_t (5)), -1);
    EXPECT_EQ (join::lastError, SaxErrc::InvalidParent);
    EXPECT_EQ (cborStreamWriter.setKey (6.5), -1);
    EXPECT_EQ (join::lastError, SaxErrc::InvalidParent);
    EXPECT_EQ (cborStreamWriter.setKey (std::string_view ("seven")), -1);
    EXPECT_EQ (join::lastError, SaxErrc::InvalidParent);
    EXPECT_EQ (cborStreamWriter.setKey ("eight"), -1);
    EXPECT_EQ (join::lastError, SaxErrc::InvalidParent);
    EXPECT_EQ (cborStreamWriter.setKey (std::span<const uint8_t> ()), -1);
    EXPECT_EQ (join::lastError, SaxErrc::InvalidParent);

    EXPECT_EQ (key (nullptr), 0);
    EXPECT_EQ (stream.str ().substr (1), std::string ({'\xF6'}));

    EXPECT_EQ (key (true), 0);
    EXPECT_EQ (stream.str ().substr (1), std::string ({'\xF5'}));

    EXPECT_EQ (key (false), 0);
    EXPECT_EQ (stream.str ().substr (1), std::string ({'\xF4'}));

    EXPECT_EQ (key (42), 0);
    EXPECT_EQ (stream.str ().substr (1), std::string ({'\x18', '\x2A'}));

    EXPECT_EQ (key (-10), 0);
    EXPECT_EQ (stream.str ().substr (1), std::string ({'\x29'}));

    EXPECT_EQ (key (uint32_t (300)), 0);
    EXPECT_EQ (stream.str ().substr (1), std::string ({'\x19', '\x01', '\x2C'}));

    EXPECT_EQ (key (int64_t (-10)), 0);
    EXPECT_EQ (stream.str ().substr (1), std::string ({'\x29'}));

    EXPECT_EQ (key (uint64_t (42)), 0);
    EXPECT_EQ (stream.str ().substr (1), std::string ({'\x18', '\x2A'}));

    EXPECT_EQ (key (1.5), 0);
    EXPECT_EQ (stream.str ().substr (1), std::string ({'\xFA', '\x3F', '\xC0', '\x00', '\x00'}));

    EXPECT_EQ (key ("a"), 0);
    EXPECT_EQ (stream.str ().substr (1), std::string ({'\x61', 'a'}));

    EXPECT_EQ (key (std::string_view ("seven")), 0);
    EXPECT_EQ (stream.str ().substr (1), std::string ({'\x65', 's', 'e', 'v', 'e', 'n'}));

    const uint8_t bytes[] = {0x00, 0xff};
    EXPECT_EQ (key (std::span<const uint8_t> (bytes, sizeof (bytes))), 0);
    EXPECT_EQ (stream.str ().substr (1), std::string ({'\x42', '\x00', '\xFF'}));

    EXPECT_EQ (key (std::span<const uint8_t> (bytes, sizeof (bytes)), -1), 0);
    EXPECT_EQ (stream.str ().substr (1), std::string ({'\x42', '\x00', '\xFF'}));
}

/**
 * @brief Test startKey method.
 */
TEST (CborStreamWriter, startKey)
{
    std::stringstream stream;
    CborStreamWriter cborStreamWriter (stream);

    EXPECT_EQ (cborStreamWriter.startKey (), -1);
    EXPECT_EQ (join::lastError, SaxErrc::InvalidParent);

    EXPECT_EQ (cborStreamWriter.startObject (1), 0);
    EXPECT_EQ (cborStreamWriter.startKey (), 0);
}

/**
 * @brief Test stopObject method.
 */
TEST (CborStreamWriter, stopObject)
{
    std::stringstream stream;
    CborStreamWriter cborStreamWriter (stream);

    EXPECT_EQ (cborStreamWriter.stopObject (), -1);
    EXPECT_EQ (join::lastError, SaxErrc::InvalidParent);

    EXPECT_EQ (cborStreamWriter.startObject (1), 0);
    EXPECT_EQ (cborStreamWriter.setKey (nullptr), 0);
    EXPECT_EQ (cborStreamWriter.setNull (), 0);
    EXPECT_EQ (cborStreamWriter.stopObject (), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xBF', '\xF6', '\xF6', '\xFF'}));
}

/**
 * @brief Test flush method.
 */
TEST (CborStreamWriter, flush)
{
    std::stringstream stream;
    CborStreamWriter cborStreamWriter (stream);

    EXPECT_EQ (cborStreamWriter.startArray (2), 0);
    EXPECT_EQ (cborStreamWriter.setNull (), 0);
    EXPECT_EQ (stream.str (), "");
    cborStreamWriter.flush ();
    EXPECT_EQ (stream.str (), std::string ({'\x9F', '\xF6'}));
    cborStreamWriter.flush ();
    EXPECT_EQ (stream.str (), std::string ({'\x9F', '\xF6'}));
    EXPECT_EQ (cborStreamWriter.setNull (), 0);
    EXPECT_EQ (cborStreamWriter.stopArray (), 0);
    EXPECT_EQ (stream.str (), std::string ({'\x9F', '\xF6', '\xF6', '\xFF'}));

    stream.str ("");
    EXPECT_EQ (cborStreamWriter.startArray (1100), 0);
    for (int i = 0; i < 1100; ++i)
    {
        EXPECT_EQ (cborStreamWriter.setNull (), 0);
    }
    EXPECT_EQ (stream.str ().size (), 1024u);
    EXPECT_EQ (cborStreamWriter.stopArray (), 0);
    EXPECT_EQ (stream.str ().size (), 1102u);
}

/**
 * @brief Test reset method.
 */
TEST (CborStreamWriter, reset)
{
    std::stringstream stream;
    CborStreamWriter cborStreamWriter (stream);

    EXPECT_EQ (cborStreamWriter.startArray (2), 0);
    EXPECT_EQ (cborStreamWriter.setNull (), 0);
    cborStreamWriter.reset ();
    EXPECT_EQ (stream.str (), "");
    EXPECT_EQ (cborStreamWriter.setNull (), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xF6'}));
}

/**
 * @brief Test writing a whole document.
 */
TEST (CborStreamWriter, document)
{
    Value value;
    value.pushBack (nullptr);
    value.pushBack (true);
    value.pushBack (int32_t (1));
    value.pushBack (uint32_t (2));
    value.pushBack (int64_t (3));
    value.pushBack (uint64_t (4));
    value.pushBack (1.23456789e-13);
    value.pushBack ("test");

    std::stringstream stream;
    CborStreamWriter cborStreamWriter (stream);

    stream.str ("");
    EXPECT_EQ (value.serialize (cborStreamWriter), 0) << join::lastError.message ();
    EXPECT_EQ (stream.str (),
               std::string ({'\x9F', '\xF6', '\xF5', '\x01', '\x02', '\x03', '\x04', '\xFB', '\x3D', '\x41', '\x5F',
                             '\xFF', '\xE5', '\x3A', '\x68', '\x5D', '\x64', 't',    'e',    's',    't',    '\xFF'}));

    value = nullptr;
    value["a"] = 1;
    stream.str ("");
    EXPECT_EQ (value.serialize (cborStreamWriter), 0) << join::lastError.message ();
    EXPECT_EQ (stream.str (), std::string ({'\xBF', '\x61', 'a', '\x01', '\xFF'}));

    Value key;
    key.pushBack (1);
    value = Object ();
    value.insert (key, 1);
    stream.str ("");
    EXPECT_EQ (value.serialize (cborStreamWriter), 0) << join::lastError.message ();
    EXPECT_EQ (stream.str (), std::string ({'\xBF', '\x9F', '\x01', '\xFF', '\x01', '\xFF'}));
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
