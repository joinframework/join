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
#include <join/protocol.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::Raw;
#ifdef JOIN_HAS_XDP
using join::Xdp;
#endif
using join::UnixDgram;
using join::UnixStream;
using join::Udp;
using join::Icmp;
using join::Tcp;

/**
 * @brief test the addr method.
 */
TEST (Endpoint, addr)
{
    Raw::Endpoint rawEndpoint;
    ASSERT_NE (rawEndpoint.addr (), nullptr);

#ifdef JOIN_HAS_XDP
    Xdp::Endpoint xdpEndpoint;
    ASSERT_NE (xdpEndpoint.addr (), nullptr);
#endif

    UnixDgram::Endpoint unixDgramEndpoint;
    ASSERT_NE (unixDgramEndpoint.addr (), nullptr);

    UnixStream::Endpoint unixStreamEndpoint;
    ASSERT_NE (unixStreamEndpoint.addr (), nullptr);

    Udp::Endpoint udpEndpoint;
    ASSERT_NE (udpEndpoint.addr (), nullptr);

    Icmp::Endpoint icmpEndpoint;
    ASSERT_NE (icmpEndpoint.addr (), nullptr);

    Tcp::Endpoint tcpEndpoint;
    ASSERT_NE (tcpEndpoint.addr (), nullptr);
}

/**
 * @brief test the length method.
 */
TEST (Endpoint, length)
{
    Raw::Endpoint rawEndpoint;
    ASSERT_EQ (rawEndpoint.length (), sizeof (struct sockaddr_ll));

#ifdef JOIN_HAS_XDP
    Xdp::Endpoint xdpEndpoint;
    ASSERT_EQ (xdpEndpoint.length (), sizeof (struct sockaddr_xdp));
#endif

    UnixDgram::Endpoint unixDgramEndpoint;
    ASSERT_EQ (unixDgramEndpoint.length (), sizeof (struct sockaddr_un));

    UnixStream::Endpoint unixStreamEndpoint;
    ASSERT_EQ (unixStreamEndpoint.length (), sizeof (struct sockaddr_un));

    Udp::Endpoint udpEndpoint4 (Udp::v4 ());
    ASSERT_EQ (udpEndpoint4.length (), sizeof (struct sockaddr_in));

    Udp::Endpoint udpEndpoint6 (Udp::v6 ());
    ASSERT_EQ (udpEndpoint6.length (), sizeof (struct sockaddr_in6));

    Icmp::Endpoint icmpEndpoint4 (Icmp::v4 ());
    ASSERT_EQ (icmpEndpoint4.length (), sizeof (struct sockaddr_in));

    Icmp::Endpoint icmpEndpoint6 (Icmp::v6 ());
    ASSERT_EQ (icmpEndpoint6.length (), sizeof (struct sockaddr_in6));

    Tcp::Endpoint tcpEndpoint4 (Tcp::v4 ());
    ASSERT_EQ (tcpEndpoint4.length (), sizeof (struct sockaddr_in));

    Tcp::Endpoint tcpEndpoint6 (Tcp::v6 ());
    ASSERT_EQ (tcpEndpoint6.length (), sizeof (struct sockaddr_in6));
}

/**
 * @brief test the device method.
 */
TEST (Endpoint, device)
{
    Raw::Endpoint rawEndpoint;
    ASSERT_EQ (rawEndpoint.device (), "");
    rawEndpoint.device ("lo");
    ASSERT_EQ (rawEndpoint.device (), "lo");

#ifdef JOIN_HAS_XDP
    Xdp::Endpoint xdpEndpoint;
    ASSERT_EQ (xdpEndpoint.device (), "");
    xdpEndpoint.device ("lo");
    ASSERT_EQ (xdpEndpoint.device (), "lo");
#endif

    UnixDgram::Endpoint unixDgramEndpoint;
    ASSERT_EQ (unixDgramEndpoint.device (), "");
    unixDgramEndpoint.device ("/path/to/file");
    ASSERT_EQ (unixDgramEndpoint.device (), "/path/to/file");

    UnixStream::Endpoint unixStreamEndpoint;
    ASSERT_EQ (unixStreamEndpoint.device (), "");
    unixStreamEndpoint.device ("/path/to/other");
    ASSERT_EQ (unixStreamEndpoint.device (), "/path/to/other");

    Udp::Endpoint udpEndpoint (Udp::v6 ());
    ASSERT_EQ (udpEndpoint.device (), "");
    udpEndpoint.device ("lo");
    ASSERT_EQ (udpEndpoint.device (), "lo");

    Icmp::Endpoint icmpEndpoint (Icmp::v6 ());
    ASSERT_EQ (icmpEndpoint.device (), "");
    icmpEndpoint.device ("lo");
    ASSERT_EQ (icmpEndpoint.device (), "lo");

    Tcp::Endpoint tcpEndpoint (Tcp::v6 ());
    ASSERT_EQ (tcpEndpoint.device (), "");
    tcpEndpoint.device ("lo");
    ASSERT_EQ (tcpEndpoint.device (), "lo");
}

#ifdef JOIN_HAS_XDP
/**
 * @brief test the queue method.
 */
TEST (Endpoint, queue)
{
    Xdp::Endpoint xdpEndpoint;
    ASSERT_EQ (xdpEndpoint.queue (), 0);

    xdpEndpoint.queue (1);
    ASSERT_EQ (xdpEndpoint.queue (), 1);

    xdpEndpoint.queue (7);
    ASSERT_EQ (xdpEndpoint.queue (), 7);
}
#endif

/**
 * @brief ip method.
 */
TEST (Endpoint, ip)
{
    Udp::Endpoint udpEndpoint;

    udpEndpoint.ip ("::");
    ASSERT_EQ (udpEndpoint.ip (), "::");
    udpEndpoint.ip ("127.0.0.1");
    ASSERT_EQ (udpEndpoint.ip (), "127.0.0.1");

    Icmp::Endpoint icmpEndpoint;

    icmpEndpoint.ip ("::");
    ASSERT_EQ (icmpEndpoint.ip (), "::");
    icmpEndpoint.ip ("127.0.0.1");
    ASSERT_EQ (icmpEndpoint.ip (), "127.0.0.1");

    Tcp::Endpoint tcpEndpoint;

    tcpEndpoint.ip ("::");
    ASSERT_EQ (tcpEndpoint.ip (), "::");
    tcpEndpoint.ip ("127.0.0.1");
    ASSERT_EQ (tcpEndpoint.ip (), "127.0.0.1");
}

/**
 * @brief port method.
 */
TEST (Endpoint, port)
{
    Udp::Endpoint udpEndpoint4 (Udp::v4 ());
    udpEndpoint4.port (80);
    ASSERT_EQ (udpEndpoint4.port (), 80);

    Udp::Endpoint udpEndpoint6 (Udp::v6 ());
    udpEndpoint6.port (443);
    ASSERT_EQ (udpEndpoint6.port (), 443);

    Tcp::Endpoint tcpEndpoint4 (Tcp::v4 ());
    tcpEndpoint4.port (80);
    ASSERT_EQ (tcpEndpoint4.port (), 80);

    Tcp::Endpoint tcpEndpoint6 (Tcp::v6 ());
    tcpEndpoint6.port (443);
    ASSERT_EQ (tcpEndpoint6.port (), 443);
}

/**
 * @brief protocol method.
 */
TEST (Endpoint, protocol)
{
    ASSERT_EQ (Udp::Endpoint ().protocol (), Udp::v4 ());
    ASSERT_EQ (Udp::Endpoint (Udp::v4 ()).protocol (), Udp::v4 ());
    ASSERT_NE (Udp::Endpoint (Udp::v4 ()).protocol (), Udp::v6 ());
    ASSERT_EQ (Udp::Endpoint (Udp::v6 ()).protocol (), Udp::v6 ());
    ASSERT_EQ (Udp::Endpoint ("127.0.0.1").protocol (), Udp::v4 ());
    ASSERT_NE (Udp::Endpoint ("127.0.0.1").protocol (), Udp::v6 ());
    ASSERT_NE (Udp::Endpoint ("::").protocol (), Udp::v4 ());
    ASSERT_EQ (Udp::Endpoint ("::").protocol (), Udp::v6 ());

    ASSERT_EQ (Icmp::Endpoint ().protocol (), Icmp::v4 ());
    ASSERT_EQ (Icmp::Endpoint (Icmp::v4 ()).protocol (), Icmp::v4 ());
    ASSERT_NE (Icmp::Endpoint (Icmp::v4 ()).protocol (), Icmp::v6 ());
    ASSERT_EQ (Icmp::Endpoint (Icmp::v6 ()).protocol (), Icmp::v6 ());
    ASSERT_EQ (Icmp::Endpoint ("127.0.0.1").protocol (), Icmp::v4 ());
    ASSERT_NE (Icmp::Endpoint ("127.0.0.1").protocol (), Icmp::v6 ());
    ASSERT_NE (Icmp::Endpoint ("::").protocol (), Icmp::v4 ());
    ASSERT_EQ (Icmp::Endpoint ("::").protocol (), Icmp::v6 ());

    ASSERT_EQ (Tcp::Endpoint ().protocol (), Tcp::v4 ());
    ASSERT_EQ (Tcp::Endpoint (Tcp::v4 ()).protocol (), Tcp::v4 ());
    ASSERT_NE (Tcp::Endpoint (Tcp::v4 ()).protocol (), Tcp::v6 ());
    ASSERT_EQ (Tcp::Endpoint (Tcp::v6 ()).protocol (), Tcp::v6 ());
    ASSERT_EQ (Tcp::Endpoint ("127.0.0.1").protocol (), Tcp::v4 ());
    ASSERT_NE (Tcp::Endpoint ("127.0.0.1").protocol (), Tcp::v6 ());
    ASSERT_NE (Tcp::Endpoint ("::").protocol (), Tcp::v4 ());
    ASSERT_EQ (Tcp::Endpoint ("::").protocol (), Tcp::v6 ());
}

/**
 * @brief equal method.
 */
TEST (Endpoint, equal)
{
#ifdef JOIN_HAS_XDP
    ASSERT_EQ (Xdp::Endpoint ("lo", 0), Xdp::Endpoint ("lo", 0));
    ASSERT_NE (Xdp::Endpoint ("lo", 0), Xdp::Endpoint ("lo", 1));
#endif

    ASSERT_EQ (UnixDgram::Endpoint ("/path/to/file"), UnixDgram::Endpoint ("/path/to/file"));
    ASSERT_NE (UnixDgram::Endpoint ("/path/to/file"), UnixDgram::Endpoint ("/path/to/other"));
    ASSERT_EQ (UnixDgram::Endpoint ("/path/to/other"), UnixDgram::Endpoint ("/path/to/other"));
    ASSERT_NE (UnixDgram::Endpoint ("/path/to/other"), UnixDgram::Endpoint ("/path/to/file"));

    ASSERT_EQ (UnixStream::Endpoint ("/path/to/file"), UnixStream::Endpoint ("/path/to/file"));
    ASSERT_NE (UnixStream::Endpoint ("/path/to/file"), UnixStream::Endpoint ("/path/to/other"));
    ASSERT_EQ (UnixStream::Endpoint ("/path/to/other"), UnixStream::Endpoint ("/path/to/other"));
    ASSERT_NE (UnixStream::Endpoint ("/path/to/other"), UnixStream::Endpoint ("/path/to/file"));

    ASSERT_EQ (Udp::Endpoint ("127.0.0.1", 80), Udp::Endpoint ("127.0.0.1", 80));
    ASSERT_NE (Udp::Endpoint ("127.0.0.1", 80), Udp::Endpoint ("fe80::57f3:baa4:fc3a:890a", 443));
    ASSERT_EQ (Udp::Endpoint ("fe80::57f3:baa4:fc3a:890a", 443), Udp::Endpoint ("fe80::57f3:baa4:fc3a:890a", 443));
    ASSERT_NE (Udp::Endpoint ("fe80::57f3:baa4:fc3a:890a", 443), Udp::Endpoint ("127.0.0.1", 80));

    ASSERT_EQ (Icmp::Endpoint ("127.0.0.1"), Icmp::Endpoint ("127.0.0.1"));
    ASSERT_NE (Icmp::Endpoint ("127.0.0.1"), Icmp::Endpoint ("fe80::57f3:baa4:fc3a:890a"));
    ASSERT_EQ (Icmp::Endpoint ("fe80::57f3:baa4:fc3a:890a"), Icmp::Endpoint ("fe80::57f3:baa4:fc3a:890a"));
    ASSERT_NE (Icmp::Endpoint ("fe80::57f3:baa4:fc3a:890a"), Icmp::Endpoint ("127.0.0.1"));

    ASSERT_EQ (Tcp::Endpoint ("127.0.0.1", 80), Tcp::Endpoint ("127.0.0.1", 80));
    ASSERT_NE (Tcp::Endpoint ("127.0.0.1", 80), Tcp::Endpoint ("fe80::57f3:baa4:fc3a:890a", 443));
    ASSERT_EQ (Tcp::Endpoint ("fe80::57f3:baa4:fc3a:890a", 443), Tcp::Endpoint ("fe80::57f3:baa4:fc3a:890a", 443));
    ASSERT_NE (Tcp::Endpoint ("fe80::57f3:baa4:fc3a:890a", 443), Tcp::Endpoint ("127.0.0.1", 80));
}

/**
 * @brief test the serialize method.
 */
TEST (Endpoint, serialize)
{
    std::stringstream stream;
    Raw::Endpoint rawEndpoint ("lo");
    ASSERT_NO_THROW (stream << rawEndpoint);
    ASSERT_EQ (stream.str (), "lo");

#ifdef JOIN_HAS_XDP
    stream.str ("");
    Xdp::Endpoint xdpEndpoint ("lo", 0);
    ASSERT_NO_THROW (stream << xdpEndpoint);
    ASSERT_EQ (stream.str (), "lo:0");
#endif

    stream.str ("");
    UnixDgram::Endpoint unixDgramEndpoint ("lo");
    ASSERT_NO_THROW (stream << unixDgramEndpoint);
    ASSERT_EQ (stream.str (), "lo");

    stream.str ("");
    UnixStream::Endpoint unixStreamEndpoint ("lo");
    ASSERT_NO_THROW (stream << unixStreamEndpoint);
    ASSERT_EQ (stream.str (), "lo");

    stream.str ("");
    Udp::Endpoint udpEndpoint ("127.0.0.1", 80);
    ASSERT_NO_THROW (stream << udpEndpoint);
    ASSERT_EQ (stream.str (), "127.0.0.1:80");

    stream.str ("");
    udpEndpoint.ip ("::");
    ASSERT_NO_THROW (stream << udpEndpoint);
    ASSERT_EQ (stream.str (), "[::]:80");

    stream.str ("");
    Icmp::Endpoint icmpEndpoint ("127.0.0.1");
    ASSERT_NO_THROW (stream << icmpEndpoint);
    ASSERT_EQ (stream.str (), "127.0.0.1");

    stream.str ("");
    icmpEndpoint.ip ("::");
    ASSERT_NO_THROW (stream << icmpEndpoint);
    ASSERT_EQ (stream.str (), "[::]");

    stream.str ("");
    Tcp::Endpoint tcpEndpoint ("127.0.0.1", 80);
    ASSERT_NO_THROW (stream << tcpEndpoint);
    ASSERT_EQ (stream.str (), "127.0.0.1:80");

    stream.str ("");
    tcpEndpoint.ip ("::");
    ASSERT_NO_THROW (stream << tcpEndpoint);
    ASSERT_EQ (stream.str (), "[::]:80");
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
