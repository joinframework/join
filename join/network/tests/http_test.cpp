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
using join::Http;

/**
 * @brief Class used to test the HTTP API.
 */
class HttpTest : public Http::Server, public ::testing::Test
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
            outFile << _sample << std::endl;
            outFile.close ();
        }
    }

    /**
     * @brief Tear down test case.
     */
    static void TearDownTestCase ()
    {
        unlink (_sampleFile.c_str ());
        rmdir  (_basePath.c_str ());
    }

protected:
    /**
     * @brief Sets up the test fixture.
     */
    void SetUp ()
    {
        this->baseLocation (_basePath);
        ASSERT_EQ (this->baseLocation (), _basePath);
        this->keepAlive (seconds (_timeout), _max);
        ASSERT_EQ (this->keepAliveTimeout (), seconds (_timeout));
        ASSERT_EQ (this->keepAliveMax (), _max);
        this->addAlias ("/", "", "$root/" + _sampleFileName);
        this->addDocumentRoot ("/", "*");
        ASSERT_EQ (this->create ({Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    }

    /**
     * @brief Tears down the test fixture.
     */
    void TearDown ()
    {
        this->close ();
    }

    /// base path.
    static const std::string _basePath;

    /// sample.
    static const std::string _sample;

    /// sample file name.
    static const std::string _sampleFileName;

    /// sample path.
    static const std::string _sampleFile;

    /// server hostname.
    static const std::string _host;

    /// server port.
    static const uint16_t _port;

    /// server keep alive timeout.
    static const int _timeout;

    /// server  keep alive max requests.
    static const int _max;
};

const std::string HttpTest::_basePath       = "/tmp/www";
const std::string HttpTest::_sample         = "<html><body><h1>It works!</h1></body></html>";
const std::string HttpTest::_sampleFileName = "sample.html";
const std::string HttpTest::_sampleFile     = _basePath + "/" + _sampleFileName;
const std::string HttpTest::_host           = "localhost";
const uint16_t    HttpTest::_port           = 5000;
const int         HttpTest::_timeout        = 5;
const int         HttpTest::_max            = 20;

/**
 * @brief Test move.
 */
TEST_F (HttpTest, move)
{
    Http::Client client1 ("127.0.0.1", 5000), client2 ("127.0.0.2", 5001);
    ASSERT_EQ (client1.host (), "127.0.0.1");
    ASSERT_EQ (client1.port (), 5000);
    ASSERT_EQ (client2.host (), "127.0.0.2");
    ASSERT_EQ (client2.port (), 5001);

    client1 = std::move (client2);
    ASSERT_EQ (client1.host (), "127.0.0.2");
    ASSERT_EQ (client1.port (), 5001);

    Http::Client client3 (std::move (client1));
    ASSERT_EQ (client3.host (), "127.0.0.2");
    ASSERT_EQ (client3.port (), 5001);
}

/**
 * @brief Test scheme method
 */
TEST_F (HttpTest, scheme)
{
    Http::Client client1 ("localhost", 80);
    ASSERT_EQ (client1.scheme (), "http");

    Http::Client client2 ("localhost", 443);
    ASSERT_EQ (client2.scheme (), "http");
}

/**
 * @brief Test host method
 */
TEST_F (HttpTest, host)
{
    Http::Client client1 ("91.66.32.78", 80);
    ASSERT_EQ (client1.host (), "91.66.32.78");

    Http::Client client2 ("localhost", 80);
    ASSERT_EQ (client2.host (), "localhost");
}

/**
 * @brief Test port method
 */
TEST_F (HttpTest, port)
{
    Http::Client client1 ("91.66.32.78", 80);
    ASSERT_EQ (client1.port (), 80);

    Http::Client client2 ("91.66.32.78", 5000);
    ASSERT_EQ (client2.port (), 5000);
}

/**
 * @brief Test authority method
 */
TEST_F (HttpTest, authority)
{
    ASSERT_EQ (Http::Client ("localhost", 80).authority (), "localhost");
    ASSERT_EQ (Http::Client ("localhost", 443).authority (), "localhost:443");
    ASSERT_EQ (Http::Client ("localhost", 5000).authority (), "localhost:5000");

    ASSERT_EQ (Http::Client ("91.66.32.78", 80).authority (), "91.66.32.78");
    ASSERT_EQ (Http::Client ("91.66.32.78", 443).authority (), "91.66.32.78:443");
    ASSERT_EQ (Http::Client ("91.66.32.78", 5000).authority (), "91.66.32.78:5000");

    ASSERT_EQ (Http::Client ("2001:db8:1234:5678::1", 80).authority (), "[2001:db8:1234:5678::1]");
    ASSERT_EQ (Http::Client ("2001:db8:1234:5678::1", 443).authority (), "[2001:db8:1234:5678::1]:443");
    ASSERT_EQ (Http::Client ("2001:db8:1234:5678::1", 5000).authority (), "[2001:db8:1234:5678::1]:5000");
}

/**
 * @brief Test url method
 */
TEST_F (HttpTest, url)
{
    ASSERT_EQ (Http::Client ("localhost", 80).url (), "http://localhost/");
    ASSERT_EQ (Http::Client ("localhost", 443).url (), "http://localhost:443/");
    ASSERT_EQ (Http::Client ("localhost", 5000).url (), "http://localhost:5000/");

    ASSERT_EQ (Http::Client ("91.66.32.78", 80).url (), "http://91.66.32.78/");
    ASSERT_EQ (Http::Client ("91.66.32.78", 443).url (), "http://91.66.32.78:443/");
    ASSERT_EQ (Http::Client ("91.66.32.78", 5000).url (), "http://91.66.32.78:5000/");

    ASSERT_EQ (Http::Client ("2001:db8:1234:5678::1", 80).url (), "http://[2001:db8:1234:5678::1]/");
    ASSERT_EQ (Http::Client ("2001:db8:1234:5678::1", 443).url (), "http://[2001:db8:1234:5678::1]:443/");
    ASSERT_EQ (Http::Client ("2001:db8:1234:5678::1", 5000).url (), "http://[2001:db8:1234:5678::1]:5000/");
}

/**
 * @brief Test keepAlive method
 */
TEST_F (HttpTest, keepAlive)
{
    Http::Client client1 ("localhost", 80, true);
    ASSERT_TRUE (client1.keepAlive ());

    client1.keepAlive (false);
    ASSERT_FALSE (client1.keepAlive ());

    Http::Client client2 ("localhost", 80, false);
    ASSERT_FALSE (client2.keepAlive ());

    client2.keepAlive (true);
    ASSERT_TRUE (client2.keepAlive ());
}

/**
 * @brief Test keepAliveTimeout method
 */
TEST_F (HttpTest, keepAliveTimeout)
{
    Http::Client client (_host, _port);
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
TEST_F (HttpTest, keepAliveMax)
{
    Http::Client client (_host, _port);
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
 * @brief Test send method
 */
TEST_F (HttpTest, send)
{
    Http::Client client1 ("172.16.13.128", 443);
    client1.timeout (500);
    client1 << HttpRequest ();
    ASSERT_TRUE (client1.fail ());
    ASSERT_EQ (join::lastError, Errc::TimedOut);

    Http::Client client2 (_host, _port, true);
    client2 << HttpRequest ();
    ASSERT_TRUE (client2.good ()) << join::lastError.message ();
    client2.close ();
    ASSERT_TRUE (client2.good ()) << join::lastError.message ();
}

/**
 * @brief Test receive method
 */
TEST_F (HttpTest, receive)
{
    Http::Client client (_host, _port);

    HttpResponse response;
    client >> response;
    ASSERT_TRUE (client.fail ());
    ASSERT_EQ (join::lastError, Errc::ConnectionClosed);

    client.clear ();
    client << HttpRequest ();
    ASSERT_TRUE (client.good ()) << join::lastError.message ();

    client >> response;
    ASSERT_TRUE (client.good ()) << join::lastError.message ();
    ASSERT_EQ (response.status (), "200");
    ASSERT_EQ (response.reason (), "ok");

    client.close ();
    ASSERT_TRUE (client.good ()) << join::lastError.message ();
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}