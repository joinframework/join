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
        writeFile (rsaPriKeyPath, rsaPriKey);
        writeFile (rsaPubKeyPath, rsaPubKey);
    }

    /**
     * @brief Tear down test case.
     */
    static void TearDownTestCase ()
    {
        // remove keys from file system.
        ::remove (rsaPriKeyPath.c_str ());
        ::remove (rsaPubKeyPath.c_str ());
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
    static const std::string rsaPriKeyPath;
    static const std::string rsaPubKeyPath;

    /// keys.
    static const std::string rsaPriKey;
    static const std::string rsaPubKey;

    /// sample text.
    static const std::string sample;

    /// signatures.
    static const std::string rsa5sig;
    static const std::string rsa1sig;
    static const std::string rsa224sig;
    static const std::string rsa256sig;
    static const std::string rsa384sig;
    static const std::string rsa512sig;
};

/// key paths.
const std::string SignatureTest::rsaPriKeyPath = "/tmp/prikey.pem";
const std::string SignatureTest::rsaPubKeyPath = "/tmp/pubkey.pem";

/// keys.
const std::string SignatureTest::rsaPriKey = "-----BEGIN RSA PRIVATE KEY-----\n"
                                             "MIIEpAIBAAKCAQEA3kf3qNpcqFm+tr6lQ9P9jE3SNUBm3YCO6oRn/Epn7RHITyGr\n"
                                             "U66G0/KQeXhOYG/6IW8/oaspgQJ2T0PifV8crY9phxy8KXDo8xBEx+zLSrLKUWyU\n"
                                             "8s4BwMGWsMRmdIt0tzkVLTGV2PYr2B8lPNni0ytS1WVoVcyyc7aVTHGW0asAbtKI\n"
                                             "xy2UZJdrEt1O3DM+Z4vZA665KzNqN0gh1FIB0GkIZJJ8kQA/+EN0SbUmncJbd8tH\n"
                                             "i5IaDks4WhUgdDPKFDORVGfty9lNrHhFayuOrs5Q9BFfo7WuL4cqh7sTGhFcmdvs\n"
                                             "QfhHxSumWu0QxaFdrBiqayUQUsqQ35IBvrFnXwIDAQABAoIBAGMG8unR5ofF+7YU\n"
                                             "dzIhpoq0PNsmhu1VkdYfCOiYCXbPfkrquBY+4ahZH05Ob7R9DpWIp3OR+pqUCztD\n"
                                             "BgtOrUsRYjUkcxuPRujKranX77Wms4XU9wK5DoTeehDkXrS7UFM0Zh/NQHH5Mg1F\n"
                                             "yuQyOeBUI8IYPIRE/peYlykITeW4EZprpVSfbC57rD0vA9iq55/uGWFKPM5TmIY5\n"
                                             "iOyH/grRvgnx9YgHG6GZoZ6Xb7KdZi7CSvS4Zz2xNpnH/qT5c628UEn09m6bWoKV\n"
                                             "zZ2/39RmPdXVEEZQp97zcfGcL6Br6IeQVKgzy2MfhiDTgBt+B1Wl7gdJMvflWdL+\n"
                                             "b9mV8nECgYEA8C9epBX0IXbrbtjBWT42oK6DWE2ifm24wFIH+xwvJOQ9pap3KYfE\n"
                                             "2GTKEX8qpfi3R5PK860iF/wDS+0OZmF9Z1WlL1qTwKuqicpXqCoxN7tKvTEfkBFw\n"
                                             "rht0EqrX+8i8BwxI7VNB84UGn58mvpWRgWwfZuv6nw13C8fFWGDz3IkCgYEA7OrN\n"
                                             "1EI28WhxZlbAizPRJEUCJKt1mA2kROytNOWHax2AEYdvtCIebeRAbUjYC1z4Ibvk\n"
                                             "ZSsIJrQQhkvF0D0ql2ULIkqWwLr76NvNi1O+9R+9QE6KikP9Pb0VkRSN2uRVVamx\n"
                                             "yYWSJaY0iORJRaZHsBjSd84MubPe5huucIgZuqcCgYEA2YhNkxc/EL//41j1ZLpw\n"
                                             "B3+G/gbyUMdXOPgSul+AY01EeDK30ilhIwvF56rhGYj1liJJAgGgi1B4O9/r1tYd\n"
                                             "GeX7wbOrdikEP9+/HFN6WBuNSWSgTX8+KrI6ol2RWD0p7sg1lJx/curcYN1n3dzg\n"
                                             "9/LRTgoT5tHvLAA2wCCvPOECgYBLrin3/vrcHNf0sAurq+7IHj80BY13+AucdNpT\n"
                                             "hmq+vdq2PsRhD3EW+43VGyrgXl9pL0MjSh69dlG9BKF5BsD/QajP2HHVuSVVWjmL\n"
                                             "o0HhvbTPwQciaduQiEFGagFyhfP4fE+tpzxrSG+Jtxrs49QMnbFF0g1gmOPAL+GG\n"
                                             "UwOdCwKBgQDPGJUC2Hb7dsEeZnXfLZcgerMpitKEGTkQmWrL3kb2w+MlL/IFpCoy\n"
                                             "w23OHfhvsWPEXGz/PZL9VxjG3dJs/6lwT6Jco8vQKWJeY/GGWQQ2zofBrlDjsBhg\n"
                                             "6yfBAiQsAbzLyLU+nDHnaVaXeYtFniEfnnZvyPPYAa5br+9Qs+oUvA==\n"
                                             "-----END RSA PRIVATE KEY-----\n";
const std::string SignatureTest::rsaPubKey = "-----BEGIN PUBLIC KEY-----\n"
                                             "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA3kf3qNpcqFm+tr6lQ9P9\n"
                                             "jE3SNUBm3YCO6oRn/Epn7RHITyGrU66G0/KQeXhOYG/6IW8/oaspgQJ2T0PifV8c\n"
                                             "rY9phxy8KXDo8xBEx+zLSrLKUWyU8s4BwMGWsMRmdIt0tzkVLTGV2PYr2B8lPNni\n"
                                             "0ytS1WVoVcyyc7aVTHGW0asAbtKIxy2UZJdrEt1O3DM+Z4vZA665KzNqN0gh1FIB\n"
                                             "0GkIZJJ8kQA/+EN0SbUmncJbd8tHi5IaDks4WhUgdDPKFDORVGfty9lNrHhFayuO\n"
                                             "rs5Q9BFfo7WuL4cqh7sTGhFcmdvsQfhHxSumWu0QxaFdrBiqayUQUsqQ35IBvrFn\n"
                                             "XwIDAQAB\n"
                                             "-----END PUBLIC KEY-----\n";

/// sample text.
const std::string SignatureTest::sample = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. "
                                          "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. "
                                          "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. "
                                          "Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.\n";

/// signatures.
const std::string SignatureTest::rsa5sig   = "u60S1HZEBgcjlO1JQjNJzNNCK8OpeMo2LQXwYnDNwgDaqUSePdWuXtYKu8r/Mtmy"
                                             "cGDCrQCWcvgZXYmebSKLxJAH8Q7NQWzJRg++VClvMdkXGya/QJUm92b4omRCKqwf"
                                             "kIKNN4ojqqDbOkN2GeUS4hYDQuvnH9joO9S6/BoJ92aLWCaGdFD+wWrxlK7+CrHq"
                                             "diBdMN4JuCzwGfjIpIOu4cfTZYoMlzwjihFsGOCzAwDAV3P5/VRFww3PI/PSaOxe"
                                             "kzuPjc9sutkDO5C0QCavS1nKfFXykdkqyc0CSJ5cKjCpC+9jxu6xXRhkcXYqbkSC"
                                             "QQLg4TgE8NxWDs3RW4lpsg==";
const std::string SignatureTest::rsa1sig   = "W4HYSRWaQu2w7I5nYc04Cnt+pFNnD1ddHUfG8XPr1koAoY3V3UN3wa8bqwwpIxoL"
                                             "qtLVbjwkiF5EgsbiYWYgqXNJhsmTnRvaUN9hKDOufCAH6nOQB1QBH7EibNZ/2frj"
                                             "ZnoY/TAW+RQH5VmYdAIDV2VQVlgv8PHoFX0AWNCWzQ0KZk2PbP0+KbYv9rlblY3Y"
                                             "sGV7i7MJI+8bYgHjczCV0S5o9QS3u2QsCpC5IUF7H3N5MknOKyx8hLrAeiq6FPl9"
                                             "MujuFSMVBT6PbFGcogEKjVRBoSpKZBzEajW+e5/OqjVnlemW4nh+h4RV2v6Gf+yK"
                                             "xHtWqgu8ZoSSex/k+qeFcg==";
const std::string SignatureTest::rsa224sig = "c+WnKME5Tw5z9As/byt2BDWlxFkD8mOkYI2ldVl3RQ03vGhZyENgASSb19z/Km6H"
                                             "Y/frFFGOoKOWCYLHeaiGz94biB9XzJd5RU+9vkOcg6gmEcT4oHCYpMyupPAeEplU"
                                             "synWkfxTl+NBHrjrrg29lYi0r7OWzbwehBVg0eeKomwuo8yhM6qlesP5p36Dadvh"
                                             "6wwv/WaRu/H7ZH2VEK//CUmhbk0lfmxkDMgSOb479gkVJc2etI+MCfiQlwokUiK1"
                                             "paxttbiE2BFD/31lTeJAbHjh7xMi+8SnN9hFtBLDepJ9b7OB3s0MCenGDl+Wz0QM"
                                             "0k2j8CZpleutP3RYuxxClw==";
const std::string SignatureTest::rsa256sig = "ptd3mA0xDu0HIV9Z65chUHHMP4POrAdPKbcjwLPl8kIdi3Wkj/ZQN1erRKoJsb7m"
                                             "nB2uFr1VlXIMno2cBUk8tacenxvpbJZl/Hlr2te/WJhnmFG1qdtHADUf3fDkysmK"
                                             "AImqm/gukduG+TDh41AlNv/v+SltzN9wF0uxU4r+0ZXSvRpIF9Nkx7YZ04bQCZ4r"
                                             "Tk3rv54eCVvKaO79u0FFvmaAlmDzp7THh1ydRhznqy/NCSyCaKIDZ3Knf/X35fdm"
                                             "fQSPBY96z/ttK8pcjbAUyVTGY4P6G1Y2vDq3yExN37koBOGdE6jRufBEFHX/R7jw"
                                             "GSBJsg6hOaGJ/HuYM66SRw==";
const std::string SignatureTest::rsa384sig = "wclpmkUNE54jw05C4f2L1KwvFH+/sVwYP6ZcPWf6ICzG9E7avNDgYsql+Yo1oPcz"
                                             "9GdF1ZJfU7ZSfRuCA6rZm1S2dw132wT3BgqHHvtgNCuCrKlBD49M7hlWAJvzUQlq"
                                             "HGxRSKUT1AFqN4B+Y8oBEvQVcbIxu6UaOTr6vJIzAqj/v2dsRmQUWFkvGE4IEF8T"
                                             "PbI7wCLcaKv9bdJaW5tJWw/kV17CiqzvJWtScUmkAVjXw1n2wdiWWYXnP2g5erKF"
                                             "G8hHumtvl0nWf+3wgypkuuvtB3fN/dWaAJxeqwAJqoxrvXiReU6CgW/5tj47Fjx/"
                                             "H0xP9MFCHT/pTd0SVfGt7Q==";
const std::string SignatureTest::rsa512sig = "MZTA8H+DiMgFAZf4udZVcCtLQa5IOHVCjXRigijaZislAveW56HNO8eP+kl24BAW"
                                             "jMfWCXB30PZxjkhZPWSLHRM4aAaFrZSbCzw9IGBQY+59YbqWvzmtJYCl95wDhMfk"
                                             "BeDUrgp6E4CLDzRlURFjsBBcERamXh2aN2GpQycvk2LZONjBV7d1y0jlSYrfFtvd"
                                             "k8JA6lJxo/B+GMuzd65mEFU9v3rrriWGGmfEKMLUReEQnYmbDh3xd6eb3CO/Tpb6"
                                             "+GgMf+bxsR7Afj6bR20vpDOpAWtCHOkekFXtgdrCgoXFx4PLYbz8Y/Htip+gW/yx"
                                             "L5wH/mVlb6960335RbqjIA==";

/**
 * @brief Test sign.
 */
TEST_F (SignatureTest, sign)
{
    ASSERT_THROW (Signature (Digest::Algorithm (0)), std::system_error);

    Signature tmp (Digest::Algorithm::MD5);
    Signature sig = std::move (tmp);
    sig << sample;
    ASSERT_TRUE (Signature::verify (sample, sig.sign (rsaPriKeyPath), rsaPubKeyPath, Digest::Algorithm::MD5)) << join::lastError.message ();
    sig.write (sample.data (), sample.size ());
    ASSERT_TRUE (Signature::verify (sample, sig.sign (rsaPriKeyPath), rsaPubKeyPath, Digest::Algorithm::MD5)) << join::lastError.message ();

    sig = std::move (Signature (Digest::Algorithm::SHA1));
    sig << sample;
    ASSERT_TRUE (Signature::verify (sample, sig.sign (rsaPriKeyPath), rsaPubKeyPath, Digest::Algorithm::SHA1)) << join::lastError.message ();
    sig.write (sample.data (), sample.size ());
    ASSERT_TRUE (Signature::verify (sample, sig.sign (rsaPriKeyPath), rsaPubKeyPath, Digest::Algorithm::SHA1)) << join::lastError.message ();

    sig = std::move (Signature (Digest::Algorithm::SHA224));
    sig << sample;
    ASSERT_TRUE (Signature::verify (sample, sig.sign (rsaPriKeyPath), rsaPubKeyPath, Digest::Algorithm::SHA224)) << join::lastError.message ();
    sig.write (sample.data (), sample.size ());
    ASSERT_TRUE (Signature::verify (sample, sig.sign (rsaPriKeyPath), rsaPubKeyPath, Digest::Algorithm::SHA224)) << join::lastError.message ();

    sig = std::move (Signature (Digest::Algorithm::SHA256));
    sig << sample;
    ASSERT_TRUE (Signature::verify (sample, sig.sign (rsaPriKeyPath), rsaPubKeyPath, Digest::Algorithm::SHA256)) << join::lastError.message ();
    sig.write (sample.data (), sample.size ());
    ASSERT_TRUE (Signature::verify (sample, sig.sign (rsaPriKeyPath), rsaPubKeyPath, Digest::Algorithm::SHA256)) << join::lastError.message ();

    sig = std::move (Signature (Digest::Algorithm::SHA384));
    sig << sample;
    ASSERT_TRUE (Signature::verify (sample, sig.sign (rsaPriKeyPath), rsaPubKeyPath, Digest::Algorithm::SHA384)) << join::lastError.message ();
    sig.write (sample.data (), sample.size ());
    ASSERT_TRUE (Signature::verify (sample, sig.sign (rsaPriKeyPath), rsaPubKeyPath, Digest::Algorithm::SHA384)) << join::lastError.message ();

    sig = std::move (Signature (Digest::Algorithm::SHA512));
    sig << sample;
    ASSERT_TRUE (Signature::verify (sample, sig.sign (rsaPriKeyPath), rsaPubKeyPath, Digest::Algorithm::SHA512)) << join::lastError.message ();
    sig.write (sample.data (), sample.size ());
    ASSERT_TRUE (Signature::verify (sample, sig.sign (rsaPriKeyPath), rsaPubKeyPath, Digest::Algorithm::SHA512)) << join::lastError.message ();
}

/**
 * @brief Test sign failures.
 */
TEST_F (SignatureTest, sign_failures)
{
    ASSERT_TRUE  (Signature::sign (sample, "/missing/priv/key", Digest::Algorithm::SHA224).empty ());
    ASSERT_EQ    (join::lastError, std::errc::no_such_file_or_directory);

    ASSERT_TRUE  (Signature::sign (sample, rsaPubKeyPath, Digest::Algorithm::SHA224).empty ());
    ASSERT_EQ    (join::lastError, DigestErrc::InvalidKey);

    ASSERT_TRUE  (Signature::sign (sample, rsaPriKeyPath, Digest::Algorithm (100)).empty ());
    ASSERT_EQ    (join::lastError, DigestErrc::InvalidAlgorithm);

    ASSERT_TRUE  (Signature::sign (nullptr, 0, rsaPriKeyPath, Digest::Algorithm::SM3).empty ());
    ASSERT_EQ    (join::lastError, DigestErrc::InvalidAlgorithm);
}

/**
 * @brief Test md5sign.
 */
TEST_F (SignatureTest, md5sign)
{
    BytesArray signature;

    ASSERT_FALSE ((signature = Signature::sign (sample, rsaPriKeyPath, Digest::Algorithm::MD5)).empty ()) << join::lastError.message ();
    ASSERT_TRUE  (Signature::verify (sample, signature, rsaPubKeyPath, Digest::Algorithm::MD5)) << join::lastError.message ();
    ASSERT_FALSE (Signature::verify (sample, signature, rsaPubKeyPath, Digest::Algorithm::SHA1));
    ASSERT_FALSE (Signature::verify (sample, signature, rsaPubKeyPath, Digest::Algorithm::SHA224));
    ASSERT_FALSE (Signature::verify (sample, signature, rsaPubKeyPath, Digest::Algorithm::SHA256));
    ASSERT_FALSE (Signature::verify (sample, signature, rsaPubKeyPath, Digest::Algorithm::SHA384));
    ASSERT_FALSE (Signature::verify (sample, signature, rsaPubKeyPath, Digest::Algorithm::SHA512));
}

/**
 * @brief Test sha1sign.
 */
TEST_F (SignatureTest, sha1sign)
{
    BytesArray signature;

    ASSERT_FALSE ((signature = Signature::sign (sample, rsaPriKeyPath, Digest::Algorithm::SHA1)).empty ()) << join::lastError.message ();
    ASSERT_FALSE (Signature::verify (sample, signature, rsaPubKeyPath, Digest::Algorithm::MD5));
    ASSERT_TRUE  (Signature::verify (sample, signature, rsaPubKeyPath, Digest::Algorithm::SHA1)) << join::lastError.message ();
    ASSERT_FALSE (Signature::verify (sample, signature, rsaPubKeyPath, Digest::Algorithm::SHA224));
    ASSERT_FALSE (Signature::verify (sample, signature, rsaPubKeyPath, Digest::Algorithm::SHA256));
    ASSERT_FALSE (Signature::verify (sample, signature, rsaPubKeyPath, Digest::Algorithm::SHA384));
    ASSERT_FALSE (Signature::verify (sample, signature, rsaPubKeyPath, Digest::Algorithm::SHA512));
}

/**
 * @brief Test sha224sign.
 */
TEST_F (SignatureTest, sha224sign)
{
    BytesArray signature;

    ASSERT_FALSE ((signature = Signature::sign (sample, rsaPriKeyPath, Digest::Algorithm::SHA224)).empty ()) << join::lastError.message ();
    ASSERT_FALSE (Signature::verify (sample, signature, rsaPubKeyPath, Digest::Algorithm::MD5));
    ASSERT_FALSE (Signature::verify (sample, signature, rsaPubKeyPath, Digest::Algorithm::SHA1));
    ASSERT_TRUE  (Signature::verify (sample, signature, rsaPubKeyPath, Digest::Algorithm::SHA224)) << join::lastError.message ();
    ASSERT_FALSE (Signature::verify (sample, signature, rsaPubKeyPath, Digest::Algorithm::SHA256));
    ASSERT_FALSE (Signature::verify (sample, signature, rsaPubKeyPath, Digest::Algorithm::SHA384));
    ASSERT_FALSE (Signature::verify (sample, signature, rsaPubKeyPath, Digest::Algorithm::SHA512));
}

/**
 * @brief Test sha256sign.
 */
TEST_F (SignatureTest, sha256sign)
{
    BytesArray signature;

    ASSERT_FALSE ((signature = Signature::sign (sample, rsaPriKeyPath, Digest::Algorithm::SHA256)).empty ()) << join::lastError.message ();
    ASSERT_FALSE (Signature::verify (sample, signature, rsaPubKeyPath, Digest::Algorithm::MD5));
    ASSERT_FALSE (Signature::verify (sample, signature, rsaPubKeyPath, Digest::Algorithm::SHA1));
    ASSERT_FALSE (Signature::verify (sample, signature, rsaPubKeyPath, Digest::Algorithm::SHA224));
    ASSERT_TRUE  (Signature::verify (sample, signature, rsaPubKeyPath, Digest::Algorithm::SHA256)) << join::lastError.message ();
    ASSERT_FALSE (Signature::verify (sample, signature, rsaPubKeyPath, Digest::Algorithm::SHA384));
    ASSERT_FALSE (Signature::verify (sample, signature, rsaPubKeyPath, Digest::Algorithm::SHA512));
}

/**
 * @brief Test sha384sign.
 */
TEST_F (SignatureTest, sha384sign)
{
    BytesArray signature;

    ASSERT_FALSE ((signature = Signature::sign (sample, rsaPriKeyPath, Digest::Algorithm::SHA384)).empty ()) << join::lastError.message ();
    ASSERT_FALSE (Signature::verify (sample, signature, rsaPubKeyPath, Digest::Algorithm::MD5));
    ASSERT_FALSE (Signature::verify (sample, signature, rsaPubKeyPath, Digest::Algorithm::SHA1));
    ASSERT_FALSE (Signature::verify (sample, signature, rsaPubKeyPath, Digest::Algorithm::SHA224));
    ASSERT_FALSE (Signature::verify (sample, signature, rsaPubKeyPath, Digest::Algorithm::SHA256));
    ASSERT_TRUE  (Signature::verify (sample, signature, rsaPubKeyPath, Digest::Algorithm::SHA384)) << join::lastError.message ();
    ASSERT_FALSE (Signature::verify (sample, signature, rsaPubKeyPath, Digest::Algorithm::SHA512));
}

/**
 * @brief Test sha512sign.
 */
TEST_F (SignatureTest, sha512sign)
{
    BytesArray signature;

    ASSERT_FALSE ((signature = Signature::sign (sample, rsaPriKeyPath, Digest::Algorithm::SHA512)).empty ()) << join::lastError.message ();
    ASSERT_FALSE (Signature::verify (sample, signature, rsaPubKeyPath, Digest::Algorithm::MD5));
    ASSERT_FALSE (Signature::verify (sample, signature, rsaPubKeyPath, Digest::Algorithm::SHA1));
    ASSERT_FALSE (Signature::verify (sample, signature, rsaPubKeyPath, Digest::Algorithm::SHA224));
    ASSERT_FALSE (Signature::verify (sample, signature, rsaPubKeyPath, Digest::Algorithm::SHA256));
    ASSERT_FALSE (Signature::verify (sample, signature, rsaPubKeyPath, Digest::Algorithm::SHA384));
    ASSERT_TRUE  (Signature::verify (sample, signature, rsaPubKeyPath, Digest::Algorithm::SHA512)) << join::lastError.message ();
}

/**
 * @brief Test verify.
 */
TEST_F (SignatureTest, verify)
{
    ASSERT_THROW (Signature (Digest::Algorithm (0)), std::system_error);

    Signature sig (Digest::Algorithm::MD5);
    sig << sample;
    ASSERT_TRUE (sig.verify (Base64::decode (rsa5sig), rsaPubKeyPath)) << join::lastError.message ();
    sig.write (sample.data (), sample.size ());
    ASSERT_TRUE (sig.verify (Base64::decode (rsa5sig), rsaPubKeyPath)) << join::lastError.message ();

    sig = std::move (Signature (Digest::Algorithm::SHA1));
    sig << sample;
    ASSERT_TRUE (sig.verify (Base64::decode (rsa1sig), rsaPubKeyPath)) << join::lastError.message ();
    sig.write (sample.data (), sample.size ());
    ASSERT_TRUE (sig.verify (Base64::decode (rsa1sig), rsaPubKeyPath)) << join::lastError.message ();

    sig = std::move (Signature (Digest::Algorithm::SHA224));
    sig << sample;
    ASSERT_TRUE (sig.verify (Base64::decode (rsa224sig), rsaPubKeyPath)) << join::lastError.message ();
    sig.write (sample.data (), sample.size ());
    ASSERT_TRUE (sig.verify (Base64::decode (rsa224sig), rsaPubKeyPath)) << join::lastError.message ();

    sig = std::move (Signature (Digest::Algorithm::SHA256));
    sig << sample;
    ASSERT_TRUE (sig.verify (Base64::decode (rsa256sig), rsaPubKeyPath)) << join::lastError.message ();
    sig.write (sample.data (), sample.size ());
    ASSERT_TRUE (sig.verify (Base64::decode (rsa256sig), rsaPubKeyPath)) << join::lastError.message ();

    sig = std::move (Signature (Digest::Algorithm::SHA384));
    sig << sample;
    ASSERT_TRUE (sig.verify (Base64::decode (rsa384sig), rsaPubKeyPath)) << join::lastError.message ();
    sig.write (sample.data (), sample.size ());
    ASSERT_TRUE (sig.verify (Base64::decode (rsa384sig), rsaPubKeyPath)) << join::lastError.message ();

    sig = std::move (Signature (Digest::Algorithm::SHA512));
    sig << sample;
    ASSERT_TRUE (sig.verify (Base64::decode (rsa512sig), rsaPubKeyPath)) << join::lastError.message ();
    sig.write (sample.data (), sample.size ());
    ASSERT_TRUE (sig.verify (Base64::decode (rsa512sig), rsaPubKeyPath)) << join::lastError.message ();
}

/**
 * @brief Test verify failures.
 */
TEST_F (SignatureTest, verify_failures)
{
    ASSERT_FALSE (Signature::verify (sample, Base64::decode (rsa224sig), "/missing/pub/key", Digest::Algorithm::SHA224));
    ASSERT_EQ    (join::lastError, std::errc::no_such_file_or_directory);

    ASSERT_FALSE (Signature::verify (sample, Base64::decode (rsa224sig), rsaPriKeyPath, Digest::Algorithm::SHA224));
    ASSERT_EQ    (join::lastError, DigestErrc::InvalidKey);

    ASSERT_FALSE (Signature::verify (sample, Base64::decode (rsa224sig), rsaPubKeyPath, Digest::Algorithm (100)));
    ASSERT_EQ    (join::lastError, DigestErrc::InvalidAlgorithm);

    ASSERT_FALSE (Signature::verify (sample, Base64::decode (rsa224sig), rsaPubKeyPath, Digest::Algorithm::SM3));
    ASSERT_EQ    (join::lastError, DigestErrc::InvalidAlgorithm);

    ASSERT_FALSE (Signature::verify (sample, Base64::decode (rsa224sig), rsaPubKeyPath, Digest::Algorithm::SHA256));
    ASSERT_EQ    (join::lastError, DigestErrc::InvalidSignature);
}

/**
 * @brief Test md5verify.
 */
TEST_F (SignatureTest, md5verify)
{
    ASSERT_TRUE  (Signature::verify (sample, Base64::decode (rsa5sig), rsaPubKeyPath, Digest::Algorithm::MD5)) << join::lastError.message ();
    ASSERT_FALSE (Signature::verify (sample, Base64::decode (rsa5sig), rsaPubKeyPath, Digest::Algorithm::SHA1));
    ASSERT_FALSE (Signature::verify (sample, Base64::decode (rsa5sig), rsaPubKeyPath, Digest::Algorithm::SHA224));
    ASSERT_FALSE (Signature::verify (sample, Base64::decode (rsa5sig), rsaPubKeyPath, Digest::Algorithm::SHA256));
    ASSERT_FALSE (Signature::verify (sample, Base64::decode (rsa5sig), rsaPubKeyPath, Digest::Algorithm::SHA384));
    ASSERT_FALSE (Signature::verify (sample, Base64::decode (rsa5sig), rsaPubKeyPath, Digest::Algorithm::SHA512));
}

/**
 * @brief Test sha1verify.
 */
TEST_F (SignatureTest, sha1verify)
{
    ASSERT_FALSE (Signature::verify (sample, Base64::decode (rsa1sig), rsaPubKeyPath, Digest::Algorithm::MD5));
    ASSERT_TRUE  (Signature::verify (sample, Base64::decode (rsa1sig), rsaPubKeyPath, Digest::Algorithm::SHA1)) << join::lastError.message ();
    ASSERT_FALSE (Signature::verify (sample, Base64::decode (rsa1sig), rsaPubKeyPath, Digest::Algorithm::SHA224));
    ASSERT_FALSE (Signature::verify (sample, Base64::decode (rsa1sig), rsaPubKeyPath, Digest::Algorithm::SHA256));
    ASSERT_FALSE (Signature::verify (sample, Base64::decode (rsa1sig), rsaPubKeyPath, Digest::Algorithm::SHA384));
    ASSERT_FALSE (Signature::verify (sample, Base64::decode (rsa1sig), rsaPubKeyPath, Digest::Algorithm::SHA512));
}

/**
 * @brief Test sha224verify.
 */
TEST_F (SignatureTest, sha224verify)
{
    ASSERT_FALSE (Signature::verify (sample, Base64::decode (rsa224sig), rsaPubKeyPath, Digest::Algorithm::MD5));
    ASSERT_FALSE (Signature::verify (sample, Base64::decode (rsa224sig), rsaPubKeyPath, Digest::Algorithm::SHA1));
    ASSERT_TRUE  (Signature::verify (sample, Base64::decode (rsa224sig), rsaPubKeyPath, Digest::Algorithm::SHA224)) << join::lastError.message ();
    ASSERT_FALSE (Signature::verify (sample, Base64::decode (rsa224sig), rsaPubKeyPath, Digest::Algorithm::SHA256));
    ASSERT_FALSE (Signature::verify (sample, Base64::decode (rsa224sig), rsaPubKeyPath, Digest::Algorithm::SHA384));
    ASSERT_FALSE (Signature::verify (sample, Base64::decode (rsa224sig), rsaPubKeyPath, Digest::Algorithm::SHA512));
}

/**
 * @brief Test sha256verify.
 */
TEST_F (SignatureTest, sha256verify)
{
    ASSERT_FALSE (Signature::verify (sample, Base64::decode (rsa256sig), rsaPubKeyPath, Digest::Algorithm::MD5));
    ASSERT_FALSE (Signature::verify (sample, Base64::decode (rsa256sig), rsaPubKeyPath, Digest::Algorithm::SHA1));
    ASSERT_FALSE (Signature::verify (sample, Base64::decode (rsa256sig), rsaPubKeyPath, Digest::Algorithm::SHA224));
    ASSERT_TRUE  (Signature::verify (sample, Base64::decode (rsa256sig), rsaPubKeyPath, Digest::Algorithm::SHA256)) << join::lastError.message ();
    ASSERT_FALSE (Signature::verify (sample, Base64::decode (rsa256sig), rsaPubKeyPath, Digest::Algorithm::SHA384));
    ASSERT_FALSE (Signature::verify (sample, Base64::decode (rsa256sig), rsaPubKeyPath, Digest::Algorithm::SHA512));
}

/**
 * @brief Test sha384verify.
 */
TEST_F (SignatureTest, sha384verify)
{
    ASSERT_FALSE (Signature::verify (sample, Base64::decode (rsa384sig), rsaPubKeyPath, Digest::Algorithm::MD5));
    ASSERT_FALSE (Signature::verify (sample, Base64::decode (rsa384sig), rsaPubKeyPath, Digest::Algorithm::SHA1));
    ASSERT_FALSE (Signature::verify (sample, Base64::decode (rsa384sig), rsaPubKeyPath, Digest::Algorithm::SHA224));
    ASSERT_FALSE (Signature::verify (sample, Base64::decode (rsa384sig), rsaPubKeyPath, Digest::Algorithm::SHA256));
    ASSERT_TRUE  (Signature::verify (sample, Base64::decode (rsa384sig), rsaPubKeyPath, Digest::Algorithm::SHA384)) << join::lastError.message ();
    ASSERT_FALSE (Signature::verify (sample, Base64::decode (rsa384sig), rsaPubKeyPath, Digest::Algorithm::SHA512));
}

/**
 * @brief Test sha512verify.
 */
TEST_F (SignatureTest, sha512verify)
{
    ASSERT_FALSE (Signature::verify (sample, Base64::decode (rsa512sig), rsaPubKeyPath, Digest::Algorithm::MD5));
    ASSERT_FALSE (Signature::verify (sample, Base64::decode (rsa512sig), rsaPubKeyPath, Digest::Algorithm::SHA1));
    ASSERT_FALSE (Signature::verify (sample, Base64::decode (rsa512sig), rsaPubKeyPath, Digest::Algorithm::SHA224));
    ASSERT_FALSE (Signature::verify (sample, Base64::decode (rsa512sig), rsaPubKeyPath, Digest::Algorithm::SHA256));
    ASSERT_FALSE (Signature::verify (sample, Base64::decode (rsa512sig), rsaPubKeyPath, Digest::Algorithm::SHA384));
    ASSERT_TRUE  (Signature::verify (sample, Base64::decode (rsa512sig), rsaPubKeyPath, Digest::Algorithm::SHA512)) << join::lastError.message ();
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
