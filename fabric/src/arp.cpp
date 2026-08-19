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

// C++.
#include <system_error>

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
Arp::Arp (const std::string& interface, NeighborManager& neighbors)
: _interface (interface)
, _neighbors (neighbors)
, _reactor (_neighbors.reactor ())
{
    _index = ::if_nametoindex (_interface.c_str ());
    if (_index == 0)
    {
        throw std::system_error (errno, std::system_category (), "arp interface lookup failed");
    }

    if (bind (_interface) == -1 || setOption (Raw::Socket::Broadcast, 1) == -1)
    {
        throw std::system_error (lastError, "arp socket setup failed");  // LCOV_EXCL_LINE
    }

    // accept only ARP replies.
    struct sock_filter code[] = {
        {0x28, 0, 0, 0x0000000c}, {0x15, 0, 3, 0x00000806}, {0x28, 0, 0, 0x00000014},
        {0x15, 0, 1, 0x00000002}, {0x6, 0, 0, 0x00040000},  {0x6, 0, 0, 0x00000000},
    };

    struct sock_fprog bpf;
    bpf.len = 6;
    bpf.filter = code;

    // best effort, validation is done in onReadable anyway.
    ::setsockopt (handle (), SOL_SOCKET, SO_ATTACH_FILTER, &bpf, sizeof (bpf));

    _reactor.addHandler (handle (), this);
}

// =========================================================================
//   CLASS     : Arp
//   METHOD    : ~Arp
// =========================================================================
Arp::~Arp ()
{
    _reactor.delHandler (handle ());
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

    return _neighbors.addNeighbor (_index, ip, mac, NUD_PERMANENT, true);
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

    return _neighbors.removeNeighbor (_index, ip, true);
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

    auto neighbor = _neighbors.findByIndex (_index, ip);
    if (!neighbor)
    {
        lastError = std::make_error_code (std::errc::no_such_device_or_address);
        return {};
    }

    if (!(neighbor->state () & NUD_VALID))
    {
        lastError = std::make_error_code (std::errc::no_such_device_or_address);  // LCOV_EXCL_LINE
        return {};                                                                // LCOV_EXCL_LINE
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
//   METHOD    : onReadable
// =========================================================================
void Arp::onReadable ([[maybe_unused]] int fd) noexcept
{
    char data[_bufferSize];

    if (read (data, sizeof (data)) < static_cast<int> (sizeof (Packet)))
    {
        return;  // LCOV_EXCL_LINE
    }

    const Packet& in = *reinterpret_cast<const Packet*> (data);

    if (in.eth.h_proto == htons (ETH_P_ARP) && in.arp.ar_hrd == htons (ARPHRD_ETHER) &&
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
