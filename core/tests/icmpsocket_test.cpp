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

using join::Errc;
using join::MacAddress;
using join::IpAddress;
using join::Icmp;

/**
 * @brief Class used to test the ICMP socket API.
 */
class IcmpSocket : public ::testing::Test
{
public:
    /**
     * @brief set up test case.
     */
    static void SetUpTestCase ()
    {
        _data = std::make_unique <char[]> (sizeof (struct icmphdr));

        struct icmphdr *icmp = reinterpret_cast <struct icmphdr *> (_data.get ());
        icmp->type = ICMP_ECHO;
        icmp->code = 0;
        icmp->checksum = 0;
        icmp->un.echo.sequence = htons (1);
        icmp->un.echo.id = htons (getpid () & 0xFFFF);
        icmp->checksum = Icmp::Socket::checksum (reinterpret_cast <uint16_t *> (icmp), sizeof (struct icmphdr), 0);
    }

protected:
    /// hostname.
    static const std::string _host;

    /// timeout.
    static const int _timeout;

    /// data.
    static std::unique_ptr <char[]> _data;
};

const std::string        IcmpSocket::_host = "127.0.0.1";
const int                IcmpSocket::_timeout = 1000;
std::unique_ptr <char[]> IcmpSocket::_data;

/**
 * @brief Test open method.
 */
TEST_F (IcmpSocket, open)
{
    Icmp::Socket icmpSocket;

    ASSERT_EQ (icmpSocket.open (Icmp::v4 ()), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.open (Icmp::v4 ()), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    icmpSocket.close ();

    ASSERT_EQ (icmpSocket.open (Icmp::v6 ()), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.open (Icmp::v6 ()), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    icmpSocket.close ();
}

/**
 * @brief Test close method.
 */
TEST_F (IcmpSocket, close)
{
    Icmp::Socket icmpSocket (Icmp::Socket::Blocking);

    ASSERT_FALSE (icmpSocket.opened ());
    ASSERT_EQ (icmpSocket.connect (_host), 0) << join::lastError.message ();
    ASSERT_TRUE (icmpSocket.opened ());
    icmpSocket.close ();
    ASSERT_FALSE (icmpSocket.opened ());
}

/**
 * @brief Test bind method.
 */
TEST_F (IcmpSocket, bind)
{
    Icmp::Socket icmpSocket (Icmp::Socket::Blocking);

    ASSERT_EQ (icmpSocket.connect (_host), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.bind (_host), -1);
    ASSERT_EQ (icmpSocket.disconnect (), 0) << join::lastError.message ();

    ASSERT_EQ (icmpSocket.bind (_host), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.connect (_host), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.disconnect (), 0) << join::lastError.message ();

    icmpSocket.close ();
}

/**
 * @brief Test bindToDevice method.
 */
TEST_F (IcmpSocket, bindToDevice)
{
    Icmp::Socket icmpSocket (Icmp::Socket::Blocking);

    ASSERT_EQ (icmpSocket.bindToDevice ("lo"), -1);

    ASSERT_EQ (icmpSocket.connect (_host), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.bindToDevice ("lo"), -1);
    ASSERT_EQ (icmpSocket.disconnect (), 0) << join::lastError.message ();

    ASSERT_EQ (icmpSocket.bindToDevice ("lo"), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.connect (_host), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.disconnect (), 0) << join::lastError.message ();

    ASSERT_EQ (icmpSocket.bindToDevice ("foo"), -1);

    icmpSocket.close ();
}

/**
 * @brief Test connect method.
 */
TEST_F (IcmpSocket, connect)
{
    Icmp::Socket icmpSocket (Icmp::Socket::Blocking);

    ASSERT_EQ (icmpSocket.connect ("255.255.255.255"), -1);

    ASSERT_EQ (icmpSocket.connect (_host), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.connect (_host), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    icmpSocket.close ();
}

/**
 * @brief disconnect method.
 */
TEST_F (IcmpSocket, disconnect)
{
    Icmp::Socket icmpSocket (Icmp::Socket::Blocking);

    ASSERT_FALSE (icmpSocket.connected ());
    ASSERT_EQ (icmpSocket.connect (_host), 0) << join::lastError.message ();
    ASSERT_TRUE (icmpSocket.connected ());
    ASSERT_EQ (icmpSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_FALSE (icmpSocket.connected ());
    icmpSocket.close ();
    ASSERT_FALSE (icmpSocket.connected ());
}

/**
 * @brief Test canRead method.
 */
TEST_F (IcmpSocket, canRead)
{
    Icmp::Socket client (Icmp::Socket::Blocking), server;
    Icmp::Endpoint from;
    char response[1024];

    ASSERT_EQ (client.canRead (), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (server.bind (_host), 0) << join::lastError.message ();
    ASSERT_EQ (client.connect (_host), 0) << join::lastError.message ();
    ASSERT_TRUE (client.waitReadyWrite (_timeout));
    ASSERT_GT (client.write (_data.get (), sizeof (struct icmphdr)), 0) << join::lastError.message ();
    ASSERT_TRUE (client.waitReadyWrite (_timeout));
    ASSERT_GT (client.write (_data.get (), sizeof (struct icmphdr)), 0) << join::lastError.message ();
    ASSERT_TRUE (server.waitReadyRead (_timeout));
    ASSERT_NE (server.canRead (), -1) << join::lastError.message ();
    ASSERT_NE (server.readFrom (response, server.canRead (), &from), -1) << join::lastError.message ();
    ASSERT_TRUE (server.waitReadyRead (_timeout));
    ASSERT_NE (server.canRead (), -1) << join::lastError.message ();
    ASSERT_NE (server.readFrom (response, server.canRead (), &from), -1) << join::lastError.message ();
    client.close ();
    server.close ();
}

/**
 * @brief Test waitReadyRead method.
 */
TEST_F (IcmpSocket, waitReadyRead)
{
    Icmp::Socket client (Icmp::Socket::NonBlocking), server;
    Icmp::Endpoint from;
    char response[1024];

    ASSERT_FALSE (client.waitReadyRead (_timeout));
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (server.bind (_host), 0) << join::lastError.message ();
    if (client.connect (_host) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (client.waitReadyWrite (_timeout));
    ASSERT_GT (client.write (_data.get (), sizeof (struct icmphdr)), 0) << join::lastError.message ();
    ASSERT_TRUE (server.waitReadyRead (_timeout));
    ASSERT_NE (server.canRead (), -1) << join::lastError.message ();
    ASSERT_NE (server.readFrom (response, server.canRead (), &from), -1) << join::lastError.message ();
    ASSERT_EQ (from, Icmp::Endpoint (_host));
    ASSERT_NE (server.writeTo (_data.get (), sizeof (struct icmphdr), from), -1) << join::lastError.message ();
    ASSERT_TRUE (client.waitReadyRead (_timeout));
    client.close ();
    server.close ();
}

/**
 * @brief Test read method.
 */
TEST_F (IcmpSocket, read)
{
    Icmp::Socket client, server;
    Icmp::Endpoint from;
    char response[1024];

    ASSERT_EQ (client.read (response, sizeof (response)), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (server.bind (_host), 0) << join::lastError.message ();
    ASSERT_EQ (client.connect (_host), 0) << join::lastError.message ();
    ASSERT_TRUE (client.waitReadyWrite (_timeout));
    ASSERT_GT (client.write (_data.get (), sizeof (struct icmphdr)), 0) << join::lastError.message ();
    ASSERT_TRUE (server.waitReadyRead (_timeout));
    ASSERT_NE (server.canRead (), -1) << join::lastError.message ();
    ASSERT_NE (server.readFrom (response, server.canRead (), &from), -1) << join::lastError.message ();
    ASSERT_EQ (from, Icmp::Endpoint (_host));
    ASSERT_NE (server.writeTo (_data.get (), sizeof (struct icmphdr), from), -1) << join::lastError.message ();
    ASSERT_TRUE (client.waitReadyRead (_timeout));
    ASSERT_NE (client.canRead (), -1) << join::lastError.message ();
    ASSERT_NE (client.read (response, client.canRead ()), -1) << join::lastError.message ();
    client.close ();
    server.close ();
}

/**
 * @brief Test readFrom method.
 */
TEST_F (IcmpSocket, readFrom)
{
    Icmp::Socket client, server;
    Icmp::Endpoint from;
    char response[1024];

    ASSERT_EQ (client.readFrom (response, sizeof (response), &from), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (server.bind (_host), 0) << join::lastError.message ();
    ASSERT_EQ (client.connect (_host), 0) << join::lastError.message ();
    ASSERT_TRUE (client.waitReadyWrite (_timeout));
    ASSERT_GT (client.write (_data.get (), sizeof (struct icmphdr)), 0) << join::lastError.message ();
    ASSERT_TRUE (server.waitReadyRead (_timeout));
    ASSERT_NE (server.canRead (), -1) << join::lastError.message ();
    ASSERT_NE (server.readFrom (response, server.canRead (), &from), -1) << join::lastError.message ();
    ASSERT_EQ (from, Icmp::Endpoint (_host));
    client.close ();
    server.close ();
}

/**
 * @brief Test waitReadyWrite method.
 */
TEST_F (IcmpSocket, waitReadyWrite)
{
    Icmp::Socket client (Icmp::Socket::NonBlocking), server;
    Icmp::Endpoint from;
    char response[1024];

    ASSERT_FALSE (client.waitReadyWrite (_timeout));
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (server.bind (_host), 0) << join::lastError.message ();
    ASSERT_EQ (client.connect (_host), 0) << join::lastError.message ();
    ASSERT_TRUE (client.waitReadyWrite (_timeout));
    ASSERT_GT (client.write (_data.get (), sizeof (struct icmphdr)), 0) << join::lastError.message ();
    ASSERT_TRUE (server.waitReadyRead (_timeout));
    ASSERT_NE (server.canRead (), -1) << join::lastError.message ();
    ASSERT_NE (server.readFrom (response, server.canRead (), &from), -1) << join::lastError.message ();
    ASSERT_EQ (from, Icmp::Endpoint (_host));
    client.close ();
    server.close ();
}

/**
 * @brief Test write method.
 */
TEST_F (IcmpSocket, write)
{
    Icmp::Socket client (Icmp::Socket::Blocking), server;
    Icmp::Endpoint from;
    char response[1024];

    ASSERT_EQ (client.write (_data.get (), sizeof (struct icmphdr)), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (server.bind (_host), 0) << join::lastError.message ();
    ASSERT_EQ (client.connect (_host), 0) << join::lastError.message ();
    ASSERT_GT (client.write (_data.get (), sizeof (struct icmphdr)), 0) << join::lastError.message ();
    ASSERT_TRUE (server.waitReadyRead (_timeout));
    ASSERT_NE (server.canRead (), -1) << join::lastError.message ();
    ASSERT_NE (server.readFrom (response, server.canRead (), &from), -1) << join::lastError.message ();
    ASSERT_EQ (from, Icmp::Endpoint (_host));
    client.close ();
    server.close ();
}

/**
 * @brief Test writeTo method.
 */
TEST_F (IcmpSocket, writeTo)
{
    Icmp::Socket client (Icmp::Socket::Blocking), server;
    Icmp::Endpoint from;
    char response[1024];

    ASSERT_EQ (server.bind (_host), 0) << join::lastError.message ();
    ASSERT_NE (client.writeTo (_data.get (), sizeof (struct icmphdr), _host), -1) << join::lastError.message ();
    ASSERT_TRUE (server.waitReadyRead (_timeout));
    ASSERT_NE (server.canRead (), -1) << join::lastError.message ();
    ASSERT_NE (server.readFrom (response, server.canRead (), &from), -1) << join::lastError.message ();
    ASSERT_EQ (from, Icmp::Endpoint (_host));
    ASSERT_EQ (client.writeTo (_data.get (), sizeof (struct icmphdr), "255.255.255.255"), -1);
    client.close ();
    server.close ();
}

/**
 * @brief Test setMode method.
 */
TEST_F (IcmpSocket, setMode)
{
    Icmp::Socket icmpSocket;

    ASSERT_EQ (icmpSocket.open (), 0) << join::lastError.message ();

    int flags = ::fcntl (icmpSocket.handle (), F_GETFL, 0);
    ASSERT_TRUE (flags & O_NONBLOCK);

    icmpSocket.setMode (Icmp::Socket::Blocking);
    flags = ::fcntl (icmpSocket.handle (), F_GETFL, 0);
    ASSERT_FALSE (flags & O_NONBLOCK);

    icmpSocket.setMode (Icmp::Socket::NonBlocking);
    flags = ::fcntl (icmpSocket.handle (), F_GETFL, 0);
    ASSERT_TRUE (flags & O_NONBLOCK);

    icmpSocket.close ();
}

/**
 * @brief Test setOption method.
 */
TEST_F (IcmpSocket, setOption)
{
    Icmp::Socket icmpSocket;

    ASSERT_EQ (icmpSocket.setOption (Icmp::Socket::RcvBuffer, 1500), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (icmpSocket.open (), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.setOption (Icmp::Socket::NoDelay, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (icmpSocket.setOption (Icmp::Socket::KeepAlive, 1), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.setOption (Icmp::Socket::KeepIdle, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (icmpSocket.setOption (Icmp::Socket::KeepIntvl, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (icmpSocket.setOption (Icmp::Socket::KeepCount, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (icmpSocket.setOption (Icmp::Socket::SndBuffer, 1500), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.setOption (Icmp::Socket::RcvBuffer, 1500), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.setOption (Icmp::Socket::TimeStamp, 1), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.setOption (Icmp::Socket::ReuseAddr, 1), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.setOption (Icmp::Socket::ReusePort, 1), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.setOption (Icmp::Socket::Broadcast, 1), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.setOption (Icmp::Socket::Ttl, 1), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.setOption (Icmp::Socket::MulticastLoop, 1), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.setOption (Icmp::Socket::MulticastTtl, 1), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.setOption (Icmp::Socket::PathMtuDiscover, 1), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.setOption (Icmp::Socket::RcvError, 1), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.setOption (Icmp::Socket::AuxData, 1), -1);
    ASSERT_EQ (join::lastError, std::errc::no_protocol_option);
    icmpSocket.close ();

    ASSERT_EQ (icmpSocket.open (Icmp::v6 ()), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.setOption (Icmp::Socket::NoDelay, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (icmpSocket.setOption (Icmp::Socket::KeepAlive, 1), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.setOption (Icmp::Socket::KeepIdle, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (icmpSocket.setOption (Icmp::Socket::KeepIntvl, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (icmpSocket.setOption (Icmp::Socket::KeepCount, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (icmpSocket.setOption (Icmp::Socket::SndBuffer, 1500), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.setOption (Icmp::Socket::RcvBuffer, 1500), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.setOption (Icmp::Socket::TimeStamp, 1), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.setOption (Icmp::Socket::ReuseAddr, 1), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.setOption (Icmp::Socket::ReusePort, 1), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.setOption (Icmp::Socket::Broadcast, 1), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.setOption (Icmp::Socket::Ttl, 1), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.setOption (Icmp::Socket::MulticastLoop, 1), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.setOption (Icmp::Socket::MulticastTtl, 1), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.setOption (Icmp::Socket::PathMtuDiscover, 1), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.setOption (Icmp::Socket::RcvError, 1), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.setOption (Icmp::Socket::AuxData, 1), -1);
    ASSERT_EQ (join::lastError, std::errc::no_protocol_option);
    icmpSocket.close ();
}

/**
 * @brief Test localEndpoint method.
 */
TEST_F (IcmpSocket, localEndpoint)
{
    Icmp::Socket icmpSocket (Icmp::Socket::Blocking);

    ASSERT_EQ (icmpSocket.localEndpoint (), Icmp::Endpoint {});
    ASSERT_EQ (icmpSocket.bind (_host), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.connect (_host), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.localEndpoint ().ip (), IpAddress (_host)) << join::lastError.message ();
    icmpSocket.close ();
}

/**
 * @brief Test opened method.
 */
TEST_F (IcmpSocket, opened)
{
    Icmp::Socket icmpSocket (Icmp::Socket::Blocking);

    ASSERT_FALSE (icmpSocket.opened ());
    ASSERT_EQ (icmpSocket.open (Icmp::v4 ()), 0) << join::lastError.message ();
    ASSERT_TRUE (icmpSocket.opened ());
    icmpSocket.close ();
    ASSERT_FALSE (icmpSocket.opened ());
}

/**
 * @brief Test connected method.
 */
TEST_F (IcmpSocket, connected)
{
    Icmp::Socket icmpSocket (Icmp::Socket::Blocking);

    ASSERT_FALSE (icmpSocket.connected ());
    ASSERT_EQ (icmpSocket.connect (_host), 0) << join::lastError.message ();
    ASSERT_TRUE (icmpSocket.connected());
    icmpSocket.close ();
    ASSERT_FALSE (icmpSocket.connected ());
}

/**
 * @brief Test encrypted method.
 */
TEST_F (IcmpSocket, encrypted)
{
    Icmp::Socket icmpSocket (Icmp::Socket::Blocking);

    ASSERT_FALSE (icmpSocket.encrypted ());
    ASSERT_EQ (icmpSocket.connect (_host), 0) << join::lastError.message ();
    ASSERT_FALSE (icmpSocket.encrypted ());
    icmpSocket.close ();
    ASSERT_FALSE (icmpSocket.encrypted ());
}

/**
 * @brief Test family method.
 */
TEST_F (IcmpSocket, family)
{
    Icmp::Socket icmpSocket;

    ASSERT_EQ (icmpSocket.family (), AF_INET);

    ASSERT_EQ (icmpSocket.bind (IpAddress (AF_INET6)), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.family (), AF_INET6);
    icmpSocket.close ();

    ASSERT_EQ (icmpSocket.bind (IpAddress (AF_INET)), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.family (), AF_INET);
    icmpSocket.close ();
}

/**
 * @brief Test type method.
 */
TEST_F (IcmpSocket, type)
{
    Icmp::Socket icmpSocket (Icmp::Socket::Blocking);

    ASSERT_EQ (icmpSocket.type (), SOCK_RAW);
}

/**
 * @brief Test protocol method.
 */
TEST_F (IcmpSocket, protocol)
{
    Icmp::Socket icmpSocket (Icmp::Socket::Blocking);

    ASSERT_EQ (icmpSocket.bind (IpAddress (AF_INET6)), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.protocol (), IPPROTO_ICMPV6);
    icmpSocket.close ();

    ASSERT_EQ (icmpSocket.bind (IpAddress (AF_INET)), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.protocol (), IPPROTO_ICMP);
    icmpSocket.close ();
}

/**
 * @brief Test handle method.
 */
TEST_F (IcmpSocket, handle)
{
    Icmp::Socket icmpSocket (Icmp::Socket::Blocking);

    ASSERT_EQ (icmpSocket.handle (), -1);
    ASSERT_EQ (icmpSocket.open (), 0) << join::lastError.message ();
    ASSERT_GT (icmpSocket.handle (), -1);
    icmpSocket.close ();
    ASSERT_EQ (icmpSocket.handle (), -1);
}

/**
 * @brief Test mtu method.
 */
TEST_F (IcmpSocket, mtu)
{
    Icmp::Socket icmpSocket (Icmp::Socket::Blocking);

    ASSERT_EQ (icmpSocket.mtu (), -1);
    ASSERT_EQ (icmpSocket.connect (_host), 0) << join::lastError.message ();
    ASSERT_NE (icmpSocket.mtu (), -1) << join::lastError.message ();
    icmpSocket.close ();
    ASSERT_EQ (icmpSocket.mtu (), -1);
}

/**
 * @brief Test ttl method.
 */
TEST_F (IcmpSocket, ttl)
{
    Icmp::Socket icmpSocket (Icmp::Socket::Blocking);

    ASSERT_EQ (icmpSocket.ttl (), 60);
}

/**
 * @brief Test checksum method.
 */
TEST_F (IcmpSocket, checksum)
{
    std::string buffer ({'\xD2', '\xB6', '\x69', '\xFD', '\x2E'});

    ASSERT_EQ (Icmp::Socket::checksum (reinterpret_cast <uint16_t *> (&buffer[0]), buffer.size (), 0), 19349);
}

/**
 * @brief Test is lower method.
 */
TEST_F (IcmpSocket, lower)
{
    Icmp::Socket icmpSocket1, icmpSocket2;

    ASSERT_EQ (icmpSocket1.open (Icmp::v4 ()), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket2.open (Icmp::v4 ()), 0) << join::lastError.message ();
    if (icmpSocket1.handle () < icmpSocket2.handle ())
    {
        ASSERT_TRUE (icmpSocket1 < icmpSocket2);
    }
    else
    {
        ASSERT_TRUE (icmpSocket2 < icmpSocket1);
    }
    icmpSocket1.close ();
    icmpSocket2.close ();
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
