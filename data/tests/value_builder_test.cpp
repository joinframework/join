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
#include <join/value_builder.hpp>
#include <join/json.hpp>
#include <join/pack.hpp>

// Libraries.
#include <gtest/gtest.h>

// C++.
#include <string_view>
#include <sstream>
#include <vector>
#include <string>

using join::Array;
using join::Object;
using join::Value;
using join::ValueBuilder;
using join::JsonReader;
using join::PackReader;
using join::PackWriter;
using join::SaxErrc;

/**
 * @brief value builder derived by a user.
 */
struct Derived : public ValueBuilder
{
    using ValueBuilder::ValueBuilder;
};

/**
 * @brief handler parsing each string it receives into its own value.
 */
struct Nested : public join::SaxHandler
{
    int setString (std::string_view json)
    {
        values.emplace_back ();
        return JsonReader ().deserialize (json.data (), json.size (), values.back ());
    }

    std::vector<Value> values;
};

/**
 * @brief Test create method.
 */
TEST (ValueBuilder, create)
{
    Value value;
    {
        ValueBuilder builder (value);
        ASSERT_EQ (builder.startArray (), 0);
        ASSERT_EQ (builder.setInt (1), 0);

        Value inner;
        {
            ValueBuilder innerBuilder (inner);
            ASSERT_EQ (innerBuilder.startArray (), 0);
            ASSERT_EQ (innerBuilder.setInt (2), 0);
            ASSERT_EQ (innerBuilder.stopArray (), 0);
        }
        ASSERT_EQ (inner, Array{2});

        ASSERT_EQ (builder.setInt (3), 0);
        ASSERT_EQ (builder.stopArray (), 0);
    }
    ASSERT_EQ (value, (Array{1, 3}));

    Nested nested;
    ASSERT_EQ (JsonReader ().deserialize (std::string ("[\"[1,{\\\"a\\\":2}]\", \"3\"]"), nested), 0)
        << join::lastError.message ();
    ASSERT_EQ (nested.values.size (), 2U);
    ASSERT_EQ (nested.values[0], (Array{1, Object{{"a", 2}}}));
    ASSERT_EQ (nested.values[1], 3);
}

/**
 * @brief Test setNull method.
 */
TEST (ValueBuilder, setNull)
{
    Value value;
    ValueBuilder builder (value);
    value = Array{};
    ASSERT_EQ (builder.setNull (), 0);
    ASSERT_TRUE (value.isNull ());

    ASSERT_EQ (builder.startArray (), 0);
    ASSERT_EQ (builder.setNull (), 0);
    ASSERT_EQ (builder.stopArray (), 0);
    ASSERT_EQ (value, Array{nullptr});
}

/**
 * @brief Test setBool method.
 */
TEST (ValueBuilder, setBool)
{
    Value value;
    ValueBuilder builder (value);
    ASSERT_EQ (builder.setBool (true), 0);
    ASSERT_EQ (value, true);

    ASSERT_EQ (builder.startArray (), 0);
    ASSERT_EQ (builder.setBool (true), 0);
    ASSERT_EQ (builder.setBool (false), 0);
    ASSERT_EQ (builder.stopArray (), 0);
    ASSERT_EQ (value, (Array{true, false}));
}

/**
 * @brief Test setInt method.
 */
TEST (ValueBuilder, setInt)
{
    Value value;
    ValueBuilder builder (value);
    ASSERT_EQ (builder.setInt (-1), 0);
    ASSERT_EQ (value.index (), Value::IntegerValue);
    ASSERT_EQ (value, -1);
    ASSERT_EQ (JsonReader ().deserialize (std::string ("42"), value), 0) << join::lastError.message ();
    ASSERT_EQ (value, 42);

    ASSERT_EQ (builder.startArray (), 0);
    ASSERT_EQ (builder.setInt (-1), 0);
    ASSERT_EQ (builder.stopArray (), 0);
    ASSERT_EQ (value[0].index (), Value::IntegerValue);
    ASSERT_EQ (value[0], -1);
}

/**
 * @brief Test setUint method.
 */
TEST (ValueBuilder, setUint)
{
    Value value;
    ValueBuilder builder (value);
    ASSERT_EQ (builder.setUint (1), 0);
    ASSERT_EQ (value.index (), Value::UnsignedValue);

    ASSERT_EQ (builder.startArray (), 0);
    ASSERT_EQ (builder.setUint (1), 0);
    ASSERT_EQ (builder.stopArray (), 0);
    ASSERT_EQ (value[0].index (), Value::UnsignedValue);
    ASSERT_EQ (value[0], 1U);
}

/**
 * @brief Test setInt64 method.
 */
TEST (ValueBuilder, setInt64)
{
    Value value;
    ValueBuilder builder (value);
    ASSERT_EQ (builder.setInt64 (-1), 0);
    ASSERT_EQ (value.index (), Value::Integer64Value);

    ASSERT_EQ (builder.startArray (), 0);
    ASSERT_EQ (builder.setInt64 (-1), 0);
    ASSERT_EQ (builder.stopArray (), 0);
    ASSERT_EQ (value[0].index (), Value::Integer64Value);
    ASSERT_EQ (value[0], int64_t (-1));
}

/**
 * @brief Test setUint64 method.
 */
TEST (ValueBuilder, setUint64)
{
    Value value;
    ValueBuilder builder (value);
    ASSERT_EQ (builder.setUint64 (1), 0);
    ASSERT_EQ (value.index (), Value::Unsigned64Value);

    ASSERT_EQ (builder.startArray (), 0);
    ASSERT_EQ (builder.setUint64 (1), 0);
    ASSERT_EQ (builder.stopArray (), 0);
    ASSERT_EQ (value[0].index (), Value::Unsigned64Value);
    ASSERT_EQ (value[0], uint64_t (1));
}

/**
 * @brief Test setDouble method.
 */
TEST (ValueBuilder, setDouble)
{
    Value value;
    ValueBuilder builder (value);
    ASSERT_EQ (builder.setDouble (1.5), 0);
    ASSERT_EQ (value, 1.5);

    ASSERT_EQ (builder.startArray (), 0);
    ASSERT_EQ (builder.setDouble (1.5), 0);
    ASSERT_EQ (builder.stopArray (), 0);
    ASSERT_EQ (value, Array{1.5});
}

/**
 * @brief Test setString method.
 */
TEST (ValueBuilder, setString)
{
    Value value;
    ValueBuilder builder (value);
    ASSERT_EQ (builder.setString ("a"), 0);
    ASSERT_EQ (value, "a");

    const std::string b ("b");
    ASSERT_EQ (builder.startArray (), 0);
    ASSERT_EQ (builder.setString ("a"), 0);
    ASSERT_EQ (builder.setString (b), 0);
    ASSERT_EQ (builder.stopArray (), 0);
    ASSERT_EQ (value, (Array{"a", "b"}));
}

/**
 * @brief Test startArray method.
 */
TEST (ValueBuilder, startArray)
{
    Value value;
    ValueBuilder builder (value);
    for (size_t i = 0; i < join::saxMaxDepth; ++i)
    {
        ASSERT_EQ (builder.startArray (), 0);
    }
    ASSERT_EQ (builder.startArray (), -1);
    ASSERT_EQ (join::lastError, SaxErrc::StackOverflow);

    builder.reset ();
    ASSERT_EQ (builder.startArray (2), 0);
    ASSERT_EQ (builder.startArray (), 0);
    ASSERT_EQ (builder.stopArray (), 0);
    ASSERT_EQ (builder.stopArray (), 0);
    ASSERT_EQ (value, Array{Value (Array{})});
}

/**
 * @brief Test stopArray method.
 */
TEST (ValueBuilder, stopArray)
{
    Value value;
    ValueBuilder builder (value);
    ASSERT_EQ (builder.stopArray (), -1);
    ASSERT_EQ (join::lastError, SaxErrc::InvalidParent);

    ASSERT_EQ (builder.startArray (), 0);
    ASSERT_EQ (builder.setInt (1), 0);
    ASSERT_EQ (builder.stopArray (), 0);
    ASSERT_EQ (value, Array{1});
}

/**
 * @brief Test startObject method.
 */
TEST (ValueBuilder, startObject)
{
    Value value;
    ValueBuilder builder (value);
    for (size_t i = 0; i < join::saxMaxDepth; ++i)
    {
        ASSERT_EQ (builder.startObject (), 0);
        ASSERT_EQ (builder.setKey ("a"), 0);
    }
    ASSERT_EQ (builder.startObject (), -1);
    ASSERT_EQ (join::lastError, SaxErrc::StackOverflow);

    builder.reset ();
    ASSERT_EQ (builder.startObject (1), 0);
    ASSERT_EQ (builder.setKey ("a"), 0);
    ASSERT_EQ (builder.startObject (), 0);
    ASSERT_EQ (builder.stopObject (), 0);
    ASSERT_EQ (builder.stopObject (), 0);
    ASSERT_EQ (value, (Object{{"a", Object{}}}));
}

/**
 * @brief Test members method.
 */
TEST (ValueBuilder, members)
{
    Value value;
    ValueBuilder builder (value);
    Object object{{"b", 1}, {"a", 2}};
    EXPECT_EQ (&builder.members (object), &object);

    Value copy;
    ValueBuilder copyBuilder (copy);
    value = Object{{"b", 1}, {"a", Array{2}}};
    ASSERT_EQ (value.serialize (copyBuilder), 0) << join::lastError.message ();
    EXPECT_EQ (copy, value);

    value = "scalar";
    ASSERT_EQ (value.serialize (copyBuilder), 0) << join::lastError.message ();
    EXPECT_EQ (copy, "scalar");
}

/**
 * @brief Test setKey method.
 */
TEST (ValueBuilder, setKey)
{
    Value value;
    ValueBuilder builder (value);
    ASSERT_EQ (builder.setKey ("a"), -1);
    ASSERT_EQ (join::lastError, SaxErrc::InvalidParent);
    ASSERT_EQ (builder.setKey (nullptr), -1);
    ASSERT_EQ (join::lastError, SaxErrc::InvalidParent);
    ASSERT_EQ (builder.setKey (true), -1);
    ASSERT_EQ (join::lastError, SaxErrc::InvalidParent);
    ASSERT_EQ (builder.setKey (int32_t (-2)), -1);
    ASSERT_EQ (join::lastError, SaxErrc::InvalidParent);
    ASSERT_EQ (builder.setKey (uint32_t (3)), -1);
    ASSERT_EQ (join::lastError, SaxErrc::InvalidParent);
    ASSERT_EQ (builder.setKey (int64_t (-4)), -1);
    ASSERT_EQ (join::lastError, SaxErrc::InvalidParent);
    ASSERT_EQ (builder.setKey (uint64_t (5)), -1);
    ASSERT_EQ (join::lastError, SaxErrc::InvalidParent);
    ASSERT_EQ (builder.setKey (6.5), -1);
    ASSERT_EQ (join::lastError, SaxErrc::InvalidParent);

    ASSERT_EQ (builder.startObject (), 0);
    ASSERT_EQ (builder.setKey (nullptr), 0);
    ASSERT_EQ (builder.setInt (0), 0);
    ASSERT_EQ (builder.setKey (true), 0);
    ASSERT_EQ (builder.setInt (1), 0);
    ASSERT_EQ (builder.setKey (int32_t (-2)), 0);
    ASSERT_EQ (builder.setInt (2), 0);
    ASSERT_EQ (builder.setKey (uint32_t (3)), 0);
    ASSERT_EQ (builder.setInt (3), 0);
    ASSERT_EQ (builder.setKey (int64_t (-4)), 0);
    ASSERT_EQ (builder.setInt (4), 0);
    ASSERT_EQ (builder.setKey (uint64_t (5)), 0);
    ASSERT_EQ (builder.setInt (5), 0);
    ASSERT_EQ (builder.setKey (6.5), 0);
    ASSERT_EQ (builder.setInt (6), 0);
    ASSERT_EQ (builder.setKey (std::string_view ("seven")), 0);
    ASSERT_EQ (builder.setInt (7), 0);
    ASSERT_EQ (builder.setKey ("eight"), 0);
    ASSERT_EQ (builder.setInt (8), 0);
    ASSERT_EQ (builder.stopObject (), 0);
    ASSERT_EQ (value.size (), 9U);
    ASSERT_TRUE (value.getObject ()[0].first.isNull ());
    ASSERT_TRUE (value.getObject ()[1].first.isBool ());
    ASSERT_TRUE (value.getObject ()[2].first.isInt ());
    ASSERT_TRUE (value.getObject ()[3].first.isUint ());
    ASSERT_TRUE (value.getObject ()[4].first.isInt64 ());
    ASSERT_TRUE (value.getObject ()[5].first.isUint64 ());
    ASSERT_TRUE (value.getObject ()[6].first.isDouble ());
    ASSERT_EQ (value["seven"], 7);
    ASSERT_EQ (value["eight"], 8);
}

/**
 * @brief Test stopObject method.
 */
TEST (ValueBuilder, stopObject)
{
    Value value;
    ValueBuilder builder (value);
    ASSERT_EQ (builder.stopObject (), -1);
    ASSERT_EQ (join::lastError, SaxErrc::InvalidParent);

    ASSERT_EQ (builder.startObject (), 0);
    ASSERT_EQ (builder.setInt (1), 0);
    ASSERT_EQ (builder.stopObject (), -1);
    ASSERT_EQ (join::lastError, SaxErrc::InvalidKey);

    builder.reset ();
    ASSERT_EQ (builder.startObject (), 0);
    ASSERT_EQ (builder.setKey ("a"), 0);
    ASSERT_EQ (builder.setInt (1), 0);
    ASSERT_EQ (builder.stopObject (), 0);
    ASSERT_EQ (value, (Object{{"a", 1}}));

    std::string pack;
    {
        std::stringstream out;
        PackWriter writer (out);
        Value source = Object{{"a", Array{1, 2}}, {"b", nullptr}};
        ASSERT_EQ (source.serialize (writer), 0) << join::lastError.message ();
        pack = out.str ();
    }
    ASSERT_EQ (PackReader ().deserialize (pack, value), 0) << join::lastError.message ();
    ASSERT_EQ (value["a"][1], 2);
    ASSERT_TRUE (value["b"].isNull ());
}

/**
 * @brief Test reset method.
 */
TEST (ValueBuilder, reset)
{
    Value value;
    ValueBuilder builder (value);
    ASSERT_EQ (builder.startArray (), 0);
    ASSERT_EQ (builder.setInt (1), 0);
    builder.reset ();
    ASSERT_EQ (builder.startObject (), 0);
    ASSERT_EQ (builder.stopObject (), 0);
    ASSERT_TRUE (value.isObject ());
    ASSERT_TRUE (value.empty ());

    ASSERT_EQ (JsonReader ().deserialize (std::string ("[1, [2, "), builder), -1);
    ASSERT_TRUE (value.isObject ());
    ASSERT_EQ (JsonReader ().deserialize (std::string ("{\"a\":3}"), builder), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_EQ (value["a"], 3);

    Value derivedValue;
    Derived derived (derivedValue);
    ASSERT_EQ (JsonReader ().deserialize (std::string ("[1, [2, "), derived), -1);
    ASSERT_EQ (JsonReader ().deserialize (std::string ("[3]"), derived), 0) << join::lastError.message ();
    ASSERT_TRUE (derivedValue.isArray ());
    ASSERT_EQ (derivedValue.size (), 1U);
    ASSERT_EQ (derivedValue[0], 3);
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
