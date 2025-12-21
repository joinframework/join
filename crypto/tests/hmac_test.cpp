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
#include <join/hmac.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::BytesArray;
using join::Digest;
using join::Hmac;

/// sample text.
const std::string sample = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. "
                           "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. "
                           "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. "
                           "Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";

/// key.
const std::string key (65, 'a');

/// keyed-hash message authentication code in binary format.
BytesArray md5bin    = {
    0xd3, 0x8c, 0xc1, 0x49, 0x08, 0xac, 0xac, 0xa1,
    0x1f, 0x83, 0xf9, 0x83, 0x37, 0xea, 0x76, 0x14
};
BytesArray sha1bin   = {
    0xea, 0x27, 0x9e, 0x14, 0xcd, 0xbf, 0xb6, 0x8d,
    0x30, 0xe2, 0xfe, 0xfc, 0x5a, 0xa7, 0x13, 0x25,
    0xf4, 0x84, 0x5a, 0xbd
};
BytesArray sha224bin = {
    0x2b, 0x56, 0xd6, 0xbe, 0x15, 0x51, 0x12, 0x24,
    0xc0, 0x6f, 0x29, 0xe8, 0x29, 0x92, 0xff, 0x1f,
    0xf1, 0x93, 0xe4, 0xe4, 0x52, 0xa0, 0x36, 0xd2,
    0x73, 0x8d, 0xf9, 0x58
};
BytesArray sha256bin = {
    0xf7, 0xd2, 0x89, 0xa6, 0x28, 0x04, 0x33, 0xfb,
    0xe7, 0x7d, 0x76, 0xfb, 0xf6, 0x65, 0x1a, 0x09,
    0x7e, 0x9b, 0x6a, 0x53, 0xb9, 0x4d, 0x6d, 0x95,
    0x8f, 0xb4, 0x57, 0x95, 0x93, 0x45, 0x41, 0x95
};
BytesArray sha384bin = {
    0x6a, 0x16, 0x63, 0x64, 0x4d, 0x12, 0x76, 0xcf,
    0x28, 0xb2, 0xd5, 0xce, 0x05, 0x99, 0x62, 0x30,
    0x2f, 0xa5, 0x6a, 0x05, 0x7c, 0x83, 0x0c, 0x17,
    0xd6, 0xd3, 0x2a, 0x45, 0xe6, 0x08, 0xd6, 0x57,
    0x19, 0x25, 0x4a, 0x0f, 0x04, 0x33, 0x72, 0xb5,
    0xd4, 0xab, 0x31, 0x88, 0xc0, 0xfe, 0x1b, 0xab
};
BytesArray sha512bin = {
    0x9d, 0x73, 0xa7, 0x16, 0x0b, 0x32, 0xd2, 0x50,
    0xc4, 0xa9, 0x2f, 0xcc, 0x07, 0x0a, 0xa7, 0x4c,
    0x86, 0x5c, 0xf3, 0xc7, 0xd4, 0xfe, 0xc0, 0xfc,
    0xff, 0xb8, 0xed, 0x36, 0x52, 0x0d, 0x25, 0x5c,
    0xf1, 0x2e, 0x4f, 0xaf, 0x99, 0x49, 0x6e, 0xef,
    0x8a, 0x97, 0x86, 0x47, 0xda, 0x85, 0xaf, 0x8c,
    0xa8, 0xc6, 0x45, 0x81, 0x5e, 0x38, 0x65, 0xa3,
    0x05, 0x06, 0xeb, 0xb9, 0x01, 0x2c, 0xbc, 0x5c
};
BytesArray sm3bin = {
    0x10, 0xd9, 0x55, 0x29, 0xe4, 0xd8, 0xbb, 0x1f,
    0xc9, 0x85, 0x14, 0x15, 0x1a, 0x36, 0x1a, 0xc5,
    0xf3, 0xad, 0x58, 0xf0, 0x4e, 0xd5, 0xdb, 0x4a,
    0xff, 0xc8, 0x45, 0x9d, 0x5e, 0x63, 0xa6, 0x97
};

/// keyed-hash message authentication code in hexadecimal format.
const std::string md5hex    = "d38cc14908acaca11f83f98337ea7614";
const std::string sha1hex   = "ea279e14cdbfb68d30e2fefc5aa71325f4845abd";
const std::string sha224hex = "2b56d6be15511224c06f29e82992ff1ff193e4e452a036d2738df958";
const std::string sha256hex = "f7d289a6280433fbe77d76fbf6651a097e9b6a53b94d6d958fb4579593454195";
const std::string sha384hex = "6a1663644d1276cf28b2d5ce059962302fa56a057c830c17d6d32a45e608d65719254a0f043372b5d4ab3188c0fe1bab";
const std::string sha512hex = "9d73a7160b32d250c4a92fcc070aa74c865cf3c7d4fec0fcffb8ed36520d255cf12e4faf99496eef8a978647da85af8ca8c645815e3865a30506ebb9012cbc5c";
const std::string sm3hex    = "10d95529e4d8bb1fc98514151a361ac5f3ad58f04ed5db4affc8459d5e63a697";

/**
 * @brief Test finalize.
 */
TEST (Hmac, finalize)
{
#if OPENSSL_VERSION_NUMBER < 0x30000000L
    ASSERT_THROW (Hmac (Digest::Algorithm (0), key), std::system_error);
#endif

    Hmac tmp (Digest::Algorithm::MD5, key);
    Hmac hmac = std::move (tmp);
    hmac << sample;
    ASSERT_EQ (hmac.finalize (), md5bin);
    hmac.write (sample.data (), sample.size ());
    ASSERT_EQ (hmac.finalize (), md5bin);

    hmac = Hmac (Digest::Algorithm::SHA1, key);
    hmac << sample;
    ASSERT_EQ (hmac.finalize (), sha1bin);
    hmac.write (sample.data (), sample.size ());
    ASSERT_EQ (hmac.finalize (), sha1bin);

    hmac = Hmac (Digest::Algorithm::SHA224, key);
    hmac << sample;
    ASSERT_EQ (hmac.finalize (), sha224bin);
    hmac.write (sample.data (), sample.size ());
    ASSERT_EQ (hmac.finalize (), sha224bin);

    hmac = Hmac (Digest::Algorithm::SHA256, key);
    hmac << sample;
    ASSERT_EQ (hmac.finalize (), sha256bin);
    hmac.write (sample.data (), sample.size ());
    ASSERT_EQ (hmac.finalize (), sha256bin);

    hmac = Hmac (Digest::Algorithm::SHA384, key);
    hmac << sample;
    ASSERT_EQ (hmac.finalize (), sha384bin);
    hmac.write (sample.data (), sample.size ());
    ASSERT_EQ (hmac.finalize (), sha384bin);

    hmac = Hmac (Digest::Algorithm::SHA512, key);
    hmac << sample;
    ASSERT_EQ (hmac.finalize (), sha512bin);
    hmac.write (sample.data (), sample.size ());
    ASSERT_EQ (hmac.finalize (), sha512bin);

    hmac = Hmac (Digest::Algorithm::SM3, key);
    hmac << sample;
    ASSERT_EQ (hmac.finalize (), sm3bin);
    hmac.write (sample.data (), sample.size ());
    ASSERT_EQ (hmac.finalize (), sm3bin);
}

/**
 * @brief Test md5bin.
 */
TEST (Hmac, md5bin)
{
    ASSERT_EQ (Hmac::md5bin (sample, key), md5bin);
}

/**
 * @brief Test md5hex.
 */
TEST (Hmac, md5hex)
{
    ASSERT_EQ (Hmac::md5hex (sample, key), md5hex);
}

/**
 * @brief Test sha1bin.
 */
TEST (Hmac, sha1bin)
{
    ASSERT_EQ (Hmac::sha1bin (sample, key), sha1bin);
}

/**
 * @brief Test sha1hex.
 */
TEST (Hmac, sha1hex)
{
    ASSERT_EQ (Hmac::sha1hex (sample, key), sha1hex);
}

/**
 * @brief Test sha224bin.
 */
TEST (Hmac, sha224bin)
{
    ASSERT_EQ (Hmac::sha224bin (sample, key), sha224bin);
}

/**
 * @brief Test sha224hex.
 */
TEST (Hmac, sha224hex)
{
    ASSERT_EQ (Hmac::sha224hex (sample, key), sha224hex);
}

/**
 * @brief Test sha256bin.
 */
TEST (Hmac, sha256bin)
{
    ASSERT_EQ (Hmac::sha256bin (sample, key), sha256bin);
}

/**
 * @brief Test sha256hex.
 */
TEST (Hmac, sha256hex)
{
    ASSERT_EQ (Hmac::sha256hex (sample, key), sha256hex);
}

/**
 * @brief Test sha384bin.
 */
TEST (Hmac, sha384bin)
{
    ASSERT_EQ (Hmac::sha384bin (sample, key), sha384bin);
}

/**
 * @brief Test sha384hex.
 */
TEST (Hmac, sha384hex)
{
    ASSERT_EQ (Hmac::sha384hex (sample, key), sha384hex);
}

/**
 * @brief Test sha512bin.
 */
TEST (Hmac, sha512bin)
{
    ASSERT_EQ (Hmac::sha512bin (sample, key), sha512bin);
}

/**
 * @brief Test sha512hex.
 */
TEST (Hmac, sha512hex)
{
    ASSERT_EQ (Hmac::sha512hex (sample, key), sha512hex);
}

/**
 * @brief Test sm3bin.
 */
TEST (Hmac, sm3bin)
{
    ASSERT_EQ (Hmac::sm3bin (sample, key), sm3bin);
}

/**
 * @brief Test sm3hex.
 */
TEST (Hmac, sm3hex)
{
    ASSERT_EQ (Hmac::sm3hex (sample, key), sm3hex);
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    join::initializeOpenSSL ();
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
