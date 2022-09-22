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
#include <string>
#include <map>

namespace join
{
    /**
     * @brief HTTP API generic error codes.
     */
    enum class HttpErrc
    {
        InvalidRequest = 1,     /**< invalid HTTP request. */
        InvalidResponse,        /**< invalid HTTP response. */
        InvalidMethod,          /**< invalid HTTP method. */
        InvalidHeader           /**< invalid HTTP header. */
    };

    /**
     * @brief HTTP API generic error category.
     */
    class HttpCategory : public std::error_category
    {
    public:
        /**
         * @brief get SAX API generic error category name.
         * @return deserializer generic error category name.
         */
        virtual const char* name () const noexcept;

        /**
         * @brief translate SAX API generic error code to human readable error string.
         * @param code error code.
         * @return human readable error string.
         */
        virtual std::string message (int code) const;
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
     * @brief HTTP message.
     */
    class HttpMessage
    {
    public:
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
         * @brief set request version (default HTTP/1.1).
         * @param v HTTP version.
         */
        void version (const std::string& v);

        /**
         * @brief get request method.
         * @return request version.
         */
        const std::string& version () const;

        /**
         * @brief add a header to the HTTP request.
         * @param name header name.
         * @param val header value.
         */
        void header (const std::string& name, const std::string& val);

        /**
         * @brief get a header by name.
         * @param name header name.
         * @return header value.
         */
        std::string header (const std::string& name) const;

        /**
         * @brief checks if there is a header with the specified name.
         * @param name name of the header to search for.
         * @return true of there is such a header, false otherwise.
         */
        bool hasHeader (const std::string& name) const;

        /**
         * @brief send to the given output stream.
         * @param out output stream.
         */
        virtual void send (std::ostream& out) const = 0;

        /**
         * @brief receive from the given input stream.
         * @param in input stream.
         */
        virtual void receive (std::istream& in) = 0;

    protected:
        /**
         * @brief read HTTP line (delimiter "\r\n").
         * @param line line read.
         * @param max max characters to read.
         * @return .
         */
        std::istream& getline (std::istream& in, std::string& line, uint32_t max);

        /// HTTP version.
        std::string _version;

        /// HTTP headers.
        std::map <std::string, std::string, details::lessNoCase> _headers;
    };
}

namespace std
{
    /// HTTP API generic error code specialization.
    template <> struct is_error_condition_enum <join::HttpErrc> : public true_type {};
}

#endif
