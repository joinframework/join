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

// Libraries.
#include <gtest/gtest.h>

// C++.
#include <limits>

using join::sax::Array;
using join::sax::Member;
using join::sax::Object;
using join::sax::Value;

/**
 * @brief Test create method.
 */
TEST (Value, create)
{
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

    Value minIntValue (std::numeric_limits <int32_t>::min ());
    ASSERT_TRUE (minIntValue.isNumber ());
    ASSERT_TRUE (minIntValue.isInt ());

    Value maxIntValue (std::numeric_limits <int32_t>::max ());
    ASSERT_TRUE (minIntValue.isNumber ());
    ASSERT_TRUE (minIntValue.isInt ());

    Value minUintValue (std::numeric_limits <uint32_t>::min ());
    ASSERT_TRUE (minUintValue.isNumber ());
    ASSERT_TRUE (minUintValue.isUint ());

    Value maxUintValue (std::numeric_limits <uint32_t>::max ());
    ASSERT_TRUE (maxUintValue.isNumber ());
    ASSERT_TRUE (maxUintValue.isUint ());

    Value minInt64Value (std::numeric_limits <int64_t>::min ());
    ASSERT_TRUE (minIntValue.isNumber ());
    ASSERT_TRUE (minIntValue.isInt64 ());

    Value maxInt64Value (std::numeric_limits <int64_t>::max ());
    ASSERT_TRUE (minInt64Value.isNumber ());
    ASSERT_TRUE (minInt64Value.isInt64 ());

    Value minUint64Value (std::numeric_limits <uint64_t>::min ());
    ASSERT_TRUE (minUint64Value.isNumber ());
    ASSERT_TRUE (minUint64Value.isUint64 ());

    Value maxUint64Value (std::numeric_limits <uint64_t>::max ());
    ASSERT_TRUE (maxUint64Value.isNumber ());
    ASSERT_TRUE (maxUint64Value.isUint64 ());

    Value minFloatValue (std::numeric_limits <float>::min ());
    ASSERT_TRUE (minFloatValue.isNumber ());
    ASSERT_TRUE (minFloatValue.isFloat ());

    Value maxFloatValue (std::numeric_limits <float>::max ());
    ASSERT_TRUE (maxFloatValue.isNumber ());
    ASSERT_TRUE (maxFloatValue.isFloat ());

    Value minDoubleValue (std::numeric_limits <double>::min ());
    ASSERT_TRUE (minDoubleValue.isNumber ());
    ASSERT_TRUE (minDoubleValue.isDouble ());

    Value maxDoubleValue (std::numeric_limits <double>::max ());
    ASSERT_TRUE (maxDoubleValue.isNumber ());
    ASSERT_TRUE (maxDoubleValue.isDouble ());

    Value stringValue ("foo");
    ASSERT_TRUE (stringValue.isString ());

    Value arrayValue (Array ({1}));
    ASSERT_TRUE (arrayValue.isArray ());

    Value objectValue (Object ({{"i", 1}}));
    ASSERT_TRUE (objectValue.isObject ());
}

/**
 * @brief Test copy method.
 */
TEST (Value, copy)
{
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

    value = std::numeric_limits <int32_t>::min ();
    Value minIntValue (value);
    ASSERT_TRUE (minIntValue.isNumber ());
    ASSERT_TRUE (minIntValue.isInt ());

    value = std::numeric_limits <int32_t>::max ();
    Value maxIntValue (value);
    ASSERT_TRUE (minIntValue.isNumber ());
    ASSERT_TRUE (minIntValue.isInt ());

    value = std::numeric_limits <uint32_t>::min ();
    Value minUintValue (value);
    ASSERT_TRUE (minUintValue.isNumber ());
    ASSERT_TRUE (minUintValue.isUint ());

    value = std::numeric_limits <uint32_t>::max ();
    Value maxUintValue (value);
    ASSERT_TRUE (maxUintValue.isNumber ());
    ASSERT_TRUE (maxUintValue.isUint ());

    value = std::numeric_limits <int64_t>::min ();
    Value minInt64Value (value);
    ASSERT_TRUE (minIntValue.isNumber ());
    ASSERT_TRUE (minIntValue.isInt64 ());

    value = std::numeric_limits <int64_t>::max ();
    Value maxInt64Value (value);
    ASSERT_TRUE (minInt64Value.isNumber ());
    ASSERT_TRUE (minInt64Value.isInt64 ());

    value = std::numeric_limits <uint64_t>::min ();
    Value minUint64Value (value);
    ASSERT_TRUE (minUint64Value.isNumber ());
    ASSERT_TRUE (minUint64Value.isUint64 ());

    value = std::numeric_limits <uint64_t>::max ();
    Value maxUint64Value (value);
    ASSERT_TRUE (maxUint64Value.isNumber ());
    ASSERT_TRUE (maxUint64Value.isUint64 ());

    value = std::numeric_limits <float>::min ();
    Value minFloatValue (value);
    ASSERT_TRUE (minFloatValue.isNumber ());
    ASSERT_TRUE (minFloatValue.isFloat ());

    value = std::numeric_limits <float>::max ();
    Value maxFloatValue (value);
    ASSERT_TRUE (maxFloatValue.isNumber ());
    ASSERT_TRUE (maxFloatValue.isFloat ());

    value = std::numeric_limits <double>::min ();
    Value minDoubleValue (value);
    ASSERT_TRUE (minDoubleValue.isNumber ());
    ASSERT_TRUE (minDoubleValue.isDouble ());

    value = std::numeric_limits <double>::max ();
    Value maxDoubleValue (value);
    ASSERT_TRUE (maxDoubleValue.isNumber ());
    ASSERT_TRUE (maxDoubleValue.isDouble ());

    value = "foo";
    Value stringValue (value);
    ASSERT_TRUE (stringValue.isString ());

    value = Array ({1});
    Value arrayValue (value);
    ASSERT_TRUE (arrayValue.isArray ());

    value = Object ({{"i", 1}});
    Value objectValue (value);
    ASSERT_TRUE (objectValue.isObject ());
}

/**
 * @brief Test move method.
 */
TEST (Value, move)
{
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

    value = std::numeric_limits <int32_t>::min ();
    Value minIntValue (std::move (value));
    ASSERT_TRUE (minIntValue.isNumber ());
    ASSERT_TRUE (minIntValue.isInt ());

    value = std::numeric_limits <int32_t>::max ();
    Value maxIntValue (std::move (value));
    ASSERT_TRUE (minIntValue.isNumber ());
    ASSERT_TRUE (minIntValue.isInt ());

    value = std::numeric_limits <uint32_t>::min ();
    Value minUintValue (std::move (value));
    ASSERT_TRUE (minUintValue.isNumber ());
    ASSERT_TRUE (minUintValue.isUint ());

    value = std::numeric_limits <uint32_t>::max ();
    Value maxUintValue (std::move (value));
    ASSERT_TRUE (maxUintValue.isNumber ());
    ASSERT_TRUE (maxUintValue.isUint ());

    value = std::numeric_limits <int64_t>::min ();
    Value minInt64Value (std::move (value));
    ASSERT_TRUE (minIntValue.isNumber ());
    ASSERT_TRUE (minIntValue.isInt64 ());

    value = std::numeric_limits <int64_t>::max ();
    Value maxInt64Value (std::move (value));
    ASSERT_TRUE (minInt64Value.isNumber ());
    ASSERT_TRUE (minInt64Value.isInt64 ());

    value = std::numeric_limits <uint64_t>::min ();
    Value minUint64Value (std::move (value));
    ASSERT_TRUE (minUint64Value.isNumber ());
    ASSERT_TRUE (minUint64Value.isUint64 ());

    value = std::numeric_limits <uint64_t>::max ();
    Value maxUint64Value (std::move (value));
    ASSERT_TRUE (maxUint64Value.isNumber ());
    ASSERT_TRUE (maxUint64Value.isUint64 ());

    value = std::numeric_limits <float>::min ();
    Value minFloatValue (std::move (value));
    ASSERT_TRUE (minFloatValue.isNumber ());
    ASSERT_TRUE (minFloatValue.isFloat ());

    value = std::numeric_limits <float>::max ();
    Value maxFloatValue (std::move (value));
    ASSERT_TRUE (maxFloatValue.isNumber ());
    ASSERT_TRUE (maxFloatValue.isFloat ());

    value = std::numeric_limits <double>::min ();
    Value minDoubleValue (std::move (value));
    ASSERT_TRUE (minDoubleValue.isNumber ());
    ASSERT_TRUE (minDoubleValue.isDouble ());

    value = std::numeric_limits <double>::max ();
    Value maxDoubleValue (std::move (value));
    ASSERT_TRUE (maxDoubleValue.isNumber ());
    ASSERT_TRUE (maxDoubleValue.isDouble ());

    value = "foo";
    Value stringValue (std::move (value));
    ASSERT_TRUE (stringValue.isString ());

    value = Array ({1});
    Value arrayValue (std::move (value));
    ASSERT_TRUE (arrayValue.isArray ());

    value = Object ({{"i", 1}});
    Value objectValue (std::move (value));
    ASSERT_TRUE (objectValue.isObject ());
}

/**
 * @brief Test is method.
 */
TEST (Value, is)
{
    Value value;

    value.set <std::nullptr_t> (nullptr);
    ASSERT_TRUE  (value.is <std::nullptr_t> ());
    ASSERT_FALSE (value.is <bool> ());
    ASSERT_FALSE (value.is <int32_t> ());
    ASSERT_FALSE (value.is <uint32_t> ());
    ASSERT_FALSE (value.is <int64_t> ());
    ASSERT_FALSE (value.is <uint64_t> ());
    ASSERT_FALSE (value.is <double> ());
    ASSERT_FALSE (value.is <std::string> ());
    ASSERT_FALSE (value.is <Array> ());
    ASSERT_FALSE (value.is <Object> ());

    value.set <int32_t> (std::numeric_limits <int32_t>::min ());
    ASSERT_FALSE (value.is <std::nullptr_t> ());
    ASSERT_FALSE (value.is <bool> ());
    ASSERT_TRUE  (value.is <int32_t> ());
    ASSERT_FALSE (value.is <uint32_t> ());
    ASSERT_FALSE (value.is <int64_t> ());
    ASSERT_FALSE (value.is <uint64_t> ());
    ASSERT_FALSE (value.is <double> ());
    ASSERT_FALSE (value.is <std::string> ());
    ASSERT_FALSE (value.is <Array> ());
    ASSERT_FALSE (value.is <Object> ());

    value.set <uint32_t> (std::numeric_limits <uint32_t>::max ());
    ASSERT_FALSE (value.is <std::nullptr_t> ());
    ASSERT_FALSE (value.is <bool> ());
    ASSERT_FALSE (value.is <int> ());
    ASSERT_TRUE  (value.is <unsigned int> ());
    ASSERT_FALSE (value.is <int64_t> ());
    ASSERT_FALSE (value.is <uint64_t> ());
    ASSERT_FALSE (value.is <double> ());
    ASSERT_FALSE (value.is <std::string> ());
    ASSERT_FALSE (value.is <Array> ());
    ASSERT_FALSE (value.is <Object> ());

    value.set <int64_t> (std::numeric_limits <int64_t>::min ());
    ASSERT_FALSE (value.is <std::nullptr_t> ());
    ASSERT_FALSE (value.is <bool> ());
    ASSERT_FALSE (value.is <int32_t> ());
    ASSERT_FALSE (value.is <uint32_t> ());
    ASSERT_TRUE  (value.is <int64_t> ());
    ASSERT_FALSE (value.is <uint64_t> ());
    ASSERT_FALSE (value.is <double> ());
    ASSERT_FALSE (value.is <std::string> ());
    ASSERT_FALSE (value.is <Array> ());
    ASSERT_FALSE (value.is <Object> ());

    value.set <uint64_t> (std::numeric_limits <uint64_t>::max ());
    ASSERT_FALSE (value.is <std::nullptr_t> ());
    ASSERT_FALSE (value.is <bool> ());
    ASSERT_FALSE (value.is <int> ());
    ASSERT_FALSE (value.is <unsigned int> ());
    ASSERT_FALSE (value.is <int64_t> ());
    ASSERT_TRUE (value.is <uint64_t> ());
    ASSERT_FALSE (value.is <double> ());
    ASSERT_FALSE (value.is <std::string> ());
    ASSERT_FALSE (value.is <Array> ());
    ASSERT_FALSE (value.is <Object> ());

    value.set <double> (std::numeric_limits <double>::min ());
    ASSERT_FALSE (value.is <std::nullptr_t> ());
    ASSERT_FALSE (value.is <bool> ());
    ASSERT_FALSE (value.is <int32_t> ());
    ASSERT_FALSE (value.is <uint32_t> ());
    ASSERT_FALSE (value.is <int64_t> ());
    ASSERT_FALSE (value.is <uint64_t> ());
    ASSERT_TRUE  (value.is <double> ());
    ASSERT_FALSE (value.is <std::string> ());
    ASSERT_FALSE (value.is <Array> ());
    ASSERT_FALSE (value.is <Object> ());

    value.set <std::string> ("foobar");
    ASSERT_FALSE (value.is <std::nullptr_t> ());
    ASSERT_FALSE (value.is <bool> ());
    ASSERT_FALSE (value.is <int32_t> ());
    ASSERT_FALSE (value.is <uint32_t> ());
    ASSERT_FALSE (value.is <int64_t> ());
    ASSERT_FALSE (value.is <uint64_t> ());
    ASSERT_FALSE (value.is <double> ());
    ASSERT_TRUE  (value.is <std::string> ());
    ASSERT_FALSE (value.is <Array> ());
    ASSERT_FALSE (value.is <Object> ());

    value.set <bool> (true);
    ASSERT_FALSE (value.is <std::nullptr_t> ());
    ASSERT_TRUE  (value.is <bool> ());
    ASSERT_FALSE (value.is <int32_t> ());
    ASSERT_FALSE (value.is <uint32_t> ());
    ASSERT_FALSE (value.is <int64_t> ());
    ASSERT_FALSE (value.is <uint64_t> ());
    ASSERT_FALSE (value.is <double> ());
    ASSERT_FALSE (value.is <std::string> ());
    ASSERT_FALSE (value.is <Array> ());
    ASSERT_FALSE (value.is <Object> ());

    value.set <Array, Array::value_type> ({1});
    ASSERT_FALSE (value.is <std::nullptr_t> ());
    ASSERT_FALSE (value.is <bool> ());
    ASSERT_FALSE (value.is <int32_t> ());
    ASSERT_FALSE (value.is <uint32_t> ());
    ASSERT_FALSE (value.is <int64_t> ());
    ASSERT_FALSE (value.is <uint64_t> ());
    ASSERT_FALSE (value.is <double> ());
    ASSERT_FALSE (value.is <std::string> ());
    ASSERT_TRUE  (value.is <Array> ());
    ASSERT_FALSE (value.is <Object> ());

    value.set <Object, Object::value_type> ({{"i", 1}});
    ASSERT_FALSE (value.is <std::nullptr_t> ());
    ASSERT_FALSE (value.is <bool> ());
    ASSERT_FALSE (value.is <int32_t> ());
    ASSERT_FALSE (value.is <uint32_t> ());
    ASSERT_FALSE (value.is <int64_t> ());
    ASSERT_FALSE (value.is <uint64_t> ());
    ASSERT_FALSE (value.is <double> ());
    ASSERT_FALSE (value.is <std::string> ());
    ASSERT_FALSE (value.is <Array> ());
    ASSERT_TRUE  (value.is <Object> ());
}

/**
 * @brief Test isNull method.
 */
TEST (Value, isNull)
{
    Value value;
    ASSERT_TRUE (value.isNull ());

    value = nullptr;
    ASSERT_TRUE (value.isNull ());

    value = true;
    ASSERT_FALSE (value.isNull ());

    value = false;
    ASSERT_FALSE (value.isNull ());

    value = std::numeric_limits <int8_t>::min ();
    ASSERT_FALSE (value.isNull ());

    value = std::numeric_limits <int8_t>::max ();
    ASSERT_FALSE (value.isNull ());

    value = std::numeric_limits <uint8_t>::min ();
    ASSERT_FALSE (value.isNull ());

    value = std::numeric_limits <uint8_t>::max ();
    ASSERT_FALSE (value.isNull ());

    value = std::numeric_limits <int16_t>::min ();
    ASSERT_FALSE (value.isNull ());

    value = std::numeric_limits <int16_t>::max ();
    ASSERT_FALSE (value.isNull ());

    value = std::numeric_limits <uint16_t>::min ();
    ASSERT_FALSE (value.isNull ());

    value = std::numeric_limits <uint16_t>::max ();
    ASSERT_FALSE (value.isNull ());

    value = std::numeric_limits <int32_t>::min ();
    ASSERT_FALSE (value.isNull ());

    value = std::numeric_limits <int32_t>::max ();
    ASSERT_FALSE (value.isNull ());

    value = std::numeric_limits <uint32_t>::min ();
    ASSERT_FALSE (value.isNull ());

    value = std::numeric_limits <uint32_t>::max ();
    ASSERT_FALSE (value.isNull ());

    value = std::numeric_limits <int64_t>::min ();
    ASSERT_FALSE (value.isNull ());

    value = std::numeric_limits <int64_t>::max ();
    ASSERT_FALSE (value.isNull ());

    value = std::numeric_limits <uint64_t>::min ();
    ASSERT_FALSE (value.isNull ());

    value = std::numeric_limits <uint64_t>::max ();
    ASSERT_FALSE (value.isNull ());

    value = std::numeric_limits <float>::min ();
    ASSERT_FALSE (value.isNull ());

    value = std::numeric_limits <float>::max ();
    ASSERT_FALSE (value.isNull ());

    value = std::numeric_limits <double>::min ();
    ASSERT_FALSE (value.isNull ());

    value = std::numeric_limits <double>::max ();
    ASSERT_FALSE (value.isNull ());

    value = "";
    ASSERT_FALSE (value.isNull ());

    value = "foo";
    ASSERT_FALSE (value.isNull ());

    value = "02:42:64:2f:6a:d0";
    ASSERT_FALSE (value.isNull ());

    value = "127.0.0.1";
    ASSERT_FALSE (value.isNull ());

    value = Array ({1});
    ASSERT_FALSE (value.isNull ());

    value = Object ({{"i", 1}});
    ASSERT_FALSE (value.isNull ());
}

/**
 * @brief Test isBool method.
 */
TEST (Value, isBool)
{
    Value value;
    ASSERT_FALSE (value.isBool ());

    value = nullptr;
    ASSERT_FALSE (value.isBool ());

    value = true;
    ASSERT_TRUE (value.isBool ());

    value = false;
    ASSERT_TRUE (value.isBool ());

    value = std::numeric_limits <int8_t>::min ();
    ASSERT_FALSE (value.isBool ());

    value = std::numeric_limits <int8_t>::max ();
    ASSERT_FALSE (value.isBool ());

    value = std::numeric_limits <uint8_t>::min ();
    ASSERT_FALSE (value.isBool ());

    value = std::numeric_limits <uint8_t>::max ();
    ASSERT_FALSE (value.isBool ());

    value = std::numeric_limits <int16_t>::min ();
    ASSERT_FALSE (value.isBool ());

    value = std::numeric_limits <int16_t>::max ();
    ASSERT_FALSE (value.isBool ());

    value = std::numeric_limits <uint16_t>::min ();
    ASSERT_FALSE (value.isBool ());

    value = std::numeric_limits <uint16_t>::max ();
    ASSERT_FALSE (value.isBool ());

    value = std::numeric_limits <int32_t>::min ();
    ASSERT_FALSE (value.isBool ());

    value = std::numeric_limits <int32_t>::max ();
    ASSERT_FALSE (value.isBool ());

    value = std::numeric_limits <uint32_t>::min ();
    ASSERT_FALSE (value.isBool ());

    value = std::numeric_limits <uint32_t>::max ();
    ASSERT_FALSE (value.isBool ());

    value = std::numeric_limits <int64_t>::min ();
    ASSERT_FALSE (value.isBool ());

    value = std::numeric_limits <int64_t>::max ();
    ASSERT_FALSE (value.isBool ());

    value = std::numeric_limits <uint64_t>::min ();
    ASSERT_FALSE (value.isBool ());

    value = std::numeric_limits <uint64_t>::max ();
    ASSERT_FALSE (value.isBool ());

    value = std::numeric_limits <float>::min ();
    ASSERT_FALSE (value.isBool ());

    value = std::numeric_limits <float>::max ();
    ASSERT_FALSE (value.isBool ());

    value = std::numeric_limits <double>::min ();
    ASSERT_FALSE (value.isBool ());

    value = std::numeric_limits <double>::max ();
    ASSERT_FALSE (value.isBool ());

    value = "";
    ASSERT_FALSE (value.isBool ());

    value = "foo";
    ASSERT_FALSE (value.isBool ());

    value = "02:42:64:2f:6a:d0";
    ASSERT_FALSE (value.isBool ());

    value = "127.0.0.1";
    ASSERT_FALSE (value.isBool ());

    value = Array ({1});
    ASSERT_FALSE (value.isBool ());

    value = Object ({{"i", 1}});
    ASSERT_FALSE (value.isBool ());
}

/**
 * @brief Test isTrue method.
 */
TEST (Value, isTrue)
{
    Value value;
    ASSERT_FALSE (value.isTrue ());

    value = nullptr;
    ASSERT_FALSE (value.isTrue ());

    value = true;
    ASSERT_TRUE (value.isTrue ());

    value = false;
    ASSERT_FALSE (value.isTrue ());

    value = std::numeric_limits <int8_t>::min ();
    ASSERT_TRUE (value.isTrue ());

    value = std::numeric_limits <int8_t>::max ();
    ASSERT_TRUE (value.isTrue ());

    value = std::numeric_limits <uint8_t>::min ();
    ASSERT_FALSE (value.isTrue ());

    value = std::numeric_limits <uint8_t>::max ();
    ASSERT_TRUE (value.isTrue ());

    value = std::numeric_limits <int16_t>::min ();
    ASSERT_TRUE (value.isTrue ());

    value = std::numeric_limits <int16_t>::max ();
    ASSERT_TRUE (value.isTrue ());

    value = std::numeric_limits <uint16_t>::min ();
    ASSERT_FALSE (value.isTrue ());

    value = std::numeric_limits <uint16_t>::max ();
    ASSERT_TRUE (value.isTrue ());

    value = std::numeric_limits <int32_t>::min ();
    ASSERT_TRUE (value.isTrue ());

    value = std::numeric_limits <int32_t>::max ();
    ASSERT_TRUE (value.isTrue ());

    value = std::numeric_limits <uint32_t>::min ();
    ASSERT_FALSE (value.isTrue ());

    value = std::numeric_limits <uint32_t>::max ();
    ASSERT_TRUE (value.isTrue ());

    value = std::numeric_limits <int64_t>::min ();
    ASSERT_TRUE (value.isTrue ());

    value = std::numeric_limits <int64_t>::max ();
    ASSERT_TRUE (value.isTrue ());

    value = std::numeric_limits <uint64_t>::min ();
    ASSERT_FALSE (value.isTrue ());

    value = std::numeric_limits <uint64_t>::max ();
    ASSERT_TRUE (value.isTrue ());

    value = std::numeric_limits <float>::min ();
    ASSERT_TRUE (value.isTrue ());

    value = std::numeric_limits <float>::max ();
    ASSERT_TRUE (value.isTrue ());

    value = std::numeric_limits <double>::min ();
    ASSERT_TRUE (value.isTrue ());

    value = std::numeric_limits <double>::max ();
    ASSERT_TRUE (value.isTrue ());

    value = "";
    ASSERT_THROW (value.isTrue (), std::bad_cast);

    value = "foo";
    ASSERT_THROW (value.isTrue (), std::bad_cast);

    value = "02:42:64:2f:6a:d0";
    ASSERT_THROW (value.isTrue (), std::bad_cast);

    value = "127.0.0.1";
    ASSERT_THROW (value.isTrue (), std::bad_cast);

    value = Array ({1});
    ASSERT_THROW (value.isTrue (), std::bad_cast);

    value = Object ({{"i", 1}});
    ASSERT_THROW (value.isTrue (), std::bad_cast);
}

/**
 * @brief Test isFalse method.
 */
TEST (Value, isFalse)
{
    Value value;
    ASSERT_TRUE (value.isFalse ());

    value = nullptr;
    ASSERT_TRUE (value.isFalse ());

    value = true;
    ASSERT_FALSE (value.isFalse ());

    value = false;
    ASSERT_TRUE (value.isFalse ());

    value = std::numeric_limits <int8_t>::min ();
    ASSERT_FALSE (value.isFalse ());

    value = std::numeric_limits <int8_t>::max ();
    ASSERT_FALSE (value.isFalse ());

    value = std::numeric_limits <uint8_t>::min ();
    ASSERT_TRUE (value.isFalse ());

    value = std::numeric_limits <uint8_t>::max ();
    ASSERT_FALSE (value.isFalse ());

    value = std::numeric_limits <int16_t>::min ();
    ASSERT_FALSE (value.isFalse ());

    value = std::numeric_limits <int16_t>::max ();
    ASSERT_FALSE (value.isFalse ());

    value = std::numeric_limits <uint16_t>::min ();
    ASSERT_TRUE (value.isFalse ());

    value = std::numeric_limits <uint16_t>::max ();
    ASSERT_FALSE (value.isFalse ());

    value = std::numeric_limits <int32_t>::min ();
    ASSERT_FALSE (value.isFalse ());

    value = std::numeric_limits <int32_t>::max ();
    ASSERT_FALSE (value.isFalse ());

    value = std::numeric_limits <uint32_t>::min ();
    ASSERT_TRUE (value.isFalse ());

    value = std::numeric_limits <uint32_t>::max ();
    ASSERT_FALSE (value.isFalse ());

    value = std::numeric_limits <int64_t>::min ();
    ASSERT_FALSE (value.isFalse ());

    value = std::numeric_limits <int64_t>::max ();
    ASSERT_FALSE (value.isFalse ());

    value = std::numeric_limits <uint64_t>::min ();
    ASSERT_TRUE (value.isFalse ());

    value = std::numeric_limits <uint64_t>::max ();
    ASSERT_FALSE (value.isFalse ());

    value = std::numeric_limits <float>::min ();
    ASSERT_FALSE (value.isFalse ());

    value = std::numeric_limits <float>::max ();
    ASSERT_FALSE (value.isFalse ());

    value = std::numeric_limits <double>::min ();
    ASSERT_FALSE (value.isFalse ());

    value = std::numeric_limits <double>::max ();
    ASSERT_FALSE (value.isFalse ());

    value = "";
    ASSERT_THROW (value.isFalse (), std::bad_cast);

    value = "foo";
    ASSERT_THROW (value.isFalse (), std::bad_cast);

    value = "02:42:64:2f:6a:d0";
    ASSERT_THROW (value.isFalse (), std::bad_cast);

    value = "127.0.0.1";
    ASSERT_THROW (value.isFalse (), std::bad_cast);

    value = Array ({1});
    ASSERT_THROW (value.isFalse (), std::bad_cast);

    value = Object ({{"i", 1}});
    ASSERT_THROW (value.isFalse (), std::bad_cast);
}

/**
 * @brief Test isNumber method.
 */
TEST (Value, isNumber)
{
    Value value;
    ASSERT_FALSE (value.isNumber ());

    value = nullptr;
    ASSERT_FALSE (value.isNumber ());

    value = true;
    ASSERT_FALSE (value.isNumber ());

    value = false;
    ASSERT_FALSE (value.isNumber ());

    value = std::numeric_limits <int8_t>::min ();
    ASSERT_TRUE (value.isNumber ());

    value = std::numeric_limits <int8_t>::max ();
    ASSERT_TRUE (value.isNumber ());

    value = std::numeric_limits <uint8_t>::min ();
    ASSERT_TRUE (value.isNumber ());

    value = std::numeric_limits <uint8_t>::max ();
    ASSERT_TRUE (value.isNumber ());

    value = std::numeric_limits <int16_t>::min ();
    ASSERT_TRUE (value.isNumber ());

    value = std::numeric_limits <int16_t>::max ();
    ASSERT_TRUE (value.isNumber ());

    value = std::numeric_limits <uint16_t>::min ();
    ASSERT_TRUE (value.isNumber ());

    value = std::numeric_limits <uint16_t>::max ();
    ASSERT_TRUE (value.isNumber ());

    value = std::numeric_limits <int32_t>::min ();
    ASSERT_TRUE (value.isNumber ());

    value = std::numeric_limits <int32_t>::max ();
    ASSERT_TRUE (value.isNumber ());

    value = std::numeric_limits <uint32_t>::min ();
    ASSERT_TRUE (value.isNumber ());

    value = std::numeric_limits <uint32_t>::max ();
    ASSERT_TRUE (value.isNumber ());

    value = std::numeric_limits <int64_t>::min ();
    ASSERT_TRUE (value.isNumber ());

    value = std::numeric_limits <int64_t>::max ();
    ASSERT_TRUE (value.isNumber ());

    value = std::numeric_limits <uint64_t>::min ();
    ASSERT_TRUE (value.isNumber ());

    value = std::numeric_limits <uint64_t>::max ();
    ASSERT_TRUE (value.isNumber ());

    value = std::numeric_limits <float>::min ();
    ASSERT_TRUE (value.isNumber ());

    value = std::numeric_limits <float>::max ();
    ASSERT_TRUE (value.isNumber ());

    value = std::numeric_limits <double>::min ();
    ASSERT_TRUE (value.isNumber ());

    value = std::numeric_limits <double>::max ();
    ASSERT_TRUE (value.isNumber ());

    value = "";
    ASSERT_FALSE (value.isNumber ());

    value = "foo";
    ASSERT_FALSE (value.isNumber ());

    value = "02:42:64:2f:6a:d0";
    ASSERT_FALSE (value.isNumber ());

    value = "127.0.0.1";
    ASSERT_FALSE (value.isNumber ());

    value = Array ({1});
    ASSERT_FALSE (value.isNumber ());

    value = Object ({{"i", 1}});
    ASSERT_FALSE (value.isNumber ());
}

/**
 * @brief Test isInt8 method.
 */
TEST (Value, isInt8)
{
    Value value;
    ASSERT_FALSE (value.isInt8 ());

    value = nullptr;
    ASSERT_FALSE (value.isInt8 ());

    value = true;
    ASSERT_FALSE (value.isInt8 ());

    value = false;
    ASSERT_FALSE (value.isInt8 ());

    value = std::numeric_limits <int8_t>::min ();
    ASSERT_TRUE (value.isInt8 ());

    value = std::numeric_limits <int8_t>::max ();
    ASSERT_TRUE (value.isInt8 ());

    value = std::numeric_limits <uint8_t>::min ();
    ASSERT_TRUE (value.isInt8 ());

    value = std::numeric_limits <uint8_t>::max ();
    ASSERT_FALSE (value.isInt8 ());

    value = std::numeric_limits <int16_t>::min ();
    ASSERT_FALSE (value.isInt8 ());

    value = std::numeric_limits <int16_t>::max ();
    ASSERT_FALSE (value.isInt8 ());

    value = std::numeric_limits <uint16_t>::min ();
    ASSERT_TRUE (value.isInt8 ());

    value = std::numeric_limits <uint16_t>::max ();
    ASSERT_FALSE (value.isInt8 ());

    value = std::numeric_limits <int32_t>::min ();
    ASSERT_FALSE (value.isInt8 ());

    value = std::numeric_limits <int32_t>::max ();
    ASSERT_FALSE (value.isInt8 ());

    value = std::numeric_limits <uint32_t>::min ();
    ASSERT_TRUE (value.isInt8 ());

    value = std::numeric_limits <uint32_t>::max ();
    ASSERT_FALSE (value.isInt8 ());

    value = std::numeric_limits <int64_t>::min ();
    ASSERT_FALSE (value.isInt8 ());

    value = std::numeric_limits <int64_t>::max ();
    ASSERT_FALSE (value.isInt8 ());

    value = std::numeric_limits <uint64_t>::min ();
    ASSERT_TRUE (value.isInt8 ());

    value = std::numeric_limits <uint64_t>::max ();
    ASSERT_FALSE (value.isInt8 ());

    value = std::numeric_limits <float>::min ();
    ASSERT_FALSE (value.isInt8 ());

    value = std::numeric_limits <float>::max ();
    ASSERT_FALSE (value.isInt8 ());

    value = std::numeric_limits <double>::min ();
    ASSERT_FALSE (value.isInt8 ());

    value = std::numeric_limits <double>::max ();
    ASSERT_FALSE (value.isInt8 ());

    value = "";
    ASSERT_FALSE (value.isInt8 ());

    value = "foo";
    ASSERT_FALSE (value.isInt8 ());

    value = "02:42:64:2f:6a:d0";
    ASSERT_FALSE (value.isInt8 ());

    value = "127.0.0.1";
    ASSERT_FALSE (value.isInt8 ());

    value = Array ({1});
    ASSERT_FALSE (value.isInt8 ());

    value = Object ({{"i", 1}});
    ASSERT_FALSE (value.isInt8 ());
}

/**
 * @brief Test isUint8 method.
 */
TEST (Value, isUint8)
{
    Value value;
    ASSERT_FALSE (value.isUint8 ());

    value = nullptr;
    ASSERT_FALSE (value.isUint8 ());

    value = true;
    ASSERT_FALSE (value.isUint8 ());

    value = false;
    ASSERT_FALSE (value.isUint8 ());

    value = std::numeric_limits <int8_t>::min ();
    ASSERT_FALSE (value.isUint8 ());

    value = std::numeric_limits <int8_t>::max ();
    ASSERT_TRUE (value.isUint8 ());

    value = std::numeric_limits <uint8_t>::min ();
    ASSERT_TRUE (value.isUint8 ());

    value = std::numeric_limits <uint8_t>::max ();
    ASSERT_TRUE (value.isUint8 ());

    value = std::numeric_limits <int16_t>::min ();
    ASSERT_FALSE (value.isUint8 ());

    value = std::numeric_limits <int16_t>::max ();
    ASSERT_FALSE (value.isUint8 ());

    value = std::numeric_limits <uint16_t>::min ();
    ASSERT_TRUE (value.isUint8 ());

    value = std::numeric_limits <uint16_t>::max ();
    ASSERT_FALSE (value.isUint8 ());

    value = std::numeric_limits <int32_t>::min ();
    ASSERT_FALSE (value.isUint8 ());

    value = std::numeric_limits <int32_t>::max ();
    ASSERT_FALSE (value.isUint8 ());

    value = std::numeric_limits <uint32_t>::min ();
    ASSERT_TRUE (value.isUint8 ());

    value = std::numeric_limits <uint32_t>::max ();
    ASSERT_FALSE (value.isUint8 ());

    value = std::numeric_limits <int64_t>::min ();
    ASSERT_FALSE (value.isUint8 ());

    value = std::numeric_limits <int64_t>::max ();
    ASSERT_FALSE (value.isUint8 ());

    value = std::numeric_limits <uint64_t>::min ();
    ASSERT_TRUE (value.isUint8 ());

    value = std::numeric_limits <uint64_t>::max ();
    ASSERT_FALSE (value.isUint8 ());

    value = std::numeric_limits <float>::min ();
    ASSERT_FALSE (value.isUint8 ());

    value = std::numeric_limits <float>::max ();
    ASSERT_FALSE (value.isUint8 ());

    value = std::numeric_limits <double>::min ();
    ASSERT_FALSE (value.isUint8 ());

    value = std::numeric_limits <double>::max ();
    ASSERT_FALSE (value.isUint8 ());

    value = "";
    ASSERT_FALSE (value.isUint8 ());

    value = "foo";
    ASSERT_FALSE (value.isUint8 ());

    value = "02:42:64:2f:6a:d0";
    ASSERT_FALSE (value.isUint8 ());

    value = "127.0.0.1";
    ASSERT_FALSE (value.isUint8 ());

    value = Array ({1});
    ASSERT_FALSE (value.isUint8 ());

    value = Object ({{"i", 1}});
    ASSERT_FALSE (value.isUint8 ());
}

/**
 * @brief Test isInt16 method.
 */
TEST (Value, isInt16)
{
    Value value;
    ASSERT_FALSE (value.isInt16 ());

    value = nullptr;
    ASSERT_FALSE (value.isInt16 ());

    value = true;
    ASSERT_FALSE (value.isInt16 ());

    value = false;
    ASSERT_FALSE (value.isInt16 ());

    value = std::numeric_limits <int8_t>::min ();
    ASSERT_TRUE (value.isInt16 ());

    value = std::numeric_limits <int8_t>::max ();
    ASSERT_TRUE (value.isInt16 ());

    value = std::numeric_limits <uint8_t>::min ();
    ASSERT_TRUE (value.isInt16 ());

    value = std::numeric_limits <uint8_t>::max ();
    ASSERT_TRUE (value.isInt16 ());

    value = std::numeric_limits <int16_t>::min ();
    ASSERT_TRUE (value.isInt16 ());

    value = std::numeric_limits <int16_t>::max ();
    ASSERT_TRUE (value.isInt16 ());

    value = std::numeric_limits <uint16_t>::min ();
    ASSERT_TRUE (value.isInt16 ());

    value = std::numeric_limits <uint16_t>::max ();
    ASSERT_FALSE (value.isInt16 ());

    value = std::numeric_limits <int32_t>::min ();
    ASSERT_FALSE (value.isInt16 ());

    value = std::numeric_limits <int32_t>::max ();
    ASSERT_FALSE (value.isInt16 ());

    value = std::numeric_limits <uint32_t>::min ();
    ASSERT_TRUE (value.isInt16 ());

    value = std::numeric_limits <uint32_t>::max ();
    ASSERT_FALSE (value.isInt16 ());

    value = std::numeric_limits <int64_t>::min ();
    ASSERT_FALSE (value.isInt16 ());

    value = std::numeric_limits <int64_t>::max ();
    ASSERT_FALSE (value.isInt16 ());

    value = std::numeric_limits <uint64_t>::min ();
    ASSERT_TRUE (value.isInt16 ());

    value = std::numeric_limits <uint64_t>::max ();
    ASSERT_FALSE (value.isInt16 ());

    value = std::numeric_limits <float>::min ();
    ASSERT_FALSE (value.isInt16 ());

    value = std::numeric_limits <float>::max ();
    ASSERT_FALSE (value.isInt16 ());

    value = std::numeric_limits <double>::min ();
    ASSERT_FALSE (value.isInt16 ());

    value = std::numeric_limits <double>::max ();
    ASSERT_FALSE (value.isInt16 ());

    value = "";
    ASSERT_FALSE (value.isInt16 ());

    value = "foo";
    ASSERT_FALSE (value.isInt16 ());

    value = "02:42:64:2f:6a:d0";
    ASSERT_FALSE (value.isInt16 ());

    value = "127.0.0.1";
    ASSERT_FALSE (value.isInt16 ());

    value = Array ({1});
    ASSERT_FALSE (value.isInt16 ());

    value = Object ({{"i", 1}});
    ASSERT_FALSE (value.isInt16 ());
}

/**
 * @brief Test isUint16 method.
 */
TEST (Value, isUint16)
{
    Value value;
    ASSERT_FALSE (value.isUint16 ());

    value = nullptr;
    ASSERT_FALSE (value.isUint16 ());

    value = true;
    ASSERT_FALSE (value.isUint16 ());

    value = false;
    ASSERT_FALSE (value.isUint16 ());

    value = std::numeric_limits <int8_t>::min ();
    ASSERT_FALSE (value.isUint16 ());

    value = std::numeric_limits <int8_t>::max ();
    ASSERT_TRUE (value.isUint16 ());

    value = std::numeric_limits <uint8_t>::min ();
    ASSERT_TRUE (value.isUint16 ());

    value = std::numeric_limits <uint8_t>::max ();
    ASSERT_TRUE (value.isUint16 ());

    value = std::numeric_limits <int16_t>::min ();
    ASSERT_FALSE (value.isUint16 ());

    value = std::numeric_limits <int16_t>::max ();
    ASSERT_TRUE (value.isUint16 ());

    value = std::numeric_limits <uint16_t>::min ();
    ASSERT_TRUE (value.isUint16 ());

    value = std::numeric_limits <uint16_t>::max ();
    ASSERT_TRUE (value.isUint16 ());

    value = std::numeric_limits <int32_t>::min ();
    ASSERT_FALSE (value.isUint16 ());

    value = std::numeric_limits <int32_t>::max ();
    ASSERT_FALSE (value.isUint16 ());

    value = std::numeric_limits <uint32_t>::min ();
    ASSERT_TRUE (value.isUint16 ());

    value = std::numeric_limits <uint32_t>::max ();
    ASSERT_FALSE (value.isUint16 ());

    value = std::numeric_limits <int64_t>::min ();
    ASSERT_FALSE (value.isUint16 ());

    value = std::numeric_limits <int64_t>::max ();
    ASSERT_FALSE (value.isUint16 ());

    value = std::numeric_limits <uint64_t>::min ();
    ASSERT_TRUE (value.isUint16 ());

    value = std::numeric_limits <uint64_t>::max ();
    ASSERT_FALSE (value.isUint16 ());

    value = std::numeric_limits <float>::min ();
    ASSERT_FALSE (value.isUint16 ());

    value = std::numeric_limits <float>::max ();
    ASSERT_FALSE (value.isUint16 ());

    value = std::numeric_limits <double>::min ();
    ASSERT_FALSE (value.isUint16 ());

    value = std::numeric_limits <double>::max ();
    ASSERT_FALSE (value.isUint16 ());

    value = "";
    ASSERT_FALSE (value.isUint16 ());

    value = "foo";
    ASSERT_FALSE (value.isUint16 ());

    value = "02:42:64:2f:6a:d0";
    ASSERT_FALSE (value.isUint16 ());

    value = "127.0.0.1";
    ASSERT_FALSE (value.isUint16 ());

    value = Array ({1});
    ASSERT_FALSE (value.isUint16 ());

    value = Object ({{"i", 1}});
    ASSERT_FALSE (value.isUint16 ());
}

/**
 * @brief Test isInt method.
 */
TEST (Value, isInt)
{
    Value value;
    ASSERT_FALSE (value.isInt ());

    value = nullptr;
    ASSERT_FALSE (value.isInt ());

    value = true;
    ASSERT_FALSE (value.isInt ());

    value = false;
    ASSERT_FALSE (value.isInt ());

    value = std::numeric_limits <int8_t>::min ();
    ASSERT_TRUE (value.isInt ());

    value = std::numeric_limits <int8_t>::max ();
    ASSERT_TRUE (value.isInt ());

    value = std::numeric_limits <uint8_t>::min ();
    ASSERT_TRUE (value.isInt ());

    value = std::numeric_limits <uint8_t>::max ();
    ASSERT_TRUE (value.isInt ());

    value = std::numeric_limits <int16_t>::min ();
    ASSERT_TRUE (value.isInt ());

    value = std::numeric_limits <int16_t>::max ();
    ASSERT_TRUE (value.isInt ());

    value = std::numeric_limits <uint16_t>::min ();
    ASSERT_TRUE (value.isInt ());

    value = std::numeric_limits <uint16_t>::max ();
    ASSERT_TRUE (value.isInt ());

    value = std::numeric_limits <int32_t>::min ();
    ASSERT_TRUE (value.isInt ());

    value = std::numeric_limits <int32_t>::max ();
    ASSERT_TRUE (value.isInt ());

    value = std::numeric_limits <uint32_t>::min ();
    ASSERT_TRUE (value.isInt ());

    value = std::numeric_limits <uint32_t>::max ();
    ASSERT_FALSE (value.isInt ());

    value = std::numeric_limits <int64_t>::min ();
    ASSERT_FALSE (value.isInt ());

    value = std::numeric_limits <int64_t>::max ();
    ASSERT_FALSE (value.isInt ());

    value = std::numeric_limits <uint64_t>::min ();
    ASSERT_TRUE (value.isInt ());

    value = std::numeric_limits <uint64_t>::max ();
    ASSERT_FALSE (value.isInt ());

    value = std::numeric_limits <float>::min ();
    ASSERT_FALSE (value.isInt ());

    value = std::numeric_limits <float>::max ();
    ASSERT_FALSE (value.isInt ());

    value = std::numeric_limits <double>::min ();
    ASSERT_FALSE (value.isInt ());

    value = std::numeric_limits <double>::max ();
    ASSERT_FALSE (value.isInt ());

    value = "";
    ASSERT_FALSE (value.isInt ());

    value = "foo";
    ASSERT_FALSE (value.isInt ());

    value = "02:42:64:2f:6a:d0";
    ASSERT_FALSE (value.isInt ());

    value = "127.0.0.1";
    ASSERT_FALSE (value.isInt ());

    value = Array ({1});
    ASSERT_FALSE (value.isInt ());

    value = Object ({{"i", 1}});
    ASSERT_FALSE (value.isInt ());
}

/**
 * @brief Test isUint method.
 */
TEST (Value, isUint)
{
    Value value;
    ASSERT_FALSE (value.isUint ());

    value = nullptr;
    ASSERT_FALSE (value.isUint ());

    value = true;
    ASSERT_FALSE (value.isUint ());

    value = false;
    ASSERT_FALSE (value.isUint ());

    value = std::numeric_limits <int8_t>::min ();
    ASSERT_FALSE (value.isUint ());

    value = std::numeric_limits <int8_t>::max ();
    ASSERT_TRUE (value.isUint ());

    value = std::numeric_limits <uint8_t>::min ();
    ASSERT_TRUE (value.isUint ());

    value = std::numeric_limits <uint8_t>::max ();
    ASSERT_TRUE (value.isUint ());

    value = std::numeric_limits <int16_t>::min ();
    ASSERT_FALSE (value.isUint ());

    value = std::numeric_limits <int16_t>::max ();
    ASSERT_TRUE (value.isUint ());

    value = std::numeric_limits <uint16_t>::min ();
    ASSERT_TRUE (value.isUint ());

    value = std::numeric_limits <uint16_t>::max ();
    ASSERT_TRUE (value.isUint ());

    value = std::numeric_limits <int32_t>::min ();
    ASSERT_FALSE (value.isUint ());

    value = std::numeric_limits <int32_t>::max ();
    ASSERT_TRUE (value.isUint ());

    value = std::numeric_limits <uint32_t>::min ();
    ASSERT_TRUE (value.isUint ());

    value = std::numeric_limits <uint32_t>::max ();
    ASSERT_TRUE (value.isUint ());

    value = std::numeric_limits <int64_t>::min ();
    ASSERT_FALSE (value.isUint ());

    value = std::numeric_limits <int64_t>::max ();
    ASSERT_FALSE (value.isUint ());

    value = std::numeric_limits <uint64_t>::min ();
    ASSERT_TRUE (value.isUint ());

    value = std::numeric_limits <uint64_t>::max ();
    ASSERT_FALSE (value.isUint ());

    value = std::numeric_limits <float>::min ();
    ASSERT_FALSE (value.isUint ());

    value = std::numeric_limits <float>::max ();
    ASSERT_FALSE (value.isUint ());

    value = std::numeric_limits <double>::min ();
    ASSERT_FALSE (value.isUint ());

    value = std::numeric_limits <double>::max ();
    ASSERT_FALSE (value.isUint ());

    value = "";
    ASSERT_FALSE (value.isUint ());

    value = "foo";
    ASSERT_FALSE (value.isUint ());

    value = "02:42:64:2f:6a:d0";
    ASSERT_FALSE (value.isUint ());

    value = "127.0.0.1";
    ASSERT_FALSE (value.isUint ());

    value = Array ({1});
    ASSERT_FALSE (value.isUint ());

    value = Object ({{"i", 1}});
    ASSERT_FALSE (value.isUint ());
}

/**
 * @brief Test isInt64 method.
 */
TEST (Value, isInt64)
{
    Value value;
    ASSERT_FALSE (value.isInt64 ());

    value = nullptr;
    ASSERT_FALSE (value.isInt64 ());

    value = true;
    ASSERT_FALSE (value.isInt64 ());

    value = false;
    ASSERT_FALSE (value.isInt64 ());

    value = std::numeric_limits <int8_t>::min ();
    ASSERT_TRUE (value.isInt64 ());

    value = std::numeric_limits <int8_t>::max ();
    ASSERT_TRUE (value.isInt64 ());

    value = std::numeric_limits <uint8_t>::min ();
    ASSERT_TRUE (value.isInt64 ());

    value = std::numeric_limits <uint8_t>::max ();
    ASSERT_TRUE (value.isInt64 ());

    value = std::numeric_limits <int16_t>::min ();
    ASSERT_TRUE (value.isInt64 ());

    value = std::numeric_limits <int16_t>::max ();
    ASSERT_TRUE (value.isInt64 ());

    value = std::numeric_limits <uint16_t>::min ();
    ASSERT_TRUE (value.isInt64 ());

    value = std::numeric_limits <uint16_t>::max ();
    ASSERT_TRUE (value.isInt64 ());

    value = std::numeric_limits <int32_t>::min ();
    ASSERT_TRUE (value.isInt64 ());

    value = std::numeric_limits <int32_t>::max ();
    ASSERT_TRUE (value.isInt64 ());

    value = std::numeric_limits <uint32_t>::min ();
    ASSERT_TRUE (value.isInt64 ());

    value = std::numeric_limits <uint32_t>::max ();
    ASSERT_TRUE (value.isInt64 ());

    value = std::numeric_limits <int64_t>::min ();
    ASSERT_TRUE (value.isInt64 ());

    value = std::numeric_limits <int64_t>::max ();
    ASSERT_TRUE (value.isInt64 ());

    value = std::numeric_limits <uint64_t>::min ();
    ASSERT_TRUE (value.isInt64 ());

    value = std::numeric_limits <uint64_t>::max ();
    ASSERT_FALSE (value.isInt64 ());

    value = std::numeric_limits <float>::min ();
    ASSERT_FALSE (value.isInt64 ());

    value = std::numeric_limits <float>::max ();
    ASSERT_FALSE (value.isInt64 ());

    value = std::numeric_limits <double>::min ();
    ASSERT_FALSE (value.isInt64 ());

    value = std::numeric_limits <double>::max ();
    ASSERT_FALSE (value.isInt64 ());

    value = "";
    ASSERT_FALSE (value.isInt64 ());

    value = "foo";
    ASSERT_FALSE (value.isInt64 ());

    value = "02:42:64:2f:6a:d0";
    ASSERT_FALSE (value.isInt64 ());

    value = "127.0.0.1";
    ASSERT_FALSE (value.isInt64 ());

    value = Array ({1});
    ASSERT_FALSE (value.isInt64 ());

    value = Object ({{"i", 1}});
    ASSERT_FALSE (value.isInt64 ());
}

/**
 * @brief Test isUint64 method.
 */
TEST (Value, isUint64)
{
    Value value;
    ASSERT_FALSE (value.isUint64 ());

    value = nullptr;
    ASSERT_FALSE (value.isUint64 ());

    value = true;
    ASSERT_FALSE (value.isUint64 ());

    value = false;
    ASSERT_FALSE (value.isUint64 ());

    value = std::numeric_limits <int8_t>::min ();
    ASSERT_FALSE (value.isUint64 ());

    value = std::numeric_limits <int8_t>::max ();
    ASSERT_TRUE (value.isUint64 ());

    value = std::numeric_limits <uint8_t>::min ();
    ASSERT_TRUE (value.isUint64 ());

    value = std::numeric_limits <uint8_t>::max ();
    ASSERT_TRUE (value.isUint64 ());

    value = std::numeric_limits <int16_t>::min ();
    ASSERT_FALSE (value.isUint64 ());

    value = std::numeric_limits <int16_t>::max ();
    ASSERT_TRUE (value.isUint64 ());

    value = std::numeric_limits <uint16_t>::min ();
    ASSERT_TRUE (value.isUint64 ());

    value = std::numeric_limits <uint16_t>::max ();
    ASSERT_TRUE (value.isUint64 ());

    value = std::numeric_limits <int32_t>::min ();
    ASSERT_FALSE (value.isUint64 ());

    value = std::numeric_limits <int32_t>::max ();
    ASSERT_TRUE (value.isUint64 ());

    value = std::numeric_limits <uint32_t>::min ();
    ASSERT_TRUE (value.isUint64 ());

    value = std::numeric_limits <uint32_t>::max ();
    ASSERT_TRUE (value.isUint64 ());

    value = std::numeric_limits <int64_t>::min ();
    ASSERT_FALSE (value.isUint64 ());

    value = std::numeric_limits <int64_t>::max ();
    ASSERT_TRUE (value.isUint64 ());

    value = std::numeric_limits <uint64_t>::min ();
    ASSERT_TRUE (value.isUint64 ());

    value = std::numeric_limits <uint64_t>::max ();
    ASSERT_TRUE (value.isUint64 ());

    value = std::numeric_limits <float>::min ();
    ASSERT_FALSE (value.isUint64 ());

    value = std::numeric_limits <float>::max ();
    ASSERT_FALSE (value.isUint64 ());

    value = std::numeric_limits <double>::min ();
    ASSERT_FALSE (value.isUint64 ());

    value = std::numeric_limits <double>::max ();
    ASSERT_FALSE (value.isUint64 ());

    value = "";
    ASSERT_FALSE (value.isUint64 ());

    value = "foo";
    ASSERT_FALSE (value.isUint64 ());

    value = "02:42:64:2f:6a:d0";
    ASSERT_FALSE (value.isUint64 ());

    value = "127.0.0.1";
    ASSERT_FALSE (value.isUint64 ());

    value = Array ({1});
    ASSERT_FALSE (value.isUint64 ());

    value = Object ({{"i", 1}});
    ASSERT_FALSE (value.isUint64 ());
}

/**
 * @brief Test isFloat method.
 */
TEST (Value, isFloat)
{
    Value value;
    ASSERT_FALSE (value.isFloat ());

    value = nullptr;
    ASSERT_FALSE (value.isFloat ());

    value = true;
    ASSERT_FALSE (value.isFloat ());

    value = false;
    ASSERT_FALSE (value.isFloat ());

    value = std::numeric_limits <int8_t>::min ();
    ASSERT_TRUE (value.isFloat ());

    value = std::numeric_limits <int8_t>::max ();
    ASSERT_TRUE (value.isFloat ());

    value = std::numeric_limits <uint8_t>::min ();
    ASSERT_TRUE (value.isFloat ());

    value = std::numeric_limits <uint8_t>::max ();
    ASSERT_TRUE (value.isFloat ());

    value = std::numeric_limits <int16_t>::min ();
    ASSERT_TRUE (value.isFloat ());

    value = std::numeric_limits <int16_t>::max ();
    ASSERT_TRUE (value.isFloat ());

    value = std::numeric_limits <uint16_t>::min ();
    ASSERT_TRUE (value.isFloat ());

    value = std::numeric_limits <uint16_t>::max ();
    ASSERT_TRUE (value.isFloat ());

    value = std::numeric_limits <int32_t>::min ();
    ASSERT_TRUE (value.isFloat ());

    value = std::numeric_limits <int32_t>::max ();
    ASSERT_TRUE (value.isFloat ());

    value = std::numeric_limits <uint32_t>::min ();
    ASSERT_TRUE (value.isFloat ());

    value = std::numeric_limits <uint32_t>::max ();
    ASSERT_TRUE (value.isFloat ());

    value = std::numeric_limits <int64_t>::min ();
    ASSERT_TRUE (value.isFloat ());

    value = std::numeric_limits <int64_t>::max ();
    ASSERT_TRUE (value.isFloat ());

    value = std::numeric_limits <uint64_t>::min ();
    ASSERT_TRUE (value.isFloat ());

    value = std::numeric_limits <uint64_t>::max ();
    ASSERT_TRUE (value.isFloat ());

    value = std::numeric_limits <float>::min ();
    ASSERT_TRUE (value.isFloat ());

    value = std::numeric_limits <float>::max ();
    ASSERT_TRUE (value.isFloat ());

    value = std::numeric_limits <double>::min ();
    ASSERT_TRUE (value.isFloat ());

    value = std::numeric_limits <double>::max ();
    ASSERT_TRUE (value.isFloat ());

    value = "";
    ASSERT_FALSE (value.isFloat ());

    value = "foo";
    ASSERT_FALSE (value.isFloat ());

    value = "02:42:64:2f:6a:d0";
    ASSERT_FALSE (value.isFloat ());

    value = "127.0.0.1";
    ASSERT_FALSE (value.isFloat ());

    value = Array ({1});
    ASSERT_FALSE (value.isFloat ());

    value = Object ({{"i", 1}});
    ASSERT_FALSE (value.isFloat ());
}

/**
 * @brief Test isDouble method.
 */
TEST (Value, isDouble)
{
    Value value;
    ASSERT_FALSE (value.isDouble ());

    value = nullptr;
    ASSERT_FALSE (value.isDouble ());

    value = true;
    ASSERT_FALSE (value.isDouble ());

    value = false;
    ASSERT_FALSE (value.isDouble ());

    value = std::numeric_limits <int8_t>::min ();
    ASSERT_TRUE (value.isDouble ());

    value = std::numeric_limits <int8_t>::max ();
    ASSERT_TRUE (value.isDouble ());

    value = std::numeric_limits <uint8_t>::min ();
    ASSERT_TRUE (value.isDouble ());

    value = std::numeric_limits <uint8_t>::max ();
    ASSERT_TRUE (value.isDouble ());

    value = std::numeric_limits <int16_t>::min ();
    ASSERT_TRUE (value.isDouble ());

    value = std::numeric_limits <int16_t>::max ();
    ASSERT_TRUE (value.isDouble ());

    value = std::numeric_limits <uint16_t>::min ();
    ASSERT_TRUE (value.isDouble ());

    value = std::numeric_limits <uint16_t>::max ();
    ASSERT_TRUE (value.isDouble ());

    value = std::numeric_limits <int32_t>::min ();
    ASSERT_TRUE (value.isDouble ());

    value = std::numeric_limits <int32_t>::max ();
    ASSERT_TRUE (value.isDouble ());

    value = std::numeric_limits <uint32_t>::min ();
    ASSERT_TRUE (value.isDouble ());

    value = std::numeric_limits <uint32_t>::max ();
    ASSERT_TRUE (value.isDouble ());

    value = std::numeric_limits <int64_t>::min ();
    ASSERT_TRUE (value.isDouble ());

    value = std::numeric_limits <int64_t>::max ();
    ASSERT_TRUE (value.isDouble ());

    value = std::numeric_limits <uint64_t>::min ();
    ASSERT_TRUE (value.isDouble ());

    value = std::numeric_limits <uint64_t>::max ();
    ASSERT_TRUE (value.isDouble ());

    value = std::numeric_limits <float>::min ();
    ASSERT_TRUE (value.isDouble ());

    value = std::numeric_limits <float>::max ();
    ASSERT_TRUE (value.isDouble ());

    value = std::numeric_limits <double>::min ();
    ASSERT_TRUE (value.isDouble ());

    value = std::numeric_limits <double>::max ();
    ASSERT_TRUE (value.isDouble ());

    value = "";
    ASSERT_FALSE (value.isDouble ());

    value = "foo";
    ASSERT_FALSE (value.isDouble ());

    value = "02:42:64:2f:6a:d0";
    ASSERT_FALSE (value.isDouble ());

    value = "127.0.0.1";
    ASSERT_FALSE (value.isDouble ());

    value = Array ({1});
    ASSERT_FALSE (value.isDouble ());

    value = Object ({{"i", 1}});
    ASSERT_FALSE (value.isDouble ());
}

/**
 * @brief Test isString method.
 */
TEST (Value, isString)
{
    Value value;
    ASSERT_FALSE (value.isString ());

    value = nullptr;
    ASSERT_FALSE (value.isString ());

    value = true;
    ASSERT_FALSE (value.isString ());

    value = false;
    ASSERT_FALSE (value.isString ());

    value = std::numeric_limits <int8_t>::min ();
    ASSERT_FALSE (value.isString ());

    value = std::numeric_limits <int8_t>::max ();
    ASSERT_FALSE (value.isString ());

    value = std::numeric_limits <uint8_t>::min ();
    ASSERT_FALSE (value.isString ());

    value = std::numeric_limits <uint8_t>::max ();
    ASSERT_FALSE (value.isString ());

    value = std::numeric_limits <int16_t>::min ();
    ASSERT_FALSE (value.isString ());

    value = std::numeric_limits <int16_t>::max ();
    ASSERT_FALSE (value.isString ());

    value = std::numeric_limits <uint16_t>::min ();
    ASSERT_FALSE (value.isString ());

    value = std::numeric_limits <uint16_t>::max ();
    ASSERT_FALSE (value.isString ());

    value = std::numeric_limits <int32_t>::min ();
    ASSERT_FALSE (value.isString ());

    value = std::numeric_limits <int32_t>::max ();
    ASSERT_FALSE (value.isString ());

    value = std::numeric_limits <uint32_t>::min ();
    ASSERT_FALSE (value.isString ());

    value = std::numeric_limits <uint32_t>::max ();
    ASSERT_FALSE (value.isString ());

    value = std::numeric_limits <int64_t>::min ();
    ASSERT_FALSE (value.isString ());

    value = std::numeric_limits <int64_t>::max ();
    ASSERT_FALSE (value.isString ());

    value = std::numeric_limits <uint64_t>::min ();
    ASSERT_FALSE (value.isString ());

    value = std::numeric_limits <uint64_t>::max ();
    ASSERT_FALSE (value.isString ());

    value = std::numeric_limits <float>::min ();
    ASSERT_FALSE (value.isString ());

    value = std::numeric_limits <float>::max ();
    ASSERT_FALSE (value.isString ());

    value = std::numeric_limits <double>::min ();
    ASSERT_FALSE (value.isString ());

    value = std::numeric_limits <double>::max ();
    ASSERT_FALSE (value.isString ());

    value = "";
    ASSERT_TRUE (value.isString ());

    value = "foo";
    ASSERT_TRUE (value.isString ());

    value = "02:42:64:2f:6a:d0";
    ASSERT_TRUE (value.isString ());

    value = "127.0.0.1";
    ASSERT_TRUE (value.isString ());

    value = Array ({1});
    ASSERT_FALSE (value.isString ());

    value = Object ({{"i", 1}});
    ASSERT_FALSE (value.isString ());
}

/**
 * @brief Test isArray method.
 */
TEST (Value, isArray)
{
    Value value;
    ASSERT_FALSE (value.isArray ());

    value = nullptr;
    ASSERT_FALSE (value.isArray ());

    value = true;
    ASSERT_FALSE (value.isArray ());

    value = false;
    ASSERT_FALSE (value.isArray ());

    value = std::numeric_limits <int8_t>::min ();
    ASSERT_FALSE (value.isArray ());

    value = std::numeric_limits <int8_t>::max ();
    ASSERT_FALSE (value.isArray ());

    value = std::numeric_limits <uint8_t>::min ();
    ASSERT_FALSE (value.isArray ());

    value = std::numeric_limits <uint8_t>::max ();
    ASSERT_FALSE (value.isArray ());

    value = std::numeric_limits <int16_t>::min ();
    ASSERT_FALSE (value.isArray ());

    value = std::numeric_limits <int16_t>::max ();
    ASSERT_FALSE (value.isArray ());

    value = std::numeric_limits <uint16_t>::min ();
    ASSERT_FALSE (value.isArray ());

    value = std::numeric_limits <uint16_t>::max ();
    ASSERT_FALSE (value.isArray ());

    value = std::numeric_limits <int32_t>::min ();
    ASSERT_FALSE (value.isArray ());

    value = std::numeric_limits <int32_t>::max ();
    ASSERT_FALSE (value.isArray ());

    value = std::numeric_limits <uint32_t>::min ();
    ASSERT_FALSE (value.isArray ());

    value = std::numeric_limits <uint32_t>::max ();
    ASSERT_FALSE (value.isArray ());

    value = std::numeric_limits <int64_t>::min ();
    ASSERT_FALSE (value.isArray ());

    value = std::numeric_limits <int64_t>::max ();
    ASSERT_FALSE (value.isArray ());

    value = std::numeric_limits <uint64_t>::min ();
    ASSERT_FALSE (value.isArray ());

    value = std::numeric_limits <uint64_t>::max ();
    ASSERT_FALSE (value.isArray ());

    value = std::numeric_limits <float>::min ();
    ASSERT_FALSE (value.isArray ());

    value = std::numeric_limits <float>::max ();
    ASSERT_FALSE (value.isArray ());

    value = std::numeric_limits <double>::min ();
    ASSERT_FALSE (value.isArray ());

    value = std::numeric_limits <double>::max ();
    ASSERT_FALSE (value.isArray ());

    value = "";
    ASSERT_FALSE (value.isArray ());

    value = "foo";
    ASSERT_FALSE (value.isArray ());

    value = "02:42:64:2f:6a:d0";
    ASSERT_FALSE (value.isArray ());

    value = "127.0.0.1";
    ASSERT_FALSE (value.isArray ());

    value = Array ({1});
    ASSERT_TRUE (value.isArray ());

    value = Object ({{"i", 1}});
    ASSERT_FALSE (value.isArray ());
}

/**
 * @brief Test isObject method.
 */
TEST (Value, isObject)
{
    Value value;
    ASSERT_FALSE (value.isObject ());

    value = nullptr;
    ASSERT_FALSE (value.isObject ());

    value = true;
    ASSERT_FALSE (value.isObject ());

    value = false;
    ASSERT_FALSE (value.isObject ());

    value = std::numeric_limits <int8_t>::min ();
    ASSERT_FALSE (value.isObject ());

    value = std::numeric_limits <int8_t>::max ();
    ASSERT_FALSE (value.isObject ());

    value = std::numeric_limits <uint8_t>::min ();
    ASSERT_FALSE (value.isObject ());

    value = std::numeric_limits <uint8_t>::max ();
    ASSERT_FALSE (value.isObject ());

    value = std::numeric_limits <int16_t>::min ();
    ASSERT_FALSE (value.isObject ());

    value = std::numeric_limits <int16_t>::max ();
    ASSERT_FALSE (value.isObject ());

    value = std::numeric_limits <uint16_t>::min ();
    ASSERT_FALSE (value.isObject ());

    value = std::numeric_limits <uint16_t>::max ();
    ASSERT_FALSE (value.isObject ());

    value = std::numeric_limits <int32_t>::min ();
    ASSERT_FALSE (value.isObject ());

    value = std::numeric_limits <int32_t>::max ();
    ASSERT_FALSE (value.isObject ());

    value = std::numeric_limits <uint32_t>::min ();
    ASSERT_FALSE (value.isObject ());

    value = std::numeric_limits <uint32_t>::max ();
    ASSERT_FALSE (value.isObject ());

    value = std::numeric_limits <int64_t>::min ();
    ASSERT_FALSE (value.isObject ());

    value = std::numeric_limits <int64_t>::max ();
    ASSERT_FALSE (value.isObject ());

    value = std::numeric_limits <uint64_t>::min ();
    ASSERT_FALSE (value.isObject ());

    value = std::numeric_limits <uint64_t>::max ();
    ASSERT_FALSE (value.isObject ());

    value = std::numeric_limits <float>::min ();
    ASSERT_FALSE (value.isObject ());

    value = std::numeric_limits <float>::max ();
    ASSERT_FALSE (value.isObject ());

    value = std::numeric_limits <double>::min ();
    ASSERT_FALSE (value.isObject ());

    value = std::numeric_limits <double>::max ();
    ASSERT_FALSE (value.isObject ());

    value = "";
    ASSERT_FALSE (value.isObject ());

    value = "foo";
    ASSERT_FALSE (value.isObject ());

    value = "02:42:64:2f:6a:d0";
    ASSERT_FALSE (value.isObject ());

    value = "127.0.0.1";
    ASSERT_FALSE (value.isObject ());

    value = Array ({1});
    ASSERT_FALSE (value.isObject ());

    value = Object ({{"i", 1}});
    ASSERT_TRUE (value.isObject ());
}

/**
 * @brief Test set method.
 */
TEST (Value, set)
{
    Value value;

    value.set <std::nullptr_t> (nullptr);
    ASSERT_TRUE (value.is <std::nullptr_t> ());

    value.set <Value::Null> (nullptr);
    ASSERT_TRUE (value.is <Value::Null> ());

    value.set <bool> (true);
    ASSERT_TRUE (value.is <bool> ());
    ASSERT_TRUE (value.get <bool> ());

    value.set <Value::Boolean> (false);
    ASSERT_TRUE (value.is <bool> ());
    ASSERT_FALSE (value.get <bool> ());

    value.set <int32_t> (std::numeric_limits <int32_t>::min ());
    ASSERT_TRUE (value.is <int32_t> ());
    ASSERT_EQ (value.get <int32_t> (), std::numeric_limits <int32_t>::min ());

    value.set <Value::Integer> (std::numeric_limits <int>::min ());
    ASSERT_TRUE (value.is <Value::Integer> ());
    ASSERT_EQ (value.get <Value::Integer> (), std::numeric_limits <int>::min ());

    value.set <uint32_t> (std::numeric_limits <uint32_t>::max ());
    ASSERT_TRUE (value.is <uint32_t> ());
    ASSERT_EQ (value.get <uint32_t> (), std::numeric_limits <uint32_t>::max ());

    value.set <Value::Unsigned> (std::numeric_limits <unsigned int>::max ());
    ASSERT_TRUE (value.is <Value::Unsigned> ());
    ASSERT_EQ (value.get <Value::Unsigned> (), std::numeric_limits <unsigned int>::max ());

    value.set <int64_t> (std::numeric_limits <int64_t>::min ());
    ASSERT_TRUE (value.is <int64_t> ());
    ASSERT_EQ (value.get <int64_t> (), std::numeric_limits <int64_t>::min ());

    value.set <Value::Integer64> (std::numeric_limits <int64_t>::min ());
    ASSERT_TRUE (value.is <Value::Integer64> ());
    ASSERT_EQ (value.get <Value::Integer64> (), std::numeric_limits <int64_t>::min ());

    value.set <uint64_t> (std::numeric_limits <uint64_t>::max ());
    ASSERT_TRUE (value.is <uint64_t> ());
    ASSERT_EQ (value.get <uint64_t> (), std::numeric_limits <uint64_t>::max ());

    value.set <Value::Unsigned64> (std::numeric_limits <uint64_t>::max ());
    ASSERT_TRUE (value.is <Value::Unsigned64> ());
    ASSERT_EQ (value.get <Value::Unsigned64> (), std::numeric_limits <uint64_t>::max ());

    value.set <double> (std::numeric_limits <double>::min ());
    ASSERT_TRUE (value.is <double> ());
    ASSERT_DOUBLE_EQ (value.get <double> (), std::numeric_limits <double>::min ());

    value.set <Value::Real> (std::numeric_limits <double>::min ());
    ASSERT_TRUE (value.is <Value::Real> ());
    ASSERT_DOUBLE_EQ (value.get <Value::Real> (), std::numeric_limits <double>::min ());

    value.set <std::string> ("foobar");
    ASSERT_TRUE (value.is <std::string> ());
    ASSERT_EQ (value.get <std::string> (), "foobar");

    value.set <Value::String> ("foobar");
    ASSERT_TRUE (value.is <Value::String> ());
    ASSERT_EQ (value.get <Value::String> (), "foobar");

    value.set <Array, Array::value_type> ({1});
    ASSERT_TRUE (value.is <Array> ());
    ASSERT_EQ (value.get <Array> (), Array ({1}));

    value.set <Object, Object::value_type> ({{"i", 1}});
    ASSERT_TRUE (value.is <Object> ());
    ASSERT_EQ (value.get <Object> (), Object ({{"i", 1}}));
}

/**
 * @brief Test get method.
 */
TEST (Value, get)
{
    Value value;

    value.set <std::nullptr_t> (nullptr);
    ASSERT_NO_THROW (value.get <std::nullptr_t> ());
    ASSERT_THROW (value.get <bool> (), std::bad_cast);
    ASSERT_THROW (value.get <int32_t> (), std::bad_cast);
    ASSERT_THROW (value.get <uint32_t> (), std::bad_cast);
    ASSERT_THROW (value.get <int64_t> (), std::bad_cast);
    ASSERT_THROW (value.get <uint64_t> (), std::bad_cast);
    ASSERT_THROW (value.get <double> (), std::bad_cast);
    ASSERT_THROW (value.get <std::string> (), std::bad_cast);
    ASSERT_THROW (value.get <Array> (), std::bad_cast);
    ASSERT_THROW (value.get <Object> (), std::bad_cast);

    value.set <bool> (false);
    ASSERT_THROW (value.get <std::nullptr_t> (), std::bad_cast);
    ASSERT_NO_THROW (value.get <bool> ());
    ASSERT_THROW (value.get <int32_t> (), std::bad_cast);
    ASSERT_THROW (value.get <uint32_t> (), std::bad_cast);
    ASSERT_THROW (value.get <int64_t> (), std::bad_cast);
    ASSERT_THROW (value.get <uint64_t> (), std::bad_cast);
    ASSERT_THROW (value.get <double> (), std::bad_cast);
    ASSERT_THROW (value.get <std::string> (), std::bad_cast);
    ASSERT_THROW (value.get <Array> (), std::bad_cast);
    ASSERT_THROW (value.get <Object> (), std::bad_cast);

    value.set <int32_t> (std::numeric_limits <int32_t>::min ());
    ASSERT_THROW (value.get <std::nullptr_t> (), std::bad_cast);
    ASSERT_THROW (value.get <bool> (), std::bad_cast);
    ASSERT_NO_THROW (value.get <int32_t> ());
    ASSERT_THROW (value.get <uint32_t> (), std::bad_cast);
    ASSERT_THROW (value.get <int64_t> (), std::bad_cast);
    ASSERT_THROW (value.get <uint64_t> (), std::bad_cast);
    ASSERT_THROW (value.get <double> (), std::bad_cast);
    ASSERT_THROW (value.get <std::string> (), std::bad_cast);
    ASSERT_THROW (value.get <Array> (), std::bad_cast);
    ASSERT_THROW (value.get <Object> (), std::bad_cast);

    value.set <uint32_t> (std::numeric_limits <uint32_t>::max ());
    ASSERT_THROW (value.get <std::nullptr_t> (), std::bad_cast);
    ASSERT_THROW (value.get <bool> (), std::bad_cast);
    ASSERT_THROW (value.get <int32_t> (), std::bad_cast);
    ASSERT_NO_THROW (value.get <uint32_t> ());
    ASSERT_THROW (value.get <int64_t> (), std::bad_cast);
    ASSERT_THROW (value.get <uint64_t> (), std::bad_cast);
    ASSERT_THROW (value.get <double> (), std::bad_cast);
    ASSERT_THROW (value.get <std::string> (), std::bad_cast);
    ASSERT_THROW (value.get <Array> (), std::bad_cast);
    ASSERT_THROW (value.get <Object> (), std::bad_cast);

    value.set <int64_t> (std::numeric_limits <int64_t>::min ());
    ASSERT_THROW (value.get <std::nullptr_t> (), std::bad_cast);
    ASSERT_THROW (value.get <bool> (), std::bad_cast);
    ASSERT_THROW (value.get <int32_t> (), std::bad_cast);
    ASSERT_THROW (value.get <uint32_t> (), std::bad_cast);
    ASSERT_NO_THROW (value.get <int64_t> ());
    ASSERT_THROW (value.get <uint64_t> (), std::bad_cast);
    ASSERT_THROW (value.get <double> (), std::bad_cast);
    ASSERT_THROW (value.get <std::string> (), std::bad_cast);
    ASSERT_THROW (value.get <Array> (), std::bad_cast);
    ASSERT_THROW (value.get <Object> (), std::bad_cast);

    value.set <uint64_t> (std::numeric_limits <uint64_t>::max ());
    ASSERT_THROW (value.get <std::nullptr_t> (), std::bad_cast);
    ASSERT_THROW (value.get <bool> (), std::bad_cast);
    ASSERT_THROW (value.get <int> (), std::bad_cast);
    ASSERT_THROW (value.get <unsigned int> (), std::bad_cast);
    ASSERT_THROW (value.get <int64_t> (), std::bad_cast);
    ASSERT_NO_THROW (value.get <uint64_t> ());
    ASSERT_THROW (value.get <double> (), std::bad_cast);
    ASSERT_THROW (value.get <std::string> (), std::bad_cast);
    ASSERT_THROW (value.get <std::nullptr_t> (), std::bad_cast);
    ASSERT_THROW (value.get <Array> (), std::bad_cast);
    ASSERT_THROW (value.get <Object> (), std::bad_cast);

    value.set <double> (std::numeric_limits <double>::min ());
    ASSERT_THROW (value.get <std::nullptr_t> (), std::bad_cast);
    ASSERT_THROW (value.get <bool> (), std::bad_cast);
    ASSERT_THROW (value.get <int32_t> (), std::bad_cast);
    ASSERT_THROW (value.get <uint32_t> (), std::bad_cast);
    ASSERT_THROW (value.get <int64_t> (), std::bad_cast);
    ASSERT_THROW (value.get <uint64_t> (), std::bad_cast);
    ASSERT_NO_THROW (value.get <double> ());
    ASSERT_THROW (value.get <std::string> (), std::bad_cast);
    ASSERT_THROW (value.get <Array> (), std::bad_cast);
    ASSERT_THROW (value.get <Object> (), std::bad_cast);

    value.set <std::string> ("foobar");
    ASSERT_THROW (value.get <std::nullptr_t> (), std::bad_cast);
    ASSERT_THROW (value.get <bool> (), std::bad_cast);
    ASSERT_THROW (value.get <int32_t> (), std::bad_cast);
    ASSERT_THROW (value.get <uint32_t> (), std::bad_cast);
    ASSERT_THROW (value.get <int64_t> (), std::bad_cast);
    ASSERT_THROW (value.get <uint64_t> (), std::bad_cast);
    ASSERT_THROW (value.get <double> (), std::bad_cast);
    ASSERT_NO_THROW (value.get <std::string> ());
    ASSERT_THROW (value.get <Array> (), std::bad_cast);
    ASSERT_THROW (value.get <Object> (), std::bad_cast);

    value.set <Array, Array::value_type> ({1});
    ASSERT_THROW (value.get <std::nullptr_t> (), std::bad_cast);
    ASSERT_THROW (value.get <bool> (), std::bad_cast);
    ASSERT_THROW (value.get <int32_t> (), std::bad_cast);
    ASSERT_THROW (value.get <uint32_t> (), std::bad_cast);
    ASSERT_THROW (value.get <int64_t> (), std::bad_cast);
    ASSERT_THROW (value.get <uint64_t> (), std::bad_cast);
    ASSERT_THROW (value.get <double> (), std::bad_cast);
    ASSERT_THROW (value.get <std::string> (), std::bad_cast);
    ASSERT_NO_THROW (value.get <Array> ());
    ASSERT_THROW (value.get <Object> (), std::bad_cast);

    value.set <Object, Object::value_type> ({{"i", 1}});
    ASSERT_THROW (value.get <std::nullptr_t> (), std::bad_cast);
    ASSERT_THROW (value.get <bool> (), std::bad_cast);
    ASSERT_THROW (value.get <int32_t> (), std::bad_cast);
    ASSERT_THROW (value.get <uint32_t> (), std::bad_cast);
    ASSERT_THROW (value.get <int64_t> (), std::bad_cast);
    ASSERT_THROW (value.get <uint64_t> (), std::bad_cast);
    ASSERT_THROW (value.get <double> (), std::bad_cast);
    ASSERT_THROW (value.get <std::string> (), std::bad_cast);
    ASSERT_THROW (value.get <Array> (), std::bad_cast);
    ASSERT_NO_THROW (value.get <Object> ());
}

/**
 * @brief Test getBool method.
 */
TEST (Value, getBool)
{
    Value value;
    ASSERT_FALSE (value.getBool ());
    ASSERT_FALSE (static_cast <bool> (value));

    value = nullptr;
    ASSERT_FALSE (value.getBool ());
    ASSERT_FALSE (static_cast <bool> (value));

    value = true;
    ASSERT_TRUE (value.getBool ());
    ASSERT_TRUE (static_cast <bool> (value));

    value = false;
    ASSERT_FALSE (value.getBool ());
    ASSERT_FALSE (static_cast <bool> (value));

    value = std::numeric_limits <int8_t>::min ();
    ASSERT_TRUE (value.getBool ());
    ASSERT_TRUE (static_cast <bool> (value));

    value = std::numeric_limits <int8_t>::max ();
    ASSERT_TRUE (value.getBool ());
    ASSERT_TRUE (static_cast <bool> (value));

    value = std::numeric_limits <uint8_t>::min ();
    ASSERT_FALSE (value.getBool ());
    ASSERT_FALSE (static_cast <bool> (value));

    value = std::numeric_limits <uint8_t>::max ();
    ASSERT_TRUE (value.getBool ());
    ASSERT_TRUE (static_cast <bool> (value));

    value = std::numeric_limits <int16_t>::min ();
    ASSERT_TRUE (value.getBool ());
    ASSERT_TRUE (static_cast <bool> (value));

    value = std::numeric_limits <int16_t>::max ();
    ASSERT_TRUE (value.getBool ());
    ASSERT_TRUE (static_cast <bool> (value));

    value = std::numeric_limits <uint16_t>::min ();
    ASSERT_FALSE (value.getBool ());
    ASSERT_FALSE (static_cast <bool> (value));

    value = std::numeric_limits <uint16_t>::max ();
    ASSERT_TRUE (value.getBool ());
    ASSERT_TRUE (static_cast <bool> (value));

    value = std::numeric_limits <int32_t>::min ();
    ASSERT_TRUE (value.getBool ());
    ASSERT_TRUE (static_cast <bool> (value));

    value = std::numeric_limits <int32_t>::max ();
    ASSERT_TRUE (value.getBool ());
    ASSERT_TRUE (static_cast <bool> (value));

    value = std::numeric_limits <uint32_t>::min ();
    ASSERT_FALSE (value.getBool ());
    ASSERT_FALSE (static_cast <bool> (value));

    value = std::numeric_limits <uint32_t>::max ();
    ASSERT_TRUE (value.getBool ());
    ASSERT_TRUE (static_cast <bool> (value));

    value = std::numeric_limits <int64_t>::min ();
    ASSERT_TRUE (value.getBool ());
    ASSERT_TRUE (static_cast <bool> (value));

    value = std::numeric_limits <int64_t>::max ();
    ASSERT_TRUE (value.getBool ());
    ASSERT_TRUE (static_cast <bool> (value));

    value = std::numeric_limits <uint64_t>::min ();
    ASSERT_FALSE (value.getBool ());
    ASSERT_FALSE (static_cast <bool> (value));

    value = std::numeric_limits <uint64_t>::max ();
    ASSERT_TRUE (value.getBool ());
    ASSERT_TRUE (static_cast <bool> (value));

    value = std::numeric_limits <float>::min ();
    ASSERT_TRUE (value.getBool ());
    ASSERT_TRUE (static_cast <bool> (value));

    value = std::numeric_limits <float>::max ();
    ASSERT_TRUE (value.getBool ());
    ASSERT_TRUE (static_cast <bool> (value));

    value = std::numeric_limits <double>::min ();
    ASSERT_TRUE (value.getBool ());
    ASSERT_TRUE (static_cast <bool> (value));

    value = std::numeric_limits <double>::max ();
    ASSERT_TRUE (value.getBool ());
    ASSERT_TRUE (static_cast <bool> (value));

    value = "";
    ASSERT_THROW (value.getBool (), std::bad_cast);
    ASSERT_THROW (static_cast <bool> (value), std::bad_cast);

    value = "foo";
    ASSERT_THROW (value.getBool (), std::bad_cast);
    ASSERT_THROW (static_cast <bool> (value), std::bad_cast);

    value = "02:42:64:2f:6a:d0";
    ASSERT_THROW (value.getBool (), std::bad_cast);
    ASSERT_THROW (static_cast <bool> (value), std::bad_cast);

    value = "127.0.0.1";
    ASSERT_THROW (value.getBool (), std::bad_cast);
    ASSERT_THROW (static_cast <bool> (value), std::bad_cast);

    value = Array ({1});
    ASSERT_THROW (value.getBool (), std::bad_cast);
    ASSERT_THROW (static_cast <bool> (value), std::bad_cast);

    value = Object ({{"i", 1}});
    ASSERT_THROW (value.getBool (), std::bad_cast);
    ASSERT_THROW (static_cast <bool> (value), std::bad_cast);
}

/**
 * @brief Test getInt8 method.
 */
TEST (Value, getInt8)
{
    Value value;
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW (static_cast <int8_t> (value), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW (static_cast <int8_t> (value), std::bad_cast);

    value = true;
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW (static_cast <int8_t> (value), std::bad_cast);

    value = false;
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW (static_cast <int8_t> (value), std::bad_cast);

    value = std::numeric_limits <int8_t>::min ();
    ASSERT_EQ (value.getInt8 (), std::numeric_limits <int8_t>::min ());
    ASSERT_EQ (static_cast <int8_t> (value), std::numeric_limits <int8_t>::min ());

    value = std::numeric_limits <int8_t>::max ();
    ASSERT_EQ (value.getInt8 (), std::numeric_limits <int8_t>::max ());
    ASSERT_EQ (static_cast <int8_t> (value), std::numeric_limits <int8_t>::max ());

    value = std::numeric_limits <uint8_t>::min ();
    ASSERT_EQ (value.getInt8 (), std::numeric_limits <uint8_t>::min ());
    ASSERT_EQ (static_cast <int8_t> (value), std::numeric_limits <uint8_t>::min ());

    value = std::numeric_limits <uint8_t>::max ();
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW (static_cast <int8_t> (value), std::bad_cast);

    value = std::numeric_limits <int16_t>::min ();
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW (static_cast <int8_t> (value), std::bad_cast);

    value = std::numeric_limits <int16_t>::max ();
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW (static_cast <int8_t> (value), std::bad_cast);

    value = std::numeric_limits <uint16_t>::min ();
    ASSERT_EQ (value.getInt8 (), std::numeric_limits <uint16_t>::min ());
    ASSERT_EQ (static_cast <int8_t> (value), std::numeric_limits <uint16_t>::min ());

    value = std::numeric_limits <uint16_t>::max ();
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW (static_cast <int8_t> (value), std::bad_cast);

    value = std::numeric_limits <int32_t>::min ();
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW (static_cast <int8_t> (value), std::bad_cast);

    value = int32_t (12);
    ASSERT_EQ (value.getInt8 (), 12);
    ASSERT_EQ (static_cast <int8_t> (value), 12);

    value = std::numeric_limits <int32_t>::max ();
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW (static_cast <int8_t> (value), std::bad_cast);

    value = std::numeric_limits <uint32_t>::min ();
    ASSERT_EQ (value.getInt8 (), std::numeric_limits <uint32_t>::min ());
    ASSERT_EQ (static_cast <int8_t> (value), std::numeric_limits <uint32_t>::min ());

    value = std::numeric_limits <uint32_t>::max ();
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW (static_cast <int8_t> (value), std::bad_cast);

    value = std::numeric_limits <int64_t>::min ();
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW (static_cast <int8_t> (value), std::bad_cast);

    value = int64_t (12);
    ASSERT_EQ (value.getInt8 (), 12);
    ASSERT_EQ (static_cast <int8_t> (value), 12);

    value = std::numeric_limits <int64_t>::max ();
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW (static_cast <int8_t> (value), std::bad_cast);

    value = std::numeric_limits <uint64_t>::min ();
    ASSERT_EQ (value.getInt8 (), std::numeric_limits <uint64_t>::min ());
    ASSERT_EQ (static_cast <int8_t> (value), std::numeric_limits <uint64_t>::min ());

    value = std::numeric_limits <uint64_t>::max ();
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW (static_cast <int8_t> (value), std::bad_cast);

    value = std::numeric_limits <float>::min ();
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW (static_cast <int8_t> (value), std::bad_cast);

    value = std::numeric_limits <float>::max ();
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW (static_cast <int8_t> (value), std::bad_cast);

    value = std::numeric_limits <double>::min ();
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW (static_cast <int8_t> (value), std::bad_cast);

    value = double (12.0);
    ASSERT_EQ (value.getInt8 (), 12);
    ASSERT_EQ (static_cast <int8_t> (value), 12);

    value = std::numeric_limits <double>::max ();
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW (static_cast <int8_t> (value), std::bad_cast);

    value = "";
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW (static_cast <int8_t> (value), std::bad_cast);

    value = "foo";
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW (static_cast <int8_t> (value), std::bad_cast);

    value = "02:42:64:2f:6a:d0";
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW (static_cast <int8_t> (value), std::bad_cast);

    value = "127.0.0.1";
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW (static_cast <int8_t> (value), std::bad_cast);

    value = Array ({1});
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW (static_cast <int8_t> (value), std::bad_cast);

    value = Object ({{"i", 1}});
    ASSERT_THROW (value.getInt8 (), std::bad_cast);
    ASSERT_THROW (static_cast <int8_t> (value), std::bad_cast);
}

/**
 * @brief Test getUint8 method.
 */
TEST (Value, getUint8)
{
    Value value;
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint8_t> (value), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint8_t> (value), std::bad_cast);

    value = true;
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint8_t> (value), std::bad_cast);

    value = false;
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint8_t> (value), std::bad_cast);

    value = std::numeric_limits <int8_t>::min ();
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint8_t> (value), std::bad_cast);

    value = std::numeric_limits <int8_t>::max ();
    ASSERT_EQ (value.getUint8 (), std::numeric_limits <int8_t>::max ());
    ASSERT_EQ (static_cast <uint8_t> (value), std::numeric_limits <int8_t>::max ());

    value = std::numeric_limits <uint8_t>::min ();
    ASSERT_EQ (value.getUint8 (), std::numeric_limits <uint8_t>::min ());
    ASSERT_EQ (static_cast <uint8_t> (value), std::numeric_limits <uint8_t>::min ());

    value = std::numeric_limits <uint8_t>::max ();
    ASSERT_EQ (value.getUint8 (), std::numeric_limits <uint8_t>::max ());
    ASSERT_EQ (static_cast <uint8_t> (value), std::numeric_limits <uint8_t>::max ());

    value = std::numeric_limits <int16_t>::min ();
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint8_t> (value), std::bad_cast);

    value = std::numeric_limits <int16_t>::max ();
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint8_t> (value), std::bad_cast);

    value = std::numeric_limits <uint16_t>::min ();
    ASSERT_EQ (value.getUint8 (), std::numeric_limits <uint16_t>::min ());
    ASSERT_EQ (static_cast <uint8_t> (value), std::numeric_limits <uint16_t>::min ());

    value = std::numeric_limits <uint16_t>::max ();
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint8_t> (value), std::bad_cast);

    value = std::numeric_limits <int32_t>::min ();
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint8_t> (value), std::bad_cast);

    value = int32_t (12);
    ASSERT_EQ (value.getUint8 (), 12);
    ASSERT_EQ (static_cast <uint8_t> (value), 12);

    value = std::numeric_limits <int32_t>::max ();
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint8_t> (value), std::bad_cast);

    value = std::numeric_limits <uint32_t>::min ();
    ASSERT_EQ (value.getUint8 (), std::numeric_limits <uint32_t>::min ());
    ASSERT_EQ (static_cast <uint8_t> (value), std::numeric_limits <uint32_t>::min ());

    value = std::numeric_limits <uint32_t>::max ();
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint8_t> (value), std::bad_cast);

    value = std::numeric_limits <int64_t>::min ();
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint8_t> (value), std::bad_cast);

    value = int64_t (12);
    ASSERT_EQ (value.getUint8 (), 12);
    ASSERT_EQ (static_cast <uint8_t> (value), 12);

    value = std::numeric_limits <int64_t>::max ();
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint8_t> (value), std::bad_cast);

    value = std::numeric_limits <uint64_t>::min ();
    ASSERT_EQ (value.getUint8 (), std::numeric_limits <uint64_t>::min ());
    ASSERT_EQ (static_cast <uint8_t> (value), std::numeric_limits <uint64_t>::min ());

    value = std::numeric_limits <uint64_t>::max ();
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint8_t> (value), std::bad_cast);

    value = std::numeric_limits <float>::min ();
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint8_t> (value), std::bad_cast);

    value = std::numeric_limits <float>::max ();
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint8_t> (value), std::bad_cast);

    value = std::numeric_limits <double>::min ();
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint8_t> (value), std::bad_cast);

    value = double (12.0);
    ASSERT_EQ (value.getUint8 (), 12);
    ASSERT_EQ (static_cast <uint8_t> (value), 12);

    value = std::numeric_limits <double>::max ();
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint8_t> (value), std::bad_cast);

    value = "";
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint8_t> (value), std::bad_cast);

    value = "foo";
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint8_t> (value), std::bad_cast);

    value = "02:42:64:2f:6a:d0";
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint8_t> (value), std::bad_cast);

    value = "127.0.0.1";
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint8_t> (value), std::bad_cast);

    value = Array ({1});
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint8_t> (value), std::bad_cast);

    value = Object ({{"i", 1}});
    ASSERT_THROW (value.getUint8 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint8_t> (value), std::bad_cast);
}

/**
 * @brief Test getInt16 method.
 */
TEST (Value, getInt16)
{
    Value value;
    ASSERT_THROW (value.getInt16 (), std::bad_cast);
    ASSERT_THROW (static_cast <int16_t> (value), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.getInt16 (), std::bad_cast);
    ASSERT_THROW (static_cast <int16_t> (value), std::bad_cast);

    value = true;
    ASSERT_THROW (value.getInt16 (), std::bad_cast);
    ASSERT_THROW (static_cast <int16_t> (value), std::bad_cast);

    value = false;
    ASSERT_THROW (value.getInt16 (), std::bad_cast);
    ASSERT_THROW (static_cast <int16_t> (value), std::bad_cast);

    value = std::numeric_limits <int8_t>::min ();
    ASSERT_EQ (value.getInt16 (), std::numeric_limits <int8_t>::min ());
    ASSERT_EQ (static_cast <int16_t> (value), std::numeric_limits <int8_t>::min ());

    value = std::numeric_limits <int8_t>::max ();
    ASSERT_EQ (value.getInt16 (), std::numeric_limits <int8_t>::max ());
    ASSERT_EQ (static_cast <int16_t> (value), std::numeric_limits <int8_t>::max ());

    value = std::numeric_limits <uint8_t>::min ();
    ASSERT_EQ (value.getInt16 (), std::numeric_limits <uint8_t>::min ());
    ASSERT_EQ (static_cast <int16_t> (value), std::numeric_limits <uint8_t>::min ());

    value = std::numeric_limits <uint8_t>::max ();
    ASSERT_EQ (value.getInt16 (), std::numeric_limits <uint8_t>::max ());
    ASSERT_EQ (static_cast <int16_t> (value), std::numeric_limits <uint8_t>::max ());

    value = std::numeric_limits <int16_t>::min ();
    ASSERT_EQ (value.getInt16 (), std::numeric_limits <int16_t>::min ());
    ASSERT_EQ (static_cast <int16_t> (value), std::numeric_limits <int16_t>::min ());

    value = std::numeric_limits <int16_t>::max ();
    ASSERT_EQ (value.getInt16 (), std::numeric_limits <int16_t>::max ());
    ASSERT_EQ (static_cast <int16_t> (value), std::numeric_limits <int16_t>::max ());

    value = std::numeric_limits <uint16_t>::min ();
    ASSERT_EQ (value.getInt16 (), std::numeric_limits <uint16_t>::min ());
    ASSERT_EQ (static_cast <int16_t> (value), std::numeric_limits <uint16_t>::min ());

    value = std::numeric_limits <uint16_t>::max ();
    ASSERT_THROW (value.getInt16 (), std::bad_cast);
    ASSERT_THROW (static_cast <int16_t> (value), std::bad_cast);

    value = std::numeric_limits <int32_t>::min ();
    ASSERT_THROW (value.getInt16 (), std::bad_cast);
    ASSERT_THROW (static_cast <int16_t> (value), std::bad_cast);

    value = int32_t (12);
    ASSERT_EQ (value.getInt16 (), 12);
    ASSERT_EQ (static_cast <int16_t> (value), 12);

    value = std::numeric_limits <int32_t>::max ();
    ASSERT_THROW (value.getInt16 (), std::bad_cast);
    ASSERT_THROW (static_cast <int16_t> (value), std::bad_cast);

    value = std::numeric_limits <uint32_t>::min ();
    ASSERT_EQ (value.getInt16 (), std::numeric_limits <uint32_t>::min ());
    ASSERT_EQ (static_cast <int16_t> (value), std::numeric_limits <uint32_t>::min ());

    value = std::numeric_limits <uint32_t>::max ();
    ASSERT_THROW (value.getInt16 (), std::bad_cast);
    ASSERT_THROW (static_cast <int16_t> (value), std::bad_cast);

    value = std::numeric_limits <int64_t>::min ();
    ASSERT_THROW (value.getInt16 (), std::bad_cast);
    ASSERT_THROW (static_cast <int16_t> (value), std::bad_cast);

    value = int64_t (12);
    ASSERT_EQ (value.getInt16 (), 12);
    ASSERT_EQ (static_cast <int16_t> (value), 12);

    value = std::numeric_limits <int64_t>::max ();
    ASSERT_THROW (value.getInt16 (), std::bad_cast);
    ASSERT_THROW (static_cast <int16_t> (value), std::bad_cast);

    value = std::numeric_limits <uint64_t>::min ();
    ASSERT_EQ (value.getInt16 (), std::numeric_limits <uint64_t>::min ());
    ASSERT_EQ (static_cast <int16_t> (value), std::numeric_limits <uint64_t>::min ());

    value = std::numeric_limits <uint64_t>::max ();
    ASSERT_THROW (value.getInt16 (), std::bad_cast);
    ASSERT_THROW (static_cast <int16_t> (value), std::bad_cast);

    value = std::numeric_limits <float>::min ();
    ASSERT_THROW (value.getInt16 (), std::bad_cast);
    ASSERT_THROW (static_cast <int16_t> (value), std::bad_cast);

    value = std::numeric_limits <float>::max ();
    ASSERT_THROW (value.getInt16 (), std::bad_cast);
    ASSERT_THROW (static_cast <int16_t> (value), std::bad_cast);

    value = std::numeric_limits <double>::min ();
    ASSERT_THROW (value.getInt16 (), std::bad_cast);
    ASSERT_THROW (static_cast <int16_t> (value), std::bad_cast);

    value = double (12.0);
    ASSERT_EQ (value.getInt16 (), 12);
    ASSERT_EQ (static_cast <int16_t> (value), 12);

    value = std::numeric_limits <double>::max ();
    ASSERT_THROW (value.getInt16 (), std::bad_cast);
    ASSERT_THROW (static_cast <int16_t> (value), std::bad_cast);

    value = "";
    ASSERT_THROW (value.getInt16 (), std::bad_cast);
    ASSERT_THROW (static_cast <int16_t> (value), std::bad_cast);

    value = "foo";
    ASSERT_THROW (value.getInt16 (), std::bad_cast);
    ASSERT_THROW (static_cast <int16_t> (value), std::bad_cast);

    value = "02:42:64:2f:6a:d0";
    ASSERT_THROW (value.getInt16 (), std::bad_cast);
    ASSERT_THROW (static_cast <int16_t> (value), std::bad_cast);

    value = "127.0.0.1";
    ASSERT_THROW (value.getInt16 (), std::bad_cast);
    ASSERT_THROW (static_cast <int16_t> (value), std::bad_cast);

    value = Array ({1});
    ASSERT_THROW (value.getInt16 (), std::bad_cast);
    ASSERT_THROW (static_cast <int16_t> (value), std::bad_cast);

    value = Object ({{"i", 1}});
    ASSERT_THROW (value.getInt16 (), std::bad_cast);
    ASSERT_THROW (static_cast <int16_t> (value), std::bad_cast);
}

/**
 * @brief Test getUint16 method.
 */
TEST (Value, getUint16)
{
    Value value;
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint16_t> (value), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint16_t> (value), std::bad_cast);

    value = true;
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint16_t> (value), std::bad_cast);

    value = false;
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint16_t> (value), std::bad_cast);

    value = std::numeric_limits <int8_t>::min ();
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint16_t> (value), std::bad_cast);

    value = std::numeric_limits <int8_t>::max ();
    ASSERT_EQ (value.getUint16 (), std::numeric_limits <int8_t>::max ());
    ASSERT_EQ (static_cast <uint16_t> (value), std::numeric_limits <int8_t>::max ());

    value = std::numeric_limits <uint8_t>::min ();
    ASSERT_EQ (value.getUint16 (), std::numeric_limits <uint8_t>::min ());
    ASSERT_EQ (static_cast <uint16_t> (value), std::numeric_limits <uint8_t>::min ());

    value = std::numeric_limits <uint8_t>::max ();
    ASSERT_EQ (value.getUint16 (), std::numeric_limits <uint8_t>::max ());
    ASSERT_EQ (static_cast <uint16_t> (value), std::numeric_limits <uint8_t>::max ());

    value = std::numeric_limits <int16_t>::min ();
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint16_t> (value), std::bad_cast);

    value = std::numeric_limits <int16_t>::max ();
    ASSERT_EQ (value.getUint16 (), std::numeric_limits <int16_t>::max ());
    ASSERT_EQ (static_cast <uint16_t> (value), std::numeric_limits <int16_t>::max ());

    value = std::numeric_limits <uint16_t>::min ();
    ASSERT_EQ (value.getUint16 (), std::numeric_limits <uint16_t>::min ());
    ASSERT_EQ (static_cast <uint16_t> (value), std::numeric_limits <uint16_t>::min ());

    value = std::numeric_limits <uint16_t>::max ();
    ASSERT_EQ (value.getUint16 (), std::numeric_limits <uint16_t>::max ());
    ASSERT_EQ (static_cast <uint16_t> (value), std::numeric_limits <uint16_t>::max ());

    value = std::numeric_limits <int32_t>::min ();
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint16_t> (value), std::bad_cast);

    value = int32_t (12);
    ASSERT_EQ (value.getUint16 (), 12);
    ASSERT_EQ (static_cast <uint16_t> (value), 12);

    value = std::numeric_limits <int32_t>::max ();
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint16_t> (value), std::bad_cast);

    value = std::numeric_limits <uint32_t>::min ();
    ASSERT_EQ (value.getUint16 (), std::numeric_limits <uint32_t>::min ());
    ASSERT_EQ (static_cast <uint16_t> (value), std::numeric_limits <uint32_t>::min ());

    value = std::numeric_limits <uint32_t>::max ();
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint16_t> (value), std::bad_cast);

    value = std::numeric_limits <int64_t>::min ();
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint16_t> (value), std::bad_cast);

    value = int64_t (12);
    ASSERT_EQ (value.getUint16 (), 12);
    ASSERT_EQ (static_cast <uint16_t> (value), 12);

    value = std::numeric_limits <int64_t>::max ();
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint16_t> (value), std::bad_cast);

    value = std::numeric_limits <uint64_t>::min ();
    ASSERT_EQ (value.getUint16 (), std::numeric_limits <uint64_t>::min ());
    ASSERT_EQ (static_cast <uint16_t> (value), std::numeric_limits <uint64_t>::min ());

    value = std::numeric_limits <uint64_t>::max ();
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint16_t> (value), std::bad_cast);

    value = std::numeric_limits <float>::min ();
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint16_t> (value), std::bad_cast);

    value = std::numeric_limits <float>::max ();
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint16_t> (value), std::bad_cast);

    value = std::numeric_limits <double>::min ();
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint16_t> (value), std::bad_cast);

    value = double (12.0);
    ASSERT_EQ (value.getUint16 (), 12);
    ASSERT_EQ (static_cast <uint16_t> (value), 12);

    value = std::numeric_limits <double>::max ();
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint16_t> (value), std::bad_cast);

    value = "";
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint16_t> (value), std::bad_cast);

    value = "foo";
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint16_t> (value), std::bad_cast);

    value = "02:42:64:2f:6a:d0";
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint16_t> (value), std::bad_cast);

    value = "127.0.0.1";
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint16_t> (value), std::bad_cast);

    value = Array ({1});
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint16_t> (value), std::bad_cast);

    value = Object ({{"i", 1}});
    ASSERT_THROW (value.getUint16 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint16_t> (value), std::bad_cast);
}

/**
 * @brief Test getInt method.
 */
TEST (Value, getInt)
{
    Value value;
    ASSERT_THROW (value.getInt (), std::bad_cast);
    ASSERT_THROW (static_cast <int32_t> (value), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.getInt (), std::bad_cast);
    ASSERT_THROW (static_cast <int32_t> (value), std::bad_cast);

    value = true;
    ASSERT_THROW (value.getInt (), std::bad_cast);
    ASSERT_THROW (static_cast <int32_t> (value), std::bad_cast);

    value = false;
    ASSERT_THROW (value.getInt (), std::bad_cast);
    ASSERT_THROW (static_cast <int32_t> (value), std::bad_cast);

    value = std::numeric_limits <int8_t>::min ();
    ASSERT_EQ (value.getInt (), std::numeric_limits <int8_t>::min ());
    ASSERT_EQ (static_cast <int32_t> (value), std::numeric_limits <int8_t>::min ());

    value = std::numeric_limits <int8_t>::max ();
    ASSERT_EQ (value.getInt (), std::numeric_limits <int8_t>::max ());
    ASSERT_EQ (static_cast <int32_t> (value), std::numeric_limits <int8_t>::max ());

    value = std::numeric_limits <uint8_t>::min ();
    ASSERT_EQ (value.getInt (), std::numeric_limits <uint8_t>::min ());
    ASSERT_EQ (static_cast <int32_t> (value), std::numeric_limits <uint8_t>::min ());

    value = std::numeric_limits <uint8_t>::max ();
    ASSERT_EQ (value.getInt (), std::numeric_limits <uint8_t>::max ());
    ASSERT_EQ (static_cast <int32_t> (value), std::numeric_limits <uint8_t>::max ());

    value = std::numeric_limits <int16_t>::min ();
    ASSERT_EQ (value.getInt (), std::numeric_limits <int16_t>::min ());
    ASSERT_EQ (static_cast <int32_t> (value), std::numeric_limits <int16_t>::min ());

    value = std::numeric_limits <int16_t>::max ();
    ASSERT_EQ (value.getInt (), std::numeric_limits <int16_t>::max ());
    ASSERT_EQ (static_cast <int32_t> (value), std::numeric_limits <int16_t>::max ());

    value = std::numeric_limits <uint16_t>::min ();
    ASSERT_EQ (value.getInt (), std::numeric_limits <uint16_t>::min ());
    ASSERT_EQ (static_cast <int32_t> (value), std::numeric_limits <uint16_t>::min ());

    value = std::numeric_limits <uint16_t>::max ();
    ASSERT_EQ (value.getInt (), std::numeric_limits <uint16_t>::max ());
    ASSERT_EQ (static_cast <int32_t> (value), std::numeric_limits <uint16_t>::max ());

    value = std::numeric_limits <int32_t>::min ();
    ASSERT_EQ (value.getInt (), std::numeric_limits <int32_t>::min ());
    ASSERT_EQ (static_cast <int32_t> (value), std::numeric_limits <int32_t>::min ());

    value = std::numeric_limits <int32_t>::max ();
    ASSERT_EQ (value.getInt (), std::numeric_limits <int32_t>::max ());
    ASSERT_EQ (static_cast <int32_t> (value), std::numeric_limits <int32_t>::max ());

    value = std::numeric_limits <uint32_t>::min ();
    ASSERT_EQ (value.getInt (), std::numeric_limits <uint32_t>::min ());
    ASSERT_EQ (static_cast <int32_t> (value), std::numeric_limits <uint32_t>::min ());

    value = std::numeric_limits <uint32_t>::max ();
    ASSERT_THROW (value.getInt (), std::bad_cast);
    ASSERT_THROW (static_cast <int32_t> (value), std::bad_cast);

    value = std::numeric_limits <int64_t>::min ();
    ASSERT_THROW (value.getInt (), std::bad_cast);
    ASSERT_THROW (static_cast <int32_t> (value), std::bad_cast);

    value = int64_t (12);
    ASSERT_EQ (value.getInt (), 12);
    ASSERT_EQ (static_cast <int32_t> (value), 12);

    value = std::numeric_limits <int64_t>::max ();
    ASSERT_THROW (value.getInt (), std::bad_cast);
    ASSERT_THROW (static_cast <int32_t> (value), std::bad_cast);

    value = std::numeric_limits <uint64_t>::min ();
    ASSERT_EQ (value.getInt (), std::numeric_limits <uint64_t>::min ());
    ASSERT_EQ (static_cast <int32_t> (value), std::numeric_limits <uint64_t>::min ());

    value = std::numeric_limits <uint64_t>::max ();
    ASSERT_THROW (value.getInt (), std::bad_cast);
    ASSERT_THROW (static_cast <int32_t> (value), std::bad_cast);

    value = std::numeric_limits <float>::min ();
    ASSERT_THROW (value.getInt (), std::bad_cast);
    ASSERT_THROW (static_cast <int32_t> (value), std::bad_cast);

    value = std::numeric_limits <float>::max ();
    ASSERT_THROW (value.getInt (), std::bad_cast);
    ASSERT_THROW (static_cast <int32_t> (value), std::bad_cast);

    value = std::numeric_limits <double>::min ();
    ASSERT_THROW (value.getInt (), std::bad_cast);
    ASSERT_THROW (static_cast <int32_t> (value), std::bad_cast);

    value = double (12.0);
    ASSERT_EQ (value.getInt (), 12);
    ASSERT_EQ (static_cast <int32_t> (value), 12);

    value = std::numeric_limits <double>::max ();
    ASSERT_THROW (value.getInt (), std::bad_cast);
    ASSERT_THROW (static_cast <int32_t> (value), std::bad_cast);

    value = "";
    ASSERT_THROW (value.getInt (), std::bad_cast);
    ASSERT_THROW (static_cast <int32_t> (value), std::bad_cast);

    value = "foo";
    ASSERT_THROW (value.getInt (), std::bad_cast);
    ASSERT_THROW (static_cast <int32_t> (value), std::bad_cast);

    value = "02:42:64:2f:6a:d0";
    ASSERT_THROW (value.getInt (), std::bad_cast);
    ASSERT_THROW (static_cast <int32_t> (value), std::bad_cast);

    value = "127.0.0.1";
    ASSERT_THROW (value.getInt (), std::bad_cast);
    ASSERT_THROW (static_cast <int32_t> (value), std::bad_cast);

    value = Array ({1});
    ASSERT_THROW (value.getInt (), std::bad_cast);
    ASSERT_THROW (static_cast <int32_t> (value), std::bad_cast);

    value = Object ({{"i", 1}});
    ASSERT_THROW (value.getInt (), std::bad_cast);
    ASSERT_THROW (static_cast <int32_t> (value), std::bad_cast);
}

/**
 * @brief Test getUint method.
 */
TEST (Value, getUint)
{
    Value value;
    ASSERT_THROW (value.getUint (), std::bad_cast);
    ASSERT_THROW (static_cast <uint32_t> (value), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.getUint (), std::bad_cast);
    ASSERT_THROW (static_cast <uint32_t> (value), std::bad_cast);

    value = true;
    ASSERT_THROW (value.getUint (), std::bad_cast);
    ASSERT_THROW (static_cast <uint32_t> (value), std::bad_cast);

    value = false;
    ASSERT_THROW (value.getUint (), std::bad_cast);
    ASSERT_THROW (static_cast <uint32_t> (value), std::bad_cast);

    value = std::numeric_limits <int8_t>::min ();
    ASSERT_THROW (value.getUint (), std::bad_cast);
    ASSERT_THROW (static_cast <uint32_t> (value), std::bad_cast);

    value = std::numeric_limits <int8_t>::max ();
    ASSERT_EQ (value.getUint (), std::numeric_limits <int8_t>::max ());
    ASSERT_EQ (static_cast <uint32_t> (value), std::numeric_limits <int8_t>::max ());

    value = std::numeric_limits <uint8_t>::min ();
    ASSERT_EQ (value.getUint (), std::numeric_limits <uint8_t>::min ());
    ASSERT_EQ (static_cast <uint32_t> (value), std::numeric_limits <uint8_t>::min ());

    value = std::numeric_limits <uint8_t>::max ();
    ASSERT_EQ (value.getUint (), std::numeric_limits <uint8_t>::max ());
    ASSERT_EQ (static_cast <uint32_t> (value), std::numeric_limits <uint8_t>::max ());

    value = std::numeric_limits <int16_t>::min ();
    ASSERT_THROW (value.getUint (), std::bad_cast);
    ASSERT_THROW (static_cast <uint32_t> (value), std::bad_cast);

    value = std::numeric_limits <int16_t>::max ();
    ASSERT_EQ (value.getUint (), std::numeric_limits <int16_t>::max ());
    ASSERT_EQ (static_cast <uint32_t> (value), std::numeric_limits <int16_t>::max ());

    value = std::numeric_limits <uint16_t>::min ();
    ASSERT_EQ (value.getUint (), std::numeric_limits <uint16_t>::min ());
    ASSERT_EQ (static_cast <uint32_t> (value), std::numeric_limits <uint16_t>::min ());

    value = std::numeric_limits <uint16_t>::max ();
    ASSERT_EQ (value.getUint (), std::numeric_limits <uint16_t>::max ());
    ASSERT_EQ (static_cast <uint32_t> (value), std::numeric_limits <uint16_t>::max ());

    value = std::numeric_limits <int32_t>::min ();
    ASSERT_THROW (value.getUint (), std::bad_cast);
    ASSERT_THROW (static_cast <uint32_t> (value), std::bad_cast);

    value = std::numeric_limits <int32_t>::max ();
    ASSERT_EQ (value.getUint (), std::numeric_limits <int32_t>::max ());
    ASSERT_EQ (static_cast <uint32_t> (value), std::numeric_limits <int32_t>::max ());

    value = std::numeric_limits <uint32_t>::min ();
    ASSERT_EQ (value.getUint (), std::numeric_limits <uint32_t>::min ());
    ASSERT_EQ (static_cast <uint32_t> (value), std::numeric_limits <uint32_t>::min ());

    value = std::numeric_limits <uint32_t>::max ();
    ASSERT_EQ (value.getUint (), std::numeric_limits <uint32_t>::max ());
    ASSERT_EQ (static_cast <uint32_t> (value), std::numeric_limits <uint32_t>::max ());

    value = std::numeric_limits <int64_t>::min ();
    ASSERT_THROW (value.getUint (), std::bad_cast);
    ASSERT_THROW (static_cast <uint32_t> (value), std::bad_cast);

    value = int64_t (12);
    ASSERT_EQ (value.getUint (), 12);
    ASSERT_EQ (static_cast <uint32_t> (value), 12);

    value = std::numeric_limits <int64_t>::max ();
    ASSERT_THROW (value.getUint (), std::bad_cast);
    ASSERT_THROW (static_cast <uint32_t> (value), std::bad_cast);

    value = std::numeric_limits <uint64_t>::min ();
    ASSERT_EQ (value.getUint (), std::numeric_limits <uint64_t>::min ());
    ASSERT_EQ (static_cast <uint32_t> (value), std::numeric_limits <uint64_t>::min ());

    value = std::numeric_limits <uint64_t>::max ();
    ASSERT_THROW (value.getUint (), std::bad_cast);
    ASSERT_THROW (static_cast <uint32_t> (value), std::bad_cast);

    value = std::numeric_limits <float>::min ();
    ASSERT_THROW (value.getUint (), std::bad_cast);
    ASSERT_THROW (static_cast <uint32_t> (value), std::bad_cast);

    value = std::numeric_limits <float>::max ();
    ASSERT_THROW (value.getUint (), std::bad_cast);
    ASSERT_THROW (static_cast <uint32_t> (value), std::bad_cast);

    value = std::numeric_limits <double>::min ();
    ASSERT_THROW (value.getUint (), std::bad_cast);
    ASSERT_THROW (static_cast <uint32_t> (value), std::bad_cast);

    value = double (12.0);
    ASSERT_EQ (value.getUint (), 12);
    ASSERT_EQ (static_cast <uint32_t> (value), 12);

    value = std::numeric_limits <double>::max ();
    ASSERT_THROW (value.getUint (), std::bad_cast);
    ASSERT_THROW (static_cast <uint32_t> (value), std::bad_cast);

    value = "";
    ASSERT_THROW (value.getUint (), std::bad_cast);
    ASSERT_THROW (static_cast <uint32_t> (value), std::bad_cast);

    value = "foo";
    ASSERT_THROW (value.getUint (), std::bad_cast);
    ASSERT_THROW (static_cast <uint32_t> (value), std::bad_cast);

    value = "02:42:64:2f:6a:d0";
    ASSERT_THROW (value.getUint (), std::bad_cast);
    ASSERT_THROW (static_cast <uint32_t> (value), std::bad_cast);

    value = "127.0.0.1";
    ASSERT_THROW (value.getUint (), std::bad_cast);
    ASSERT_THROW (static_cast <uint32_t> (value), std::bad_cast);

    value = Array ({1});
    ASSERT_THROW (value.getUint (), std::bad_cast);
    ASSERT_THROW (static_cast <uint32_t> (value), std::bad_cast);

    value = Object ({{"i", 1}});
    ASSERT_THROW (value.getUint (), std::bad_cast);
    ASSERT_THROW (static_cast <uint32_t> (value), std::bad_cast);
}

/**
 * @brief Test getInt64 method.
 */
TEST (Value, getInt64)
{
    Value value;
    ASSERT_THROW (value.getInt64 (), std::bad_cast);
    ASSERT_THROW (static_cast <int64_t> (value), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.getInt64 (), std::bad_cast);
    ASSERT_THROW (static_cast <int64_t> (value), std::bad_cast);

    value = true;
    ASSERT_THROW (value.getInt64 (), std::bad_cast);
    ASSERT_THROW (static_cast <int64_t> (value), std::bad_cast);

    value = false;
    ASSERT_THROW (value.getInt64 (), std::bad_cast);
    ASSERT_THROW (static_cast <int64_t> (value), std::bad_cast);

    value = std::numeric_limits <int8_t>::min ();
    ASSERT_EQ (value.getInt64 (), std::numeric_limits <int8_t>::min ());
    ASSERT_EQ (static_cast <int64_t> (value), std::numeric_limits <int8_t>::min ());

    value = std::numeric_limits <int8_t>::max ();
    ASSERT_EQ (value.getInt64 (), std::numeric_limits <int8_t>::max ());
    ASSERT_EQ (static_cast <int64_t> (value), std::numeric_limits <int8_t>::max ());

    value = std::numeric_limits <uint8_t>::min ();
    ASSERT_EQ (value.getInt64 (), std::numeric_limits <uint8_t>::min ());
    ASSERT_EQ (static_cast <int64_t> (value), std::numeric_limits <uint8_t>::min ());

    value = std::numeric_limits <uint8_t>::max ();
    ASSERT_EQ (value.getInt64 (), std::numeric_limits <uint8_t>::max ());
    ASSERT_EQ (static_cast <int64_t> (value), std::numeric_limits <uint8_t>::max ());

    value = std::numeric_limits <int16_t>::min ();
    ASSERT_EQ (value.getInt64 (), std::numeric_limits <int16_t>::min ());
    ASSERT_EQ (static_cast <int64_t> (value), std::numeric_limits <int16_t>::min ());

    value = std::numeric_limits <int16_t>::max ();
    ASSERT_EQ (value.getInt64 (), std::numeric_limits <int16_t>::max ());
    ASSERT_EQ (static_cast <int64_t> (value), std::numeric_limits <int16_t>::max ());

    value = std::numeric_limits <uint16_t>::min ();
    ASSERT_EQ (value.getInt64 (), std::numeric_limits <uint16_t>::min ());
    ASSERT_EQ (static_cast <int64_t> (value), std::numeric_limits <uint16_t>::min ());

    value = std::numeric_limits <uint16_t>::max ();
    ASSERT_EQ (value.getInt64 (), std::numeric_limits <uint16_t>::max ());
    ASSERT_EQ (static_cast <int64_t> (value), std::numeric_limits <uint16_t>::max ());

    value = std::numeric_limits <int32_t>::min ();
    ASSERT_EQ (value.getInt64 (), std::numeric_limits <int32_t>::min ());
    ASSERT_EQ (static_cast <int64_t> (value), std::numeric_limits <int32_t>::min ());

    value = std::numeric_limits <int32_t>::max ();
    ASSERT_EQ (value.getInt64 (), std::numeric_limits <int32_t>::max ());
    ASSERT_EQ (static_cast <int64_t> (value), std::numeric_limits <int32_t>::max ());

    value = std::numeric_limits <uint32_t>::min ();
    ASSERT_EQ (value.getInt64 (), std::numeric_limits <uint32_t>::min ());
    ASSERT_EQ (static_cast <int64_t> (value), std::numeric_limits <uint32_t>::min ());

    value = std::numeric_limits <uint32_t>::max ();
    ASSERT_EQ (value.getInt64 (), std::numeric_limits <uint32_t>::max ());
    ASSERT_EQ (static_cast <int64_t> (value), std::numeric_limits <uint32_t>::max ());

    value = std::numeric_limits <int64_t>::min ();
    ASSERT_EQ (value.getInt64 (), std::numeric_limits <int64_t>::min ());
    ASSERT_EQ (static_cast <int64_t> (value), std::numeric_limits <int64_t>::min ());

    value = std::numeric_limits <int64_t>::max ();
    ASSERT_EQ (value.getInt64 (), std::numeric_limits <int64_t>::max ());
    ASSERT_EQ (static_cast <int64_t> (value), std::numeric_limits <int64_t>::max ());

    value = std::numeric_limits <uint64_t>::min ();
    ASSERT_EQ (value.getInt64 (), std::numeric_limits <uint64_t>::min ());
    ASSERT_EQ (static_cast <int64_t> (value), std::numeric_limits <uint64_t>::min ());

    value = std::numeric_limits <uint64_t>::max ();
    ASSERT_THROW (value.getInt64 (), std::bad_cast);
    ASSERT_THROW (static_cast <int64_t> (value), std::bad_cast);

    value = std::numeric_limits <float>::min ();
    ASSERT_THROW (value.getInt64 (), std::bad_cast);
    ASSERT_THROW (static_cast <int64_t> (value), std::bad_cast);

    value = std::numeric_limits <float>::max ();
    ASSERT_THROW (value.getInt64 (), std::bad_cast);
    ASSERT_THROW (static_cast <int64_t> (value), std::bad_cast);

    value = std::numeric_limits <double>::min ();
    ASSERT_THROW (value.getInt64 (), std::bad_cast);
    ASSERT_THROW (static_cast <int64_t> (value), std::bad_cast);

    value = double (12.0);
    ASSERT_EQ (value.getInt64 (), 12);
    ASSERT_EQ (static_cast <int64_t> (value), 12);

    value = std::numeric_limits <double>::max ();
    ASSERT_THROW (value.getInt64 (), std::bad_cast);
    ASSERT_THROW (static_cast <int64_t> (value), std::bad_cast);

    value = "";
    ASSERT_THROW (value.getInt64 (), std::bad_cast);
    ASSERT_THROW (static_cast <int64_t> (value), std::bad_cast);

    value = "foo";
    ASSERT_THROW (value.getInt64 (), std::bad_cast);
    ASSERT_THROW (static_cast <int64_t> (value), std::bad_cast);

    value = "02:42:64:2f:6a:d0";
    ASSERT_THROW (value.getInt64 (), std::bad_cast);
    ASSERT_THROW (static_cast <int64_t> (value), std::bad_cast);

    value = "127.0.0.1";
    ASSERT_THROW (value.getInt64 (), std::bad_cast);
    ASSERT_THROW (static_cast <int64_t> (value), std::bad_cast);

    value = Array ({1});
    ASSERT_THROW (value.getInt64 (), std::bad_cast);
    ASSERT_THROW (static_cast <int64_t> (value), std::bad_cast);

    value = Object ({{"i", 1}});
    ASSERT_THROW (value.getInt64 (), std::bad_cast);
    ASSERT_THROW (static_cast <int64_t> (value), std::bad_cast);
}

/**
 * @brief Test getUint64 method.
 */
TEST (Value, getUint64)
{
    Value value;
    ASSERT_THROW (value.getUint64 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint64_t> (value), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.getUint64 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint64_t> (value), std::bad_cast);

    value = true;
    ASSERT_THROW (value.getUint64 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint64_t> (value), std::bad_cast);

    value = false;
    ASSERT_THROW (value.getUint64 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint64_t> (value), std::bad_cast);

    value = std::numeric_limits <int8_t>::min ();
    ASSERT_THROW (value.getUint64 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint64_t> (value), std::bad_cast);

    value = std::numeric_limits <int8_t>::max ();
    ASSERT_EQ (value.getUint64 (), std::numeric_limits <int8_t>::max ());
    ASSERT_EQ (static_cast <uint64_t> (value), std::numeric_limits <int8_t>::max ());

    value = std::numeric_limits <uint8_t>::min ();
    ASSERT_EQ (value.getUint64 (), std::numeric_limits <uint8_t>::min ());
    ASSERT_EQ (static_cast <uint64_t> (value), std::numeric_limits <uint8_t>::min ());

    value = std::numeric_limits <uint8_t>::max ();
    ASSERT_EQ (value.getUint64 (), std::numeric_limits <uint8_t>::max ());
    ASSERT_EQ (static_cast <uint64_t> (value), std::numeric_limits <uint8_t>::max ());

    value = std::numeric_limits <int16_t>::min ();
    ASSERT_THROW (value.getUint64 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint64_t> (value), std::bad_cast);

    value = std::numeric_limits <int16_t>::max ();
    ASSERT_EQ (value.getUint64 (), std::numeric_limits <int16_t>::max ());
    ASSERT_EQ (static_cast <uint64_t> (value), std::numeric_limits <int16_t>::max ());

    value = std::numeric_limits <uint16_t>::min ();
    ASSERT_EQ (value.getUint64 (), std::numeric_limits <uint16_t>::min ());
    ASSERT_EQ (static_cast <uint64_t> (value), std::numeric_limits <uint16_t>::min ());

    value = std::numeric_limits <uint16_t>::max ();
    ASSERT_EQ (value.getUint64 (), std::numeric_limits <uint16_t>::max ());
    ASSERT_EQ (static_cast <uint64_t> (value), std::numeric_limits <uint16_t>::max ());

    value = std::numeric_limits <int32_t>::min ();
    ASSERT_THROW (value.getUint64 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint64_t> (value), std::bad_cast);

    value = std::numeric_limits <int32_t>::max ();
    ASSERT_EQ (value.getUint64 (), std::numeric_limits <int32_t>::max ());
    ASSERT_EQ (static_cast <uint64_t> (value), std::numeric_limits <int32_t>::max ());

    value = std::numeric_limits <uint32_t>::min ();
    ASSERT_EQ (value.getUint64 (), std::numeric_limits <uint32_t>::min ());
    ASSERT_EQ (static_cast <uint64_t> (value), std::numeric_limits <uint32_t>::min ());

    value = std::numeric_limits <uint32_t>::max ();
    ASSERT_EQ (value.getUint64 (), std::numeric_limits <uint32_t>::max ());
    ASSERT_EQ (static_cast <uint64_t> (value), std::numeric_limits <uint32_t>::max ());

    value = std::numeric_limits <int64_t>::min ();
    ASSERT_THROW (value.getUint64 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint64_t> (value), std::bad_cast);

    value = std::numeric_limits <int64_t>::max ();
    ASSERT_EQ (value.getUint64 (), std::numeric_limits <int64_t>::max ());
    ASSERT_EQ (static_cast <uint64_t> (value), std::numeric_limits <int64_t>::max ());

    value = std::numeric_limits <uint64_t>::min ();
    ASSERT_EQ (value.getUint64 (), std::numeric_limits <uint64_t>::min ());
    ASSERT_EQ (static_cast <uint64_t> (value), std::numeric_limits <uint64_t>::min ());

    value = std::numeric_limits <uint64_t>::max ();
    ASSERT_EQ (value.getUint64 (), std::numeric_limits <uint64_t>::max ());
    ASSERT_EQ (static_cast <uint64_t> (value), std::numeric_limits <uint64_t>::max ());

    value = std::numeric_limits <float>::min ();
    ASSERT_THROW (value.getUint64 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint64_t> (value), std::bad_cast);

    value = std::numeric_limits <float>::max ();
    ASSERT_THROW (value.getUint64 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint64_t> (value), std::bad_cast);

    value = std::numeric_limits <double>::min ();
    ASSERT_THROW (value.getUint64 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint64_t> (value), std::bad_cast);

    value = double (12.0);
    ASSERT_EQ (value.getUint64 (), 12);
    ASSERT_EQ (static_cast <uint64_t> (value), 12);

    value = std::numeric_limits <double>::max ();
    ASSERT_THROW (value.getUint64 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint64_t> (value), std::bad_cast);

    value = "";
    ASSERT_THROW (value.getUint64 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint64_t> (value), std::bad_cast);

    value = "foo";
    ASSERT_THROW (value.getUint64 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint64_t> (value), std::bad_cast);

    value = "02:42:64:2f:6a:d0";
    ASSERT_THROW (value.getUint64 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint64_t> (value), std::bad_cast);

    value = "127.0.0.1";
    ASSERT_THROW (value.getUint64 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint64_t> (value), std::bad_cast);

    value = Array ({1});
    ASSERT_THROW (value.getUint64 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint64_t> (value), std::bad_cast);

    value = Object ({{"i", 1}});
    ASSERT_THROW (value.getUint64 (), std::bad_cast);
    ASSERT_THROW (static_cast <uint64_t> (value), std::bad_cast);
}

/**
 * @brief Test getFloat method.
 */
TEST (Value, getFloat)
{
    Value value;
    ASSERT_THROW (value.getFloat (), std::bad_cast);
    ASSERT_THROW (static_cast <float> (value), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.getFloat (), std::bad_cast);
    ASSERT_THROW (static_cast <float> (value), std::bad_cast);

    value = true;
    ASSERT_THROW (value.getFloat (), std::bad_cast);
    ASSERT_THROW (static_cast <float> (value), std::bad_cast);

    value = false;
    ASSERT_THROW (value.getFloat (), std::bad_cast);
    ASSERT_THROW (static_cast <float> (value), std::bad_cast);

    value = std::numeric_limits <int8_t>::min ();
    ASSERT_EQ (value.getFloat (), std::numeric_limits <int8_t>::min ());
    ASSERT_EQ (static_cast <float> (value), std::numeric_limits <int8_t>::min ());

    value = std::numeric_limits <int8_t>::max ();
    ASSERT_EQ (value.getFloat (), std::numeric_limits <int8_t>::max ());
    ASSERT_EQ (static_cast <float> (value), std::numeric_limits <int8_t>::max ());

    value = std::numeric_limits <uint8_t>::min ();
    ASSERT_EQ (value.getFloat (), std::numeric_limits <uint8_t>::min ());
    ASSERT_EQ (static_cast <float> (value), std::numeric_limits <uint8_t>::min ());

    value = std::numeric_limits <uint8_t>::max ();
    ASSERT_EQ (value.getFloat (), std::numeric_limits <uint8_t>::max ());
    ASSERT_EQ (static_cast <float> (value), std::numeric_limits <uint8_t>::max ());

    value = std::numeric_limits <int16_t>::min ();
    ASSERT_EQ (value.getFloat (), std::numeric_limits <int16_t>::min ());
    ASSERT_EQ (static_cast <float> (value), std::numeric_limits <int16_t>::min ());

    value = std::numeric_limits <int16_t>::max ();
    ASSERT_EQ (value.getFloat (), std::numeric_limits <int16_t>::max ());
    ASSERT_EQ (static_cast <float> (value), std::numeric_limits <int16_t>::max ());

    value = std::numeric_limits <uint16_t>::min ();
    ASSERT_EQ (value.getFloat (), std::numeric_limits <uint16_t>::min ());
    ASSERT_EQ (static_cast <float> (value), std::numeric_limits <uint16_t>::min ());

    value = std::numeric_limits <uint16_t>::max ();
    ASSERT_EQ (value.getFloat (), std::numeric_limits <uint16_t>::max ());
    ASSERT_EQ (static_cast <float> (value), std::numeric_limits <uint16_t>::max ());

    value = std::numeric_limits <int32_t>::min ();
    ASSERT_EQ (value.getFloat (), std::numeric_limits <int32_t>::min ());
    ASSERT_EQ (static_cast <float> (value), std::numeric_limits <int32_t>::min ());

    value = std::numeric_limits <int32_t>::max ();
    ASSERT_EQ (value.getFloat (), std::numeric_limits <int32_t>::max ());
    ASSERT_EQ (static_cast <float> (value), std::numeric_limits <int32_t>::max ());

    value = std::numeric_limits <uint32_t>::min ();
    ASSERT_EQ (value.getFloat (), std::numeric_limits <uint32_t>::min ());
    ASSERT_EQ (static_cast <float> (value), std::numeric_limits <uint32_t>::min ());

    value = std::numeric_limits <uint32_t>::max ();
    ASSERT_EQ (value.getFloat (), std::numeric_limits <uint32_t>::max ());
    ASSERT_EQ (static_cast <float> (value), std::numeric_limits <uint32_t>::max ());

    value = std::numeric_limits <int64_t>::min ();
    ASSERT_EQ (value.getFloat (), std::numeric_limits <int64_t>::min ());
    ASSERT_EQ (static_cast <float> (value), std::numeric_limits <int64_t>::min ());

    value = std::numeric_limits <int64_t>::max ();
    ASSERT_EQ (value.getFloat (), std::numeric_limits <int64_t>::max ());
    ASSERT_EQ (static_cast <float> (value), std::numeric_limits <int64_t>::max ());

    value = std::numeric_limits <uint64_t>::min ();
    ASSERT_EQ (value.getFloat (), std::numeric_limits <uint64_t>::min ());
    ASSERT_EQ (static_cast <float> (value), std::numeric_limits <uint64_t>::min ());

    value = std::numeric_limits <uint64_t>::max ();
    ASSERT_EQ (value.getFloat (), std::numeric_limits <uint64_t>::max ());
    ASSERT_EQ (static_cast <float> (value), std::numeric_limits <uint64_t>::max ());

    value = std::numeric_limits <float>::min ();
    ASSERT_EQ (value.getFloat (), std::numeric_limits <float>::min ());
    ASSERT_EQ (static_cast <float> (value), std::numeric_limits <float>::min ());

    value = std::numeric_limits <float>::max ();
    ASSERT_EQ (value.getFloat (), std::numeric_limits <float>::max ());
    ASSERT_EQ (static_cast <float> (value), std::numeric_limits <float>::max ());

    value = std::numeric_limits <double>::min ();
    ASSERT_EQ (value.getFloat (), static_cast <float> (std::numeric_limits <double>::min ()));
    ASSERT_EQ (static_cast <float> (value), static_cast <float> (std::numeric_limits <double>::min ()));

    value = std::numeric_limits <double>::max ();
    ASSERT_EQ (value.getFloat (), static_cast <float> (std::numeric_limits <double>::max ()));
    ASSERT_EQ (static_cast <float> (value), static_cast <float> (std::numeric_limits <double>::max ()));

    value = "";
    ASSERT_THROW (value.getFloat (), std::bad_cast);
    ASSERT_THROW (static_cast <float> (value), std::bad_cast);

    value = "foo";
    ASSERT_THROW (value.getFloat (), std::bad_cast);
    ASSERT_THROW (static_cast <float> (value), std::bad_cast);

    value = "02:42:64:2f:6a:d0";
    ASSERT_THROW (value.getFloat (), std::bad_cast);
    ASSERT_THROW (static_cast <float> (value), std::bad_cast);

    value = "127.0.0.1";
    ASSERT_THROW (value.getFloat (), std::bad_cast);
    ASSERT_THROW (static_cast <float> (value), std::bad_cast);

    value = Array ({1});
    ASSERT_THROW (value.getFloat (), std::bad_cast);
    ASSERT_THROW (static_cast <float> (value), std::bad_cast);

    value = Object ({{"i", 1}});
    ASSERT_THROW (value.getFloat (), std::bad_cast);
    ASSERT_THROW (static_cast <float> (value), std::bad_cast);
}

/**
 * @brief Test getDouble method.
 */
TEST (Value, getDouble)
{
    Value value;
    ASSERT_THROW (value.getDouble (), std::bad_cast);
    ASSERT_THROW (static_cast <double> (value), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.getDouble (), std::bad_cast);
    ASSERT_THROW (static_cast <double> (value), std::bad_cast);

    value = true;
    ASSERT_THROW (value.getDouble (), std::bad_cast);
    ASSERT_THROW (static_cast <double> (value), std::bad_cast);

    value = false;
    ASSERT_THROW (value.getDouble (), std::bad_cast);
    ASSERT_THROW (static_cast <double> (value), std::bad_cast);

    value = std::numeric_limits <int8_t>::min ();
    ASSERT_EQ (value.getDouble (), std::numeric_limits <int8_t>::min ());
    ASSERT_EQ (static_cast <double> (value), std::numeric_limits <int8_t>::min ());

    value = std::numeric_limits <int8_t>::max ();
    ASSERT_EQ (value.getDouble (), std::numeric_limits <int8_t>::max ());
    ASSERT_EQ (static_cast <double> (value), std::numeric_limits <int8_t>::max ());

    value = std::numeric_limits <uint8_t>::min ();
    ASSERT_EQ (value.getDouble (), std::numeric_limits <uint8_t>::min ());
    ASSERT_EQ (static_cast <double> (value), std::numeric_limits <uint8_t>::min ());

    value = std::numeric_limits <uint8_t>::max ();
    ASSERT_EQ (value.getDouble (), std::numeric_limits <uint8_t>::max ());
    ASSERT_EQ (static_cast <double> (value), std::numeric_limits <uint8_t>::max ());

    value = std::numeric_limits <int16_t>::min ();
    ASSERT_EQ (value.getDouble (), std::numeric_limits <int16_t>::min ());
    ASSERT_EQ (static_cast <double> (value), std::numeric_limits <int16_t>::min ());

    value = std::numeric_limits <int16_t>::max ();
    ASSERT_EQ (value.getDouble (), std::numeric_limits <int16_t>::max ());
    ASSERT_EQ (static_cast <double> (value), std::numeric_limits <int16_t>::max ());

    value = std::numeric_limits <uint16_t>::min ();
    ASSERT_EQ (value.getDouble (), std::numeric_limits <uint16_t>::min ());
    ASSERT_EQ (static_cast <double> (value), std::numeric_limits <uint16_t>::min ());

    value = std::numeric_limits <uint16_t>::max ();
    ASSERT_EQ (value.getDouble (), std::numeric_limits <uint16_t>::max ());
    ASSERT_EQ (static_cast <double> (value), std::numeric_limits <uint16_t>::max ());

    value = std::numeric_limits <int32_t>::min ();
    ASSERT_EQ (value.getDouble (), std::numeric_limits <int32_t>::min ());
    ASSERT_EQ (static_cast <double> (value), std::numeric_limits <int32_t>::min ());

    value = std::numeric_limits <int32_t>::max ();
    ASSERT_EQ (value.getDouble (), std::numeric_limits <int32_t>::max ());
    ASSERT_EQ (static_cast <double> (value), std::numeric_limits <int32_t>::max ());

    value = std::numeric_limits <uint32_t>::min ();
    ASSERT_EQ (value.getDouble (), std::numeric_limits <uint32_t>::min ());
    ASSERT_EQ (static_cast <double> (value), std::numeric_limits <uint32_t>::min ());

    value = std::numeric_limits <uint32_t>::max ();
    ASSERT_EQ (value.getDouble (), std::numeric_limits <uint32_t>::max ());
    ASSERT_EQ (static_cast <double> (value), std::numeric_limits <uint32_t>::max ());

    value = std::numeric_limits <int64_t>::min ();
    ASSERT_EQ (value.getDouble (), std::numeric_limits <int64_t>::min ());
    ASSERT_EQ (static_cast <double> (value), std::numeric_limits <int64_t>::min ());

    value = std::numeric_limits <int64_t>::max ();
    ASSERT_EQ (value.getDouble (), std::numeric_limits <int64_t>::max ());
    ASSERT_EQ (static_cast <double> (value), std::numeric_limits <int64_t>::max ());

    value = std::numeric_limits <uint64_t>::min ();
    ASSERT_EQ (value.getDouble (), std::numeric_limits <uint64_t>::min ());
    ASSERT_EQ (static_cast <double> (value), std::numeric_limits <uint64_t>::min ());

    value = std::numeric_limits <uint64_t>::max ();
    ASSERT_EQ (value.getDouble (), std::numeric_limits <uint64_t>::max ());
    ASSERT_EQ (static_cast <double> (value), std::numeric_limits <uint64_t>::max ());

    value = std::numeric_limits <float>::min ();
    ASSERT_EQ (value.getDouble (), std::numeric_limits <float>::min ());
    ASSERT_EQ (static_cast <double> (value), std::numeric_limits <float>::min ());

    value = std::numeric_limits <float>::max ();
    ASSERT_EQ (value.getDouble (), std::numeric_limits <float>::max ());
    ASSERT_EQ (static_cast <double> (value), std::numeric_limits <float>::max ());

    value = std::numeric_limits <double>::min ();
    ASSERT_EQ (value.getDouble (), std::numeric_limits <double>::min ());
    ASSERT_EQ (static_cast <double> (value), std::numeric_limits <double>::min ());

    value = std::numeric_limits <double>::max ();
    ASSERT_EQ (value.getDouble (), std::numeric_limits <double>::max ());
    ASSERT_EQ (static_cast <double> (value), std::numeric_limits <double>::max ());

    value = "";
    ASSERT_THROW (value.getDouble (), std::bad_cast);
    ASSERT_THROW (static_cast <double> (value), std::bad_cast);

    value = "foo";
    ASSERT_THROW (value.getDouble (), std::bad_cast);
    ASSERT_THROW (static_cast <double> (value), std::bad_cast);

    value = "02:42:64:2f:6a:d0";
    ASSERT_THROW (value.getDouble (), std::bad_cast);
    ASSERT_THROW (static_cast <double> (value), std::bad_cast);

    value = "127.0.0.1";
    ASSERT_THROW (value.getDouble (), std::bad_cast);
    ASSERT_THROW (static_cast <double> (value), std::bad_cast);

    value = Array ({1});
    ASSERT_THROW (value.getDouble (), std::bad_cast);
    ASSERT_THROW (static_cast <double> (value), std::bad_cast);

    value = Object ({{"i", 1}});
    ASSERT_THROW (value.getDouble (), std::bad_cast);
    ASSERT_THROW (static_cast <double> (value), std::bad_cast);
}

/**
 * @brief Test getString method.
 */
TEST (Value, getString)
{
    Value value;
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW (static_cast <const char *> (value), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW (static_cast <const char *> (value), std::bad_cast);

    value = true;
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW (static_cast <const char *> (value), std::bad_cast);

    value = false;
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW (static_cast <const char *> (value), std::bad_cast);

    value = std::numeric_limits <int8_t>::min ();
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW (static_cast <const char *> (value), std::bad_cast);

    value = std::numeric_limits <int8_t>::max ();
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW (static_cast <const char *> (value), std::bad_cast);

    value = std::numeric_limits <uint8_t>::min ();
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW (static_cast <const char *> (value), std::bad_cast);

    value = std::numeric_limits <uint8_t>::max ();
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW (static_cast <const char *> (value), std::bad_cast);

    value = std::numeric_limits <int16_t>::min ();
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW (static_cast <const char *> (value), std::bad_cast);

    value = std::numeric_limits <int16_t>::max ();
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW (static_cast <const char *> (value), std::bad_cast);

    value = std::numeric_limits <uint16_t>::min ();
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW (static_cast <const char *> (value), std::bad_cast);

    value = std::numeric_limits <uint16_t>::max ();
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW (static_cast <const char *> (value), std::bad_cast);

    value = std::numeric_limits <int32_t>::min ();
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW (static_cast <const char *> (value), std::bad_cast);

    value = std::numeric_limits <int32_t>::max ();
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW (static_cast <const char *> (value), std::bad_cast);

    value = std::numeric_limits <uint32_t>::min ();
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW (static_cast <const char *> (value), std::bad_cast);

    value = std::numeric_limits <uint32_t>::max ();
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW (static_cast <const char *> (value), std::bad_cast);

    value = std::numeric_limits <int64_t>::min ();
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW (static_cast <const char *> (value), std::bad_cast);

    value = std::numeric_limits <int64_t>::max ();
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW (static_cast <const char *> (value), std::bad_cast);

    value = std::numeric_limits <uint64_t>::min ();
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW (static_cast <const char *> (value), std::bad_cast);

    value = std::numeric_limits <uint64_t>::max ();
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW (static_cast <const char *> (value), std::bad_cast);

    value = std::numeric_limits <float>::min ();
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW (static_cast <const char *> (value), std::bad_cast);

    value = std::numeric_limits <float>::max ();
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW (static_cast <const char *> (value), std::bad_cast);

    value = std::numeric_limits <double>::min ();
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW (static_cast <const char *> (value), std::bad_cast);

    value = std::numeric_limits <double>::max ();
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW (static_cast <const char *> (value), std::bad_cast);

    value = "";
    ASSERT_EQ (value.getString (), "");
    ASSERT_STREQ (static_cast <const char *> (value), "");

    value = "foo";
    ASSERT_EQ (value.getString (), "foo");
    ASSERT_STREQ (static_cast <const char *> (value), "foo");

    value = "02:42:64:2f:6a:d0";
    ASSERT_STREQ (value.getString ().c_str (), "02:42:64:2f:6a:d0");
    ASSERT_STREQ (static_cast <const char *> (value), "02:42:64:2f:6a:d0");

    value = "127.0.0.1";
    ASSERT_STREQ (value.getString ().c_str (), "127.0.0.1");
    ASSERT_STREQ (static_cast <const char *> (value), "127.0.0.1");

    value = Array ({1});
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW (static_cast <const char *> (value), std::bad_cast);

    value = Object ({{"i", 1}});
    ASSERT_THROW (value.getString (), std::bad_cast);
    ASSERT_THROW (static_cast <const char *> (value), std::bad_cast);
}

/**
 * @brief Test getArray method.
 */
TEST (Value, getArray)
{
    Value value;
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = true;
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = false;
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = std::numeric_limits <int8_t>::min ();
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = std::numeric_limits <int8_t>::max ();
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = std::numeric_limits <uint8_t>::min ();
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = std::numeric_limits <uint8_t>::max ();
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = std::numeric_limits <int16_t>::min ();
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = std::numeric_limits <int16_t>::max ();
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = std::numeric_limits <uint16_t>::min ();
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = std::numeric_limits <uint16_t>::max ();
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = std::numeric_limits <int32_t>::min ();
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = std::numeric_limits <int32_t>::max ();
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = std::numeric_limits <uint32_t>::min ();
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = std::numeric_limits <uint32_t>::max ();
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = std::numeric_limits <int64_t>::min ();
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = std::numeric_limits <int64_t>::max ();
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = std::numeric_limits <uint64_t>::min ();
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = std::numeric_limits <uint64_t>::max ();
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = std::numeric_limits <float>::min ();
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = std::numeric_limits <float>::max ();
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = std::numeric_limits <double>::min ();
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = std::numeric_limits <double>::max ();
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = "";
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = "foo";
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = "02:42:64:2f:6a:d0";
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = "127.0.0.1";
    ASSERT_THROW (value.getArray (), std::bad_cast);

    value = Array ({1});
    ASSERT_NO_THROW (value.getArray ());

    value = Object ({{"i", 1}});
    ASSERT_THROW (value.getArray (), std::bad_cast);
}

/**
 * @brief Test getObject method.
 */
TEST (Value, getObject)
{
    Value value;
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = true;
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = false;
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = std::numeric_limits <int8_t>::min ();
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = std::numeric_limits <int8_t>::max ();
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = std::numeric_limits <uint8_t>::min ();
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = std::numeric_limits <uint8_t>::max ();
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = std::numeric_limits <int16_t>::min ();
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = std::numeric_limits <int16_t>::max ();
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = std::numeric_limits <uint16_t>::min ();
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = std::numeric_limits <uint16_t>::max ();
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = std::numeric_limits <int32_t>::min ();
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = std::numeric_limits <int32_t>::max ();
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = std::numeric_limits <uint32_t>::min ();
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = std::numeric_limits <uint32_t>::max ();
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = std::numeric_limits <int64_t>::min ();
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = std::numeric_limits <int64_t>::max ();
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = std::numeric_limits <uint64_t>::min ();
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = std::numeric_limits <uint64_t>::max ();
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = std::numeric_limits <float>::min ();
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = std::numeric_limits <float>::max ();
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = std::numeric_limits <double>::min ();
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = std::numeric_limits <double>::max ();
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = "";
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = "foo";
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = "02:42:64:2f:6a:d0";
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = "127.0.0.1";
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = Array ({1});
    ASSERT_THROW (value.getObject (), std::bad_cast);

    value = Object ({{"i", 1}});
    ASSERT_NO_THROW (value.getObject ());
}

/**
 * @brief Test index method.
 */
TEST (Value, index)
{
    Value value;

    value.set <std::nullptr_t> (nullptr);
    ASSERT_EQ (value.index (), static_cast <size_t> (Value::Index::Null));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Boolean));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Integer));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Unsigned));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Integer64));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Unsigned64));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Real));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::String));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::ArrayValue));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::ObjectValue));

    value.set <bool> (true);
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Null));
    ASSERT_EQ (value.index (), static_cast <size_t> (Value::Index::Boolean));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Integer));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Unsigned));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Integer64));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Unsigned64));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Real));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::String));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::ArrayValue));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::ObjectValue));

    value.set <int32_t> (std::numeric_limits <int32_t>::min ());
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Null));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Boolean));
    ASSERT_EQ (value.index (), static_cast <size_t> (Value::Index::Integer));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Unsigned));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Integer64));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Unsigned64));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Real));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::String));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::ArrayValue));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::ObjectValue));

    value.set <uint32_t> (std::numeric_limits <uint32_t>::max ());
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Null));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Boolean));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Integer));
    ASSERT_EQ (value.index (), static_cast <size_t> (Value::Index::Unsigned));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Integer64));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Unsigned64));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Real));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::String));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::ArrayValue));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::ObjectValue));

    value.set <int64_t> (std::numeric_limits <int64_t>::min ());
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Null));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Boolean));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Integer));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Unsigned));
    ASSERT_EQ (value.index (), static_cast <size_t> (Value::Index::Integer64));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Unsigned64));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Real));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::String));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::ArrayValue));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::ObjectValue));

    value.set <uint64_t> (std::numeric_limits <uint64_t>::max ());
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Null));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Boolean));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Integer));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Unsigned));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Integer64));
    ASSERT_EQ (value.index (), static_cast <size_t> (Value::Index::Unsigned64));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Real));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::String));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::ArrayValue));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::ObjectValue));

    value.set <double> (std::numeric_limits <double>::min ());
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Null));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Boolean));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Integer));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Unsigned));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Integer64));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Unsigned64));
    ASSERT_EQ (value.index (), static_cast <size_t> (Value::Index::Real));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::String));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::ArrayValue));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::ObjectValue));

    value.set <std::string> ("foobar");
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Null));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Boolean));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Integer));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Unsigned));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Integer64));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Unsigned64));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Real));
    ASSERT_EQ (value.index (), static_cast <size_t> (Value::Index::String));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::ArrayValue));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::ObjectValue));

    value.set <Array, Array::value_type> ({1});
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Null));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Boolean));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Integer));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Unsigned));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Integer64));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Unsigned64));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Real));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::String));
    ASSERT_EQ (value.index (), static_cast <size_t> (Value::Index::ArrayValue));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::ObjectValue));

    value.set <Object, Object::value_type> ({{"i", 1}});
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Null));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Boolean));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Integer));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Unsigned));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Integer64));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Unsigned64));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::Real));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::String));
    ASSERT_NE (value.index (), static_cast <size_t> (Value::Index::ArrayValue));
    ASSERT_EQ (value.index (), static_cast <size_t> (Value::Index::ObjectValue));
}

/**
 * @brief Test at method.
 */
TEST (Value, at)
{
    Value value;

    value["null"] = nullptr;
    ASSERT_TRUE (value.at ("null").isNull ());

    value["boolean"] = true;
    ASSERT_TRUE (value.at ("boolean").isBool ());

    value["integer"] = std::numeric_limits <int64_t>::max ();
    ASSERT_TRUE (value.at ("integer").isInt64 ());

    value["double"] = std::numeric_limits <double>::max ();
    ASSERT_TRUE (value.at ("double").isDouble ());

    value["string"] = "foobar";
    ASSERT_TRUE (value.at ("string").isString ());

    value["array"] = Array ({1, 2, 3, 4});
    ASSERT_TRUE (value.at ("array").isArray ());

    value["object"] = Object ({{"foo", 1}, {"bar", 2}});
    ASSERT_TRUE (value.at ("object").isObject ());

    ASSERT_THROW (value.at ("non existing path"), std::out_of_range);
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

    value["integer"] = std::numeric_limits <int64_t>::max ();
    ASSERT_TRUE (value["integer"].isInt64 ());

    value["double"] = std::numeric_limits <double>::max ();
    ASSERT_TRUE (value["double"].isDouble ());

    value["string"] = "foobar";
    ASSERT_TRUE (value["string"].isString ());

    value["array"] = Array ({1, 2, 3, 4});
    ASSERT_TRUE (value["array"].isArray ());

    value["object"] = Object ({{"foo", 1}, {"bar", 2}});
    ASSERT_TRUE (value["object"].isObject ());
}

/**
 * @brief Test empty method.
 */
TEST (Value, empty)
{
    Value value;
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = true;
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = false;
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = std::numeric_limits <int8_t>::min ();
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = std::numeric_limits <int8_t>::max ();
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = std::numeric_limits <uint8_t>::min ();
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = std::numeric_limits <uint8_t>::max ();
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = std::numeric_limits <int16_t>::min ();
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = std::numeric_limits <int16_t>::max ();
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = std::numeric_limits <uint16_t>::min ();
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = std::numeric_limits <uint16_t>::max ();
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = std::numeric_limits <int32_t>::min ();
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = std::numeric_limits <int32_t>::max ();
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = std::numeric_limits <uint32_t>::min ();
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = std::numeric_limits <uint32_t>::max ();
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = std::numeric_limits <int64_t>::min ();
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = std::numeric_limits <int64_t>::max ();
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = std::numeric_limits <uint64_t>::min ();
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = std::numeric_limits <uint64_t>::max ();
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = std::numeric_limits <float>::min ();
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = std::numeric_limits <float>::max ();
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = std::numeric_limits <double>::min ();
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = std::numeric_limits <double>::max ();
    ASSERT_THROW (value.empty (), std::bad_cast);

    value = "";
    ASSERT_TRUE (value.empty ());

    value = "foo";
    ASSERT_FALSE (value.empty ());

    value = "02:42:64:2f:6a:d0";
    ASSERT_FALSE (value.empty ());

    value = "127.0.0.1";
    ASSERT_FALSE (value.empty ());

    value = Array ({});
    ASSERT_TRUE (value.empty ());

    value = Array ({1, 2, 3, 4});
    ASSERT_FALSE (value.empty ());

    value = Object ({});
    ASSERT_TRUE (value.empty ());

    value = Object ({{"i", 1}});
    ASSERT_FALSE (value.empty ());
}

/**
 * @brief Test size method.
 */
TEST (Value, size)
{
    Value value;
    ASSERT_THROW (value.size (), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.size (), std::bad_cast);

    value = true;
    ASSERT_THROW (value.size (), std::bad_cast);

    value = false;
    ASSERT_THROW (value.size (), std::bad_cast);

    value = std::numeric_limits <int8_t>::min ();
    ASSERT_THROW (value.size (), std::bad_cast);

    value = std::numeric_limits <int8_t>::max ();
    ASSERT_THROW (value.size (), std::bad_cast);

    value = std::numeric_limits <uint8_t>::min ();
    ASSERT_THROW (value.size (), std::bad_cast);

    value = std::numeric_limits <uint8_t>::max ();
    ASSERT_THROW (value.size (), std::bad_cast);

    value = std::numeric_limits <int16_t>::min ();
    ASSERT_THROW (value.size (), std::bad_cast);

    value = std::numeric_limits <int16_t>::max ();
    ASSERT_THROW (value.size (), std::bad_cast);

    value = std::numeric_limits <uint16_t>::min ();
    ASSERT_THROW (value.size (), std::bad_cast);

    value = std::numeric_limits <uint16_t>::max ();
    ASSERT_THROW (value.size (), std::bad_cast);

    value = std::numeric_limits <int32_t>::min ();
    ASSERT_THROW (value.size (), std::bad_cast);

    value = std::numeric_limits <int32_t>::max ();
    ASSERT_THROW (value.size (), std::bad_cast);

    value = std::numeric_limits <uint32_t>::min ();
    ASSERT_THROW (value.size (), std::bad_cast);

    value = std::numeric_limits <uint32_t>::max ();
    ASSERT_THROW (value.size (), std::bad_cast);

    value = std::numeric_limits <int64_t>::min ();
    ASSERT_THROW (value.size (), std::bad_cast);

    value = std::numeric_limits <int64_t>::max ();
    ASSERT_THROW (value.size (), std::bad_cast);

    value = std::numeric_limits <uint64_t>::min ();
    ASSERT_THROW (value.size (), std::bad_cast);

    value = std::numeric_limits <uint64_t>::max ();
    ASSERT_THROW (value.size (), std::bad_cast);

    value = std::numeric_limits <float>::min ();
    ASSERT_THROW (value.size (), std::bad_cast);

    value = std::numeric_limits <float>::max ();
    ASSERT_THROW (value.size (), std::bad_cast);

    value = std::numeric_limits <double>::min ();
    ASSERT_THROW (value.size (), std::bad_cast);

    value = std::numeric_limits <double>::max ();
    ASSERT_THROW (value.size (), std::bad_cast);

    value = "";
    ASSERT_EQ (value.size (), 0);

    value = "foo";
    ASSERT_EQ (value.size (), 3);

    value = "02:42:64:2f:6a:d0";
    ASSERT_EQ (value.size (), 17);

    value = "127.0.0.1";
    ASSERT_EQ (value.size (), 9);

    value = Array ({1});
    ASSERT_EQ (value.size (), 1);

    value = Object ({{"i", 1}});
    ASSERT_EQ (value.size (), 1);
}
/**
 * @brief Test reserve method.
 */
TEST (Value, reserve)
{
    Value value;
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = true;
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = false;
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = std::numeric_limits <int8_t>::min ();
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = std::numeric_limits <int8_t>::max ();
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = std::numeric_limits <uint8_t>::min ();
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = std::numeric_limits <uint8_t>::max ();
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = std::numeric_limits <int16_t>::min ();
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = std::numeric_limits <int16_t>::max ();
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = std::numeric_limits <uint16_t>::min ();
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = std::numeric_limits <uint16_t>::max ();
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = std::numeric_limits <int32_t>::min ();
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = std::numeric_limits <int32_t>::max ();
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = std::numeric_limits <uint32_t>::min ();
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = std::numeric_limits <uint32_t>::max ();
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = std::numeric_limits <int64_t>::min ();
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = std::numeric_limits <int64_t>::max ();
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = std::numeric_limits <uint64_t>::min ();
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = std::numeric_limits <uint64_t>::max ();
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = std::numeric_limits <float>::min ();
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = std::numeric_limits <float>::max ();
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = std::numeric_limits <double>::min ();
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = std::numeric_limits <double>::max ();
    ASSERT_THROW (value.reserve (12), std::bad_cast);

    value = "";
    ASSERT_NO_THROW (value.reserve (12));

    value = "foo";
    ASSERT_NO_THROW (value.reserve (12));

    value = "02:42:64:2f:6a:d0";
    ASSERT_NO_THROW (value.reserve (12));

    value = "127.0.0.1";
    ASSERT_NO_THROW (value.reserve (12));

    value = Array ({1});
    ASSERT_NO_THROW (value.reserve (12));

    value = Object ({{"i", 1}});
    ASSERT_NO_THROW (value.reserve (12));
}

/**
 * @brief Test clear method.
 */
TEST (Value, clear)
{
    Value value;
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = true;
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = false;
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = std::numeric_limits <int8_t>::min ();
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = std::numeric_limits <int8_t>::max ();
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = std::numeric_limits <uint8_t>::min ();
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = std::numeric_limits <uint8_t>::max ();
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = std::numeric_limits <int16_t>::min ();
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = std::numeric_limits <int16_t>::max ();
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = std::numeric_limits <uint16_t>::min ();
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = std::numeric_limits <uint16_t>::max ();
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = std::numeric_limits <int32_t>::min ();
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = std::numeric_limits <int32_t>::max ();
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = std::numeric_limits <uint32_t>::min ();
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = std::numeric_limits <uint32_t>::max ();
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = std::numeric_limits <int64_t>::min ();
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = std::numeric_limits <int64_t>::max ();
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = std::numeric_limits <uint64_t>::min ();
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = std::numeric_limits <uint64_t>::max ();
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = std::numeric_limits <float>::min ();
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = std::numeric_limits <float>::max ();
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = std::numeric_limits <double>::min ();
    ASSERT_THROW (value.clear (), std::bad_cast);

    value = std::numeric_limits <double>::max ();
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

    value = Array ({1});
    ASSERT_FALSE (value.empty ());
    ASSERT_NO_THROW (value.clear ());
    ASSERT_TRUE (value.empty ());

    value = Object ({{"i", 1}});
    ASSERT_FALSE (value.empty ());
    ASSERT_NO_THROW (value.clear ());
    ASSERT_TRUE (value.empty ());
}

/**
 * @brief Test insert method.
 */
TEST (Value, insert)
{
    Value value;
    ASSERT_THROW (value.insert (std::make_pair ("i", 1)), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.insert (std::make_pair ("i", 1)), std::bad_cast);

    value = true;
    ASSERT_THROW (value.insert (std::make_pair ("i", 1)), std::bad_cast);

    value = false;
    ASSERT_THROW (value.insert (std::make_pair ("i", 1)), std::bad_cast);

    value = std::numeric_limits <int8_t>::min ();
    ASSERT_THROW (value.insert (std::make_pair ("i", 1)), std::bad_cast);

    value = std::numeric_limits <int8_t>::max ();
    ASSERT_THROW (value.insert (std::make_pair ("i", 1)), std::bad_cast);

    value = std::numeric_limits <uint8_t>::min ();
    ASSERT_THROW (value.insert (std::make_pair ("i", 1)), std::bad_cast);

    value = std::numeric_limits <uint8_t>::max ();
    ASSERT_THROW (value.insert (std::make_pair ("i", 1)), std::bad_cast);

    value = std::numeric_limits <int16_t>::min ();
    ASSERT_THROW (value.insert (std::make_pair ("i", 1)), std::bad_cast);

    value = std::numeric_limits <int16_t>::max ();
    ASSERT_THROW (value.insert (std::make_pair ("i", 1)), std::bad_cast);

    value = std::numeric_limits <uint16_t>::min ();
    ASSERT_THROW (value.insert (std::make_pair ("i", 1)), std::bad_cast);

    value = std::numeric_limits <uint16_t>::max ();
    ASSERT_THROW (value.insert (std::make_pair ("i", 1)), std::bad_cast);

    value = std::numeric_limits <int32_t>::min ();
    ASSERT_THROW (value.insert (std::make_pair ("i", 1)), std::bad_cast);

    value = std::numeric_limits <int32_t>::max ();
    ASSERT_THROW (value.insert (std::make_pair ("i", 1)), std::bad_cast);

    value = std::numeric_limits <uint32_t>::min ();
    ASSERT_THROW (value.insert (std::make_pair ("i", 1)), std::bad_cast);

    value = std::numeric_limits <uint32_t>::max ();
    ASSERT_THROW (value.insert (std::make_pair ("i", 1)), std::bad_cast);

    value = std::numeric_limits <int64_t>::min ();
    ASSERT_THROW (value.insert (std::make_pair ("i", 1)), std::bad_cast);

    value = std::numeric_limits <int64_t>::max ();
    ASSERT_THROW (value.insert (std::make_pair ("i", 1)), std::bad_cast);

    value = std::numeric_limits <uint64_t>::min ();
    ASSERT_THROW (value.insert (std::make_pair ("i", 1)), std::bad_cast);

    value = std::numeric_limits <uint64_t>::max ();
    ASSERT_THROW (value.insert (std::make_pair ("i", 1)), std::bad_cast);

    value = std::numeric_limits <float>::min ();
    ASSERT_THROW (value.insert (std::make_pair ("i", 1)), std::bad_cast);

    value = std::numeric_limits <float>::max ();
    ASSERT_THROW (value.insert (std::make_pair ("i", 1)), std::bad_cast);

    value = std::numeric_limits <double>::min ();
    ASSERT_THROW (value.insert (std::make_pair ("i", 1)), std::bad_cast);

    value = std::numeric_limits <double>::max ();
    ASSERT_THROW (value.insert (std::make_pair ("i", 1)), std::bad_cast);

    value = "";
    ASSERT_THROW (value.insert (std::make_pair ("i", 1)), std::bad_cast);

    value = "foo";
    ASSERT_THROW (value.insert (std::make_pair ("i", 1)), std::bad_cast);

    value = "02:42:64:2f:6a:d0";
    ASSERT_THROW (value.insert (std::make_pair ("i", 1)), std::bad_cast);

    value = "127.0.0.1";
    ASSERT_THROW (value.insert (std::make_pair ("i", 1)), std::bad_cast);

    value = Array ({1});
    ASSERT_THROW (value.insert (std::make_pair ("i", 1)), std::bad_cast);

    value = Object ({{"i", 1}});
    ASSERT_NO_THROW (value.insert (std::make_pair ("i", 1)));
}

/**
 * @brief Test erase method.
 */
TEST (Value, erase)
{
    Value value;
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = true;
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = false;
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = std::numeric_limits <int8_t>::min ();
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = std::numeric_limits <int8_t>::max ();
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = std::numeric_limits <uint8_t>::min ();
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = std::numeric_limits <uint8_t>::max ();
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = std::numeric_limits <int16_t>::min ();
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = std::numeric_limits <int16_t>::max ();
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = std::numeric_limits <uint16_t>::min ();
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = std::numeric_limits <uint16_t>::max ();
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = std::numeric_limits <int32_t>::min ();
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = std::numeric_limits <int32_t>::max ();
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = std::numeric_limits <uint32_t>::min ();
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = std::numeric_limits <uint32_t>::max ();
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = std::numeric_limits <int64_t>::min ();
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = std::numeric_limits <int64_t>::max ();
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = std::numeric_limits <uint64_t>::min ();
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = std::numeric_limits <uint64_t>::max ();
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = std::numeric_limits <float>::min ();
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = std::numeric_limits <float>::max ();
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = std::numeric_limits <double>::min ();
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = std::numeric_limits <double>::max ();
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = "";
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = "foo";
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = "02:42:64:2f:6a:d0";
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = "127.0.0.1";
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = Array ({1});
    ASSERT_THROW (value.erase ("i"), std::bad_cast);

    value = Object ({{"i", 1}});
    ASSERT_EQ (value.erase ("i"), 1);
    ASSERT_EQ (value.erase ("j"), 0);
}

/**
 * @brief Test pushBack method.
 */
TEST (Value, pushBack)
{
    Value value;
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = true;
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = false;
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = std::numeric_limits <int8_t>::min ();
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = std::numeric_limits <int8_t>::max ();
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = std::numeric_limits <uint8_t>::min ();
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = std::numeric_limits <uint8_t>::max ();
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = std::numeric_limits <int16_t>::min ();
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = std::numeric_limits <int16_t>::max ();
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = std::numeric_limits <uint16_t>::min ();
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = std::numeric_limits <uint16_t>::max ();
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = std::numeric_limits <int32_t>::min ();
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = std::numeric_limits <int32_t>::max ();
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = std::numeric_limits <uint32_t>::min ();
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = std::numeric_limits <uint32_t>::max ();
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = std::numeric_limits <int64_t>::min ();
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = std::numeric_limits <int64_t>::max ();
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = std::numeric_limits <uint64_t>::min ();
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = std::numeric_limits <uint64_t>::max ();
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = std::numeric_limits <float>::min ();
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = std::numeric_limits <float>::max ();
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = std::numeric_limits <double>::min ();
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = std::numeric_limits <double>::max ();
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = "";
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = "foo";
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = "02:42:64:2f:6a:d0";
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = "127.0.0.1";
    ASSERT_THROW (value.pushBack (1), std::bad_cast);

    value = Array ({1});
    ASSERT_NO_THROW (value.pushBack (1));

    value = Object ({{"i", 1}});
    ASSERT_THROW (value.pushBack (1), std::bad_cast);
}

/**
 * @brief Test popBack method.
 */
TEST (Value, popBack)
{
    Value value;
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = nullptr;
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = true;
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = false;
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = std::numeric_limits <int8_t>::min ();
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = std::numeric_limits <int8_t>::max ();
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = std::numeric_limits <uint8_t>::min ();
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = std::numeric_limits <uint8_t>::max ();
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = std::numeric_limits <int16_t>::min ();
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = std::numeric_limits <int16_t>::max ();
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = std::numeric_limits <uint16_t>::min ();
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = std::numeric_limits <uint16_t>::max ();
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = std::numeric_limits <int32_t>::min ();
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = std::numeric_limits <int32_t>::max ();
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = std::numeric_limits <uint32_t>::min ();
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = std::numeric_limits <uint32_t>::max ();
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = std::numeric_limits <int64_t>::min ();
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = std::numeric_limits <int64_t>::max ();
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = std::numeric_limits <uint64_t>::min ();
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = std::numeric_limits <uint64_t>::max ();
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = std::numeric_limits <float>::min ();
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = std::numeric_limits <float>::max ();
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = std::numeric_limits <double>::min ();
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = std::numeric_limits <double>::max ();
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = "";
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = "foo";
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = "02:42:64:2f:6a:d0";
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = "127.0.0.1";
    ASSERT_THROW (value.popBack (), std::bad_cast);

    value = Array ({1});
    ASSERT_NO_THROW (value.popBack ());

    value = Object ({{"i", 1}});
    ASSERT_THROW (value.popBack (), std::bad_cast);
}

/**
 * @brief Test contains method.
 */
TEST (Value, contains)
{
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

    value = std::numeric_limits <int8_t>::min ();
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = std::numeric_limits <int8_t>::max ();
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = std::numeric_limits <uint8_t>::min ();
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = std::numeric_limits <uint8_t>::max ();
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = std::numeric_limits <int16_t>::min ();
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = std::numeric_limits <int16_t>::max ();
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = std::numeric_limits <uint16_t>::min ();
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = std::numeric_limits <uint16_t>::max ();
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = std::numeric_limits <int32_t>::min ();
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = std::numeric_limits <int32_t>::max ();
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = std::numeric_limits <uint32_t>::min ();
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = std::numeric_limits <uint32_t>::max ();
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = std::numeric_limits <int64_t>::min ();
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = std::numeric_limits <int64_t>::max ();
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = std::numeric_limits <uint64_t>::min ();
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = std::numeric_limits <uint64_t>::max ();
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = std::numeric_limits <float>::min ();
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = std::numeric_limits <float>::max ();
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = std::numeric_limits <double>::min ();
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = std::numeric_limits <double>::max ();
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

    value = Array ({});
    ASSERT_FALSE (value.contains (0));
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = Array ({1});
    ASSERT_TRUE (value.contains (0));
    ASSERT_THROW (value.contains ("i"), std::bad_cast);

    value = Object ({});
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_FALSE (value.contains ("i"));

    value = Object ({{"i", 1}});
    ASSERT_THROW (value.contains (0), std::bad_cast);
    ASSERT_TRUE (value.contains ("i"));
}

/**
 * @brief Test swap method.
 */
TEST (Value, swap)
{
    Value value = nullptr;

    Value other = Array ({1});
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

    other = std::numeric_limits <int32_t>::min ();
    ASSERT_TRUE (value.isBool ());
    ASSERT_TRUE (other.isInt ());
    value.swap (other);
    ASSERT_TRUE (value.isInt ());
    ASSERT_TRUE (other.isBool ());

    other = std::numeric_limits <uint32_t>::max ();
    ASSERT_TRUE (value.isInt ());
    ASSERT_TRUE (other.isUint ());
    value.swap (other);
    ASSERT_TRUE (value.isUint ());
    ASSERT_TRUE (other.isInt ());

    other = std::numeric_limits <int64_t>::min ();
    ASSERT_TRUE (value.isUint ());
    ASSERT_TRUE (other.isInt64 ());
    value.swap (other);
    ASSERT_TRUE (value.isInt64 ());
    ASSERT_TRUE (other.isUint ());

    other = std::numeric_limits <uint64_t>::max ();
    ASSERT_TRUE (value.isInt64 ());
    ASSERT_TRUE (other.isUint64 ());
    value.swap (other);
    ASSERT_TRUE (value.isUint64 ());
    ASSERT_TRUE (other.isInt64 ());

    other = std::numeric_limits <float>::min ();
    ASSERT_TRUE (value.isUint64 ());
    ASSERT_TRUE (other.isFloat ());
    value.swap (other);
    ASSERT_TRUE (value.isFloat ());
    ASSERT_TRUE (other.isUint64 ());

    other = std::numeric_limits <double>::max ();
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

    other = Array ({1});
    ASSERT_TRUE (value.isString ());
    ASSERT_TRUE (other.isArray ());
    value.swap (other);
    ASSERT_TRUE (value.isArray ());
    ASSERT_TRUE (other.isString ());

    other = Object ({{"i", 1}});
    ASSERT_TRUE (value.isArray ());
    ASSERT_TRUE (other.isObject ());
    value.swap (other);
    ASSERT_TRUE (value.isObject ());
    ASSERT_TRUE (other.isArray ());
}

/**
 * @brief equal operators.
 */
TEST (Value, equal)
{
    Value value;

    value.set <std::nullptr_t> (nullptr);
    ASSERT_TRUE  (value == nullptr);
    ASSERT_FALSE (value == true);
    ASSERT_FALSE (value == false);
    ASSERT_FALSE (value == std::numeric_limits <int32_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits <int32_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits <uint32_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits <uint32_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits <int64_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits <int64_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits <uint64_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits <uint64_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits <double>::min ());
    ASSERT_FALSE (value == std::numeric_limits <double>::max ());
    ASSERT_FALSE (value == "foo");
    ASSERT_FALSE (value == Array ({1}));
    ASSERT_FALSE (value == Object ({{"i", 1}}));

    value.set <bool> (true);
    ASSERT_FALSE (value == nullptr);
    ASSERT_TRUE  (value == true);
    ASSERT_FALSE (value == false);
    ASSERT_FALSE (value == std::numeric_limits <int32_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits <int32_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits <uint32_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits <uint32_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits <int64_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits <int64_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits <uint64_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits <uint64_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits <double>::min ());
    ASSERT_FALSE (value == std::numeric_limits <double>::max ());
    ASSERT_FALSE (value == "foo");
    ASSERT_FALSE (value == Array ({1}));
    ASSERT_FALSE (value == Object ({{"i", 1}}));

    value.set <int32_t> (std::numeric_limits <int32_t>::min ());
    ASSERT_FALSE (value == nullptr);
    ASSERT_FALSE (value == true);
    ASSERT_FALSE (value == false);
    ASSERT_TRUE  (value == std::numeric_limits <int32_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits <int32_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits <uint32_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits <uint32_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits <int64_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits <int64_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits <uint64_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits <uint64_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits <double>::min ());
    ASSERT_FALSE (value == std::numeric_limits <double>::max ());
    ASSERT_FALSE (value == "foo");
    ASSERT_FALSE (value == Array ({1}));
    ASSERT_FALSE (value == Object ({{"i", 1}}));

    value.set <uint32_t> (std::numeric_limits <uint32_t>::max ());
    ASSERT_FALSE (value == nullptr);
    ASSERT_FALSE (value == true);
    ASSERT_FALSE (value == false);
    ASSERT_FALSE (value == std::numeric_limits <int32_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits <int32_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits <uint32_t>::min ());
    ASSERT_TRUE  (value == std::numeric_limits <uint32_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits <int64_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits <int64_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits <uint64_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits <uint64_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits <double>::min ());
    ASSERT_FALSE (value == std::numeric_limits <double>::max ());
    ASSERT_FALSE (value == "foo");
    ASSERT_FALSE (value == Array ({1}));
    ASSERT_FALSE (value == Object ({{"i", 1}}));

    value.set <int64_t> (std::numeric_limits <int64_t>::min ());
    ASSERT_FALSE (value == nullptr);
    ASSERT_FALSE (value == true);
    ASSERT_FALSE (value == false);
    ASSERT_FALSE (value == std::numeric_limits <int32_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits <int32_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits <uint32_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits <uint32_t>::max ());
    ASSERT_TRUE  (value == std::numeric_limits <int64_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits <int64_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits <uint64_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits <uint64_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits <double>::min ());
    ASSERT_FALSE (value == std::numeric_limits <double>::max ());
    ASSERT_FALSE (value == "foo");
    ASSERT_FALSE (value == Array ({1}));
    ASSERT_FALSE (value == Object ({{"i", 1}}));

    value.set <uint64_t> (std::numeric_limits <uint64_t>::max ());
    ASSERT_FALSE (value == nullptr);
    ASSERT_FALSE (value == true);
    ASSERT_FALSE (value == false);
    ASSERT_FALSE (value == std::numeric_limits <int32_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits <int32_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits <uint32_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits <uint32_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits <int64_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits <int64_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits <uint64_t>::min ());
    ASSERT_TRUE  (value == std::numeric_limits <uint64_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits <double>::min ());
    ASSERT_FALSE (value == std::numeric_limits <double>::max ());
    ASSERT_FALSE (value == "foo");
    ASSERT_FALSE (value == Array ({1}));
    ASSERT_FALSE (value == Object ({{"i", 1}}));

    value.set <double> (std::numeric_limits <double>::min ());
    ASSERT_FALSE (value == nullptr);
    ASSERT_FALSE (value == true);
    ASSERT_FALSE (value == false);
    ASSERT_FALSE (value == std::numeric_limits <int32_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits <int32_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits <uint32_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits <uint32_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits <int64_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits <int64_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits <uint64_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits <uint64_t>::max ());
    ASSERT_TRUE  (value == std::numeric_limits <double>::min ());
    ASSERT_FALSE (value == std::numeric_limits <double>::max ());
    ASSERT_FALSE (value == "foo");
    ASSERT_FALSE (value == Array ({1}));
    ASSERT_FALSE (value == Object ({{"i", 1}}));

    value.set <std::string> ("foo");
    ASSERT_FALSE (value == nullptr);
    ASSERT_FALSE (value == true);
    ASSERT_FALSE (value == false);
    ASSERT_FALSE (value == std::numeric_limits <int32_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits <int32_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits <uint32_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits <uint32_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits <int64_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits <int64_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits <uint64_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits <uint64_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits <double>::min ());
    ASSERT_FALSE (value == std::numeric_limits <double>::max ());
    ASSERT_TRUE  (value == "foo");
    ASSERT_FALSE (value == "bar");
    ASSERT_FALSE (value == Array ({1}));
    ASSERT_FALSE (value == Object ({{"i", 1}}));

    value.set <Array, Array::value_type> ({1});
    ASSERT_FALSE (value == nullptr);
    ASSERT_FALSE (value == true);
    ASSERT_FALSE (value == false);
    ASSERT_FALSE (value == std::numeric_limits <int32_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits <int32_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits <uint32_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits <uint32_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits <int64_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits <int64_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits <uint64_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits <uint64_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits <double>::min ());
    ASSERT_FALSE (value == std::numeric_limits <double>::max ());
    ASSERT_FALSE (value == "foo");
    ASSERT_TRUE  (value == Array ({1}));
    ASSERT_FALSE (value == Object ({{"i", 1}}));

    value.set <Object, Object::value_type> ({{"i", 1}});
    ASSERT_FALSE (value == nullptr);
    ASSERT_FALSE (value == true);
    ASSERT_FALSE (value == false);
    ASSERT_FALSE (value == std::numeric_limits <int32_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits <int32_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits <uint32_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits <uint32_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits <int64_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits <int64_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits <uint64_t>::min ());
    ASSERT_FALSE (value == std::numeric_limits <uint64_t>::max ());
    ASSERT_FALSE (value == std::numeric_limits <double>::min ());
    ASSERT_FALSE (value == std::numeric_limits <double>::max ());
    ASSERT_FALSE (value == "foo");
    ASSERT_FALSE (value == Array ({1}));
    ASSERT_TRUE  (value == Object ({{"i", 1}}));

    value.set <std::nullptr_t> (nullptr);
    ASSERT_TRUE  (nullptr == value);
    ASSERT_FALSE (true == value);
    ASSERT_FALSE (false == value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits <double>::min () == value);
    ASSERT_FALSE (std::numeric_limits <double>::max () == value);
    ASSERT_FALSE ("foo" == value);
    ASSERT_FALSE (Array ({1}) == value);
    ASSERT_FALSE (Object ({{"i", 1}}) == value);

    value.set <bool> (true);
    ASSERT_FALSE (nullptr == value);
    ASSERT_TRUE  (true == value);
    ASSERT_FALSE (false == value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits <double>::min () == value);
    ASSERT_FALSE (std::numeric_limits <double>::max () == value);
    ASSERT_FALSE ("foo" == value);
    ASSERT_FALSE (Array ({1}) == value);
    ASSERT_FALSE (Object ({{"i", 1}}) == value);

    value.set <int32_t> (std::numeric_limits <int32_t>::min ());
    ASSERT_FALSE (nullptr == value);
    ASSERT_FALSE (true == value);
    ASSERT_FALSE (false == value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits <double>::min () == value);
    ASSERT_FALSE (std::numeric_limits <double>::max () == value);
    ASSERT_FALSE ("foo" == value);
    ASSERT_FALSE (Array ({1}) == value);
    ASSERT_FALSE (Object ({{"i", 1}}) == value);

    value.set <uint32_t> (std::numeric_limits <uint32_t>::max ());
    ASSERT_FALSE (nullptr == value);
    ASSERT_FALSE (true == value);
    ASSERT_FALSE (false == value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::min () == value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits <double>::min () == value);
    ASSERT_FALSE (std::numeric_limits <double>::max () == value);
    ASSERT_FALSE ("foo" == value);
    ASSERT_FALSE (Array ({1}) == value);
    ASSERT_FALSE (Object ({{"i", 1}}) == value);

    value.set <int64_t> (std::numeric_limits <int64_t>::min ());
    ASSERT_FALSE (nullptr == value);
    ASSERT_FALSE (true == value);
    ASSERT_FALSE (false == value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::max () == value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits <double>::min () == value);
    ASSERT_FALSE (std::numeric_limits <double>::max () == value);
    ASSERT_FALSE ("foo" == value);
    ASSERT_FALSE (Array ({1}) == value);
    ASSERT_FALSE (Object ({{"i", 1}}) == value);

    value.set <uint64_t> (std::numeric_limits <uint64_t>::max ());
    ASSERT_FALSE (nullptr == value);
    ASSERT_FALSE (true == value);
    ASSERT_FALSE (false == value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::min () == value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits <double>::min () == value);
    ASSERT_FALSE (std::numeric_limits <double>::max () == value);
    ASSERT_FALSE ("foo" == value);
    ASSERT_FALSE (Array ({1}) == value);
    ASSERT_FALSE (Object ({{"i", 1}}) == value);

    value.set <double> (std::numeric_limits <double>::min ());
    ASSERT_FALSE (nullptr == value);
    ASSERT_FALSE (true == value);
    ASSERT_FALSE (false == value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::max () == value);
    ASSERT_TRUE  (std::numeric_limits <double>::min () == value);
    ASSERT_FALSE (std::numeric_limits <double>::max () == value);
    ASSERT_FALSE ("foo" == value);
    ASSERT_FALSE (Array ({1}) == value);
    ASSERT_FALSE (Object ({{"i", 1}}) == value);

    value.set <std::string> ("foo");
    ASSERT_FALSE (nullptr == value);
    ASSERT_FALSE (true == value);
    ASSERT_FALSE (false == value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits <double>::min () == value);
    ASSERT_FALSE (std::numeric_limits <double>::max () == value);
    ASSERT_TRUE  ("foo" == value);
    ASSERT_FALSE ("bar" == value);
    ASSERT_FALSE (Array ({1}) == value);
    ASSERT_FALSE (Object ({{"i", 1}}) == value);

    value.set <Array, Array::value_type> ({1});
    ASSERT_FALSE (nullptr == value);
    ASSERT_FALSE (true == value);
    ASSERT_FALSE (false == value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits <double>::min () == value);
    ASSERT_FALSE (std::numeric_limits <double>::max () == value);
    ASSERT_FALSE ("foo" == value);
    ASSERT_TRUE  (Array ({1}) == value);
    ASSERT_FALSE (Object ({{"i", 1}}) == value);

    value.set <Object, Object::value_type> ({{"i", 1}});
    ASSERT_FALSE (nullptr == value);
    ASSERT_FALSE (true == value);
    ASSERT_FALSE (false == value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::min () == value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::max () == value);
    ASSERT_FALSE (std::numeric_limits <double>::min () == value);
    ASSERT_FALSE (std::numeric_limits <double>::max () == value);
    ASSERT_FALSE ("foo" == value);
    ASSERT_FALSE (Array ({1}) == value);
    ASSERT_TRUE  (Object ({{"i", 1}}) == value);
}

/**
 * @brief notEqual operators.
 */
TEST (Value, notEqual)
{
    Value value;

    value.set <std::nullptr_t> (nullptr);
    ASSERT_FALSE (value != nullptr);
    ASSERT_TRUE  (value != true);
    ASSERT_TRUE  (value != false);
    ASSERT_TRUE  (value != std::numeric_limits <int32_t>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <int32_t>::max ());
    ASSERT_TRUE  (value != std::numeric_limits <uint32_t>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <uint32_t>::max ());
    ASSERT_TRUE  (value != std::numeric_limits <int64_t>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <int64_t>::max ());
    ASSERT_TRUE  (value != std::numeric_limits <uint64_t>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <uint64_t>::max ());
    ASSERT_TRUE  (value != std::numeric_limits <double>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <double>::max ());
    ASSERT_TRUE  (value != "foo");
    ASSERT_TRUE  (value != Array ({1}));
    ASSERT_TRUE  (value != Object ({{"i", 1}}));

    value.set <bool> (true);
    ASSERT_TRUE  (value != nullptr);
    ASSERT_FALSE (value != true);
    ASSERT_TRUE  (value != false);
    ASSERT_TRUE  (value != std::numeric_limits <int32_t>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <int32_t>::max ());
    ASSERT_TRUE  (value != std::numeric_limits <uint32_t>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <uint32_t>::max ());
    ASSERT_TRUE  (value != std::numeric_limits <int64_t>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <int64_t>::max ());
    ASSERT_TRUE  (value != std::numeric_limits <uint64_t>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <uint64_t>::max ());
    ASSERT_TRUE  (value != std::numeric_limits <double>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <double>::max ());
    ASSERT_TRUE  (value != "foo");
    ASSERT_TRUE  (value != Array ({1}));
    ASSERT_TRUE  (value != Object ({{"i", 1}}));

    value.set <int32_t> (std::numeric_limits <int32_t>::min ());
    ASSERT_TRUE  (value != nullptr);
    ASSERT_TRUE  (value != true);
    ASSERT_TRUE  (value != false);
    ASSERT_FALSE (value != std::numeric_limits <int32_t>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <int32_t>::max ());
    ASSERT_TRUE  (value != std::numeric_limits <uint32_t>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <uint32_t>::max ());
    ASSERT_TRUE  (value != std::numeric_limits <int64_t>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <int64_t>::max ());
    ASSERT_TRUE  (value != std::numeric_limits <uint64_t>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <uint64_t>::max ());
    ASSERT_TRUE  (value != std::numeric_limits <double>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <double>::max ());
    ASSERT_TRUE  (value != "foo");
    ASSERT_TRUE  (value != Array ({1}));
    ASSERT_TRUE  (value != Object ({{"i", 1}}));

    value.set <uint32_t> (std::numeric_limits <uint32_t>::max ());
    ASSERT_TRUE  (value != nullptr);
    ASSERT_TRUE  (value != true);
    ASSERT_TRUE  (value != false);
    ASSERT_TRUE  (value != std::numeric_limits <int32_t>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <int32_t>::max ());
    ASSERT_TRUE  (value != std::numeric_limits <uint32_t>::min ());
    ASSERT_FALSE (value != std::numeric_limits <uint32_t>::max ());
    ASSERT_TRUE  (value != std::numeric_limits <int64_t>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <int64_t>::max ());
    ASSERT_TRUE  (value != std::numeric_limits <uint64_t>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <uint64_t>::max ());
    ASSERT_TRUE  (value != std::numeric_limits <double>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <double>::max ());
    ASSERT_TRUE  (value != "foo");
    ASSERT_TRUE  (value != Array ({1}));
    ASSERT_TRUE  (value != Object ({{"i", 1}}));

    value.set <int64_t> (std::numeric_limits <int64_t>::min ());
    ASSERT_TRUE  (value != nullptr);
    ASSERT_TRUE  (value != true);
    ASSERT_TRUE  (value != false);
    ASSERT_TRUE  (value != std::numeric_limits <int32_t>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <int32_t>::max ());
    ASSERT_TRUE  (value != std::numeric_limits <uint32_t>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <uint32_t>::max ());
    ASSERT_FALSE (value != std::numeric_limits <int64_t>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <int64_t>::max ());
    ASSERT_TRUE  (value != std::numeric_limits <uint64_t>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <uint64_t>::max ());
    ASSERT_TRUE  (value != std::numeric_limits <double>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <double>::max ());
    ASSERT_TRUE  (value != "foo");
    ASSERT_TRUE  (value != Array ({1}));
    ASSERT_TRUE  (value != Object ({{"i", 1}}));

    value.set <uint64_t> (std::numeric_limits <uint64_t>::max ());
    ASSERT_TRUE  (value != nullptr);
    ASSERT_TRUE  (value != true);
    ASSERT_TRUE  (value != false);
    ASSERT_TRUE  (value != std::numeric_limits <int32_t>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <int32_t>::max ());
    ASSERT_TRUE  (value != std::numeric_limits <uint32_t>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <uint32_t>::max ());
    ASSERT_TRUE  (value != std::numeric_limits <int64_t>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <int64_t>::max ());
    ASSERT_TRUE  (value != std::numeric_limits <uint64_t>::min ());
    ASSERT_FALSE (value != std::numeric_limits <uint64_t>::max ());
    ASSERT_TRUE  (value != std::numeric_limits <double>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <double>::max ());
    ASSERT_TRUE  (value != "foo");
    ASSERT_TRUE  (value != Array ({1}));
    ASSERT_TRUE  (value != Object ({{"i", 1}}));

    value.set <double> (std::numeric_limits <double>::min ());
    ASSERT_TRUE  (value != nullptr);
    ASSERT_TRUE  (value != true);
    ASSERT_TRUE  (value != false);
    ASSERT_TRUE  (value != std::numeric_limits <int32_t>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <int32_t>::max ());
    ASSERT_TRUE  (value != std::numeric_limits <uint32_t>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <uint32_t>::max ());
    ASSERT_TRUE  (value != std::numeric_limits <int64_t>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <int64_t>::max ());
    ASSERT_TRUE  (value != std::numeric_limits <uint64_t>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <uint64_t>::max ());
    ASSERT_FALSE (value != std::numeric_limits <double>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <double>::max ());
    ASSERT_TRUE  (value != "foo");
    ASSERT_TRUE  (value != Array ({1}));
    ASSERT_TRUE  (value != Object ({{"i", 1}}));

    value.set <std::string> ("foo");
    ASSERT_TRUE  (value != nullptr);
    ASSERT_TRUE  (value != true);
    ASSERT_TRUE  (value != false);
    ASSERT_TRUE  (value != std::numeric_limits <int32_t>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <int32_t>::max ());
    ASSERT_TRUE  (value != std::numeric_limits <uint32_t>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <uint32_t>::max ());
    ASSERT_TRUE  (value != std::numeric_limits <int64_t>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <int64_t>::max ());
    ASSERT_TRUE  (value != std::numeric_limits <uint64_t>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <uint64_t>::max ());
    ASSERT_TRUE  (value != std::numeric_limits <double>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <double>::max ());
    ASSERT_FALSE (value != "foo");
    ASSERT_TRUE  (value != Array ({1}));
    ASSERT_TRUE  (value != Object ({{"i", 1}}));

    value.set <Array, Array::value_type> ({1});
    ASSERT_TRUE  (value != nullptr);
    ASSERT_TRUE  (value != true);
    ASSERT_TRUE  (value != false);
    ASSERT_TRUE  (value != std::numeric_limits <int32_t>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <int32_t>::max ());
    ASSERT_TRUE  (value != std::numeric_limits <uint32_t>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <uint32_t>::max ());
    ASSERT_TRUE  (value != std::numeric_limits <int64_t>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <int64_t>::max ());
    ASSERT_TRUE  (value != std::numeric_limits <uint64_t>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <uint64_t>::max ());
    ASSERT_TRUE  (value != std::numeric_limits <double>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <double>::max ());
    ASSERT_TRUE  (value != "foo");
    ASSERT_FALSE (value != Array ({1}));
    ASSERT_TRUE  (value != Object ({{"i", 1}}));

    value.set <Object, Object::value_type> ({{"i", 1}});
    ASSERT_TRUE  (value != nullptr);
    ASSERT_TRUE  (value != true);
    ASSERT_TRUE  (value != false);
    ASSERT_TRUE  (value != std::numeric_limits <int32_t>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <int32_t>::max ());
    ASSERT_TRUE  (value != std::numeric_limits <uint32_t>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <uint32_t>::max ());
    ASSERT_TRUE  (value != std::numeric_limits <int64_t>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <int64_t>::max ());
    ASSERT_TRUE  (value != std::numeric_limits <uint64_t>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <uint64_t>::max ());
    ASSERT_TRUE  (value != std::numeric_limits <double>::min ());
    ASSERT_TRUE  (value != std::numeric_limits <double>::max ());
    ASSERT_TRUE  (value != "foo");
    ASSERT_TRUE  (value != Array ({1}));
    ASSERT_FALSE (value != Object ({{"i", 1}}));

    value.set <std::nullptr_t> (nullptr);
    ASSERT_FALSE (nullptr != value);
    ASSERT_TRUE  (true != value);
    ASSERT_TRUE  (false != value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::max () != value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::max () != value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::max () != value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::max () != value);
    ASSERT_TRUE  (std::numeric_limits <double>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <double>::max () != value);
    ASSERT_TRUE  ("foo" != value);
    ASSERT_TRUE  (Array ({1}) != value);
    ASSERT_TRUE  (Object ({{"i", 1}}) != value);

    value.set <bool> (true);
    ASSERT_TRUE  (nullptr != value);
    ASSERT_FALSE (true != value);
    ASSERT_TRUE  (false != value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::max () != value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::max () != value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::max () != value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::max () != value);
    ASSERT_TRUE  (std::numeric_limits <double>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <double>::max () != value);
    ASSERT_TRUE  ("foo" != value);
    ASSERT_TRUE  (Array ({1}) != value);
    ASSERT_TRUE  (Object ({{"i", 1}}) != value);

    value.set <int32_t> (std::numeric_limits <int32_t>::min ());
    ASSERT_TRUE  (nullptr != value);
    ASSERT_TRUE  (true != value);
    ASSERT_TRUE  (false != value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::max () != value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::max () != value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::max () != value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::max () != value);
    ASSERT_TRUE  (std::numeric_limits <double>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <double>::max () != value);
    ASSERT_TRUE  ("foo" != value);
    ASSERT_TRUE  (Array ({1}) != value);
    ASSERT_TRUE  (Object ({{"i", 1}}) != value);

    value.set <uint32_t> (std::numeric_limits <uint32_t>::max ());
    ASSERT_TRUE  (nullptr != value);
    ASSERT_TRUE  (true != value);
    ASSERT_TRUE  (false != value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::max () != value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::min () != value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::max () != value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::max () != value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::max () != value);
    ASSERT_TRUE  (std::numeric_limits <double>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <double>::max () != value);
    ASSERT_TRUE  ("foo" != value);
    ASSERT_TRUE  (Array ({1}) != value);
    ASSERT_TRUE  (Object ({{"i", 1}}) != value);

    value.set <int64_t> (std::numeric_limits <int64_t>::min ());
    ASSERT_TRUE  (nullptr != value);
    ASSERT_TRUE  (true != value);
    ASSERT_TRUE  (false != value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::max () != value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::max () != value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::max () != value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::max () != value);
    ASSERT_TRUE  (std::numeric_limits <double>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <double>::max () != value);
    ASSERT_TRUE  ("foo" != value);
    ASSERT_TRUE  (Array ({1}) != value);
    ASSERT_TRUE  (Object ({{"i", 1}}) != value);

    value.set <uint64_t> (std::numeric_limits <uint64_t>::max ());
    ASSERT_TRUE  (nullptr != value);
    ASSERT_TRUE  (true != value);
    ASSERT_TRUE  (false != value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::max () != value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::max () != value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::max () != value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::min () != value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::max () != value);
    ASSERT_TRUE  (std::numeric_limits <double>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <double>::max () != value);
    ASSERT_TRUE  ("foo" != value);
    ASSERT_TRUE  (Array ({1}) != value);
    ASSERT_TRUE  (Object ({{"i", 1}}) != value);

    value.set <double> (std::numeric_limits <double>::min ());
    ASSERT_TRUE  (nullptr != value);
    ASSERT_TRUE  (true != value);
    ASSERT_TRUE  (false != value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::max () != value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::max () != value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::max () != value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::max () != value);
    ASSERT_FALSE (std::numeric_limits <double>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <double>::max () != value);
    ASSERT_TRUE  ("foo" != value);
    ASSERT_TRUE  (Array ({1}) != value);
    ASSERT_TRUE  (Object ({{"i", 1}}) != value);

    value.set <std::string> ("foo");
    ASSERT_TRUE  (nullptr != value);
    ASSERT_TRUE  (true != value);
    ASSERT_TRUE  (false != value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::max () != value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::max () != value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::max () != value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::max () != value);
    ASSERT_TRUE  (std::numeric_limits <double>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <double>::max () != value);
    ASSERT_FALSE ("foo" != value);
    ASSERT_TRUE  ("bar" != value);
    ASSERT_TRUE  (Array ({1}) != value);
    ASSERT_TRUE  (Object ({{"i", 1}}) != value);

    value.set <Array, Array::value_type> ({1});
    ASSERT_TRUE  (nullptr != value);
    ASSERT_TRUE  (true != value);
    ASSERT_TRUE  (false != value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::max () != value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::max () != value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::max () != value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::max () != value);
    ASSERT_TRUE  (std::numeric_limits <double>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <double>::max () != value);
    ASSERT_TRUE  ("foo" != value);
    ASSERT_FALSE (Array ({1}) != value);
    ASSERT_TRUE  (Object ({{"i", 1}}) != value);

    value.set <Object, Object::value_type> ({{"i", 1}});
    ASSERT_TRUE  (nullptr != value);
    ASSERT_TRUE  (true != value);
    ASSERT_TRUE  (false != value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::max () != value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::max () != value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::max () != value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::max () != value);
    ASSERT_TRUE  (std::numeric_limits <double>::min () != value);
    ASSERT_TRUE  (std::numeric_limits <double>::max () != value);
    ASSERT_TRUE  ("foo" != value);
    ASSERT_TRUE  (Array ({1}) != value);
    ASSERT_FALSE (Object ({{"i", 1}}) != value);
}

/**
 * @brief Test lower operator.
 */
TEST (Value, lower)
{
    Value value;

    value.set <std::nullptr_t> (nullptr);
    ASSERT_FALSE (value < nullptr);
    ASSERT_TRUE  (value < true);
    ASSERT_TRUE  (value < false);
    ASSERT_TRUE  (value < std::numeric_limits <int32_t>::min ());
    ASSERT_TRUE  (value < std::numeric_limits <int32_t>::max ());
    ASSERT_TRUE  (value < std::numeric_limits <uint32_t>::min ());
    ASSERT_TRUE  (value < std::numeric_limits <uint32_t>::max ());
    ASSERT_TRUE  (value < std::numeric_limits <int64_t>::min ());
    ASSERT_TRUE  (value < std::numeric_limits <int64_t>::max ());
    ASSERT_TRUE  (value < std::numeric_limits <uint64_t>::min ());
    ASSERT_TRUE  (value < std::numeric_limits <uint64_t>::max ());
    ASSERT_TRUE  (value < std::numeric_limits <double>::min ());
    ASSERT_TRUE  (value < std::numeric_limits <double>::max ());
    ASSERT_TRUE  (value < "foo");
    ASSERT_TRUE  (value < Array ({1}));
    ASSERT_TRUE  (value < Object ({{"i", 1}}));

    value.set <bool> (true);
    ASSERT_FALSE (value < nullptr);
    ASSERT_FALSE (value < true);
    ASSERT_FALSE (value < false);
    ASSERT_TRUE  (value < std::numeric_limits <int32_t>::min ());
    ASSERT_TRUE  (value < std::numeric_limits <int32_t>::max ());
    ASSERT_TRUE  (value < std::numeric_limits <uint32_t>::min ());
    ASSERT_TRUE  (value < std::numeric_limits <uint32_t>::max ());
    ASSERT_TRUE  (value < std::numeric_limits <int64_t>::min ());
    ASSERT_TRUE  (value < std::numeric_limits <int64_t>::max ());
    ASSERT_TRUE  (value < std::numeric_limits <uint64_t>::min ());
    ASSERT_TRUE  (value < std::numeric_limits <uint64_t>::max ());
    ASSERT_TRUE  (value < std::numeric_limits <double>::min ());
    ASSERT_TRUE  (value < std::numeric_limits <double>::max ());
    ASSERT_TRUE  (value < "foo");
    ASSERT_TRUE  (value < Array ({1}));
    ASSERT_TRUE  (value < Object ({{"i", 1}}));

    value.set <int32_t> (std::numeric_limits <int32_t>::min ());
    ASSERT_FALSE (value < nullptr);
    ASSERT_FALSE (value < true);
    ASSERT_FALSE (value < false);
    ASSERT_FALSE (value < std::numeric_limits <int32_t>::min ());
    ASSERT_TRUE  (value < std::numeric_limits <int32_t>::max ());
    ASSERT_TRUE  (value < std::numeric_limits <uint32_t>::min ());
    ASSERT_TRUE  (value < std::numeric_limits <uint32_t>::max ());
    ASSERT_FALSE (value < std::numeric_limits <int64_t>::min ());
    ASSERT_TRUE  (value < std::numeric_limits <int64_t>::max ());
    ASSERT_TRUE  (value < std::numeric_limits <uint64_t>::min ());
    ASSERT_TRUE  (value < std::numeric_limits <uint64_t>::max ());
    ASSERT_TRUE  (value < std::numeric_limits <double>::min ());
    ASSERT_TRUE  (value < std::numeric_limits <double>::max ());
    ASSERT_TRUE  (value < "foo");
    ASSERT_TRUE  (value < Array ({1}));
    ASSERT_TRUE  (value < Object ({{"i", 1}}));

    value.set <int64_t> (std::numeric_limits <int64_t>::max ());
    ASSERT_FALSE (value < nullptr);
    ASSERT_FALSE (value < true);
    ASSERT_FALSE (value < false);
    ASSERT_FALSE (value < std::numeric_limits <int32_t>::min ());
    ASSERT_FALSE (value < std::numeric_limits <int32_t>::max ());
    ASSERT_FALSE (value < std::numeric_limits <uint32_t>::min ());
    ASSERT_FALSE (value < std::numeric_limits <uint32_t>::max ());
    ASSERT_FALSE (value < std::numeric_limits <int64_t>::min ());
    ASSERT_FALSE (value < std::numeric_limits <int64_t>::max ());
    ASSERT_FALSE (value < std::numeric_limits <uint64_t>::min ());
    ASSERT_TRUE  (value < std::numeric_limits <uint64_t>::max ());
    ASSERT_FALSE (value < std::numeric_limits <double>::min ());
    ASSERT_TRUE  (value < std::numeric_limits <double>::max ());
    ASSERT_TRUE  (value < "foo");
    ASSERT_TRUE  (value < Array ({1}));
    ASSERT_TRUE  (value < Object ({{"i", 1}}));

    value.set <double> (std::numeric_limits <double>::min ());
    ASSERT_FALSE (value < nullptr);
    ASSERT_FALSE (value < true);
    ASSERT_FALSE (value < false);
    ASSERT_FALSE (value < std::numeric_limits <int32_t>::min ());
    ASSERT_TRUE  (value < std::numeric_limits <int32_t>::max ());
    ASSERT_FALSE (value < std::numeric_limits <uint32_t>::min ());
    ASSERT_TRUE  (value < std::numeric_limits <uint32_t>::max ());
    ASSERT_FALSE (value < std::numeric_limits <int64_t>::min ());
    ASSERT_TRUE  (value < std::numeric_limits <int64_t>::max ());
    ASSERT_FALSE (value < std::numeric_limits <uint64_t>::min ());
    ASSERT_TRUE  (value < std::numeric_limits <uint64_t>::max ());
    ASSERT_FALSE (value < std::numeric_limits <double>::min ());
    ASSERT_TRUE  (value < std::numeric_limits <double>::max ());
    ASSERT_TRUE  (value < "foo");
    ASSERT_TRUE  (value < Array ({1}));
    ASSERT_TRUE  (value < Object ({{"i", 1}}));

    value.set <std::string> ("foo");
    ASSERT_FALSE (value < nullptr);
    ASSERT_FALSE (value < true);
    ASSERT_FALSE (value < false);
    ASSERT_FALSE (value < std::numeric_limits <int32_t>::min ());
    ASSERT_FALSE (value < std::numeric_limits <int32_t>::max ());
    ASSERT_FALSE (value < std::numeric_limits <uint32_t>::min ());
    ASSERT_FALSE (value < std::numeric_limits <uint32_t>::max ());
    ASSERT_FALSE (value < std::numeric_limits <int64_t>::min ());
    ASSERT_FALSE (value < std::numeric_limits <int64_t>::max ());
    ASSERT_FALSE (value < std::numeric_limits <uint64_t>::min ());
    ASSERT_FALSE (value < std::numeric_limits <uint64_t>::max ());
    ASSERT_FALSE (value < std::numeric_limits <double>::min ());
    ASSERT_FALSE (value < std::numeric_limits <double>::max ());
    ASSERT_FALSE (value < "foo");
    ASSERT_TRUE  (value < Array ({1}));
    ASSERT_TRUE  (value < Object ({{"i", 1}}));

    value.set <Array, Array::value_type> ({1});
    ASSERT_FALSE (value < nullptr);
    ASSERT_FALSE (value < true);
    ASSERT_FALSE (value < false);
    ASSERT_FALSE (value < std::numeric_limits <int32_t>::min ());
    ASSERT_FALSE (value < std::numeric_limits <int32_t>::max ());
    ASSERT_FALSE (value < std::numeric_limits <uint32_t>::min ());
    ASSERT_FALSE (value < std::numeric_limits <uint32_t>::max ());
    ASSERT_FALSE (value < std::numeric_limits <int64_t>::min ());
    ASSERT_FALSE (value < std::numeric_limits <int64_t>::max ());
    ASSERT_FALSE (value < std::numeric_limits <uint64_t>::min ());
    ASSERT_FALSE (value < std::numeric_limits <uint64_t>::max ());
    ASSERT_FALSE (value < std::numeric_limits <double>::min ());
    ASSERT_FALSE (value < std::numeric_limits <double>::max ());
    ASSERT_FALSE (value < "foo");
    ASSERT_FALSE (value < Array ({1}));
    ASSERT_TRUE  (value < Object ({{"i", 1}}));

    value.set <Object, Object::value_type> ({{"i", 1}});
    ASSERT_FALSE (value < nullptr);
    ASSERT_FALSE (value < true);
    ASSERT_FALSE (value < false);
    ASSERT_FALSE (value < std::numeric_limits <int32_t>::min ());
    ASSERT_FALSE (value < std::numeric_limits <int32_t>::max ());
    ASSERT_FALSE (value < std::numeric_limits <uint32_t>::min ());
    ASSERT_FALSE (value < std::numeric_limits <uint32_t>::max ());
    ASSERT_FALSE (value < std::numeric_limits <int64_t>::min ());
    ASSERT_FALSE (value < std::numeric_limits <int64_t>::max ());
    ASSERT_FALSE (value < std::numeric_limits <uint64_t>::min ());
    ASSERT_FALSE (value < std::numeric_limits <uint64_t>::max ());
    ASSERT_FALSE (value < std::numeric_limits <double>::min ());
    ASSERT_FALSE (value < std::numeric_limits <double>::max ());
    ASSERT_FALSE (value < "foo");
    ASSERT_FALSE (value < Array ({1}));
    ASSERT_FALSE (value < Object ({{"i", 1}}));

    value.set <std::nullptr_t> (nullptr);
    ASSERT_FALSE (nullptr < value);
    ASSERT_FALSE (true < value);
    ASSERT_FALSE (false < value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::min () < value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::max () < value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::min () < value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::max () < value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::min () < value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::max () < value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::min () < value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::max () < value);
    ASSERT_FALSE (std::numeric_limits <double>::min () < value);
    ASSERT_FALSE (std::numeric_limits <double>::max () < value);
    ASSERT_FALSE ("foo" < value);
    ASSERT_FALSE (Array ({1}) < value);
    ASSERT_FALSE (Object ({{"i", 1}}) < value);

    value.set <bool> (true);
    ASSERT_TRUE  (nullptr < value);
    ASSERT_FALSE (true < value);
    ASSERT_TRUE  (false < value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::min () < value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::max () < value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::min () < value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::max () < value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::min () < value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::max () < value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::min () < value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::max () < value);
    ASSERT_FALSE (std::numeric_limits <double>::min () < value);
    ASSERT_FALSE (std::numeric_limits <double>::max () < value);
    ASSERT_FALSE ("foo" < value);
    ASSERT_FALSE (Array ({1}) < value);
    ASSERT_FALSE (Object ({{"i", 1}}) < value);

    value.set <int32_t> (std::numeric_limits <int32_t>::min ());
    ASSERT_TRUE  (nullptr < value);
    ASSERT_TRUE  (true < value);
    ASSERT_TRUE  (false < value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::min () < value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::max () < value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::min () < value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::max () < value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::min () < value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::max () < value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::min () < value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::max () < value);
    ASSERT_FALSE (std::numeric_limits <double>::min () < value);
    ASSERT_FALSE (std::numeric_limits <double>::max () < value);
    ASSERT_FALSE ("foo" < value);
    ASSERT_FALSE (Array ({1}) < value);
    ASSERT_FALSE (Object ({{"i", 1}}) < value);

    value.set <int64_t> (std::numeric_limits <int64_t>::max ());
    ASSERT_TRUE  (nullptr < value);
    ASSERT_TRUE  (true < value);
    ASSERT_TRUE  (false < value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::min () < value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::max () < value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::min () < value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::max () < value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::min () < value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::max () < value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::min () < value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::max () < value);
    ASSERT_TRUE  (std::numeric_limits <double>::min () < value);
    ASSERT_FALSE (std::numeric_limits <double>::max () < value);
    ASSERT_FALSE ("foo" < value);
    ASSERT_FALSE (Array ({1}) < value);
    ASSERT_FALSE (Object ({{"i", 1}}) < value);

    value.set <double> (std::numeric_limits <double>::min ());
    ASSERT_TRUE  (nullptr < value);
    ASSERT_TRUE  (true < value);
    ASSERT_TRUE  (false < value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::min () < value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::max () < value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::min () < value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::max () < value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::min () < value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::max () < value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::min () < value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::max () < value);
    ASSERT_FALSE (std::numeric_limits <double>::min () < value);
    ASSERT_FALSE (std::numeric_limits <double>::max () < value);
    ASSERT_FALSE ("foo" < value);
    ASSERT_FALSE (Array ({1}) < value);
    ASSERT_FALSE (Object ({{"i", 1}}) < value);

    value.set <std::string> ("foo");
    ASSERT_TRUE  (nullptr < value);
    ASSERT_TRUE  (true < value);
    ASSERT_TRUE  (false < value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::min () < value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::max () < value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::min () < value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::max () < value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::min () < value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::max () < value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::min () < value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::max () < value);
    ASSERT_TRUE  (std::numeric_limits <double>::min () < value);
    ASSERT_TRUE  (std::numeric_limits <double>::max () < value);
    ASSERT_FALSE ("foo" < value);
    ASSERT_FALSE (Array ({1}) < value);
    ASSERT_FALSE (Object ({{"i", 1}}) < value);

    value.set <Array, Array::value_type> ({1});
    ASSERT_TRUE  (nullptr < value);
    ASSERT_TRUE  (true < value);
    ASSERT_TRUE  (false < value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::min () < value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::max () < value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::min () < value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::max () < value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::min () < value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::max () < value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::min () < value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::max () < value);
    ASSERT_TRUE  (std::numeric_limits <double>::min () < value);
    ASSERT_TRUE  (std::numeric_limits <double>::max () < value);
    ASSERT_TRUE  ("foo" < value);
    ASSERT_FALSE (Array ({1}) < value);
    ASSERT_FALSE (Object ({{"i", 1}}) < value);

    value.set <Object, Object::value_type> ({{"i", 1}});
    ASSERT_TRUE  (nullptr < value);
    ASSERT_TRUE  (true < value);
    ASSERT_TRUE  (false < value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::min () < value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::max () < value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::min () < value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::max () < value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::min () < value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::max () < value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::min () < value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::max () < value);
    ASSERT_TRUE  (std::numeric_limits <double>::min () < value);
    ASSERT_TRUE  (std::numeric_limits <double>::max () < value);
    ASSERT_TRUE  ("foo" < value);
    ASSERT_TRUE  (Array ({1}) < value);
    ASSERT_FALSE (Object ({{"i", 1}}) < value);
}

/**
 * @brief Test greater operator.
 */
TEST (Value, greater)
{
    Value value;

    value.set <std::nullptr_t> (nullptr);
    ASSERT_FALSE (value > nullptr);
    ASSERT_FALSE (value > true);
    ASSERT_FALSE (value > false);
    ASSERT_FALSE (value > std::numeric_limits <int32_t>::min ());
    ASSERT_FALSE (value > std::numeric_limits <int32_t>::max ());
    ASSERT_FALSE (value > std::numeric_limits <uint32_t>::min ());
    ASSERT_FALSE (value > std::numeric_limits <uint32_t>::max ());
    ASSERT_FALSE (value > std::numeric_limits <int64_t>::min ());
    ASSERT_FALSE (value > std::numeric_limits <int64_t>::max ());
    ASSERT_FALSE (value > std::numeric_limits <uint64_t>::min ());
    ASSERT_FALSE (value > std::numeric_limits <uint64_t>::max ());
    ASSERT_FALSE (value > std::numeric_limits <double>::min ());
    ASSERT_FALSE (value > std::numeric_limits <double>::max ());
    ASSERT_FALSE (value > "foo");
    ASSERT_FALSE (value > Array ({1}));
    ASSERT_FALSE (value > Object ({{"i", 1}}));

    value.set <bool> (true);
    ASSERT_TRUE  (value > nullptr);
    ASSERT_FALSE (value > true);
    ASSERT_TRUE  (value > false);
    ASSERT_FALSE (value > std::numeric_limits <int32_t>::min ());
    ASSERT_FALSE (value > std::numeric_limits <int32_t>::max ());
    ASSERT_FALSE (value > std::numeric_limits <uint32_t>::min ());
    ASSERT_FALSE (value > std::numeric_limits <uint32_t>::max ());
    ASSERT_FALSE (value > std::numeric_limits <int64_t>::min ());
    ASSERT_FALSE (value > std::numeric_limits <int64_t>::max ());
    ASSERT_FALSE (value > std::numeric_limits <uint64_t>::min ());
    ASSERT_FALSE (value > std::numeric_limits <uint64_t>::max ());
    ASSERT_FALSE (value > std::numeric_limits <double>::min ());
    ASSERT_FALSE (value > std::numeric_limits <double>::max ());
    ASSERT_FALSE (value > "foo");
    ASSERT_FALSE (value > Array ({1}));
    ASSERT_FALSE (value > Object ({{"i", 1}}));

    value.set <int32_t> (std::numeric_limits <int32_t>::min ());
    ASSERT_TRUE  (value > nullptr);
    ASSERT_TRUE  (value > true);
    ASSERT_TRUE  (value > false);
    ASSERT_FALSE (value > std::numeric_limits <int32_t>::min ());
    ASSERT_FALSE (value > std::numeric_limits <int32_t>::max ());
    ASSERT_FALSE (value > std::numeric_limits <uint32_t>::min ());
    ASSERT_FALSE (value > std::numeric_limits <uint32_t>::max ());
    ASSERT_TRUE  (value > std::numeric_limits <int64_t>::min ());
    ASSERT_FALSE (value > std::numeric_limits <int64_t>::max ());
    ASSERT_FALSE (value > std::numeric_limits <uint64_t>::min ());
    ASSERT_FALSE (value > std::numeric_limits <uint64_t>::max ());
    ASSERT_FALSE (value > std::numeric_limits <double>::min ());
    ASSERT_FALSE (value > std::numeric_limits <double>::max ());
    ASSERT_FALSE (value > "foo");
    ASSERT_FALSE (value > Array ({1}));
    ASSERT_FALSE (value > Object ({{"i", 1}}));

    value.set <int64_t> (std::numeric_limits <int64_t>::max ());
    ASSERT_TRUE  (value > nullptr);
    ASSERT_TRUE  (value > true);
    ASSERT_TRUE  (value > false);
    ASSERT_TRUE  (value > std::numeric_limits <int32_t>::min ());
    ASSERT_TRUE  (value > std::numeric_limits <int32_t>::max ());
    ASSERT_TRUE  (value > std::numeric_limits <uint32_t>::min ());
    ASSERT_TRUE  (value > std::numeric_limits <uint32_t>::max ());
    ASSERT_TRUE  (value > std::numeric_limits <int64_t>::min ());
    ASSERT_FALSE (value > std::numeric_limits <int64_t>::max ());
    ASSERT_TRUE  (value > std::numeric_limits <uint64_t>::min ());
    ASSERT_FALSE (value > std::numeric_limits <uint64_t>::max ());
    ASSERT_TRUE  (value > std::numeric_limits <double>::min ());
    ASSERT_FALSE (value > std::numeric_limits <double>::max ());
    ASSERT_FALSE (value > "foo");
    ASSERT_FALSE (value > Array ({1}));
    ASSERT_FALSE (value > Object ({{"i", 1}}));

    value.set <double> (std::numeric_limits <double>::min ());
    ASSERT_TRUE  (value > nullptr);
    ASSERT_TRUE  (value > true);
    ASSERT_TRUE  (value > false);
    ASSERT_TRUE  (value > std::numeric_limits <int32_t>::min ());
    ASSERT_FALSE (value > std::numeric_limits <int32_t>::max ());
    ASSERT_TRUE  (value > std::numeric_limits <uint32_t>::min ());
    ASSERT_FALSE (value > std::numeric_limits <uint32_t>::max ());
    ASSERT_TRUE  (value > std::numeric_limits <int64_t>::min ());
    ASSERT_FALSE (value > std::numeric_limits <int64_t>::max ());
    ASSERT_TRUE  (value > std::numeric_limits <uint64_t>::min ());
    ASSERT_FALSE (value > std::numeric_limits <uint64_t>::max ());
    ASSERT_FALSE (value > std::numeric_limits <double>::min ());
    ASSERT_FALSE (value > std::numeric_limits <double>::max ());
    ASSERT_FALSE (value > "foo");
    ASSERT_FALSE (value > Array ({1}));
    ASSERT_FALSE (value > Object ({{"i", 1}}));

    value.set <std::string> ("foo");
    ASSERT_TRUE  (value > nullptr);
    ASSERT_TRUE  (value > true);
    ASSERT_TRUE  (value > false);
    ASSERT_TRUE  (value > std::numeric_limits <int32_t>::min ());
    ASSERT_TRUE  (value > std::numeric_limits <int32_t>::max ());
    ASSERT_TRUE  (value > std::numeric_limits <uint32_t>::min ());
    ASSERT_TRUE  (value > std::numeric_limits <uint32_t>::max ());
    ASSERT_TRUE  (value > std::numeric_limits <int64_t>::min ());
    ASSERT_TRUE  (value > std::numeric_limits <int64_t>::max ());
    ASSERT_TRUE  (value > std::numeric_limits <uint64_t>::min ());
    ASSERT_TRUE  (value > std::numeric_limits <uint64_t>::max ());
    ASSERT_TRUE  (value > std::numeric_limits <double>::min ());
    ASSERT_TRUE  (value > std::numeric_limits <double>::max ());
    ASSERT_FALSE (value > "foo");
    ASSERT_FALSE (value > Array ({1}));
    ASSERT_FALSE (value > Object ({{"i", 1}}));

    value.set <Array, Array::value_type> ({1});
    ASSERT_TRUE  (value > nullptr);
    ASSERT_TRUE  (value > true);
    ASSERT_TRUE  (value > false);
    ASSERT_TRUE  (value > std::numeric_limits <int32_t>::min ());
    ASSERT_TRUE  (value > std::numeric_limits <int32_t>::max ());
    ASSERT_TRUE  (value > std::numeric_limits <uint32_t>::min ());
    ASSERT_TRUE  (value > std::numeric_limits <uint32_t>::max ());
    ASSERT_TRUE  (value > std::numeric_limits <int64_t>::min ());
    ASSERT_TRUE  (value > std::numeric_limits <int64_t>::max ());
    ASSERT_TRUE  (value > std::numeric_limits <uint64_t>::min ());
    ASSERT_TRUE  (value > std::numeric_limits <uint64_t>::max ());
    ASSERT_TRUE  (value > std::numeric_limits <double>::min ());
    ASSERT_TRUE  (value > std::numeric_limits <double>::max ());
    ASSERT_TRUE  (value > "foo");
    ASSERT_FALSE (value > Array ({1}));
    ASSERT_FALSE (value > Object ({{"i", 1}}));

    value.set <Object, Object::value_type> ({{"i", 1}});
    ASSERT_TRUE  (value > nullptr);
    ASSERT_TRUE  (value > true);
    ASSERT_TRUE  (value > false);
    ASSERT_TRUE  (value > std::numeric_limits <int32_t>::min ());
    ASSERT_TRUE  (value > std::numeric_limits <int32_t>::max ());
    ASSERT_TRUE  (value > std::numeric_limits <uint32_t>::min ());
    ASSERT_TRUE  (value > std::numeric_limits <uint32_t>::max ());
    ASSERT_TRUE  (value > std::numeric_limits <int64_t>::min ());
    ASSERT_TRUE  (value > std::numeric_limits <int64_t>::max ());
    ASSERT_TRUE  (value > std::numeric_limits <uint64_t>::min ());
    ASSERT_TRUE  (value > std::numeric_limits <uint64_t>::max ());
    ASSERT_TRUE  (value > std::numeric_limits <double>::min ());
    ASSERT_TRUE  (value > std::numeric_limits <double>::max ());
    ASSERT_TRUE  (value > "foo");
    ASSERT_TRUE  (value > Array ({1}));
    ASSERT_FALSE (value > Object ({{"i", 1}}));

    value.set <std::nullptr_t> (nullptr);
    ASSERT_FALSE (nullptr > value);
    ASSERT_TRUE  (true > value);
    ASSERT_TRUE  (false > value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::min () > value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::max () > value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::min () > value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::max () > value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::min () > value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::max () > value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::min () > value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::max () > value);
    ASSERT_TRUE  (std::numeric_limits <double>::min () > value);
    ASSERT_TRUE  (std::numeric_limits <double>::max () > value);
    ASSERT_TRUE  ("foo" > value);
    ASSERT_TRUE  (Array ({1}) > value);
    ASSERT_TRUE  (Object ({{"i", 1}}) > value);

    value.set <bool> (true);
    ASSERT_FALSE (nullptr > value);
    ASSERT_FALSE (true > value);
    ASSERT_FALSE (false > value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::min () > value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::max () > value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::min () > value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::max () > value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::min () > value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::max () > value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::min () > value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::max () > value);
    ASSERT_TRUE  (std::numeric_limits <double>::min () > value);
    ASSERT_TRUE  (std::numeric_limits <double>::max () > value);
    ASSERT_TRUE  ("foo" > value);
    ASSERT_TRUE  (Array ({1}) > value);
    ASSERT_TRUE  (Object ({{"i", 1}}) > value);

    value.set <int32_t> (std::numeric_limits <int32_t>::min ());
    ASSERT_FALSE (nullptr > value);
    ASSERT_FALSE (true > value);
    ASSERT_FALSE (false > value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::min () > value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::max () > value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::min () > value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::max () > value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::min () > value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::max () > value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::min () > value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::max () > value);
    ASSERT_TRUE  (std::numeric_limits <double>::min () > value);
    ASSERT_TRUE  (std::numeric_limits <double>::max () > value);
    ASSERT_TRUE  ("foo" > value);
    ASSERT_TRUE  (Array ({1}) > value);
    ASSERT_TRUE  (Object ({{"i", 1}}) > value);

    value.set <int64_t> (std::numeric_limits <int64_t>::max ());
    ASSERT_FALSE (nullptr > value);
    ASSERT_FALSE (true > value);
    ASSERT_FALSE (false > value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::min () > value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::max () > value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::min () > value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::max () > value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::min () > value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::max () > value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::min () > value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::max () > value);
    ASSERT_FALSE (std::numeric_limits <double>::min () > value);
    ASSERT_TRUE  (std::numeric_limits <double>::max () > value);
    ASSERT_TRUE  ("foo" > value);
    ASSERT_TRUE  (Array ({1}) > value);
    ASSERT_TRUE  (Object ({{"i", 1}}) > value);

    value.set <double> (std::numeric_limits <double>::min ());
    ASSERT_FALSE (nullptr > value);
    ASSERT_FALSE (true > value);
    ASSERT_FALSE (false > value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::min () > value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::max () > value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::min () > value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::max () > value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::min () > value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::max () > value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::min () > value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::max () > value);
    ASSERT_FALSE (std::numeric_limits <double>::min () > value);
    ASSERT_TRUE  (std::numeric_limits <double>::max () > value);
    ASSERT_TRUE  ("foo" > value);
    ASSERT_TRUE  (Array ({1}) > value);
    ASSERT_TRUE  (Object ({{"i", 1}}) > value);

    value.set <std::string> ("foo");
    ASSERT_FALSE (nullptr > value);
    ASSERT_FALSE (true > value);
    ASSERT_FALSE (false > value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::min () > value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::max () > value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::min () > value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::max () > value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::min () > value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::max () > value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::min () > value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::max () > value);
    ASSERT_FALSE (std::numeric_limits <double>::min () > value);
    ASSERT_FALSE (std::numeric_limits <double>::max () > value);
    ASSERT_FALSE ("foo" > value);
    ASSERT_TRUE  (Array ({1}) > value);
    ASSERT_TRUE  (Object ({{"i", 1}}) > value);

    value.set <Array, Array::value_type> ({1});
    ASSERT_FALSE (nullptr > value);
    ASSERT_FALSE (true > value);
    ASSERT_FALSE (false > value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::min () > value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::max () > value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::min () > value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::max () > value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::min () > value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::max () > value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::min () > value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::max () > value);
    ASSERT_FALSE (std::numeric_limits <double>::min () > value);
    ASSERT_FALSE (std::numeric_limits <double>::max () > value);
    ASSERT_FALSE ("foo" > value);
    ASSERT_FALSE (Array ({1}) > value);
    ASSERT_TRUE  (Object ({{"i", 1}}) > value);

    value.set <Object, Object::value_type> ({{"i", 1}});
    ASSERT_FALSE (nullptr > value);
    ASSERT_FALSE (true > value);
    ASSERT_FALSE (false > value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::min () > value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::max () > value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::min () > value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::max () > value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::min () > value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::max () > value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::min () > value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::max () > value);
    ASSERT_FALSE (std::numeric_limits <double>::min () > value);
    ASSERT_FALSE (std::numeric_limits <double>::max () > value);
    ASSERT_FALSE ("foo" > value);
    ASSERT_FALSE (Array ({1}) > value);
    ASSERT_FALSE (Object ({{"i", 1}}) > value);
}

/**
 * @brief Test lowerOrEqual operator.
 */
TEST (Value, lowerOrEqual)
{
    Value value;

    value.set <std::nullptr_t> (nullptr);
    ASSERT_TRUE  (value <= nullptr);
    ASSERT_TRUE  (value <= true);
    ASSERT_TRUE  (value <= false);
    ASSERT_TRUE  (value <= std::numeric_limits <int32_t>::min ());
    ASSERT_TRUE  (value <= std::numeric_limits <int32_t>::max ());
    ASSERT_TRUE  (value <= std::numeric_limits <uint32_t>::min ());
    ASSERT_TRUE  (value <= std::numeric_limits <uint32_t>::max ());
    ASSERT_TRUE  (value <= std::numeric_limits <int64_t>::min ());
    ASSERT_TRUE  (value <= std::numeric_limits <int64_t>::max ());
    ASSERT_TRUE  (value <= std::numeric_limits <uint64_t>::min ());
    ASSERT_TRUE  (value <= std::numeric_limits <uint64_t>::max ());
    ASSERT_TRUE  (value <= std::numeric_limits <double>::min ());
    ASSERT_TRUE  (value <= std::numeric_limits <double>::max ());
    ASSERT_TRUE  (value <= "foo");
    ASSERT_TRUE  (value <= Array ({1}));
    ASSERT_TRUE  (value <= Object ({{"i", 1}}));

    value.set <bool> (true);
    ASSERT_FALSE (value <= nullptr);
    ASSERT_TRUE  (value <= true);
    ASSERT_FALSE (value <= false);
    ASSERT_TRUE  (value <= std::numeric_limits <int32_t>::min ());
    ASSERT_TRUE  (value <= std::numeric_limits <int32_t>::max ());
    ASSERT_TRUE  (value <= std::numeric_limits <uint32_t>::min ());
    ASSERT_TRUE  (value <= std::numeric_limits <uint32_t>::max ());
    ASSERT_TRUE  (value <= std::numeric_limits <int64_t>::min ());
    ASSERT_TRUE  (value <= std::numeric_limits <int64_t>::max ());
    ASSERT_TRUE  (value <= std::numeric_limits <uint64_t>::min ());
    ASSERT_TRUE  (value <= std::numeric_limits <uint64_t>::max ());
    ASSERT_TRUE  (value <= std::numeric_limits <double>::min ());
    ASSERT_TRUE  (value <= std::numeric_limits <double>::max ());
    ASSERT_TRUE  (value <= "foo");
    ASSERT_TRUE  (value <= Array ({1}));
    ASSERT_TRUE  (value <= Object ({{"i", 1}}));

    value.set <int32_t> (std::numeric_limits <int32_t>::min ());
    ASSERT_FALSE (value <= nullptr);
    ASSERT_FALSE (value <= true);
    ASSERT_FALSE (value <= false);
    ASSERT_TRUE  (value <= std::numeric_limits <int32_t>::min ());
    ASSERT_TRUE  (value <= std::numeric_limits <int32_t>::max ());
    ASSERT_TRUE  (value <= std::numeric_limits <uint32_t>::min ());
    ASSERT_TRUE  (value <= std::numeric_limits <uint32_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits <int64_t>::min ());
    ASSERT_TRUE  (value <= std::numeric_limits <int64_t>::max ());
    ASSERT_TRUE  (value <= std::numeric_limits <uint64_t>::min ());
    ASSERT_TRUE  (value <= std::numeric_limits <uint64_t>::max ());
    ASSERT_TRUE  (value <= std::numeric_limits <double>::min ());
    ASSERT_TRUE  (value <= std::numeric_limits <double>::max ());
    ASSERT_TRUE  (value <= "foo");
    ASSERT_TRUE  (value <= Array ({1}));
    ASSERT_TRUE  (value <= Object ({{"i", 1}}));

    value.set <uint32_t> (std::numeric_limits <uint32_t>::max ());
    ASSERT_FALSE (value <= nullptr);
    ASSERT_FALSE (value <= true);
    ASSERT_FALSE (value <= false);
    ASSERT_FALSE (value <= std::numeric_limits <int32_t>::min ());
    ASSERT_FALSE (value <= std::numeric_limits <int32_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits <uint32_t>::min ());
    ASSERT_TRUE  (value <= std::numeric_limits <uint32_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits <int64_t>::min ());
    ASSERT_TRUE  (value <= std::numeric_limits <int64_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits <uint64_t>::min ());
    ASSERT_TRUE  (value <= std::numeric_limits <uint64_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits <double>::min ());
    ASSERT_TRUE  (value <= std::numeric_limits <double>::max ());
    ASSERT_TRUE  (value <= "foo");
    ASSERT_TRUE  (value <= Array ({1}));
    ASSERT_TRUE  (value <= Object ({{"i", 1}}));

    value.set <int64_t> (std::numeric_limits <int64_t>::min ());
    ASSERT_FALSE (value <= nullptr);
    ASSERT_FALSE (value <= true);
    ASSERT_FALSE (value <= false);
    ASSERT_TRUE  (value <= std::numeric_limits <int32_t>::min ());
    ASSERT_TRUE  (value <= std::numeric_limits <int32_t>::max ());
    ASSERT_TRUE  (value <= std::numeric_limits <uint32_t>::min ());
    ASSERT_TRUE  (value <= std::numeric_limits <uint32_t>::max ());
    ASSERT_TRUE  (value <= std::numeric_limits <int64_t>::min ());
    ASSERT_TRUE  (value <= std::numeric_limits <int64_t>::max ());
    ASSERT_TRUE  (value <= std::numeric_limits <uint64_t>::min ());
    ASSERT_TRUE  (value <= std::numeric_limits <uint64_t>::max ());
    ASSERT_TRUE  (value <= std::numeric_limits <double>::min ());
    ASSERT_TRUE  (value <= std::numeric_limits <double>::max ());
    ASSERT_TRUE  (value <= "foo");
    ASSERT_TRUE  (value <= Array ({1}));
    ASSERT_TRUE  (value <= Object ({{"i", 1}}));

    value.set <uint64_t> (std::numeric_limits <uint64_t>::max ());
    ASSERT_FALSE (value <= nullptr);
    ASSERT_FALSE (value <= true);
    ASSERT_FALSE (value <= false);
    ASSERT_FALSE (value <= std::numeric_limits <int32_t>::min ());
    ASSERT_FALSE (value <= std::numeric_limits <int32_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits <uint32_t>::min ());
    ASSERT_FALSE (value <= std::numeric_limits <uint32_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits <int64_t>::min ());
    ASSERT_FALSE (value <= std::numeric_limits <int64_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits <uint64_t>::min ());
    ASSERT_TRUE  (value <= std::numeric_limits <uint64_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits <double>::min ());
    ASSERT_TRUE  (value <= std::numeric_limits <double>::max ());
    ASSERT_TRUE  (value <= "foo");
    ASSERT_TRUE  (value <= Array ({1}));
    ASSERT_TRUE  (value <= Object ({{"i", 1}}));

    value.set <double> (std::numeric_limits <double>::min ());
    ASSERT_FALSE (value <= nullptr);
    ASSERT_FALSE (value <= true);
    ASSERT_FALSE (value <= false);
    ASSERT_FALSE (value <= std::numeric_limits <int32_t>::min ());
    ASSERT_TRUE  (value <= std::numeric_limits <int32_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits <uint32_t>::min ());
    ASSERT_TRUE  (value <= std::numeric_limits <uint32_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits <int64_t>::min ());
    ASSERT_TRUE  (value <= std::numeric_limits <int64_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits <uint64_t>::min ());
    ASSERT_TRUE  (value <= std::numeric_limits <uint64_t>::max ());
    ASSERT_TRUE  (value <= std::numeric_limits <double>::min ());
    ASSERT_TRUE  (value <= std::numeric_limits <double>::max ());
    ASSERT_TRUE  (value <= "foo");
    ASSERT_TRUE  (value <= Array ({1}));
    ASSERT_TRUE  (value <= Object ({{"i", 1}}));

    value.set <std::string> ("foo");
    ASSERT_FALSE (value <= nullptr);
    ASSERT_FALSE (value <= true);
    ASSERT_FALSE (value <= false);
    ASSERT_FALSE (value <= std::numeric_limits <int32_t>::min ());
    ASSERT_FALSE (value <= std::numeric_limits <int32_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits <uint32_t>::min ());
    ASSERT_FALSE (value <= std::numeric_limits <uint32_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits <int64_t>::min ());
    ASSERT_FALSE (value <= std::numeric_limits <int64_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits <uint64_t>::min ());
    ASSERT_FALSE (value <= std::numeric_limits <uint64_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits <double>::min ());
    ASSERT_FALSE (value <= std::numeric_limits <double>::max ());
    ASSERT_TRUE  (value <= "foo");
    ASSERT_TRUE  (value <= Array ({1}));
    ASSERT_TRUE  (value <= Object ({{"i", 1}}));

    value.set <Array, Array::value_type> ({1});
    ASSERT_FALSE (value <= nullptr);
    ASSERT_FALSE (value <= true);
    ASSERT_FALSE (value <= false);
    ASSERT_FALSE (value <= std::numeric_limits <int32_t>::min ());
    ASSERT_FALSE (value <= std::numeric_limits <int32_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits <uint32_t>::min ());
    ASSERT_FALSE (value <= std::numeric_limits <uint32_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits <int64_t>::min ());
    ASSERT_FALSE (value <= std::numeric_limits <int64_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits <uint64_t>::min ());
    ASSERT_FALSE (value <= std::numeric_limits <uint64_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits <double>::min ());
    ASSERT_FALSE (value <= std::numeric_limits <double>::max ());
    ASSERT_FALSE (value <= "foo");
    ASSERT_TRUE  (value <= Array ({1}));
    ASSERT_TRUE  (value <= Object ({{"i", 1}}));

    value.set <Object, Object::value_type> ({{"i", 1}});
    ASSERT_FALSE (value <= nullptr);
    ASSERT_FALSE (value <= true);
    ASSERT_FALSE (value <= false);
    ASSERT_FALSE (value <= std::numeric_limits <int32_t>::min ());
    ASSERT_FALSE (value <= std::numeric_limits <int32_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits <uint32_t>::min ());
    ASSERT_FALSE (value <= std::numeric_limits <uint32_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits <int64_t>::min ());
    ASSERT_FALSE (value <= std::numeric_limits <int64_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits <uint64_t>::min ());
    ASSERT_FALSE (value <= std::numeric_limits <uint64_t>::max ());
    ASSERT_FALSE (value <= std::numeric_limits <double>::min ());
    ASSERT_FALSE (value <= std::numeric_limits <double>::max ());
    ASSERT_FALSE (value <= "foo");
    ASSERT_FALSE (value <= Array ({1}));
    ASSERT_TRUE  (value <= Object ({{"i", 1}}));

    value.set <std::nullptr_t> (nullptr);
    ASSERT_TRUE  (nullptr <= value);
    ASSERT_FALSE (true <= value);
    ASSERT_FALSE (false <= value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::max () <= value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::max () <= value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::max () <= value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::max () <= value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::max () <= value);
    ASSERT_FALSE (std::numeric_limits <double>::min () <= value);
    ASSERT_FALSE (std::numeric_limits <double>::max () <= value);
    ASSERT_FALSE ("foo" <= value);
    ASSERT_FALSE (Array ({1}) <= value);
    ASSERT_FALSE (Object ({{"i", 1}}) <= value);

    value.set <bool> (true);
    ASSERT_TRUE  (nullptr <= value);
    ASSERT_TRUE  (true <= value);
    ASSERT_TRUE  (false <= value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::max () <= value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::max () <= value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::max () <= value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::max () <= value);
    ASSERT_FALSE (std::numeric_limits <double>::min () <= value);
    ASSERT_FALSE (std::numeric_limits <double>::max () <= value);
    ASSERT_FALSE ("foo" <= value);
    ASSERT_FALSE (Array ({1}) <= value);
    ASSERT_FALSE (Object ({{"i", 1}}) <= value);

    value.set <int32_t> (std::numeric_limits <int32_t>::min ());
    ASSERT_TRUE  (nullptr <= value);
    ASSERT_TRUE  (true <= value);
    ASSERT_TRUE  (false <= value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::max () <= value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::max () <= value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::max () <= value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::max () <= value);
    ASSERT_FALSE (std::numeric_limits <double>::min () <= value);
    ASSERT_FALSE (std::numeric_limits <double>::max () <= value);
    ASSERT_FALSE ("foo" <= value);
    ASSERT_FALSE (Array ({1}) <= value);
    ASSERT_FALSE (Object ({{"i", 1}}) <= value);

    value.set <uint32_t> (std::numeric_limits <uint32_t>::max ());
    ASSERT_TRUE (nullptr <= value);
    ASSERT_TRUE (true <= value);
    ASSERT_TRUE (false <= value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::min () <= value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::max () <= value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::min () <= value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::max () <= value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::max () <= value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::max () <= value);
    ASSERT_TRUE  (std::numeric_limits <double>::min () <= value);
    ASSERT_FALSE (std::numeric_limits <double>::max () <= value);
    ASSERT_FALSE ("foo" <= value);
    ASSERT_FALSE (Array ({1}) <= value);
    ASSERT_FALSE (Object ({{"i", 1}}) <= value);

    value.set <int64_t> (std::numeric_limits <int64_t>::min ());
    ASSERT_TRUE  (nullptr <= value);
    ASSERT_TRUE  (true <= value);
    ASSERT_TRUE  (false <= value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::max () <= value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::max () <= value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::max () <= value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::max () <= value);
    ASSERT_FALSE (std::numeric_limits <double>::min () <= value);
    ASSERT_FALSE (std::numeric_limits <double>::max () <= value);
    ASSERT_FALSE ("foo" <= value);
    ASSERT_FALSE (Array ({1}) <= value);
    ASSERT_FALSE (Object ({{"i", 1}}) <= value);

    value.set <uint64_t> (std::numeric_limits <uint64_t>::max ());
    ASSERT_TRUE  (nullptr <= value);
    ASSERT_TRUE  (true <= value);
    ASSERT_TRUE  (false <= value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::min () <= value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::max () <= value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::min () <= value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::max () <= value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::min () <= value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::max () <= value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::min () <= value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::max () <= value);
    ASSERT_TRUE  (std::numeric_limits <double>::min () <= value);
    ASSERT_FALSE (std::numeric_limits <double>::max () <= value);
    ASSERT_FALSE ("foo" <= value);
    ASSERT_FALSE (Array ({1}) <= value);
    ASSERT_FALSE (Object ({{"i", 1}}) <= value);

    value.set <double> (std::numeric_limits <double>::min ());
    ASSERT_TRUE  (nullptr <= value);
    ASSERT_TRUE  (true <= value);
    ASSERT_TRUE  (false <= value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::max () <= value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::max () <= value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::max () <= value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::min () <= value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::max () <= value);
    ASSERT_TRUE  (std::numeric_limits <double>::min () <= value);
    ASSERT_FALSE (std::numeric_limits <double>::max () <= value);
    ASSERT_FALSE ("foo" <= value);
    ASSERT_FALSE (Array ({1}) <= value);
    ASSERT_FALSE (Object ({{"i", 1}}) <= value);

    value.set <std::string> ("foo");
    ASSERT_TRUE  (nullptr <= value);
    ASSERT_TRUE  (true <= value);
    ASSERT_TRUE  (false <= value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::min () <= value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::max () <= value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::min () <= value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::max () <= value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::min () <= value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::max () <= value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::min () <= value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::max () <= value);
    ASSERT_TRUE  (std::numeric_limits <double>::min () <= value);
    ASSERT_TRUE  (std::numeric_limits <double>::max () <= value);
    ASSERT_TRUE  ("foo" <= value);
    ASSERT_FALSE (Array ({1}) <= value);
    ASSERT_FALSE (Object ({{"i", 1}}) <= value);

    value.set <Array, Array::value_type> ({1});
    ASSERT_TRUE  (nullptr <= value);
    ASSERT_TRUE  (true <= value);
    ASSERT_TRUE  (false <= value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::min () <= value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::max () <= value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::min () <= value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::max () <= value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::min () <= value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::max () <= value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::min () <= value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::max () <= value);
    ASSERT_TRUE  (std::numeric_limits <double>::min () <= value);
    ASSERT_TRUE  (std::numeric_limits <double>::max () <= value);
    ASSERT_TRUE  ("foo" <= value);
    ASSERT_TRUE  (Array ({1}) <= value);
    ASSERT_FALSE (Object ({{"i", 1}}) <= value);

    value.set <Object, Object::value_type> ({{"i", 1}});
    ASSERT_TRUE  (nullptr <= value);
    ASSERT_TRUE  (true <= value);
    ASSERT_TRUE  (false <= value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::min () <= value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::max () <= value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::min () <= value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::max () <= value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::min () <= value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::max () <= value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::min () <= value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::max () <= value);
    ASSERT_TRUE  (std::numeric_limits <double>::min () <= value);
    ASSERT_TRUE  (std::numeric_limits <double>::max () <= value);
    ASSERT_TRUE  ("foo" <= value);
    ASSERT_TRUE  (Array ({1}) <= value);
    ASSERT_TRUE  (Object ({{"i", 1}}) <= value);
}

/**
 * @brief Test greaterOrEqual operator.
 */
TEST (Value, greaterOrEqual)
{
    Value value;

    value.set <std::nullptr_t> (nullptr);
    ASSERT_TRUE  (value >= nullptr);
    ASSERT_FALSE (value >= true);
    ASSERT_FALSE (value >= false);
    ASSERT_FALSE (value >= std::numeric_limits <int32_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits <int32_t>::max ());
    ASSERT_FALSE (value >= std::numeric_limits <uint32_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits <uint32_t>::max ());
    ASSERT_FALSE (value >= std::numeric_limits <int64_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits <int64_t>::max ());
    ASSERT_FALSE (value >= std::numeric_limits <uint64_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits <uint64_t>::max ());
    ASSERT_FALSE (value >= std::numeric_limits <double>::min ());
    ASSERT_FALSE (value >= std::numeric_limits <double>::max ());
    ASSERT_FALSE (value >= "foo");
    ASSERT_FALSE (value >= Array ({1}));
    ASSERT_FALSE (value >= Object ({{"i", 1}}));

    value.set <bool> (true);
    ASSERT_TRUE  (value >= nullptr);
    ASSERT_TRUE  (value >= true);
    ASSERT_TRUE  (value >= false);
    ASSERT_FALSE (value >= std::numeric_limits <int32_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits <int32_t>::max ());
    ASSERT_FALSE (value >= std::numeric_limits <uint32_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits <uint32_t>::max ());
    ASSERT_FALSE (value >= std::numeric_limits <int64_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits <int64_t>::max ());
    ASSERT_FALSE (value >= std::numeric_limits <uint64_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits <uint64_t>::max ());
    ASSERT_FALSE (value >= std::numeric_limits <double>::min ());
    ASSERT_FALSE (value >= std::numeric_limits <double>::max ());
    ASSERT_FALSE (value >= "foo");
    ASSERT_FALSE (value >= Array ({1}));
    ASSERT_FALSE (value >= Object ({{"i", 1}}));

    value.set <int32_t> (std::numeric_limits <int32_t>::min ());
    ASSERT_TRUE  (value >= nullptr);
    ASSERT_TRUE  (value >= true);
    ASSERT_TRUE  (value >= false);
    ASSERT_TRUE  (value >= std::numeric_limits <int32_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits <int32_t>::max ());
    ASSERT_FALSE (value >= std::numeric_limits <uint32_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits <uint32_t>::max ());
    ASSERT_TRUE  (value >= std::numeric_limits <int64_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits <int64_t>::max ());
    ASSERT_FALSE (value >= std::numeric_limits <uint64_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits <uint64_t>::max ());
    ASSERT_FALSE (value >= std::numeric_limits <double>::min ());
    ASSERT_FALSE (value >= std::numeric_limits <double>::max ());
    ASSERT_FALSE (value >= "foo");
    ASSERT_FALSE (value >= Array ({1}));
    ASSERT_FALSE (value >= Object ({{"i", 1}}));

    value.set <uint32_t> (std::numeric_limits <uint32_t>::max ());
    ASSERT_TRUE  (value >= nullptr);
    ASSERT_TRUE  (value >= true);
    ASSERT_TRUE  (value >= false);
    ASSERT_TRUE  (value >= std::numeric_limits <int32_t>::min ());
    ASSERT_TRUE  (value >= std::numeric_limits <int32_t>::max ());
    ASSERT_TRUE  (value >= std::numeric_limits <uint32_t>::min ());
    ASSERT_TRUE  (value >= std::numeric_limits <uint32_t>::max ());
    ASSERT_TRUE  (value >= std::numeric_limits <int64_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits <int64_t>::max ());
    ASSERT_TRUE  (value >= std::numeric_limits <uint64_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits <uint64_t>::max ());
    ASSERT_TRUE  (value >= std::numeric_limits <double>::min ());
    ASSERT_FALSE (value >= std::numeric_limits <double>::max ());
    ASSERT_FALSE (value >= "foo");
    ASSERT_FALSE (value >= Array ({1}));
    ASSERT_FALSE (value >= Object ({{"i", 1}}));

    value.set <int64_t> (std::numeric_limits <int64_t>::min ());
    ASSERT_TRUE  (value >= nullptr);
    ASSERT_TRUE  (value >= true);
    ASSERT_TRUE  (value >= false);
    ASSERT_FALSE (value >= std::numeric_limits <int32_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits <int32_t>::max ());
    ASSERT_FALSE (value >= std::numeric_limits <uint32_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits <uint32_t>::max ());
    ASSERT_TRUE  (value >= std::numeric_limits <int64_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits <int64_t>::max ());
    ASSERT_FALSE (value >= std::numeric_limits <uint64_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits <uint64_t>::max ());
    ASSERT_FALSE (value >= std::numeric_limits <double>::min ());
    ASSERT_FALSE (value >= std::numeric_limits <double>::max ());
    ASSERT_FALSE (value >= "foo");
    ASSERT_FALSE (value >= Array ({1}));
    ASSERT_FALSE (value >= Object ({{"i", 1}}));

    value.set <uint64_t> (std::numeric_limits <uint64_t>::max ());
    ASSERT_TRUE  (value >= nullptr);
    ASSERT_TRUE  (value >= true);
    ASSERT_TRUE  (value >= false);
    ASSERT_TRUE  (value >= std::numeric_limits <int32_t>::min ());
    ASSERT_TRUE  (value >= std::numeric_limits <int32_t>::max ());
    ASSERT_TRUE  (value >= std::numeric_limits <uint32_t>::min ());
    ASSERT_TRUE  (value >= std::numeric_limits <uint32_t>::max ());
    ASSERT_TRUE  (value >= std::numeric_limits <int64_t>::min ());
    ASSERT_TRUE  (value >= std::numeric_limits <int64_t>::max ());
    ASSERT_TRUE  (value >= std::numeric_limits <uint64_t>::min ());
    ASSERT_TRUE  (value >= std::numeric_limits <uint64_t>::max ());
    ASSERT_TRUE  (value >= std::numeric_limits <double>::min ());
    ASSERT_FALSE (value >= std::numeric_limits <double>::max ());
    ASSERT_FALSE (value >= "foo");
    ASSERT_FALSE (value >= Array ({1}));
    ASSERT_FALSE (value >= Object ({{"i", 1}}));

    value.set <double> (std::numeric_limits <double>::min ());
    ASSERT_TRUE  (value >= nullptr);
    ASSERT_TRUE  (value >= true);
    ASSERT_TRUE  (value >= false);
    ASSERT_TRUE  (value >= std::numeric_limits <int32_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits <int32_t>::max ());
    ASSERT_TRUE  (value >= std::numeric_limits <uint32_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits <uint32_t>::max ());
    ASSERT_TRUE  (value >= std::numeric_limits <int64_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits <int64_t>::max ());
    ASSERT_TRUE  (value >= std::numeric_limits <uint64_t>::min ());
    ASSERT_FALSE (value >= std::numeric_limits <uint64_t>::max ());
    ASSERT_TRUE  (value >= std::numeric_limits <double>::min ());
    ASSERT_FALSE (value >= std::numeric_limits <double>::max ());
    ASSERT_FALSE (value >= "foo");
    ASSERT_FALSE (value >= Array ({1}));
    ASSERT_FALSE (value >= Object ({{"i", 1}}));

    value.set <std::string> ("foo");
    ASSERT_TRUE  (value >= nullptr);
    ASSERT_TRUE  (value >= true);
    ASSERT_TRUE  (value >= false);
    ASSERT_TRUE  (value >= std::numeric_limits <int32_t>::min ());
    ASSERT_TRUE  (value >= std::numeric_limits <int32_t>::max ());
    ASSERT_TRUE  (value >= std::numeric_limits <uint32_t>::min ());
    ASSERT_TRUE  (value >= std::numeric_limits <uint32_t>::max ());
    ASSERT_TRUE  (value >= std::numeric_limits <int64_t>::min ());
    ASSERT_TRUE  (value >= std::numeric_limits <int64_t>::max ());
    ASSERT_TRUE  (value >= std::numeric_limits <uint64_t>::min ());
    ASSERT_TRUE  (value >= std::numeric_limits <uint64_t>::max ());
    ASSERT_TRUE  (value >= std::numeric_limits <double>::min ());
    ASSERT_TRUE  (value >= std::numeric_limits <double>::max ());
    ASSERT_TRUE  (value >= "foo");
    ASSERT_FALSE (value >= Array ({1}));
    ASSERT_FALSE (value >= Object ({{"i", 1}}));

    value.set <Array, Array::value_type> ({1});
    ASSERT_TRUE  (value >= nullptr);
    ASSERT_TRUE  (value >= true);
    ASSERT_TRUE  (value >= false);
    ASSERT_TRUE  (value >= std::numeric_limits <int32_t>::min ());
    ASSERT_TRUE  (value >= std::numeric_limits <int32_t>::max ());
    ASSERT_TRUE  (value >= std::numeric_limits <uint32_t>::min ());
    ASSERT_TRUE  (value >= std::numeric_limits <uint32_t>::max ());
    ASSERT_TRUE  (value >= std::numeric_limits <int64_t>::min ());
    ASSERT_TRUE  (value >= std::numeric_limits <int64_t>::max ());
    ASSERT_TRUE  (value >= std::numeric_limits <uint64_t>::min ());
    ASSERT_TRUE  (value >= std::numeric_limits <uint64_t>::max ());
    ASSERT_TRUE  (value >= std::numeric_limits <double>::min ());
    ASSERT_TRUE  (value >= std::numeric_limits <double>::max ());
    ASSERT_TRUE  (value >= "foo");
    ASSERT_TRUE  (value >= Array ({1}));
    ASSERT_FALSE (value >= Object ({{"i", 1}}));

    value.set <Object, Object::value_type> ({{"i", 1}});
    ASSERT_TRUE  (value >= nullptr);
    ASSERT_TRUE  (value >= true);
    ASSERT_TRUE  (value >= false);
    ASSERT_TRUE  (value >= std::numeric_limits <int32_t>::min ());
    ASSERT_TRUE  (value >= std::numeric_limits <int32_t>::max ());
    ASSERT_TRUE  (value >= std::numeric_limits <uint32_t>::min ());
    ASSERT_TRUE  (value >= std::numeric_limits <uint32_t>::max ());
    ASSERT_TRUE  (value >= std::numeric_limits <int64_t>::min ());
    ASSERT_TRUE  (value >= std::numeric_limits <int64_t>::max ());
    ASSERT_TRUE  (value >= std::numeric_limits <uint64_t>::min ());
    ASSERT_TRUE  (value >= std::numeric_limits <uint64_t>::max ());
    ASSERT_TRUE  (value >= std::numeric_limits <double>::min ());
    ASSERT_TRUE  (value >= std::numeric_limits <double>::max ());
    ASSERT_TRUE  (value >= "foo");
    ASSERT_TRUE  (value >= Array ({1}));
    ASSERT_TRUE  (value >= Object ({{"i", 1}}));

    value.set <std::nullptr_t> (nullptr);
    ASSERT_TRUE  (nullptr >= value);
    ASSERT_TRUE  (true >= value);
    ASSERT_TRUE  (false >= value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::min () >= value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::max () >= value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::min () >= value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::max () >= value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::min () >= value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::max () >= value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::min () >= value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::max () >= value);
    ASSERT_TRUE  (std::numeric_limits <double>::min () >= value);
    ASSERT_TRUE  (std::numeric_limits <double>::max () >= value);
    ASSERT_TRUE  ("foo" >= value);
    ASSERT_TRUE  (Array ({1}) >= value);
    ASSERT_TRUE  (Object ({{"i", 1}}) >= value);

    value.set <bool> (true);
    ASSERT_FALSE (nullptr >= value);
    ASSERT_TRUE  (true >= value);
    ASSERT_FALSE (false >= value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::min () >= value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::max () >= value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::min () >= value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::max () >= value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::min () >= value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::max () >= value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::min () >= value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::max () >= value);
    ASSERT_TRUE  (std::numeric_limits <double>::min () >= value);
    ASSERT_TRUE  (std::numeric_limits <double>::max () >= value);
    ASSERT_TRUE  ("foo" >= value);
    ASSERT_TRUE  (Array ({1}) >= value);
    ASSERT_TRUE  (Object ({{"i", 1}}) >= value);

    value.set <int32_t> (std::numeric_limits <int32_t>::min ());
    ASSERT_FALSE (nullptr >= value);
    ASSERT_FALSE (true >= value);
    ASSERT_FALSE (false >= value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::min () >= value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::max () >= value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::min () >= value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::min () >= value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::max () >= value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::min () >= value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::max () >= value);
    ASSERT_TRUE  (std::numeric_limits <double>::min () >= value);
    ASSERT_TRUE  (std::numeric_limits <double>::max () >= value);
    ASSERT_TRUE  ("foo" >= value);
    ASSERT_TRUE  (Array ({1}) >= value);
    ASSERT_TRUE  (Object ({{"i", 1}}) >= value);

    value.set <uint32_t> (std::numeric_limits <uint32_t>::max ());
    ASSERT_FALSE (nullptr >= value);
    ASSERT_FALSE (true >= value);
    ASSERT_FALSE (false >= value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::min () >= value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::min () >= value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::min () >= value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::min () >= value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits <double>::min () >= value);
    ASSERT_TRUE  (std::numeric_limits <double>::max () >= value);
    ASSERT_TRUE  ("foo" >= value);
    ASSERT_TRUE  (Array ({1}) >= value);
    ASSERT_TRUE  (Object ({{"i", 1}}) >= value);

    value.set <int64_t> (std::numeric_limits <int64_t>::min ());
    ASSERT_FALSE (nullptr >= value);
    ASSERT_FALSE (true >= value);
    ASSERT_FALSE (false >= value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::min () >= value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::max () >= value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::min () >= value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::max () >= value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::min () >= value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::max () >= value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::min () >= value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::max () >= value);
    ASSERT_TRUE  (std::numeric_limits <double>::min () >= value);
    ASSERT_TRUE  (std::numeric_limits <double>::max () >= value);
    ASSERT_TRUE  ("foo" >= value);
    ASSERT_TRUE  (Array ({1}) >= value);
    ASSERT_TRUE  (Object ({{"i", 1}}) >= value);

    value.set <uint64_t> (std::numeric_limits <uint64_t>::max ());
    ASSERT_FALSE (nullptr >= value);
    ASSERT_FALSE (true >= value);
    ASSERT_FALSE (false >= value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::min () >= value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::min () >= value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::min () >= value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::min () >= value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits <double>::min () >= value);
    ASSERT_TRUE  (std::numeric_limits <double>::max () >= value);
    ASSERT_TRUE  ("foo" >= value);
    ASSERT_TRUE  (Array ({1}) >= value);
    ASSERT_TRUE  (Object ({{"i", 1}}) >= value);

    value.set <double> (std::numeric_limits <double>::min ());
    ASSERT_FALSE (nullptr >= value);
    ASSERT_FALSE (true >= value);
    ASSERT_FALSE (false >= value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::min () >= value);
    ASSERT_TRUE  (std::numeric_limits <int32_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::min () >= value);
    ASSERT_TRUE  (std::numeric_limits <uint32_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::min () >= value);
    ASSERT_TRUE  (std::numeric_limits <int64_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::min () >= value);
    ASSERT_TRUE  (std::numeric_limits <uint64_t>::max () >= value);
    ASSERT_TRUE  (std::numeric_limits <double>::min () >= value);
    ASSERT_TRUE  (std::numeric_limits <double>::max () >= value);
    ASSERT_TRUE  ("foo" >= value);
    ASSERT_TRUE  (Array ({1}) >= value);
    ASSERT_TRUE  (Object ({{"i", 1}}) >= value);

    value.set <std::string> ("foo");
    ASSERT_FALSE (nullptr >= value);
    ASSERT_FALSE (true >= value);
    ASSERT_FALSE (false >= value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::min () >= value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::min () >= value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::min () >= value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::min () >= value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits <double>::min () >= value);
    ASSERT_FALSE (std::numeric_limits <double>::max () >= value);
    ASSERT_TRUE  ("foo" >= value);
    ASSERT_TRUE  (Array ({1}) >= value);
    ASSERT_TRUE  (Object ({{"i", 1}}) >= value);

    value.set <Array, Array::value_type> ({1});
    ASSERT_FALSE (nullptr >= value);
    ASSERT_FALSE (true >= value);
    ASSERT_FALSE (false >= value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::min () >= value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::min () >= value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::min () >= value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::min () >= value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits <double>::min () >= value);
    ASSERT_FALSE (std::numeric_limits <double>::max () >= value);
    ASSERT_FALSE ("foo" >= value);
    ASSERT_TRUE  (Array ({1}) >= value);
    ASSERT_TRUE  (Object ({{"i", 1}}) >= value);

    value.set <Object, Object::value_type> ({{"i", 1}});
    ASSERT_FALSE (nullptr >= value);
    ASSERT_FALSE (true >= value);
    ASSERT_FALSE (false >= value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::min () >= value);
    ASSERT_FALSE (std::numeric_limits <int32_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::min () >= value);
    ASSERT_FALSE (std::numeric_limits <uint32_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::min () >= value);
    ASSERT_FALSE (std::numeric_limits <int64_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::min () >= value);
    ASSERT_FALSE (std::numeric_limits <uint64_t>::max () >= value);
    ASSERT_FALSE (std::numeric_limits <double>::min () >= value);
    ASSERT_FALSE (std::numeric_limits <double>::max () >= value);
    ASSERT_FALSE ("foo" >= value);
    ASSERT_FALSE (Array ({1}) >= value);
    ASSERT_TRUE  (Object ({{"i", 1}}) >= value);
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
