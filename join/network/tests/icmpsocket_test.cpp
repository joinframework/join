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
using join::net::MacAddress;
using join::net::IpAddress;
using join::net::Icmp;

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
        data_ = std::make_unique <char[]> (sizeof (struct icmphdr));

        struct icmphdr *icmp = reinterpret_cast <struct icmphdr *> (data_.get ());
        icmp->type = ICMP_ECHO;
        icmp->code = 0;
        icmp->checksum = 0;
        icmp->un.echo.sequence = htons (1);
        icmp->un.echo.id = htons (getpid () & 0xFFFF);
        icmp->checksum = Icmp::Socket::checksum (reinterpret_cast <uint16_t *> (icmp), sizeof (struct icmphdr), 0);
    }

protected:
    /// hostname.
    static const std::string host_;

    /// timeout.
    static const int timeout_;

    /// data.
    static std::unique_ptr <char[]> data_;
};

const std::string        IcmpSocket::host_    = "localhost";
const int                IcmpSocket::timeout_ = 1000;
std::unique_ptr <char[]> IcmpSocket::data_;

/**
 * @brief Test open method.
 */
TEST_F (IcmpSocket, open)
{
    Icmp::Socket icmpSocket;

    ASSERT_EQ (icmpSocket.open (Icmp::v4 ()), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.close (), 0) << join::lastError.message ();

    ASSERT_EQ (icmpSocket.open (Icmp::v6 ()), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test close method.
 */
TEST_F (IcmpSocket, close)
{
    Icmp::Socket icmpSocket (Icmp::Socket::Blocking);

    ASSERT_EQ (icmpSocket.connect (Icmp::Resolver::resolve (host_)), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test bind method.
 */
TEST_F (IcmpSocket, bind)
{
    Icmp::Socket client (Icmp::Socket::Blocking), server;
    Icmp::Endpoint from;
    char response[1024];

    ASSERT_EQ (server.bind (Icmp::Resolver::resolve (host_)), 0) << join::lastError.message ();
    ASSERT_EQ (client.bind (Icmp::Resolver::resolve (host_)), 0) << join::lastError.message ();
    ASSERT_EQ (client.connect (Icmp::Resolver::resolve (host_)), 0) << join::lastError.message ();
    ASSERT_TRUE (client.waitReadyWrite (timeout_));
    ASSERT_GT (client.write (data_.get (), sizeof (struct icmphdr)), 0) << join::lastError.message ();
    ASSERT_TRUE (server.waitReadyRead (timeout_));
    ASSERT_NE (server.canRead (), -1) << join::lastError.message ();
    ASSERT_NE (server.readFrom (response, server.canRead (), &from), -1) << join::lastError.message ();
    ASSERT_EQ (from, Icmp::Resolver::resolve (host_));
    ASSERT_EQ (client.close (), 0) << join::lastError.message ();
    ASSERT_EQ (server.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test connect method.
 */
TEST_F (IcmpSocket, connect)
{
    Icmp::Socket icmpSocket (Icmp::Socket::Blocking);

    ASSERT_EQ (icmpSocket.connect (Icmp::Resolver::resolve (host_)), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief disconnect method.
 */
TEST_F (IcmpSocket, disconnect)
{
    Icmp::Socket icmpSocket (Icmp::Socket::Blocking);

    ASSERT_FALSE (icmpSocket.connected ());
    ASSERT_EQ (icmpSocket.connect (Icmp::Resolver::resolve (host_)), 0) << join::lastError.message ();
    ASSERT_TRUE (icmpSocket.connected ());
    ASSERT_EQ (icmpSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_FALSE (icmpSocket.connected ());
    ASSERT_EQ (icmpSocket.close (), 0) << join::lastError.message ();
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

    ASSERT_EQ (server.bind (Icmp::Resolver::resolve (host_)), 0) << join::lastError.message ();
    ASSERT_EQ (client.connect (Icmp::Resolver::resolve (host_)), 0) << join::lastError.message ();
    ASSERT_TRUE (client.waitReadyWrite (timeout_));
    ASSERT_GT (client.write (data_.get (), sizeof (struct icmphdr)), 0) << join::lastError.message ();
    ASSERT_TRUE (client.waitReadyWrite (timeout_));
    ASSERT_GT (client.write (data_.get (), sizeof (struct icmphdr)), 0) << join::lastError.message ();
    ASSERT_TRUE (server.waitReadyRead (timeout_));
    ASSERT_NE (server.canRead (), -1) << join::lastError.message ();
    ASSERT_NE (server.readFrom (response, server.canRead (), &from), -1) << join::lastError.message ();
    ASSERT_TRUE (server.waitReadyRead (timeout_));
    ASSERT_NE (server.canRead (), -1) << join::lastError.message ();
    ASSERT_NE (server.readFrom (response, server.canRead (), &from), -1) << join::lastError.message ();
    ASSERT_EQ (client.close (), 0) << join::lastError.message ();
    ASSERT_EQ (server.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test waitReadyRead method.
 */
TEST_F (IcmpSocket, waitReadyRead)
{
    Icmp::Socket client (Icmp::Socket::NonBlocking), server;
    Icmp::Endpoint from;
    char response[1024];

    ASSERT_EQ (server.bind (Icmp::Resolver::resolve (host_)), 0) << join::lastError.message ();
    if (client.connect (Icmp::Resolver::resolve (host_)) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (client.waitReadyWrite (timeout_));
    ASSERT_GT (client.write (data_.get (), sizeof (struct icmphdr)), 0) << join::lastError.message ();
    ASSERT_TRUE (server.waitReadyRead (timeout_));
    ASSERT_NE (server.canRead (), -1) << join::lastError.message ();
    ASSERT_NE (server.readFrom (response, server.canRead (), &from), -1) << join::lastError.message ();
    ASSERT_EQ (from, Icmp::Resolver::resolve (host_));
    ASSERT_NE (server.writeTo (data_.get (), sizeof (struct icmphdr), from), -1) << join::lastError.message ();
    ASSERT_TRUE (client.waitReadyRead (timeout_));
    ASSERT_EQ (client.close (), 0) << join::lastError.message ();
    ASSERT_EQ (server.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test read method.
 */
TEST_F (IcmpSocket, read)
{
    Icmp::Socket client, server;
    Icmp::Endpoint from;
    char response[1024];

    ASSERT_EQ (server.bind (Icmp::Resolver::resolve (host_)), 0) << join::lastError.message ();
    ASSERT_EQ (client.connect (Icmp::Resolver::resolve (host_)), 0) << join::lastError.message ();
    ASSERT_TRUE (client.waitReadyWrite (timeout_));
    ASSERT_GT (client.write (data_.get (), sizeof (struct icmphdr)), 0) << join::lastError.message ();
    ASSERT_TRUE (server.waitReadyRead (timeout_));
    ASSERT_NE (server.canRead (), -1) << join::lastError.message ();
    ASSERT_NE (server.readFrom (response, server.canRead (), &from), -1) << join::lastError.message ();
    ASSERT_EQ (from, Icmp::Resolver::resolve (host_));
    ASSERT_NE (server.writeTo (data_.get (), sizeof (struct icmphdr), from), -1) << join::lastError.message ();
    ASSERT_TRUE (client.waitReadyRead (timeout_));
    ASSERT_NE (client.canRead (), -1) << join::lastError.message ();
    ASSERT_NE (client.read (response, client.canRead ()), -1) << join::lastError.message ();
    ASSERT_EQ (client.close (), 0) << join::lastError.message ();
    ASSERT_EQ (server.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test readFrom method.
 */
TEST_F (IcmpSocket, readFrom)
{
    Icmp::Socket client, server;
    Icmp::Endpoint from;
    char response[1024];

    ASSERT_EQ (server.bind (Icmp::Resolver::resolve (host_)), 0) << join::lastError.message ();
    ASSERT_EQ (client.connect (Icmp::Resolver::resolve (host_)), 0) << join::lastError.message ();
    ASSERT_TRUE (client.waitReadyWrite (timeout_));
    ASSERT_GT (client.write (data_.get (), sizeof (struct icmphdr)), 0) << join::lastError.message ();
    ASSERT_TRUE (server.waitReadyRead (timeout_));
    ASSERT_NE (server.canRead (), -1) << join::lastError.message ();
    ASSERT_NE (server.readFrom (response, server.canRead (), &from), -1) << join::lastError.message ();
    ASSERT_EQ (from, Icmp::Resolver::resolve (host_));
    ASSERT_EQ (client.close (), 0) << join::lastError.message ();
    ASSERT_EQ (server.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test waitReadyWrite method.
 */
TEST_F (IcmpSocket, waitReadyWrite)
{
    Icmp::Socket client (Icmp::Socket::NonBlocking), server;
    Icmp::Endpoint from;
    char response[1024];

    ASSERT_EQ (server.bind (Icmp::Resolver::resolve (host_)), 0) << join::lastError.message ();
    ASSERT_EQ (client.connect (Icmp::Resolver::resolve (host_)), 0) << join::lastError.message ();
    ASSERT_TRUE (client.waitReadyWrite (timeout_));
    ASSERT_GT (client.write (data_.get (), sizeof (struct icmphdr)), 0) << join::lastError.message ();
    ASSERT_TRUE (server.waitReadyRead (timeout_));
    ASSERT_NE (server.canRead (), -1) << join::lastError.message ();
    ASSERT_NE (server.readFrom (response, server.canRead (), &from), -1) << join::lastError.message ();
    ASSERT_EQ (from, Icmp::Resolver::resolve (host_));
    ASSERT_EQ (client.close (), 0) << join::lastError.message ();
    ASSERT_EQ (server.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test write method.
 */
TEST_F (IcmpSocket, write)
{
    Icmp::Socket client (Icmp::Socket::Blocking), server;
    Icmp::Endpoint from;
    char response[1024];

    ASSERT_EQ (server.bind (Icmp::Resolver::resolve (host_)), 0) << join::lastError.message ();
    ASSERT_EQ (client.connect (Icmp::Resolver::resolve (host_)), 0) << join::lastError.message ();
    ASSERT_GT (client.write (data_.get (), sizeof (struct icmphdr)), 0) << join::lastError.message ();
    ASSERT_TRUE (server.waitReadyRead (timeout_));
    ASSERT_NE (server.canRead (), -1) << join::lastError.message ();
    ASSERT_NE (server.readFrom (response, server.canRead (), &from), -1) << join::lastError.message ();
    ASSERT_EQ (from, Icmp::Resolver::resolve (host_));
    ASSERT_EQ (client.close (), 0) << join::lastError.message ();
    ASSERT_EQ (server.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test writeTo method.
 */
TEST_F (IcmpSocket, writeTo)
{
    Icmp::Socket client (Icmp::Socket::Blocking), server;
    Icmp::Endpoint from;
    char response[1024];

    ASSERT_EQ (server.bind (Icmp::Resolver::resolve (host_)), 0) << join::lastError.message ();
    ASSERT_NE (client.writeTo (data_.get (), sizeof (struct icmphdr), Icmp::Resolver::resolve (host_)), -1) << join::lastError.message ();
    ASSERT_TRUE (server.waitReadyRead (timeout_));
    ASSERT_NE (server.canRead (), -1) << join::lastError.message ();
    ASSERT_NE (server.readFrom (response, server.canRead (), &from), -1) << join::lastError.message ();
    ASSERT_EQ (from, Icmp::Resolver::resolve (host_));
    ASSERT_EQ (client.close (), 0) << join::lastError.message ();
    ASSERT_EQ (server.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test setMode method.
 */
TEST_F (IcmpSocket, setMode)
{
    Icmp::Socket icmpSocket (Icmp::Socket::Blocking);

    ASSERT_EQ (icmpSocket.connect (Icmp::Resolver::resolve (host_)), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.setMode (Icmp::Socket::NonBlocking), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test setOption method.
 */
TEST_F (IcmpSocket, setOption)
{
    Icmp::Socket icmpSocket (Icmp::Socket::Blocking);

    ASSERT_EQ (icmpSocket.setOption (Icmp::Socket::MulticastLoop, 1), -1);
    ASSERT_EQ (icmpSocket.connect (Icmp::Resolver::resolve (host_)), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.setOption (Icmp::Socket::MulticastLoop, 1), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test localEndpoint method.
 */
TEST_F (IcmpSocket, localEndpoint)
{
    Icmp::Socket icmpSocket (Icmp::Socket::Blocking);

    ASSERT_EQ (icmpSocket.bind (Icmp::Resolver::resolve (host_)), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.connect (Icmp::Resolver::resolve (host_)), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.localEndpoint ().ip (), Icmp::Resolver::resolve (host_).ip ()) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.close (), 0) << join::lastError.message ();
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
    ASSERT_EQ (icmpSocket.connect (Icmp::Resolver::resolve (host_)), 0) << join::lastError.message ();
    ASSERT_TRUE (icmpSocket.opened ());
    ASSERT_EQ (icmpSocket.close (), 0) << join::lastError.message ();
    ASSERT_FALSE (icmpSocket.opened ());
}

/**
 * @brief Test connected method.
 */
TEST_F (IcmpSocket, connected)
{
    Icmp::Socket icmpSocket (Icmp::Socket::Blocking);

    ASSERT_FALSE (icmpSocket.connected ());
    ASSERT_EQ (icmpSocket.connect (Icmp::Resolver::resolve (host_)), 0) << join::lastError.message ();
    ASSERT_TRUE (icmpSocket.connected());
    ASSERT_EQ (icmpSocket.close (), 0) << join::lastError.message ();
    ASSERT_FALSE (icmpSocket.connected ());
}

/**
 * @brief Test encrypted method.
 */
TEST_F (IcmpSocket, encrypted)
{
    Icmp::Socket icmpSocket (Icmp::Socket::Blocking);

    ASSERT_FALSE (icmpSocket.opened ());
    ASSERT_EQ (icmpSocket.open (Icmp::v4 ()), 0) << join::lastError.message ();
    ASSERT_FALSE (icmpSocket.encrypted ());
    ASSERT_EQ (icmpSocket.connect (Icmp::Resolver::resolveHost (host_)), 0) << join::lastError.message ();
    ASSERT_FALSE (icmpSocket.encrypted ());
    ASSERT_EQ (icmpSocket.close (), 0) << join::lastError.message ();
    ASSERT_FALSE (icmpSocket.encrypted ());
}

/**
 * @brief Test family method.
 */
TEST_F (IcmpSocket, family)
{
    Icmp::Socket icmpSocket;

    ASSERT_EQ (icmpSocket.family (), AF_INET);

    ASSERT_EQ (icmpSocket.bind ({IpAddress (AF_INET6)}), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.family (), AF_INET6);
    ASSERT_EQ (icmpSocket.close (), 0) << join::lastError.message ();

    ASSERT_EQ (icmpSocket.bind ({IpAddress (AF_INET)}), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.family (), AF_INET);
    ASSERT_EQ (icmpSocket.close (), 0) << join::lastError.message ();
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

    ASSERT_EQ (icmpSocket.bind ({IpAddress (AF_INET6)}), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.protocol (), IPPROTO_ICMPV6);
    ASSERT_EQ (icmpSocket.close (), 0) << join::lastError.message ();

    ASSERT_EQ (icmpSocket.bind ({IpAddress (AF_INET)}), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.protocol (), IPPROTO_ICMP);
    ASSERT_EQ (icmpSocket.close (), 0) << join::lastError.message ();
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
    ASSERT_EQ (icmpSocket.close (), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.handle (), -1);
}

/**
 * @brief Test mtu method.
 */
TEST_F (IcmpSocket, mtu)
{
    Icmp::Socket icmpSocket (Icmp::Socket::Blocking);

    ASSERT_EQ (icmpSocket.connect (Icmp::Resolver::resolve (host_)), 0) << join::lastError.message ();
    ASSERT_NE (icmpSocket.mtu (), -1) << join::lastError.message ();
    ASSERT_EQ (icmpSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test randomize method.
 */
TEST_F (IcmpSocket, randomize)
{
    ASSERT_GT (Icmp::Socket::randomize <int> (), 0);
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
    ASSERT_EQ (icmpSocket1.close (), 0) << join::lastError.message ();
    ASSERT_EQ (icmpSocket2.close (), 0) << join::lastError.message ();
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
