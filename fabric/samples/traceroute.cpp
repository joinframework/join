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
#include <join/ping.hpp>

// C++.
#include <iostream>
#include <iomanip>

// C.
#include <unistd.h>

using join::IpAddress;
using join::PingStats;
using join::Ping;

// =========================================================================
//   CLASS     :
//   METHOD    : version
// =========================================================================
void version ()
{
    std::cout << "traceroute version " << JOIN_VERSION << "\n";
}

// =========================================================================
//   CLASS     :
//   METHOD    : usage
// =========================================================================
void usage ()
{
    std::cout << "usage: traceroute [options] destination\n";
    std::cout << "\n";
    std::cout << "  -4          use IPv4 (default)\n";
    std::cout << "  -6          use IPv6\n";
    std::cout << "  -h          display this help and exit\n";
    std::cout << "  -I device   bind to the given device\n";
    std::cout << "  -M          set the don't fragment flag and report the MTU a hop rejects the probe on\n";
    std::cout << "  -m hops     maximum number of hops to probe (default: 30)\n";
    std::cout << "  -q probes   number of probes sent per hop (default: 3)\n";
    std::cout << "  -s size     number of data bytes to send (default: 56)\n";
    std::cout << "  -v          display version information and exit\n";
    std::cout << "  -W timeout  answer timeout in ms (default: 5000)\n";
    std::cout << "\n";
    std::cout << "opening an ICMP socket requires the CAP_NET_RAW capability:\n";
    std::cout << "  sudo setcap cap_net_raw+ep ./traceroute\n";
}

// =========================================================================
//   CLASS     :
//   METHOD    : onStart
// =========================================================================
void onStart (const PingStats& stats)
{
    std::cout << "traceroute to " << stats.host () << " (" << stats.address () << "), ";
    std::cout << stats.size () << " data bytes" << std::endl;
}

// =========================================================================
//   CLASS     :
//   METHOD    : onHop
// =========================================================================
void onHop (const PingStats& stats)
{
    std::cout << std::setw (2) << stats.hop () << "  ";

    if (stats.from ().isWildcard ())
    {
        std::cout << "*" << std::endl;
        return;
    }

    std::cout << stats.from () << "  ";
    std::cout << std::fixed << std::setprecision (3) << (stats.avg ().count () / 1000.0) << " ms";
    std::cout << std::endl;
}

// =========================================================================
//   CLASS     :
//   METHOD    : onFailure
// =========================================================================
void onFailure (const PingStats& stats)
{
    if (stats.mtu ())
    {
        std::cout << stats.error () << std::endl;
    }
}

// =========================================================================
//   CLASS     :
//   METHOD    : main
// =========================================================================
int main (int argc, char* argv[])
{
    int family = AF_INET, maxHops = 30, probes = 3, size = 56, timeout = 5000, opt = 0;
    int df = IP_PMTUDISC_DONT;
    std::string device;

    while ((opt = getopt (argc, argv, "46hI:Mm:q:s:vW:")) != -1)
    {
        switch (opt)
        {
            case '4':
                family = AF_INET;
                break;
            case '6':
                family = AF_INET6;
                break;
            case 'I':
                device = optarg;
                break;
            case 'M':
                df = IP_PMTUDISC_DO;
                break;
            case 'm':
                maxHops = std::stoi (optarg);
                break;
            case 'q':
                probes = std::stoi (optarg);
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
            case 'h':
                usage ();
                return 0;
            default:
                usage ();
                return 1;
        }
    }

    if (optind >= argc)
    {
        usage ();
        return 1;
    }

    Ping client (device);

    client.onStart = onStart;
    client.onFailure = onFailure;
    client.onHop = onHop;

    if (client.trace (IpAddress (family), argv[optind], maxHops, probes, size, df,
                      std::chrono::milliseconds (timeout)) == -1)
    {
        std::cerr << "traceroute: destination not reached" << std::endl;
        return 1;
    }

    return 0;
}
