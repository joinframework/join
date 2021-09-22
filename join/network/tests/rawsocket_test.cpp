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

// C.
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <net/ethernet.h>

using join::MacAddress;
using join::IpAddress;
using join::Raw;

/**
 * @brief Class used to test the raw socket API.
 */
class RawSocket : public ::testing::Test, Raw::Socket::Observer
{
public:
    /**
     * @brief set up test case.
     */
    static void SetUpTestCase ()
    {
        // fill in data.
        memcpy (packet_.data, "this is a test", strlen ("this is a test"));

        // fill in UDP header.
        packet_.ip.protocol = IPPROTO_UDP;
        packet_.ip.saddr    = *reinterpret_cast <const uint32_t *> (IpAddress ("127.0.0.1").addr ());
        packet_.ip.daddr    = *reinterpret_cast <const uint32_t *> (IpAddress ("127.0.0.1").addr ());
        packet_.udp.source  = htons (5000);
        packet_.udp.dest    = htons (5000);
        packet_.udp.len     = htons (sizeof (Packet) - sizeof (packet_.eth) - sizeof (packet_.ip));
        packet_.ip.tot_len  = packet_.udp.len;
        packet_.udp.check   = Raw::Socket::checksum (reinterpret_cast <uint16_t *> (&packet_.ip), sizeof (Packet) - sizeof (packet_.eth));

        // fill in IP header.
        packet_.ip.ihl      = sizeof (packet_.ip) >> 2;
        packet_.ip.version  = IPVERSION;
        packet_.ip.tos      = IPTOS_CLASS_CS6 | IPTOS_ECN_NOT_ECT;
        packet_.ip.tot_len  = htons (sizeof (Packet) - sizeof (packet_.eth));
        packet_.ip.id       = htons (join::randomize <uint16_t> ());
        packet_.ip.frag_off = htons (IP_DF);
        packet_.ip.ttl      = IPDEFTTL;
        packet_.ip.check    = Raw::Socket::checksum (reinterpret_cast <uint16_t *> (&packet_.ip), sizeof (packet_.ip));

        // fill in ETH header.
        memcpy (packet_.eth.h_dest, MacAddress::wildcard.addr (), 6);
        memcpy (packet_.eth.h_source, MacAddress::wildcard.addr (), 6);
        packet_.eth.h_proto = htons (ETH_P_IP);
    }

protected:
    /**
     * @brief Sets up the test fixture.
     */
    void SetUp ()
    {
        ASSERT_EQ (bind (interface_), 0) << join::lastError.message ();
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
            int nread = read (buffer.get (), canRead ());
            if (size_t (nread) < sizeof (Packet))
            {
                return;
            }

            Packet *data = reinterpret_cast <Packet *> (buffer.get ());
            if (data->ip.id != packet_.ip.id)
            {
                return;
            }

            write (buffer.get (), nread);
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
    static Packet packet_;

    /// hostname.
    static const std::string interface_;

    /// timeout.
    static const int timeout_;
};

RawSocket::Packet RawSocket::packet_;
const std::string RawSocket::interface_ = "lo";
const int         RawSocket::timeout_   = 1000;

/**
 * @brief Test open method.
 */
TEST_F (RawSocket, open)
{
    Raw::Socket rawSocket;

    ASSERT_EQ (rawSocket.open (), 0) << join::lastError.message ();
    ASSERT_EQ (rawSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test close method.
 */
TEST_F (RawSocket, close)
{
    Raw::Socket rawSocket (Raw::Socket::Blocking);

    ASSERT_FALSE (rawSocket.opened ());
    ASSERT_EQ (rawSocket.bind (interface_), 0) << join::lastError.message ();
    ASSERT_TRUE (rawSocket.opened ());
    ASSERT_EQ (rawSocket.close (), 0) << join::lastError.message ();
    ASSERT_FALSE (rawSocket.opened ());
}

/**
 * @brief Test bind method.
 */
TEST_F (RawSocket, bind)
{
    Raw::Socket rawSocket (Raw::Socket::Blocking);

    ASSERT_EQ (rawSocket.bind (interface_), 0) << join::lastError.message ();
    ASSERT_EQ (rawSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test canRead method.
 */
TEST_F (RawSocket, canRead)
{
    Raw::Socket rawSocket (Raw::Socket::Blocking);

    ASSERT_EQ (rawSocket.bind (interface_), 0) << join::lastError.message ();
    ASSERT_TRUE (rawSocket.waitReadyWrite (timeout_)) << join::lastError.message ();
    ASSERT_EQ (rawSocket.write (reinterpret_cast <char* > (&packet_), sizeof (packet_)), sizeof (packet_)) << join::lastError.message ();
    ASSERT_TRUE (rawSocket.waitReadyRead (timeout_)) << join::lastError.message ();
    ASSERT_GT (rawSocket.canRead (), 0) << join::lastError.message ();
    ASSERT_EQ (rawSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test waitReadyRead method.
 */
TEST_F (RawSocket, waitReadyRead)
{
    Raw::Socket rawSocket;

    ASSERT_EQ (rawSocket.bind (interface_), 0) << join::lastError.message ();
    ASSERT_TRUE (rawSocket.waitReadyWrite (timeout_)) << join::lastError.message ();
    ASSERT_EQ (rawSocket.write (reinterpret_cast <char* > (&packet_), sizeof (packet_)), sizeof (packet_)) << join::lastError.message ();
    ASSERT_TRUE (rawSocket.waitReadyRead (timeout_)) << join::lastError.message ();
    ASSERT_EQ (rawSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test read method.
 */
TEST_F (RawSocket, read)
{
    Raw::Socket rawSocket (Raw::Socket::Blocking);
    char data[1024];

    ASSERT_EQ (rawSocket.bind (interface_), 0) << join::lastError.message ();
    ASSERT_TRUE (rawSocket.waitReadyWrite (timeout_)) << join::lastError.message ();
    ASSERT_EQ (rawSocket.write (reinterpret_cast <char* > (&packet_), sizeof (packet_)), sizeof (packet_)) << join::lastError.message ();
    ASSERT_TRUE (rawSocket.waitReadyRead (timeout_)) << join::lastError.message ();
    ASSERT_GT (rawSocket.read (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_EQ (rawSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test waitReadyWrite method.
 */
TEST_F (RawSocket, waitReadyWrite)
{
    Raw::Socket rawSocket;

    ASSERT_EQ (rawSocket.bind (interface_), 0) << join::lastError.message ();
    ASSERT_TRUE (rawSocket.waitReadyWrite (timeout_)) << join::lastError.message ();
    ASSERT_EQ (rawSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test write method.
 */
TEST_F (RawSocket, write)
{
    Raw::Socket rawSocket;

    ASSERT_EQ (rawSocket.bind (interface_), 0) << join::lastError.message ();
    ASSERT_TRUE (rawSocket.waitReadyWrite (timeout_)) << join::lastError.message ();
    ASSERT_EQ (rawSocket.write (reinterpret_cast <char* > (&packet_), sizeof (packet_)), sizeof (packet_)) << join::lastError.message ();
    ASSERT_TRUE (rawSocket.waitReadyRead (timeout_)) << join::lastError.message ();
    ASSERT_EQ (rawSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test setMode method.
 */
TEST_F (RawSocket, setMode)
{
    Raw::Socket rawSocket (Raw::Socket::Blocking);

    ASSERT_EQ (rawSocket.open (), 0) << join::lastError.message ();
    ASSERT_EQ (rawSocket.setMode (Raw::Socket::NonBlocking), 0) << join::lastError.message ();
    ASSERT_EQ (rawSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test setOption method.
 */
TEST_F (RawSocket, setOption)
{
    Raw::Socket rawSocket (Raw::Socket::Blocking);

    ASSERT_EQ (rawSocket.setOption (Raw::Socket::Broadcast, 1), -1);
    ASSERT_EQ (rawSocket.bind (interface_), 0) << join::lastError.message ();
    ASSERT_EQ (rawSocket.setOption (Raw::Socket::Broadcast, 1), 0) << join::lastError.message ();
    ASSERT_EQ (rawSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test localEndpoint method.
 */
TEST_F (RawSocket, localEndpoint)
{
    Raw::Socket rawSocket;

    ASSERT_EQ (rawSocket.bind (interface_), 0) << join::lastError.message ();
    ASSERT_EQ (rawSocket.localEndpoint ().device (), interface_);
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
    ASSERT_EQ (rawSocket.close (), 0) << join::lastError.message ();
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
    ASSERT_EQ (rawSocket.close (), 0) << join::lastError.message ();
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
    ASSERT_EQ (rawSocket.close (), 0) << join::lastError.message ();
    ASSERT_EQ (rawSocket.handle (), -1);
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
    ASSERT_EQ (rawSocket1.close (), 0) << join::lastError.message ();
    ASSERT_EQ (rawSocket2.close (), 0) << join::lastError.message ();
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}