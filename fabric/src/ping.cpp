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
#include <join/utils.hpp>
#include <join/error.hpp>
#include <join/ping.hpp>

// C++.
#include <algorithm>
#include <iostream>
#include <sstream>
#include <thread>
#include <cmath>

// C.
#include <linux/errqueue.h>
#include <netinet/icmp6.h>
#include <linux/icmp.h>
#include <netinet/ip6.h>
#include <netinet/ip.h>
#include <sys/time.h>

using join::IpAddress;
using join::Icmp;
using join::Dns;
using join::PingAnswer;
using join::PingStats;
using join::Ping;

// =========================================================================
//   CLASS     : PingStats
//   METHOD    : PingStats
// =========================================================================
PingStats::PingStats (const std::string& device, const IpAddress& source, const std::string& host, int size,
                      std::chrono::milliseconds interval, int hop, int df)
: _device (device)
, _source (source)
, _host (host)
, _size (size)
, _interval (interval)
, _hop (hop)
, _df (df)
, _start (std::chrono::steady_clock::now ())
, _stop (_start)
{
}

// =========================================================================
//   CLASS     : PingStats
//   METHOD    : sample
// =========================================================================
void PingStats::sample (std::chrono::microseconds rtt) noexcept
{
    const uint64_t value = static_cast<uint64_t> (rtt.count ());

    ++_samples;

    if ((_samples == 1) || (value < _min))
    {
        _min = value;
    }

    if (value > _max)
    {
        _max = value;
    }

    const double delta = static_cast<double> (value) - _mean;
    _mean += delta / _samples;
    _m2 += delta * (static_cast<double> (value) - _mean);
}

// =========================================================================
//   CLASS     : PingStats
//   METHOD    : mdev
// =========================================================================
std::chrono::microseconds PingStats::mdev () const noexcept
{
    if (_samples == 0)
    {
        return std::chrono::microseconds (0);
    }

    return std::chrono::microseconds (static_cast<uint64_t> (std::sqrt (_m2 / _samples)));
}

// =========================================================================
//   CLASS     : PingStats
//   METHOD    : reset
// =========================================================================
void PingStats::reset () noexcept
{
    _sequence = 0;
    _route = 0;
    _answer = PingAnswer ();
    _sent = 0;
    _received = 0;
    _samples = 0;
    _min = 0;
    _max = 0;
    _mean = 0.0;
    _m2 = 0.0;
    _start = std::chrono::steady_clock::now ();
    _stop = _start;
}

// =========================================================================
//   CLASS     : Ping
//   METHOD    : Ping
// =========================================================================
Ping::Ping (int ttl, Reactor& reactor)
: Ping (std::string (), ttl, reactor)
{
}

// =========================================================================
//   CLASS     : Ping
//   METHOD    : Ping
// =========================================================================
Ping::Ping (const std::string& device, int ttl, Reactor& reactor)
#ifdef DEBUG
: onStart (defaultOnStart)
, onSuccess (defaultOnSuccess)
, onFailure (defaultOnFailure)
, onHop (nullptr)
, onStop (defaultOnStop)
#else
: onStart (nullptr)
, onSuccess (nullptr)
, onFailure (nullptr)
, onHop (nullptr)
, onStop (nullptr)
#endif
, _buffer (std::make_unique<char[]> (_bufferSize))
, _device (device)
, _identity (randomize<uint16_t> ())
, _ttl (ttl)
, _reactor (reactor)
{
}

// =========================================================================
//   CLASS     : Ping
//   METHOD    : ping
// =========================================================================
int Ping::ping (const IpAddress& source, const std::string& destination, int size, int count,
                std::chrono::milliseconds interval, int df, std::chrono::milliseconds timeout)
{
    PingStats stats (_device, source, destination, size, interval, _ttl, df);

    Icmp::Socket socket;

    if (open (socket, stats) == -1)
    {
        notify (onFailure, stats);
        return 0;
    }

    notify (onStart, stats);

    for (int i = 0; i < count; ++i)
    {
        const uint32_t transmitted = stats.sent ();

        if (echo (socket, stats, timeout) == 0)
        {
            notify (onSuccess, stats);
        }
        else
        {
            notify (onFailure, stats);

            if (isFatal (stats, transmitted))
            {
                break;
            }
        }

        if ((i < (count - 1)) && (interval.count () > 0))
        {
            std::this_thread::sleep_for (interval);
        }
    }

    close (socket);

    stats._stop = std::chrono::steady_clock::now ();

    notify (onStop, stats);

    return static_cast<int> (stats.received ());
}

// =========================================================================
//   CLASS     : Ping
//   METHOD    : ping4
// =========================================================================
int Ping::ping4 (const std::string& destination, int size, int count, std::chrono::milliseconds interval, int ttl,
                 int df, std::chrono::milliseconds timeout)
{
    return Ping (ttl).ping (IpAddress (AF_INET), destination, size, count, interval, df, timeout);
}

// =========================================================================
//   CLASS     : Ping
//   METHOD    : ping6
// =========================================================================
int Ping::ping6 (const std::string& destination, int size, int count, std::chrono::milliseconds interval, int ttl,
                 int df, std::chrono::milliseconds timeout)
{
    return Ping (ttl).ping (IpAddress (AF_INET6), destination, size, count, interval, df, timeout);
}

// =========================================================================
//   CLASS     : Ping
//   METHOD    : pathMtu
// =========================================================================
int Ping::pathMtu (const IpAddress& source, const std::string& destination, std::chrono::milliseconds timeout)
{
    const std::chrono::milliseconds interval (static_cast<int> (_retryDelay));

    PingStats stats (_device, source, destination, 0, interval, _ttl, IP_PMTUDISC_DO);

    Icmp::Socket socket;

    if (open (socket, stats) == -1)
    {
        notify (onFailure, stats);
        return -1;
    }

    const int overhead = headerSize (stats);

    int mtu = static_cast<int> (_bufferSize);
    bool narrowed = false;
    int result = -1;

    stats._size = std::max (0, mtu - overhead);

    notify (onStart, stats);

    for (int probe = 0; probe < _maxMtuProbes; ++probe)
    {
        const int previous = mtu;

        for (int retry = 0; retry < _defaultProbes; ++retry)
        {
            const uint32_t transmitted = stats.sent ();

            if (echo (socket, stats, timeout) == 0)
            {
                notify (onSuccess, stats);
                result = (!narrowed && (stats.route () > 0)) ? stats.route () : mtu;
                break;
            }

            notify (onFailure, stats);

            if (stats.mtu () && (static_cast<int> (stats.mtu ()) < mtu))
            {
                mtu = static_cast<int> (stats.mtu ());
                narrowed = true;
                break;
            }

            if (isFatal (stats, transmitted))
            {
                break;
            }

            if (retry < (_defaultProbes - 1))
            {
                std::this_thread::sleep_for (stats.interval ());
            }
        }

        if (mtu == previous)
        {
            break;
        }

        stats._size = std::max (0, mtu - overhead);
    }

    close (socket);

    stats._stop = std::chrono::steady_clock::now ();

    notify (onStop, stats);

    return result;
}

// =========================================================================
//   CLASS     : Ping
//   METHOD    : trace
// =========================================================================
int Ping::trace (const IpAddress& source, const std::string& destination, int maxHops, int probes, int size, int df,
                 std::chrono::milliseconds timeout)
{
    PingStats stats (_device, source, destination, size, std::chrono::milliseconds (0), _ttl, df);

    Icmp::Socket socket;

    if (open (socket, stats) == -1)
    {
        notify (onFailure, stats);
        return -1;
    }

    notify (onStart, stats);

    const int overhead = headerSize (stats);
    int reached = -1, hop = 1;
    bool aborted = false;

    while ((hop <= maxHops) && (reached == -1) && !aborted)
    {
        stats.reset ();
        stats._hop = hop;

        bool narrowed = false;

        for (int probe = 0; (probe < probes) && !narrowed && !aborted; ++probe)
        {
            const uint32_t transmitted = stats.sent ();

            if (echo (socket, stats, timeout) == 0)
            {
                reached = hop;
                notify (onSuccess, stats);
            }
            else if (stats.expired ())
            {
                notify (onSuccess, stats);
            }
            else
            {
                notify (onFailure, stats);

                const int shrunk = std::max (0, static_cast<int> (stats.mtu ()) - overhead);

                narrowed = (stats.mtu () && (shrunk < stats.size ()));

                if (narrowed)
                {
                    stats._size = shrunk;
                }
                else
                {
                    aborted = isFatal (stats, transmitted);
                }
            }
        }

        if (!narrowed)
        {
            notify (onHop, stats);

            ++hop;
        }
    }

    close (socket);

    stats._stop = std::chrono::steady_clock::now ();

    notify (onStop, stats);

    return reached;
}

// =========================================================================
//   CLASS     : Ping
//   METHOD    : isFatal
// =========================================================================
bool Ping::isFatal (const PingStats& stats, uint32_t transmitted)
{
    return (stats.sent () == transmitted) && (stats.code () != Errc::TemporaryError) &&
           (stats.code () != Errc::OutOfMemory);
}

// =========================================================================
//   CLASS     : Ping
//   METHOD    : headerSize
// =========================================================================
int Ping::headerSize (const PingStats& stats)
{
    return ((stats.address ().family () == AF_INET6) ? static_cast<int> (sizeof (struct ip6_hdr))
                                                     : static_cast<int> (sizeof (struct iphdr))) +
           _icmpHeaderSize;
}

// =========================================================================
//   CLASS     : Ping
//   METHOD    : open
// =========================================================================
int Ping::open (Icmp::Socket& socket, PingStats& stats)
{
    if (IpAddress::isIpAddress (stats.host ()))
    {
        stats._address = IpAddress (stats.host ());
    }
    else
    {
        stats._address = Dns::Resolver::lookupAddress (stats.host (), stats.source ().family ());
    }

    if (stats._address.isWildcard () || (stats._address.family () != stats.source ().family ()))
    {
        return fail (stats, make_error_code (Errc::NotFound), "ping: unknown host");
    }

    if (socket.open ((stats.address ().family () == AF_INET6) ? Icmp::v6 () : Icmp::v4 ()) == -1)
    {
        return fail (stats, lastError, "ping: open: " + lastError.message ());  // LCOV_EXCL_LINE
    }

    if (!_device.empty () && (socket.bindToDevice (_device) == -1))
    {
        return fail (stats, lastError, "ping: bind to device: " + lastError.message ());
    }

    if (!stats.source ().isWildcard () && (socket.bind (Icmp::Endpoint (stats.source ())) == -1))
    {
        return fail (stats, lastError, "ping: bind: " + lastError.message ());
    }

    if (socket.connect (Icmp::Endpoint (stats.address ())) == -1)
    {
        return fail (stats, lastError, "ping: connect: " + lastError.message ());
    }

    if (socket.setOption (Icmp::Socket::RcvError, 1) == -1)
    {
        return fail (stats, lastError, "ping: enable error queue: " + lastError.message ());  // LCOV_EXCL_LINE
    }

    if (socket.setOption (Icmp::Socket::TimeStamp, 1) == -1)
    {
        return fail (stats, lastError, "ping: enable timestamp: " + lastError.message ());  // LCOV_EXCL_LINE
    }

    if (socket.setOption (Icmp::Socket::PathMtuDiscover, stats.df ()) == -1)
    {
        return fail (stats, lastError, "ping: set path mtu discovery: " + lastError.message ());  // LCOV_EXCL_LINE
    }

    const bool inet6 = (socket.family () == AF_INET6);

    int on = 1;

    if (inet6 && (::setsockopt (socket.handle (), IPPROTO_IPV6, IPV6_RECVHOPLIMIT, &on, sizeof (on)) == -1))
    {
        // LCOV_EXCL_START
        std::error_code code (errno, std::generic_category ());
        return fail (stats, code, "ping: enable hop limit: " + code.message ());
        // LCOV_EXCL_STOP
    }

    if (setFilter (socket, stats) == -1)
    {
        return -1;  // LCOV_EXCL_LINE
    }

    if (_reactor.addHandler (socket.handle (), this) == -1)
    {
        return fail (stats, lastError, "ping: add handler: " + lastError.message ());  // LCOV_EXCL_LINE
    }

    return 0;
}

// =========================================================================
//   CLASS     : Ping
//   METHOD    : setFilter
// =========================================================================
int Ping::setFilter (Icmp::Socket& socket, PingStats& stats)
{
    if (socket.family () == AF_INET6)
    {
        struct icmp6_filter filter;
        ICMP6_FILTER_SETBLOCKALL (&filter);
        ICMP6_FILTER_SETPASS (ICMP6_ECHO_REPLY, &filter);

        if (::setsockopt (socket.handle (), IPPROTO_ICMPV6, ICMP6_FILTER, &filter, sizeof (filter)) == -1)
        {
            // LCOV_EXCL_START
            std::error_code code (errno, std::generic_category ());
            return fail (stats, code, "ping: set icmp message filter: " + code.message ());
            // LCOV_EXCL_STOP
        }

        return 0;
    }

    struct icmp_filter filter;
    filter.data = ~(1 << ICMP_ECHOREPLY);

    if (::setsockopt (socket.handle (), SOL_RAW, ICMP_FILTER, &filter, sizeof (filter)) == -1)
    {
        // LCOV_EXCL_START
        std::error_code code (errno, std::generic_category ());
        return fail (stats, code, "ping: set icmp message filter: " + code.message ());
        // LCOV_EXCL_STOP
    }

    return 0;
}

// =========================================================================
//   CLASS     : Ping
//   METHOD    : close
// =========================================================================
void Ping::close (Icmp::Socket& socket)
{
    _reactor.delHandler (socket.handle ());
    socket.close ();
}

// =========================================================================
//   CLASS     : Ping
//   METHOD    : echo
// =========================================================================
int Ping::echo (Icmp::Socket& socket, PingStats& stats, std::chrono::milliseconds timeout)
{
    if (socket.setOption (Icmp::Socket::Ttl, stats.hop ()) == -1)
    {
        return fail (stats, lastError, "ping: set time to live: " + lastError.message ());
    }

    stats._route = socket.mtu ();

    const int size = stats.size () + _icmpHeaderSize;

    uint16_t sequence = 0;
    PendingRequest* request = nullptr;

    {
        ScopedLock<Mutex> lock (_syncMutex);

        sequence = ++_sequence;

        auto inserted = _pending.emplace (sequence, std::make_unique<PendingRequest> ());
        if (inserted.second)
        {
            request = inserted.first->second.get ();
        }
    }

    if (request == nullptr)
    {
        return fail (stats, make_error_code (Errc::InUse), "ping: no sequence number available");  // LCOV_EXCL_LINE
    }

    const bool stamped = stats.size () >= static_cast<int> (sizeof (struct timeval));

    auto data = std::make_unique<char[]> (size);

    for (int i = 0; i < stats.size (); ++i)
    {
        data[i + _icmpHeaderSize] = static_cast<char> (i);
    }

    if (stamped)
    {
        ::gettimeofday (reinterpret_cast<struct timeval*> (data.get () + _icmpHeaderSize), nullptr);
    }

    if (socket.family () == AF_INET6)
    {
        struct icmp6_hdr* icmp = reinterpret_cast<struct icmp6_hdr*> (data.get ());
        icmp->icmp6_type = ICMP6_ECHO_REQUEST;
        icmp->icmp6_code = 0;
        icmp->icmp6_cksum = 0;
        icmp->icmp6_seq = htons (sequence);
        icmp->icmp6_id = htons (_identity);
    }
    else
    {
        struct icmphdr* icmp = reinterpret_cast<struct icmphdr*> (data.get ());
        icmp->type = ICMP_ECHO;
        icmp->code = 0;
        icmp->checksum = 0;
        icmp->un.echo.sequence = htons (sequence);
        icmp->un.echo.id = htons (_identity);
        icmp->checksum = join::checksum (reinterpret_cast<uint16_t*> (icmp), size, 0);
    }

    int result = -1;

    for (int attempt = 0; attempt < 2; ++attempt)
    {
        if (attempt)
        {
            std::this_thread::sleep_for (std::chrono::milliseconds (static_cast<int> (_retryDelay)));  // LCOV_EXCL_LINE
        }

        ScopedLock<Mutex> lock (_syncMutex);

        request->sent = std::chrono::steady_clock::now ();

        int written = socket.write (data.get (), size);

        if ((written == -1) && (lastError == std::errc::no_buffer_space) && (attempt == 0))
        {
            continue;  // LCOV_EXCL_LINE
        }

        if (written == -1)
        {
            std::string error = "ping: write: " + lastError.message ();
            std::error_code code = lastError;
            uint32_t mtu = 0;

            if (code == std::errc::message_size)
            {
                int value = socket.mtu ();
                if (value > 0)
                {
                    mtu = static_cast<uint32_t> (value);
                    error += " mtu=" + std::to_string (value);
                }
            }

            _pending.erase (sequence);
            fail (stats, code, error);
            stats._answer.mtu = mtu;

            break;
        }

#ifdef DEBUG
        dump (data.get (), std::min (size, static_cast<int> (_dumpSize)));
#endif

        ++stats._sent;
        stats._sequence = sequence;

        if (!request->cond.timedWait (lock, timeout, [request] () {
                return request->answered;
            }))
        {
            _pending.erase (sequence);
            fail (stats, make_error_code (Errc::TimedOut), "ping: timed out");

            break;
        }

        stats._answer = request->answer;
        _pending.erase (sequence);

        const bool replied = !stats._answer.code;

        if (replied || stats._answer.expired)
        {
            stats.sample (stats._answer.rtt);
        }

        if (replied)
        {
            ++stats._received;
            result = 0;
        }
        else
        {
            lastError = stats._answer.code;
        }

        break;
    }

    return result;
}

// =========================================================================
//   CLASS     : Ping
//   METHOD    : onReadable
// =========================================================================
void Ping::onReadable (int fd)
{
    struct sockaddr_storage addr = {};
    char control[CMSG_SPACE (sizeof (int)) + CMSG_SPACE (sizeof (struct timeval))];

    struct iovec iov;
    iov.iov_base = _buffer.get ();
    iov.iov_len = _bufferSize;

    struct msghdr msg;
    msg.msg_name = &addr;
    msg.msg_namelen = sizeof (addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = control;
    msg.msg_controllen = sizeof (control);
    msg.msg_flags = 0;

    int packetSize = ::recvmsg (fd, &msg, 0);
    if ((packetSize < 0) || (msg.msg_flags & MSG_TRUNC))
    {
        return;  // LCOV_EXCL_LINE
    }

    const int family = addr.ss_family;
    if ((family != AF_INET) && (family != AF_INET6))
    {
        return;  // LCOV_EXCL_LINE
    }

#ifdef DEBUG
    dump (_buffer.get (), std::min (packetSize, static_cast<int> (_dumpSize)));
#endif

    int ttl = 0;
    struct timeval* received = nullptr;

    if ((msg.msg_flags & MSG_CTRUNC) == 0)
    {
        for (struct cmsghdr* cmsg = CMSG_FIRSTHDR (&msg); cmsg != nullptr; cmsg = CMSG_NXTHDR (&msg, cmsg))
        {
            if ((cmsg->cmsg_level == SOL_SOCKET) && (cmsg->cmsg_type == SO_TIMESTAMP))
            {
                received = reinterpret_cast<struct timeval*> (CMSG_DATA (cmsg));
            }
            else if ((cmsg->cmsg_level == IPPROTO_IPV6) && (cmsg->cmsg_type == IPV6_HOPLIMIT))
            {
                ::memcpy (&ttl, CMSG_DATA (cmsg), sizeof (ttl));
            }
        }
    }

    uint32_t offset = 0;
    uint16_t sequence = 0;

    if (family == AF_INET6)
    {
        if (packetSize < _icmpHeaderSize)
        {
            return;  // LCOV_EXCL_LINE
        }

        struct icmp6_hdr* icmp = reinterpret_cast<struct icmp6_hdr*> (_buffer.get ());
        if ((icmp->icmp6_type != ICMP6_ECHO_REPLY) || (ntohs (icmp->icmp6_id) != _identity))
        {
            return;  // LCOV_EXCL_LINE
        }

        sequence = ntohs (icmp->icmp6_seq);
    }
    else
    {
        if (packetSize < static_cast<int> (sizeof (struct iphdr)))
        {
            return;  // LCOV_EXCL_LINE
        }

        struct iphdr* ip = reinterpret_cast<struct iphdr*> (_buffer.get ());
        offset = ip->ihl * 4;
        packetSize -= offset;
        ttl = ip->ttl;

        if (packetSize < _icmpHeaderSize)
        {
            return;  // LCOV_EXCL_LINE
        }

        struct icmphdr* icmp = reinterpret_cast<struct icmphdr*> (_buffer.get () + offset);

        if ((icmp->type != ICMP_ECHOREPLY) || (ntohs (icmp->un.echo.id) != _identity) ||
            join::checksum (reinterpret_cast<uint16_t*> (icmp), packetSize, 0))
        {
            return;  // LCOV_EXCL_LINE
        }

        sequence = ntohs (icmp->un.echo.sequence);
    }

    const int dataSize = packetSize - _icmpHeaderSize;
    const bool stamped = dataSize >= static_cast<int> (sizeof (struct timeval));
    const char* payload = _buffer.get () + offset + _icmpHeaderSize;

    for (int i = stamped ? static_cast<int> (sizeof (struct timeval)) : 0; i < dataSize; ++i)
    {
        if (payload[i] != static_cast<char> (i))
        {
            return;  // LCOV_EXCL_LINE
        }
    }

    std::chrono::microseconds rtt{0};

    if ((received != nullptr) && stamped)
    {
        struct timeval stamp;
        ::memcpy (&stamp, payload, sizeof (stamp));

        auto begin = std::chrono::seconds (stamp.tv_sec) + std::chrono::microseconds (stamp.tv_usec);
        auto end = std::chrono::seconds (received->tv_sec) + std::chrono::microseconds (received->tv_usec);

        if (end > begin)
        {
            rtt = std::chrono::duration_cast<std::chrono::microseconds> (end - begin);
        }
    }

    ScopedLock<Mutex> lock (_syncMutex);

    auto it = _pending.find (sequence);
    if (it != _pending.end ())
    {
        PendingRequest& request = *it->second;

        request.answer.from = IpAddress (*reinterpret_cast<struct sockaddr*> (&addr));
        request.answer.ttl = ttl;
        request.answer.size = packetSize;
        request.answer.rtt = rtt;
        request.answered = true;
        request.cond.signal ();
    }
}

// =========================================================================
//   CLASS     : Ping
//   METHOD    : onError
// =========================================================================
void Ping::onError (int fd)
{
    char data[_icmpHeaderSize];
    char control[CMSG_SPACE (sizeof (struct sock_extended_err) + sizeof (struct sockaddr_storage)) +
                 CMSG_SPACE (sizeof (struct timeval))];

    struct iovec iov;
    iov.iov_base = data;
    iov.iov_len = sizeof (data);

    struct msghdr msg;
    msg.msg_name = nullptr;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = control;
    msg.msg_controllen = sizeof (control);
    msg.msg_flags = 0;

    if (::recvmsg (fd, &msg, MSG_ERRQUEUE) < _icmpHeaderSize)
    {
        return;  // LCOV_EXCL_LINE
    }

    if (msg.msg_flags & MSG_CTRUNC)
    {
        return;  // LCOV_EXCL_LINE
    }

    struct sock_extended_err* error = nullptr;

    for (struct cmsghdr* cmsg = CMSG_FIRSTHDR (&msg); cmsg != nullptr; cmsg = CMSG_NXTHDR (&msg, cmsg))
    {
        if (((cmsg->cmsg_level == IPPROTO_IPV6) && (cmsg->cmsg_type == IPV6_RECVERR)) ||
            ((cmsg->cmsg_level == IPPROTO_IP) && (cmsg->cmsg_type == IP_RECVERR)))
        {
            error = reinterpret_cast<struct sock_extended_err*> (CMSG_DATA (cmsg));
        }
    }

    if (error == nullptr)
    {
        return;  // LCOV_EXCL_LINE
    }

    int family = AF_INET;
    uint16_t sequence = 0;

    if (error->ee_origin == SO_EE_ORIGIN_ICMP6)
    {
        struct icmp6_hdr* icmp = reinterpret_cast<struct icmp6_hdr*> (data);
        if ((icmp->icmp6_type != ICMP6_ECHO_REQUEST) || (ntohs (icmp->icmp6_id) != _identity))
        {
            return;  // LCOV_EXCL_LINE
        }

        family = AF_INET6;
        sequence = ntohs (icmp->icmp6_seq);
    }
    else if (error->ee_origin == SO_EE_ORIGIN_ICMP)
    {
        struct icmphdr* icmp = reinterpret_cast<struct icmphdr*> (data);
        if ((icmp->type != ICMP_ECHO) || (ntohs (icmp->un.echo.id) != _identity))
        {
            return;  // LCOV_EXCL_LINE
        }

        family = AF_INET;
        sequence = ntohs (icmp->un.echo.sequence);
    }
    else
    {
        return;  // LCOV_EXCL_LINE
    }

    const bool expired =
        (family == AF_INET6) ? (error->ee_type == ICMP6_TIME_EXCEEDED) : (error->ee_type == ICMP_TIME_EXCEEDED);

    const bool tooBig = (family == AF_INET6)
                            ? (error->ee_type == ICMP6_PACKET_TOO_BIG)
                            : ((error->ee_type == ICMP_DEST_UNREACH) && (error->ee_code == ICMP_FRAG_NEEDED));

    const IpAddress from (*SO_EE_OFFENDER (error));

    std::ostringstream os;
    os << "from " << from << " icmp_seq=" << sequence << " " << message (family, error->ee_type, error->ee_code);

    uint32_t mtu = 0;

    if (tooBig)
    {
        mtu = error->ee_info;
        os << " mtu=" << mtu;
    }

    std::error_code code = make_error_code (Errc::ConnectionRefused);

    if (tooBig)
    {
        code = make_error_code (Errc::MessageTooLong);
    }
    else if (expired)
    {
        code = make_error_code (Errc::TimedOut);
    }

    ScopedLock<Mutex> lock (_syncMutex);

    auto it = _pending.find (sequence);
    if (it != _pending.end ())
    {
        PendingRequest& request = *it->second;

        request.answer.from = from;
        request.answer.rtt =
            std::chrono::duration_cast<std::chrono::microseconds> (std::chrono::steady_clock::now () - request.sent);
        request.answer.mtu = mtu;
        request.answer.expired = expired;
        request.answer.error = os.str ();
        request.answer.code = code;
        request.answered = true;
        request.cond.signal ();
    }
}

// =========================================================================
//   CLASS     : Ping
//   METHOD    : message
// =========================================================================
const char* Ping::message (int family, uint8_t type, uint8_t code) noexcept
{
    if (family == AF_INET6)
    {
        switch (type)
        {
            case ICMP6_DST_UNREACH:
                switch (code)
                {
                    case ICMP6_DST_UNREACH_NOROUTE:
                        return "destination unreachable: no route";
                    case ICMP6_DST_UNREACH_ADMIN:
                        return "destination unreachable: administratively prohibited";
                    case ICMP6_DST_UNREACH_BEYONDSCOPE:
                        return "destination unreachable: beyond scope of source address";
                    case ICMP6_DST_UNREACH_ADDR:
                        return "destination unreachable: address unreachable";
                    case ICMP6_DST_UNREACH_NOPORT:
                        return "destination unreachable: port unreachable";
                    default:
                        return "destination unreachable";
                }

            case ICMP6_PACKET_TOO_BIG:
                return "packet too big";

            case ICMP6_TIME_EXCEEDED:
                switch (code)
                {
                    case ICMP6_TIME_EXCEED_TRANSIT:
                        return "time exceeded: hop limit";
                    case ICMP6_TIME_EXCEED_REASSEMBLY:
                        return "time exceeded: defragmentation failure";
                    default:
                        return "time exceeded";
                }

            case ICMP6_PARAM_PROB:
                switch (code)
                {
                    case ICMP6_PARAMPROB_HEADER:
                        return "parameter problem: wrong header field";
                    case ICMP6_PARAMPROB_NEXTHEADER:
                        return "parameter problem: unknown header";
                    case ICMP6_PARAMPROB_OPTION:
                        return "parameter problem: unknown option";
                    default:
                        return "parameter problem";
                }

            default:
                return "unknown error";
        }
    }

    switch (type)
    {
        case ICMP_DEST_UNREACH:
            switch (code)
            {
                case ICMP_NET_UNREACH:
                    return "destination net unreachable";
                case ICMP_HOST_UNREACH:
                    return "destination host unreachable";
                case ICMP_PROT_UNREACH:
                    return "destination protocol unreachable";
                case ICMP_PORT_UNREACH:
                    return "destination port unreachable";
                case ICMP_FRAG_NEEDED:
                    return "fragmentation needed and don't fragment set";
                case ICMP_SR_FAILED:
                    return "source route failed";
                case ICMP_NET_UNKNOWN:
                    return "destination net unknown";
                case ICMP_HOST_UNKNOWN:
                    return "destination host unknown";
                case ICMP_HOST_ISOLATED:
                    return "source host isolated";
                case ICMP_NET_ANO:
                    return "destination net prohibited";
                case ICMP_HOST_ANO:
                    return "destination host prohibited";
                case ICMP_NET_UNR_TOS:
                    return "destination net unreachable for type of service";
                case ICMP_HOST_UNR_TOS:
                    return "destination host unreachable for type of service";
                case ICMP_PKT_FILTERED:
                    return "packet filtered";
                case ICMP_PREC_VIOLATION:
                    return "precedence violation";
                case ICMP_PREC_CUTOFF:
                    return "precedence cutoff";
                default:
                    return "destination unreachable";
            }

        case ICMP_TIME_EXCEEDED:
            switch (code)
            {
                case ICMP_EXC_TTL:
                    return "time to live exceeded";
                case ICMP_EXC_FRAGTIME:
                    return "fragment reassembly time exceeded";
                default:
                    return "time exceeded";
            }

        case ICMP_PARAMETERPROB:
            return "parameter problem";

        case ICMP_REDIRECT:
            return "redirect";

        default:
            return "unknown error";
    }
}

// =========================================================================
//   CLASS     : Ping
//   METHOD    : notify
// =========================================================================
void Ping::notify (const PingNotify& function, const PingStats& stats)
{
    if (function)
    {
        function (stats);
    }
}

// =========================================================================
//   CLASS     : Ping
//   METHOD    : fail
// =========================================================================
int Ping::fail (PingStats& stats, const std::error_code& code, const std::string& error)
{
    stats._answer = PingAnswer ();
    stats._answer.code = code;
    stats._answer.error = error;
    lastError = code;
    return -1;
}

#ifdef DEBUG

// =========================================================================
//   CLASS     : Ping
//   METHOD    : defaultOnStart
// =========================================================================
void Ping::defaultOnStart (const PingStats& stats)
{
    std::cout << "PING " << stats.host () << " (" << stats.address () << ") ";

    if (!stats.source ().isWildcard () || !stats.device ().empty ())
    {
        std::cout << "from " << stats.source () << " " << stats.device () << ": ";
    }

    std::cout << stats.size () << " data bytes" << std::endl;
}

// =========================================================================
//   CLASS     : Ping
//   METHOD    : defaultOnSuccess
// =========================================================================
void Ping::defaultOnSuccess (const PingStats& stats)
{
    if (stats.expired ())
    {
        std::cout << "from " << stats.from () << ":";
        std::cout << " icmp_seq=" << stats.sequence ();
        std::cout << " hop=" << stats.hop ();
        std::cout << " time=" << (stats.rtt ().count () / 1000.0) << " ms" << std::endl;
        return;
    }

    std::cout << stats.packetSize () << " bytes from " << stats.from () << ":";
    std::cout << " icmp_seq=" << stats.sequence ();
    std::cout << " ttl=" << stats.ttl ();
    std::cout << " time=" << (stats.rtt ().count () / 1000.0) << " ms" << std::endl;
}

// =========================================================================
//   CLASS     : Ping
//   METHOD    : defaultOnFailure
// =========================================================================
void Ping::defaultOnFailure (const PingStats& stats)
{
    std::cout << stats.error () << std::endl;
}

// =========================================================================
//   CLASS     : Ping
//   METHOD    : defaultOnStop
// =========================================================================
void Ping::defaultOnStop (const PingStats& stats)
{
    std::cout << "--- " << stats.host () << " ping statistics ---" << std::endl;

    std::cout << stats.sent () << " packets transmitted, ";
    std::cout << stats.received () << " received, ";
    std::cout << stats.loss () << "% packet loss, time ";
    std::cout << (stats.total ().count () / 1000.0) << " ms" << std::endl;

    std::cout << "rtt min/avg/max/mdev = ";
    std::cout << (stats.min ().count () / 1000.0) << "/";
    std::cout << (stats.avg ().count () / 1000.0) << "/";
    std::cout << (stats.max ().count () / 1000.0) << "/";
    std::cout << (stats.mdev ().count () / 1000.0) << " ms" << std::endl;
}

#endif
