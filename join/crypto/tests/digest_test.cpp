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

using join::Base64;
using join::BytesArray;
using join::Digest;

/// sample text.
const std::string sample = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. "
                           "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. "
                           "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. "
                           "Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.\n";

/// signatures.
const std::string sha1digest   = "4518012e1b365e504001dbc94120624f15b8bbd5";
const std::string sha224digest = "95d3938a52cba86a6c61b95ab975e16b0a989cc9be584c86a1eb3ef4";
const std::string sha256digest = "56293a80e0394d252e995f2debccea8223e4b5b2b150bee212729b3b39ac4d46";
const std::string sha384digest = "84d998482e9ec38a8877f1624a193cfa803611190e19cc6bca2791492265f0446544bdb285d89a8bfce2826077e11cd2";
const std::string sha512digest = "0b7b28ca2bf28e253929c8a29ddb0ac2a39226f86702ad1b1e51703d5dcebd42aff774969bb7e23bf6c439bab4eae37cdfc86978a176c27e835cdef9c8aaf7de";
const std::string sm3digest    = "0c3ac7cfbffb0593bc1f9d7778f38ba334ab9c2e46617de6d5e0455d668c6a65";

/**
 * @brief Test get.
 */
TEST (Digest, get)
{
    ASSERT_THROW (Digest (Digest::Algorithm (0)), std::system_error);

    Digest digest = std::move (Digest (Digest::Algorithm::SHA1));
    digest << sample;
    ASSERT_EQ (Digest::bin2hex (digest.get ()), sha1digest);
    digest.write (sample.data (), sample.size ());
    ASSERT_EQ (Digest::bin2hex (digest.get ()), sha1digest);

    digest = std::move (Digest (Digest::Algorithm::SHA224));
    digest << sample;
    ASSERT_EQ (Digest::bin2hex (digest.get ()), sha224digest);
    digest.write (sample.data (), sample.size ());
    ASSERT_EQ (Digest::bin2hex (digest.get ()), sha224digest);

    digest = std::move (Digest (Digest::Algorithm::SHA256));
    digest << sample;
    ASSERT_EQ (Digest::bin2hex (digest.get ()), sha256digest);
    digest.write (sample.data (), sample.size ());
    ASSERT_EQ (Digest::bin2hex (digest.get ()), sha256digest);

    digest = std::move (Digest (Digest::Algorithm::SHA384));
    digest << sample;
    ASSERT_EQ (Digest::bin2hex (digest.get ()), sha384digest);
    digest.write (sample.data (), sample.size ());
    ASSERT_EQ (Digest::bin2hex (digest.get ()), sha384digest);

    digest = std::move (Digest (Digest::Algorithm::SHA512));
    digest << sample;
    ASSERT_EQ (Digest::bin2hex (digest.get ()), sha512digest);
    digest.write (sample.data (), sample.size ());
    ASSERT_EQ (Digest::bin2hex (digest.get ()), sha512digest);

    digest = std::move (Digest (Digest::Algorithm::SM3));
    digest << sample;
    ASSERT_EQ (Digest::bin2hex (digest.get ()), sm3digest);
    digest.write (sample.data (), sample.size ());
    ASSERT_EQ (Digest::bin2hex (digest.get ()), sm3digest);
}

/**
 * @brief Test sha1.
 */
TEST (Digest, sha1)
{
    ASSERT_EQ (Digest::bin2hex (Digest::sha1 (sample)), sha1digest);
}

/**
 * @brief Test sha224.
 */
TEST (Digest, sha224)
{
    ASSERT_EQ (Digest::bin2hex (Digest::sha224 (sample)), sha224digest);
}

/**
 * @brief Test sha256.
 */
TEST (Digest, sha256)
{
    ASSERT_EQ (Digest::bin2hex (Digest::sha256 (sample)), sha256digest);
}

/**
 * @brief Test sha384.
 */
TEST (Digest, sha384)
{
    ASSERT_EQ (Digest::bin2hex (Digest::sha384 (sample)), sha384digest);
}

/**
 * @brief Test sha512.
 */
TEST (Digest, sha512)
{
    ASSERT_EQ (Digest::bin2hex (Digest::sha512 (sample)), sha512digest);
}

/**
 * @brief Test sm3.
 */
TEST (Digest, sm3)
{
    ASSERT_EQ (Digest::bin2hex (Digest::sm3 (sample)), sm3digest);
}

/**
 * @brief Test algorithm.
 */
TEST (Digest, algorithm)
{
    ASSERT_STREQ (Digest::algorithm (Digest::Algorithm::SHA1), "SHA1");
    ASSERT_STREQ (Digest::algorithm (Digest::Algorithm::SHA224), "SHA224");
    ASSERT_STREQ (Digest::algorithm (Digest::Algorithm::SHA256), "SHA256");
    ASSERT_STREQ (Digest::algorithm (Digest::Algorithm::SHA384), "SHA384");
    ASSERT_STREQ (Digest::algorithm (Digest::Algorithm::SHA512), "SHA512");
    ASSERT_STREQ (Digest::algorithm (Digest::Algorithm::SM3), "SM3");
    ASSERT_STREQ (Digest::algorithm (Digest::Algorithm (100)), "UNKNOWN");
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
