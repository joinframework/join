/**
 * MIT License
 *
 * Copyright (c) 2025 Mathieu Rabine
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
using join::Netlink;

/**
 * @brief Class used to test the netlink socket API.
 */
class NetlinkSocket : public ::testing::Test
{
public:
    /**
     * @brief set up test case.
     */
    static void SetUpTestCase ()
    {
        _data = std::make_unique <char[]> (4096);
        memset (_data.get (), 0, 4096);

        // netlink header.
        struct nlmsghdr *nlh = reinterpret_cast <struct nlmsghdr *> (_data.get ());
        nlh->nlmsg_len = NLMSG_LENGTH (sizeof (struct rtgenmsg));
        nlh->nlmsg_type = RTM_GETLINK;
        nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
        nlh->nlmsg_seq = 1;

        // general message.
        struct rtgenmsg *rtgen = reinterpret_cast <struct rtgenmsg*> (NLMSG_DATA (nlh));
        rtgen->rtgen_family = AF_UNSPEC;
    }

protected:
    /// groups.
    static const uint32_t _groups;

    /// timeout.
    static const int _timeout;

    /// data.
    static std::unique_ptr <char[]> _data;
};

const uint32_t NetlinkSocket::_groups = RTMGRP_LINK;
const int NetlinkSocket::_timeout = 1000;
std::unique_ptr <char[]> NetlinkSocket::_data;

/**
 * @brief Test open method.
 */
TEST_F (NetlinkSocket, open)
{
    Netlink::Socket netlinkSocket;

    ASSERT_EQ (netlinkSocket.open (Netlink::rt ()), 0) << join::lastError.message ();
    ASSERT_EQ (netlinkSocket.open (Netlink::rt ()), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    netlinkSocket.close ();

    ASSERT_EQ (netlinkSocket.open (Netlink::nf ()), 0) << join::lastError.message ();
    ASSERT_EQ (netlinkSocket.open (Netlink::nf ()), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    netlinkSocket.close ();
}

/**
 * @brief Test close method.
 */
TEST_F (NetlinkSocket, close)
{
    Netlink::Socket netlinkSocket (Netlink::Socket::Blocking);

    ASSERT_FALSE (netlinkSocket.opened ());
    ASSERT_EQ (netlinkSocket.connect (_groups), 0) << join::lastError.message ();
    ASSERT_TRUE (netlinkSocket.opened ());
    netlinkSocket.close ();
    ASSERT_FALSE (netlinkSocket.opened ());
}

/**
 * @brief Test bind method.
 */
TEST_F (NetlinkSocket, bind)
{
    Netlink::Socket netlinkSocket (Netlink::Socket::Blocking);

    ASSERT_EQ (netlinkSocket.connect (_groups), 0) << join::lastError.message ();
    ASSERT_EQ (netlinkSocket.bind (_groups), 0) << join::lastError.message ();
    ASSERT_EQ (netlinkSocket.disconnect (), 0) << join::lastError.message ();

    ASSERT_EQ (netlinkSocket.bind (_groups), 0) << join::lastError.message ();
    ASSERT_EQ (netlinkSocket.connect (_groups), 0) << join::lastError.message ();
    ASSERT_EQ (netlinkSocket.disconnect (), 0) << join::lastError.message ();

    netlinkSocket.close ();
}

/**
 * @brief Test bindToDevice method.
 */
TEST_F (NetlinkSocket, bindToDevice)
{
    Netlink::Socket netlinkSocket (Netlink::Socket::Blocking);

    ASSERT_EQ (netlinkSocket.bindToDevice ("lo"), -1);

    ASSERT_EQ (netlinkSocket.connect (_groups), 0) << join::lastError.message ();
    ASSERT_EQ (netlinkSocket.bindToDevice ("lo"), -1);
    ASSERT_EQ (netlinkSocket.disconnect (), 0) << join::lastError.message ();

    ASSERT_EQ (netlinkSocket.bindToDevice ("lo"), 0) << join::lastError.message ();
    ASSERT_EQ (netlinkSocket.connect (_groups), 0) << join::lastError.message ();
    ASSERT_EQ (netlinkSocket.disconnect (), 0) << join::lastError.message ();

    ASSERT_EQ (netlinkSocket.bindToDevice ("foo"), -1);

    netlinkSocket.close ();
}

/**
 * @brief Test connect method.
 */
TEST_F (NetlinkSocket, connect)
{
    Netlink::Socket netlinkSocket (Netlink::Socket::Blocking);

    ASSERT_EQ (netlinkSocket.connect (_groups), 0) << join::lastError.message ();
    ASSERT_EQ (netlinkSocket.connect (_groups), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    netlinkSocket.close ();
}

/**
 * @brief disconnect method.
 */
TEST_F (NetlinkSocket, disconnect)
{
    Netlink::Socket netlinkSocket (Netlink::Socket::Blocking);

    ASSERT_FALSE (netlinkSocket.connected ());
    ASSERT_EQ (netlinkSocket.connect (_groups), 0) << join::lastError.message ();
    ASSERT_TRUE (netlinkSocket.connected ());
    ASSERT_EQ (netlinkSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_FALSE (netlinkSocket.connected ());
    netlinkSocket.close ();
    ASSERT_FALSE (netlinkSocket.connected ());
}

/**
 * @brief Test canRead method.
 */
TEST_F (NetlinkSocket, canRead)
{
    Netlink::Socket netlinkSocket (Netlink::Socket::Blocking);

    ASSERT_EQ (netlinkSocket.canRead (), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (netlinkSocket.connect (_groups), 0) << join::lastError.message ();
    ASSERT_TRUE (netlinkSocket.waitReadyWrite (_timeout));
    ASSERT_GT (netlinkSocket.write (_data.get (), reinterpret_cast <struct nlmsghdr*> (_data.get ())->nlmsg_len), 0) << join::lastError.message ();
    ASSERT_EQ (netlinkSocket.canRead (), -1);
    netlinkSocket.close ();
}

/**
 * @brief Test waitReadyRead method.
 */
TEST_F (NetlinkSocket, waitReadyRead)
{
    Netlink::Socket netlinkSocket;
    char data[1024];

    ASSERT_FALSE (netlinkSocket.waitReadyRead (_timeout));
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (netlinkSocket.connect (_groups), 0) << join::lastError.message ();
    ASSERT_TRUE (netlinkSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (netlinkSocket.write (data, sizeof (data)), sizeof (data)) << join::lastError.message ();
    ASSERT_TRUE (netlinkSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    netlinkSocket.close ();
}

/**
 * @brief Test read method.
 */
TEST_F (NetlinkSocket, read)
{
    Netlink::Socket netlinkSocket;
    char data[1024];

    ASSERT_EQ (netlinkSocket.read (data, sizeof (data)), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (netlinkSocket.connect (_groups), 0) << join::lastError.message ();
    ASSERT_TRUE (netlinkSocket.waitReadyWrite (_timeout));
    ASSERT_GT (netlinkSocket.write (_data.get (), reinterpret_cast <struct nlmsghdr*> (_data.get ())->nlmsg_len), 0) << join::lastError.message ();
    ASSERT_TRUE (netlinkSocket.waitReadyRead (_timeout));
    ASSERT_NE (netlinkSocket.read (data, sizeof (data)), -1) << join::lastError.message ();
    netlinkSocket.close ();
}

/**
 * @brief Test readFrom method.
 */
TEST_F (NetlinkSocket, readFrom)
{
    Netlink::Socket netlinkSocket (Netlink::Socket::Blocking);
    Netlink::Endpoint from;
    char data [4096];

    ASSERT_EQ (netlinkSocket.readFrom (data, sizeof (data)), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (netlinkSocket.bind (_groups), 0) << join::lastError.message ();
    ASSERT_EQ (netlinkSocket.connect (_groups), 0) << join::lastError.message ();
    ASSERT_TRUE (netlinkSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_GT (netlinkSocket.write (_data.get (), reinterpret_cast <struct nlmsghdr*> (_data.get ())->nlmsg_len), 0) << join::lastError.message ();
    ASSERT_TRUE (netlinkSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_GT (netlinkSocket.readFrom (data, sizeof (data), &from), 0) << join::lastError.message ();
    ASSERT_EQ (from, Netlink::Endpoint (_groups));
    netlinkSocket.close ();
}

/**
 * @brief Test waitReadyWrite method.
 */
TEST_F (NetlinkSocket, waitReadyWrite)
{
    Netlink::Socket netlinkSocket;

    ASSERT_FALSE (netlinkSocket.waitReadyWrite (_timeout));
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (netlinkSocket.connect (_groups), 0) << join::lastError.message ();
    ASSERT_TRUE (netlinkSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    netlinkSocket.close ();
}

/**
 * @brief Test write method.
 */
TEST_F (NetlinkSocket, write)
{
    Netlink::Socket netlinkSocket (Netlink::Socket::Blocking);

    ASSERT_EQ (netlinkSocket.write (_data.get (), reinterpret_cast <struct nlmsghdr*> (_data.get ())->nlmsg_len), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (netlinkSocket.connect (_groups), 0) << join::lastError.message ();
    ASSERT_TRUE (netlinkSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_GT (netlinkSocket.write (_data.get (), reinterpret_cast <struct nlmsghdr*> (_data.get ())->nlmsg_len), 0) << join::lastError.message ();
    ASSERT_TRUE (netlinkSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_EQ (netlinkSocket.disconnect (), 0) << join::lastError.message ();
    netlinkSocket.close ();
}

/**
 * @brief Test writeTo method.
 */
TEST_F (NetlinkSocket, writeTo)
{
    Netlink::Socket netlinkSocket (Netlink::Socket::Blocking);

    ASSERT_EQ (netlinkSocket.open (Netlink::rt ()), 0) << join::lastError.message ();
    ASSERT_TRUE (netlinkSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_GT (netlinkSocket.writeTo (_data.get (), reinterpret_cast <struct nlmsghdr*> (_data.get ())->nlmsg_len, _groups), 0) << join::lastError.message ();
    ASSERT_TRUE (netlinkSocket.waitReadyRead (_timeout));
    netlinkSocket.close ();
}

/**
 * @brief Test setMode method.
 */
TEST_F (NetlinkSocket, setMode)
{
    Netlink::Socket netlinkSocket;

    ASSERT_EQ (netlinkSocket.open (), 0) << join::lastError.message ();

    int flags = ::fcntl (netlinkSocket.handle (), F_GETFL, 0);
    ASSERT_TRUE (flags & O_NONBLOCK);

    netlinkSocket.setMode (Netlink::Socket::Blocking);
    flags = ::fcntl (netlinkSocket.handle (), F_GETFL, 0);
    ASSERT_FALSE (flags & O_NONBLOCK);

    netlinkSocket.setMode (Netlink::Socket::NonBlocking);
    flags = ::fcntl (netlinkSocket.handle (), F_GETFL, 0);
    ASSERT_TRUE (flags & O_NONBLOCK);

    netlinkSocket.close ();
}

/**
 * @brief Test setOption method.
 */
TEST_F (NetlinkSocket, setOption)
{
    Netlink::Socket netlinkSocket;

    ASSERT_EQ (netlinkSocket.setOption (Netlink::Socket::RcvBuffer, 1500), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (netlinkSocket.open (Netlink::rt ()), 0) << join::lastError.message ();
    ASSERT_EQ (netlinkSocket.setOption (Netlink::Socket::NoDelay, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (netlinkSocket.setOption (Netlink::Socket::KeepAlive, 1), 0) << join::lastError.message ();
    ASSERT_EQ (netlinkSocket.setOption (Netlink::Socket::KeepIdle, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (netlinkSocket.setOption (Netlink::Socket::KeepIntvl, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (netlinkSocket.setOption (Netlink::Socket::KeepCount, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (netlinkSocket.setOption (Netlink::Socket::SndBuffer, 1500), 0) << join::lastError.message ();
    ASSERT_EQ (netlinkSocket.setOption (Netlink::Socket::RcvBuffer, 1500), 0) << join::lastError.message ();
    ASSERT_EQ (netlinkSocket.setOption (Netlink::Socket::TimeStamp, 1), 0) << join::lastError.message ();
    ASSERT_EQ (netlinkSocket.setOption (Netlink::Socket::ReuseAddr, 1), 0) << join::lastError.message ();
    ASSERT_EQ (netlinkSocket.setOption (Netlink::Socket::ReusePort, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (netlinkSocket.setOption (Netlink::Socket::Broadcast, 1), 0) << join::lastError.message ();
    ASSERT_EQ (netlinkSocket.setOption (Netlink::Socket::Ttl, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (netlinkSocket.setOption (Netlink::Socket::MulticastLoop, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (netlinkSocket.setOption (Netlink::Socket::MulticastTtl, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (netlinkSocket.setOption (Netlink::Socket::PathMtuDiscover, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (netlinkSocket.setOption (Netlink::Socket::RcvError, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (netlinkSocket.setOption (Netlink::Socket::AuxData, 1), -1);
    ASSERT_EQ (join::lastError, std::errc::no_protocol_option);
    netlinkSocket.close ();

    ASSERT_EQ (netlinkSocket.open (Netlink::nf ()), 0) << join::lastError.message ();
    ASSERT_EQ (netlinkSocket.setOption (Netlink::Socket::NoDelay, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (netlinkSocket.setOption (Netlink::Socket::KeepAlive, 1), 0) << join::lastError.message ();
    ASSERT_EQ (netlinkSocket.setOption (Netlink::Socket::KeepIdle, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (netlinkSocket.setOption (Netlink::Socket::KeepIntvl, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (netlinkSocket.setOption (Netlink::Socket::KeepCount, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (netlinkSocket.setOption (Netlink::Socket::SndBuffer, 1500), 0) << join::lastError.message ();
    ASSERT_EQ (netlinkSocket.setOption (Netlink::Socket::RcvBuffer, 1500), 0) << join::lastError.message ();
    ASSERT_EQ (netlinkSocket.setOption (Netlink::Socket::TimeStamp, 1), 0) << join::lastError.message ();
    ASSERT_EQ (netlinkSocket.setOption (Netlink::Socket::ReuseAddr, 1), 0) << join::lastError.message ();
    ASSERT_EQ (netlinkSocket.setOption (Netlink::Socket::ReusePort, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (netlinkSocket.setOption (Netlink::Socket::Broadcast, 1), 0) << join::lastError.message ();
    ASSERT_EQ (netlinkSocket.setOption (Netlink::Socket::Ttl, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (netlinkSocket.setOption (Netlink::Socket::MulticastLoop, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (netlinkSocket.setOption (Netlink::Socket::MulticastTtl, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (netlinkSocket.setOption (Netlink::Socket::PathMtuDiscover, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (netlinkSocket.setOption (Netlink::Socket::RcvError, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (netlinkSocket.setOption (Netlink::Socket::AuxData, 1), -1);
    ASSERT_EQ (join::lastError, std::errc::no_protocol_option);
    netlinkSocket.close ();
}

/**
 * @brief Test localEndpoint method.
 */
TEST_F (NetlinkSocket, localEndpoint)
{
    Netlink::Socket netlinkSocket (Netlink::Socket::Blocking);

    ASSERT_EQ (netlinkSocket.localEndpoint (), Netlink::Endpoint {});
    ASSERT_EQ (netlinkSocket.bind (_groups), 0) << join::lastError.message ();
    ASSERT_EQ (netlinkSocket.connect (_groups), 0) << join::lastError.message ();
    ASSERT_EQ (netlinkSocket.localEndpoint (), Netlink::Endpoint (_groups)) << join::lastError.message ();
    netlinkSocket.close ();
}

/**
 * @brief Test remoteEndpoint method.
 */
TEST_F (NetlinkSocket, remoteEndpoint)
{
    Netlink::Socket netlinkSocket (Netlink::Socket::Blocking);

    ASSERT_EQ (netlinkSocket.remoteEndpoint (), Netlink::Endpoint {});
    ASSERT_EQ (netlinkSocket.bind (_groups), 0) << join::lastError.message ();
    ASSERT_EQ (netlinkSocket.connect (_groups), 0) << join::lastError.message ();
    ASSERT_EQ (netlinkSocket.remoteEndpoint (), Netlink::Endpoint (_groups)) << join::lastError.message ();
    netlinkSocket.close ();
}

/**
 * @brief Test opened method.
 */
TEST_F (NetlinkSocket, opened)
{
    Netlink::Socket netlinkSocket (Netlink::Socket::Blocking);

    ASSERT_FALSE (netlinkSocket.opened ());
    ASSERT_EQ (netlinkSocket.open (Netlink::rt ()), 0) << join::lastError.message ();
    ASSERT_TRUE (netlinkSocket.opened ());
    ASSERT_EQ (netlinkSocket.connect (_groups), 0) << join::lastError.message ();
    ASSERT_TRUE (netlinkSocket.opened ());
    netlinkSocket.close ();
    ASSERT_FALSE (netlinkSocket.opened ());
}

/**
 * @brief Test connected method.
 */
TEST_F (NetlinkSocket, connected)
{
    Netlink::Socket netlinkSocket (Netlink::Socket::Blocking);

    ASSERT_FALSE (netlinkSocket.opened ());
    ASSERT_EQ (netlinkSocket.open (Netlink::rt ()), 0) << join::lastError.message ();
    ASSERT_FALSE (netlinkSocket.connected ());
    ASSERT_EQ (netlinkSocket.connect (_groups), 0) << join::lastError.message ();
    ASSERT_TRUE (netlinkSocket.connected ());
    netlinkSocket.close ();
    ASSERT_FALSE (netlinkSocket.connected ());
}

/**
 * @brief Test encrypted method.
 */
TEST_F (NetlinkSocket, encrypted)
{
    Netlink::Socket netlinkSocket (Netlink::Socket::Blocking);

    ASSERT_FALSE (netlinkSocket.opened ());
    ASSERT_EQ (netlinkSocket.open (Netlink::rt ()), 0) << join::lastError.message ();
    ASSERT_FALSE (netlinkSocket.encrypted ());
    ASSERT_EQ (netlinkSocket.connect (_groups), 0) << join::lastError.message ();
    ASSERT_FALSE (netlinkSocket.encrypted ());
    netlinkSocket.close ();
    ASSERT_FALSE (netlinkSocket.encrypted ());
}

/**
 * @brief Test family method.
 */
TEST_F (NetlinkSocket, family)
{
    Netlink::Socket netlinkSocket;

    ASSERT_EQ (netlinkSocket.family (), AF_NETLINK);
}

/**
 * @brief Test type method.
 */
TEST_F (NetlinkSocket, type)
{
    Netlink::Socket netlinkSocket;

    ASSERT_EQ (netlinkSocket.type (), SOCK_RAW);
}

/**
 * @brief Test protocol method.
 */
TEST_F (NetlinkSocket, protocol)
{
    Netlink::Socket netlinkSocket;

    ASSERT_EQ (netlinkSocket.protocol (), NETLINK_ROUTE);

    ASSERT_EQ (netlinkSocket.open (Netlink::rt ()), 0) << join::lastError.message ();
    ASSERT_EQ (netlinkSocket.protocol (), NETLINK_ROUTE);
    netlinkSocket.close ();

    ASSERT_EQ (netlinkSocket.open (Netlink::nf ()), 0) << join::lastError.message ();
    ASSERT_EQ (netlinkSocket.protocol (), NETLINK_NETFILTER);
    netlinkSocket.close ();
}

/**
 * @brief Test handle method.
 */
TEST_F (NetlinkSocket, handle)
{
    Netlink::Socket netlinkSocket (Netlink::Socket::Blocking);

    ASSERT_EQ (netlinkSocket.handle (), -1);
    ASSERT_EQ (netlinkSocket.open (), 0) << join::lastError.message ();
    ASSERT_GT (netlinkSocket.handle (), -1);
    netlinkSocket.close ();
    ASSERT_EQ (netlinkSocket.handle (), -1);
}

/**
 * @brief Test mtu method.
 */
TEST_F (NetlinkSocket, mtu)
{
    Netlink::Socket netlinkSocket (Netlink::Socket::Blocking);

    ASSERT_EQ (netlinkSocket.mtu (), -1);
    ASSERT_EQ (netlinkSocket.connect (_groups), 0) << join::lastError.message ();
    ASSERT_EQ (netlinkSocket.mtu (), -1);
    netlinkSocket.close ();
}

/**
 * @brief Test checksum method.
 */
TEST_F (NetlinkSocket, checksum)
{
    std::string buffer ({'\xD2', '\xB6', '\x69', '\xFD', '\x2E'});

    ASSERT_EQ (Netlink::Socket::checksum (reinterpret_cast <uint16_t *> (&buffer[0]), buffer.size (), 0), 19349);
}


/**
 * @brief Test is lower method.
 */
TEST_F (NetlinkSocket, lower)
{
    Netlink::Socket netlinkSocket1, netlinkSocket2;

    ASSERT_EQ (netlinkSocket1.open (Netlink::rt ()), 0) << join::lastError.message ();
    ASSERT_EQ (netlinkSocket2.open (Netlink::rt ()), 0) << join::lastError.message ();
    if (netlinkSocket1.handle () < netlinkSocket2.handle ())
    {
        ASSERT_TRUE (netlinkSocket1 < netlinkSocket2);
    }
    else
    {
        ASSERT_TRUE (netlinkSocket2 < netlinkSocket1);
    }
    netlinkSocket1.close ();
    netlinkSocket2.close ();
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
