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

using join::Errc;
using join::HttpErrc;
using join::HttpCategory;
using join::HttpMessage;

// =========================================================================
//   CLASS     : HttpCategory
//   METHOD    : name
// =========================================================================
const char* HttpCategory::name () const noexcept
{
    return "libjoin";
}

// =========================================================================
//   CLASS     : HttpCategory
//   METHOD    : message
// =========================================================================
std::string HttpCategory::message (int code) const
{
    switch (static_cast <HttpErrc> (code))
    {
        case HttpErrc::InvalidRequest:
            return "invalid HTTP request";
        case HttpErrc::InvalidResponse:
            return "invalid HTTP response";
        case HttpErrc::InvalidMethod:
            return "invalid HTTP method";
        case HttpErrc::InvalidHeader:
            return "invalid HTTP header";
        default:
            return "success";
    }
}

// =========================================================================
//   CLASS     :
//   METHOD    : httpCategory
// =========================================================================
const std::error_category& join::httpCategory ()
{
    static HttpCategory instance;
    return instance;
}

// =========================================================================
//   CLASS     :
//   METHOD    : make_error_code
// =========================================================================
std::error_code join::make_error_code (HttpErrc code)
{
    return std::error_code (static_cast <int> (code), httpCategory ());
}

// =========================================================================
//   CLASS     :
//   METHOD    : make_error_condition
// =========================================================================
std::error_condition join::make_error_condition (HttpErrc code)
{
    return std::error_condition (static_cast <int> (code), httpCategory ());
}

// =========================================================================
//   CLASS     : HttpMessage
//   METHOD    : HttpMessage
// =========================================================================
HttpMessage::HttpMessage ()
: _version ("HTTP/1.1")
{
}

// =========================================================================
//   CLASS     : HttpMessage
//   METHOD    : HttpMessage
// =========================================================================
HttpMessage::HttpMessage (const HttpMessage& other)
: _version (other._version),
  _headers (other._headers)
{
}

// =========================================================================
//   CLASS     : HttpMessage
//   METHOD    : operator=
// =========================================================================
HttpMessage& HttpMessage::operator= (const HttpMessage& other)
{
    _version = other._version;
    _headers = other._headers;

    return *this;
}

// =========================================================================
//   CLASS     : HttpMessage
//   METHOD    : HttpMessage
// =========================================================================
HttpMessage::HttpMessage (HttpMessage&& other)
: _version (std::move (other._version)),
  _headers (std::move (other._headers))
{
    other._version = "HTTP/1.1";
}

// =========================================================================
//   CLASS     : HttpMessage
//   METHOD    : operator=
// =========================================================================
HttpMessage& HttpMessage::operator= (HttpMessage&& other)
{
    _version = std::move (other._version);
    _headers = std::move (other._headers);

    other._version = "HTTP/1.1";

    return *this;
}

// =========================================================================
//   CLASS     : HttpMessage
//   METHOD    : version
// =========================================================================
const std::string& HttpMessage::version () const
{
    return _version;
}

// =========================================================================
//   CLASS     : HttpMessage
//   METHOD    : version
// =========================================================================
void HttpMessage::version (const std::string& v)
{
    _version = v;
}

// =========================================================================
//   CLASS     : HttpMessage
//   METHOD    : hasHeader
// =========================================================================
bool HttpMessage::hasHeader (const std::string& name) const
{
    return _headers.find (name) != _headers.end ();
}

// =========================================================================
//   CLASS     : HttpMessage
//   METHOD    : header
// =========================================================================
std::string HttpMessage::header (const std::string& name) const
{
    auto it = _headers.find (name);
    if (it != _headers.end ())
    {
        return it->second;
    }

    return {};
}

// =========================================================================
//   CLASS     : HttpMessage
//   METHOD    : header
// =========================================================================
void HttpMessage::header (const std::string& name, const std::string& val)
{
    _headers[name] = val;
}

// =========================================================================
//   CLASS     : HttpMessage
//   METHOD    : header
// =========================================================================
void HttpMessage::header (const HeaderMap::value_type& h)
{
    header (h.first, h.second);
}

// =========================================================================
//   CLASS     : HttpMessage
//   METHOD    : headers
// =========================================================================
const HttpMessage::HeaderMap& HttpMessage::headers () const
{
    return _headers;
}

// =========================================================================
//   CLASS     : HttpMessage
//   METHOD    : headers
// =========================================================================
void HttpMessage::headers (const HeaderMap& heads)
{
    for (auto const& head : heads)
    {
        header (head);
    }
}

// =========================================================================
//   CLASS     : HttpMessage
//   METHOD    : dumpHeaders
// =========================================================================
std::string HttpMessage::dumpHeaders () const
{
    std::string out;

    for (auto const& head : _headers)
    {
        out += head.first + ": " + head.second + "\r\n";
    }
    out += "\r\n";

    return out;
}

// =========================================================================
//   CLASS     : HttpMessage
//   METHOD    : clear
// =========================================================================
void HttpMessage::clear ()
{
    _version = "HTTP/1.1";
    _headers.clear ();
}

// =========================================================================
//   CLASS     : HttpMessage
//   METHOD    : receive
// =========================================================================
void HttpMessage::receive (std::istream& in)
{
    bool firstLine = true;
    std::string line;

    while (getline (in, line, 4096))
    {
    #ifdef DEBUG
        std::cout << line << "\r\n";
    #endif

        if (firstLine)
        {
            if (parseFirstLine (line) == -1)
            {
                in.setstate (std::ios_base::failbit);
                return;
            }
            firstLine = false;
            continue;
        }

        if (line.empty ())
        {
            break;
        }

        if (parseHeader (line) == -1)
        {
            in.setstate (std::ios_base::failbit);
            return;
        }
    }
}

// =========================================================================
//   CLASS     : HttpMessage
//   METHOD    : getline
// =========================================================================
std::istream& HttpMessage::getline (std::istream& in, std::string& line, uint32_t max)
{
    line.clear ();

    while (max--)
    {
        char ch = in.get ();
        if (in.eof ())
        {
            join::lastError = make_error_code (Errc::OperationFailed);
            return in;
        }

        if (ch == '\r')
        {
            continue;
        }

        if (ch == '\n')
        {
            return in;
        }

        line.push_back (ch);
    }

    join::lastError = make_error_code (Errc::MessageTooLong);
    in.setstate (std::ios_base::failbit);

    return in;
}

// =========================================================================
//   CLASS     : HttpMessage
//   METHOD    : parseHeader
// =========================================================================
int HttpMessage::parseHeader (const std::string& head)
{
    size_t pos = head.find (": ");
    if (pos == std::string::npos)
    {
        join::lastError = make_error_code (HttpErrc::InvalidHeader);
        return -1;
    }

    _headers[head.substr (0, pos)] = head.substr (pos + 2);

    return 0;
}
