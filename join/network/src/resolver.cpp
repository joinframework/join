/**
 * MIT License
 *
 * Copyright (c) 2023 Mathieu Rabine
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
#include <join/socket.hpp>
#include <join/utils.hpp>

// C++.
#include <sstream>
#include <string>
#include <vector>
#include <regex>

// C.
#include <resolv.h>
#include <netdb.h>

using join::IpAddress;
using join::IpAddressList;
using join::AliasList;
using join::ServerList;
using join::ExchangerList;
using join::Resolver;
using join::QuestionRecord;
using join::AnswerRecord;
using join::Udp;

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : Resolver
// =========================================================================
Resolver::Resolver ()
: Resolver ("")
{
}

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : Resolver
// =========================================================================
Resolver::Resolver (const std::string& interface)
#ifdef DEBUG
: _onSuccess (defaultOnSuccess),
  _onFailure (defaultOnFailure),
  _interface (interface)
#else
: _interface (interface)
#endif
{
}

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : nameServers
// =========================================================================
IpAddressList Resolver::nameServers ()
{
    IpAddressList addressList;

    struct __res_state res;
    if (res_ninit (&res) == 0)
    {
        for (int i = 0; i < res.nscount; ++i)
        {
            if (res.nsaddr_list[i].sin_family == AF_INET)
            {
                addressList.emplace_back (&res.nsaddr_list[i].sin_addr, sizeof (struct in_addr));
            }
            else if (res._u._ext.nsaddrs[i] != nullptr && res._u._ext.nsaddrs[i]->sin6_family == AF_INET6)
            {
                addressList.emplace_back (&res._u._ext.nsaddrs[i]->sin6_addr, sizeof (struct in6_addr));
            }
        }
        res_nclose (&res);
    }

    return addressList;
}

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : resolveAllHost
// =========================================================================
IpAddressList Resolver::resolveAllHost (const std::string& host, int family, const IpAddress& server, uint16_t port, int timeout)
{
    IpAddressList addresses;

    DnsPacket packet;
    packet.src = IpAddress (server.family ());
    packet.dest = server;
    packet.port = port;

    QuestionRecord question;
    question.host = host;
    question.type = (family == AF_INET6) ? RecordType::AAAA : RecordType::A;
    question.dnsclass = RecordClass::IN;

    packet.questions.push_back (question);

    if (lookup (packet, timeout) == -1)
    {
        return addresses;
    }

    for (auto const& answer : packet.answers)
    {
        if (!answer.addr.isWildcard ())
        {
            addresses.push_back (answer.addr);
        }
    }

    return addresses;
}

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : resolveAllHost
// =========================================================================
IpAddressList Resolver::resolveAllHost (const std::string& host, int family)
{
    for (auto const& server : nameServers ())
    {
        IpAddressList addresses = Resolver ().resolveAllHost (host, family, server);
        if (!addresses.empty ())
        {
            return addresses;
        }
    }

    return {};
}

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : resolveAllHost
// =========================================================================
IpAddressList Resolver::resolveAllHost (const std::string& host, const IpAddress& server, uint16_t port, int timeout)
{
    IpAddressList addresses;

    for (auto const& family : { AF_INET6, AF_INET })
    {
        IpAddressList tmp = resolveAllHost (host, family, server, port, timeout);
        addresses.insert (addresses.end (), tmp.begin (), tmp.end ());
    }

    return addresses;
}

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : resolveAllHost
// =========================================================================
IpAddressList Resolver::resolveAllHost (const std::string& host)
{
    for (auto const& server : nameServers ())
    {
        IpAddressList addresses = Resolver ().resolveAllHost (host, server);
        if (!addresses.empty ())
        {
            return addresses;
        }
    }

    return {};
}

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : resolveHost
// =========================================================================
IpAddress Resolver::resolveHost (const std::string& host, int family, const IpAddress& server, uint16_t port, int timeout)
{
    for (auto const& address : resolveAllHost (host, family, server, port, timeout))
    {
        return address;
    }

    return IpAddress (family);
}

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : resolveHost
// =========================================================================
IpAddress Resolver::resolveHost (const std::string& host, int family)
{
    for (auto const& address : Resolver ().resolveAllHost (host, family))
    {
        return address;
    }

    return IpAddress (family);
}

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : resolveHost
// =========================================================================
IpAddress Resolver::resolveHost (const std::string& host, const IpAddress& server, uint16_t port, int timeout)
{
    for (auto const& address : resolveAllHost (host, server, port, timeout))
    {
        return address;
    }

    return {};
}

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : resolveHost
// =========================================================================
IpAddress Resolver::resolveHost (const std::string& host)
{
    for (auto const& address : resolveAllHost (host))
    {
        return address;
    }

    return {};
}

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : resolveAllAddress
// =========================================================================
AliasList Resolver::resolveAllAddress (const IpAddress& address, const IpAddress& server, uint16_t port, int timeout)
{
    AliasList aliases;

    DnsPacket packet;
    packet.src = IpAddress (server.family ());
    packet.dest = server;
    packet.port = port;

    QuestionRecord question;
    question.host = address.toArpa ();
    question.type = RecordType::PTR;
    question.dnsclass = RecordClass::IN;

    packet.questions.push_back (question);

    if (lookup (packet, timeout) == -1)
    {
        return aliases;
    }

    for (auto const& answer : packet.answers)
    {
        if (!answer.name.empty ())
        {
            aliases.insert (answer.name);
        }
    }

    return aliases;
}

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : resolveAllAddress
// =========================================================================
AliasList Resolver::resolveAllAddress (const IpAddress& address)
{
    for (auto const& server : nameServers ())
    {
        AliasList aliases = Resolver ().resolveAllAddress (address, server);
        if (!aliases.empty ())
        {
            return aliases;
        }
    }

    return {};
}

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : resolveAddress
// =========================================================================
std::string Resolver::resolveAddress (const IpAddress& address, const IpAddress& server, uint16_t port, int timeout)
{
    for (auto const& alias : resolveAllAddress (address, server, port, timeout))
    {
        return alias;
    }

    return {};
}

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : resolveAddress
// =========================================================================
std::string Resolver::resolveAddress (const IpAddress& address)
{
    for (auto const& alias : resolveAllAddress (address))
    {
        return alias;
    }

    return {};
}

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : resolveAllNameServer
// =========================================================================
ServerList Resolver::resolveAllNameServer (const std::string& host, const IpAddress& server, uint16_t port, int timeout)
{
    ServerList names;

    DnsPacket packet;
    packet.src = IpAddress (server.family ());
    packet.dest = server;
    packet.port = port;

    QuestionRecord question;
    question.host = host;
    question.type = RecordType::NS;
    question.dnsclass = RecordClass::IN;

    packet.questions.push_back (question);

    if (lookup (packet, timeout) == -1)
    {
        return names;
    }

    for (auto const& answer : packet.answers)
    {
        if (!answer.name.empty ())
        {
            names.insert (answer.name);
        }
    }

    return names;
}

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : resolveAllNameServer
// =========================================================================
ServerList Resolver::resolveAllNameServer (const std::string& host)
{
    for (auto const& server : nameServers ())
    {
        ServerList names = Resolver ().resolveAllNameServer (host, server);
        if (!names.empty ())
        {
            return names;
        }
    }

    return {};
}

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : resolveNameServer
// =========================================================================
std::string Resolver::resolveNameServer (const std::string& host, const IpAddress& server, uint16_t port, int timeout)
{
    for (auto const& name : resolveAllNameServer (host, server, port, timeout))
    {
        return name;
    }

    return {};
}

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : resolveNameServer
// =========================================================================
std::string Resolver::resolveNameServer (const std::string& host)
{
    for (auto const& name : resolveAllNameServer (host))
    {
        return name;
    }

    return {};
}

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : resolveAuthority
// =========================================================================
std::string Resolver::resolveAuthority (const std::string& host, const IpAddress& server, uint16_t port, int timeout)
{
    DnsPacket packet;
    packet.src = IpAddress (server.family ());
    packet.dest = server;
    packet.port = port;

    QuestionRecord question;
    question.host = host;
    question.type = RecordType::SOA;
    question.dnsclass = RecordClass::IN;

    packet.questions.push_back (question);

    if (lookup (packet, timeout) == 0)
    {
        for (auto const& answer : packet.answers)
        {
            if (!answer.name.empty ())
            {
                return answer.name;
            }
        }
    }

    return {};
}

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : resolveAuthority
// =========================================================================
std::string Resolver::resolveAuthority (const std::string& host)
{
    for (auto const& server : nameServers ())
    {
        std::string name = Resolver ().resolveAuthority (host, server);
        if (!name.empty ())
        {
            return name;
        }
    }

    return {};
}

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : resolveAllMailExchanger
// =========================================================================
ExchangerList Resolver::resolveAllMailExchanger (const std::string& host, const IpAddress& server, uint16_t port, int timeout)
{
    ExchangerList exchangers;

    DnsPacket packet;
    packet.src = IpAddress (server.family ());
    packet.dest = server;
    packet.port = port;

    QuestionRecord question;
    question.host = host;
    question.type = RecordType::MX;
    question.dnsclass = RecordClass::IN;

    packet.questions.push_back (question);

    if (lookup (packet, timeout) == -1)
    {
        return exchangers;
    }

    for (auto const& answer : packet.answers)
    {
        if (!answer.name.empty ())
        {
            exchangers.insert (answer.name);
        }
    }

    return exchangers;
}

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : resolveAllMailExchanger
// =========================================================================
ExchangerList Resolver::resolveAllMailExchanger (const std::string& host)
{
    for (auto const& server : nameServers ())
    {
        ExchangerList aliases = Resolver ().resolveAllMailExchanger (host, server);
        if (!aliases.empty ())
        {
            return aliases;
        }
    }

    return {};
}

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : resolveMailExchanger
// =========================================================================
std::string Resolver::resolveMailExchanger (const std::string& host, const IpAddress& server, uint16_t port, int timeout)
{
    for (auto const& exchanger : resolveAllMailExchanger (host, server, port, timeout))
    {
        return exchanger;
    }

    return {};
}

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : resolveMailExchanger
// =========================================================================
std::string Resolver::resolveMailExchanger (const std::string& host)
{
    for (auto const& exchanger : resolveAllMailExchanger (host))
    {
        return exchanger;
    }

    return {};
}

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : resolveService
// =========================================================================
uint16_t Resolver::resolveService (const std::string& service)
{
    struct servent entry, *res;
    char buffer[1024];

    int status = getservbyname_r (service.c_str (), nullptr, &entry, buffer, sizeof buffer, &res);
    if ((status == 0) && (res != nullptr))
    {
        return ntohs (entry.s_port);
    }

    return 0;
}

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : typeName
// =========================================================================
std::string Resolver::typeName (uint16_t recordType)
{
    switch (recordType)
    {
        OUT_ENUM (A);
        OUT_ENUM (NS);
        OUT_ENUM (CNAME);
        OUT_ENUM (SOA);
        OUT_ENUM (PTR);
        OUT_ENUM (MX);
        OUT_ENUM (AAAA);
    }

    return "UNKNOWN";
}

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : className
// =========================================================================
std::string Resolver::className (uint16_t recordClass)
{
    switch (recordClass)
    {
        OUT_ENUM (IN);
    }

    return "UNKNOWN";
}

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : lookup
// =========================================================================
int Resolver::lookup (DnsPacket& packet, int timeout)
{
    Udp::Socket socket;

    if ((socket.bind (packet.src) == -1) || (socket.bindToDevice (_interface) == -1))
    {
        notify (_onFailure, packet);
        return -1;
    }

    if (socket.connect ({packet.dest, packet.port}) == -1)
    {
        notify (_onFailure, packet);
        return -1;
    }

    std::stringstream data;
    uint16_t reqid = join::randomize <uint16_t> ();
    setHeader (reqid, 1 << 8, packet.questions.size (), 0, 0, 0, data);

    for (auto const& question : packet.questions)
    {
        encodeQuestion (question.host, question.type, question.dnsclass, data);
    }

    if (socket.write (data.str ().c_str (), data.str ().length ()) == -1)
    {
        notify (_onFailure, packet);
        return -1;
    }

    uint16_t resid = 0, flags = 0, qcount = 0, ancount = 0, nscount = 0, arcount = 0;
    auto elapsed = std::chrono::milliseconds::zero ();
    std::unique_ptr <char []> buf;
    
    for (;;)
    {
        auto beg = std::chrono::high_resolution_clock::now ();

        if ((timeout <= elapsed.count ()) || !socket.waitReadyRead (timeout - elapsed.count ()))
        {
            lastError = std::make_error_code (std::errc::timed_out);
            notify (_onFailure, packet);
            return -1;
        }

        buf = std::make_unique <char []> (socket.canRead ());
        if (buf == nullptr)
        {
            lastError = std::make_error_code (std::errc::not_enough_memory);
            notify (_onFailure, packet);
            return -1;
        }

        int size = socket.read (buf.get (), socket.canRead ());
        if (size < 0)
        {
            notify (_onFailure, packet);
            return -1;
        }

        data.rdbuf ()->pubsetbuf (buf.get (), size);
        getHeader (resid, flags, qcount, ancount, nscount, arcount, data);

        if (resid != reqid || !std::bitset <16> (flags).test (15))
        {
            elapsed += std::chrono::duration_cast <std::chrono::milliseconds> (std::chrono::high_resolution_clock::now () - beg);
            continue;
        }

        int error = std::bitset <4> (flags).to_ulong ();
        if (error || ancount == 0)
        {
            lastError = parseError (error);
            notify (_onFailure, packet);
            return -1;
        }

        break;
    }

    packet.questions.clear ();

    for (uint16_t i = 0; i < qcount; ++i)
    {
        packet.questions.emplace_back (decodeQuestion (data));
    }

    for (uint16_t i = 0; i < ancount; ++i)
    {
        packet.answers.emplace_back (decodeAnswer (data));
    }

    for (uint16_t i = 0; i < nscount; ++i)
    {
        packet.authorities.emplace_back (decodeAnswer (data));
    }

    for (uint16_t i = 0; i < arcount; ++i)
    {
        packet.additionals.emplace_back (decodeAnswer (data));
    }

    notify (_onSuccess, packet);

    return 0;
}

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : setHeader
// =========================================================================
void Resolver::setHeader (uint16_t id, uint16_t flags, uint16_t qcount, uint16_t ancount, uint16_t nscount, uint16_t arcount, std::stringstream& data)
{
    id = htons (id);
    data.write (reinterpret_cast <char *> (&id), sizeof (id));

    flags = htons (flags);
    data.write (reinterpret_cast <char *> (&flags), sizeof (flags));

    qcount = htons (qcount);
    data.write (reinterpret_cast <char *> (&qcount), sizeof (qcount));

    ancount = htons (ancount);
    data.write (reinterpret_cast <char *> (&ancount), sizeof (ancount));

    nscount = htons (nscount);
    data.write (reinterpret_cast <char *> (&nscount), sizeof (nscount));

    arcount = htons (arcount);
    data.write (reinterpret_cast <char *> (&arcount), sizeof (arcount));
}

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : getHeader
// =========================================================================
void Resolver::getHeader (uint16_t& id, uint16_t& flags, uint16_t& qcount, uint16_t& ancount, uint16_t& nscount, uint16_t& arcount, std::stringstream& data)
{
    data.read (reinterpret_cast <char *> (&id), sizeof (id));
    id = ntohs (id);

    data.read (reinterpret_cast <char *> (&flags), sizeof (flags));
    flags = ntohs (flags);

    data.read (reinterpret_cast <char *> (&qcount), sizeof (qcount));
    qcount = ntohs (qcount);

    data.read (reinterpret_cast <char *> (&ancount), sizeof (ancount));
    ancount = ntohs (ancount);

    data.read (reinterpret_cast <char *> (&nscount), sizeof (nscount));
    nscount = ntohs (nscount);

    data.read (reinterpret_cast <char *> (&arcount), sizeof (arcount));
    arcount = ntohs (arcount);
}

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : encodeName
// =========================================================================
void Resolver::encodeName (const std::string& host, std::stringstream& data)
{
    std::istringstream iss (host);

    for (std::string token; std::getline (iss, token, '.');)
    {
        data << static_cast <uint8_t> (token.size ());
        data << token;
    }

    data << '\0';
}

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : decodeName
// =========================================================================
std::string Resolver::decodeName (std::stringstream& data)
{
    std::string decoded;

    for (;;)
    {
        auto pos = data.tellg ();

        uint16_t offset = 0;
        data.read (reinterpret_cast <char *> (&offset), sizeof (offset));
        offset = ntohs (offset);

        if (offset & 0xC000)
        {
            pos = data.tellg ();
            data.seekg (std::bitset <14> (offset).to_ulong ());
            decoded += decodeName (data);
            data.seekg (pos);
            break;
        }
        else
        {
            data.seekg (pos);

            uint8_t size = 0;
            data.read (reinterpret_cast <char *> (&size), sizeof (size));

            if (size == 0)
            {
                if (decoded.back () == '.')
                {
                    decoded.pop_back ();
                }
                break;
            }

            decoded.resize (decoded.size () + size);
            data.read (&decoded[decoded.size () - size], size);
            decoded += '.';
        }
    }

    return decoded;
}

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : decodeMail
// =========================================================================
std::string Resolver::decodeMail (std::stringstream& data)
{
    std::string mail = decodeName (data);
    std::size_t pos = 0;

    while ((pos = mail.find (".", pos)) != std::string::npos)
    {
        if (pos > 1 && mail[pos-1] == '/')
        {
            mail.replace (pos-1, 2, ".");
        }
        else
        {
            mail.replace (pos, 1, "@");
            break;
        }
    }

    return mail;
}

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : decodeQuestion
// =========================================================================
void Resolver::encodeQuestion (const std::string& host, uint16_t type, uint16_t dnsclass, std::stringstream& data)
{
    encodeName (host, data);

    type = htons (type);
    data.write (reinterpret_cast <char *> (&type), sizeof (type));

    dnsclass = htons (dnsclass);
    data.write (reinterpret_cast <char *> (&dnsclass), sizeof (dnsclass));
}

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : decodeQuestion
// =========================================================================
QuestionRecord Resolver::decodeQuestion (std::stringstream& data)
{
    QuestionRecord question;

    question.host = decodeName (data);

    data.read (reinterpret_cast <char *> (&question.type), sizeof (question.type));
    question.type = ntohs (question.type);

    data.read (reinterpret_cast <char *> (&question.dnsclass), sizeof (question.dnsclass));
    question.dnsclass = ntohs (question.dnsclass);

    return question;
}

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : decodeAnswer
// =========================================================================
AnswerRecord Resolver::decodeAnswer (std::stringstream& data)
{
    AnswerRecord answer;

    answer.host = decodeName (data);

    data.read (reinterpret_cast <char *> (&answer.type), sizeof (answer.type));
    answer.type = ntohs (answer.type);

    data.read (reinterpret_cast <char *> (&answer.dnsclass), sizeof (answer.dnsclass));
    answer.dnsclass = ntohs (answer.dnsclass);

    data.read (reinterpret_cast <char *> (&answer.ttl), sizeof (answer.ttl));
    answer.ttl = ntohl (answer.ttl);

    uint16_t dataLen = 0;
    data.read (reinterpret_cast <char *> (&dataLen), sizeof (dataLen));
    dataLen = ntohs (dataLen);

    if (answer.type == RecordType::A)
    {
        struct in_addr addr;
        data.read (reinterpret_cast <char *> (&addr), sizeof (addr));
        answer.addr = IpAddress (&addr, sizeof (struct in_addr));
    }
    else if (answer.type == RecordType::NS)
    {
        answer.name = decodeName (data);
    }
    else if (answer.type == RecordType::CNAME)
    {
        answer.name = decodeName (data);
    }
    else if (answer.type == RecordType::SOA)
    {
        answer.name = decodeName (data);

        answer.mail = decodeMail (data);

        data.read (reinterpret_cast <char *> (&answer.serial), sizeof (answer.serial));
        answer.serial = ntohl (answer.serial);

        data.read (reinterpret_cast <char *> (&answer.refresh), sizeof (answer.refresh));
        answer.refresh = ntohl (answer.refresh);

        data.read (reinterpret_cast <char *> (&answer.retry), sizeof (answer.retry));
        answer.retry = ntohl (answer.retry);

        data.read (reinterpret_cast <char *> (&answer.expire), sizeof (answer.expire));
        answer.expire = ntohl (answer.expire);

        data.read (reinterpret_cast <char *> (&answer.minimum), sizeof (answer.minimum));
        answer.minimum = ntohl (answer.minimum);
    }
    else if (answer.type == RecordType::PTR)
    {
        answer.name = decodeName (data);
    }
    else if (answer.type == RecordType::MX)
    {
        data.read (reinterpret_cast <char *> (&answer.mxpref), sizeof (answer.mxpref));
        answer.mxpref = ntohs (answer.mxpref);

        answer.name = decodeName (data);
    }
    else if (answer.type == RecordType::AAAA)
    {
        struct in6_addr addr;
        data.read (reinterpret_cast <char *> (&addr), sizeof (addr));
        answer.addr = IpAddress (&addr, sizeof (struct in6_addr));
    }

    return answer;
}

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : parseError
// =========================================================================
std::error_code Resolver::parseError (int error)
{
    std::error_code code;

    switch (error)
    {
        case 0:
            // The name server was unable to find a matching entry (not a regular DNS error).
        case 3:
            // The domain name referenced in the query does not exist.
            code = make_error_code (Errc::NotFound);
            break;
        case 1:
            // The name server was unable to interpret the query.
        case 4:
            // The name server does not support the requested kind of query.
            code = make_error_code (Errc::InvalidParam);
            break;
        case 2:
            // The name server was unable to process the query due to an internal problem.
            code = make_error_code (Errc::OperationFailed);
            break;
        case 5:
            // The name server refuses to perform the specified operation for policy reasons.
            code = make_error_code (Errc::PermissionDenied);
            break;
        default:
            // The name server answered with an unknown error.
            code = make_error_code (Errc::UnknownError);
            break;
    }

    return code;
}

#ifdef DEBUG
// =========================================================================
//   CLASS     : Resolver
//   METHOD    : defaultOnSuccess
// =========================================================================
void Resolver::defaultOnSuccess (const DnsPacket& packet)
{
    std::cout << std::endl;
    std::cout << "SERVER: " << packet.dest << "#" << packet.port << std::endl;

    std::cout << std::endl;
    std::cout << ";; QUESTION SECTION: " << std::endl;
    for (auto const& question : packet.questions)
    {
        std::cout << question.host;
        std::cout << "  " << typeName (question.type);
        std::cout << "  " << className (question.dnsclass);
        std::cout << std::endl;
    }

    std::cout << std::endl;
    std::cout << ";; ANSWER SECTION: " << std::endl;
    for (auto const& answer : packet.answers)
    {
        std::cout << answer.host;
        std::cout << "  " << typeName (answer.type);
        std::cout << "  " << className (answer.dnsclass);
        std::cout << "  " << answer.ttl;
        if (answer.type == RecordType::A)
        {
            std::cout << "  " << answer.addr;
        }
        else if (answer.type == RecordType::NS)
        {
            std::cout << "  " << answer.name;
        }
        else if (answer.type == RecordType::CNAME)
        {
            std::cout << "  " << answer.name;
        }
        else if (answer.type == RecordType::SOA)
        {
            std::cout << "  " << answer.name;
            std::cout << "  " << answer.mail;
            std::cout << "  " << answer.serial;
            std::cout << "  " << answer.refresh;
            std::cout << "  " << answer.retry;
            std::cout << "  " << answer.expire;
            std::cout << "  " << answer.minimum;
        }
        else if (answer.type == RecordType::PTR)
        {
            std::cout << "  " << answer.name;
        }
        else if (answer.type == RecordType::MX)
        {
            std::cout << "  " << answer.mxpref;
            std::cout << "  " << answer.name;
        }
        else if (answer.type == RecordType::AAAA)
        {
            std::cout << "  " << answer.addr;
        }
        std::cout << std::endl;
    }
}

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : defaultOnFailure
// =========================================================================
void Resolver::defaultOnFailure (const DnsPacket& packet)
{
    std::cout << std::endl;
    std::cout << "SERVER: " << packet.dest << "#" << packet.port << std::endl;

    std::cout << std::endl;
    std::cout << ";; QUESTION SECTION: " << std::endl;
    for (auto const& question : packet.questions)
    {
        std::cout << question.host;
        std::cout << "  " << typeName (question.type);
        std::cout << "  " << className (question.dnsclass);
        std::cout << std::endl;
    }

    std::cout << std::endl;
    std::cout << lastError.message () << std::endl;
}
#endif

// =========================================================================
//   CLASS     : Resolver
//   METHOD    : notify
// =========================================================================
void Resolver::notify (const DnsNotify& function, const DnsPacket& packet)
{
    if (function)
    {
        function (packet);
    }
}
