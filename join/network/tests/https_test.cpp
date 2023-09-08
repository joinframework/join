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
#include <join/httpclient.hpp>
#include <join/httpserver.hpp>

// Libraries.
#include <gtest/gtest.h>

// C++.
#include <fstream>

using namespace std::chrono;

using join::Errc;
using join::TlsErrc;
using join::Resolver;
using join::HttpMethod;
using join::HttpRequest;
using join::HttpResponse;
using join::HttpErrc;
using join::Https;

/**
 * @brief Class used to test the HTTP API.
 */
class HttpsTest : public Https::Server, public ::testing::Test
{
public:
    /**
     * @brief Set up test case.
     */
    static void SetUpTestCase ()
    {
        mkdir (_basePath.c_str (), 0777);
        std::ofstream outFile (_sampleFile.c_str ());
        if (outFile.is_open ())
        {
            outFile << _sample;
            outFile.close ();
        }

        std::ofstream rootCertFile (_rootcert);
        if (rootCertFile.is_open ())
        {
            rootCertFile << "-----BEGIN CERTIFICATE-----" << std::endl;
            rootCertFile << "MIIChjCCAisCFBuHxbqMUGyl7OQUQcoRg3pOBJF+MAoGCCqGSM49BAMCMIHEMQsw" << std::endl;
            rootCertFile << "CQYDVQQGEwJGUjESMBAGA1UECAwJT2NjaXRhbmllMRAwDgYDVQQHDAdDYXN0cmVz" << std::endl;
            rootCertFile << "MRcwFQYDVQQKDA5Kb2luIEZyYW1ld29yazEtMCsGA1UECwwkSm9pbiBGcmFtZXdv" << std::endl;
            rootCertFile << "cmsgQ2VydGlmaWNhdGUgQXV0aG9yaXR5MR0wGwYDVQQDDBRjYS5qb2luZnJhbWV3" << std::endl;
            rootCertFile << "b3JrLm5ldDEoMCYGCSqGSIb3DQEJARYZc3VwcG9ydEBqb2luZnJhbWV3b3JrLm5l" << std::endl;
            rootCertFile << "dDAeFw0yMjA3MDUxNjMxMTZaFw0zMjA3MDIxNjMxMTZaMIHEMQswCQYDVQQGEwJG" << std::endl;
            rootCertFile << "UjESMBAGA1UECAwJT2NjaXRhbmllMRAwDgYDVQQHDAdDYXN0cmVzMRcwFQYDVQQK" << std::endl;
            rootCertFile << "DA5Kb2luIEZyYW1ld29yazEtMCsGA1UECwwkSm9pbiBGcmFtZXdvcmsgQ2VydGlm" << std::endl;
            rootCertFile << "aWNhdGUgQXV0aG9yaXR5MR0wGwYDVQQDDBRjYS5qb2luZnJhbWV3b3JrLm5ldDEo" << std::endl;
            rootCertFile << "MCYGCSqGSIb3DQEJARYZc3VwcG9ydEBqb2luZnJhbWV3b3JrLm5ldDBZMBMGByqG" << std::endl;
            rootCertFile << "SM49AgEGCCqGSM49AwEHA0IABASk0zCrKtXQi0Ycx+Anx+VWv8gncbPmNQ1yutii" << std::endl;
            rootCertFile << "gQjP2mF9NIqlxpcKNuE/6DDnfSzCEDhFyvGiK0NJ1C3RBowwCgYIKoZIzj0EAwID" << std::endl;
            rootCertFile << "SQAwRgIhAIFqdbxTb5kRjy4UY0N205ZEhHSMK89p2oUyn4iNbXH2AiEAtmV1UyRX" << std::endl;
            rootCertFile << "DIAGr/F+1SwQMPoJzSQxZ7NdxjNgW286e9Q=" << std::endl;
            rootCertFile << "-----END CERTIFICATE-----" << std::endl;
            rootCertFile.close ();
        }

        mkdir (_certPath.c_str (), 0777);
        std::ofstream certFile (_certFile);
        if (certFile.is_open ())
        {
            certFile << "-----BEGIN CERTIFICATE-----" << std::endl;
            certFile << "MIIDgDCCAyagAwIBAgIUR3ZIuKMt0BdaOZQnPwhSMR9qzfgwCgYIKoZIzj0EAwIw" << std::endl;
            certFile << "gcQxCzAJBgNVBAYTAkZSMRIwEAYDVQQIDAlPY2NpdGFuaWUxEDAOBgNVBAcMB0Nh" << std::endl;
            certFile << "c3RyZXMxFzAVBgNVBAoMDkpvaW4gRnJhbWV3b3JrMS0wKwYDVQQLDCRKb2luIEZy" << std::endl;
            certFile << "YW1ld29yayBDZXJ0aWZpY2F0ZSBBdXRob3JpdHkxHTAbBgNVBAMMFGNhLmpvaW5m" << std::endl;
            certFile << "cmFtZXdvcmsubmV0MSgwJgYJKoZIhvcNAQkBFhlzdXBwb3J0QGpvaW5mcmFtZXdv" << std::endl;
            certFile << "cmsubmV0MB4XDTIyMDcwNzEyMTIxMFoXDTMyMDcwNDEyMTIxMFowgagxCzAJBgNV" << std::endl;
            certFile << "BAYTAkZSMRIwEAYDVQQIDAlPY2NpdGFuaWUxEDAOBgNVBAcMB0Nhc3RyZXMxFzAV" << std::endl;
            certFile << "BgNVBAoMDkpvaW4gRnJhbWV3b3JrMRswGQYDVQQLDBJKb2luIEZyYW1ld29yayBE" << std::endl;
            certFile << "ZXYxEzARBgNVBAMMCmxvY2FsaG9zdC4xKDAmBgkqhkiG9w0BCQEWGXN1cHBvcnRA" << std::endl;
            certFile << "am9pbmZyYW1ld29yay5uZXQwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIB" << std::endl;
            certFile << "AQDSNtw5zEoJFPf6Rl0Y1n8BQfE0YTPCELvFAeioUfj8CAnUleHL9pwAEFg6kgoG" << std::endl;
            certFile << "hvwto5/yWGPUqNNfe3xbFTJcHgMhgtjqy5H6sYDkTi3kYIIMBfTHr8NI7HWE8Nz1" << std::endl;
            certFile << "qU1snjtERnkoLilIZf/2BojNVMtHC1H316WbMicXS0v7HQo3lv6PYSana9Q9ow9O" << std::endl;
            certFile << "2/FiW5qq1eOhI1ZedRanX+bl0jHWCd3WsI87+5bTaQrfetdHTOmav6O17Iq9FiTh" << std::endl;
            certFile << "Sg9fbM3s2Hw15kI+mws029dhcwXs5sYY+NgtrQwjR5qH+54BdUaPwQfl/KyulfEl" << std::endl;
            certFile << "TJykJ+3w6MorxUr55F68uBNbAgMBAAGjRTBDMAsGA1UdDwQEAwIF4DAdBgNVHSUE" << std::endl;
            certFile << "FjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwFQYDVR0RBA4wDIIKbG9jYWxob3N0LjAK" << std::endl;
            certFile << "BggqhkjOPQQDAgNIADBFAiA120ufIbhcw7BJQ1L6WudDdW2mHrVXvdgeOzVGgz1d" << std::endl;
            certFile << "iAIhAMm/sWI3yzb2IMPffxWKYusWEQE2hZvs24ESSC/ZZ0s+" << std::endl;
            certFile << "-----END CERTIFICATE-----" << std::endl;
            certFile.close ();
        }

        [[maybe_unused]] int result;
        result = std::system (std::string ("/usr/bin/c_rehash " + _certPath).c_str ());

        std::ofstream keyFile (_key);
        if (keyFile.is_open ())
        {
            keyFile << "-----BEGIN RSA PRIVATE KEY-----" << std::endl;
            keyFile << "MIIEowIBAAKCAQEA0jbcOcxKCRT3+kZdGNZ/AUHxNGEzwhC7xQHoqFH4/AgJ1JXh" << std::endl;
            keyFile << "y/acABBYOpIKBob8LaOf8lhj1KjTX3t8WxUyXB4DIYLY6suR+rGA5E4t5GCCDAX0" << std::endl;
            keyFile << "x6/DSOx1hPDc9alNbJ47REZ5KC4pSGX/9gaIzVTLRwtR99elmzInF0tL+x0KN5b+" << std::endl;
            keyFile << "j2Emp2vUPaMPTtvxYluaqtXjoSNWXnUWp1/m5dIx1gnd1rCPO/uW02kK33rXR0zp" << std::endl;
            keyFile << "mr+jteyKvRYk4UoPX2zN7Nh8NeZCPpsLNNvXYXMF7ObGGPjYLa0MI0eah/ueAXVG" << std::endl;
            keyFile << "j8EH5fysrpXxJUycpCft8OjKK8VK+eRevLgTWwIDAQABAoIBAAzdlK7o5OMXaHHl" << std::endl;
            keyFile << "2o7Jme5Oxd9pz4wiEAvnqQCcO7vZFhjvr2kXR8btOSkkhP6PRmHYsNJZPIroZj9i" << std::endl;
            keyFile << "xGKisnlW0OQ9KN995ApO0M+oRUDD81GfD7Mk+7O73Rls0GksmnN6X7A3C/U8lgQ7" << std::endl;
            keyFile << "UeYR0k+Wz/YiKDsd9KHB+QiA8D6HFQ9I8Y2P97KOcYnxXZfSwNm+ENNU3wShZOl2" << std::endl;
            keyFile << "ZYJJ4DE+5m2SwZ6g8b5Zre4cDbOduwuz/jXzjy2tAZBlTS4DVpYlhd14z+ssUWiu" << std::endl;
            keyFile << "AdS/nqSF7Obj0TRhoGNfrkisFzV4itavQ5DKGj/6hjueIJVLteUOzcCeg26YosNy" << std::endl;
            keyFile << "QzZSjOECgYEA7y3InEoh93/4HZCZmdwN8KfZtqirX0t966FntgAT8RkIs+KvNS8B" << std::endl;
            keyFile << "m3RfNLa/EuDt5zTmHRGx+oeN+17i9QQjKWcR0NnJ6aSZbvJByj3yKxLF9XVllzp/" << std::endl;
            keyFile << "vHSSyB264RoKIrWmFN6cCO4u4h9ZPY75pASWBCDMdnGK8axAcqAnlqsCgYEA4P+Y" << std::endl;
            keyFile << "FF9RW4rhrVU4dpXSfcr6vOwqfp9F9vhTVL0JS/SLOFoJNNpS9Rnq3pVLEuKyCphd" << std::endl;
            keyFile << "3nk9VFfoRygmMaGBvwGaXZPPvosoaIUgOdTt7KIfSHPichBEVxRuWCrtTGGkG0ok" << std::endl;
            keyFile << "s/RPHhvxZE267vsVj1PktK8Yr5Ba0AL2ycztNhECgYB5OAwHYe8LIBlg6otelk+e" << std::endl;
            keyFile << "W4OU9rE8L+eWx4vniuyQce6eNNI1syguYHFsJv56E/OfDYlezDwWzCLidnmyUjF7" << std::endl;
            keyFile << "51f5MJgLyTdWKoO7e1/EAtS/jYs6dRSOL8rAj4jKU0c1xjhxNU2BnS23vsmc0Fyn" << std::endl;
            keyFile << "iwd4+iKGGQ+hYnqbXZ4S1wKBgD/3an0gPDkSWua0e8D7B0TMGEztt4cYMQPtxYMp" << std::endl;
            keyFile << "2yLE+2+h6UwlZcBZBfUR7K4J1SQ9/THqtgzskRTpzTH/AKwVAJXqF/3MAkj00Byg" << std::endl;
            keyFile << "9KN50/r9NzvGdCdtn5FhYuV8PPOlOJoQsw2UVCR4FNUsfQyqhTL5NMN0/tx0e0UU" << std::endl;
            keyFile << "BbyBAoGBANu5ifByauVELH8UEl5rXRu1S9iAVV+Bc5jboXwc4VxJtEyomGJ7+YdL" << std::endl;
            keyFile << "5c9LFV+STUp7CE12uSXQZTQM0tEjPinLntRinNzu9tIHR1vy7FZHEwMFIgB4VTY7" << std::endl;
            keyFile << "ALRYv1/QpTuywpNUFRS15JkfGNf5JIkrUEWLgkX3OVCBsRGHUugy" << std::endl;
            keyFile << "-----END RSA PRIVATE KEY-----" << std::endl;
            keyFile.close ();
        }
    }

    /**
     * @brief Tear down test case.
     */
    static void TearDownTestCase ()
    {
        unlink (_sampleFile.c_str ());
        rmdir  (_basePath.c_str ());
        unlink (_rootcert.c_str ());
        unlink (_certFile.c_str ());
        rmdir  (_certPath.c_str ());
        unlink (_key.c_str ());
    }

protected:
    /**
     * @brief Sets up the test fixture.
     */
    void SetUp ()
    {
        this->baseLocation (_basePath + "/");
        ASSERT_EQ (this->baseLocation (), _basePath);
        this->uploadLocation (_uploadPath + "/");
        ASSERT_EQ (this->uploadLocation (), _uploadPath);
        this->keepAlive (seconds (_timeout), _max);
        ASSERT_EQ (this->keepAliveTimeout (), seconds (_timeout));
        ASSERT_EQ (this->keepAliveMax (), _max);
        ASSERT_EQ (this->setCertificate (_certFile, _key), 0) << join::lastError.message ();
        ASSERT_EQ (this->setCipher (join::defaultCipher_), 0) << join::lastError.message ();
    #if OPENSSL_VERSION_NUMBER >= 0x10101000L
        ASSERT_EQ (this->setCipher_1_3 (join::defaultCipher_1_3_), 0) << join::lastError.message ();
    #endif
        this->addAlias ("/", "", _sampleFile);
        this->addAlias ("/authorized/", "file", _sampleFile, accessHandler);
        this->addDocumentRoot ("/", "*");
        this->addDocumentRoot ("/no/", "file");
        this->addRedirect ("/redirect/", "file", "https://$host:$port/");
        this->addExecute (HttpMethod::Get, "/exec/", "null", nullptr);
        this->addExecute (HttpMethod::Get, "/exec/", "get", getHandler);
        this->addExecute (HttpMethod::Post, "/exec/", "post", postHandler);
        this->addUpload ("/upload/", "null", nullptr);
        ASSERT_EQ (this->create ({Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
        ASSERT_EQ (this->create ({Resolver::resolveHost (_host), _port}), -1);
        ASSERT_EQ (join::lastError, Errc::InUse);
    }

    /**
     * @brief Tears down the test fixture.
     */
    void TearDown ()
    {
        this->close ();
    }

    /**
     * @brief handle authentication.
     * @param worker worker thread context.
     * @param errc error code.
     * @return true on success, false otherwise.
     */
    static bool accessHandler (Https::Worker* worker, std::error_code& errc)
    {
        std::string authorisation = worker->header ("Authorization");
        std::string authType      = authorisation.substr (0, authorisation.find (' '));
        std::string token         = authorisation.substr (authorisation.find (' ') + 1);
        if (authType != "Bearer")
        {
            errc = make_error_code (HttpErrc::Unauthorized);
            return false;
        }
        if (token != _token)
        {
            errc = make_error_code (HttpErrc::Forbidden);
            return false;
        }
        return true;
    }

    /**
     * @brief handle dynamic get content.
     * @param worker worker thread context.
     */
    static void getHandler (Https::Worker* worker)
    {
        worker->header ("Content-Type", "text/html");
        if (worker->hasHeader ("Accept-Encoding"))
        {
            if (worker->header ("Accept-Encoding").find ("gzip") != std::string::npos)
            {
                worker->header ("Content-Encoding", "gzip");
            }
            else if (worker->header ("Accept-Encoding").find ("deflate") != std::string::npos)
            {
                worker->header ("Content-Encoding", "deflate");
            }
        }
        worker->header ("Transfer-Encoding", "chunked");
        worker->sendHeaders ();
        worker->write (_sample.c_str (), _sample.size ());
        worker->flush ();
    }

    /**
     * @brief handle dynamic post content.
     * @param worker worker thread context.
     */
    static void postHandler (Https::Worker* worker)
    {
        std::string data;
        data.resize (4);
        worker->read (&data[0], data.size ());
        if (data == "test")
        {
            worker->sendHeaders ();
        }
        else
        {
            worker->sendError ("400", "Bad Request");
        }
        worker->flush ();
    }

    /// base path.
    static const std::string _basePath;

    /// upload path.
    static const std::string _uploadPath;

    /// sample.
    static const std::string _sample;

    /// sample file name.
    static const std::string _sampleFileName;

    /// sample path.
    static const std::string _sampleFile;

    /// session token.
    static const std::string _token;

    /// server hostname.
    static const std::string _host;

    /// server port.
    static const uint16_t _port;

    /// server keep alive timeout.
    static const int _timeout;

    /// server  keep alive max requests.
    static const int _max;

    /// root certificate.
    static const std::string _rootcert;

    /// certificate path.
    static const std::string _certPath;

    /// certificate file.
    static const std::string _certFile;

    /// private key.
    static const std::string _key;
};

const std::string HttpsTest::_basePath       = "/tmp/www";
const std::string HttpsTest::_uploadPath     = "/tmp/upload";
const std::string HttpsTest::_sample         = "<html><body><h1>It works!</h1></body></html>";
const std::string HttpsTest::_sampleFileName = "sample.html";
const std::string HttpsTest::_sampleFile     = _basePath + "/" + _sampleFileName;
const std::string HttpsTest::_token          = "adlSaJkmBLpgnRRCjkCgQ4uaCagKHsIN";
const std::string HttpsTest::_host           = "localhost";
const uint16_t    HttpsTest::_port           = 5000;
const int         HttpsTest::_timeout        = 5;
const int         HttpsTest::_max            = 20;
const std::string HttpsTest::_rootcert       = "/tmp/https_test_root.cert";
const std::string HttpsTest::_certPath       = "/tmp/certs";
const std::string HttpsTest::_certFile       = _certPath + "/https_test.cert";
const std::string HttpsTest::_key            = "/tmp/https_test.key";

/**
 * @brief Test move.
 */
TEST_F (HttpsTest, move)
{
    Https::Client client1 ("127.0.0.1", 5000), client2 ("127.0.0.2", 5001);
    ASSERT_EQ (client1.host (), "127.0.0.1");
    ASSERT_EQ (client1.port (), 5000);
    ASSERT_EQ (client2.host (), "127.0.0.2");
    ASSERT_EQ (client2.port (), 5001);

    client1 = std::move (client2);
    ASSERT_EQ (client1.host (), "127.0.0.2");
    ASSERT_EQ (client1.port (), 5001);

    Https::Client client3 (std::move (client1));
    ASSERT_EQ (client3.host (), "127.0.0.2");
    ASSERT_EQ (client3.port (), 5001);
}

/**
 * @brief Test scheme method
 */
TEST_F (HttpsTest, scheme)
{
    Https::Client client1 ("localhost", 80);
    ASSERT_EQ (client1.scheme (), "https");

    Https::Client client2 ("localhost", 443);
    ASSERT_EQ (client2.scheme (), "https");
}

/**
 * @brief Test host method
 */
TEST_F (HttpsTest, host)
{
    Https::Client client1 ("91.66.32.78", 80);
    ASSERT_EQ (client1.host (), "91.66.32.78");

    Https::Client client2 ("localhost", 80);
    ASSERT_EQ (client2.host (), "localhost");
}

/**
 * @brief Test port method
 */
TEST_F (HttpsTest, port)
{
    Https::Client client1 ("91.66.32.78", 80);
    ASSERT_EQ (client1.port (), 80);

    Https::Client client2 ("91.66.32.78", 5000);
    ASSERT_EQ (client2.port (), 5000);
}

/**
 * @brief Test authority method
 */
TEST_F (HttpsTest, authority)
{
    ASSERT_EQ (Https::Client ("localhost", 80).authority (), "localhost:80");
    ASSERT_EQ (Https::Client ("localhost", 443).authority (), "localhost");
    ASSERT_EQ (Https::Client ("localhost", 5000).authority (), "localhost:5000");

    ASSERT_EQ (Https::Client ("91.66.32.78", 80).authority (), "91.66.32.78:80");
    ASSERT_EQ (Https::Client ("91.66.32.78", 443).authority (), "91.66.32.78");
    ASSERT_EQ (Https::Client ("91.66.32.78", 5000).authority (), "91.66.32.78:5000");

    ASSERT_EQ (Https::Client ("2001:db8:1234:5678::1", 80).authority (), "[2001:db8:1234:5678::1]:80");
    ASSERT_EQ (Https::Client ("2001:db8:1234:5678::1", 443).authority (), "[2001:db8:1234:5678::1]");
    ASSERT_EQ (Https::Client ("2001:db8:1234:5678::1", 5000).authority (), "[2001:db8:1234:5678::1]:5000");
}

/**
 * @brief Test url method
 */
TEST_F (HttpsTest, url)
{
    ASSERT_EQ (Https::Client ("localhost", 80).url (), "https://localhost:80/");
    ASSERT_EQ (Https::Client ("localhost", 443).url (), "https://localhost/");
    ASSERT_EQ (Https::Client ("localhost", 5000).url (), "https://localhost:5000/");

    ASSERT_EQ (Https::Client ("91.66.32.78", 80).url (), "https://91.66.32.78:80/");
    ASSERT_EQ (Https::Client ("91.66.32.78", 443).url (), "https://91.66.32.78/");
    ASSERT_EQ (Https::Client ("91.66.32.78", 5000).url (), "https://91.66.32.78:5000/");

    ASSERT_EQ (Https::Client ("2001:db8:1234:5678::1", 80).url (), "https://[2001:db8:1234:5678::1]:80/");
    ASSERT_EQ (Https::Client ("2001:db8:1234:5678::1", 443).url (), "https://[2001:db8:1234:5678::1]/");
    ASSERT_EQ (Https::Client ("2001:db8:1234:5678::1", 5000).url (), "https://[2001:db8:1234:5678::1]:5000/");
}

/**
 * @brief Test keepAlive method
 */
TEST_F (HttpsTest, keepAlive)
{
    Https::Client client1 ("localhost", 80);
    ASSERT_TRUE (client1.keepAlive ());

    client1.keepAlive (false);
    ASSERT_FALSE (client1.keepAlive ());

    Https::Client client2 ("localhost", 80, false);
    ASSERT_FALSE (client2.keepAlive ());

    client2.keepAlive (true);
    ASSERT_TRUE (client2.keepAlive ());
}

/**
 * @brief Test keepAliveTimeout method
 */
TEST_F (HttpsTest, keepAliveTimeout)
{
    Https::Client client (_host, _port);
    client.setVerify (true, 1);
    ASSERT_EQ (client.setCaFile (_rootcert), 0) << join::lastError.message ();
    ASSERT_EQ (client.setCipher (join::defaultCipher_), 0) << join::lastError.message ();
#if OPENSSL_VERSION_NUMBER >= 0x10101000L
    ASSERT_EQ (client.setCipher_1_3 (join::defaultCipher_1_3_), 0) << join::lastError.message ();
#endif
    ASSERT_EQ (client.keepAliveTimeout (), seconds::zero ());

    HttpRequest request;
    request.method (HttpMethod::Head);
    request.header ("Connection", "keep-alive");
    client << request;
    ASSERT_TRUE (client.good ()) << join::lastError.message ();
    ASSERT_EQ (client.keepAliveTimeout (), seconds::zero ());

    HttpResponse response;
    client >> response;
    ASSERT_TRUE (client.good ()) << join::lastError.message ();
    ASSERT_EQ (client.keepAliveTimeout (), seconds (_timeout));

    request.header ("Connection", "close");
    client << request;
    ASSERT_TRUE (client.good ()) << join::lastError.message ();
    ASSERT_EQ (client.keepAliveTimeout (), seconds (_timeout));

    client >> response;
    ASSERT_TRUE (client.good ()) << join::lastError.message ();
    ASSERT_EQ (client.keepAliveTimeout (), seconds::zero ());

    client.close ();
    ASSERT_TRUE (client.good ()) << join::lastError.message ();
    ASSERT_EQ (client.keepAliveTimeout (), seconds::zero ());
}

/**
 * @brief Test keepAliveMax method
 */
TEST_F (HttpsTest, keepAliveMax)
{
    Https::Client client (_host, _port);
    client.setVerify (true, 1);
    ASSERT_EQ (client.setCaFile (_rootcert), 0) << join::lastError.message ();
    ASSERT_EQ (client.setCipher (join::defaultCipher_), 0) << join::lastError.message ();
#if OPENSSL_VERSION_NUMBER >= 0x10101000L
    ASSERT_EQ (client.setCipher_1_3 (join::defaultCipher_1_3_), 0) << join::lastError.message ();
#endif
    ASSERT_EQ (client.keepAliveMax (), -1);

    HttpRequest request;
    request.method (HttpMethod::Head);
    request.header ("Connection", "keep-alive");
    client << request;
    ASSERT_TRUE (client.good ()) << join::lastError.message ();
    ASSERT_EQ (client.keepAliveMax (), -1);

    HttpResponse response;
    client >> response;
    ASSERT_TRUE (client.good ()) << join::lastError.message ();
    ASSERT_EQ (client.keepAliveMax (), _max);

    request.header ("Connection", "close");
    client << request;
    ASSERT_TRUE (client.good ()) << join::lastError.message ();
    ASSERT_EQ (client.keepAliveMax (), _max);

    client >> response;
    ASSERT_TRUE (client.good ()) << join::lastError.message ();
    ASSERT_EQ (client.keepAliveMax (), 0);

    client.close ();
    ASSERT_TRUE (client.good ()) << join::lastError.message ();
    ASSERT_EQ (client.keepAliveMax (), -1);
}

/**
 * @brief Test bad request
 */
TEST_F (HttpsTest, badRequest)
{
    Https::Client client (_host, _port);
    client.setVerify (true, 1);
    ASSERT_EQ (client.setCaFile (_rootcert), 0) << join::lastError.message ();
    ASSERT_EQ (client.setCipher (join::defaultCipher_), 0) << join::lastError.message ();
#if OPENSSL_VERSION_NUMBER >= 0x10101000L
    ASSERT_EQ (client.setCipher_1_3 (join::defaultCipher_1_3_), 0) << join::lastError.message ();
#endif

    HttpRequest request;
    request.path ("\r\n");
    ASSERT_EQ (client.send (request), 0) << join::lastError.message ();

    HttpResponse response;
    ASSERT_EQ (client.receive (response), 0) << join::lastError.message ();
    ASSERT_EQ (response.status (), "400");
    ASSERT_EQ (response.reason (), "Bad Request");

    request.clear ();
    request.header ("Host", "");
    ASSERT_EQ (client.send (request), 0) << join::lastError.message ();

    response.clear ();
    ASSERT_EQ (client.receive (response), 0) << join::lastError.message ();
    ASSERT_EQ (response.status (), "400");
    ASSERT_EQ (response.reason (), "Bad Request");
}

/**
 * @brief Test invalid method
 */
TEST_F (HttpsTest, invalidMethod)
{
    Https::Client client (_host, _port);
    client.setVerify (true, 1);
    ASSERT_EQ (client.setCaFile (_rootcert), 0) << join::lastError.message ();
    ASSERT_EQ (client.setCipher (join::defaultCipher_), 0) << join::lastError.message ();
#if OPENSSL_VERSION_NUMBER >= 0x10101000L
    ASSERT_EQ (client.setCipher_1_3 (join::defaultCipher_1_3_), 0) << join::lastError.message ();
#endif

    HttpRequest request;
    request.method (HttpMethod (100));
    ASSERT_EQ (client.send (request), 0) << join::lastError.message ();

    HttpResponse response;
    ASSERT_EQ (client.receive (response), 0) << join::lastError.message ();
    ASSERT_EQ (response.status (), "405");
    ASSERT_EQ (response.reason (), "Method Not Allowed");
}

/**
 * @brief Test header too large
 */
TEST_F (HttpsTest, headerTooLarge)
{
    Https::Client client (_host, _port);
    client.setVerify (true, 1);
    ASSERT_EQ (client.setCaFile (_rootcert), 0) << join::lastError.message ();
    ASSERT_EQ (client.setCipher (join::defaultCipher_), 0) << join::lastError.message ();
#if OPENSSL_VERSION_NUMBER >= 0x10101000L
    ASSERT_EQ (client.setCipher_1_3 (join::defaultCipher_1_3_), 0) << join::lastError.message ();
#endif

    HttpRequest request;
    request.header ("User-Agent", std::string (8192, 'a'));
    ASSERT_EQ (client.send (request), 0) << join::lastError.message ();

    HttpResponse response;
    ASSERT_EQ (client.receive (response), 0) << join::lastError.message ();
    ASSERT_EQ (response.status (), "494");
    ASSERT_EQ (response.reason (), "Request Header Too Large");
}

/**
 * @brief Test not found
 */
TEST_F (HttpsTest, notFound)
{
    Https::Client client (_host, _port);
    client.setVerify (true, 1);
    ASSERT_EQ (client.setCaFile (_rootcert), 0) << join::lastError.message ();
    ASSERT_EQ (client.setCipher (join::defaultCipher_), 0) << join::lastError.message ();
#if OPENSSL_VERSION_NUMBER >= 0x10101000L
    ASSERT_EQ (client.setCipher_1_3 (join::defaultCipher_1_3_), 0) << join::lastError.message ();
#endif

    HttpRequest request;
    request.path ("/invalid/path");
    ASSERT_EQ (client.send (request), 0) << join::lastError.message ();

    HttpResponse response;
    ASSERT_EQ (client.receive (response), 0) << join::lastError.message ();
    ASSERT_EQ (response.status (), "404");
    ASSERT_EQ (response.reason (), "Not Found");

    request.clear ();
    request.path ("/no/file");
    ASSERT_EQ (client.send (request), 0) << join::lastError.message ();

    response.clear ();
    ASSERT_EQ (client.receive (response), 0) << join::lastError.message ();
    ASSERT_EQ (response.status (), "404");
    ASSERT_EQ (response.reason (), "Not Found");
}

/**
 * @brief Test not modified
 */
TEST_F (HttpsTest, notModified)
{
    struct stat sbuf;
    std::stringstream modifTime;
    ASSERT_EQ (stat (_sampleFile.c_str (), &sbuf), 0);
    modifTime << std::put_time (std::gmtime (&sbuf.st_ctime), "%a, %d %b %Y %H:%M:%S GMT");

    Https::Client client (_host, _port);
    client.setVerify (true, 1);
    ASSERT_EQ (client.setCaFile (_rootcert), 0) << join::lastError.message ();
    ASSERT_EQ (client.setCipher (join::defaultCipher_), 0) << join::lastError.message ();
#if OPENSSL_VERSION_NUMBER >= 0x10101000L
    ASSERT_EQ (client.setCipher_1_3 (join::defaultCipher_1_3_), 0) << join::lastError.message ();
#endif

    HttpRequest request;
    request.header ("If-Modified-Since", modifTime.str ());
    ASSERT_EQ (client.send (request), 0) << join::lastError.message ();

    HttpResponse response;
    ASSERT_EQ (client.receive (response), 0) << join::lastError.message ();
    ASSERT_EQ (response.status (), "304");
    ASSERT_EQ (response.reason (), "Not Modified");
}

/**
 * @brief Test redirect
 */
TEST_F (HttpsTest, redirect)
{
    Https::Client client (_host, _port);
    client.setVerify (true, 1);
    ASSERT_EQ (client.setCaFile (_rootcert), 0) << join::lastError.message ();
    ASSERT_EQ (client.setCipher (join::defaultCipher_), 0) << join::lastError.message ();
#if OPENSSL_VERSION_NUMBER >= 0x10101000L
    ASSERT_EQ (client.setCipher_1_3 (join::defaultCipher_1_3_), 0) << join::lastError.message ();
#endif

    HttpRequest request;
    request.path ("/redirect/file");
    ASSERT_EQ (client.send (request), 0) << join::lastError.message ();

    HttpResponse response;
    ASSERT_EQ (client.receive (response), 0) << join::lastError.message ();
    ASSERT_EQ (response.status (), "307");
    ASSERT_EQ (response.reason (), "Temporary Redirect");

    int len = std::stoi (response.header ("Content-Length"));
    ASSERT_GT (len, 0);
    std::string payload;
    payload.resize (len);
    client.read (&payload[0], payload.size ());

    request.clear ();
    request.path ("/redirect/file");
    request.version ("HTTP/1.0");
    ASSERT_EQ (client.send (request), 0) << join::lastError.message ();

    response.clear ();
    ASSERT_EQ (client.receive (response), 0) << join::lastError.message ();
    ASSERT_EQ (response.status (), "302");
    ASSERT_EQ (response.reason (), "Found");

    len = std::stoi (response.header ("Content-Length"));
    ASSERT_GT (len, 0);
    payload.resize (len);
    client.read (&payload[0], payload.size ());
}

/**
 * @brief Test server error
 */
TEST_F (HttpsTest, serverError)
{
    Https::Client client (_host, _port);
    client.setVerify (true, 1);
    ASSERT_EQ (client.setCaFile (_rootcert), 0) << join::lastError.message ();
    ASSERT_EQ (client.setCipher (join::defaultCipher_), 0) << join::lastError.message ();
#if OPENSSL_VERSION_NUMBER >= 0x10101000L
    ASSERT_EQ (client.setCipher_1_3 (join::defaultCipher_1_3_), 0) << join::lastError.message ();
#endif

    HttpRequest request;
    request.path ("/exec/null");
    ASSERT_EQ (client.send (request), 0) << join::lastError.message ();

    HttpResponse response;
    ASSERT_EQ (client.receive (response), 0) << join::lastError.message ();
    ASSERT_EQ (response.status (), "500");
    ASSERT_EQ (response.reason (), "Internal Server Error");

    request.clear ();
    request.method (HttpMethod::Post);
    request.path ("/upload/null");
    ASSERT_EQ (client.send (request), 0) << join::lastError.message ();

    response.clear ();
    ASSERT_EQ (client.receive (response), 0) << join::lastError.message ();
    ASSERT_EQ (response.status (), "500");
    ASSERT_EQ (response.reason (), "Internal Server Error");

    client.close ();
    ASSERT_TRUE (client.good ()) << join::lastError.message ();
}

/**
 * @brief Test unauthorized
 */
TEST_F (HttpsTest, unauthorized)
{
    Https::Client client (_host, _port);
    client.setVerify (true, 1);
    ASSERT_EQ (client.setCaFile (_rootcert), 0) << join::lastError.message ();
    ASSERT_EQ (client.setCipher (join::defaultCipher_), 0) << join::lastError.message ();
#if OPENSSL_VERSION_NUMBER >= 0x10101000L
    ASSERT_EQ (client.setCipher_1_3 (join::defaultCipher_1_3_), 0) << join::lastError.message ();
#endif

    HttpRequest request;
    request.path ("/authorized/file");
    ASSERT_EQ (client.send (request), 0) << join::lastError.message ();

    HttpResponse response;
    ASSERT_EQ (client.receive (response), 0) << join::lastError.message ();
    ASSERT_EQ (response.status (), "401");
    ASSERT_EQ (response.reason (), "Unauthorized");

    request.clear ();
    request.path ("/authorized/file");
    request.header ("Authorization", "Basic YWxhZGRpbjpvcGVuc2VzYW1l");
    ASSERT_EQ (client.send (request), 0) << join::lastError.message ();

    response.clear ();
    ASSERT_EQ (client.receive (response), 0) << join::lastError.message ();
    ASSERT_EQ (response.status (), "401");
    ASSERT_EQ (response.reason (), "Unauthorized");

    client.close ();
    ASSERT_TRUE (client.good ()) << join::lastError.message ();
}

/**
 * @brief Test forbidden
 */
TEST_F (HttpsTest, forbidden)
{
    Https::Client client (_host, _port);

    HttpRequest request;
    request.path ("/authorized/file");
    request.header ("Authorization", "Bearer YWxhZGRpbjpzZXNhbWVPdXZyZVRvaQ");
    ASSERT_EQ (client.send (request), 0) << join::lastError.message ();

    HttpResponse response;
    ASSERT_EQ (client.receive (response), 0) << join::lastError.message ();
    ASSERT_EQ (response.status (), "403");
    ASSERT_EQ (response.reason (), "Forbidden");

    client.close ();
    ASSERT_TRUE (client.good ()) << join::lastError.message ();
}

/**
 * @brief Test head
 */
TEST_F (HttpsTest, head)
{
    Https::Client client (_host, _port);
    client.setVerify (true, 1);
    ASSERT_EQ (client.setCaFile (_rootcert), 0) << join::lastError.message ();
    ASSERT_EQ (client.setCipher (join::defaultCipher_), 0) << join::lastError.message ();
#if OPENSSL_VERSION_NUMBER >= 0x10101000L
    ASSERT_EQ (client.setCipher_1_3 (join::defaultCipher_1_3_), 0) << join::lastError.message ();
#endif

    HttpRequest request;
    request.method (HttpMethod::Head);
    ASSERT_EQ (client.send (request), 0) << join::lastError.message ();

    HttpResponse response;
    ASSERT_EQ (client.receive (response), 0) << join::lastError.message ();
    ASSERT_EQ (response.status (), "200");
    ASSERT_EQ (response.reason (), "OK");

    request.clear ();
    request.method (HttpMethod::Head);
    request.path ("/authorized/file");
    request.header ("Authorization", "Bearer adlSaJkmBLpgnRRCjkCgQ4uaCagKHsIN");
    ASSERT_EQ (client.send (request), 0) << join::lastError.message ();

    response.clear ();
    ASSERT_EQ (client.receive (response), 0) << join::lastError.message ();
    ASSERT_EQ (response.status (), "200");
    ASSERT_EQ (response.reason (), "OK");

    client.close ();
    ASSERT_TRUE (client.good ()) << join::lastError.message ();
}

/**
 * @brief Test get
 */
TEST_F (HttpsTest, get)
{
    Https::Client client (_host, _port);
    client.setVerify (true, 1);
    ASSERT_EQ (client.setCaFile (_rootcert), 0) << join::lastError.message ();
    ASSERT_EQ (client.setCipher (join::defaultCipher_), 0) << join::lastError.message ();
#if OPENSSL_VERSION_NUMBER >= 0x10101000L
    ASSERT_EQ (client.setCipher_1_3 (join::defaultCipher_1_3_), 0) << join::lastError.message ();
#endif

    HttpRequest request;
    request.method (HttpMethod::Get);
    ASSERT_EQ (client.send (request), 0) << join::lastError.message ();

    HttpResponse response;
    ASSERT_EQ (client.receive (response), 0) << join::lastError.message ();
    ASSERT_EQ (response.status (), "200");
    ASSERT_EQ (response.reason (), "OK");

    ASSERT_EQ (std::stoi (response.header ("Content-Length")), _sample.size ());
    std::string payload;
    payload.resize (_sample.size ());
    client.read (&payload[0], payload.size ());
    ASSERT_EQ (payload, _sample);

    request.clear ();
    request.method (HttpMethod::Get);
    request.path ("/exec/get");
    request.header ("Accept-Encoding", "gzip");
    ASSERT_EQ (client.send (request), 0) << join::lastError.message ();

    response.clear ();
    ASSERT_EQ (client.receive (response), 0) << join::lastError.message ();
    ASSERT_EQ (response.status (), "200");
    ASSERT_EQ (response.reason (), "OK");

    payload.clear ();
    payload.resize (_sample.size ());
    client.read (&payload[0], payload.size ());
    ASSERT_EQ (payload, _sample);

    request.clear ();
    request.method (HttpMethod::Get);
    request.path ("/exec/get");
    request.header ("Accept-Encoding", "deflate");
    ASSERT_EQ (client.send (request), 0) << join::lastError.message ();

    response.clear ();
    ASSERT_EQ (client.receive (response), 0) << join::lastError.message ();
    ASSERT_EQ (response.status (), "200");
    ASSERT_EQ (response.reason (), "OK");

    payload.clear ();
    payload.resize (_sample.size ());
    client.read (&payload[0], payload.size ());
    ASSERT_EQ (payload, _sample);

    client.close ();
    ASSERT_TRUE (client.good ()) << join::lastError.message ();
}

/**
 * @brief Test post
 */
TEST_F (HttpsTest, post)
{
    Https::Client client (_host, _port);
    client.setVerify (true, 1);
    ASSERT_EQ (client.setCaFile (_rootcert), 0) << join::lastError.message ();
    ASSERT_EQ (client.setCipher (join::defaultCipher_), 0) << join::lastError.message ();
#if OPENSSL_VERSION_NUMBER >= 0x10101000L
    ASSERT_EQ (client.setCipher_1_3 (join::defaultCipher_1_3_), 0) << join::lastError.message ();
#endif

    HttpRequest request;
    request.method (HttpMethod::Post);
    request.path ("/exec/post");
    request.header ("Content-Length", "4");
    ASSERT_EQ (client.send (request), 0) << join::lastError.message ();
    ASSERT_TRUE (client.write ("test", 4)) << join::lastError.message ();
    client.flush ();

    HttpResponse response;
    ASSERT_EQ (client.receive (response), 0) << join::lastError.message ();
    ASSERT_EQ (response.status (), "200");
    ASSERT_EQ (response.reason (), "OK");

    request.clear ();
    request.method (HttpMethod::Post);
    request.path ("/exec/post");
    request.header ("Transfer-Encoding", "chunked");
    request.header ("Content-Encoding", "gzip");
    ASSERT_EQ (client.send (request), 0) << join::lastError.message ();
    ASSERT_TRUE (client.write ("test", 4)) << join::lastError.message ();
    client.flush ();

    response.clear ();
    ASSERT_EQ (client.receive (response), 0) << join::lastError.message ();
    ASSERT_EQ (response.status (), "200");
    ASSERT_EQ (response.reason (), "OK");
    ASSERT_TRUE (client.good ()) << join::lastError.message ();

    request.clear ();
    request.method (HttpMethod::Post);
    request.path ("/exec/post");
    request.header ("Transfer-Encoding", "chunked");
    request.header ("Content-Encoding", "deflate");
    ASSERT_EQ (client.send (request), 0) << join::lastError.message ();
    ASSERT_TRUE (client.write ("test", 4)) << join::lastError.message ();
    client.flush ();

    response.clear ();
    ASSERT_EQ (client.receive (response), 0) << join::lastError.message ();
    ASSERT_EQ (response.status (), "200");
    ASSERT_EQ (response.reason (), "OK");

    client.close ();
    ASSERT_TRUE (client.good ()) << join::lastError.message ();
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
