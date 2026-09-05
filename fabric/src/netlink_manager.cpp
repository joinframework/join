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
#include <join/netlink_manager.hpp>

// C++.
#include <system_error>

using join::Function;
using join::Reactor;
using join::NetlinkManager;

// =========================================================================
//   CLASS     : NetlinkManager
//   METHOD    : NetlinkManager
// =========================================================================
NetlinkManager::NetlinkManager (uint32_t groups, Reactor& reactor)
: _buffer (std::make_unique<char[]> (_bufferSize))
, _seq (0)
, _reactor (reactor)
{
    _socket.open (Netlink::rt ());
    _socket.bind (groups);
    _socket.setOption (Netlink::Socket::RcvBuffer, _rcvBufferSize);
}

// =========================================================================
//   CLASS     : NetlinkManager
//   METHOD    : reactor
// =========================================================================
Reactor& NetlinkManager::reactor () const noexcept
{
    return _reactor;
}

// =========================================================================
//   CLASS     : NetlinkManager
//   METHOD    : start
// =========================================================================
void NetlinkManager::start ()
{
    _reactor.addHandler (_socket.handle (), this);
}

// =========================================================================
//   CLASS     : NetlinkManager
//   METHOD    : stop
// =========================================================================
void NetlinkManager::stop ()
{
    _reactor.delHandler (_socket.handle ());
}

// =========================================================================
//   CLASS     : NetlinkManager
//   METHOD    : sendRequest
// =========================================================================
int NetlinkManager::sendRequest (struct nlmsghdr* nlh, bool sync, std::chrono::milliseconds timeout)
{
    PendingRequest* request = nullptr;

    if (sync)
    {
        ScopedLock<Mutex> lock (_syncMutex);

        auto inserted = _pending.emplace (nlh->nlmsg_seq, std::make_unique<PendingRequest> ());
        if (!inserted.second)
        {
            // LCOV_EXCL_START
            lastError = make_error_code (Errc::OperationFailed);
            return -1;
            // LCOV_EXCL_STOP
        }

        request = inserted.first->second.get ();
    }

    int result = 0;

    if ((nlh->nlmsg_flags & NLM_F_DUMP) == NLM_F_DUMP)
    {
        std::error_code error;

        Function<void ()> fn = [this, nlh, &result, &error] () {
            result = _socket.write (reinterpret_cast<const char*> (nlh), nlh->nlmsg_len);
            error = lastError;
        };
        _reactor.invoke (&fn);

        if (result == -1)
        {
            lastError = error;  // LCOV_EXCL_LINE
        }
    }
    else
    {
        result = _socket.write (reinterpret_cast<const char*> (nlh), nlh->nlmsg_len);
    }

    if (!sync)
    {
        return (result == -1) ? -1 : 0;
    }

    ScopedLock<Mutex> lock (_syncMutex);

    if (result == -1)
    {
        // LCOV_EXCL_START
        _pending.erase (nlh->nlmsg_seq);
        return -1;
        // LCOV_EXCL_STOP
    }

    if (!request->cond.timedWait (lock, timeout, [request] () {
            return request->done;
        }))
    {
        // LCOV_EXCL_START
        _pending.erase (nlh->nlmsg_seq);
        lastError = make_error_code (Errc::TimedOut);
        return -1;
        // LCOV_EXCL_STOP
    }

    if (request->error)
    {
        lastError = request->error;
        _pending.erase (nlh->nlmsg_seq);
        return -1;
    }

    _pending.erase (nlh->nlmsg_seq);

    return 0;
}

// =========================================================================
//   CLASS     : NetlinkManager
//   METHOD    : onReadable
// =========================================================================
void NetlinkManager::onReadable ([[maybe_unused]] int fd)
{
    ssize_t len = _socket.read (_buffer.get (), _bufferSize);
    if (len == -1)
    {
        // LCOV_EXCL_START
        notifyAllRequests (lastError);
        return;
        // LCOV_EXCL_STOP
    }

    struct nlmsghdr* nlh = reinterpret_cast<struct nlmsghdr*> (_buffer.get ());
    while (NLMSG_OK (nlh, len))
    {
        if (nlh->nlmsg_type == NLMSG_DONE)
        {
            notifyRequest (nlh->nlmsg_seq);
        }
        else if (nlh->nlmsg_type == NLMSG_ERROR)
        {
            struct nlmsgerr* err = static_cast<struct nlmsgerr*> (NLMSG_DATA (nlh));
            notifyRequest (err->msg.nlmsg_seq, std::error_code (-err->error, std::generic_category ()));
        }
        else
        {
            onMessage (nlh);
        }

        nlh = NLMSG_NEXT (nlh, len);
    }
}

// =========================================================================
//   CLASS     : NetlinkManager
//   METHOD    : notifyRequest
// =========================================================================
void NetlinkManager::notifyRequest (uint32_t seq, const std::error_code& error)
{
    ScopedLock<Mutex> lock (_syncMutex);

    auto it = _pending.find (seq);
    if (it != _pending.end ())
    {
        it->second->error = error;
        it->second->done = true;
        it->second->cond.signal ();
    }
}

// =========================================================================
//   CLASS     : NetlinkManager
//   METHOD    : notifyAllRequests
// =========================================================================
void NetlinkManager::notifyAllRequests (const std::error_code& error)
{
    // LCOV_EXCL_START
    ScopedLock<Mutex> lock (_syncMutex);

    for (auto& entry : _pending)
    {
        entry.second->error = error;
        entry.second->done = true;
        entry.second->cond.signal ();
    }
    // LCOV_EXCL_STOP
}

// =========================================================================
//   CLASS     : NetlinkManager
//   METHOD    : addAttributes
// =========================================================================
void NetlinkManager::addAttributes (struct nlmsghdr* nlh, int type, const void* data, int alen)
{
    int len = RTA_LENGTH (alen);
    struct rtattr* rta =
        reinterpret_cast<struct rtattr*> (reinterpret_cast<char*> (nlh) + NLMSG_ALIGN (nlh->nlmsg_len));
    rta->rta_type = type;
    rta->rta_len = len;
    memcpy (RTA_DATA (rta), data, alen);
    nlh->nlmsg_len = NLMSG_ALIGN (nlh->nlmsg_len) + RTA_ALIGN (len);
}

// =========================================================================
//   CLASS     : NetlinkManager
//   METHOD    : startNestedAttributes
// =========================================================================
struct rtattr* NetlinkManager::startNestedAttributes (struct nlmsghdr* nlh, int type)
{
    struct rtattr* nested =
        reinterpret_cast<struct rtattr*> (reinterpret_cast<char*> (nlh) + NLMSG_ALIGN (nlh->nlmsg_len));
    addAttributes (nlh, type, nullptr, 0);
    return nested;
}

// =========================================================================
//   CLASS     : NetlinkManager
//   METHOD    : stopNestedAttributes
// =========================================================================
int NetlinkManager::stopNestedAttributes (struct nlmsghdr* nlh, struct rtattr* nested)
{
    nested->rta_len = reinterpret_cast<char*> (nlh) + NLMSG_ALIGN (nlh->nlmsg_len) - reinterpret_cast<char*> (nested);
    return nlh->nlmsg_len;
}
