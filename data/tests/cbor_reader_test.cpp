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
#include <stdexcept>
#include <optional>
#include <sstream>
#include <string>
#include <vector>
#include <limits>
#include <span>

// C.
#include <cmath>

using join::Value;
using join::SaxErrc;
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
 * @brief handler recording the events it receives.
 */
struct Recorder : public join::SaxHandler
{
    int setNull ()
    {
        return record ("null");
    }

    int setBool (bool value)
    {
        return record (value ? "true" : "false");
    }

    int setInt (int32_t value)
    {
        return record ("i:" + std::to_string (value));
    }

    int setUint (uint32_t value)
    {
        return record ("u:" + std::to_string (value));
    }

    int setInt64 (int64_t value)
    {
        return record ("I:" + std::to_string (value));
    }

    int setUint64 (uint64_t value)
    {
        return record ("U:" + std::to_string (value));
    }

    int setDouble (double value)
    {
        doubles.push_back (value);
        return record ("d");
    }

    int setString (std::string_view value)
    {
        return record ("s:" + std::string (value));
    }

    int setBinary (std::span<const uint8_t> value, std::optional<int8_t> = std::nullopt)
    {
        return record ("b:" + std::to_string (value.size ()));
    }

    int startArray (uint32_t size = 0)
    {
        return record ("[" + std::to_string (size));
    }

    int stopArray ()
    {
        return record ("]");
    }

    int startObject (uint32_t size = 0)
    {
        return record ("{" + std::to_string (size));
    }

    int stopObject ()
    {
        return record ("}");
    }

    int setKey (std::nullptr_t)
    {
        return record ("knull");
    }

    int setKey (bool value)
    {
        return record (value ? "ktrue" : "kfalse");
    }

    int setKey (int32_t value)
    {
        return record ("ki:" + std::to_string (value));
    }

    int setKey (uint32_t value)
    {
        return record ("ku:" + std::to_string (value));
    }

    int setKey (int64_t value)
    {
        return record ("kI:" + std::to_string (value));
    }

    int setKey (uint64_t value)
    {
        return record ("kU:" + std::to_string (value));
    }

    int setKey (double value)
    {
        doubles.push_back (value);
        return record ("kd");
    }

    int setKey (std::string_view value)
    {
        return record ("ks:" + std::string (value));
    }

    int setKey (std::span<const uint8_t> value, std::optional<int8_t> = std::nullopt)
    {
        return record ("kb:" + std::to_string (value.size ()));
    }

    int startKey ()
    {
        return record ("k");
    }

    int record (const std::string& event)
    {
        log += log.empty () ? event : " " + event;
        return 0;
    }

    std::string log;
    std::vector<double> doubles;
};

/**
 * @brief handler rejecting array and object keys.
 */
struct RejectKey : public join::SaxHandler
{
    int startKey ()
    {
        return -1;
    }
};

/**
 * @brief handler throwing on strings.
 */
struct Throwing : public join::SaxHandler
{
    int setString (std::string_view)
    {
        throw std::range_error ("user");
    }
};

/**
 * @brief handler parsing the strings it receives as nested documents.
 */
struct Nested : public join::SaxHandler
{
    int setString (std::string_view value)
    {
        if (value.starts_with ('\x81'))
        {
            Nested inner;
            std::stringstream stream{std::string (value)};
            if (CborReader ().deserialize (stream, inner) == -1)
            {
                return -1;
            }
            strings.insert (strings.end (), inner.strings.begin (), inner.strings.end ());
        }
        strings.emplace_back (value);
        return 0;
    }

    std::vector<std::string> strings;
};

/**
 * @brief parse a document from a string and from a stream.
 * @param document document to parse.
 * @return events recorded, identical for both inputs, or "error".
 */
static std::string events (const std::string& document)
{
    Recorder fromString, fromStream;
    std::stringstream stream (document);
    if (CborReader ().deserialize (document, fromString) != 0 || CborReader ().deserialize (stream, fromStream) != 0 ||
        fromString.log != fromStream.log)
    {
        return "error";
    }
    return fromString.log;
}

/**
 * @brief parse a document expected to fail from a string and from a stream.
 * @param document document to parse.
 * @return error, identical for both inputs, or an empty error code.
 */
static std::error_code error (const std::string& document)
{
    join::SaxHandler handler;
    std::stringstream stream (document);
    if (CborReader ().deserialize (stream, handler) != -1)
    {
        return {};
    }
    std::error_code fromStream = join::lastError;
    if (CborReader ().deserialize (document, handler) != -1 || join::lastError != fromStream)
    {
        return {};
    }
    return fromStream;
}

/**
 * @brief Test deserialize method.
 */
TEST (CborReader, deserialize)
{
    std::stringstream stream;
    Value value;

    std::string str = hex ("8101");
    char data[] = {'\x81', '\x01', '\x00'};

    stream.str (str);
    ASSERT_EQ (CborReader ().deserialize (stream, value), 0) << join::lastError.message ();
    ASSERT_EQ (value.size (), 1);
    ASSERT_EQ (value[0], 1);

    ASSERT_EQ (CborReader ().deserialize (str, value), 0) << join::lastError.message ();
    ASSERT_EQ (value[0], 1);

    ASSERT_EQ (CborReader ().deserialize (data, sizeof (data) - 1, value), 0) << join::lastError.message ();
    ASSERT_EQ (value[0], 1);

    ASSERT_EQ (CborReader ().deserialize (&data[0], &data[sizeof (data) - 1], value), 0) << join::lastError.message ();
    ASSERT_EQ (value[0], 1);

    ASSERT_EQ (CborReader ().deserialize (&data[0], &data[sizeof (data)], value), -1);
    ASSERT_EQ (join::lastError, SaxErrc::ExtraData);
    ASSERT_TRUE (value.isNull ());

    ASSERT_EQ (error (""), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("18")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("19")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("1a")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("1b")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("38")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("39")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("3a")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("3b")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("58")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("59")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("5a")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("5b")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("78")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("79")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("7a")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("7b")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("98")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("99")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("9a")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("9b")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("b8")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("b9")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("ba")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("bb")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("d8")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("d9")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("da")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("db")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("f9")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("fa")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("fb")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("c0")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("81")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("82f6")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("9f")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("9ff6")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("a1")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("a1f6")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("bf")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("bff6")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("6461")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("4461")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("5f")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("7f")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("5f41")), SaxErrc::Truncated);
    ASSERT_EQ (error (hex ("7f61")), SaxErrc::Truncated);

    std::string nested = std::string (256, '\x81') + '\x01';
    ASSERT_EQ (CborReader ().deserialize (nested, value), 0) << join::lastError.message ();
    ASSERT_EQ (error ('\x81' + nested), SaxErrc::StackOverflow);
    ASSERT_EQ (error (std::string (257, '\x9f')), SaxErrc::StackOverflow);
    std::string objects;
    for (int i = 0; i < 257; ++i)
    {
        objects += hex ("a16161");
    }
    ASSERT_EQ (error (objects + '\x01'), SaxErrc::StackOverflow);

    std::stringstream out;
    CborStreamWriter writer (out);
    ASSERT_EQ (CborReader ().deserialize (nested, writer), 0) << join::lastError.message ();
    ASSERT_EQ (out.str (), std::string (256, '\x9f') + '\x01' + std::string (256, '\xff'));

    Throwing throwing;
    ASSERT_THROW (CborReader ().deserialize (hex ("816161"), throwing), std::range_error);

    join::ValueBuilder<Value> builder (value);
    for (size_t depth = 0; depth < join::saxMaxDepth; ++depth)
    {
        builder.startArray ();
    }
    ASSERT_EQ (CborReader ().deserialize (hex ("8101"), builder), -1);
    for (size_t depth = 0; depth < join::saxMaxDepth; ++depth)
    {
        builder.startArray ();
    }
    ASSERT_EQ (CborReader ().deserialize (hex ("9f01ff"), builder), -1);
    for (size_t depth = 0; depth < join::saxMaxDepth; ++depth)
    {
        builder.startArray ();
    }
    ASSERT_EQ (CborReader ().deserialize (hex ("a10101"), builder), -1);
    for (size_t depth = 0; depth < join::saxMaxDepth; ++depth)
    {
        builder.startArray ();
    }
    ASSERT_EQ (CborReader ().deserialize (hex ("bf0101ff"), builder), -1);

    using Stopping = join::BasicCborReader<join::CborReadMode::StopParsingOnDone>;
    std::stringstream messages (hex ("0102f5"));
    ASSERT_EQ (Stopping ().deserialize (messages, value), 0) << join::lastError.message ();
    ASSERT_EQ (value, 1);
    ASSERT_EQ (Stopping ().deserialize (messages, value), 0) << join::lastError.message ();
    ASSERT_EQ (value, 2);
    ASSERT_EQ (Stopping ().deserialize (messages, value), 0) << join::lastError.message ();
    ASSERT_EQ (value, true);
    ASSERT_EQ (Stopping ().deserialize (messages, value), -1);
    ASSERT_EQ (join::lastError, SaxErrc::Truncated);
    ASSERT_EQ (Stopping ().deserialize (hex ("0000"), value), 0) << join::lastError.message ();

    using Strict = join::BasicCborReader<join::CborReadMode::ValidateEncoding | join::CborReadMode::StopParsingOnDone>;
    std::stringstream strict (hex ("616161ff"));
    ASSERT_EQ (Strict ().deserialize (strict, value), 0) << join::lastError.message ();
    ASSERT_EQ (value.getString (), "a");
    ASSERT_EQ (Strict ().deserialize (strict, value), -1);
    ASSERT_EQ (join::lastError, SaxErrc::InvalidEncoding);
}

/**
 * @brief Test CBOR parsing of the RFC 8949 appendix A examples.
 */
TEST (CborReader, pass)
{
    ASSERT_EQ (events (hex ("00")), "u:0");
    ASSERT_EQ (events (hex ("0a")), "u:10");
    ASSERT_EQ (events (hex ("17")), "u:23");
    ASSERT_EQ (events (hex ("1818")), "u:24");
    ASSERT_EQ (events (hex ("1864")), "u:100");
    ASSERT_EQ (events (hex ("1903e8")), "u:1000");
    ASSERT_EQ (events (hex ("1a000f4240")), "u:1000000");
    ASSERT_EQ (events (hex ("1affffffff")), "u:4294967295");
    ASSERT_EQ (events (hex ("1b0000000100000000")), "U:4294967296");
    ASSERT_EQ (events (hex ("1b000000e8d4a51000")), "U:1000000000000");
    ASSERT_EQ (events (hex ("1bffffffffffffffff")), "U:18446744073709551615");
    ASSERT_EQ (events (hex ("20")), "i:-1");
    ASSERT_EQ (events (hex ("29")), "i:-10");
    ASSERT_EQ (events (hex ("3863")), "i:-100");
    ASSERT_EQ (events (hex ("3903e7")), "i:-1000");
    ASSERT_EQ (events (hex ("3a7fffffff")), "i:-2147483648");
    ASSERT_EQ (events (hex ("3a80000000")), "I:-2147483649");
    ASSERT_EQ (events (hex ("3b7fffffffffffffff")), "I:-9223372036854775808");
    ASSERT_EQ (events (hex ("f4")), "false");
    ASSERT_EQ (events (hex ("f5")), "true");
    ASSERT_EQ (events (hex ("f6")), "null");
    ASSERT_EQ (events (hex ("f7")), "null");
    ASSERT_EQ (events (hex ("80")), "[0 ]");
    ASSERT_EQ (events (hex ("83010203")), "[3 u:1 u:2 u:3 ]");
    ASSERT_EQ (events (hex ("8301820203820405")), "[3 u:1 [2 u:2 u:3 ] [2 u:4 u:5 ] ]");
    ASSERT_EQ (events (hex ("a0")), "{0 }");
    ASSERT_EQ (events (hex ("a201020304")), "{2 ku:1 u:2 ku:3 u:4 }");
    ASSERT_EQ (events (hex ("a26161016162820203")), "{2 ks:a u:1 ks:b [2 u:2 u:3 ] }");
    ASSERT_EQ (events (hex ("826161a161626163")), "[2 s:a {1 ks:b s:c } ]");
    ASSERT_EQ (events (hex ("9fff")), "[0 ]");
    ASSERT_EQ (events (hex ("9f018202039f0405ffff")), "[0 u:1 [2 u:2 u:3 ] [0 u:4 u:5 ] ]");
    ASSERT_EQ (events (hex ("9f01820203820405ff")), "[0 u:1 [2 u:2 u:3 ] [2 u:4 u:5 ] ]");
    ASSERT_EQ (events (hex ("83018202039f0405ff")), "[3 u:1 [2 u:2 u:3 ] [0 u:4 u:5 ] ]");
    ASSERT_EQ (events (hex ("83019f0203ff820405")), "[3 u:1 [0 u:2 u:3 ] [2 u:4 u:5 ] ]");
    ASSERT_EQ (events (hex ("bf61610161629f0203ffff")), "{0 ks:a u:1 ks:b [0 u:2 u:3 ] }");
    ASSERT_EQ (events (hex ("826161bf61626163ff")), "[2 s:a {0 ks:b s:c } ]");
    ASSERT_EQ (events (hex ("bf6346756ef563416d7421ff")), "{0 ks:Fun true ks:Amt i:-2 }");
}

/**
 * @brief Test CBOR parsing fail.
 */
TEST (CborReader, fail)
{
    ASSERT_EQ (error (hex ("1c")), SaxErrc::InvalidValue);
    ASSERT_EQ (error (hex ("1d")), SaxErrc::InvalidValue);
    ASSERT_EQ (error (hex ("1e")), SaxErrc::InvalidValue);
    ASSERT_EQ (error (hex ("1f")), SaxErrc::InvalidValue);
    ASSERT_EQ (error (hex ("3c")), SaxErrc::InvalidValue);
    ASSERT_EQ (error (hex ("3f")), SaxErrc::InvalidValue);
    ASSERT_EQ (error (hex ("5c")), SaxErrc::InvalidValue);
    ASSERT_EQ (error (hex ("7c")), SaxErrc::InvalidValue);
    ASSERT_EQ (error (hex ("9c")), SaxErrc::InvalidValue);
    ASSERT_EQ (error (hex ("bc")), SaxErrc::InvalidValue);
    ASSERT_EQ (error (hex ("dc")), SaxErrc::InvalidValue);
    ASSERT_EQ (error (hex ("df")), SaxErrc::InvalidValue);
    ASSERT_EQ (error (hex ("fc")), SaxErrc::InvalidValue);
    ASSERT_EQ (error (hex ("fe")), SaxErrc::InvalidValue);
    ASSERT_EQ (error (hex ("ff")), SaxErrc::InvalidValue);
    ASSERT_EQ (error (hex ("e0")), SaxErrc::InvalidValue);
    ASSERT_EQ (error (hex ("f3")), SaxErrc::InvalidValue);
    ASSERT_EQ (error (hex ("f800")), SaxErrc::InvalidValue);
    ASSERT_EQ (error (hex ("f820")), SaxErrc::InvalidValue);
    ASSERT_EQ (error (hex ("f8ff")), SaxErrc::InvalidValue);
    ASSERT_EQ (error (hex ("c0ff")), SaxErrc::InvalidValue);
    ASSERT_EQ (error (hex ("9fc0ff")), SaxErrc::InvalidValue);
    ASSERT_EQ (error (hex ("3b8000000000000000")), SaxErrc::InvalidValue);
    ASSERT_EQ (error (hex ("3bffffffffffffffff")), SaxErrc::InvalidValue);
    ASSERT_EQ (error (hex ("5b0000000100000000")), SaxErrc::InvalidValue);
    ASSERT_EQ (error (hex ("7b0000000100000000")), SaxErrc::InvalidValue);
    ASSERT_EQ (error (hex ("9b0000000100000000")), SaxErrc::InvalidValue);
    ASSERT_EQ (error (hex ("bb0000000100000000")), SaxErrc::InvalidValue);
    ASSERT_EQ (error (hex ("5f6161ff")), SaxErrc::InvalidValue);
    ASSERT_EQ (error (hex ("7f4161ff")), SaxErrc::InvalidValue);
    ASSERT_EQ (error (hex ("5f5fffff")), SaxErrc::InvalidValue);
    ASSERT_EQ (error (hex ("7f7fffff")), SaxErrc::InvalidValue);
    ASSERT_EQ (error (hex ("5f5cff")), SaxErrc::InvalidValue);
    ASSERT_EQ (error (hex ("7f7bffffffffffffffff")), SaxErrc::InvalidValue);
    ASSERT_EQ (error (hex ("5f5b0000000100000000")), SaxErrc::InvalidValue);

    ASSERT_EQ (error (hex ("bf6161ff")), SaxErrc::InvalidKey);
    ASSERT_EQ (error (hex ("0000")), SaxErrc::ExtraData);
}

/**
 * @brief Test CBOR parsing of floating point numbers.
 */
TEST (CborReader, dbl)
{
    auto number = [] (const char* doc) {
        Recorder recorder;
        EXPECT_EQ (CborReader ().deserialize (hex (doc), recorder), 0) << doc;
        return recorder.doubles.size () == 1 ? recorder.doubles[0] : -1.0;
    };

    ASSERT_EQ (number ("f90000"), 0.0);
    ASSERT_TRUE (std::signbit (number ("f98000")));
    ASSERT_EQ (number ("f93c00"), 1.0);
    ASSERT_EQ (number ("f93e00"), 1.5);
    ASSERT_EQ (number ("f97bff"), 65504.0);
    ASSERT_EQ (number ("f90001"), 5.960464477539063e-8);
    ASSERT_EQ (number ("f90400"), 6.103515625e-5);
    ASSERT_EQ (number ("f9c400"), -4.0);
    ASSERT_EQ (number ("f97c00"), std::numeric_limits<double>::infinity ());
    ASSERT_EQ (number ("f9fc00"), -std::numeric_limits<double>::infinity ());
    ASSERT_TRUE (std::isnan (number ("f97e00")));
    ASSERT_EQ (number ("fa47c35000"), 100000.0);
    ASSERT_EQ (number ("fa7f7fffff"), 3.4028234663852886e+38);
    ASSERT_EQ (number ("fa7f800000"), std::numeric_limits<double>::infinity ());
    ASSERT_EQ (number ("fb3ff199999999999a"), 1.1);
    ASSERT_EQ (number ("fb7e37e43c8800759c"), 1.0e+300);
    ASSERT_TRUE (std::isnan (number ("fb7ff8000000000000")));
    ASSERT_EQ (events (hex ("83f93c00fa3fc00000fb3ff199999999999a")), "[3 d d d ]");
}

/**
 * @brief Test CBOR parsing of text strings.
 */
TEST (CborReader, str)
{
    ASSERT_EQ (events (hex ("60")), "s:");
    ASSERT_EQ (events (hex ("6161")), "s:a");
    ASSERT_EQ (events (hex ("6449455446")), "s:IETF");
    ASSERT_EQ (events (hex ("62225c")), "s:\"\\");
    ASSERT_EQ (events (hex ("63e6b0b4")), "s:\xe6\xb0\xb4");
    ASSERT_EQ (events (hex ("7fff")), "s:");
    ASSERT_EQ (events (hex ("7f657374726561646d696e67ff")), "s:streaming");
    ASSERT_EQ (events (hex ("7f6060ff")), "s:");

    std::string big (100000, 'x');
    ASSERT_EQ (events (hex ("7a000186a0") + big), "s:" + big);
    ASSERT_EQ (events (hex ("7f7a000186a0") + big + hex ("6178ff")), "s:" + big + "x");

    Nested nested;
    std::stringstream stream (hex ("7f67817f61616162ffff"));
    ASSERT_EQ (CborReader ().deserialize (stream, nested), 0) << join::lastError.message ();
    ASSERT_EQ (nested.strings, std::vector<std::string> ({"ab", hex ("817f61616162ff")}));
}

/**
 * @brief Test CBOR parsing of byte strings.
 */
TEST (CborReader, bin)
{
    Value value;

    ASSERT_EQ (events (hex ("40")), "b:0");
    ASSERT_EQ (events (hex ("4401020304")), "b:4");
    ASSERT_EQ (events (hex ("5fff")), "b:0");
    ASSERT_EQ (events (hex ("5f42010243030405ff")), "b:5");

    ASSERT_EQ (CborReader ().deserialize (hex ("5f42010243030405ff"), value), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isBinary ());
    ASSERT_EQ (std::vector<uint8_t> (value.getBinary ().begin (), value.getBinary ().end ()),
               std::vector<uint8_t> ({1, 2, 3, 4, 5}));
    ASSERT_FALSE (value.getSubtype ().has_value ());

    std::string big (100000, 'x');
    ASSERT_EQ (events (hex ("5a000186a0") + big), "b:100000");
}

/**
 * @brief Test CBOR parsing of tags.
 */
TEST (CborReader, tag)
{
    ASSERT_EQ (events (hex ("c074323031332d30332d32315432303a30343a30305a")), "s:2013-03-21T20:04:00Z");
    ASSERT_EQ (events (hex ("c11a514b67b0")), "u:1363896240");
    ASSERT_EQ (events (hex ("c1fb41d452d9ec200000")), "d");
    ASSERT_EQ (events (hex ("d74401020304")), "b:4");
    ASSERT_EQ (events (hex ("d818456449455446")), "b:5");
    ASSERT_EQ (events (hex ("d82076687474703a2f2f7777772e6578616d706c652e636f6d")), "s:http://www.example.com");
    ASSERT_EQ (events (hex ("d9d9f7c101")), "u:1");
    ASSERT_EQ (events (std::string (100000, '\xc0') + '\x01'), "u:1");
    ASSERT_EQ (events (hex ("a1c10102")), "{1 ku:1 u:2 }");
}

/**
 * @brief Test CBOR parsing of map keys.
 */
TEST (CborReader, key)
{
    ASSERT_EQ (events (hex ("a1f601")), "{1 knull u:1 }");
    ASSERT_EQ (events (hex ("a1f401")), "{1 kfalse u:1 }");
    ASSERT_EQ (events (hex ("a1f501")), "{1 ktrue u:1 }");
    ASSERT_EQ (events (hex ("a1f701")), "{1 knull u:1 }");
    ASSERT_EQ (events (hex ("a10101")), "{1 ku:1 u:1 }");
    ASSERT_EQ (events (hex ("a11b000000010000000001")), "{1 kU:4294967296 u:1 }");
    ASSERT_EQ (events (hex ("a12001")), "{1 ki:-1 u:1 }");
    ASSERT_EQ (events (hex ("a13b000000010000000001")), "{1 kI:-4294967297 u:1 }");
    ASSERT_EQ (events (hex ("a3f93c0001fa3fc0000002fb3ff199999999999a03")), "{3 kd u:1 kd u:2 kd u:3 }");
    ASSERT_EQ (events (hex ("a1616101")), "{1 ks:a u:1 }");
    ASSERT_EQ (events (hex ("a142000101")), "{1 kb:2 u:1 }");
    ASSERT_EQ (events (hex ("a17f6161ff01")), "{1 ks:a u:1 }");
    ASSERT_EQ (events (hex ("a15f4100ff01")), "{1 kb:1 u:1 }");
    ASSERT_EQ (events (hex ("a1810102")), "{1 k [1 u:1 ] u:2 }");
    ASSERT_EQ (events (hex ("a1a1010203")), "{1 k {1 ku:1 u:2 } u:3 }");
    ASSERT_EQ (events (hex ("a19fff01")), "{1 k [0 ] u:1 }");
    ASSERT_EQ (events (hex ("a1bfff01")), "{1 k {0 } u:1 }");
    ASSERT_EQ (events (hex ("bf9f01ff02ff")), "{0 k [0 u:1 ] u:2 }");

    Value value;
    ASSERT_EQ (CborReader ().deserialize (hex ("a3f501616101810102"), value), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_TRUE (value.contains (true));
    ASSERT_TRUE (value.contains ("a"));

    RejectKey rejectKey;
    ASSERT_EQ (CborReader ().deserialize (hex ("a1810102"), rejectKey), -1);
    ASSERT_EQ (CborReader ().deserialize (hex ("a1a1010203"), rejectKey), -1);
    ASSERT_EQ (CborReader ().deserialize (hex ("a19fff01"), rejectKey), -1);
    ASSERT_EQ (CborReader ().deserialize (hex ("a1bfff01"), rejectKey), -1);
}

/**
 * @brief Test CBOR parsing with UTF-8 validation.
 */
TEST (CborReader, encoding)
{
    using Validator = join::BasicCborReader<join::CborReadMode::ValidateEncoding>;

    join::SaxHandler handler;
    std::stringstream stream;

    ASSERT_EQ (Validator ().deserialize (hex ("63e6b0b4"), handler), 0) << join::lastError.message ();
    stream.clear ();
    stream.str (hex ("63e6b0b4"));
    ASSERT_EQ (Validator ().deserialize (stream, handler), 0) << join::lastError.message ();

    ASSERT_EQ (Validator ().deserialize (hex ("7f62c3a9ff"), handler), 0) << join::lastError.message ();
    stream.clear ();
    stream.str (hex ("7f62c3a9ff"));
    ASSERT_EQ (Validator ().deserialize (stream, handler), 0) << join::lastError.message ();

    ASSERT_EQ (Validator ().deserialize (hex ("41ff"), handler), 0) << join::lastError.message ();
    stream.clear ();
    stream.str (hex ("41ff"));
    ASSERT_EQ (Validator ().deserialize (stream, handler), 0) << join::lastError.message ();

    ASSERT_EQ (Validator ().deserialize (hex ("5f41ff41feff"), handler), 0) << join::lastError.message ();
    stream.clear ();
    stream.str (hex ("5f41ff41feff"));
    ASSERT_EQ (Validator ().deserialize (stream, handler), 0) << join::lastError.message ();

    ASSERT_EQ (Validator ().deserialize (hex ("61ff"), handler), -1);
    ASSERT_EQ (join::lastError, SaxErrc::InvalidEncoding);
    stream.clear ();
    stream.str (hex ("61ff"));
    ASSERT_EQ (Validator ().deserialize (stream, handler), -1);
    ASSERT_EQ (join::lastError, SaxErrc::InvalidEncoding);

    ASSERT_EQ (Validator ().deserialize (hex ("7f61c361a9ff"), handler), -1);
    ASSERT_EQ (join::lastError, SaxErrc::InvalidEncoding);
    stream.clear ();
    stream.str (hex ("7f61c361a9ff"));
    ASSERT_EQ (Validator ().deserialize (stream, handler), -1);
    ASSERT_EQ (join::lastError, SaxErrc::InvalidEncoding);

    ASSERT_EQ (Validator ().deserialize (hex ("a161ff01"), handler), -1);
    ASSERT_EQ (join::lastError, SaxErrc::InvalidEncoding);
    stream.clear ();
    stream.str (hex ("a161ff01"));
    ASSERT_EQ (Validator ().deserialize (stream, handler), -1);
    ASSERT_EQ (join::lastError, SaxErrc::InvalidEncoding);

    ASSERT_EQ (events (hex ("61ff")), "s:\xff");

    Value value;
    ASSERT_EQ (value.deserialize<Validator> (hex ("6449455446")), 0) << join::lastError.message ();
    ASSERT_EQ (value.getString (), "IETF");
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
