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

using join::UnixDgram;
using join::UnixStream;
using join::Raw;
using join::Udp;
using join::Icmp;
using join::Tcp;
using join::Tls;

/**
 * @brief test the addr method.
 */
TEST (Endpoint, addr)
{
    UnixDgram::Endpoint unixDgramEndpoint;
    ASSERT_NE (unixDgramEndpoint.addr (), nullptr);

    UnixStream::Endpoint unixStreamEndpoint;
    ASSERT_NE (unixStreamEndpoint.addr (), nullptr);

    Raw::Endpoint rawEndpoint;
    ASSERT_NE (rawEndpoint.addr (), nullptr);

    Udp::Endpoint udpEndpoint;
    ASSERT_NE (udpEndpoint.addr (), nullptr);

    Icmp::Endpoint icmpEndpoint;
    ASSERT_NE (icmpEndpoint.addr (), nullptr);

    Tcp::Endpoint tcpEndpoint;
    ASSERT_NE (tcpEndpoint.addr (), nullptr);

    Tls::Endpoint tlsEndpoint;
    ASSERT_NE (tlsEndpoint.addr (), nullptr);
}

/**
 * @brief test the length method.
 */
TEST (Endpoint, length)
{
    UnixDgram::Endpoint unixDgramEndpoint;
    ASSERT_EQ (unixDgramEndpoint.length (), sizeof (struct sockaddr_un));

    UnixStream::Endpoint unixStreamEndpoint;
    ASSERT_EQ (unixStreamEndpoint.length (), sizeof (struct sockaddr_un));

    Raw::Endpoint rawEndpoint;
    ASSERT_EQ (rawEndpoint.length (), sizeof (struct sockaddr_ll));

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

    Tls::Endpoint tlsEndpoint4 (Tls::v4 ());
    ASSERT_EQ (tlsEndpoint4.length (), sizeof (struct sockaddr_in));

    Tls::Endpoint tlsEndpoint6 (Tls::v6 ());
    ASSERT_EQ (tlsEndpoint6.length (), sizeof (struct sockaddr_in6));
}

/**
 * @brief test the device method.
 */
TEST (Endpoint, device)
{
    UnixDgram::Endpoint unixDgramEndpoint;
    unixDgramEndpoint.device ("/path/to/file");
    ASSERT_EQ (unixDgramEndpoint.device (), "/path/to/file");

    UnixStream::Endpoint unixStreamEndpoint;
    unixStreamEndpoint.device ("/path/to/other");
    ASSERT_EQ (unixStreamEndpoint.device (), "/path/to/other");

    Raw::Endpoint rawEndpoint;
    rawEndpoint.device ("lo");
    ASSERT_EQ (rawEndpoint.device (), "lo");

    Udp::Endpoint udpEndpoint (Udp::v6 ());
    udpEndpoint.device ("lo");
    ASSERT_EQ (udpEndpoint.device (), "lo");

    Icmp::Endpoint icmpEndpoint (Icmp::v6 ());
    icmpEndpoint.device ("lo");
    ASSERT_EQ (icmpEndpoint.device (), "lo");

    Tcp::Endpoint tcpEndpoint (Tcp::v6 ());
    tcpEndpoint.device ("lo");
    ASSERT_EQ (tcpEndpoint.device (), "lo");

    Tls::Endpoint tlsEndpoint (Tls::v6 ());
    tlsEndpoint.device ("lo");
    ASSERT_EQ (tlsEndpoint.device (), "lo");
}

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

    Tls::Endpoint tlsEndpoint;

    tlsEndpoint.ip ("::");
    ASSERT_EQ (tlsEndpoint.ip (), "::");
    tlsEndpoint.ip ("127.0.0.1");
    ASSERT_EQ (tlsEndpoint.ip (), "127.0.0.1");
}

/**
 * @brief port method.
 */
TEST (Endpoint, port)
{
    Udp::Endpoint udpEndpoint;

    udpEndpoint.port (5000);
    ASSERT_EQ (udpEndpoint.port (), 5000);

    Tcp::Endpoint tcpEndpoint;

    tcpEndpoint.port (80);
    ASSERT_EQ (tcpEndpoint.port (), 80);

    Tls::Endpoint tlsEndpoint;

    tlsEndpoint.port (80);
    ASSERT_EQ (tlsEndpoint.port (), 80);
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

    ASSERT_EQ (Tls::Endpoint ().protocol (), Tls::v4 ());
    ASSERT_EQ (Tls::Endpoint (Tls::v4 ()).protocol (), Tls::v4 ());
    ASSERT_NE (Tls::Endpoint (Tls::v4 ()).protocol (), Tls::v6 ());
    ASSERT_EQ (Tls::Endpoint (Tls::v6 ()).protocol (), Tls::v6 ());
    ASSERT_EQ (Tls::Endpoint ("127.0.0.1").protocol (), Tls::v4 ());
    ASSERT_NE (Tls::Endpoint ("127.0.0.1").protocol (), Tls::v6 ());
    ASSERT_NE (Tls::Endpoint ("::").protocol (), Tls::v4 ());
    ASSERT_EQ (Tls::Endpoint ("::").protocol (), Tls::v6 ());
}

/**
 * @brief equal method.
 */
TEST (Endpoint, equal)
{
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

    ASSERT_EQ (Tls::Endpoint ("127.0.0.1", 80), Tls::Endpoint ("127.0.0.1", 80));
    ASSERT_NE (Tls::Endpoint ("127.0.0.1", 80), Tls::Endpoint ("fe80::57f3:baa4:fc3a:890a", 443));
    ASSERT_EQ (Tls::Endpoint ("fe80::57f3:baa4:fc3a:890a", 443), Tls::Endpoint ("fe80::57f3:baa4:fc3a:890a", 443));
    ASSERT_NE (Tls::Endpoint ("fe80::57f3:baa4:fc3a:890a", 443), Tls::Endpoint ("127.0.0.1", 80));
}

/**
 * @brief thest the serialize method.
 */
TEST (Endpoint, serialize)
{
    std::stringstream stream;
    UnixDgram::Endpoint unixDgramEndpoint ("lo");
    ASSERT_NO_THROW (stream << unixDgramEndpoint);
    ASSERT_EQ (stream.str (), "lo");

    stream.str ("");
    UnixStream::Endpoint unixStreamEndpoint ("lo");
    ASSERT_NO_THROW (stream << unixStreamEndpoint);
    ASSERT_EQ (stream.str (), "lo");

    stream.str ("");
    Raw::Endpoint rawEndpoint ("lo");
    ASSERT_NO_THROW (stream << rawEndpoint);
    ASSERT_EQ (stream.str (), "lo");

    stream.str ("");
    Udp::Endpoint udpEndpoint ("127.0.0.1", 80);
    ASSERT_NO_THROW (stream << udpEndpoint);
    ASSERT_EQ (stream.str (), "127.0.0.1:80");

    stream.str ("");
    Icmp::Endpoint icmpEndpoint ("127.0.0.1");
    ASSERT_NO_THROW (stream << icmpEndpoint);
    ASSERT_EQ (stream.str (), "127.0.0.1");

    stream.str ("");
    Tcp::Endpoint tcpEndpoint ("127.0.0.1", 80);
    ASSERT_NO_THROW (stream << tcpEndpoint);
    ASSERT_EQ (stream.str (), "127.0.0.1:80");

    stream.str ("");
    Tls::Endpoint tlsEndpoint ("127.0.0.1", 80);
    ASSERT_NO_THROW (stream << tlsEndpoint);
    ASSERT_EQ (stream.str (), "127.0.0.1:80");
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
