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
#include <join/netlinkmanager.hpp>

using join::Reactor;
using join::NetlinkManager;

// =========================================================================
//   CLASS     : NetlinkManager
//   METHOD    : NetlinkManager
// =========================================================================
NetlinkManager::NetlinkManager (uint32_t groups, Reactor* reactor)
: _buffer (std::make_unique<char[]> (_bufferSize))
, _seq (0)
, _reactor (reactor)
{
    open (Netlink::rt ());
    bind (groups);
    if (_reactor == nullptr)
    {
        _reactor = ReactorThread::reactor ();
    }
}

// =========================================================================
//   CLASS     : NetlinkManager
//   METHOD    : reactor
// =========================================================================
Reactor* NetlinkManager::reactor () const noexcept
{
    return _reactor;
}

// =========================================================================
//   CLASS     : NetlinkManager
//   METHOD    : start
// =========================================================================
void NetlinkManager::start ()
{
    _reactor->addHandler (handle (), this);
}

// =========================================================================
//   CLASS     : NetlinkManager
//   METHOD    : stop
// =========================================================================
void NetlinkManager::stop ()
{
    _reactor->delHandler (handle ());
}

// =========================================================================
//   CLASS     : NetlinkManager
//   METHOD    : sendRequest
// =========================================================================
int NetlinkManager::sendRequest (struct nlmsghdr* nlh, bool sync, std::chrono::milliseconds timeout)
{
    if (!sync)
    {
        if (write (reinterpret_cast<const char*> (nlh), nlh->nlmsg_len) == -1)
        {
            return -1;
        }

        return 0;
    }

    ScopedLock<Mutex> lock (_syncMutex);

    if (write (reinterpret_cast<const char*> (nlh), nlh->nlmsg_len) == -1)
    {
        return -1;
    }

    return waitResponse (lock, nlh->nlmsg_seq, timeout);
}

// =========================================================================
//   CLASS     : NetlinkManager
//   METHOD    : onReceive
// =========================================================================
void NetlinkManager::onReceive ([[maybe_unused]] int fd)
{
    ssize_t len = read (_buffer.get (), _bufferSize);
    if (len != -1)
    {
        struct nlmsghdr* nlh = reinterpret_cast<struct nlmsghdr*> (_buffer.get ());
        while (NLMSG_OK (nlh, len))
        {
            if (nlh->nlmsg_type == NLMSG_DONE)
            {
                notifyRequest (nlh->nlmsg_seq, 0);
                break;
            }

            if (nlh->nlmsg_type == NLMSG_ERROR)
            {
                struct nlmsgerr* err = static_cast<struct nlmsgerr*> (NLMSG_DATA (nlh));
                notifyRequest (err->msg.nlmsg_seq, -err->error);
                nlh = NLMSG_NEXT (nlh, len);
                continue;
            }

            onMessage (nlh);

            nlh = NLMSG_NEXT (nlh, len);
        }
    }
}

// =========================================================================
//   CLASS     : NetlinkManager
//   METHOD    : notifyRequest
// =========================================================================
void NetlinkManager::notifyRequest (uint32_t seq, int error)
{
    ScopedLock<Mutex> lock (_syncMutex);

    auto it = _pending.find (seq);
    if (it != _pending.end ())
    {
        it->second->error = error;
        it->second->cond.signal ();
    }
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
    rta->rta_len  = len;
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
