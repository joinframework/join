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
using join::IpAddress;
using join::Resolver;
using join::HttpMethod;
using join::HttpRequest;
using join::HttpResponse;
using join::HttpErrc;
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
            outFile << _sample;
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
        this->baseLocation (_basePath + "/");
        ASSERT_EQ (this->baseLocation (), _basePath);
        this->keepAlive (seconds (_timeout), _max);
        ASSERT_EQ (this->keepAliveTimeout (), seconds (_timeout));
        ASSERT_EQ (this->keepAliveMax (), _max);
        this->addAlias ("/", "", _sampleFile);
        this->addAlias ("/authorized/", "file", _sampleFile, accessHandler);
        this->addDocumentRoot ("/", "*");
        this->addDocumentRoot ("/no/", "file");
        this->addRedirect ("/redirect/", "file", "https://$host:$port/");
        this->addExecute (HttpMethod::Get, "/exec/", "null", nullptr);
        this->addExecute (HttpMethod::Get, "/exec/", "get", getHandler);
        this->addExecute (HttpMethod::Post, "/exec/", "post", postHandler);
        ASSERT_EQ (this->create ({IpAddress::ipv6Wildcard, _port}), 0) << join::lastError.message ();
        ASSERT_EQ (this->create ({IpAddress::ipv6Wildcard, _port}), -1);
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
     * @param type authorization type.
     * @param token session token.
     * @param errc error code.
     * @return true on success, false otherwise.
     */
    static bool accessHandler (const std::string& type, const std::string& token, std::error_code& errc)
    {
        if (type != "Bearer")
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
    static void getHandler (Http::Worker* worker)
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
    static void postHandler (Http::Worker* worker)
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

    /// server keep alive max requests.
    static const int _max;
};

const std::string HttpTest::_basePath       = "/tmp/www";
const std::string HttpTest::_sample         = "<html><body><h1>It works!</h1></body></html>";
const std::string HttpTest::_sampleFileName = "sample.html";
const std::string HttpTest::_sampleFile     = _basePath + "/" + _sampleFileName;
const std::string HttpTest::_token          = "adlSaJkmBLpgnRRCjkCgQ4uaCagKHsIN";
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
    Http::Client client1 ("localhost", 80);
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
 * @brief Test bad request
 */
TEST_F (HttpTest, badRequest)
{
    Http::Client client (_host, _port);

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
TEST_F (HttpTest, invalidMethod)
{
    Http::Client client (_host, _port);

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
TEST_F (HttpTest, headerTooLarge)
{
    Http::Client client (_host, _port);

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
TEST_F (HttpTest, notFound)
{
    Http::Client client (_host, _port);

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
TEST_F (HttpTest, notModified)
{
    struct stat sbuf;
    std::stringstream modifTime;
    ASSERT_EQ (stat (_sampleFile.c_str (), &sbuf), 0);
    modifTime << std::put_time (std::gmtime (&sbuf.st_ctime), "%a, %d %b %Y %H:%M:%S GMT");

    Http::Client client (_host, _port);

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
TEST_F (HttpTest, redirect)
{
    Http::Client client (_host, _port);

    HttpRequest request;
    request.path ("/redirect/file");
    ASSERT_EQ (client.send (request), 0) << join::lastError.message ();

    HttpResponse response;
    ASSERT_EQ (client.receive (response), 0) << join::lastError.message ();
    ASSERT_EQ (response.status (), "307");
    ASSERT_EQ (response.reason (), "Temporary Redirect");

    ASSERT_GT (response.contentLength (), 0);
    std::string payload;
    payload.resize (response.contentLength ());
    client.read (&payload[0], payload.size ());

    request.clear ();
    request.path ("/redirect/file");
    request.version ("HTTP/1.0");
    ASSERT_EQ (client.send (request), 0) << join::lastError.message ();

    response.clear ();
    ASSERT_EQ (client.receive (response), 0) << join::lastError.message ();
    ASSERT_EQ (response.status (), "302");
    ASSERT_EQ (response.reason (), "Found");

    ASSERT_GT (response.contentLength (), 0);
    payload.resize (response.contentLength ());
    client.read (&payload[0], payload.size ());
}

/**
 * @brief Test server error
 */
TEST_F (HttpTest, serverError)
{
    Http::Client client (_host, _port);

    HttpRequest request;
    request.path ("/exec/null");
    ASSERT_EQ (client.send (request), 0) << join::lastError.message ();

    HttpResponse response;
    ASSERT_EQ (client.receive (response), 0) << join::lastError.message ();
    ASSERT_EQ (response.status (), "500");
    ASSERT_EQ (response.reason (), "Internal Server Error");

    client.close ();
    ASSERT_TRUE (client.good ()) << join::lastError.message ();
}

/**
 * @brief Test unauthorized
 */
TEST_F (HttpTest, unauthorized)
{
    Http::Client client (_host, _port);

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
TEST_F (HttpTest, forbidden)
{
    Http::Client client (_host, _port);

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
TEST_F (HttpTest, head)
{
    Http::Client client (_host, _port);

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
TEST_F (HttpTest, get)
{
    Http::Client client (_host, _port);

    HttpRequest request;
    request.method (HttpMethod::Get);
    ASSERT_EQ (client.send (request), 0) << join::lastError.message ();

    HttpResponse response;
    ASSERT_EQ (client.receive (response), 0) << join::lastError.message ();
    ASSERT_EQ (response.status (), "200");
    ASSERT_EQ (response.reason (), "OK");

    ASSERT_EQ (response.contentLength (), _sample.size ());
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
TEST_F (HttpTest, post)
{
    Http::Client client (_host, _port);

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
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
