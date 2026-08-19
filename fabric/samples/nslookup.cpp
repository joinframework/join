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
#include <join/resolver.hpp>
#include <join/version.hpp>
#include <join/error.hpp>

// C++.
#include <iostream>
#include <string>

// C.
#include <unistd.h>

using join::DnsMessage;
using join::TlsContext;
using join::IpAddress;
using join::DnsPacket;
using join::lastError;
using join::Dns;
using join::Dot;

// =========================================================================
//   CLASS     :
//   METHOD    : version
// =========================================================================
void version ()
{
    std::cout << "nslookup version " << JOIN_VERSION << "\n";
}

// =========================================================================
//   CLASS     :
//   METHOD    : usage
// =========================================================================
void usage ()
{
    std::cout << "usage: nslookup [options] name [server]\n";
    std::cout << "\n";
    std::cout << "  -h          display this help and exit\n";
    std::cout << "  -p port     name server port (default: 53, 853 with -T)\n";
    std::cout << "  -t type     record type: A, AAAA, PTR, NS or MX\n";
    std::cout << "              (default: PTR for an address, A and AAAA otherwise)\n";
    std::cout << "  -T          query over DNS over TLS\n";
    std::cout << "  -v          display version information and exit\n";
    std::cout << "  -W timeout  query timeout in ms (default: 5000)\n";
}

// =========================================================================
//   CLASS     :
//   METHOD    : onSuccess
// =========================================================================
void onSuccess (const DnsPacket& packet)
{
    for (auto const& answer : packet.answers)
    {
        std::cout << answer.host << "  " << answer.ttl << "  " << DnsMessage::typeName (answer.type) << "  ";

        switch (answer.type)
        {
            case DnsMessage::RecordType::A:
            case DnsMessage::RecordType::AAAA:
                std::cout << answer.addr;
                break;
            case DnsMessage::RecordType::MX:
                std::cout << answer.mxpref << " " << answer.name;
                break;
            default:
                std::cout << answer.name;
                break;
        }

        std::cout << std::endl;
    }
}

// =========================================================================
//   CLASS     :
//   METHOD    : onFailure
// =========================================================================
void onFailure (const DnsPacket& packet)
{
    for (auto const& question : packet.questions)
    {
        std::cerr << "nslookup: " << question.host << ": " << DnsMessage::typeName (question.type) << ": "
                  << lastError.message () << std::endl;
    }
}

// =========================================================================
//   CLASS     :
//   METHOD    : lookup
// =========================================================================
template <typename Resolver>
int lookup (Resolver&& resolver, const std::string& name, const std::string& type, std::chrono::milliseconds timeout)
{
    resolver.onSuccess = onSuccess;
    resolver.onFailure = onFailure;

    if (type == "A")
    {
        return resolver.resolveAllAddress (name, AF_INET, timeout).empty ();
    }

    if (type == "AAAA")
    {
        return resolver.resolveAllAddress (name, AF_INET6, timeout).empty ();
    }

    if (type == "NS")
    {
        return resolver.resolveAllNameServer (name, timeout).empty ();
    }

    if (type == "MX")
    {
        return resolver.resolveAllMailExchanger (name, timeout).empty ();
    }

    if ((type == "PTR") || (type.empty () && IpAddress::isIpAddress (name)))
    {
        if (!IpAddress::isIpAddress (name))
        {
            std::cerr << "nslookup: " << name << ": not an address" << std::endl;
            return 1;
        }

        return resolver.resolveAllName (IpAddress (name), timeout).empty ();
    }

    return resolver.resolveAllAddress (name, timeout).empty ();
}

// =========================================================================
//   CLASS     :
//   METHOD    : main
// =========================================================================
int main (int argc, char* argv[])
{
    int timeout = 5000, port = 0, opt = 0;
    bool tls = false;
    std::string type;

    while ((opt = getopt (argc, argv, "hp:t:TvW:")) != -1)
    {
        switch (opt)
        {
            case 'p':
                port = std::stoi (optarg);
                break;
            case 't':
                type = optarg;
                break;
            case 'T':
                tls = true;
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

    std::string name = argv[optind];
    std::string server = (optind + 1 < argc) ? argv[optind + 1] : std::string ();

    if (server.empty ())
    {
        for (auto const& configured : Dns::Resolver::nameServers ())
        {
            server = configured.toString ();
            break;
        }
    }

    if (server.empty ())
    {
        std::cerr << "nslookup: no name server configured" << std::endl;
        return 1;
    }

    std::cout << "Server: " << server << (tls ? " (DNS over TLS)" : "") << std::endl;

    if (tls)
    {
        TlsContext context (TlsContext::TlsClient);
        context.setAlpnProtocols ({"dot"});

        return lookup (Dot::Resolver (context, server, port ? port : Dot::defaultPort), name, type,
                       std::chrono::milliseconds (timeout));
    }

    return lookup (Dns::Resolver (server, port ? port : Dns::defaultPort), name, type,
                   std::chrono::milliseconds (timeout));
}
