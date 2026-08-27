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
#include <join/async_socket.hpp>
#include <join/condition.hpp>

// Libraries.
#include <gtest/gtest.h>

// C.
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <net/ethernet.h>

using join::Errc;
using join::Mutex;
using join::Condition;
using join::ScopedLock;
using join::IpAddress;
using join::MacAddress;
using join::Raw;

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

    /// timeout.
    static const int _timeout;
};

Mutex RawAsyncSocket::_mut;
Condition RawAsyncSocket::_cond;
std::error_code RawAsyncSocket::_code;
int RawAsyncSocket::_completions = 0;
size_t RawAsyncSocket::_transferred = 0;
RawAsyncSocket::Packet RawAsyncSocket::_packet;
char RawAsyncSocket::_buf[2048] = {};
const std::string RawAsyncSocket::_interface = "lo";
const int RawAsyncSocket::_timeout = 1000;

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
 * @brief Test asyncWrite method.
 */
TEST_F (RawAsyncSocket, asyncWrite)
{
    Raw::AsyncSocket rawSocket;

    ASSERT_EQ (rawSocket.asyncWrite (reinterpret_cast<char*> (&_packet), sizeof (_packet), nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (rawSocket.bind (_interface), 0) << join::lastError.message ();
    ASSERT_EQ (rawSocket.asyncWrite (reinterpret_cast<char*> (&_packet), sizeof (_packet), onCompletion), 0)
        << join::lastError.message ();

    ASSERT_TRUE (wait (1));
    ASSERT_FALSE (_code) << _code.message ();
    ASSERT_EQ (_transferred, sizeof (_packet));

    rawSocket.close ();
}

/**
 * @brief Test asyncRead method.
 */
TEST_F (RawAsyncSocket, asyncRead)
{
    Raw::AsyncSocket rawSocket;

    ASSERT_EQ (rawSocket.asyncRead (_buf, sizeof (_buf), nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (rawSocket.bind (_interface), 0) << join::lastError.message ();
    ASSERT_EQ (rawSocket.asyncRead (_buf, sizeof (_buf), onCompletion), 0) << join::lastError.message ();

    ASSERT_EQ (rawSocket.asyncRead (_buf, sizeof (_buf), nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);

    ASSERT_EQ (rawSocket.asyncWrite (reinterpret_cast<char*> (&_packet), sizeof (_packet), nullptr), 0)
        << join::lastError.message ();

    ASSERT_TRUE (wait (1));
    ASSERT_FALSE (_code) << _code.message ();
    ASSERT_GT (_transferred, 0u);

    // a message larger than the buffer must be reported as truncated.
    ASSERT_EQ (rawSocket.asyncRead (_buf, sizeof (_packet) / 2, onCompletion), 0) << join::lastError.message ();
    ASSERT_EQ (rawSocket.asyncWrite (reinterpret_cast<char*> (&_packet), sizeof (_packet), nullptr), 0)
        << join::lastError.message ();

    ASSERT_TRUE (wait (2));
    ASSERT_EQ (_code, Errc::MessageTooLong) << _code.message ();

    rawSocket.close ();
}

/**
 * @brief Test cancelRead method.
 */
TEST_F (RawAsyncSocket, cancelRead)
{
    Raw::AsyncSocket rawSocket;

    ASSERT_EQ (rawSocket.cancelRead (), 0) << join::lastError.message ();

    ASSERT_EQ (rawSocket.bind (_interface), 0) << join::lastError.message ();
    ASSERT_EQ (rawSocket.asyncRead (_buf, sizeof (_buf), onCompletion), 0) << join::lastError.message ();
    ASSERT_EQ (rawSocket.cancelRead (), 0) << join::lastError.message ();

    ASSERT_TRUE (wait (1));
    ASSERT_EQ (_code, std::errc::operation_canceled) << _code.message ();

    rawSocket.close ();
}

/**
 * @brief Test cancelWrite method.
 */
TEST_F (RawAsyncSocket, cancelWrite)
{
    Raw::AsyncSocket rawSocket;

    ASSERT_EQ (rawSocket.cancelWrite (), 0) << join::lastError.message ();

    ASSERT_EQ (rawSocket.bind (_interface), 0) << join::lastError.message ();
    ASSERT_EQ (rawSocket.cancelWrite (), 0) << join::lastError.message ();

    rawSocket.close ();
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
    ASSERT_EQ (rawSocket.asyncWrite (reinterpret_cast<char*> (&_packet), sizeof (_packet), onCompletion), 0)
        << join::lastError.message ();

    ASSERT_TRUE (wait (1));
    ASSERT_FALSE (_code) << _code.message ();
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
