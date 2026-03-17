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

// C.
#include <linux/if_arp.h>
#include <linux/if.h>

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
uint32_t Interface::master () const
{
    ScopedLock<Mutex> lock (_mutex);
    return _master;
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : name
// =========================================================================
std::string Interface::name () const
{
    ScopedLock<Mutex> lock (_mutex);
    return _name;
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : mtu
// =========================================================================
int Interface::mtu (uint32_t mtuBytes, bool sync) const
{
    return _manager->mtu (_index, mtuBytes, sync);
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : mtu
// =========================================================================
uint32_t Interface::mtu () const
{
    ScopedLock<Mutex> lock (_mutex);
    return _mtu;
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : kind
// =========================================================================
std::string Interface::kind () const
{
    ScopedLock<Mutex> lock (_mutex);
    return _kind;
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : mac
// =========================================================================
int Interface::mac (const MacAddress& macAddress, bool sync) const
{
    return _manager->mac (_index, macAddress, sync);
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : mac
// =========================================================================
MacAddress Interface::mac () const
{
    ScopedLock<Mutex> lock (_mutex);
    return _mac;
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : addAddress
// =========================================================================
int Interface::addAddress (const IpAddress& ipAddress, uint32_t prefix, const IpAddress& broadcast, bool sync) const
{
    return _manager->addAddress (_index, ipAddress, prefix, broadcast, sync);
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : addAddress
// =========================================================================
int Interface::addAddress (const Address& address, bool sync) const
{
    return addAddress (address.ip, address.prefix, address.broadcast, sync);
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : removeAddress
// =========================================================================
int Interface::removeAddress (const IpAddress& ipAddress, uint32_t prefix, const IpAddress& broadcast, bool sync) const
{
    return _manager->removeAddress (_index, ipAddress, prefix, broadcast, sync);
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : removeAddress
// =========================================================================
int Interface::removeAddress (const Address& address, bool sync) const
{
    return removeAddress (address.ip, address.prefix, address.broadcast, sync);
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : addressList
// =========================================================================
Interface::AddressList Interface::addressList () const
{
    ScopedLock<Mutex> lock (_mutex);
    return _addresses;
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : hasAddress
// =========================================================================
bool Interface::hasAddress (const IpAddress& ipAddress) const
{
    ScopedLock<Mutex> lock (_mutex);

    for (const auto& addr : _addresses)
    {
        if (addr.ip == IpAddress (ipAddress.addr (), ipAddress.length (), _index))
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
bool Interface::hasLocalAddress () const
{
    ScopedLock<Mutex> lock (_mutex);

    for (const auto& addr : _addresses)
    {
        if (addr.ip.isLinkLocal ())
        {
            return true;
        }
    }

    return false;
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : addToBridge
// =========================================================================
int Interface::addToBridge (uint32_t masterIndex, bool sync) const
{
    return _manager->addToBridge (_index, masterIndex, sync);
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : addToBridge
// =========================================================================
int Interface::addToBridge (const std::string& masterName, bool sync) const
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
int Interface::removeFromBridge (bool sync) const
{
    return _manager->removeFromBridge (_index, sync);
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : flags
// =========================================================================
uint32_t Interface::flags () const
{
    ScopedLock<Mutex> lock (_mutex);
    return _flags;
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : enable
// =========================================================================
int Interface::enable (bool enabled, bool sync) const
{
    return _manager->enable (_index, enabled, sync);
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : isEnabled
// =========================================================================
bool Interface::isEnabled () const
{
    return (flags () & IFF_UP) != 0;
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : isRunning
// =========================================================================
bool Interface::isRunning () const
{
    return (flags () & IFF_RUNNING) != 0;
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : isLoopback
// =========================================================================
bool Interface::isLoopback () const
{
    return (flags () & IFF_LOOPBACK) != 0;
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : isPointToPoint
// =========================================================================
bool Interface::isPointToPoint () const
{
    return (flags () & IFF_POINTOPOINT) != 0;
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : isDummy
// =========================================================================
bool Interface::isDummy () const
{
    return kind () == "dummy";
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : isBridge
// =========================================================================
bool Interface::isBridge () const
{
    return kind () == "bridge";
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : isVlan
// =========================================================================
bool Interface::isVlan () const
{
    return kind () == "vlan";
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : isVeth
// =========================================================================
bool Interface::isVeth () const
{
    return kind () == "veth";
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : isGre
// =========================================================================
bool Interface::isGre () const
{
    const auto k = kind ();
    return k == "gre" || k == "ip6gre";
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : isTun
// =========================================================================
bool Interface::isTun () const
{
    return kind () == "tun";
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : supportsBroadcast
// =========================================================================
bool Interface::supportsBroadcast () const
{
    return (flags () & IFF_BROADCAST) != 0;
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : supportsMulticast
// =========================================================================
bool Interface::supportsMulticast () const
{
    return (flags () & IFF_MULTICAST) != 0;
}

// =========================================================================
//   CLASS     : Interface
//   METHOD    : supportsIpv4
// =========================================================================
bool Interface::supportsIpv4 () const
{
    ScopedLock<Mutex> lock (_mutex);

    for (const auto& addr : _addresses)
    {
        if (addr.ip.family () == AF_INET)
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
bool Interface::supportsIpv6 () const
{
    ScopedLock<Mutex> lock (_mutex);

    for (const auto& addr : _addresses)
    {
        if (addr.ip.family () == AF_INET6)
        {
            return true;
        }
    }

    return false;
}
