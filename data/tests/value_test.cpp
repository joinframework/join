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
#include <join/error.hpp>
#include <join/json.hpp>
#include <join/pack.hpp>

// Libraries.
#include <gtest/gtest.h>

// C++.
#include <string_view>
#include <algorithm>
#include <stdexcept>
#include <optional>
#include <sstream>
#include <fstream>
#include <limits>
#include <span>

using join::Array;
using join::Member;
using join::Object;
using join::Value;
using join::JsonReader;
using join::JsonReadMode;
using join::JsonWriter;
using join::JsonCanonicalizer;
using join::PackReader;
using join::PackWriter;
using join::SaxErrc;

/**
 * @brief stream buffer failing on its first write.
 */
struct FailingBuffer : public std::streambuf
{
    std::streamsize xsputn (const char* data, std::streamsize size) override
    {
        if (!failed)
        {
            failed = true;
            throw std::runtime_error ("write failed");
        }
        written.append (data, size);
        return size;
    }

    bool failed = false;
    std::string written;
};

/**
 * @brief handler counting the values it receives.
 */
struct Counter : public join::SaxHandler
{
    int setNull ()
    {
        return count ();
    }

    int setBool (bool)
    {
        return count ();
    }

    int setInt (int32_t)
    {
        return count ();
    }

    int setUint (uint32_t)
    {
        return count ();
    }

    int setInt64 (int64_t)
    {
        return count ();
    }

    int setUint64 (uint64_t)
    {
        return count ();
    }

    int setDouble (double)
    {
        return count ();
    }

    int setString (std::string_view)
    {
        return count ();
    }

    int count ()
    {
        ++values;
        return 0;
    }

    size_t values = 0;
};

/**
 * @brief Test create method.
 */
TEST (Value, create)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    static_assert (std::is_nothrow_default_constructible_v<Value>);
    Value defaultValue;
    ASSERT_TRUE (defaultValue.isNull ());

    Value nullValue (nullptr);
    ASSERT_TRUE (nullValue.isNull ());

    Value trueValue (true);
    ASSERT_TRUE (trueValue.isBool ());
    ASSERT_TRUE (trueValue.isTrue ());

    Value falseValue (false);
    ASSERT_TRUE (falseValue.isBool ());
    ASSERT_TRUE (falseValue.isFalse ());

    Value minIntValue (std::numeric_limits<int32_t>::min ());
    ASSERT_TRUE (minIntValue.isNumber ());
    ASSERT_TRUE (minIntValue.isInt ());

    Value maxIntValue (std::numeric_limits<int32_t>::max ());
    ASSERT_TRUE (minIntValue.isNumber ());
    ASSERT_TRUE (minIntValue.isInt ());

    Value minUintValue (std::numeric_limits<uint32_t>::min ());
    ASSERT_TRUE (minUintValue.isNumber ());
    ASSERT_TRUE (minUintValue.isUint ());

    Value maxUintValue (std::numeric_limits<uint32_t>::max ());
    ASSERT_TRUE (maxUintValue.isNumber ());
    ASSERT_TRUE (maxUintValue.isUint ());

    Value minInt64Value (std::numeric_limits<int64_t>::min ());
    ASSERT_TRUE (minIntValue.isNumber ());
    ASSERT_TRUE (minIntValue.isInt64 ());

    Value maxInt64Value (std::numeric_limits<int64_t>::max ());
    ASSERT_TRUE (minInt64Value.isNumber ());
    ASSERT_TRUE (minInt64Value.isInt64 ());

    Value minUint64Value (std::numeric_limits<uint64_t>::min ());
    ASSERT_TRUE (minUint64Value.isNumber ());
    ASSERT_TRUE (minUint64Value.isUint64 ());

    Value maxUint64Value (std::numeric_limits<uint64_t>::max ());
    ASSERT_TRUE (maxUint64Value.isNumber ());
    ASSERT_TRUE (maxUint64Value.isUint64 ());

    Value minFloatValue (std::numeric_limits<float>::min ());
    ASSERT_TRUE (minFloatValue.isNumber ());
    ASSERT_TRUE (minFloatValue.isFloat ());

    Value maxFloatValue (std::numeric_limits<float>::max ());
    ASSERT_TRUE (maxFloatValue.isNumber ());
    ASSERT_TRUE (maxFloatValue.isFloat ());

    Value minDoubleValue (std::numeric_limits<double>::min ());
    ASSERT_TRUE (minDoubleValue.isNumber ());
    ASSERT_TRUE (minDoubleValue.isDouble ());

    Value maxDoubleValue (std::numeric_limits<double>::max ());
    ASSERT_TRUE (maxDoubleValue.isNumber ());
    ASSERT_TRUE (maxDoubleValue.isDouble ());

    Value stringValue ("foo");
    ASSERT_TRUE (stringValue.isString ());

    char buffer[] = "bar";
    Value bufferValue (buffer);
    ASSERT_TRUE (bufferValue.isString ());
    ASSERT_EQ (bufferValue.getString (), "bar");

    char* pointer = buffer;
    Value pointerValue (pointer);
    ASSERT_TRUE (pointerValue.isString ());
    ASSERT_EQ (pointerValue.getString (), "bar");

    const std::string string ("baz");
    Value copiedStringValue (string);
    ASSERT_TRUE (copiedStringValue.isString ());
    ASSERT_EQ (copiedStringValue.getString (), "baz");

    Value movedStringValue (std::string ("qux"));
    ASSERT_TRUE (movedStringValue.isString ());
    ASSERT_EQ (movedStringValue.getString (), "qux");

    Value viewStringValue (std::string_view ("quux", 3));
    ASSERT_TRUE (viewStringValue.isString ());
    ASSERT_EQ (viewStringValue.getString (), "quu");

    const uint8_t bytes[] = {0x00, 0x01, 0xff};
    Value binaryValue (std::span<const uint8_t> (bytes, sizeof (bytes)));
    ASSERT_TRUE (binaryValue.isBinary ());
    ASSERT_TRUE (std::ranges::equal (binaryValue.getBinary (), std::span<const uint8_t> (bytes, sizeof (bytes))));
    ASSERT_EQ (binaryValue.getSubtype (), std::nullopt);

    Value extensionValue (std::span<const uint8_t> (bytes, sizeof (bytes)), -1);
    ASSERT_TRUE (extensionValue.isBinary ());
    ASSERT_TRUE (std::ranges::equal (extensionValue.getBinary (), std::span<const uint8_t> (bytes, sizeof (bytes))));
    ASSERT_EQ (extensionValue.getSubtype (), -1);

    Value arrayValue (array.getArray ());
    ASSERT_TRUE (arrayValue.isArray ());
    ASSERT_EQ (arrayValue.size (), 1U);
    ASSERT_EQ (arrayValue[0], 1);

    Value objectValue (object.getObject ());
    ASSERT_TRUE (objectValue.isObject ());
    ASSERT_EQ (objectValue["i"], 1);
}

/**
 * @brief Test copy method.
 */
TEST (Value, copy)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    Value value;
    Value defaultValue (value);
    ASSERT_TRUE (defaultValue.isNull ());

    value = nullptr;
    Value nullValue (value);
    ASSERT_TRUE (nullValue.isNull ());

    value = true;
    Value trueValue (value);
    ASSERT_TRUE (trueValue.isBool ());
    ASSERT_TRUE (trueValue.isTrue ());

    value = false;
    Value falseValue (value);
    ASSERT_TRUE (falseValue.isBool ());
    ASSERT_TRUE (falseValue.isFalse ());

    value = std::numeric_limits<int32_t>::min ();
    Value minIntValue (value);
    ASSERT_TRUE (minIntValue.isNumber ());
    ASSERT_TRUE (minIntValue.isInt ());

    value = std::numeric_limits<int32_t>::max ();
    Value maxIntValue (value);
    ASSERT_TRUE (minIntValue.isNumber ());
    ASSERT_TRUE (minIntValue.isInt ());

    value = std::numeric_limits<uint32_t>::min ();
    Value minUintValue (value);
    ASSERT_TRUE (minUintValue.isNumber ());
    ASSERT_TRUE (minUintValue.isUint ());

    value = std::numeric_limits<uint32_t>::max ();
    Value maxUintValue (value);
    ASSERT_TRUE (maxUintValue.isNumber ());
    ASSERT_TRUE (maxUintValue.isUint ());

    value = std::numeric_limits<int64_t>::min ();
    Value minInt64Value (value);
    ASSERT_TRUE (minIntValue.isNumber ());
    ASSERT_TRUE (minIntValue.isInt64 ());

    value = std::numeric_limits<int64_t>::max ();
    Value maxInt64Value (value);
    ASSERT_TRUE (minInt64Value.isNumber ());
    ASSERT_TRUE (minInt64Value.isInt64 ());

    value = std::numeric_limits<uint64_t>::min ();
    Value minUint64Value (value);
    ASSERT_TRUE (minUint64Value.isNumber ());
    ASSERT_TRUE (minUint64Value.isUint64 ());

    value = std::numeric_limits<uint64_t>::max ();
    Value maxUint64Value (value);
    ASSERT_TRUE (maxUint64Value.isNumber ());
    ASSERT_TRUE (maxUint64Value.isUint64 ());

    value = std::numeric_limits<float>::min ();
    Value minFloatValue (value);
    ASSERT_TRUE (minFloatValue.isNumber ());
    ASSERT_TRUE (minFloatValue.isFloat ());

    value = std::numeric_limits<float>::max ();
    Value maxFloatValue (value);
    ASSERT_TRUE (maxFloatValue.isNumber ());
    ASSERT_TRUE (maxFloatValue.isFloat ());

    value = std::numeric_limits<double>::min ();
    Value minDoubleValue (value);
    ASSERT_TRUE (minDoubleValue.isNumber ());
    ASSERT_TRUE (minDoubleValue.isDouble ());

    value = std::numeric_limits<double>::max ();
    Value maxDoubleValue (value);
    ASSERT_TRUE (maxDoubleValue.isNumber ());
    ASSERT_TRUE (maxDoubleValue.isDouble ());

    value = "foo";
    Value stringValue (value);
    ASSERT_TRUE (stringValue.isString ());

    const uint8_t bytes[] = {0x00, 0x01, 0xff};
    value = std::span<const uint8_t> (bytes, sizeof (bytes));
    Value binaryValue (value);
    ASSERT_TRUE (binaryValue.isBinary ());
    ASSERT_TRUE (std::ranges::equal (binaryValue.getBinary (), std::span<const uint8_t> (bytes, sizeof (bytes))));

    value = Value (std::span<const uint8_t> (bytes, sizeof (bytes)), -1);
    Value extensionValue (value);
    ASSERT_TRUE (extensionValue.isBinary ());
    ASSERT_TRUE (std::ranges::equal (extensionValue.getBinary (), std::span<const uint8_t> (bytes, sizeof (bytes))));
    ASSERT_EQ (extensionValue.getSubtype (), -1);

    value = array.getArray ();
    Value arrayValue (value);
    ASSERT_TRUE (arrayValue.isArray ());

    value = object.getObject ();
    Value objectValue (value);
    ASSERT_TRUE (objectValue.isObject ());
}

/**
 * @brief Test move method.
 */
TEST (Value, move)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    Value value;
    Value defaultValue (std::move (value));
    ASSERT_TRUE (defaultValue.isNull ());

    value = nullptr;
    Value nullValue (std::move (value));
    ASSERT_TRUE (nullValue.isNull ());

    value = true;
    Value trueValue (std::move (value));
    ASSERT_TRUE (trueValue.isBool ());
    ASSERT_TRUE (trueValue.isTrue ());

    value = false;
    Value falseValue (std::move (value));
    ASSERT_TRUE (falseValue.isBool ());
    ASSERT_TRUE (falseValue.isFalse ());

    value = std::numeric_limits<int32_t>::min ();
    Value minIntValue (std::move (value));
    ASSERT_TRUE (minIntValue.isNumber ());
    ASSERT_TRUE (minIntValue.isInt ());

    value = std::numeric_limits<int32_t>::max ();
    Value maxIntValue (std::move (value));
    ASSERT_TRUE (minIntValue.isNumber ());
    ASSERT_TRUE (minIntValue.isInt ());

    value = std::numeric_limits<uint32_t>::min ();
    Value minUintValue (std::move (value));
    ASSERT_TRUE (minUintValue.isNumber ());
    ASSERT_TRUE (minUintValue.isUint ());

    value = std::numeric_limits<uint32_t>::max ();
    Value maxUintValue (std::move (value));
    ASSERT_TRUE (maxUintValue.isNumber ());
    ASSERT_TRUE (maxUintValue.isUint ());

    value = std::numeric_limits<int64_t>::min ();
    Value minInt64Value (std::move (value));
    ASSERT_TRUE (minIntValue.isNumber ());
    ASSERT_TRUE (minIntValue.isInt64 ());

    value = std::numeric_limits<int64_t>::max ();
    Value maxInt64Value (std::move (value));
    ASSERT_TRUE (minInt64Value.isNumber ());
    ASSERT_TRUE (minInt64Value.isInt64 ());

    value = std::numeric_limits<uint64_t>::min ();
    Value minUint64Value (std::move (value));
    ASSERT_TRUE (minUint64Value.isNumber ());
    ASSERT_TRUE (minUint64Value.isUint64 ());

    value = std::numeric_limits<uint64_t>::max ();
    Value maxUint64Value (std::move (value));
    ASSERT_TRUE (maxUint64Value.isNumber ());
    ASSERT_TRUE (maxUint64Value.isUint64 ());

    value = std::numeric_limits<float>::min ();
    Value minFloatValue (std::move (value));
    ASSERT_TRUE (minFloatValue.isNumber ());
    ASSERT_TRUE (minFloatValue.isFloat ());

    value = std::numeric_limits<float>::max ();
    Value maxFloatValue (std::move (value));
    ASSERT_TRUE (maxFloatValue.isNumber ());
    ASSERT_TRUE (maxFloatValue.isFloat ());

    value = std::numeric_limits<double>::min ();
    Value minDoubleValue (std::move (value));
    ASSERT_TRUE (minDoubleValue.isNumber ());
    ASSERT_TRUE (minDoubleValue.isDouble ());

    value = std::numeric_limits<double>::max ();
    Value maxDoubleValue (std::move (value));
    ASSERT_TRUE (maxDoubleValue.isNumber ());
    ASSERT_TRUE (maxDoubleValue.isDouble ());

    value = "foo";
    Value stringValue (std::move (value));
    ASSERT_TRUE (stringValue.isString ());

    value = array.getArray ();
    Value arrayValue (std::move (value));
    ASSERT_TRUE (arrayValue.isArray ());

    value = object.getObject ();
    Value objectValue (std::move (value));
    ASSERT_TRUE (objectValue.isObject ());
}

/**
 * @brief Test isNull method.
 */
TEST (Value, isNull)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    Value value;
    ASSERT_TRUE (value.isNull ());

    value = nullptr;
    ASSERT_TRUE (value.isNull ());

    value = true;
    ASSERT_FALSE (value.isNull ());

    value = false;
    ASSERT_FALSE (value.isNull ());

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_FALSE (value.isNull ());

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_FALSE (value.isNull ());

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_FALSE (value.isNull ());

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_FALSE (value.isNull ());

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_FALSE (value.isNull ());

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_FALSE (value.isNull ());

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_FALSE (value.isNull ());

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_FALSE (value.isNull ());

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_FALSE (value.isNull ());

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_FALSE (value.isNull ());

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_FALSE (value.isNull ());

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_FALSE (value.isNull ());

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_FALSE (value.isNull ());

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_FALSE (value.isNull ());

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_FALSE (value.isNull ());

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_FALSE (value.isNull ());

    value = std::numeric_limits<float>::min ();
    ASSERT_FALSE (value.isNull ());

    value = std::numeric_limits<float>::max ();
    ASSERT_FALSE (value.isNull ());

    value = std::numeric_limits<double>::min ();
    ASSERT_FALSE (value.isNull ());

    value = std::numeric_limits<double>::max ();
    ASSERT_FALSE (value.isNull ());

    value = "";
    ASSERT_FALSE (value.isNull ());

    value = "foo";
    ASSERT_FALSE (value.isNull ());

    value = "02:42:64:2f:6a:d0";
    ASSERT_FALSE (value.isNull ());

    value = "127.0.0.1";
    ASSERT_FALSE (value.isNull ());

    value = array.getArray ();
    ASSERT_FALSE (value.isNull ());

    value = object.getObject ();
    ASSERT_FALSE (value.isNull ());
}

/**
 * @brief Test isBool method.
 */
TEST (Value, isBool)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    Value value;
    ASSERT_FALSE (value.isBool ());

    value = nullptr;
    ASSERT_FALSE (value.isBool ());

    value = true;
    ASSERT_TRUE (value.isBool ());

    value = false;
    ASSERT_TRUE (value.isBool ());

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_FALSE (value.isBool ());

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_FALSE (value.isBool ());

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_FALSE (value.isBool ());

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_FALSE (value.isBool ());

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_FALSE (value.isBool ());

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_FALSE (value.isBool ());

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_FALSE (value.isBool ());

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_FALSE (value.isBool ());

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_FALSE (value.isBool ());

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_FALSE (value.isBool ());

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_FALSE (value.isBool ());

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_FALSE (value.isBool ());

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_FALSE (value.isBool ());

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_FALSE (value.isBool ());

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_FALSE (value.isBool ());

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_FALSE (value.isBool ());

    value = std::numeric_limits<float>::min ();
    ASSERT_FALSE (value.isBool ());

    value = std::numeric_limits<float>::max ();
    ASSERT_FALSE (value.isBool ());

    value = std::numeric_limits<double>::min ();
    ASSERT_FALSE (value.isBool ());

    value = std::numeric_limits<double>::max ();
    ASSERT_FALSE (value.isBool ());

    value = "";
    ASSERT_FALSE (value.isBool ());

    value = "foo";
    ASSERT_FALSE (value.isBool ());

    value = "02:42:64:2f:6a:d0";
    ASSERT_FALSE (value.isBool ());

    value = "127.0.0.1";
    ASSERT_FALSE (value.isBool ());

    value = array.getArray ();
    ASSERT_FALSE (value.isBool ());

    value = object.getObject ();
    ASSERT_FALSE (value.isBool ());
}

/**
 * @brief Test isTrue method.
 */
TEST (Value, isTrue)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    Value value;
    ASSERT_FALSE (value.isTrue ());

    value = nullptr;
    ASSERT_FALSE (value.isTrue ());

    value = true;
    ASSERT_TRUE (value.isTrue ());

    value = false;
    ASSERT_FALSE (value.isTrue ());

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_TRUE (value.isTrue ());

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_TRUE (value.isTrue ());

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_FALSE (value.isTrue ());

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_TRUE (value.isTrue ());

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_TRUE (value.isTrue ());

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_TRUE (value.isTrue ());

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_FALSE (value.isTrue ());

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_TRUE (value.isTrue ());

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_TRUE (value.isTrue ());

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_TRUE (value.isTrue ());

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_FALSE (value.isTrue ());

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_TRUE (value.isTrue ());

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_TRUE (value.isTrue ());

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_TRUE (value.isTrue ());

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_FALSE (value.isTrue ());

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_TRUE (value.isTrue ());

    value = std::numeric_limits<float>::min ();
    ASSERT_TRUE (value.isTrue ());

    value = std::numeric_limits<float>::max ();
    ASSERT_TRUE (value.isTrue ());

    value = std::numeric_limits<double>::min ();
    ASSERT_TRUE (value.isTrue ());

    value = std::numeric_limits<double>::max ();
    ASSERT_TRUE (value.isTrue ());

    value = "";
    ASSERT_THROW (value.isTrue (), std::bad_cast);

    value = "foo";
    ASSERT_THROW (value.isTrue (), std::bad_cast);

    value = "02:42:64:2f:6a:d0";
    ASSERT_THROW (value.isTrue (), std::bad_cast);

    value = "127.0.0.1";
    ASSERT_THROW (value.isTrue (), std::bad_cast);

    value = array.getArray ();
    ASSERT_THROW (value.isTrue (), std::bad_cast);

    value = object.getObject ();
    ASSERT_THROW (value.isTrue (), std::bad_cast);
}

/**
 * @brief Test isFalse method.
 */
TEST (Value, isFalse)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    Value value;
    ASSERT_TRUE (value.isFalse ());

    value = nullptr;
    ASSERT_TRUE (value.isFalse ());

    value = true;
    ASSERT_FALSE (value.isFalse ());

    value = false;
    ASSERT_TRUE (value.isFalse ());

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_FALSE (value.isFalse ());

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_FALSE (value.isFalse ());

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_TRUE (value.isFalse ());

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_FALSE (value.isFalse ());

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_FALSE (value.isFalse ());

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_FALSE (value.isFalse ());

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_TRUE (value.isFalse ());

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_FALSE (value.isFalse ());

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_FALSE (value.isFalse ());

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_FALSE (value.isFalse ());

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_TRUE (value.isFalse ());

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_FALSE (value.isFalse ());

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_FALSE (value.isFalse ());

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_FALSE (value.isFalse ());

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_TRUE (value.isFalse ());

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_FALSE (value.isFalse ());

    value = std::numeric_limits<float>::min ();
    ASSERT_FALSE (value.isFalse ());

    value = std::numeric_limits<float>::max ();
    ASSERT_FALSE (value.isFalse ());

    value = std::numeric_limits<double>::min ();
    ASSERT_FALSE (value.isFalse ());

    value = std::numeric_limits<double>::max ();
    ASSERT_FALSE (value.isFalse ());

    value = "";
    ASSERT_THROW (value.isFalse (), std::bad_cast);

    value = "foo";
    ASSERT_THROW (value.isFalse (), std::bad_cast);

    value = "02:42:64:2f:6a:d0";
    ASSERT_THROW (value.isFalse (), std::bad_cast);

    value = "127.0.0.1";
    ASSERT_THROW (value.isFalse (), std::bad_cast);

    value = array.getArray ();
    ASSERT_THROW (value.isFalse (), std::bad_cast);

    value = object.getObject ();
    ASSERT_THROW (value.isFalse (), std::bad_cast);
}

/**
 * @brief Test isNumber method.
 */
TEST (Value, isNumber)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    Value value;
    ASSERT_FALSE (value.isNumber ());

    value = nullptr;
    ASSERT_FALSE (value.isNumber ());

    value = true;
    ASSERT_FALSE (value.isNumber ());

    value = false;
    ASSERT_FALSE (value.isNumber ());

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_TRUE (value.isNumber ());

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_TRUE (value.isNumber ());

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_TRUE (value.isNumber ());

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_TRUE (value.isNumber ());

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_TRUE (value.isNumber ());

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_TRUE (value.isNumber ());

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_TRUE (value.isNumber ());

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_TRUE (value.isNumber ());

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_TRUE (value.isNumber ());

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_TRUE (value.isNumber ());

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_TRUE (value.isNumber ());

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_TRUE (value.isNumber ());

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_TRUE (value.isNumber ());

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_TRUE (value.isNumber ());

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_TRUE (value.isNumber ());

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_TRUE (value.isNumber ());

    value = std::numeric_limits<float>::min ();
    ASSERT_TRUE (value.isNumber ());

    value = std::numeric_limits<float>::max ();
    ASSERT_TRUE (value.isNumber ());

    value = std::numeric_limits<double>::min ();
    ASSERT_TRUE (value.isNumber ());

    value = std::numeric_limits<double>::max ();
    ASSERT_TRUE (value.isNumber ());

    value = "";
    ASSERT_FALSE (value.isNumber ());

    value = "foo";
    ASSERT_FALSE (value.isNumber ());

    value = "02:42:64:2f:6a:d0";
    ASSERT_FALSE (value.isNumber ());

    value = "127.0.0.1";
    ASSERT_FALSE (value.isNumber ());

    value = array.getArray ();
    ASSERT_FALSE (value.isNumber ());

    value = object.getObject ();
    ASSERT_FALSE (value.isNumber ());
}

/**
 * @brief Test isInt8 method.
 */
TEST (Value, isInt8)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    Value value;
    ASSERT_FALSE (value.isInt8 ());

    value = nullptr;
    ASSERT_FALSE (value.isInt8 ());

    value = true;
    ASSERT_FALSE (value.isInt8 ());

    value = false;
    ASSERT_FALSE (value.isInt8 ());

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_TRUE (value.isInt8 ());

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_TRUE (value.isInt8 ());

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_TRUE (value.isInt8 ());

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_FALSE (value.isInt8 ());

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_FALSE (value.isInt8 ());

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_FALSE (value.isInt8 ());

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_TRUE (value.isInt8 ());

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_FALSE (value.isInt8 ());

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_FALSE (value.isInt8 ());

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_FALSE (value.isInt8 ());

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_TRUE (value.isInt8 ());

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_FALSE (value.isInt8 ());

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_FALSE (value.isInt8 ());

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_FALSE (value.isInt8 ());

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_TRUE (value.isInt8 ());

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_FALSE (value.isInt8 ());

    value = std::numeric_limits<float>::min ();
    ASSERT_FALSE (value.isInt8 ());

    value = std::numeric_limits<float>::max ();
    ASSERT_FALSE (value.isInt8 ());

    value = std::numeric_limits<double>::min ();
    ASSERT_FALSE (value.isInt8 ());

    value = std::numeric_limits<double>::max ();
    ASSERT_FALSE (value.isInt8 ());

    value = "";
    ASSERT_FALSE (value.isInt8 ());

    value = "foo";
    ASSERT_FALSE (value.isInt8 ());

    value = "02:42:64:2f:6a:d0";
    ASSERT_FALSE (value.isInt8 ());

    value = "127.0.0.1";
    ASSERT_FALSE (value.isInt8 ());

    value = array.getArray ();
    ASSERT_FALSE (value.isInt8 ());

    value = object.getObject ();
    ASSERT_FALSE (value.isInt8 ());
}

/**
 * @brief Test isUint8 method.
 */
TEST (Value, isUint8)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    Value value;
    ASSERT_FALSE (value.isUint8 ());

    value = nullptr;
    ASSERT_FALSE (value.isUint8 ());

    value = true;
    ASSERT_FALSE (value.isUint8 ());

    value = false;
    ASSERT_FALSE (value.isUint8 ());

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_FALSE (value.isUint8 ());

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_TRUE (value.isUint8 ());

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_TRUE (value.isUint8 ());

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_TRUE (value.isUint8 ());

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_FALSE (value.isUint8 ());

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_FALSE (value.isUint8 ());

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_TRUE (value.isUint8 ());

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_FALSE (value.isUint8 ());

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_FALSE (value.isUint8 ());

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_FALSE (value.isUint8 ());

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_TRUE (value.isUint8 ());

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_FALSE (value.isUint8 ());

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_FALSE (value.isUint8 ());

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_FALSE (value.isUint8 ());

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_TRUE (value.isUint8 ());

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_FALSE (value.isUint8 ());

    value = std::numeric_limits<float>::min ();
    ASSERT_FALSE (value.isUint8 ());

    value = std::numeric_limits<float>::max ();
    ASSERT_FALSE (value.isUint8 ());

    value = std::numeric_limits<double>::min ();
    ASSERT_FALSE (value.isUint8 ());

    value = std::numeric_limits<double>::max ();
    ASSERT_FALSE (value.isUint8 ());

    value = "";
    ASSERT_FALSE (value.isUint8 ());

    value = "foo";
    ASSERT_FALSE (value.isUint8 ());

    value = "02:42:64:2f:6a:d0";
    ASSERT_FALSE (value.isUint8 ());

    value = "127.0.0.1";
    ASSERT_FALSE (value.isUint8 ());

    value = array.getArray ();
    ASSERT_FALSE (value.isUint8 ());

    value = object.getObject ();
    ASSERT_FALSE (value.isUint8 ());
}

/**
 * @brief Test isInt16 method.
 */
TEST (Value, isInt16)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    Value value;
    ASSERT_FALSE (value.isInt16 ());

    value = nullptr;
    ASSERT_FALSE (value.isInt16 ());

    value = true;
    ASSERT_FALSE (value.isInt16 ());

    value = false;
    ASSERT_FALSE (value.isInt16 ());

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_TRUE (value.isInt16 ());

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_TRUE (value.isInt16 ());

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_TRUE (value.isInt16 ());

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_TRUE (value.isInt16 ());

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_TRUE (value.isInt16 ());

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_TRUE (value.isInt16 ());

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_TRUE (value.isInt16 ());

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_FALSE (value.isInt16 ());

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_FALSE (value.isInt16 ());

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_FALSE (value.isInt16 ());

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_TRUE (value.isInt16 ());

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_FALSE (value.isInt16 ());

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_FALSE (value.isInt16 ());

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_FALSE (value.isInt16 ());

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_TRUE (value.isInt16 ());

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_FALSE (value.isInt16 ());

    value = std::numeric_limits<float>::min ();
    ASSERT_FALSE (value.isInt16 ());

    value = std::numeric_limits<float>::max ();
    ASSERT_FALSE (value.isInt16 ());

    value = std::numeric_limits<double>::min ();
    ASSERT_FALSE (value.isInt16 ());

    value = std::numeric_limits<double>::max ();
    ASSERT_FALSE (value.isInt16 ());

    value = "";
    ASSERT_FALSE (value.isInt16 ());

    value = "foo";
    ASSERT_FALSE (value.isInt16 ());

    value = "02:42:64:2f:6a:d0";
    ASSERT_FALSE (value.isInt16 ());

    value = "127.0.0.1";
    ASSERT_FALSE (value.isInt16 ());

    value = array.getArray ();
    ASSERT_FALSE (value.isInt16 ());

    value = object.getObject ();
    ASSERT_FALSE (value.isInt16 ());
}

/**
 * @brief Test isUint16 method.
 */
TEST (Value, isUint16)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    Value value;
    ASSERT_FALSE (value.isUint16 ());

    value = nullptr;
    ASSERT_FALSE (value.isUint16 ());

    value = true;
    ASSERT_FALSE (value.isUint16 ());

    value = false;
    ASSERT_FALSE (value.isUint16 ());

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_FALSE (value.isUint16 ());

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_TRUE (value.isUint16 ());

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_TRUE (value.isUint16 ());

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_TRUE (value.isUint16 ());

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_FALSE (value.isUint16 ());

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_TRUE (value.isUint16 ());

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_TRUE (value.isUint16 ());

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_TRUE (value.isUint16 ());

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_FALSE (value.isUint16 ());

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_FALSE (value.isUint16 ());

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_TRUE (value.isUint16 ());

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_FALSE (value.isUint16 ());

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_FALSE (value.isUint16 ());

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_FALSE (value.isUint16 ());

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_TRUE (value.isUint16 ());

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_FALSE (value.isUint16 ());

    value = std::numeric_limits<float>::min ();
    ASSERT_FALSE (value.isUint16 ());

    value = std::numeric_limits<float>::max ();
    ASSERT_FALSE (value.isUint16 ());

    value = std::numeric_limits<double>::min ();
    ASSERT_FALSE (value.isUint16 ());

    value = std::numeric_limits<double>::max ();
    ASSERT_FALSE (value.isUint16 ());

    value = "";
    ASSERT_FALSE (value.isUint16 ());

    value = "foo";
    ASSERT_FALSE (value.isUint16 ());

    value = "02:42:64:2f:6a:d0";
    ASSERT_FALSE (value.isUint16 ());

    value = "127.0.0.1";
    ASSERT_FALSE (value.isUint16 ());

    value = array.getArray ();
    ASSERT_FALSE (value.isUint16 ());

    value = object.getObject ();
    ASSERT_FALSE (value.isUint16 ());
}

/**
 * @brief Test isInt method.
 */
TEST (Value, isInt)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    Value value;
    ASSERT_FALSE (value.isInt ());

    value = nullptr;
    ASSERT_FALSE (value.isInt ());

    value = true;
    ASSERT_FALSE (value.isInt ());

    value = false;
    ASSERT_FALSE (value.isInt ());

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_TRUE (value.isInt ());

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_TRUE (value.isInt ());

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_TRUE (value.isInt ());

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_TRUE (value.isInt ());

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_TRUE (value.isInt ());

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_TRUE (value.isInt ());

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_TRUE (value.isInt ());

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_TRUE (value.isInt ());

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_TRUE (value.isInt ());

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_TRUE (value.isInt ());

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_TRUE (value.isInt ());

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_FALSE (value.isInt ());

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_FALSE (value.isInt ());

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_FALSE (value.isInt ());

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_TRUE (value.isInt ());

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_FALSE (value.isInt ());

    value = std::numeric_limits<float>::min ();
    ASSERT_FALSE (value.isInt ());

    value = std::numeric_limits<float>::max ();
    ASSERT_FALSE (value.isInt ());

    value = std::numeric_limits<double>::min ();
    ASSERT_FALSE (value.isInt ());

    value = std::numeric_limits<double>::max ();
    ASSERT_FALSE (value.isInt ());

    value = "";
    ASSERT_FALSE (value.isInt ());

    value = "foo";
    ASSERT_FALSE (value.isInt ());

    value = "02:42:64:2f:6a:d0";
    ASSERT_FALSE (value.isInt ());

    value = "127.0.0.1";
    ASSERT_FALSE (value.isInt ());

    value = array.getArray ();
    ASSERT_FALSE (value.isInt ());

    value = object.getObject ();
    ASSERT_FALSE (value.isInt ());
}

/**
 * @brief Test isUint method.
 */
TEST (Value, isUint)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    Value value;
    ASSERT_FALSE (value.isUint ());

    value = nullptr;
    ASSERT_FALSE (value.isUint ());

    value = true;
    ASSERT_FALSE (value.isUint ());

    value = false;
    ASSERT_FALSE (value.isUint ());

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_FALSE (value.isUint ());

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_TRUE (value.isUint ());

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_TRUE (value.isUint ());

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_TRUE (value.isUint ());

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_FALSE (value.isUint ());

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_TRUE (value.isUint ());

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_TRUE (value.isUint ());

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_TRUE (value.isUint ());

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_FALSE (value.isUint ());

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_TRUE (value.isUint ());

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_TRUE (value.isUint ());

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_TRUE (value.isUint ());

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_FALSE (value.isUint ());

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_FALSE (value.isUint ());

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_TRUE (value.isUint ());

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_FALSE (value.isUint ());

    value = std::numeric_limits<float>::min ();
    ASSERT_FALSE (value.isUint ());

    value = std::numeric_limits<float>::max ();
    ASSERT_FALSE (value.isUint ());

    value = std::numeric_limits<double>::min ();
    ASSERT_FALSE (value.isUint ());

    value = std::numeric_limits<double>::max ();
    ASSERT_FALSE (value.isUint ());

    value = "";
    ASSERT_FALSE (value.isUint ());

    value = "foo";
    ASSERT_FALSE (value.isUint ());

    value = "02:42:64:2f:6a:d0";
    ASSERT_FALSE (value.isUint ());

    value = "127.0.0.1";
    ASSERT_FALSE (value.isUint ());

    value = array.getArray ();
    ASSERT_FALSE (value.isUint ());

    value = object.getObject ();
    ASSERT_FALSE (value.isUint ());
}

/**
 * @brief Test isInt64 method.
 */
TEST (Value, isInt64)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    Value value;
    ASSERT_FALSE (value.isInt64 ());

    value = nullptr;
    ASSERT_FALSE (value.isInt64 ());

    value = true;
    ASSERT_FALSE (value.isInt64 ());

    value = false;
    ASSERT_FALSE (value.isInt64 ());

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_TRUE (value.isInt64 ());

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_TRUE (value.isInt64 ());

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_TRUE (value.isInt64 ());

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_TRUE (value.isInt64 ());

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_TRUE (value.isInt64 ());

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_TRUE (value.isInt64 ());

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_TRUE (value.isInt64 ());

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_TRUE (value.isInt64 ());

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_TRUE (value.isInt64 ());

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_TRUE (value.isInt64 ());

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_TRUE (value.isInt64 ());

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_TRUE (value.isInt64 ());

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_TRUE (value.isInt64 ());

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_TRUE (value.isInt64 ());

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_TRUE (value.isInt64 ());

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_FALSE (value.isInt64 ());

    value = std::numeric_limits<float>::min ();
    ASSERT_FALSE (value.isInt64 ());

    value = std::numeric_limits<float>::max ();
    ASSERT_FALSE (value.isInt64 ());

    value = std::numeric_limits<double>::min ();
    ASSERT_FALSE (value.isInt64 ());

    value = std::numeric_limits<double>::max ();
    ASSERT_FALSE (value.isInt64 ());

    value = "";
    ASSERT_FALSE (value.isInt64 ());

    value = "foo";
    ASSERT_FALSE (value.isInt64 ());

    value = "02:42:64:2f:6a:d0";
    ASSERT_FALSE (value.isInt64 ());

    value = "127.0.0.1";
    ASSERT_FALSE (value.isInt64 ());

    value = array.getArray ();
    ASSERT_FALSE (value.isInt64 ());

    value = object.getObject ();
    ASSERT_FALSE (value.isInt64 ());
}

/**
 * @brief Test isUint64 method.
 */
TEST (Value, isUint64)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    Value value;
    ASSERT_FALSE (value.isUint64 ());

    value = nullptr;
    ASSERT_FALSE (value.isUint64 ());

    value = true;
    ASSERT_FALSE (value.isUint64 ());

    value = false;
    ASSERT_FALSE (value.isUint64 ());

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_FALSE (value.isUint64 ());

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_TRUE (value.isUint64 ());

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_TRUE (value.isUint64 ());

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_TRUE (value.isUint64 ());

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_FALSE (value.isUint64 ());

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_TRUE (value.isUint64 ());

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_TRUE (value.isUint64 ());

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_TRUE (value.isUint64 ());

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_FALSE (value.isUint64 ());

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_TRUE (value.isUint64 ());

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_TRUE (value.isUint64 ());

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_TRUE (value.isUint64 ());

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_FALSE (value.isUint64 ());

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_TRUE (value.isUint64 ());

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_TRUE (value.isUint64 ());

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_TRUE (value.isUint64 ());

    value = std::numeric_limits<float>::min ();
    ASSERT_FALSE (value.isUint64 ());

    value = std::numeric_limits<float>::max ();
    ASSERT_FALSE (value.isUint64 ());

    value = std::numeric_limits<double>::min ();
    ASSERT_FALSE (value.isUint64 ());

    value = std::numeric_limits<double>::max ();
    ASSERT_FALSE (value.isUint64 ());

    value = "";
    ASSERT_FALSE (value.isUint64 ());

    value = "foo";
    ASSERT_FALSE (value.isUint64 ());

    value = "02:42:64:2f:6a:d0";
    ASSERT_FALSE (value.isUint64 ());

    value = "127.0.0.1";
    ASSERT_FALSE (value.isUint64 ());

    value = array.getArray ();
    ASSERT_FALSE (value.isUint64 ());

    value = object.getObject ();
    ASSERT_FALSE (value.isUint64 ());
}

/**
 * @brief Test isFloat method.
 */
TEST (Value, isFloat)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    Value value;
    ASSERT_FALSE (value.isFloat ());

    value = nullptr;
    ASSERT_FALSE (value.isFloat ());

    value = true;
    ASSERT_FALSE (value.isFloat ());

    value = false;
    ASSERT_FALSE (value.isFloat ());

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_TRUE (value.isFloat ());

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_TRUE (value.isFloat ());

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_TRUE (value.isFloat ());

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_TRUE (value.isFloat ());

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_TRUE (value.isFloat ());

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_TRUE (value.isFloat ());

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_TRUE (value.isFloat ());

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_TRUE (value.isFloat ());

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_TRUE (value.isFloat ());

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_TRUE (value.isFloat ());

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_TRUE (value.isFloat ());

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_TRUE (value.isFloat ());

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_TRUE (value.isFloat ());

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_TRUE (value.isFloat ());

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_TRUE (value.isFloat ());

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_TRUE (value.isFloat ());

    value = std::numeric_limits<float>::min ();
    ASSERT_TRUE (value.isFloat ());

    value = std::numeric_limits<float>::max ();
    ASSERT_TRUE (value.isFloat ());

    value = std::numeric_limits<double>::min ();
    ASSERT_TRUE (value.isFloat ());

    value = std::numeric_limits<double>::max ();
    ASSERT_TRUE (value.isFloat ());

    value = "";
    ASSERT_FALSE (value.isFloat ());

    value = "foo";
    ASSERT_FALSE (value.isFloat ());

    value = "02:42:64:2f:6a:d0";
    ASSERT_FALSE (value.isFloat ());

    value = "127.0.0.1";
    ASSERT_FALSE (value.isFloat ());

    value = array.getArray ();
    ASSERT_FALSE (value.isFloat ());

    value = object.getObject ();
    ASSERT_FALSE (value.isFloat ());
}

/**
 * @brief Test isDouble method.
 */
TEST (Value, isDouble)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    Value value;
    ASSERT_FALSE (value.isDouble ());

    value = nullptr;
    ASSERT_FALSE (value.isDouble ());

    value = true;
    ASSERT_FALSE (value.isDouble ());

    value = false;
    ASSERT_FALSE (value.isDouble ());

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_TRUE (value.isDouble ());

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_TRUE (value.isDouble ());

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_TRUE (value.isDouble ());

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_TRUE (value.isDouble ());

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_TRUE (value.isDouble ());

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_TRUE (value.isDouble ());

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_TRUE (value.isDouble ());

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_TRUE (value.isDouble ());

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_TRUE (value.isDouble ());

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_TRUE (value.isDouble ());

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_TRUE (value.isDouble ());

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_TRUE (value.isDouble ());

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_TRUE (value.isDouble ());

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_TRUE (value.isDouble ());

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_TRUE (value.isDouble ());

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_TRUE (value.isDouble ());

    value = std::numeric_limits<float>::min ();
    ASSERT_TRUE (value.isDouble ());

    value = std::numeric_limits<float>::max ();
    ASSERT_TRUE (value.isDouble ());

    value = std::numeric_limits<double>::min ();
    ASSERT_TRUE (value.isDouble ());

    value = std::numeric_limits<double>::max ();
    ASSERT_TRUE (value.isDouble ());

    value = "";
    ASSERT_FALSE (value.isDouble ());

    value = "foo";
    ASSERT_FALSE (value.isDouble ());

    value = "02:42:64:2f:6a:d0";
    ASSERT_FALSE (value.isDouble ());

    value = "127.0.0.1";
    ASSERT_FALSE (value.isDouble ());

    value = array.getArray ();
    ASSERT_FALSE (value.isDouble ());

    value = object.getObject ();
    ASSERT_FALSE (value.isDouble ());
}

/**
 * @brief Test isString method.
 */
TEST (Value, isString)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    Value value;
    ASSERT_FALSE (value.isString ());

    value = nullptr;
    ASSERT_FALSE (value.isString ());

    value = true;
    ASSERT_FALSE (value.isString ());

    value = false;
    ASSERT_FALSE (value.isString ());

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_FALSE (value.isString ());

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_FALSE (value.isString ());

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_FALSE (value.isString ());

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_FALSE (value.isString ());

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_FALSE (value.isString ());

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_FALSE (value.isString ());

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_FALSE (value.isString ());

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_FALSE (value.isString ());

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_FALSE (value.isString ());

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_FALSE (value.isString ());

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_FALSE (value.isString ());

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_FALSE (value.isString ());

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_FALSE (value.isString ());

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_FALSE (value.isString ());

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_FALSE (value.isString ());

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_FALSE (value.isString ());

    value = std::numeric_limits<float>::min ();
    ASSERT_FALSE (value.isString ());

    value = std::numeric_limits<float>::max ();
    ASSERT_FALSE (value.isString ());

    value = std::numeric_limits<double>::min ();
    ASSERT_FALSE (value.isString ());

    value = std::numeric_limits<double>::max ();
    ASSERT_FALSE (value.isString ());

    value = "";
    ASSERT_TRUE (value.isString ());

    value = "foo";
    ASSERT_TRUE (value.isString ());

    value = "02:42:64:2f:6a:d0";
    ASSERT_TRUE (value.isString ());

    value = "127.0.0.1";
    ASSERT_TRUE (value.isString ());

    value = array.getArray ();
    ASSERT_FALSE (value.isString ());

    value = object.getObject ();
    ASSERT_FALSE (value.isString ());
}

/**
 * @brief Test isBinary method.
 */
TEST (Value, isBinary)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    const uint8_t bytes[] = {0x00, 0x01, 0xff};

    Value value;
    ASSERT_FALSE (value.isBinary ());

    value = nullptr;
    ASSERT_FALSE (value.isBinary ());

    value = true;
    ASSERT_FALSE (value.isBinary ());

    value = false;
    ASSERT_FALSE (value.isBinary ());

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_FALSE (value.isBinary ());

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_FALSE (value.isBinary ());

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_FALSE (value.isBinary ());

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_FALSE (value.isBinary ());

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_FALSE (value.isBinary ());

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_FALSE (value.isBinary ());

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_FALSE (value.isBinary ());

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_FALSE (value.isBinary ());

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_FALSE (value.isBinary ());

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_FALSE (value.isBinary ());

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_FALSE (value.isBinary ());

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_FALSE (value.isBinary ());

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_FALSE (value.isBinary ());

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_FALSE (value.isBinary ());

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_FALSE (value.isBinary ());

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_FALSE (value.isBinary ());

    value = std::numeric_limits<float>::min ();
    ASSERT_FALSE (value.isBinary ());

    value = std::numeric_limits<float>::max ();
    ASSERT_FALSE (value.isBinary ());

    value = std::numeric_limits<double>::min ();
    ASSERT_FALSE (value.isBinary ());

    value = std::numeric_limits<double>::max ();
    ASSERT_FALSE (value.isBinary ());

    value = "";
    ASSERT_FALSE (value.isBinary ());

    value = "foo";
    ASSERT_FALSE (value.isBinary ());

    value = "02:42:64:2f:6a:d0";
    ASSERT_FALSE (value.isBinary ());

    value = "127.0.0.1";
    ASSERT_FALSE (value.isBinary ());

    value = std::span<const uint8_t> (bytes, sizeof (bytes));
    ASSERT_TRUE (value.isBinary ());

    value = array.getArray ();
    ASSERT_FALSE (value.isBinary ());

    value = object.getObject ();
    ASSERT_FALSE (value.isBinary ());
}

/**
 * @brief Test isArray method.
 */
TEST (Value, isArray)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    Value value;
    ASSERT_FALSE (value.isArray ());

    value = nullptr;
    ASSERT_FALSE (value.isArray ());

    value = true;
    ASSERT_FALSE (value.isArray ());

    value = false;
    ASSERT_FALSE (value.isArray ());

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_FALSE (value.isArray ());

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_FALSE (value.isArray ());

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_FALSE (value.isArray ());

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_FALSE (value.isArray ());

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_FALSE (value.isArray ());

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_FALSE (value.isArray ());

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_FALSE (value.isArray ());

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_FALSE (value.isArray ());

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_FALSE (value.isArray ());

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_FALSE (value.isArray ());

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_FALSE (value.isArray ());

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_FALSE (value.isArray ());

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_FALSE (value.isArray ());

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_FALSE (value.isArray ());

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_FALSE (value.isArray ());

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_FALSE (value.isArray ());

    value = std::numeric_limits<float>::min ();
    ASSERT_FALSE (value.isArray ());

    value = std::numeric_limits<float>::max ();
    ASSERT_FALSE (value.isArray ());

    value = std::numeric_limits<double>::min ();
    ASSERT_FALSE (value.isArray ());

    value = std::numeric_limits<double>::max ();
    ASSERT_FALSE (value.isArray ());

    value = "";
    ASSERT_FALSE (value.isArray ());

    value = "foo";
    ASSERT_FALSE (value.isArray ());

    value = "02:42:64:2f:6a:d0";
    ASSERT_FALSE (value.isArray ());

    value = "127.0.0.1";
    ASSERT_FALSE (value.isArray ());

    value = array.getArray ();
    ASSERT_TRUE (value.isArray ());

    value = object.getObject ();
    ASSERT_FALSE (value.isArray ());
}

/**
 * @brief Test isObject method.
 */
TEST (Value, isObject)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    Value value;
    ASSERT_FALSE (value.isObject ());

    value = nullptr;
    ASSERT_FALSE (value.isObject ());

    value = true;
    ASSERT_FALSE (value.isObject ());

    value = false;
    ASSERT_FALSE (value.isObject ());

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_FALSE (value.isObject ());

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_FALSE (value.isObject ());

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_FALSE (value.isObject ());

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_FALSE (value.isObject ());

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_FALSE (value.isObject ());

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_FALSE (value.isObject ());

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_FALSE (value.isObject ());

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_FALSE (value.isObject ());

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_FALSE (value.isObject ());

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_FALSE (value.isObject ());

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_FALSE (value.isObject ());

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_FALSE (value.isObject ());

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_FALSE (value.isObject ());

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_FALSE (value.isObject ());

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_FALSE (value.isObject ());

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_FALSE (value.isObject ());

    value = std::numeric_limits<float>::min ();
    ASSERT_FALSE (value.isObject ());

    value = std::numeric_limits<float>::max ();
    ASSERT_FALSE (value.isObject ());

    value = std::numeric_limits<double>::min ();
    ASSERT_FALSE (value.isObject ());

    value = std::numeric_limits<double>::max ();
    ASSERT_FALSE (value.isObject ());

    value = "";
    ASSERT_FALSE (value.isObject ());

    value = "foo";
    ASSERT_FALSE (value.isObject ());

    value = "02:42:64:2f:6a:d0";
    ASSERT_FALSE (value.isObject ());

    value = "127.0.0.1";
    ASSERT_FALSE (value.isObject ());

    value = array.getArray ();
    ASSERT_FALSE (value.isObject ());

    value = object.getObject ();
    ASSERT_TRUE (value.isObject ());
}

/**
 * @brief Test getBool method.
 */
TEST (Value, getBool)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    Value value;
    ASSERT_FALSE (value.getBool ());
    ASSERT_FALSE (static_cast<bool> (value));

    value = nullptr;
    ASSERT_FALSE (value.getBool ());
    ASSERT_FALSE (static_cast<bool> (value));

    value = true;
    ASSERT_TRUE (value.getBool ());
    ASSERT_TRUE (static_cast<bool> (value));

    value = false;
    ASSERT_FALSE (value.getBool ());
    ASSERT_FALSE (static_cast<bool> (value));

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_TRUE (value.getBool ());
    ASSERT_TRUE (static_cast<bool> (value));

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_TRUE (value.getBool ());
    ASSERT_TRUE (static_cast<bool> (value));

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_FALSE (value.getBool ());
    ASSERT_FALSE (static_cast<bool> (value));

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_TRUE (value.getBool ());
    ASSERT_TRUE (static_cast<bool> (value));

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_TRUE (value.getBool ());
    ASSERT_TRUE (static_cast<bool> (value));

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_TRUE (value.getBool ());
    ASSERT_TRUE (static_cast<bool> (value));

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_FALSE (value.getBool ());
    ASSERT_FALSE (static_cast<bool> (value));

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_TRUE (value.getBool ());
    ASSERT_TRUE (static_cast<bool> (value));

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_TRUE (value.getBool ());
    ASSERT_TRUE (static_cast<bool> (value));

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_TRUE (value.getBool ());
    ASSERT_TRUE (static_cast<bool> (value));

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_FALSE (value.getBool ());
    ASSERT_FALSE (static_cast<bool> (value));

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_TRUE (value.getBool ());
    ASSERT_TRUE (static_cast<bool> (value));

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_TRUE (value.getBool ());
    ASSERT_TRUE (static_cast<bool> (value));

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_TRUE (value.getBool ());
    ASSERT_TRUE (static_cast<bool> (value));

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_FALSE (value.getBool ());
    ASSERT_FALSE (static_cast<bool> (value));

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_TRUE (value.getBool ());
    ASSERT_TRUE (static_cast<bool> (value));

    value = std::numeric_limits<float>::min ();
    ASSERT_TRUE (value.getBool ());
    ASSERT_TRUE (static_cast<bool> (value));

    value = std::numeric_limits<float>::max ();
    ASSERT_TRUE (value.getBool ());
    ASSERT_TRUE (static_cast<bool> (value));

    value = std::numeric_limits<double>::min ();
    ASSERT_TRUE (value.getBool ());
    ASSERT_TRUE (static_cast<bool> (value));

    value = std::numeric_limits<double>::max ();
    ASSERT_TRUE (value.getBool ());
    ASSERT_TRUE (static_cast<bool> (value));

    bool tmp = false;
    (void)tmp;

    value = "";
    ASSERT_THROW (value.getBool (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<bool> (value)), std::bad_cast);

    value = "foo";
    ASSERT_THROW (value.getBool (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<bool> (value)), std::bad_cast);
    value = "02:42:64:2f:6a:d0";
    ASSERT_THROW (value.getBool (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<bool> (value)), std::bad_cast);

    value = "127.0.0.1";
    ASSERT_THROW (value.getBool (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<bool> (value)), std::bad_cast);
    value = array.getArray ();
    ASSERT_THROW (value.getBool (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<bool> (value)), std::bad_cast);

    value = object.getObject ();
    ASSERT_THROW (value.getBool (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<bool> (value)), std::bad_cast);
}

/**
 * @brief Test getInt8 method.
 */
TEST (Value, getInt8)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    int8_t tmp = 0;
    (void)tmp;

    Value value;
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int8_t> (value)), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int8_t> (value)), std::bad_cast);

    value = true;
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int8_t> (value)), std::bad_cast);

    value = false;
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int8_t> (value)), std::bad_cast);

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_EQ (value.getInt8 (), std::numeric_limits<int8_t>::min ());
    ASSERT_EQ (static_cast<int8_t> (value), std::numeric_limits<int8_t>::min ());

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_EQ (value.getInt8 (), std::numeric_limits<int8_t>::max ());
    ASSERT_EQ (static_cast<int8_t> (value), std::numeric_limits<int8_t>::max ());

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_EQ (value.getInt8 (), std::numeric_limits<uint8_t>::min ());
    ASSERT_EQ (static_cast<int8_t> (value), std::numeric_limits<uint8_t>::min ());

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int8_t> (value)), std::bad_cast);

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int8_t> (value)), std::bad_cast);

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int8_t> (value)), std::bad_cast);

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_EQ (value.getInt8 (), std::numeric_limits<uint16_t>::min ());
    ASSERT_EQ (static_cast<int8_t> (value), std::numeric_limits<uint16_t>::min ());

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int8_t> (value)), std::bad_cast);

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int8_t> (value)), std::bad_cast);

    value = int32_t (12);
    ASSERT_EQ (value.getInt8 (), 12);
    ASSERT_EQ (static_cast<int8_t> (value), 12);

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int8_t> (value)), std::bad_cast);

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_EQ (value.getInt8 (), std::numeric_limits<uint32_t>::min ());
    ASSERT_EQ (static_cast<int8_t> (value), std::numeric_limits<uint32_t>::min ());

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int8_t> (value)), std::bad_cast);

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int8_t> (value)), std::bad_cast);

    value = int64_t (12);
    ASSERT_EQ (value.getInt8 (), 12);
    ASSERT_EQ (static_cast<int8_t> (value), 12);

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int8_t> (value)), std::bad_cast);

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_EQ (value.getInt8 (), std::numeric_limits<uint64_t>::min ());
    ASSERT_EQ (static_cast<int8_t> (value), std::numeric_limits<uint64_t>::min ());

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int8_t> (value)), std::bad_cast);

    value = std::numeric_limits<float>::min ();
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int8_t> (value)), std::bad_cast);

    value = std::numeric_limits<float>::max ();
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int8_t> (value)), std::bad_cast);

    value = std::numeric_limits<double>::min ();
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int8_t> (value)), std::bad_cast);

    value = double (12.0);
    ASSERT_EQ (value.getInt8 (), 12);
    ASSERT_EQ (static_cast<int8_t> (value), 12);

    value = std::numeric_limits<double>::max ();
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int8_t> (value)), std::bad_cast);

    value = "";
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int8_t> (value)), std::bad_cast);

    value = "foo";
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int8_t> (value)), std::bad_cast);

    value = "02:42:64:2f:6a:d0";
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int8_t> (value)), std::bad_cast);

    value = "127.0.0.1";
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int8_t> (value)), std::bad_cast);

    value = array.getArray ();
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int8_t> (value)), std::bad_cast);

    value = object.getObject ();
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int8_t> (value)), std::bad_cast);
}

/**
 * @brief Test getUint8 method.
 */
TEST (Value, getUint8)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    uint8_t tmp = 0;
    (void)tmp;

    Value value;
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint8_t> (value)), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint8_t> (value)), std::bad_cast);

    value = true;
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint8_t> (value)), std::bad_cast);

    value = false;
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint8_t> (value)), std::bad_cast);

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint8_t> (value)), std::bad_cast);

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_EQ (value.getUint8 (), std::numeric_limits<int8_t>::max ());
    ASSERT_EQ (static_cast<uint8_t> (value), std::numeric_limits<int8_t>::max ());

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_EQ (value.getUint8 (), std::numeric_limits<uint8_t>::min ());
    ASSERT_EQ (static_cast<uint8_t> (value), std::numeric_limits<uint8_t>::min ());

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_EQ (value.getUint8 (), std::numeric_limits<uint8_t>::max ());
    ASSERT_EQ (static_cast<uint8_t> (value), std::numeric_limits<uint8_t>::max ());

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint8_t> (value)), std::bad_cast);

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint8_t> (value)), std::bad_cast);

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_EQ (value.getUint8 (), std::numeric_limits<uint16_t>::min ());
    ASSERT_EQ (static_cast<uint8_t> (value), std::numeric_limits<uint16_t>::min ());

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint8_t> (value)), std::bad_cast);

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint8_t> (value)), std::bad_cast);

    value = int32_t (12);
    ASSERT_EQ (value.getUint8 (), 12);
    ASSERT_EQ (static_cast<uint8_t> (value), 12);

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint8_t> (value)), std::bad_cast);

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_EQ (value.getUint8 (), std::numeric_limits<uint32_t>::min ());
    ASSERT_EQ (static_cast<uint8_t> (value), std::numeric_limits<uint32_t>::min ());

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint8_t> (value)), std::bad_cast);

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint8_t> (value)), std::bad_cast);

    value = int64_t (12);
    ASSERT_EQ (value.getUint8 (), 12);
    ASSERT_EQ (static_cast<uint8_t> (value), 12);

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint8_t> (value)), std::bad_cast);

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_EQ (value.getUint8 (), std::numeric_limits<uint64_t>::min ());
    ASSERT_EQ (static_cast<uint8_t> (value), std::numeric_limits<uint64_t>::min ());

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint8_t> (value)), std::bad_cast);

    value = std::numeric_limits<float>::min ();
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint8_t> (value)), std::bad_cast);

    value = std::numeric_limits<float>::max ();
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint8_t> (value)), std::bad_cast);

    value = std::numeric_limits<double>::min ();
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint8_t> (value)), std::bad_cast);

    value = double (12.0);
    ASSERT_EQ (value.getUint8 (), 12);
    ASSERT_EQ (static_cast<uint8_t> (value), 12);

    value = std::numeric_limits<double>::max ();
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint8_t> (value)), std::bad_cast);

    value = "";
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint8_t> (value)), std::bad_cast);

    value = "foo";
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint8_t> (value)), std::bad_cast);

    value = "02:42:64:2f:6a:d0";
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint8_t> (value)), std::bad_cast);

    value = "127.0.0.1";
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint8_t> (value)), std::bad_cast);

    value = array.getArray ();
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint8_t> (value)), std::bad_cast);

    value = object.getObject ();
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint8_t> (value)), std::bad_cast);
}

/**
 * @brief Test getInt16 method.
 */
TEST (Value, getInt16)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    int16_t tmp = 0;
    (void)tmp;

    Value value;
    ASSERT_THROW (value.getInt16 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int16_t> (value)), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.getInt16 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int16_t> (value)), std::bad_cast);

    value = true;
    ASSERT_THROW (value.getInt16 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int16_t> (value)), std::bad_cast);

    value = false;
    ASSERT_THROW (value.getInt16 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int16_t> (value)), std::bad_cast);

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_EQ (value.getInt16 (), std::numeric_limits<int8_t>::min ());
    ASSERT_EQ (static_cast<int16_t> (value), std::numeric_limits<int8_t>::min ());

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_EQ (value.getInt16 (), std::numeric_limits<int8_t>::max ());
    ASSERT_EQ (static_cast<int16_t> (value), std::numeric_limits<int8_t>::max ());

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_EQ (value.getInt16 (), std::numeric_limits<uint8_t>::min ());
    ASSERT_EQ (static_cast<int16_t> (value), std::numeric_limits<uint8_t>::min ());

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_EQ (value.getInt16 (), std::numeric_limits<uint8_t>::max ());
    ASSERT_EQ (static_cast<int16_t> (value), std::numeric_limits<uint8_t>::max ());

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_EQ (value.getInt16 (), std::numeric_limits<int16_t>::min ());
    ASSERT_EQ (static_cast<int16_t> (value), std::numeric_limits<int16_t>::min ());

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_EQ (value.getInt16 (), std::numeric_limits<int16_t>::max ());
    ASSERT_EQ (static_cast<int16_t> (value), std::numeric_limits<int16_t>::max ());

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_EQ (value.getInt16 (), std::numeric_limits<uint16_t>::min ());
    ASSERT_EQ (static_cast<int16_t> (value), std::numeric_limits<uint16_t>::min ());

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_THROW (value.getInt16 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int16_t> (value)), std::bad_cast);

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_THROW (value.getInt16 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int16_t> (value)), std::bad_cast);

    value = int32_t (12);
    ASSERT_EQ (value.getInt16 (), 12);
    ASSERT_EQ (static_cast<int16_t> (value), 12);

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_THROW (value.getInt16 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int16_t> (value)), std::bad_cast);

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_EQ (value.getInt16 (), std::numeric_limits<uint32_t>::min ());
    ASSERT_EQ (static_cast<int16_t> (value), std::numeric_limits<uint32_t>::min ());

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_THROW (value.getInt16 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int16_t> (value)), std::bad_cast);

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_THROW (value.getInt16 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int16_t> (value)), std::bad_cast);

    value = int64_t (12);
    ASSERT_EQ (value.getInt16 (), 12);
    ASSERT_EQ (static_cast<int16_t> (value), 12);

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_THROW (value.getInt16 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int16_t> (value)), std::bad_cast);

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_EQ (value.getInt16 (), std::numeric_limits<uint64_t>::min ());
    ASSERT_EQ (static_cast<int16_t> (value), std::numeric_limits<uint64_t>::min ());

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_THROW (value.getInt16 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int16_t> (value)), std::bad_cast);

    value = std::numeric_limits<float>::min ();
    ASSERT_THROW (value.getInt16 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int16_t> (value)), std::bad_cast);

    value = std::numeric_limits<float>::max ();
    ASSERT_THROW (value.getInt16 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int16_t> (value)), std::bad_cast);

    value = std::numeric_limits<double>::min ();
    ASSERT_THROW (value.getInt16 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int16_t> (value)), std::bad_cast);

    value = double (12.0);
    ASSERT_EQ (value.getInt16 (), 12);
    ASSERT_EQ (static_cast<int16_t> (value), 12);

    value = std::numeric_limits<double>::max ();
    ASSERT_THROW (value.getInt16 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int16_t> (value)), std::bad_cast);

    value = "";
    ASSERT_THROW (value.getInt16 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int16_t> (value)), std::bad_cast);

    value = "foo";
    ASSERT_THROW (value.getInt16 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int16_t> (value)), std::bad_cast);

    value = "02:42:64:2f:6a:d0";
    ASSERT_THROW (value.getInt16 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int16_t> (value)), std::bad_cast);

    value = "127.0.0.1";
    ASSERT_THROW (value.getInt16 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int16_t> (value)), std::bad_cast);

    value = array.getArray ();
    ASSERT_THROW (value.getInt16 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int16_t> (value)), std::bad_cast);

    value = object.getObject ();
    ASSERT_THROW (value.getInt16 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int16_t> (value)), std::bad_cast);
}

/**
 * @brief Test getUint16 method.
 */
TEST (Value, getUint16)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    uint16_t tmp = 0;
    (void)tmp;

    Value value;
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint16_t> (value)), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint16_t> (value)), std::bad_cast);

    value = true;
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint16_t> (value)), std::bad_cast);

    value = false;
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint16_t> (value)), std::bad_cast);

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint16_t> (value)), std::bad_cast);

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_EQ (value.getUint16 (), std::numeric_limits<int8_t>::max ());
    ASSERT_EQ (static_cast<uint16_t> (value), std::numeric_limits<int8_t>::max ());

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_EQ (value.getUint16 (), std::numeric_limits<uint8_t>::min ());
    ASSERT_EQ (static_cast<uint16_t> (value), std::numeric_limits<uint8_t>::min ());

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_EQ (value.getUint16 (), std::numeric_limits<uint8_t>::max ());
    ASSERT_EQ (static_cast<uint16_t> (value), std::numeric_limits<uint8_t>::max ());

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint16_t> (value)), std::bad_cast);

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_EQ (value.getUint16 (), std::numeric_limits<int16_t>::max ());
    ASSERT_EQ (static_cast<uint16_t> (value), std::numeric_limits<int16_t>::max ());

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_EQ (value.getUint16 (), std::numeric_limits<uint16_t>::min ());
    ASSERT_EQ (static_cast<uint16_t> (value), std::numeric_limits<uint16_t>::min ());

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_EQ (value.getUint16 (), std::numeric_limits<uint16_t>::max ());
    ASSERT_EQ (static_cast<uint16_t> (value), std::numeric_limits<uint16_t>::max ());

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint16_t> (value)), std::bad_cast);

    value = int32_t (12);
    ASSERT_EQ (value.getUint16 (), 12);
    ASSERT_EQ (static_cast<uint16_t> (value), 12);

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint16_t> (value)), std::bad_cast);

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_EQ (value.getUint16 (), std::numeric_limits<uint32_t>::min ());
    ASSERT_EQ (static_cast<uint16_t> (value), std::numeric_limits<uint32_t>::min ());

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint16_t> (value)), std::bad_cast);

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint16_t> (value)), std::bad_cast);

    value = int64_t (12);
    ASSERT_EQ (value.getUint16 (), 12);
    ASSERT_EQ (static_cast<uint16_t> (value), 12);

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint16_t> (value)), std::bad_cast);

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_EQ (value.getUint16 (), std::numeric_limits<uint64_t>::min ());
    ASSERT_EQ (static_cast<uint16_t> (value), std::numeric_limits<uint64_t>::min ());

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint16_t> (value)), std::bad_cast);

    value = std::numeric_limits<float>::min ();
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint16_t> (value)), std::bad_cast);

    value = std::numeric_limits<float>::max ();
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint16_t> (value)), std::bad_cast);

    value = std::numeric_limits<double>::min ();
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint16_t> (value)), std::bad_cast);

    value = double (12.0);
    ASSERT_EQ (value.getUint16 (), 12);
    ASSERT_EQ (static_cast<uint16_t> (value), 12);

    value = std::numeric_limits<double>::max ();
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint16_t> (value)), std::bad_cast);

    value = "";
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint16_t> (value)), std::bad_cast);

    value = "foo";
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint16_t> (value)), std::bad_cast);

    value = "02:42:64:2f:6a:d0";
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint16_t> (value)), std::bad_cast);

    value = "127.0.0.1";
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint16_t> (value)), std::bad_cast);

    value = array.getArray ();
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint16_t> (value)), std::bad_cast);

    value = object.getObject ();
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint16_t> (value)), std::bad_cast);
}

/**
 * @brief Test getInt method.
 */
TEST (Value, getInt)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    int32_t tmp = 0;
    (void)tmp;

    Value value;
    ASSERT_THROW (value.getInt (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int32_t> (value)), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.getInt (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int32_t> (value)), std::bad_cast);

    value = true;
    ASSERT_THROW (value.getInt (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int32_t> (value)), std::bad_cast);

    value = false;
    ASSERT_THROW (value.getInt (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int32_t> (value)), std::bad_cast);

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_EQ (value.getInt (), std::numeric_limits<int8_t>::min ());
    ASSERT_EQ (static_cast<int32_t> (value), std::numeric_limits<int8_t>::min ());

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_EQ (value.getInt (), std::numeric_limits<int8_t>::max ());
    ASSERT_EQ (static_cast<int32_t> (value), std::numeric_limits<int8_t>::max ());

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_EQ (value.getInt (), std::numeric_limits<uint8_t>::min ());
    ASSERT_EQ (static_cast<int32_t> (value), std::numeric_limits<uint8_t>::min ());

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_EQ (value.getInt (), std::numeric_limits<uint8_t>::max ());
    ASSERT_EQ (static_cast<int32_t> (value), std::numeric_limits<uint8_t>::max ());

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_EQ (value.getInt (), std::numeric_limits<int16_t>::min ());
    ASSERT_EQ (static_cast<int32_t> (value), std::numeric_limits<int16_t>::min ());

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_EQ (value.getInt (), std::numeric_limits<int16_t>::max ());
    ASSERT_EQ (static_cast<int32_t> (value), std::numeric_limits<int16_t>::max ());

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_EQ (value.getInt (), std::numeric_limits<uint16_t>::min ());
    ASSERT_EQ (static_cast<int32_t> (value), std::numeric_limits<uint16_t>::min ());

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_EQ (value.getInt (), std::numeric_limits<uint16_t>::max ());
    ASSERT_EQ (static_cast<int32_t> (value), std::numeric_limits<uint16_t>::max ());

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_EQ (value.getInt (), std::numeric_limits<int32_t>::min ());
    ASSERT_EQ (static_cast<int32_t> (value), std::numeric_limits<int32_t>::min ());

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_EQ (value.getInt (), std::numeric_limits<int32_t>::max ());
    ASSERT_EQ (static_cast<int32_t> (value), std::numeric_limits<int32_t>::max ());

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_EQ (value.getInt (), std::numeric_limits<uint32_t>::min ());
    ASSERT_EQ (static_cast<int32_t> (value), std::numeric_limits<uint32_t>::min ());

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_THROW (value.getInt (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int32_t> (value)), std::bad_cast);

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_THROW (value.getInt (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int32_t> (value)), std::bad_cast);

    value = int64_t (12);
    ASSERT_EQ (value.getInt (), 12);
    ASSERT_EQ (static_cast<int32_t> (value), 12);

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_THROW (value.getInt (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int32_t> (value)), std::bad_cast);

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_EQ (value.getInt (), std::numeric_limits<uint64_t>::min ());
    ASSERT_EQ (static_cast<int32_t> (value), std::numeric_limits<uint64_t>::min ());

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_THROW (value.getInt (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int32_t> (value)), std::bad_cast);

    value = std::numeric_limits<float>::min ();
    ASSERT_THROW (value.getInt (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int32_t> (value)), std::bad_cast);

    value = std::numeric_limits<float>::max ();
    ASSERT_THROW (value.getInt (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int32_t> (value)), std::bad_cast);

    value = std::numeric_limits<double>::min ();
    ASSERT_THROW (value.getInt (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int32_t> (value)), std::bad_cast);

    value = double (12.0);
    ASSERT_EQ (value.getInt (), 12);
    ASSERT_EQ (static_cast<int32_t> (value), 12);

    value = std::numeric_limits<double>::max ();
    ASSERT_THROW (value.getInt (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int32_t> (value)), std::bad_cast);

    value = "";
    ASSERT_THROW (value.getInt (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int32_t> (value)), std::bad_cast);

    value = "foo";
    ASSERT_THROW (value.getInt (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int32_t> (value)), std::bad_cast);

    value = "02:42:64:2f:6a:d0";
    ASSERT_THROW (value.getInt (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int32_t> (value)), std::bad_cast);

    value = "127.0.0.1";
    ASSERT_THROW (value.getInt (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int32_t> (value)), std::bad_cast);

    value = array.getArray ();
    ASSERT_THROW (value.getInt (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int32_t> (value)), std::bad_cast);

    value = object.getObject ();
    ASSERT_THROW (value.getInt (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int32_t> (value)), std::bad_cast);
}

/**
 * @brief Test getUint method.
 */
TEST (Value, getUint)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    uint32_t tmp = 0;
    (void)tmp;

    Value value;
    ASSERT_THROW (value.getUint (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint32_t> (value)), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.getUint (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint32_t> (value)), std::bad_cast);

    value = true;
    ASSERT_THROW (value.getUint (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint32_t> (value)), std::bad_cast);

    value = false;
    ASSERT_THROW (value.getUint (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint32_t> (value)), std::bad_cast);

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_THROW (value.getUint (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint32_t> (value)), std::bad_cast);

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_EQ (value.getUint (), std::numeric_limits<int8_t>::max ());
    ASSERT_EQ (static_cast<uint32_t> (value), std::numeric_limits<int8_t>::max ());

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_EQ (value.getUint (), std::numeric_limits<uint8_t>::min ());
    ASSERT_EQ (static_cast<uint32_t> (value), std::numeric_limits<uint8_t>::min ());

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_EQ (value.getUint (), std::numeric_limits<uint8_t>::max ());
    ASSERT_EQ (static_cast<uint32_t> (value), std::numeric_limits<uint8_t>::max ());

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_THROW (value.getUint (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint32_t> (value)), std::bad_cast);

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_EQ (value.getUint (), std::numeric_limits<int16_t>::max ());
    ASSERT_EQ (static_cast<uint32_t> (value), std::numeric_limits<int16_t>::max ());

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_EQ (value.getUint (), std::numeric_limits<uint16_t>::min ());
    ASSERT_EQ (static_cast<uint32_t> (value), std::numeric_limits<uint16_t>::min ());

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_EQ (value.getUint (), std::numeric_limits<uint16_t>::max ());
    ASSERT_EQ (static_cast<uint32_t> (value), std::numeric_limits<uint16_t>::max ());

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_THROW (value.getUint (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint32_t> (value)), std::bad_cast);

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_EQ (value.getUint (), std::numeric_limits<int32_t>::max ());
    ASSERT_EQ (static_cast<uint32_t> (value), std::numeric_limits<int32_t>::max ());

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_EQ (value.getUint (), std::numeric_limits<uint32_t>::min ());
    ASSERT_EQ (static_cast<uint32_t> (value), std::numeric_limits<uint32_t>::min ());

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_EQ (value.getUint (), std::numeric_limits<uint32_t>::max ());
    ASSERT_EQ (static_cast<uint32_t> (value), std::numeric_limits<uint32_t>::max ());

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_THROW (value.getUint (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint32_t> (value)), std::bad_cast);

    value = int64_t (12);
    ASSERT_EQ (value.getUint (), 12);
    ASSERT_EQ (static_cast<uint32_t> (value), 12);

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_THROW (value.getUint (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint32_t> (value)), std::bad_cast);

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_EQ (value.getUint (), std::numeric_limits<uint64_t>::min ());
    ASSERT_EQ (static_cast<uint32_t> (value), std::numeric_limits<uint64_t>::min ());

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_THROW (value.getUint (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint32_t> (value)), std::bad_cast);

    value = std::numeric_limits<float>::min ();
    ASSERT_THROW (value.getUint (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint32_t> (value)), std::bad_cast);

    value = std::numeric_limits<float>::max ();
    ASSERT_THROW (value.getUint (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint32_t> (value)), std::bad_cast);

    value = std::numeric_limits<double>::min ();
    ASSERT_THROW (value.getUint (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint32_t> (value)), std::bad_cast);

    value = double (12.0);
    ASSERT_EQ (value.getUint (), 12);
    ASSERT_EQ (static_cast<uint32_t> (value), 12);

    value = std::numeric_limits<double>::max ();
    ASSERT_THROW (value.getUint (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint32_t> (value)), std::bad_cast);

    value = "";
    ASSERT_THROW (value.getUint (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint32_t> (value)), std::bad_cast);

    value = "foo";
    ASSERT_THROW (value.getUint (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint32_t> (value)), std::bad_cast);

    value = "02:42:64:2f:6a:d0";
    ASSERT_THROW (value.getUint (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint32_t> (value)), std::bad_cast);

    value = "127.0.0.1";
    ASSERT_THROW (value.getUint (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint32_t> (value)), std::bad_cast);

    value = array.getArray ();
    ASSERT_THROW (value.getUint (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint32_t> (value)), std::bad_cast);

    value = object.getObject ();
    ASSERT_THROW (value.getUint (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint32_t> (value)), std::bad_cast);
}

/**
 * @brief Test getInt64 method.
 */
TEST (Value, getInt64)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    int64_t tmp = 0;
    (void)tmp;

    Value value;
    ASSERT_THROW (value.getInt64 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int64_t> (value)), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.getInt64 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int64_t> (value)), std::bad_cast);

    value = true;
    ASSERT_THROW (value.getInt64 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int64_t> (value)), std::bad_cast);

    value = false;
    ASSERT_THROW (value.getInt64 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int64_t> (value)), std::bad_cast);

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_EQ (value.getInt64 (), std::numeric_limits<int8_t>::min ());
    ASSERT_EQ (static_cast<int64_t> (value), std::numeric_limits<int8_t>::min ());

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_EQ (value.getInt64 (), std::numeric_limits<int8_t>::max ());
    ASSERT_EQ (static_cast<int64_t> (value), std::numeric_limits<int8_t>::max ());

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_EQ (value.getInt64 (), std::numeric_limits<uint8_t>::min ());
    ASSERT_EQ (static_cast<int64_t> (value), std::numeric_limits<uint8_t>::min ());

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_EQ (value.getInt64 (), std::numeric_limits<uint8_t>::max ());
    ASSERT_EQ (static_cast<int64_t> (value), std::numeric_limits<uint8_t>::max ());

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_EQ (value.getInt64 (), std::numeric_limits<int16_t>::min ());
    ASSERT_EQ (static_cast<int64_t> (value), std::numeric_limits<int16_t>::min ());

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_EQ (value.getInt64 (), std::numeric_limits<int16_t>::max ());
    ASSERT_EQ (static_cast<int64_t> (value), std::numeric_limits<int16_t>::max ());

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_EQ (value.getInt64 (), std::numeric_limits<uint16_t>::min ());
    ASSERT_EQ (static_cast<int64_t> (value), std::numeric_limits<uint16_t>::min ());

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_EQ (value.getInt64 (), std::numeric_limits<uint16_t>::max ());
    ASSERT_EQ (static_cast<int64_t> (value), std::numeric_limits<uint16_t>::max ());

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_EQ (value.getInt64 (), std::numeric_limits<int32_t>::min ());
    ASSERT_EQ (static_cast<int64_t> (value), std::numeric_limits<int32_t>::min ());

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_EQ (value.getInt64 (), std::numeric_limits<int32_t>::max ());
    ASSERT_EQ (static_cast<int64_t> (value), std::numeric_limits<int32_t>::max ());

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_EQ (value.getInt64 (), std::numeric_limits<uint32_t>::min ());
    ASSERT_EQ (static_cast<int64_t> (value), std::numeric_limits<uint32_t>::min ());

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_EQ (value.getInt64 (), std::numeric_limits<uint32_t>::max ());
    ASSERT_EQ (static_cast<int64_t> (value), std::numeric_limits<uint32_t>::max ());

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_EQ (value.getInt64 (), std::numeric_limits<int64_t>::min ());
    ASSERT_EQ (static_cast<int64_t> (value), std::numeric_limits<int64_t>::min ());

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_EQ (value.getInt64 (), std::numeric_limits<int64_t>::max ());
    ASSERT_EQ (static_cast<int64_t> (value), std::numeric_limits<int64_t>::max ());

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_EQ (value.getInt64 (), std::numeric_limits<uint64_t>::min ());
    ASSERT_EQ (static_cast<int64_t> (value), std::numeric_limits<uint64_t>::min ());

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_THROW (value.getInt64 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int64_t> (value)), std::bad_cast);

    value = std::numeric_limits<float>::min ();
    ASSERT_THROW (value.getInt64 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int64_t> (value)), std::bad_cast);

    value = std::numeric_limits<float>::max ();
    ASSERT_THROW (value.getInt64 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int64_t> (value)), std::bad_cast);

    value = std::numeric_limits<double>::min ();
    ASSERT_THROW (value.getInt64 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int64_t> (value)), std::bad_cast);

    value = double (12.0);
    ASSERT_EQ (value.getInt64 (), 12);
    ASSERT_EQ (static_cast<int64_t> (value), 12);

    value = std::numeric_limits<double>::max ();
    ASSERT_THROW (value.getInt64 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int64_t> (value)), std::bad_cast);

    value = "";
    ASSERT_THROW (value.getInt64 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int64_t> (value)), std::bad_cast);

    value = "foo";
    ASSERT_THROW (value.getInt64 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int64_t> (value)), std::bad_cast);

    value = "02:42:64:2f:6a:d0";
    ASSERT_THROW (value.getInt64 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int64_t> (value)), std::bad_cast);

    value = "127.0.0.1";
    ASSERT_THROW (value.getInt64 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int64_t> (value)), std::bad_cast);

    value = array.getArray ();
    ASSERT_THROW (value.getInt64 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int64_t> (value)), std::bad_cast);

    value = object.getObject ();
    ASSERT_THROW (value.getInt64 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<int64_t> (value)), std::bad_cast);
}

/**
 * @brief Test getUint64 method.
 */
TEST (Value, getUint64)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    uint64_t tmp = 0;
    (void)tmp;

    Value value;
    ASSERT_THROW (value.getUint64 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint64_t> (value)), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.getUint64 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint64_t> (value)), std::bad_cast);

    value = true;
    ASSERT_THROW (value.getUint64 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint64_t> (value)), std::bad_cast);

    value = false;
    ASSERT_THROW (value.getUint64 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint64_t> (value)), std::bad_cast);

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_THROW (value.getUint64 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint64_t> (value)), std::bad_cast);

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_EQ (value.getUint64 (), std::numeric_limits<int8_t>::max ());
    ASSERT_EQ (static_cast<uint64_t> (value), std::numeric_limits<int8_t>::max ());

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_EQ (value.getUint64 (), std::numeric_limits<uint8_t>::min ());
    ASSERT_EQ (static_cast<uint64_t> (value), std::numeric_limits<uint8_t>::min ());

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_EQ (value.getUint64 (), std::numeric_limits<uint8_t>::max ());
    ASSERT_EQ (static_cast<uint64_t> (value), std::numeric_limits<uint8_t>::max ());

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_THROW (value.getUint64 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint64_t> (value)), std::bad_cast);

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_EQ (value.getUint64 (), std::numeric_limits<int16_t>::max ());
    ASSERT_EQ (static_cast<uint64_t> (value), std::numeric_limits<int16_t>::max ());

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_EQ (value.getUint64 (), std::numeric_limits<uint16_t>::min ());
    ASSERT_EQ (static_cast<uint64_t> (value), std::numeric_limits<uint16_t>::min ());

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_EQ (value.getUint64 (), std::numeric_limits<uint16_t>::max ());
    ASSERT_EQ (static_cast<uint64_t> (value), std::numeric_limits<uint16_t>::max ());

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_THROW (value.getUint64 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint64_t> (value)), std::bad_cast);

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_EQ (value.getUint64 (), std::numeric_limits<int32_t>::max ());
    ASSERT_EQ (static_cast<uint64_t> (value), std::numeric_limits<int32_t>::max ());

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_EQ (value.getUint64 (), std::numeric_limits<uint32_t>::min ());
    ASSERT_EQ (static_cast<uint64_t> (value), std::numeric_limits<uint32_t>::min ());

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_EQ (value.getUint64 (), std::numeric_limits<uint32_t>::max ());
    ASSERT_EQ (static_cast<uint64_t> (value), std::numeric_limits<uint32_t>::max ());

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_THROW (value.getUint64 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint64_t> (value)), std::bad_cast);

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_EQ (value.getUint64 (), std::numeric_limits<int64_t>::max ());
    ASSERT_EQ (static_cast<uint64_t> (value), std::numeric_limits<int64_t>::max ());

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_EQ (value.getUint64 (), std::numeric_limits<uint64_t>::min ());
    ASSERT_EQ (static_cast<uint64_t> (value), std::numeric_limits<uint64_t>::min ());

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_EQ (value.getUint64 (), std::numeric_limits<uint64_t>::max ());
    ASSERT_EQ (static_cast<uint64_t> (value), std::numeric_limits<uint64_t>::max ());

    value = std::numeric_limits<float>::min ();
    ASSERT_THROW (value.getUint64 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint64_t> (value)), std::bad_cast);

    value = std::numeric_limits<float>::max ();
    ASSERT_THROW (value.getUint64 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint64_t> (value)), std::bad_cast);

    value = std::numeric_limits<double>::min ();
    ASSERT_THROW (value.getUint64 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint64_t> (value)), std::bad_cast);

    value = double (12.0);
    ASSERT_EQ (value.getUint64 (), 12);
    ASSERT_EQ (static_cast<uint64_t> (value), 12);

    value = std::numeric_limits<double>::max ();
    ASSERT_THROW (value.getUint64 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint64_t> (value)), std::bad_cast);

    value = "";
    ASSERT_THROW (value.getUint64 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint64_t> (value)), std::bad_cast);

    value = "foo";
    ASSERT_THROW (value.getUint64 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint64_t> (value)), std::bad_cast);

    value = "02:42:64:2f:6a:d0";
    ASSERT_THROW (value.getUint64 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint64_t> (value)), std::bad_cast);

    value = "127.0.0.1";
    ASSERT_THROW (value.getUint64 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint64_t> (value)), std::bad_cast);

    value = array.getArray ();
    ASSERT_THROW (value.getUint64 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint64_t> (value)), std::bad_cast);

    value = object.getObject ();
    ASSERT_THROW (value.getUint64 (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<uint64_t> (value)), std::bad_cast);
}

/**
 * @brief Test getFloat method.
 */
TEST (Value, getFloat)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    float tmp = 0.0f;
    (void)tmp;

    Value value;
    ASSERT_THROW (value.getFloat (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<float> (value)), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.getFloat (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<float> (value)), std::bad_cast);

    value = true;
    ASSERT_THROW (value.getFloat (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<float> (value)), std::bad_cast);

    value = false;
    ASSERT_THROW (value.getFloat (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<float> (value)), std::bad_cast);

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_EQ (value.getFloat (), std::numeric_limits<int8_t>::min ());
    ASSERT_EQ (static_cast<float> (value), std::numeric_limits<int8_t>::min ());

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_EQ (value.getFloat (), std::numeric_limits<int8_t>::max ());
    ASSERT_EQ (static_cast<float> (value), std::numeric_limits<int8_t>::max ());

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_EQ (value.getFloat (), std::numeric_limits<uint8_t>::min ());
    ASSERT_EQ (static_cast<float> (value), std::numeric_limits<uint8_t>::min ());

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_EQ (value.getFloat (), std::numeric_limits<uint8_t>::max ());
    ASSERT_EQ (static_cast<float> (value), std::numeric_limits<uint8_t>::max ());

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_EQ (value.getFloat (), std::numeric_limits<int16_t>::min ());
    ASSERT_EQ (static_cast<float> (value), std::numeric_limits<int16_t>::min ());

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_EQ (value.getFloat (), std::numeric_limits<int16_t>::max ());
    ASSERT_EQ (static_cast<float> (value), std::numeric_limits<int16_t>::max ());

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_EQ (value.getFloat (), std::numeric_limits<uint16_t>::min ());
    ASSERT_EQ (static_cast<float> (value), std::numeric_limits<uint16_t>::min ());

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_EQ (value.getFloat (), std::numeric_limits<uint16_t>::max ());
    ASSERT_EQ (static_cast<float> (value), std::numeric_limits<uint16_t>::max ());

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_EQ (value.getFloat (), std::numeric_limits<int32_t>::min ());
    ASSERT_EQ (static_cast<float> (value), std::numeric_limits<int32_t>::min ());

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_EQ (value.getFloat (), std::numeric_limits<int32_t>::max ());
    ASSERT_EQ (static_cast<float> (value), std::numeric_limits<int32_t>::max ());

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_EQ (value.getFloat (), std::numeric_limits<uint32_t>::min ());
    ASSERT_EQ (static_cast<float> (value), std::numeric_limits<uint32_t>::min ());

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_EQ (value.getFloat (), std::numeric_limits<uint32_t>::max ());
    ASSERT_EQ (static_cast<float> (value), std::numeric_limits<uint32_t>::max ());

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_EQ (value.getFloat (), std::numeric_limits<int64_t>::min ());
    ASSERT_EQ (static_cast<float> (value), std::numeric_limits<int64_t>::min ());

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_EQ (value.getFloat (), std::numeric_limits<int64_t>::max ());
    ASSERT_EQ (static_cast<float> (value), std::numeric_limits<int64_t>::max ());

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_EQ (value.getFloat (), std::numeric_limits<uint64_t>::min ());
    ASSERT_EQ (static_cast<float> (value), std::numeric_limits<uint64_t>::min ());

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_EQ (value.getFloat (), std::numeric_limits<uint64_t>::max ());
    ASSERT_EQ (static_cast<float> (value), std::numeric_limits<uint64_t>::max ());

    value = std::numeric_limits<float>::min ();
    ASSERT_EQ (value.getFloat (), std::numeric_limits<float>::min ());
    ASSERT_EQ (static_cast<float> (value), std::numeric_limits<float>::min ());

    value = std::numeric_limits<float>::max ();
    ASSERT_EQ (value.getFloat (), std::numeric_limits<float>::max ());
    ASSERT_EQ (static_cast<float> (value), std::numeric_limits<float>::max ());

    value = std::numeric_limits<double>::min ();
    ASSERT_EQ (value.getFloat (), static_cast<float> (std::numeric_limits<double>::min ()));
    ASSERT_EQ (static_cast<float> (value), static_cast<float> (std::numeric_limits<double>::min ()));

    value = std::numeric_limits<double>::max ();
    ASSERT_EQ (value.getFloat (), static_cast<float> (std::numeric_limits<double>::max ()));
    ASSERT_EQ (static_cast<float> (value), static_cast<float> (std::numeric_limits<double>::max ()));

    value = "";
    ASSERT_THROW (value.getFloat (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<float> (value)), std::bad_cast);

    value = "foo";
    ASSERT_THROW (value.getFloat (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<float> (value)), std::bad_cast);

    value = "02:42:64:2f:6a:d0";
    ASSERT_THROW (value.getFloat (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<float> (value)), std::bad_cast);

    value = "127.0.0.1";
    ASSERT_THROW (value.getFloat (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<float> (value)), std::bad_cast);

    value = array.getArray ();
    ASSERT_THROW (value.getFloat (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<float> (value)), std::bad_cast);

    value = object.getObject ();
    ASSERT_THROW (value.getFloat (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<float> (value)), std::bad_cast);
}

/**
 * @brief Test getDouble method.
 */
TEST (Value, getDouble)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    double tmp = 0.0;
    (void)tmp;

    Value value;
    ASSERT_THROW (value.getDouble (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<double> (value)), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.getDouble (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<double> (value)), std::bad_cast);

    value = true;
    ASSERT_THROW (value.getDouble (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<double> (value)), std::bad_cast);

    value = false;
    ASSERT_THROW (value.getDouble (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<double> (value)), std::bad_cast);

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_EQ (value.getDouble (), std::numeric_limits<int8_t>::min ());
    ASSERT_EQ (static_cast<double> (value), std::numeric_limits<int8_t>::min ());

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_EQ (value.getDouble (), std::numeric_limits<int8_t>::max ());
    ASSERT_EQ (static_cast<double> (value), std::numeric_limits<int8_t>::max ());

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_EQ (value.getDouble (), std::numeric_limits<uint8_t>::min ());
    ASSERT_EQ (static_cast<double> (value), std::numeric_limits<uint8_t>::min ());

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_EQ (value.getDouble (), std::numeric_limits<uint8_t>::max ());
    ASSERT_EQ (static_cast<double> (value), std::numeric_limits<uint8_t>::max ());

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_EQ (value.getDouble (), std::numeric_limits<int16_t>::min ());
    ASSERT_EQ (static_cast<double> (value), std::numeric_limits<int16_t>::min ());

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_EQ (value.getDouble (), std::numeric_limits<int16_t>::max ());
    ASSERT_EQ (static_cast<double> (value), std::numeric_limits<int16_t>::max ());

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_EQ (value.getDouble (), std::numeric_limits<uint16_t>::min ());
    ASSERT_EQ (static_cast<double> (value), std::numeric_limits<uint16_t>::min ());

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_EQ (value.getDouble (), std::numeric_limits<uint16_t>::max ());
    ASSERT_EQ (static_cast<double> (value), std::numeric_limits<uint16_t>::max ());

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_EQ (value.getDouble (), std::numeric_limits<int32_t>::min ());
    ASSERT_EQ (static_cast<double> (value), std::numeric_limits<int32_t>::min ());

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_EQ (value.getDouble (), std::numeric_limits<int32_t>::max ());
    ASSERT_EQ (static_cast<double> (value), std::numeric_limits<int32_t>::max ());

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_EQ (value.getDouble (), std::numeric_limits<uint32_t>::min ());
    ASSERT_EQ (static_cast<double> (value), std::numeric_limits<uint32_t>::min ());

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_EQ (value.getDouble (), std::numeric_limits<uint32_t>::max ());
    ASSERT_EQ (static_cast<double> (value), std::numeric_limits<uint32_t>::max ());

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_EQ (value.getDouble (), std::numeric_limits<int64_t>::min ());
    ASSERT_EQ (static_cast<double> (value), std::numeric_limits<int64_t>::min ());

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_EQ (value.getDouble (), std::numeric_limits<int64_t>::max ());
    ASSERT_EQ (static_cast<double> (value), std::numeric_limits<int64_t>::max ());

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_EQ (value.getDouble (), std::numeric_limits<uint64_t>::min ());
    ASSERT_EQ (static_cast<double> (value), std::numeric_limits<uint64_t>::min ());

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_EQ (value.getDouble (), std::numeric_limits<uint64_t>::max ());
    ASSERT_EQ (static_cast<double> (value), std::numeric_limits<uint64_t>::max ());

    value = std::numeric_limits<float>::min ();
    ASSERT_EQ (value.getDouble (), std::numeric_limits<float>::min ());
    ASSERT_EQ (static_cast<double> (value), std::numeric_limits<float>::min ());

    value = std::numeric_limits<float>::max ();
    ASSERT_EQ (value.getDouble (), std::numeric_limits<float>::max ());
    ASSERT_EQ (static_cast<double> (value), std::numeric_limits<float>::max ());

    value = std::numeric_limits<double>::min ();
    ASSERT_EQ (value.getDouble (), std::numeric_limits<double>::min ());
    ASSERT_EQ (static_cast<double> (value), std::numeric_limits<double>::min ());

    value = std::numeric_limits<double>::max ();
    ASSERT_EQ (value.getDouble (), std::numeric_limits<double>::max ());
    ASSERT_EQ (static_cast<double> (value), std::numeric_limits<double>::max ());

    value = "";
    ASSERT_THROW (value.getDouble (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<double> (value)), std::bad_cast);

    value = "foo";
    ASSERT_THROW (value.getDouble (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<double> (value)), std::bad_cast);

    value = "02:42:64:2f:6a:d0";
    ASSERT_THROW (value.getDouble (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<double> (value)), std::bad_cast);

    value = "127.0.0.1";
    ASSERT_THROW (value.getDouble (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<double> (value)), std::bad_cast);

    value = array.getArray ();
    ASSERT_THROW (value.getDouble (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<double> (value)), std::bad_cast);

    value = object.getObject ();
    ASSERT_THROW (value.getDouble (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<double> (value)), std::bad_cast);
}

/**
 * @brief Test getString method.
 */
TEST (Value, getString)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    std::string_view tmp;
    (void)tmp;

    Value value;
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<std::string_view> (value)), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<std::string_view> (value)), std::bad_cast);

    value = true;
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<std::string_view> (value)), std::bad_cast);

    value = false;
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<std::string_view> (value)), std::bad_cast);

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<std::string_view> (value)), std::bad_cast);

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<std::string_view> (value)), std::bad_cast);

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<std::string_view> (value)), std::bad_cast);

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<std::string_view> (value)), std::bad_cast);

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<std::string_view> (value)), std::bad_cast);

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<std::string_view> (value)), std::bad_cast);

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<std::string_view> (value)), std::bad_cast);

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<std::string_view> (value)), std::bad_cast);

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<std::string_view> (value)), std::bad_cast);

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<std::string_view> (value)), std::bad_cast);

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<std::string_view> (value)), std::bad_cast);

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<std::string_view> (value)), std::bad_cast);

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<std::string_view> (value)), std::bad_cast);

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<std::string_view> (value)), std::bad_cast);

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<std::string_view> (value)), std::bad_cast);

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<std::string_view> (value)), std::bad_cast);

    value = std::numeric_limits<float>::min ();
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<std::string_view> (value)), std::bad_cast);

    value = std::numeric_limits<float>::max ();
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<std::string_view> (value)), std::bad_cast);

    value = std::numeric_limits<double>::min ();
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<std::string_view> (value)), std::bad_cast);

    value = std::numeric_limits<double>::max ();
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<std::string_view> (value)), std::bad_cast);

    value = "";
    ASSERT_EQ (value.getString (), "");
    ASSERT_EQ (static_cast<std::string_view> (value), "");

    value = "foo";
    ASSERT_EQ (value.getString (), "foo");
    ASSERT_EQ (static_cast<std::string_view> (value), "foo");

    value = "02:42:64:2f:6a:d0";
    ASSERT_EQ (value.getString (), "02:42:64:2f:6a:d0");
    ASSERT_EQ (static_cast<std::string_view> (value), "02:42:64:2f:6a:d0");

    value = "127.0.0.1";
    ASSERT_EQ (value.getString (), "127.0.0.1");
    ASSERT_EQ (static_cast<std::string_view> (value), "127.0.0.1");

    value = array.getArray ();
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<std::string_view> (value)), std::bad_cast);

    value = object.getObject ();
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW ((tmp = static_cast<std::string_view> (value)), std::bad_cast);
}

/**
 * @brief Test getBinary method.
 */
TEST (Value, getBinary)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    const uint8_t bytes[] = {0x00, 0x01, 0xff};

    Value value;
    ASSERT_THROW (value.getBinary (), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.getBinary (), std::bad_cast);

    value = true;
    ASSERT_THROW (value.getBinary (), std::bad_cast);

    value = false;
    ASSERT_THROW (value.getBinary (), std::bad_cast);

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_THROW (value.getBinary (), std::bad_cast);

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_THROW (value.getBinary (), std::bad_cast);

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_THROW (value.getBinary (), std::bad_cast);

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_THROW (value.getBinary (), std::bad_cast);

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_THROW (value.getBinary (), std::bad_cast);

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_THROW (value.getBinary (), std::bad_cast);

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_THROW (value.getBinary (), std::bad_cast);

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_THROW (value.getBinary (), std::bad_cast);

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_THROW (value.getBinary (), std::bad_cast);

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_THROW (value.getBinary (), std::bad_cast);

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_THROW (value.getBinary (), std::bad_cast);

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_THROW (value.getBinary (), std::bad_cast);

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_THROW (value.getBinary (), std::bad_cast);

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_THROW (value.getBinary (), std::bad_cast);

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_THROW (value.getBinary (), std::bad_cast);

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_THROW (value.getBinary (), std::bad_cast);

    value = std::numeric_limits<float>::min ();
    ASSERT_THROW (value.getBinary (), std::bad_cast);

    value = std::numeric_limits<float>::max ();
    ASSERT_THROW (value.getBinary (), std::bad_cast);

    value = std::numeric_limits<double>::min ();
    ASSERT_THROW (value.getBinary (), std::bad_cast);

    value = std::numeric_limits<double>::max ();
    ASSERT_THROW (value.getBinary (), std::bad_cast);

    value = "";
    ASSERT_THROW (value.getBinary (), std::bad_cast);

    value = "foo";
    ASSERT_THROW (value.getBinary (), std::bad_cast);

    value = "02:42:64:2f:6a:d0";
    ASSERT_THROW (value.getBinary (), std::bad_cast);

    value = "127.0.0.1";
    ASSERT_THROW (value.getBinary (), std::bad_cast);

    value = std::span<const uint8_t> (bytes, sizeof (bytes));
    ASSERT_TRUE (std::ranges::equal (value.getBinary (), std::span<const uint8_t> (bytes, sizeof (bytes))));

    value = array.getArray ();
    ASSERT_THROW (value.getBinary (), std::bad_cast);

    value = object.getObject ();
    ASSERT_THROW (value.getBinary (), std::bad_cast);
}

/**
 * @brief Test getSubtype method.
 */
TEST (Value, getSubtype)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    const uint8_t bytes[] = {0x00, 0x01, 0xff};

    Value value;
    ASSERT_THROW (value.getSubtype (), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.getSubtype (), std::bad_cast);

    value = true;
    ASSERT_THROW (value.getSubtype (), std::bad_cast);

    value = false;
    ASSERT_THROW (value.getSubtype (), std::bad_cast);

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_THROW (value.getSubtype (), std::bad_cast);

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_THROW (value.getSubtype (), std::bad_cast);

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_THROW (value.getSubtype (), std::bad_cast);

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_THROW (value.getSubtype (), std::bad_cast);

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_THROW (value.getSubtype (), std::bad_cast);

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_THROW (value.getSubtype (), std::bad_cast);

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_THROW (value.getSubtype (), std::bad_cast);

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_THROW (value.getSubtype (), std::bad_cast);

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_THROW (value.getSubtype (), std::bad_cast);

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_THROW (value.getSubtype (), std::bad_cast);

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_THROW (value.getSubtype (), std::bad_cast);

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_THROW (value.getSubtype (), std::bad_cast);

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_THROW (value.getSubtype (), std::bad_cast);

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_THROW (value.getSubtype (), std::bad_cast);

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_THROW (value.getSubtype (), std::bad_cast);

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_THROW (value.getSubtype (), std::bad_cast);

    value = std::numeric_limits<float>::min ();
    ASSERT_THROW (value.getSubtype (), std::bad_cast);

    value = std::numeric_limits<float>::max ();
    ASSERT_THROW (value.getSubtype (), std::bad_cast);

    value = std::numeric_limits<double>::min ();
    ASSERT_THROW (value.getSubtype (), std::bad_cast);

    value = std::numeric_limits<double>::max ();
    ASSERT_THROW (value.getSubtype (), std::bad_cast);

    value = "";
    ASSERT_THROW (value.getSubtype (), std::bad_cast);

    value = "foo";
    ASSERT_THROW (value.getSubtype (), std::bad_cast);

    value = "02:42:64:2f:6a:d0";
    ASSERT_THROW (value.getSubtype (), std::bad_cast);

    value = "127.0.0.1";
    ASSERT_THROW (value.getSubtype (), std::bad_cast);

    value = std::span<const uint8_t> (bytes, sizeof (bytes));
    ASSERT_EQ (value.getSubtype (), std::nullopt);

    value = Value (std::span<const uint8_t> (bytes, sizeof (bytes)), -1);
    ASSERT_EQ (value.getSubtype (), -1);

    value = array.getArray ();
    ASSERT_THROW (value.getSubtype (), std::bad_cast);

    value = object.getObject ();
    ASSERT_THROW (value.getSubtype (), std::bad_cast);
}

/**
 * @brief Test getArray method.
 */
TEST (Value, getArray)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    Value value;
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = true;
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = false;
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = std::numeric_limits<float>::min ();
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = std::numeric_limits<float>::max ();
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = std::numeric_limits<double>::min ();
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = std::numeric_limits<double>::max ();
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = "";
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = "foo";
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = "02:42:64:2f:6a:d0";
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = "127.0.0.1";
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = array.getArray ();
    ASSERT_NO_THROW (value.getArray ());

    value = object.getObject ();
    ASSERT_THROW (value.getArray (), std::bad_cast);
}

/**
 * @brief Test getObject method.
 */
TEST (Value, getObject)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    Value value;
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = true;
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = false;
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = std::numeric_limits<float>::min ();
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = std::numeric_limits<float>::max ();
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = std::numeric_limits<double>::min ();
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = std::numeric_limits<double>::max ();
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = "";
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = "foo";
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = "02:42:64:2f:6a:d0";
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = "127.0.0.1";
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = array.getArray ();
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = object.getObject ();
    ASSERT_NO_THROW (value.getObject ());
}

/**
 * @brief Test index method.
 */
TEST (Value, index)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    Value value;

    value = nullptr;
    ASSERT_EQ (value.index (), static_cast<size_t> (Value::Index::NullValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::BooleanValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::IntegerValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::UnsignedValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::Integer64Value));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::Unsigned64Value));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::RealValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::StringValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::ArrayValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::ObjectValue));

    value = bool (true);
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::NullValue));
    ASSERT_EQ (value.index (), static_cast<size_t> (Value::Index::BooleanValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::IntegerValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::UnsignedValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::Integer64Value));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::Unsigned64Value));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::RealValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::StringValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::ArrayValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::ObjectValue));

    value = int32_t (std::numeric_limits<int32_t>::min ());
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::NullValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::BooleanValue));
    ASSERT_EQ (value.index (), static_cast<size_t> (Value::Index::IntegerValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::UnsignedValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::Integer64Value));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::Unsigned64Value));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::RealValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::StringValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::ArrayValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::ObjectValue));

    value = uint32_t (std::numeric_limits<uint32_t>::max ());
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::NullValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::BooleanValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::IntegerValue));
    ASSERT_EQ (value.index (), static_cast<size_t> (Value::Index::UnsignedValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::Integer64Value));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::Unsigned64Value));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::RealValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::StringValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::ArrayValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::ObjectValue));

    value = int64_t (std::numeric_limits<int64_t>::min ());
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::NullValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::BooleanValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::IntegerValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::UnsignedValue));
    ASSERT_EQ (value.index (), static_cast<size_t> (Value::Index::Integer64Value));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::Unsigned64Value));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::RealValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::StringValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::ArrayValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::ObjectValue));

    value = uint64_t (std::numeric_limits<uint64_t>::max ());
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::NullValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::BooleanValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::IntegerValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::UnsignedValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::Integer64Value));
    ASSERT_EQ (value.index (), static_cast<size_t> (Value::Index::Unsigned64Value));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::RealValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::StringValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::ArrayValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::ObjectValue));

    value = double (std::numeric_limits<double>::min ());
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::NullValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::BooleanValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::IntegerValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::UnsignedValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::Integer64Value));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::Unsigned64Value));
    ASSERT_EQ (value.index (), static_cast<size_t> (Value::Index::RealValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::StringValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::ArrayValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::ObjectValue));

    value = std::string ("foobar");
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::NullValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::BooleanValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::IntegerValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::UnsignedValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::Integer64Value));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::Unsigned64Value));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::RealValue));
    ASSERT_EQ (value.index (), static_cast<size_t> (Value::Index::StringValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::ArrayValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::ObjectValue));

    value = array.getArray ();
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::NullValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::BooleanValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::IntegerValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::UnsignedValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::Integer64Value));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::Unsigned64Value));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::RealValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::StringValue));
    ASSERT_EQ (value.index (), static_cast<size_t> (Value::Index::ArrayValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::ObjectValue));

    value = object.getObject ();
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::NullValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::BooleanValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::IntegerValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::UnsignedValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::Integer64Value));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::Unsigned64Value));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::RealValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::StringValue));
    ASSERT_NE (value.index (), static_cast<size_t> (Value::Index::ArrayValue));
    ASSERT_EQ (value.index (), static_cast<size_t> (Value::Index::ObjectValue));
}

/**
 * @brief Test at method.
 */
TEST (Value, at)
{
    Value value = Array ();

    value.pushBack (nullptr);
    ASSERT_TRUE (value.at (0).isNull ());

    value.pushBack (true);
    ASSERT_TRUE (value.at (1).isBool ());

    value.pushBack (std::numeric_limits<int64_t>::max ());
    ASSERT_TRUE (value.at (2).isInt64 ());

    value.pushBack (std::numeric_limits<double>::max ());
    ASSERT_TRUE (value.at (3).isDouble ());

    value.pushBack ("foobar");
    ASSERT_TRUE (value.at (4).isString ());

    value.pushBack (Array ());
    ASSERT_TRUE (value.at (5).isArray ());

    value.pushBack (Object ());
    ASSERT_TRUE (value.at (6).isObject ());

    ASSERT_THROW (value.at (7), std::out_of_range);

    value = Object ();

    value["null"] = nullptr;
    ASSERT_TRUE (value.at ("null").isNull ());

    value["boolean"] = true;
    ASSERT_TRUE (value.at ("boolean").isBool ());

    value["integer"] = std::numeric_limits<int64_t>::max ();
    ASSERT_TRUE (value.at ("integer").isInt64 ());

    value["double"] = std::numeric_limits<double>::max ();
    ASSERT_TRUE (value.at ("double").isDouble ());

    value["string"] = "foobar";
    ASSERT_TRUE (value.at ("string").isString ());

    value["array"] = Array ();
    ASSERT_TRUE (value.at ("array").isArray ());

    value["object"] = Object ();
    ASSERT_TRUE (value.at ("object").isObject ());

    ASSERT_THROW (value.at ("non existing path"), std::out_of_range);

    value = nullptr;
    value[nullptr] = 0;
    value[true] = 1;
    value[int32_t (-2)] = 2;
    value[uint32_t (3)] = 3;
    value[int64_t (-4)] = 4;
    value[uint64_t (5)] = 5;
    value[6.5] = 6;
    value["seven"] = 7;
    const Value& constant = value;
    ASSERT_EQ (value.at (nullptr), 0);
    ASSERT_EQ (constant.at (nullptr), 0);
    ASSERT_EQ (value.at (true), 1);
    ASSERT_EQ (constant.at (true), 1);
    ASSERT_EQ (value.at (int32_t (-2)), 2);
    ASSERT_EQ (constant.at (int32_t (-2)), 2);
    ASSERT_EQ (value.at (uint32_t (3)), 3);
    ASSERT_EQ (constant.at (uint32_t (3)), 3);
    ASSERT_EQ (value.at (int64_t (-4)), 4);
    ASSERT_EQ (constant.at (int64_t (-4)), 4);
    ASSERT_EQ (value.at (uint64_t (5)), 5);
    ASSERT_EQ (constant.at (uint64_t (5)), 5);
    ASSERT_EQ (value.at (6.5), 6);
    ASSERT_EQ (constant.at (6.5), 6);
    ASSERT_EQ (value.at (std::string_view ("seven")), 7);
    ASSERT_EQ (constant.at (std::string_view ("seven")), 7);
    ASSERT_EQ (value.at ("seven"), 7);
    ASSERT_EQ (constant.at ("seven"), 7);
    ASSERT_EQ (value.at (std::string ("seven")), 7);
    ASSERT_EQ (constant.at (std::string ("seven")), 7);
    ASSERT_THROW (value.at (8), std::out_of_range);
    ASSERT_THROW (constant.at (8), std::out_of_range);
    ASSERT_THROW (constant.at ("eight"), std::out_of_range);

    value = nullptr;
    value["seven"] = 7;
    ASSERT_THROW (value.at (nullptr), std::out_of_range);
    ASSERT_THROW (constant.at (nullptr), std::out_of_range);
    ASSERT_THROW (value.at (true), std::out_of_range);
    ASSERT_THROW (constant.at (true), std::out_of_range);
    ASSERT_THROW (value.at (uint32_t (3)), std::out_of_range);
    ASSERT_THROW (constant.at (uint32_t (3)), std::out_of_range);
    ASSERT_THROW (value.at (int64_t (-4)), std::out_of_range);
    ASSERT_THROW (constant.at (int64_t (-4)), std::out_of_range);
    ASSERT_THROW (value.at (uint64_t (5)), std::out_of_range);
    ASSERT_THROW (constant.at (uint64_t (5)), std::out_of_range);
    ASSERT_THROW (value.at (6.5), std::out_of_range);
    ASSERT_THROW (constant.at (6.5), std::out_of_range);

    value = nullptr;
    value.pushBack (0);
    value.pushBack (1);
    value.pushBack (2);
    value.pushBack (3);
    ASSERT_EQ (value.at (int32_t (0)), 0);
    ASSERT_EQ (constant.at (int32_t (0)), 0);
    ASSERT_EQ (value.at (uint32_t (1)), 1);
    ASSERT_EQ (constant.at (uint32_t (1)), 1);
    ASSERT_EQ (value.at (int64_t (2)), 2);
    ASSERT_EQ (constant.at (int64_t (2)), 2);
    ASSERT_EQ (value.at (uint64_t (3)), 3);
    ASSERT_EQ (constant.at (uint64_t (3)), 3);
    ASSERT_THROW (value.at (-1), std::out_of_range);
    ASSERT_THROW (constant.at (-1), std::out_of_range);
    ASSERT_THROW (value.at (4), std::out_of_range);
    ASSERT_THROW (constant.at (4), std::out_of_range);
    ASSERT_THROW (value.at (nullptr), std::bad_cast);
    ASSERT_THROW (constant.at (nullptr), std::bad_cast);
    ASSERT_THROW (value.at (true), std::bad_cast);
    ASSERT_THROW (constant.at (true), std::bad_cast);
    ASSERT_THROW (value.at (1.0), std::bad_cast);
    ASSERT_THROW (constant.at (1.0), std::bad_cast);
    ASSERT_THROW (value.at ("seven"), std::bad_cast);
    ASSERT_THROW (constant.at ("seven"), std::bad_cast);
}

/**
 * @brief Test subscript operator.
 */
TEST (Value, subscript)
{
    Value value;
    value[nullptr] = 0;
    value[true] = 1;
    value[int32_t (-2)] = 2;
    value[uint32_t (3)] = 3;
    value[int64_t (-4)] = 4;
    value[uint64_t (5)] = 5;
    value[6.5] = 6;
    value[std::string_view ("seven")] = 7;
    value["eight"] = 8;
    value[std::string ("nine")] = 9;
    ASSERT_EQ (value.size (), 10U);
    ASSERT_EQ (value[nullptr], 0);
    ASSERT_EQ (value[true], 1);
    ASSERT_EQ (value[int32_t (-2)], 2);
    ASSERT_EQ (value[uint32_t (3)], 3);
    ASSERT_EQ (value[int64_t (-4)], 4);
    ASSERT_EQ (value[uint64_t (5)], 5);
    ASSERT_EQ (value[6.5], 6);
    ASSERT_EQ (value[std::string_view ("seven")], 7);
    ASSERT_EQ (value["eight"], 8);
    ASSERT_EQ (value[std::string ("nine")], 9);
    ASSERT_EQ (value.size (), 10U);

    value = nullptr;
    value[true] = 1;
    ASSERT_TRUE (value.isObject ());
    value = nullptr;
    value[int32_t (-2)] = 2;
    ASSERT_TRUE (value.isObject ());
    value = nullptr;
    value[uint32_t (3)] = 3;
    ASSERT_TRUE (value.isObject ());
    value = nullptr;
    value[int64_t (-4)] = 4;
    ASSERT_TRUE (value.isObject ());
    value = nullptr;
    value[uint64_t (5)] = 5;
    ASSERT_TRUE (value.isObject ());
    value = nullptr;
    value[6.5] = 6;
    ASSERT_TRUE (value.isObject ());

    value = nullptr;
    value.pushBack (0);
    value.pushBack (1);
    value.pushBack (2);
    value.pushBack (3);
    ASSERT_EQ (value[int32_t (0)], 0);
    ASSERT_EQ (value[uint32_t (1)], 1);
    ASSERT_EQ (value[int64_t (2)], 2);
    ASSERT_EQ (value[uint64_t (3)], 3);
    ASSERT_EQ (value[Value (uint64_t (3))], 3);
    ASSERT_THROW (value[nullptr], std::bad_cast);
    ASSERT_THROW (value[true], std::bad_cast);
    ASSERT_THROW (value[1.0], std::bad_cast);
    ASSERT_THROW (value["seven"], std::bad_cast);

    value = true;
    ASSERT_THROW (value[0], std::bad_cast);
    ASSERT_THROW (value["seven"], std::bad_cast);
}

/**
 * @brief Test assign method.
 */
TEST (Value, assign)
{
    Value value;

    value["null"] = nullptr;
    ASSERT_TRUE (value["null"].isNull ());

    value["boolean"] = true;
    ASSERT_TRUE (value["boolean"].isBool ());

    value["integer"] = std::numeric_limits<int64_t>::max ();
    ASSERT_TRUE (value["integer"].isInt64 ());

    value["double"] = std::numeric_limits<double>::max ();
    ASSERT_TRUE (value["double"].isDouble ());

    value["string"] = "foobar";
    ASSERT_TRUE (value["string"].isString ());

    const uint8_t bytes[] = {0x00, 0x01, 0xff};
    value["binary"] = std::span<const uint8_t> (bytes, sizeof (bytes));
    ASSERT_TRUE (value["binary"].isBinary ());
    ASSERT_TRUE (std::ranges::equal (value["binary"].getBinary (), std::span<const uint8_t> (bytes, sizeof (bytes))));

    value["array"] = Array ();
    ASSERT_TRUE (value["array"].isArray ());

    value["object"] = Object ();
    ASSERT_TRUE (value["object"].isObject ());

    const std::string string ("foo");
    value["braced"] = {string};
    ASSERT_TRUE (value["braced"].isString ());

    Value root;
    root.pushBack (2);
    value["moved"] = std::move (root);
    ASSERT_EQ (value["moved"][0], 2);
    ASSERT_TRUE (root.isNull ());
}

/**
 * @brief Test empty method.
 */
TEST (Value, empty)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    Value value;
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = true;
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = false;
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = std::numeric_limits<float>::min ();
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = std::numeric_limits<float>::max ();
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = std::numeric_limits<double>::min ();
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = std::numeric_limits<double>::max ();
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = "";
    ASSERT_TRUE (value.empty ());

    value = "foo";
    ASSERT_FALSE (value.empty ());

    value = "02:42:64:2f:6a:d0";
    ASSERT_FALSE (value.empty ());

    value = "127.0.0.1";
    ASSERT_FALSE (value.empty ());

    const uint8_t bytes[] = {0x00, 0x01, 0xff};
    value = std::span<const uint8_t> ();
    ASSERT_TRUE (value.empty ());

    value = std::span<const uint8_t> (bytes, sizeof (bytes));
    ASSERT_FALSE (value.empty ());

    value = Array ();
    ASSERT_TRUE (value.empty ());

    value = array.getArray ();
    ASSERT_FALSE (value.empty ());

    value = Object ();
    ASSERT_TRUE (value.empty ());

    value = object.getObject ();
    ASSERT_FALSE (value.empty ());
}

/**
 * @brief Test size method.
 */
TEST (Value, size)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    Value value;
    ASSERT_THROW (value.size (), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.size (), std::bad_cast);

    value = true;
    ASSERT_THROW (value.size (), std::bad_cast);

    value = false;
    ASSERT_THROW (value.size (), std::bad_cast);

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_THROW (value.size (), std::bad_cast);

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_THROW (value.size (), std::bad_cast);

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_THROW (value.size (), std::bad_cast);

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_THROW (value.size (), std::bad_cast);

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_THROW (value.size (), std::bad_cast);

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_THROW (value.size (), std::bad_cast);

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_THROW (value.size (), std::bad_cast);

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_THROW (value.size (), std::bad_cast);

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_THROW (value.size (), std::bad_cast);

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_THROW (value.size (), std::bad_cast);

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_THROW (value.size (), std::bad_cast);

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_THROW (value.size (), std::bad_cast);

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_THROW (value.size (), std::bad_cast);

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_THROW (value.size (), std::bad_cast);

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_THROW (value.size (), std::bad_cast);

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_THROW (value.size (), std::bad_cast);

    value = std::numeric_limits<float>::min ();
    ASSERT_THROW (value.size (), std::bad_cast);

    value = std::numeric_limits<float>::max ();
    ASSERT_THROW (value.size (), std::bad_cast);

    value = std::numeric_limits<double>::min ();
    ASSERT_THROW (value.size (), std::bad_cast);

    value = std::numeric_limits<double>::max ();
    ASSERT_THROW (value.size (), std::bad_cast);

    value = "";
    ASSERT_EQ (value.size (), 0);

    value = "foo";
    ASSERT_EQ (value.size (), 3);

    value = "02:42:64:2f:6a:d0";
    ASSERT_EQ (value.size (), 17);

    value = "127.0.0.1";
    ASSERT_EQ (value.size (), 9);

    const uint8_t bytes[] = {0x00, 0x01, 0xff};
    value = std::span<const uint8_t> ();
    ASSERT_EQ (value.size (), 0);

    value = std::span<const uint8_t> (bytes, sizeof (bytes));
    ASSERT_EQ (value.size (), 3);

    value = array.getArray ();
    ASSERT_EQ (value.size (), 1);

    value = object.getObject ();
    ASSERT_EQ (value.size (), 1);
}

/**
 * @brief Test reserve method.
 */
TEST (Value, reserve)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    Value value;
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = true;
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = false;
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = std::numeric_limits<float>::min ();
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = std::numeric_limits<float>::max ();
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = std::numeric_limits<double>::min ();
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = std::numeric_limits<double>::max ();
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = "";
    ASSERT_NO_THROW (value.reserve (12));

    value = "foo";
    ASSERT_NO_THROW (value.reserve (12));

    value = "02:42:64:2f:6a:d0";
    ASSERT_NO_THROW (value.reserve (12));

    value = "127.0.0.1";
    ASSERT_NO_THROW (value.reserve (12));

    value = array.getArray ();
    ASSERT_NO_THROW (value.reserve (12));

    value = object.getObject ();
    ASSERT_NO_THROW (value.reserve (12));

    value = "foo";
    ASSERT_THROW (value.reserve (std::numeric_limits<uint32_t>::max ()), std::bad_alloc);

    value = array.getArray ();
    ASSERT_THROW (value.reserve (size_t (std::numeric_limits<uint32_t>::max ()) + 1), std::bad_alloc);
}

/**
 * @brief Test clear method.
 */
TEST (Value, clear)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    Value value;
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = true;
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = false;
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = std::numeric_limits<float>::min ();
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = std::numeric_limits<float>::max ();
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = std::numeric_limits<double>::min ();
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = std::numeric_limits<double>::max ();
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = "";
    ASSERT_TRUE (value.empty ());
    ASSERT_NO_THROW (value.clear ());
    ASSERT_TRUE (value.empty ());

    value = "foo";
    ASSERT_FALSE (value.empty ());
    ASSERT_NO_THROW (value.clear ());
    ASSERT_TRUE (value.empty ());

    value = "02:42:64:2f:6a:d0";
    ASSERT_FALSE (value.empty ());
    ASSERT_NO_THROW (value.clear ());
    ASSERT_TRUE (value.empty ());

    value = "127.0.0.1";
    ASSERT_FALSE (value.empty ());
    ASSERT_NO_THROW (value.clear ());
    ASSERT_TRUE (value.empty ());

    value = array.getArray ();
    ASSERT_FALSE (value.empty ());
    ASSERT_NO_THROW (value.clear ());
    ASSERT_TRUE (value.empty ());

    value = object.getObject ();
    ASSERT_FALSE (value.empty ());
    ASSERT_NO_THROW (value.clear ());
    ASSERT_TRUE (value.empty ());
}

/**
 * @brief Test insert method.
 */
TEST (Value, insert)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    Value value;
    ASSERT_THROW (value.insert (Member{"i", 1}), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.insert (Member{"i", 1}), std::bad_cast);

    value = true;
    ASSERT_THROW (value.insert (Member{"i", 1}), std::bad_cast);

    value = false;
    ASSERT_THROW (value.insert (Member{"i", 1}), std::bad_cast);

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_THROW (value.insert (Member{"i", 1}), std::bad_cast);

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_THROW (value.insert (Member{"i", 1}), std::bad_cast);

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_THROW (value.insert (Member{"i", 1}), std::bad_cast);

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_THROW (value.insert (Member{"i", 1}), std::bad_cast);

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_THROW (value.insert (Member{"i", 1}), std::bad_cast);

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_THROW (value.insert (Member{"i", 1}), std::bad_cast);

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_THROW (value.insert (Member{"i", 1}), std::bad_cast);

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_THROW (value.insert (Member{"i", 1}), std::bad_cast);

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_THROW (value.insert (Member{"i", 1}), std::bad_cast);

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_THROW (value.insert (Member{"i", 1}), std::bad_cast);

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_THROW (value.insert (Member{"i", 1}), std::bad_cast);

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_THROW (value.insert (Member{"i", 1}), std::bad_cast);

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_THROW (value.insert (Member{"i", 1}), std::bad_cast);

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_THROW (value.insert (Member{"i", 1}), std::bad_cast);

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_THROW (value.insert (Member{"i", 1}), std::bad_cast);

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_THROW (value.insert (Member{"i", 1}), std::bad_cast);

    value = std::numeric_limits<float>::min ();
    ASSERT_THROW (value.insert (Member{"i", 1}), std::bad_cast);

    value = std::numeric_limits<float>::max ();
    ASSERT_THROW (value.insert (Member{"i", 1}), std::bad_cast);

    value = std::numeric_limits<double>::min ();
    ASSERT_THROW (value.insert (Member{"i", 1}), std::bad_cast);

    value = std::numeric_limits<double>::max ();
    ASSERT_THROW (value.insert (Member{"i", 1}), std::bad_cast);

    value = "";
    ASSERT_THROW (value.insert (Member{"i", 1}), std::bad_cast);

    value = "foo";
    ASSERT_THROW (value.insert (Member{"i", 1}), std::bad_cast);

    value = "02:42:64:2f:6a:d0";
    ASSERT_THROW (value.insert (Member{"i", 1}), std::bad_cast);

    value = "127.0.0.1";
    ASSERT_THROW (value.insert (Member{"i", 1}), std::bad_cast);

    value = array.getArray ();
    ASSERT_THROW (value.insert (Member{"i", 1}), std::bad_cast);

    value = object.getObject ();
    ASSERT_NO_THROW (value.insert (Member{"i", 1}));
}

/**
 * @brief Test erase method.
 */
TEST (Value, erase)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    Value value;
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = true;
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = false;
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = std::numeric_limits<float>::min ();
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = std::numeric_limits<float>::max ();
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = std::numeric_limits<double>::min ();
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = std::numeric_limits<double>::max ();
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = "";
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = "foo";
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = "02:42:64:2f:6a:d0";
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = "127.0.0.1";
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = array.getArray ();
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = object.getObject ();
    ASSERT_EQ (value.erase ("i"), 1);
    ASSERT_EQ (value.erase ("j"), 0);

    value = nullptr;
    value[nullptr] = 0;
    value[true] = 1;
    value[int32_t (-2)] = 2;
    value[uint32_t (3)] = 3;
    value[int64_t (-4)] = 4;
    value[uint64_t (5)] = 5;
    value[6.5] = 6;
    value["seven"] = 7;
    value["eight"] = 8;
    value["nine"] = 9;
    ASSERT_EQ (value.erase (nullptr), 1);
    ASSERT_EQ (value.erase (nullptr), 0);
    ASSERT_EQ (value.erase (true), 1);
    ASSERT_EQ (value.erase (true), 0);
    ASSERT_EQ (value.erase (int32_t (-2)), 1);
    ASSERT_EQ (value.erase (int32_t (-2)), 0);
    ASSERT_EQ (value.erase (uint32_t (3)), 1);
    ASSERT_EQ (value.erase (uint32_t (3)), 0);
    ASSERT_EQ (value.erase (int64_t (-4)), 1);
    ASSERT_EQ (value.erase (int64_t (-4)), 0);
    ASSERT_EQ (value.erase (uint64_t (5)), 1);
    ASSERT_EQ (value.erase (uint64_t (5)), 0);
    ASSERT_EQ (value.erase (6.5), 1);
    ASSERT_EQ (value.erase (6.5), 0);
    ASSERT_EQ (value.erase (std::string_view ("seven")), 1);
    ASSERT_EQ (value.erase (std::string_view ("seven")), 0);
    ASSERT_EQ (value.erase ("eight"), 1);
    ASSERT_EQ (value.erase ("eight"), 0);
    ASSERT_EQ (value.erase (std::string ("nine")), 1);
    ASSERT_EQ (value.erase (std::string ("nine")), 0);
    ASSERT_TRUE (value.empty ());
}

/**
 * @brief Test emplaceBack method.
 */
TEST (Value, emplaceBack)
{
    Value value;
    ASSERT_EQ (value.emplaceBack (1), 1);
    ASSERT_EQ (value.emplaceBack ("foo"), "foo");
    ASSERT_EQ (value.size (), 2U);
    ASSERT_EQ (value[1], "foo");

    value = true;
    ASSERT_THROW (value.emplaceBack (1), std::bad_cast);
}

/**
 * @brief Test pushBack method.
 */
TEST (Value, pushBack)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    Value value;
    ASSERT_NO_THROW (value.pushBack (1));

    value = nullptr;
    ASSERT_NO_THROW (value.pushBack (1));

    value = true;
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = false;
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = std::numeric_limits<float>::min ();
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = std::numeric_limits<float>::max ();
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = std::numeric_limits<double>::min ();
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = std::numeric_limits<double>::max ();
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = "";
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = "foo";
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = "02:42:64:2f:6a:d0";
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = "127.0.0.1";
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = array.getArray ();
    ASSERT_NO_THROW (value.pushBack (1));

    value = object.getObject ();
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    const uint8_t bytes[] = {0x00, 0x01, 0xff};
    value = Array ();
    ASSERT_TRUE (value.pushBack (std::span<const uint8_t> (bytes, sizeof (bytes))).isBinary ());
    ASSERT_TRUE (std::ranges::equal (value[0].getBinary (), std::span<const uint8_t> (bytes, sizeof (bytes))));
    ASSERT_EQ (value.pushBack (std::span<const uint8_t> (bytes, sizeof (bytes)), -1).getSubtype (), -1);
    ASSERT_TRUE (std::ranges::equal (value[1].getBinary (), std::span<const uint8_t> (bytes, sizeof (bytes))));
}

/**
 * @brief Test popBack method.
 */
TEST (Value, popBack)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    Value value;
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = true;
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = false;
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = std::numeric_limits<float>::min ();
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = std::numeric_limits<float>::max ();
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = std::numeric_limits<double>::min ();
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = std::numeric_limits<double>::max ();
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = "";
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = "foo";
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = "02:42:64:2f:6a:d0";
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = "127.0.0.1";
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = array.getArray ();
    ASSERT_NO_THROW (value.popBack ());
    ASSERT_TRUE (value.empty ());
    ASSERT_NO_THROW (value.popBack ());
    ASSERT_TRUE (value.empty ());

    value = object.getObject ();
    ASSERT_THROW (value.popBack (), std::bad_cast);
}

/**
 * @brief Test contains method.
 */
TEST (Value, contains)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    Value value;
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = true;
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = false;
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = std::numeric_limits<int8_t>::min ();
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = std::numeric_limits<int8_t>::max ();
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = std::numeric_limits<uint8_t>::min ();
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = std::numeric_limits<uint8_t>::max ();
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = std::numeric_limits<int16_t>::min ();
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = std::numeric_limits<int16_t>::max ();
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = std::numeric_limits<uint16_t>::min ();
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = std::numeric_limits<uint16_t>::max ();
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = std::numeric_limits<int32_t>::min ();
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = std::numeric_limits<int32_t>::max ();
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = std::numeric_limits<uint32_t>::min ();
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = std::numeric_limits<uint32_t>::max ();
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = std::numeric_limits<int64_t>::min ();
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = std::numeric_limits<int64_t>::max ();
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = std::numeric_limits<uint64_t>::min ();
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = std::numeric_limits<uint64_t>::max ();
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = std::numeric_limits<float>::min ();
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = std::numeric_limits<float>::max ();
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = std::numeric_limits<double>::min ();
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = std::numeric_limits<double>::max ();
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = "";
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = "foo";
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = "02:42:64:2f:6a:d0";
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = "127.0.0.1";
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = Array ();
    ASSERT_FALSE (value.contains (0));
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = array.getArray ();
    ASSERT_TRUE (value.contains (0));
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = Object ();
    ASSERT_FALSE (value.contains (0));
    ASSERT_FALSE (value.contains ("i"));

    value = object.getObject ();
    ASSERT_FALSE (value.contains (0));
    ASSERT_TRUE (value.contains ("i"));

    value = nullptr;
    value[nullptr] = 0;
    value[true] = 1;
    value[int32_t (-2)] = 2;
    value[uint32_t (3)] = 3;
    value[int64_t (-4)] = 4;
    value[uint64_t (5)] = 5;
    value[6.5] = 6;
    value["seven"] = 7;
    ASSERT_TRUE (value.contains (nullptr));
    ASSERT_TRUE (value.contains (true));
    ASSERT_TRUE (value.contains (int32_t (-2)));
    ASSERT_TRUE (value.contains (uint32_t (3)));
    ASSERT_TRUE (value.contains (int64_t (-4)));
    ASSERT_TRUE (value.contains (uint64_t (5)));
    ASSERT_TRUE (value.contains (6.5));
    ASSERT_TRUE (value.contains (std::string_view ("seven")));
    ASSERT_TRUE (value.contains ("seven"));
    ASSERT_TRUE (value.contains (std::string ("seven")));
    ASSERT_FALSE (value.contains (false));
    ASSERT_FALSE (value.contains (std::string_view ("eight")));

    value = nullptr;
    value["seven"] = 7;
    ASSERT_FALSE (value.contains (nullptr));
    ASSERT_FALSE (value.contains (uint32_t (3)));
    ASSERT_FALSE (value.contains (int64_t (-4)));
    ASSERT_FALSE (value.contains (uint64_t (5)));
    ASSERT_FALSE (value.contains (6.5));
    ASSERT_TRUE (value.contains (Value ("seven")));
    ASSERT_FALSE (value.contains (Value ("eight")));

    value = nullptr;
    value.pushBack (0);
    value.pushBack (1);
    ASSERT_TRUE (value.contains (int32_t (1)));
    ASSERT_FALSE (value.contains (int32_t (2)));
    ASSERT_FALSE (value.contains (int32_t (-1)));
    ASSERT_TRUE (value.contains (uint32_t (1)));
    ASSERT_FALSE (value.contains (uint32_t (2)));
    ASSERT_TRUE (value.contains (int64_t (1)));
    ASSERT_FALSE (value.contains (int64_t (2)));
    ASSERT_FALSE (value.contains (int64_t (-1)));
    ASSERT_TRUE (value.contains (uint64_t (1)));
    ASSERT_FALSE (value.contains (uint64_t (2)));
    ASSERT_TRUE (value.contains (Value (1)));
    ASSERT_FALSE (value.contains (Value (2)));
    ASSERT_THROW (value.contains (nullptr), std::bad_cast);
    ASSERT_THROW (value.contains (true), std::bad_cast);
    ASSERT_THROW (value.contains (1.0), std::bad_cast);
}

/**
 * @brief Test swap method.
 */
TEST (Value, swap)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    Value value = nullptr;

    Value other = array.getArray ();
    ASSERT_TRUE (value.isNull ());
    ASSERT_TRUE (other.isArray ());
    value.swap (other);
    ASSERT_TRUE (value.isArray ());
    ASSERT_TRUE (other.isNull ());

    other = true;
    ASSERT_TRUE (value.isArray ());
    ASSERT_TRUE (other.isBool ());
    value.swap (other);
    ASSERT_TRUE (value.isBool ());
    ASSERT_TRUE (other.isArray ());

    other = std::numeric_limits<int32_t>::min ();
    ASSERT_TRUE (value.isBool ());
    ASSERT_TRUE (other.isInt ());
    value.swap (other);
    ASSERT_TRUE (value.isInt ());
    ASSERT_TRUE (other.isBool ());

    other = std::numeric_limits<uint32_t>::max ();
    ASSERT_TRUE (value.isInt ());
    ASSERT_TRUE (other.isUint ());
    value.swap (other);
    ASSERT_TRUE (value.isUint ());
    ASSERT_TRUE (other.isInt ());

    other = std::numeric_limits<int64_t>::min ();
    ASSERT_TRUE (value.isUint ());
    ASSERT_TRUE (other.isInt64 ());
    value.swap (other);
    ASSERT_TRUE (value.isInt64 ());
    ASSERT_TRUE (other.isUint ());

    other = std::numeric_limits<uint64_t>::max ();
    ASSERT_TRUE (value.isInt64 ());
    ASSERT_TRUE (other.isUint64 ());
    value.swap (other);
    ASSERT_TRUE (value.isUint64 ());
    ASSERT_TRUE (other.isInt64 ());

    other = std::numeric_limits<float>::min ();
    ASSERT_TRUE (value.isUint64 ());
    ASSERT_TRUE (other.isFloat ());
    value.swap (other);
    ASSERT_TRUE (value.isFloat ());
    ASSERT_TRUE (other.isUint64 ());

    other = std::numeric_limits<double>::max ();
    ASSERT_TRUE (value.isFloat ());
    ASSERT_TRUE (other.isDouble ());
    value.swap (other);
    ASSERT_TRUE (value.isDouble ());
    ASSERT_TRUE (other.isFloat ());

    other = "foo";
    ASSERT_TRUE (value.isDouble ());
    ASSERT_TRUE (other.isString ());
    value.swap (other);
    ASSERT_TRUE (value.isString ());
    ASSERT_TRUE (other.isDouble ());

    other = array.getArray ();
    ASSERT_TRUE (value.isString ());
    ASSERT_TRUE (other.isArray ());
    value.swap (other);
    ASSERT_TRUE (value.isArray ());
    ASSERT_TRUE (other.isString ());

    other = object.getObject ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_TRUE (other.isObject ());
    value.swap (other);
    ASSERT_TRUE (value.isObject ());
    ASSERT_TRUE (other.isArray ());

    value = nullptr;
    value["a"] = 1;
    value["b"] = "foo";
    value["a"].swap (value["b"]);
    ASSERT_EQ (value["a"], "foo");
    ASSERT_EQ (value["b"], 1);
}

/**
 * @brief Test serialize method.
 */
TEST (Value, serialize)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    Value keys;
    keys[nullptr] = 0;
    keys[true] = 1;
    keys[int32_t (-2)] = 2;
    keys[uint32_t (3)] = 3;
    keys[int64_t (-4)] = 4;
    keys[uint64_t (5)] = 5;
    keys[6.5] = 6;
    keys["seven"] = 7;
    Value copy;
    join::ValueBuilder builder (copy);
    ASSERT_EQ (keys.serialize (builder), 0) << join::lastError.message ();
    EXPECT_EQ (copy, keys);

    Value value;
    value.pushBack (1);
    value.pushBack (nullptr);
    value.pushBack (true);
    value.pushBack ("test");
    value.pushBack (1.23456789e-13);

    std::stringstream out;
    JsonWriter jsonWriter (out);
    ASSERT_EQ (value.serialize (jsonWriter), 0) << join::lastError.message ();
    EXPECT_EQ (out.str (), "[1,null,true,\"test\",1.23456789e-13]");

    out.str ("");
    ASSERT_EQ (value.serialize<JsonWriter> (out), 0) << join::lastError.message ();
    EXPECT_EQ (out.str (), "[1,null,true,\"test\",1.23456789e-13]");

    std::stringstream indented;
    JsonWriter indentedWriter (indented, 4);
    ASSERT_EQ (value.serialize (indentedWriter), 0) << join::lastError.message ();
    out.str ("");
    ASSERT_EQ (value.serialize<JsonWriter> (out, 4), 0) << join::lastError.message ();
    EXPECT_EQ (out.str (), indented.str ());

    const uint8_t bytes[] = {0x00, 0xff};
    Value binary;
    binary.pushBack (std::span<const uint8_t> (bytes, sizeof (bytes)));
    out.str ("");
    ASSERT_EQ (binary.serialize<PackWriter> (out), 0) << join::lastError.message ();
    EXPECT_EQ (out.str (), std::string ({'\x91', '\xc4', '\x02', '\x00', '\xff'}));

    Value extension;
    extension.pushBack (std::span<const uint8_t> (bytes, sizeof (bytes)), -1);
    out.str ("");
    ASSERT_EQ (extension.serialize<PackWriter> (out), 0) << join::lastError.message ();
    EXPECT_EQ (out.str (), std::string ({'\x91', '\xd5', '\xff', '\x00', '\xff'}));

    Value binaryKey;
    binaryKey[Value (std::span<const uint8_t> (bytes, sizeof (bytes)), -1)] = 1;
    out.str ("");
    ASSERT_EQ (binaryKey.serialize<PackWriter> (out), 0) << join::lastError.message ();
    EXPECT_EQ (out.str (), std::string ({'\x81', '\xd5', '\xff', '\x00', '\xff', '\x01'}));

    out.str ("");
    PackWriter packWriter (out);
    ASSERT_EQ (value.serialize (packWriter), 0) << join::lastError.message ();
    EXPECT_EQ (out.str (), std::string ({'\x95', '\x01', '\xc0', '\xc3', '\xa4', '\x74', '\x65', '\x73', '\x74', '\xcb',
                                         '\x3d', '\x41', '\x5f', '\xff', '\xe5', '\x3a', '\x68', '\x5d'}));

    out.str ("");
    ASSERT_EQ (value.serialize<PackWriter> (out), 0) << join::lastError.message ();
    EXPECT_EQ (out.str (), std::string ({'\x95', '\x01', '\xc0', '\xc3', '\xa4', '\x74', '\x65', '\x73', '\x74', '\xcb',
                                         '\x3d', '\x41', '\x5f', '\xff', '\xe5', '\x3a', '\x68', '\x5d'}));

    Counter counter;
    ASSERT_EQ (value.serialize (counter), 0) << join::lastError.message ();
    EXPECT_EQ (counter.values, 5U);

    value = Object ();
    value["numbers"].pushBack (333333333.33333329);
    value["numbers"].pushBack (1E30);
    value["numbers"].pushBack (4.50);
    value["numbers"].pushBack (2e-3);
    value["numbers"].pushBack (0.000000000000000000000000001);
    value["string"] = "\u20ac$\u000F\u000aA'\u0042\u0022\u005c\\\"/";
    value["literals"].pushBack (nullptr);
    value["literals"].pushBack (true);
    value["literals"].pushBack (false);

    out.str ("");
    ASSERT_EQ (value.serialize<JsonCanonicalizer> (out), 0) << join::lastError.message ();
    EXPECT_EQ (out.str (),
               "{\"literals\":[null,true,false],\"numbers\":[333333333.3333333,1e+30,4.5,0.002,1e-27],\"string\":\"€$"
               "\\u000f\\nA'B\\\"\\\\\\\\\\\"/\"}");

    value = Object ();
    value.insert (array, 1);
    out.str ("");
    ASSERT_EQ (value.serialize (packWriter), 0) << join::lastError.message ();
    EXPECT_EQ (out.str (), std::string ({'\x81', '\x91', '\x01', '\x01'}));

    value = Object ();
    value.insert (object, 1);
    out.str ("");
    ASSERT_EQ (value.serialize (packWriter), 0) << join::lastError.message ();
    EXPECT_EQ (out.str (), std::string ({'\x81', '\x81', '\xa1', '\x69', '\x01', '\x01'}));

    out.str ("");
    ASSERT_EQ (value.serialize<JsonWriter> (out), -1);
    ASSERT_EQ (join::lastError, SaxErrc::InvalidKey);

    value = nullptr;
    value.pushBack (7);
    ASSERT_EQ (value.serialize (packWriter), 0) << join::lastError.message ();
    ASSERT_EQ (out.str (), std::string ({'\x91', '\x07'}));

    FailingBuffer buffer;
    std::ostream failing (&buffer);
    JsonWriter failingWriter (failing);
    value = nullptr;
    value.pushBack (std::string (2000, 'a'));
    ASSERT_THROW (value.serialize (failingWriter), std::runtime_error);
    value = nullptr;
    value.pushBack (1);
    value.pushBack (2);
    ASSERT_EQ (value.serialize (failingWriter), 0) << join::lastError.message ();
    ASSERT_EQ (buffer.written, "[1,2]");
}

/**
 * @brief Test deserialize method.
 */
TEST (Value, deserialize)
{
    Value value;
    ASSERT_EQ (value.deserialize<JsonReader> (std::string ("[1,{\"a\":\"b\"}]")), 0) << join::lastError.message ();
    ASSERT_EQ (value.size (), 2U);
    ASSERT_EQ (value.at (0), 1);
    ASSERT_EQ (value.at (1).at ("a"), "b");

    const char json[] = "{\"c\":[true,null]}";
    ASSERT_EQ (value.deserialize<JsonReader> (json, sizeof (json) - 1), 0) << join::lastError.message ();
    ASSERT_EQ (value.size (), 1U);
    ASSERT_EQ (value.at ("c").size (), 2U);
    ASSERT_EQ (value.at ("c").at (0), true);
    ASSERT_EQ (value.at ("c").at (1), nullptr);

    ASSERT_EQ (value.deserialize<JsonReader> (&json[5], &json[sizeof (json) - 2]), 0) << join::lastError.message ();
    ASSERT_EQ (value.size (), 2U);
    ASSERT_EQ (value.at (0), true);
    ASSERT_EQ (value.at (1), nullptr);

    std::stringstream stream ("42");
    ASSERT_EQ (value.deserialize<JsonReader> (stream), 0) << join::lastError.message ();
    ASSERT_EQ (value, 42);

    ASSERT_EQ (value.deserialize<join::BasicJsonReader<JsonReadMode::ParseComments>> (std::string ("/* c */ 1.5")), 0)
        << join::lastError.message ();
    ASSERT_EQ (value, 1.5);

    ASSERT_EQ (value.deserialize<JsonReader> (std::string ("[1, [2, ")), -1);
    ASSERT_TRUE (value.isNull ());

    ASSERT_EQ (value.deserialize<PackReader> (std::string ({'\x92', '\x01', '\xa1', '\x61'})), 0)
        << join::lastError.message ();
    ASSERT_EQ (value.size (), 2U);
    ASSERT_EQ (value.at (0), 1);
    ASSERT_EQ (value.at (1), "a");

    ASSERT_EQ (value.deserialize<PackReader> (std::string ({'\x92', '\x01'})), -1);
    ASSERT_EQ (join::lastError, SaxErrc::Truncated);
    ASSERT_TRUE (value.isNull ());
}

/**
 * @brief equal operators.
 */
TEST (Value, equal)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    Value value;

    value = nullptr;
    ASSERT_TRUE (value == nullptr);
    ASSERT_FALSE (value == true);
    ASSERT_FALSE (value == false);
    ASSERT_FALSE (value == std::numeric_limits<int32_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits<int32_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits<uint32_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits<uint32_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits<int64_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits<int64_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits<uint64_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits<uint64_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits<double>::min ());
    ASSERT_FALSE (value == std::numeric_limits<double>::max ());
    ASSERT_FALSE (value == "foo");
    ASSERT_FALSE (value == array.getArray ());
    ASSERT_FALSE (value == object.getObject ());

    value = bool (true);
    ASSERT_FALSE (value == nullptr);
    ASSERT_TRUE (value == true);
    ASSERT_FALSE (value == false);
    ASSERT_FALSE (value == std::numeric_limits<int32_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits<int32_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits<uint32_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits<uint32_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits<int64_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits<int64_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits<uint64_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits<uint64_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits<double>::min ());
    ASSERT_FALSE (value == std::numeric_limits<double>::max ());
    ASSERT_FALSE (value == "foo");
    ASSERT_FALSE (value == array.getArray ());
    ASSERT_FALSE (value == object.getObject ());

    value = int32_t (std::numeric_limits<int32_t>::min ());
    ASSERT_FALSE (value == nullptr);
    ASSERT_FALSE (value == true);
    ASSERT_FALSE (value == false);
    ASSERT_TRUE (value == std::numeric_limits<int32_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits<int32_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits<uint32_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits<uint32_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits<int64_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits<int64_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits<uint64_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits<uint64_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits<double>::min ());
    ASSERT_FALSE (value == std::numeric_limits<double>::max ());
    ASSERT_FALSE (value == "foo");
    ASSERT_FALSE (value == array.getArray ());
    ASSERT_FALSE (value == object.getObject ());

    value = uint32_t (std::numeric_limits<uint32_t>::max ());
    ASSERT_FALSE (value == nullptr);
    ASSERT_FALSE (value == true);
    ASSERT_FALSE (value == false);
    ASSERT_FALSE (value == std::numeric_limits<int32_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits<int32_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits<uint32_t>::min ());
    ASSERT_TRUE (value == std::numeric_limits<uint32_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits<int64_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits<int64_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits<uint64_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits<uint64_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits<double>::min ());
    ASSERT_FALSE (value == std::numeric_limits<double>::max ());
    ASSERT_FALSE (value == "foo");
    ASSERT_FALSE (value == array.getArray ());
    ASSERT_FALSE (value == object.getObject ());

    value = int64_t (std::numeric_limits<int64_t>::min ());
    ASSERT_FALSE (value == nullptr);
    ASSERT_FALSE (value == true);
    ASSERT_FALSE (value == false);
    ASSERT_FALSE (value == std::numeric_limits<int32_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits<int32_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits<uint32_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits<uint32_t>::max ());
    ASSERT_TRUE (value == std::numeric_limits<int64_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits<int64_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits<uint64_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits<uint64_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits<double>::min ());
    ASSERT_FALSE (value == std::numeric_limits<double>::max ());
    ASSERT_FALSE (value == "foo");
    ASSERT_FALSE (value == array.getArray ());
    ASSERT_FALSE (value == object.getObject ());

    value = uint64_t (std::numeric_limits<uint64_t>::max ());
    ASSERT_FALSE (value == nullptr);
    ASSERT_FALSE (value == true);
    ASSERT_FALSE (value == false);
    ASSERT_FALSE (value == std::numeric_limits<int32_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits<int32_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits<uint32_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits<uint32_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits<int64_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits<int64_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits<uint64_t>::min ());
    ASSERT_TRUE (value == std::numeric_limits<uint64_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits<double>::min ());
    ASSERT_FALSE (value == std::numeric_limits<double>::max ());
    ASSERT_FALSE (value == "foo");
    ASSERT_FALSE (value == array.getArray ());
    ASSERT_FALSE (value == object.getObject ());

    value = double (std::numeric_limits<double>::min ());
    ASSERT_FALSE (value == nullptr);
    ASSERT_FALSE (value == true);
    ASSERT_FALSE (value == false);
    ASSERT_FALSE (value == std::numeric_limits<int32_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits<int32_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits<uint32_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits<uint32_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits<int64_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits<int64_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits<uint64_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits<uint64_t>::max ());
    ASSERT_TRUE (value == std::numeric_limits<double>::min ());
    ASSERT_FALSE (value == std::numeric_limits<double>::max ());
    ASSERT_FALSE (value == "foo");
    ASSERT_FALSE (value == array.getArray ());
    ASSERT_FALSE (value == object.getObject ());

    value = std::string ("foo");
    ASSERT_FALSE (value == nullptr);
    ASSERT_FALSE (value == true);
    ASSERT_FALSE (value == false);
    ASSERT_FALSE (value == std::numeric_limits<int32_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits<int32_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits<uint32_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits<uint32_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits<int64_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits<int64_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits<uint64_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits<uint64_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits<double>::min ());
    ASSERT_FALSE (value == std::numeric_limits<double>::max ());
    ASSERT_TRUE (value == "foo");
    ASSERT_FALSE (value == "bar");
    ASSERT_FALSE (value == array.getArray ());
    ASSERT_FALSE (value == object.getObject ());

    value = array.getArray ();
    ASSERT_FALSE (value == nullptr);
    ASSERT_FALSE (value == true);
    ASSERT_FALSE (value == false);
    ASSERT_FALSE (value == std::numeric_limits<int32_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits<int32_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits<uint32_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits<uint32_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits<int64_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits<int64_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits<uint64_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits<uint64_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits<double>::min ());
    ASSERT_FALSE (value == std::numeric_limits<double>::max ());
    ASSERT_FALSE (value == "foo");
    ASSERT_TRUE (value == array.getArray ());
    ASSERT_FALSE (value == object.getObject ());

    value = object.getObject ();
    ASSERT_FALSE (value == nullptr);
    ASSERT_FALSE (value == true);
    ASSERT_FALSE (value == false);
    ASSERT_FALSE (value == std::numeric_limits<int32_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits<int32_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits<uint32_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits<uint32_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits<int64_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits<int64_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits<uint64_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits<uint64_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits<double>::min ());
    ASSERT_FALSE (value == std::numeric_limits<double>::max ());
    ASSERT_FALSE (value == "foo");
    ASSERT_FALSE (value == array.getArray ());
    ASSERT_TRUE (value == object.getObject ());

    value = nullptr;
    ASSERT_TRUE (nullptr == value);
    ASSERT_FALSE (true == value);
    ASSERT_FALSE (false == value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits<double>::min () == value);
    ASSERT_FALSE (std::numeric_limits<double>::max () == value);
    ASSERT_FALSE ("foo" == value);
    ASSERT_FALSE (array.getArray () == value);
    ASSERT_FALSE (object.getObject () == value);

    value = bool (true);
    ASSERT_FALSE (nullptr == value);
    ASSERT_TRUE (true == value);
    ASSERT_FALSE (false == value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits<double>::min () == value);
    ASSERT_FALSE (std::numeric_limits<double>::max () == value);
    ASSERT_FALSE ("foo" == value);
    ASSERT_FALSE (array.getArray () == value);
    ASSERT_FALSE (object.getObject () == value);

    value = int32_t (std::numeric_limits<int32_t>::min ());
    ASSERT_FALSE (nullptr == value);
    ASSERT_FALSE (true == value);
    ASSERT_FALSE (false == value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits<double>::min () == value);
    ASSERT_FALSE (std::numeric_limits<double>::max () == value);
    ASSERT_FALSE ("foo" == value);
    ASSERT_FALSE (array.getArray () == value);
    ASSERT_FALSE (object.getObject () == value);

    value = uint32_t (std::numeric_limits<uint32_t>::max ());
    ASSERT_FALSE (nullptr == value);
    ASSERT_FALSE (true == value);
    ASSERT_FALSE (false == value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::min () == value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits<double>::min () == value);
    ASSERT_FALSE (std::numeric_limits<double>::max () == value);
    ASSERT_FALSE ("foo" == value);
    ASSERT_FALSE (array.getArray () == value);
    ASSERT_FALSE (object.getObject () == value);

    value = int64_t (std::numeric_limits<int64_t>::min ());
    ASSERT_FALSE (nullptr == value);
    ASSERT_FALSE (true == value);
    ASSERT_FALSE (false == value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::max () == value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits<double>::min () == value);
    ASSERT_FALSE (std::numeric_limits<double>::max () == value);
    ASSERT_FALSE ("foo" == value);
    ASSERT_FALSE (array.getArray () == value);
    ASSERT_FALSE (object.getObject () == value);

    value = uint64_t (std::numeric_limits<uint64_t>::max ());
    ASSERT_FALSE (nullptr == value);
    ASSERT_FALSE (true == value);
    ASSERT_FALSE (false == value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::min () == value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits<double>::min () == value);
    ASSERT_FALSE (std::numeric_limits<double>::max () == value);
    ASSERT_FALSE ("foo" == value);
    ASSERT_FALSE (array.getArray () == value);
    ASSERT_FALSE (object.getObject () == value);

    value = double (std::numeric_limits<double>::min ());
    ASSERT_FALSE (nullptr == value);
    ASSERT_FALSE (true == value);
    ASSERT_FALSE (false == value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::max () == value);
    ASSERT_TRUE (std::numeric_limits<double>::min () == value);
    ASSERT_FALSE (std::numeric_limits<double>::max () == value);
    ASSERT_FALSE ("foo" == value);
    ASSERT_FALSE (array.getArray () == value);
    ASSERT_FALSE (object.getObject () == value);

    value = std::string ("foo");
    ASSERT_FALSE (nullptr == value);
    ASSERT_FALSE (true == value);
    ASSERT_FALSE (false == value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits<double>::min () == value);
    ASSERT_FALSE (std::numeric_limits<double>::max () == value);
    ASSERT_TRUE ("foo" == value);
    ASSERT_FALSE ("bar" == value);
    ASSERT_FALSE (array.getArray () == value);
    ASSERT_FALSE (object.getObject () == value);

    value = array.getArray ();
    ASSERT_FALSE (nullptr == value);
    ASSERT_FALSE (true == value);
    ASSERT_FALSE (false == value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits<double>::min () == value);
    ASSERT_FALSE (std::numeric_limits<double>::max () == value);
    ASSERT_FALSE ("foo" == value);
    ASSERT_TRUE (array.getArray () == value);
    ASSERT_FALSE (object.getObject () == value);

    value = object.getObject ();
    ASSERT_FALSE (nullptr == value);
    ASSERT_FALSE (true == value);
    ASSERT_FALSE (false == value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits<double>::min () == value);
    ASSERT_FALSE (std::numeric_limits<double>::max () == value);
    ASSERT_FALSE ("foo" == value);
    ASSERT_FALSE (array.getArray () == value);
    ASSERT_TRUE (object.getObject () == value);

    const uint8_t bytes[] = {0x00, 0x01, 0xff};
    value = std::span<const uint8_t> (bytes, sizeof (bytes));
    ASSERT_TRUE (value == Value (std::span<const uint8_t> (bytes, sizeof (bytes))));
    ASSERT_FALSE (value == Value (std::span<const uint8_t> (bytes, sizeof (bytes)), 0));

    value = Value (std::span<const uint8_t> (bytes, sizeof (bytes)), 0);
    ASSERT_TRUE (value == Value (std::span<const uint8_t> (bytes, sizeof (bytes)), 0));
    ASSERT_FALSE (value == Value (std::span<const uint8_t> (bytes, sizeof (bytes)), -1));
}

/**
 * @brief notEqual operators.
 */
TEST (Value, notEqual)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    Value value;

    value = nullptr;
    ASSERT_FALSE (value != nullptr);
    ASSERT_TRUE (value != true);
    ASSERT_TRUE (value != false);
    ASSERT_TRUE (value != std::numeric_limits<int32_t>::min ());
    ASSERT_TRUE (value != std::numeric_limits<int32_t>::max ());
    ASSERT_TRUE (value != std::numeric_limits<uint32_t>::min ());
    ASSERT_TRUE (value != std::numeric_limits<uint32_t>::max ());
    ASSERT_TRUE (value != std::numeric_limits<int64_t>::min ());
    ASSERT_TRUE (value != std::numeric_limits<int64_t>::max ());
    ASSERT_TRUE (value != std::numeric_limits<uint64_t>::min ());
    ASSERT_TRUE (value != std::numeric_limits<uint64_t>::max ());
    ASSERT_TRUE (value != std::numeric_limits<double>::min ());
    ASSERT_TRUE (value != std::numeric_limits<double>::max ());
    ASSERT_TRUE (value != "foo");
    ASSERT_TRUE (value != array.getArray ());
    ASSERT_TRUE (value != object.getObject ());

    value = bool (true);
    ASSERT_TRUE (value != nullptr);
    ASSERT_FALSE (value != true);
    ASSERT_TRUE (value != false);
    ASSERT_TRUE (value != std::numeric_limits<int32_t>::min ());
    ASSERT_TRUE (value != std::numeric_limits<int32_t>::max ());
    ASSERT_TRUE (value != std::numeric_limits<uint32_t>::min ());
    ASSERT_TRUE (value != std::numeric_limits<uint32_t>::max ());
    ASSERT_TRUE (value != std::numeric_limits<int64_t>::min ());
    ASSERT_TRUE (value != std::numeric_limits<int64_t>::max ());
    ASSERT_TRUE (value != std::numeric_limits<uint64_t>::min ());
    ASSERT_TRUE (value != std::numeric_limits<uint64_t>::max ());
    ASSERT_TRUE (value != std::numeric_limits<double>::min ());
    ASSERT_TRUE (value != std::numeric_limits<double>::max ());
    ASSERT_TRUE (value != "foo");
    ASSERT_TRUE (value != array.getArray ());
    ASSERT_TRUE (value != object.getObject ());

    value = int32_t (std::numeric_limits<int32_t>::min ());
    ASSERT_TRUE (value != nullptr);
    ASSERT_TRUE (value != true);
    ASSERT_TRUE (value != false);
    ASSERT_FALSE (value != std::numeric_limits<int32_t>::min ());
    ASSERT_TRUE (value != std::numeric_limits<int32_t>::max ());
    ASSERT_TRUE (value != std::numeric_limits<uint32_t>::min ());
    ASSERT_TRUE (value != std::numeric_limits<uint32_t>::max ());
    ASSERT_TRUE (value != std::numeric_limits<int64_t>::min ());
    ASSERT_TRUE (value != std::numeric_limits<int64_t>::max ());
    ASSERT_TRUE (value != std::numeric_limits<uint64_t>::min ());
    ASSERT_TRUE (value != std::numeric_limits<uint64_t>::max ());
    ASSERT_TRUE (value != std::numeric_limits<double>::min ());
    ASSERT_TRUE (value != std::numeric_limits<double>::max ());
    ASSERT_TRUE (value != "foo");
    ASSERT_TRUE (value != array.getArray ());
    ASSERT_TRUE (value != object.getObject ());

    value = uint32_t (std::numeric_limits<uint32_t>::max ());
    ASSERT_TRUE (value != nullptr);
    ASSERT_TRUE (value != true);
    ASSERT_TRUE (value != false);
    ASSERT_TRUE (value != std::numeric_limits<int32_t>::min ());
    ASSERT_TRUE (value != std::numeric_limits<int32_t>::max ());
    ASSERT_TRUE (value != std::numeric_limits<uint32_t>::min ());
    ASSERT_FALSE (value != std::numeric_limits<uint32_t>::max ());
    ASSERT_TRUE (value != std::numeric_limits<int64_t>::min ());
    ASSERT_TRUE (value != std::numeric_limits<int64_t>::max ());
    ASSERT_TRUE (value != std::numeric_limits<uint64_t>::min ());
    ASSERT_TRUE (value != std::numeric_limits<uint64_t>::max ());
    ASSERT_TRUE (value != std::numeric_limits<double>::min ());
    ASSERT_TRUE (value != std::numeric_limits<double>::max ());
    ASSERT_TRUE (value != "foo");
    ASSERT_TRUE (value != array.getArray ());
    ASSERT_TRUE (value != object.getObject ());

    value = int64_t (std::numeric_limits<int64_t>::min ());
    ASSERT_TRUE (value != nullptr);
    ASSERT_TRUE (value != true);
    ASSERT_TRUE (value != false);
    ASSERT_TRUE (value != std::numeric_limits<int32_t>::min ());
    ASSERT_TRUE (value != std::numeric_limits<int32_t>::max ());
    ASSERT_TRUE (value != std::numeric_limits<uint32_t>::min ());
    ASSERT_TRUE (value != std::numeric_limits<uint32_t>::max ());
    ASSERT_FALSE (value != std::numeric_limits<int64_t>::min ());
    ASSERT_TRUE (value != std::numeric_limits<int64_t>::max ());
    ASSERT_TRUE (value != std::numeric_limits<uint64_t>::min ());
    ASSERT_TRUE (value != std::numeric_limits<uint64_t>::max ());
    ASSERT_TRUE (value != std::numeric_limits<double>::min ());
    ASSERT_TRUE (value != std::numeric_limits<double>::max ());
    ASSERT_TRUE (value != "foo");
    ASSERT_TRUE (value != array.getArray ());
    ASSERT_TRUE (value != object.getObject ());

    value = uint64_t (std::numeric_limits<uint64_t>::max ());
    ASSERT_TRUE (value != nullptr);
    ASSERT_TRUE (value != true);
    ASSERT_TRUE (value != false);
    ASSERT_TRUE (value != std::numeric_limits<int32_t>::min ());
    ASSERT_TRUE (value != std::numeric_limits<int32_t>::max ());
    ASSERT_TRUE (value != std::numeric_limits<uint32_t>::min ());
    ASSERT_TRUE (value != std::numeric_limits<uint32_t>::max ());
    ASSERT_TRUE (value != std::numeric_limits<int64_t>::min ());
    ASSERT_TRUE (value != std::numeric_limits<int64_t>::max ());
    ASSERT_TRUE (value != std::numeric_limits<uint64_t>::min ());
    ASSERT_FALSE (value != std::numeric_limits<uint64_t>::max ());
    ASSERT_TRUE (value != std::numeric_limits<double>::min ());
    ASSERT_TRUE (value != std::numeric_limits<double>::max ());
    ASSERT_TRUE (value != "foo");
    ASSERT_TRUE (value != array.getArray ());
    ASSERT_TRUE (value != object.getObject ());

    value = double (std::numeric_limits<double>::min ());
    ASSERT_TRUE (value != nullptr);
    ASSERT_TRUE (value != true);
    ASSERT_TRUE (value != false);
    ASSERT_TRUE (value != std::numeric_limits<int32_t>::min ());
    ASSERT_TRUE (value != std::numeric_limits<int32_t>::max ());
    ASSERT_TRUE (value != std::numeric_limits<uint32_t>::min ());
    ASSERT_TRUE (value != std::numeric_limits<uint32_t>::max ());
    ASSERT_TRUE (value != std::numeric_limits<int64_t>::min ());
    ASSERT_TRUE (value != std::numeric_limits<int64_t>::max ());
    ASSERT_TRUE (value != std::numeric_limits<uint64_t>::min ());
    ASSERT_TRUE (value != std::numeric_limits<uint64_t>::max ());
    ASSERT_FALSE (value != std::numeric_limits<double>::min ());
    ASSERT_TRUE (value != std::numeric_limits<double>::max ());
    ASSERT_TRUE (value != "foo");
    ASSERT_TRUE (value != array.getArray ());
    ASSERT_TRUE (value != object.getObject ());

    value = std::string ("foo");
    ASSERT_TRUE (value != nullptr);
    ASSERT_TRUE (value != true);
    ASSERT_TRUE (value != false);
    ASSERT_TRUE (value != std::numeric_limits<int32_t>::min ());
    ASSERT_TRUE (value != std::numeric_limits<int32_t>::max ());
    ASSERT_TRUE (value != std::numeric_limits<uint32_t>::min ());
    ASSERT_TRUE (value != std::numeric_limits<uint32_t>::max ());
    ASSERT_TRUE (value != std::numeric_limits<int64_t>::min ());
    ASSERT_TRUE (value != std::numeric_limits<int64_t>::max ());
    ASSERT_TRUE (value != std::numeric_limits<uint64_t>::min ());
    ASSERT_TRUE (value != std::numeric_limits<uint64_t>::max ());
    ASSERT_TRUE (value != std::numeric_limits<double>::min ());
    ASSERT_TRUE (value != std::numeric_limits<double>::max ());
    ASSERT_FALSE (value != "foo");
    ASSERT_TRUE (value != array.getArray ());
    ASSERT_TRUE (value != object.getObject ());

    value = array.getArray ();
    ASSERT_TRUE (value != nullptr);
    ASSERT_TRUE (value != true);
    ASSERT_TRUE (value != false);
    ASSERT_TRUE (value != std::numeric_limits<int32_t>::min ());
    ASSERT_TRUE (value != std::numeric_limits<int32_t>::max ());
    ASSERT_TRUE (value != std::numeric_limits<uint32_t>::min ());
    ASSERT_TRUE (value != std::numeric_limits<uint32_t>::max ());
    ASSERT_TRUE (value != std::numeric_limits<int64_t>::min ());
    ASSERT_TRUE (value != std::numeric_limits<int64_t>::max ());
    ASSERT_TRUE (value != std::numeric_limits<uint64_t>::min ());
    ASSERT_TRUE (value != std::numeric_limits<uint64_t>::max ());
    ASSERT_TRUE (value != std::numeric_limits<double>::min ());
    ASSERT_TRUE (value != std::numeric_limits<double>::max ());
    ASSERT_TRUE (value != "foo");
    ASSERT_FALSE (value != array.getArray ());
    ASSERT_TRUE (value != object.getObject ());

    value = object.getObject ();
    ASSERT_TRUE (value != nullptr);
    ASSERT_TRUE (value != true);
    ASSERT_TRUE (value != false);
    ASSERT_TRUE (value != std::numeric_limits<int32_t>::min ());
    ASSERT_TRUE (value != std::numeric_limits<int32_t>::max ());
    ASSERT_TRUE (value != std::numeric_limits<uint32_t>::min ());
    ASSERT_TRUE (value != std::numeric_limits<uint32_t>::max ());
    ASSERT_TRUE (value != std::numeric_limits<int64_t>::min ());
    ASSERT_TRUE (value != std::numeric_limits<int64_t>::max ());
    ASSERT_TRUE (value != std::numeric_limits<uint64_t>::min ());
    ASSERT_TRUE (value != std::numeric_limits<uint64_t>::max ());
    ASSERT_TRUE (value != std::numeric_limits<double>::min ());
    ASSERT_TRUE (value != std::numeric_limits<double>::max ());
    ASSERT_TRUE (value != "foo");
    ASSERT_TRUE (value != array.getArray ());
    ASSERT_FALSE (value != object.getObject ());

    value = nullptr;
    ASSERT_FALSE (nullptr != value);
    ASSERT_TRUE (true != value);
    ASSERT_TRUE (false != value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::min () != value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::max () != value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::min () != value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::max () != value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::min () != value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::max () != value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::min () != value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::max () != value);
    ASSERT_TRUE (std::numeric_limits<double>::min () != value);
    ASSERT_TRUE (std::numeric_limits<double>::max () != value);
    ASSERT_TRUE ("foo" != value);
    ASSERT_TRUE (array.getArray () != value);
    ASSERT_TRUE (object.getObject () != value);

    value = bool (true);
    ASSERT_TRUE (nullptr != value);
    ASSERT_FALSE (true != value);
    ASSERT_TRUE (false != value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::min () != value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::max () != value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::min () != value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::max () != value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::min () != value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::max () != value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::min () != value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::max () != value);
    ASSERT_TRUE (std::numeric_limits<double>::min () != value);
    ASSERT_TRUE (std::numeric_limits<double>::max () != value);
    ASSERT_TRUE ("foo" != value);
    ASSERT_TRUE (array.getArray () != value);
    ASSERT_TRUE (object.getObject () != value);

    value = int32_t (std::numeric_limits<int32_t>::min ());
    ASSERT_TRUE (nullptr != value);
    ASSERT_TRUE (true != value);
    ASSERT_TRUE (false != value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::min () != value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::max () != value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::min () != value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::max () != value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::min () != value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::max () != value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::min () != value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::max () != value);
    ASSERT_TRUE (std::numeric_limits<double>::min () != value);
    ASSERT_TRUE (std::numeric_limits<double>::max () != value);
    ASSERT_TRUE ("foo" != value);
    ASSERT_TRUE (array.getArray () != value);
    ASSERT_TRUE (object.getObject () != value);

    value = uint32_t (std::numeric_limits<uint32_t>::max ());
    ASSERT_TRUE (nullptr != value);
    ASSERT_TRUE (true != value);
    ASSERT_TRUE (false != value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::min () != value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::max () != value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::min () != value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::max () != value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::min () != value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::max () != value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::min () != value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::max () != value);
    ASSERT_TRUE (std::numeric_limits<double>::min () != value);
    ASSERT_TRUE (std::numeric_limits<double>::max () != value);
    ASSERT_TRUE ("foo" != value);
    ASSERT_TRUE (array.getArray () != value);
    ASSERT_TRUE (object.getObject () != value);

    value = int64_t (std::numeric_limits<int64_t>::min ());
    ASSERT_TRUE (nullptr != value);
    ASSERT_TRUE (true != value);
    ASSERT_TRUE (false != value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::min () != value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::max () != value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::min () != value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::max () != value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::min () != value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::max () != value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::min () != value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::max () != value);
    ASSERT_TRUE (std::numeric_limits<double>::min () != value);
    ASSERT_TRUE (std::numeric_limits<double>::max () != value);
    ASSERT_TRUE ("foo" != value);
    ASSERT_TRUE (array.getArray () != value);
    ASSERT_TRUE (object.getObject () != value);

    value = uint64_t (std::numeric_limits<uint64_t>::max ());
    ASSERT_TRUE (nullptr != value);
    ASSERT_TRUE (true != value);
    ASSERT_TRUE (false != value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::min () != value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::max () != value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::min () != value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::max () != value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::min () != value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::max () != value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::min () != value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::max () != value);
    ASSERT_TRUE (std::numeric_limits<double>::min () != value);
    ASSERT_TRUE (std::numeric_limits<double>::max () != value);
    ASSERT_TRUE ("foo" != value);
    ASSERT_TRUE (array.getArray () != value);
    ASSERT_TRUE (object.getObject () != value);

    value = double (std::numeric_limits<double>::min ());
    ASSERT_TRUE (nullptr != value);
    ASSERT_TRUE (true != value);
    ASSERT_TRUE (false != value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::min () != value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::max () != value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::min () != value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::max () != value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::min () != value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::max () != value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::min () != value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::max () != value);
    ASSERT_FALSE (std::numeric_limits<double>::min () != value);
    ASSERT_TRUE (std::numeric_limits<double>::max () != value);
    ASSERT_TRUE ("foo" != value);
    ASSERT_TRUE (array.getArray () != value);
    ASSERT_TRUE (object.getObject () != value);

    value = std::string ("foo");
    ASSERT_TRUE (nullptr != value);
    ASSERT_TRUE (true != value);
    ASSERT_TRUE (false != value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::min () != value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::max () != value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::min () != value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::max () != value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::min () != value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::max () != value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::min () != value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::max () != value);
    ASSERT_TRUE (std::numeric_limits<double>::min () != value);
    ASSERT_TRUE (std::numeric_limits<double>::max () != value);
    ASSERT_FALSE ("foo" != value);
    ASSERT_TRUE ("bar" != value);
    ASSERT_TRUE (array.getArray () != value);
    ASSERT_TRUE (object.getObject () != value);

    value = array.getArray ();
    ASSERT_TRUE (nullptr != value);
    ASSERT_TRUE (true != value);
    ASSERT_TRUE (false != value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::min () != value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::max () != value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::min () != value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::max () != value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::min () != value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::max () != value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::min () != value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::max () != value);
    ASSERT_TRUE (std::numeric_limits<double>::min () != value);
    ASSERT_TRUE (std::numeric_limits<double>::max () != value);
    ASSERT_TRUE ("foo" != value);
    ASSERT_FALSE (array.getArray () != value);
    ASSERT_TRUE (object.getObject () != value);

    value = object.getObject ();
    ASSERT_TRUE (nullptr != value);
    ASSERT_TRUE (true != value);
    ASSERT_TRUE (false != value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::min () != value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::max () != value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::min () != value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::max () != value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::min () != value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::max () != value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::min () != value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::max () != value);
    ASSERT_TRUE (std::numeric_limits<double>::min () != value);
    ASSERT_TRUE (std::numeric_limits<double>::max () != value);
    ASSERT_TRUE ("foo" != value);
    ASSERT_TRUE (array.getArray () != value);
    ASSERT_FALSE (object.getObject () != value);
}

/**
 * @brief Test lower operator.
 */
TEST (Value, lower)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    Value value;

    value = nullptr;
    ASSERT_FALSE (value < nullptr);
    ASSERT_TRUE (value < true);
    ASSERT_TRUE (value < false);
    ASSERT_TRUE (value < std::numeric_limits<int32_t>::min ());
    ASSERT_TRUE (value < std::numeric_limits<int32_t>::max ());
    ASSERT_TRUE (value < std::numeric_limits<uint32_t>::min ());
    ASSERT_TRUE (value < std::numeric_limits<uint32_t>::max ());
    ASSERT_TRUE (value < std::numeric_limits<int64_t>::min ());
    ASSERT_TRUE (value < std::numeric_limits<int64_t>::max ());
    ASSERT_TRUE (value < std::numeric_limits<uint64_t>::min ());
    ASSERT_TRUE (value < std::numeric_limits<uint64_t>::max ());
    ASSERT_TRUE (value < std::numeric_limits<double>::min ());
    ASSERT_TRUE (value < std::numeric_limits<double>::max ());
    ASSERT_TRUE (value < "foo");
    ASSERT_TRUE (value < array.getArray ());
    ASSERT_TRUE (value < object.getObject ());

    value = bool (true);
    ASSERT_FALSE (value < nullptr);
    ASSERT_FALSE (value < true);
    ASSERT_FALSE (value < false);
    ASSERT_TRUE (value < std::numeric_limits<int32_t>::min ());
    ASSERT_TRUE (value < std::numeric_limits<int32_t>::max ());
    ASSERT_TRUE (value < std::numeric_limits<uint32_t>::min ());
    ASSERT_TRUE (value < std::numeric_limits<uint32_t>::max ());
    ASSERT_TRUE (value < std::numeric_limits<int64_t>::min ());
    ASSERT_TRUE (value < std::numeric_limits<int64_t>::max ());
    ASSERT_TRUE (value < std::numeric_limits<uint64_t>::min ());
    ASSERT_TRUE (value < std::numeric_limits<uint64_t>::max ());
    ASSERT_TRUE (value < std::numeric_limits<double>::min ());
    ASSERT_TRUE (value < std::numeric_limits<double>::max ());
    ASSERT_TRUE (value < "foo");
    ASSERT_TRUE (value < array.getArray ());
    ASSERT_TRUE (value < object.getObject ());

    value = int32_t (std::numeric_limits<int32_t>::min ());
    ASSERT_FALSE (value < nullptr);
    ASSERT_FALSE (value < true);
    ASSERT_FALSE (value < false);
    ASSERT_FALSE (value < std::numeric_limits<int32_t>::min ());
    ASSERT_TRUE (value < std::numeric_limits<int32_t>::max ());
    ASSERT_TRUE (value < std::numeric_limits<uint32_t>::min ());
    ASSERT_TRUE (value < std::numeric_limits<uint32_t>::max ());
    ASSERT_FALSE (value < std::numeric_limits<int64_t>::min ());
    ASSERT_TRUE (value < std::numeric_limits<int64_t>::max ());
    ASSERT_TRUE (value < std::numeric_limits<uint64_t>::min ());
    ASSERT_TRUE (value < std::numeric_limits<uint64_t>::max ());
    ASSERT_TRUE (value < std::numeric_limits<double>::min ());
    ASSERT_TRUE (value < std::numeric_limits<double>::max ());
    ASSERT_TRUE (value < "foo");
    ASSERT_TRUE (value < array.getArray ());
    ASSERT_TRUE (value < object.getObject ());

    value = int64_t (std::numeric_limits<int64_t>::max ());
    ASSERT_FALSE (value < nullptr);
    ASSERT_FALSE (value < true);
    ASSERT_FALSE (value < false);
    ASSERT_FALSE (value < std::numeric_limits<int32_t>::min ());
    ASSERT_FALSE (value < std::numeric_limits<int32_t>::max ());
    ASSERT_FALSE (value < std::numeric_limits<uint32_t>::min ());
    ASSERT_FALSE (value < std::numeric_limits<uint32_t>::max ());
    ASSERT_FALSE (value < std::numeric_limits<int64_t>::min ());
    ASSERT_FALSE (value < std::numeric_limits<int64_t>::max ());
    ASSERT_FALSE (value < std::numeric_limits<uint64_t>::min ());
    ASSERT_TRUE (value < std::numeric_limits<uint64_t>::max ());
    ASSERT_FALSE (value < std::numeric_limits<double>::min ());
    ASSERT_TRUE (value < std::numeric_limits<double>::max ());
    ASSERT_TRUE (value < "foo");
    ASSERT_TRUE (value < array.getArray ());
    ASSERT_TRUE (value < object.getObject ());

    value = double (std::numeric_limits<double>::min ());
    ASSERT_FALSE (value < nullptr);
    ASSERT_FALSE (value < true);
    ASSERT_FALSE (value < false);
    ASSERT_FALSE (value < std::numeric_limits<int32_t>::min ());
    ASSERT_TRUE (value < std::numeric_limits<int32_t>::max ());
    ASSERT_FALSE (value < std::numeric_limits<uint32_t>::min ());
    ASSERT_TRUE (value < std::numeric_limits<uint32_t>::max ());
    ASSERT_FALSE (value < std::numeric_limits<int64_t>::min ());
    ASSERT_TRUE (value < std::numeric_limits<int64_t>::max ());
    ASSERT_FALSE (value < std::numeric_limits<uint64_t>::min ());
    ASSERT_TRUE (value < std::numeric_limits<uint64_t>::max ());
    ASSERT_FALSE (value < std::numeric_limits<double>::min ());
    ASSERT_TRUE (value < std::numeric_limits<double>::max ());
    ASSERT_TRUE (value < "foo");
    ASSERT_TRUE (value < array.getArray ());
    ASSERT_TRUE (value < object.getObject ());

    value = std::string ("foo");
    ASSERT_FALSE (value < nullptr);
    ASSERT_FALSE (value < true);
    ASSERT_FALSE (value < false);
    ASSERT_FALSE (value < std::numeric_limits<int32_t>::min ());
    ASSERT_FALSE (value < std::numeric_limits<int32_t>::max ());
    ASSERT_FALSE (value < std::numeric_limits<uint32_t>::min ());
    ASSERT_FALSE (value < std::numeric_limits<uint32_t>::max ());
    ASSERT_FALSE (value < std::numeric_limits<int64_t>::min ());
    ASSERT_FALSE (value < std::numeric_limits<int64_t>::max ());
    ASSERT_FALSE (value < std::numeric_limits<uint64_t>::min ());
    ASSERT_FALSE (value < std::numeric_limits<uint64_t>::max ());
    ASSERT_FALSE (value < std::numeric_limits<double>::min ());
    ASSERT_FALSE (value < std::numeric_limits<double>::max ());
    ASSERT_FALSE (value < "foo");
    ASSERT_TRUE (value < array.getArray ());
    ASSERT_TRUE (value < object.getObject ());

    value = array.getArray ();
    ASSERT_FALSE (value < nullptr);
    ASSERT_FALSE (value < true);
    ASSERT_FALSE (value < false);
    ASSERT_FALSE (value < std::numeric_limits<int32_t>::min ());
    ASSERT_FALSE (value < std::numeric_limits<int32_t>::max ());
    ASSERT_FALSE (value < std::numeric_limits<uint32_t>::min ());
    ASSERT_FALSE (value < std::numeric_limits<uint32_t>::max ());
    ASSERT_FALSE (value < std::numeric_limits<int64_t>::min ());
    ASSERT_FALSE (value < std::numeric_limits<int64_t>::max ());
    ASSERT_FALSE (value < std::numeric_limits<uint64_t>::min ());
    ASSERT_FALSE (value < std::numeric_limits<uint64_t>::max ());
    ASSERT_FALSE (value < std::numeric_limits<double>::min ());
    ASSERT_FALSE (value < std::numeric_limits<double>::max ());
    ASSERT_FALSE (value < "foo");
    ASSERT_FALSE (value < array.getArray ());
    ASSERT_TRUE (value < object.getObject ());

    value = object.getObject ();
    ASSERT_FALSE (value < nullptr);
    ASSERT_FALSE (value < true);
    ASSERT_FALSE (value < false);
    ASSERT_FALSE (value < std::numeric_limits<int32_t>::min ());
    ASSERT_FALSE (value < std::numeric_limits<int32_t>::max ());
    ASSERT_FALSE (value < std::numeric_limits<uint32_t>::min ());
    ASSERT_FALSE (value < std::numeric_limits<uint32_t>::max ());
    ASSERT_FALSE (value < std::numeric_limits<int64_t>::min ());
    ASSERT_FALSE (value < std::numeric_limits<int64_t>::max ());
    ASSERT_FALSE (value < std::numeric_limits<uint64_t>::min ());
    ASSERT_FALSE (value < std::numeric_limits<uint64_t>::max ());
    ASSERT_FALSE (value < std::numeric_limits<double>::min ());
    ASSERT_FALSE (value < std::numeric_limits<double>::max ());
    ASSERT_FALSE (value < "foo");
    ASSERT_FALSE (value < array.getArray ());
    ASSERT_FALSE (value < object.getObject ());

    value = nullptr;
    ASSERT_FALSE (nullptr < value);
    ASSERT_FALSE (true < value);
    ASSERT_FALSE (false < value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::min () < value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::max () < value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::min () < value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::max () < value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::min () < value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::max () < value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::min () < value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::max () < value);
    ASSERT_FALSE (std::numeric_limits<double>::min () < value);
    ASSERT_FALSE (std::numeric_limits<double>::max () < value);
    ASSERT_FALSE ("foo" < value);
    ASSERT_FALSE (array.getArray () < value);
    ASSERT_FALSE (object.getObject () < value);

    value = bool (true);
    ASSERT_TRUE (nullptr < value);
    ASSERT_FALSE (true < value);
    ASSERT_TRUE (false < value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::min () < value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::max () < value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::min () < value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::max () < value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::min () < value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::max () < value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::min () < value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::max () < value);
    ASSERT_FALSE (std::numeric_limits<double>::min () < value);
    ASSERT_FALSE (std::numeric_limits<double>::max () < value);
    ASSERT_FALSE ("foo" < value);
    ASSERT_FALSE (array.getArray () < value);
    ASSERT_FALSE (object.getObject () < value);

    value = int32_t (std::numeric_limits<int32_t>::min ());
    ASSERT_TRUE (nullptr < value);
    ASSERT_TRUE (true < value);
    ASSERT_TRUE (false < value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::min () < value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::max () < value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::min () < value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::max () < value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::min () < value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::max () < value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::min () < value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::max () < value);
    ASSERT_FALSE (std::numeric_limits<double>::min () < value);
    ASSERT_FALSE (std::numeric_limits<double>::max () < value);
    ASSERT_FALSE ("foo" < value);
    ASSERT_FALSE (array.getArray () < value);
    ASSERT_FALSE (object.getObject () < value);

    value = int64_t (std::numeric_limits<int64_t>::max ());
    ASSERT_TRUE (nullptr < value);
    ASSERT_TRUE (true < value);
    ASSERT_TRUE (false < value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::min () < value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::max () < value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::min () < value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::max () < value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::min () < value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::max () < value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::min () < value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::max () < value);
    ASSERT_TRUE (std::numeric_limits<double>::min () < value);
    ASSERT_FALSE (std::numeric_limits<double>::max () < value);
    ASSERT_FALSE ("foo" < value);
    ASSERT_FALSE (array.getArray () < value);
    ASSERT_FALSE (object.getObject () < value);

    value = double (std::numeric_limits<double>::min ());
    ASSERT_TRUE (nullptr < value);
    ASSERT_TRUE (true < value);
    ASSERT_TRUE (false < value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::min () < value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::max () < value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::min () < value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::max () < value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::min () < value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::max () < value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::min () < value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::max () < value);
    ASSERT_FALSE (std::numeric_limits<double>::min () < value);
    ASSERT_FALSE (std::numeric_limits<double>::max () < value);
    ASSERT_FALSE ("foo" < value);
    ASSERT_FALSE (array.getArray () < value);
    ASSERT_FALSE (object.getObject () < value);

    value = std::string ("foo");
    ASSERT_TRUE (nullptr < value);
    ASSERT_TRUE (true < value);
    ASSERT_TRUE (false < value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::min () < value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::max () < value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::min () < value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::max () < value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::min () < value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::max () < value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::min () < value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::max () < value);
    ASSERT_TRUE (std::numeric_limits<double>::min () < value);
    ASSERT_TRUE (std::numeric_limits<double>::max () < value);
    ASSERT_FALSE ("foo" < value);
    ASSERT_FALSE (array.getArray () < value);
    ASSERT_FALSE (object.getObject () < value);

    value = array.getArray ();
    ASSERT_TRUE (nullptr < value);
    ASSERT_TRUE (true < value);
    ASSERT_TRUE (false < value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::min () < value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::max () < value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::min () < value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::max () < value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::min () < value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::max () < value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::min () < value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::max () < value);
    ASSERT_TRUE (std::numeric_limits<double>::min () < value);
    ASSERT_TRUE (std::numeric_limits<double>::max () < value);
    ASSERT_TRUE ("foo" < value);
    ASSERT_FALSE (array.getArray () < value);
    ASSERT_FALSE (object.getObject () < value);

    value = object.getObject ();
    ASSERT_TRUE (nullptr < value);
    ASSERT_TRUE (true < value);
    ASSERT_TRUE (false < value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::min () < value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::max () < value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::min () < value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::max () < value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::min () < value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::max () < value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::min () < value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::max () < value);
    ASSERT_TRUE (std::numeric_limits<double>::min () < value);
    ASSERT_TRUE (std::numeric_limits<double>::max () < value);
    ASSERT_TRUE ("foo" < value);
    ASSERT_TRUE (array.getArray () < value);
    ASSERT_FALSE (object.getObject () < value);

    const uint8_t bytes[] = {0x00, 0x01, 0xff};
    value = Value (std::span<const uint8_t> (bytes, sizeof (bytes)), 0);
    ASSERT_TRUE (Value (std::span<const uint8_t> (bytes, sizeof (bytes))) < value);
    ASSERT_TRUE (Value (std::span<const uint8_t> (bytes, sizeof (bytes)), -1) < value);
    ASSERT_FALSE (Value (std::span<const uint8_t> (bytes, sizeof (bytes)), 1) < value);
}

/**
 * @brief Test greater operator.
 */
TEST (Value, greater)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    Value value;

    value = nullptr;
    ASSERT_FALSE (value > nullptr);
    ASSERT_FALSE (value > true);
    ASSERT_FALSE (value > false);
    ASSERT_FALSE (value > std::numeric_limits<int32_t>::min ());
    ASSERT_FALSE (value > std::numeric_limits<int32_t>::max ());
    ASSERT_FALSE (value > std::numeric_limits<uint32_t>::min ());
    ASSERT_FALSE (value > std::numeric_limits<uint32_t>::max ());
    ASSERT_FALSE (value > std::numeric_limits<int64_t>::min ());
    ASSERT_FALSE (value > std::numeric_limits<int64_t>::max ());
    ASSERT_FALSE (value > std::numeric_limits<uint64_t>::min ());
    ASSERT_FALSE (value > std::numeric_limits<uint64_t>::max ());
    ASSERT_FALSE (value > std::numeric_limits<double>::min ());
    ASSERT_FALSE (value > std::numeric_limits<double>::max ());
    ASSERT_FALSE (value > "foo");
    ASSERT_FALSE (value > array.getArray ());
    ASSERT_FALSE (value > object.getObject ());

    value = bool (true);
    ASSERT_TRUE (value > nullptr);
    ASSERT_FALSE (value > true);
    ASSERT_TRUE (value > false);
    ASSERT_FALSE (value > std::numeric_limits<int32_t>::min ());
    ASSERT_FALSE (value > std::numeric_limits<int32_t>::max ());
    ASSERT_FALSE (value > std::numeric_limits<uint32_t>::min ());
    ASSERT_FALSE (value > std::numeric_limits<uint32_t>::max ());
    ASSERT_FALSE (value > std::numeric_limits<int64_t>::min ());
    ASSERT_FALSE (value > std::numeric_limits<int64_t>::max ());
    ASSERT_FALSE (value > std::numeric_limits<uint64_t>::min ());
    ASSERT_FALSE (value > std::numeric_limits<uint64_t>::max ());
    ASSERT_FALSE (value > std::numeric_limits<double>::min ());
    ASSERT_FALSE (value > std::numeric_limits<double>::max ());
    ASSERT_FALSE (value > "foo");
    ASSERT_FALSE (value > array.getArray ());
    ASSERT_FALSE (value > object.getObject ());

    value = int32_t (std::numeric_limits<int32_t>::min ());
    ASSERT_TRUE (value > nullptr);
    ASSERT_TRUE (value > true);
    ASSERT_TRUE (value > false);
    ASSERT_FALSE (value > std::numeric_limits<int32_t>::min ());
    ASSERT_FALSE (value > std::numeric_limits<int32_t>::max ());
    ASSERT_FALSE (value > std::numeric_limits<uint32_t>::min ());
    ASSERT_FALSE (value > std::numeric_limits<uint32_t>::max ());
    ASSERT_TRUE (value > std::numeric_limits<int64_t>::min ());
    ASSERT_FALSE (value > std::numeric_limits<int64_t>::max ());
    ASSERT_FALSE (value > std::numeric_limits<uint64_t>::min ());
    ASSERT_FALSE (value > std::numeric_limits<uint64_t>::max ());
    ASSERT_FALSE (value > std::numeric_limits<double>::min ());
    ASSERT_FALSE (value > std::numeric_limits<double>::max ());
    ASSERT_FALSE (value > "foo");
    ASSERT_FALSE (value > array.getArray ());
    ASSERT_FALSE (value > object.getObject ());

    value = int64_t (std::numeric_limits<int64_t>::max ());
    ASSERT_TRUE (value > nullptr);
    ASSERT_TRUE (value > true);
    ASSERT_TRUE (value > false);
    ASSERT_TRUE (value > std::numeric_limits<int32_t>::min ());
    ASSERT_TRUE (value > std::numeric_limits<int32_t>::max ());
    ASSERT_TRUE (value > std::numeric_limits<uint32_t>::min ());
    ASSERT_TRUE (value > std::numeric_limits<uint32_t>::max ());
    ASSERT_TRUE (value > std::numeric_limits<int64_t>::min ());
    ASSERT_FALSE (value > std::numeric_limits<int64_t>::max ());
    ASSERT_TRUE (value > std::numeric_limits<uint64_t>::min ());
    ASSERT_FALSE (value > std::numeric_limits<uint64_t>::max ());
    ASSERT_TRUE (value > std::numeric_limits<double>::min ());
    ASSERT_FALSE (value > std::numeric_limits<double>::max ());
    ASSERT_FALSE (value > "foo");
    ASSERT_FALSE (value > array.getArray ());
    ASSERT_FALSE (value > object.getObject ());

    value = double (std::numeric_limits<double>::min ());
    ASSERT_TRUE (value > nullptr);
    ASSERT_TRUE (value > true);
    ASSERT_TRUE (value > false);
    ASSERT_TRUE (value > std::numeric_limits<int32_t>::min ());
    ASSERT_FALSE (value > std::numeric_limits<int32_t>::max ());
    ASSERT_TRUE (value > std::numeric_limits<uint32_t>::min ());
    ASSERT_FALSE (value > std::numeric_limits<uint32_t>::max ());
    ASSERT_TRUE (value > std::numeric_limits<int64_t>::min ());
    ASSERT_FALSE (value > std::numeric_limits<int64_t>::max ());
    ASSERT_TRUE (value > std::numeric_limits<uint64_t>::min ());
    ASSERT_FALSE (value > std::numeric_limits<uint64_t>::max ());
    ASSERT_FALSE (value > std::numeric_limits<double>::min ());
    ASSERT_FALSE (value > std::numeric_limits<double>::max ());
    ASSERT_FALSE (value > "foo");
    ASSERT_FALSE (value > array.getArray ());
    ASSERT_FALSE (value > object.getObject ());

    value = std::string ("foo");
    ASSERT_TRUE (value > nullptr);
    ASSERT_TRUE (value > true);
    ASSERT_TRUE (value > false);
    ASSERT_TRUE (value > std::numeric_limits<int32_t>::min ());
    ASSERT_TRUE (value > std::numeric_limits<int32_t>::max ());
    ASSERT_TRUE (value > std::numeric_limits<uint32_t>::min ());
    ASSERT_TRUE (value > std::numeric_limits<uint32_t>::max ());
    ASSERT_TRUE (value > std::numeric_limits<int64_t>::min ());
    ASSERT_TRUE (value > std::numeric_limits<int64_t>::max ());
    ASSERT_TRUE (value > std::numeric_limits<uint64_t>::min ());
    ASSERT_TRUE (value > std::numeric_limits<uint64_t>::max ());
    ASSERT_TRUE (value > std::numeric_limits<double>::min ());
    ASSERT_TRUE (value > std::numeric_limits<double>::max ());
    ASSERT_FALSE (value > "foo");
    ASSERT_FALSE (value > array.getArray ());
    ASSERT_FALSE (value > object.getObject ());

    value = array.getArray ();
    ASSERT_TRUE (value > nullptr);
    ASSERT_TRUE (value > true);
    ASSERT_TRUE (value > false);
    ASSERT_TRUE (value > std::numeric_limits<int32_t>::min ());
    ASSERT_TRUE (value > std::numeric_limits<int32_t>::max ());
    ASSERT_TRUE (value > std::numeric_limits<uint32_t>::min ());
    ASSERT_TRUE (value > std::numeric_limits<uint32_t>::max ());
    ASSERT_TRUE (value > std::numeric_limits<int64_t>::min ());
    ASSERT_TRUE (value > std::numeric_limits<int64_t>::max ());
    ASSERT_TRUE (value > std::numeric_limits<uint64_t>::min ());
    ASSERT_TRUE (value > std::numeric_limits<uint64_t>::max ());
    ASSERT_TRUE (value > std::numeric_limits<double>::min ());
    ASSERT_TRUE (value > std::numeric_limits<double>::max ());
    ASSERT_TRUE (value > "foo");
    ASSERT_FALSE (value > array.getArray ());
    ASSERT_FALSE (value > object.getObject ());

    value = object.getObject ();
    ASSERT_TRUE (value > nullptr);
    ASSERT_TRUE (value > true);
    ASSERT_TRUE (value > false);
    ASSERT_TRUE (value > std::numeric_limits<int32_t>::min ());
    ASSERT_TRUE (value > std::numeric_limits<int32_t>::max ());
    ASSERT_TRUE (value > std::numeric_limits<uint32_t>::min ());
    ASSERT_TRUE (value > std::numeric_limits<uint32_t>::max ());
    ASSERT_TRUE (value > std::numeric_limits<int64_t>::min ());
    ASSERT_TRUE (value > std::numeric_limits<int64_t>::max ());
    ASSERT_TRUE (value > std::numeric_limits<uint64_t>::min ());
    ASSERT_TRUE (value > std::numeric_limits<uint64_t>::max ());
    ASSERT_TRUE (value > std::numeric_limits<double>::min ());
    ASSERT_TRUE (value > std::numeric_limits<double>::max ());
    ASSERT_TRUE (value > "foo");
    ASSERT_TRUE (value > array.getArray ());
    ASSERT_FALSE (value > object.getObject ());

    value = nullptr;
    ASSERT_FALSE (nullptr > value);
    ASSERT_TRUE (true > value);
    ASSERT_TRUE (false > value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::min () > value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::max () > value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::min () > value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::max () > value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::min () > value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::max () > value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::min () > value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::max () > value);
    ASSERT_TRUE (std::numeric_limits<double>::min () > value);
    ASSERT_TRUE (std::numeric_limits<double>::max () > value);
    ASSERT_TRUE ("foo" > value);
    ASSERT_TRUE (array.getArray () > value);
    ASSERT_TRUE (object.getObject () > value);

    value = bool (true);
    ASSERT_FALSE (nullptr > value);
    ASSERT_FALSE (true > value);
    ASSERT_FALSE (false > value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::min () > value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::max () > value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::min () > value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::max () > value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::min () > value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::max () > value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::min () > value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::max () > value);
    ASSERT_TRUE (std::numeric_limits<double>::min () > value);
    ASSERT_TRUE (std::numeric_limits<double>::max () > value);
    ASSERT_TRUE ("foo" > value);
    ASSERT_TRUE (array.getArray () > value);
    ASSERT_TRUE (object.getObject () > value);

    value = int32_t (std::numeric_limits<int32_t>::min ());
    ASSERT_FALSE (nullptr > value);
    ASSERT_FALSE (true > value);
    ASSERT_FALSE (false > value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::min () > value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::max () > value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::min () > value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::max () > value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::min () > value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::max () > value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::min () > value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::max () > value);
    ASSERT_TRUE (std::numeric_limits<double>::min () > value);
    ASSERT_TRUE (std::numeric_limits<double>::max () > value);
    ASSERT_TRUE ("foo" > value);
    ASSERT_TRUE (array.getArray () > value);
    ASSERT_TRUE (object.getObject () > value);

    value = int64_t (std::numeric_limits<int64_t>::max ());
    ASSERT_FALSE (nullptr > value);
    ASSERT_FALSE (true > value);
    ASSERT_FALSE (false > value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::min () > value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::max () > value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::min () > value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::max () > value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::min () > value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::max () > value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::min () > value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::max () > value);
    ASSERT_FALSE (std::numeric_limits<double>::min () > value);
    ASSERT_TRUE (std::numeric_limits<double>::max () > value);
    ASSERT_TRUE ("foo" > value);
    ASSERT_TRUE (array.getArray () > value);
    ASSERT_TRUE (object.getObject () > value);

    value = double (std::numeric_limits<double>::min ());
    ASSERT_FALSE (nullptr > value);
    ASSERT_FALSE (true > value);
    ASSERT_FALSE (false > value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::min () > value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::max () > value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::min () > value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::max () > value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::min () > value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::max () > value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::min () > value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::max () > value);
    ASSERT_FALSE (std::numeric_limits<double>::min () > value);
    ASSERT_TRUE (std::numeric_limits<double>::max () > value);
    ASSERT_TRUE ("foo" > value);
    ASSERT_TRUE (array.getArray () > value);
    ASSERT_TRUE (object.getObject () > value);

    value = std::string ("foo");
    ASSERT_FALSE (nullptr > value);
    ASSERT_FALSE (true > value);
    ASSERT_FALSE (false > value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::min () > value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::max () > value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::min () > value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::max () > value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::min () > value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::max () > value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::min () > value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::max () > value);
    ASSERT_FALSE (std::numeric_limits<double>::min () > value);
    ASSERT_FALSE (std::numeric_limits<double>::max () > value);
    ASSERT_FALSE ("foo" > value);
    ASSERT_TRUE (array.getArray () > value);
    ASSERT_TRUE (object.getObject () > value);

    value = array.getArray ();
    ASSERT_FALSE (nullptr > value);
    ASSERT_FALSE (true > value);
    ASSERT_FALSE (false > value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::min () > value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::max () > value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::min () > value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::max () > value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::min () > value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::max () > value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::min () > value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::max () > value);
    ASSERT_FALSE (std::numeric_limits<double>::min () > value);
    ASSERT_FALSE (std::numeric_limits<double>::max () > value);
    ASSERT_FALSE ("foo" > value);
    ASSERT_FALSE (array.getArray () > value);
    ASSERT_TRUE (object.getObject () > value);

    value = object.getObject ();
    ASSERT_FALSE (nullptr > value);
    ASSERT_FALSE (true > value);
    ASSERT_FALSE (false > value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::min () > value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::max () > value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::min () > value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::max () > value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::min () > value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::max () > value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::min () > value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::max () > value);
    ASSERT_FALSE (std::numeric_limits<double>::min () > value);
    ASSERT_FALSE (std::numeric_limits<double>::max () > value);
    ASSERT_FALSE ("foo" > value);
    ASSERT_FALSE (array.getArray () > value);
    ASSERT_FALSE (object.getObject () > value);
}

/**
 * @brief Test lowerOrEqual operator.
 */
TEST (Value, lowerOrEqual)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    Value value;

    value = nullptr;
    ASSERT_TRUE (value <= nullptr);
    ASSERT_TRUE (value <= true);
    ASSERT_TRUE (value <= false);
    ASSERT_TRUE (value <= std::numeric_limits<int32_t>::min ());
    ASSERT_TRUE (value <= std::numeric_limits<int32_t>::max ());
    ASSERT_TRUE (value <= std::numeric_limits<uint32_t>::min ());
    ASSERT_TRUE (value <= std::numeric_limits<uint32_t>::max ());
    ASSERT_TRUE (value <= std::numeric_limits<int64_t>::min ());
    ASSERT_TRUE (value <= std::numeric_limits<int64_t>::max ());
    ASSERT_TRUE (value <= std::numeric_limits<uint64_t>::min ());
    ASSERT_TRUE (value <= std::numeric_limits<uint64_t>::max ());
    ASSERT_TRUE (value <= std::numeric_limits<double>::min ());
    ASSERT_TRUE (value <= std::numeric_limits<double>::max ());
    ASSERT_TRUE (value <= "foo");
    ASSERT_TRUE (value <= array.getArray ());
    ASSERT_TRUE (value <= object.getObject ());

    value = bool (true);
    ASSERT_FALSE (value <= nullptr);
    ASSERT_TRUE (value <= true);
    ASSERT_FALSE (value <= false);
    ASSERT_TRUE (value <= std::numeric_limits<int32_t>::min ());
    ASSERT_TRUE (value <= std::numeric_limits<int32_t>::max ());
    ASSERT_TRUE (value <= std::numeric_limits<uint32_t>::min ());
    ASSERT_TRUE (value <= std::numeric_limits<uint32_t>::max ());
    ASSERT_TRUE (value <= std::numeric_limits<int64_t>::min ());
    ASSERT_TRUE (value <= std::numeric_limits<int64_t>::max ());
    ASSERT_TRUE (value <= std::numeric_limits<uint64_t>::min ());
    ASSERT_TRUE (value <= std::numeric_limits<uint64_t>::max ());
    ASSERT_TRUE (value <= std::numeric_limits<double>::min ());
    ASSERT_TRUE (value <= std::numeric_limits<double>::max ());
    ASSERT_TRUE (value <= "foo");
    ASSERT_TRUE (value <= array.getArray ());
    ASSERT_TRUE (value <= object.getObject ());

    value = int32_t (std::numeric_limits<int32_t>::min ());
    ASSERT_FALSE (value <= nullptr);
    ASSERT_FALSE (value <= true);
    ASSERT_FALSE (value <= false);
    ASSERT_TRUE (value <= std::numeric_limits<int32_t>::min ());
    ASSERT_TRUE (value <= std::numeric_limits<int32_t>::max ());
    ASSERT_TRUE (value <= std::numeric_limits<uint32_t>::min ());
    ASSERT_TRUE (value <= std::numeric_limits<uint32_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits<int64_t>::min ());
    ASSERT_TRUE (value <= std::numeric_limits<int64_t>::max ());
    ASSERT_TRUE (value <= std::numeric_limits<uint64_t>::min ());
    ASSERT_TRUE (value <= std::numeric_limits<uint64_t>::max ());
    ASSERT_TRUE (value <= std::numeric_limits<double>::min ());
    ASSERT_TRUE (value <= std::numeric_limits<double>::max ());
    ASSERT_TRUE (value <= "foo");
    ASSERT_TRUE (value <= array.getArray ());
    ASSERT_TRUE (value <= object.getObject ());

    value = uint32_t (std::numeric_limits<uint32_t>::max ());
    ASSERT_FALSE (value <= nullptr);
    ASSERT_FALSE (value <= true);
    ASSERT_FALSE (value <= false);
    ASSERT_FALSE (value <= std::numeric_limits<int32_t>::min ());
    ASSERT_FALSE (value <= std::numeric_limits<int32_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits<uint32_t>::min ());
    ASSERT_TRUE (value <= std::numeric_limits<uint32_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits<int64_t>::min ());
    ASSERT_TRUE (value <= std::numeric_limits<int64_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits<uint64_t>::min ());
    ASSERT_TRUE (value <= std::numeric_limits<uint64_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits<double>::min ());
    ASSERT_TRUE (value <= std::numeric_limits<double>::max ());
    ASSERT_TRUE (value <= "foo");
    ASSERT_TRUE (value <= array.getArray ());
    ASSERT_TRUE (value <= object.getObject ());

    value = int64_t (std::numeric_limits<int64_t>::min ());
    ASSERT_FALSE (value <= nullptr);
    ASSERT_FALSE (value <= true);
    ASSERT_FALSE (value <= false);
    ASSERT_TRUE (value <= std::numeric_limits<int32_t>::min ());
    ASSERT_TRUE (value <= std::numeric_limits<int32_t>::max ());
    ASSERT_TRUE (value <= std::numeric_limits<uint32_t>::min ());
    ASSERT_TRUE (value <= std::numeric_limits<uint32_t>::max ());
    ASSERT_TRUE (value <= std::numeric_limits<int64_t>::min ());
    ASSERT_TRUE (value <= std::numeric_limits<int64_t>::max ());
    ASSERT_TRUE (value <= std::numeric_limits<uint64_t>::min ());
    ASSERT_TRUE (value <= std::numeric_limits<uint64_t>::max ());
    ASSERT_TRUE (value <= std::numeric_limits<double>::min ());
    ASSERT_TRUE (value <= std::numeric_limits<double>::max ());
    ASSERT_TRUE (value <= "foo");
    ASSERT_TRUE (value <= array.getArray ());
    ASSERT_TRUE (value <= object.getObject ());

    value = uint64_t (std::numeric_limits<uint64_t>::max ());
    ASSERT_FALSE (value <= nullptr);
    ASSERT_FALSE (value <= true);
    ASSERT_FALSE (value <= false);
    ASSERT_FALSE (value <= std::numeric_limits<int32_t>::min ());
    ASSERT_FALSE (value <= std::numeric_limits<int32_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits<uint32_t>::min ());
    ASSERT_FALSE (value <= std::numeric_limits<uint32_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits<int64_t>::min ());
    ASSERT_FALSE (value <= std::numeric_limits<int64_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits<uint64_t>::min ());
    ASSERT_TRUE (value <= std::numeric_limits<uint64_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits<double>::min ());
    ASSERT_TRUE (value <= std::numeric_limits<double>::max ());
    ASSERT_TRUE (value <= "foo");
    ASSERT_TRUE (value <= array.getArray ());
    ASSERT_TRUE (value <= object.getObject ());

    value = double (std::numeric_limits<double>::min ());
    ASSERT_FALSE (value <= nullptr);
    ASSERT_FALSE (value <= true);
    ASSERT_FALSE (value <= false);
    ASSERT_FALSE (value <= std::numeric_limits<int32_t>::min ());
    ASSERT_TRUE (value <= std::numeric_limits<int32_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits<uint32_t>::min ());
    ASSERT_TRUE (value <= std::numeric_limits<uint32_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits<int64_t>::min ());
    ASSERT_TRUE (value <= std::numeric_limits<int64_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits<uint64_t>::min ());
    ASSERT_TRUE (value <= std::numeric_limits<uint64_t>::max ());
    ASSERT_TRUE (value <= std::numeric_limits<double>::min ());
    ASSERT_TRUE (value <= std::numeric_limits<double>::max ());
    ASSERT_TRUE (value <= "foo");
    ASSERT_TRUE (value <= array.getArray ());
    ASSERT_TRUE (value <= object.getObject ());

    value = std::string ("foo");
    ASSERT_FALSE (value <= nullptr);
    ASSERT_FALSE (value <= true);
    ASSERT_FALSE (value <= false);
    ASSERT_FALSE (value <= std::numeric_limits<int32_t>::min ());
    ASSERT_FALSE (value <= std::numeric_limits<int32_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits<uint32_t>::min ());
    ASSERT_FALSE (value <= std::numeric_limits<uint32_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits<int64_t>::min ());
    ASSERT_FALSE (value <= std::numeric_limits<int64_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits<uint64_t>::min ());
    ASSERT_FALSE (value <= std::numeric_limits<uint64_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits<double>::min ());
    ASSERT_FALSE (value <= std::numeric_limits<double>::max ());
    ASSERT_TRUE (value <= "foo");
    ASSERT_TRUE (value <= array.getArray ());
    ASSERT_TRUE (value <= object.getObject ());

    value = array.getArray ();
    ASSERT_FALSE (value <= nullptr);
    ASSERT_FALSE (value <= true);
    ASSERT_FALSE (value <= false);
    ASSERT_FALSE (value <= std::numeric_limits<int32_t>::min ());
    ASSERT_FALSE (value <= std::numeric_limits<int32_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits<uint32_t>::min ());
    ASSERT_FALSE (value <= std::numeric_limits<uint32_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits<int64_t>::min ());
    ASSERT_FALSE (value <= std::numeric_limits<int64_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits<uint64_t>::min ());
    ASSERT_FALSE (value <= std::numeric_limits<uint64_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits<double>::min ());
    ASSERT_FALSE (value <= std::numeric_limits<double>::max ());
    ASSERT_FALSE (value <= "foo");
    ASSERT_TRUE (value <= array.getArray ());
    ASSERT_TRUE (value <= object.getObject ());

    value = object.getObject ();
    ASSERT_FALSE (value <= nullptr);
    ASSERT_FALSE (value <= true);
    ASSERT_FALSE (value <= false);
    ASSERT_FALSE (value <= std::numeric_limits<int32_t>::min ());
    ASSERT_FALSE (value <= std::numeric_limits<int32_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits<uint32_t>::min ());
    ASSERT_FALSE (value <= std::numeric_limits<uint32_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits<int64_t>::min ());
    ASSERT_FALSE (value <= std::numeric_limits<int64_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits<uint64_t>::min ());
    ASSERT_FALSE (value <= std::numeric_limits<uint64_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits<double>::min ());
    ASSERT_FALSE (value <= std::numeric_limits<double>::max ());
    ASSERT_FALSE (value <= "foo");
    ASSERT_FALSE (value <= array.getArray ());
    ASSERT_TRUE (value <= object.getObject ());

    value = nullptr;
    ASSERT_TRUE (nullptr <= value);
    ASSERT_FALSE (true <= value);
    ASSERT_FALSE (false <= value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::max () <= value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::max () <= value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::max () <= value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::max () <= value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::max () <= value);
    ASSERT_FALSE (std::numeric_limits<double>::min () <= value);
    ASSERT_FALSE (std::numeric_limits<double>::max () <= value);
    ASSERT_FALSE ("foo" <= value);
    ASSERT_FALSE (array.getArray () <= value);
    ASSERT_FALSE (object.getObject () <= value);

    value = bool (true);
    ASSERT_TRUE (nullptr <= value);
    ASSERT_TRUE (true <= value);
    ASSERT_TRUE (false <= value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::max () <= value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::max () <= value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::max () <= value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::max () <= value);
    ASSERT_FALSE (std::numeric_limits<double>::min () <= value);
    ASSERT_FALSE (std::numeric_limits<double>::max () <= value);
    ASSERT_FALSE ("foo" <= value);
    ASSERT_FALSE (array.getArray () <= value);
    ASSERT_FALSE (object.getObject () <= value);

    value = int32_t (std::numeric_limits<int32_t>::min ());
    ASSERT_TRUE (nullptr <= value);
    ASSERT_TRUE (true <= value);
    ASSERT_TRUE (false <= value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::max () <= value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::max () <= value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::max () <= value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::max () <= value);
    ASSERT_FALSE (std::numeric_limits<double>::min () <= value);
    ASSERT_FALSE (std::numeric_limits<double>::max () <= value);
    ASSERT_FALSE ("foo" <= value);
    ASSERT_FALSE (array.getArray () <= value);
    ASSERT_FALSE (object.getObject () <= value);

    value = uint32_t (std::numeric_limits<uint32_t>::max ());
    ASSERT_TRUE (nullptr <= value);
    ASSERT_TRUE (true <= value);
    ASSERT_TRUE (false <= value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::min () <= value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::max () <= value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::min () <= value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::max () <= value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::max () <= value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::max () <= value);
    ASSERT_TRUE (std::numeric_limits<double>::min () <= value);
    ASSERT_FALSE (std::numeric_limits<double>::max () <= value);
    ASSERT_FALSE ("foo" <= value);
    ASSERT_FALSE (array.getArray () <= value);
    ASSERT_FALSE (object.getObject () <= value);

    value = int64_t (std::numeric_limits<int64_t>::min ());
    ASSERT_TRUE (nullptr <= value);
    ASSERT_TRUE (true <= value);
    ASSERT_TRUE (false <= value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::max () <= value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::max () <= value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::max () <= value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::max () <= value);
    ASSERT_FALSE (std::numeric_limits<double>::min () <= value);
    ASSERT_FALSE (std::numeric_limits<double>::max () <= value);
    ASSERT_FALSE ("foo" <= value);
    ASSERT_FALSE (array.getArray () <= value);
    ASSERT_FALSE (object.getObject () <= value);

    value = uint64_t (std::numeric_limits<uint64_t>::max ());
    ASSERT_TRUE (nullptr <= value);
    ASSERT_TRUE (true <= value);
    ASSERT_TRUE (false <= value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::min () <= value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::max () <= value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::min () <= value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::max () <= value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::min () <= value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::max () <= value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::min () <= value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::max () <= value);
    ASSERT_TRUE (std::numeric_limits<double>::min () <= value);
    ASSERT_FALSE (std::numeric_limits<double>::max () <= value);
    ASSERT_FALSE ("foo" <= value);
    ASSERT_FALSE (array.getArray () <= value);
    ASSERT_FALSE (object.getObject () <= value);

    value = double (std::numeric_limits<double>::min ());
    ASSERT_TRUE (nullptr <= value);
    ASSERT_TRUE (true <= value);
    ASSERT_TRUE (false <= value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::max () <= value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::max () <= value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::max () <= value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::max () <= value);
    ASSERT_TRUE (std::numeric_limits<double>::min () <= value);
    ASSERT_FALSE (std::numeric_limits<double>::max () <= value);
    ASSERT_FALSE ("foo" <= value);
    ASSERT_FALSE (array.getArray () <= value);
    ASSERT_FALSE (object.getObject () <= value);

    value = std::string ("foo");
    ASSERT_TRUE (nullptr <= value);
    ASSERT_TRUE (true <= value);
    ASSERT_TRUE (false <= value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::min () <= value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::max () <= value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::min () <= value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::max () <= value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::min () <= value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::max () <= value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::min () <= value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::max () <= value);
    ASSERT_TRUE (std::numeric_limits<double>::min () <= value);
    ASSERT_TRUE (std::numeric_limits<double>::max () <= value);
    ASSERT_TRUE ("foo" <= value);
    ASSERT_FALSE (array.getArray () <= value);
    ASSERT_FALSE (object.getObject () <= value);

    value = array.getArray ();
    ASSERT_TRUE (nullptr <= value);
    ASSERT_TRUE (true <= value);
    ASSERT_TRUE (false <= value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::min () <= value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::max () <= value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::min () <= value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::max () <= value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::min () <= value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::max () <= value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::min () <= value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::max () <= value);
    ASSERT_TRUE (std::numeric_limits<double>::min () <= value);
    ASSERT_TRUE (std::numeric_limits<double>::max () <= value);
    ASSERT_TRUE ("foo" <= value);
    ASSERT_TRUE (array.getArray () <= value);
    ASSERT_FALSE (object.getObject () <= value);

    value = object.getObject ();
    ASSERT_TRUE (nullptr <= value);
    ASSERT_TRUE (true <= value);
    ASSERT_TRUE (false <= value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::min () <= value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::max () <= value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::min () <= value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::max () <= value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::min () <= value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::max () <= value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::min () <= value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::max () <= value);
    ASSERT_TRUE (std::numeric_limits<double>::min () <= value);
    ASSERT_TRUE (std::numeric_limits<double>::max () <= value);
    ASSERT_TRUE ("foo" <= value);
    ASSERT_TRUE (array.getArray () <= value);
    ASSERT_TRUE (object.getObject () <= value);

    value = std::numeric_limits<double>::quiet_NaN ();
    ASSERT_FALSE (value <= value);
    ASSERT_FALSE (value <= 1);
}

/**
 * @brief Test greaterOrEqual operator.
 */
TEST (Value, greaterOrEqual)
{
    Value array;
    array.pushBack (1);

    Value object;
    object["i"] = 1;

    Value value;

    value = nullptr;
    ASSERT_TRUE (value >= nullptr);
    ASSERT_FALSE (value >= true);
    ASSERT_FALSE (value >= false);
    ASSERT_FALSE (value >= std::numeric_limits<int32_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits<int32_t>::max ());
    ASSERT_FALSE (value >= std::numeric_limits<uint32_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits<uint32_t>::max ());
    ASSERT_FALSE (value >= std::numeric_limits<int64_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits<int64_t>::max ());
    ASSERT_FALSE (value >= std::numeric_limits<uint64_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits<uint64_t>::max ());
    ASSERT_FALSE (value >= std::numeric_limits<double>::min ());
    ASSERT_FALSE (value >= std::numeric_limits<double>::max ());
    ASSERT_FALSE (value >= "foo");
    ASSERT_FALSE (value >= array.getArray ());
    ASSERT_FALSE (value >= object.getObject ());

    value = bool (true);
    ASSERT_TRUE (value >= nullptr);
    ASSERT_TRUE (value >= true);
    ASSERT_TRUE (value >= false);
    ASSERT_FALSE (value >= std::numeric_limits<int32_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits<int32_t>::max ());
    ASSERT_FALSE (value >= std::numeric_limits<uint32_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits<uint32_t>::max ());
    ASSERT_FALSE (value >= std::numeric_limits<int64_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits<int64_t>::max ());
    ASSERT_FALSE (value >= std::numeric_limits<uint64_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits<uint64_t>::max ());
    ASSERT_FALSE (value >= std::numeric_limits<double>::min ());
    ASSERT_FALSE (value >= std::numeric_limits<double>::max ());
    ASSERT_FALSE (value >= "foo");
    ASSERT_FALSE (value >= array.getArray ());
    ASSERT_FALSE (value >= object.getObject ());

    value = int32_t (std::numeric_limits<int32_t>::min ());
    ASSERT_TRUE (value >= nullptr);
    ASSERT_TRUE (value >= true);
    ASSERT_TRUE (value >= false);
    ASSERT_TRUE (value >= std::numeric_limits<int32_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits<int32_t>::max ());
    ASSERT_FALSE (value >= std::numeric_limits<uint32_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits<uint32_t>::max ());
    ASSERT_TRUE (value >= std::numeric_limits<int64_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits<int64_t>::max ());
    ASSERT_FALSE (value >= std::numeric_limits<uint64_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits<uint64_t>::max ());
    ASSERT_FALSE (value >= std::numeric_limits<double>::min ());
    ASSERT_FALSE (value >= std::numeric_limits<double>::max ());
    ASSERT_FALSE (value >= "foo");
    ASSERT_FALSE (value >= array.getArray ());
    ASSERT_FALSE (value >= object.getObject ());

    value = uint32_t (std::numeric_limits<uint32_t>::max ());
    ASSERT_TRUE (value >= nullptr);
    ASSERT_TRUE (value >= true);
    ASSERT_TRUE (value >= false);
    ASSERT_TRUE (value >= std::numeric_limits<int32_t>::min ());
    ASSERT_TRUE (value >= std::numeric_limits<int32_t>::max ());
    ASSERT_TRUE (value >= std::numeric_limits<uint32_t>::min ());
    ASSERT_TRUE (value >= std::numeric_limits<uint32_t>::max ());
    ASSERT_TRUE (value >= std::numeric_limits<int64_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits<int64_t>::max ());
    ASSERT_TRUE (value >= std::numeric_limits<uint64_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits<uint64_t>::max ());
    ASSERT_TRUE (value >= std::numeric_limits<double>::min ());
    ASSERT_FALSE (value >= std::numeric_limits<double>::max ());
    ASSERT_FALSE (value >= "foo");
    ASSERT_FALSE (value >= array.getArray ());
    ASSERT_FALSE (value >= object.getObject ());

    value = int64_t (std::numeric_limits<int64_t>::min ());
    ASSERT_TRUE (value >= nullptr);
    ASSERT_TRUE (value >= true);
    ASSERT_TRUE (value >= false);
    ASSERT_FALSE (value >= std::numeric_limits<int32_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits<int32_t>::max ());
    ASSERT_FALSE (value >= std::numeric_limits<uint32_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits<uint32_t>::max ());
    ASSERT_TRUE (value >= std::numeric_limits<int64_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits<int64_t>::max ());
    ASSERT_FALSE (value >= std::numeric_limits<uint64_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits<uint64_t>::max ());
    ASSERT_FALSE (value >= std::numeric_limits<double>::min ());
    ASSERT_FALSE (value >= std::numeric_limits<double>::max ());
    ASSERT_FALSE (value >= "foo");
    ASSERT_FALSE (value >= array.getArray ());
    ASSERT_FALSE (value >= object.getObject ());

    value = uint64_t (std::numeric_limits<uint64_t>::max ());
    ASSERT_TRUE (value >= nullptr);
    ASSERT_TRUE (value >= true);
    ASSERT_TRUE (value >= false);
    ASSERT_TRUE (value >= std::numeric_limits<int32_t>::min ());
    ASSERT_TRUE (value >= std::numeric_limits<int32_t>::max ());
    ASSERT_TRUE (value >= std::numeric_limits<uint32_t>::min ());
    ASSERT_TRUE (value >= std::numeric_limits<uint32_t>::max ());
    ASSERT_TRUE (value >= std::numeric_limits<int64_t>::min ());
    ASSERT_TRUE (value >= std::numeric_limits<int64_t>::max ());
    ASSERT_TRUE (value >= std::numeric_limits<uint64_t>::min ());
    ASSERT_TRUE (value >= std::numeric_limits<uint64_t>::max ());
    ASSERT_TRUE (value >= std::numeric_limits<double>::min ());
    ASSERT_FALSE (value >= std::numeric_limits<double>::max ());
    ASSERT_FALSE (value >= "foo");
    ASSERT_FALSE (value >= array.getArray ());
    ASSERT_FALSE (value >= object.getObject ());

    value = double (std::numeric_limits<double>::min ());
    ASSERT_TRUE (value >= nullptr);
    ASSERT_TRUE (value >= true);
    ASSERT_TRUE (value >= false);
    ASSERT_TRUE (value >= std::numeric_limits<int32_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits<int32_t>::max ());
    ASSERT_TRUE (value >= std::numeric_limits<uint32_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits<uint32_t>::max ());
    ASSERT_TRUE (value >= std::numeric_limits<int64_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits<int64_t>::max ());
    ASSERT_TRUE (value >= std::numeric_limits<uint64_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits<uint64_t>::max ());
    ASSERT_TRUE (value >= std::numeric_limits<double>::min ());
    ASSERT_FALSE (value >= std::numeric_limits<double>::max ());
    ASSERT_FALSE (value >= "foo");
    ASSERT_FALSE (value >= array.getArray ());
    ASSERT_FALSE (value >= object.getObject ());

    value = std::string ("foo");
    ASSERT_TRUE (value >= nullptr);
    ASSERT_TRUE (value >= true);
    ASSERT_TRUE (value >= false);
    ASSERT_TRUE (value >= std::numeric_limits<int32_t>::min ());
    ASSERT_TRUE (value >= std::numeric_limits<int32_t>::max ());
    ASSERT_TRUE (value >= std::numeric_limits<uint32_t>::min ());
    ASSERT_TRUE (value >= std::numeric_limits<uint32_t>::max ());
    ASSERT_TRUE (value >= std::numeric_limits<int64_t>::min ());
    ASSERT_TRUE (value >= std::numeric_limits<int64_t>::max ());
    ASSERT_TRUE (value >= std::numeric_limits<uint64_t>::min ());
    ASSERT_TRUE (value >= std::numeric_limits<uint64_t>::max ());
    ASSERT_TRUE (value >= std::numeric_limits<double>::min ());
    ASSERT_TRUE (value >= std::numeric_limits<double>::max ());
    ASSERT_TRUE (value >= "foo");
    ASSERT_FALSE (value >= array.getArray ());
    ASSERT_FALSE (value >= object.getObject ());

    value = array.getArray ();
    ASSERT_TRUE (value >= nullptr);
    ASSERT_TRUE (value >= true);
    ASSERT_TRUE (value >= false);
    ASSERT_TRUE (value >= std::numeric_limits<int32_t>::min ());
    ASSERT_TRUE (value >= std::numeric_limits<int32_t>::max ());
    ASSERT_TRUE (value >= std::numeric_limits<uint32_t>::min ());
    ASSERT_TRUE (value >= std::numeric_limits<uint32_t>::max ());
    ASSERT_TRUE (value >= std::numeric_limits<int64_t>::min ());
    ASSERT_TRUE (value >= std::numeric_limits<int64_t>::max ());
    ASSERT_TRUE (value >= std::numeric_limits<uint64_t>::min ());
    ASSERT_TRUE (value >= std::numeric_limits<uint64_t>::max ());
    ASSERT_TRUE (value >= std::numeric_limits<double>::min ());
    ASSERT_TRUE (value >= std::numeric_limits<double>::max ());
    ASSERT_TRUE (value >= "foo");
    ASSERT_TRUE (value >= array.getArray ());
    ASSERT_FALSE (value >= object.getObject ());

    value = object.getObject ();
    ASSERT_TRUE (value >= nullptr);
    ASSERT_TRUE (value >= true);
    ASSERT_TRUE (value >= false);
    ASSERT_TRUE (value >= std::numeric_limits<int32_t>::min ());
    ASSERT_TRUE (value >= std::numeric_limits<int32_t>::max ());
    ASSERT_TRUE (value >= std::numeric_limits<uint32_t>::min ());
    ASSERT_TRUE (value >= std::numeric_limits<uint32_t>::max ());
    ASSERT_TRUE (value >= std::numeric_limits<int64_t>::min ());
    ASSERT_TRUE (value >= std::numeric_limits<int64_t>::max ());
    ASSERT_TRUE (value >= std::numeric_limits<uint64_t>::min ());
    ASSERT_TRUE (value >= std::numeric_limits<uint64_t>::max ());
    ASSERT_TRUE (value >= std::numeric_limits<double>::min ());
    ASSERT_TRUE (value >= std::numeric_limits<double>::max ());
    ASSERT_TRUE (value >= "foo");
    ASSERT_TRUE (value >= array.getArray ());
    ASSERT_TRUE (value >= object.getObject ());

    value = nullptr;
    ASSERT_TRUE (nullptr >= value);
    ASSERT_TRUE (true >= value);
    ASSERT_TRUE (false >= value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::min () >= value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::max () >= value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::min () >= value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::max () >= value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::min () >= value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::max () >= value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::min () >= value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::max () >= value);
    ASSERT_TRUE (std::numeric_limits<double>::min () >= value);
    ASSERT_TRUE (std::numeric_limits<double>::max () >= value);
    ASSERT_TRUE ("foo" >= value);
    ASSERT_TRUE (array.getArray () >= value);
    ASSERT_TRUE (object.getObject () >= value);

    value = bool (true);
    ASSERT_FALSE (nullptr >= value);
    ASSERT_TRUE (true >= value);
    ASSERT_FALSE (false >= value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::min () >= value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::max () >= value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::min () >= value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::max () >= value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::min () >= value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::max () >= value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::min () >= value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::max () >= value);
    ASSERT_TRUE (std::numeric_limits<double>::min () >= value);
    ASSERT_TRUE (std::numeric_limits<double>::max () >= value);
    ASSERT_TRUE ("foo" >= value);
    ASSERT_TRUE (array.getArray () >= value);
    ASSERT_TRUE (object.getObject () >= value);

    value = int32_t (std::numeric_limits<int32_t>::min ());
    ASSERT_FALSE (nullptr >= value);
    ASSERT_FALSE (true >= value);
    ASSERT_FALSE (false >= value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::min () >= value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::max () >= value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::min () >= value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::min () >= value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::max () >= value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::min () >= value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::max () >= value);
    ASSERT_TRUE (std::numeric_limits<double>::min () >= value);
    ASSERT_TRUE (std::numeric_limits<double>::max () >= value);
    ASSERT_TRUE ("foo" >= value);
    ASSERT_TRUE (array.getArray () >= value);
    ASSERT_TRUE (object.getObject () >= value);

    value = uint32_t (std::numeric_limits<uint32_t>::max ());
    ASSERT_FALSE (nullptr >= value);
    ASSERT_FALSE (true >= value);
    ASSERT_FALSE (false >= value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::min () >= value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::min () >= value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::min () >= value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::min () >= value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits<double>::min () >= value);
    ASSERT_TRUE (std::numeric_limits<double>::max () >= value);
    ASSERT_TRUE ("foo" >= value);
    ASSERT_TRUE (array.getArray () >= value);
    ASSERT_TRUE (object.getObject () >= value);

    value = int64_t (std::numeric_limits<int64_t>::min ());
    ASSERT_FALSE (nullptr >= value);
    ASSERT_FALSE (true >= value);
    ASSERT_FALSE (false >= value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::min () >= value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::max () >= value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::min () >= value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::max () >= value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::min () >= value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::max () >= value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::min () >= value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::max () >= value);
    ASSERT_TRUE (std::numeric_limits<double>::min () >= value);
    ASSERT_TRUE (std::numeric_limits<double>::max () >= value);
    ASSERT_TRUE ("foo" >= value);
    ASSERT_TRUE (array.getArray () >= value);
    ASSERT_TRUE (object.getObject () >= value);

    value = uint64_t (std::numeric_limits<uint64_t>::max ());
    ASSERT_FALSE (nullptr >= value);
    ASSERT_FALSE (true >= value);
    ASSERT_FALSE (false >= value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::min () >= value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::min () >= value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::min () >= value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::min () >= value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits<double>::min () >= value);
    ASSERT_TRUE (std::numeric_limits<double>::max () >= value);
    ASSERT_TRUE ("foo" >= value);
    ASSERT_TRUE (array.getArray () >= value);
    ASSERT_TRUE (object.getObject () >= value);

    value = double (std::numeric_limits<double>::min ());
    ASSERT_FALSE (nullptr >= value);
    ASSERT_FALSE (true >= value);
    ASSERT_FALSE (false >= value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::min () >= value);
    ASSERT_TRUE (std::numeric_limits<int32_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::min () >= value);
    ASSERT_TRUE (std::numeric_limits<uint32_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::min () >= value);
    ASSERT_TRUE (std::numeric_limits<int64_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::min () >= value);
    ASSERT_TRUE (std::numeric_limits<uint64_t>::max () >= value);
    ASSERT_TRUE (std::numeric_limits<double>::min () >= value);
    ASSERT_TRUE (std::numeric_limits<double>::max () >= value);
    ASSERT_TRUE ("foo" >= value);
    ASSERT_TRUE (array.getArray () >= value);
    ASSERT_TRUE (object.getObject () >= value);

    value = std::string ("foo");
    ASSERT_FALSE (nullptr >= value);
    ASSERT_FALSE (true >= value);
    ASSERT_FALSE (false >= value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::min () >= value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::min () >= value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::min () >= value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::min () >= value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits<double>::min () >= value);
    ASSERT_FALSE (std::numeric_limits<double>::max () >= value);
    ASSERT_TRUE ("foo" >= value);
    ASSERT_TRUE (array.getArray () >= value);
    ASSERT_TRUE (object.getObject () >= value);

    value = array.getArray ();
    ASSERT_FALSE (nullptr >= value);
    ASSERT_FALSE (true >= value);
    ASSERT_FALSE (false >= value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::min () >= value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::min () >= value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::min () >= value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::min () >= value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits<double>::min () >= value);
    ASSERT_FALSE (std::numeric_limits<double>::max () >= value);
    ASSERT_FALSE ("foo" >= value);
    ASSERT_TRUE (array.getArray () >= value);
    ASSERT_TRUE (object.getObject () >= value);

    value = object.getObject ();
    ASSERT_FALSE (nullptr >= value);
    ASSERT_FALSE (true >= value);
    ASSERT_FALSE (false >= value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::min () >= value);
    ASSERT_FALSE (std::numeric_limits<int32_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::min () >= value);
    ASSERT_FALSE (std::numeric_limits<uint32_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::min () >= value);
    ASSERT_FALSE (std::numeric_limits<int64_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::min () >= value);
    ASSERT_FALSE (std::numeric_limits<uint64_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits<double>::min () >= value);
    ASSERT_FALSE (std::numeric_limits<double>::max () >= value);
    ASSERT_FALSE ("foo" >= value);
    ASSERT_FALSE (array.getArray () >= value);
    ASSERT_TRUE (object.getObject () >= value);
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
