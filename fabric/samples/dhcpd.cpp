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
#include <join/version.hpp>
#include <join/error.hpp>
#include <join/dhcp.hpp>

// C++.
#include <iostream>
#include <map>

// C.
#include <unistd.h>

using join::lastError;
using join::IpAddress;
using join::MacAddress;
using join::IpList;
using join::DhcpOption;
using join::DhcpPacket;
using join::Dhcp;
using join::DhcpMessage;

/**
 * @brief a DHCP server handing out addresses from a pool.
 */
class Server : public Dhcp::Server
{
public:
    /**
     * @brief create the server instance.
     * @param device interface to serve.
     * @param first first address of the pool.
     * @param count number of addresses in the pool.
     * @param options options to advertise to the clients.
     */
    Server (const std::string& device, const IpAddress& first, uint32_t count, const DhcpOption& options)
    : Dhcp::Server (device)
    , _first (ntohl (*reinterpret_cast<const uint32_t*> (first.addr ())))
    , _count (count)
    , _options (options)
    {
    }

protected:
    /**
     * @brief offer an address to a client asking for one.
     * @param request message received.
     */
    void onDiscover (const DhcpPacket& request) override
    {
        IpAddress address = allocate (request);

        if (address.isWildcard ())
        {
            std::cout << "no address left for " << request.hardware << std::endl;
            return;
        }

        std::cout << "offering " << address << " to " << request.hardware << std::endl;
        offer (request, address, _options);
    }

    /**
     * @brief acknowledge the address a client asks to keep.
     * @param request message received.
     */
    void onRequest (const DhcpPacket& request) override
    {
        const IpAddress* wants = request.options.getIf<IpAddress> (DhcpOption::RequestedIpAddress);
        IpAddress address = (wants != nullptr) ? *wants : request.client;

        auto lease = _leases.find (request.hardware);
        if ((lease == _leases.end ()) || (lease->second != address))
        {
            std::cout << "refusing " << address << " to " << request.hardware << std::endl;
            nak (request, "address not leased to this client");
            return;
        }

        std::cout << "acknowledging " << address << " to " << request.hardware << std::endl;
        ack (request, address, _options);
    }

    /**
     * @brief give the address of a client back to the pool.
     * @param request message received.
     */
    void onRelease (const DhcpPacket& request) override
    {
        std::cout << "releasing " << request.client << " from " << request.hardware << std::endl;
        _leases.erase (request.hardware);
    }

    /**
     * @brief drop an address a client reported as already in use.
     * @param request message received.
     */
    void onDecline (const DhcpPacket& request) override
    {
        std::cout << "declined by " << request.hardware << std::endl;
        _leases.erase (request.hardware);
    }

    /**
     * @brief answer a client asking only for the parameters.
     * @param request message received.
     */
    void onInform (const DhcpPacket& request) override
    {
        std::cout << "informing " << request.client << std::endl;

        DhcpOption options = _options;
        options.erase (DhcpOption::IpAddressLeaseTime);
        options.erase (DhcpOption::RenewalTimeValue);
        options.erase (DhcpOption::RebindingTimeValue);

        ack (request, IpAddress::ipv4Wildcard, options);
    }

    /**
     * @brief pick an address for the given client.
     * @param request message received.
     * @return the address picked, wildcard if the pool is exhausted.
     */
    IpAddress allocate (const DhcpPacket& request)
    {
        auto lease = _leases.find (request.hardware);
        if (lease != _leases.end ())
        {
            return lease->second;
        }

        for (uint32_t offset = 0; offset < _count; ++offset)
        {
            uint32_t candidate = htonl (_first + offset);
            IpAddress address (&candidate, sizeof (candidate));

            bool taken = false;
            for (auto const& entry : _leases)
            {
                taken = taken || (entry.second == address);
            }

            if (!taken)
            {
                _leases[request.hardware] = address;
                return address;
            }
        }

        return IpAddress::ipv4Wildcard;
    }

    /// first address of the pool, in host order.
    const uint32_t _first;

    /// number of addresses in the pool.
    const uint32_t _count;

    /// options advertised to the clients.
    const DhcpOption _options;

    /// addresses handed out, indexed by client hardware address.
    std::map<MacAddress, IpAddress> _leases;
};

// =========================================================================
//   CLASS     :
//   METHOD    : version
// =========================================================================
void version ()
{
    std::cout << "dhcpd version " << JOIN_VERSION << "\n";
}

// =========================================================================
//   CLASS     :
//   METHOD    : usage
// =========================================================================
void usage ()
{
    std::cout << "usage: dhcpd [options] -I device -p address\n";
    std::cout << "\n";
    std::cout << "  -d domain   domain name to advertise\n";
    std::cout << "  -g address  default gateway to advertise\n";
    std::cout << "  -h          display this help and exit\n";
    std::cout << "  -I device   interface to serve\n";
    std::cout << "  -l seconds  lease time to advertise (default: 86400)\n";
    std::cout << "  -m address  subnet mask to advertise (default: 255.255.255.0)\n";
    std::cout << "  -n count    number of addresses in the pool (default: 16)\n";
    std::cout << "  -p address  first address of the pool\n";
    std::cout << "  -s address  DNS server to advertise\n";
    std::cout << "  -v          display version information and exit\n";
    std::cout << "\n";
    std::cout << "serving DHCP requires the CAP_NET_RAW capability:\n";
    std::cout << "  sudo setcap cap_net_raw+ep ./dhcpd\n";
}

// =========================================================================
//   CLASS     :
//   METHOD    : main
// =========================================================================
int main (int argc, char* argv[])
{
    std::string device, pool, mask = "255.255.255.0", domain, gateway, dns;
    int lease = 86400, count = 16, opt = 0;

    while ((opt = getopt (argc, argv, "d:g:hI:l:m:n:p:s:v")) != -1)
    {
        switch (opt)
        {
            case 'd':
                domain = optarg;
                break;
            case 'g':
                gateway = optarg;
                break;
            case 'I':
                device = optarg;
                break;
            case 'l':
                lease = std::stoi (optarg);
                break;
            case 'm':
                mask = optarg;
                break;
            case 'n':
                count = std::stoi (optarg);
                break;
            case 'p':
                pool = optarg;
                break;
            case 's':
                dns = optarg;
                break;
            case 'v':
                version ();
                return 0;
            case 'h':
                usage ();
                return 0;
            default:
                usage ();
                return 1;
        }
    }

    if (device.empty () || pool.empty ())
    {
        usage ();
        return 1;
    }

    IpAddress first (pool);
    if (first.family () != AF_INET)
    {
        std::cout << "dhcpd: the address pool must be IPv4" << std::endl;
        return 1;
    }

    DhcpOption options;
    options.insert (DhcpOption::IpAddressLeaseTime, lease);
    options.insert (DhcpOption::RenewalTimeValue, lease / 2);
    options.insert (DhcpOption::RebindingTimeValue, (lease * 7) / 8);
    options.insert (DhcpOption::SubnetMask, mask);

    if (!gateway.empty ())
    {
        options.insert (DhcpOption::Router, IpList{gateway});
    }

    if (!dns.empty ())
    {
        options.insert (DhcpOption::DomainNameServer, IpList{dns});
    }

    if (!domain.empty ())
    {
        options.insert (DhcpOption::DomainName, domain);
    }

    Server server (device, first, static_cast<uint32_t> (count), options);

    std::cout << "serving " << count << " addresses from " << first << " on " << device << std::endl;
    std::cout << options;

    pause ();

    return 0;
}
