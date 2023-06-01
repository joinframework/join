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
#include <join/signature.hpp>
#include <join/error.hpp>

// Libraries.
#include <gtest/gtest.h>

// C++.
#include <fstream>

using join::Errc;
using join::Base64;
using join::Digest;
using join::DigestErrc;
using join::Signature;
using join::BytesArray;

/**
 * @brief Class used to test the signature API.
 */
class SignatureTest : public ::testing::Test
{
public:
    /**
     * @brief Set up test case.
     */
    static void SetUpTestCase ()
    {
        // write keys to file system.
        writeFile (ecPriKeyPath, ecPriKey);
        writeFile (ecPubKeyPath, ecPubKey);
    }

    /**
     * @brief Tear down test case.
     */
    static void TearDownTestCase ()
    {
        // remove keys from file system.
        ::remove (ecPriKeyPath.c_str ());
        ::remove (ecPubKeyPath.c_str ());
    }

    /**
     * @brief Write a file on file system.
     * @param path File location.
     * @param data string to write.
     */
    static void writeFile (const std::string& path, const std::string& data)
    {
        std::ofstream file (path.c_str (), std::ios::out | std::ios::binary);
        file.write (data.data (), data.size ());
    }

protected:
    /// sample text.
    static const std::string sample;

    /// key paths.
    static const std::string ecPriKeyPath;
    static const std::string ecPubKeyPath;

    /// keys.
    static const std::string ecPriKey;
    static const std::string ecPubKey;

    /// signatures.
    static const std::string ec224sig;
    static const std::string ec256sig;
    static const std::string ec384sig;
    static const std::string ec512sig;
};

/// sample text.
const std::string SignatureTest::sample = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. "
                                          "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. "
                                          "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. "
                                          "Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.\n";

/// key paths.
const std::string SignatureTest::ecPriKeyPath = "/tmp/eckey.pem";
const std::string SignatureTest::ecPubKeyPath = "/tmp/ec.pub";

/// keys.
const std::string SignatureTest::ecPriKey = "-----BEGIN EC PRIVATE KEY-----\n"
                                            "MHcCAQEEINr5bOw4vbLCnIAGREN73D+Ne/hn75zgoH/Cv1wxUlQboAoGCCqGSM49\n"
                                            "AwEHoUQDQgAEO1le+TMvvryHdQVr72RgVwBLkfT4fhMekHFp+3JUqCaod0it/h/j\n"
                                            "OPZPc69Xj/kLNG816GoqxpEZC+u4qrbFNg==\n"
                                            "-----END EC PRIVATE KEY-----\n";
const std::string SignatureTest::ecPubKey = "-----BEGIN PUBLIC KEY-----\n"
                                            "MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEO1le+TMvvryHdQVr72RgVwBLkfT4\n"
                                            "fhMekHFp+3JUqCaod0it/h/jOPZPc69Xj/kLNG816GoqxpEZC+u4qrbFNg==\n"
                                            "-----END PUBLIC KEY-----\n";

/// signatures.
const std::string SignatureTest::ec224sig = "MEQCIC+0hx5U/CNiTLpbqTdsK7MJp3/XnAyl1llL38ekG9jiAiAvPoW6bSj2ND/YsiLEi1aX9uKBr/QdnxRlsovnm6qnCg==";
const std::string SignatureTest::ec256sig = "MEYCIQDUFQ6H6eomxyBZAZuB1X0vXJOUtA+RRL+/fLgBw7AfSwIhAOigUCSGNnyGG8Rj/zwBixdiYUBJC0Tap+yE5kSo2OT0";
const std::string SignatureTest::ec384sig = "MEQCIDhsu5VXg45AgeunWA+7eb6phIfn1QRNSpidug/BSZ3BAiBjoxbinxnMiqD6nzuB1VX0iZO1ZAbH2W3j8aTZZtF1qQ==";
const std::string SignatureTest::ec512sig = "MEYCIQD+m/z8dKV2tTsiYFUNnI0rXuRkD7PpCxMakakYhHbnawIhAP0Yn2P8aT8pn5QHIIRcz6xL4YE6GddUIPy6zXhbDgqS";

/**
 * @brief Test sign.
 */
TEST_F (SignatureTest, sign)
{
    BytesArray signature;

    ASSERT_TRUE  (Signature::sign (sample, "/missing/priv/key", Digest::Algorithm::SHA224).empty ());
    ASSERT_EQ    (join::lastError, std::errc::no_such_file_or_directory);

    ASSERT_TRUE  (Signature::sign (sample, ecPubKeyPath, Digest::Algorithm::SHA224).empty ());
    ASSERT_EQ    (join::lastError, DigestErrc::InvalidKey);

    ASSERT_TRUE  (Signature::sign (sample, ecPriKeyPath, Digest::Algorithm (100)).empty ());
    ASSERT_EQ    (join::lastError, DigestErrc::InvalidAlgorithm);

    ASSERT_TRUE  (Signature::sign (nullptr, 0, ecPriKeyPath, Digest::Algorithm::SM3).empty ());
    ASSERT_EQ    (join::lastError, Errc::OperationFailed);

    ASSERT_FALSE ((signature = Signature::sign (sample, ecPriKeyPath, Digest::Algorithm::SHA224)).empty ()) << join::lastError.message ();
    ASSERT_TRUE  (Signature::verify (sample, signature, ecPubKeyPath, Digest::Algorithm::SHA224)) << join::lastError.message ();
    ASSERT_FALSE (Signature::verify (sample, signature, ecPubKeyPath, Digest::Algorithm::SHA256));
    ASSERT_FALSE (Signature::verify (sample, signature, ecPubKeyPath, Digest::Algorithm::SHA384));
    ASSERT_FALSE (Signature::verify (sample, signature, ecPubKeyPath, Digest::Algorithm::SHA512));

    ASSERT_FALSE ((signature = Signature::sign (sample, ecPriKeyPath, Digest::Algorithm::SHA256)).empty ()) << join::lastError.message ();
    ASSERT_FALSE (Signature::verify (sample, signature, ecPubKeyPath, Digest::Algorithm::SHA224));
    ASSERT_TRUE  (Signature::verify (sample, signature, ecPubKeyPath, Digest::Algorithm::SHA256)) << join::lastError.message ();
    ASSERT_FALSE (Signature::verify (sample, signature, ecPubKeyPath, Digest::Algorithm::SHA384));
    ASSERT_FALSE (Signature::verify (sample, signature, ecPubKeyPath, Digest::Algorithm::SHA512));

    ASSERT_FALSE ((signature = Signature::sign (sample, ecPriKeyPath, Digest::Algorithm::SHA384)).empty ()) << join::lastError.message ();
    ASSERT_FALSE (Signature::verify (sample, signature, ecPubKeyPath, Digest::Algorithm::SHA224));
    ASSERT_FALSE (Signature::verify (sample, signature, ecPubKeyPath, Digest::Algorithm::SHA256));
    ASSERT_TRUE  (Signature::verify (sample, signature, ecPubKeyPath, Digest::Algorithm::SHA384)) << join::lastError.message ();
    ASSERT_FALSE (Signature::verify (sample, signature, ecPubKeyPath, Digest::Algorithm::SHA512));

    ASSERT_FALSE ((signature = Signature::sign (sample, ecPriKeyPath, Digest::Algorithm::SHA512)).empty ()) << join::lastError.message ();
    ASSERT_FALSE (Signature::verify (sample, signature, ecPubKeyPath, Digest::Algorithm::SHA224));
    ASSERT_FALSE (Signature::verify (sample, signature, ecPubKeyPath, Digest::Algorithm::SHA256));
    ASSERT_FALSE (Signature::verify (sample, signature, ecPubKeyPath, Digest::Algorithm::SHA384));
    ASSERT_TRUE  (Signature::verify (sample, signature, ecPubKeyPath, Digest::Algorithm::SHA512)) << join::lastError.message ();
}

/**
 * @brief Test verify.
 */
TEST_F (SignatureTest, verify)
{
    ASSERT_FALSE (Signature::verify (sample, Base64::decode (ec224sig), "/missing/pub/key", Digest::Algorithm::SHA224));
    ASSERT_EQ    (join::lastError, std::errc::no_such_file_or_directory);

    ASSERT_FALSE (Signature::verify (sample, Base64::decode (ec224sig), ecPriKeyPath, Digest::Algorithm::SHA224));
    ASSERT_EQ    (join::lastError, DigestErrc::InvalidKey);

    ASSERT_FALSE (Signature::verify (sample, Base64::decode (ec224sig), ecPubKeyPath, Digest::Algorithm (100)));
    ASSERT_EQ    (join::lastError, DigestErrc::InvalidAlgorithm);

    ASSERT_FALSE (Signature::verify (sample, Base64::decode (ec224sig), ecPubKeyPath, Digest::Algorithm::SM3));
    ASSERT_EQ    (join::lastError, Errc::OperationFailed);

    ASSERT_FALSE (Signature::verify (sample, Base64::decode (ec224sig), ecPubKeyPath, Digest::Algorithm::SHA256));
    ASSERT_EQ    (join::lastError, DigestErrc::InvalidSignature);

    ASSERT_TRUE  (Signature::verify (sample, Base64::decode (ec224sig), ecPubKeyPath, Digest::Algorithm::SHA224)) << join::lastError.message ();
    ASSERT_FALSE (Signature::verify (sample, Base64::decode (ec224sig), ecPubKeyPath, Digest::Algorithm::SHA256));
    ASSERT_FALSE (Signature::verify (sample, Base64::decode (ec224sig), ecPubKeyPath, Digest::Algorithm::SHA384));
    ASSERT_FALSE (Signature::verify (sample, Base64::decode (ec224sig), ecPubKeyPath, Digest::Algorithm::SHA512));

    ASSERT_FALSE (Signature::verify (sample, Base64::decode (ec256sig), ecPubKeyPath, Digest::Algorithm::SHA224));
    ASSERT_TRUE  (Signature::verify (sample, Base64::decode (ec256sig), ecPubKeyPath, Digest::Algorithm::SHA256)) << join::lastError.message ();
    ASSERT_FALSE (Signature::verify (sample, Base64::decode (ec256sig), ecPubKeyPath, Digest::Algorithm::SHA384));
    ASSERT_FALSE (Signature::verify (sample, Base64::decode (ec256sig), ecPubKeyPath, Digest::Algorithm::SHA512));

    ASSERT_FALSE (Signature::verify (sample, Base64::decode (ec384sig), ecPubKeyPath, Digest::Algorithm::SHA224));
    ASSERT_FALSE (Signature::verify (sample, Base64::decode (ec384sig), ecPubKeyPath, Digest::Algorithm::SHA256));
    ASSERT_TRUE  (Signature::verify (sample, Base64::decode (ec384sig), ecPubKeyPath, Digest::Algorithm::SHA384)) << join::lastError.message ();
    ASSERT_FALSE (Signature::verify (sample, Base64::decode (ec384sig), ecPubKeyPath, Digest::Algorithm::SHA512));

    ASSERT_FALSE (Signature::verify (sample, Base64::decode (ec512sig), ecPubKeyPath, Digest::Algorithm::SHA224));
    ASSERT_FALSE (Signature::verify (sample, Base64::decode (ec512sig), ecPubKeyPath, Digest::Algorithm::SHA256));
    ASSERT_FALSE (Signature::verify (sample, Base64::decode (ec512sig), ecPubKeyPath, Digest::Algorithm::SHA384));
    ASSERT_TRUE  (Signature::verify (sample, Base64::decode (ec512sig), ecPubKeyPath, Digest::Algorithm::SHA512)) << join::lastError.message ();
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
