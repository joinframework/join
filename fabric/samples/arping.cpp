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
#include <join/arp.hpp>

// C++.
#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>

// C.
#include <unistd.h>

using join::lastError;
using join::IpAddress;
using join::MacAddress;
using join::Arp;

// =========================================================================
//   CLASS     :
//   METHOD    : version
// =========================================================================
void version ()
{
    std::cout << "arping version " << JOIN_VERSION << "\n";
}

// =========================================================================
//   CLASS     :
//   METHOD    : usage
// =========================================================================
void usage ()
{
    std::cout << "usage: arping [options] -I device destination\n";
    std::cout << "\n";
    std::cout << "  -c count    number of ARP requests to send (default: 4)\n";
    std::cout << "  -h          display this help and exit\n";
    std::cout << "  -I device   interface the ARP requests are sent through\n";
    std::cout << "  -i interval interval between two ARP requests in ms (default: 500)\n";
    std::cout << "  -v          display version information and exit\n";
    std::cout << "  -W timeout  answer timeout in ms (default: 5000)\n";
    std::cout << "\n";
    std::cout << "sending an ARP request requires the CAP_NET_RAW capability:\n";
    std::cout << "  sudo setcap cap_net_raw+ep ./arping\n";
}

// =========================================================================
//   CLASS     :
//   METHOD    : main
// =========================================================================
int main (int argc, char* argv[])
{
    int count = 4, interval = 500, timeout = 5000, opt = 0;
    std::string device;

    while ((opt = getopt (argc, argv, "c:hI:i:vW:")) != -1)
    {
        switch (opt)
        {
            case 'c':
                count = std::stoi (optarg);
                break;
            case 'I':
                device = optarg;
                break;
            case 'i':
                interval = std::stoi (optarg);
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

    if ((optind >= argc) || device.empty ())
    {
        usage ();
        return 1;
    }

    IpAddress address (argv[optind]);

    std::cout << "ARPING " << address << " on " << device << std::endl;

    Arp arp (device);

    int received = 0;

    for (int i = 0; i < count; ++i)
    {
        auto start = std::chrono::steady_clock::now ();

        MacAddress mac = arp.request (address, std::chrono::milliseconds (timeout));

        auto rtt = std::chrono::duration_cast<std::chrono::microseconds> (std::chrono::steady_clock::now () - start);

        if (mac.isWildcard ())
        {
            std::cout << "arping: " << address << ": " << lastError.message () << std::endl;
        }
        else
        {
            std::cout << "reply from " << address << " [" << mac << "]";
            std::cout << " index=" << (i + 1);
            std::cout << " time=" << std::fixed << std::setprecision (3) << (rtt.count () / 1000.0) << " ms";
            std::cout << std::endl;

            ++received;
        }

        if ((i < (count - 1)) && (interval > 0))
        {
            std::this_thread::sleep_for (std::chrono::milliseconds (interval));
        }
    }

    std::cout << "--- " << address << " arping statistics ---" << std::endl;
    std::cout << count << " requests sent, " << received << " replies received, ";
    std::cout << std::fixed << std::setprecision (0) << ((count - received) * 100.0) / count << "% loss" << std::endl;

    return (received == 0);
}
