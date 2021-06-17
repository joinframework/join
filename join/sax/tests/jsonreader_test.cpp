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

// C++.
#include <sstream>
#include <fstream>
#include <chrono>

// C.
#include <cmath>

using join::sax::Array;
using join::sax::Member;
using join::sax::Object;
using join::sax::Value;

using join::sax::SaxErrc;
using join::sax::JsonErrc;
using join::sax::JsonReader;

/**
 * @brief Test JSON parsing pass.
 */
TEST (JsonReader, pass)
{
    std::stringstream stream;
    Value value;

    stream.clear ();
    stream.str ("[]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_TRUE (value.empty ());

    stream.clear ();
    stream.str ("[1234567890]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isInt ());
    ASSERT_EQ (value[0], 1234567890);

    stream.clear ();
    stream.str ("[-9876.543210]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    ASSERT_DOUBLE_EQ (value[0].getDouble (), -9876.543210);

    stream.clear ();
    stream.str ("[0.123456789e-12]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    ASSERT_DOUBLE_EQ (value[0].getDouble (), 0.123456789e-12);

    stream.clear ();
    stream.str ("[1.234567890E+34]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    ASSERT_DOUBLE_EQ (value[0].getDouble (), 1.234567890E+34);

    stream.clear ();
    stream.str ("[NaN]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    ASSERT_TRUE (!std::signbit (value[0].getDouble ()) && std::isnan (value[0].getDouble ()));

    stream.clear ();
    stream.str ("[-NaN]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    ASSERT_TRUE (std::signbit (value[0].getDouble ()) && std::isnan (value[0].getDouble ()));

    stream.clear ();
    stream.str ("[Inf]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    ASSERT_TRUE (!std::signbit (value[0].getDouble ()) && std::isinf (value[0].getDouble ()));

    stream.clear ();
    stream.str ("[-Inf]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    ASSERT_TRUE (std::signbit (value[0].getDouble ()) && std::isinf (value[0].getDouble ()));

    stream.clear ();
    stream.str ("[Infinity]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    ASSERT_TRUE (!std::signbit (value[0].getDouble ()) && std::isinf (value[0].getDouble ()));

    stream.clear ();
    stream.str ("[-Infinity]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    ASSERT_TRUE (std::signbit (value[0].getDouble ()) && std::isinf (value[0].getDouble ()));

    stream.clear ();
    stream.str ("[true]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isBool ());
    ASSERT_EQ (value[0], true);

    stream.clear ();
    stream.str ("[false]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isBool ());
    ASSERT_EQ (value[0], false);

    stream.clear ();
    stream.str ("[null]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isNull ());
    ASSERT_EQ (value[0], nullptr);

    stream.clear ();
    stream.str ("[0.5 ,98.6\n,\n99.44\n,\n1066,\n1e1\n,0.1e1\n,1e-1\n,1e00\n,2e+00\n,2e-00\n,\"rosebud\"]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_EQ (value.size (), 11);
    ASSERT_TRUE (value[0].isDouble ());
    ASSERT_DOUBLE_EQ (value[0].getDouble (), 0.5);
    ASSERT_TRUE (value[1].isDouble ());
    ASSERT_DOUBLE_EQ (value[1].getDouble (), 98.6);
    ASSERT_TRUE (value[2].isDouble ());
    ASSERT_DOUBLE_EQ (value[2].getDouble (), 99.44);
    ASSERT_TRUE (value[3].isInt ());
    ASSERT_EQ (value[3].getInt (), 1066);
    ASSERT_TRUE (value[4].isDouble ());
    ASSERT_DOUBLE_EQ (value[4].getDouble (), 1e1);
    ASSERT_TRUE (value[5].isDouble ());
    ASSERT_DOUBLE_EQ (value[5].getDouble (), 0.1e1);
    ASSERT_TRUE (value[6].isDouble ());
    ASSERT_DOUBLE_EQ (value[6].getDouble (), 1e-1);
    ASSERT_TRUE (value[7].isDouble ());
    ASSERT_DOUBLE_EQ (value[7].getDouble (), 1e00);
    ASSERT_TRUE (value[8].isDouble ());
    ASSERT_DOUBLE_EQ (value[8].getDouble (), 2e+00);
    ASSERT_TRUE (value[9].isDouble ());
    ASSERT_DOUBLE_EQ (value[9].getDouble (), 2e-00);
    ASSERT_TRUE (value[10].isString ());
    ASSERT_STREQ (value[10].getString ().c_str (), "rosebud");

    stream.clear ();
    stream.str ("[[[[[[[[[[[[[[[[[[[\"Not too deep\"]]]]]]]]]]]]]]]]]]]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();

    stream.clear ();
    stream.str ("{}");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_TRUE (value.empty ());

    stream.clear ();
    stream.str ("{\"integer\": 1234567890}");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["integer"].isInt ());
    ASSERT_EQ (value["integer"], 1234567890);

    stream.clear ();
    stream.str ("{\"real\": -9876.543210}");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["real"].isDouble ());
    ASSERT_DOUBLE_EQ (value["real"].getDouble (), -9876.543210);

    stream.clear ();
    stream.str ("{\"e\": 0.123456789e-12}");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["e"].isDouble ());
    ASSERT_DOUBLE_EQ (value["e"].getDouble (), 0.123456789e-12);

    stream.clear ();
    stream.str ("{\"E\": 1.234567890E+34}");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["E"].isDouble ());
    ASSERT_DOUBLE_EQ (value["E"].getDouble (), 1.234567890E+34);

    stream.clear ();
    stream.str ("{\"\":  23456789012E66}");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[""].isDouble ());
    ASSERT_DOUBLE_EQ (value[""].getDouble (), 23456789012E66);

    stream.clear ();
    stream.str ("{\"zero\": 0}");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["zero"].isInt ());
    ASSERT_EQ (value["zero"], 0);

    stream.clear ();
    stream.str ("{\"one\": 1}");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["one"].isInt ());
    ASSERT_EQ (value["one"], 1);

    stream.clear ();
    stream.str ("{\"space\": \" \"}");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["space"].isString ());
    ASSERT_EQ (value["space"], " ");

    stream.clear ();
    stream.str ("{\"quote\": \"\\\"\"}");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["quote"].isString ());
    ASSERT_EQ (value["quote"], "\"");

    stream.clear ();
    stream.str ("{\"backslash\": \"\\\\\"}");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["backslash"].isString ());
    ASSERT_EQ (value["backslash"], "\\");

    stream.clear ();
    stream.str ("{\"controls\": \"\\b\\f\\n\\r\\t\"}");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["controls"].isString ());
    ASSERT_EQ (value["controls"], "\b\f\n\r\t");

    stream.clear ();
    stream.str ("{\"slash\": \"/ & \\\\/\"}");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["slash"].isString ());
    ASSERT_EQ (value["slash"], "/ & \\/");

    stream.clear ();
    stream.str ("{\"alpha\": \"abcdefghijklmnopqrstuvwyz\"}");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["alpha"].isString ());
    ASSERT_EQ (value["alpha"], "abcdefghijklmnopqrstuvwyz");

    stream.clear ();
    stream.str ("{\"ALPHA\": \"ABCDEFGHIJKLMNOPQRSTUVWYZ\"}");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["ALPHA"].isString ());
    ASSERT_EQ (value["ALPHA"], "ABCDEFGHIJKLMNOPQRSTUVWYZ");

    stream.clear ();
    stream.str ("{\"digit\": \"0123456789\"}");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["digit"].isString ());
    ASSERT_EQ (value["digit"], "0123456789");

    stream.clear ();
    stream.str ("{\"0123456789\": \"digit\"}");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["0123456789"].isString ());
    ASSERT_EQ (value["0123456789"], "digit");

    stream.clear ();
    stream.str ("{\"special\": \"`1~!@#$%^&*()_+-={':[,]}|;.</>?\"}");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["special"].isString ());
    ASSERT_EQ (value["special"], "`1~!@#$%^&*()_+-={':[,]}|;.</>?");

    stream.clear ();
    stream.str ("{\"hex\": \"\\u0123\\u4567\\u89AB\\uCDEF\\uabcd\\uef4A\"}");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["hex"].isString ());
    ASSERT_EQ (value["hex"].getString (), std::string ("\xC4\xA3\xE4\x95\xA7\xE8\xA6\xAB\xEC\xB7\xAF\xEA\xAF\x8D\xEE\xBD\x8A"));

    stream.clear ();
    stream.str ("{\"true\": true}");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["true"].isBool ());
    ASSERT_EQ (value["true"], true);

    stream.clear ();
    stream.str ("{\"false\": false}");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["false"].isBool ());
    ASSERT_EQ (value["false"], false);

    stream.clear ();
    stream.str ("{\"null\": null}");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["null"].isNull ());
    ASSERT_EQ (value["null"], nullptr);

    stream.clear ();
    stream.str ("{\"array\":[  ]}");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["array"].isArray ());
    ASSERT_TRUE (value["array"].empty ());

    stream.clear ();
    stream.str ("{\"object\":{  }}");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["object"].isObject ());
    ASSERT_TRUE (value["object"].empty ());

    stream.clear ();
    stream.str ("{\"address\": \"50 St. James Street\"}");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["address"].isString ());
    ASSERT_EQ (value["address"], "50 St. James Street");

    stream.clear ();
    stream.str ("{\"url\": \"https://www.sierrawireless.com/\"}");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["url"].isString ());
    ASSERT_EQ (value["url"], "https://www.sierrawireless.com/");

    stream.clear ();
    stream.str ("{\"comment\": \"// /* <!-- --\",\n\"# -- --> */\": \" \"}");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["comment"].isString ());
    ASSERT_EQ (value["comment"], "// /* <!-- --");
    ASSERT_TRUE (value["# -- --> */"].isString ());
    ASSERT_EQ (value["# -- --> */"], " ");

    stream.clear ();
    stream.str ("{\" s p a c e d \" :[1,2 , 3\n\n,\n4 , 5        ,          6           ,7        ],\"compact\":[1,2,3,4,5,6,7]}");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[" s p a c e d "].isArray ());
    ASSERT_EQ (value[" s p a c e d "][0], 1);
    ASSERT_EQ (value[" s p a c e d "][1], 2);
    ASSERT_EQ (value[" s p a c e d "][2], 3);
    ASSERT_EQ (value[" s p a c e d "][3], 4);
    ASSERT_EQ (value[" s p a c e d "][4], 5);
    ASSERT_EQ (value[" s p a c e d "][5], 6);
    ASSERT_EQ (value[" s p a c e d "][6], 7);
    ASSERT_TRUE (value["compact"].isArray ());
    ASSERT_EQ (value["compact"][0], 1);
    ASSERT_EQ (value["compact"][1], 2);
    ASSERT_EQ (value["compact"][2], 3);
    ASSERT_EQ (value["compact"][3], 4);
    ASSERT_EQ (value["compact"][4], 5);
    ASSERT_EQ (value["compact"][5], 6);
    ASSERT_EQ (value["compact"][6], 7);

    stream.clear ();
    stream.str ("{\"object with 1 member\":[\"array with 1 element\"]}");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["object with 1 member"].isArray ());
    ASSERT_EQ (value["object with 1 member"][0], "array with 1 element");

    stream.clear ();
    stream.str ("{\"quotes\": \"&#34; \\u0022 %22 0x22 034 &#x22;\"}");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["quotes"].isString ());
    ASSERT_EQ (value["quotes"], "&#34; \" %22 0x22 034 &#x22;");

    stream.clear ();
    stream.str ("{\"\\u0022\\b\\f\\n\\r\\t`1~!@#$%^&*()_+-=[]{}|;:',./<>?\"\n: \"A key can be any string\"}");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["\"\b\f\n\r\t`1~!@#$%^&*()_+-=[]{}|;:',./<>?"].isString ());
    ASSERT_EQ (value["\"\b\f\n\r\t`1~!@#$%^&*()_+-=[]{}|;:',./<>?"], "A key can be any string");
}

/**
 * @brief Test JSON parsing fail.
 */
TEST (JsonReader, fail)
{
    std::stringstream stream;
    Value value;

    stream.clear ();
    stream.str ("\"payload should be an object or array, not a string\"");
    ASSERT_EQ (value.deserialize <JsonReader> (stream), -1);

    stream.clear ();
    stream.str ("[Infinit]");
    ASSERT_EQ (value.deserialize <JsonReader> (stream), -1);

    stream.clear ();
    stream.str ("[nul]");
    ASSERT_EQ (value.deserialize <JsonReader> (stream), -1);

    stream.clear ();
    stream.str ("[tru]");
    ASSERT_EQ (value.deserialize <JsonReader> (stream), -1);

    stream.clear ();
    stream.str ("[fals]");
    ASSERT_EQ (value.deserialize <JsonReader> (stream), -1);

    stream.clear ();
    stream.str ("[\"Unclosed array\"");
    ASSERT_EQ (value.deserialize <JsonReader> (stream), -1);

    stream.clear ();
    stream.str ("{unquoted_key: \"keys must be quoted\"}");
    ASSERT_EQ (value.deserialize <JsonReader> (stream), -1);

    stream.clear ();
    stream.str ("[\"extra comma\",]");
    ASSERT_EQ (value.deserialize <JsonReader> (stream), -1);

    stream.clear ();
    stream.str ("[\"double extra comma\",,]");
    ASSERT_EQ (value.deserialize <JsonReader> (stream), -1);

    stream.clear ();
    stream.str ("[   , \"<-- missing value\"]");
    ASSERT_EQ (value.deserialize <JsonReader> (stream), -1);

    stream.clear ();
    stream.str ("[\"Comma after the close\"],");
    ASSERT_EQ (value.deserialize <JsonReader> (stream), -1);

    stream.clear ();
    stream.str ("[\"Extra close\"]]");
    ASSERT_EQ (value.deserialize <JsonReader> (stream), -1);

    stream.clear ();
    stream.str ("[\"Missing quote]");
    ASSERT_EQ (value.deserialize <JsonReader> (stream), -1);

    stream.clear ();
    stream.str ("{\"Extra comma\": true,}");
    ASSERT_EQ (value.deserialize <JsonReader> (stream), -1);

    stream.clear ();
    stream.str ("{\"Extra value after close\": true} \"misplaced quoted value\"");
    ASSERT_EQ (value.deserialize <JsonReader> (stream), -1);

    stream.clear ();
    stream.str ("{\"Illegal expression\": 1 + 2}");
    ASSERT_EQ (value.deserialize <JsonReader> (stream), -1);

    stream.clear ();
    stream.str ("{\"Illegal invocation\": alert()}");
    ASSERT_EQ (value.deserialize <JsonReader> (stream), -1);

    stream.clear ();
    stream.str ("{\"Numbers cannot have leading zeroes\": 013}");
    ASSERT_EQ (value.deserialize <JsonReader> (stream), -1);

    stream.clear ();
    stream.str ("{\"Numbers cannot be hex\": 0x14}");
    ASSERT_EQ (value.deserialize <JsonReader> (stream), -1);

    stream.clear ();
    stream.str ("[\"Illegal backslash escape: \\x15\"]");
    ASSERT_EQ (value.deserialize <JsonReader> (stream), -1);

    stream.clear ();
    stream.str ("[\\naked]");
    ASSERT_EQ (value.deserialize <JsonReader> (stream), -1);

    stream.clear ();
    stream.str ("[\"Illegal backslash escape: \\017\"]");
    ASSERT_EQ (value.deserialize <JsonReader> (stream), -1);

    stream.clear ();
    stream.str ("[[[[[[[[[[[[[[[[[[[[\"Too deep\"]]]]]]]]]]]]]]]]]]]]");
    ASSERT_EQ (value.deserialize <JsonReader> (stream), -1);

    stream.clear ();
    stream.str ("{\"Missing colon\" null}");
    ASSERT_EQ (value.deserialize <JsonReader> (stream), -1);

    stream.clear ();
    stream.str ("{\"Double colon\":: null}");
    ASSERT_EQ (value.deserialize <JsonReader> (stream), -1);

    stream.clear ();
    stream.str ("{\"Comma instead of colon\", null}");
    ASSERT_EQ (value.deserialize <JsonReader> (stream), -1);

    stream.clear ();
    stream.str ("[\"Colon instead of comma\": false]");
    ASSERT_EQ (value.deserialize <JsonReader> (stream), -1);

    stream.clear ();
    stream.str ("[\"Bad value\", truth]");
    ASSERT_EQ (value.deserialize <JsonReader> (stream), -1);

    stream.clear ();
    stream.str ("['single quote']");
    ASSERT_EQ (value.deserialize <JsonReader> (stream), -1);

    stream.clear ();
    stream.str ("[\"\ttab\tcharacter\tin\tstring\t\"]");
    ASSERT_EQ (value.deserialize <JsonReader> (stream), -1);

    stream.clear ();
    stream.str ("[\"tab\\\t  character\\\t  in\\\t  string\\\t  \"]");
    ASSERT_EQ (value.deserialize <JsonReader> (stream), -1);

    stream.clear ();
    stream.str ("[\"line\nbreak\"]");
    ASSERT_EQ (value.deserialize <JsonReader> (stream), -1);

    stream.clear ();
    stream.str ("[\"line\\\nbreak\"]");
    ASSERT_EQ (value.deserialize <JsonReader> (stream), -1);

    stream.clear ();
    stream.str ("[0e]");
    ASSERT_EQ (value.deserialize <JsonReader> (stream), -1);

    stream.clear ();
    stream.str ("[0e+]");
    ASSERT_EQ (value.deserialize <JsonReader> (stream), -1);

    stream.clear ();
    stream.str ("[0e+-1]");
    ASSERT_EQ (value.deserialize <JsonReader> (stream), -1);

    stream.clear ();
    stream.str ("{\"Comma instead of closing brace\": true,");
    ASSERT_EQ (value.deserialize <JsonReader> (stream), -1);

    stream.clear ();
    stream.str ("[\"Mismatch\"}");
    ASSERT_EQ (value.deserialize <JsonReader> (stream), -1);
}

/**
 * @brief Test JSON parse double.
 */
TEST (JsonReader, dbl)
{
    std::stringstream stream;
    Value value;

    stream.clear ();
    stream.str ("[0.0]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 0.0);

    stream.clear ();
    stream.str ("[-0.0]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), -0.0);

    stream.clear ();
    stream.str ("[1.0]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 1.0);

    stream.clear ();
    stream.str ("[-1.0]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), -1.0);

    stream.clear ();
    stream.str ("[1.5]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 1.5);

    stream.clear ();
    stream.str ("[-1.5]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), -1.5);

    stream.clear ();
    stream.str ("[3.1416]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 3.1416);

    stream.clear ();
    stream.str ("[1E10]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 1E10);

    stream.clear ();
    stream.str ("[1e10]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 1e10);

    stream.clear ();
    stream.str ("[1E+10]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 1E+10);

    stream.clear ();
    stream.str ("[1E-10]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 1E-10);

    stream.clear ();
    stream.str ("[-1E10]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), -1E10);

    stream.clear ();
    stream.str ("[-1e10]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), -1e10);

    stream.clear ();
    stream.str ("[-1E+10]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), -1E+10);

    stream.clear ();
    stream.str ("[-1E-10]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), -1E-10);

    stream.clear ();
    stream.str ("[1.234E+10]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 1.234E+10);

    stream.clear ();
    stream.str ("[1.234E-10]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 1.234E-10);

    stream.clear ();
    stream.str ("[1.79769e+308]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 1.79769e+308);

    stream.clear ();
    stream.str ("[2.22507e-308]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 2.22507e-308);

    stream.clear ();
    stream.str ("[-1.79769e+308]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), -1.79769e+308);

    stream.clear ();
    stream.str ("[-2.22507e-308]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), -2.22507e-308);

    stream.clear ();
    stream.str ("[-4.9406564584124654e-324]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 4.9406564584124654e-324);

    stream.clear ();
    stream.str ("[2.2250738585072009e-308]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 2.2250738585072009e-308);

    stream.clear ();
    stream.str ("[2.2250738585072014e-308]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 2.2250738585072014e-308);

    stream.clear ();
    stream.str ("[1.7976931348623157e+308]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 1.7976931348623157e+308);

    stream.clear ();
    stream.str ("[1e-10000]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 0.0);

    stream.clear ();
    stream.str ("[18446744073709551616]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 18446744073709551616.0);

    stream.clear ();
    stream.str ("[-9223372036854775809]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), -9223372036854775809.0);

    stream.clear ();
    stream.str ("[0.9868011474609375]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 0.9868011474609375);

    stream.clear ();
    stream.str ("[123e34]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 123e34);

    stream.clear ();
    stream.str ("[45913141877270640000.0]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 45913141877270640000.0);

    stream.clear ();
    stream.str ("[2.2250738585072011e-308]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 2.2250738585072011e-308);

    stream.clear ();
    stream.str ("[1e-214748363]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 0.0);

    stream.clear ();
    stream.str ("[1e-214748364]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 0.0);

    stream.clear ();
    stream.str ("[0.017976931348623157e+310]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 1.7976931348623157e+308);

    stream.clear ();
    stream.str ("[2.2250738585072012e-308]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 2.2250738585072014e-308);

    stream.clear ();
    stream.str ("[0.999999999999999944488848768742172978818416595458984375]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 1.0);

    stream.clear ();
    stream.str ("[0.999999999999999944488848768742172978818416595458984374]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 0.99999999999999989);

    stream.clear ();
    stream.str ("[0.999999999999999944488848768742172978818416595458984376]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 1.0);

    stream.clear ();
    stream.str ("[1.00000000000000011102230246251565404236316680908203125]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 1.0);

    stream.clear ();
    stream.str ("[1.00000000000000011102230246251565404236316680908203124]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 1.0);

    stream.clear ();
    stream.str ("[1.00000000000000011102230246251565404236316680908203126]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 1.00000000000000022);

    stream.clear ();
    stream.str ("[72057594037927928.0]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 72057594037927928.0);

    stream.clear ();
    stream.str ("[72057594037927936.0]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 72057594037927936.0);

    stream.clear ();
    stream.str ("[72057594037927932.0]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 72057594037927936.0);

    stream.clear ();
    stream.str ("[7205759403792793199999e-5]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 72057594037927928.0);

    stream.clear ();
    stream.str ("[7205759403792793200001e-5]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 72057594037927936.0);

    stream.clear ();
    stream.str ("[9223372036854774784.0]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 9223372036854774784.0);

    stream.clear ();
    stream.str ("[9223372036854775808.0]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 9223372036854775808.0);

    stream.clear ();
    stream.str ("[9223372036854775296.0]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 9223372036854775808.0);

    stream.clear ();
    stream.str ("[922337203685477529599999e-5]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 9223372036854774784.0);

    stream.clear ();
    stream.str ("[922337203685477529600001e-5]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 9223372036854775808.0);

    stream.clear ();
    stream.str ("[10141204801825834086073718800384]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 10141204801825834086073718800384.0);

    stream.clear ();
    stream.str ("[10141204801825835211973625643008]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 10141204801825835211973625643008.0);

    stream.clear ();
    stream.str ("[10141204801825834649023672221696]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 10141204801825835211973625643008.0);

    stream.clear ();
    stream.str ("[1014120480182583464902367222169599999e-5]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 10141204801825834086073718800384.0);

    stream.clear ();
    stream.str ("[1014120480182583464902367222169600001e-5]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 10141204801825835211973625643008.0);

    stream.clear ();
    stream.str ("[5708990770823838890407843763683279797179383808]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 5708990770823838890407843763683279797179383808.0);

    stream.clear ();
    stream.str ("[5708990770823839524233143877797980545530986496]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 5708990770823839524233143877797980545530986496.0);

    stream.clear ();
    stream.str ("[5708990770823839207320493820740630171355185152]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 5708990770823839524233143877797980545530986496.0);

    stream.clear ();
    stream.str ("[5708990770823839207320493820740630171355185151999e-3]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 5708990770823838890407843763683279797179383808.0);

    stream.clear ();
    stream.str ("[5708990770823839207320493820740630171355185152001e-3]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 5708990770823839524233143877797980545530986496.0);

    stream.clear ();
    stream.str ("[100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
                "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
                "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
                "0000000000]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 1E308);

    stream.clear ();
    stream.str ("[2.22507385850720113605740979670913197593481954635164564802342610972482222202107694551652952390813508"
                "79141491589130396211068700864386945946455276572074078206217433799881410632673292535522868813721490129"
                "81122451451889849057222307285255133155755015914397476397983411801999323962548289017107081850690630666"
                "65599493827577257201576306269066333264756530000924588831643303777979186961204949739037782970490505108"
                "06099407302629371289589500035837999672072543043602840788957717961509455167482434710307026091446215722"
                "89880258182545180325707018860872113128079512233426288368622321503775666622503982534335974568884423900"
                "26549819838548794829220689472168983109969836584681402285424333066033985088644580400103493397042756718"
                "6443383770486037861622771738545623065874679014086723327636718751234567890123456789012345678901e-308]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_DOUBLE_EQ (value[0].getDouble (), 2.2250738585072014e-308);
}

/**
 * @brief Test JSON parse string.
 */
TEST (JsonReader, str)
{
    std::stringstream stream;
    Value value;

    stream.clear ();
    stream.str ("[\"\"]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isString ());
    EXPECT_STREQ (value[0].getString ().c_str (), "");

    stream.clear ();
    stream.str ("[\"Hello\"]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isString ());
    EXPECT_STREQ (value[0].getString ().c_str (), "Hello");

    stream.clear ();
    stream.str ("[\"Hello\\nWorld\"]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isString ());
    EXPECT_STREQ (value[0].getString ().c_str (), "Hello\nWorld");

    stream.clear ();
    stream.str ("[\"Hello\\u0000World\"]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isString ());
    EXPECT_STREQ (value[0].getString ().c_str (), "Hello\0World");

    stream.clear ();
    stream.str ("[\"\\\"\\\\/\\b\\f\\n\\r\\t\"]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isString ());
    EXPECT_STREQ (value[0].getString ().c_str (), "\"\\/\b\f\n\r\t");

    stream.clear ();
    stream.str ("[\"\\u0024\"]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isString ());
    EXPECT_STREQ (value[0].getString ().c_str (), "\x24");

    stream.clear ();
    stream.str ("[\"\\u00A2\"]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isString ());
    EXPECT_STREQ (value[0].getString ().c_str (), "\xC2\xA2");

    stream.clear ();
    stream.str ("[\"\\u20AC\"]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isString ());
    EXPECT_STREQ (value[0].getString ().c_str (), "\xE2\x82\xAC");

    stream.clear ();
    stream.str ("[\"\\uD834\\uDD1E\"]");
    ASSERT_NE (value.deserialize <JsonReader> (stream), -1) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isString ());
    EXPECT_STREQ (value[0].getString ().c_str (), "\xF0\x9D\x84\x9E");
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
