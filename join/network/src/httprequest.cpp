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
#include <regex>

using join::HttpMethod;
using join::HttpRequest;

// =========================================================================
//   CLASS     : HttpRequest
//   METHOD    : HttpRequest
// =========================================================================
HttpRequest::HttpRequest (HttpMethod method)
: HttpMessage (),
  _method (method)
{
}

// =========================================================================
//   CLASS     : HttpRequest
//   METHOD    : HttpRequest
// =========================================================================
HttpRequest::HttpRequest (const HttpRequest& other)
: HttpMessage (other),
  _method (other._method),
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
  _parameters (std::move (other._parameters))
{
    other._method = Get;
}

// =========================================================================
//   CLASS     : HttpRequest
//   METHOD    : operator=
// =========================================================================
HttpRequest& HttpRequest::operator= (HttpRequest&& other)
{
    HttpMessage::operator= (std::move (other));

    _method = other._method;
    _parameters = std::move (other._parameters);

    other._method = Get;

    return *this;
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
        case Get:
            return "GET";
        case Put:
            return "PUT";
        case Post:
            return "POST";
        case Delete:
            return "DELETE";
        default:
            return "UNKNOWN";
    }
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
//   CLASS     : HttpMessage
//   METHOD    : hasParameter
// =========================================================================
bool HttpRequest::hasParameter (const std::string& name) const
{
    return _parameters.find (name) != _parameters.end ();
}

// =========================================================================
//   CLASS     : HttpRequest
//   METHOD    : query
// =========================================================================
std::string HttpRequest::query () const
{
    std::string out;

    for (auto next = _parameters.begin (); next != _parameters.end (); ++next)
    {
        out += next == _parameters.begin () ? "?" : "&";
        out += next->first + "=" + next->second;
    }

    return out;
}

// =========================================================================
//   CLASS     : HttpRequest
//   METHOD    : uri
// =========================================================================
std::string HttpRequest::uri () const
{
    return _path + query ();
}

// =========================================================================
//   CLASS     : HttpRequest
//   METHOD    : send
// =========================================================================
void HttpRequest::send (std::ostream& out) const
{
    out << methodString () << " " << uri () << " " << version () << "\r\n";
    for (auto const& header : _headers)
    {
        out << header.first << ": " << header.second << "\r\n";
    }
    out << "\r\n";
}

// =========================================================================
//   CLASS     : HttpRequest
//   METHOD    : receive
// =========================================================================
void HttpRequest::receive (std::istream& in)
{
    bool firstLine = true;
    std::string line;

    while (getline (in, line, 4096))
    {
        if (firstLine)
        {
            size_t pos1 = line.find (" ");
            if (pos1 == std::string::npos)
            {
                return;
            }

            size_t pos2 = line.find (" ", pos1 + 1);
            if (pos2 == std::string::npos)
            {
                return;
            }

            // get method.
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
                in.setstate (std::ios_base::failbit);
                return;
            }

            // get path.
            _path = line.substr (pos1, pos2 - pos1);

            // process query parameters.
            pos1 = _path.find ("?");
            if (pos1 != std::string::npos)
            {
                storeParameters (_path.substr (pos1 + 1));
                _path = _path.substr (0, pos1);
            }

            // decode and normalize path.
            decodeUrl (_path);
            normalize (_path);

            // get version.
            _version = line.substr (++pos2);

            firstLine = false;
            continue;
        }

        if (line.empty ())
        {
            // end of headers.
            break;
        }

        size_t pos = line.find (": ");
        if (pos == std::string::npos)
        {
            return;
        }

        _headers[line.substr (0, pos)] = line.substr (pos + 2);
    }
}

// =========================================================================
//   CLASS     : HttpRequest
//   METHOD    : receive
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
            size_t pos = path.find ("/", (path.front() == '/') ? 1 : 0);
            output.append (path.substr (0, pos));
            path.erase (0, pos);
        }
    }
    path.swap (output);

    return path;
}

// =========================================================================
//   CLASS     : HttpRequest
//   METHOD    : storeParameters
// =========================================================================
void HttpRequest::storeParameters (const std::string &query)
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
