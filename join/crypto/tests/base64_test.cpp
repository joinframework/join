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
#include <join/base64.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::Base64;
using join::BytesArray;

/// strings to encode.
const std::string decodedString1 = "this is the string to encode";
const std::string decodedString2 = "this is the string to encode !!!";

/// arrays to encode.
const BytesArray decodedArray1 = BytesArray (decodedString1.begin (), decodedString1.end ());
const BytesArray decodedArray2 = BytesArray (decodedString2.begin (), decodedString2.end ());

/// strings to decode.
const std::string encodedString1 = "dGhpcyBpcyB0aGUgc3RyaW5nIHRvIGVuY29kZQ==";
const std::string encodedString2 = "dGhpcyBpcyB0aGUgc3RyaW5nIHRvIGVuY29kZSAhISE=";

/**
 * @brief base64 encoding test.
 */
TEST (Base64, encode)
{
    EXPECT_EQ (encodedString1, Base64::encode (decodedString1));
    EXPECT_EQ (encodedString2, Base64::encode (decodedString2));
    EXPECT_EQ (encodedString1, Base64::encode (decodedArray1));
    EXPECT_EQ (encodedString2, Base64::encode (decodedArray2));
    EXPECT_EQ (encodedString1, Base64::encode (decodedArray1.data (), decodedArray1.size ()));
    EXPECT_EQ (encodedString2, Base64::encode (decodedArray2.data (), decodedArray2.size ()));
}

/**
 * @brief base64 decoding test.
 */
TEST (Base64, decode)
{
    EXPECT_EQ (decodedArray1, Base64::decode (encodedString1));
    EXPECT_EQ (decodedArray2, Base64::decode (encodedString2));
    EXPECT_EQ (BytesArray {}, Base64::decode (std::string {}));
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
