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
#include <join/httpclient.hpp>

// Libraries.
#include <gtest/gtest.h>

using namespace std::chrono;

using join::Errc;
using join::TlsErrc;
using join::HttpMethod;
using join::HttpRequest;
using join::HttpResponse;
using join::HttpClient;
using join::Tls;

const std::string host = "joinframework.net";

/**
 * @brief Test move.
 */
TEST (HttpClient, move)
{
    HttpClient client1 ("127.0.0.1", 5000), client2 ("127.0.0.2", 5001);
    ASSERT_EQ (client1.host (), "127.0.0.1");
    ASSERT_EQ (client1.port (), 5000);
    ASSERT_EQ (client2.host (), "127.0.0.2");
    ASSERT_EQ (client2.port (), 5001);

    client1 = std::move (client2);
    ASSERT_EQ (client1.host (), "127.0.0.2");
    ASSERT_EQ (client1.port (), 5001);

    HttpClient client3 (std::move (client1));
    ASSERT_EQ (client3.host (), "127.0.0.2");
    ASSERT_EQ (client3.port (), 5001);
}

/**
 * @brief Test scheme method
 */
TEST (HttpClient, scheme)
{
    HttpClient client1 (host, 80, false);
    ASSERT_EQ (client1.scheme (), "http");

    HttpClient client2 (host, 80, true);
    ASSERT_EQ (client2.scheme (), "https");

    HttpClient client3 (host, 443, true);
    ASSERT_EQ (client3.scheme (), "https");

    HttpClient client4 (host, 443, false);
    ASSERT_EQ (client4.scheme (), "http");
}

/**
 * @brief Test host method
 */
TEST (HttpClient, host)
{
    HttpClient client1 (host, 80, false);
    ASSERT_EQ (client1.host (), host);

    HttpClient client2 ("91.66.32.78", 443, true);
    ASSERT_EQ (client2.host (), "91.66.32.78");
}

/**
 * @brief Test port method
 */
TEST (HttpClient, port)
{
    HttpClient client1 (host, 80, false);
    ASSERT_EQ (client1.port (), 80);

    HttpClient client2 (host, 443, true);
    ASSERT_EQ (client2.port (), 443);
}

/**
 * @brief Test authority method
 */
TEST (HttpClient, authority)
{
    ASSERT_EQ (HttpClient (host, 80, false).authority (), host);
    ASSERT_EQ (HttpClient (host, 443, false).authority (), host + ":443");
    ASSERT_EQ (HttpClient (host, 5000, false).authority (), host + ":5000");
    ASSERT_EQ (HttpClient (host, 80, true).authority (), host + ":80");
    ASSERT_EQ (HttpClient (host, 443, true).authority (), host);
    ASSERT_EQ (HttpClient (host, 5001, true).authority (), host + ":5001");

    ASSERT_EQ (HttpClient ("91.66.32.78", 80, false).authority (), "91.66.32.78");
    ASSERT_EQ (HttpClient ("91.66.32.78", 443, false).authority (), "91.66.32.78:443");
    ASSERT_EQ (HttpClient ("91.66.32.78", 5000, false).authority (), "91.66.32.78:5000");
    ASSERT_EQ (HttpClient ("91.66.32.78", 80, true).authority (), "91.66.32.78:80");
    ASSERT_EQ (HttpClient ("91.66.32.78", 443, true).authority (), "91.66.32.78");
    ASSERT_EQ (HttpClient ("91.66.32.78", 5001, true).authority (), "91.66.32.78:5001");

    ASSERT_EQ (HttpClient ("2001:db8:1234:5678::1", 80, false).authority (), "[2001:db8:1234:5678::1]");
    ASSERT_EQ (HttpClient ("2001:db8:1234:5678::1", 443, false).authority (), "[2001:db8:1234:5678::1]:443");
    ASSERT_EQ (HttpClient ("2001:db8:1234:5678::1", 5000, false).authority (), "[2001:db8:1234:5678::1]:5000");
    ASSERT_EQ (HttpClient ("2001:db8:1234:5678::1", 80, true).authority (), "[2001:db8:1234:5678::1]:80");
    ASSERT_EQ (HttpClient ("2001:db8:1234:5678::1", 443, true).authority (), "[2001:db8:1234:5678::1]");
    ASSERT_EQ (HttpClient ("2001:db8:1234:5678::1", 5001, true).authority (), "[2001:db8:1234:5678::1]:5001");
}

/**
 * @brief Test url method
 */
TEST (HttpClient, url)
{
    ASSERT_EQ (HttpClient (host, 80, false).url (), "http://" + host + "/");
    ASSERT_EQ (HttpClient (host, 443, false).url (), "http://" + host + ":443/");
    ASSERT_EQ (HttpClient (host, 5000, false).url (), "http://" + host + ":5000/");
    ASSERT_EQ (HttpClient (host, 80, true).url (), "https://" + host + ":80/");
    ASSERT_EQ (HttpClient (host, 443, true).url (), "https://" + host + "/");
    ASSERT_EQ (HttpClient (host, 5001, true).url (), "https://" + host + ":5001/");

    ASSERT_EQ (HttpClient ("91.66.32.78", 80, false).url (), "http://91.66.32.78/");
    ASSERT_EQ (HttpClient ("91.66.32.78", 443, false).url (), "http://91.66.32.78:443/");
    ASSERT_EQ (HttpClient ("91.66.32.78", 5000, false).url (), "http://91.66.32.78:5000/");
    ASSERT_EQ (HttpClient ("91.66.32.78", 80, true).url (), "https://91.66.32.78:80/");
    ASSERT_EQ (HttpClient ("91.66.32.78", 443, true).url (), "https://91.66.32.78/");
    ASSERT_EQ (HttpClient ("91.66.32.78", 5001, true).url (), "https://91.66.32.78:5001/");

    ASSERT_EQ (HttpClient ("2001:db8:1234:5678::1", 80, false).url (), "http://[2001:db8:1234:5678::1]/");
    ASSERT_EQ (HttpClient ("2001:db8:1234:5678::1", 443, false).url (), "http://[2001:db8:1234:5678::1]:443/");
    ASSERT_EQ (HttpClient ("2001:db8:1234:5678::1", 5000, false).url (), "http://[2001:db8:1234:5678::1]:5000/");
    ASSERT_EQ (HttpClient ("2001:db8:1234:5678::1", 80, true).url (), "https://[2001:db8:1234:5678::1]:80/");
    ASSERT_EQ (HttpClient ("2001:db8:1234:5678::1", 443, true).url (), "https://[2001:db8:1234:5678::1]/");
    ASSERT_EQ (HttpClient ("2001:db8:1234:5678::1", 5001, true).url (), "https://[2001:db8:1234:5678::1]:5001/");
}

/**
 * @brief Test keepAlive method
 */
TEST (HttpClient, keepAlive)
{
    HttpClient client1 (host, 80, false, true);
    ASSERT_TRUE (client1.keepAlive ());

    client1.keepAlive (false);
    ASSERT_FALSE (client1.keepAlive ());

    HttpClient client2 (host, 443, true, false);
    ASSERT_FALSE (client2.keepAlive ());

    client2.keepAlive (true);
    ASSERT_TRUE (client2.keepAlive ());
}

/**
 * @brief Test keepAliveTimeout method
 */
TEST (HttpClient, DISABLED_keepAliveTimeout)
{
    HttpClient client (host);
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
    ASSERT_EQ (client.keepAliveTimeout (), seconds (20));

    request.header ("Connection", "close");
    client << request;
    ASSERT_TRUE (client.good ()) << join::lastError.message ();
    ASSERT_EQ (client.keepAliveTimeout (), seconds (20));

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
TEST (HttpClient, DISABLED_keepAliveMax)
{
    HttpClient client (host);
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
    ASSERT_EQ (client.keepAliveMax (), 100);

    request.header ("Connection", "close");
    client << request;
    ASSERT_TRUE (client.good ()) << join::lastError.message ();
    ASSERT_EQ (client.keepAliveMax (), 100);

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
TEST (HttpClient, DISABLED_send)
{
    HttpClient client ("172.16.13.128", 80);
    client.timeout (500);
    client << HttpRequest ();
    ASSERT_TRUE (client.fail ());
    ASSERT_EQ (join::lastError, Errc::TimedOut);
    client.clear ();

    client = HttpClient (host, 80, true);
    client << HttpRequest ();
    ASSERT_TRUE (client.fail ());
    ASSERT_EQ (join::lastError, TlsErrc::TlsProtocolError);
    client.clear ();

    client = HttpClient (host, 443, true);
    client << HttpRequest ();
    ASSERT_TRUE (client.good ()) << join::lastError.message ();

    client.close ();
    ASSERT_TRUE (client.good ()) << join::lastError.message ();
}

/**
 * @brief Test receive method
 */
TEST (HttpClient, DISABLED_receive)
{
    HttpClient client (host, 443, true);

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
