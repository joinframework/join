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
#include <join/neighbormanager.hpp>

// C.
#include <linux/rtnetlink.h>

using join::NeighborManager;
using join::NeighborList;
using join::Neighbor;
using join::MacAddress;
using join::IpAddress;

// =========================================================================
//   CLASS     : NeighborManager
//   METHOD    : NeighborManager
// =========================================================================
NeighborManager::NeighborManager (Reactor* reactor)
: NetlinkManager (RTMGRP_NEIGH, reactor)
{
    start ();
    refresh ();
}

// =========================================================================
//   CLASS     : NeighborManager
//   METHOD    : ~NeighborManager
// =========================================================================
NeighborManager::~NeighborManager ()
{
    stop ();
}

// =========================================================================
//   CLASS     : NeighborManager
//   METHOD    : instance
// =========================================================================
NeighborManager& NeighborManager::instance ()
{
    static NeighborManager manager;
    return manager;
}

// =========================================================================
//   CLASS     : NeighborManager
//   METHOD    : findByIndex
// =========================================================================
Neighbor::Ptr NeighborManager::findByIndex (uint32_t index, const IpAddress& ipAddress)
{
    ScopedLock<Mutex> lock (_neighMutex);

    auto it = _neighbors.find ({index, IpAddress (ipAddress.addr (), ipAddress.length (), index)});
    if (it == _neighbors.end ())
    {
        return nullptr;
    }

    return it->second;
}

// =========================================================================
//   CLASS     : NeighborManager
//   METHOD    : findByName
// =========================================================================
Neighbor::Ptr NeighborManager::findByName (const std::string& interfaceName, const IpAddress& ipAddress)
{
    return findByIndex (if_nametoindex (interfaceName.c_str ()), ipAddress);
}

// =========================================================================
//   CLASS     : NeighborManager
//   METHOD    : enumerate
// =========================================================================
NeighborList NeighborManager::enumerate ()
{
    ScopedLock<Mutex> lock (_neighMutex);

    NeighborList neighbors;
    neighbors.reserve (_neighbors.size ());

    for (auto& neighbor : _neighbors)
    {
        neighbors.push_back (neighbor.second);
    }

    return neighbors;
}

// =========================================================================
//   CLASS     : NeighborManager
//   METHOD    : enumerate
// =========================================================================
NeighborList NeighborManager::enumerate (uint32_t index)
{
    ScopedLock<Mutex> lock (_neighMutex);

    NeighborList neighbors;

    for (auto& neighbor : _neighbors)
    {
        if (neighbor.first._index == index)
        {
            neighbors.push_back (neighbor.second);
        }
    }

    return neighbors;
}

// =========================================================================
//   CLASS     : NeighborManager
//   METHOD    : enumerate
// =========================================================================
NeighborList NeighborManager::enumerate (const std::string& interfaceName)
{
    return enumerate (if_nametoindex (interfaceName.c_str ()));
}

// =========================================================================
//   CLASS     : NeighborManager
//   METHOD    : refresh
// =========================================================================
int NeighborManager::refresh ()
{
    {
        ScopedLock<Mutex> lock (_neighMutex);
        _neighbors.clear ();
    }

    return dumpNeighbors (true);
}

// =========================================================================
//   CLASS     : NeighborManager
//   METHOD    : addNeighborListener
// =========================================================================
uint64_t NeighborManager::addNeighborListener (const NeighborNotify& cb)
{
    uint64_t id = ++_listenerCounter;

    pushJob ([this, id, cb] () {
        _neighborListeners.emplace (id, cb);
    });

    return id;
}

// =========================================================================
//   CLASS     : NeighborManager
//   METHOD    : removeNeighborListener
// =========================================================================
void NeighborManager::removeNeighborListener (uint64_t id)
{
    pushJob ([this, id] () {
        _neighborListeners.erase (id);
    });
}

// =========================================================================
//   CLASS     : NeighborManager
//   METHOD    : addNeighbor
// =========================================================================
int NeighborManager::addNeighbor (uint32_t index, const IpAddress& ipAddress, const MacAddress& macAddress,
                                  uint16_t state, bool sync)
{
    char buffer[_bufferSize] = {};

    // netlink header.
    struct nlmsghdr* nlh = reinterpret_cast<struct nlmsghdr*> (buffer);
    nlh->nlmsg_len       = NLMSG_LENGTH (sizeof (struct ndmsg));
    nlh->nlmsg_type      = RTM_NEWNEIGH;
    nlh->nlmsg_flags     = NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE | NLM_F_EXCL;
    nlh->nlmsg_seq       = ++_seq;

    // neighbor message.
    struct ndmsg* ndm = reinterpret_cast<struct ndmsg*> (NLMSG_DATA (nlh));
    ndm->ndm_family   = ipAddress.family ();
    ndm->ndm_ifindex  = static_cast<int> (index);
    ndm->ndm_state    = state;
    ndm->ndm_flags    = 0;
    ndm->ndm_type     = RTN_UNICAST;

    // ip address.
    addAttributes (nlh, NDA_DST, ipAddress.addr (), ipAddress.length ());

    // mac address.
    addAttributes (nlh, NDA_LLADDR, macAddress.addr (), macAddress.length ());

    return sendRequest (nlh, sync);
}

// =========================================================================
//   CLASS     : NeighborManager
//   METHOD    : addNeighbor
// =========================================================================
int NeighborManager::addNeighbor (const std::string& interfaceName, const IpAddress& ipAddress,
                                  const MacAddress& macAddress, uint16_t state, bool sync)
{
    return addNeighbor (if_nametoindex (interfaceName.c_str ()), ipAddress, macAddress, state, sync);
}

// =========================================================================
//   CLASS     : NeighborManager
//   METHOD    : removeNeighbor
// =========================================================================
int NeighborManager::removeNeighbor (uint32_t index, const IpAddress& ipAddress, bool sync)
{
    char buffer[_bufferSize] = {};

    // netlink header.
    struct nlmsghdr* nlh = reinterpret_cast<struct nlmsghdr*> (buffer);
    nlh->nlmsg_len       = NLMSG_LENGTH (sizeof (struct ndmsg));
    nlh->nlmsg_type      = RTM_DELNEIGH;
    nlh->nlmsg_flags     = NLM_F_REQUEST | NLM_F_ACK;
    nlh->nlmsg_seq       = ++_seq;

    // neighbor message.
    struct ndmsg* ndm = reinterpret_cast<struct ndmsg*> (NLMSG_DATA (nlh));
    ndm->ndm_family   = ipAddress.family ();
    ndm->ndm_ifindex  = static_cast<int> (index);

    // ip address.
    addAttributes (nlh, NDA_DST, ipAddress.addr (), ipAddress.length ());

    return sendRequest (nlh, sync);
}

// =========================================================================
//   CLASS     : NeighborManager
//   METHOD    : removeNeighbor
// =========================================================================
int NeighborManager::removeNeighbor (const std::string& interfaceName, const IpAddress& ipAddress, bool sync)
{
    return removeNeighbor (if_nametoindex (interfaceName.c_str ()), ipAddress, sync);
}

// =========================================================================
//   CLASS     : NeighborManager
//   METHOD    : flushNeighbors
// =========================================================================
int NeighborManager::flushNeighbors (uint32_t index, bool sync)
{
    NeighborList neighbors = enumerate (index);

    int res = 0;
    for (auto& neighbor : neighbors)
    {
        if (removeNeighbor (index, neighbor->ip (), sync) != 0)
        {
            res = -1;
        }
    }

    return res;
}

// =========================================================================
//   CLASS     : NeighborManager
//   METHOD    : flushNeighbors
// =========================================================================
int NeighborManager::flushNeighbors (const std::string& interfaceName, bool sync)
{
    return flushNeighbors (if_nametoindex (interfaceName.c_str ()), sync);
}

// =========================================================================
//   CLASS     : NeighborManager
//   METHOD    : setNeighbor
// =========================================================================
int NeighborManager::setNeighbor (uint32_t index, const IpAddress& ipAddress, const MacAddress& macAddress,
                                  uint16_t state, bool sync)
{
    char buffer[_bufferSize] = {};

    // netlink header.
    struct nlmsghdr* nlh = reinterpret_cast<struct nlmsghdr*> (buffer);
    nlh->nlmsg_len       = NLMSG_LENGTH (sizeof (struct ndmsg));
    nlh->nlmsg_type      = RTM_NEWNEIGH;
    nlh->nlmsg_flags     = NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE | NLM_F_REPLACE;
    nlh->nlmsg_seq       = ++_seq;

    // neighbor message.
    struct ndmsg* ndm = reinterpret_cast<struct ndmsg*> (NLMSG_DATA (nlh));
    ndm->ndm_family   = ipAddress.family ();
    ndm->ndm_ifindex  = static_cast<int> (index);
    ndm->ndm_state    = state;
    ndm->ndm_flags    = 0;
    ndm->ndm_type     = RTN_UNICAST;

    // ip address.
    addAttributes (nlh, NDA_DST, ipAddress.addr (), ipAddress.length ());

    // mac address.
    addAttributes (nlh, NDA_LLADDR, macAddress.addr (), macAddress.length ());

    return sendRequest (nlh, sync);
}

// =========================================================================
//   CLASS     : NeighborManager
//   METHOD    : dumpNeighbors
// =========================================================================
int NeighborManager::dumpNeighbors (bool sync)
{
    char buffer[_bufferSize] = {};

    // netlink header.
    struct nlmsghdr* nlh = reinterpret_cast<struct nlmsghdr*> (buffer);
    nlh->nlmsg_len       = NLMSG_LENGTH (sizeof (struct ndmsg));
    nlh->nlmsg_type      = RTM_GETNEIGH;
    nlh->nlmsg_flags     = NLM_F_REQUEST | NLM_F_DUMP;
    nlh->nlmsg_seq       = ++_seq;

    // request all families.
    struct ndmsg* ndm = reinterpret_cast<struct ndmsg*> (NLMSG_DATA (nlh));
    ndm->ndm_family   = AF_UNSPEC;

    return sendRequest (nlh, sync);
}

// =========================================================================
//   CLASS     : NeighborManager
//   METHOD    : onMessage
// =========================================================================
void NeighborManager::onMessage (struct nlmsghdr* nlh)
{
    switch (nlh->nlmsg_type)
    {
        case RTM_NEWNEIGH:
        case RTM_DELNEIGH:
            onNeighborMessage (nlh);
            break;

        default:
            break;
    }
}

// =========================================================================
//   CLASS     : NeighborManager
//   METHOD    : onNeighborMessage
// =========================================================================
void NeighborManager::onNeighborMessage (struct nlmsghdr* nlh)
{
    struct ndmsg* ndm  = reinterpret_cast<struct ndmsg*> (NLMSG_DATA (nlh));
    struct rtattr* rta = reinterpret_cast<struct rtattr*> (reinterpret_cast<char*> (NLMSG_DATA (nlh)) +
                                                           NLMSG_ALIGN (sizeof (struct ndmsg)));
    int len            = static_cast<int> (NLMSG_PAYLOAD (nlh, sizeof (struct ndmsg)));

    if (ndm->ndm_ifindex <= 0)
    {
        return;
    }

    if (ndm->ndm_family != AF_INET && ndm->ndm_family != AF_INET6)
    {
        return;
    }

    socklen_t addrlen = (ndm->ndm_family == AF_INET6) ? IpAddress::ipv6Length : IpAddress::ipv4Length;

    uint32_t index = static_cast<uint32_t> (ndm->ndm_ifindex);
    IpAddress ipAddress (ndm->ndm_family);
    MacAddress macAddress;

    while (RTA_OK (rta, len))
    {
        switch (rta->rta_type)
        {
            case NDA_DST:
                ipAddress = IpAddress (RTA_DATA (rta), addrlen, index);
                break;

            case NDA_LLADDR:
                macAddress = MacAddress (reinterpret_cast<uint8_t*> (RTA_DATA (rta)), IFHWADDRLEN);
                break;

            default:
                break;
        }

        rta = RTA_NEXT (rta, len);
    }

    if (ipAddress.isWildcard ())
    {
        return;
    }

    if (nlh->nlmsg_type == RTM_DELNEIGH)
    {
        Neighbor::Ptr neighbor;

        {
            ScopedLock<Mutex> lock (_neighMutex);
            auto it = _neighbors.find ({index, ipAddress});
            if (it != _neighbors.end ())
            {
                neighbor = it->second;
                _neighbors.erase (it);
            }
        }

        if (neighbor)
        {
            NeighborInfo info{};
            info.neighbor = neighbor;
            info.flags    = NeighborChangeType::Deleted;
            notifyNeighborUpdate (info);
        }

        return;
    }

    NeighborInfo info{};
    Neighbor::Ptr neighbor = acquire (index, ipAddress, info);

    {
        ScopedLock<Mutex> lock (neighbor->_mutex);
        info.flags |= updateValue (neighbor->_mac, macAddress, NeighborChangeType::MacChanged);
        info.flags |= updateValue (neighbor->_state, ndm->ndm_state, NeighborChangeType::StateChanged);
    }

    notifyNeighborUpdate (info);
}

// =========================================================================
//   CLASS     : NeighborManager
//   METHOD    : notifyNeighborUpdate
// =========================================================================
void NeighborManager::notifyNeighborUpdate (const NeighborInfo& info)
{
    for (auto& listener : _neighborListeners)
    {
        if (listener.second)
        {
            listener.second (info);
        }
    }
}

// =========================================================================
//   CLASS     : NeighborManager
//   METHOD    : acquire
// =========================================================================
Neighbor::Ptr NeighborManager::acquire (uint32_t index, const IpAddress& ipAddress, NeighborInfo& info)
{
    ScopedLock<Mutex> lock (_neighMutex);

    NeighborKey key = {index, ipAddress};

    auto it = _neighbors.find (key);
    if (it != _neighbors.end ())
    {
        info.flags |= NeighborChangeType::Modified;
        info.neighbor = it->second;
        return it->second;
    }

    Neighbor::Ptr neighbor (new Neighbor (this, key));
    _neighbors.emplace (key, neighbor);

    info.flags |= NeighborChangeType::Added;
    info.neighbor = neighbor;

    return neighbor;
}
