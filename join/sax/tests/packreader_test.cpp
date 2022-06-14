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
#include <join/pack.hpp>

// libraries.
#include <gtest/gtest.h>

// C++.
#include <sstream>

// C.
#include <cmath>

using join::Array;
using join::Member;
using join::Object;
using join::Value;

using join::SaxErrc;
using join::PackReader;

/**
 * @brief Test deserialize method.
 */
TEST (PackReader, deserialize)
{
    std::stringstream stream;
    Value value;

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x00'}));
    std::string str ({'\xdd', '\x00', '\x00', '\x00', '\x00'});
    char data[] = {'\xdd', '\x00', '\x00', '\x00', '\x00', '\x00'};

    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_TRUE (value.empty ());

    ASSERT_EQ (value.deserialize <PackReader> (str), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_TRUE (value.empty ());

    ASSERT_EQ (value.deserialize <PackReader> (data, sizeof (data) - 1), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_TRUE (value.empty ());

    ASSERT_EQ (value.deserialize <PackReader> (&data[0], &data[sizeof (data) - 1]), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_TRUE (value.empty ());

    ASSERT_EQ (value.deserialize <PackReader> (&data[0], &data[sizeof (data)]), -1);
    ASSERT_EQ (join::lastError, SaxErrc::ExtraData);
}

/**
 * @brief Test MessagePack parsing pass.
 */
TEST (PackReader, pass)
{
    std::stringstream stream;
    Value value;

    stream.clear ();
    stream.str (std::string ({'\x90'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_TRUE (value.empty ());

    stream.clear ();
    stream.str (std::string ({'\xdc', '\x00', '\x01', '\xce', '\x49', '\x96', '\x02', '\xd2'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isInt ());
    ASSERT_EQ (value[0], 1234567890);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\xc0', '\xc3', '\x4a', '\x45', '\x87', '\xe7', '\xc0', '\x6e'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    ASSERT_DOUBLE_EQ (value[0].getDouble (), -9876.543210);

    stream.clear ();
    stream.str (std::string ({'\x91', '\xcb', '\x3d', '\x41', '\x5f', '\xff', '\xe5', '\x3a', '\x68', '\x5d'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    ASSERT_DOUBLE_EQ (value[0].getDouble (), 0.123456789e-12);

    stream.clear ();
    stream.str (std::string ({'\xdc', '\x00', '\x01', '\xcb', '\x47', '\x03', '\x05', '\x82', '\xff', '\xd7', '\x14', '\x75'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    ASSERT_DOUBLE_EQ (value[0].getDouble (), 1.234567890E+34);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xc3'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isBool ());
    ASSERT_EQ (value[0], true);

    stream.clear ();
    stream.str (std::string ({'\x91', '\xc2'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isBool ());
    ASSERT_EQ (value[0], false);

    stream.clear ();
    stream.str (std::string ({'\xdc', '\x00', '\x01', '\xc0'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isNull ());
    ASSERT_EQ (value[0], nullptr);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x0b', '\xcb', '\x3f', '\xe0', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\xcb', '\x40', '\x58', '\xa6', '\x66', '\x66',
                              '\x66', '\x66', '\x66', '\xcb', '\x40', '\x58', '\xdc', '\x28', '\xf5', '\xc2', '\x8f', '\x5c', '\xcd', '\x04', '\x2a', '\xcb', '\x40', '\x24', '\x00', '\x00',
                              '\x00', '\x00', '\x00', '\x00', '\xcb', '\x3f', '\xf0', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\xcb', '\x3f', '\xb9', '\x99', '\x99', '\x99', '\x99',
                              '\x99', '\x9a', '\xcb', '\x3f', '\xf0', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\xcb', '\x40', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00',
                              '\xcb', '\x40', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\xa7', '\x72', '\x6f', '\x73', '\x65', '\x62', '\x75', '\x64'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
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
    ASSERT_EQ (value[10], "rosebud");

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xdd', '\x00', '\x00', '\x00', '\x01', '\xdd', '\x00', '\x00', '\x00', '\x01', '\xdd', '\x00', '\x00', '\x00', '\x01',
                              '\xdd', '\x00', '\x00', '\x00', '\x01', '\xdd', '\x00', '\x00', '\x00', '\x01', '\xdd', '\x00', '\x00', '\x00', '\x01', '\xdd', '\x00', '\x00', '\x00', '\x01',
                              '\xdd', '\x00', '\x00', '\x00', '\x01', '\xdd', '\x00', '\x00', '\x00', '\x01', '\xdd', '\x00', '\x00', '\x00', '\x01', '\xdd', '\x00', '\x00', '\x00', '\x01',
                              '\xdd', '\x00', '\x00', '\x00', '\x01', '\xdd', '\x00', '\x00', '\x00', '\x01', '\xdd', '\x00', '\x00', '\x00', '\x01', '\xdd', '\x00', '\x00', '\x00', '\x01',
                              '\xdd', '\x00', '\x00', '\x00', '\x01', '\xdd', '\x00', '\x00', '\x00', '\x01', '\xdd', '\x00', '\x00', '\x00', '\x01', '\xac', '\x4e', '\x6f', '\x74', '\x20',
                              '\x74', '\x6f', '\x6f', '\x20', '\x64', '\x65', '\x65', '\x70'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();

    stream.clear ();
    stream.str (std::string ({'\x80'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_TRUE (value.empty ());

    stream.clear ();
    stream.str (std::string ({'\xde', '\x00', '\x01', '\xa7', '\x69', '\x6e', '\x74', '\x65', '\x67', '\x65', '\x72', '\xce', '\x49', '\x96', '\x02', '\xd2'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["integer"].isInt ());
    ASSERT_EQ (value["integer"], 1234567890);

    stream.clear ();
    stream.str (std::string ({'\xdf', '\x00', '\x00', '\x00', '\x01', '\xa4', '\x72', '\x65', '\x61', '\x6c', '\xcb', '\xc0', '\xc3', '\x4a', '\x45', '\x87', '\xe7', '\xc0', '\x6e'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["real"].isDouble ());
    ASSERT_DOUBLE_EQ (value["real"].getDouble (), -9876.543210);

    stream.clear ();
    stream.str (std::string ({'\x81', '\xa1', '\x65', '\xcb', '\x3d', '\x41', '\x5f', '\xff', '\xe5', '\x3a', '\x68', '\x5d'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["e"].isDouble ());
    ASSERT_DOUBLE_EQ (value["e"].getDouble (), 0.123456789e-12);

    stream.clear ();
    stream.str (std::string ({'\xde', '\x00', '\x01', '\xa1', '\x45', '\xcb', '\x47', '\x03', '\x05', '\x82', '\xff', '\xd7', '\x14', '\x75'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["E"].isDouble ());
    ASSERT_DOUBLE_EQ (value["E"].getDouble (), 1.234567890E+34);

    stream.clear ();
    stream.str (std::string ({'\xdf', '\x00', '\x00', '\x00', '\x01', '\xa0', '\xcb', '\x4f', '\xc9', '\xee', '\x09', '\x3a', '\x64', '\xb8', '\x54'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[""].isDouble ());
    ASSERT_DOUBLE_EQ (value[""].getDouble (), 23456789012E66);

    stream.clear ();
    stream.str (std::string ({'\x81', '\xa4', '\x7a', '\x65', '\x72', '\x6f', '\x00'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["zero"].isInt ());
    ASSERT_EQ (value["zero"], 0);

    stream.clear ();
    stream.str (std::string ({'\xde', '\x00', '\x01', '\xa3', '\x6f', '\x6e', '\x65', '\x01'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["one"].isInt ());
    ASSERT_EQ (value["one"], 1);

    stream.clear ();
    stream.str (std::string ({'\xdf', '\x00', '\x00', '\x00', '\x01', '\xa5', '\x73', '\x70', '\x61', '\x63', '\x65', '\xa1', '\x20'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["space"].isString ());
    ASSERT_EQ (value["space"], " ");

    stream.clear ();
    stream.str (std::string ({'\x81', '\xa5', '\x71', '\x75', '\x6f', '\x74', '\x65', '\xa1', '\x22'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["quote"].isString ());
    ASSERT_EQ (value["quote"], "\"");

    stream.clear ();
    stream.str (std::string ({'\xde', '\x00', '\x01', '\xa9', '\x62', '\x61', '\x63', '\x6b', '\x73', '\x6c', '\x61', '\x73', '\x68', '\xa1', '\x5c'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["backslash"].isString ());
    ASSERT_EQ (value["backslash"], "\\");

    stream.clear ();
    stream.str (std::string ({'\xdf', '\x00', '\x00', '\x00', '\x01', '\xa8', '\x63', '\x6f', '\x6e', '\x74', '\x72', '\x6f', '\x6c', '\x73', '\xa5', '\x08', '\x0c', '\x0a', '\x0d', '\x09'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["controls"].isString ());
    ASSERT_EQ (value["controls"], "\b\f\n\r\t");

    stream.clear ();
    stream.str (std::string ({'\x81', '\xa5', '\x73', '\x6c', '\x61', '\x73', '\x68', '\xa6', '\x2f', '\x20', '\x26', '\x20', '\x5c', '\x2f'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["slash"].isString ());
    ASSERT_EQ (value["slash"], "/ & \\/");

    stream.clear ();
    stream.str (std::string ({'\xde', '\x00', '\x01', '\xa5', '\x61', '\x6c', '\x70', '\x68', '\x61', '\xb9', '\x61', '\x62', '\x63', '\x64', '\x65', '\x66', '\x67', '\x68',
                              '\x69', '\x6a', '\x6b', '\x6c', '\x6d', '\x6e', '\x6f', '\x70', '\x71', '\x72', '\x73', '\x74', '\x75', '\x76', '\x77', '\x79', '\x7a'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["alpha"].isString ());
    ASSERT_EQ (value["alpha"], "abcdefghijklmnopqrstuvwyz");

    stream.clear ();
    stream.str (std::string ({'\xdf', '\x00', '\x00', '\x00', '\x01', '\xa5', '\x41', '\x4c', '\x50', '\x48', '\x41', '\xb9', '\x41', '\x42', '\x43', '\x44', '\x45', '\x46', '\x47', '\x48',
                              '\x49', '\x4a', '\x4b', '\x4c', '\x4d', '\x4e', '\x4f', '\x50', '\x51', '\x52', '\x53', '\x54', '\x55', '\x56', '\x57', '\x59', '\x5a'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["ALPHA"].isString ());
    ASSERT_EQ (value["ALPHA"], "ABCDEFGHIJKLMNOPQRSTUVWYZ");

    stream.clear ();
    stream.str (std::string ({'\xdf', '\x00', '\x00', '\x00', '\x01', '\xa5', '\x64', '\x69', '\x67', '\x69', '\x74', '\xaa', '\x30', '\x31', '\x32', '\x33', '\x34', '\x35', '\x36', '\x37',
                              '\x38', '\x39'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["digit"].isString ());
    ASSERT_EQ (value["digit"], "0123456789");

    stream.clear ();
    stream.str (std::string ({'\xdf', '\x00', '\x00', '\x00', '\x01', '\xaa', '\x30', '\x31', '\x32', '\x33', '\x34', '\x35', '\x36', '\x37', '\x38', '\x39', '\xa5', '\x64', '\x69', '\x67',
                              '\x69', '\x74'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["0123456789"].isString ());
    ASSERT_EQ (value["0123456789"], "digit");

    stream.clear ();
    stream.str (std::string ({'\xdf', '\x00', '\x00', '\x00', '\x01', '\xa7', '\x73', '\x70', '\x65', '\x63', '\x69', '\x61', '\x6c', '\xbf', '\x60', '\x31', '\x7e', '\x21', '\x40', '\x23',
                              '\x24', '\x25', '\x5e', '\x26', '\x2a', '\x28', '\x29', '\x5f', '\x2b', '\x2d', '\x3d', '\x7b', '\x27', '\x3a', '\x5b', '\x2c', '\x5d', '\x7d', '\x7c', '\x3b',
                              '\x2e', '\x3c', '\x2f', '\x3e', '\x3f'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["special"].isString ());
    ASSERT_EQ (value["special"], "`1~!@#$%^&*()_+-={':[,]}|;.</>?");

    stream.clear ();
    stream.str (std::string ({'\xdf', '\x00', '\x00', '\x00', '\x01', '\xa3', '\x68', '\x65', '\x78', '\xb1', '\xc4', '\xa3', '\xe4', '\x95', '\xa7', '\xe8', '\xa6', '\xab', '\xec', '\xb7',
                              '\xaf', '\xea', '\xaf', '\x8d', '\xee', '\xbd', '\x8a'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["hex"].isString ());
    ASSERT_EQ (value["hex"].getString (), std::string ({'\xC4', '\xA3', '\xE4', '\x95', '\xA7', '\xE8', '\xA6', '\xAB', '\xEC', '\xB7', '\xAF', '\xEA', '\xAF', '\x8D', '\xEE', '\xBD', '\x8A'}));

    stream.clear ();
    stream.str (std::string ({'\xdf', '\x00', '\x00', '\x00', '\x01', '\xa4', '\x74', '\x72', '\x75', '\x65', '\xc3'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["true"].isBool ());
    ASSERT_EQ (value["true"], true);

    stream.clear ();
    stream.str (std::string ({'\xdf', '\x00', '\x00', '\x00', '\x01', '\xa5', '\x66', '\x61', '\x6c', '\x73', '\x65', '\xc2'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["false"].isBool ());
    ASSERT_EQ (value["false"], false);

    stream.clear ();
    stream.str (std::string ({'\xdf', '\x00', '\x00', '\x00', '\x01', '\xa4', '\x6e', '\x75', '\x6c', '\x6c', '\xc0'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["null"].isNull ());
    ASSERT_EQ (value["null"], nullptr);

    stream.clear ();
    stream.str (std::string ({'\xdf', '\x00', '\x00', '\x00', '\x01', '\xa5', '\x61', '\x72', '\x72', '\x61', '\x79', '\xdd', '\x00', '\x00', '\x00', '\x00'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["array"].isArray ());
    ASSERT_TRUE (value["array"].empty ());

    stream.clear ();
    stream.str (std::string ({'\xdf', '\x00', '\x00', '\x00', '\x01', '\xa6', '\x6f', '\x62', '\x6a', '\x65', '\x63', '\x74', '\xdf', '\x00', '\x00', '\x00', '\x00'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["object"].isObject ());
    ASSERT_TRUE (value["object"].empty ());

    stream.clear ();
    stream.str (std::string ({'\xdf', '\x00', '\x00', '\x00', '\x01', '\xa7', '\x61', '\x64', '\x64', '\x72', '\x65', '\x73', '\x73', '\xb3', '\x35', '\x30', '\x20', '\x53', '\x74', '\x2e',
                              '\x20', '\x4a', '\x61', '\x6d', '\x65', '\x73', '\x20', '\x53', '\x74', '\x72', '\x65', '\x65', '\x74'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["address"].isString ());
    ASSERT_EQ (value["address"], "50 St. James Street");

    stream.clear ();
    stream.str (std::string ({'\xdf', '\x00', '\x00', '\x00', '\x01', '\xa3', '\x75', '\x72', '\x6c', '\xbe', '\x68', '\x74', '\x74', '\x70', '\x73', '\x3a', '\x2f', '\x2f', '\x77', '\x77',
                              '\x77', '\x2e', '\x6a', '\x6f', '\x69', '\x6e', '\x66', '\x72', '\x61', '\x6d', '\x65', '\x77', '\x6f', '\x72', '\x6b', '\x2e', '\x6e', '\x65', '\x74', '\x2f'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["url"].isString ());
    ASSERT_EQ (value["url"], "https://www.joinframework.net/");

    stream.clear ();
    stream.str (std::string ({'\xdf', '\x00', '\x00', '\x00', '\x02', '\xa7', '\x63', '\x6f', '\x6d', '\x6d', '\x65', '\x6e', '\x74', '\xad', '\x2f', '\x2f', '\x20', '\x2f', '\x2a', '\x20',
                              '\x3c', '\x21', '\x2d', '\x2d', '\x20', '\x2d', '\x2d', '\xab', '\x23', '\x20', '\x2d', '\x2d', '\x20', '\x2d', '\x2d', '\x3e', '\x20', '\x2a', '\x2f', '\xa1',
                              '\x20'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["comment"].isString ());
    ASSERT_EQ (value["comment"], "// /* <!-- --");
    ASSERT_TRUE (value["# -- --> */"].isString ());
    ASSERT_EQ (value["# -- --> */"], " ");

    stream.clear ();
    stream.str (std::string ({'\xdf', '\x00', '\x00', '\x00', '\x02', '\xad', '\x20', '\x73', '\x20', '\x70', '\x20', '\x61', '\x20', '\x63', '\x20', '\x65', '\x20', '\x64', '\x20', '\xdd',
                              '\x00', '\x00', '\x00', '\x07', '\x01', '\x02', '\x03', '\x04', '\x05', '\x06', '\x07', '\xa7', '\x63', '\x6f', '\x6d', '\x70', '\x61', '\x63', '\x74', '\xdd',
                              '\x00', '\x00', '\x00', '\x07', '\x01', '\x02', '\x03', '\x04', '\x05', '\x06', '\x07'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
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
    stream.str (std::string ({'\xdf', '\x00', '\x00', '\x00', '\x01', '\xb4', '\x6f', '\x62', '\x6a', '\x65', '\x63', '\x74', '\x20', '\x77', '\x69', '\x74', '\x68', '\x20', '\x31', '\x20',
                              '\x6d', '\x65', '\x6d', '\x62', '\x65', '\x72', '\xdd', '\x00', '\x00', '\x00', '\x01', '\xb4', '\x61', '\x72', '\x72', '\x61', '\x79', '\x20', '\x77', '\x69',
                              '\x74', '\x68', '\x20', '\x31', '\x20', '\x65', '\x6c', '\x65', '\x6d', '\x65', '\x6e', '\x74'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["object with 1 member"].isArray ());
    ASSERT_EQ (value["object with 1 member"][0], "array with 1 element");

    stream.clear ();
    stream.str (std::string ({'\xdf', '\x00', '\x00', '\x00', '\x01', '\xa6', '\x71', '\x75', '\x6f', '\x74', '\x65', '\x73', '\xbb', '\x26', '\x23', '\x33', '\x34', '\x3b', '\x20', '\x22',
                              '\x20', '\x25', '\x32', '\x32', '\x20', '\x30', '\x78', '\x32', '\x32', '\x20', '\x30', '\x33', '\x34', '\x20', '\x26', '\x23', '\x78', '\x32', '\x32', '\x3b'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["quotes"].isString ());
    ASSERT_EQ (value["quotes"], "&#34; \" %22 0x22 034 &#x22;");

    stream.clear ();
    stream.str (std::string ({'\xdf', '\x00', '\x00', '\x00', '\x01', '\xd9', '\x25', '\x22', '\x08', '\x0c', '\x0a', '\x0d', '\x09', '\x60', '\x31', '\x7e', '\x21', '\x40', '\x23', '\x24',
                              '\x25', '\x5e', '\x26', '\x2a', '\x28', '\x29', '\x5f', '\x2b', '\x2d', '\x3d', '\x5b', '\x5d', '\x7b', '\x7d', '\x7c', '\x3b', '\x3a', '\x27', '\x2c', '\x2e',
                              '\x2f', '\x3c', '\x3e', '\x3f', '\xb7', '\x41', '\x20', '\x6b', '\x65', '\x79', '\x20', '\x63', '\x61', '\x6e', '\x20', '\x62', '\x65', '\x20', '\x61', '\x6e',
                              '\x79', '\x20', '\x73', '\x74', '\x72', '\x69', '\x6e', '\x67'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isObject ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value["\"\b\f\n\r\t`1~!@#$%^&*()_+-=[]{}|;:',./<>?"].isString ());
    ASSERT_EQ (value["\"\b\f\n\r\t`1~!@#$%^&*()_+-=[]{}|;:',./<>?"], "A key can be any string");
}

/**
 * @brief Test MessagePack parsing fail.
 */
TEST (PackReader, fail)
{
    std::stringstream stream;
    Value value;

    stream.clear ();
    stream.str (std::string ({'\xd9', '\x32', '\x70', '\x61', '\x79', '\x6c', '\x6f', '\x61', '\x64', '\x20', '\x73', '\x68', '\x6f', '\x75', '\x6c', '\x64', '\x20', '\x62', '\x65', '\x20',
                              '\x61', '\x6e', '\x20', '\x6f', '\x62', '\x6a', '\x65', '\x63', '\x74', '\x20', '\x6f', '\x72', '\x20', '\x61', '\x72', '\x72', '\x61', '\x79', '\x2c', '\x20',
                              '\x6e', '\x6f', '\x74', '\x20', '\x61', '\x20', '\x73', '\x74', '\x72', '\x69', '\x6e', '\x67'}));
    ASSERT_NE (value.deserialize <PackReader> (stream), 0);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xdd', '\x00', '\x00', '\x00', '\x01', '\xdd', '\x00', '\x00', '\x00', '\x01', '\xdd', '\x00', '\x00', '\x00', '\x01',
                              '\xdd', '\x00', '\x00', '\x00', '\x01', '\xdd', '\x00', '\x00', '\x00', '\x01', '\xdd', '\x00', '\x00', '\x00', '\x01', '\xdd', '\x00', '\x00', '\x00', '\x01',
                              '\xdd', '\x00', '\x00', '\x00', '\x01', '\xdd', '\x00', '\x00', '\x00', '\x01', '\xdd', '\x00', '\x00', '\x00', '\x01', '\xdd', '\x00', '\x00', '\x00', '\x01',
                              '\xdd', '\x00', '\x00', '\x00', '\x01', '\xdd', '\x00', '\x00', '\x00', '\x01', '\xdd', '\x00', '\x00', '\x00', '\x01', '\xdd', '\x00', '\x00', '\x00', '\x01',
                              '\xdd', '\x00', '\x00', '\x00', '\x01', '\xdd', '\x00', '\x00', '\x00', '\x01', '\xdd', '\x00', '\x00', '\x00', '\x01', '\xdd', '\x00', '\x00', '\x00', '\x01',
                              '\xa8', '\x54', '\x6f', '\x6f', '\x20', '\x64', '\x65', '\x65', '\x70'}));
    ASSERT_NE (value.deserialize <PackReader> (stream), 0);
}

/**
 * @brief Test MessagePack parse double.
 */
TEST (PackReader, dbl)
{
    std::stringstream stream;
    Value value;

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), 0.0);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x80', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), -0.0);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x3f', '\xf0', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), 1.0);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\xbf', '\xf0', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), -1.0);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x3f', '\xf8', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), 1.5);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\xbf', '\xf8', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), -1.5);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x40', '\x09', '\x21', '\xff', '\x2e', '\x48', '\xe8', '\xa7'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), 3.1416);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x42', '\x02', '\xa0', '\x5f', '\x20', '\x00', '\x00', '\x00'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), 1E10);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x3d', '\xdb', '\x7C', '\xdf', '\xd9', '\xd7', '\xbd', '\xbb'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), 1E-10);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\xc2', '\x02', '\xa0', '\x5f', '\x20', '\x00', '\x00', '\x00'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), -1E10);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\xbd', '\xdb', '\x7c', '\xdf', '\xd9', '\xd7', '\xbd', '\xbb'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), -1E-10);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x42', '\x06', '\xfc', '\x2b', '\xa8', '\x00', '\x00', '\x00'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), 1.234E+10);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x3d', '\xe0', '\xf5', '\xc0', '\x63', '\x56', '\x43', '\xa8'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), 1.234E-10);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x7f', '\xef', '\xff', '\xfc', '\x57', '\xca', '\x82', '\xae'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), 1.79769e+308);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x00', '\x0f', '\xff', '\xfe', '\x2e', '\x81', '\x59', '\xd0'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), 2.22507e-308);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\xff', '\xef', '\xff', '\xfc', '\x57', '\xca', '\x82', '\xae'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), -1.79769e+308);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x80', '\x0f', '\xff', '\xfe', '\x2e', '\x81', '\x59', '\xd0'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), -2.22507e-308);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x80', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x01'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), -4.9406564584124654e-324);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x00', '\x0f', '\xff', '\xff', '\xff', '\xff', '\xff', '\xff'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), 2.2250738585072009e-308);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x00', '\x10', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), 2.2250738585072014e-308);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x7f', '\xef', '\xff', '\xff', '\xff', '\xff', '\xff', '\xff'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), 1.7976931348623157e+308);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), 0.0);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x3f', '\xef', '\x93', '\xe0', '\x00', '\x00', '\x00', '\x00'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), 0.9868011474609375);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x47', '\x6d', '\x9c', '\x75', '\xd3', '\xac', '\x07', '\x2b'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), 123e34);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x44', '\x03', '\xe9', '\x61', '\xfa', '\x3b', '\xa6', '\xa0'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), 45913141877270640000.0);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x00', '\x0f', '\xff', '\xff', '\xff', '\xff', '\xff', '\xff'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), 2.2250738585072011e-308);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), 0.0);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), 0.0);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x7f', '\xef', '\xff', '\xff', '\xff', '\xff', '\xff', '\xff'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), 1.7976931348623157e+308);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x00', '\x10', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), 2.2250738585072014e-308);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x3f', '\xf0', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), 1.0);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x3f', '\xef', '\xff', '\xff', '\xff', '\xff', '\xff', '\xff'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), 0.99999999999999989);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x3f', '\xf0', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), 1.0);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x3f', '\xf0', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), 1.0);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x3f', '\xf0', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), 1.0);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x3f', '\xf0', '\x00', '\x00', '\x00', '\x00', '\x00', '\x01'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), 1.00000000000000022);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x43', '\x6f', '\xff', '\xff', '\xff', '\xff', '\xff', '\xff'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), 72057594037927928.0);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x43', '\x70', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), 72057594037927936.0);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x43', '\x70', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), 72057594037927936.0);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x43', '\x6f', '\xff', '\xff', '\xff', '\xff', '\xff', '\xff'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), 72057594037927928.0);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x43', '\x70', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), 72057594037927936.0);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x43', '\xdf', '\xff', '\xff', '\xff', '\xff', '\xff', '\xff'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), 9223372036854774784.0);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x43', '\xe0', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), 9223372036854775808.0);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x43', '\xe0', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), 9223372036854775808.0);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x43', '\xdf', '\xff', '\xff', '\xff', '\xff', '\xff', '\xff'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), 9223372036854774784.0);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x43', '\xe0', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), 9223372036854775808.0);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x46', '\x5f', '\xff', '\xff', '\xff', '\xff', '\xff', '\xff'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), 10141204801825834086073718800384.0);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x46', '\x60', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), 10141204801825835211973625643008.0);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x49', '\x6f', '\xff', '\xff', '\xff', '\xff', '\xff', '\xff'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), 5708990770823838890407843763683279797179383808.0);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x49', '\x70', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), 5708990770823839524233143877797980545530986496.0);

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xcb', '\x00', '\x10', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isDouble ());
    EXPECT_EQ (value[0].getDouble (), 2.2250738585072014e-308);
}

/**
 * @brief Test MessagePack parse string.
 */
TEST (PackReader, str)
{
    std::stringstream stream;
    Value value;

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xa0'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isString ());
    EXPECT_STREQ (value[0].getString ().c_str (), "");

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xda', '\x00', '\x05', '\x48', '\x65', '\x6c', '\x6c', '\x6f'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isString ());
    EXPECT_STREQ (value[0].getString ().c_str (), "Hello");

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xdb', '\x00', '\x00', '\x00', '\x0b', '\x48', '\x65', '\x6c', '\x6c', '\x6f', '\x0a', '\x57', '\x6f', '\x72', '\x6c', '\x64'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isString ());
    EXPECT_STREQ (value[0].getString ().c_str (), "Hello\nWorld");

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xab', '\x48', '\x65', '\x6c', '\x6c', '\x6f', '\x00', '\x57', '\x6f', '\x72', '\x6c', '\x64'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isString ());
    EXPECT_STREQ (value[0].getString ().c_str (), "Hello\0World");

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xda', '\x00', '\x08', '\x22', '\x5c', '\x2f', '\x08', '\x0c', '\x0a', '\x0d', '\x09'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isString ());
    EXPECT_STREQ (value[0].getString ().c_str (), "\"\\/\b\f\n\r\t");

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xdb', '\x00', '\x00', '\x00', '\x01', '\x24'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isString ());
    EXPECT_STREQ (value[0].getString ().c_str (), "\x24");

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xa2', '\xc2', '\xa2'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isString ());
    EXPECT_STREQ (value[0].getString ().c_str (), "\xC2\xA2");

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xda', '\x00', '\x03', '\xe2', '\x82', '\xac'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isString ());
    EXPECT_STREQ (value[0].getString ().c_str (), "\xE2\x82\xAC");

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xdb', '\x00', '\x00', '\x00', '\x04', '\xf0', '\x9d', '\x84', '\x9e'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isString ());
    EXPECT_STREQ (value[0].getString ().c_str (), "\xF0\x9D\x84\x9E");
}

/**
 * @brief Test MessagePack parse bin.
 */
TEST (PackReader, bin)
{
    std::stringstream stream;
    Value value;

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xc4', '\x00'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isString ());
    EXPECT_STREQ (value[0].getString ().c_str (), "");

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xc5', '\x00', '\x05', '\x48', '\x65', '\x6c', '\x6c', '\x6f'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isString ());
    EXPECT_STREQ (value[0].getString ().c_str (), "Hello");

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xc6', '\x00', '\x00', '\x00', '\x0b', '\x48', '\x65', '\x6c', '\x6c', '\x6f', '\x0a', '\x57', '\x6f', '\x72', '\x6c', '\x64'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isString ());
    EXPECT_STREQ (value[0].getString ().c_str (), "Hello\nWorld");

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xc4', '\x0b', '\x48', '\x65', '\x6c', '\x6c', '\x6f', '\x00', '\x57', '\x6f', '\x72', '\x6c', '\x64'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isString ());
    EXPECT_STREQ (value[0].getString ().c_str (), "Hello\0World");

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xc5', '\x00', '\x08', '\x22', '\x5c', '\x2f', '\x08', '\x0c', '\x0a', '\x0d', '\x09'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isString ());
    EXPECT_STREQ (value[0].getString ().c_str (), "\"\\/\b\f\n\r\t");

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xc6', '\x00', '\x00', '\x00', '\x01', '\x24'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isString ());
    EXPECT_STREQ (value[0].getString ().c_str (), "\x24");

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xc4', '\x02', '\xc2', '\xa2'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isString ());
    EXPECT_STREQ (value[0].getString ().c_str (), "\xC2\xA2");

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xc5', '\x00', '\x03', '\xe2', '\x82', '\xac'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
    ASSERT_TRUE (value.isArray ());
    ASSERT_FALSE (value.empty ());
    ASSERT_TRUE (value[0].isString ());
    EXPECT_STREQ (value[0].getString ().c_str (), "\xE2\x82\xAC");

    stream.clear ();
    stream.str (std::string ({'\xdd', '\x00', '\x00', '\x00', '\x01', '\xc6', '\x00', '\x00', '\x00', '\x04', '\xf0', '\x9d', '\x84', '\x9e'}));
    ASSERT_EQ (value.deserialize <PackReader> (stream), 0) << join::lastError.message ();
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
