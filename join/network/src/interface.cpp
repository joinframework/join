/**
 * MIT License
 *
 * Copyright (c) 2025 Mathieu Rabine
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
#include <join/interfacemanager.hpp>
#include <join/interface.hpp>
#include <join/error.hpp>

// C++.
#include <algorithm>
#include <chrono>

// C.
#include <linux/if_arp.h>
#include <linux/if.h>
#include <cstring>

using join::Interface;
using join::InterfaceManager;
using join::IpAddress;
using join::MacAddress;

// =========================================================================
//   CLASS     : Interface
//   METHOD    : Interface
// =========================================================================
Interface::Interface (InterfaceManager* manager, uint32_t index)
: _manager (manager)
, _index (index)
{
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : index
// =========================================================================
uint32_t Interface::index () const noexcept
{
    return _index;
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : master
// =========================================================================
uint32_t Interface::master () const noexcept
{
    return _master;
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : name
// =========================================================================
const std::string& Interface::name () const noexcept
{
    return _name;
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : mtu
// =========================================================================
int Interface::mtu (uint32_t mtuBytes, bool sync)
{
    return _manager->mtu (_index, mtuBytes, sync);
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : mtu
// =========================================================================
uint32_t Interface::mtu () const noexcept
{
    return _mtu;
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : kind
// =========================================================================
const std::string& Interface::kind () const noexcept
{
    return _kind;
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : mac
// =========================================================================
int Interface::mac (const MacAddress& macAddress, bool sync)
{
    return _manager->mac (_index, macAddress, sync);
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : mac
// =========================================================================
const MacAddress& Interface::mac () const noexcept
{
    return _mac;
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : addAddress
// =========================================================================
int Interface::addAddress (const IpAddress& ipAddress, uint32_t prefix, const IpAddress& broadcast, bool sync)
{
    return _manager->addAddress (_index, ipAddress, prefix, broadcast, sync);
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : addAddress
// =========================================================================
int Interface::addAddress (const Address& address, bool sync)
{
    return addAddress (std::get <0> (address), std::get <1> (address), std::get <2> (address), sync);
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : removeAddress
// =========================================================================
int Interface::removeAddress (const IpAddress& ipAddress, uint32_t prefix, const IpAddress& broadcast, bool sync)
{
    return _manager->removeAddress (_index, ipAddress, prefix, broadcast, sync);
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : removeAddress
// =========================================================================
int Interface::removeAddress (const Address& address, bool sync)
{
    return removeAddress (std::get <0> (address), std::get <1> (address), std::get <2> (address), sync);
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : addressList
// =========================================================================
const Interface::AddressList& Interface::addressList () const noexcept
{
    return _addresses;
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : hasAddress
// =========================================================================
bool Interface::hasAddress (const IpAddress& ipAddress)
{
    ScopedLock lock (_mutex);

    for (const auto& addr : _addresses)
    {
        if (std::get <0> (addr) == IpAddress (ipAddress.addr (), ipAddress.length (), _index))
        {
            return true;
        }
    }

    return false;
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : hasLocalAddress
// =========================================================================
bool Interface::hasLocalAddress ()
{
    ScopedLock lock (_mutex);

    for (const auto& addr : _addresses)
    {
        if (std::get <0> (addr).isLinkLocal ())
        {
            return true;
        }
    }

    return false;
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : addRoute
// =========================================================================
int Interface::addRoute (const IpAddress& dest, uint32_t prefix, const IpAddress& gateway, uint32_t metric, bool sync)
{
    return _manager->addRoute (_index, dest, prefix, gateway, &metric, sync);
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : addRoute
// =========================================================================
int Interface::addRoute (const Route& route, bool sync)
{
    return addRoute (std::get <0> (route), std::get <1> (route), std::get <2> (route), std::get <3> (route), sync);
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : removeRoute
// =========================================================================
int Interface::removeRoute (const IpAddress& dest, uint32_t prefix, const IpAddress& gateway, uint32_t metric, bool sync)
{
    return _manager->removeRoute (_index, dest, prefix, gateway, &metric, sync);
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : removeRoute
// =========================================================================
int Interface::removeRoute (const Route& route, bool sync)
{
    return removeRoute (std::get <0> (route), std::get <1> (route), std::get <2> (route), std::get <3> (route), sync);
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : routeList
// =========================================================================
const Interface::RouteList& Interface::routeList () const noexcept
{
    return _routes;
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : hasRoute
// =========================================================================
bool Interface::hasRoute (const IpAddress& dest, uint32_t prefix, const IpAddress& gateway, uint32_t metric)
{
    ScopedLock lock (_mutex);

    for (const auto& rt : _routes)
    {
        if (std::get <0> (rt) == dest &&
            std::get <1> (rt) == prefix &&
            std::get <2> (rt) == gateway &&
            std::get <3> (rt) == metric)
        {
            return true;
        }
    }

    return false;
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : hasRoute
// =========================================================================
bool Interface::hasRoute (const Route& route)
{
    return hasRoute (std::get <0> (route), std::get <1> (route), std::get <2> (route), std::get <3> (route));
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : addToBridge
// =========================================================================
int Interface::addToBridge (uint32_t masterIndex, bool sync)
{
    return _manager->addToBridge (_index, masterIndex, sync);
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : addToBridge
// =========================================================================
int Interface::addToBridge (const std::string& masterName, bool sync)
{
    int index = if_nametoindex (masterName.c_str ());
    if (index == 0)
    {
        lastError = make_error_code (Errc::InvalidParam);
        return -1;
    }

    return addToBridge (index, sync);
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : removeFromBridge
// =========================================================================
int Interface::removeFromBridge (bool sync)
{
    return _manager->removeFromBridge (_index, sync);
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : flags
// =========================================================================
uint32_t Interface::flags () const noexcept
{
    return _flags;
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : enable
// =========================================================================
int Interface::enable (bool enabled, bool sync)
{
    return _manager->enable (_index, enabled, sync);
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : isEnabled
// =========================================================================
bool Interface::isEnabled () const noexcept
{
    return (_flags & IFF_UP);
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : isRunning
// =========================================================================
bool Interface::isRunning () const noexcept
{
    return (_flags & IFF_RUNNING);
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : isLoopback
// =========================================================================
bool Interface::isLoopback () const noexcept
{
    return (_flags & IFF_LOOPBACK);
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : isPointToPoint
// =========================================================================
bool Interface::isPointToPoint () const noexcept
{
    return (_flags & IFF_POINTOPOINT);
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : isDummy
// =========================================================================
bool Interface::isDummy () const noexcept
{
    return (_kind == "dummy");
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : isBridge
// =========================================================================
bool Interface::isBridge () const noexcept
{
    return (_kind == "bridge");
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : isVlan
// =========================================================================
bool Interface::isVlan () const noexcept
{
    return (_kind == "vlan");
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : isVeth
// =========================================================================
bool Interface::isVeth () const noexcept
{
    return (_kind == "veth");
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : isGre
// =========================================================================
bool Interface::isGre () const noexcept
{
    return ((_kind == "gre") || (_kind == "ip6gre"));
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : isTun
// =========================================================================
bool Interface::isTun () const noexcept
{
    return (_kind == "tun");
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : supportsBroadcast
// =========================================================================
bool Interface::supportsBroadcast () const noexcept
{
    return (_flags & IFF_BROADCAST);
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : supportsMulticast
// =========================================================================
bool Interface::supportsMulticast () const noexcept
{
    return (_flags & IFF_MULTICAST);
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : supportsIpv4
// =========================================================================
bool Interface::supportsIpv4 ()
{
    ScopedLock lock (_mutex);

    for (auto const& addr : _addresses)
    {
        if (std::get <0> (addr).family () == AF_INET)
        {
            return true;
        }
    }

    return false;
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : supportsIpv6
// =========================================================================
bool Interface::supportsIpv6 ()
{
    ScopedLock lock (_mutex);

    for (auto const& addr : _addresses)
    {
        if (std::get <0> (addr).family () == AF_INET6)
        {
            return true;
        }
    }

    return false;
}
