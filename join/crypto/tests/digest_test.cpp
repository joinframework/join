/**
 * MIT License
 *
 * Copyright (c) 2022 Mathieu Rabine
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

// C++.
#include <fstream>

using join::Base64;
using join::Digest;
using join::BytesArray;

/**
 * @brief Class used to test the Digest API.
 */
class DigestTest : public ::testing::Test
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
const std::string DigestTest::sample = "Lorem ipsum dolor sit amet, consectetur adipiscing elit.";

/// key paths.
const std::string DigestTest::ecPriKeyPath = "/tmp/eckey.pem";
const std::string DigestTest::ecPubKeyPath = "/tmp/ec.pub";

/// keys.
const std::string DigestTest::ecPriKey = "-----BEGIN EC PRIVATE KEY-----\n"
                                         "MHcCAQEEINr5bOw4vbLCnIAGREN73D+Ne/hn75zgoH/Cv1wxUlQboAoGCCqGSM49\n"
                                         "AwEHoUQDQgAEO1le+TMvvryHdQVr72RgVwBLkfT4fhMekHFp+3JUqCaod0it/h/j\n"
                                         "OPZPc69Xj/kLNG816GoqxpEZC+u4qrbFNg==\n"
                                         "-----END EC PRIVATE KEY-----\n"
                                         "\n";
const std::string DigestTest::ecPubKey = "-----BEGIN PUBLIC KEY-----\n"
                                         "MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEO1le+TMvvryHdQVr72RgVwBLkfT4\n"
                                         "fhMekHFp+3JUqCaod0it/h/jOPZPc69Xj/kLNG816GoqxpEZC+u4qrbFNg==\n"
                                         "-----END PUBLIC KEY-----\n"
                                         "\n";

/// signatures.
const std::string DigestTest::ec224sig = "MEUCIQDQmHwdnA6UTIY1o3JOTUArxK8X5S9d8Q0dvhau6FLm3AIgBIJwNTCtlwdIoFNvl3rJYTCm63eRRoRjxZ6aztnlhx0=";
const std::string DigestTest::ec256sig = "MEUCIBuLUJ0VaLARkB0dwQETlUlgY/OxFnoQzPlftsE+pI6KAiEA4T9bdqPZzrlJO+kRZ2JjlPB1w4UT9nBEAziFFqOTPh4=";
const std::string DigestTest::ec384sig = "MEQCIHWEGsFiCQMLKqJTpCBhxhgOgKyKZyCi09eU35RHc5SAAiBJ+HrDNSwA+196VCPnyCCJfb7VXRXq8fJa7hpXtI9q2g==";
const std::string DigestTest::ec512sig = "MEUCIQCdxy/5SmIzEpVmNYgt7zgmZa/mllzP8fAzobwr7+f5nwIgexC1VBjGQVB17NVKe9GglmksyRzMg13rEvFYR0I0RoM=";

/**
 * @brief get algorithm name test.
 */
TEST_F (DigestTest, algorithmName)
{
    ASSERT_STREQ (Digest::algorithmName (Digest::SHA224), "SHA224");
    ASSERT_STREQ (Digest::algorithmName (Digest::SHA256), "SHA256");
    ASSERT_STREQ (Digest::algorithmName (Digest::SHA384), "SHA384");
    ASSERT_STREQ (Digest::algorithmName (Digest::SHA512), "SHA512");
}

/**
 * @brief signature test.
 */
TEST_F (DigestTest, sign)
{
    BytesArray signature;

    signature = Digest::sign (sample, ecPriKeyPath, Digest::SHA224);
    ASSERT_TRUE  (Digest::verifySignature (sample, signature, ecPubKeyPath, Digest::SHA224));
    ASSERT_FALSE (Digest::verifySignature (sample, signature, ecPubKeyPath, Digest::SHA256));
    ASSERT_FALSE (Digest::verifySignature (sample, signature, ecPubKeyPath, Digest::SHA384));
    ASSERT_FALSE (Digest::verifySignature (sample, signature, ecPubKeyPath, Digest::SHA512));

    signature = Digest::sign (sample, ecPriKeyPath, Digest::SHA256);
    ASSERT_FALSE (Digest::verifySignature (sample, signature, ecPubKeyPath, Digest::SHA224));
    ASSERT_TRUE  (Digest::verifySignature (sample, signature, ecPubKeyPath, Digest::SHA256));
    ASSERT_FALSE (Digest::verifySignature (sample, signature, ecPubKeyPath, Digest::SHA384));
    ASSERT_FALSE (Digest::verifySignature (sample, signature, ecPubKeyPath, Digest::SHA512));

    signature = Digest::sign (sample, ecPriKeyPath, Digest::SHA384);
    ASSERT_FALSE (Digest::verifySignature (sample, signature, ecPubKeyPath, Digest::SHA224));
    ASSERT_FALSE (Digest::verifySignature (sample, signature, ecPubKeyPath, Digest::SHA256));
    ASSERT_TRUE  (Digest::verifySignature (sample, signature, ecPubKeyPath, Digest::SHA384));
    ASSERT_FALSE (Digest::verifySignature (sample, signature, ecPubKeyPath, Digest::SHA512));

    signature = Digest::sign (sample, ecPriKeyPath, Digest::SHA512);
    ASSERT_FALSE (Digest::verifySignature (sample, signature, ecPubKeyPath, Digest::SHA224));
    ASSERT_FALSE (Digest::verifySignature (sample, signature, ecPubKeyPath, Digest::SHA256));
    ASSERT_FALSE (Digest::verifySignature (sample, signature, ecPubKeyPath, Digest::SHA384));
    ASSERT_TRUE  (Digest::verifySignature (sample, signature, ecPubKeyPath, Digest::SHA512));
}

/**
 * @brief signature verification test.
 */
TEST_F (DigestTest, verifySignature)
{
    ASSERT_TRUE  (Digest::verifySignature (sample, Base64::decode (ec224sig), ecPubKeyPath, Digest::SHA224));
    ASSERT_FALSE (Digest::verifySignature (sample, Base64::decode (ec224sig), ecPubKeyPath, Digest::SHA256));
    ASSERT_FALSE (Digest::verifySignature (sample, Base64::decode (ec224sig), ecPubKeyPath, Digest::SHA384));
    ASSERT_FALSE (Digest::verifySignature (sample, Base64::decode (ec224sig), ecPubKeyPath, Digest::SHA512));

    ASSERT_FALSE (Digest::verifySignature (sample, Base64::decode (ec256sig), ecPubKeyPath, Digest::SHA224));
    ASSERT_TRUE  (Digest::verifySignature (sample, Base64::decode (ec256sig), ecPubKeyPath, Digest::SHA256));
    ASSERT_FALSE (Digest::verifySignature (sample, Base64::decode (ec256sig), ecPubKeyPath, Digest::SHA384));
    ASSERT_FALSE (Digest::verifySignature (sample, Base64::decode (ec256sig), ecPubKeyPath, Digest::SHA512));

    ASSERT_FALSE (Digest::verifySignature (sample, Base64::decode (ec384sig), ecPubKeyPath, Digest::SHA224));
    ASSERT_FALSE (Digest::verifySignature (sample, Base64::decode (ec384sig), ecPubKeyPath, Digest::SHA256));
    ASSERT_TRUE  (Digest::verifySignature (sample, Base64::decode (ec384sig), ecPubKeyPath, Digest::SHA384));
    ASSERT_FALSE (Digest::verifySignature (sample, Base64::decode (ec384sig), ecPubKeyPath, Digest::SHA512));

    ASSERT_FALSE (Digest::verifySignature (sample, Base64::decode (ec512sig), ecPubKeyPath, Digest::SHA224));
    ASSERT_FALSE (Digest::verifySignature (sample, Base64::decode (ec512sig), ecPubKeyPath, Digest::SHA256));
    ASSERT_FALSE (Digest::verifySignature (sample, Base64::decode (ec512sig), ecPubKeyPath, Digest::SHA384));
    ASSERT_TRUE  (Digest::verifySignature (sample, Base64::decode (ec512sig), ecPubKeyPath, Digest::SHA512));
}

/**
 * @brief signature conversion test.
 */
TEST_F (DigestTest, convertSignature)
{
    BytesArray signature;

    signature = Digest::toFlat (Base64::decode (ec224sig));
    ASSERT_FALSE (signature.empty ());
    signature = Digest::toDer (signature);
    ASSERT_FALSE (signature.empty ());
    ASSERT_EQ (Base64::encode (signature), ec224sig);

    signature = Digest::toFlat (Base64::decode (ec256sig));
    ASSERT_FALSE (signature.empty ());
    signature = Digest::toDer (signature);
    ASSERT_FALSE (signature.empty ());
    ASSERT_EQ (Base64::encode (signature), ec256sig);

    signature = Digest::toFlat (Base64::decode (ec384sig));
    ASSERT_FALSE (signature.empty ());
    signature = Digest::toDer (signature);
    ASSERT_FALSE (signature.empty ());
    ASSERT_EQ (Base64::encode (signature), ec384sig);

    signature = Digest::toFlat (Base64::decode (ec512sig));
    ASSERT_FALSE (signature.empty ());
    signature = Digest::toDer (signature);
    ASSERT_FALSE (signature.empty ());
    ASSERT_EQ (Base64::encode (signature), ec512sig);
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
