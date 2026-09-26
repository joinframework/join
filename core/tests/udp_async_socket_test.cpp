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
#include <join/async_datagram_socket.hpp>
#include <join/condition.hpp>

// Libraries.
#include <gtest/gtest.h>

// C.
#include <unistd.h>

using join::Errc;
using join::Mutex;
using join::Condition;
using join::ScopedLock;
using join::IpAddress;
using join::Udp;
using join::Thread;
using join::Proactor;
using join::LocalMem;

/**
 * @brief Class used to test the udp asynchronous datagram socket API.
 */
class UdpAsyncSocket : public ::testing::Test
{
protected:
    /**
     * @brief Sets up the test fixture.
     */
    void SetUp () override
    {
        ASSERT_EQ (server ().bind ({IpAddress::ipv6Wildcard, _port}), 0) << join::lastError.message ();
        ASSERT_NE (server ().asyncReadFrom (onEchoRead, _echobuf, sizeof (_echobuf), _echofrom), -1)
            << join::lastError.message ();

        ScopedLock<Mutex> lock (_mut);

        _code = {};
        _completions = 0;
        _transferred = 0;
        _more = false;
        _rearms = 0;
        _ttl = -1;
    }

    /**
     * @brief Tears down the test fixture.
     */
    void TearDown () override
    {
        server ().close ();
    }

    /**
     * @brief get the echo server socket.
     * @return the echo server socket.
     */
    static Udp::AsyncSocket& server ()
    {
        static Udp::AsyncSocket sock;
        return sock;
    }

    /**
     * @brief send back the datagram received by the echo server.
     * @param ec error reported by the socket.
     * @param data buffer holding the data received.
     * @param size number of bytes read.
     * @param from endpoint the datagram was received from.
     * @param control control messages received.
     * @param controlSize size of the control messages.
     * @param more true if the read stays armed.
     */
    static void onEchoRead (const std::error_code& ec, [[maybe_unused]] const char* data, size_t size,
                            [[maybe_unused]] const Udp::Endpoint& from, [[maybe_unused]] const char* control,
                            [[maybe_unused]] size_t controlSize, [[maybe_unused]] bool more)
    {
        if (!ec)
        {
            server ().asyncWriteTo (onEchoWrite, _echobuf, size, _echofrom);
        }
    }

    /**
     * @brief wait for the next datagram to echo.
     * @param ec error reported by the socket.
     * @param size number of bytes written.
     */
    static void onEchoWrite (const std::error_code& ec, [[maybe_unused]] size_t size)
    {
        if (!ec)
        {
            server ().asyncReadFrom (onEchoRead, _echobuf, sizeof (_echobuf), _echofrom);
        }
    }

    /**
     * @brief report a completion to the test thread.
     * @param ec error reported by the socket.
     * @param size number of bytes transferred.
     */
    static void onReport (const std::error_code& ec, size_t size)
    {
        ScopedLock<Mutex> lock (_mut);

        _code = ec;
        _transferred = size;
        ++_completions;
        _cond.signal ();
    }

    /**
     * @brief report a read completion to the test thread.
     * @param ec error reported by the socket.
     * @param data buffer holding the data received.
     * @param size number of bytes read.
     * @param more true if the read stays armed.
     */
    static void onReportRead (const std::error_code& ec, [[maybe_unused]] const char* data, size_t size,
                              [[maybe_unused]] bool more)
    {
        onReport (ec, size);
    }

    /**
     * @brief report a read completion and the endpoint it came from to the test thread.
     * @param ec error reported by the socket.
     * @param data buffer holding the data received.
     * @param size number of bytes read.
     * @param from endpoint the datagram was received from.
     * @param control control messages received.
     * @param controlSize size of the control messages.
     * @param more true if the read stays armed.
     */
    static void onReportFrom (const std::error_code& ec, [[maybe_unused]] const char* data, size_t size,
                              const Udp::Endpoint& from, [[maybe_unused]] const char* control,
                              [[maybe_unused]] size_t controlSize, [[maybe_unused]] bool more)
    {
        {
            ScopedLock<Mutex> lock (_mut);
            _from = from;
        }

        onReport (ec, size);
    }

    /**
     * @brief report a multishot completion to the test thread.
     * @param ec error reported by the socket.
     * @param data buffer holding the data received.
     * @param size number of bytes read.
     * @param from endpoint the datagram was received from.
     * @param control control messages received.
     * @param controlSize size of the control messages.
     * @param more true if the read stays armed.
     */
    static void onReportMulti (const std::error_code& ec, const char* data, size_t size, const Udp::Endpoint& from,
                               [[maybe_unused]] const char* control, [[maybe_unused]] size_t controlSize, bool more)
    {
        ScopedLock<Mutex> lock (_mut);

        if (!ec && (size <= sizeof (_buf)))
        {
            ::memcpy (_buf, data, size);
        }

        _code = ec;
        _transferred = size;
        _from = from;
        _more = more;
        ++_completions;
        _cond.signal ();
    }

    /**
     * @brief report a read completion and the time to live it was received with to the test thread.
     * @param ec error reported by the socket.
     * @param data buffer holding the data received.
     * @param size number of bytes read.
     * @param from endpoint the datagram was received from.
     * @param control control messages received.
     * @param controlSize size of the control messages.
     * @param more true if the read stays armed.
     */
    static void onReportControl (const std::error_code& ec, [[maybe_unused]] const char* data, size_t size,
                                 [[maybe_unused]] const Udp::Endpoint& from, const char* control, size_t controlSize,
                                 [[maybe_unused]] bool more)
    {
        {
            ScopedLock<Mutex> lock (_mut);
            if (!ec)
            {
                _ttl = ttlOf (control, controlSize);
            }
        }

        onReport (ec, size);
    }

    /**
     * @brief fill a control buffer asking to send a datagram with the given time to live.
     * @param control control buffer, at least CMSG_SPACE (sizeof (int)) bytes long.
     * @param ttl time to live.
     * @return size of the control messages.
     */
    static size_t ttlControl (char* control, int ttl)
    {
        ::memset (control, 0, CMSG_SPACE (sizeof (int)));

        struct cmsghdr* cmsg = reinterpret_cast<struct cmsghdr*> (control);
        cmsg->cmsg_level = IPPROTO_IP;
        cmsg->cmsg_type = IP_TTL;
        cmsg->cmsg_len = CMSG_LEN (sizeof (int));
        ::memcpy (CMSG_DATA (cmsg), &ttl, sizeof (ttl));

        return CMSG_SPACE (sizeof (int));
    }

    /**
     * @brief get the time to live reported by the control messages received.
     * @param control control messages received.
     * @param controlSize size of the control messages.
     * @return the time to live, -1 if not reported.
     */
    static int ttlOf (const char* control, size_t controlSize)
    {
        struct msghdr msg = {};
        msg.msg_control = const_cast<char*> (control);
        msg.msg_controllen = controlSize;

        for (struct cmsghdr* cmsg = CMSG_FIRSTHDR (&msg); cmsg != nullptr; cmsg = CMSG_NXTHDR (&msg, cmsg))
        {
            if ((cmsg->cmsg_level == IPPROTO_IP) && (cmsg->cmsg_type == IP_TTL))
            {
                int ttl = 0;
                ::memcpy (&ttl, CMSG_DATA (cmsg), sizeof (ttl));
                return ttl;
            }
        }

        return -1;
    }

    /**
     * @brief handler resubmitting a read from within itself.
     * @param ec error reported by the socket.
     * @param data buffer holding the data received.
     * @param size number of bytes read.
     * @param from endpoint the datagram was received from.
     * @param control control messages received.
     * @param controlSize size of the control messages.
     * @param more true if the read stays armed.
     */
    static void onRead (const std::error_code& ec, [[maybe_unused]] const char* data, size_t size,
                        [[maybe_unused]] const Udp::Endpoint& from, [[maybe_unused]] const char* control,
                        [[maybe_unused]] size_t controlSize, [[maybe_unused]] bool more)
    {
        if (!ec && (_rearms > 0))
        {
            --_rearms;
            _current->asyncReadFrom (onRead, _buf, sizeof (_buf), _from);
        }

        onReport (ec, size);
    }

    /**
     * @brief handler resubmitting a write from within itself.
     * @param ec error reported by the socket.
     * @param size number of bytes written.
     */
    static void onWrite (const std::error_code& ec, size_t size)
    {
        if (!ec && (_rearms > 0))
        {
            --_rearms;
            _current->asyncWriteTo (onWrite, "two", 3, _dest);
        }

        onReport (ec, size);
    }

    /**
     * @brief get the socket receiving the move performed from a handler.
     * @return the socket receiving the move performed from a handler.
     */
    static Udp::AsyncSocket& moved ()
    {
        static Udp::AsyncSocket sock;
        return sock;
    }

    /**
     * @brief handler moving the socket from within itself.
     * @param ec error reported by the socket.
     * @param data buffer holding the data received.
     * @param size number of bytes read.
     * @param from endpoint the datagram was received from.
     * @param control control messages received.
     * @param controlSize size of the control messages.
     * @param more true if the read stays armed.
     */
    static void onReadAndMove (const std::error_code& ec, [[maybe_unused]] const char* data, size_t size,
                               [[maybe_unused]] const Udp::Endpoint& from, [[maybe_unused]] const char* control,
                               [[maybe_unused]] size_t controlSize, [[maybe_unused]] bool more)
    {
        moved () = std::move (*_current);

        onReport (ec, size);
    }

    /**
     * @brief handler closing the socket from within itself.
     * @param ec error reported by the socket.
     * @param size number of bytes written.
     */
    static void onWriteAndClose (const std::error_code& ec, size_t size)
    {
        _current->close ();

        onReport (ec, size);
    }

    /**
     * @brief report a wait completion to the test thread.
     * @param ec error reported by the socket.
     * @param more true if the wait stays armed.
     */
    static void onReportWait (const std::error_code& ec, bool more)
    {
        ScopedLock<Mutex> lock (_mut);

        _code = ec;
        _more = more;
        ++_completions;
        _cond.signal ();
    }

    /// condition mutex.
    static Mutex _mut;

    /// condition variable.
    static Condition _cond;

    /// last reported error.
    static std::error_code _code;

    /// number of completions reported.
    static int _completions;

    /// number of bytes reported by the last completion.
    static size_t _transferred;

    /// read buffer.
    static char _buf[1024];

    /// endpoint the last datagram was received from.
    static Udp::Endpoint _from;

    /// last reported multishot state.
    static bool _more;

    /// time to live reported by the last control messages received.
    static int _ttl;

    /// buffer used by the echo server.
    static char _echobuf[1024];

    /// endpoint the echo server received the last datagram from.
    static Udp::Endpoint _echofrom;

    /// socket used by the resubmitting handler.
    static Udp::AsyncSocket* _current;

    /// number of resubmissions left to perform from a handler.
    static int _rearms;

    /// destination used by the resubmitting write handler.
    static Udp::Endpoint _dest;

    /// host.
    static const std::string _host;

    /// port.
    static const uint16_t _port;

    /// timeout.
    static const std::chrono::milliseconds _timeout;
};

Mutex UdpAsyncSocket::_mut;
Condition UdpAsyncSocket::_cond;
std::error_code UdpAsyncSocket::_code;
int UdpAsyncSocket::_completions = 0;
size_t UdpAsyncSocket::_transferred = 0;
char UdpAsyncSocket::_buf[1024] = {};
Udp::Endpoint UdpAsyncSocket::_from;
bool UdpAsyncSocket::_more = false;
int UdpAsyncSocket::_ttl = -1;
char UdpAsyncSocket::_echobuf[1024] = {};
Udp::Endpoint UdpAsyncSocket::_echofrom;
Udp::AsyncSocket* UdpAsyncSocket::_current = nullptr;
int UdpAsyncSocket::_rearms = 0;
Udp::Endpoint UdpAsyncSocket::_dest;
const std::string UdpAsyncSocket::_host = "127.0.0.1";
const uint16_t UdpAsyncSocket::_port = 5036;
const std::chrono::milliseconds UdpAsyncSocket::_timeout{1000};

/**
 * @brief Test move.
 */
TEST_F (UdpAsyncSocket, move)
{
    Udp::AsyncSocket client1, client3;
    Udp::Endpoint dest (_host, _port);

    ASSERT_EQ (client1.open (dest.protocol ()), 0) << join::lastError.message ();
    ASSERT_TRUE (client1.opened ());

    Udp::AsyncSocket client2 (std::move (client1));
    ASSERT_TRUE (client2.opened ());
    ASSERT_FALSE (client1.opened ());

    ASSERT_EQ (client1.asyncReadFrom (nullptr, _buf, sizeof (_buf), _from), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (client1.asyncWriteTo (nullptr, "one", 3, dest), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (client1.cancel (0), 0) << join::lastError.message ();
    ASSERT_EQ (client1.cancel (0), 0) << join::lastError.message ();
    client1.close ();

    ASSERT_NE (client2.asyncReadFrom (onRead, _buf, sizeof (_buf), _from), -1) << join::lastError.message ();

    client3 = std::move (client2);

    ASSERT_TRUE (client3.opened ());
    ASSERT_FALSE (client2.opened ());

    ASSERT_NE (client3.asyncWriteTo (nullptr, "hello", 5, dest), -1) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
        ASSERT_EQ (_transferred, 5u);
        ASSERT_EQ (std::string (_buf, 5), "hello");
    }

    client3.close ();

    Udp::AsyncSocket client4;
    ASSERT_EQ (client4.open (dest.protocol ()), 0) << join::lastError.message ();

    _current = &client4;

    ASSERT_NE (client4.asyncReadFrom (onReadAndMove, _buf, sizeof (_buf), _from), -1) << join::lastError.message ();
    ASSERT_NE (client4.asyncWriteTo (nullptr, "moved", 5, dest), -1) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 2;
        }));
        ASSERT_FALSE (_code) << _code.message ();
        ASSERT_EQ (_transferred, 5u);
        ASSERT_EQ (std::string (_buf, 5), "moved");
    }

    ASSERT_TRUE (moved ().opened ());
    ASSERT_FALSE (client4.opened ());

    moved ().close ();
    _current = nullptr;
}

/**
 * @brief Test open method.
 */
TEST_F (UdpAsyncSocket, open)
{
    Udp::AsyncSocket client;

    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_EQ (client.open (), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    client.close ();
}

/**
 * @brief Test close method.
 */
TEST_F (UdpAsyncSocket, close)
{
    Udp::AsyncSocket client;

    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_TRUE (client.opened ());
    client.close ();
    ASSERT_FALSE (client.opened ());
}

/**
 * @brief Test bind method.
 */
TEST_F (UdpAsyncSocket, bind)
{
    Udp::AsyncSocket client;

    ASSERT_EQ (client.open (Udp::v6 ()), 0) << join::lastError.message ();
    ASSERT_EQ (client.bind (IpAddress (AF_INET6)), 0) << join::lastError.message ();
    client.close ();
}

/**
 * @brief Test bindToDevice method.
 */
TEST_F (UdpAsyncSocket, bindToDevice)
{
    Udp::AsyncSocket client;

    ASSERT_EQ (client.bindToDevice ("lo"), -1);
    ASSERT_EQ (client.open (Udp::v6 ()), 0) << join::lastError.message ();
    ASSERT_EQ (client.bindToDevice ("lo"), 0) << join::lastError.message ();
    ASSERT_EQ (client.bindToDevice ("foo"), -1);
    client.close ();
}

/**
 * @brief Test asyncWait method.
 */
TEST_F (UdpAsyncSocket, asyncWait)
{
    Udp::AsyncSocket client;
    Udp::Endpoint closed (_host, 1);

    ASSERT_EQ (client.asyncWait (nullptr, true, false), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (client.connect ({_host, _port}), 0) << join::lastError.message ();

    ASSERT_EQ (client.asyncWait (nullptr, true, true), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (client.asyncWait (nullptr, false, false), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);

    ASSERT_NE (client.asyncWait (onReportWait, false, true), -1) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
        ASSERT_FALSE (_more);
    }

    ASSERT_NE (client.asyncWait (onReportWait, true, false), -1) << join::lastError.message ();
    ASSERT_NE (client.asyncWrite (nullptr, "hello", 5), -1) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 2;
        }));
        ASSERT_FALSE (_code) << _code.message ();
        ASSERT_FALSE (_more);
    }

    ASSERT_EQ (::recv (client.handle (), _buf, sizeof (_buf), 0), 5) << strerror (errno);
    ASSERT_EQ (std::string (_buf, 5), "hello");

#ifdef JOIN_HAS_IO_URING
    for (size_t i = 0; i < Udp::AsyncSocket::_opCount; ++i)
    {
        ASSERT_NE (client.asyncWait (nullptr, true, false), -1) << join::lastError.message ();
    }

    ASSERT_EQ (client.asyncWait (nullptr, true, false), -1);
    ASSERT_EQ (join::lastError, Errc::OutOfMemory);

    for (size_t i = 0; i < Udp::AsyncSocket::_opCount; ++i)
    {
        ASSERT_EQ (client.cancel (i), 0) << join::lastError.message ();
    }
#endif

    client.close ();
    ASSERT_EQ (client.connect (closed), 0) << join::lastError.message ();
    ASSERT_NE (client.asyncWait (onReportWait, true, false), -1) << join::lastError.message ();
    ASSERT_NE (client.asyncWrite (nullptr, "hello", 5), -1) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _code == std::errc::connection_refused;
        }));
        ASSERT_FALSE (_more);
    }

    client.close ();
}

/**
 * @brief Test asyncWaitMulti method.
 */
TEST_F (UdpAsyncSocket, asyncWaitMulti)
{
    Udp::AsyncSocket client;
    Udp::Endpoint closed (_host, 1);

    ASSERT_EQ (client.asyncWaitMulti (nullptr, true, false), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (client.connect ({_host, _port}), 0) << join::lastError.message ();

    ASSERT_EQ (client.asyncWaitMulti (nullptr, true, true), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (client.asyncWaitMulti (nullptr, false, false), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);

    ssize_t index = client.asyncWaitMulti (onReportWait, true, false);
    ASSERT_NE (index, -1) << join::lastError.message ();

    for (int i = 1; i <= 2; ++i)
    {
        ASSERT_NE (client.asyncWrite (nullptr, "hello", 5), -1) << join::lastError.message ();

        {
            ScopedLock<Mutex> lock (_mut);
            ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&client, i] () {
                return (_completions >= i) && (client.canRead () >= 5);
            }));
            ASSERT_FALSE (_code) << _code.message ();
            ASSERT_TRUE (_more);
        }

        ASSERT_EQ (::recv (client.handle (), _buf, sizeof (_buf), 0), 5) << strerror (errno);
    }

    ASSERT_EQ (client.cancel (static_cast<size_t> (index)), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _code == std::errc::operation_canceled;
        }));
        ASSERT_FALSE (_more);
    }

    int completions = 0;

    {
        ScopedLock<Mutex> lock (_mut);
        _code = {};
        _more = true;
        completions = _completions;
    }

    client.close ();
    ASSERT_EQ (client.connect (closed), 0) << join::lastError.message ();
    ASSERT_NE (client.asyncWaitMulti (onReportWait, true, false), -1) << join::lastError.message ();
    ASSERT_NE (client.asyncWrite (nullptr, "hello", 5), -1) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [completions] () {
            return _completions > completions;
        }));
        ASSERT_EQ (_code, std::errc::connection_refused);
        ASSERT_FALSE (_more);
        ASSERT_FALSE (_cond.timedWait (lock, std::chrono::milliseconds (100), [completions] () {
            return _completions > (completions + 1);
        }));
    }

    client.close ();
}

/**
 * @brief Test connect method.
 */
TEST_F (UdpAsyncSocket, connect)
{
    Udp::AsyncSocket client;

    ASSERT_EQ (client.connect ({"255.255.255.255", _port}), -1);

    ASSERT_EQ (client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (client.connected ());
    ASSERT_EQ (client.connect ({_host, _port}), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    client.close ();
}

/**
 * @brief Test disconnect method.
 */
TEST_F (UdpAsyncSocket, disconnect)
{
    Udp::AsyncSocket client;

    ASSERT_EQ (client.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (client.connected ());
    ASSERT_EQ (client.disconnect (), 0) << join::lastError.message ();
    ASSERT_FALSE (client.connected ());
    client.close ();
}

/**
 * @brief Test asyncWriteTo method.
 */
TEST_F (UdpAsyncSocket, asyncWriteTo)
{
    Proactor proactor;
    Udp::AsyncSocket client (proactor);
    Udp::Endpoint dest (_host, _port);

    ASSERT_FALSE (client.opened ());
    ASSERT_NE (client.asyncWriteTo (onReport, "hello", 5, dest), -1) << join::lastError.message ();
    ASSERT_TRUE (client.opened ());

    Thread th ([&proactor] () {
        proactor.run ();
    });

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
        ASSERT_EQ (_transferred, 5u);
    }

#ifdef JOIN_HAS_IO_URING
    for (size_t i = 0; i < Udp::AsyncSocket::_opCount; ++i)
    {
        ASSERT_NE (client.asyncWriteTo (nullptr, "hello", 5, dest), -1) << join::lastError.message ();
    }

    ASSERT_EQ (client.asyncWriteTo (nullptr, "hello", 5, dest), -1);
    ASSERT_EQ (join::lastError, Errc::OutOfMemory);
#endif

    client.close ();

    proactor.stop ();
    th.join ();
}

/**
 * @brief Test asyncReadFrom method.
 */
TEST_F (UdpAsyncSocket, asyncReadFrom)
{
    Udp::AsyncSocket client;

    ASSERT_EQ (client.asyncReadFrom (nullptr, _buf, sizeof (_buf), _from), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_NE (client.asyncReadFrom (onReportFrom, _buf, sizeof (_buf), _from), -1) << join::lastError.message ();

    ASSERT_NE (client.asyncWrite (nullptr, "hello", 5), -1) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
        ASSERT_EQ (_transferred, 5u);
        ASSERT_EQ (std::string (_buf, 5), "hello");
    }

#ifdef JOIN_HAS_IO_URING
    for (size_t i = 0; i < Udp::AsyncSocket::_opCount; ++i)
    {
        ASSERT_NE (client.asyncReadFrom (nullptr, _buf, sizeof (_buf), _from), -1) << join::lastError.message ();
    }

    ASSERT_EQ (client.asyncReadFrom (nullptr, _buf, sizeof (_buf), _from), -1);
    ASSERT_EQ (join::lastError, Errc::OutOfMemory);
#endif

    client.close ();

    ASSERT_EQ (_from, Udp::Endpoint (_host, _port));
}

/**
 * @brief Test asyncReadFromMulti method.
 */
TEST_F (UdpAsyncSocket, asyncReadFromMulti)
{
    Udp::AsyncSocket client;

    ASSERT_EQ (client.asyncReadFromMulti (nullptr, 0), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (client.connect ({_host, _port}), 0) << join::lastError.message ();

    LocalMem::Allocator<4, sizeof (_buf)> arena;
    ASSERT_EQ (client.registerBufferRing (0, arena), 0) << join::lastError.message ();

    ssize_t index = client.asyncReadFromMulti (onReportMulti, 0);
    ASSERT_NE (index, -1) << join::lastError.message ();

    for (int i = 1; i <= 2; ++i)
    {
        ASSERT_NE (client.asyncWrite (nullptr, "hello", 5), -1) << join::lastError.message ();

        {
            ScopedLock<Mutex> lock (_mut);
            ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [i] () {
                return _completions >= i;
            }));
            ASSERT_FALSE (_code) << _code.message ();
            ASSERT_EQ (_transferred, 5u);
            ASSERT_EQ (std::string (_buf, 5), "hello");
            ASSERT_EQ (_from, Udp::Endpoint (_host, _port));
            ASSERT_TRUE (_more);
        }
    }

    ASSERT_EQ (client.cancel (static_cast<size_t> (index)), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 3;
        }));
        ASSERT_EQ (_code, std::errc::operation_canceled);
        ASSERT_FALSE (_more);
    }

#ifdef JOIN_HAS_IO_URING
    for (size_t i = 0; i < Udp::AsyncSocket::_opCount; ++i)
    {
        ASSERT_NE (client.asyncReadFromMulti (nullptr, 0), -1) << join::lastError.message ();
    }

    ASSERT_EQ (client.asyncReadFromMulti (nullptr, 0), -1);
    ASSERT_EQ (join::lastError, Errc::OutOfMemory);
#endif

    client.close ();

    ASSERT_EQ (client.unregisterBufferRing (0), 0) << join::lastError.message ();
}

/**
 * @brief Test the control messages of the asyncReadFrom, asyncReadFromMulti and asyncWriteTo methods.
 */
TEST_F (UdpAsyncSocket, control)
{
    Udp::AsyncSocket receiver, sender;
    alignas (struct cmsghdr) char out[CMSG_SPACE (sizeof (int))];
    alignas (struct cmsghdr) char in[64];
    int on = 1;

    ASSERT_EQ (receiver.bind ({_host, 0}), 0) << join::lastError.message ();
    ASSERT_EQ (::setsockopt (receiver.handle (), IPPROTO_IP, IP_RECVTTL, &on, sizeof (on)), 0);

    Udp::Endpoint dest = receiver.localEndpoint ();

    ASSERT_NE (receiver.asyncReadFrom (onReportControl, _buf, sizeof (_buf), _from, in, sizeof (in)), -1)
        << join::lastError.message ();
    ASSERT_NE (sender.asyncWriteTo (nullptr, "hello", 5, dest, out, ttlControl (out, 42)), -1)
        << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
        ASSERT_EQ (_transferred, 5u);
        ASSERT_EQ (_ttl, 42);
    }

    LocalMem::Allocator<4, sizeof (_buf)> arena;
    ASSERT_EQ (receiver.registerBufferRing (1, arena), 0) << join::lastError.message ();

    ssize_t index = receiver.asyncReadFromMulti (onReportControl, 1, CMSG_SPACE (sizeof (int)));
    ASSERT_NE (index, -1) << join::lastError.message ();

    ASSERT_NE (sender.asyncWriteTo (nullptr, "hello", 5, dest, out, ttlControl (out, 43)), -1)
        << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 2;
        }));
        ASSERT_FALSE (_code) << _code.message ();
        ASSERT_EQ (_transferred, 5u);
        ASSERT_EQ (_ttl, 43);
    }

    ASSERT_EQ (receiver.cancel (static_cast<size_t> (index)), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 3;
        }));
        ASSERT_EQ (_code, std::errc::operation_canceled);
    }

    sender.close ();
    receiver.close ();

    ASSERT_EQ (receiver.unregisterBufferRing (1), 0) << join::lastError.message ();
}

/**
 * @brief Test asyncWrite method.
 */
TEST_F (UdpAsyncSocket, asyncWrite)
{
    Udp::AsyncSocket client;

    ASSERT_EQ (client.asyncWrite (nullptr, "hello", 5), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_NE (client.asyncWrite (onReport, "hello", 5), -1) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
        ASSERT_EQ (_transferred, 5u);
    }

    ASSERT_EQ (::close (client.handle ()), 0);

    ASSERT_NE (client.asyncWrite (onReport, "hello", 5), -1) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 2;
        }));
        ASSERT_EQ (_code, std::errc::bad_file_descriptor) << _code.message ();
    }

    client.close ();
}

#ifdef JOIN_HAS_IO_URING
/**
 * @brief Test asyncWriteFixed method.
 */
TEST_F (UdpAsyncSocket, asyncWriteFixed)
{
    Udp::AsyncSocket client;
    LocalMem::Allocator<1, 1024> arena;

    char* buf = static_cast<char*> (arena.allocate (sizeof (_buf)));
    ASSERT_NE (buf, nullptr);

    ASSERT_EQ (client.asyncWriteFixed (nullptr, buf, 5, 0), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (client.registerFixedBuffers (arena), 0) << join::lastError.message ();

    ASSERT_EQ (client.connect ({_host, _port}), 0) << join::lastError.message ();

    ::memcpy (buf, "hello", 5);

    ASSERT_NE (client.asyncWriteFixed (onReport, buf, 5, 0), -1) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
        ASSERT_EQ (_transferred, 5u);
    }

    client.close ();

    ASSERT_EQ (client.unregisterFixedBuffers (), 0) << join::lastError.message ();
}
#endif

/**
 * @brief Test asyncRead method.
 */
TEST_F (UdpAsyncSocket, asyncRead)
{
    Udp::AsyncSocket client;

    ASSERT_EQ (client.asyncRead (nullptr, _buf, sizeof (_buf)), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_NE (client.asyncRead (onReportRead, _buf, sizeof (_buf)), -1) << join::lastError.message ();
    ASSERT_NE (client.asyncWrite (nullptr, "hello", 5), -1) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
        ASSERT_EQ (_transferred, 5u);
        ASSERT_EQ (std::string (_buf, 5), "hello");
    }


    ASSERT_EQ (::close (client.handle ()), 0);

    ASSERT_NE (client.asyncRead (onReportRead, _buf, sizeof (_buf)), -1) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 2;
        }));
        ASSERT_EQ (_code, std::errc::bad_file_descriptor) << _code.message ();
    }

    client.close ();
}

#ifdef JOIN_HAS_IO_URING
/**
 * @brief Test asyncReadFixed method.
 */
TEST_F (UdpAsyncSocket, asyncReadFixed)
{
    Udp::AsyncSocket client;
    LocalMem::Allocator<1, 1024> arena;

    char* buf = static_cast<char*> (arena.allocate (sizeof (_buf)));
    ASSERT_NE (buf, nullptr);

    ASSERT_EQ (client.asyncReadFixed (nullptr, buf, sizeof (_buf), 0), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (client.registerFixedBuffers (arena), 0) << join::lastError.message ();

    ASSERT_EQ (client.connect ({_host, _port}), 0) << join::lastError.message ();

    ASSERT_NE (client.asyncReadFixed (onReportRead, buf, sizeof (_buf), 0), -1) << join::lastError.message ();
    ASSERT_NE (client.asyncWrite (nullptr, "hello", 5), -1) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
        ASSERT_EQ (_transferred, 5u);
        ASSERT_EQ (std::string (buf, 5), "hello");
    }

    client.close ();

    ASSERT_EQ (client.unregisterFixedBuffers (), 0) << join::lastError.message ();
}
#endif

/**
 * @brief Test asyncReadFrom method resubmitted from its own handler.
 */
TEST_F (UdpAsyncSocket, resubmit)
{
    Udp::AsyncSocket client;
    Udp::Endpoint dest (_host, _port);

    _current = &client;
    _rearms = 1;

    ASSERT_EQ (client.bind (Udp::Endpoint (_host, 0)), 0) << join::lastError.message ();
    ASSERT_NE (client.asyncReadFrom (onRead, _buf, sizeof (_buf), _from), -1) << join::lastError.message ();
    ASSERT_NE (client.asyncWriteTo (nullptr, "one", 3, dest), -1) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
    }

    ASSERT_NE (client.asyncWriteTo (nullptr, "two", 3, dest), -1) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 2;
        }));
        ASSERT_FALSE (_code) << _code.message ();
    }

    _dest = Udp::Endpoint (_host, _port);
    _rearms = 1;

    ASSERT_NE (client.asyncWriteTo (onWrite, "one", 3, _dest), -1) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 4;
        }));
        ASSERT_FALSE (_code) << _code.message ();
    }

    client.close ();
    _current = nullptr;
}

/**
 * @brief Test close called from within a write handler.
 */
TEST_F (UdpAsyncSocket, closeFromWriteHandler)
{
    Udp::AsyncSocket client;
    Udp::Endpoint dest (_host, _port);

    _current = &client;

    ASSERT_NE (client.asyncWriteTo (onWriteAndClose, "hello", 5, dest), -1) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
    }

    ASSERT_FALSE (client.opened ());
    _current = nullptr;
}

/**
 * @brief Test a datagram larger than the supplied buffer.
 */
TEST_F (UdpAsyncSocket, truncated)
{
    Udp::AsyncSocket client;
    char small[4] = {};

    ASSERT_EQ (client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_NE (client.asyncReadFrom (onReportFrom, small, sizeof (small), _from), -1) << join::lastError.message ();
    ASSERT_NE (client.asyncWrite (nullptr, "hello world", 11), -1) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_EQ (_code, Errc::MessageTooLong);
    }

    client.close ();
}

/**
 * @brief Test an empty datagram.
 */
TEST_F (UdpAsyncSocket, empty)
{
    Udp::AsyncSocket client;
    Udp::Socket sender;
    Udp::Endpoint self (_host, uint16_t (_port + 2));

    ASSERT_EQ (client.bind (self), 0) << join::lastError.message ();
    ASSERT_NE (client.asyncReadFrom (onReportFrom, _buf, sizeof (_buf), _from), -1) << join::lastError.message ();

    ASSERT_EQ (sender.writeTo ("", 0, self), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
        ASSERT_EQ (_transferred, 0u);
    }

    sender.close ();
    client.close ();
}

/**
 * @brief Test cancel method.
 */
TEST_F (UdpAsyncSocket, cancel)
{
    {
        Udp::AsyncSocket client;

        ASSERT_EQ (client.cancel (0), 0) << join::lastError.message ();
        ASSERT_EQ (client.cancel (Udp::AsyncSocket::_opCount), -1);
        ASSERT_EQ (join::lastError, Errc::InvalidParam);

        ASSERT_EQ (client.bind (Udp::Endpoint (_host, 0)), 0) << join::lastError.message ();

        ssize_t index = client.asyncWait (onReportWait, true, false);
        ASSERT_NE (index, -1) << join::lastError.message ();

        ASSERT_EQ (client.cancel (static_cast<size_t> (index)), 0) << join::lastError.message ();

        {
            ScopedLock<Mutex> lock (_mut);
            ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
                return _completions >= 1;
            }));
            ASSERT_EQ (_code, std::errc::operation_canceled);
            ASSERT_FALSE (_more);
        }

        client.close ();
    }

    {
        ScopedLock<Mutex> lock (_mut);
        _code = {};
        _completions = 0;
        _more = false;
        _transferred = 0;
    }

    {
        Udp::AsyncSocket client;

        ASSERT_EQ (client.cancel (0), 0) << join::lastError.message ();
        ASSERT_EQ (client.bind (Udp::Endpoint (_host, 0)), 0) << join::lastError.message ();
        ASSERT_NE (client.asyncReadFrom (onReportFrom, _buf, sizeof (_buf), _from), -1) << join::lastError.message ();
        ASSERT_EQ (client.cancel (0), 0) << join::lastError.message ();

        {
            ScopedLock<Mutex> lock (_mut);
            ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
                return _completions >= 1;
            }));
            ASSERT_EQ (_code, std::errc::operation_canceled);
        }

        client.close ();
    }

    {
        ScopedLock<Mutex> lock (_mut);
        _code = {};
        _completions = 0;
        _more = false;
        _transferred = 0;
    }

    {
        Udp::AsyncSocket client;

        ASSERT_EQ (client.cancel (0), 0) << join::lastError.message ();
        ASSERT_EQ (client.open (), 0) << join::lastError.message ();
        ASSERT_EQ (client.cancel (0), 0) << join::lastError.message ();
        client.close ();
    }
}

/**
 * @brief Test setOption method.
 */
TEST_F (UdpAsyncSocket, setOption)
{
    Udp::AsyncSocket client;

    ASSERT_EQ (client.setOption (Udp::Socket::RcvBuffer, 4096), -1);
    ASSERT_EQ (client.open (Udp::v6 ()), 0) << join::lastError.message ();
    ASSERT_EQ (client.setOption (Udp::Socket::RcvBuffer, 4096), 0) << join::lastError.message ();
    client.close ();
}

/**
 * @brief Test localEndpoint method.
 */
TEST_F (UdpAsyncSocket, localEndpoint)
{
    Udp::AsyncSocket client;

    ASSERT_EQ (client.open (Udp::v6 ()), 0) << join::lastError.message ();
    ASSERT_EQ (client.bind ({IpAddress::ipv6Wildcard, uint16_t (_port + 1)}), 0) << join::lastError.message ();
    ASSERT_EQ (client.localEndpoint ().port (), uint16_t (_port + 1));
    client.close ();
}

/**
 * @brief Test remoteEndpoint method.
 */
TEST_F (UdpAsyncSocket, remoteEndpoint)
{
    Udp::AsyncSocket client;

    ASSERT_EQ (client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (client.remoteEndpoint ().ip (), _host);
    ASSERT_EQ (client.remoteEndpoint ().port (), _port);
    client.close ();
}

/**
 * @brief Test opened method.
 */
TEST_F (UdpAsyncSocket, opened)
{
    Udp::AsyncSocket client;

    ASSERT_FALSE (client.opened ());
    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_TRUE (client.opened ());
    client.close ();
    ASSERT_FALSE (client.opened ());
}

/**
 * @brief Test connected method.
 */
TEST_F (UdpAsyncSocket, connected)
{
    Udp::AsyncSocket client;

    ASSERT_FALSE (client.connected ());
    ASSERT_EQ (client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (client.connected ());
    client.close ();
    ASSERT_FALSE (client.connected ());
}

/**
 * @brief Test canRead method.
 */
TEST_F (UdpAsyncSocket, canRead)
{
    Udp::AsyncSocket client;

    ASSERT_EQ (client.canRead (), -1);
    ASSERT_EQ (client.open (Udp::v6 ()), 0) << join::lastError.message ();
    ASSERT_EQ (client.canRead (), 0) << join::lastError.message ();
    client.close ();
}

/**
 * @brief Test mtu method.
 */
TEST_F (UdpAsyncSocket, mtu)
{
    Udp::AsyncSocket client;

    ASSERT_EQ (client.mtu (), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_GT (client.mtu (), 0) << join::lastError.message ();
    client.close ();
}

/**
 * @brief Test ttl method.
 */
TEST_F (UdpAsyncSocket, ttl)
{
    Udp::AsyncSocket client;

    ASSERT_EQ (client.ttl (), 60);

    Udp::AsyncSocket other (32);

    ASSERT_EQ (other.ttl (), 32);
}

/**
 * @brief Test family method.
 */
TEST_F (UdpAsyncSocket, family)
{
    Udp::AsyncSocket client;

    ASSERT_EQ (client.open (Udp::v6 ()), 0) << join::lastError.message ();
    ASSERT_EQ (client.family (), AF_INET6);
    client.close ();
}

/**
 * @brief Test type method.
 */
TEST_F (UdpAsyncSocket, type)
{
    Udp::AsyncSocket client;

    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_EQ (client.type (), SOCK_DGRAM);
    client.close ();
}

/**
 * @brief Test protocol method.
 */
TEST_F (UdpAsyncSocket, protocol)
{
    Udp::AsyncSocket client;

    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_EQ (client.protocol (), IPPROTO_UDP);
    client.close ();
}

/**
 * @brief Test handle method.
 */
TEST_F (UdpAsyncSocket, handle)
{
    Udp::AsyncSocket client;

    ASSERT_EQ (client.handle (), -1);
    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_GT (client.handle (), -1);
    client.close ();
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
