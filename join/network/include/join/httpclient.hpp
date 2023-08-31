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
#include <join/httpmessage.hpp>
#include <join/socketstream.hpp>

// C++.
#include <chrono>

namespace join
{
    /**
     * @brief Basic HTTP client.
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
        virtual ~BasicHttpClient () = default;

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

            if (this->port () && (this->port () != Resolver::resolveService (this->scheme ())))
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
         */
        void send (const HttpRequest& request)
        {
            if (this->needReconnection ())
            {
                this->reconnect (this->url ());
                if (this->fail ())
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
                tmp.header ("Connection", this->_keep ? "keep-alive" : "close");
            }
            if (!tmp.hasHeader ("Host"))
            {
                tmp.header ("Host", this->authority ());
            }
            if (!tmp.hasHeader ("User-Agent"))
            {
                tmp.header ("User-Agent", "join/" JOIN_VERSION);
            }

            tmp.writeHeaders (*this);
            if (this->fail ())
            {
                return;
            }

            this->flush ();
        }

        /**
         * @brief receive HTTP response.
         * @param response HTTP response received.
         */
        void receive (HttpResponse& response)
        {
            response.readHeaders (*this);
            if (this->fail ())
            {
                return;
            }

            std::string connection = response.header ("Connection");
            std::string alive = response.header ("Keep-Alive");

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

            this->_timestamp = std::chrono::steady_clock::now ();
        }

    protected:
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
            this->close ();
            this->connect (endpoint);
        }

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
    constexpr BasicHttpClient <Protocol>& operator<< (BasicHttpClient <Protocol>& out, const HttpRequest& request)
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
     * @brief Basic HTTPS client.
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
    constexpr BasicHttpSecureClient <Protocol>& operator<< (BasicHttpSecureClient <Protocol>& out, const HttpRequest& request)
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
