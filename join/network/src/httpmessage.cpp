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
using join::HttpMethod;
using join::HttpRequest;
using join::HttpResponse;

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
        case HttpErrc::BadRequest:
            return "bad request";
        case HttpErrc::Unauthorized:
            return "unauthorized";
        case HttpErrc::Forbidden:
            return "forbidden";
        case HttpErrc::NotFound:
            return "not found";
        case HttpErrc::Unsupported:
            return "method not allowed";
        case HttpErrc::LengthRequired:
            return "length required";
        case HttpErrc::PayloadTooLarge:
            return "payload too large";
        case HttpErrc::UriTooLong:
            return "URI too long";
        case HttpErrc::HeaderTooLarge:
            return "request header too large";
        case HttpErrc::ServerError:
            return "internal server error";
        case HttpErrc::NotImplemented:
            return "not implemented";
        case HttpErrc::BadGateway:
            return "bad gateway";
        default:
            return "success";
    }
}

// =========================================================================
//   CLASS     : HttpCategory
//   METHOD    : equivalent
// =========================================================================
bool HttpCategory::equivalent (const std::error_code& code, int condition) const noexcept
{
    switch (static_cast <HttpErrc> (condition))
    {
        case HttpErrc::HeaderTooLarge:
            return code == join::Errc::MessageTooLong;
        default:
            return false;
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
//   METHOD    : contentLength
// =========================================================================
size_t HttpMessage::contentLength () const
{
    size_t length = 0;
    std::istringstream conv (header ("Content-Length"));
    return (conv >> length && conv.eof ()) ? length : 0;
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
//   METHOD    : readHeaders
// =========================================================================
int HttpMessage::readHeaders (std::istream& in)
{
    bool firstLine = true;
    std::string line;

    for (;;)
    {
        if (!join::getline (in, line, _maxHeaderLen))
        {
            return -1;
        }

    #ifdef DEBUG
        std::cout << line << std::endl;
    #endif

        if (firstLine)
        {
            if (parseFirstLine (line) == -1)
            {
                return -1;
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
            return -1;
        }
    }

    return 0;
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
        join::lastError = make_error_code (HttpErrc::BadRequest);
        return -1;
    }

    _headers[head.substr (0, pos)] = head.substr (pos + 2);

    return 0;
}

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
//   CLASS     : HttpRequest
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
//   METHOD    : host
// =========================================================================
std::string HttpRequest::host () const
{
    std::string host = header ("Host");
    if (host.front () == '[')
    {
        auto end = host.find ("]");
        if (end == std::string::npos)
        {
            return {};
        }
        return host.substr (0, ++end);
    }
    return host.substr (0, host.find (":"));
}

// =========================================================================
//   CLASS     : HttpRequest
//   METHOD    : auth
// =========================================================================
std::string HttpRequest::auth () const
{
    std::string authorization = header ("Authorization");
    return authorization.substr (0, authorization.find (" "));
}

// =========================================================================
//   CLASS     : HttpRequest
//   METHOD    : credentials
// =========================================================================
std::string HttpRequest::credentials () const
{
    std::string authorization = header ("Authorization");
    auto beg = authorization.find (" ");
    if (beg == std::string::npos)
    {
        return {};
    }
    return authorization.substr (++beg);
}

// =========================================================================
//   CLASS     : HttpRequest
//   METHOD    : clear
// =========================================================================
void HttpRequest::clear ()
{
    HttpMessage::clear ();
    _method = Get;
    _path = "/";
    _parameters.clear ();
}

// =========================================================================
//   CLASS     : HttpRequest
//   METHOD    : writeHeaders
// =========================================================================
int HttpRequest::writeHeaders (std::ostream& out) const
{
    out << methodString () << " " << urn () << " " << version () << "\r\n";
    out << dumpHeaders ();
    if (out.fail ())
    {
        return -1;
    }
    return 0;
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
        join::lastError = make_error_code (HttpErrc::BadRequest);
        return -1;
    }

    size_t pos2 = line.find (" ", pos1 + 1);
    if (pos2 == std::string::npos)
    {
        join::lastError = make_error_code (HttpErrc::BadRequest);
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
        join::lastError = make_error_code (HttpErrc::Unsupported);
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
    size_t pos = 0;

    while (pos < url.length ())
    {
        if (url[pos] == '%')
        {
            std::stringstream ss1, ss2;
            ss1 << url[++pos];
            ss2 << url[++pos];
            unsigned int dec, dec1, dec2;
            ss1 >> std::hex >> dec1;
            ss2 >> std::hex >> dec2;
            dec = (dec1 << 4) + dec2;
            oss << static_cast <char> (dec);
            ++pos;
        }
        else
        {
            oss << url[pos++];
        }
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
    HttpMessage::clear ();
    _status.clear ();
    _reason.clear ();
}

// =========================================================================
//   CLASS     : HttpResponse
//   METHOD    : writeHeaders
// =========================================================================
int HttpResponse::writeHeaders (std::ostream& out) const
{
    out << version () << " " << status () << " " << reason () << "\r\n";
    out << dumpHeaders ();
    if (out.fail ())
    {
        return -1;
    }
    return 0;
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
        join::lastError = make_error_code (HttpErrc::BadRequest);
        return -1;
    }

    size_t pos2 = line.find (" ", pos1 + 1);
    if (pos2 == std::string::npos)
    {
        join::lastError = make_error_code (HttpErrc::BadRequest);
        return -1;
    }

    _version = line.substr (0, pos1++);
    _status  = line.substr (pos1, pos2 - pos1);
    _reason  = line.substr (++pos2);

    return 0;
}
