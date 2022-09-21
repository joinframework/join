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
#include <join/utils.hpp>

// C++.
#include <iostream>
#include <string>
#include <map>

namespace join
{
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
         * @brief set path.
         * @param p path.
         */
        void path (const std::string& p);

        /**
         * @brief get path.
         * @return path.
         */
        const std::string& path () const;

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

        /// HTTP path;
        std::string _path;

        /// HTTP version.
        std::string _version;

        /// HTTP headers.
        std::map <std::string, std::string, details::lessNoCase> _headers;
    };
}

#endif
