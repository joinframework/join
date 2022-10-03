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

// libjoin.
#include <join/resolver.hpp>

// C++.
#include <string>
#include <vector>
#include <regex>

// C.
#include <resolv.h>
#include <netdb.h>

using join::IpAddress;
using join::IpAddressList;
using join::Resolver;

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : nameServers
// =========================================================================
IpAddressList Resolver::nameServers ()
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

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : resolveHost
// =========================================================================
IpAddress Resolver::resolveHost (const std::string& host)
{
    for (auto const& address : resolveAllHost (host))
    {
        return address;
    }

    return IpAddress ();
}

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : resolveHost
// =========================================================================
IpAddress Resolver::resolveHost (const std::string& host, int family)
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

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : resolveAllHost
// =========================================================================
IpAddressList Resolver::resolveAllHost (const std::string& host)
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

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : resolveAddress
// =========================================================================
std::string Resolver::resolveAddress (const IpAddress& address)
{
    struct sockaddr_storage addr;
    memset (&addr, 0, sizeof (struct sockaddr_storage));

    if (address.family () == AF_INET6)
    {
        struct sockaddr_in6* sa = reinterpret_cast <struct sockaddr_in6*> (&addr);
        sa->sin6_family         = address.family ();
        sa->sin6_scope_id       = address.scope ();
        memcpy (&sa->sin6_addr, address.addr (), address.length ());
    }
    else
    {
        struct sockaddr_in* sa  = reinterpret_cast <struct sockaddr_in*> (&addr);
        sa->sin_family          = address.family ();
        memcpy (&sa->sin_addr, address.addr (), address.length ());
    }

    char host[NI_MAXHOST] = "";
    int result = -1, trials = 0;

    for (;;)
    {
        result = getnameinfo (reinterpret_cast <sockaddr*> (&addr), sizeof (struct sockaddr_storage), host, sizeof (host), nullptr, 0, NI_NAMEREQD);

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

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : resolveService
// =========================================================================
uint16_t Resolver::resolveService (const std::string& service)
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
