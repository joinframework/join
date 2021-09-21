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
#include <join/acceptor.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::Errc;
using join::IpAddress;
using join::Tcp;

/**
 * @brief Class used to test the TCP socket API.
 */
class TcpSocket : public ::testing::Test, public Tcp::Acceptor::Observer
{
protected:
    /**
     * @brief Sets up the test fixture.
     */
    void SetUp ()
    {
        ASSERT_EQ (bind ({Tcp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
        ASSERT_EQ (listen (), 0) << join::lastError.message ();
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
        Tcp::Socket sock = accept ();
        if (sock.connected ())
        {
            char buf[1024];
            for (;;)
            {
                // echo received data.
                int nread = sock.read (buf, sizeof (buf));
                if (nread == -1)
                {
                    if (join::lastError == Errc::TemporaryError)
                    {
                        if (sock.waitReadyRead (timeout_))
                            continue;
                    }
                    break;
                }
                sock.writeData (buf, nread);
            }
            sock.close ();
        }
    }

    /// host.
    static const std::string host_;

    /// port.
    static const uint16_t port_;

    /// timeout.
    static const int timeout_;
};

const std::string TcpSocket::host_    = "localhost";
const uint16_t    TcpSocket::port_    = 5000;
const int         TcpSocket::timeout_ = 1000;

/**
 * @brief Test open method.
 */
TEST_F (TcpSocket, open)
{
    Tcp::Socket tcpSocket;

    ASSERT_EQ (tcpSocket.open (Tcp::v4 ()), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.close (), 0) << join::lastError.message ();

    ASSERT_EQ (tcpSocket.open (Tcp::v6 ()), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test close method.
 */
TEST_F (TcpSocket, close)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);

    ASSERT_FALSE (tcpSocket.opened ());
    ASSERT_EQ (tcpSocket.connect ({Tcp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.opened());
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_FALSE (tcpSocket.opened());
    ASSERT_EQ (tcpSocket.close (), 0) << join::lastError.message ();
    ASSERT_FALSE (tcpSocket.opened());
}

/**
 * @brief Test bind method.
 */
TEST_F (TcpSocket, bind)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);

    ASSERT_EQ (tcpSocket.bind ({Tcp::Resolver::resolveHost (host_)}), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.connect ({Tcp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test connect method.
 */
TEST_F (TcpSocket, connect)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);

    ASSERT_EQ (tcpSocket.connect ({Tcp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.close (), 0) << join::lastError.message ();

    ASSERT_EQ (tcpSocket.connect (Tcp::Resolver::resolve (host_ + ":" + std::to_string (port_))), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test waitConnected method.
 */
TEST_F (TcpSocket, waitConnected)
{
    Tcp::Socket tcpSocket;

    if (tcpSocket.connect ({Tcp::Resolver::resolveHost (host_), port_}) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
        ASSERT_TRUE (tcpSocket.connecting ());
    }
    ASSERT_TRUE (tcpSocket.waitConnected (timeout_)) << join::lastError.message ();
    if (tcpSocket.disconnect () == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (tcpSocket.waitDisconnected (timeout_)) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test disconnect method.
 */
TEST_F (TcpSocket, disconnect)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);

    ASSERT_FALSE (tcpSocket.connected ());
    ASSERT_EQ (tcpSocket.connect ({Tcp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.connected ());
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_FALSE (tcpSocket.connected ());
    ASSERT_EQ (tcpSocket.close (), 0) << join::lastError.message ();
    ASSERT_FALSE (tcpSocket.connected ());
}

/**
 * @brief Test waitDisconnected method.
 */
TEST_F (TcpSocket, waitDisconnected)
{
    Tcp::Socket tcpSocket;

    if (tcpSocket.connect ({Tcp::Resolver::resolveHost (host_), port_}) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
        ASSERT_TRUE (tcpSocket.connecting ());
    }
    ASSERT_TRUE (tcpSocket.waitConnected (timeout_)) << join::lastError.message ();
    if (tcpSocket.disconnect () == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (tcpSocket.waitDisconnected (timeout_)) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test canRead method.
 */
TEST_F (TcpSocket, canRead)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (tcpSocket.connect ({Tcp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitReadyWrite (timeout_)) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.writeData (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitReadyRead (timeout_)) << join::lastError.message ();
    ASSERT_GT (tcpSocket.canRead (), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test waitReadyRead method.
 */
TEST_F (TcpSocket, waitReadyRead)
{
    Tcp::Socket tcpSocket;
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    if (tcpSocket.connect ({Tcp::Resolver::resolveHost (host_), port_}) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (tcpSocket.waitConnected (timeout_)) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitReadyWrite (timeout_)) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.writeData (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitReadyRead (timeout_)) << join::lastError.message ();
    if (tcpSocket.disconnect () == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (tcpSocket.waitDisconnected (timeout_)) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test read method.
 */
TEST_F (TcpSocket, read)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (tcpSocket.connect ({Tcp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitReadyWrite (timeout_)) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.writeData (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitReadyRead (timeout_)) << join::lastError.message ();
    ASSERT_GT (tcpSocket.read (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test readChar method.
 */
TEST_F (TcpSocket, readChar)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);
    char data;

    ASSERT_EQ (tcpSocket.connect ({Tcp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitReadyWrite (timeout_)) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.writeData ("b", 1), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitReadyRead (timeout_)) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.readChar (data), 0) << join::lastError.message ();
    ASSERT_EQ (data, 'b');
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test readLine method.
 */
TEST_F (TcpSocket, readLine)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);
    std::string data;

    ASSERT_EQ (tcpSocket.connect ({Tcp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitReadyWrite (timeout_)) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.writeData ("readLine\n", strlen ("readLine\n")), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitReadyRead (timeout_)) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.readLine (data, 1024), 0) << join::lastError.message ();
    ASSERT_EQ (data, "readLine");
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test readData method.
 */
TEST_F (TcpSocket, readData)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (tcpSocket.connect ({Tcp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitReadyWrite (timeout_)) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.writeData (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitReadyRead (timeout_)) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.readData (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test waitReadyWrite method.
 */
TEST_F (TcpSocket, waitReadyWrite)
{
    Tcp::Socket tcpSocket;

    if (tcpSocket.connect ({Tcp::Resolver::resolveHost (host_), port_}) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (tcpSocket.waitConnected (timeout_)) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitReadyWrite (timeout_)) << join::lastError.message ();
    if (tcpSocket.disconnect () == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (tcpSocket.waitDisconnected (timeout_)) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test write method.
 */
TEST_F (TcpSocket, write)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (tcpSocket.connect ({Tcp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitReadyWrite (timeout_)) << join::lastError.message ();
    ASSERT_GT (tcpSocket.write (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitReadyRead (timeout_)) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test writeData method.
 */
TEST_F (TcpSocket, writeData)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (tcpSocket.connect ({Tcp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitReadyWrite (timeout_)) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.writeData (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitReadyRead (timeout_)) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test setMode method.
 */
TEST_F (TcpSocket, setMode)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);

    ASSERT_EQ (tcpSocket.connect ({Tcp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.setMode (Tcp::Socket::NonBlocking), 0);
    if (tcpSocket.disconnect () == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (tcpSocket.waitDisconnected (timeout_)) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test setOption method.
 */
TEST_F (TcpSocket, setOption)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);

    ASSERT_EQ (tcpSocket.setOption (Tcp::Socket::NoDelay, 1), -1);
    ASSERT_EQ (tcpSocket.connect ({Tcp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.setOption (Tcp::Socket::NoDelay, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test localEndpoint method.
 */
TEST_F (TcpSocket, localEndpoint)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);

    ASSERT_EQ (tcpSocket.bind ({Tcp::Resolver::resolveHost (host_), uint16_t (port_ + 1)}), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.connect ({Tcp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.localEndpoint (), Tcp::Endpoint (Tcp::Resolver::resolveHost (host_), uint16_t (port_ + 1))) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test remoteEndpoint method.
 */
TEST_F (TcpSocket, remoteEndpoint)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);

    ASSERT_EQ (tcpSocket.bind ({Tcp::Resolver::resolveHost (host_), uint16_t (port_ + 1)}), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.connect ({Tcp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.remoteEndpoint (), Tcp::Endpoint (Tcp::Resolver::resolveHost (host_), port_)) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test opened method.
 */
TEST_F (TcpSocket, opened)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);

    ASSERT_FALSE (tcpSocket.opened ());
    ASSERT_EQ (tcpSocket.open (Tcp::Resolver::resolveHost (host_).family ()), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.opened ());
    ASSERT_EQ (tcpSocket.connect ({Tcp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.opened ());
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_FALSE (tcpSocket.opened ());
    ASSERT_EQ (tcpSocket.close (), 0) << join::lastError.message ();
    ASSERT_FALSE (tcpSocket.opened ());
}

/**
 * @brief Test connected method.
 */
TEST_F (TcpSocket, connected)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);

    ASSERT_FALSE (tcpSocket.connected());
    ASSERT_EQ (tcpSocket.open (Tcp::Resolver::resolveHost (host_).family ()), 0) << join::lastError.message ();
    ASSERT_FALSE (tcpSocket.connected());
    ASSERT_EQ (tcpSocket.connect ({Tcp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.connected());
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_FALSE (tcpSocket.connected());
    ASSERT_EQ (tcpSocket.close (), 0) << join::lastError.message ();
    ASSERT_FALSE (tcpSocket.connected());
}

/**
 * @brief Test encrypted method.
 */
TEST_F (TcpSocket, encrypted)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);

    ASSERT_FALSE (tcpSocket.encrypted ());
    ASSERT_EQ (tcpSocket.open (Tcp::Resolver::resolveHost (host_).family ()), 0) << join::lastError.message ();
    ASSERT_FALSE (tcpSocket.encrypted ());
    ASSERT_EQ (tcpSocket.connect ({Tcp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_FALSE (tcpSocket.encrypted ());
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_FALSE (tcpSocket.encrypted ());
    ASSERT_EQ (tcpSocket.close (), 0) << join::lastError.message ();
    ASSERT_FALSE (tcpSocket.encrypted ());
}

/**
 * @brief Test family method.
 */
TEST_F (TcpSocket, family)
{
    Tcp::Socket tcpSocket;

    ASSERT_EQ (tcpSocket.family (), AF_INET);

    ASSERT_EQ (tcpSocket.bind (IpAddress (AF_INET6)), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.family (), AF_INET6);
    ASSERT_EQ (tcpSocket.close (), 0) << join::lastError.message ();

    ASSERT_EQ (tcpSocket.bind (IpAddress (AF_INET)), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.family (), AF_INET);
    ASSERT_EQ (tcpSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test type method.
 */
TEST_F (TcpSocket, type)
{
    Tcp::Socket tcpSocket;

    ASSERT_EQ (tcpSocket.type (), SOCK_STREAM);
}

/**
 * @brief Test protocol method.
 */
TEST_F (TcpSocket, protocol)
{
    Tcp::Socket tcpSocket;

    ASSERT_EQ (tcpSocket.protocol (), IPPROTO_TCP);
}

/**
 * @brief Test handle method.
 */
TEST_F (TcpSocket, handle)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);

    ASSERT_EQ (tcpSocket.handle (), -1);
    ASSERT_EQ (tcpSocket.open (Tcp::Resolver::resolveHost (host_).family ()), 0) << join::lastError.message ();
    ASSERT_GT (tcpSocket.handle (), -1);
    ASSERT_EQ (tcpSocket.connect ({Tcp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_GT (tcpSocket.handle (), -1);
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.handle (), -1);
    ASSERT_EQ (tcpSocket.close (), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.handle (), -1);
}

/**
 * @brief Test mtu method.
 */
TEST_F (TcpSocket, mtu)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);

    ASSERT_EQ (tcpSocket.connect ({Tcp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_NE (tcpSocket.mtu (), -1) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.mtu (), -1);
    ASSERT_EQ (tcpSocket.close (), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.mtu (), -1);
}

/**
 * @brief Test randomize method.
 */
TEST_F (TcpSocket, randomize)
{
    ASSERT_GT (Tcp::Socket::randomize <int> (), 0);
}

/**
 * @brief Test lower method.
 */
TEST_F (TcpSocket, lower)
{
    Tcp::Socket tcpSocket1, tcpSocket2;

    ASSERT_EQ (tcpSocket1.open (Tcp::Resolver::resolveHost (host_).family ()), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket2.open (Tcp::Resolver::resolveHost (host_).family ()), 0) << join::lastError.message ();
    if (tcpSocket1.handle() < tcpSocket2.handle())
    {
        ASSERT_TRUE (tcpSocket1 < tcpSocket2);
    }
    else
    {
        ASSERT_TRUE (tcpSocket2 < tcpSocket1);
    }
    ASSERT_EQ (tcpSocket1.close (), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket2.close (), 0) << join::lastError.message ();
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
