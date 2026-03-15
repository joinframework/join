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

// libjoin.
#include <join/error.hpp>
#include <join/arp.hpp>

// C.
#include <cstring>

#ifndef NUD_VALID
#define NUD_VALID (NUD_PERMANENT | NUD_NOARP | NUD_REACHABLE | NUD_PROBE | NUD_STALE | NUD_DELAY)
#endif

using join::MacAddress;
using join::IpAddress;
using join::Raw;
using join::Arp;

// =========================================================================
//   CLASS     : Arp
//   METHOD    : Arp
// =========================================================================
Arp::Arp (const std::string& interface, NeighborManager* neighbors)
: _interface (interface)
, _neighbors (neighbors ? neighbors : &NeighborManager::instance ())
, _reactor (_neighbors->reactor ())
{
}

// =========================================================================
//   CLASS     : Arp
//   METHOD    : add
// =========================================================================
int Arp::add (const MacAddress& mac, const IpAddress& ip)
{
    if (ip.family () != AF_INET)
    {
        lastError = make_error_code (Errc::InvalidParam);
        return -1;
    }

    int index = ::if_nametoindex (_interface.c_str ());
    if (index == 0)
    {
        lastError = std::error_code (errno, std::generic_category ());
        return -1;
    }

    return _neighbors->addNeighbor (index, ip, mac, NUD_PERMANENT, true);
}

// =========================================================================
//   CLASS     : Arp
//   METHOD    : add
// =========================================================================
int Arp::add (const std::string& interface, const MacAddress& mac, const IpAddress& ip)
{
    return Arp (interface).add (mac, ip);
}

// =========================================================================
//   CLASS     : Arp
//   METHOD    : remove
// =========================================================================
int Arp::remove (const IpAddress& ip)
{
    if (ip.family () != AF_INET)
    {
        lastError = make_error_code (Errc::InvalidParam);
        return -1;
    }

    int index = ::if_nametoindex (_interface.c_str ());
    if (index == 0)
    {
        lastError = std::error_code (errno, std::generic_category ());
        return -1;
    }

    return _neighbors->removeNeighbor (index, ip, true);
}

// =========================================================================
//   CLASS     : Arp
//   METHOD    : remove
// =========================================================================
int Arp::remove (const std::string& interface, const IpAddress& ip)
{
    return Arp (interface).remove (ip);
}

// =========================================================================
//   CLASS     : Arp
//   METHOD    : cache
// =========================================================================
MacAddress Arp::cache (const IpAddress& ip)
{
    if (ip.family () != AF_INET)
    {
        lastError = make_error_code (Errc::InvalidParam);
        return {};
    }

    int index = ::if_nametoindex (_interface.c_str ());
    if (index == 0)
    {
        lastError = std::error_code (errno, std::generic_category ());
        return {};
    }

    auto neighbor = _neighbors->findByIndex (index, ip);
    if (!neighbor)
    {
        lastError = std::make_error_code (std::errc::no_such_device_or_address);
        return {};
    }

    if (!(neighbor->state () & NUD_VALID))
    {
        lastError = std::make_error_code (std::errc::no_such_device_or_address);
        return {};
    }

    return neighbor->mac ();
}

// =========================================================================
//   CLASS     : Arp
//   METHOD    : cache
// =========================================================================
MacAddress Arp::cache (const std::string& interface, const IpAddress& ip)
{
    return Arp (interface).cache (ip);
}

// =========================================================================
//   CLASS     : Arp
//   METHOD    : onReceive
// =========================================================================
void Arp::onReceive () noexcept
{
    Packet in{};

    if (read (reinterpret_cast<char*> (&in), sizeof (in)) == static_cast<int> (sizeof (in)) &&
        in.eth.h_proto == htons (ETH_P_ARP) && in.arp.ar_hrd == htons (ARPHRD_ETHER) &&
        in.arp.ar_pro == htons (ETH_P_IP) && in.arp.ar_hln == ETH_ALEN && in.arp.ar_pln == 4 &&
        in.arp.ar_op == htons (ARPOP_REPLY))
    {
        ScopedLock<Mutex> lock (_syncMutex);

        auto it = _pending.find (in.arp.ar_sip);
        if (it != _pending.end ())
        {
            it->second->mac = MacAddress (in.arp.ar_sha, ETH_ALEN);
            it->second->cond.signal ();
        }
    }
}
