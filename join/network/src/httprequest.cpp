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

// C++.
#include <sstream>

using join::HttpErrc;
using join::HttpMethod;
using join::HttpRequest;

// =========================================================================
//   CLASS     : HttpRequest
//   METHOD    : HttpRequest
// =========================================================================
HttpRequest::HttpRequest ()
: HttpMessage (),
  _path ("/")
{
}

// =========================================================================
//   CLASS     : HttpRequest
//   METHOD    : HttpRequest
// =========================================================================
HttpRequest::HttpRequest (HttpMethod method)
: HttpMessage (),
  _method (method),
  _path ("/")
{
}

// =========================================================================
//   CLASS     : HttpRequest
//   METHOD    : HttpRequest
// =========================================================================
HttpRequest::HttpRequest (const HttpRequest& other)
: HttpMessage (other),
  _method (other._method),
  _path (other._path),
  _parameters (other._parameters)
{
}

// =========================================================================
//   CLASS     : HttpRequest
//   METHOD    : operator=
// =========================================================================
HttpRequest& HttpRequest::operator= (const HttpRequest& other)
{
    HttpMessage::operator= (other);

    _method = other._method;
    _path = other._path;
    _parameters = other._parameters;

    return *this;
}

// =========================================================================
//   CLASS     : HttpRequest
//   METHOD    : HttpRequest
// =========================================================================
HttpRequest::HttpRequest (HttpRequest&& other)
: HttpMessage (std::move (other)),
  _method (other._method),
  _path (std::move (other._path)),
  _parameters (std::move (other._parameters))
{
    other._method = Get;
    other._path = "/";
}

// =========================================================================
//   CLASS     : HttpRequest
//   METHOD    : operator=
// =========================================================================
HttpRequest& HttpRequest::operator= (HttpRequest&& other)
{
    HttpMessage::operator= (std::move (other));

    _method = other._method;
    _path = std::move (other._path);
    _parameters = std::move (other._parameters);

    other._method = Get;
    other._path = "/";

    return *this;
}

// =========================================================================
//   CLASS     : HttpRequest
//   METHOD    : method
// =========================================================================
HttpMethod HttpRequest::method () const
{
    return _method;
}

// =========================================================================
//   CLASS     : HttpRequest
//   METHOD    : methodString
// =========================================================================
std::string HttpRequest::methodString () const
{
    switch (_method)
    {
        case Head:
            return "HEAD";
        case Put:
            return "PUT";
        case Post:
            return "POST";
        case Delete:
            return "DELETE";
        default:
            return "GET";
    }
}

// =========================================================================
//   CLASS     : HttpRequest
//   METHOD    : method
// =========================================================================
void HttpRequest::method (HttpMethod meth)
{
    _method = meth;
}

// =========================================================================
//   CLASS     : HttpRequest
//   METHOD    : path
// =========================================================================
const std::string& HttpRequest::path () const
{
    return _path;
}

// =========================================================================
//   CLASS     : HttpRequest
//   METHOD    : path
// =========================================================================
void HttpRequest::path (const std::string& p)
{
    _path = p;
}

// =========================================================================
//   CLASS     : HttpMessage
//   METHOD    : hasParameter
// =========================================================================
bool HttpRequest::hasParameter (const std::string& name) const
{
    return _parameters.find (name) != _parameters.end ();
}

// =========================================================================
//   CLASS     : HttpRequest
//   METHOD    : parameter
// =========================================================================
std::string HttpRequest::parameter (const std::string& name) const
{
    auto it = _parameters.find (name);
    if (it != _parameters.end ())
    {
        return it->second;
    }

    return {};
}

// =========================================================================
//   CLASS     : HttpRequest
//   METHOD    : parameter
// =========================================================================
void HttpRequest::parameter (const std::string &name, const std::string &var)
{
    _parameters[name] = var;
}

// =========================================================================
//   CLASS     : HttpRequest
//   METHOD    : parameter
// =========================================================================
void HttpRequest::parameter (const ParameterMap::value_type& param)
{
    parameter (param.first, param.second);
}

// =========================================================================
//   CLASS     : HttpMessage
//   METHOD    : parameters
// =========================================================================
const HttpRequest::ParameterMap& HttpRequest::parameters () const
{
    return _parameters;
}

// =========================================================================
//   CLASS     : HttpMessage
//   METHOD    : parameters
// =========================================================================
void HttpRequest::parameters (const ParameterMap& params)
{
    for (auto const& param : params)
    {
        parameter (param);
    }
}

// =========================================================================
//   CLASS     : HttpRequest
//   METHOD    : dumpParameters
// =========================================================================
std::string HttpRequest::dumpParameters () const
{
    std::string params;

    for (auto const& param : _parameters)
    {
        params += param.first + "=" + param.second + "&";
    }

    if (params.size ())
    {
        params.pop_back ();
    }

    return params;
}

// =========================================================================
//   CLASS     : HttpRequest
//   METHOD    : query
// =========================================================================
std::string HttpRequest::query () const
{
    std::string params = dumpParameters ();

    if (params.size ())
    {
        params.insert (0, "?");
    }

    return params;
}

// =========================================================================
//   CLASS     : HttpRequest
//   METHOD    : urn
// =========================================================================
std::string HttpRequest::urn () const
{
    return path () + query ();
}

// =========================================================================
//   CLASS     : HttpRequest
//   METHOD    : clear
// =========================================================================
void HttpRequest::clear ()
{
    _method = Get;
    _path = "/";
    _parameters.clear ();
    HttpMessage::clear ();
}

// =========================================================================
//   CLASS     : HttpRequest
//   METHOD    : send
// =========================================================================
void HttpRequest::send (std::ostream& out) const
{
    out << methodString () << " " << urn () << " " << version () << "\r\n";
    out << dumpHeaders ();
}

// =========================================================================
//   CLASS     : HttpRequest
//   METHOD    : parseFirstLine
// =========================================================================
int HttpRequest::parseFirstLine (const std::string& line)
{
    size_t pos1 = line.find (" ");
    if (pos1 == std::string::npos)
    {
        join::lastError = make_error_code (HttpErrc::InvalidRequest);
        return -1;
    }

    size_t pos2 = line.find (" ", pos1 + 1);
    if (pos2 == std::string::npos)
    {
        join::lastError = make_error_code (HttpErrc::InvalidRequest);
        return -1;
    }

    std::string method = line.substr (0, pos1++);

    if (method.compare ("HEAD") == 0)
    {
        _method = HttpMethod::Head;
    }
    else if (method.compare ("GET") == 0)
    {
        _method = HttpMethod::Get;
    }
    else if (method.compare ("PUT") == 0)
    {
        _method = HttpMethod::Put;
    }
    else if (method.compare ("POST") == 0)
    {
        _method = HttpMethod::Post;
    }
    else if (method.compare ("DELETE") == 0)
    {
        _method = HttpMethod::Delete;
    }
    else
    {
        join::lastError = make_error_code (HttpErrc::InvalidMethod);
        return -1;
    }

    _path = line.substr (pos1, pos2 - pos1);

    pos1 = _path.find ("?");
    if (pos1 != std::string::npos)
    {
        store (_path.substr (pos1 + 1));
        _path = _path.substr (0, pos1);
    }

    decodeUrl (_path);
    normalize (_path);

    _version = line.substr (++pos2);

    return 0;
}

// =========================================================================
//   CLASS     : HttpRequest
//   METHOD    : decodeUrl
// =========================================================================
std::string& HttpRequest::decodeUrl (std::string &url)
{
    std::ostringstream oss;

    for (size_t pos = 0; pos < url.length (); ++pos)
    {
        if (url[pos] == '%')
        {
            std::stringstream ss1, ss2;
            ss1 << url[pos + 1];
            ss2 << url[pos + 2];
            unsigned int dec, dec1, dec2;
            ss1 >> std::hex >> dec1;
            ss2 >> std::hex >> dec2;
            dec = (dec1 << 4) + dec2;
            oss << static_cast <char> (dec);
            pos += 2;
        }
        else
            oss << url[pos];
    }
    url = oss.str ();

    return url;
}

// =========================================================================
//   CLASS     :
//   METHOD    : removeLastSegment
// =========================================================================
std::string& removeLastSegment (std::string &path)
{
    while (!path.empty ())
    {
        if (path.back () == '/')
        {
            path.pop_back ();
            break;
        }
        path.pop_back ();
    }

    return path;
}

// =========================================================================
//   CLASS     : HttpRequest
//   METHOD    : normalize
// =========================================================================
std::string& HttpRequest::normalize (std::string& path)
{
    std::string output;
    std::size_t pos = 0;

    while ((pos = path.find ("//", pos)) != std::string::npos)
    {
        path.replace (pos, 2, "/");
    }

    // rfc3986 (see https://tools.ietf.org/html/rfc3986#section-5.2.4).
    while (!path.empty ())
    {
        if (path.compare (0, 3, "../", 0, 3) == 0)
        {
            path.erase (0, 3);
        }
        else if (path.compare (0, 2, "./", 0, 2) == 0)
        {
            path.erase (0, 2);
        }
        else if (path.compare (0, 3, "/./", 0, 3) == 0)
        {
            path.replace (0, 3, "/");
        }
        else if (path.compare ("/.") == 0)
        {
            path.replace (0, 2, "/");
        }
        else if (path.compare (0, 4, "/../", 0, 4) == 0)
        {
            path.replace (0, 4, "/");
            removeLastSegment (output);
        }
        else if (path.compare ("/..") == 0)
        {
            path.replace (0, 3, "/");
            removeLastSegment (output);
        }
        else if (path.find_first_not_of (".") == std::string::npos)
        {
            path.clear();
        }
        else
        {
            pos = path.find ("/", (path.front() == '/') ? 1 : 0);
            output.append (path.substr (0, pos));
            path.erase (0, pos);
        }
    }

    path.swap (output);

    return path;
}

// =========================================================================
//   CLASS     : HttpRequest
//   METHOD    : store
// =========================================================================
void HttpRequest::store (const std::string &query)
{
    size_t pos = 0;

    for (;;)
    {
        size_t equalPos = query.find ("=", pos);
        if (equalPos == std::string::npos)
            return;

        size_t sepPos = query.find ("&", equalPos + 1);
        if (sepPos == std::string::npos)
            sepPos = query.length ();

        std::string name  = query.substr (pos, equalPos - pos);
        decodeUrl (name);

        std::string value = query.substr (equalPos + 1, sepPos - equalPos - 1);
        decodeUrl (value);

        parameter (name, value);

        if (sepPos == query.length ())
            return;

        pos = sepPos + 1;
    }
}
