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
#include <join/httpmessage.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::Errc;
using join::HttpErrc;
using join::HttpResponse;

/**
 * @brief Test copy.
 */
TEST (HttpResponse, copy)
{
    HttpResponse response1, response2;

    response2.response ("404", "Not Found");
    ASSERT_EQ (response2.status (), "404");

    response1 = response2;
    ASSERT_EQ (response1.status (), "404");

    HttpResponse response3 (response1);
    ASSERT_EQ (response3.status (), "404");
}

/**
 * @brief Test move.
 */
TEST (HttpResponse, move)
{
    HttpResponse response1, response2;

    response2.response ("404", "Not Found");
    ASSERT_EQ (response2.status (), "404");

    response1 = std::move (response2);
    ASSERT_EQ (response1.status (), "404");

    HttpResponse response3 (std::move (response1));
    ASSERT_EQ (response3.status (), "404");
}

/**
 * @brief Test version.
 */
TEST (HttpResponse, version)
{
    HttpResponse response;
    ASSERT_EQ (response.version (), "HTTP/1.1");

    response.version ("HTTP/1.0");
    ASSERT_EQ (response.version (), "HTTP/1.0");

    response.version ("HTTP/2.0");
    ASSERT_EQ (response.version (), "HTTP/2.0");
}

/**
 * @brief Test response.
 */
TEST (HttpResponse, response)
{
    HttpResponse response;

    response.response ("404", "Not Found");
    ASSERT_EQ (response.status (), "404");
    ASSERT_EQ (response.reason (), "Not Found");
}

/**
 * @brief Test hasHeader.
 */
TEST (HttpResponse, hasHeader)
{
    HttpResponse response;
    ASSERT_FALSE (response.hasHeader ("Connection"));

    response.header ("Connection", "keep-alive");
    ASSERT_TRUE (response.hasHeader ("Connection"));
}

/**
 * @brief Test header.
 */
TEST (HttpResponse, header)
{
    HttpResponse response;
    ASSERT_EQ (response.header ("Connection"), "");

    response.clear ();
    response.header ("Connection", "keep-alive");
    ASSERT_EQ (response.header ("Connection"), "keep-alive");

    response.clear ();
    response.header ({"Accept", "*/*"});
    ASSERT_EQ (response.header ("Accept"), "*/*");

    response.clear ();
    response.headers ({{"Connection", "keep-alive"}, {"Accept", "*/*"}});
    ASSERT_EQ (response.headers (), HttpResponse::HeaderMap ({{"Connection", "keep-alive"}, {"Accept", "*/*"}}));
}

/**
 * @brief Test dumpHeaders.
 */
TEST (HttpResponse, dumpHeaders)
{
    HttpResponse response;
    ASSERT_EQ (response.dumpHeaders (), "\r\n");

    response.header ("Accept", "*/*");
    response.header ("Connection", "keep-alive");
    ASSERT_EQ (response.dumpHeaders (), "Accept: */*\r\nConnection: keep-alive\r\n\r\n");
}

/**
 * @brief Test clear.
 */
TEST (HttpResponse, clear)
{
    HttpResponse response;

    response.response ("200", "OK");
    response.version ("HTTP/2.0");
    response.header ("Accept", "*/*");
    ASSERT_EQ (response.status (), "200");
    ASSERT_EQ (response.reason (), "OK");
    ASSERT_EQ (response.version (), "HTTP/2.0");
    ASSERT_EQ (response.header ("Accept"), "*/*");

    response.clear ();
    ASSERT_EQ (response.status (), "");
    ASSERT_EQ (response.reason (), "");
    ASSERT_EQ (response.version (), "HTTP/1.1");
    ASSERT_EQ (response.header ("Accept"), "");
}

/**
 * @brief Test send.
 */
TEST (HttpResponse, send)
{
    HttpResponse response;
    response.version ("HTTP/1.0");
    response.response ("200", "OK");
    response.header ("Connection", "keep-alive");

    std::stringstream ss;
    ASSERT_EQ (response.writeHeaders (ss), 0) << join::lastError.message ();
    ASSERT_EQ (ss.str (), "HTTP/1.0 200 OK\r\nConnection: keep-alive\r\n\r\n");
}

/**
 * @brief Test receive.
 */
TEST (HttpResponse, receive)
{
    std::stringstream ss;
    ss << "HTTP/1.0 301 Redirect\r\n";
    ss << "Connection: keep-alive\r\n";
    ss << "Content-Type: text/html; charset=\"UTF-8\"\r\n";
    ss << "\r\n";

    HttpResponse response;
    ASSERT_EQ (response.readHeaders (ss), 0) << join::lastError.message ();
    ASSERT_EQ (response.status (), "301");
    ASSERT_EQ (response.reason (), "Redirect");
    ASSERT_EQ (response.version (), "HTTP/1.0");
    ASSERT_EQ (response.header ("Connection"), "keep-alive");
    ASSERT_EQ (response.header ("Content-Type"), "text/html; charset=\"UTF-8\"");

    ss.clear ();
    ss.str ("");
    ss << "HTTP/1.0";

    ASSERT_EQ (response.readHeaders (ss), -1);

    ss.clear ();
    ss.str ("");
    ss << "HTTP/1.0\r\n";

    ASSERT_EQ (response.readHeaders (ss), -1);
    ASSERT_EQ (join::lastError, HttpErrc::BadRequest);

    ss.clear ();
    ss.str ("");
    ss << "HTTP/1.0 200\r\n";

    ASSERT_EQ (response.readHeaders (ss), -1);
    ASSERT_EQ (join::lastError, HttpErrc::BadRequest);

    ss.clear ();
    ss.str ("");
    ss << "HTTP/1.0 200 OK\r\n";
    ss << "Connection keep-alive\r\n";
    ss << "\r\n";

    ASSERT_EQ (response.readHeaders (ss), -1);
    ASSERT_EQ (join::lastError, HttpErrc::BadRequest);

    ss.clear ();
    ss.str (std::string (8192, 'X'));

    ASSERT_EQ (response.readHeaders (ss), -1);
    ASSERT_EQ (join::lastError, Errc::MessageTooLong);
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
