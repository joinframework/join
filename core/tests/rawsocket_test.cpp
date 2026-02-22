/**
 * MIT License
 *
 * Copyright (c) 2021 Mathieu Rabine
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
#include <join/reactor.hpp>
#include <join/socket.hpp>

// Libraries.
#include <gtest/gtest.h>

// C.
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <net/ethernet.h>

using join::Errc;
using join::MacAddress;
using join::IpAddress;
using join::ReactorThread;
using join::Raw;

/**
 * @brief Class used to test the raw socket API.
 */
class RawSocket : public Raw::Socket, public ::testing::Test
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
        _packet.ip.saddr    = *reinterpret_cast <const uint32_t *> (IpAddress ("127.0.0.1").addr ());
        _packet.ip.daddr    = *reinterpret_cast <const uint32_t *> (IpAddress ("127.0.0.1").addr ());
        _packet.udp.source  = htons (5000);
        _packet.udp.dest    = htons (5000);
        _packet.udp.len     = htons (sizeof (Packet) - sizeof (_packet.eth) - sizeof (_packet.ip));
        _packet.ip.tot_len  = _packet.udp.len;
        _packet.udp.check   = Raw::Socket::checksum (reinterpret_cast <uint16_t *> (&_packet.ip), sizeof (Packet) - sizeof (_packet.eth));

        // fill in IP header.
        _packet.ip.ihl      = sizeof (_packet.ip) >> 2;
        _packet.ip.version  = IPVERSION;
        _packet.ip.tos      = IPTOS_CLASS_CS6 | IPTOS_ECN_NOT_ECT;
        _packet.ip.tot_len  = htons (sizeof (Packet) - sizeof (_packet.eth));
        _packet.ip.id       = htons (join::randomize <uint16_t> ());
        _packet.ip.frag_off = htons (IP_DF);
        _packet.ip.ttl      = IPDEFTTL;
        _packet.ip.check    = Raw::Socket::checksum (reinterpret_cast <uint16_t *> (&_packet.ip), sizeof (_packet.ip));

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
        ASSERT_EQ (this->bind (_interface), 0) << join::lastError.message ();
        ASSERT_EQ (ReactorThread::reactor ()->addHandler (this), 0) << join::lastError.message ();
    }

    /**
     * @brief Tears down the test fixture.
     */
    void TearDown () override
    {
        ASSERT_EQ (ReactorThread::reactor ()->delHandler (this), 0) << join::lastError.message ();
        this->close ();
    }

    /**
     * @brief method called when data are ready to be read on handle.
     */
    virtual void onReceive () override
    {
        auto buffer = std::make_unique <char []> (this->canRead ());
        if (buffer)
        {
            int nread = this->read (buffer.get (), this->canRead ());
            if (size_t (nread) < sizeof (Packet))
            {
                return;
            }

            Packet *data = reinterpret_cast <Packet *> (buffer.get ());
            if (data->ip.id != _packet.ip.id)
            {
                return;
            }

            this->write (buffer.get (), nread);
        }
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

    /// packet.
    static Packet _packet;

    /// hostname.
    static const std::string _interface;

    /// timeout.
    static const int _timeout;
};

RawSocket::Packet RawSocket::_packet;
const std::string RawSocket::_interface = "lo";
const int         RawSocket::_timeout = 1000;

/**
 * @brief Test open method.
 */
TEST_F (RawSocket, open)
{
    Raw::Socket rawSocket;

    ASSERT_EQ (rawSocket.open (), 0) << join::lastError.message ();
    ASSERT_EQ (rawSocket.open (), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    rawSocket.close ();
}

/**
 * @brief Test close method.
 */
TEST_F (RawSocket, close)
{
    Raw::Socket rawSocket (Raw::Socket::Blocking);

    ASSERT_FALSE (rawSocket.opened ());
    ASSERT_EQ (rawSocket.bind (_interface), 0) << join::lastError.message ();
    ASSERT_TRUE (rawSocket.opened ());
    rawSocket.close ();
    ASSERT_FALSE (rawSocket.opened ());
}

/**
 * @brief Test bind method.
 */
TEST_F (RawSocket, bind)
{
    Raw::Socket rawSocket (Raw::Socket::Blocking);

    ASSERT_EQ (rawSocket.bind (_interface), 0) << join::lastError.message ();
    rawSocket.close ();
}

/**
 * @brief Test canRead method.
 */
TEST_F (RawSocket, canRead)
{
    Raw::Socket rawSocket (Raw::Socket::Blocking);

    ASSERT_EQ (rawSocket.canRead (), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (rawSocket.bind (_interface), 0) << join::lastError.message ();
    ASSERT_TRUE (rawSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (rawSocket.write (reinterpret_cast <char* > (&_packet), sizeof (_packet)), sizeof (_packet)) << join::lastError.message ();
    ASSERT_TRUE (rawSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_GT (rawSocket.canRead (), 0) << join::lastError.message ();
    rawSocket.close ();
}

/**
 * @brief Test waitReadyRead method.
 */
TEST_F (RawSocket, waitReadyRead)
{
    Raw::Socket rawSocket;

    ASSERT_FALSE (rawSocket.waitReadyRead (_timeout));
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (rawSocket.bind (_interface), 0) << join::lastError.message ();
    ASSERT_TRUE (rawSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (rawSocket.write (reinterpret_cast <char* > (&_packet), sizeof (_packet)), sizeof (_packet)) << join::lastError.message ();
    ASSERT_TRUE (rawSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    rawSocket.close ();
}

/**
 * @brief Test read method.
 */
TEST_F (RawSocket, read)
{
    Raw::Socket rawSocket (Raw::Socket::Blocking);
    char data[1024];

    ASSERT_EQ (rawSocket.read (data, sizeof (data)), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (rawSocket.bind (_interface), 0) << join::lastError.message ();
    ASSERT_TRUE (rawSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (rawSocket.write (reinterpret_cast <char* > (&_packet), sizeof (_packet)), sizeof (_packet)) << join::lastError.message ();
    ASSERT_TRUE (rawSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_GT (rawSocket.read (data, sizeof (data)), 0) << join::lastError.message ();
    rawSocket.close ();
}

/**
 * @brief Test waitReadyWrite method.
 */
TEST_F (RawSocket, waitReadyWrite)
{
    Raw::Socket rawSocket;

    ASSERT_FALSE (rawSocket.waitReadyWrite (_timeout));
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (rawSocket.bind (_interface), 0) << join::lastError.message ();
    ASSERT_TRUE (rawSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    rawSocket.close ();
}

/**
 * @brief Test write method.
 */
TEST_F (RawSocket, write)
{
    Raw::Socket rawSocket;

    ASSERT_EQ (rawSocket.write (reinterpret_cast <char* > (&_packet), sizeof (_packet)), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (rawSocket.bind (_interface), 0) << join::lastError.message ();
    ASSERT_TRUE (rawSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (rawSocket.write (reinterpret_cast <char* > (&_packet), sizeof (_packet)), sizeof (_packet)) << join::lastError.message ();
    ASSERT_TRUE (rawSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    rawSocket.close ();
}

/**
 * @brief Test setMode method.
 */
TEST_F (RawSocket, setMode)
{
    Raw::Socket rawSocket;

    ASSERT_EQ (rawSocket.open (), 0) << join::lastError.message ();

    int flags = ::fcntl (rawSocket.handle (), F_GETFL, 0);
    ASSERT_TRUE (flags & O_NONBLOCK);

    rawSocket.setMode (Raw::Socket::Blocking);
    flags = ::fcntl (rawSocket.handle (), F_GETFL, 0);
    ASSERT_FALSE (flags & O_NONBLOCK);

    rawSocket.setMode (Raw::Socket::NonBlocking);
    flags = ::fcntl (rawSocket.handle (), F_GETFL, 0);
    ASSERT_TRUE (flags & O_NONBLOCK);

    rawSocket.close ();
}

/**
 * @brief Test setOption method.
 */
TEST_F (RawSocket, setOption)
{
    Raw::Socket rawSocket;

    ASSERT_EQ (rawSocket.setOption (Raw::Socket::RcvBuffer, 1500), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (rawSocket.open (), 0) << join::lastError.message ();
    ASSERT_EQ (rawSocket.setOption (Raw::Socket::NoDelay, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (rawSocket.setOption (Raw::Socket::KeepAlive, 1), 0) << join::lastError.message ();
    ASSERT_EQ (rawSocket.setOption (Raw::Socket::KeepIdle, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (rawSocket.setOption (Raw::Socket::KeepIntvl, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (rawSocket.setOption (Raw::Socket::KeepCount, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (rawSocket.setOption (Raw::Socket::SndBuffer, 1500), 0) << join::lastError.message ();
    ASSERT_EQ (rawSocket.setOption (Raw::Socket::RcvBuffer, 1500), 0) << join::lastError.message ();
    ASSERT_EQ (rawSocket.setOption (Raw::Socket::TimeStamp, 1), 0) << join::lastError.message ();
    ASSERT_EQ (rawSocket.setOption (Raw::Socket::ReuseAddr, 1), 0) << join::lastError.message ();
    ASSERT_EQ (rawSocket.setOption (Raw::Socket::ReusePort, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (rawSocket.setOption (Raw::Socket::Broadcast, 1), 0) << join::lastError.message ();
    ASSERT_EQ (rawSocket.setOption (Raw::Socket::Ttl, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (rawSocket.setOption (Raw::Socket::MulticastLoop, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (rawSocket.setOption (Raw::Socket::MulticastTtl, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (rawSocket.setOption (Raw::Socket::PathMtuDiscover, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (rawSocket.setOption (Raw::Socket::RcvError, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (rawSocket.setOption (Raw::Socket::AuxData, 1), 0) << join::lastError.message ();
    rawSocket.close ();
}

/**
 * @brief Test localEndpoint method.
 */
TEST_F (RawSocket, localEndpoint)
{
    Raw::Socket rawSocket;

    ASSERT_EQ (rawSocket.localEndpoint (), Raw::Endpoint {});
    ASSERT_EQ (rawSocket.bind (_interface), 0) << join::lastError.message ();
    ASSERT_EQ (rawSocket.localEndpoint ().device (), _interface);
}

/**
 * @brief Test opened method.
 */
TEST_F (RawSocket, opened)
{
    Raw::Socket rawSocket (Raw::Socket::Blocking);

    ASSERT_FALSE (rawSocket.opened ());
    ASSERT_EQ (rawSocket.open (), 0) << join::lastError.message ();
    ASSERT_TRUE (rawSocket.opened ());
    rawSocket.close ();
    ASSERT_FALSE (rawSocket.opened ());
}

/**
 * @brief Test encrypted method.
 */
TEST_F (RawSocket, encrypted)
{
    Raw::Socket rawSocket;

    ASSERT_FALSE (rawSocket.encrypted ());
    ASSERT_EQ (rawSocket.open (), 0) << join::lastError.message ();
    ASSERT_FALSE (rawSocket.encrypted ());
    rawSocket.close ();
    ASSERT_FALSE (rawSocket.encrypted ());
}

/**
 * @brief Test family method.
 */
TEST_F (RawSocket, family)
{
    Raw::Socket rawSocket;

    ASSERT_EQ (rawSocket.family (), AF_PACKET);
}

/**
 * @brief Test type method.
 */
TEST_F (RawSocket, type)
{
    Raw::Socket rawSocket;

    ASSERT_EQ (rawSocket.type (), SOCK_RAW);
}

/**
 * @brief Test protocol method.
 */
TEST_F (RawSocket, protocol)
{
    Raw::Socket rawSocket;

    ASSERT_EQ (rawSocket.protocol (), htons (ETH_P_ALL));
}

/**
 * @brief Test handle method.
 */
TEST_F (RawSocket, handle)
{
    Raw::Socket rawSocket (Raw::Socket::Blocking);

    ASSERT_EQ (rawSocket.handle (), -1);
    ASSERT_EQ (rawSocket.open (), 0) << join::lastError.message ();
    ASSERT_GT (rawSocket.handle (), -1);
    rawSocket.close ();
    ASSERT_EQ (rawSocket.handle (), -1);
}

/**
 * @brief Test checksum method.
 */
TEST_F (RawSocket, checksum)
{
    std::string buffer ({'\xD2', '\xB6', '\x69', '\xFD', '\x2E'});

    ASSERT_EQ (Raw::Socket::checksum (reinterpret_cast <uint16_t *> (&buffer[0]), buffer.size (), 0), 19349);
}

/**
 * @brief Test lower method.
 */
TEST_F (RawSocket, lower)
{
    Raw::Socket rawSocket1, rawSocket2;

    ASSERT_EQ (rawSocket1.open (), 0) << join::lastError.message ();
    ASSERT_EQ (rawSocket2.open (), 0) << join::lastError.message ();
    if (rawSocket1.handle () < rawSocket2.handle ())
    {
        ASSERT_TRUE (rawSocket1 < rawSocket2);
    }
    else
    {
        ASSERT_TRUE (rawSocket2 < rawSocket1);
    }
    rawSocket1.close ();
    rawSocket2.close ();
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
