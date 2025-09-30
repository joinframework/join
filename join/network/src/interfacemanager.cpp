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
#include <join/reactor.hpp>

// C++.
#include <iostream>
#include <mutex>

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
InterfaceManager::InterfaceManager()
: _buffer (std::make_unique <char []> (_bufferSize))
, _seq (0)
{
    if (open (Netlink::rt ()) == -1)
    {
        throw std::runtime_error ("failed to open netlink socket");
    }

    if (bind (RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR | RTMGRP_IPV4_ROUTE | RTMGRP_IPV6_ROUTE) == -1)
    {
        close ();
        throw std::runtime_error ("failed to bind netlink socket");
    }

    if (Reactor::instance ()->addHandler (this) == -1)
    {
        close ();
        throw std::runtime_error ("failed to handle netlink socket events");
    }
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : ~InterfaceManager
// =========================================================================
InterfaceManager::~InterfaceManager ()
{
    Reactor::instance ()->delHandler (this);
    close ();
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : instance
// =========================================================================
InterfaceManager* InterfaceManager::instance ()
{
    static std::once_flag initialized;
    static InterfaceManager manager;

    std::call_once (initialized, [] () {
        manager.refresh (true);
    });

    return &manager;
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : findByIndex
// =========================================================================
Interface::Ptr InterfaceManager::findByIndex (uint32_t interfaceIndex)
{
    ScopedLock lock (_ifMutex);

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
    ScopedLock lock (_ifMutex);

    InterfaceList ifaces;

    for (auto& interface : _interfaces)
    {
        ifaces.insert (interface.second);
    }

    return ifaces;
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : refresh
// =========================================================================
int InterfaceManager::refresh (bool sync)
{
    if (dumpLink (sync) != 0 || dumpAddress (sync) != 0 || dumpRoute (sync) != 0)
    {
        return -1;
    }

    return 0;
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : addLinkListener
// =========================================================================
void InterfaceManager::addLinkListener (const LinkNotify& cb)
{
    ScopedLock lock (_linkMutex);
    _linkListeners.push_back (cb);
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : removeLinkListener
// =========================================================================
void InterfaceManager::removeLinkListener (const LinkNotify& cb)
{
    ScopedLock lock (_linkMutex);
    auto it = std::remove_if (_linkListeners.begin(), _linkListeners.end(),
        [&] (const LinkNotify& existing) {
            return existing.target_type () == cb.target_type () &&
                   existing.target <void (int)> () == cb.target <void (int)> ();
        });
    _linkListeners.erase (it, _linkListeners.end ());
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : addAddressListener
// =========================================================================
void InterfaceManager::addAddressListener (const AddressNotify& cb)
{
    ScopedLock lock (_addressMutex);
    _addressListeners.push_back (cb);
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : removeAddressListener
// =========================================================================
void InterfaceManager::removeAddressListener (const AddressNotify& cb)
{
    ScopedLock lock (_addressMutex);
    auto it = std::remove_if (_addressListeners.begin(), _addressListeners.end(),
        [&] (const AddressNotify& existing) {
            return existing.target_type () == cb.target_type () &&
                   existing.target <void (int)> () == cb.target <void (int)> ();
        });
    _addressListeners.erase (it, _addressListeners.end ());
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : addRouteListener
// =========================================================================
void InterfaceManager::addRouteListener (const RouteNotify& cb)
{
    ScopedLock lock (_routeMutex);
    _routeListeners.push_back (cb);
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : removeRouteListener
// =========================================================================
void InterfaceManager::removeRouteListener (const RouteNotify& cb)
{
    ScopedLock lock (_routeMutex);
    auto it = std::remove_if (_routeListeners.begin(), _routeListeners.end(),
        [&] (const RouteNotify& existing) {
            return existing.target_type () == cb.target_type () &&
                   existing.target <void (int)> () == cb.target <void (int)> ();
        });
    _routeListeners.erase (it, _routeListeners.end ());
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : createDummyInterface
// =========================================================================
int InterfaceManager::createDummyInterface (const std::string& interfaceName, bool sync)
{
    char buffer[_bufferSize];
    memset (buffer, 0, sizeof (buffer));

    // netlink header.
    struct nlmsghdr *nlh = reinterpret_cast <struct nlmsghdr *> (buffer);
    nlh->nlmsg_len = NLMSG_LENGTH (sizeof (struct ifinfomsg));
    nlh->nlmsg_type = RTM_NEWLINK;
    nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE | NLM_F_EXCL;
    nlh->nlmsg_seq = ++_seq;

    // info message.
    struct ifinfomsg *ifi = reinterpret_cast <struct ifinfomsg*> (NLMSG_DATA (nlh));
    ifi->ifi_family = AF_UNSPEC;

    // set interface name
    addAttributes (nlh, IFLA_IFNAME, interfaceName.c_str (), interfaceName.length () + 1);

    // start nested link info attributes
    struct rtattr *linkinfo = startNestedAttributes (nlh, IFLA_LINKINFO);

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
    char buffer[_bufferSize];
    memset (buffer, 0, sizeof (buffer));

    // netlink header.
    struct nlmsghdr *nlh = reinterpret_cast <struct nlmsghdr *> (buffer);
    nlh->nlmsg_len = NLMSG_LENGTH (sizeof (struct ifinfomsg));
    nlh->nlmsg_type = RTM_NEWLINK;
    nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE | NLM_F_EXCL;
    nlh->nlmsg_seq = ++_seq;

    // info message.
    struct ifinfomsg *ifi = reinterpret_cast <struct ifinfomsg*> (NLMSG_DATA (nlh));
    ifi->ifi_family = AF_UNSPEC;

    // set interface name.
    addAttributes (nlh, IFLA_IFNAME, interfaceName.c_str (), interfaceName.length () + 1);

    // start nested link info attributes.
    struct rtattr *linkinfo = startNestedAttributes (nlh, IFLA_LINKINFO);

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
int InterfaceManager::createVlanInterface (const std::string& interfaceName, uint32_t parentIndex, uint16_t id, uint16_t proto, bool sync)
{
    if ((id == reservedVlanId) || (id > maxVlanId))
    {
        lastError = make_error_code (Errc::InvalidParam);
        return -1;
    }

    char buffer[_bufferSize];
    memset (buffer, 0, sizeof (buffer));

    // netlink header.
    struct nlmsghdr *nlh = reinterpret_cast <struct nlmsghdr *> (buffer);
    nlh->nlmsg_len = NLMSG_LENGTH (sizeof (struct ifinfomsg));
    nlh->nlmsg_type = RTM_NEWLINK;
    nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE | NLM_F_EXCL;
    nlh->nlmsg_seq = ++_seq;

    // info message.
    struct ifinfomsg *ifi = reinterpret_cast <struct ifinfomsg*> (NLMSG_DATA (nlh));
    ifi->ifi_family = AF_UNSPEC;

    // set interface name.
    addAttributes (nlh, IFLA_IFNAME, interfaceName.c_str (), interfaceName.length () + 1);

    // set parent interface link.
    addAttributes (nlh, IFLA_LINK, &parentIndex, sizeof (uint32_t));

    // start nested link info attributes.
    struct rtattr *linkinfo = startNestedAttributes (nlh, IFLA_LINKINFO);

    // add interface kind.
    std::string kind = "vlan";
    addAttributes (nlh, IFLA_INFO_KIND, kind.c_str (), kind.size () + 1);

    // start nested link info data.
    struct rtattr *data = startNestedAttributes (nlh, IFLA_INFO_DATA);

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
int InterfaceManager::createVlanInterface (const std::string& interfaceName, const std::string& parentName, uint16_t id, uint16_t proto, bool sync)
{
    return createVlanInterface (interfaceName, if_nametoindex (parentName.c_str ()), id, proto, sync);
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : createVethInterface
// =========================================================================
int InterfaceManager::createVethInterface (const std::string& hostName, const std::string& peerName, pid_t* pid, bool sync)
{
    char buffer[_bufferSize];
    memset (buffer, 0, sizeof (buffer));

    // netlink header.
    struct nlmsghdr *nlh = reinterpret_cast <struct nlmsghdr *> (buffer);
    nlh->nlmsg_len = NLMSG_LENGTH (sizeof (struct ifinfomsg));
    nlh->nlmsg_type = RTM_NEWLINK;
    nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE | NLM_F_EXCL;
    nlh->nlmsg_seq = ++_seq;

    // info message.
    struct ifinfomsg *ifi = reinterpret_cast <struct ifinfomsg*> (NLMSG_DATA (nlh));
    ifi->ifi_family = AF_UNSPEC;

    // add pid if specified.
    if (pid && (*pid > 0))
    {
        addAttributes (nlh, IFLA_NET_NS_PID, pid, sizeof (pid_t));
    }

    // add host interface name.
    addAttributes (nlh, IFLA_IFNAME, hostName.c_str (), hostName.length () + 1);

    // start nested link info attributes.
    struct rtattr *linkinfo = startNestedAttributes (nlh, IFLA_LINKINFO);

    // add info kind.
    std::string kind = "veth";
    addAttributes (nlh, IFLA_INFO_KIND, kind.c_str (), kind.size () + 1);

    // start nested link info data.
    struct rtattr *data = startNestedAttributes (nlh, IFLA_INFO_DATA);

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
    if(localAddress.family () != remoteAddress.family ())
    {
        lastError = make_error_code (Errc::InvalidParam);
        return -1;
    }

    char buffer[_bufferSize];
    memset (buffer, 0, sizeof (buffer));

    // netlink header.
    struct nlmsghdr *nlh = reinterpret_cast <struct nlmsghdr *> (buffer);
    nlh->nlmsg_len = NLMSG_LENGTH (sizeof (struct ifinfomsg));
    nlh->nlmsg_type = RTM_NEWLINK;
    nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE | NLM_F_EXCL;
    nlh->nlmsg_seq = ++_seq;

    // interface informations.
    struct ifinfomsg *ifi = reinterpret_cast <struct ifinfomsg*> (NLMSG_DATA (nlh));
    ifi->ifi_family = AF_UNSPEC;

    // set tunnel name.
    addAttributes (nlh, IFLA_IFNAME, tunnelName.c_str (), tunnelName.length () + 1);

    // start nested link info attributes.
    struct rtattr * linkinfo = startNestedAttributes (nlh, IFLA_LINKINFO);

    // add info kind.
    std::string kind = localAddress.isIpv4Address () ? "gre" : "ip6gre";
    addAttributes (nlh, IFLA_INFO_KIND, kind.c_str (), kind.length () + 1);

    // start nested link info data.
    struct rtattr * data = startNestedAttributes (nlh, IFLA_INFO_DATA);

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
    return createGreInterface (tunnelName, if_nametoindex (parentName.c_str ()), localAddress, remoteAddress, ikey, okey, ttl, sync);
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : removeInterface
// =========================================================================
int InterfaceManager::removeInterface (uint32_t interfaceIndex, bool sync)
{
    char buffer[_bufferSize];
    memset (buffer, 0, sizeof (buffer));

    // netlink header.
    struct nlmsghdr *nlh = reinterpret_cast <struct nlmsghdr *> (buffer);
    nlh->nlmsg_len = NLMSG_LENGTH (sizeof (struct ifinfomsg));
    nlh->nlmsg_type = RTM_DELLINK;
    nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    nlh->nlmsg_seq = ++_seq;

    // info message.
    struct ifinfomsg *ifi = reinterpret_cast <struct ifinfomsg*> (NLMSG_DATA (nlh));
    ifi->ifi_family = AF_UNSPEC;
    ifi->ifi_index = interfaceIndex;

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
//   METHOD    : addAttributes
// =========================================================================
void InterfaceManager::addAttributes (struct nlmsghdr *nlh, int type, const void *data, int alen)
{
    int len = RTA_LENGTH (alen);
    struct rtattr *rta = reinterpret_cast <struct rtattr *> (reinterpret_cast <char *> (nlh) + NLMSG_ALIGN (nlh->nlmsg_len));
    rta->rta_type = type;
    rta->rta_len = len;
    memcpy (RTA_DATA (rta), data, alen);
    nlh->nlmsg_len = NLMSG_ALIGN (nlh->nlmsg_len) + RTA_ALIGN (len);
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : startNestedAttributes
// =========================================================================
struct rtattr* InterfaceManager::startNestedAttributes (struct nlmsghdr *nlh, int type)
{
    struct rtattr *nested = reinterpret_cast <struct rtattr *> (reinterpret_cast <char *> (nlh) + NLMSG_ALIGN (nlh->nlmsg_len));
    addAttributes (nlh, type, nullptr, 0);
    return nested;
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : stopNestedAttributes
// =========================================================================
int InterfaceManager::stopNestedAttributes (struct nlmsghdr *nlh, struct rtattr *nested)
{
    nested->rta_len = reinterpret_cast <char *> (nlh) + NLMSG_ALIGN (nlh->nlmsg_len) - reinterpret_cast <char *> (nested);
    return nlh->nlmsg_len;
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : addPeerInfoData
// =========================================================================
void InterfaceManager::addPeerInfoData (struct nlmsghdr *nlh, const std::string& peerName)
{
    // start nested peer info data.
    struct rtattr *peerinfo = startNestedAttributes (nlh, VETH_INFO_PEER);

    // peer info message.
    struct ifinfomsg *ifi = reinterpret_cast <struct ifinfomsg *> (reinterpret_cast <char *> (nlh) + nlh->nlmsg_len);
    ifi->ifi_family = AF_UNSPEC;

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
    char buffer[_bufferSize];
    memset (buffer, 0, sizeof (buffer));

    // netlink header.
    struct nlmsghdr *nlh = reinterpret_cast <struct nlmsghdr *> (buffer);
    nlh->nlmsg_len = NLMSG_LENGTH (sizeof (struct ifinfomsg));
    nlh->nlmsg_type = RTM_SETLINK;
    nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    nlh->nlmsg_seq = ++_seq;

    // info message.
    struct ifinfomsg *ifi = reinterpret_cast <struct ifinfomsg*> (NLMSG_DATA (nlh));
    ifi->ifi_family = AF_UNSPEC;
    ifi->ifi_index = interfaceIndex;

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
    char buffer[_bufferSize];
    memset (buffer, 0, sizeof (buffer));

    // netlink header.
    struct nlmsghdr *nlh = reinterpret_cast <struct nlmsghdr *> (buffer);
    nlh->nlmsg_len = NLMSG_LENGTH (sizeof (struct ifinfomsg));
    nlh->nlmsg_type = RTM_SETLINK;
    nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    nlh->nlmsg_seq = ++_seq;

    // info message.
    struct ifinfomsg *ifi = reinterpret_cast <struct ifinfomsg*> (NLMSG_DATA (nlh));
    ifi->ifi_family = AF_UNSPEC;
    ifi->ifi_index = interfaceIndex;

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
    char buffer[_bufferSize];
    memset (buffer, 0, sizeof(buffer));

    // netlink header.
    struct nlmsghdr *nlh = reinterpret_cast <struct nlmsghdr *> (buffer);
    nlh->nlmsg_len = NLMSG_LENGTH (sizeof (struct ifinfomsg));
    nlh->nlmsg_type = RTM_SETLINK;
    nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    nlh->nlmsg_seq = ++_seq;

    struct ifinfomsg* ifi = reinterpret_cast<struct ifinfomsg*>(NLMSG_DATA(nlh));
    ifi->ifi_family = AF_UNSPEC;
    ifi->ifi_index = interfaceIndex;

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
    char buffer[_bufferSize];
    memset (buffer, 0, sizeof (buffer));

    // netlink header.
    struct nlmsghdr *nlh = reinterpret_cast <struct nlmsghdr *> (buffer);
    nlh->nlmsg_len = NLMSG_LENGTH (sizeof (struct ifinfomsg));
    nlh->nlmsg_type = RTM_SETLINK;
    nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    nlh->nlmsg_seq = ++_seq;

    // info message.
    struct ifinfomsg *ifi = reinterpret_cast <struct ifinfomsg*> (NLMSG_DATA (nlh));
    ifi->ifi_family = AF_UNSPEC;
    ifi->ifi_index = interfaceIndex;
    ifi->ifi_flags = enabled ? IFF_UP : 0;
    ifi->ifi_change = IFF_UP;

    // send request.
    return sendRequest (nlh, sync);
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : addAddress
// =========================================================================
int InterfaceManager::addAddress (uint32_t interfaceIndex, const IpAddress& ipAddress, uint32_t prefix, const IpAddress& broadcast, bool sync)
{
    char buffer[_bufferSize];
    memset (buffer, 0, sizeof (buffer));

    // netlink header.
    struct nlmsghdr *nlh = reinterpret_cast <struct nlmsghdr *> (buffer);
    nlh->nlmsg_len = NLMSG_LENGTH (sizeof (struct ifaddrmsg));
    nlh->nlmsg_type = RTM_NEWADDR;
    nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE | NLM_F_EXCL;
    nlh->nlmsg_seq = ++_seq;

    // address message.
    struct ifaddrmsg *ifa = reinterpret_cast <struct ifaddrmsg*> (NLMSG_DATA (nlh));
    ifa->ifa_family = ipAddress.family ();
    ifa->ifa_prefixlen = prefix;
    ifa->ifa_index = interfaceIndex;

    // add local address.
    addAttributes (nlh, IFA_LOCAL, ipAddress.addr (), ipAddress.length ());

    // add address.
    addAttributes (nlh, IFA_ADDRESS, ipAddress.addr (), ipAddress.length ());

    // add broadcast address if specified.
    if (ipAddress.family () == AF_INET && broadcast.isBroadcast ())
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
int InterfaceManager::removeAddress (uint32_t interfaceIndex, const IpAddress& ipAddress, uint32_t prefix, const IpAddress& broadcast, bool sync)
{
    char buffer[_bufferSize];
    memset (buffer, 0, sizeof (buffer));

    // netlink header.
    struct nlmsghdr *nlh = reinterpret_cast <struct nlmsghdr *> (buffer);
    nlh->nlmsg_len = NLMSG_LENGTH (sizeof (struct ifaddrmsg));
    nlh->nlmsg_type = RTM_DELADDR;
    nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    nlh->nlmsg_seq = ++_seq;

    // address message.
    struct ifaddrmsg *ifa = reinterpret_cast <struct ifaddrmsg*> (NLMSG_DATA (nlh));
    ifa->ifa_family = ipAddress.family ();
    ifa->ifa_prefixlen = prefix;
    ifa->ifa_index = interfaceIndex;

    // add local address.
    addAttributes (nlh, IFA_LOCAL, ipAddress.addr (), ipAddress.length ());

    // add address.
    addAttributes (nlh, IFA_ADDRESS, ipAddress.addr (), ipAddress.length ());

    // add broadcast address if specified.
    if (ipAddress.family () == AF_INET && broadcast.isBroadcast ())
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
int InterfaceManager::addRoute (uint32_t interfaceIndex, const IpAddress& dest, uint32_t prefix, const IpAddress& gateway, uint32_t* metric, bool sync)
{
    char buffer[_bufferSize];
    memset (buffer, 0, sizeof (buffer));

    // netlink header.
    struct nlmsghdr *nlh = reinterpret_cast <struct nlmsghdr *> (buffer);
    nlh->nlmsg_len = NLMSG_LENGTH (sizeof (struct rtmsg));
    nlh->nlmsg_type = RTM_NEWROUTE;
    nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE | NLM_F_REPLACE;
    nlh->nlmsg_seq = ++_seq;

    // routing message.
    struct rtmsg *rtm = reinterpret_cast <struct rtmsg*> (NLMSG_DATA (nlh));
    rtm->rtm_family = dest.family ();
    rtm->rtm_dst_len = prefix;
    rtm->rtm_table = RT_TABLE_MAIN;
    rtm->rtm_protocol = RTPROT_STATIC;
    rtm->rtm_scope = RT_SCOPE_UNIVERSE;
    rtm->rtm_type = RTN_UNICAST;

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
int InterfaceManager::removeRoute (uint32_t interfaceIndex, const IpAddress& dest, uint32_t prefix, const IpAddress& gateway, uint32_t* metric, bool sync)
{
    char buffer[_bufferSize];
    memset (buffer, 0, sizeof (buffer));

    // netlink header.
    struct nlmsghdr *nlh = reinterpret_cast <struct nlmsghdr *> (buffer);
    nlh->nlmsg_len = NLMSG_LENGTH (sizeof (struct rtmsg));
    nlh->nlmsg_type = RTM_DELROUTE;
    nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    nlh->nlmsg_seq = ++_seq;

    // routing message.
    struct rtmsg *rtm = reinterpret_cast <struct rtmsg*> (NLMSG_DATA (nlh));
    rtm->rtm_family = dest.family ();
    rtm->rtm_dst_len = prefix;
    rtm->rtm_table = RT_TABLE_MAIN;
    rtm->rtm_protocol = RTPROT_STATIC;
    rtm->rtm_scope = RT_SCOPE_UNIVERSE;
    rtm->rtm_type = RTN_UNICAST;

    // add destination.
    addAttributes (nlh, RTA_DST, dest.addr (), dest.length ());

    // add gateway if specified.
    if (!gateway.isWildcard ())
    {
        addAttributes(nlh, RTA_GATEWAY, gateway.addr (), gateway.length ()); 
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
    char buffer[_bufferSize];
    memset (buffer, 0, sizeof (buffer));

    // netlink header.
    struct nlmsghdr *nlh = reinterpret_cast <struct nlmsghdr *> (buffer);
    nlh->nlmsg_len = NLMSG_LENGTH (sizeof (struct rtgenmsg));
    nlh->nlmsg_type = RTM_GETLINK;
    nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
    nlh->nlmsg_seq = ++_seq;

    // general message.
    struct rtgenmsg *rtgen = reinterpret_cast <struct rtgenmsg*> (NLMSG_DATA (nlh));
    rtgen->rtgen_family = AF_UNSPEC;

    // send request.
    return sendRequest (nlh, sync);
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : dumpAddress
// =========================================================================
int InterfaceManager::dumpAddress (bool sync)
{
    char buffer[_bufferSize];
    memset (buffer, 0, sizeof (buffer));

    // netlink header.
    struct nlmsghdr *nlh = reinterpret_cast <struct nlmsghdr *> (buffer);
    nlh->nlmsg_len = NLMSG_LENGTH (sizeof (struct rtgenmsg));
    nlh->nlmsg_type = RTM_GETADDR;
    nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
    nlh->nlmsg_seq = ++_seq;

    // general message.
    struct rtgenmsg *rtgen = reinterpret_cast <struct rtgenmsg*> (NLMSG_DATA (nlh));
    rtgen->rtgen_family = AF_UNSPEC;

    // send request.
    return sendRequest (nlh, sync);
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : dumpRoute
// =========================================================================
int InterfaceManager::dumpRoute (bool sync)
{
    char buffer[_bufferSize];
    memset (buffer, 0, sizeof (buffer));

    // netlink header.
    struct nlmsghdr *nlh = reinterpret_cast <struct nlmsghdr *> (buffer);
    nlh->nlmsg_len = NLMSG_LENGTH (sizeof (struct rtgenmsg));
    nlh->nlmsg_type = RTM_GETROUTE;
    nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
    nlh->nlmsg_seq = ++_seq;

    // general message.
    struct rtgenmsg *rtgen = reinterpret_cast <struct rtgenmsg*> (NLMSG_DATA (nlh));
    rtgen->rtgen_family = AF_UNSPEC;

    // send request.
    return sendRequest (nlh, sync);
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : sendRequest
// =========================================================================
int InterfaceManager::sendRequest (struct nlmsghdr* nlh, bool sync)
{
    ScopedLock lock (_syncMutex);

    int sent = write (reinterpret_cast <const char *> (nlh), nlh->nlmsg_len);
    if (sent < 0)
    {
        return -1;
    }

    if (sync)
    {
        return waitResponse (lock, nlh->nlmsg_seq);
    }

    return 0;
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : waitResponse
// =========================================================================
int InterfaceManager::waitResponse (ScopedLock& lock, uint32_t seq, uint32_t timeout)
{
    auto inserted = _pending.emplace (seq, std::make_shared <PendingRequest> ());
    if (!inserted.second)
    {
        lastError = make_error_code (Errc::OperationFailed);
        return -1;
    }

    if (!inserted.first->second->cond.timedWait (lock, std::chrono::milliseconds (timeout)))
    {
        _pending.erase (inserted.first);
        lastError = make_error_code (Errc::TimedOut);
        return -1;
    }

    if (inserted.first->second->error != 0)
    {
        _pending.erase (inserted.first);
        lastError = std::make_error_code (static_cast <std::errc> (inserted.first->second->error));
        return -1;
    }

    _pending.erase (inserted.first);

    return 0;
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : onReceive
// =========================================================================
void InterfaceManager::onReceive ()
{
    ssize_t len = read (_buffer.get (), _bufferSize);
    if (len != -1)
    {
        struct nlmsghdr *nlh = reinterpret_cast <struct nlmsghdr *> (_buffer.get ());
        while (NLMSG_OK (nlh, len))
        {
            if (nlh->nlmsg_type == NLMSG_DONE)
            {
                notifyRequest (nlh->nlmsg_seq, 0);
                break;
            }

            if (nlh->nlmsg_type == NLMSG_ERROR)
            {
                struct nlmsgerr* err = static_cast <struct nlmsgerr*> (NLMSG_DATA (nlh));
                notifyRequest (err->msg.nlmsg_seq, -err->error);
                nlh = NLMSG_NEXT (nlh, len);
                continue;
            }

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

            nlh = NLMSG_NEXT (nlh, len);
        }
    }
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : onLinkMessage
// =========================================================================
void InterfaceManager::onLinkMessage (struct nlmsghdr* nlh)
{
    struct ifinfomsg* ifi = reinterpret_cast <struct ifinfomsg*> (NLMSG_DATA (nlh));
    if (!ifi || (ifi->ifi_family == AF_BRIDGE))
    {
        return;
    }

    LinkInfo info = {};
    info.index = ifi->ifi_index;

    struct rtattr* rta = IFLA_RTA (ifi);
    int len = IFLA_PAYLOAD (nlh);

    if (nlh->nlmsg_type == RTM_DELLINK)
    {
        info.flags |= ChangeType::Deleted;
        notifyLinkUpdate (info);
        ScopedLock lock (_ifMutex);
        _interfaces.erase (info.index);
        return;
    }

    Interface::Ptr iface = acquire (info);

    iface->_mutex.lock ();

    if ((iface->_flags & IFF_UP) != (ifi->ifi_flags & IFF_UP))
    {
        info.flags |= ChangeType::AdminStateChanged;
    }

    if ((iface->_flags & IFF_RUNNING) != (ifi->ifi_flags & IFF_RUNNING))
    {
        info.flags |= ChangeType::OperStateChanged;
    }

    iface->_flags = ifi->ifi_flags;

    while (RTA_OK (rta, len))
    {
        switch (rta->rta_type)
        {
            case IFLA_ADDRESS:
                info.flags |= updateValue (iface->_mac, MacAddress (reinterpret_cast <uint8_t *> (RTA_DATA (rta)), IFHWADDRLEN), ChangeType::MacChanged);
                break;

            case IFLA_IFNAME:
                info.flags |= updateValue (iface->_name, std::string (reinterpret_cast <char *> (RTA_DATA (rta))), ChangeType::NameChanged);
                break;

            case IFLA_MTU:
                info.flags |= updateValue (iface->_mtu, *reinterpret_cast <uint32_t *> (RTA_DATA (rta)), ChangeType::MtuChanged);
                break;

            case IFLA_LINKINFO:
                onLinkInfoMessage (iface, rta, info.flags);
                break;

            case IFLA_MASTER:
                info.flags |= updateValue (iface->_master, *reinterpret_cast <uint32_t *> (RTA_DATA (rta)), ChangeType::MasterChanged);
                break;

            default:
                break;
        }

        rta = RTA_NEXT (rta, len);
    }

    iface->_mutex.unlock ();

    notifyLinkUpdate (info);
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : onLinkInfoMessage
// =========================================================================
void InterfaceManager::onLinkInfoMessage (Interface::Ptr& iface, struct rtattr* rta, ChangeType& flags)
{
    struct rtattr* attr = reinterpret_cast <struct rtattr*> (RTA_DATA (rta));
    int len = RTA_PAYLOAD (rta);

    while (RTA_OK (attr, len))
    {
        switch (attr->rta_type)
        {
            case IFLA_INFO_KIND:
                flags |= updateValue (iface->_kind, std::string (reinterpret_cast <char *> (RTA_DATA (attr))), ChangeType::KindChanged);
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
    struct ifaddrmsg* ifa = reinterpret_cast <struct ifaddrmsg*> (NLMSG_DATA (nlh));
    if (!ifa)
    {
        return;
    }

    AddressInfo info = {};
    info.index = ifa->ifa_index;

    struct rtattr* rta = IFA_RTA (ifa);
    int len = IFA_PAYLOAD (nlh);

    socklen_t addrlen = (ifa->ifa_family == AF_INET6) ? IpAddress::ipv6Length 
                                                      : IpAddress::ipv4Length;

    std::get <1> (info.address) = ifa->ifa_prefixlen;
    std::get <2> (info.address) = IpAddress (ifa->ifa_family);

    while (RTA_OK (rta, len))
    {
        switch (rta->rta_type)
        {
            case IFA_ADDRESS:
            case IFA_LOCAL:
                std::get <0> (info.address) = IpAddress (RTA_DATA (rta), addrlen, ifa->ifa_index);
                break;

            case IFA_BROADCAST:
                std::get <2> (info.address) = IpAddress (RTA_DATA (rta), addrlen);
                break;

            default:
                break;
        }

        rta = RTA_NEXT (rta, len);
    }

    Interface::Ptr iface = findByIndex (info.index);
    if (!iface)
    {
        return;
    }

    iface->_mutex.lock ();

    if (nlh->nlmsg_type == RTM_NEWADDR)
    {
        auto it = std::find_if (iface->_addresses.begin (), iface->_addresses.end (),
            [&] (const Interface::Address& a) {
                return std::get <0> (a) == std::get <0> (info.address);
            }
        );

        if (it != iface->_addresses.end ())
        {
            std::get <1> (*it) = std::get <1> (info.address);
            std::get <2> (*it) = std::get <2> (info.address);
            info.flags |= ChangeType::Modified;
        }
        else
        {
            iface->_addresses.push_back (info.address);
            info.flags |= ChangeType::Added;
        }
    }
    else
    {
        auto it = std::remove_if (iface->_addresses.begin (), iface->_addresses.end (),
            [&] (const Interface::Address& a) {
                return std::get <0> (a) == std::get <0> (info.address);
            }
        );

        iface->_addresses.erase (it, iface->_addresses.end ());
        info.flags |= ChangeType::Deleted;
    }

    iface->_mutex.unlock ();

    notifyAddressUpdate (info);
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : onRouteMessage
// =========================================================================
void InterfaceManager::onRouteMessage (struct nlmsghdr* nlh)
{
    struct rtmsg* rtm = reinterpret_cast <struct rtmsg*> (NLMSG_DATA (nlh));
    if (!rtm)
    {
        return;
    }

    RouteInfo info = {};

    struct rtattr* rta = RTM_RTA (rtm);
    int len = RTM_PAYLOAD (nlh);

    socklen_t addrlen = (rtm->rtm_family == AF_INET6) ? IpAddress::ipv6Length
                                                      : IpAddress::ipv4Length;

    std::get <1> (info.route) = rtm->rtm_dst_len;

    while (RTA_OK (rta, len))
    {
        switch (rta->rta_type)
        {
            case RTA_DST:
                std::get <0> (info.route) = IpAddress (RTA_DATA (rta), addrlen);
                break;

            case RTA_GATEWAY:
                std::get <2> (info.route) = IpAddress (RTA_DATA (rta), addrlen);
                break;

            case RTA_PRIORITY:
                std::get <3> (info.route) = *reinterpret_cast <uint32_t*> (RTA_DATA (rta));
                break;

            case RTA_OIF:
                info.index = *reinterpret_cast <uint32_t*> (RTA_DATA (rta));
                break;

            default:
                break;
        }

        rta = RTA_NEXT (rta, len);
    }

    if (info.index == 0)
    {
        return;
    }

    Interface::Ptr iface = findByIndex (info.index);
    if (!iface)
    {
        return;
    }

    iface->_mutex.lock ();

    if (nlh->nlmsg_type == RTM_NEWROUTE)
    {
        auto it = std::find_if (iface->_routes.begin (), iface->_routes.end (),
            [&] (const Interface::Route& r) {
                return std::get <0> (r) == std::get <0> (info.route) &&
                       std::get <1> (r) == std::get <1> (info.route) &&
                       std::get <2> (r) == std::get <2> (info.route);
            }
        );

        if (it != iface->_routes.end ())
        {
            std::get <3> (*it) = std::get <3> (info.route);
            info.flags |= ChangeType::Modified;
        }
        else
        {
            iface->_routes.push_back (info.route);
            info.flags |= ChangeType::Added;
        }
    }
    else
    {
        auto it = std::remove_if (iface->_routes.begin (), iface->_routes.end (),
            [&] (const Interface::Route& r) {
                return std::get <0> (r) == std::get <0> (info.route) &&
                       std::get <1> (r) == std::get <1> (info.route) &&
                       std::get <2> (r) == std::get <2> (info.route);
            }
        );

        iface->_routes.erase (it, iface->_routes.end ());
        info.flags |= ChangeType::Deleted;
    }

    iface->_mutex.unlock ();

    notifyRouteUpdate (info);
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : notifyRequest
// =========================================================================
void InterfaceManager::notifyRequest(uint32_t seq, int error)
{
    ScopedLock lock (_syncMutex);

    auto it = _pending.find (seq);
    if (it != _pending.end ())
    {
        it->second->error = error;
        it->second->cond.signal ();
    }
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : notifyLinkUpdate
// =========================================================================
void InterfaceManager::notifyLinkUpdate (const LinkInfo& info)
{
    ScopedLock lock (_linkMutex);

    for (auto& listener : _linkListeners) 
    {
        if (listener)
        {
            listener (info);
        }
    }
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : notifyAddressUpdate
// =========================================================================
void InterfaceManager::notifyAddressUpdate (const AddressInfo& info)
{
    ScopedLock lock (_addressMutex);

    for (auto& listener : _addressListeners) 
    {
        if (listener)
        {
            listener (info);
        }
    }
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : notifyRouteUpdate
// =========================================================================
void InterfaceManager::notifyRouteUpdate (const RouteInfo& info)
{
    ScopedLock lock (_routeMutex);

    for (auto& listener : _routeListeners) 
    {
        if (listener)
        {
            listener (info);
        }
    }
}

// =========================================================================
//   CLASS     : InterfaceManager
//   METHOD    : acquire
// =========================================================================
Interface::Ptr InterfaceManager::acquire (LinkInfo& info)
{
    ScopedLock lock (_ifMutex);

    auto it = _interfaces.find (info.index);
    if (it != _interfaces.end ())
    {
        info.flags |= ChangeType::Modified;
        return it->second;
    }

   Interface::Ptr iface (new Interface (this, info.index));
    _interfaces[info.index] = iface;
    info.flags |= ChangeType::Added;

    return iface;
}
