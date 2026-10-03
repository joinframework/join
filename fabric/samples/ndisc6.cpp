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

// C.
#include <unistd.h>

using join::lastError;
using join::Errc;
using join::IpAddress;
using join::NeighborAdvertisement;
using join::Ndp;

// =========================================================================
//   CLASS     :
//   METHOD    : version
// =========================================================================
void version ()
{
    std::cout << "ndisc6 version " << JOIN_VERSION << "\n";
}

// =========================================================================
//   CLASS     :
//   METHOD    : usage
// =========================================================================
void usage ()
{
    std::cout << "usage: ndisc6 [options] address device\n";
    std::cout << "\n";
    std::cout << "  -h          display this help and exit\n";
    std::cout << "  -r attempts number of solicitations to send (default: 3)\n";
    std::cout << "  -v          display version information and exit\n";
    std::cout << "  -w wait     time to wait for an advertisement in ms (default: 1000)\n";
    std::cout << "\n";
    std::cout << "soliciting neighbors requires the CAP_NET_RAW capability:\n";
    std::cout << "  sudo setcap cap_net_raw+ep ./ndisc6\n";
}

// =========================================================================
//   CLASS     :
//   METHOD    : print
// =========================================================================
void print (const NeighborAdvertisement& advert)
{
    std::cout << "neighbor " << advert.target << " [" << advert.link << "] from " << advert.src;
    std::cout << ((advert.flags & ND_NA_FLAG_ROUTER) ? " router" : "");
    std::cout << ((advert.flags & ND_NA_FLAG_SOLICITED) ? " solicited" : "");
    std::cout << ((advert.flags & ND_NA_FLAG_OVERRIDE) ? " override" : "") << std::endl;
}

// =========================================================================
//   CLASS     :
//   METHOD    : main
// =========================================================================
int main (int argc, char* argv[])
{
    try
    {
        int attempts = 3, wait = 1000, opt = 0;

        while ((opt = getopt (argc, argv, "hr:vw:")) != -1)
        {
            switch (opt)
            {
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

        if ((optind + 2) > argc)
        {
            usage ();
            return 1;
        }

        IpAddress target (argv[optind]);
        Ndp::Client client (argv[optind + 1]);

        for (int i = 0; i < attempts; ++i)
        {
            NeighborAdvertisement advert;

            if (client.neighborSolicit (target, advert, std::chrono::milliseconds (wait)) == 0)
            {
                print (advert);
                return 0;
            }

            if (lastError != Errc::TimedOut)
            {
                throw std::system_error (lastError);
            }
        }
    }
    catch (const std::exception& e)
    {
        std::cout << "ndisc6: " << e.what () << std::endl;
        return 1;
    }

    std::cout << "ndisc6: no response" << std::endl;
    return 2;
}
