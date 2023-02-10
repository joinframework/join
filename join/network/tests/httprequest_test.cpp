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
using join::HttpMethod;
using join::HttpRequest;

/**
 * @brief Test copy.
 */
TEST (HttpRequest, copy)
{
    HttpRequest request1, request2 (HttpMethod::Post);
    ASSERT_EQ (request1.method (), HttpMethod::Get);
    ASSERT_EQ (request2.method (), HttpMethod::Post);

    request1 = request2;
    ASSERT_EQ (request1.method (), HttpMethod::Post);

    HttpRequest request3 (request1);
    ASSERT_EQ (request3.method (), HttpMethod::Post);
}

/**
 * @brief Test move.
 */
TEST (HttpRequest, move)
{
    HttpRequest request1, request2 (HttpMethod::Post);
    ASSERT_EQ (request1.method (), HttpMethod::Get);
    ASSERT_EQ (request2.method (), HttpMethod::Post);

    request1 = std::move (request2);
    ASSERT_EQ (request1.method (), HttpMethod::Post);

    HttpRequest request3 (std::move (request1));
    ASSERT_EQ (request3.method (), HttpMethod::Post);
}

/**
 * @brief Test method.
 */
TEST (HttpRequest, method)
{
    HttpRequest request;
    ASSERT_EQ (request.method (), HttpMethod::Get);

    request.method (HttpMethod::Head);
    ASSERT_EQ (request.method (), HttpMethod::Head);

    request.method (HttpMethod::Get);
    ASSERT_EQ (request.method (), HttpMethod::Get);

    request.method (HttpMethod::Put);
    ASSERT_EQ (request.method (), HttpMethod::Put);

    request.method (HttpMethod::Post);
    ASSERT_EQ (request.method (), HttpMethod::Post);

    request.method (HttpMethod::Delete);
    ASSERT_EQ (request.method (), HttpMethod::Delete);
}

/**
 * @brief Test methodString.
 */
TEST (HttpRequest, methodString)
{
    HttpRequest request;
    ASSERT_EQ (request.methodString (), "GET");

    request.method (HttpMethod::Head);
    ASSERT_EQ (request.methodString (), "HEAD");

    request.method (HttpMethod::Get);
    ASSERT_EQ (request.methodString (), "GET");

    request.method (HttpMethod::Put);
    ASSERT_EQ (request.methodString (), "PUT");

    request.method (HttpMethod::Post);
    ASSERT_EQ (request.methodString (), "POST");

    request.method (HttpMethod::Delete);
    ASSERT_EQ (request.methodString (), "DELETE");
}

/**
 * @brief Test path.
 */
TEST (HttpRequest, path)
{
    HttpRequest request;
    ASSERT_EQ (request.path (), "/");

    request.path ("/path");
    ASSERT_EQ (request.path (), "/path");

    request.path ("/another/path");
    ASSERT_EQ (request.path (), "/another/path");
}

/**
 * @brief Test version.
 */
TEST (HttpRequest, version)
{
    HttpRequest request;
    ASSERT_EQ (request.version (), "HTTP/1.1");

    request.version ("HTTP/1.0");
    ASSERT_EQ (request.version (), "HTTP/1.0");

    request.version ("HTTP/2.0");
    ASSERT_EQ (request.version (), "HTTP/2.0");
}

/**
 * @brief Test hasHeader.
 */
TEST (HttpRequest, hasHeader)
{
    HttpRequest request;
    ASSERT_FALSE (request.hasHeader ("Connection"));

    request.header ("Connection", "keep-alive");
    ASSERT_TRUE (request.hasHeader ("Connection"));
}

/**
 * @brief Test header.
 */
TEST (HttpRequest, header)
{
    HttpRequest request;
    ASSERT_EQ (request.header ("Connection"), "");

    request.clear ();
    request.header ("Connection", "keep-alive");
    ASSERT_EQ (request.header ("Connection"), "keep-alive");

    request.clear ();
    request.header ({"Accept", "*/*"});
    ASSERT_EQ (request.header ("Accept"), "*/*");

    request.clear ();
    request.headers ({{"Connection", "keep-alive"}, {"Accept", "*/*"}});
    ASSERT_EQ (request.headers (), HttpRequest::HeaderMap ({{"Connection", "keep-alive"}, {"Accept", "*/*"}}));
}

/**
 * @brief Test dumpHeaders.
 */
TEST (HttpRequest, dumpHeaders)
{
    HttpRequest request;
    ASSERT_EQ (request.dumpHeaders (), "\r\n");

    request.header ("Accept", "*/*");
    request.header ("Connection", "keep-alive");
    ASSERT_EQ (request.dumpHeaders (), "Accept: */*\r\nConnection: keep-alive\r\n\r\n");
}

/**
 * @brief Test hasParameter.
 */
TEST (HttpRequest, hasParameter)
{
    HttpRequest request;
    ASSERT_FALSE (request.hasParameter ("val1"));

    request.parameter ("val1", "1");
    ASSERT_TRUE (request.hasParameter ("val1"));
}

/**
 * @brief Test parameter.
 */
TEST (HttpRequest, parameter)
{
    HttpRequest request;
    ASSERT_EQ (request.parameter ("val1"), "");

    request.clear ();
    request.parameter ("val1", "1");
    ASSERT_EQ (request.parameter ("val1"), "1");

    request.clear ();
    request.parameter ({"val2", "2"});
    ASSERT_EQ (request.parameter ("val2"), "2");

    request.clear ();
    request.parameters ({{"val3", "3"}, {"val4", "4"}});
    ASSERT_EQ (request.parameters (), HttpRequest::ParameterMap ({{"val3", "3"}, {"val4", "4"}}));
}

/**
 * @brief Test dumpParameters.
 */
TEST (HttpRequest, dumpParameters)
{
    HttpRequest request;
    ASSERT_EQ (request.dumpParameters (), "");

    request.parameter ("val1", "1");
    request.parameter ("val2", "2");
    ASSERT_EQ (request.dumpParameters (), "val1=1&val2=2");
}

/**
 * @brief Test query.
 */
TEST (HttpRequest, query)
{
    HttpRequest request;
    ASSERT_EQ (request.query (), "");

    request.parameter ("val1", "1");
    request.parameter ("val2", "2");
    ASSERT_EQ (request.query (), "?val1=1&val2=2");
}

/**
 * @brief Test urn.
 */
TEST (HttpRequest, urn)
{
    HttpRequest request;
    ASSERT_EQ (request.urn (), "/");

    request.path ("/path");
    request.parameter ("val1", "1");
    request.parameter ("val2", "2");
    ASSERT_EQ (request.urn (), "/path?val1=1&val2=2");
}

/**
 * @brief Test clear.
 */
TEST (HttpRequest, clear)
{
    HttpRequest request;

    request.method (HttpMethod::Delete);
    request.path ("/path");
    request.version ("HTTP/2.0");
    request.parameter ("val1", "1");
    request.header ("Accept", "*/*");
    ASSERT_EQ (request.method (), HttpMethod::Delete);
    ASSERT_EQ (request.path (), "/path");
    ASSERT_EQ (request.version (), "HTTP/2.0");
    ASSERT_EQ (request.parameter ("val1"), "1");
    ASSERT_EQ (request.header ("Accept"), "*/*");

    request.clear ();
    ASSERT_EQ (request.method (), HttpMethod::Get);
    ASSERT_EQ (request.path (), "/");
    ASSERT_EQ (request.version (), "HTTP/1.1");
    ASSERT_EQ (request.parameter ("val1"), "");
    ASSERT_EQ (request.header ("Accept"), "");
}

/**
 * @brief Test writeHeaders.
 */
TEST (HttpRequest, writeHeaders)
{
    HttpRequest request;
    request.method (HttpMethod::Head);
    request.path ("/path");
    request.parameter ("val1", "1");
    request.parameter ("val2", "2");
    request.version ("HTTP/1.0");
    request.header ("Connection", "keep-alive");

    std::stringstream ss;
    request.writeHeaders (ss);
    ASSERT_EQ (ss.str (), "HEAD /path?val1=1&val2=2 HTTP/1.0\r\nConnection: keep-alive\r\n\r\n");
}

/**
 * @brief Test readHeaders.
 */
TEST (HttpRequest, readHeaders)
{
    std::stringstream ss;
    ss << "GET /path?val1=1&val2=2 HTTP/1.0\r\n";
    ss << "Connection: keep-alive\r\n";
    ss << "\r\n";

    HttpRequest request;
    request.readHeaders (ss);
    ASSERT_TRUE (ss.good ()) << join::lastError.message ();
    ASSERT_EQ (request.method (), HttpMethod::Get);
    ASSERT_EQ (request.path (), "/path");
    ASSERT_EQ (request.parameter ("val1"), "1");
    ASSERT_EQ (request.parameter ("val2"), "2");
    ASSERT_EQ (request.version (), "HTTP/1.0");
    ASSERT_EQ (request.header ("Connection"), "keep-alive");

    ss.clear ();
    ss.str ("");
    ss << "HEAD / HTTP/1.1\r\n";
    ss << "\r\n";

    request.readHeaders (ss);
    ASSERT_TRUE (ss.good ()) << join::lastError.message ();
    ASSERT_EQ (request.method (), HttpMethod::Head);

    ss.clear ();
    ss.str ("");
    ss << "PUT / HTTP/1.1\r\n";
    ss << "\r\n";

    request.readHeaders (ss);
    ASSERT_TRUE (ss.good ()) << join::lastError.message ();
    ASSERT_EQ (request.method (), HttpMethod::Put);

    ss.clear ();
    ss.str ("");
    ss << "POST / HTTP/1.1\r\n";
    ss << "\r\n";

    request.readHeaders (ss);
    ASSERT_TRUE (ss.good ()) << join::lastError.message ();
    ASSERT_EQ (request.method (), HttpMethod::Post);

    ss.clear ();
    ss.str ("");
    ss << "DELETE / HTTP/1.1\r\n";
    ss << "\r\n";

    request.readHeaders (ss);
    ASSERT_TRUE (ss.good ()) << join::lastError.message ();
    ASSERT_EQ (request.method (), HttpMethod::Delete);

    ss.clear ();
    ss.str ("");
    ss << "GET";

    request.readHeaders (ss);
    ASSERT_TRUE (ss.fail ());
    ASSERT_EQ (join::lastError, Errc::ConnectionClosed);

    ss.clear ();
    ss.str ("");
    ss << "GET\r\n";
    ss << "\r\n";

    request.readHeaders (ss);
    ASSERT_TRUE (ss.fail ());
    ASSERT_EQ (join::lastError, HttpErrc::BadRequest);

    ss.clear ();
    ss.str ("");
    ss << "GET /\r\n";
    ss << "\r\n";

    request.readHeaders (ss);
    ASSERT_TRUE (ss.fail ());
    ASSERT_EQ (join::lastError, HttpErrc::BadRequest);

    ss.clear ();
    ss.str ("");
    ss << "BLAH / HTTP/1.1\r\n";
    ss << "\r\n";

    request.readHeaders (ss);
    ASSERT_TRUE (ss.fail ());
    ASSERT_EQ (join::lastError, HttpErrc::ForbiddenMethod);

    ss.clear ();
    ss.str ("");
    ss << "GET / HTTP/1.1\r\n";
    ss << "Connection keep-alive\r\n";
    ss << "\r\n";

    request.readHeaders (ss);
    ASSERT_TRUE (ss.fail ());
    ASSERT_EQ (join::lastError, HttpErrc::BadRequest);

    ss.clear ();
    ss.str (std::string (8192, 'X'));

    request.readHeaders (ss);
    ASSERT_TRUE (ss.fail ());
    ASSERT_EQ (join::lastError, HttpErrc::HeaderTooLarge);
}

/**
 * @brief Test decodeUrl.
 */
TEST (HttpRequest, decodeUrl)
{
    std::stringstream ss;
    ss << "GET /foo%20bar?baz=3%20fuz HTTP/1.1\r\n";
    ss << "\r\n";

    HttpRequest request;
    request.readHeaders (ss);
    ASSERT_EQ (request.path (), "/foo bar");
    ASSERT_EQ (request.parameter ("baz"), "3 fuz");
}

/**
 * @brief Test normalize.
 */
TEST (HttpRequest, normalize)
{
    std::stringstream ss;
    ss << "GET // HTTP/1.1\r\n";
    ss << "\r\n";

    HttpRequest request;
    request.readHeaders (ss);
    ASSERT_TRUE (ss.good ()) << join::lastError.message ();
    ASSERT_EQ (request.path (), "/");

    ss.clear ();
    ss.str ("");
    ss << "GET ../ HTTP/1.1\r\n";
    ss << "\r\n";

    request.readHeaders (ss);
    ASSERT_TRUE (ss.good ()) << join::lastError.message ();
    ASSERT_EQ (request.path (), "");

    ss.clear ();
    ss.str ("");
    ss << "GET ./ HTTP/1.1\r\n";
    ss << "\r\n";

    request.readHeaders (ss);
    ASSERT_TRUE (ss.good ()) << join::lastError.message ();
    ASSERT_EQ (request.path (), "");

    ss.clear ();
    ss.str ("");
    ss << "GET /./ HTTP/1.1\r\n";
    ss << "\r\n";

    request.readHeaders (ss);
    ASSERT_TRUE (ss.good ()) << join::lastError.message ();
    ASSERT_EQ (request.path (), "/");

    ss.clear ();
    ss.str ("");
    ss << "GET /. HTTP/1.1\r\n";
    ss << "\r\n";

    request.readHeaders (ss);
    ASSERT_TRUE (ss.good ()) << join::lastError.message ();
    ASSERT_EQ (request.path (), "/");

    ss.clear ();
    ss.str ("");
    ss << "GET /../ HTTP/1.1\r\n";
    ss << "\r\n";

    request.readHeaders (ss);
    ASSERT_TRUE (ss.good ()) << join::lastError.message ();
    ASSERT_EQ (request.path (), "/");

    ss.clear ();
    ss.str ("");
    ss << "GET /path/../ HTTP/1.1\r\n";
    ss << "\r\n";

    request.readHeaders (ss);
    ASSERT_TRUE (ss.good ()) << join::lastError.message ();
    ASSERT_EQ (request.path (), "/");

    ss.clear ();
    ss.str ("");
    ss << "GET /.. HTTP/1.1\r\n";
    ss << "\r\n";

    request.readHeaders (ss);
    ASSERT_TRUE (ss.good ()) << join::lastError.message ();
    ASSERT_EQ (request.path (), "/");

    ss.clear ();
    ss.str ("");
    ss << "GET /path/.. HTTP/1.1\r\n";
    ss << "\r\n";

    request.readHeaders (ss);
    ASSERT_TRUE (ss.good ()) << join::lastError.message ();
    ASSERT_EQ (request.path (), "/");

    ss.clear ();
    ss.str ("");
    ss << "GET . HTTP/1.1\r\n";
    ss << "\r\n";

    request.readHeaders (ss);
    ASSERT_TRUE (ss.good ()) << join::lastError.message ();
    ASSERT_EQ (request.path (), "");
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
