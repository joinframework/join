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

using join::Errc;
using join::MacAddress;
using join::IpAddress;
using join::Reactor;
using join::Udp;

/**
 * @brief Class used to test the UDP socket API.
 */
class UdpSocket : public Udp::Socket, public ::testing::Test
{
protected:
    /**
     * @brief Sets up the test fixture.
     */
    void SetUp () override
    {
        ASSERT_EQ (this->bind ({IpAddress::ipv6Wildcard, _port}), 0) << join::lastError.message ();
        ASSERT_EQ (Reactor::instance ()->addHandler (this), 0) << join::lastError.message ();
    }

    /**
     * @brief Tears down the test fixture.
     */
    void TearDown () override
    {
        ASSERT_EQ (Reactor::instance ()->delHandler (this, true), 0) << join::lastError.message ();
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
            Udp::Endpoint from;
            int nread = this->readFrom (buffer.get (), this->canRead (), &from);
            if (nread > 0)
            {
                this->writeTo (buffer.get (), nread, from);
            }
        }
    }

    /// host.
    static const std::string _host;

    /// port.
    static const uint16_t _port;

    /// timeout.
    static const int _timeout;
};

const std::string UdpSocket::_host = "127.0.0.1";
const uint16_t    UdpSocket::_port = 5000;
const int         UdpSocket::_timeout = 1000;

/**
 * @brief Test open method.
 */
TEST_F (UdpSocket, open)
{
    Udp::Socket udpSocket;

    ASSERT_EQ (udpSocket.open (Udp::v4 ()), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.open (Udp::v4 ()), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    udpSocket.close ();

    ASSERT_EQ (udpSocket.open (Udp::v6 ()), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.open (Udp::v6 ()), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    udpSocket.close ();
}

/**
 * @brief Test close method.
 */
TEST_F (UdpSocket, close)
{
    Udp::Socket udpSocket (Udp::Socket::Blocking);

    ASSERT_FALSE (udpSocket.opened ());
    ASSERT_EQ (udpSocket.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (udpSocket.opened ());
    udpSocket.close ();
    ASSERT_FALSE (udpSocket.opened ());
}

/**
 * @brief Test bind method.
 */
TEST_F (UdpSocket, bind)
{
    Udp::Socket udpSocket (Udp::Socket::Blocking);

    ASSERT_EQ (udpSocket.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.bind ({_host, uint16_t (_port + 1)}), -1);
    ASSERT_EQ (udpSocket.disconnect (), 0) << join::lastError.message ();

    ASSERT_EQ (udpSocket.bind ({_host, uint16_t (_port + 1)}), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.disconnect (), 0) << join::lastError.message ();

    udpSocket.close ();
}

/**
 * @brief Test bindToDevice method.
 */
TEST_F (UdpSocket, bindToDevice)
{
    Udp::Socket udpSocket (Udp::Socket::Blocking);

    ASSERT_EQ (udpSocket.bindToDevice ("lo"), -1);

    ASSERT_EQ (udpSocket.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.bindToDevice ("lo"), -1);
    ASSERT_EQ (udpSocket.disconnect (), 0) << join::lastError.message ();

    ASSERT_EQ (udpSocket.bindToDevice ("lo"), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.connect (_host), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.disconnect (), 0) << join::lastError.message ();

    ASSERT_EQ (udpSocket.bindToDevice ("foo"), -1);

    udpSocket.close ();
}

/**
 * @brief Test connect method.
 */
TEST_F (UdpSocket, connect)
{
    Udp::Socket udpSocket (Udp::Socket::Blocking);

    ASSERT_EQ (udpSocket.connect ({"255.255.255.255", _port}), -1);

    ASSERT_EQ (udpSocket.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.connect ({_host, _port}), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    udpSocket.close ();
}

/**
 * @brief disconnect method.
 */
TEST_F (UdpSocket, disconnect)
{
    Udp::Socket udpSocket (Udp::Socket::Blocking);

    ASSERT_FALSE (udpSocket.connected ());
    ASSERT_EQ (udpSocket.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (udpSocket.connected ());
    ASSERT_EQ (udpSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_FALSE (udpSocket.connected ());
    udpSocket.close ();
    ASSERT_FALSE (udpSocket.connected());
}

/**
 * @brief Test canRead method.
 */
TEST_F (UdpSocket, canRead)
{
    Udp::Socket udpSocket (Udp::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (udpSocket.canRead (), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (udpSocket.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (udpSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (udpSocket.write (data, sizeof (data)), sizeof (data)) << join::lastError.message ();
    ASSERT_TRUE (udpSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_GT (udpSocket.canRead (), 0) << join::lastError.message ();
    udpSocket.close ();
}

/**
 * @brief Test waitReadyRead method.
 */
TEST_F (UdpSocket, waitReadyRead)
{
    Udp::Socket udpSocket;
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_FALSE (udpSocket.waitReadyRead (_timeout));
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (udpSocket.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (udpSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (udpSocket.write (data, sizeof (data)), sizeof (data)) << join::lastError.message ();
    ASSERT_TRUE (udpSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    udpSocket.close ();
}

/**
 * @brief Test read method.
 */
TEST_F (UdpSocket, read)
{
    Udp::Socket udpSocket (Udp::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (udpSocket.read (data, sizeof (data)), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (udpSocket.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (udpSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (udpSocket.write (data, sizeof (data)), sizeof (data)) << join::lastError.message ();
    ASSERT_TRUE (udpSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_EQ (udpSocket.read (data, sizeof (data)), sizeof (data)) << join::lastError.message ();
    udpSocket.close ();
}

/**
 * @brief Test readFrom method.
 */
TEST_F (UdpSocket, readFrom)
{
    Udp::Socket udpSocket (Udp::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};
    Udp::Endpoint from;

    ASSERT_EQ (udpSocket.readFrom (data, sizeof (data)), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (udpSocket.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (udpSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (udpSocket.write (data, sizeof (data)), sizeof (data)) << join::lastError.message ();
    ASSERT_TRUE (udpSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_EQ (udpSocket.readFrom (data, udpSocket.canRead (), &from), sizeof (data)) << join::lastError.message ();
    udpSocket.close ();
    ASSERT_EQ (from, Udp::Endpoint (_host, _port));
}

/**
 * @brief Test waitReadyWrite method.
 */
TEST_F (UdpSocket, waitReadyWrite)
{
    Udp::Socket udpSocket;

    ASSERT_FALSE (udpSocket.waitReadyWrite (_timeout));
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (udpSocket.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (udpSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    udpSocket.close ();
}

/**
 * @brief Test write method.
 */
TEST_F (UdpSocket, write)
{
    Udp::Socket udpSocket (Udp::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (udpSocket.write (data, sizeof (data)), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (udpSocket.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (udpSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (udpSocket.write (data, sizeof (data)), sizeof (data)) << join::lastError.message ();
    ASSERT_TRUE (udpSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    udpSocket.close ();
}

/**
 * @brief Test writeTo method.
 */
TEST_F (UdpSocket, writeTo)
{
    Udp::Socket udpSocket (Udp::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (udpSocket.open (Udp::v6 ()), 0) << join::lastError.message ();
    ASSERT_TRUE (udpSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (udpSocket.writeTo (data, sizeof (data), {"255.255.255.255", _port}), -1);
    ASSERT_EQ (udpSocket.writeTo (data, sizeof (data), {_host, _port}), sizeof (data)) << join::lastError.message ();
    ASSERT_TRUE (udpSocket.waitReadyRead (_timeout));
    udpSocket.close ();
}

/**
 * @brief Test setMode method.
 */
TEST_F (UdpSocket, setMode)
{
    Udp::Socket udpSocket;

    ASSERT_EQ (udpSocket.open (), 0) << join::lastError.message ();

    int flags = ::fcntl (udpSocket.handle (), F_GETFL, 0);
    ASSERT_TRUE (flags & O_NONBLOCK);

    udpSocket.setMode (Udp::Socket::Blocking);
    flags = ::fcntl (udpSocket.handle (), F_GETFL, 0);
    ASSERT_FALSE (flags & O_NONBLOCK);

    udpSocket.setMode (Udp::Socket::NonBlocking);
    flags = ::fcntl (udpSocket.handle (), F_GETFL, 0);
    ASSERT_TRUE (flags & O_NONBLOCK);

    udpSocket.close ();
}

/**
 * @brief Test setOption method.
 */
TEST_F (UdpSocket, setOption)
{
    Udp::Socket udpSocket;

    ASSERT_EQ (udpSocket.setOption (Udp::Socket::RcvBuffer, 1500), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (udpSocket.open (), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.setOption (Udp::Socket::NoDelay, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (udpSocket.setOption (Udp::Socket::KeepAlive, 1), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.setOption (Udp::Socket::KeepIdle, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (udpSocket.setOption (Udp::Socket::KeepIntvl, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (udpSocket.setOption (Udp::Socket::KeepCount, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (udpSocket.setOption (Udp::Socket::SndBuffer, 1500), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.setOption (Udp::Socket::RcvBuffer, 1500), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.setOption (Udp::Socket::TimeStamp, 1), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.setOption (Udp::Socket::ReuseAddr, 1), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.setOption (Udp::Socket::ReusePort, 1), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.setOption (Udp::Socket::Broadcast, 1), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.setOption (Udp::Socket::Ttl, 1), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.setOption (Udp::Socket::MulticastLoop, 1), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.setOption (Udp::Socket::MulticastTtl, 1), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.setOption (Udp::Socket::PathMtuDiscover, 1), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.setOption (Udp::Socket::RcvError, 1), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.setOption (Udp::Socket::AuxData, 1), -1);
    ASSERT_EQ (join::lastError, std::errc::no_protocol_option);
    udpSocket.close ();

    ASSERT_EQ (udpSocket.open (Udp::v6 ()), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.setOption (Udp::Socket::NoDelay, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (udpSocket.setOption (Udp::Socket::KeepAlive, 1), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.setOption (Udp::Socket::KeepIdle, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (udpSocket.setOption (Udp::Socket::KeepIntvl, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (udpSocket.setOption (Udp::Socket::KeepCount, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (udpSocket.setOption (Udp::Socket::SndBuffer, 1500), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.setOption (Udp::Socket::RcvBuffer, 1500), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.setOption (Udp::Socket::TimeStamp, 1), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.setOption (Udp::Socket::ReuseAddr, 1), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.setOption (Udp::Socket::ReusePort, 1), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.setOption (Udp::Socket::Broadcast, 1), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.setOption (Udp::Socket::Ttl, 1), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.setOption (Udp::Socket::MulticastLoop, 1), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.setOption (Udp::Socket::MulticastTtl, 1), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.setOption (Udp::Socket::PathMtuDiscover, 1), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.setOption (Udp::Socket::RcvError, 1), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.setOption (Udp::Socket::AuxData, 1), -1);
    ASSERT_EQ (join::lastError, std::errc::no_protocol_option);
    udpSocket.close ();
}

/**
 * @brief Test localEndpoint method.
 */
TEST_F (UdpSocket, localEndpoint)
{
    Udp::Socket udpSocket (Udp::Socket::Blocking);

    ASSERT_EQ (udpSocket.localEndpoint (), Udp::Endpoint {});
    ASSERT_EQ (udpSocket.bind ({_host, uint16_t (_port + 1)}), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.localEndpoint (), Udp::Endpoint (_host, uint16_t (_port + 1))) << join::lastError.message ();
    udpSocket.close ();
}

/**
 * @brief Test remoteEndpoint method.
 */
TEST_F (UdpSocket, remoteEndpoint)
{
    Udp::Socket udpSocket (Udp::Socket::Blocking);

    ASSERT_EQ (udpSocket.remoteEndpoint (), Udp::Endpoint {});
    ASSERT_EQ (udpSocket.bind ({_host, uint16_t (_port + 1)}), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.remoteEndpoint (), Udp::Endpoint (_host, _port)) << join::lastError.message ();
    udpSocket.close ();
}

/**
 * @brief Test opened method.
 */
TEST_F (UdpSocket, opened)
{
    Udp::Socket udpSocket (Udp::Socket::Blocking);

    ASSERT_FALSE (udpSocket.opened ());
    ASSERT_EQ (udpSocket.open (Udp::v6 ()), 0) << join::lastError.message ();
    ASSERT_TRUE (udpSocket.opened ());
    ASSERT_EQ (udpSocket.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (udpSocket.opened ());
    udpSocket.close ();
    ASSERT_FALSE (udpSocket.opened ());
}

/**
 * @brief Test connected method.
 */
TEST_F (UdpSocket, connected)
{
    Udp::Socket udpSocket (Udp::Socket::Blocking);

    ASSERT_FALSE (udpSocket.opened ());
    ASSERT_EQ (udpSocket.open (Udp::v6 ()), 0) << join::lastError.message ();
    ASSERT_FALSE (udpSocket.connected ());
    ASSERT_EQ (udpSocket.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (udpSocket.connected ());
    udpSocket.close ();
    ASSERT_FALSE (udpSocket.connected ());
}

/**
 * @brief Test encrypted method.
 */
TEST_F (UdpSocket, encrypted)
{
    Udp::Socket udpSocket (Udp::Socket::Blocking);

    ASSERT_FALSE (udpSocket.opened ());
    ASSERT_EQ (udpSocket.open (Udp::v6 ()), 0) << join::lastError.message ();
    ASSERT_FALSE (udpSocket.encrypted ());
    ASSERT_EQ (udpSocket.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_FALSE (udpSocket.encrypted ());
    udpSocket.close ();
    ASSERT_FALSE (udpSocket.encrypted ());
}

/**
 * @brief Test family method.
 */
TEST_F (UdpSocket, family)
{
    Udp::Socket udpSocket;

    ASSERT_EQ (udpSocket.family (), AF_INET);

    ASSERT_EQ (udpSocket.open (Udp::v4 ()), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.family (), AF_INET);
    udpSocket.close ();

    ASSERT_EQ (udpSocket.open (Udp::v6 ()), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.family (), AF_INET6);
    udpSocket.close ();
}

/**
 * @brief Test type method.
 */
TEST_F (UdpSocket, type)
{
    Udp::Socket udpSocket;

    ASSERT_EQ (udpSocket.type (), SOCK_DGRAM);
}

/**
 * @brief Test protocol method.
 */
TEST_F (UdpSocket, protocol)
{
    Udp::Socket udpSocket;

    ASSERT_EQ (udpSocket.protocol (), IPPROTO_UDP);
}

/**
 * @brief Test handle method.
 */
TEST_F (UdpSocket, handle)
{
    Udp::Socket udpSocket (Udp::Socket::Blocking);

    ASSERT_EQ (udpSocket.handle (), -1);
    ASSERT_EQ (udpSocket.open (), 0) << join::lastError.message ();
    ASSERT_GT (udpSocket.handle (), -1);
    udpSocket.close ();
    ASSERT_EQ (udpSocket.handle (), -1);
}

/**
 * @brief Test mtu method.
 */
TEST_F (UdpSocket, mtu)
{
    Udp::Socket udpSocket (Udp::Socket::Blocking);

    ASSERT_EQ (udpSocket.mtu (), -1);
    ASSERT_EQ (udpSocket.connect ({"127.0.0.1", _port}), 0) << join::lastError.message ();
    ASSERT_NE (udpSocket.mtu (), -1) << join::lastError.message ();
    udpSocket.close ();

    ASSERT_EQ (udpSocket.mtu (), -1);
    ASSERT_EQ (udpSocket.connect ({"::1", _port}), 0) << join::lastError.message ();
    ASSERT_NE (udpSocket.mtu (), -1) << join::lastError.message ();
    udpSocket.close ();
}

/**
 * @brief Test checksum method.
 */
TEST_F (UdpSocket, checksum)
{
    std::string buffer ({'\xD2', '\xB6', '\x69', '\xFD', '\x2E'});

    ASSERT_EQ (Udp::Socket::checksum (reinterpret_cast <uint16_t *> (&buffer[0]), buffer.size (), 0), 19349);
}

/**
 * @brief Test is lower method.
 */
TEST_F (UdpSocket, lower)
{
    Udp::Socket udpSocket1, udpSocket2;

    ASSERT_EQ (udpSocket1.open (Udp::v4 ()), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket2.open (Udp::v4 ()), 0) << join::lastError.message ();
    if (udpSocket1.handle () < udpSocket2.handle ())
    {
        ASSERT_TRUE (udpSocket1 < udpSocket2);
    }
    else
    {
        ASSERT_TRUE (udpSocket2 < udpSocket1);
    }
    udpSocket1.close ();
    udpSocket2.close ();
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
