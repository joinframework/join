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
#include <join/httprequest.hpp>

// Libraries.
#include <gtest/gtest.h>

// C++.
#include <sstream>

using join::HttpMethod;
using join::HttpRequest;

/**
 * @brief Test method.
 */
TEST (HttpRequest, method)
{
    HttpRequest request;

    request.method (HttpMethod::Head);
    EXPECT_EQ (request.method (), HttpMethod::Head);

    request.method (HttpMethod::Get);
    EXPECT_EQ (request.method (), HttpMethod::Get);

    request.method (HttpMethod::Post);
    EXPECT_EQ (request.method (), HttpMethod::Post);

    request.method (HttpMethod::Put);
    EXPECT_EQ (request.method (), HttpMethod::Put);

    request.method (HttpMethod::Delete);
    EXPECT_EQ (request.method (), HttpMethod::Delete);
}

/**
 * @brief Test methodString.
 */
TEST (HttpRequest, methodString)
{
    HttpRequest request;

    request.method (HttpMethod::Head);
    EXPECT_EQ (request.methodString (), "HEAD");

    request.method (HttpMethod::Get);
    EXPECT_EQ (request.methodString (), "GET");

    request.method (HttpMethod::Post);
    EXPECT_EQ (request.methodString (), "POST");

    request.method (HttpMethod::Put);
    EXPECT_EQ (request.methodString (), "PUT");

    request.method (HttpMethod::Delete);
    EXPECT_EQ (request.methodString (), "DELETE");
}

/**
 * @brief Test path.
 */
TEST (HttpRequest, path)
{
    HttpRequest request;
    EXPECT_EQ (request.path (), "/");

    request.path ("/path");
    EXPECT_EQ (request.path (), "/path");

    request.path ("/another/path");
    EXPECT_EQ (request.path (), "/another/path");
}

/**
 * @brief Test version.
 */
TEST (HttpRequest, version)
{
    HttpRequest request;
    EXPECT_EQ (request.version (), "HTTP/1.1");

    request.version ("HTTP/1.0");
    EXPECT_EQ (request.version (), "HTTP/1.0");

    request.version ("HTTP/2.0");
    EXPECT_EQ (request.version (), "HTTP/2.0");
}

/**
 * @brief Test header.
 */
TEST (HttpRequest, header)
{
    HttpRequest request;

    EXPECT_EQ (request.header ("Connection"), "");

    request.header ("Connection", "keep-alive");
    EXPECT_EQ (request.header ("Connection"), "keep-alive");
}

/**
 * @brief Test hasHeader.
 */
TEST (HttpRequest, hasHeader)
{
    HttpRequest request;

    EXPECT_FALSE (request.hasHeader ("Connection"));

    request.header ("Connection", "keep-alive");
    EXPECT_TRUE (request.hasHeader ("Connection"));
}

/**
 * @brief Test parameter.
 */
TEST (HttpRequest, parameter)
{
    HttpRequest request;

    EXPECT_EQ (request.parameter ("val1"), "");

    request.parameter ("val1", "1");
    EXPECT_EQ (request.parameter ("val1"), "1");
}

/**
 * @brief Test hasParameter.
 */
TEST (HttpRequest, hasParameter)
{
    HttpRequest request;

    EXPECT_FALSE (request.hasParameter ("val1"));

    request.parameter ("val1", "1");
    EXPECT_TRUE (request.hasParameter ("val1"));
}

/**
 * @brief Test query.
 */
TEST (HttpRequest, query)
{
    HttpRequest request;
    EXPECT_EQ (request.query (), "");

    request.parameter ("val1", "1");
    request.parameter ("val2", "2");
    EXPECT_EQ (request.query (), "?val1=1&val2=2");
}

/**
 * @brief Test uri.
 */
TEST (HttpRequest, uri)
{
    HttpRequest request;

    EXPECT_EQ (request.uri (), "/");

    request.path ("/path");
    request.parameter ("val1", "1");
    request.parameter ("val2", "2");
    EXPECT_EQ (request.uri (), "/path?val1=1&val2=2");
}

/**
 * @brief Test send.
 */
TEST (HttpRequest, send)
{
    HttpRequest request;
    request.method (HttpMethod::Get);
    request.path ("/path");
    request.parameter ("val1", "1");
    request.parameter ("val2", "2");
    request.version ("HTTP/1.0");
    request.header ("Connection", "keep-alive");

    std::stringstream ss;
    request.send (ss);
    EXPECT_EQ (ss.str (), "GET /path?val1=1&val2=2 HTTP/1.0\r\nConnection: keep-alive\r\n\r\n");
}

/**
 * @brief Test receive.
 */
TEST (HttpRequest, receive)
{
    std::stringstream ss;
    ss << "GET /path?val1=1&val2=2 HTTP/1.0\r\n";
    ss << "Connection: keep-alive\r\n";
    ss << "\r\n";

    HttpRequest request;
    request.receive (ss);
    EXPECT_EQ (request.method (), HttpMethod::Get);
    EXPECT_EQ (request.path (), "/path");
    EXPECT_EQ (request.parameter ("val1"), "1");
    EXPECT_EQ (request.parameter ("val2"), "2");
    EXPECT_EQ (request.version (), "HTTP/1.0");
    EXPECT_EQ (request.header ("Connection"), "keep-alive");
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
