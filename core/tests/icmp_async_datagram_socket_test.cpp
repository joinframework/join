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
using join::Icmp;

/**
 * @brief Class used to test the icmp asynchronous datagram socket API.
 */
class IcmpAsyncDatagramSocket : public ::testing::Test
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

    /// echo request sent by the tests.
    static char _data[sizeof (struct icmphdr)];

    /// host.
    static const std::string _host;

    /// timeout.
    static const int _timeout;
};

Mutex IcmpAsyncDatagramSocket::_mut;
Condition IcmpAsyncDatagramSocket::_cond;
std::error_code IcmpAsyncDatagramSocket::_code;
int IcmpAsyncDatagramSocket::_completions = 0;
size_t IcmpAsyncDatagramSocket::_transferred = 0;
char IcmpAsyncDatagramSocket::_buf[1024] = {};
Icmp::Endpoint IcmpAsyncDatagramSocket::_from;
char IcmpAsyncDatagramSocket::_data[sizeof (struct icmphdr)] = {};
const std::string IcmpAsyncDatagramSocket::_host = "127.0.0.1";
const int IcmpAsyncDatagramSocket::_timeout = 1000;

/**
 * @brief Test open method.
 */
TEST_F (IcmpAsyncDatagramSocket, open)
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
TEST_F (IcmpAsyncDatagramSocket, close)
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
TEST_F (IcmpAsyncDatagramSocket, bind)
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
TEST_F (IcmpAsyncDatagramSocket, bindToDevice)
{
    Icmp::AsyncSocket client;

    ASSERT_EQ (client.bindToDevice ("lo"), -1);
    ASSERT_EQ (client.open (Icmp::v4 ()), 0) << join::lastError.message ();
    ASSERT_EQ (client.bindToDevice ("lo"), 0) << join::lastError.message ();
    ASSERT_EQ (client.bindToDevice ("foo"), -1);
    client.close ();
}

/**
 * @brief Test connect method.
 */
TEST_F (IcmpAsyncDatagramSocket, connect)
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
TEST_F (IcmpAsyncDatagramSocket, disconnect)
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
TEST_F (IcmpAsyncDatagramSocket, asyncWriteTo)
{
    Icmp::AsyncSocket client;
    Icmp::Endpoint dest (_host);

    ASSERT_FALSE (client.opened ());
    ASSERT_EQ (client.asyncWriteTo (_data, sizeof (_data), dest, onReport), 0) << join::lastError.message ();
    ASSERT_TRUE (client.opened ());

    ASSERT_EQ (client.asyncWriteTo (_data, sizeof (_data), dest, nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
        ASSERT_EQ (_transferred, sizeof (_data));
    }

    client.close ();
}

/**
 * @brief Test asyncReadFrom method.
 */
TEST_F (IcmpAsyncDatagramSocket, asyncReadFrom)
{
    Icmp::AsyncSocket client, server;

    ASSERT_EQ (server.asyncReadFrom (_buf, sizeof (_buf), _from, nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (server.bind (_host), 0) << join::lastError.message ();
    ASSERT_EQ (server.asyncReadFrom (_buf, sizeof (_buf), _from, onReport), 0) << join::lastError.message ();

    ASSERT_EQ (server.asyncReadFrom (_buf, sizeof (_buf), _from, nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);

    ASSERT_EQ (client.connect (_host), 0) << join::lastError.message ();
    ASSERT_EQ (client.asyncWrite (_data, sizeof (_data), nullptr), 0) << join::lastError.message ();

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
 * @brief Test asyncWrite method.
 */
TEST_F (IcmpAsyncDatagramSocket, asyncWrite)
{
    Icmp::AsyncSocket client;

    ASSERT_EQ (client.asyncWrite (_data, sizeof (_data), nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (client.connect (_host), 0) << join::lastError.message ();
    ASSERT_EQ (client.asyncWrite (_data, sizeof (_data), onReport), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
        ASSERT_EQ (_transferred, sizeof (_data));
    }

    client.close ();
}

/**
 * @brief Test asyncRead method.
 */
TEST_F (IcmpAsyncDatagramSocket, asyncRead)
{
    Icmp::AsyncSocket client;

    ASSERT_EQ (client.asyncRead (_buf, sizeof (_buf), nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (client.connect (_host), 0) << join::lastError.message ();
    ASSERT_EQ (client.asyncRead (_buf, sizeof (_buf), onReport), 0) << join::lastError.message ();
    ASSERT_EQ (client.asyncWrite (_data, sizeof (_data), nullptr), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
        ASSERT_GT (_transferred, 0u);
    }

    client.close ();
}

/**
 * @brief Test cancelRead method.
 */
TEST_F (IcmpAsyncDatagramSocket, cancelRead)
{
    Icmp::AsyncSocket client;

    ASSERT_EQ (client.cancelRead (), 0) << join::lastError.message ();
    ASSERT_EQ (client.bind (_host), 0) << join::lastError.message ();
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
TEST_F (IcmpAsyncDatagramSocket, cancelWrite)
{
    Icmp::AsyncSocket client;

    ASSERT_EQ (client.cancelWrite (), 0) << join::lastError.message ();
    ASSERT_EQ (client.open (Icmp::v4 ()), 0) << join::lastError.message ();
    ASSERT_EQ (client.cancelWrite (), 0) << join::lastError.message ();
    client.close ();
}

/**
 * @brief Test setOption method.
 */
TEST_F (IcmpAsyncDatagramSocket, setOption)
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
TEST_F (IcmpAsyncDatagramSocket, localEndpoint)
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
TEST_F (IcmpAsyncDatagramSocket, remoteEndpoint)
{
    Icmp::AsyncSocket client;

    ASSERT_EQ (client.connect (_host), 0) << join::lastError.message ();
    ASSERT_EQ (client.remoteEndpoint (), Icmp::Endpoint (_host));
    client.close ();
}

/**
 * @brief Test opened method.
 */
TEST_F (IcmpAsyncDatagramSocket, opened)
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
TEST_F (IcmpAsyncDatagramSocket, connected)
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
TEST_F (IcmpAsyncDatagramSocket, canRead)
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
TEST_F (IcmpAsyncDatagramSocket, mtu)
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
TEST_F (IcmpAsyncDatagramSocket, ttl)
{
    Icmp::AsyncSocket client;

    ASSERT_EQ (client.ttl (), 60);

    Icmp::AsyncSocket other (32);

    ASSERT_EQ (other.ttl (), 32);
}

/**
 * @brief Test family method.
 */
TEST_F (IcmpAsyncDatagramSocket, family)
{
    Icmp::AsyncSocket client;

    ASSERT_EQ (client.open (Icmp::v4 ()), 0) << join::lastError.message ();
    ASSERT_EQ (client.family (), AF_INET);
    client.close ();
}

/**
 * @brief Test type method.
 */
TEST_F (IcmpAsyncDatagramSocket, type)
{
    Icmp::AsyncSocket client;

    ASSERT_EQ (client.type (), SOCK_RAW);
}

/**
 * @brief Test protocol method.
 */
TEST_F (IcmpAsyncDatagramSocket, protocol)
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
TEST_F (IcmpAsyncDatagramSocket, handle)
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
