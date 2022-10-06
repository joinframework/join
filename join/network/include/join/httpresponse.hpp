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

#ifndef __JOIN_HTTP_RESPONSE_HPP__
#define __JOIN_HTTP_RESPONSE_HPP__

// libjoin.
#include <join/httpmessage.hpp>

// C++.
#include <map>

namespace join
{
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
         * @brief send response to the given output stream.
         * @param out output stream.
         */
        virtual void send (std::ostream& out) const override;

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

#endif
