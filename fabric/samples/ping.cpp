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
    std::cout << "ping version " << JOIN_VERSION << "\n";
}

// =========================================================================
//   CLASS     :
//   METHOD    : usage
// =========================================================================
void usage ()
{
    std::cout << "usage: ping [options] destination\n";
    std::cout << "\n";
    std::cout << "  -4          use IPv4 (default)\n";
    std::cout << "  -6          use IPv6\n";
    std::cout << "  -c count    number of echo requests to send (default: 4)\n";
    std::cout << "  -h          display this help and exit\n";
    std::cout << "  -I device   bind to the given device\n";
    std::cout << "  -i interval interval between two echo requests in ms (default: 500)\n";
    std::cout << "  -M          discover the path MTU instead of pinging\n";
    std::cout << "  -s size     number of data bytes to send (default: 56)\n";
    std::cout << "  -t ttl      time to live of the echo requests (default: 60)\n";
    std::cout << "  -v          display version information and exit\n";
    std::cout << "  -W timeout  answer timeout in ms (default: 5000)\n";
    std::cout << "\n";
    std::cout << "opening an ICMP socket requires the CAP_NET_RAW capability:\n";
    std::cout << "  sudo setcap cap_net_raw+ep ./ping\n";
}

// =========================================================================
//   CLASS     :
//   METHOD    : onStart
// =========================================================================
void onStart (const PingStats& stats)
{
    std::cout << "PING " << stats.host () << " (" << stats.address () << ") ";

    if (!stats.device ().empty ())
    {
        std::cout << "from " << stats.device () << " ";
    }

    std::cout << stats.size () << " data bytes" << std::endl;
}

// =========================================================================
//   CLASS     :
//   METHOD    : onSuccess
// =========================================================================
void onSuccess (const PingStats& stats)
{
    std::cout << stats.packetSize () << " bytes from " << stats.from () << ":";
    std::cout << " icmp_seq=" << stats.sequence ();
    std::cout << " ttl=" << stats.ttl ();

    if (stats.rtt ().count ())
    {
        std::cout << " time=" << std::fixed << std::setprecision (3) << (stats.rtt ().count () / 1000.0) << " ms";
    }

    std::cout << std::endl;
}

// =========================================================================
//   CLASS     :
//   METHOD    : onFailure
// =========================================================================
void onFailure (const PingStats& stats)
{
    std::cout << stats.error () << std::endl;
}

// =========================================================================
//   CLASS     :
//   METHOD    : onStop
// =========================================================================
void onStop (const PingStats& stats)
{
    std::cout << "--- " << stats.host () << " ping statistics ---" << std::endl;

    std::cout << stats.sent () << " packets transmitted, " << stats.received () << " received, ";
    std::cout << std::fixed << std::setprecision (0) << stats.loss () << "% packet loss, ";
    std::cout << "time " << std::setprecision (3) << (stats.total ().count () / 1000.0) << " ms" << std::endl;

    if (stats.received () && stats.max ().count ())
    {
        std::cout << "rtt min/avg/max/mdev = ";
        std::cout << std::fixed << std::setprecision (3) << (stats.min ().count () / 1000.0) << "/";
        std::cout << (stats.avg ().count () / 1000.0) << "/";
        std::cout << (stats.max ().count () / 1000.0) << "/";
        std::cout << (stats.mdev ().count () / 1000.0) << " ms" << std::endl;
    }
}

// =========================================================================
//   CLASS     :
//   METHOD    : main
// =========================================================================
int main (int argc, char* argv[])
{
    int family = AF_INET, count = 4, size = 56, ttl = 60, interval = 500, timeout = 5000, opt = 0;
    bool discover = false;
    std::string device;

    while ((opt = getopt (argc, argv, "46c:hI:i:Ms:t:vW:")) != -1)
    {
        switch (opt)
        {
            case '4':
                family = AF_INET;
                break;
            case '6':
                family = AF_INET6;
                break;
            case 'c':
                count = std::stoi (optarg);
                break;
            case 'I':
                device = optarg;
                break;
            case 'i':
                interval = std::stoi (optarg);
                break;
            case 'M':
                discover = true;
                break;
            case 's':
                size = std::stoi (optarg);
                break;
            case 't':
                ttl = std::stoi (optarg);
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

    Ping client (device, ttl);

    client.onStart = onStart;
    client.onSuccess = onSuccess;
    client.onFailure = onFailure;
    client.onStop = onStop;

    if (discover)
    {
        int mtu = client.pathMtu (IpAddress (family), argv[optind], std::chrono::milliseconds (timeout));
        if (mtu == -1)
        {
            std::cerr << "ping: could not discover the path MTU" << std::endl;
            return 1;
        }

        std::cout << "path mtu = " << mtu << std::endl;

        return 0;
    }

    if (client.ping (IpAddress (family), argv[optind], size, count, std::chrono::milliseconds (interval),
                     IP_PMTUDISC_DONT, std::chrono::milliseconds (timeout)) == 0)
    {
        return 1;
    }

    return 0;
}
