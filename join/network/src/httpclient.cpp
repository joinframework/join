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

using namespace std::chrono;

using join::HttpRequest;
using join::HttpResponse;
using join::HttpClient;
using join::Tls;

// =========================================================================
//   CLASS     : HttpClient
//   METHOD    : HttpClient
// =========================================================================
HttpClient::HttpClient (const char* host, uint16_t port, bool encrypt, bool keepAlive)
: Tls::Stream (),
  _host (host),
  _port (port),
  _encrypt (encrypt),
  _keep (keepAlive),
  _keepTimeout (seconds::zero ())
{
}

// =========================================================================
//   CLASS     : HttpClient
//   METHOD    : HttpClient
// =========================================================================
HttpClient::HttpClient (const std::string& host, uint16_t port, bool encrypt, bool keepAlive)
: HttpClient (host.c_str (), port, encrypt, keepAlive)
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
  _keep (other._keep),
  _keepTimeout (other._keepTimeout),
  _keepMax (other._keepMax)
{
    other._port = 443;
    other._encrypt = true;
    other._keep = true;
    other._keepTimeout = seconds::zero ();
    other._keepMax = -1;
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
    _keep = other._keep;
    _keepTimeout = other._keepTimeout;
    _keepMax = other._keepMax;

    other._port = 443;
    other._encrypt = true;
    other._keep = true;
    other._keepTimeout = seconds::zero ();
    other._keepMax = -1;

    return *this;
}

// =========================================================================
//   CLASS     : HttpClient
//   METHOD    : close
// =========================================================================
void HttpClient::close ()
{
    Tls::Stream::close ();
    _keepTimeout = seconds::zero ();
    _keepMax = -1;
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
//   METHOD    : authority
// =========================================================================
std::string HttpClient::authority () const
{
    std::string out;
    if (IpAddress::isIpv6Address (host ()))
        out += "[" + host () + "]";
    else
        out += host ();
    if (port () && (port () != Resolver::resolveService (scheme ())))
        out += ":" + std::to_string (port ());
    return out;
}

// =========================================================================
//   CLASS     : HttpClient
//   METHOD    : url
// =========================================================================
std::string HttpClient::url () const
{
    return scheme () + "://" + authority () + "/";
}

// =========================================================================
//   CLASS     : HttpClient
//   METHOD    : keepAlive
// =========================================================================
void HttpClient::keepAlive (bool keep)
{
    _keep = keep;
}

// =========================================================================
//   CLASS     : HttpClient
//   METHOD    : keepAlive
// =========================================================================
bool HttpClient::keepAlive () const
{
    return _keep;
}

// =========================================================================
//   CLASS     : HttpClient
//   METHOD    : keepAliveTimeout
// =========================================================================
seconds HttpClient::keepAliveTimeout () const
{
    return _keepTimeout;
}

// =========================================================================
//   CLASS     : HttpClient
//   METHOD    : keepAliveMax
// =========================================================================
int HttpClient::keepAliveMax () const
{
    return _keepMax;
}

// =========================================================================
//   CLASS     : HttpClient
//   METHOD    : send
// =========================================================================
void HttpClient::send (const HttpRequest& request)
{
    if (needReconnection ())
    {
        reconnect (url ());
        if (fail ())
        {
            return;
        }
    }

    if (needEncryption ())
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
        tmp.header ("Connection", _keep ? "keep-alive" : "close");
    }
    if (!tmp.hasHeader ("Host"))
    {
        tmp.header ("Host", authority ());
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
    if (fail ())
    {
        return;
    }

    std::string connection = response.header ("Connection");
    std::string alive = response.header ("Keep-Alive");

    if (compareNoCase (connection, "keep-alive"))
    {
        size_t pos = alive.find ("timeout=");
        if (pos != std::string::npos)
        {
            _keepTimeout = seconds (::atoi (alive.substr (pos + 8, alive.find (",", pos + 8)).c_str ()));
        }

        pos = alive.find ("max=");
        if (pos != std::string::npos)
        {
            _keepMax = ::atoi (alive.substr (pos + 4, alive.find (",", pos + 4)).c_str ());
        }
    }
    else if (compareNoCase (connection, "close"))
    {
        _keepTimeout = seconds::zero ();
        _keepMax = 0;
    }

    _timestamp = steady_clock::now ();
}

// =========================================================================
//   CLASS     : HttpClient
//   METHOD    : expired
// =========================================================================
bool HttpClient::expired () const
{
    return (_keepTimeout < duration_cast <seconds> (steady_clock::now () - _timestamp)) || (_keepMax == 0);
}

// =========================================================================
//   CLASS     : HttpClient
//   METHOD    : needReconnection
// =========================================================================
bool HttpClient::needReconnection ()
{
    return !connected () || expired ();
}

// =========================================================================
//   CLASS     : HttpClient
//   METHOD    : reconnect
// =========================================================================
void HttpClient::reconnect (const Endpoint& endpoint)
{
    close ();
    connect (endpoint);
}

// =========================================================================
//   CLASS     : HttpClient
//   METHOD    : needEncryption
// =========================================================================
bool HttpClient::needEncryption () const
{
    return _encrypt && !encrypted ();
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
