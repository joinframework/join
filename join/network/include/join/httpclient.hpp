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
#include <join/socketstream.hpp>
#include <join/httprequest.hpp>
#include <join/httpresponse.hpp>

namespace join
{
    /**
     * @brief HTTP client.
     */
    class HttpClient : public Tls::Stream
    {
    public:
        /**
         * @brief create the HttpClient instance.
         * @param endpoint endpoint.
         * @param encrypt use HTTTPS scheme.
         * @param keepAlive use a persistent connection.
         */
        HttpClient (const Tls::Endpoint& endpoint, bool encrypt = true, bool keepAlive = false);

        /**
         * @brief create the HttpClient instance.
         * @param host host.
         * @param port port.
         * @param encrypt use HTTTPS scheme.
         * @param keepAlive use a persistent connection.
         */
        HttpClient (const std::string& host, uint16_t port, bool encrypt = true, bool keepAlive = false);

        /**
         * @brief create the HttpClient instance by copy.
         * @param other request to copy.
         */
        HttpClient (const HttpClient& other) = delete;

        /**
         * @brief assign the HttpClient instance by copy.
         * @param other request to copy.
         * @return a reference of the current object.
         */
        HttpClient& operator= (const HttpClient& other) = delete;

        /**
         * @brief create the HttpClient instance by move.
         * @param other request to move.
         */
        HttpClient (HttpClient&& other);

        /**
         * @brief assign the HttpClient instance by move.
         * @param other request to move.
         * @return a reference of the current object.
         */
        HttpClient& operator= (HttpClient&& other);

        /**
         * @brief destroy the HttpClient instance.
         */
        virtual ~HttpClient () = default;

        /**
         * @brief get HTTP scheme.
         * @return htpp or https.
         */
        std::string scheme () const;

        /**
         * @brief get host.
         * @return host.
         */
        const std::string& host () const;

        /**
         * @brief get port.
         * @return port.
         */
        uint16_t port () const;

        /**
         * @brief enable/disable HTTP keep alive support.
         * @param k true if supported, false otherwise.
         */
        void keepAlive (bool k);

        /**
         * @brief checks if HTTP keep alive is supported.
         * @return true if supported, false otherwise.
         */
        bool keepAlive () const;

        /**
         * @brief send HTTP request.
         * @param request HTTP request to send.
         */
        void send (const HttpRequest& request);

        /**
         * @brief receive HTTP response.
         * @param response HTTP response received.
         */
        void receive (HttpResponse& response);

    protected:
        /// HTTP host.
        std::string _host;

        /// HTTP port.
        uint16_t _port = 443;

        /// HTTP encryption.
        bool _encrypt = true;

        /// HTTP keep alive.
        bool _keepAlive = false;
    };

    /**
     * @brief write HTTP request to the HTTP stream.
     * @param out HTTP output stream.
     * @param request HTTP request.
     * @return a reference to the HTTP output stream.
     */
    HttpClient& operator<< (HttpClient& out, const HttpRequest& request);

    /**
     * @brief read HTTP response from the HTTP stream.
     * @param in HTTP input stream.
     * @param response HTTP response.
     * @return a reference to the HTTP input stream.
     */
    HttpClient& operator>> (HttpClient& in, HttpResponse& response);
}

#endif
