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
#include <join/httpresponse.hpp>

using join::HttpErrc;
using join::HttpResponse;

// =========================================================================
//   CLASS     : HttpResponse
//   METHOD    : HttpResponse
// =========================================================================
HttpResponse::HttpResponse (const HttpResponse& other)
: HttpMessage (other),
  _status (other._status),
  _reason (other._reason)
{
}

// =========================================================================
//   CLASS     : HttpResponse
//   METHOD    : operator=
// =========================================================================
HttpResponse& HttpResponse::operator= (const HttpResponse& other)
{
    HttpMessage::operator= (other);

    _status = other._status;
    _reason = other._reason;

    return *this;
}

// =========================================================================
//   CLASS     : HttpResponse
//   METHOD    : HttpResponse
// =========================================================================
HttpResponse::HttpResponse (HttpResponse&& other)
: HttpMessage (std::move (other)),
  _status (std::move (other._status)),
  _reason (std::move (other._reason))
{
}

// =========================================================================
//   CLASS     : HttpResponse
//   METHOD    : operator=
// =========================================================================
HttpResponse& HttpResponse::operator= (HttpResponse&& other)
{
    HttpMessage::operator= (std::move (other));

    _status = std::move (other._status);
    _reason = std::move (other._reason);

    return *this;
}

// =========================================================================
//   CLASS     : HttpResponse
//   METHOD    : status
// =========================================================================
const std::string& HttpResponse::status () const
{
    return _status;
}

// =========================================================================
//   CLASS     : HttpResponse
//   METHOD    : reason
// =========================================================================
const std::string& HttpResponse::reason () const
{
    return _reason;
}

// =========================================================================
//   CLASS     : HttpResponse
//   METHOD    : response
// =========================================================================
void HttpResponse::response (const std::string& status, const std::string& reason)
{
    _status = status;
    _reason = reason;
}

// =========================================================================
//   CLASS     : HttpResponse
//   METHOD    : clear
// =========================================================================
void HttpResponse::clear ()
{
    _status.clear ();
    _reason.clear ();
    HttpMessage::clear ();
}

// =========================================================================
//   CLASS     : HttpResponse
//   METHOD    : send
// =========================================================================
void HttpResponse::send (std::ostream& out) const
{
    out << version () << " " << status () << " " << reason () << "\r\n";
    out << dumpHeaders ();
}

// =========================================================================
//   CLASS     : HttpResponse
//   METHOD    : parseFirstLine
// =========================================================================
int HttpResponse::parseFirstLine (const std::string& line)
{
    size_t pos1 = line.find (" ");
    if (pos1 == std::string::npos)
    {
        join::lastError = make_error_code (HttpErrc::InvalidResponse);
        return -1;
    }

    size_t pos2 = line.find (" ", pos1 + 1);
    if (pos2 == std::string::npos)
    {
        join::lastError = make_error_code (HttpErrc::InvalidResponse);
        return -1;
    }

    _version = line.substr (0, pos1++);

    _status = line.substr (pos1, pos2 - pos1);

    _reason = line.substr (++pos2);

    return 0;
}
