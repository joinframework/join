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
#include <join/socket.hpp>
#include <join/arp.hpp>

// C++.
#include <chrono>

// C.
#include <linux/filter.h>
#include <sys/ioctl.h>
#include <cstring>
#include <cerrno>

using join::MacAddress;
using join::IpAddress;
using join::Raw;
using join::Arp;

// =========================================================================
//   CLASS     : Arp
//   METHOD    : Arp
// =========================================================================
Arp::Arp (const std::string& interface)
: _interface (interface)
{
}

// =========================================================================
//   CLASS     : Arp
//   METHOD    : get
// =========================================================================
MacAddress Arp::get (const IpAddress& ip)
{
    if (ip.family () != AF_INET)
    {
        lastError = make_error_code (Errc::InvalidParam);
        return {};
    }

    if (ip == IpAddress::ipv4Address (_interface))
    {
        return MacAddress::address (_interface);
    }

    MacAddress mac = cache (ip);

    return mac.isWildcard () ? request (ip) : mac;
}

// =========================================================================
//   CLASS     : Arp
//   METHOD    : get
// =========================================================================
MacAddress Arp::get (const IpAddress& ip, const std::string& interface)
{
    return Arp (interface).get (ip);
}

// =========================================================================
//   CLASS     : Arp
//   METHOD    : request
// =========================================================================
MacAddress Arp::request (const IpAddress& ip)
{
    if (ip.family () != AF_INET)
    {
        lastError = make_error_code (Errc::InvalidParam);
        return {};
    }

    Raw::Socket stream;

    // bind to interface and allow broadcast.
    if (stream.bind (_interface) == -1 || stream.setOption (Raw::Socket::Broadcast, 1) == -1)
    {
        return {};
    }

    // generated using 'tcpdump -dd arp'
    struct sock_filter code[] = {
        { 0x28, 0, 0, 0x0000000c },
        { 0x15, 0, 1, 0x00000806 },
        { 0x6,  0, 0, 0x00040000 },
        { 0x6,  0, 0, 0x00000000 },
    };

    struct sock_fprog bpf = {
        .len = 4,
        .filter = code,
    };

    // filter ARP only.
    ::setsockopt (stream.handle (), SOL_SOCKET, SO_ATTACH_FILTER, &bpf, sizeof (bpf));

    Packet out;
    ::memset (&out, 0, sizeof (Packet));

    ::memcpy (out.eth.h_dest, MacAddress::broadcast.addr (), ETH_ALEN);
    ::memcpy (out.eth.h_source, MacAddress::address (_interface).addr (), ETH_ALEN);
    out.eth.h_proto = ::htons (ETH_P_ARP);

    out.arp.ar_hrd  = ::htons (ARPHRD_ETHER);
    out.arp.ar_pro  = ::htons (ETH_P_IP);
    out.arp.ar_hln  = ETH_ALEN;
    out.arp.ar_pln  = 4;
    out.arp.ar_op   = ::htons (ARPOP_REQUEST);
    ::memcpy (out.arp.ar_sha, MacAddress::address (_interface).addr (), ETH_ALEN);
    out.arp.ar_sip  = *reinterpret_cast <const uint32_t *> (IpAddress::ipv4Address (_interface).addr ());
    ::memcpy (out.arp.ar_tha, MacAddress::wildcard.addr (), ETH_ALEN);
    out.arp.ar_tip  = *reinterpret_cast <const uint32_t *> (ip.addr ());

    if (stream.write (reinterpret_cast <const char *> (&out), sizeof (Packet)) == -1)
    {
        return {};
    }

    auto elapsed = std::chrono::milliseconds::zero ();
    int timeout  = 1000;

    for (;;)
    {
        auto beg = std::chrono::high_resolution_clock::now ();

        if ((timeout <= elapsed.count ()) || !stream.waitReadyRead (timeout - elapsed.count ()))
        {
            lastError = std::make_error_code (std::errc::no_such_device_or_address);
            break;
        }

        auto buffer = std::make_unique <char []> (stream.canRead ());
        if (buffer == nullptr)
        {
            lastError = make_error_code (Errc::OutOfMemory);
            break;
        }

        int size = stream.read (reinterpret_cast <char *> (buffer.get ()), stream.canRead ());
        if (size_t (size) < sizeof (Packet))
        {
            elapsed += std::chrono::duration_cast <std::chrono::milliseconds> (std::chrono::high_resolution_clock::now () - beg);
            continue;
        }

        Packet *in = reinterpret_cast <Packet *> (buffer.get ());
        if (in->eth.h_proto != htons (ETH_P_ARP)                     ||
            in->arp.ar_hrd  != htons (ARPHRD_ETHER)                  ||
            in->arp.ar_pro  != htons (ETH_P_IP)                      ||
            in->arp.ar_hln  != ETH_ALEN                              ||
            in->arp.ar_pln  != 4                                     ||
            in->arp.ar_op   != htons (ARPOP_REPLY)                   ||
            ::memcmp (in->arp.ar_tha, out.arp.ar_sha, ETH_ALEN) != 0)
        {
            elapsed += std::chrono::duration_cast <std::chrono::milliseconds> (std::chrono::high_resolution_clock::now () - beg);
            continue;
        }

        return MacAddress (reinterpret_cast <uint8_t*> (in->arp.ar_sha), ETH_ALEN);
    }

    return {};
}

// =========================================================================
//   CLASS     : Arp
//   METHOD    : request
// =========================================================================
MacAddress Arp::request (const IpAddress& ip, const std::string& interface)
{
    return Arp (interface).request (ip);
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

    int fd = ::socket (AF_INET, SOCK_DGRAM, 0);
    if (fd != -1)
    {
        struct arpreq areq;
        ::memset (&areq, 0, sizeof (areq));
        ::strncpy (areq.arp_dev, _interface.c_str (), IFNAMSIZ - 1);
        ::memcpy (areq.arp_ha.sa_data, mac.addr (), mac.length ());
        areq.arp_flags = ATF_PERM | ATF_COM;

        struct sockaddr_in *in = reinterpret_cast <struct sockaddr_in *> (&areq.arp_pa);
        in->sin_addr.s_addr    = *reinterpret_cast <const uint32_t *> (ip.addr ());
        in->sin_family         = AF_INET;

        int result = ::ioctl (fd, SIOCSARP, &areq);
        ::close (fd);
        
        if (result != -1)
        {
            return 0;
        }
    }

    lastError = std::make_error_code (static_cast <std::errc> (errno));
    return -1;
}

// =========================================================================
//   CLASS     : Arp
//   METHOD    : add
// =========================================================================
int Arp::add (const MacAddress& mac, const IpAddress& ip, const std::string& interface)
{
    return Arp (interface).add (mac, ip);
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

    int fd = ::socket (AF_INET, SOCK_DGRAM, 0);
    if (fd != -1)
    {
        struct arpreq areq;
        ::memset (&areq, 0, sizeof (areq));
        ::strncpy (areq.arp_dev, _interface.c_str (), IFNAMSIZ - 1);

        struct sockaddr_in *in = reinterpret_cast <struct sockaddr_in *> (&areq.arp_pa);
        in->sin_addr.s_addr    = *reinterpret_cast <const uint32_t *> (ip.addr ());
        in->sin_family         = AF_INET;

        int result = ::ioctl (fd, SIOCGARP, &areq);
        ::close (fd);

        if (result != -1)
        {
            if (areq.arp_flags & ATF_COM)
            {
                return MacAddress (reinterpret_cast <uint8_t*> (areq.arp_ha.sa_data), ETH_ALEN);
            }

            lastError = make_error_code (Errc::NotFound);
            return {};
        }
    }

    lastError = std::make_error_code (static_cast <std::errc> (errno));
    return {};
}

// =========================================================================
//   CLASS     : Arp
//   METHOD    : cache
// =========================================================================
MacAddress Arp::cache (const IpAddress& ip, const std::string& interface)
{
    return Arp (interface).cache (ip);
}
