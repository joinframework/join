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

// C++.

using join::HttpMessage;

// =========================================================================
//   CLASS     : HttpMessage
//   METHOD    : HttpMessage
// =========================================================================
HttpMessage::HttpMessage ()
: _path ("/"),
  _version ("HTTP/1.1")
{
}

// =========================================================================
//   CLASS     : HttpMessage
//   METHOD    : HttpMessage
// =========================================================================
HttpMessage::HttpMessage (const HttpMessage& other)
: _path (other._path),
  _version (other._version),
  _headers (other._headers)
{
}

// =========================================================================
//   CLASS     : HttpMessage
//   METHOD    : operator=
// =========================================================================
HttpMessage& HttpMessage::operator= (const HttpMessage& other)
{
    _path = other._path;
    _version = other._version;
    _headers = other._headers;

    return *this;
}

// =========================================================================
//   CLASS     : HttpMessage
//   METHOD    : HttpMessage
// =========================================================================
HttpMessage::HttpMessage (HttpMessage&& other)
: _path (std::move (other._path)),
  _version (std::move (other._version)),
  _headers (std::move (other._headers))
{
    other._path = "/";
    other._version = "HTTP/1.1";
}

// =========================================================================
//   CLASS     : HttpMessage
//   METHOD    : operator=
// =========================================================================
HttpMessage& HttpMessage::operator= (HttpMessage&& other)
{
    _path = std::move (other._path);
    _version = std::move (other._version);
    _headers = std::move (other._headers);

    other._path = "/";
    other._version = "HTTP/1.1";

    return *this;
}

// =========================================================================
//   CLASS     : HttpMessage
//   METHOD    : setPath
// =========================================================================
void HttpMessage::path (const std::string& p)
{
    _path = p;
}

// =========================================================================
//   CLASS     : HttpMessage
//   METHOD    : getPath
// =========================================================================
const std::string& HttpMessage::path () const
{
    return _path;
}

// =========================================================================
//   CLASS     : HttpMessage
//   METHOD    : setVersion
// =========================================================================
void HttpMessage::version (const std::string& v)
{
    _version = v;
}

// =========================================================================
//   CLASS     : HttpMessage
//   METHOD    : getVersion
// =========================================================================
const std::string& HttpMessage::version () const
{
    return _version;
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
//   METHOD    : getHeader
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
//   METHOD    : hasHeader
// =========================================================================
bool HttpMessage::hasHeader (const std::string& name) const
{
    return _headers.find (name) != _headers.end ();
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

    in.setstate (std::ios_base::failbit);
    return in;
}
