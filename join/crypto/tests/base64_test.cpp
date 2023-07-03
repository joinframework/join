/**
 * MIT License
 *
 * Copyright (c) 2023 Mathieu Rabine
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

using join::BytesArray;
using join::Encoder;
using join::Decoder;
using join::Base64;

/// strings to encode.
const std::string decodedString = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do "
                                  "eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut e"
                                  "nim ad minim veniam, quis nostrud exercitation ullamco laboris n"
                                  "isi ut aliquip ex ea commodo consequat. Duis aute irure dolor in"
                                  " reprehenderit in voluptate velit esse cillum dolore eu fugiat n"
                                  "ulla pariatur. Excepteur sint occaecat cupidatat non proident, s"
                                  "unt in culpa qui officia deserunt mollit anim id est laborum.";

/// strings to decode.
const std::string encodedString = "TG9yZW0gaXBzdW0gZG9sb3Igc2l0IGFtZXQsIGNvbnNlY3RldHVyIGFkaXBpc2Np"
                                  "bmcgZWxpdCwgc2VkIGRvIGVpdXNtb2QgdGVtcG9yIGluY2lkaWR1bnQgdXQgbGFi"
                                  "b3JlIGV0IGRvbG9yZSBtYWduYSBhbGlxdWEuIFV0IGVuaW0gYWQgbWluaW0gdmVu"
                                  "aWFtLCBxdWlzIG5vc3RydWQgZXhlcmNpdGF0aW9uIHVsbGFtY28gbGFib3JpcyBu"
                                  "aXNpIHV0IGFsaXF1aXAgZXggZWEgY29tbW9kbyBjb25zZXF1YXQuIER1aXMgYXV0"
                                  "ZSBpcnVyZSBkb2xvciBpbiByZXByZWhlbmRlcml0IGluIHZvbHVwdGF0ZSB2ZWxp"
                                  "dCBlc3NlIGNpbGx1bSBkb2xvcmUgZXUgZnVnaWF0IG51bGxhIHBhcmlhdHVyLiBF"
                                  "eGNlcHRldXIgc2ludCBvY2NhZWNhdCBjdXBpZGF0YXQgbm9uIHByb2lkZW50LCBz"
                                  "dW50IGluIGN1bHBhIHF1aSBvZmZpY2lhIGRlc2VydW50IG1vbGxpdCBhbmltIGlk"
                                  "IGVzdCBsYWJvcnVtLg==";

/// arrays to encode.
const BytesArray decodedArray   = BytesArray (decodedString.begin (), decodedString.end ());

/**
 * @brief encoder get test.
 */
TEST (Encoder, get)
{
    Encoder encoder;
    encoder << decodedString;
    ASSERT_EQ (encodedString, encoder.get ());
    encoder.write (decodedString.data (), decodedString.size ());
    ASSERT_EQ (encodedString, encoder.get ());
}

/**
 * @brief decoder get test.
 */
TEST (Decoder, get)
{
    Decoder decoder;
    decoder << encodedString;
    ASSERT_EQ (decodedArray, decoder.get ());
    decoder.write (encodedString.data (), encodedString.size ());
    ASSERT_EQ (decodedArray, decoder.get ());
}

/**
 * @brief base64 encoding test.
 */
TEST (Base64, encode)
{
    ASSERT_EQ (encodedString, Base64::encode (decodedString));
    ASSERT_EQ (encodedString, Base64::encode (decodedArray));
}

/**
 * @brief base64 decoding test.
 */
TEST (Base64, decode)
{
    ASSERT_EQ (decodedArray, Base64::decode (encodedString));
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
