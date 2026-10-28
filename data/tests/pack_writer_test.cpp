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
#include <join/value.hpp>
#include <join/pack.hpp>

// libraries.
#include <gtest/gtest.h>

// C++.
#include <sstream>

using join::PackWriter;
using join::Value;
using join::Array;
using join::Object;
using join::SaxErrc;

/**
 * @brief Test setNull method.
 */
TEST (PackWriter, setNull)
{
    std::stringstream stream;
    PackWriter packWriter (stream);

    stream.str ("");
    EXPECT_EQ (packWriter.setNull (), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xC0'}));
}

/**
 * @brief Test setBool method.
 */
TEST (PackWriter, setBool)
{
    std::stringstream stream;
    PackWriter packWriter (stream);

    stream.str ("");
    EXPECT_EQ (packWriter.setBool (true), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xC3'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setBool (false), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xC2'}));
}

/**
 * @brief Test setInt method.
 */
TEST (PackWriter, setInt)
{
    std::stringstream stream;
    PackWriter packWriter (stream);

    stream.str ("");
    EXPECT_EQ (packWriter.setInt (-1234567890), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xD2', '\xB6', '\x69', '\xFD', '\x2E'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setInt (-32769), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xD2', '\xFF', '\xFF', '\x7F', '\xFF'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setInt (-32768), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xD1', '\x80', '\x00'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setInt (-1066), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xD1', '\xFB', '\xD6'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setInt (-33), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xD0', '\xDF'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setInt (-32), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xE0'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setInt (3), 0);
    EXPECT_EQ (stream.str (), std::string ({'\x03'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setInt (127), 0);
    EXPECT_EQ (stream.str (), std::string ({'\x7F'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setInt (128), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xCC', '\x80'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setInt (255), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xCC', '\xFF'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setInt (256), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xCD', '\x01', '\x00'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setInt (1066), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xCD', '\x04', '\x2A'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setInt (65535), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xCD', '\xFF', '\xFF'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setInt (65536), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xCE', '\x00', '\x01', '\x00', '\x00'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setInt (1234567890), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xCE', '\x49', '\x96', '\x02', '\xd2'}));
}

/**
 * @brief Test setUint method.
 */
TEST (PackWriter, setUint)
{
    std::stringstream stream;
    PackWriter packWriter (stream);

    stream.str ("");
    EXPECT_EQ (packWriter.setUint (3), 0);
    EXPECT_EQ (stream.str (), std::string ({'\x03'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setUint (127), 0);
    EXPECT_EQ (stream.str (), std::string ({'\x7F'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setUint (128), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xCC', '\x80'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setUint (255), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xCC', '\xFF'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setUint (256), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xCD', '\x01', '\x00'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setUint (1066), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xCD', '\x04', '\x2A'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setUint (65535), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xCD', '\xFF', '\xFF'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setUint (65536), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xCE', '\x00', '\x01', '\x00', '\x00'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setUint (1234567890), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xCE', '\x49', '\x96', '\x02', '\xd2'}));
}

/**
 * @brief Test setInt64 method.
 */
TEST (PackWriter, setInt64)
{
    std::stringstream stream;
    PackWriter packWriter (stream);

    stream.str ("");
    EXPECT_EQ (packWriter.setInt64 (-2147483649), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xD3', '\xFF', '\xFF', '\xFF', '\xFF', '\x7F', '\xFF', '\xFF', '\xFF'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setInt64 (-2147483648), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xD2', '\x80', '\x00', '\x00', '\x00'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setInt64 (-1234567890), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xD2', '\xB6', '\x69', '\xFD', '\x2E'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setInt64 (-32769), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xD2', '\xFF', '\xFF', '\x7F', '\xFF'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setInt64 (-32768), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xD1', '\x80', '\x00'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setInt64 (-1066), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xD1', '\xFB', '\xD6'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setInt64 (-33), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xD0', '\xDF'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setInt64 (-32), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xE0'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setInt64 (3), 0);
    EXPECT_EQ (stream.str (), std::string ({'\x03'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setInt64 (127), 0);
    EXPECT_EQ (stream.str (), std::string ({'\x7F'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setInt64 (128), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xCC', '\x80'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setInt64 (255), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xCC', '\xFF'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setInt64 (256), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xCD', '\x01', '\x00'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setInt64 (1066), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xCD', '\x04', '\x2A'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setInt64 (65535), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xCD', '\xFF', '\xFF'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setInt64 (65536), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xCE', '\x00', '\x01', '\x00', '\x00'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setInt64 (1234567890), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xCE', '\x49', '\x96', '\x02', '\xd2'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setInt64 (4294967295), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xCE', '\xFF', '\xFF', '\xFF', '\xFF'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setInt64 (4294967296), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xCF', '\x00', '\x00', '\x00', '\x01', '\x00', '\x00', '\x00', '\x00'}));
}

/**
 * @brief Test setUint64 method.
 */
TEST (PackWriter, setUint64)
{
    std::stringstream stream;
    PackWriter packWriter (stream);

    stream.str ("");
    EXPECT_EQ (packWriter.setUint64 (3), 0);
    EXPECT_EQ (stream.str (), std::string ({'\x03'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setUint64 (127), 0);
    EXPECT_EQ (stream.str (), std::string ({'\x7F'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setUint64 (128), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xCC', '\x80'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setUint64 (255), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xCC', '\xFF'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setUint64 (256), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xCD', '\x01', '\x00'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setUint64 (1066), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xCD', '\x04', '\x2A'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setUint64 (65535), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xCD', '\xFF', '\xFF'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setUint64 (65536), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xCE', '\x00', '\x01', '\x00', '\x00'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setUint64 (1234567890), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xCE', '\x49', '\x96', '\x02', '\xd2'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setUint64 (4294967295), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xCE', '\xFF', '\xFF', '\xFF', '\xFF'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setUint64 (4294967296), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xCF', '\x00', '\x00', '\x00', '\x01', '\x00', '\x00', '\x00', '\x00'}));
}

/**
 * @brief Test setDouble method.
 */
TEST (PackWriter, setDouble)
{
    std::stringstream stream;
    PackWriter packWriter (stream);

    stream.str ("");
    EXPECT_EQ (packWriter.setDouble (98.6), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xCB', '\x40', '\x58', '\xA6', '\x66', '\x66', '\x66', '\x66', '\x66'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setDouble (0.1e1), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xCA', '\x3F', '\x80', '\x00', '\x00'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setDouble (-9876.543210), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xCB', '\xC0', '\xC3', '\x4A', '\x45', '\x87', '\xE7', '\xC0', '\x6E'}));

    stream.str ("");
    EXPECT_EQ (packWriter.setDouble (0.123456789e-12), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xCB', '\x3D', '\x41', '\x5F', '\xFF', '\xE5', '\x3A', '\x68', '\x5D'}));
}

/**
 * @brief Test setString method.
 */
TEST (PackWriter, setString)
{
    std::stringstream stream;
    PackWriter packWriter (stream);

    stream.str ("");
    EXPECT_EQ (packWriter.setString (std::string (31, 'x')), 0);
    EXPECT_EQ (stream.str ().compare (0, 6, std::string ({'\xBF', '\x78', '\x78', '\x78', '\x78', '\x78'})), 0);

    stream.str ("");
    EXPECT_EQ (packWriter.setString (std::string (32, 'x')), 0);
    EXPECT_EQ (stream.str ().compare (0, 6, std::string ({'\xD9', '\x20', '\x78', '\x78', '\x78', '\x78'})), 0);

    stream.str ("");
    EXPECT_EQ (packWriter.setString (std::string (255, 'x')), 0);
    EXPECT_EQ (stream.str ().compare (0, 6, std::string ({'\xD9', '\xFF', '\x78', '\x78', '\x78', '\x78'})), 0);

    stream.str ("");
    EXPECT_EQ (packWriter.setString (std::string (256, 'x')), 0);
    EXPECT_EQ (stream.str ().compare (0, 6, std::string ({'\xDA', '\x01', '\x00', '\x78', '\x78', '\x78'})), 0);

    stream.str ("");
    EXPECT_EQ (packWriter.setString (std::string (65535, 'x')), 0);
    EXPECT_EQ (stream.str ().compare (0, 6, std::string ({'\xDA', '\xFF', '\xFF', '\x78', '\x78', '\x78'})), 0);

    stream.str ("");
    EXPECT_EQ (packWriter.setString (std::string (65536, 'x')), 0);
    EXPECT_EQ (stream.str ().compare (0, 6, std::string ({'\xDB', '\x00', '\x01', '\x00', '\x00', '\x78'})), 0);
}

/**
 * @brief Test startArray method.
 */
TEST (PackWriter, startArray)
{
    std::stringstream stream;
    PackWriter packWriter (stream);

    stream.str ("");
    EXPECT_EQ (packWriter.startArray (15), 0);
    EXPECT_EQ (packWriter.stopArray (), 0);
    EXPECT_EQ (stream.str (), std::string ({'\x9F'}));

    stream.str ("");
    EXPECT_EQ (packWriter.startArray (16), 0);
    EXPECT_EQ (packWriter.stopArray (), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xDC', '\x00', '\x10'}));

    stream.str ("");
    EXPECT_EQ (packWriter.startArray (65535), 0);
    EXPECT_EQ (packWriter.stopArray (), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xDC', '\xFF', '\xFF'}));

    stream.str ("");
    EXPECT_EQ (packWriter.startArray (65536), 0);
    EXPECT_EQ (packWriter.stopArray (), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xDD', '\x00', '\x01', '\x00', '\x00'}));

    stream.str ("");
    EXPECT_EQ (packWriter.startArray (2000), 0);
    for (int i = 0; i < 2000; ++i)
    {
        ASSERT_EQ (packWriter.setNull (), 0);
    }
    EXPECT_EQ (packWriter.stopArray (), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xDC', '\x07', '\xD0'}) + std::string (2000, '\xC0'));
}

/**
 * @brief Test stopArray method.
 */
TEST (PackWriter, stopArray)
{
    std::stringstream stream;
    PackWriter packWriter (stream);

    EXPECT_EQ (packWriter.stopArray (), -1);
    EXPECT_EQ (join::lastError, SaxErrc::InvalidParent);

    EXPECT_EQ (packWriter.startArray (1), 0);
    EXPECT_EQ (packWriter.setNull (), 0);
    EXPECT_EQ (packWriter.stopArray (), 0);
    EXPECT_EQ (stream.str (), std::string ({'\x91', '\xC0'}));
}

/**
 * @brief Test startObject method.
 */
TEST (PackWriter, startObject)
{
    std::stringstream stream;
    PackWriter packWriter (stream);

    stream.str ("");
    EXPECT_EQ (packWriter.startObject (15), 0);
    EXPECT_EQ (packWriter.stopObject (), 0);
    EXPECT_EQ (stream.str (), std::string ({'\x8F'}));

    stream.str ("");
    EXPECT_EQ (packWriter.startObject (16), 0);
    EXPECT_EQ (packWriter.stopObject (), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xDE', '\x00', '\x10'}));

    stream.str ("");
    EXPECT_EQ (packWriter.startObject (65535), 0);
    EXPECT_EQ (packWriter.stopObject (), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xDE', '\xFF', '\xFF'}));

    stream.str ("");
    EXPECT_EQ (packWriter.startObject (65536), 0);
    EXPECT_EQ (packWriter.stopObject (), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xDF', '\x00', '\x01', '\x00', '\x00'}));
}

/**
 * @brief Test members method.
 */
TEST (PackWriter, members)
{
    std::stringstream stream;
    PackWriter packWriter (stream);
    Object object{{"b", 1}, {"a", 2}};
    EXPECT_EQ (&packWriter.members (object), &object);
}

/**
 * @brief Test setKey method.
 */
TEST (PackWriter, setKey)
{
    std::stringstream stream;
    PackWriter packWriter (stream);

    auto key = [&] (auto value) {
        stream.str ("");
        packWriter.reset ();
        packWriter.startObject (1);
        int result = packWriter.setKey (value);
        packWriter.flush ();
        return result;
    };

    EXPECT_EQ (packWriter.setKey (nullptr), -1);
    EXPECT_EQ (join::lastError, SaxErrc::InvalidParent);
    EXPECT_EQ (packWriter.setKey (true), -1);
    EXPECT_EQ (join::lastError, SaxErrc::InvalidParent);
    EXPECT_EQ (packWriter.setKey (int32_t (-2)), -1);
    EXPECT_EQ (join::lastError, SaxErrc::InvalidParent);
    EXPECT_EQ (packWriter.setKey (uint32_t (3)), -1);
    EXPECT_EQ (join::lastError, SaxErrc::InvalidParent);
    EXPECT_EQ (packWriter.setKey (int64_t (-4)), -1);
    EXPECT_EQ (join::lastError, SaxErrc::InvalidParent);
    EXPECT_EQ (packWriter.setKey (uint64_t (5)), -1);
    EXPECT_EQ (join::lastError, SaxErrc::InvalidParent);
    EXPECT_EQ (packWriter.setKey (6.5), -1);
    EXPECT_EQ (join::lastError, SaxErrc::InvalidParent);
    EXPECT_EQ (packWriter.setKey (std::string_view ("seven")), -1);
    EXPECT_EQ (join::lastError, SaxErrc::InvalidParent);
    EXPECT_EQ (packWriter.setKey ("eight"), -1);
    EXPECT_EQ (join::lastError, SaxErrc::InvalidParent);

    EXPECT_EQ (key (nullptr), 0);
    EXPECT_EQ (stream.str ().substr (1), std::string ({'\xC0'}));

    EXPECT_EQ (key (true), 0);
    EXPECT_EQ (stream.str ().substr (1), std::string ({'\xC3'}));

    EXPECT_EQ (key (false), 0);
    EXPECT_EQ (stream.str ().substr (1), std::string ({'\xC2'}));

    EXPECT_EQ (key (42), 0);
    EXPECT_EQ (stream.str ().substr (1), std::string ({'\x2A'}));

    EXPECT_EQ (key (300), 0);
    EXPECT_EQ (stream.str ().substr (1), std::string ({'\xCD', '\x01', '\x2C'}));

    EXPECT_EQ (key (-10), 0);
    EXPECT_EQ (stream.str ().substr (1), std::string ({'\xF6'}));

    EXPECT_EQ (key (uint32_t (42)), 0);
    EXPECT_EQ (stream.str ().substr (1), std::string ({'\x2A'}));

    EXPECT_EQ (key (int64_t (-10)), 0);
    EXPECT_EQ (stream.str ().substr (1), std::string ({'\xF6'}));

    EXPECT_EQ (key (uint64_t (42)), 0);
    EXPECT_EQ (stream.str ().substr (1), std::string ({'\x2A'}));

    EXPECT_EQ (key (0.1e1), 0);
    EXPECT_EQ (stream.str ().substr (1), std::string ({'\xCA', '\x3F', '\x80', '\x00', '\x00'}));

    EXPECT_EQ (key (98.6), 0);
    EXPECT_EQ (stream.str ().substr (1),
               std::string ({'\xCB', '\x40', '\x58', '\xA6', '\x66', '\x66', '\x66', '\x66', '\x66'}));

    EXPECT_EQ (key (std::string (31, 'x')), 0);
    EXPECT_EQ (stream.str ().compare (1, 6, std::string ({'\xBF', '\x78', '\x78', '\x78', '\x78', '\x78'})), 0);

    EXPECT_EQ (key (std::string (32, 'x')), 0);
    EXPECT_EQ (stream.str ().compare (1, 6, std::string ({'\xD9', '\x20', '\x78', '\x78', '\x78', '\x78'})), 0);

    EXPECT_EQ (key (std::string (255, 'x')), 0);
    EXPECT_EQ (stream.str ().compare (1, 6, std::string ({'\xD9', '\xFF', '\x78', '\x78', '\x78', '\x78'})), 0);

    EXPECT_EQ (key (std::string (256, 'x')), 0);
    EXPECT_EQ (stream.str ().compare (1, 6, std::string ({'\xDA', '\x01', '\x00', '\x78', '\x78', '\x78'})), 0);

    EXPECT_EQ (key (std::string (65535, 'x')), 0);
    EXPECT_EQ (stream.str ().compare (1, 6, std::string ({'\xDA', '\xFF', '\xFF', '\x78', '\x78', '\x78'})), 0);

    EXPECT_EQ (key (std::string (65536, 'x')), 0);
    EXPECT_EQ (stream.str ().compare (1, 6, std::string ({'\xDB', '\x00', '\x01', '\x00', '\x00', '\x78'})), 0);

    EXPECT_EQ (key ("a"), 0);
    EXPECT_EQ (stream.str ().substr (1), std::string ({'\xA1', 'a'}));
}

/**
 * @brief Test stopObject method.
 */
TEST (PackWriter, stopObject)
{
    std::stringstream stream;
    PackWriter packWriter (stream);

    EXPECT_EQ (packWriter.stopObject (), -1);
    EXPECT_EQ (join::lastError, SaxErrc::InvalidParent);

    EXPECT_EQ (packWriter.startObject (0), 0);
    EXPECT_EQ (packWriter.stopObject (), 0);
    EXPECT_EQ (stream.str (), std::string ({'\x80'}));
}

/**
 * @brief Test flush method.
 */
TEST (PackWriter, flush)
{
    std::stringstream stream;
    PackWriter packWriter (stream);

    EXPECT_EQ (packWriter.startArray (2), 0);
    EXPECT_EQ (packWriter.setNull (), 0);
    EXPECT_EQ (stream.str (), "");
    packWriter.flush ();
    EXPECT_EQ (stream.str (), std::string ({'\x92', '\xC0'}));
    packWriter.flush ();
    EXPECT_EQ (stream.str (), std::string ({'\x92', '\xC0'}));
    EXPECT_EQ (packWriter.setNull (), 0);
    EXPECT_EQ (packWriter.stopArray (), 0);
    EXPECT_EQ (stream.str (), std::string ({'\x92', '\xC0', '\xC0'}));
}

/**
 * @brief Test reset method.
 */
TEST (PackWriter, reset)
{
    std::stringstream stream;
    PackWriter packWriter (stream);

    EXPECT_EQ (packWriter.startArray (2), 0);
    EXPECT_EQ (packWriter.setNull (), 0);
    packWriter.reset ();
    EXPECT_EQ (stream.str (), "");
    EXPECT_EQ (packWriter.setNull (), 0);
    EXPECT_EQ (stream.str (), std::string ({'\xC0'}));
}

/**
 * @brief Test writing a whole document.
 */
TEST (PackWriter, document)
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
    PackWriter packWriter (stream);

    stream.str ("");
    EXPECT_EQ (value.serialize (packWriter), 0) << join::lastError.message ();
    EXPECT_EQ (stream.str (),
               std::string ({'\x98', '\xC0', '\xC3', '\x01', '\x02', '\x03', '\x04', '\xCB', '\x3D', '\x41', '\x5F',
                             '\xFF', '\xE5', '\x3A', '\x68', '\x5D', '\xA4', '\x74', '\x65', '\x73', '\x74'}));

    value = Object{{"a", 1}};
    stream.str ("");
    EXPECT_EQ (value.serialize (packWriter), 0) << join::lastError.message ();
    EXPECT_EQ (stream.str (), std::string ({'\x81', '\xA1', 'a', '\x01'}));

    value = Object{{Array{1}, 1}};
    stream.str ("");
    EXPECT_EQ (value.serialize (packWriter), -1);
    EXPECT_EQ (join::lastError, SaxErrc::InvalidKey);
    EXPECT_EQ (stream.str (), "");

    value = Array{7};
    EXPECT_EQ (value.serialize (packWriter), 0) << join::lastError.message ();
    EXPECT_EQ (stream.str (), std::string ({'\x91', '\x07'}));
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
