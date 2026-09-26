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
#include <iostream>
#include <atomic>
#include <thread>

// C.
#include <unistd.h>

using join::lastError;
using join::Errc;
using join::RouterAdvertisement;
using join::Ndp;

// =========================================================================
//   CLASS     :
//   METHOD    : version
// =========================================================================
void version ()
{
    std::cout << "rdisc6 version " << JOIN_VERSION << "\n";
}

// =========================================================================
//   CLASS     :
//   METHOD    : usage
// =========================================================================
void usage ()
{
    std::cout << "usage: rdisc6 [options] device\n";
    std::cout << "\n";
    std::cout << "  -h          display this help and exit\n";
    std::cout << "  -m          wait for every router instead of the first one\n";
    std::cout << "  -r attempts number of solicitations to send (default: 3)\n";
    std::cout << "  -v          display version information and exit\n";
    std::cout << "  -w wait     time to wait for an advertisement in ms (default: 1000)\n";
    std::cout << "\n";
    std::cout << "soliciting routers requires the CAP_NET_RAW capability:\n";
    std::cout << "  sudo setcap cap_net_raw+ep ./rdisc6\n";
}

// =========================================================================
//   CLASS     :
//   METHOD    : print
// =========================================================================
void print (const RouterAdvertisement& advert)
{
    std::cout << "router " << advert.src << " [" << advert.link << "] lifetime " << advert.lifetime << "s";
    std::cout << ((advert.flags & ND_RA_FLAG_MANAGED) ? " managed" : "");
    std::cout << ((advert.flags & ND_RA_FLAG_OTHER) ? " other" : "") << "\n";

    if (advert.mtu)
    {
        std::cout << "  mtu " << advert.mtu << "\n";
    }

    for (auto const& prefix : advert.prefixes)
    {
        std::cout << "  prefix " << prefix.prefix << "/" << static_cast<int> (prefix.length);
        std::cout << " valid " << prefix.valid << "s preferred " << prefix.preferred << "s\n";
    }

    for (auto const& rdnss : advert.rdnss)
    {
        for (auto const& server : rdnss.servers)
        {
            std::cout << "  dns " << server << " lifetime " << rdnss.lifetime << "s\n";
        }
    }

    std::cout << std::flush;
}

// =========================================================================
//   CLASS     :
//   METHOD    : main
// =========================================================================
int main (int argc, char* argv[])
{
    std::atomic<int> received{0};
    bool multiple = false;

    try
    {
        int attempts = 3, wait = 1000, opt = 0;

        while ((opt = getopt (argc, argv, "hmr:vw:")) != -1)
        {
            switch (opt)
            {
                case 'm':
                    multiple = true;
                    break;
                case 'r':
                    attempts = std::stoi (optarg);
                    break;
                case 'w':
                    wait = std::stoi (optarg);
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

        if (optind >= argc)
        {
            usage ();
            return 1;
        }

        Ndp::Client client (argv[optind]);

        if (multiple)
        {
            client.addAdvertisementListener ([&received] (const RouterAdvertisement& advert) {
                print (advert);
                ++received;
            });
        }

        for (int i = 0; (i < attempts) && !received; ++i)
        {
            RouterAdvertisement advert;

            if (multiple)
            {
                if (client.solicit () == -1)
                {
                    throw std::system_error (lastError);
                }

                std::this_thread::sleep_for (std::chrono::milliseconds (wait));
            }
            else if (client.solicit (advert, std::chrono::milliseconds (wait)) == 0)
            {
                print (advert);
                ++received;
            }
            else if (lastError != Errc::TimedOut)
            {
                throw std::system_error (lastError);
            }
        }
    }
    catch (const std::exception& e)
    {
        std::cout << "rdisc6: " << e.what () << std::endl;
        return 1;
    }

    if (!received)
    {
        std::cout << "rdisc6: no response" << std::endl;
        return 2;
    }

    return 0;
}
