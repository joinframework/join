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

using join::Errc;
using join::Mutex;
using join::Condition;
using join::ScopedLock;
using join::IpAddress;
using join::Udp;

/**
 * @brief Class used to test the udp asynchronous datagram socket API.
 */
class UdpAsyncDatagramSocket : public ::testing::Test
{
protected:
    /**
     * @brief Sets up the test fixture.
     */
    void SetUp () override
    {
        ASSERT_EQ (server ().bind ({IpAddress::ipv6Wildcard, _port}), 0) << join::lastError.message ();
        ASSERT_EQ (server ().asyncReadFrom (_echobuf, sizeof (_echobuf), _echofrom, onEchoRead), 0)
            << join::lastError.message ();

        ScopedLock<Mutex> lock (_mut);

        _code = {};
        _completions = 0;
        _transferred = 0;
        _rearms = 0;
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
     * @param size number of bytes read.
     */
    static void onEchoRead (const std::error_code& ec, size_t size)
    {
        if (!ec)
        {
            server ().asyncWriteTo (_echobuf, size, _echofrom, onEchoWrite);
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
            server ().asyncReadFrom (_echobuf, sizeof (_echobuf), _echofrom, onEchoRead);
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
     * @brief handler resubmitting a read from within itself.
     * @param ec error reported by the socket.
     * @param size number of bytes read.
     */
    static void onRead (const std::error_code& ec, size_t size)
    {
        if (!ec && (_rearms > 0))
        {
            --_rearms;
            _current->asyncReadFrom (_buf, sizeof (_buf), _from, onRead);
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
            _current->asyncWriteTo ("two", 3, _dest, onWrite);
        }

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
    static const int _timeout;
};

Mutex UdpAsyncDatagramSocket::_mut;
Condition UdpAsyncDatagramSocket::_cond;
std::error_code UdpAsyncDatagramSocket::_code;
int UdpAsyncDatagramSocket::_completions = 0;
size_t UdpAsyncDatagramSocket::_transferred = 0;
char UdpAsyncDatagramSocket::_buf[1024] = {};
Udp::Endpoint UdpAsyncDatagramSocket::_from;
char UdpAsyncDatagramSocket::_echobuf[1024] = {};
Udp::Endpoint UdpAsyncDatagramSocket::_echofrom;
Udp::AsyncSocket* UdpAsyncDatagramSocket::_current = nullptr;
int UdpAsyncDatagramSocket::_rearms = 0;
Udp::Endpoint UdpAsyncDatagramSocket::_dest;
const std::string UdpAsyncDatagramSocket::_host = "127.0.0.1";
const uint16_t UdpAsyncDatagramSocket::_port = 5036;
const int UdpAsyncDatagramSocket::_timeout = 1000;

/**
 * @brief Test open method.
 */
TEST_F (UdpAsyncDatagramSocket, open)
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
TEST_F (UdpAsyncDatagramSocket, close)
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
TEST_F (UdpAsyncDatagramSocket, bind)
{
    Udp::AsyncSocket client;

    ASSERT_EQ (client.open (Udp::v6 ()), 0) << join::lastError.message ();
    ASSERT_EQ (client.bind (IpAddress (AF_INET6)), 0) << join::lastError.message ();
    client.close ();
}

/**
 * @brief Test bindToDevice method.
 */
TEST_F (UdpAsyncDatagramSocket, bindToDevice)
{
    Udp::AsyncSocket client;

    ASSERT_EQ (client.bindToDevice ("lo"), -1);
    ASSERT_EQ (client.open (Udp::v6 ()), 0) << join::lastError.message ();
    ASSERT_EQ (client.bindToDevice ("lo"), 0) << join::lastError.message ();
    ASSERT_EQ (client.bindToDevice ("foo"), -1);
    client.close ();
}

/**
 * @brief Test connect method.
 */
TEST_F (UdpAsyncDatagramSocket, connect)
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
TEST_F (UdpAsyncDatagramSocket, disconnect)
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
TEST_F (UdpAsyncDatagramSocket, asyncWriteTo)
{
    Udp::AsyncSocket client;
    Udp::Endpoint dest (_host, _port);

    ASSERT_FALSE (client.opened ());
    ASSERT_EQ (client.asyncWriteTo ("hello", 5, dest, onReport), 0) << join::lastError.message ();
    ASSERT_TRUE (client.opened ());

    ASSERT_EQ (client.asyncWriteTo ("hello", 5, dest, nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
        ASSERT_EQ (_transferred, 5u);
    }

    client.close ();
}

/**
 * @brief Test asyncReadFrom method.
 */
TEST_F (UdpAsyncDatagramSocket, asyncReadFrom)
{
    Udp::AsyncSocket client;

    ASSERT_EQ (client.asyncReadFrom (_buf, sizeof (_buf), _from, nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (client.asyncReadFrom (_buf, sizeof (_buf), _from, onReport), 0) << join::lastError.message ();

    ASSERT_EQ (client.asyncReadFrom (_buf, sizeof (_buf), _from, nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);

    ASSERT_EQ (client.asyncWrite ("hello", 5, nullptr), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
        ASSERT_EQ (_transferred, 5u);
        ASSERT_EQ (std::string (_buf, 5), "hello");
    }

    client.close ();

    ASSERT_EQ (_from, Udp::Endpoint (_host, _port));
}

/**
 * @brief Test asyncWrite method.
 */
TEST_F (UdpAsyncDatagramSocket, asyncWrite)
{
    Udp::AsyncSocket client;

    ASSERT_EQ (client.asyncWrite ("hello", 5, nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (client.asyncWrite ("hello", 5, onReport), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
        ASSERT_EQ (_transferred, 5u);
    }

    client.close ();
}

/**
 * @brief Test asyncRead method.
 */
TEST_F (UdpAsyncDatagramSocket, asyncRead)
{
    Udp::AsyncSocket client;

    ASSERT_EQ (client.asyncRead (_buf, sizeof (_buf), nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (client.asyncRead (_buf, sizeof (_buf), onReport), 0) << join::lastError.message ();
    ASSERT_EQ (client.asyncWrite ("hello", 5, nullptr), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
        ASSERT_EQ (_transferred, 5u);
        ASSERT_EQ (std::string (_buf, 5), "hello");
    }

    client.close ();
}

/**
 * @brief Test asyncReadFrom method resubmitted from its own handler.
 */
TEST_F (UdpAsyncDatagramSocket, resubmit)
{
    Udp::AsyncSocket client;
    Udp::Endpoint dest (_host, _port);

    _current = &client;
    _rearms = 1;

    ASSERT_EQ (client.bind (Udp::Endpoint (_host, 0)), 0) << join::lastError.message ();
    ASSERT_EQ (client.asyncReadFrom (_buf, sizeof (_buf), _from, onRead), 0) << join::lastError.message ();
    ASSERT_EQ (client.asyncWriteTo ("one", 3, dest, nullptr), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
    }

    ASSERT_EQ (client.asyncWriteTo ("two", 3, dest, nullptr), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 2;
        }));
        ASSERT_FALSE (_code) << _code.message ();
    }

    _dest = Udp::Endpoint (_host, _port);
    _rearms = 1;

    ASSERT_EQ (client.asyncWriteTo ("one", 3, _dest, onWrite), 0) << join::lastError.message ();

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
TEST_F (UdpAsyncDatagramSocket, closeFromWriteHandler)
{
    Udp::AsyncSocket client;
    Udp::Endpoint dest (_host, _port);

    _current = &client;

    ASSERT_EQ (client.asyncWriteTo ("hello", 5, dest, onWriteAndClose), 0) << join::lastError.message ();

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
TEST_F (UdpAsyncDatagramSocket, truncated)
{
    Udp::AsyncSocket client;
    char small[4] = {};

    ASSERT_EQ (client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (client.asyncReadFrom (small, sizeof (small), _from, onReport), 0) << join::lastError.message ();
    ASSERT_EQ (client.asyncWrite ("hello world", 11, nullptr), 0) << join::lastError.message ();

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
TEST_F (UdpAsyncDatagramSocket, empty)
{
    Udp::AsyncSocket client;
    Udp::Socket sender;
    Udp::Endpoint self (_host, uint16_t (_port + 2));

    ASSERT_EQ (client.bind (self), 0) << join::lastError.message ();
    ASSERT_EQ (client.asyncReadFrom (_buf, sizeof (_buf), _from, onReport), 0) << join::lastError.message ();

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
 * @brief Test cancelRead method.
 */
TEST_F (UdpAsyncDatagramSocket, cancelRead)
{
    Udp::AsyncSocket client;

    ASSERT_EQ (client.cancelRead (), 0) << join::lastError.message ();
    ASSERT_EQ (client.bind (Udp::Endpoint (_host, 0)), 0) << join::lastError.message ();
    ASSERT_EQ (client.asyncReadFrom (_buf, sizeof (_buf), _from, onReport), 0) << join::lastError.message ();
    ASSERT_EQ (client.cancelRead (), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_EQ (_code, std::errc::operation_canceled);
    }

    client.close ();
}

/**
 * @brief Test cancelWrite method.
 */
TEST_F (UdpAsyncDatagramSocket, cancelWrite)
{
    Udp::AsyncSocket client;

    ASSERT_EQ (client.cancelWrite (), 0) << join::lastError.message ();
    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_EQ (client.cancelWrite (), 0) << join::lastError.message ();
    client.close ();
}

/**
 * @brief Test setOption method.
 */
TEST_F (UdpAsyncDatagramSocket, setOption)
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
TEST_F (UdpAsyncDatagramSocket, localEndpoint)
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
TEST_F (UdpAsyncDatagramSocket, remoteEndpoint)
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
TEST_F (UdpAsyncDatagramSocket, opened)
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
TEST_F (UdpAsyncDatagramSocket, connected)
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
TEST_F (UdpAsyncDatagramSocket, canRead)
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
TEST_F (UdpAsyncDatagramSocket, mtu)
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
TEST_F (UdpAsyncDatagramSocket, ttl)
{
    Udp::AsyncSocket client;

    ASSERT_EQ (client.ttl (), 60);

    Udp::AsyncSocket other (32);

    ASSERT_EQ (other.ttl (), 32);
}

/**
 * @brief Test family method.
 */
TEST_F (UdpAsyncDatagramSocket, family)
{
    Udp::AsyncSocket client;

    ASSERT_EQ (client.open (Udp::v6 ()), 0) << join::lastError.message ();
    ASSERT_EQ (client.family (), AF_INET6);
    client.close ();
}

/**
 * @brief Test type method.
 */
TEST_F (UdpAsyncDatagramSocket, type)
{
    Udp::AsyncSocket client;

    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_EQ (client.type (), SOCK_DGRAM);
    client.close ();
}

/**
 * @brief Test protocol method.
 */
TEST_F (UdpAsyncDatagramSocket, protocol)
{
    Udp::AsyncSocket client;

    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_EQ (client.protocol (), IPPROTO_UDP);
    client.close ();
}

/**
 * @brief Test handle method.
 */
TEST_F (UdpAsyncDatagramSocket, handle)
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
