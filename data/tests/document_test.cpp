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
#include <join/document.hpp>
#include <join/json.hpp>
#include <join/pack.hpp>

// Libraries.
#include <gtest/gtest.h>

// C++.
#include <stdexcept>
#include <sstream>

using join::Array;
using join::Object;
using join::Value;
using join::Document;
using join::JsonReader;
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
 * @brief document derived by a user.
 */
struct Derived : public Document
{
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
 * @brief Test move constructor.
 */
TEST (Document, moveConstruct)
{
    Document document;
    ASSERT_EQ (JsonReader ().deserialize (std::string ("[1,{\"a\":2}]"), document), 0) << join::lastError.message ();
    Document moved (std::move (document));
    ASSERT_EQ (moved.root (), (Array{1, Object{{"a", 2}}}));

    Document partial;
    ASSERT_EQ (partial.startArray (), 0);
    ASSERT_EQ (partial.setInt (1), 0);
    ASSERT_EQ (partial.startObject (), 0);
    ASSERT_EQ (partial.setKey (Value ("a")), 0);
    Document resumed (std::move (partial));
    ASSERT_EQ (resumed.setInt (2), 0);
    ASSERT_EQ (resumed.stopObject (), 0);
    ASSERT_EQ (resumed.stopArray (), 0);
    ASSERT_EQ (resumed.root (), (Array{1, Object{{"a", 2}}}));
    ASSERT_EQ (partial.stopArray (), -1);
    ASSERT_EQ (join::lastError, SaxErrc::InvalidParent);
}

/**
 * @brief Test move assignment.
 */
TEST (Document, moveAssign)
{
    Document document;
    ASSERT_EQ (JsonReader ().deserialize (std::string ("[1,{\"a\":2}]"), document), 0) << join::lastError.message ();
    Document moved;
    moved = std::move (document);
    ASSERT_EQ (moved.root (), (Array{1, Object{{"a", 2}}}));

    Document partial;
    ASSERT_EQ (partial.startArray (), 0);
    ASSERT_EQ (partial.setInt (1), 0);
    ASSERT_EQ (partial.startObject (), 0);
    ASSERT_EQ (partial.setKey (Value ("a")), 0);
    Document resumed;
    ASSERT_EQ (resumed.startArray (), 0);
    resumed = std::move (partial);
    ASSERT_EQ (resumed.setInt (2), 0);
    ASSERT_EQ (resumed.stopObject (), 0);
    ASSERT_EQ (resumed.stopArray (), 0);
    ASSERT_EQ (resumed.root (), (Array{1, Object{{"a", 2}}}));
    ASSERT_EQ (partial.stopArray (), -1);
    ASSERT_EQ (join::lastError, SaxErrc::InvalidParent);
}

/**
 * @brief Test root method.
 */
TEST (Document, root)
{
    Document document;
    ASSERT_TRUE (document.root ().isNull ());

    document.root () = Array{1, 2};
    const Document& constant = document;
    ASSERT_TRUE (constant.root ().isArray ());
    ASSERT_EQ (constant.root ().size (), 2U);
}

/**
 * @brief Test serialize method.
 */
TEST (Document, serialize)
{
    Document document;
    Value& value = document.root ();
    value.pushBack (1);
    value.pushBack (nullptr);
    value.pushBack (true);
    value.pushBack ("test");
    value.pushBack (1.23456789e-13);

    std::stringstream out;
    JsonWriter jsonWriter (out);
    ASSERT_EQ (document.serialize (jsonWriter), 0) << join::lastError.message ();
    EXPECT_EQ (out.str (), "[1,null,true,\"test\",1.23456789e-13]");

    out.str ("");
    PackWriter packWriter (out);
    ASSERT_EQ (document.serialize (packWriter), 0) << join::lastError.message ();
    EXPECT_EQ (out.str (), std::string ({'\x95', '\x01', '\xc0', '\xc3', '\xa4', '\x74', '\x65', '\x73', '\x74', '\xcb',
                                         '\x3d', '\x41', '\x5f', '\xff', '\xe5', '\x3a', '\x68', '\x5d'}));

    Counter counter;
    ASSERT_EQ (document.serialize (counter), 0) << join::lastError.message ();
    EXPECT_EQ (counter.values, 5U);

    value = Object{};
    value["numbers"] = Array{333333333.33333329, 1E30, 4.50, 2e-3, 0.000000000000000000000000001};
    value["string"] = "\u20ac$\u000F\u000aA'\u0042\u0022\u005c\\\"/";
    value["literals"] = Array{nullptr, true, false};

    out.str ("");
    JsonCanonicalizer canonicalizer (out);
    ASSERT_EQ (document.serialize (canonicalizer), 0) << join::lastError.message ();
    EXPECT_EQ (out.str (),
               "{\"literals\":[null,true,false],\"numbers\":[333333333.3333333,1e+30,4.5,0.002,1e-27],\"string\":\"€$"
               "\\u000f\\nA'B\\\"\\\\\\\\\\\"/\"}");

    value = Object{{Array{1}, 1}};
    out.str ("");
    ASSERT_EQ (document.serialize (packWriter), -1);
    ASSERT_EQ (join::lastError, SaxErrc::InvalidKey);
    ASSERT_EQ (out.str (), "");

    value = Array{7};
    ASSERT_EQ (document.serialize (packWriter), 0) << join::lastError.message ();
    ASSERT_EQ (out.str (), std::string ({'\x91', '\x07'}));

    FailingBuffer buffer;
    std::ostream failing (&buffer);
    JsonWriter failingWriter (failing);
    value = Array{std::string (2000, 'a')};
    ASSERT_THROW (document.serialize (failingWriter), std::runtime_error);
    value = Array{1, 2};
    ASSERT_EQ (document.serialize (failingWriter), 0) << join::lastError.message ();
    ASSERT_EQ (buffer.written, "[1,2]");
}

/**
 * @brief Test setNull method.
 */
TEST (Document, setNull)
{
    Document document;
    document.root () = Array{};
    ASSERT_EQ (document.setNull (), 0);
    ASSERT_TRUE (document.root ().isNull ());

    ASSERT_EQ (document.startArray (), 0);
    ASSERT_EQ (document.setNull (), 0);
    ASSERT_EQ (document.stopArray (), 0);
    ASSERT_EQ (document.root (), Array{nullptr});
}

/**
 * @brief Test setBool method.
 */
TEST (Document, setBool)
{
    Document document;
    ASSERT_EQ (document.setBool (true), 0);
    ASSERT_EQ (document.root (), true);

    ASSERT_EQ (document.startArray (), 0);
    ASSERT_EQ (document.setBool (true), 0);
    ASSERT_EQ (document.setBool (false), 0);
    ASSERT_EQ (document.stopArray (), 0);
    ASSERT_EQ (document.root (), (Array{true, false}));
}

/**
 * @brief Test setInt method.
 */
TEST (Document, setInt)
{
    Document document;
    ASSERT_EQ (document.setInt (-1), 0);
    ASSERT_EQ (document.root ().index (), Value::IntegerValue);
    ASSERT_EQ (document.root (), -1);
    ASSERT_EQ (JsonReader ().deserialize (std::string ("42"), document), 0) << join::lastError.message ();
    ASSERT_EQ (document.root (), 42);

    ASSERT_EQ (document.startArray (), 0);
    ASSERT_EQ (document.setInt (-1), 0);
    ASSERT_EQ (document.stopArray (), 0);
    ASSERT_EQ (document.root ()[0].index (), Value::IntegerValue);
    ASSERT_EQ (document.root ()[0], -1);
}

/**
 * @brief Test setUint method.
 */
TEST (Document, setUint)
{
    Document document;
    ASSERT_EQ (document.setUint (1), 0);
    ASSERT_EQ (document.root ().index (), Value::UnsignedValue);

    ASSERT_EQ (document.startArray (), 0);
    ASSERT_EQ (document.setUint (1), 0);
    ASSERT_EQ (document.stopArray (), 0);
    ASSERT_EQ (document.root ()[0].index (), Value::UnsignedValue);
    ASSERT_EQ (document.root ()[0], 1U);
}

/**
 * @brief Test setInt64 method.
 */
TEST (Document, setInt64)
{
    Document document;
    ASSERT_EQ (document.setInt64 (-1), 0);
    ASSERT_EQ (document.root ().index (), Value::Integer64Value);

    ASSERT_EQ (document.startArray (), 0);
    ASSERT_EQ (document.setInt64 (-1), 0);
    ASSERT_EQ (document.stopArray (), 0);
    ASSERT_EQ (document.root ()[0].index (), Value::Integer64Value);
    ASSERT_EQ (document.root ()[0], int64_t (-1));
}

/**
 * @brief Test setUint64 method.
 */
TEST (Document, setUint64)
{
    Document document;
    ASSERT_EQ (document.setUint64 (1), 0);
    ASSERT_EQ (document.root ().index (), Value::Unsigned64Value);

    ASSERT_EQ (document.startArray (), 0);
    ASSERT_EQ (document.setUint64 (1), 0);
    ASSERT_EQ (document.stopArray (), 0);
    ASSERT_EQ (document.root ()[0].index (), Value::Unsigned64Value);
    ASSERT_EQ (document.root ()[0], uint64_t (1));
}

/**
 * @brief Test setDouble method.
 */
TEST (Document, setDouble)
{
    Document document;
    ASSERT_EQ (document.setDouble (1.5), 0);
    ASSERT_EQ (document.root (), 1.5);

    ASSERT_EQ (document.startArray (), 0);
    ASSERT_EQ (document.setDouble (1.5), 0);
    ASSERT_EQ (document.stopArray (), 0);
    ASSERT_EQ (document.root (), Array{1.5});
}

/**
 * @brief Test setString method.
 */
TEST (Document, setString)
{
    Document document;
    ASSERT_EQ (document.setString ("a"), 0);
    ASSERT_EQ (document.root (), "a");

    const std::string b ("b");
    ASSERT_EQ (document.startArray (), 0);
    ASSERT_EQ (document.setString ("a"), 0);
    ASSERT_EQ (document.setString (b), 0);
    ASSERT_EQ (document.stopArray (), 0);
    ASSERT_EQ (document.root (), (Array{"a", "b"}));
}

/**
 * @brief Test startArray method.
 */
TEST (Document, startArray)
{
    Document document;
    for (size_t i = 0; i < join::saxMaxDepth; ++i)
    {
        ASSERT_EQ (document.startArray (), 0);
    }
    ASSERT_EQ (document.startArray (), -1);
    ASSERT_EQ (join::lastError, SaxErrc::StackOverflow);

    document.reset ();
    ASSERT_EQ (document.startArray (2), 0);
    ASSERT_EQ (document.startArray (), 0);
    ASSERT_EQ (document.stopArray (), 0);
    ASSERT_EQ (document.stopArray (), 0);
    ASSERT_EQ (document.root (), Array{Value (Array{})});
}

/**
 * @brief Test stopArray method.
 */
TEST (Document, stopArray)
{
    Document document;
    ASSERT_EQ (document.stopArray (), -1);
    ASSERT_EQ (join::lastError, SaxErrc::InvalidParent);

    ASSERT_EQ (document.startArray (), 0);
    ASSERT_EQ (document.setInt (1), 0);
    ASSERT_EQ (document.stopArray (), 0);
    ASSERT_EQ (document.root (), Array{1});
}

/**
 * @brief Test startObject method.
 */
TEST (Document, startObject)
{
    Document document;
    for (size_t i = 0; i < join::saxMaxDepth; ++i)
    {
        ASSERT_EQ (document.startObject (), 0);
        ASSERT_EQ (document.setKey ("a"), 0);
    }
    ASSERT_EQ (document.startObject (), -1);
    ASSERT_EQ (join::lastError, SaxErrc::StackOverflow);

    document.reset ();
    ASSERT_EQ (document.startObject (1), 0);
    ASSERT_EQ (document.setKey ("a"), 0);
    ASSERT_EQ (document.startObject (), 0);
    ASSERT_EQ (document.stopObject (), 0);
    ASSERT_EQ (document.stopObject (), 0);
    ASSERT_EQ (document.root (), (Object{{"a", Object{}}}));
}

/**
 * @brief Test members method.
 */
TEST (Document, members)
{
    Document document;
    Object object{{"b", 1}, {"a", 2}};
    EXPECT_EQ (&document.members (object), &object);

    Document copy;
    document.root () = Object{{"b", 1}, {"a", Array{2}}};
    ASSERT_EQ (document.serialize (copy), 0) << join::lastError.message ();
    EXPECT_EQ (copy.root (), document.root ());

    document.root () = "scalar";
    ASSERT_EQ (document.serialize (copy), 0) << join::lastError.message ();
    EXPECT_EQ (copy.root (), "scalar");
}

/**
 * @brief Test setKey method.
 */
TEST (Document, setKey)
{
    Document document;
    ASSERT_EQ (document.setKey ("a"), -1);
    ASSERT_EQ (join::lastError, SaxErrc::InvalidParent);
    const Value a ("a");
    ASSERT_EQ (document.setKey (a), -1);
    ASSERT_EQ (join::lastError, SaxErrc::InvalidParent);

    ASSERT_EQ (document.startObject (), 0);
    ASSERT_EQ (document.setKey ("a"), 0);
    ASSERT_EQ (document.setInt (1), 0);
    ASSERT_EQ (document.setKey (Value (2)), 0);
    ASSERT_EQ (document.setInt (3), 0);
    const Value c ("c");
    ASSERT_EQ (document.setKey (c), 0);
    ASSERT_EQ (document.setInt (4), 0);
    ASSERT_EQ (document.stopObject (), 0);
    ASSERT_EQ (document.root ()["a"], 1);
    ASSERT_EQ (document.root ().getObject ()[1].first, 2);
    ASSERT_EQ (document.root ()["c"], 4);
}

/**
 * @brief Test stopObject method.
 */
TEST (Document, stopObject)
{
    Document document;
    ASSERT_EQ (document.stopObject (), -1);
    ASSERT_EQ (join::lastError, SaxErrc::InvalidParent);

    ASSERT_EQ (document.startObject (), 0);
    ASSERT_EQ (document.setInt (1), 0);
    ASSERT_EQ (document.stopObject (), -1);
    ASSERT_EQ (join::lastError, SaxErrc::InvalidKey);

    document.reset ();
    ASSERT_EQ (document.startObject (), 0);
    ASSERT_EQ (document.setKey ("a"), 0);
    ASSERT_EQ (document.setInt (1), 0);
    ASSERT_EQ (document.stopObject (), 0);
    ASSERT_EQ (document.root (), (Object{{"a", 1}}));

    std::string pack;
    {
        std::stringstream out;
        PackWriter writer (out);
        Document source;
        source.root () = Object{{"a", Array{1, 2}}, {"b", nullptr}};
        ASSERT_EQ (source.serialize (writer), 0) << join::lastError.message ();
        pack = out.str ();
    }
    ASSERT_EQ (PackReader ().deserialize (pack, document), 0) << join::lastError.message ();
    ASSERT_EQ (document.root ()["a"][1], 2);
    ASSERT_TRUE (document.root ()["b"].isNull ());
}

/**
 * @brief Test reset method.
 */
TEST (Document, reset)
{
    Document document;
    ASSERT_EQ (document.startArray (), 0);
    ASSERT_EQ (document.setInt (1), 0);
    document.reset ();
    ASSERT_EQ (document.startObject (), 0);
    ASSERT_EQ (document.stopObject (), 0);
    ASSERT_TRUE (document.root ().isObject ());
    ASSERT_TRUE (document.root ().empty ());

    ASSERT_EQ (JsonReader ().deserialize (std::string ("[1, [2, "), document), -1);
    ASSERT_TRUE (document.root ().isObject ());
    ASSERT_EQ (JsonReader ().deserialize (std::string ("{\"a\":3}"), document), 0) << join::lastError.message ();
    ASSERT_TRUE (document.root ().isObject ());
    ASSERT_EQ (document.root ()["a"], 3);

    Derived derived;
    ASSERT_EQ (JsonReader ().deserialize (std::string ("[1, [2, "), derived), -1);
    ASSERT_EQ (JsonReader ().deserialize (std::string ("[3]"), derived), 0) << join::lastError.message ();
    ASSERT_TRUE (derived.root ().isArray ());
    ASSERT_EQ (derived.root ().size (), 1U);
    ASSERT_EQ (derived.root ()[0], 3);
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
