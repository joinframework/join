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

// C.
#include <linux/if_tunnel.h>
#include <linux/if_tun.h>
#include <linux/veth.h>

using join::Interface;
using join::InterfaceList;
using join::InterfaceManager;

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : InterfaceManager
// =========================================================================
InterfaceManager::InterfaceManager (Reactor* reactor)
: NetlinkManager (RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR | RTMGRP_IPV4_ROUTE | RTMGRP_IPV6_ROUTE,
                  reactor)
{
    start ();
    refresh ();
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : ~InterfaceManager
// =========================================================================
InterfaceManager::~InterfaceManager ()
{
    stop ();
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : instance
// =========================================================================
InterfaceManager& InterfaceManager::instance ()
{
    static InterfaceManager manager;
    return manager;
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : findByIndex
// =========================================================================
Interface::Ptr InterfaceManager::findByIndex (uint32_t interfaceIndex)
{
    ScopedLock<Mutex> lock (_ifMutex);

    auto it = _interfaces.find (interfaceIndex);
    if (it == _interfaces.end ())
    {
        return nullptr;
    }

    return it->second;
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : findByName
// =========================================================================
Interface::Ptr InterfaceManager::findByName (const std::string& interfaceName)
{
    return findByIndex (if_nametoindex (interfaceName.c_str ()));
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : enumerate
// =========================================================================
InterfaceList InterfaceManager::enumerate ()
{
    ScopedLock<Mutex> lock (_ifMutex);

    InterfaceList ifaces;
    ifaces.reserve (_interfaces.size ());

    for (auto& interface : _interfaces)
    {
        ifaces.push_back (interface.second);
    }

    return ifaces;
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : refresh
// =========================================================================
int InterfaceManager::refresh ()
{
    {
        ScopedLock<Mutex> lock (_ifMutex);
        _interfaces.clear ();
    }

    return -(dumpLink (true) != 0 || dumpAddress (true) != 0 || dumpRoute (true) != 0);
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : addLinkListener
// =========================================================================
uint64_t InterfaceManager::addLinkListener (const LinkNotify& cb)
{
    uint64_t id = ++_listenerCounter;

    pushJob ([this, id, cb] () {
        _linkListeners.emplace (id, cb);
    });

    return id;
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : removeLinkListener
// =========================================================================
void InterfaceManager::removeLinkListener (uint64_t id)
{
    pushJob ([this, id] () {
        _linkListeners.erase (id);
    });
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : addAddressListener
// =========================================================================
uint64_t InterfaceManager::addAddressListener (const AddressNotify& cb)
{
    uint64_t id = ++_listenerCounter;

    pushJob ([this, id, cb] () {
        _addressListeners.emplace (id, cb);
    });

    return id;
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : removeAddressListener
// =========================================================================
void InterfaceManager::removeAddressListener (uint64_t id)
{
    pushJob ([this, id] () {
        _addressListeners.erase (id);
    });
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : addRouteListener
// =========================================================================
uint64_t InterfaceManager::addRouteListener (const RouteNotify& cb)
{
    uint64_t id = ++_listenerCounter;

    pushJob ([this, id, cb] () {
        _routeListeners.emplace (id, cb);
    });

    return id;
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : removeRouteListener
// =========================================================================
void InterfaceManager::removeRouteListener (uint64_t id)
{
    pushJob ([this, id] () {
        _routeListeners.erase (id);
    });
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : createDummyInterface
// =========================================================================
int InterfaceManager::createDummyInterface (const std::string& interfaceName, bool sync)
{
    char buffer[_bufferSize] = {};

    // netlink header.
    struct nlmsghdr* nlh = reinterpret_cast<struct nlmsghdr*> (buffer);
    nlh->nlmsg_len       = NLMSG_LENGTH (sizeof (struct ifinfomsg));
    nlh->nlmsg_type      = RTM_NEWLINK;
    nlh->nlmsg_flags     = NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE | NLM_F_EXCL;
    nlh->nlmsg_seq       = ++_seq;

    // info message.
    struct ifinfomsg* ifi = reinterpret_cast<struct ifinfomsg*> (NLMSG_DATA (nlh));
    ifi->ifi_family       = AF_UNSPEC;

    // set interface name
    addAttributes (nlh, IFLA_IFNAME, interfaceName.c_str (), interfaceName.length () + 1);

    // start nested link info attributes
    struct rtattr* linkinfo = startNestedAttributes (nlh, IFLA_LINKINFO);

    // add interface kind
    std::string kind = "dummy";
    addAttributes (nlh, IFLA_INFO_KIND, kind.c_str (), kind.size () + 1);

    // stop nested link info attributes.
    stopNestedAttributes (nlh, linkinfo);

    // send request.
    return sendRequest (nlh, sync);
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : createBridgeInterface
// =========================================================================
int InterfaceManager::createBridgeInterface (const std::string& interfaceName, bool sync)
{
    char buffer[_bufferSize] = {};

    // netlink header.
    struct nlmsghdr* nlh = reinterpret_cast<struct nlmsghdr*> (buffer);
    nlh->nlmsg_len       = NLMSG_LENGTH (sizeof (struct ifinfomsg));
    nlh->nlmsg_type      = RTM_NEWLINK;
    nlh->nlmsg_flags     = NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE | NLM_F_EXCL;
    nlh->nlmsg_seq       = ++_seq;

    // info message.
    struct ifinfomsg* ifi = reinterpret_cast<struct ifinfomsg*> (NLMSG_DATA (nlh));
    ifi->ifi_family       = AF_UNSPEC;

    // set interface name.
    addAttributes (nlh, IFLA_IFNAME, interfaceName.c_str (), interfaceName.length () + 1);

    // start nested link info attributes.
    struct rtattr* linkinfo = startNestedAttributes (nlh, IFLA_LINKINFO);

    // add interface kind.
    std::string kind = "bridge";
    addAttributes (nlh, IFLA_INFO_KIND, kind.c_str (), kind.size () + 1);

    // stop nested link info attributes.
    stopNestedAttributes (nlh, linkinfo);

    // send request.
    return sendRequest (nlh, sync);
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : createVlanInterface
// =========================================================================
int InterfaceManager::createVlanInterface (const std::string& interfaceName, uint32_t parentIndex, uint16_t id,
                                           uint16_t proto, bool sync)
{
    if ((id == reservedVlanId) || (id > maxVlanId))
    {
        lastError = make_error_code (Errc::InvalidParam);
        return -1;
    }

    char buffer[_bufferSize] = {};

    // netlink header.
    struct nlmsghdr* nlh = reinterpret_cast<struct nlmsghdr*> (buffer);
    nlh->nlmsg_len       = NLMSG_LENGTH (sizeof (struct ifinfomsg));
    nlh->nlmsg_type      = RTM_NEWLINK;
    nlh->nlmsg_flags     = NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE | NLM_F_EXCL;
    nlh->nlmsg_seq       = ++_seq;

    // info message.
    struct ifinfomsg* ifi = reinterpret_cast<struct ifinfomsg*> (NLMSG_DATA (nlh));
    ifi->ifi_family       = AF_UNSPEC;

    // set interface name.
    addAttributes (nlh, IFLA_IFNAME, interfaceName.c_str (), interfaceName.length () + 1);

    // set parent interface link.
    addAttributes (nlh, IFLA_LINK, &parentIndex, sizeof (uint32_t));

    // start nested link info attributes.
    struct rtattr* linkinfo = startNestedAttributes (nlh, IFLA_LINKINFO);

    // add interface kind.
    std::string kind = "vlan";
    addAttributes (nlh, IFLA_INFO_KIND, kind.c_str (), kind.size () + 1);

    // start nested link info data.
    struct rtattr* data = startNestedAttributes (nlh, IFLA_INFO_DATA);

    // add VLAN id.
    addAttributes (nlh, IFLA_VLAN_ID, &id, sizeof (uint16_t));

    // add VLAN protocol.
    uint16_t vproto = htons (proto);
    addAttributes (nlh, IFLA_VLAN_PROTOCOL, &vproto, sizeof (uint16_t));

    // stop nested link info data.
    stopNestedAttributes (nlh, data);

    // stop nested link info attributes.
    stopNestedAttributes (nlh, linkinfo);

    // send request.
    return sendRequest (nlh, sync);
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : createVlanInterface
// =========================================================================
int InterfaceManager::createVlanInterface (const std::string& interfaceName, const std::string& parentName, uint16_t id,
                                           uint16_t proto, bool sync)
{
    return createVlanInterface (interfaceName, if_nametoindex (parentName.c_str ()), id, proto, sync);
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : createVethInterface
// =========================================================================
int InterfaceManager::createVethInterface (const std::string& hostName, const std::string& peerName, pid_t* pid,
                                           bool sync)
{
    char buffer[_bufferSize] = {};

    // netlink header.
    struct nlmsghdr* nlh = reinterpret_cast<struct nlmsghdr*> (buffer);
    nlh->nlmsg_len       = NLMSG_LENGTH (sizeof (struct ifinfomsg));
    nlh->nlmsg_type      = RTM_NEWLINK;
    nlh->nlmsg_flags     = NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE | NLM_F_EXCL;
    nlh->nlmsg_seq       = ++_seq;

    // info message.
    struct ifinfomsg* ifi = reinterpret_cast<struct ifinfomsg*> (NLMSG_DATA (nlh));
    ifi->ifi_family       = AF_UNSPEC;

    // add pid if specified.
    if (pid && (*pid > 0))
    {
        addAttributes (nlh, IFLA_NET_NS_PID, pid, sizeof (pid_t));
    }

    // add host interface name.
    addAttributes (nlh, IFLA_IFNAME, hostName.c_str (), hostName.length () + 1);

    // start nested link info attributes.
    struct rtattr* linkinfo = startNestedAttributes (nlh, IFLA_LINKINFO);

    // add info kind.
    std::string kind = "veth";
    addAttributes (nlh, IFLA_INFO_KIND, kind.c_str (), kind.size () + 1);

    // start nested link info data.
    struct rtattr* data = startNestedAttributes (nlh, IFLA_INFO_DATA);

    // add peer info data.
    addPeerInfoData (nlh, peerName);

    // stop nested link info data.
    stopNestedAttributes (nlh, data);

    // stop nested link info attributes.
    stopNestedAttributes (nlh, linkinfo);

    // send request.
    return sendRequest (nlh, sync);
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : createGreInterface
// =========================================================================
int InterfaceManager::createGreInterface (const std::string& tunnelName, uint32_t parentIndex,
                                          const IpAddress& localAddress, const IpAddress& remoteAddress,
                                          const uint32_t* ikey, const uint32_t* okey, uint8_t ttl, bool sync)
{
    if (localAddress.family () != remoteAddress.family ())
    {
        lastError = make_error_code (Errc::InvalidParam);
        return -1;
    }

    char buffer[_bufferSize] = {};

    // netlink header.
    struct nlmsghdr* nlh = reinterpret_cast<struct nlmsghdr*> (buffer);
    nlh->nlmsg_len       = NLMSG_LENGTH (sizeof (struct ifinfomsg));
    nlh->nlmsg_type      = RTM_NEWLINK;
    nlh->nlmsg_flags     = NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE | NLM_F_EXCL;
    nlh->nlmsg_seq       = ++_seq;

    // interface informations.
    struct ifinfomsg* ifi = reinterpret_cast<struct ifinfomsg*> (NLMSG_DATA (nlh));
    ifi->ifi_family       = AF_UNSPEC;

    // set tunnel name.
    addAttributes (nlh, IFLA_IFNAME, tunnelName.c_str (), tunnelName.length () + 1);

    // start nested link info attributes.
    struct rtattr* linkinfo = startNestedAttributes (nlh, IFLA_LINKINFO);

    // add info kind.
    std::string kind = localAddress.isIpv4Address () ? "gre" : "ip6gre";
    addAttributes (nlh, IFLA_INFO_KIND, kind.c_str (), kind.length () + 1);

    // start nested link info data.
    struct rtattr* data = startNestedAttributes (nlh, IFLA_INFO_DATA);

    uint16_t iflags = 0, oflags = 0;

    // add inbound key.
    if (ikey)
    {
        iflags |= GRE_KEY;
        addAttributes (nlh, IFLA_GRE_IKEY, ikey, sizeof (uint32_t));
    }

    // add outbound key.
    if (okey)
    {
        oflags |= GRE_KEY;
        addAttributes (nlh, IFLA_GRE_OKEY, okey, sizeof (uint32_t));
    }

    // add parent interface.
    addAttributes (nlh, IFLA_GRE_LINK, &parentIndex, sizeof (uint32_t));

    // add local and remote address.
    addAttributes (nlh, IFLA_GRE_LOCAL, localAddress.addr (), localAddress.length ());
    addAttributes (nlh, IFLA_GRE_REMOTE, remoteAddress.addr (), remoteAddress.length ());

    // add hop limit.
    addAttributes (nlh, IFLA_GRE_TTL, &ttl, sizeof (uint8_t));

    uint8_t pmtudisc = 1;
    addAttributes (nlh, IFLA_GRE_PMTUDISC, &pmtudisc, sizeof (uint8_t));

    uint8_t tos = 0;
    addAttributes (nlh, IFLA_GRE_TOS, &tos, sizeof (uint8_t));

    // add encapsulation informations.
    uint16_t encaptype = TUNNEL_ENCAP_NONE;
    addAttributes (nlh, IFLA_GRE_ENCAP_TYPE, &encaptype, sizeof (uint16_t));

    uint16_t encapflags = 0;
    addAttributes (nlh, IFLA_GRE_ENCAP_FLAGS, &encapflags, sizeof (uint16_t));

    uint16_t encapsport = htons (0);
    addAttributes (nlh, IFLA_GRE_ENCAP_SPORT, &encapsport, sizeof (uint16_t));

    uint16_t encapdport = htons (0);
    addAttributes (nlh, IFLA_GRE_ENCAP_DPORT, &encapdport, sizeof (uint16_t));

    // add flags.
    addAttributes (nlh, IFLA_GRE_IFLAGS, &iflags, sizeof (uint16_t));
    addAttributes (nlh, IFLA_GRE_OFLAGS, &oflags, sizeof (uint16_t));

    // stop nested link info data.
    stopNestedAttributes (nlh, data);

    // stop nested link info attributes.
    stopNestedAttributes (nlh, linkinfo);

    // send request.
    return sendRequest (nlh, sync);
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : createGreInterface
// =========================================================================
int InterfaceManager::createGreInterface (const std::string& tunnelName, const std::string& parentName,
                                          const IpAddress& localAddress, const IpAddress& remoteAddress,
                                          const uint32_t* ikey, const uint32_t* okey, uint8_t ttl, bool sync)
{
    return createGreInterface (tunnelName, if_nametoindex (parentName.c_str ()), localAddress, remoteAddress, ikey,
                               okey, ttl, sync);
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : removeInterface
// =========================================================================
int InterfaceManager::removeInterface (uint32_t interfaceIndex, bool sync)
{
    char buffer[_bufferSize] = {};

    // netlink header.
    struct nlmsghdr* nlh = reinterpret_cast<struct nlmsghdr*> (buffer);
    nlh->nlmsg_len       = NLMSG_LENGTH (sizeof (struct ifinfomsg));
    nlh->nlmsg_type      = RTM_DELLINK;
    nlh->nlmsg_flags     = NLM_F_REQUEST | NLM_F_ACK;
    nlh->nlmsg_seq       = ++_seq;

    // info message.
    struct ifinfomsg* ifi = reinterpret_cast<struct ifinfomsg*> (NLMSG_DATA (nlh));
    ifi->ifi_family       = AF_UNSPEC;
    ifi->ifi_index        = interfaceIndex;

    // send request.
    return sendRequest (nlh, sync);
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : removeInterface
// =========================================================================
int InterfaceManager::removeInterface (const std::string& interfaceName, bool sync)
{
    return removeInterface (if_nametoindex (interfaceName.c_str ()), sync);
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : addPeerInfoData
// =========================================================================
void InterfaceManager::addPeerInfoData (struct nlmsghdr* nlh, const std::string& peerName)
{
    // start nested peer info data.
    struct rtattr* peerinfo = startNestedAttributes (nlh, VETH_INFO_PEER);

    // peer info message.
    struct ifinfomsg* ifi = reinterpret_cast<struct ifinfomsg*> (reinterpret_cast<char*> (nlh) + nlh->nlmsg_len);
    ifi->ifi_family       = AF_UNSPEC;

    nlh->nlmsg_len += NLMSG_ALIGN (sizeof (struct ifinfomsg));

    // set peer interface name.
    addAttributes (nlh, IFLA_IFNAME, peerName.c_str (), peerName.length () + 1);

    // stop nested peer info data.
    stopNestedAttributes (nlh, peerinfo);
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : mtu
// =========================================================================
int InterfaceManager::mtu (uint32_t interfaceIndex, uint32_t mtuBytes, bool sync)
{
    char buffer[_bufferSize] = {};

    // netlink header.
    struct nlmsghdr* nlh = reinterpret_cast<struct nlmsghdr*> (buffer);
    nlh->nlmsg_len       = NLMSG_LENGTH (sizeof (struct ifinfomsg));
    nlh->nlmsg_type      = RTM_SETLINK;
    nlh->nlmsg_flags     = NLM_F_REQUEST | NLM_F_ACK;
    nlh->nlmsg_seq       = ++_seq;

    // info message.
    struct ifinfomsg* ifi = reinterpret_cast<struct ifinfomsg*> (NLMSG_DATA (nlh));
    ifi->ifi_family       = AF_UNSPEC;
    ifi->ifi_index        = interfaceIndex;

    // add mtu.
    addAttributes (nlh, IFLA_MTU, &mtuBytes, sizeof (uint32_t));

    // send request.
    return sendRequest (nlh, sync);
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : mac
// =========================================================================
int InterfaceManager::mac (uint32_t interfaceIndex, const MacAddress& macAddress, bool sync)
{
    char buffer[_bufferSize] = {};

    // netlink header.
    struct nlmsghdr* nlh = reinterpret_cast<struct nlmsghdr*> (buffer);
    nlh->nlmsg_len       = NLMSG_LENGTH (sizeof (struct ifinfomsg));
    nlh->nlmsg_type      = RTM_SETLINK;
    nlh->nlmsg_flags     = NLM_F_REQUEST | NLM_F_ACK;
    nlh->nlmsg_seq       = ++_seq;

    // info message.
    struct ifinfomsg* ifi = reinterpret_cast<struct ifinfomsg*> (NLMSG_DATA (nlh));
    ifi->ifi_family       = AF_UNSPEC;
    ifi->ifi_index        = interfaceIndex;

    // add mac address.
    addAttributes (nlh, IFLA_ADDRESS, macAddress.addr (), macAddress.length ());

    // send request.
    return sendRequest (nlh, sync);
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : addToBridge
// =========================================================================
int InterfaceManager::addToBridge (uint32_t interfaceIndex, const uint32_t masterIndex, bool sync)
{
    char buffer[_bufferSize] = {};

    // netlink header.
    struct nlmsghdr* nlh = reinterpret_cast<struct nlmsghdr*> (buffer);
    nlh->nlmsg_len       = NLMSG_LENGTH (sizeof (struct ifinfomsg));
    nlh->nlmsg_type      = RTM_SETLINK;
    nlh->nlmsg_flags     = NLM_F_REQUEST | NLM_F_ACK;
    nlh->nlmsg_seq       = ++_seq;

    struct ifinfomsg* ifi = reinterpret_cast<struct ifinfomsg*> (NLMSG_DATA (nlh));
    ifi->ifi_family       = AF_UNSPEC;
    ifi->ifi_index        = interfaceIndex;

    // add bridge index.
    addAttributes (nlh, IFLA_MASTER, &masterIndex, sizeof (uint32_t));

    // send request.
    return sendRequest (nlh, sync);
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : removeFromBridge
// =========================================================================
int InterfaceManager::removeFromBridge (uint32_t interfaceIndex, bool sync)
{
    return addToBridge (interfaceIndex, 0, sync);
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : enable
// =========================================================================
int InterfaceManager::enable (uint32_t interfaceIndex, bool enabled, bool sync)
{
    char buffer[_bufferSize] = {};

    // netlink header.
    struct nlmsghdr* nlh = reinterpret_cast<struct nlmsghdr*> (buffer);
    nlh->nlmsg_len       = NLMSG_LENGTH (sizeof (struct ifinfomsg));
    nlh->nlmsg_type      = RTM_SETLINK;
    nlh->nlmsg_flags     = NLM_F_REQUEST | NLM_F_ACK;
    nlh->nlmsg_seq       = ++_seq;

    // info message.
    struct ifinfomsg* ifi = reinterpret_cast<struct ifinfomsg*> (NLMSG_DATA (nlh));
    ifi->ifi_family       = AF_UNSPEC;
    ifi->ifi_index        = interfaceIndex;
    ifi->ifi_flags        = enabled ? IFF_UP : 0;
    ifi->ifi_change       = IFF_UP;

    // send request.
    return sendRequest (nlh, sync);
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : addAddress
// =========================================================================
int InterfaceManager::addAddress (uint32_t interfaceIndex, const IpAddress& ipAddress, uint32_t prefix,
                                  const IpAddress& broadcast, bool sync)
{
    char buffer[_bufferSize] = {};

    // netlink header.
    struct nlmsghdr* nlh = reinterpret_cast<struct nlmsghdr*> (buffer);
    nlh->nlmsg_len       = NLMSG_LENGTH (sizeof (struct ifaddrmsg));
    nlh->nlmsg_type      = RTM_NEWADDR;
    nlh->nlmsg_flags     = NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE | NLM_F_EXCL;
    nlh->nlmsg_seq       = ++_seq;

    // address message.
    struct ifaddrmsg* ifa = reinterpret_cast<struct ifaddrmsg*> (NLMSG_DATA (nlh));
    ifa->ifa_family       = ipAddress.family ();
    ifa->ifa_prefixlen    = prefix;
    ifa->ifa_index        = interfaceIndex;

    // add local address.
    addAttributes (nlh, IFA_LOCAL, ipAddress.addr (), ipAddress.length ());

    // add address.
    addAttributes (nlh, IFA_ADDRESS, ipAddress.addr (), ipAddress.length ());

    // add broadcast address if specified.
    if (ipAddress.family () == AF_INET && broadcast.isBroadcast (prefix))
    {
        addAttributes (nlh, IFA_BROADCAST, broadcast.addr (), broadcast.length ());
    }

    // send request.
    return sendRequest (nlh, sync);
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : removeAddress
// =========================================================================
int InterfaceManager::removeAddress (uint32_t interfaceIndex, const IpAddress& ipAddress, uint32_t prefix,
                                     const IpAddress& broadcast, bool sync)
{
    char buffer[_bufferSize] = {};

    // netlink header.
    struct nlmsghdr* nlh = reinterpret_cast<struct nlmsghdr*> (buffer);
    nlh->nlmsg_len       = NLMSG_LENGTH (sizeof (struct ifaddrmsg));
    nlh->nlmsg_type      = RTM_DELADDR;
    nlh->nlmsg_flags     = NLM_F_REQUEST | NLM_F_ACK;
    nlh->nlmsg_seq       = ++_seq;

    // address message.
    struct ifaddrmsg* ifa = reinterpret_cast<struct ifaddrmsg*> (NLMSG_DATA (nlh));
    ifa->ifa_family       = ipAddress.family ();
    ifa->ifa_prefixlen    = prefix;
    ifa->ifa_index        = interfaceIndex;

    // add local address.
    addAttributes (nlh, IFA_LOCAL, ipAddress.addr (), ipAddress.length ());

    // add address.
    addAttributes (nlh, IFA_ADDRESS, ipAddress.addr (), ipAddress.length ());

    // add broadcast address if specified.
    if (ipAddress.family () == AF_INET && broadcast.isBroadcast (prefix))
    {
        addAttributes (nlh, IFA_BROADCAST, broadcast.addr (), broadcast.length ());
    }

    // send request.
    return sendRequest (nlh, sync);
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : addRoute
// =========================================================================
int InterfaceManager::addRoute (uint32_t interfaceIndex, const IpAddress& dest, uint32_t prefix,
                                const IpAddress& gateway, uint32_t* metric, bool sync)
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
    addAttributes (nlh, RTA_OIF, &interfaceIndex, sizeof (uint32_t));

    // add metric if specified.
    if (metric)
    {
        addAttributes (nlh, RTA_PRIORITY, metric, sizeof (uint32_t));
    }

    // send request.
    return sendRequest (nlh, sync);
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : removeRoute
// =========================================================================
int InterfaceManager::removeRoute (uint32_t interfaceIndex, const IpAddress& dest, uint32_t prefix,
                                   const IpAddress& gateway, uint32_t* metric, bool sync)
{
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

    // add gateway if specified.
    if (!gateway.isWildcard ())
    {
        addAttributes (nlh, RTA_GATEWAY, gateway.addr (), gateway.length ());
    }

    // add output interface.
    addAttributes (nlh, RTA_OIF, &interfaceIndex, sizeof (uint32_t));

    // add metric if specified.
    if (metric)
    {
        addAttributes (nlh, RTA_PRIORITY, metric, sizeof (uint32_t));
    }

    // send request.
    return sendRequest (nlh, sync);
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : dumpLink
// =========================================================================
int InterfaceManager::dumpLink (bool sync)
{
    char buffer[_bufferSize] = {};

    // netlink header.
    struct nlmsghdr* nlh = reinterpret_cast<struct nlmsghdr*> (buffer);
    nlh->nlmsg_len       = NLMSG_LENGTH (sizeof (struct rtgenmsg));
    nlh->nlmsg_type      = RTM_GETLINK;
    nlh->nlmsg_flags     = NLM_F_REQUEST | NLM_F_DUMP;
    nlh->nlmsg_seq       = ++_seq;

    // general message.
    struct rtgenmsg* rtgen = reinterpret_cast<struct rtgenmsg*> (NLMSG_DATA (nlh));
    rtgen->rtgen_family    = AF_UNSPEC;

    // send request.
    return sendRequest (nlh, sync);
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : dumpAddress
// =========================================================================
int InterfaceManager::dumpAddress (bool sync)
{
    char buffer[_bufferSize] = {};

    // netlink header.
    struct nlmsghdr* nlh = reinterpret_cast<struct nlmsghdr*> (buffer);
    nlh->nlmsg_len       = NLMSG_LENGTH (sizeof (struct rtgenmsg));
    nlh->nlmsg_type      = RTM_GETADDR;
    nlh->nlmsg_flags     = NLM_F_REQUEST | NLM_F_DUMP;
    nlh->nlmsg_seq       = ++_seq;

    // general message.
    struct rtgenmsg* rtgen = reinterpret_cast<struct rtgenmsg*> (NLMSG_DATA (nlh));
    rtgen->rtgen_family    = AF_UNSPEC;

    // send request.
    return sendRequest (nlh, sync);
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : dumpRoute
// =========================================================================
int InterfaceManager::dumpRoute (bool sync)
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

    // send request.
    return sendRequest (nlh, sync);
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : onMessage
// =========================================================================
void InterfaceManager::onMessage (struct nlmsghdr* nlh)
{
    switch (nlh->nlmsg_type)
    {
        case RTM_NEWLINK:
        case RTM_DELLINK:
            onLinkMessage (nlh);
            break;

        case RTM_NEWADDR:
        case RTM_DELADDR:
            onAddressMessage (nlh);
            break;

        case RTM_NEWROUTE:
        case RTM_DELROUTE:
            onRouteMessage (nlh);
            break;

        default:
            break;
    }
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : onLinkMessage
// =========================================================================
void InterfaceManager::onLinkMessage (struct nlmsghdr* nlh)
{
    struct ifinfomsg* ifi = reinterpret_cast<struct ifinfomsg*> (NLMSG_DATA (nlh));
    struct rtattr* rta    = IFLA_RTA (ifi);
    int len               = IFLA_PAYLOAD (nlh);

    if (ifi->ifi_family == AF_BRIDGE)
    {
        return;
    }

    LinkInfo info{};

    if (nlh->nlmsg_type == RTM_DELLINK)
    {
        info.flags |= InterfaceChangeType::Deleted;
        {
            ScopedLock<Mutex> lock (_ifMutex);
            auto it = _interfaces.find (ifi->ifi_index);
            if (it != _interfaces.end ())
            {
                info.interface = it->second;
                _interfaces.erase (it);
            }
        }
        notifyLinkUpdate (info);
        return;
    }

    Interface::Ptr iface = acquire (ifi->ifi_index, info);

    {
        ScopedLock<Mutex> lock (iface->_mutex);

        if ((iface->_flags & IFF_UP) != (ifi->ifi_flags & IFF_UP))
        {
            info.flags |= InterfaceChangeType::AdminStateChanged;
        }

        if ((iface->_flags & IFF_RUNNING) != (ifi->ifi_flags & IFF_RUNNING))
        {
            info.flags |= InterfaceChangeType::OperStateChanged;
        }

        iface->_flags = ifi->ifi_flags;

        while (RTA_OK (rta, len))
        {
            switch (rta->rta_type)
            {
                case IFLA_ADDRESS:
                    info.flags |=
                        updateValue (iface->_mac, MacAddress (reinterpret_cast<uint8_t*> (RTA_DATA (rta)), IFHWADDRLEN),
                                     InterfaceChangeType::MacChanged);
                    break;

                case IFLA_IFNAME:
                    info.flags |= updateValue (iface->_name, std::string (reinterpret_cast<char*> (RTA_DATA (rta))),
                                               InterfaceChangeType::NameChanged);
                    break;

                case IFLA_MTU:
                    info.flags |= updateValue (iface->_mtu, *reinterpret_cast<uint32_t*> (RTA_DATA (rta)),
                                               InterfaceChangeType::MtuChanged);
                    break;

                case IFLA_LINKINFO:
                    onLinkInfoMessage (iface, rta, info.flags);
                    break;

                case IFLA_MASTER:
                    info.flags |= updateValue (iface->_master, *reinterpret_cast<uint32_t*> (RTA_DATA (rta)),
                                               InterfaceChangeType::MasterChanged);
                    break;

                default:
                    break;
            }

            rta = RTA_NEXT (rta, len);
        }
    }

    notifyLinkUpdate (info);
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : onLinkInfoMessage
// =========================================================================
void InterfaceManager::onLinkInfoMessage (Interface::Ptr& iface, struct rtattr* rta, InterfaceChangeType& flags)
{
    struct rtattr* attr = reinterpret_cast<struct rtattr*> (RTA_DATA (rta));
    int len             = RTA_PAYLOAD (rta);

    while (RTA_OK (attr, len))
    {
        switch (attr->rta_type)
        {
            case IFLA_INFO_KIND:
                flags |= updateValue (iface->_kind, std::string (reinterpret_cast<char*> (RTA_DATA (attr))),
                                      InterfaceChangeType::KindChanged);
                break;

            default:
                break;
        }

        attr = RTA_NEXT (attr, len);
    }
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : onAddressMessage
// =========================================================================
void InterfaceManager::onAddressMessage (struct nlmsghdr* nlh)
{
    struct ifaddrmsg* ifa = reinterpret_cast<struct ifaddrmsg*> (NLMSG_DATA (nlh));
    struct rtattr* rta    = IFA_RTA (ifa);
    int len               = IFA_PAYLOAD (nlh);

    socklen_t addrlen = (ifa->ifa_family == AF_INET6) ? IpAddress::ipv6Length : IpAddress::ipv4Length;

    AddressInfo info{};
    std::get<1> (info.address) = ifa->ifa_prefixlen;
    std::get<2> (info.address) = IpAddress (ifa->ifa_family);

    while (RTA_OK (rta, len))
    {
        switch (rta->rta_type)
        {
            case IFA_LOCAL:
                std::get<0> (info.address) = IpAddress (RTA_DATA (rta), addrlen, ifa->ifa_index);
                break;

            case IFA_ADDRESS:
                if (std::get<0> (info.address).isWildcard ())
                {
                    std::get<0> (info.address) = IpAddress (RTA_DATA (rta), addrlen, ifa->ifa_index);
                }
                break;

            case IFA_BROADCAST:
                std::get<2> (info.address) = IpAddress (RTA_DATA (rta), addrlen);
                break;

            default:
                break;
        }

        rta = RTA_NEXT (rta, len);
    }

    info.interface = findByIndex (ifa->ifa_index);
    if (!info.interface)
    {
        return;
    }

    {
        ScopedLock<Mutex> lock (info.interface->_mutex);

        if (nlh->nlmsg_type == RTM_NEWADDR)
        {
            auto it = std::find_if (info.interface->_addresses.begin (), info.interface->_addresses.end (),
                                    [&] (const Interface::Address& a) {
                                        return std::get<0> (a) == std::get<0> (info.address);
                                    });

            if (it != info.interface->_addresses.end ())
            {
                std::get<1> (*it) = std::get<1> (info.address);
                std::get<2> (*it) = std::get<2> (info.address);
                info.flags |= InterfaceChangeType::Modified;
            }
            else
            {
                info.interface->_addresses.push_back (info.address);
                info.flags |= InterfaceChangeType::Added;
            }
        }
        else
        {
            auto it = std::remove_if (info.interface->_addresses.begin (), info.interface->_addresses.end (),
                                      [&] (const Interface::Address& a) {
                                          return std::get<0> (a) == std::get<0> (info.address);
                                      });

            info.interface->_addresses.erase (it, info.interface->_addresses.end ());
            info.flags |= InterfaceChangeType::Deleted;
        }
    }

    notifyAddressUpdate (info);
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : onRouteMessage
// =========================================================================
void InterfaceManager::onRouteMessage (struct nlmsghdr* nlh)
{
    struct rtmsg* rtm  = reinterpret_cast<struct rtmsg*> (NLMSG_DATA (nlh));
    struct rtattr* rta = RTM_RTA (rtm);
    int len            = RTM_PAYLOAD (nlh);

    socklen_t addrlen = (rtm->rtm_family == AF_INET6) ? IpAddress::ipv6Length : IpAddress::ipv4Length;

    RouteInfo info{};
    std::get<1> (info.route) = rtm->rtm_dst_len;
    uint32_t index           = 0;

    while (RTA_OK (rta, len))
    {
        switch (rta->rta_type)
        {
            case RTA_DST:
                std::get<0> (info.route) = IpAddress (RTA_DATA (rta), addrlen);
                break;

            case RTA_GATEWAY:
                std::get<2> (info.route) = IpAddress (RTA_DATA (rta), addrlen);
                break;

            case RTA_PRIORITY:
                std::get<3> (info.route) = *reinterpret_cast<uint32_t*> (RTA_DATA (rta));
                break;

            case RTA_OIF:
                index = *reinterpret_cast<uint32_t*> (RTA_DATA (rta));
                break;

            default:
                break;
        }

        rta = RTA_NEXT (rta, len);
    }

    info.interface = findByIndex (index);
    if (!info.interface)
    {
        return;
    }

    {
        ScopedLock<Mutex> lock (info.interface->_mutex);

        if (nlh->nlmsg_type == RTM_NEWROUTE)
        {
            auto it = std::find_if (
                info.interface->_routes.begin (), info.interface->_routes.end (), [&] (const Interface::Route& r) {
                    return std::get<0> (r) == std::get<0> (info.route) && std::get<1> (r) == std::get<1> (info.route) &&
                           std::get<2> (r) == std::get<2> (info.route);
                });

            if (it != info.interface->_routes.end ())
            {
                std::get<3> (*it) = std::get<3> (info.route);
                info.flags |= InterfaceChangeType::Modified;
            }
            else
            {
                info.interface->_routes.push_back (info.route);
                info.flags |= InterfaceChangeType::Added;
            }
        }
        else
        {
            auto it = std::remove_if (
                info.interface->_routes.begin (), info.interface->_routes.end (), [&] (const Interface::Route& r) {
                    return std::get<0> (r) == std::get<0> (info.route) && std::get<1> (r) == std::get<1> (info.route) &&
                           std::get<2> (r) == std::get<2> (info.route);
                });

            info.interface->_routes.erase (it, info.interface->_routes.end ());
            info.flags |= InterfaceChangeType::Deleted;
        }
    }

    notifyRouteUpdate (info);
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : notifyLinkUpdate
// =========================================================================
void InterfaceManager::notifyLinkUpdate (const LinkInfo& info)
{
    for (auto& listener : _linkListeners)
    {
        if (listener.second)
        {
            listener.second (info);
        }
    }
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : notifyAddressUpdate
// =========================================================================
void InterfaceManager::notifyAddressUpdate (const AddressInfo& info)
{
    for (auto& listener : _addressListeners)
    {
        if (listener.second)
        {
            listener.second (info);
        }
    }
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : notifyRouteUpdate
// =========================================================================
void InterfaceManager::notifyRouteUpdate (const RouteInfo& info)
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
//   CLASS     : InterfaceManager
//   METHOD    : acquire
// =========================================================================
Interface::Ptr InterfaceManager::acquire (uint32_t index, LinkInfo& info)
{
    ScopedLock<Mutex> lock (_ifMutex);

    auto it = _interfaces.find (index);
    if (it != _interfaces.end ())
    {
        info.interface = it->second;
        return it->second;
    }

    Interface::Ptr iface (new Interface (this, index));
    _interfaces[index] = iface;
    info.interface     = iface;
    info.flags |= InterfaceChangeType::Added;

    return iface;
}
