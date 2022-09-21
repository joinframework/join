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
#include <join/version.hpp>
#include <join/httpclient.hpp>

using join::HttpRequest;
using join::HttpResponse;
using join::HttpClient;
using join::Tls;

// =========================================================================
//   CLASS     : HttpClient
//   METHOD    : HttpClient
// =========================================================================
HttpClient::HttpClient (const Tls::Endpoint& endpoint, bool encrypt, bool keepAlive)
: HttpClient (endpoint.hostname (), endpoint.port (), encrypt, keepAlive)
{
}

// =========================================================================
//   CLASS     : HttpClient
//   METHOD    : HttpClient
// =========================================================================
HttpClient::HttpClient (const std::string& host, uint16_t port, bool encrypt, bool keepAlive)
: Tls::Stream (),
  _host (host),
  _port (port),
  _encrypt (encrypt),
  _keepAlive (keepAlive)
{
}

// =========================================================================
//   CLASS     : HttpClient
//   METHOD    : HttpClient
// =========================================================================
HttpClient::HttpClient (HttpClient&& other)
: Tls::Stream (std::move (other)),
  _host (std::move (other._host)),
  _port (other._port),
  _encrypt (other._encrypt),
  _keepAlive (other._keepAlive)
{
    other._port = 443;
    other._encrypt = true;
    other._keepAlive = false;
}

// =========================================================================
//   CLASS     : HttpClient
//   METHOD    : operator=
// =========================================================================
HttpClient& HttpClient::operator= (HttpClient&& other)
{
    Tls::Stream::operator= (std::move (other));
    _host = std::move (other._host);
    _port = other._port;
    _encrypt = other._encrypt;
    _keepAlive = other._keepAlive;

    other._port = 443;
    other._encrypt = true;
    other._keepAlive = false;

    return *this;
}

// =========================================================================
//   CLASS     : HttpClient
//   METHOD    : scheme
// =========================================================================
std::string HttpClient::scheme () const
{
    return _encrypt ? "https" : "http";
}

// =========================================================================
//   CLASS     : HttpClient
//   METHOD    : host
// =========================================================================
const std::string& HttpClient::host () const
{
    return _host;
}

// =========================================================================
//   CLASS     : HttpClient
//   METHOD    : port
// =========================================================================
uint16_t HttpClient::port () const
{
    return _port;
}

// =========================================================================
//   CLASS     : HttpClient
//   METHOD    : keepAlive
// =========================================================================
void HttpClient::keepAlive (bool k)
{
    _keepAlive = k;
}

// =========================================================================
//   CLASS     : HttpClient
//   METHOD    : keepAlive
// =========================================================================
bool HttpClient::keepAlive () const
{
    return _keepAlive;
}

// =========================================================================
//   CLASS     : HttpClient
//   METHOD    : send
// =========================================================================
void HttpClient::send (const HttpRequest& request)
{
    if (!connected ())
    {
        connect ({Tls::Resolver::resolveHost (_host), _port});
        if (fail ())
        {
            return;
        }
    }

    if (_encrypt && !encrypted ())
    {
        startEncryption ();
        if (fail ())
        {
            return;
        }
    }

    HttpRequest tmp (request);

    if (!tmp.hasHeader ("Accept"))
    {
        tmp.header ("Accept", "*/*");
    }
    if (!tmp.hasHeader ("Connection"))
    {
        tmp.header ("Connection", _keepAlive ? "keep-alive" : "close");
    }
    if (!tmp.hasHeader ("Host"))
    {
        std::string host;
        if (IpAddress::isIpAddress (_host) && IpAddress (_host).isIpv6Address ())
            host += "[" + _host + "]";
        else
            host += _host;
        host += ":" + std::to_string (_port);
        tmp.header ("Host", host);
    }
    if (!tmp.hasHeader ("User-Agent"))
    {
        tmp.header ("User-Agent", "join/" JOIN_VERSION);
    }

    tmp.send (*this);
    flush ();
}

// =========================================================================
//   CLASS     : HttpClient
//   METHOD    : receive
// =========================================================================
void HttpClient::receive (HttpResponse& response)
{
    response.receive (*this);

    if (compareNoCase (response.header ("Connection"), "close"))
    {
        close ();
    }
}

// =========================================================================
//   CLASS     :
//   METHOD    : operator<<
// =========================================================================
HttpClient& join::operator<< (HttpClient& out, const HttpRequest& request)
{
    out.send (request);
    return out;
}

// =========================================================================
//   CLASS     :
//   METHOD    : operator>>
// =========================================================================
HttpClient& join::operator>> (HttpClient& in, HttpResponse& response)
{
    in.receive (response);
    return in;
}
