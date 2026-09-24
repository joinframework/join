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
#include <join/async_raw_socket.hpp>
#include <join/condition.hpp>

// Libraries.
#include <gtest/gtest.h>

// C.
#include <net/ethernet.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <unistd.h>

using join::Errc;
using join::Mutex;
using join::Condition;
using join::ScopedLock;
using join::IpAddress;
using join::MacAddress;
using join::Raw;
using join::Proactor;
using join::LocalMem;

/**
 * @brief Class used to test the raw asynchronous socket API.
 */
class RawAsyncSocket : public ::testing::Test
{
public:
    /**
     * @brief set up test case.
     */
    static void SetUpTestCase ()
    {
        // fill in data.
        memcpy (_packet.data, "this is a test", strlen ("this is a test"));

        // fill in UDP header.
        _packet.ip.protocol = IPPROTO_UDP;
        _packet.ip.saddr = *reinterpret_cast<const uint32_t*> (IpAddress ("127.0.0.1").addr ());
        _packet.ip.daddr = *reinterpret_cast<const uint32_t*> (IpAddress ("127.0.0.1").addr ());
        _packet.udp.source = htons (5000);
        _packet.udp.dest = htons (5000);
        _packet.udp.len = htons (sizeof (Packet) - sizeof (_packet.eth) - sizeof (_packet.ip));
        _packet.ip.tot_len = _packet.udp.len;
        _packet.udp.check =
            join::checksum (reinterpret_cast<uint16_t*> (&_packet.ip), sizeof (Packet) - sizeof (_packet.eth));

        // fill in IP header.
        _packet.ip.ihl = sizeof (_packet.ip) >> 2;
        _packet.ip.version = IPVERSION;
        _packet.ip.tos = IPTOS_CLASS_CS6 | IPTOS_ECN_NOT_ECT;
        _packet.ip.tot_len = htons (sizeof (Packet) - sizeof (_packet.eth));
        _packet.ip.id = htons (join::randomize<uint16_t> ());
        _packet.ip.frag_off = htons (IP_DF);
        _packet.ip.ttl = IPDEFTTL;
        _packet.ip.check = join::checksum (reinterpret_cast<uint16_t*> (&_packet.ip), sizeof (_packet.ip));

        // fill in ETH header.
        memcpy (_packet.eth.h_dest, MacAddress::wildcard.addr (), 6);
        memcpy (_packet.eth.h_source, MacAddress::wildcard.addr (), 6);
        _packet.eth.h_proto = htons (ETH_P_IP);
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
     * @brief report a completion to the waiting test.
     * @param ec error reported by the socket.
     * @param size number of bytes transferred.
     */
    static void onCompletion (const std::error_code& ec, size_t size)
    {
        ScopedLock<Mutex> lock (_mut);

        _code = ec;
        _transferred = size;
        ++_completions;
        _cond.signal ();
    }

    /**
     * @brief report a read completion to the waiting test.
     * @param ec error reported by the socket.
     * @param data buffer holding the data received.
     * @param size number of bytes read.
     * @param more true if the read stays armed.
     */
    static void onReadCompletion (const std::error_code& ec, [[maybe_unused]] const char* data, size_t size,
                                  [[maybe_unused]] bool more)
    {
        onCompletion (ec, size);
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

        onCompletion (ec, size);
    }

    /**
     * @brief handler resubmitting a write from within itself.
     */
    static void onWrite (const std::error_code& ec, size_t size)
    {
        if (!ec && (_rearms > 0))
        {
            --_rearms;
            _current->asyncWrite (reinterpret_cast<char*> (&_packet), sizeof (_packet), onWrite);
        }

        onCompletion (ec, size);
    }

    /**
     * @brief handler closing the socket from within itself.
     */
    static void onWriteAndClose (const std::error_code& ec, size_t size)
    {
        _current->close ();
        onCompletion (ec, size);
    }

    /**
     * @brief report a multishot completion to the waiting test.
     * @param ec error reported by the socket.
     * @param data buffer holding the data received.
     * @param size number of bytes read.
     * @param more true if the read stays armed.
     */
    static void onReportMulti (const std::error_code& ec, const char* data, size_t size, bool more)
    {
        ScopedLock<Mutex> lock (_mut);

        if (!ec && (size <= sizeof (_buf)))
        {
            ::memcpy (_buf, data, size);
        }

        _code = ec;
        _transferred = size;
        _more = more;
        ++_completions;
        _cond.signal ();
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
     * @brief Raw packet.
     */
    struct __attribute__ ((packed)) Packet
    {
        struct ethhdr eth = {};
        struct iphdr ip = {};
        struct udphdr udp = {};
        char data[16] = {};
    };

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

    /// mutex.
    static Mutex _mut;

    /// condition variable.
    static Condition _cond;

    /// error reported by the last completion.
    static std::error_code _code;

    /// number of completions reported.
    static int _completions;

    /// number of bytes reported by the last completion.
    static size_t _transferred;

    /// packet.
    static Packet _packet;

    /// read buffer.
    static char _buf[2048];

    /// interface.
    static const std::string _interface;

    /// socket used by the resubmitting handler.
    static Raw::AsyncSocket* _current;

    /// last reported multishot state.
    static bool _more;

    /// number of resubmissions left to perform from a handler.
    static int _rearms;

    /// timeout.
    static const std::chrono::milliseconds _timeout;
};

Mutex RawAsyncSocket::_mut;
Condition RawAsyncSocket::_cond;
std::error_code RawAsyncSocket::_code;
int RawAsyncSocket::_completions = 0;
size_t RawAsyncSocket::_transferred = 0;
RawAsyncSocket::Packet RawAsyncSocket::_packet;
char RawAsyncSocket::_buf[2048] = {};
const std::string RawAsyncSocket::_interface = "lo";
Raw::AsyncSocket* RawAsyncSocket::_current = nullptr;
bool RawAsyncSocket::_more = false;
int RawAsyncSocket::_rearms = 0;
const std::chrono::milliseconds RawAsyncSocket::_timeout{1000};

/**
 * @brief Test move.
 */
TEST_F (RawAsyncSocket, move)
{
    Raw::AsyncSocket client1, client3;

    ASSERT_EQ (client1.bind (_interface), 0) << join::lastError.message ();
    ASSERT_TRUE (client1.opened ());

    Raw::AsyncSocket client2 (std::move (client1));
    ASSERT_TRUE (client2.opened ());
    ASSERT_FALSE (client1.opened ());

    ASSERT_EQ (client1.asyncRead (_buf, sizeof (_buf), nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (client1.asyncWrite (reinterpret_cast<char*> (&_packet), sizeof (_packet), nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (client1.cancel (0), 0) << join::lastError.message ();
    ASSERT_EQ (client1.cancel (0), 0) << join::lastError.message ();
    client1.close ();

    ASSERT_NE (client2.asyncRead (_buf, sizeof (_buf), onRead), -1) << join::lastError.message ();

    client3 = std::move (client2);

    ASSERT_TRUE (client3.opened ());
    ASSERT_FALSE (client2.opened ());

    ASSERT_NE (client3.asyncWrite (reinterpret_cast<char*> (&_packet), sizeof (_packet), nullptr), -1)
        << join::lastError.message ();

    ASSERT_TRUE (wait (1));
    ASSERT_FALSE (code ()) << code ().message ();
    ASSERT_GT (_transferred, 0u);

    client3.close ();
}

/**
 * @brief Test open method.
 */
TEST_F (RawAsyncSocket, open)
{
    Raw::AsyncSocket rawSocket;

    ASSERT_EQ (rawSocket.open (), 0) << join::lastError.message ();
    ASSERT_EQ (rawSocket.open (), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    rawSocket.close ();
}

/**
 * @brief Test close method.
 */
TEST_F (RawAsyncSocket, close)
{
    Raw::AsyncSocket rawSocket;

    ASSERT_EQ (rawSocket.open (), 0) << join::lastError.message ();
    ASSERT_TRUE (rawSocket.opened ());
    rawSocket.close ();
    ASSERT_FALSE (rawSocket.opened ());
}

/**
 * @brief Test bind method.
 */
TEST_F (RawAsyncSocket, bind)
{
    Raw::AsyncSocket rawSocket;

    ASSERT_EQ (rawSocket.bind (_interface), 0) << join::lastError.message ();
    rawSocket.close ();
}

/**
 * @brief Test bindToDevice method.
 */
TEST_F (RawAsyncSocket, bindToDevice)
{
    Raw::AsyncSocket rawSocket;

    ASSERT_EQ (rawSocket.bindToDevice (_interface), -1);
    ASSERT_EQ (rawSocket.open (), 0) << join::lastError.message ();
    ASSERT_EQ (rawSocket.bindToDevice (_interface), 0) << join::lastError.message ();
    ASSERT_EQ (rawSocket.bindToDevice ("foo"), -1);
    rawSocket.close ();
}

/**
 * @brief Test asyncWait method.
 */
TEST_F (RawAsyncSocket, asyncWait)
{
    Raw::AsyncSocket rawSocket;

    ASSERT_EQ (rawSocket.asyncWait (true, false, nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (rawSocket.bind (_interface), 0) << join::lastError.message ();

    ASSERT_EQ (rawSocket.asyncWait (true, true, nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (rawSocket.asyncWait (false, false, nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);

    ASSERT_NE (rawSocket.asyncWait (false, true, onReportWait), -1) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
        ASSERT_FALSE (_more);
    }

    ASSERT_NE (rawSocket.asyncWait (true, false, onReportWait), -1) << join::lastError.message ();
    ASSERT_NE (rawSocket.asyncWrite (reinterpret_cast<char*> (&_packet), sizeof (_packet), nullptr), -1)
        << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 2;
        }));
        ASSERT_FALSE (_code) << _code.message ();
        ASSERT_FALSE (_more);
    }

    while (::recv (rawSocket.handle (), _buf, sizeof (_buf), MSG_DONTWAIT) > 0)
    {
    }

#ifdef JOIN_HAS_IO_URING
    for (size_t i = 0; i < Raw::AsyncSocket::_opCount; ++i)
    {
        ASSERT_NE (rawSocket.asyncWait (true, false, nullptr), -1) << join::lastError.message ();
    }

    ASSERT_EQ (rawSocket.asyncWait (true, false, nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::OutOfMemory);
#endif

    rawSocket.close ();
}

/**
 * @brief Test asyncWaitMulti method.
 */
TEST_F (RawAsyncSocket, asyncWaitMulti)
{
    Raw::AsyncSocket rawSocket;

    ASSERT_EQ (rawSocket.asyncWaitMulti (true, false, nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (rawSocket.bind (_interface), 0) << join::lastError.message ();

    ASSERT_EQ (rawSocket.asyncWaitMulti (true, true, nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (rawSocket.asyncWaitMulti (false, false, nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);

    ssize_t index = rawSocket.asyncWaitMulti (true, false, onReportWait);
    ASSERT_NE (index, -1) << join::lastError.message ();

    for (int i = 1; i <= 2; ++i)
    {
        ASSERT_NE (rawSocket.asyncWrite (reinterpret_cast<char*> (&_packet), sizeof (_packet), nullptr), -1)
            << join::lastError.message ();

        {
            ScopedLock<Mutex> lock (_mut);
            ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&rawSocket, i] () {
                return (_completions >= i) && (rawSocket.canRead () >= 1);
            }));
            ASSERT_FALSE (_code) << _code.message ();
            ASSERT_TRUE (_more);
        }

        while (::recv (rawSocket.handle (), _buf, sizeof (_buf), MSG_DONTWAIT) > 0)
        {
        }
    }

    ASSERT_EQ (rawSocket.cancel (static_cast<size_t> (index)), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _code == std::errc::operation_canceled;
        }));
        ASSERT_FALSE (_more);
    }

    rawSocket.close ();
}

/**
 * @brief Test asyncReadMulti method.
 */
TEST_F (RawAsyncSocket, asyncReadMulti)
{
    Raw::AsyncSocket rawSocket;

    ASSERT_EQ (rawSocket.asyncReadMulti (0, nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (rawSocket.bind (_interface), 0) << join::lastError.message ();

    LocalMem::Allocator<4, sizeof (_buf)> arena;
    ASSERT_EQ (rawSocket.registerBufferRing (0, arena), 0) << join::lastError.message ();

    ssize_t index = rawSocket.asyncReadMulti (0, onReportMulti);
    ASSERT_NE (index, -1) << join::lastError.message ();

    for (int i = 1; i <= 2; ++i)
    {
        ASSERT_NE (rawSocket.asyncWrite (reinterpret_cast<char*> (&_packet), sizeof (_packet), nullptr), -1)
            << join::lastError.message ();

        ASSERT_TRUE (wait (i));
        ASSERT_FALSE (code ()) << code ().message ();
        ASSERT_GT (_transferred, 0u);
        ASSERT_TRUE (_more);
    }

    ASSERT_EQ (rawSocket.cancel (static_cast<size_t> (index)), 0) << join::lastError.message ();

    ASSERT_TRUE (wait (3));
    ASSERT_EQ (code (), std::errc::operation_canceled);
    ASSERT_FALSE (_more);

#ifdef JOIN_HAS_IO_URING
    for (size_t i = 0; i < Raw::AsyncSocket::_opCount; ++i)
    {
        ASSERT_NE (rawSocket.asyncReadMulti (0, nullptr), -1) << join::lastError.message ();
    }

    ASSERT_EQ (rawSocket.asyncReadMulti (0, nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::OutOfMemory);
#endif

    rawSocket.close ();

    ASSERT_EQ (rawSocket.unregisterBufferRing (0), 0) << join::lastError.message ();
}

#ifdef JOIN_HAS_IO_URING
/**
 * @brief Test registerFixedBuffers method.
 */
TEST_F (RawAsyncSocket, registerFixedBuffers)
{
    Proactor proactor;
    Raw::AsyncSocket rawSocket (proactor);

    LocalMem::Allocator<1, 1024, 4096> arena;

    ASSERT_EQ (rawSocket.registerFixedBuffers (arena), 0) << join::lastError.message ();
    ASSERT_EQ (rawSocket.registerFixedBuffers (arena), -1);
    ASSERT_EQ (rawSocket.unregisterFixedBuffers (), 0) << join::lastError.message ();
    ASSERT_EQ (rawSocket.unregisterFixedBuffers (), -1);
}
#endif

/**
 * @brief Test asyncWrite method.
 */
TEST_F (RawAsyncSocket, asyncWrite)
{
    Raw::AsyncSocket rawSocket;

    ASSERT_EQ (rawSocket.asyncWrite (reinterpret_cast<char*> (&_packet), sizeof (_packet), nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (rawSocket.bind (_interface), 0) << join::lastError.message ();
    ASSERT_NE (rawSocket.asyncWrite (reinterpret_cast<char*> (&_packet), sizeof (_packet), onCompletion), -1)
        << join::lastError.message ();

    ASSERT_TRUE (wait (1));
    ASSERT_FALSE (code ()) << code ().message ();
    ASSERT_EQ (_transferred, sizeof (_packet));

    rawSocket.close ();
}

#ifdef JOIN_HAS_IO_URING
/**
 * @brief Test asyncWriteFixed method.
 */
TEST_F (RawAsyncSocket, asyncWriteFixed)
{
    Raw::AsyncSocket rawSocket;
    LocalMem::Allocator<1, 2048> arena;

    char* buf = static_cast<char*> (arena.allocate (sizeof (_buf)));
    ASSERT_NE (buf, nullptr);

    ASSERT_EQ (rawSocket.asyncWriteFixed (buf, sizeof (_packet), 0, nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (rawSocket.registerFixedBuffers (arena), 0) << join::lastError.message ();

    ASSERT_EQ (rawSocket.bind (_interface), 0) << join::lastError.message ();

    ::memcpy (buf, &_packet, sizeof (_packet));

    ASSERT_NE (rawSocket.asyncWriteFixed (buf, sizeof (_packet), 0, onCompletion), -1) << join::lastError.message ();

    ASSERT_TRUE (wait (1));
    ASSERT_FALSE (code ()) << code ().message ();
    ASSERT_EQ (_transferred, sizeof (_packet));

    rawSocket.close ();

    ASSERT_EQ (rawSocket.unregisterFixedBuffers (), 0) << join::lastError.message ();
}
#endif

/**
 * @brief Test asyncRead method.
 */
TEST_F (RawAsyncSocket, asyncRead)
{
    Raw::AsyncSocket rawSocket;

    ASSERT_EQ (rawSocket.asyncRead (_buf, sizeof (_buf), nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (rawSocket.bind (_interface), 0) << join::lastError.message ();
    ASSERT_NE (rawSocket.asyncRead (_buf, sizeof (_buf), onReadCompletion), -1) << join::lastError.message ();

    ASSERT_NE (rawSocket.asyncWrite (reinterpret_cast<char*> (&_packet), sizeof (_packet), nullptr), -1)
        << join::lastError.message ();

    ASSERT_TRUE (wait (1));
    ASSERT_FALSE (code ()) << code ().message ();
    ASSERT_GT (_transferred, 0u);

    // a message larger than the buffer must be reported as truncated.
    ASSERT_NE (rawSocket.asyncRead (_buf, sizeof (_packet) / 2, onReadCompletion), -1) << join::lastError.message ();
    ASSERT_NE (rawSocket.asyncWrite (reinterpret_cast<char*> (&_packet), sizeof (_packet), nullptr), -1)
        << join::lastError.message ();

    ASSERT_TRUE (wait (2));
    ASSERT_EQ (code (), Errc::MessageTooLong) << code ().message ();


    ASSERT_EQ (::close (rawSocket.handle ()), 0);

    ASSERT_NE (rawSocket.asyncRead (_buf, sizeof (_buf), onReadCompletion), -1) << join::lastError.message ();

    ASSERT_TRUE (wait (3));
    ASSERT_EQ (code (), std::errc::bad_file_descriptor) << code ().message ();

    rawSocket.close ();
}

#ifdef JOIN_HAS_IO_URING
/**
 * @brief Test asyncReadFixed method.
 */
TEST_F (RawAsyncSocket, asyncReadFixed)
{
    Raw::AsyncSocket rawSocket;
    LocalMem::Allocator<1, 2048> arena;

    char* buf = static_cast<char*> (arena.allocate (sizeof (_buf)));
    ASSERT_NE (buf, nullptr);

    ASSERT_EQ (rawSocket.asyncReadFixed (buf, sizeof (_buf), 0, nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (rawSocket.registerFixedBuffers (arena), 0) << join::lastError.message ();

    ASSERT_EQ (rawSocket.bind (_interface), 0) << join::lastError.message ();

    ASSERT_NE (rawSocket.asyncReadFixed (buf, sizeof (_buf), 0, onReadCompletion), -1) << join::lastError.message ();
    ASSERT_NE (rawSocket.asyncWrite (reinterpret_cast<char*> (&_packet), sizeof (_packet), nullptr), -1)
        << join::lastError.message ();

    ASSERT_TRUE (wait (1));
    ASSERT_FALSE (code ()) << code ().message ();
    ASSERT_GT (_transferred, 0u);

    rawSocket.close ();

    ASSERT_EQ (rawSocket.unregisterFixedBuffers (), 0) << join::lastError.message ();
}
#endif

/**
 * @brief Test async operations resubmitted from their own handlers.
 */
TEST_F (RawAsyncSocket, resubmit)
{
    Raw::AsyncSocket client;

    _current = &client;
    _rearms = 1;

    ASSERT_EQ (client.bind (_interface), 0) << join::lastError.message ();
    ASSERT_NE (client.asyncRead (_buf, sizeof (_buf), onRead), -1) << join::lastError.message ();
    ASSERT_NE (client.asyncWrite (reinterpret_cast<char*> (&_packet), sizeof (_packet), nullptr), -1)
        << join::lastError.message ();

    ASSERT_TRUE (wait (1));
    ASSERT_FALSE (code ()) << code ().message ();

    ASSERT_NE (client.asyncWrite (reinterpret_cast<char*> (&_packet), sizeof (_packet), nullptr), -1)
        << join::lastError.message ();

    ASSERT_TRUE (wait (2));
    ASSERT_FALSE (code ()) << code ().message ();

    _rearms = 1;
    ASSERT_NE (client.asyncWrite (reinterpret_cast<char*> (&_packet), sizeof (_packet), onWrite), -1)
        << join::lastError.message ();

    ASSERT_TRUE (wait (4));
    ASSERT_FALSE (code ()) << code ().message ();

    client.close ();
    _current = nullptr;
}

/**
 * @brief Test close called from within a write handler.
 */
TEST_F (RawAsyncSocket, closeFromWriteHandler)
{
    Raw::AsyncSocket client;

    _current = &client;

    ASSERT_EQ (client.bind (_interface), 0) << join::lastError.message ();
    ASSERT_NE (client.asyncWrite (reinterpret_cast<char*> (&_packet), sizeof (_packet), onWriteAndClose), -1)
        << join::lastError.message ();

    ASSERT_TRUE (wait (1));
    ASSERT_FALSE (code ()) << code ().message ();
    ASSERT_FALSE (client.opened ());

    _current = nullptr;
}

/**
 * @brief Test a packet larger than the supplied buffer.
 */
TEST_F (RawAsyncSocket, truncated)
{
    Raw::AsyncSocket client;
    char small[sizeof (_packet) / 2] = {};

    ASSERT_EQ (client.bind (_interface), 0) << join::lastError.message ();
    ASSERT_NE (client.asyncRead (small, sizeof (small), onReadCompletion), -1) << join::lastError.message ();
    ASSERT_NE (client.asyncWrite (reinterpret_cast<char*> (&_packet), sizeof (_packet), nullptr), -1)
        << join::lastError.message ();

    ASSERT_TRUE (wait (1));
    ASSERT_EQ (code (), Errc::MessageTooLong) << code ().message ();

    client.close ();
}

/**
 * @brief Test cancel method.
 */
TEST_F (RawAsyncSocket, cancel)
{
    {
        Raw::AsyncSocket rawSocket;

        ASSERT_EQ (rawSocket.cancel (0), 0) << join::lastError.message ();
        ASSERT_EQ (rawSocket.cancel (Raw::AsyncSocket::_opCount), -1);
        ASSERT_EQ (join::lastError, Errc::InvalidParam);

        ASSERT_EQ (rawSocket.bind (_interface), 0) << join::lastError.message ();

        ssize_t index = rawSocket.asyncWait (true, false, onReportWait);
        ASSERT_NE (index, -1) << join::lastError.message ();

        ASSERT_EQ (rawSocket.cancel (static_cast<size_t> (index)), 0) << join::lastError.message ();

        {
            ScopedLock<Mutex> lock (_mut);
            ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
                return _completions >= 1;
            }));
            ASSERT_EQ (_code, std::errc::operation_canceled);
            ASSERT_FALSE (_more);
        }

        rawSocket.close ();
    }

    {
        ScopedLock<Mutex> lock (_mut);
        _code = {};
        _completions = 0;
        _more = false;
        _transferred = 0;
    }

    {
        Raw::AsyncSocket rawSocket;

        ASSERT_EQ (rawSocket.cancel (0), 0) << join::lastError.message ();

        ASSERT_EQ (rawSocket.bind (_interface), 0) << join::lastError.message ();
        ASSERT_NE (rawSocket.asyncRead (_buf, sizeof (_buf), onReadCompletion), -1) << join::lastError.message ();
        ASSERT_EQ (rawSocket.cancel (0), 0) << join::lastError.message ();

        ASSERT_TRUE (wait (1));
        ASSERT_EQ (code (), std::errc::operation_canceled) << code ().message ();

        rawSocket.close ();
    }

    {
        ScopedLock<Mutex> lock (_mut);
        _code = {};
        _completions = 0;
        _more = false;
        _transferred = 0;
    }

    {
        Raw::AsyncSocket rawSocket;

        ASSERT_EQ (rawSocket.cancel (0), 0) << join::lastError.message ();

        ASSERT_EQ (rawSocket.bind (_interface), 0) << join::lastError.message ();
        ASSERT_EQ (rawSocket.cancel (0), 0) << join::lastError.message ();

        rawSocket.close ();
    }
}

/**
 * @brief Test setOption method.
 */
TEST_F (RawAsyncSocket, setOption)
{
    Raw::AsyncSocket rawSocket;

    ASSERT_EQ (rawSocket.setOption (Raw::Socket::RcvBuffer, 1500), -1);
    ASSERT_EQ (rawSocket.open (), 0) << join::lastError.message ();
    ASSERT_EQ (rawSocket.setOption (Raw::Socket::RcvBuffer, 1500), 0) << join::lastError.message ();
    ASSERT_EQ (rawSocket.setOption (Raw::Socket::SndBuffer, 1500), 0) << join::lastError.message ();
    rawSocket.close ();
}

/**
 * @brief Test localEndpoint method.
 */
TEST_F (RawAsyncSocket, localEndpoint)
{
    Raw::AsyncSocket rawSocket;

    ASSERT_EQ (rawSocket.bind (_interface), 0) << join::lastError.message ();
    ASSERT_EQ (rawSocket.localEndpoint ().device (), _interface);
    rawSocket.close ();
}

/**
 * @brief Test opened method.
 */
TEST_F (RawAsyncSocket, opened)
{
    Raw::AsyncSocket rawSocket;

    ASSERT_FALSE (rawSocket.opened ());
    ASSERT_EQ (rawSocket.open (), 0) << join::lastError.message ();
    ASSERT_TRUE (rawSocket.opened ());
    rawSocket.close ();
    ASSERT_FALSE (rawSocket.opened ());
}

/**
 * @brief Test canRead method.
 */
TEST_F (RawAsyncSocket, canRead)
{
    Raw::AsyncSocket rawSocket;

    ASSERT_EQ (rawSocket.canRead (), -1);
    ASSERT_EQ (rawSocket.bind (_interface), 0) << join::lastError.message ();
    ASSERT_NE (rawSocket.asyncWrite (reinterpret_cast<char*> (&_packet), sizeof (_packet), onCompletion), -1)
        << join::lastError.message ();

    ASSERT_TRUE (wait (1));
    ASSERT_FALSE (code ()) << code ().message ();
    ASSERT_GT (rawSocket.canRead (), 0) << join::lastError.message ();

    rawSocket.close ();
}

/**
 * @brief Test family method.
 */
TEST_F (RawAsyncSocket, family)
{
    Raw::AsyncSocket rawSocket;

    ASSERT_EQ (rawSocket.open (), 0) << join::lastError.message ();
    ASSERT_EQ (rawSocket.family (), AF_PACKET);
    rawSocket.close ();
}

/**
 * @brief Test type method.
 */
TEST_F (RawAsyncSocket, type)
{
    Raw::AsyncSocket rawSocket;

    ASSERT_EQ (rawSocket.open (), 0) << join::lastError.message ();
    ASSERT_EQ (rawSocket.type (), SOCK_RAW);
    rawSocket.close ();
}

/**
 * @brief Test protocol method.
 */
TEST_F (RawAsyncSocket, protocol)
{
    Raw::AsyncSocket rawSocket;

    ASSERT_EQ (rawSocket.open (), 0) << join::lastError.message ();
    ASSERT_EQ (rawSocket.protocol (), Raw ().protocol ());
    rawSocket.close ();
}

/**
 * @brief Test handle method.
 */
TEST_F (RawAsyncSocket, handle)
{
    Raw::AsyncSocket rawSocket;

    ASSERT_EQ (rawSocket.handle (), -1);
    ASSERT_EQ (rawSocket.open (), 0) << join::lastError.message ();
    ASSERT_GT (rawSocket.handle (), -1);
    rawSocket.close ();
    ASSERT_EQ (rawSocket.handle (), -1);
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);

    return RUN_ALL_TESTS ();
}
