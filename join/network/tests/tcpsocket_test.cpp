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
#include <join/acceptor.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::Errc;
using join::IpAddress;
using join::Resolver;
using join::Reactor;
using join::Tcp;

/**
 * @brief Class used to test the TCP socket API.
 */
class TcpSocket : public join::EventHandler, public ::testing::Test
{
protected:
    /**
     * @brief Sets up the test fixture.
     */
    void SetUp ()
    {
        ASSERT_EQ (_acceptor.bind ({Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
        ASSERT_EQ (_acceptor.listen (), 0) << join::lastError.message ();
        ASSERT_EQ (Reactor::instance ()->addHandler (this), 0) << join::lastError.message ();
    }

    /**
     * @brief Tears down the test fixture.
     */
    void TearDown ()
    {
        ASSERT_EQ (Reactor::instance ()->delHandler (this), 0) << join::lastError.message ();
        _acceptor.close ();
    }

    /**
     * @brief method called when data are ready to be read on handle.
     */
    virtual void onReceive () override
    {
        Tcp::Socket sock = _acceptor.accept ();
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
                        if (sock.waitReadyRead (_timeout))
                            continue;
                    }
                    break;
                }
                sock.writeExactly (buf, nread);
            }
            sock.close ();
        }
    }

    /**
     * @brief method called when handle is closed.
     */
    virtual void onClose () override
    {
        // do nothing.
    }

    /**
     * @brief method called when an error occured on handle.
     */
    virtual void onError () override
    {
        // do nothing.
    }

    /**
     * @brief get native handle.
     * @return native handle.
     */
    virtual int handle () const override
    {
        return _acceptor.handle ();
    }

    /// server socket.
    static Tcp::Acceptor _acceptor;

    /// host.
    static const std::string _host;

    /// port.
    static const uint16_t _port;

    /// timeout.
    static const int _timeout;
};

Tcp::Acceptor     TcpSocket::_acceptor;
const std::string TcpSocket::_host    = "localhost";
const uint16_t    TcpSocket::_port    = 5000;
const int         TcpSocket::_timeout = 1000;

/**
 * @brief Test move.
 */
TEST_F (TcpSocket, move)
{
    Tcp::Socket tcpSocket1 (Tcp::Socket::Blocking), tcpSocket2;

    ASSERT_EQ (tcpSocket1.connect ({Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket1.connected());
    tcpSocket2 = std::move (tcpSocket1);
    ASSERT_TRUE (tcpSocket2.connected());
    tcpSocket2.close ();
}

/**
 * @brief Test open method.
 */
TEST_F (TcpSocket, open)
{
    Tcp::Socket tcpSocket;

    ASSERT_EQ (tcpSocket.open (Tcp::v4 ()), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.open (Tcp::v4 ()), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    tcpSocket.close ();

    ASSERT_EQ (tcpSocket.open (Tcp::v6 ()), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.open (Tcp::v6 ()), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    tcpSocket.close ();
}

/**
 * @brief Test close method.
 */
TEST_F (TcpSocket, close)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);

    ASSERT_FALSE (tcpSocket.opened ());
    ASSERT_EQ (tcpSocket.connect ({Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.opened());
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_FALSE (tcpSocket.opened());
    tcpSocket.close ();
    ASSERT_FALSE (tcpSocket.opened());
}

/**
 * @brief Test bind method.
 */
TEST_F (TcpSocket, bind)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);

    ASSERT_EQ (tcpSocket.connect ({Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.bind (_host), -1);
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();

    ASSERT_EQ (tcpSocket.bind (_host), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.connect ({Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();

    tcpSocket.close ();
}

/**
 * @brief Test connect method.
 */
TEST_F (TcpSocket, connect)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);

    ASSERT_EQ (tcpSocket.connect ({"255.255.255.255", _port}), -1);

    ASSERT_EQ (tcpSocket.connect ({Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.connect ({Resolver::resolveHost (_host), _port}), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    tcpSocket.close ();

    ASSERT_EQ (tcpSocket.connect (_host + ":" + std::to_string (_port)), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.connect (_host + ":" + std::to_string (_port)), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    tcpSocket.close ();
}

/**
 * @brief Test waitConnected method.
 */
TEST_F (TcpSocket, waitConnected)
{
    Tcp::Socket tcpSocket;

    ASSERT_FALSE (tcpSocket.waitConnected (_timeout));
    if (tcpSocket.connect ({Resolver::resolveHost (_host), _port}) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
        ASSERT_TRUE (tcpSocket.connecting ());
    }
    ASSERT_TRUE (tcpSocket.waitConnected (_timeout)) << join::lastError.message ();
    if (tcpSocket.disconnect () == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (tcpSocket.waitDisconnected (_timeout)) << join::lastError.message ();
    tcpSocket.close ();
}

/**
 * @brief Test disconnect method.
 */
TEST_F (TcpSocket, disconnect)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);

    ASSERT_FALSE (tcpSocket.connected ());
    ASSERT_EQ (tcpSocket.connect ({Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.connected ());
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_FALSE (tcpSocket.connected ());
    tcpSocket.close ();
    ASSERT_FALSE (tcpSocket.connected ());
}

/**
 * @brief Test waitDisconnected method.
 */
TEST_F (TcpSocket, waitDisconnected)
{
    Tcp::Socket tcpSocket;

    if (tcpSocket.connect ({Resolver::resolveHost (_host), _port}) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
        ASSERT_TRUE (tcpSocket.connecting ());
    }
    ASSERT_TRUE (tcpSocket.waitConnected (_timeout)) << join::lastError.message ();
    ASSERT_FALSE (tcpSocket.waitDisconnected (_timeout));
    if (tcpSocket.disconnect () == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (tcpSocket.waitDisconnected (_timeout)) << join::lastError.message ();
    tcpSocket.close ();
}

/**
 * @brief Test canRead method.
 */
TEST_F (TcpSocket, canRead)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (tcpSocket.canRead (), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (tcpSocket.connect ({Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.writeExactly (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_GT (tcpSocket.canRead (), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    tcpSocket.close ();
}

/**
 * @brief Test waitReadyRead method.
 */
TEST_F (TcpSocket, waitReadyRead)
{
    Tcp::Socket tcpSocket;
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_FALSE (tcpSocket.waitReadyRead (_timeout));
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    if (tcpSocket.connect ({Resolver::resolveHost (_host), _port}) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (tcpSocket.waitConnected (_timeout)) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.writeExactly (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    if (tcpSocket.disconnect () == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (tcpSocket.waitDisconnected (_timeout)) << join::lastError.message ();
    tcpSocket.close ();
}

/**
 * @brief Test read method.
 */
TEST_F (TcpSocket, read)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (tcpSocket.read (data, sizeof (data)), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (tcpSocket.connect ({Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.writeExactly (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_GT (tcpSocket.read (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    tcpSocket.close ();
}

/**
 * @brief Test readExactly method.
 */
TEST_F (TcpSocket, readExactly)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (tcpSocket.connect ({Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.writeExactly (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.readExactly (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    tcpSocket.close ();
}

/**
 * @brief Test waitReadyWrite method.
 */
TEST_F (TcpSocket, waitReadyWrite)
{
    Tcp::Socket tcpSocket;

    ASSERT_FALSE (tcpSocket.waitReadyWrite (_timeout));
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    if (tcpSocket.connect ({Resolver::resolveHost (_host), _port}) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (tcpSocket.waitConnected (_timeout)) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    if (tcpSocket.disconnect () == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (tcpSocket.waitDisconnected (_timeout)) << join::lastError.message ();
    tcpSocket.close ();
}

/**
 * @brief Test write method.
 */
TEST_F (TcpSocket, write)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (tcpSocket.write (data, sizeof (data)), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (tcpSocket.connect ({Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_GT (tcpSocket.write (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    tcpSocket.close ();
}

/**
 * @brief Test writeExactly method.
 */
TEST_F (TcpSocket, writeExactly)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (tcpSocket.connect ({Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.writeExactly (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    tcpSocket.close ();
}

/**
 * @brief Test setMode method.
 */
TEST_F (TcpSocket, setMode)
{
    Tcp::Socket tcpSocket;

    ASSERT_EQ (tcpSocket.setMode (Tcp::Socket::Blocking), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.connect ({Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.setMode (Tcp::Socket::NonBlocking), 0);
    if (tcpSocket.disconnect () == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (tcpSocket.waitDisconnected (_timeout)) << join::lastError.message ();
    tcpSocket.close ();
}

/**
 * @brief Test setOption method.
 */
TEST_F (TcpSocket, setOption)
{
    Tcp::Socket tcpSocket;

    ASSERT_EQ (tcpSocket.setOption (Tcp::Socket::RcvBuffer, 1500), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (tcpSocket.open (), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.setOption (Tcp::Socket::NoDelay, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.setOption (Tcp::Socket::KeepAlive, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.setOption (Tcp::Socket::KeepIdle, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.setOption (Tcp::Socket::KeepIntvl, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.setOption (Tcp::Socket::KeepCount, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.setOption (Tcp::Socket::SndBuffer, 1500), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.setOption (Tcp::Socket::RcvBuffer, 1500), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.setOption (Tcp::Socket::TimeStamp, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.setOption (Tcp::Socket::ReuseAddr, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.setOption (Tcp::Socket::ReusePort, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.setOption (Tcp::Socket::Broadcast, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.setOption (Tcp::Socket::Ttl, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.setOption (Tcp::Socket::MulticastLoop, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.setOption (Tcp::Socket::MulticastTtl, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (tcpSocket.setOption (Tcp::Socket::PathMtuDiscover, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.setOption (Tcp::Socket::RcvError, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.setOption (Tcp::Socket::AuxData, 1), -1);
    ASSERT_EQ (join::lastError, std::errc::no_protocol_option);
    tcpSocket.close ();

    ASSERT_EQ (tcpSocket.open (Tcp::v6 ()), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.setOption (Tcp::Socket::NoDelay, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.setOption (Tcp::Socket::KeepAlive, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.setOption (Tcp::Socket::KeepIdle, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.setOption (Tcp::Socket::KeepIntvl, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.setOption (Tcp::Socket::KeepCount, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.setOption (Tcp::Socket::SndBuffer, 1500), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.setOption (Tcp::Socket::RcvBuffer, 1500), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.setOption (Tcp::Socket::TimeStamp, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.setOption (Tcp::Socket::ReuseAddr, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.setOption (Tcp::Socket::ReusePort, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.setOption (Tcp::Socket::Broadcast, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.setOption (Tcp::Socket::Ttl, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.setOption (Tcp::Socket::MulticastLoop, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.setOption (Tcp::Socket::MulticastTtl, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (tcpSocket.setOption (Tcp::Socket::PathMtuDiscover, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.setOption (Tcp::Socket::RcvError, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.setOption (Tcp::Socket::AuxData, 1), -1);
    ASSERT_EQ (join::lastError, std::errc::no_protocol_option);
    tcpSocket.close ();
}

/**
 * @brief Test localEndpoint method.
 */
TEST_F (TcpSocket, localEndpoint)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);

    ASSERT_EQ (tcpSocket.localEndpoint (), Tcp::Endpoint {});
    ASSERT_EQ (tcpSocket.bind ({Resolver::resolveHost (_host), uint16_t (_port + 1)}), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.connect ({Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.localEndpoint (), Tcp::Endpoint (Resolver::resolveHost (_host), uint16_t (_port + 1))) << join::lastError.message ();
    tcpSocket.close ();
}

/**
 * @brief Test remoteEndpoint method.
 */
TEST_F (TcpSocket, remoteEndpoint)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);

    ASSERT_EQ (tcpSocket.remoteEndpoint (), Tcp::Endpoint {});
    ASSERT_EQ (tcpSocket.bind ({Resolver::resolveHost (_host), uint16_t (_port + 1)}), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.connect ({Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.remoteEndpoint (), Tcp::Endpoint (Resolver::resolveHost (_host), _port)) << join::lastError.message ();
    tcpSocket.close ();
}

/**
 * @brief Test opened method.
 */
TEST_F (TcpSocket, opened)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);

    ASSERT_FALSE (tcpSocket.opened ());
    ASSERT_EQ (tcpSocket.open (Resolver::resolveHost (_host).family ()), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.opened ());
    ASSERT_EQ (tcpSocket.connect ({Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.opened ());
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_FALSE (tcpSocket.opened ());
    tcpSocket.close ();
    ASSERT_FALSE (tcpSocket.opened ());
}

/**
 * @brief Test connected method.
 */
TEST_F (TcpSocket, connected)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);

    ASSERT_FALSE (tcpSocket.connected());
    ASSERT_EQ (tcpSocket.open (Resolver::resolveHost (_host).family ()), 0) << join::lastError.message ();
    ASSERT_FALSE (tcpSocket.connected());
    ASSERT_EQ (tcpSocket.connect ({Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.connected());
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_FALSE (tcpSocket.connected());
    tcpSocket.close ();
    ASSERT_FALSE (tcpSocket.connected());
}

/**
 * @brief Test encrypted method.
 */
TEST_F (TcpSocket, encrypted)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);

    ASSERT_FALSE (tcpSocket.encrypted ());
    ASSERT_EQ (tcpSocket.open (Resolver::resolveHost (_host).family ()), 0) << join::lastError.message ();
    ASSERT_FALSE (tcpSocket.encrypted ());
    ASSERT_EQ (tcpSocket.connect ({Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_FALSE (tcpSocket.encrypted ());
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_FALSE (tcpSocket.encrypted ());
    tcpSocket.close ();
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
    tcpSocket.close ();

    ASSERT_EQ (tcpSocket.bind (IpAddress (AF_INET)), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.family (), AF_INET);
    tcpSocket.close ();
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
    ASSERT_EQ (tcpSocket.open (Resolver::resolveHost (_host).family ()), 0) << join::lastError.message ();
    ASSERT_GT (tcpSocket.handle (), -1);
    ASSERT_EQ (tcpSocket.connect ({Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_GT (tcpSocket.handle (), -1);
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.handle (), -1);
    tcpSocket.close ();
    ASSERT_EQ (tcpSocket.handle (), -1);
}

/**
 * @brief Test mtu method.
 */
TEST_F (TcpSocket, mtu)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);

    ASSERT_EQ (tcpSocket.mtu (), -1);
    ASSERT_EQ (tcpSocket.connect ({Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_NE (tcpSocket.mtu (), -1) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.mtu (), -1);
    tcpSocket.close ();
    ASSERT_EQ (tcpSocket.mtu (), -1);
}

/**
 * @brief Test checksum method.
 */
TEST_F (TcpSocket, checksum)
{
    std::string buffer ({'\xD2', '\xB6', '\x69', '\xFD', '\x2E'});

    ASSERT_EQ (Tcp::Socket::checksum (reinterpret_cast <uint16_t *> (&buffer[0]), buffer.size (), 0), 19349);
}

/**
 * @brief Test lower method.
 */
TEST_F (TcpSocket, lower)
{
    Tcp::Socket tcpSocket1, tcpSocket2;

    ASSERT_EQ (tcpSocket1.open (Resolver::resolveHost (_host).family ()), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket2.open (Resolver::resolveHost (_host).family ()), 0) << join::lastError.message ();
    if (tcpSocket1.handle() < tcpSocket2.handle())
    {
        ASSERT_TRUE (tcpSocket1 < tcpSocket2);
    }
    else
    {
        ASSERT_TRUE (tcpSocket2 < tcpSocket1);
    }
    tcpSocket1.close ();
    tcpSocket2.close ();
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
