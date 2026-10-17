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
#include <join/json.hpp>
#include <join/sax.hpp>

// libraries.
#include <gtest/gtest.h>

// C++.
#include <type_traits>
#include <string>

using join::Value;
using join::Object;
using join::SaxHandler;
using join::JsonReader;

/**
 * @brief handler counting strings only.
 */
struct StringCounter : public SaxHandler
{
    int setString (const std::string&)
    {
        ++strings;
        return 0;
    }

    size_t strings = 0;
};

static_assert (!std::is_polymorphic<SaxHandler>::value, "SaxHandler must not have a vtable");
static_assert (std::is_empty<SaxHandler>::value, "SaxHandler must be empty");
static_assert (sizeof (StringCounter) == sizeof (size_t), "SaxHandler must not add any size");

/**
 * @brief Test setNull method.
 */
TEST (SaxHandler, setNull)
{
    SaxHandler handler;
    EXPECT_EQ (handler.setNull (), 0);
}

/**
 * @brief Test setBool method.
 */
TEST (SaxHandler, setBool)
{
    SaxHandler handler;
    EXPECT_EQ (handler.setBool (true), 0);
}

/**
 * @brief Test setInt method.
 */
TEST (SaxHandler, setInt)
{
    SaxHandler handler;
    EXPECT_EQ (handler.setInt (-1), 0);
}

/**
 * @brief Test setUint method.
 */
TEST (SaxHandler, setUint)
{
    SaxHandler handler;
    EXPECT_EQ (handler.setUint (1), 0);
}

/**
 * @brief Test setInt64 method.
 */
TEST (SaxHandler, setInt64)
{
    SaxHandler handler;
    EXPECT_EQ (handler.setInt64 (-1), 0);
}

/**
 * @brief Test setUint64 method.
 */
TEST (SaxHandler, setUint64)
{
    SaxHandler handler;
    EXPECT_EQ (handler.setUint64 (1), 0);
}

/**
 * @brief Test setDouble method.
 */
TEST (SaxHandler, setDouble)
{
    SaxHandler handler;
    EXPECT_EQ (handler.setDouble (1.5), 0);
}

/**
 * @brief Test setString method.
 */
TEST (SaxHandler, setString)
{
    SaxHandler handler;
    EXPECT_EQ (handler.setString ("a"), 0);

    StringCounter counter;
    ASSERT_EQ (JsonReader ().deserialize (std::string (R"({"a":["b",1,null,{"c":"d"}]})"), counter), 0)
        << join::lastError.message ();
    EXPECT_EQ (counter.strings, 2U);
}

/**
 * @brief Test startArray method.
 */
TEST (SaxHandler, startArray)
{
    SaxHandler handler;
    EXPECT_EQ (handler.startArray (), 0);
    EXPECT_EQ (handler.startArray (2), 0);
}

/**
 * @brief Test stopArray method.
 */
TEST (SaxHandler, stopArray)
{
    SaxHandler handler;
    EXPECT_EQ (handler.stopArray (), 0);
}

/**
 * @brief Test startObject method.
 */
TEST (SaxHandler, startObject)
{
    SaxHandler handler;
    EXPECT_EQ (handler.startObject (), 0);
    EXPECT_EQ (handler.startObject (2), 0);
}

/**
 * @brief Test members method.
 */
TEST (SaxHandler, members)
{
    SaxHandler handler;
    Object object{{"b", 1}, {"a", 2}};
    EXPECT_EQ (&handler.members (object), &object);
}

/**
 * @brief Test setKey method.
 */
TEST (SaxHandler, setKey)
{
    SaxHandler handler;
    EXPECT_EQ (handler.setKey (Value ("a")), 0);
}

/**
 * @brief Test stopObject method.
 */
TEST (SaxHandler, stopObject)
{
    SaxHandler handler;
    EXPECT_EQ (handler.stopObject (), 0);
}

/**
 * @brief Test reset method.
 */
TEST (SaxHandler, reset)
{
    SaxHandler handler;
    handler.reset ();
    EXPECT_EQ (handler.setNull (), 0);
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
