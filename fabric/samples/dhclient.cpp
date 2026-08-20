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

// C.
#include <unistd.h>

using join::lastError;
using join::IpAddress;
using join::DhcpOption;
using join::DhcpPacket;
using join::Dhcp;
using join::DhcpMessage;

// =========================================================================
//   CLASS     :
//   METHOD    : version
// =========================================================================
void version ()
{
    std::cout << "dhclient version " << JOIN_VERSION << "\n";
}

// =========================================================================
//   CLASS     :
//   METHOD    : usage
// =========================================================================
void usage ()
{
    std::cout << "usage: dhclient [options] -I device\n";
    std::cout << "\n";
    std::cout << "  -h          display this help and exit\n";
    std::cout << "  -H name     host name to advertise\n";
    std::cout << "  -i address  ask for the parameters of an address already configured\n";
    std::cout << "  -I device   interface to acquire a lease on\n";
    std::cout << "  -r address  release the given lease instead of acquiring one\n";
    std::cout << "  -S address  address of the server holding the lease, required by -r\n";
    std::cout << "  -s size     maximum message size to advertise (>= 576)\n";
    std::cout << "  -v          display version information and exit\n";
    std::cout << "  -W timeout  answer timeout in ms (default: 5000)\n";
    std::cout << "  -x address  address to ask for\n";
    std::cout << "\n";
    std::cout << "acquiring a lease requires the CAP_NET_RAW capability:\n";
    std::cout << "  sudo setcap cap_net_raw+ep ./dhclient\n";
}

// =========================================================================
//   CLASS     :
//   METHOD    : main
// =========================================================================
int main (int argc, char* argv[])
{
    std::string device, hostname, release, server, configured;
    IpAddress wants = IpAddress::ipv4Wildcard;
    int timeout = 5000, size = 0, opt = 0;

    while ((opt = getopt (argc, argv, "hH:i:I:r:S:s:vW:x:")) != -1)
    {
        switch (opt)
        {
            case 'H':
                hostname = optarg;
                break;
            case 'i':
                configured = optarg;
                break;
            case 'I':
                device = optarg;
                break;
            case 'r':
                release = optarg;
                break;
            case 'S':
                server = optarg;
                break;
            case 's':
                size = std::stoi (optarg);
                break;
            case 'v':
                version ();
                return 0;
            case 'W':
                timeout = std::stoi (optarg);
                break;
            case 'x':
                wants = IpAddress (optarg);
                break;
            case 'h':
                usage ();
                return 0;
            default:
                usage ();
                return 1;
        }
    }

    if (device.empty ())
    {
        usage ();
        return 1;
    }

    Dhcp::Client client (device, static_cast<uint16_t> (size), hostname);

    if (!release.empty ())
    {
        if (server.empty ())
        {
            usage ();
            return 1;
        }

        std::cout << "releasing " << release << " to " << server << std::endl;

        if (client.release (release, server) == -1)
        {
            std::cout << "dhclient: " << lastError.message () << std::endl;
            return 1;
        }

        return 0;
    }

    if (!configured.empty ())
    {
        DhcpPacket::Ptr parameters = client.inform (configured, std::chrono::milliseconds (timeout));
        if (parameters == nullptr)
        {
            std::cout << "dhclient: no answer: " << lastError.message () << std::endl;
            return 1;
        }

        std::cout << "parameters for " << configured << std::endl;
        std::cout << parameters->options;

        return 0;
    }

    std::cout << "soliciting a lease on " << device << " [" << client.hardware () << "]" << std::endl;

    DhcpPacket::Ptr offer = client.discover (wants, std::chrono::milliseconds (timeout));
    if (offer == nullptr)
    {
        std::cout << "dhclient: no offer: " << lastError.message () << std::endl;
        return 1;
    }

    const IpAddress* from = offer->options.getIf<IpAddress> (DhcpOption::ServerIdentifier);
    if (from == nullptr)
    {
        std::cout << "dhclient: offer names no server, ignoring it" << std::endl;
        return 1;
    }

    std::cout << "offer of " << offer->your << " from " << *from << std::endl;

    DhcpPacket::Ptr lease = client.request (offer->your, *from, std::chrono::milliseconds (timeout));
    if (lease == nullptr)
    {
        const std::string reason = client.reason ();
        std::cout << "dhclient: request refused: " << (reason.empty () ? lastError.message () : reason) << std::endl;
        return 1;
    }

    std::cout << "bound to " << lease->your << std::endl;
    std::cout << lease->options;

    return 0;
}
