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
#include <join/socket.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::net::MacAddress;
using join::net::IpAddress;
using join::net::Udp;

/**
 * @brief Class used to test the UDP socket API.
 */
class UdpSocket : public ::testing::Test, public Udp::Socket::Observer
{
protected:
    /**
     * @brief Sets up the test fixture.
     */
    void SetUp ()
    {
        ASSERT_EQ (bind ({Udp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
        ASSERT_EQ (start (), 0) << join::lastError.message ();
    }

    /**
     * @brief Tears down the test fixture.
     */
    void TearDown ()
    {
        ASSERT_EQ (stop (), 0) << join::lastError.message ();
        ASSERT_EQ (close (), 0) << join::lastError.message ();
    }

    /**
     * @brief method called on receive.
     */
    virtual void onReceive () override
    {
        auto buffer = std::make_unique <char []> (canRead ());
        if (buffer)
        {
            Udp::Endpoint from;
            int nread = readFrom (buffer.get (), canRead (), &from);
            if (nread > 0)
            {
                writeTo (buffer.get (), nread, from);
            }
        }
    }

    /// host.
    static const std::string host_;

    /// port.
    static const uint16_t port_;

    /// timeout.
    static const int timeout_;
};

const std::string UdpSocket::host_    = "localhost";
const uint16_t    UdpSocket::port_    = 5000;
const int         UdpSocket::timeout_ = 1000;

/**
 * @brief Test open method.
 */
TEST_F (UdpSocket, open)
{
    Udp::Socket udpSocket;

    ASSERT_EQ (udpSocket.open (Udp::v4 ()), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.close (), 0) << join::lastError.message ();

    ASSERT_EQ (udpSocket.open (Udp::v6 ()), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test close method.
 */
TEST_F (UdpSocket, close)
{
    Udp::Socket udpSocket (Udp::Socket::Blocking);

    ASSERT_FALSE (udpSocket.opened ());
    ASSERT_EQ (udpSocket.connect ({Udp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_TRUE (udpSocket.opened ());
    ASSERT_EQ (udpSocket.close (), 0) << join::lastError.message ();
    ASSERT_FALSE (udpSocket.opened ());
}

/**
 * @brief Test bind method.
 */
TEST_F (UdpSocket, bind)
{
    Udp::Socket udpSocket (Udp::Socket::Blocking);

    ASSERT_EQ (udpSocket.bind ({Udp::Resolver::resolveHost (host_), uint16_t (port_ + 1)}), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.connect ({Udp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test connect method.
 */
TEST_F (UdpSocket, connect)
{
    Udp::Socket udpSocket (Udp::Socket::Blocking);

    ASSERT_EQ (udpSocket.connect ({Udp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.close (), 0) << join::lastError.message ();

    ASSERT_EQ (udpSocket.connect (Udp::Resolver::resolve (host_ + ":" + std::to_string (port_))), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief disconnect method.
 */
TEST_F (UdpSocket, disconnect)
{
    Udp::Socket udpSocket (Udp::Socket::Blocking);

    ASSERT_FALSE (udpSocket.connected ());
    ASSERT_EQ (udpSocket.connect ({Udp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_TRUE (udpSocket.connected ());
    ASSERT_EQ (udpSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_FALSE (udpSocket.connected ());
    ASSERT_EQ (udpSocket.close (), 0) << join::lastError.message ();
    ASSERT_FALSE (udpSocket.connected());
}

/**
 * @brief Test canRead method.
 */
TEST_F (UdpSocket, canRead)
{
    Udp::Socket udpSocket (Udp::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (udpSocket.connect ({Udp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_TRUE (udpSocket.waitReadyWrite (timeout_)) << join::lastError.message ();
    ASSERT_EQ (udpSocket.write (data, sizeof (data)), sizeof (data)) << join::lastError.message ();
    ASSERT_TRUE (udpSocket.waitReadyRead (timeout_)) << join::lastError.message ();
    ASSERT_GT (udpSocket.canRead (), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test waitReadyRead method.
 */
TEST_F (UdpSocket, waitReadyRead)
{
    Udp::Socket udpSocket;
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (udpSocket.connect ({Udp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_TRUE (udpSocket.waitReadyWrite (timeout_)) << join::lastError.message ();
    ASSERT_EQ (udpSocket.write (data, sizeof (data)), sizeof (data)) << join::lastError.message ();
    ASSERT_TRUE (udpSocket.waitReadyRead (timeout_)) << join::lastError.message ();
    ASSERT_EQ (udpSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test read method.
 */
TEST_F (UdpSocket, read)
{
    Udp::Socket udpSocket (Udp::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (udpSocket.connect ({Udp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_TRUE (udpSocket.waitReadyWrite (timeout_)) << join::lastError.message ();
    ASSERT_EQ (udpSocket.write (data, sizeof (data)), sizeof (data)) << join::lastError.message ();
    ASSERT_TRUE (udpSocket.waitReadyRead (timeout_)) << join::lastError.message ();
    ASSERT_EQ (udpSocket.read (data, sizeof (data)), sizeof (data)) << join::lastError.message ();
    ASSERT_EQ (udpSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test readFrom method.
 */
TEST_F (UdpSocket, readFrom)
{
    Udp::Socket udpSocket (Udp::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};
    Udp::Endpoint from;

    ASSERT_EQ (udpSocket.connect ({Udp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_TRUE (udpSocket.waitReadyWrite (timeout_)) << join::lastError.message ();
    ASSERT_EQ (udpSocket.write (data, sizeof (data)), sizeof (data)) << join::lastError.message ();
    ASSERT_TRUE (udpSocket.waitReadyRead (timeout_)) << join::lastError.message ();
    ASSERT_EQ (udpSocket.readFrom (data, udpSocket.canRead (), &from), sizeof (data)) << join::lastError.message ();
    ASSERT_EQ (udpSocket.close (), 0) << join::lastError.message ();
    ASSERT_EQ (from, Udp::Endpoint (Udp::Resolver::resolveHost (host_), port_));
}

/**
 * @brief Test waitReadyWrite method.
 */
TEST_F (UdpSocket, waitReadyWrite)
{
    Udp::Socket udpSocket;

    ASSERT_EQ (udpSocket.connect ({Udp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_TRUE (udpSocket.waitReadyWrite (timeout_)) << join::lastError.message ();
    ASSERT_EQ (udpSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test write method.
 */
TEST_F (UdpSocket, write)
{
    Udp::Socket udpSocket (Udp::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (udpSocket.connect ({Udp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_TRUE (udpSocket.waitReadyWrite (timeout_)) << join::lastError.message ();
    ASSERT_EQ (udpSocket.write (data, sizeof (data)), sizeof (data)) << join::lastError.message ();
    ASSERT_TRUE (udpSocket.waitReadyRead (timeout_)) << join::lastError.message ();
    ASSERT_EQ (udpSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test writeTo method.
 */
TEST_F (UdpSocket, writeTo)
{
    Udp::Socket udpSocket (Udp::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (udpSocket.open (Udp::v4 ()), 0) << join::lastError.message ();
    ASSERT_TRUE (udpSocket.waitReadyWrite (timeout_)) << join::lastError.message ();
    ASSERT_EQ (udpSocket.writeTo (data, sizeof (data), {Udp::Resolver::resolveHost (host_), port_}), sizeof (data)) << join::lastError.message ();
    ASSERT_TRUE (udpSocket.waitReadyRead (timeout_));
    ASSERT_EQ (udpSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test setMode method.
 */
TEST_F (UdpSocket, setMode)
{
    Udp::Socket udpSocket (Udp::Socket::Blocking);

    ASSERT_EQ (udpSocket.connect ({Udp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.setMode (Udp::Socket::NonBlocking), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test setOption method.
 */
TEST_F (UdpSocket, setOption)
{
    Udp::Socket udpSocket (Udp::Socket::Blocking);

    ASSERT_EQ (udpSocket.setOption (Udp::Socket::MulticastLoop, 1), -1);
    ASSERT_EQ (udpSocket.connect ({Udp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.setOption (Udp::Socket::MulticastLoop, 1), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test localEndpoint method.
 */
TEST_F (UdpSocket, localEndpoint)
{
    Udp::Socket udpSocket (Udp::Socket::Blocking);

    ASSERT_EQ (udpSocket.bind ({Udp::Resolver::resolveHost (host_), uint16_t (port_ + 1)}), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.connect ({Udp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.localEndpoint (), Udp::Endpoint (Udp::Resolver::resolveHost (host_), uint16_t (port_ + 1))) << join::lastError.message ();
    ASSERT_EQ (udpSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test mtu method.
 */
TEST_F (UdpSocket, remoteEndpoint)
{
    Udp::Socket udpSocket (Udp::Socket::Blocking);

    ASSERT_EQ (udpSocket.bind ({Udp::Resolver::resolveHost (host_), uint16_t (port_ + 1)}), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.connect ({Udp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.remoteEndpoint (), Udp::Endpoint (Udp::Resolver::resolveHost (host_), port_)) << join::lastError.message ();
    ASSERT_EQ (udpSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test opened method.
 */
TEST_F (UdpSocket, opened)
{
    Udp::Socket udpSocket (Udp::Socket::Blocking);

    ASSERT_FALSE (udpSocket.opened ());
    ASSERT_EQ (udpSocket.open (Udp::v4 ()), 0) << join::lastError.message ();
    ASSERT_TRUE (udpSocket.opened ());
    ASSERT_EQ (udpSocket.connect ({Udp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_TRUE (udpSocket.opened ());
    ASSERT_EQ (udpSocket.close (), 0) << join::lastError.message ();
    ASSERT_FALSE (udpSocket.opened ());
}

/**
 * @brief Test connected method.
 */
TEST_F (UdpSocket, connected)
{
    Udp::Socket udpSocket (Udp::Socket::Blocking);

    ASSERT_FALSE (udpSocket.opened ());
    ASSERT_EQ (udpSocket.open (Udp::v4 ()), 0) << join::lastError.message ();
    ASSERT_FALSE (udpSocket.connected ());
    ASSERT_EQ (udpSocket.connect ({Udp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_TRUE (udpSocket.connected ());
    ASSERT_EQ (udpSocket.close (), 0) << join::lastError.message ();
    ASSERT_FALSE (udpSocket.connected ());
}

/**
 * @brief Test encrypted method.
 */
TEST_F (UdpSocket, encrypted)
{
    Udp::Socket udpSocket (Udp::Socket::Blocking);

    ASSERT_FALSE (udpSocket.opened ());
    ASSERT_EQ (udpSocket.open (Udp::v4 ()), 0) << join::lastError.message ();
    ASSERT_FALSE (udpSocket.encrypted ());
    ASSERT_EQ (udpSocket.connect ({Udp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_FALSE (udpSocket.encrypted ());
    ASSERT_EQ (udpSocket.close (), 0) << join::lastError.message ();
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
    ASSERT_EQ (udpSocket.close (), 0) << join::lastError.message ();

    ASSERT_EQ (udpSocket.open (Udp::v6 ()), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.family (), AF_INET6);
    ASSERT_EQ (udpSocket.close (), 0) << join::lastError.message ();
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
    ASSERT_EQ (udpSocket.close (), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket.handle (), -1);
}

/**
 * @brief Test mtu method.
 */
TEST_F (UdpSocket, mtu)
{
    Udp::Socket udpSocket (Udp::Socket::Blocking);

    ASSERT_EQ (udpSocket.connect ({Udp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_NE (udpSocket.mtu (), -1) << join::lastError.message ();
    ASSERT_EQ (udpSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test randomize method.
 */
TEST_F (UdpSocket, randomize)
{
    ASSERT_GT (Udp::Socket::randomize <int> (), 0);
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
    ASSERT_EQ (udpSocket1.close (), 0) << join::lastError.message ();
    ASSERT_EQ (udpSocket2.close (), 0) << join::lastError.message ();
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
