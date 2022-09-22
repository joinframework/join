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

#ifndef __JOIN_HTTP_REQUEST_HPP__
#define __JOIN_HTTP_REQUEST_HPP__

// libjoin.
#include <join/httpmessage.hpp>

// C++.
#include <map>

namespace join
{
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
     * @brief HTTP request.
     */
    class HttpRequest : public HttpMessage
    {
    public:
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
         * @brief set request method (default GET).
         * @param meth HTTP method.
         */
        void method (HttpMethod meth);

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
         * @brief add query parameters to the HTTP request.
         * @param name parameter name.
         * @param val parameter value.
         * @return true successfully added.
         */
        void parameter (const std::string& name, const std::string& val);

        /**
         * @brief get a parameter by name.
         * @param name parameter name.
         * @return parameter value.
         */
        std::string parameter (const std::string& name) const;

        /**
         * @brief checks if there is a parameter with the specified name.
         * @param name name of the parameter to search for.
         * @return true of there is such a parameter, false otherwise.
         */
        bool hasParameter (const std::string& name) const;

        /**
         * @brief get query.
         * @return query.
         */
        std::string query () const;

        /**
         * @brief get URI.
         * @return URI.
         */
        std::string uri () const;

        /**
         * @brief send request to the given output stream.
         * @param out output stream.
         */
        virtual void send (std::ostream& out) const override;

        /**
         * @brief receive request from the given input stream.
         * @param in input stream.
         */
        virtual void receive (std::istream& in) override;

    protected:
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
        void storeParameters (const std::string &query);

        /// HTTP method.
        HttpMethod _method = Get;

        /// HTTP path;
        std::string _path;

        /// HTTP query parameters.
        std::map <std::string, std::string> _parameters;
    };
}

#endif
