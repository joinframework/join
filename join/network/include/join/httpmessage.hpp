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

#ifndef __JOIN_HTTP_MESSAGE_HPP__
#define __JOIN_HTTP_MESSAGE_HPP__

// libjoin.
#include <join/error.hpp>
#include <join/utils.hpp>

// C++.
#include <system_error>
#include <iostream>
#include <sstream>
#include <string>
#include <map>

namespace join
{
    /**
     * @brief HTTP API generic error codes.
     */
    enum class HttpErrc
    {
        BadRequest = 1,     /**< malformed request syntax. */
        Unauthorized,       /**< authentication is required. */
        Forbidden,          /**< missing required permissions. */
        NotFound,           /**< resource could not be found. */
        Unsupported,        /**< method is not supported. */
        LengthRequired,     /**< length was not specified. */
        PayloadTooLarge,    /**< request payload is too large. */
        UriTooLong,         /**< request URI is too long. */
        HeaderTooLarge,     /**< request header is too large. */
        ServerError,        /**< generic error. */
        NotImplemented,     /**< not implemented. */
        BadGateway,         /**< invalid response from the upstream server. */
    };

    /**
     * @brief HTTP API generic error category.
     */
    class HttpCategory : public std::error_category
    {
    public:
        /**
         * @brief get HTTP API generic error category name.
         * @return deserializer generic error category name.
         */
        virtual const char* name () const noexcept;

        /**
         * @brief translate HTTP API generic error code to human readable error string.
         * @param code error code.
         * @return human readable error string.
         */
        virtual std::string message (int code) const;

        /**
         * @brief find equivalent from Errc to system error code.
         * @param code System error code.
         * @param condition Errc.
         * @return true if equivalent, false otherwise.
         */
        virtual bool equivalent (const std::error_code& code, int condition) const noexcept;
    };

    /**
     * @brief get error category.
     * @return the created std::error_category object.
     */
    const std::error_category& httpCategory ();

    /**
     * @brief create an std::error_code object.
     * @param code error code number.
     * @return the created std::error_code object.
     */
    std::error_code make_error_code (HttpErrc code);

    /**
     * @brief create an std::error_condition object.
     * @param code error code number.
     * @return the created std::error_condition object.
     */
    std::error_condition make_error_condition (HttpErrc code);

    /**
     * @brief enumeration of HTTP methods.
     */
    enum HttpMethod
    {
        Head    = 1L << 0,  /**< retrieve informations identified by the Request-URI without message-body. */
        Get     = 1L << 1,  /**< retrieve informations identified by the Request-URI. */
        Put     = 1L << 2,  /**< request that the enclosed entity be stored under the supplied Request-URI. */
        Post    = 1L << 3,  /**< request that the enclosed entity is accepted as a new subordinate of the resource identified by the Request-URI. */
        Delete  = 1L << 4,  /**< request that the server delete the resource identified by the Request-URI. */
    };

    /**
     * @brief perform binary AND on HttpMethod.
     * @param __a bitset.
     * @param __b other bitset.
     * @return bitset result of binary AND on HttpMethod.
     */
    __inline__ HttpMethod operator& (HttpMethod __a, HttpMethod __b)
    { return HttpMethod (static_cast <int> (__a) & static_cast <int> (__b)); }

    /**
     * @brief perform binary OR on HttpMethod.
     * @param __a bitset.
     * @param __b other bitset.
     * @return bitset result of binary OR on HttpMethod.
     */
    __inline__ HttpMethod operator| (HttpMethod __a, HttpMethod __b)
    { return HttpMethod (static_cast <int> (__a) | static_cast <int> (__b)); }

    /**
     * @brief perform binary XOR on HttpMethod.
     * @param __a bitset.
     * @param __b other bitset.
     * @return bitset result of binary XOR on HttpMethod.
     */
    __inline__ HttpMethod operator^ (HttpMethod __a, HttpMethod __b)
    { return HttpMethod (static_cast <int> (__a) ^ static_cast <int> (__b)); }

    /**
     * @brief perform binary NOT on HttpMethod.
     * @param __a bitset.
     * @return bitset result of binary NOT on HttpMethod.
     */
    __inline__ HttpMethod operator~ (HttpMethod __a)
    { return HttpMethod (~static_cast <int> (__a)); }

    /**
     * @brief perform binary AND on HttpMethod.
     * @param __a bitset.
     * @param __b other bitset.
     * @return bitset result of binary AND on HttpMethod.
     */
    __inline__ const HttpMethod& operator&= (HttpMethod& __a, HttpMethod __b)
    { return __a = __a & __b; }

    /**
     * @brief perform binary OR on HttpMethod.
     * @param __a bitset.
     * @param __b other bitset.
     * @return bitset result of binary OR.
     */
    __inline__ const HttpMethod& operator|= (HttpMethod& __a, HttpMethod __b)
    { return __a = __a | __b; }

    /**
     * @brief perform binary XOR on HttpMethod.
     * @param __a bitset.
     * @param __b other bitset.
     * @return bitset result of binary XOR on HttpMethod.
     */
    __inline__ const HttpMethod& operator^= (HttpMethod& __a, HttpMethod __b)
    { return __a = __a ^ __b; }

    /**
     * @brief HTTP message.
     */
    class HttpMessage
    {
    public:
        // headers map.
        using HeaderMap = std::map <std::string, std::string, details::lessNoCase>;

        /**
         * @brief create the HttpMessage instance.
         */
        HttpMessage ();

        /**
         * @brief create the HttpMessage instance by copy.
         * @param other request to copy.
         */
        HttpMessage (const HttpMessage& other);

        /**
         * @brief assign the HttpMessage instance by copy.
         * @param other request to copy.
         * @return a reference of the current object.
         */
        HttpMessage& operator= (const HttpMessage& other);

        /**
         * @brief create the HttpMessage instance by move.
         * @param other request to move.
         */
        HttpMessage (HttpMessage&& other);

        /**
         * @brief assign the HttpMessage instance by move.
         * @param other request to move.
         * @return a reference of the current object.
         */
        HttpMessage& operator= (HttpMessage&& other);

        /**
         * @brief destroy the HttpMessage instance.
         */
        virtual ~HttpMessage () = default;

        /**
         * @brief get HTTP version.
         * @return HTTP version.
         */
        const std::string& version () const;

        /**
         * @brief set HTTP version (default "HTTP/1.1").
         * @param v HTTP version.
         */
        void version (const std::string& v);

        /**
         * @brief checks if there is a header with the specified name.
         * @param name name of the header to search for.
         * @return true of there is such a header, false otherwise.
         */
        bool hasHeader (const std::string& name) const;

        /**
         * @brief get header by name.
         * @param name header name.
         * @return header value.
         */
        std::string header (const std::string& name) const;

        /**
         * @brief add header to the HTTP request.
         * @param name header name.
         * @param val header value.
         */
        void header (const std::string& name, const std::string& val);

        /**
         * @brief add header to the HTTP request.
         * @param h header.
         */
        void header (const HeaderMap::value_type& h);

        /**
         * @brief get headers map.
         * @return headers map.
         */
        const HeaderMap& headers () const;

        /**
         * @brief add headers to the HTTP request.
         * @param heads headers.
         */
        void headers (const HeaderMap& heads);

        /**
         * @brief dump headers.
         * @return headers.
         */
        std::string dumpHeaders () const;

        /**
         * @brief get content length.
         * @return content length.
         */
        size_t contentLength () const;

        /**
         * @brief clear HTTP message.
         */
        virtual void clear ();

        /**
         * @brief read HTTP header from the given input stream.
         * @param in input stream.
         * @return 0 on success, -1 on failure.
         */
        virtual int readHeaders (std::istream& in);

        /**
         * @brief write HTTP header to the given output stream.
         * @param out output stream.
         * @return 0 on success, -1 on failure.
         */
        virtual int writeHeaders (std::ostream& out) const = 0;

    protected:
        /**
         * @brief parse first line.
         * @param line first line to parse.
         * @return 0 on success, -1 on failure.
         */
        virtual int parseFirstLine (const std::string& line) = 0;

        /**
         * @brief parse HTTP header.
         * @param head HTTP header to parse.
         * @return 0 on success, -1 on failure.
         */
        virtual int parseHeader (const std::string& head);

        /// HTTP max header size.
        static const std::streamsize _maxHeaderLen = 2048;

        /// HTTP version.
        std::string _version;

        /// HTTP headers.
        HeaderMap _headers;
    };

    /**
     * @brief HTTP request.
     */
    class HttpRequest : public HttpMessage
    {
    public:
        // parameters map.
        using ParameterMap = std::map <std::string, std::string>;

        /**
         * @brief create the HttpRequest instance by default.
         */
        HttpRequest ();

        /**
         * @brief create the HttpRequest instance by default.
         * @param method request method.
         */
        HttpRequest (HttpMethod method);

        /**
         * @brief create the HttpRequest instance by copy.
         * @param other request to copy.
         */
        HttpRequest (const HttpRequest& other);

        /**
         * @brief assign the HttpRequest instance by copy.
         * @param other request to copy.
         * @return a reference of the current object.
         */
        HttpRequest& operator= (const HttpRequest& other);

        /**
         * @brief create the HttpRequest instance by move.
         * @param other request to move.
         */
        HttpRequest (HttpRequest&& other);

        /**
         * @brief assign the HttpRequest instance by move.
         * @param other request to move.
         * @return a reference of the current object.
         */
        HttpRequest& operator= (HttpRequest&& other);

        /**
         * @brief destroy the HttpRequest instance.
         */
        virtual ~HttpRequest () = default;

        /**
         * @brief get request method.
         * @return request method.
         */
        HttpMethod method () const;

        /**
         * @brief get request method string.
         * @return request method string.
         */
        std::string methodString () const;

        /**
         * @brief set request method (default GET).
         * @param meth HTTP method.
         */
        void method (HttpMethod meth);

        /**
         * @brief get path.
         * @return path.
         */
        const std::string& path () const;

        /**
         * @brief set path.
         * @param p path.
         */
        void path (const std::string& p);

        /**
         * @brief checks if there is a parameter with the specified name.
         * @param name name of the parameter to search for.
         * @return true of there is such a parameter, false otherwise.
         */
        bool hasParameter (const std::string& name) const;

        /**
         * @brief get a parameter by name.
         * @param name parameter name.
         * @return parameter value.
         */
        std::string parameter (const std::string& name) const;

        /**
         * @brief add query parameter to the HTTP request.
         * @param name parameter name.
         * @param val parameter value.
         */
        void parameter (const std::string& name, const std::string& val);

        /**
         * @brief add query parameter to the HTTP request.
         * @param param parameter.
         */
        void parameter (const ParameterMap::value_type& param);

        /**
         * @brief get query parameters map.
         * @return query parameters map.
         */
        const ParameterMap& parameters () const;

        /**
         * @brief add query parameters to the HTTP request.
         * @param params parameters.
         */
        void parameters (const ParameterMap& params);

        /**
         * @brief dump parameters.
         * @return parameters.
         */
        std::string dumpParameters () const;

        /**
         * @brief get query.
         * @return query.
         */
        std::string query () const;

        /**
         * @brief get URN.
         * @return URN.
         */
        std::string urn () const;

        /**
         * @brief get host.
         * @return host.
         */
        std::string host () const;

        /**
         * @brief get autorization type.
         * @return autorization type.
         */
        std::string auth () const;

        /**
         * @brief get credentials.
         * @return credentials.
         */
        std::string credentials () const;

        /**
         * @brief clear HTTP message.
         */
        virtual void clear () override;

        /**
         * @brief write HTTP header to the given output stream.
         * @param out output stream.
         * @return 0 on success, -1 on failure.
         */
        virtual int writeHeaders (std::ostream& out) const override;

    protected:
        /**
         * @brief parse first line.
         * @param line first line to parse.
         * @return 0 on success, -1 on failure.
         */
        virtual int parseFirstLine (const std::string& line) override;

        /**
         * @brief decode url (ex. %20 ==> ' ').
         * @param url url to decode.
         * @return a reference to the url object.
         */
        std::string& decodeUrl (std::string &url);

        /**
         * @brief produce a normalized path (collapse duplicated separator and remove dot segment).
         * @param path path to normalize.
         * @return normalized path.
         */
        std::string& normalize (std::string& path);

        /**
         * @brief store parameters received in request.
         * @param query parameters received in request.
         */
        void store (const std::string &query);

        /// HTTP method.
        HttpMethod _method = Get;

        /// HTTP path;
        std::string _path;

        /// HTTP query parameters.
        ParameterMap _parameters;
    };

    /**
     * @brief HTTP response.
     */
    class HttpResponse : public HttpMessage
    {
    public:
        /**
         * @brief create the HttpResponse instance.
         */
        HttpResponse () = default;

        /**
         * @brief create the HttpResponse instance by copy.
         * @param other request to copy.
         */
        HttpResponse (const HttpResponse& other);

        /**
         * @brief assign the HttpResponse instance by copy.
         * @param other request to copy.
         * @return a reference of the current object.
         */
        HttpResponse& operator= (const HttpResponse& other);

        /**
         * @brief create the HttpResponse instance by move.
         * @param other request to move.
         */
        HttpResponse (HttpResponse&& other);

        /**
         * @brief assign the HttpResponse instance by move.
         * @param other request to move.
         * @return a reference of the current object.
         */
        HttpResponse& operator= (HttpResponse&& other);

        /**
         * @brief destroy the HttpResponse instance.
         */
        virtual ~HttpResponse () = default;

        /**
         * @brief get HTTP response status.
         * @return HTTP response status.
         */
        const std::string& status () const;

        /**
         * @brief get HTTP response reason.
         * @return HTTP response reason.
         */
        const std::string& reason () const;

        /**
         * @brief set HTTP response status.
         * @param status HTTP status.
         * @param reason HTTP reason.
         */
        void response (const std::string& status, const std::string& reason = {});

        /**
         * @brief clear HTTP message.
         */
        virtual void clear () override;

        /**
         * @brief write HTTP header to the given output stream.
         * @param out output stream.
         * @return 0 on success, -1 on failure.
         */
        virtual int writeHeaders (std::ostream& out) const override;

    protected:
        /**
         * @brief parse first line.
         * @param line first line to parse.
         * @return 0 on success, -1 on failure.
         */
        virtual int parseFirstLine (const std::string& line) override;

        /// HTTP status.
        std::string _status;

        /// HTTP reason.
        std::string _reason;
    };
}

namespace std
{
    /// HTTP API generic error code specialization.
    template <> struct is_error_condition_enum <join::HttpErrc> : public true_type {};
}

#endif
