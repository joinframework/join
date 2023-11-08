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

#ifndef __JOIN_HTTP_CLIENT_HPP__
#define __JOIN_HTTP_CLIENT_HPP__

// libjoin.
#include <join/version.hpp>
#include <join/zstream.hpp>
#include <join/chunkstream.hpp>
#include <join/httpmessage.hpp>
#include <join/socketstream.hpp>

// C++.
#include <chrono>

namespace join
{
    /**
     * @brief basic HTTP client.
     */
    template <class Protocol> 
    class BasicHttpClient : public Protocol::Stream
    {
    public:
        using Endpoint = typename Protocol::Endpoint;

        /**
         * @brief create the basic HTTP client instance.
         * @param host host.
         * @param port port (default: 80).
         * @param keepAlive use a persistent connection.
         */
        BasicHttpClient (const char* host, uint16_t port = 80, bool keepAlive = true)
        : _host (host),
          _port (port),
          _keep (keepAlive),
          _keepTimeout (std::chrono::seconds::zero ())
        {
        }

        /**
         * @brief create the basic HTTP client instance.
         * @param host host.
         * @param port port (default: 80).
         * @param keepAlive use a persistent connection.
         */
        BasicHttpClient (const std::string& host, uint16_t port = 80, bool keepAlive = true)
        : BasicHttpClient (host.c_str (), port, keepAlive)
        {
        }

        /**
         * @brief create the basic HTTP client instance by copy.
         * @param other request to copy.
         */
        BasicHttpClient (const BasicHttpClient& other) = delete;

        /**
         * @brief assign the basic HTTP client instance by copy.
         * @param other request to copy.
         * @return a reference of the current object.
         */
        BasicHttpClient& operator= (const BasicHttpClient& other) = delete;

        /**
         * @brief create the basic HTTP client instance by move.
         * @param other request to move.
         */
        BasicHttpClient (BasicHttpClient&& other)
        : Protocol::Stream (std::move (other)),
          _host (std::move (other._host)),
          _port (other._port),
          _keep (other._keep),
          _keepTimeout (other._keepTimeout),
          _keepMax (other._keepMax)
        {
        }

        /**
         * @brief assign the basic HTTP client instance by move.
         * @param other request to move.
         * @return a reference of the current object.
         */
        BasicHttpClient& operator= (BasicHttpClient&& other)
        {
            Protocol::Stream::operator= (std::move (other));
            this->_host = std::move (other._host);
            this->_port = other._port;
            this->_keep = other._keep;
            this->_keepTimeout = other._keepTimeout;
            this->_keepMax = other._keepMax;
            return *this;
        }

        /**
         * @brief destroy the basic HTTP client instance.
         */
        virtual ~BasicHttpClient ()
        {
            clearEncoding ();
        }

        /**
         * @brief close the connection.
         * @throw std::ios_base::failure.
         */
        void close () override
        {
            Protocol::Stream::close ();
            this->_keepTimeout = std::chrono::seconds::zero ();
            this->_keepMax = -1;
        }

        /**
         * @brief get HTTP scheme.
         * @return htpp or https.
         */
        virtual std::string scheme () const
        {
            return "http";
        }

        /**
         * @brief get host.
         * @return host.
         */
        const std::string& host () const
        {
            return this->_host;
        }

        /**
         * @brief get port.
         * @return port.
         */
        uint16_t port () const
        {
            return this->_port;
        }

        /**
         * @brief get authority.
         * @return authority.
         */
        std::string authority () const
        {
            std::string auth;

            if (IpAddress::isIpv6Address (this->host ()))
            {
                auth += "[" + this->host () + "]";
            }
            else
            {
                auth += this->host ();
            }

            if (this->port () != Resolver::resolveService (this->scheme ()))
            {
                auth += ":" + std::to_string (this->port ());
            }

            return auth;
        }

        /**
         * @brief get URL.
         * @return URL.
         */
        std::string url () const
        {
            return this->scheme () + "://" + this->authority () + "/";
        }

        /**
         * @brief checks if HTTP keep alive is supported.
         * @return true if supported, false otherwise.
         */
        bool keepAlive () const
        {
            return this->_keep;
        }

        /**
         * @brief enable/disable HTTP keep alive support.
         * @param keep true if supported, false otherwise.
         */
        void keepAlive (bool keep)
        {
            this->_keep = keep;
        }

        /**
         * @brief get HTTP keep alive timeout.
         * @return HTTP keep alive timeout.
         */
        std::chrono::seconds keepAliveTimeout () const
        {
            return this->_keepTimeout;
        }

        /**
         * @brief get HTTP keep alive max.
         * @return HTTP keep alive max.
         */
        int keepAliveMax () const
        {
            return this->_keepMax;
        }

        /**
         * @brief send HTTP request.
         * @param request HTTP request to send.
         * @return 0 on success, -1 on failure.
         */
        int send (HttpRequest& request)
        {
            // restore concrete stream.
            clearEncoding ();

            // check if reconnection is required.
            if (this->needReconnection ())
            {
                this->reconnect (this->url ());
                if (this->fail ())
                {
                    return -1;
                }
            }

            // set missing request headers.
            if (!request.hasHeader ("Accept"))
            {
                request.header ("Accept", "*/*");
            }
            if (!request.hasHeader ("Connection"))
            {
                request.header ("Connection", this->_keep ? "keep-alive" : "close");
            }
            if (!request.hasHeader ("Host"))
            {
                request.header ("Host", this->authority ());
            }
            if (!request.hasHeader ("User-Agent"))
            {
                request.header ("User-Agent", "join/" JOIN_VERSION);
            }

            // write request headers.
            if (request.writeHeaders (*this) == -1)
            {
                return -1;
            }

            // flush request headers.
            this->flush ();

            // set encoding.
            if (request.hasHeader ("Transfer-Encoding"))
            {
                this->setEncoding (join::rsplit (request.header ("Transfer-Encoding"), ","));
            }
            if (request.hasHeader ("Content-Encoding"))
            {
                this->setEncoding (join::rsplit (request.header ("Content-Encoding"), ","));
            }

            return 0;
        }

        /**
         * @brief receive HTTP response.
         * @param response HTTP response received.
         * @return 0 on success, -1 on failure.
         */
        int receive (HttpResponse& response)
        {
            // restore concrete stream.
            this->clearEncoding ();

            // read response headers.
            if (response.readHeaders (*this) == -1)
            {
                return -1;
            }

            // get connection.
            std::string connection = response.header ("Connection");
            std::string alive = response.header ("Keep-Alive");

            // check connection.
            if (join::compareNoCase (connection, "keep-alive"))
            {
                size_t pos = alive.find ("timeout=");
                if (pos != std::string::npos)
                {
                    this->_keepTimeout = std::chrono::seconds (::atoi (alive.substr (pos + 8, alive.find (",", pos + 8)).c_str ()));
                }

                pos = alive.find ("max=");
                if (pos != std::string::npos)
                {
                    this->_keepMax = ::atoi (alive.substr (pos + 4, alive.find (",", pos + 4)).c_str ());
                }
            }
            else if (join::compareNoCase (connection, "close"))
            {
                this->_keepTimeout = std::chrono::seconds::zero ();
                this->_keepMax = 0;
            }

            // set encoding.
            if (response.hasHeader ("Transfer-Encoding"))
            {
                this->setEncoding (join::rsplit (response.header ("Transfer-Encoding"), ","));
            }
            if (response.hasHeader ("Content-Encoding"))
            {
                this->setEncoding (join::rsplit (response.header ("Content-Encoding"), ","));
            }

            // get timestamp.
            this->_timestamp = std::chrono::steady_clock::now ();

            return 0;
        }

    protected:
        /**
         * @brief set stream encoding.
         * @param encodings encodings applied to the stream.
         */
        void setEncoding (const std::vector <std::string>& encodings)
        {
            for (auto const& encoding : encodings)
            {
                if (encoding.find ("gzip") != std::string::npos)
                {
                    this->_streambuf = new Zstreambuf (this->_streambuf, Zstream::Gzip, this->_wrapped);
                    this->_wrapped = true;
                }
                else if (encoding.find ("deflate") != std::string::npos)
                {
                    this->_streambuf = new Zstreambuf (this->_streambuf, Zstream::Deflate, this->_wrapped);
                    this->_wrapped = true;
                }
                else if (encoding.find ("chunked") != std::string::npos)
                {
                    this->_streambuf = new Chunkstreambuf (this->_streambuf, this->_wrapped);
                    this->_wrapped = true;
                }
            }

            this->set_rdbuf (this->_streambuf);
        }

        /**
         * @brief clear stream encoding.
         */
        void clearEncoding ()
        {
            if (this->_wrapped && this->_streambuf)
            {
                delete this->_streambuf;
                this->_streambuf = nullptr;
            }

            this->_streambuf = &this->_sockbuf;
            this->_wrapped = false;

            this->set_rdbuf (this->_streambuf);
        }

        /**
         * @brief check if HTTP keep alive is expired.
         * @return true if HTTP keep alive is expired.
         */
        bool expired () const
        {
            auto duration = std::chrono::steady_clock::now () - this->_timestamp;
            return (this->_keepTimeout < std::chrono::duration_cast <std::chrono::seconds> (duration)) || (this->_keepMax == 0);
        }

        /**
         * @brief check if client must reconnect.
         * @return true if reconnection is required.
         */
        bool needReconnection ()
        {
            return !this->connected () || this->expired ();
        }

        /**
         * @brief perform reconnection to the given endpoint.
         * @param endpoint endpoint to connect to.
         */
        virtual void reconnect (const Endpoint& endpoint)
        {
            this->disconnect ();
            this->close ();
            this->connect (endpoint);
        }

        /// HTTP stream buffer.
        std::streambuf* _streambuf = nullptr;

        /// HTTP stream status.
        bool _wrapped = false;

        /// HTTP timestamp.
        std::chrono::time_point <std::chrono::steady_clock> _timestamp;

        /// HTTP host.
        std::string _host;

        /// HTTP port.
        uint16_t _port;

        /// HTTP keep alive.
        bool _keep;

        /// HTTP keep alive timeout.
        std::chrono::seconds _keepTimeout;

        /// HTTP keep alive max.
        int _keepMax = -1;
    };

    /**
     * @brief write HTTP request to the HTTP stream.
     * @param out HTTP output stream.
     * @param request HTTP request.
     * @return a reference to the HTTP output stream.
     */
    template <class Protocol>
    constexpr BasicHttpClient <Protocol>& operator<< (BasicHttpClient <Protocol>& out, HttpRequest& request)
    {
        out.send (request);
        return out;
    }

    /**
     * @brief read HTTP response from the HTTP stream.
     * @param in HTTP input stream.
     * @param response HTTP response.
     * @return a reference to the HTTP input stream.
     */
    template <class Protocol>
    constexpr BasicHttpClient <Protocol>& operator>> (BasicHttpClient <Protocol>& in, HttpResponse& response)
    {
        in.receive (response);
        return in;
    }

    /**
     * @brief basic HTTPS client.
     */
    template <class Protocol> 
    class BasicHttpSecureClient : public BasicHttpClient <Protocol>
    {
    public:
        using Endpoint = typename Protocol::Endpoint;

        /**
         * @brief create the basic HTTPS client instance.
         * @param host host.
         * @param port port (default: 443).
         * @param keepAlive use a persistent connection.
         */
        BasicHttpSecureClient (const char* host, uint16_t port = 443, bool keepAlive = true)
        : BasicHttpClient <Protocol> (host, port, keepAlive)
        {
        }

        /**
         * @brief create the basic HTTPS client instance.
         * @param host host.
         * @param port port (default: 443).
         * @param keepAlive use a persistent connection.
         */
        BasicHttpSecureClient (const std::string& host, uint16_t port = 443, bool keepAlive = true)
        : BasicHttpSecureClient (host.c_str (), port, keepAlive)
        {
        }

        /**
         * @brief create the basic HTTPS client instance by copy.
         * @param other request to copy.
         */
        BasicHttpSecureClient (const BasicHttpSecureClient& other) = delete;

        /**
         * @brief assign the basic HTTPS client instance by copy.
         * @param other request to copy.
         * @return a reference of the current object.
         */
        BasicHttpSecureClient& operator= (const BasicHttpSecureClient& other) = delete;

        /**
         * @brief create the basic HTTPS client instance by move.
         * @param other request to move.
         */
        BasicHttpSecureClient (BasicHttpSecureClient&& other)
        : BasicHttpClient <Protocol> (std::move (other))
        {
        }

        /**
         * @brief assign the basic HTTPS client instance by move.
         * @param other request to move.
         * @return a reference of the current object.
         */
        BasicHttpSecureClient& operator= (BasicHttpSecureClient&& other)
        {
            BasicHttpClient <Protocol>::operator= (std::move (other));
            return *this;
        }

        /**
         * @brief destroy the basic HTTPS client instance.
         */
        virtual ~BasicHttpSecureClient () = default;

        /**
         * @brief get HTTP scheme.
         * @return htpp or https.
         */
        std::string scheme () const override
        {
            return "https";
        }

    protected:
        /**
         * @brief perform reconnection to the given endpoint.
         * @param endpoint endpoint to connect to.
         */
        void reconnect (const Endpoint& endpoint) override
        {
            this->disconnect ();
            this->close ();
            this->connectEncrypted (endpoint);
        }
    };

    /**
     * @brief write request to the HTTPS stream.
     * @param out HTTP output stream.
     * @param request request.
     * @return a reference to the HTTPS output stream.
     */
    template <class Protocol>
    constexpr BasicHttpSecureClient <Protocol>& operator<< (BasicHttpSecureClient <Protocol>& out, HttpRequest& request)
    {
        out.send (request);
        return out;
    }

    /**
     * @brief read response from the HTTPS stream.
     * @param in HTTPS input stream.
     * @param response response.
     * @return a reference to the HTTPS input stream.
     */
    template <class Protocol>
    constexpr BasicHttpSecureClient <Protocol>& operator>> (BasicHttpSecureClient <Protocol>& in, HttpResponse& response)
    {
        in.receive (response);
        return in;
    }
}

#endif
