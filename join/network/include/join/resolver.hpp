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
#include <join/endpoint.hpp>

// C++.
#include <string>
#include <vector>
#include <regex>

// C.
#include <resolv.h>
#include <netdb.h>

namespace join
{
    /**
     * @brief basic domain name resolution class.
     */
    template <class Protocol>
    class BasicResolver
    {
    public:
        using Endpoint     = typename Protocol::Endpoint;
        using EndpointList = std::vector <Endpoint>;

        /**
         * @brief default constructor.
         */
        BasicResolver () = delete;

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicResolver (const BasicResolver& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to copy.
         * @return a reference to the current object.
         */
        BasicResolver& operator= (const BasicResolver& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        BasicResolver (BasicResolver&& other) = delete;

        /**
         * @brief move assignment operator.
         * @param other other object to move.
         * @return a reference to the current object.
         */
        BasicResolver& operator= (BasicResolver&& other) = delete;

        /**
         * @brief destroy instance.
         */
        virtual ~BasicResolver () = delete;

        /**
         * @brief get IP address of the currently configured name servers.
         * @return a list of configured name servers.
         */
        static IpAddressList nameServers ()
        {
            IpAddressList addressList;

            struct __res_state res;
            if (res_ninit (&res) == 0)
            {
                for (int i = 0; i < res.nscount; ++i)
                {
                    if (res.nsaddr_list[i].sin_family == AF_INET)
                    {
                        addressList.emplace_back (&res.nsaddr_list[i].sin_addr, sizeof (struct in_addr));
                    }
                    else if (res._u._ext.nsaddrs[i] != nullptr && res._u._ext.nsaddrs[i]->sin6_family == AF_INET6)
                    {
                        addressList.emplace_back (&res._u._ext.nsaddrs[i]->sin6_addr, sizeof (struct in6_addr));
                    }
                }
                res_nclose (&res);
            }

            return addressList;
        }

        /**
         * @brief resolve URL using system resolver.
         * @param url URL to resolve.
         * @return the first resolved endpoint found.
         */
        static Endpoint resolve (const std::string& url)
        {
            for (auto const& endpoint : resolveAll (url))
            {
                return endpoint;
            }

            return Endpoint ();
        }

        /**
         * @brief resolve URL using system resolver and matching address family.
         * @param url URL to resolve.
         * @param family Address family.
         * @return the first resolved endpoint found that match address family.
         */
        static Endpoint resolve (const std::string& url, int family)
        {
            for (auto const& endpoint : resolveAll (url))
            {
                if (endpoint.ip ().family () == family)
                {
                    return endpoint;
                }
            }

            return Endpoint (IpAddress (family), 0);
        }

        /**
         * @brief resolve URL using system resolver and return all endpoints found.
         * @param url URL to resolve.
         * @return the resolved endpoint list.
         */
        static EndpointList resolveAll (const std::string& url)
        {
            // regular expression inspired by rfc3986 (see https://www.ietf.org/rfc/rfc3986.txt)
            // ex.
            // 0: https://example.com:8080/foo/bar.html?val=1#frag  # URL
            // 1: https                                             # Scheme
            // 2: example.com                                       # Host
            // 3: 8080                                              # Port
            // 4: /foo/bar.html                                     # Path
            // 5: val=1                                             # Query
            // 6: frag                                              # Fragment
            //                0    1              2                                    3         4            5            6
            std::regex reg (R"(^(?:([^:/?#]+)://)?([a-z0-9\-._~%]+|\[[a-f0-9:.]+\])(?::([0-9]+))?([^?#]*)(?:\?([^#]*))?(?:#(.*))?)");
            std::cmatch match;

            EndpointList endpoints;
            std::string host (url);
            uint16_t port = 0;

            if (std::regex_match (url.c_str (), match, reg))
            {
                host = match[2];
                host.erase (0, host.find_first_not_of ("["));
                host.erase (host.find_last_not_of ("]") + 1);
                port = (match[3].length ()) ? uint16_t (std::stoi (match[3])) : resolveService (match[1]);
            }

            for (auto const& ip : resolveAllHost (host))
            {
                endpoints.emplace_back (ip, port);
            }

            return endpoints;
        }

        /**
         * @brief resolve host name using system resolver.
         * @param host host name to resolve.
         * @return the first resolved IP address found.
         */
        static IpAddress resolveHost (const std::string& host)
        {
            for (auto const& address : resolveAllHost (host))
            {
                return address;
            }

            return IpAddress ();
        }

        /**
         * @brief resolve host name using system resolver and matching address family.
         * @param host host name to resolve.
         * @param family Address family.
         * @return the first resolved IP address found that match address family.
         */
        static IpAddress resolveHost (const std::string& host, int family)
        {
            for (auto const& address : resolveAllHost (host))
            {
                if (address.family () == family)
                {
                    return address;
                }
            }

            return IpAddress (family);
        }

        /**
         * @brief resolve host name using system resolver and return all IP address found.
         * @param host host name to resolve.
         * @return the resolved IP address list.
         */
        static IpAddressList resolveAllHost (const std::string& host)
        {
            struct addrinfo* info = nullptr;
            int result = -1, trials = 0;

            for (;;)
            {
                // convert domain name system (DNS) host names and IP addresses to structured binary formats.
                result = getaddrinfo (host.c_str (), nullptr, nullptr, &info);

                if (result && info)
                    freeaddrinfo (info);

                if (result != EAI_AGAIN || ++trials > 1)
                    break;

                res_init ();
            }

            if (result != 0)
            {
                return {};
            }

            IpAddressList addressList;

            for (struct addrinfo *rp = info; rp != nullptr; rp = rp->ai_next)
            {
                addressList.emplace_back (*rp->ai_addr);
            }
            freeaddrinfo (info);

            return addressList;
        }

        /**
         * @brief resolve host address using system resolver.
         * @param address host address to resolve.
         * @return the first resolved alias.
         */
        static std::string resolveAddress (const IpAddress& address)
        {
            Endpoint endpoint (address);
            char host[NI_MAXHOST] = "";
            int result = -1, trials = 0;

            for (;;)
            {
                result = getnameinfo (endpoint.addr (), endpoint.length (), host, sizeof (host), nullptr, 0, NI_NAMEREQD);

                if (result != EAI_AGAIN || ++trials > 1)
                    break;

                res_init ();
            }

            if (result != 0)
            {
                return address.toIpv4 ().toString ();
            }

            return host;
        }

        /**
         * @brief resolve service name using system resolver.
         * @param service service name to resolve (ex. "http", "ftp", "ssh" etc...).
         * @return the port resolved.
         */
        static uint16_t resolveService (const std::string& service)
        {
            struct servent entry, *res;
            char buffer[1024];

            // try to resolve service name.
            int status = getservbyname_r (service.c_str (), nullptr, &entry, buffer, sizeof buffer, &res);
            if ((status == 0) && (res != nullptr))
            {
                return ntohs (entry.s_port);
            }

            return 0;
        }
    };
}

#endif
