/**
 * MIT License
 *
 * Copyright (c) 2021 Mathieu Rabine
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

#ifndef __JOIN_RESOLVER_HPP__
#define __JOIN_RESOLVER_HPP__

// libjoin.
#include <join/macaddress.hpp>

namespace join
{
    /**
     * @brief basic domain name resolution class.
     */
    class Resolver
    {
    public:
        /**
         * @brief default constructor.
         */
        Resolver () = delete;

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        Resolver (const Resolver& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to copy.
         * @return a reference to the current object.
         */
        Resolver& operator= (const Resolver& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        Resolver (Resolver&& other) = delete;

        /**
         * @brief move assignment operator.
         * @param other other object to move.
         * @return a reference to the current object.
         */
        Resolver& operator= (Resolver&& other) = delete;

        /**
         * @brief destroy instance.
         */
        virtual ~Resolver () = delete;

        /**
         * @brief get IP address of the currently configured name servers.
         * @return a list of configured name servers.
         */
        static IpAddressList nameServers ();

        /**
         * @brief resolve host name using system resolver.
         * @param host host name to resolve.
         * @return the first resolved IP address found.
         */
        static IpAddress resolveHost (const std::string& host);

        /**
         * @brief resolve host name using system resolver and matching address family.
         * @param host host name to resolve.
         * @param family Address family.
         * @return the first resolved IP address found that match address family.
         */
        static IpAddress resolveHost (const std::string& host, int family);

        /**
         * @brief resolve host name using system resolver and return all IP address found.
         * @param host host name to resolve.
         * @return the resolved IP address list.
         */
        static IpAddressList resolveAllHost (const std::string& host);

        /**
         * @brief resolve host address using system resolver.
         * @param address host address to resolve.
         * @return the first resolved alias.
         */
        static std::string resolveAddress (const IpAddress& address);

        /**
         * @brief resolve service name using system resolver.
         * @param service service name to resolve (ex. "http", "ftp", "ssh" etc...).
         * @return the port resolved.
         */
        static uint16_t resolveService (const std::string& service);
    };
}

#endif
