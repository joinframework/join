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
#include <join/ndp.hpp>

// C++.
#include <stdexcept>
#include <algorithm>
#include <iostream>

// C.
#include <unistd.h>
#include <signal.h>

using join::lastError;
using join::IpAddress;
using join::NdpPrefix;
using join::RouterSolicitation;
using join::RouterAdvertisement;
using join::Ndp;

// =========================================================================
//   CLASS     :
//   METHOD    : version
// =========================================================================
void version ()
{
    std::cout << "radvd version " << JOIN_VERSION << "\n";
}

// =========================================================================
//   CLASS     :
//   METHOD    : usage
// =========================================================================
void usage ()
{
    std::cout << "usage: radvd [options] device\n";
    std::cout << "\n";
    std::cout << "  -h          display this help and exit\n";
    std::cout << "  -i seconds  interval between two unsolicited advertisements (default: 200)\n";
    std::cout << "  -l seconds  router lifetime to advertise, 0 or interval to 9000 (default: 3 x interval)\n";
    std::cout << "  -M          advertise that addresses are available through DHCPv6\n";
    std::cout << "  -m mtu      link MTU to advertise\n";
    std::cout << "  -O          advertise that other settings are available through DHCPv6\n";
    std::cout << "  -p prefix   prefix to advertise, as address[/length], may be repeated\n";
    std::cout << "  -s address  recursive DNS server to advertise, may be repeated\n";
    std::cout << "  -v          display version information and exit\n";
    std::cout << "\n";
    std::cout << "advertising requires the CAP_NET_RAW capability:\n";
    std::cout << "  sudo setcap cap_net_raw+ep ./radvd\n";
}

// =========================================================================
//   CLASS     :
//   METHOD    : main
// =========================================================================
int main (int argc, char* argv[])
{
    try
    {
        RouterAdvertisement advert;
        advert.hopLimit = 64;
        advert.rdnss.resize (1);

        int interval = 200, lifetime = -1, length = 64, opt = 0;

        while ((opt = getopt (argc, argv, "hi:l:Mm:Op:s:v")) != -1)
        {
            std::string arg = optarg ? optarg : "";

            switch (opt)
            {
                case 'i':
                    interval = std::stoi (arg);
                    break;
                case 'l':
                    lifetime = std::stoi (arg);
                    break;
                case 'M':
                    advert.flags |= ND_RA_FLAG_MANAGED;
                    break;
                case 'm':
                    advert.mtu = static_cast<uint32_t> (std::stoul (arg));
                    break;
                case 'O':
                    advert.flags |= ND_RA_FLAG_OTHER;
                    break;
                case 'p':
                    length = (arg.find ('/') != std::string::npos) ? std::stoi (arg.substr (arg.find ('/') + 1)) : 64;
                    if ((length < 0) || (length > 128))
                    {
                        throw std::invalid_argument ("invalid prefix length");
                    }
                    advert.prefixes.emplace_back ();
                    advert.prefixes.back ().prefix = arg.substr (0, arg.find ('/'));
                    advert.prefixes.back ().length = static_cast<uint8_t> (length);
                    break;
                case 's':
                    advert.rdnss[0].servers.emplace_back (arg);
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

        if ((optind >= argc) || (interval <= 0))
        {
            usage ();
            return 1;
        }

        if (lifetime < 0)
        {
            lifetime = std::min (3 * interval, 9000);
        }

        if ((lifetime != 0) && ((lifetime < interval) || (lifetime > 9000)))
        {
            throw std::invalid_argument ("router lifetime must be 0 or between the interval and 9000 seconds");
        }

        advert.lifetime = static_cast<uint16_t> (lifetime);
        advert.rdnss[0].lifetime = static_cast<uint32_t> (3 * interval);

        if (advert.rdnss[0].servers.empty ())
        {
            advert.rdnss.clear ();
        }

        sigset_t signals;
        sigemptyset (&signals);
        sigaddset (&signals, SIGINT);
        sigaddset (&signals, SIGTERM);
        pthread_sigmask (SIG_BLOCK, &signals, nullptr);

        Ndp::Server server (argv[optind]);

        server.addSolicitationListener ([&server, advert] (const RouterSolicitation& solicitation) {
            std::cout << "solicited by " << solicitation.src << std::endl;
            server.advertise (advert, solicitation.src.isWildcard () ? IpAddress::ipv6AllNodes : solicitation.src);
        });

        struct timespec timeout = {interval, 0};

        do
        {
            if (server.advertise (advert) == -1)
            {
                throw std::system_error (lastError);
            }
        }
        while (sigtimedwait (&signals, nullptr, &timeout) == -1);

        advert.lifetime = 0;
        server.advertise (advert);
    }
    catch (const std::exception& e)
    {
        std::cout << "radvd: " << e.what () << std::endl;
        return 1;
    }

    return 0;
}
