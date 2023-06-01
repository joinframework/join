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
#include <join/tlskey.hpp>

// Libraries.
#include <gtest/gtest.h>

// C++.
#include <fstream>

using join::TlsKey;

/**
 * @brief Class used to test TLS key API.
 */
class TlsKeyTest : public ::testing::Test
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
    /// key paths.
    static const std::string ecPriKeyPath;
    static const std::string ecPubKeyPath;

    /// keys.
    static const std::string ecPriKey;
    static const std::string ecPubKey;
};

/// key paths.
const std::string TlsKeyTest::ecPriKeyPath = "/tmp/eckey.pem";
const std::string TlsKeyTest::ecPubKeyPath = "/tmp/ec.pub";

/// keys.
const std::string TlsKeyTest::ecPriKey = "-----BEGIN EC PRIVATE KEY-----\n"
                                         "MHcCAQEEINr5bOw4vbLCnIAGREN73D+Ne/hn75zgoH/Cv1wxUlQboAoGCCqGSM49\n"
                                         "AwEHoUQDQgAEO1le+TMvvryHdQVr72RgVwBLkfT4fhMekHFp+3JUqCaod0it/h/j\n"
                                         "OPZPc69Xj/kLNG816GoqxpEZC+u4qrbFNg==\n"
                                         "-----END EC PRIVATE KEY-----\n";
const std::string TlsKeyTest::ecPubKey = "-----BEGIN PUBLIC KEY-----\n"
                                         "MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEO1le+TMvvryHdQVr72RgVwBLkfT4\n"
                                         "fhMekHFp+3JUqCaod0it/h/jOPZPc69Xj/kLNG816GoqxpEZC+u4qrbFNg==\n"
                                         "-----END PUBLIC KEY-----\n";

/**
 * @brief Test sign.
 */
TEST_F (TlsKeyTest, handle)
{
    ASSERT_THROW (TlsKey ("/missing/key"), std::system_error);
    ASSERT_THROW (TlsKey (ecPriKeyPath, TlsKey::Public), std::system_error);
    ASSERT_THROW (TlsKey (ecPubKeyPath, TlsKey::Private), std::system_error);

    ASSERT_EQ (TlsKey ().handle (), nullptr);
    ASSERT_NE (TlsKey (ecPriKeyPath, TlsKey::Private).handle (), nullptr);
    ASSERT_NE (TlsKey (ecPubKeyPath, TlsKey::Public).handle (), nullptr);
}

/**
 * @brief Test sign.
 */
TEST_F (TlsKeyTest, length)
{
    ASSERT_EQ (TlsKey ().length (), -1);
    ASSERT_EQ (TlsKey (ecPriKeyPath, TlsKey::Private).length (), 256);
    ASSERT_EQ (TlsKey (ecPubKeyPath, TlsKey::Public).length (), 256);
}

/**
 * @brief Test sign.
 */
TEST_F (TlsKeyTest, swap)
{
    ASSERT_EQ (TlsKey ().type (), TlsKey::Private);
    
    TlsKey key1 (ecPriKeyPath, TlsKey::Private), key2 (ecPubKeyPath, TlsKey::Public);
    ASSERT_EQ (key1.type (), TlsKey::Private);
    ASSERT_EQ (key2.type (), TlsKey::Public);

    key1.swap (key2);
    ASSERT_EQ (key1.type (), TlsKey::Public);
    ASSERT_EQ (key2.type (), TlsKey::Private);
}

/**
 * @brief Test sign.
 */
TEST_F (TlsKeyTest, type)
{
    TlsKey key1 (ecPriKeyPath, TlsKey::Private);
    ASSERT_EQ (key1.type (), TlsKey::Private);

    TlsKey key2 (ecPubKeyPath, TlsKey::Public);
    ASSERT_EQ (key2.type (), TlsKey::Public);
}

/**
 * @brief Test sign.
 */
TEST_F (TlsKeyTest, clear)
{
    TlsKey key (ecPubKeyPath, TlsKey::Public);
    ASSERT_EQ (key.type (), TlsKey::Public);
    ASSERT_NE (key.handle (), nullptr);

    key.clear ();
    ASSERT_EQ (key.type (), TlsKey::Private);
    ASSERT_EQ (key.handle (), nullptr);
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
