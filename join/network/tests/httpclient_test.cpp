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
using join::Http;
using join::Https;

const std::string host = "joinframework.net";

/**
 * @brief Test move.
 */
TEST (HttpClient, move)
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
TEST (HttpClient, scheme)
{
    Http::Client client1 (host, 80);
    ASSERT_EQ (client1.scheme (), "http");

    Https::Client client2 (host, 80);
    ASSERT_EQ (client2.scheme (), "https");

    Https::Client client3 (host, 443);
    ASSERT_EQ (client3.scheme (), "https");

    Http::Client client4 (host, 443);
    ASSERT_EQ (client4.scheme (), "http");
}

/**
 * @brief Test host method
 */
TEST (HttpClient, host)
{
    Http::Client client1 (host, 80);
    ASSERT_EQ (client1.host (), host);

    Https::Client client2 ("91.66.32.78", 443);
    ASSERT_EQ (client2.host (), "91.66.32.78");
}

/**
 * @brief Test port method
 */
TEST (HttpClient, port)
{
    Http::Client client1 (host, 80);
    ASSERT_EQ (client1.port (), 80);

    Https::Client client2 (host, 443);
    ASSERT_EQ (client2.port (), 443);
}

/**
 * @brief Test authority method
 */
TEST (HttpClient, authority)
{
    ASSERT_EQ (Http::Client (host, 80).authority (), host);
    ASSERT_EQ (Http::Client (host, 443).authority (), host + ":443");
    ASSERT_EQ (Http::Client (host, 5000).authority (), host + ":5000");
    ASSERT_EQ (Https::Client (host, 80).authority (), host + ":80");
    ASSERT_EQ (Https::Client (host, 443).authority (), host);
    ASSERT_EQ (Https::Client (host, 5001).authority (), host + ":5001");

    ASSERT_EQ (Http::Client ("91.66.32.78", 80).authority (), "91.66.32.78");
    ASSERT_EQ (Http::Client ("91.66.32.78", 443).authority (), "91.66.32.78:443");
    ASSERT_EQ (Http::Client ("91.66.32.78", 5000).authority (), "91.66.32.78:5000");
    ASSERT_EQ (Https::Client ("91.66.32.78", 80).authority (), "91.66.32.78:80");
    ASSERT_EQ (Https::Client ("91.66.32.78", 443).authority (), "91.66.32.78");
    ASSERT_EQ (Https::Client ("91.66.32.78", 5001).authority (), "91.66.32.78:5001");

    ASSERT_EQ (Http::Client ("2001:db8:1234:5678::1", 80).authority (), "[2001:db8:1234:5678::1]");
    ASSERT_EQ (Http::Client ("2001:db8:1234:5678::1", 443).authority (), "[2001:db8:1234:5678::1]:443");
    ASSERT_EQ (Http::Client ("2001:db8:1234:5678::1", 5000).authority (), "[2001:db8:1234:5678::1]:5000");
    ASSERT_EQ (Https::Client ("2001:db8:1234:5678::1", 80).authority (), "[2001:db8:1234:5678::1]:80");
    ASSERT_EQ (Https::Client ("2001:db8:1234:5678::1", 443).authority (), "[2001:db8:1234:5678::1]");
    ASSERT_EQ (Https::Client ("2001:db8:1234:5678::1", 5001).authority (), "[2001:db8:1234:5678::1]:5001");
}

/**
 * @brief Test url method
 */
TEST (HttpClient, url)
{
    ASSERT_EQ (Http::Client (host, 80).url (), "http://" + host + "/");
    ASSERT_EQ (Http::Client (host, 443).url (), "http://" + host + ":443/");
    ASSERT_EQ (Http::Client (host, 5000).url (), "http://" + host + ":5000/");
    ASSERT_EQ (Https::Client (host, 80).url (), "https://" + host + ":80/");
    ASSERT_EQ (Https::Client (host, 443).url (), "https://" + host + "/");
    ASSERT_EQ (Https::Client (host, 5001).url (), "https://" + host + ":5001/");

    ASSERT_EQ (Http::Client ("91.66.32.78", 80).url (), "http://91.66.32.78/");
    ASSERT_EQ (Http::Client ("91.66.32.78", 443).url (), "http://91.66.32.78:443/");
    ASSERT_EQ (Http::Client ("91.66.32.78", 5000).url (), "http://91.66.32.78:5000/");
    ASSERT_EQ (Https::Client ("91.66.32.78", 80).url (), "https://91.66.32.78:80/");
    ASSERT_EQ (Https::Client ("91.66.32.78", 443).url (), "https://91.66.32.78/");
    ASSERT_EQ (Https::Client ("91.66.32.78", 5001).url (), "https://91.66.32.78:5001/");

    ASSERT_EQ (Http::Client ("2001:db8:1234:5678::1", 80).url (), "http://[2001:db8:1234:5678::1]/");
    ASSERT_EQ (Http::Client ("2001:db8:1234:5678::1", 443).url (), "http://[2001:db8:1234:5678::1]:443/");
    ASSERT_EQ (Http::Client ("2001:db8:1234:5678::1", 5000).url (), "http://[2001:db8:1234:5678::1]:5000/");
    ASSERT_EQ (Https::Client ("2001:db8:1234:5678::1", 80).url (), "https://[2001:db8:1234:5678::1]:80/");
    ASSERT_EQ (Https::Client ("2001:db8:1234:5678::1", 443).url (), "https://[2001:db8:1234:5678::1]/");
    ASSERT_EQ (Https::Client ("2001:db8:1234:5678::1", 5001).url (), "https://[2001:db8:1234:5678::1]:5001/");
}

/**
 * @brief Test keepAlive method
 */
TEST (HttpClient, keepAlive)
{
    Http::Client client1 (host, 80, true);
    ASSERT_TRUE (client1.keepAlive ());

    client1.keepAlive (false);
    ASSERT_FALSE (client1.keepAlive ());

    Https::Client client2 (host, 443, false);
    ASSERT_FALSE (client2.keepAlive ());

    client2.keepAlive (true);
    ASSERT_TRUE (client2.keepAlive ());
}

/**
 * @brief Test keepAliveTimeout method
 */
TEST (HttpClient, keepAliveTimeout)
{
    Https::Client client (host);
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
TEST (HttpClient, keepAliveMax)
{
    Https::Client client (host);
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
    ASSERT_NE (client.keepAliveMax (), 0);

    request.header ("Connection", "close");
    client << request;
    ASSERT_TRUE (client.good ()) << join::lastError.message ();
    ASSERT_NE (client.keepAliveMax (), 0);

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
TEST (HttpClient, send)
{
    Http::Client client1 ("172.16.13.128", 443);
    client1.timeout (500);
    client1 << HttpRequest ();
    ASSERT_TRUE (client1.fail ());
    ASSERT_EQ (join::lastError, Errc::TimedOut);

    Http::Client client2 (host, 80, true);
    client2 << HttpRequest ();
    ASSERT_TRUE (client2.good ()) << join::lastError.message ();
    client2.close ();
    ASSERT_TRUE (client2.good ()) << join::lastError.message ();

    Https::Client client3 ("172.16.13.128", 80);
    client3.timeout (500);
    client3 << HttpRequest ();
    ASSERT_TRUE (client3.fail ());
    ASSERT_EQ (join::lastError, Errc::TimedOut);

    Https::Client client4 (host, 443, true);
    client4 << HttpRequest ();
    ASSERT_TRUE (client4.good ()) << join::lastError.message ();
    client4.close ();
    ASSERT_TRUE (client4.good ()) << join::lastError.message ();

    Https::Client client5 (host, 80);
    client5 << HttpRequest ();
    ASSERT_TRUE (client5.fail ());
    ASSERT_EQ (join::lastError, TlsErrc::TlsProtocolError);
}

/**
 * @brief Test receive method
 */
TEST (HttpClient, receive)
{
    Http::Client client1 (host, 80);

    HttpResponse response;
    client1 >> response;
    ASSERT_TRUE (client1.fail ());
    ASSERT_EQ (join::lastError, Errc::ConnectionClosed);

    client1.clear ();
    client1 << HttpRequest ();
    ASSERT_TRUE (client1.good ()) << join::lastError.message ();

    client1 >> response;
    ASSERT_TRUE (client1.good ()) << join::lastError.message ();
    ASSERT_EQ (response.status (), "302");
    ASSERT_EQ (response.reason (), "Found");

    client1.close ();
    ASSERT_TRUE (client1.good ()) << join::lastError.message ();

    Https::Client client2 (host, 443);

    client2 >> response;
    ASSERT_TRUE (client2.fail ());
    ASSERT_EQ (join::lastError, Errc::ConnectionClosed);

    client2.clear ();
    client2 << HttpRequest ();
    ASSERT_TRUE (client2.good ()) << join::lastError.message ();

    client2 >> response;
    ASSERT_TRUE (client2.good ()) << join::lastError.message ();
    ASSERT_EQ (response.status (), "200");
    ASSERT_EQ (response.reason (), "OK");

    client2.close ();
    ASSERT_TRUE (client2.good ()) << join::lastError.message ();
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
