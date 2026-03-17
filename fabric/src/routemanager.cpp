/**
 * MIT License
 *
 * Copyright (c) 2026 Mathieu Rabine
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
#include <join/routemanager.hpp>

// C.
#include <linux/rtnetlink.h>

using join::RouteManager;
using join::RouteList;
using join::Route;
using join::IpAddress;

// =========================================================================
//   CLASS     : RouteManager
//   METHOD    : RouteManager
// =========================================================================
RouteManager::RouteManager (Reactor* reactor)
: NetlinkManager (RTMGRP_IPV4_ROUTE | RTMGRP_IPV6_ROUTE, reactor)
{
    start ();
    refresh ();
}

// =========================================================================
//   CLASS     : RouteManager
//   METHOD    : ~RouteManager
// =========================================================================
RouteManager::~RouteManager ()
{
    stop ();
}

// =========================================================================
//   CLASS     : RouteManager
//   METHOD    : instance
// =========================================================================
RouteManager& RouteManager::instance ()
{
    static RouteManager manager;
    return manager;
}

// =========================================================================
//   CLASS     : RouteManager
//   METHOD    : findByIndex
// =========================================================================
Route::Ptr RouteManager::findByIndex (uint32_t index, const IpAddress& dest, uint32_t prefix)
{
    ScopedLock<Mutex> lock (_routeMutex);

    auto it = _routes.find ({index, dest, prefix});
    if (it == _routes.end ())
    {
        return nullptr;
    }

    return it->second;
}

// =========================================================================
//   CLASS     : RouteManager
//   METHOD    : findByName
// =========================================================================
Route::Ptr RouteManager::findByName (const std::string& interfaceName, const IpAddress& dest, uint32_t prefix)
{
    return findByIndex (if_nametoindex (interfaceName.c_str ()), dest, prefix);
}

// =========================================================================
//   CLASS     : RouteManager
//   METHOD    : enumerate
// =========================================================================
RouteList RouteManager::enumerate ()
{
    ScopedLock<Mutex> lock (_routeMutex);

    RouteList routes;
    routes.reserve (_routes.size ());

    for (auto& route : _routes)
    {
        routes.push_back (route.second);
    }

    return routes;
}

// =========================================================================
//   CLASS     : RouteManager
//   METHOD    : enumerate
// =========================================================================
RouteList RouteManager::enumerate (uint32_t index)
{
    ScopedLock<Mutex> lock (_routeMutex);

    RouteList routes;

    for (auto& route : _routes)
    {
        if (route.first._index == index)
        {
            routes.push_back (route.second);
        }
    }

    return routes;
}

// =========================================================================
//   CLASS     : RouteManager
//   METHOD    : enumerate
// =========================================================================
RouteList RouteManager::enumerate (const std::string& interfaceName)
{
    return enumerate (if_nametoindex (interfaceName.c_str ()));
}

// =========================================================================
//   CLASS     : RouteManager
//   METHOD    : refresh
// =========================================================================
int RouteManager::refresh ()
{
    {
        ScopedLock<Mutex> lock (_routeMutex);
        _routes.clear ();
    }

    return dumpRoutes (true);
}

// =========================================================================
//   CLASS     : RouteManager
//   METHOD    : addRouteListener
// =========================================================================
uint64_t RouteManager::addRouteListener (const RouteNotify& cb)
{
    uint64_t id = ++_listenerCounter;

    pushJob ([this, id, cb] () {
        _routeListeners.emplace (id, cb);
    });

    return id;
}

// =========================================================================
//   CLASS     : RouteManager
//   METHOD    : removeRouteListener
// =========================================================================
void RouteManager::removeRouteListener (uint64_t id)
{
    pushJob ([this, id] () {
        _routeListeners.erase (id);
    });
}

// =========================================================================
//   CLASS     : RouteManager
//   METHOD    : addRoute
// =========================================================================
int RouteManager::addRoute (uint32_t index, const IpAddress& dest, uint32_t prefix, const IpAddress& gateway,
                            uint32_t metric, bool sync)
{
    char buffer[_bufferSize] = {};

    // netlink header.
    struct nlmsghdr* nlh = reinterpret_cast<struct nlmsghdr*> (buffer);
    nlh->nlmsg_len       = NLMSG_LENGTH (sizeof (struct rtmsg));
    nlh->nlmsg_type      = RTM_NEWROUTE;
    nlh->nlmsg_flags     = NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE | NLM_F_EXCL;
    nlh->nlmsg_seq       = ++_seq;

    // routing message.
    struct rtmsg* rtm = reinterpret_cast<struct rtmsg*> (NLMSG_DATA (nlh));
    rtm->rtm_family   = dest.family ();
    rtm->rtm_dst_len  = prefix;
    rtm->rtm_table    = RT_TABLE_MAIN;
    rtm->rtm_protocol = RTPROT_STATIC;
    rtm->rtm_scope    = RT_SCOPE_UNIVERSE;
    rtm->rtm_type     = RTN_UNICAST;

    // add destination.
    addAttributes (nlh, RTA_DST, dest.addr (), dest.length ());

    // add gateway if specified.
    if (!gateway.isWildcard ())
    {
        addAttributes (nlh, RTA_GATEWAY, gateway.addr (), gateway.length ());
    }

    // add output interface.
    addAttributes (nlh, RTA_OIF, &index, sizeof (uint32_t));

    // add metric if specified.
    if (metric)
    {
        addAttributes (nlh, RTA_PRIORITY, &metric, sizeof (uint32_t));
    }

    return sendRequest (nlh, sync);
}

// =========================================================================
//   CLASS     : RouteManager
//   METHOD    : addRoute
// =========================================================================
int RouteManager::addRoute (const std::string& interfaceName, const IpAddress& dest, uint32_t prefix,
                            const IpAddress& gateway, uint32_t metric, bool sync)
{
    return addRoute (if_nametoindex (interfaceName.c_str ()), dest, prefix, gateway, metric, sync);
}

// =========================================================================
//   CLASS     : RouteManager
//   METHOD    : removeRoute
// =========================================================================
int RouteManager::removeRoute (uint32_t index, const IpAddress& dest, uint32_t prefix, bool sync)
{
    auto route = findByIndex (index, dest, prefix);
    if (!route)
    {
        lastError = make_error_code (Errc::NotFound);
        return -1;
    }

    char buffer[_bufferSize] = {};

    // netlink header.
    struct nlmsghdr* nlh = reinterpret_cast<struct nlmsghdr*> (buffer);
    nlh->nlmsg_len       = NLMSG_LENGTH (sizeof (struct rtmsg));
    nlh->nlmsg_type      = RTM_DELROUTE;
    nlh->nlmsg_flags     = NLM_F_REQUEST | NLM_F_ACK;
    nlh->nlmsg_seq       = ++_seq;

    // routing message.
    struct rtmsg* rtm = reinterpret_cast<struct rtmsg*> (NLMSG_DATA (nlh));
    rtm->rtm_family   = dest.family ();
    rtm->rtm_dst_len  = prefix;
    rtm->rtm_table    = RT_TABLE_MAIN;
    rtm->rtm_protocol = RTPROT_STATIC;
    rtm->rtm_scope    = RT_SCOPE_UNIVERSE;
    rtm->rtm_type     = RTN_UNICAST;

    // add destination.
    addAttributes (nlh, RTA_DST, dest.addr (), dest.length ());

    // add gateway.
    IpAddress gw = route->gateway ();
    if (!gw.isWildcard ())
    {
        addAttributes (nlh, RTA_GATEWAY, gw.addr (), gw.length ());
    }

    // add output interface.
    addAttributes (nlh, RTA_OIF, &index, sizeof (uint32_t));

    return sendRequest (nlh, sync);
}

// =========================================================================
//   CLASS     : RouteManager
//   METHOD    : removeRoute
// =========================================================================
int RouteManager::removeRoute (const std::string& interfaceName, const IpAddress& dest, uint32_t prefix, bool sync)
{
    return removeRoute (if_nametoindex (interfaceName.c_str ()), dest, prefix, sync);
}

// =========================================================================
//   CLASS     : RouteManager
//   METHOD    : flushRoutes
// =========================================================================
int RouteManager::flushRoutes (uint32_t index, bool sync)
{
    RouteList routes = enumerate (index);

    int res = 0;
    for (auto& route : routes)
    {
        if (route->isKernel ())
        {
            continue;
        }

        if (removeRoute (index, route->dest (), route->prefix (), sync) != 0)
        {
            res = -1;
        }
    }

    return res;
}

// =========================================================================
//   CLASS     : RouteManager
//   METHOD    : flushRoutes
// =========================================================================
int RouteManager::flushRoutes (const std::string& interfaceName, bool sync)
{
    return flushRoutes (if_nametoindex (interfaceName.c_str ()), sync);
}

// =========================================================================
//   CLASS     : RouteManager
//   METHOD    : setRoute
// =========================================================================
int RouteManager::setRoute (uint32_t index, const IpAddress& dest, uint32_t prefix, const IpAddress& gateway,
                            uint32_t metric, bool sync)
{
    char buffer[_bufferSize] = {};

    // netlink header.
    struct nlmsghdr* nlh = reinterpret_cast<struct nlmsghdr*> (buffer);
    nlh->nlmsg_len       = NLMSG_LENGTH (sizeof (struct rtmsg));
    nlh->nlmsg_type      = RTM_NEWROUTE;
    nlh->nlmsg_flags     = NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE | NLM_F_REPLACE;
    nlh->nlmsg_seq       = ++_seq;

    // routing message.
    struct rtmsg* rtm = reinterpret_cast<struct rtmsg*> (NLMSG_DATA (nlh));
    rtm->rtm_family   = dest.family ();
    rtm->rtm_dst_len  = prefix;
    rtm->rtm_table    = RT_TABLE_MAIN;
    rtm->rtm_protocol = RTPROT_STATIC;
    rtm->rtm_scope    = RT_SCOPE_UNIVERSE;
    rtm->rtm_type     = RTN_UNICAST;

    // add destination.
    addAttributes (nlh, RTA_DST, dest.addr (), dest.length ());

    // add gateway if specified.
    if (!gateway.isWildcard ())
    {
        addAttributes (nlh, RTA_GATEWAY, gateway.addr (), gateway.length ());
    }

    // add output interface.
    addAttributes (nlh, RTA_OIF, &index, sizeof (uint32_t));

    // add metric if specified.
    if (metric)
    {
        addAttributes (nlh, RTA_PRIORITY, &metric, sizeof (uint32_t));
    }

    return sendRequest (nlh, sync);
}

// =========================================================================
//   CLASS     : RouteManager
//   METHOD    : dumpRoutes
// =========================================================================
int RouteManager::dumpRoutes (bool sync)
{
    char buffer[_bufferSize] = {};

    // netlink header.
    struct nlmsghdr* nlh = reinterpret_cast<struct nlmsghdr*> (buffer);
    nlh->nlmsg_len       = NLMSG_LENGTH (sizeof (struct rtgenmsg));
    nlh->nlmsg_type      = RTM_GETROUTE;
    nlh->nlmsg_flags     = NLM_F_REQUEST | NLM_F_DUMP;
    nlh->nlmsg_seq       = ++_seq;

    // general message.
    struct rtgenmsg* rtgen = reinterpret_cast<struct rtgenmsg*> (NLMSG_DATA (nlh));
    rtgen->rtgen_family    = AF_UNSPEC;

    return sendRequest (nlh, sync);
}

// =========================================================================
//   CLASS     : RouteManager
//   METHOD    : onMessage
// =========================================================================
void RouteManager::onMessage (struct nlmsghdr* nlh)
{
    switch (nlh->nlmsg_type)
    {
        case RTM_NEWROUTE:
        case RTM_DELROUTE:
            onRouteMessage (nlh);
            break;

        default:
            break;
    }
}

// =========================================================================
//   CLASS     : RouteManager
//   METHOD    : onRouteMessage
// =========================================================================
void RouteManager::onRouteMessage (struct nlmsghdr* nlh)
{
    struct rtmsg* rtm  = reinterpret_cast<struct rtmsg*> (NLMSG_DATA (nlh));
    struct rtattr* rta = RTM_RTA (rtm);
    int len            = RTM_PAYLOAD (nlh);

    if (rtm->rtm_family != AF_INET && rtm->rtm_family != AF_INET6)
    {
        return;
    }

    if (rtm->rtm_table != RT_TABLE_MAIN)
    {
        return;
    }

    socklen_t addrlen = (rtm->rtm_family == AF_INET6) ? IpAddress::ipv6Length : IpAddress::ipv4Length;

    uint32_t index = 0;
    IpAddress dest (rtm->rtm_family);
    IpAddress gateway (rtm->rtm_family);
    uint32_t metric = 0;

    while (RTA_OK (rta, len))
    {
        switch (rta->rta_type)
        {
            case RTA_DST:
                dest = IpAddress (RTA_DATA (rta), addrlen);
                break;

            case RTA_GATEWAY:
                gateway = IpAddress (RTA_DATA (rta), addrlen);
                break;

            case RTA_PRIORITY:
                metric = *reinterpret_cast<uint32_t*> (RTA_DATA (rta));
                break;

            case RTA_OIF:
                index = *reinterpret_cast<uint32_t*> (RTA_DATA (rta));
                break;

            default:
                break;
        }

        rta = RTA_NEXT (rta, len);
    }

    if (index == 0)
    {
        return;
    }

    if (nlh->nlmsg_type == RTM_DELROUTE)
    {
        Route::Ptr route;

        {
            ScopedLock<Mutex> lock (_routeMutex);
            auto it = _routes.find ({index, dest, rtm->rtm_dst_len});
            if (it != _routes.end ())
            {
                route = it->second;
                _routes.erase (it);
            }
        }

        if (route)
        {
            RouteInfo info{};
            info.route = route;
            info.flags = RouteChangeType::Deleted;
            notifyRouteUpdate (info);
        }

        return;
    }

    RouteInfo info{};
    Route::Ptr route = acquire (index, dest, rtm->rtm_dst_len, info);

    {
        ScopedLock<Mutex> lock (route->_mutex);
        info.flags |= updateValue (route->_gateway, gateway, RouteChangeType::GatewayChanged);
        info.flags |= updateValue (route->_metric, metric, RouteChangeType::MetricChanged);
        info.flags |= updateValue (route->_type, rtm->rtm_type, RouteChangeType::TypeChanged);
        info.flags |= updateValue (route->_scope, rtm->rtm_scope, RouteChangeType::ScopeChanged);
        info.flags |= updateValue (route->_protocol, rtm->rtm_protocol, RouteChangeType::ProtocolChanged);
    }

    notifyRouteUpdate (info);
}

// =========================================================================
//   CLASS     : RouteManager
//   METHOD    : notifyRouteUpdate
// =========================================================================
void RouteManager::notifyRouteUpdate (const RouteInfo& info)
{
    for (auto& listener : _routeListeners)
    {
        if (listener.second)
        {
            listener.second (info);
        }
    }
}

// =========================================================================
//   CLASS     : RouteManager
//   METHOD    : acquire
// =========================================================================
Route::Ptr RouteManager::acquire (uint32_t index, const IpAddress& dest, uint32_t prefix, RouteInfo& info)
{
    ScopedLock<Mutex> lock (_routeMutex);

    RouteKey key = {index, dest, prefix};

    auto it = _routes.find (key);
    if (it != _routes.end ())
    {
        info.flags |= RouteChangeType::Modified;
        info.route = it->second;
        return it->second;
    }

    Route::Ptr route (new Route (this, key));
    _routes.emplace (key, route);

    info.flags |= RouteChangeType::Added;
    info.route = route;

    return route;
}
