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

using join::HttpRequest;
using join::HttpResponse;
using join::HttpClient;

/**
 * @brief Test scheme method
 */
TEST (HttpClient, scheme)
{
    HttpClient client1 ("joinframework.net", 80, false);
    ASSERT_EQ (client1.scheme (), "http");

    HttpClient client2 ("joinframework.net", 80, true);
    ASSERT_EQ (client2.scheme (), "https");

    HttpClient client3 ("joinframework.net", 443, true);
    ASSERT_EQ (client3.scheme (), "https");

    HttpClient client4 ("joinframework.net", 443, false);
    ASSERT_EQ (client4.scheme (), "http");
}

/**
 * @brief Test host method
 */
TEST (HttpClient, host)
{
    HttpClient client1 ("joinframework.net", 80, false);
    ASSERT_EQ (client1.host (), "joinframework.net");

    HttpClient client2 ("91.66.32.78", 443, true);
    ASSERT_EQ (client2.host (), "91.66.32.78");
}

/**
 * @brief Test port method
 */
TEST (HttpClient, port)
{
    HttpClient client1 ("joinframework.net", 80, false);
    ASSERT_EQ (client1.port (), 80);

    HttpClient client2 ("joinframework.net", 443, true);
    ASSERT_EQ (client2.port (), 443);
}

/**
 * @brief Test keepAlive method
 */
TEST (HttpClient, keepAlive)
{
    HttpClient client1 ("joinframework.net", 80, false, true);
    ASSERT_TRUE (client1.keepAlive ());

    client1.keepAlive (false);
    ASSERT_FALSE (client1.keepAlive ());

    HttpClient client2 ("joinframework.net", 443, true, false);
    ASSERT_FALSE (client2.keepAlive ());

    client2.keepAlive (true);
    ASSERT_TRUE (client2.keepAlive ());
}

/**
 * @brief Test send method
 */
TEST (HttpClient, send)
{
    HttpClient client ("joinframework.net", 443, true);

    client << HttpRequest ();
    ASSERT_TRUE (client.good ()) << join::lastError.message ();

    client.close ();
    ASSERT_TRUE (client.good ()) << join::lastError.message ();
}

/**
 * @brief Test receive method
 */
TEST (HttpClient, receive)
{
    HttpClient client ("joinframework.net", 443, true);
    HttpResponse response;
    
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
    join::crypto::initializeOpenSSL ();
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
