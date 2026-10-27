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

using join::Value;
using join::Array;
using join::Object;
using join::JsonCanonicalizer;
using join::JsonErrc;
using join::SaxErrc;

/**
 * @brief Test members method.
 */
TEST (JsonCanonicalizer, members)
{
    Object object{{"b", 1}, {"\u20ac", 2}, {"a", 3}, {"\U0001F600", 4}};

    std::stringstream out;
    JsonCanonicalizer writer (out);
    auto sorted = writer.members (object);
    ASSERT_EQ (sorted.size (), 4U);
    EXPECT_EQ (sorted[0].get ().first, "a");
    EXPECT_EQ (sorted[1].get ().first, "b");
    EXPECT_EQ (sorted[2].get ().first, "\u20ac");
    EXPECT_EQ (sorted[3].get ().first, "\U0001F600");

    Value value;
    value = Object{{"b", 1}, {"a", 2}};
    ASSERT_EQ (value.serialize (writer), 0) << join::lastError.message ();
    EXPECT_EQ (out.str (), "{\"a\":2,\"b\":1}");
    EXPECT_EQ (value.getObject ()[0].first, "b");
    EXPECT_EQ (value.getObject ()[1].first, "a");
}

/**
 * @brief Test flush method.
 */
TEST (JsonCanonicalizer, flush)
{
    std::stringstream out;
    JsonCanonicalizer writer (out);

    EXPECT_EQ (writer.startObject (), 0);
    EXPECT_EQ (writer.setKey ("a"), 0);
    EXPECT_EQ (writer.setInt (1), 0);
    EXPECT_EQ (out.str (), "");
    writer.flush ();
    EXPECT_EQ (out.str (), "{\"a\":1");
    EXPECT_EQ (writer.stopObject (), 0);
    EXPECT_EQ (out.str (), "{\"a\":1}");
}

/**
 * @brief Test reset method.
 */
TEST (JsonCanonicalizer, reset)
{
    std::stringstream out;
    JsonCanonicalizer writer (out);

    EXPECT_EQ (writer.startObject (), 0);
    EXPECT_EQ (writer.setKey ("a"), 0);
    writer.reset ();
    EXPECT_EQ (out.str (), "");
    EXPECT_EQ (writer.setInt (2), 0);
    EXPECT_EQ (out.str (), "2");
}

/**
 * @brief Test nan.
 */
TEST (JsonCanonicalizer, nan)
{
    Value value;
    value.pushBack (std::numeric_limits<double>::quiet_NaN ());
    value.pushBack (-std::numeric_limits<double>::quiet_NaN ());

    std::stringstream out;
    JsonCanonicalizer writer (out);
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (out.str (), "[null,null]");
}

/**
 * @brief Test infinity.
 */
TEST (JsonCanonicalizer, infinity)
{
    Value value;
    value.pushBack (std::numeric_limits<double>::infinity ());
    value.pushBack (-std::numeric_limits<double>::infinity ());

    std::stringstream out;
    JsonCanonicalizer writer (out);
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (out.str (), "[null,null]");
}

/**
 * @brief Test array.
 */
TEST (JsonCanonicalizer, array)
{
    Value value;
    value.pushBack (56);
    value.pushBack (Object{{"d", true}, {"10", nullptr}, {"1", Array{}}});
    value.pushBack (-53.0);

    std::stringstream out;
    JsonCanonicalizer writer (out);
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (out.str (), "[56,{\"1\":[],\"10\":null,\"d\":true},-53]");
}

/**
 * @brief Test french.
 */
TEST (JsonCanonicalizer, french)
{
    Value value;
    value["peach"] = "This sorting order";
    value["péché"] = "is wrong according to French";
    value["pêche"] = "but canonicalization MUST";
    value["sin"] = "ignore locale";

    std::stringstream out;
    JsonCanonicalizer writer (out);
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (out.str (),
               "{\"peach\":\"This sorting order\",\"péché\":\"is wrong according to French\",\"pêche\":\"but "
               "canonicalization MUST\",\"sin\":\"ignore locale\"}");
}

/**
 * @brief Test structures.
 */
TEST (JsonCanonicalizer, structures)
{
    Value value;
    value["1"] = Object{{"f", Object{{"f", "hi"}, {"F", 5}}}, {"\n", 56.0}};
    value["10"] = Object{};
    value[""] = "empty";
    value["a"] = Object{};
    value["111"] = Array{Object{{"e", "yes"}, {"E", "no"}}};
    value["A"] = Object{};

    std::stringstream out;
    JsonCanonicalizer writer (out);
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (out.str (),
               "{\"\":\"empty\",\"1\":{\"\\n\":56,\"f\":{\"F\":5,\"f\":\"hi\"}},\"10\":{},\"111\":[{\"E\":\"no\",\"e\":"
               "\"yes\"}],\"A\":{},\"a\":{}}");
}

/**
 * @brief Test unicode.
 */
TEST (JsonCanonicalizer, unicode)
{
    Value value;
    value["Unnormalized Unicode"] = "A\u030a";

    std::stringstream out;
    JsonCanonicalizer writer (out);
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (out.str (), "{\"Unnormalized Unicode\":\"Å\"}");
}

/**
 * @brief Test values.
 */
TEST (JsonCanonicalizer, values)
{
    Value value;
    value["numbers"] = Array{333333333.33333329, 1E30, 4.50, 2e-3, 0.000000000000000000000000001};
    value["string"] = "\u20ac$\u000F\u000aA'\u0042\u0022\u005c\\\"/";
    value["literals"] = Array{nullptr, true, false};

    std::stringstream out;
    JsonCanonicalizer writer (out);
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (out.str (),
               "{\"literals\":[null,true,false],\"numbers\":[333333333.3333333,1e+30,4.5,0.002,1e-27],\"string\":\"€$"
               "\\u000f\\nA'B\\\"\\\\\\\\\\\"/\"}");
}

/**
 * @brief Test weird.
 */
TEST (JsonCanonicalizer, weird)
{
    Value value;
    value["\u20ac"] = "Euro Sign";
    value["\r"] = "Carriage Return";
    value["\u000a"] = "Newline";
    value["1"] = "One";
    value["\u0080"] = "Control\u007f";
    value["\U0001f602"] = "Smiley";
    value["\u00f6"] = "Latin Small Letter O With Diaeresis";
    value["\ufb33"] = "Hebrew Letter Dalet With Dagesh";
    value["</script>"] = "Browser Challenge";

    std::stringstream out;
    JsonCanonicalizer writer (out);
    ASSERT_NE (value.serialize (writer), -1) << join::lastError.message ();
    EXPECT_EQ (out.str (),
               "{\"\\n\":\"Newline\",\"\\r\":\"Carriage Return\",\"1\":\"One\",\"</script>\":\"Browser "
               "Challenge\",\"\":\"Control\",\"ö\":\"Latin Small Letter O With Diaeresis\",\"€\":\"Euro "
               "Sign\",\"😂\":\"Smiley\",\"דּ\":\"Hebrew Letter Dalet With Dagesh\"}");
}

/**
 * @brief Test invalid.
 */
TEST (JsonCanonicalizer, invalid)
{
    Value value;
    value["\xFF"] = "Invalid";
    value["a"] = "Ascii";

    std::stringstream out;
    JsonCanonicalizer writer (out);
    ASSERT_EQ (value.serialize (writer), -1);
    ASSERT_EQ (join::lastError, JsonErrc::InvalidEncoding);

    value = Object{{"a", "\xF0"}};
    out.str ("");
    ASSERT_EQ (value.serialize (writer), -1);
    ASSERT_EQ (join::lastError, JsonErrc::InvalidEncoding);

    value = Array{"\xFF"};
    out.str ("");
    ASSERT_EQ (value.serialize (writer), -1);
    ASSERT_EQ (join::lastError, JsonErrc::InvalidEncoding);
}

/**
 * @brief Test writing a whole document.
 */
TEST (JsonCanonicalizer, document)
{
    Value value;
    value = Object{{Value (1), "One"}, {"a", "Ascii"}};

    std::stringstream out;
    JsonCanonicalizer writer (out);
    EXPECT_EQ (value.serialize (writer), -1);
    EXPECT_EQ (join::lastError, SaxErrc::InvalidKey);
    EXPECT_EQ (out.str (), "");

    out.str ("");
    EXPECT_EQ (writer.setNull (), 0);
    EXPECT_EQ (out.str (), "null");

    value = Object{{"b", 1}, {"a", Array{1}}};
    out.str ("");
    JsonCanonicalizer indented (out, 2);
    EXPECT_EQ (value.serialize (indented), 0) << join::lastError.message ();
    EXPECT_EQ (out.str (), "{\"a\":[1],\"b\":1}");
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
