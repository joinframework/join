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
using join::Icmp;
using join::Thread;
using join::Proactor;
using join::LocalMem;

/**
 * @brief Class used to test the icmp asynchronous datagram socket API.
 */
class IcmpAsyncSocket : public ::testing::Test
{
public:
    /**
     * @brief set up test case.
     */
    static void SetUpTestCase ()
    {
        struct icmphdr* icmp = reinterpret_cast<struct icmphdr*> (_data);

        icmp->type = ICMP_ECHO;
        icmp->code = 0;
        icmp->checksum = 0;
        icmp->un.echo.sequence = htons (1);
        icmp->un.echo.id = htons (getpid () & 0xFFFF);
        icmp->checksum = join::checksum (reinterpret_cast<uint16_t*> (icmp), sizeof (struct icmphdr), 0);
    }

protected:
    /**
     * @brief Sets up the test fixture.
     */
    void SetUp () override
    {
        ScopedLock<Mutex> lock (_mut);

        _code = {};
        _completions = 0;
        _more = false;
        _transferred = 0;
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
     * @param more true if the read stays armed.
     */
    static void onReportFrom (const std::error_code& ec, [[maybe_unused]] const char* data, size_t size,
                              const Icmp::Endpoint& from, [[maybe_unused]] bool more)
    {
        {
            ScopedLock<Mutex> lock (_mut);
            _from = from;
        }

        onReport (ec, size);
    }

    /**
     * @brief handler resubmitting a read from within itself.
     */
    static void onRead (const std::error_code& ec, [[maybe_unused]] const char* data, size_t size,
                        [[maybe_unused]] bool more)
    {
        if (!ec && (_rearms > 0))
        {
            --_rearms;
            _current->asyncRead (_buf, sizeof (_buf), onRead);
        }

        onReport (ec, size);
    }

    /**
     * @brief handler resubmitting a write from within itself.
     */
    static void onWrite (const std::error_code& ec, size_t size)
    {
        if (!ec && (_rearms > 0))
        {
            --_rearms;
            _current->asyncWrite (_data, sizeof (_data), onWrite);
        }

        onReport (ec, size);
    }

    /**
     * @brief handler closing the socket from within itself.
     */
    static void onWriteAndClose (const std::error_code& ec, size_t size)
    {
        _current->close ();
        onReport (ec, size);
    }

    /**
     * @brief get the error reported by the last completion.
     * @return copy of the reported error.
     */
    static std::error_code code ()
    {
        ScopedLock<Mutex> lock (_mut);

        return _code;
    }

    /**
     * @brief wait for the expected number of completions.
     * @param expected number of completions to wait for.
     * @return true on success, false on timeout.
     */
    static bool wait (int expected)
    {
        ScopedLock<Mutex> lock (_mut);

        return _cond.timedWait (lock, std::chrono::milliseconds (_timeout), [expected] () {
            return _completions >= expected;
        });
    }

    /**
     * @brief report a multishot completion to the test thread.
     * @param ec error reported by the socket.
     * @param data buffer holding the data received.
     * @param size number of bytes read.
     * @param from endpoint the datagram was received from.
     * @param more true if the read stays armed.
     */
    static void onReportMulti (const std::error_code& ec, const char* data, size_t size, const Icmp::Endpoint& from,
                               bool more)
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
    static Icmp::Endpoint _from;

    /// last reported multishot state.
    static bool _more;

    /// echo request sent by the tests.
    static char _data[sizeof (struct icmphdr)];

    /// host.
    static const std::string _host;

    /// socket used by the resubmitting handler.
    static Icmp::AsyncSocket* _current;

    /// number of resubmissions left to perform from a handler.
    static int _rearms;

    /// timeout.
    static const std::chrono::milliseconds _timeout;
};

Mutex IcmpAsyncSocket::_mut;
Condition IcmpAsyncSocket::_cond;
std::error_code IcmpAsyncSocket::_code;
int IcmpAsyncSocket::_completions = 0;
size_t IcmpAsyncSocket::_transferred = 0;
char IcmpAsyncSocket::_buf[1024] = {};
Icmp::Endpoint IcmpAsyncSocket::_from;
bool IcmpAsyncSocket::_more = false;
char IcmpAsyncSocket::_data[sizeof (struct icmphdr)] = {};
const std::string IcmpAsyncSocket::_host = "127.0.0.1";
Icmp::AsyncSocket* IcmpAsyncSocket::_current = nullptr;
int IcmpAsyncSocket::_rearms = 0;
const std::chrono::milliseconds IcmpAsyncSocket::_timeout{1000};

/**
 * @brief Test move.
 */
TEST_F (IcmpAsyncSocket, move)
{
    Icmp::AsyncSocket client1, client3;

    ASSERT_EQ (client1.connect (_host), 0) << join::lastError.message ();
    ASSERT_TRUE (client1.opened ());

    Icmp::AsyncSocket client2 (std::move (client1));
    ASSERT_TRUE (client2.opened ());
    ASSERT_FALSE (client1.opened ());

    ASSERT_EQ (client1.asyncRead (_buf, sizeof (_buf), nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (client1.asyncWrite (_data, sizeof (_data), nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (client1.cancel (0), 0) << join::lastError.message ();
    ASSERT_EQ (client1.cancel (0), 0) << join::lastError.message ();
    client1.close ();

    ASSERT_NE (client2.asyncRead (_buf, sizeof (_buf), onRead), -1) << join::lastError.message ();

    client3 = std::move (client2);

    ASSERT_TRUE (client3.opened ());
    ASSERT_FALSE (client2.opened ());

    ASSERT_NE (client3.asyncWrite (_data, sizeof (_data), nullptr), -1) << join::lastError.message ();

    ASSERT_TRUE (wait (1));
    ASSERT_FALSE (code ()) << code ().message ();
    ASSERT_GT (_transferred, 0u);

    client3.close ();
}

/**
 * @brief Test open method.
 */
TEST_F (IcmpAsyncSocket, open)
{
    Icmp::AsyncSocket client;

    ASSERT_EQ (client.open (Icmp::v4 ()), 0) << join::lastError.message ();
    ASSERT_EQ (client.open (Icmp::v4 ()), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    client.close ();

    ASSERT_EQ (client.open (Icmp::v6 ()), 0) << join::lastError.message ();
    ASSERT_EQ (client.open (Icmp::v6 ()), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    client.close ();
}

/**
 * @brief Test close method.
 */
TEST_F (IcmpAsyncSocket, close)
{
    Icmp::AsyncSocket client;

    ASSERT_EQ (client.open (Icmp::v4 ()), 0) << join::lastError.message ();
    ASSERT_TRUE (client.opened ());
    client.close ();
    ASSERT_FALSE (client.opened ());
}

/**
 * @brief Test bind method.
 */
TEST_F (IcmpAsyncSocket, bind)
{
    Icmp::AsyncSocket client;

    ASSERT_EQ (client.connect (_host), 0) << join::lastError.message ();
    ASSERT_EQ (client.bind (_host), -1);
    ASSERT_EQ (client.disconnect (), 0) << join::lastError.message ();

    ASSERT_EQ (client.bind (_host), 0) << join::lastError.message ();
    client.close ();
}

/**
 * @brief Test bindToDevice method.
 */
TEST_F (IcmpAsyncSocket, bindToDevice)
{
    Icmp::AsyncSocket client;

    ASSERT_EQ (client.bindToDevice ("lo"), -1);
    ASSERT_EQ (client.open (Icmp::v4 ()), 0) << join::lastError.message ();
    ASSERT_EQ (client.bindToDevice ("lo"), 0) << join::lastError.message ();
    ASSERT_EQ (client.bindToDevice ("foo"), -1);
    client.close ();
}

/**
 * @brief Test asyncWait method.
 */
TEST_F (IcmpAsyncSocket, asyncWait)
{
    Icmp::AsyncSocket client;

    ASSERT_EQ (client.asyncWait (true, false, nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (client.connect (_host), 0) << join::lastError.message ();

    ASSERT_EQ (client.asyncWait (true, true, nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (client.asyncWait (false, false, nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);

    ASSERT_NE (client.asyncWait (false, true, onReportWait), -1) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
        ASSERT_FALSE (_more);
    }

    ASSERT_NE (client.asyncWait (true, false, onReportWait), -1) << join::lastError.message ();
    ASSERT_NE (client.asyncWrite (_data, sizeof (_data), nullptr), -1) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 2;
        }));
        ASSERT_FALSE (_code) << _code.message ();
        ASSERT_FALSE (_more);
    }

    while (::recv (client.handle (), _buf, sizeof (_buf), MSG_DONTWAIT) > 0)
    {
    }

#ifdef JOIN_HAS_IO_URING
    for (size_t i = 0; i < Icmp::AsyncSocket::_opCount; ++i)
    {
        ASSERT_NE (client.asyncWait (true, false, nullptr), -1) << join::lastError.message ();
    }

    ASSERT_EQ (client.asyncWait (true, false, nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::OutOfMemory);
#endif

    client.close ();
}

/**
 * @brief Test asyncWaitMulti method.
 */
TEST_F (IcmpAsyncSocket, asyncWaitMulti)
{
    Icmp::AsyncSocket client;

    ASSERT_EQ (client.asyncWaitMulti (true, false, nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (client.connect (_host), 0) << join::lastError.message ();

    ASSERT_EQ (client.asyncWaitMulti (true, true, nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (client.asyncWaitMulti (false, false, nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);

    ssize_t index = client.asyncWaitMulti (true, false, onReportWait);
    ASSERT_NE (index, -1) << join::lastError.message ();

    for (int i = 1; i <= 2; ++i)
    {
        ASSERT_NE (client.asyncWrite (_data, sizeof (_data), nullptr), -1) << join::lastError.message ();

        {
            ScopedLock<Mutex> lock (_mut);
            ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&client, i] () {
                return (_completions >= i) && (client.canRead () >= 1);
            }));
            ASSERT_FALSE (_code) << _code.message ();
            ASSERT_TRUE (_more);
        }

        while (::recv (client.handle (), _buf, sizeof (_buf), MSG_DONTWAIT) > 0)
        {
        }
    }

    ASSERT_EQ (client.cancel (static_cast<size_t> (index)), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _code == std::errc::operation_canceled;
        }));
        ASSERT_FALSE (_more);
    }

    client.close ();
}

/**
 * @brief Test connect method.
 */
TEST_F (IcmpAsyncSocket, connect)
{
    Icmp::AsyncSocket client;

    ASSERT_EQ (client.connect ("255.255.255.255"), -1);

    ASSERT_EQ (client.connect (_host), 0) << join::lastError.message ();
    ASSERT_TRUE (client.connected ());
    ASSERT_EQ (client.connect (_host), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    client.close ();
}

/**
 * @brief Test disconnect method.
 */
TEST_F (IcmpAsyncSocket, disconnect)
{
    Icmp::AsyncSocket client;

    ASSERT_EQ (client.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (client.connect (_host), 0) << join::lastError.message ();
    ASSERT_TRUE (client.connected ());
    ASSERT_EQ (client.disconnect (), 0) << join::lastError.message ();
    ASSERT_FALSE (client.connected ());
    client.close ();
}

/**
 * @brief Test asyncWriteTo method.
 */
TEST_F (IcmpAsyncSocket, asyncWriteTo)
{
    Proactor proactor;
    Icmp::AsyncSocket client (proactor);
    Icmp::Endpoint dest (_host);

    ASSERT_FALSE (client.opened ());
    ASSERT_NE (client.asyncWriteTo (_data, sizeof (_data), dest, onReport), -1) << join::lastError.message ();
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
        ASSERT_EQ (_transferred, sizeof (_data));
    }

    client.close ();

    proactor.stop ();
    th.join ();
}

/**
 * @brief Test asyncReadFrom method.
 */
TEST_F (IcmpAsyncSocket, asyncReadFrom)
{
    Icmp::AsyncSocket client, server;

    ASSERT_EQ (server.asyncReadFrom (_buf, sizeof (_buf), _from, nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (server.bind (_host), 0) << join::lastError.message ();
    ASSERT_NE (server.asyncReadFrom (_buf, sizeof (_buf), _from, onReportFrom), -1) << join::lastError.message ();

    ASSERT_EQ (client.connect (_host), 0) << join::lastError.message ();
    ASSERT_NE (client.asyncWrite (_data, sizeof (_data), nullptr), -1) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
        ASSERT_GT (_transferred, 0u);
    }

    client.close ();
    server.close ();

    ASSERT_EQ (_from, Icmp::Endpoint (_host));
}

/**
 * @brief Test asyncReadFromMulti method.
 */
TEST_F (IcmpAsyncSocket, asyncReadFromMulti)
{
    Icmp::AsyncSocket client, server;

    ASSERT_EQ (server.asyncReadFromMulti (0, nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (server.bind (_host), 0) << join::lastError.message ();

    LocalMem::Allocator<4, sizeof (_buf)> arena;
    ASSERT_EQ (server.registerBufferRing (0, arena), 0) << join::lastError.message ();

    ssize_t index = server.asyncReadFromMulti (0, onReportMulti);
    ASSERT_NE (index, -1) << join::lastError.message ();

    ASSERT_EQ (client.connect (_host), 0) << join::lastError.message ();

    for (int i = 1; i <= 2; ++i)
    {
        ASSERT_NE (client.asyncWrite (_data, sizeof (_data), nullptr), -1) << join::lastError.message ();

        {
            ScopedLock<Mutex> lock (_mut);
            ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [i] () {
                return _completions >= i;
            }));
            ASSERT_FALSE (_code) << _code.message ();
            ASSERT_GT (_transferred, 0u);
            ASSERT_EQ (_from, Icmp::Endpoint (_host));
            ASSERT_TRUE (_more);
        }
    }

    ASSERT_EQ (server.cancel (static_cast<size_t> (index)), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _code == std::errc::operation_canceled;
        }));
        ASSERT_FALSE (_more);
    }

#ifdef JOIN_HAS_IO_URING
    for (size_t i = 0; i < Icmp::AsyncSocket::_opCount; ++i)
    {
        ASSERT_NE (server.asyncReadFromMulti (0, nullptr), -1) << join::lastError.message ();
    }

    ASSERT_EQ (server.asyncReadFromMulti (0, nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::OutOfMemory);
#endif

    client.close ();
    server.close ();

    ASSERT_EQ (server.unregisterBufferRing (0), 0) << join::lastError.message ();
}

/**
 * @brief Test asyncWrite method.
 */
TEST_F (IcmpAsyncSocket, asyncWrite)
{
    Icmp::AsyncSocket client;

    ASSERT_EQ (client.asyncWrite (_data, sizeof (_data), nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (client.connect (_host), 0) << join::lastError.message ();
    ASSERT_NE (client.asyncWrite (_data, sizeof (_data), onReport), -1) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
        ASSERT_EQ (_transferred, sizeof (_data));
    }

    ASSERT_EQ (::close (client.handle ()), 0);

    ASSERT_NE (client.asyncWrite ("hello", 5, onReport), -1) << join::lastError.message ();

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
TEST_F (IcmpAsyncSocket, asyncWriteFixed)
{
    Icmp::AsyncSocket client;
    LocalMem::Allocator<1, 1024> arena;

    char* buf = static_cast<char*> (arena.allocate (sizeof (_buf)));
    ASSERT_NE (buf, nullptr);

    ASSERT_EQ (client.asyncWriteFixed (buf, sizeof (_data), 0, nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (client.registerFixedBuffers (arena), 0) << join::lastError.message ();

    ASSERT_EQ (client.connect (_host), 0) << join::lastError.message ();

    ::memcpy (buf, _data, sizeof (_data));

    ASSERT_NE (client.asyncWriteFixed (buf, sizeof (_data), 0, onReport), -1) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
        ASSERT_EQ (_transferred, sizeof (_data));
    }

    client.close ();

    ASSERT_EQ (client.unregisterFixedBuffers (), 0) << join::lastError.message ();
}
#endif

/**
 * @brief Test asyncRead method.
 */
TEST_F (IcmpAsyncSocket, asyncRead)
{
    Icmp::AsyncSocket client;

    ASSERT_EQ (client.asyncRead (_buf, sizeof (_buf), nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (client.connect (_host), 0) << join::lastError.message ();
    ASSERT_NE (client.asyncRead (_buf, sizeof (_buf), onReportRead), -1) << join::lastError.message ();
    ASSERT_NE (client.asyncWrite (_data, sizeof (_data), nullptr), -1) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
        ASSERT_GT (_transferred, 0u);
    }

    ASSERT_EQ (::close (client.handle ()), 0);

    ASSERT_NE (client.asyncRead (_buf, sizeof (_buf), onReportRead), -1) << join::lastError.message ();

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
TEST_F (IcmpAsyncSocket, asyncReadFixed)
{
    Icmp::AsyncSocket client;
    LocalMem::Allocator<1, 1024> arena;

    char* buf = static_cast<char*> (arena.allocate (sizeof (_buf)));
    ASSERT_NE (buf, nullptr);

    ASSERT_EQ (client.asyncReadFixed (buf, sizeof (_buf), 0, nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (client.registerFixedBuffers (arena), 0) << join::lastError.message ();

    ASSERT_EQ (client.connect (_host), 0) << join::lastError.message ();

    ASSERT_NE (client.asyncReadFixed (buf, sizeof (_buf), 0, onReportRead), -1) << join::lastError.message ();
    ASSERT_NE (client.asyncWrite (_data, sizeof (_data), nullptr), -1) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
        ASSERT_GT (_transferred, 0u);
    }

    client.close ();

    ASSERT_EQ (client.unregisterFixedBuffers (), 0) << join::lastError.message ();
}
#endif

/**
 * @brief Test async operations resubmitted from their own handlers.
 */
TEST_F (IcmpAsyncSocket, resubmit)
{
    Icmp::AsyncSocket client;

    _current = &client;
    _rearms = 1;

    ASSERT_EQ (client.connect (_host), 0) << join::lastError.message ();

    ASSERT_NE (client.asyncRead (_buf, sizeof (_buf), onRead), -1) << join::lastError.message ();

    ASSERT_NE (client.asyncWrite (_data, sizeof (_data), nullptr), -1) << join::lastError.message ();

    ASSERT_TRUE (wait (1));
    ASSERT_FALSE (code ()) << code ().message ();

    ASSERT_TRUE (wait (2));
    ASSERT_FALSE (code ()) << code ().message ();

    client.close ();
    _current = nullptr;
}

/**
 * @brief Test close called from within a write handler.
 */
TEST_F (IcmpAsyncSocket, closeFromWriteHandler)
{
    Icmp::AsyncSocket client;

    _current = &client;

    ASSERT_EQ (client.connect (_host), 0) << join::lastError.message ();

    ASSERT_NE (client.asyncWrite (_data, sizeof (_data), onWriteAndClose), -1) << join::lastError.message ();

    ASSERT_TRUE (wait (1));
    ASSERT_FALSE (code ()) << code ().message ();
    ASSERT_FALSE (client.opened ());

    _current = nullptr;
}

/**
 * @brief Test a datagram larger than the supplied buffer.
 */
TEST_F (IcmpAsyncSocket, truncated)
{
    Icmp::AsyncSocket client;
    char small[sizeof (struct icmphdr) / 2] = {};

    ASSERT_EQ (client.connect (_host), 0) << join::lastError.message ();
    ASSERT_NE (client.asyncRead (small, sizeof (small), onReportRead), -1) << join::lastError.message ();
    ASSERT_NE (client.asyncWrite (_data, sizeof (_data), nullptr), -1) << join::lastError.message ();

    ASSERT_TRUE (wait (1));
    ASSERT_EQ (code (), Errc::MessageTooLong) << code ().message ();

    client.close ();
}

/**
 * @brief Test cancel method.
 */
TEST_F (IcmpAsyncSocket, cancel)
{
    {
        Icmp::AsyncSocket client;

        ASSERT_EQ (client.cancel (0), 0) << join::lastError.message ();
        ASSERT_EQ (client.cancel (Icmp::AsyncSocket::_opCount), -1);
        ASSERT_EQ (join::lastError, Errc::InvalidParam);

        ASSERT_EQ (client.connect (_host), 0) << join::lastError.message ();

        ssize_t index = client.asyncWait (true, false, onReportWait);
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
        Icmp::AsyncSocket client;

        ASSERT_EQ (client.cancel (0), 0) << join::lastError.message ();
        ASSERT_EQ (client.bind (_host), 0) << join::lastError.message ();
        ASSERT_NE (client.asyncReadFrom (_buf, sizeof (_buf), _from, onReportFrom), -1) << join::lastError.message ();
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
        Icmp::AsyncSocket client;

        ASSERT_EQ (client.cancel (0), 0) << join::lastError.message ();
        ASSERT_EQ (client.open (Icmp::v4 ()), 0) << join::lastError.message ();
        ASSERT_EQ (client.cancel (0), 0) << join::lastError.message ();
        client.close ();
    }
}

/**
 * @brief Test setOption method.
 */
TEST_F (IcmpAsyncSocket, setOption)
{
    Icmp::AsyncSocket client;

    ASSERT_EQ (client.setOption (Icmp::Socket::RcvBuffer, 1500), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (client.open (Icmp::v4 ()), 0) << join::lastError.message ();
    ASSERT_EQ (client.setOption (Icmp::Socket::RcvBuffer, 1500), 0) << join::lastError.message ();
    client.close ();
}

/**
 * @brief Test localEndpoint method.
 */
TEST_F (IcmpAsyncSocket, localEndpoint)
{
    Icmp::AsyncSocket client;

    ASSERT_EQ (client.localEndpoint (), Icmp::Endpoint{});
    ASSERT_EQ (client.bind (_host), 0) << join::lastError.message ();
    ASSERT_EQ (client.localEndpoint ().ip (), IpAddress (_host));
    client.close ();
}

/**
 * @brief Test remoteEndpoint method.
 */
TEST_F (IcmpAsyncSocket, remoteEndpoint)
{
    Icmp::AsyncSocket client;

    ASSERT_EQ (client.connect (_host), 0) << join::lastError.message ();
    ASSERT_EQ (client.remoteEndpoint (), Icmp::Endpoint (_host));
    client.close ();
}

/**
 * @brief Test opened method.
 */
TEST_F (IcmpAsyncSocket, opened)
{
    Icmp::AsyncSocket client;

    ASSERT_FALSE (client.opened ());
    ASSERT_EQ (client.open (Icmp::v4 ()), 0) << join::lastError.message ();
    ASSERT_TRUE (client.opened ());
    client.close ();
    ASSERT_FALSE (client.opened ());
}

/**
 * @brief Test connected method.
 */
TEST_F (IcmpAsyncSocket, connected)
{
    Icmp::AsyncSocket client;

    ASSERT_FALSE (client.connected ());
    ASSERT_EQ (client.connect (_host), 0) << join::lastError.message ();
    ASSERT_TRUE (client.connected ());
    client.close ();
    ASSERT_FALSE (client.connected ());
}

/**
 * @brief Test canRead method.
 */
TEST_F (IcmpAsyncSocket, canRead)
{
    Icmp::AsyncSocket client;

    ASSERT_EQ (client.canRead (), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (client.open (Icmp::v4 ()), 0) << join::lastError.message ();
    ASSERT_EQ (client.canRead (), 0) << join::lastError.message ();
    client.close ();
}

/**
 * @brief Test mtu method.
 */
TEST_F (IcmpAsyncSocket, mtu)
{
    Icmp::AsyncSocket client;

    ASSERT_EQ (client.mtu (), -1);
    ASSERT_EQ (client.connect (_host), 0) << join::lastError.message ();
    ASSERT_NE (client.mtu (), -1) << join::lastError.message ();
    client.close ();
    ASSERT_EQ (client.mtu (), -1);
}

/**
 * @brief Test ttl method.
 */
TEST_F (IcmpAsyncSocket, ttl)
{
    Icmp::AsyncSocket client;

    ASSERT_EQ (client.ttl (), 60);

    Icmp::AsyncSocket other (32);

    ASSERT_EQ (other.ttl (), 32);
}

/**
 * @brief Test family method.
 */
TEST_F (IcmpAsyncSocket, family)
{
    Icmp::AsyncSocket client;

    ASSERT_EQ (client.open (Icmp::v4 ()), 0) << join::lastError.message ();
    ASSERT_EQ (client.family (), AF_INET);
    client.close ();
}

/**
 * @brief Test type method.
 */
TEST_F (IcmpAsyncSocket, type)
{
    Icmp::AsyncSocket client;

    ASSERT_EQ (client.type (), SOCK_RAW);
}

/**
 * @brief Test protocol method.
 */
TEST_F (IcmpAsyncSocket, protocol)
{
    Icmp::AsyncSocket client;

    ASSERT_EQ (client.bind (IpAddress (AF_INET6)), 0) << join::lastError.message ();
    ASSERT_EQ (client.protocol (), IPPROTO_ICMPV6);
    client.close ();

    ASSERT_EQ (client.bind (IpAddress (AF_INET)), 0) << join::lastError.message ();
    ASSERT_EQ (client.protocol (), IPPROTO_ICMP);
    client.close ();
}

/**
 * @brief Test handle method.
 */
TEST_F (IcmpAsyncSocket, handle)
{
    Icmp::AsyncSocket client;

    ASSERT_EQ (client.handle (), -1);
    ASSERT_EQ (client.open (Icmp::v4 ()), 0) << join::lastError.message ();
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
