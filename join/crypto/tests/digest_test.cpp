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
#include <join/digest.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::BytesArray;
using join::Digest;

/// sample text.
const std::string sample = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. "
                           "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. "
                           "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. "
                           "Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";

/// message digest in binary format.
BytesArray md5bin    = {
    0xdb, 0x89, 0xbb, 0x5c, 0xea, 0xb8, 0x7f, 0x9c,
    0x0f, 0xcc, 0x2a, 0xb3, 0x6c, 0x18, 0x9c, 0x2c
};
BytesArray sha1bin   = {
    0xcd, 0x36, 0xb3, 0x70, 0x75, 0x8a, 0x25, 0x9b,
    0x34, 0x84, 0x50, 0x84, 0xa6, 0xcc, 0x38, 0x47,
    0x3c, 0xb9, 0x5e, 0x27
};
BytesArray sha224bin = {
    0xb2, 0xd9, 0xd4, 0x97, 0xbc, 0xc3, 0xe5, 0xbe,
    0x0c, 0xa6, 0x7f, 0x08, 0xc8, 0x60, 0x87, 0xa5,
    0x13, 0x22, 0xae, 0x48, 0xb2, 0x20, 0xed, 0x92,
    0x41, 0xca, 0xd7, 0xa5
};
BytesArray sha256bin = {
    0x2d, 0x8c, 0x2f, 0x6d, 0x97, 0x8c, 0xa2, 0x17,
    0x12, 0xb5, 0xf6, 0xde, 0x36, 0xc9, 0xd3, 0x1f,
    0xa8, 0xe9, 0x6a, 0x4f, 0xa5, 0xd8, 0xff, 0x8b,
    0x01, 0x88, 0xdf, 0xb9, 0xe7, 0xc1, 0x71, 0xbb
};
BytesArray sha384bin = {
    0xd3, 0xb5, 0x71, 0x0e, 0x17, 0xda, 0x84, 0x21,
    0x6f, 0x1b, 0xf0, 0x80, 0x79, 0xbb, 0xbb, 0xf4,
    0x53, 0x03, 0xba, 0xef, 0xc6, 0xec, 0xd6, 0x77,
    0x91, 0x0a, 0x1c, 0x33, 0xc8, 0x6c, 0xb1, 0x64,
    0x28, 0x1f, 0x0f, 0x2d, 0xca, 0xb5, 0x5b, 0xba,
    0xdc, 0x5e, 0x86, 0x06, 0xbd, 0xbc, 0x16, 0xb6
};
BytesArray sha512bin = {
    0x8b, 0xa7, 0x60, 0xca, 0xc2, 0x9c, 0xb2, 0xb2,
    0xce, 0x66, 0x85, 0x8e, 0xad, 0x16, 0x91, 0x74,
    0x05, 0x7a, 0xa1, 0x29, 0x8c, 0xcd, 0x58, 0x15,
    0x14, 0xe6, 0xdb, 0x6d, 0xee, 0x32, 0x85, 0x28,
    0x0e, 0xe6, 0xe3, 0xa5, 0x4c, 0x93, 0x19, 0x07,
    0x1d, 0xc8, 0x16, 0x5f, 0xf0, 0x61, 0xd7, 0x77,
    0x83, 0x10, 0x0d, 0x44, 0x9c, 0x93, 0x7f, 0xf1,
    0xfb, 0x4c, 0xd1, 0xbb, 0x51, 0x6a, 0x69, 0xb9
};
BytesArray sm3bin = {
    0x72, 0x77, 0xa3, 0x7e, 0x8b, 0xba, 0xf2, 0x27,
    0x50, 0x83, 0x88, 0x3e, 0xa9, 0xd5, 0x63, 0x58,
    0x79, 0x65, 0x1d, 0x54, 0xef, 0xe3, 0x43, 0x43,
    0xaa, 0x69, 0xbd, 0x29, 0xc9, 0xe5, 0x57, 0x0c
};

/// message digest in hexadecimal format.
const std::string md5hex    = "db89bb5ceab87f9c0fcc2ab36c189c2c";
const std::string sha1hex   = "cd36b370758a259b34845084a6cc38473cb95e27";
const std::string sha224hex = "b2d9d497bcc3e5be0ca67f08c86087a51322ae48b220ed9241cad7a5";
const std::string sha256hex = "2d8c2f6d978ca21712b5f6de36c9d31fa8e96a4fa5d8ff8b0188dfb9e7c171bb";
const std::string sha384hex = "d3b5710e17da84216f1bf08079bbbbf45303baefc6ecd677910a1c33c86cb164281f0f2dcab55bbadc5e8606bdbc16b6";
const std::string sha512hex = "8ba760cac29cb2b2ce66858ead169174057aa1298ccd581514e6db6dee3285280ee6e3a54c9319071dc8165ff061d77783100d449c937ff1fb4cd1bb516a69b9";
const std::string sm3hex    = "7277a37e8bbaf2275083883ea9d5635879651d54efe34343aa69bd29c9e5570c";

/**
 * @brief Test finalize.
 */
TEST (Digest, finalize)
{
    ASSERT_THROW (Digest (Digest::Algorithm (0)), std::system_error);

    Digest tmp (Digest::Algorithm::MD5);
    Digest digest = std::move(tmp);
    digest << sample;
    ASSERT_EQ (digest.finalize (), md5bin);
    digest.write (sample.data (), sample.size ());
    ASSERT_EQ (digest.finalize (), md5bin);

    digest = std::move (Digest (Digest::Algorithm::SHA1));
    digest << sample;
    ASSERT_EQ (digest.finalize (), sha1bin);
    digest.write (sample.data (), sample.size ());
    ASSERT_EQ (digest.finalize (), sha1bin);

    digest = std::move (Digest (Digest::Algorithm::SHA224));
    digest << sample;
    ASSERT_EQ (digest.finalize (), sha224bin);
    digest.write (sample.data (), sample.size ());
    ASSERT_EQ (digest.finalize (), sha224bin);

    digest = std::move (Digest (Digest::Algorithm::SHA256));
    digest << sample;
    ASSERT_EQ (digest.finalize (), sha256bin);
    digest.write (sample.data (), sample.size ());
    ASSERT_EQ (digest.finalize (), sha256bin);

    digest = std::move (Digest (Digest::Algorithm::SHA384));
    digest << sample;
    ASSERT_EQ (digest.finalize (), sha384bin);
    digest.write (sample.data (), sample.size ());
    ASSERT_EQ (digest.finalize (), sha384bin);

    digest = std::move (Digest (Digest::Algorithm::SHA512));
    digest << sample;
    ASSERT_EQ (digest.finalize (), sha512bin);
    digest.write (sample.data (), sample.size ());
    ASSERT_EQ (digest.finalize (), sha512bin);

    digest = std::move (Digest (Digest::Algorithm::SM3));
    digest << sample;
    ASSERT_EQ (digest.finalize (), sm3bin);
    digest.write (sample.data (), sample.size ());
    ASSERT_EQ (digest.finalize (), sm3bin);
}

/**
 * @brief Test md5bin.
 */
TEST (Digest, md5bin)
{
    ASSERT_EQ (Digest::md5bin (sample), md5bin);
}

/**
 * @brief Test md5hex.
 */
TEST (Digest, md5hex)
{
    ASSERT_EQ (Digest::md5hex (sample), md5hex);
}

/**
 * @brief Test sha1bin.
 */
TEST (Digest, sha1bin)
{
    ASSERT_EQ (Digest::sha1bin (sample), sha1bin);
}

/**
 * @brief Test sha1hex.
 */
TEST (Digest, sha1hex)
{
    ASSERT_EQ (Digest::sha1hex (sample), sha1hex);
}

/**
 * @brief Test sha224bin.
 */
TEST (Digest, sha224bin)
{
    ASSERT_EQ (Digest::sha224bin (sample), sha224bin);
}

/**
 * @brief Test sha224hex.
 */
TEST (Digest, sha224hex)
{
    ASSERT_EQ (Digest::sha224hex (sample), sha224hex);
}

/**
 * @brief Test sha256bin.
 */
TEST (Digest, sha256bin)
{
    ASSERT_EQ (Digest::sha256bin (sample), sha256bin);
}

/**
 * @brief Test sha256hex.
 */
TEST (Digest, sha256hex)
{
    ASSERT_EQ (Digest::sha256hex (sample), sha256hex);
}

/**
 * @brief Test sha384bin.
 */
TEST (Digest, sha384bin)
{
    ASSERT_EQ (Digest::sha384bin (sample), sha384bin);
}

/**
 * @brief Test sha384hex.
 */
TEST (Digest, sha384hex)
{
    ASSERT_EQ (Digest::sha384hex (sample), sha384hex);
}

/**
 * @brief Test sha512bin.
 */
TEST (Digest, sha512bin)
{
    ASSERT_EQ (Digest::sha512bin (sample), sha512bin);
}

/**
 * @brief Test sha512hex.
 */
TEST (Digest, sha512hex)
{
    ASSERT_EQ (Digest::sha512hex (sample), sha512hex);
}

/**
 * @brief Test sm3bin.
 */
TEST (Digest, sm3bin)
{
    ASSERT_EQ (Digest::sm3bin (sample), sm3bin);
}

/**
 * @brief Test sm3hex.
 */
TEST (Digest, sm3hex)
{
    ASSERT_EQ (Digest::sm3hex (sample), sm3hex);
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
