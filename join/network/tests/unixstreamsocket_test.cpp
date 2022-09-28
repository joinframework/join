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
using join::UnixStream;
/**
 * @brief Class used to test the unix stream socket API.
 */
class UnixStreamSocket : public ::testing::Test, public UnixStream::Acceptor::Observer
{
protected:
    /**
     * @brief Sets up the test fixture.
     */
    void SetUp ()
    {
        ASSERT_EQ (bind (_serverpath), 0) << join::lastError.message ();
        ASSERT_EQ (listen (), 0) << join::lastError.message ();
        ASSERT_EQ (start (), 0) << join::lastError.message ();
    }

    /**
     * @brief Tears down the test fixture.
     */
    void TearDown ()
    {
        ASSERT_EQ (stop (), 0) << join::lastError.message ();
        close ();
    }

        /**
     * @brief method called on receive.
     */
    virtual void onReceive () override
    {
        UnixStream::Socket sock = accept ();
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

    /// path.
    static const std::string _serverpath;
    static const std::string _clientpath;

    /// timeout.
    static const int _timeout;
};

const std::string UnixStreamSocket::_serverpath = "/tmp/unixserver_test.sock";
const std::string UnixStreamSocket::_clientpath = "/tmp/unixclient_test.sock";
const int         UnixStreamSocket::_timeout = 1000;

/**
 * @brief Test open method.
 */
TEST_F (UnixStreamSocket, open)
{
    UnixStream::Socket unixSocket;

    ASSERT_EQ (unixSocket.open (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.open (), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    unixSocket.close ();
}

/**
 * @brief Test close method.
 */
TEST_F (UnixStreamSocket, close)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);

    ASSERT_FALSE (unixSocket.opened ());
    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.opened());
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_FALSE (unixSocket.opened());
    unixSocket.close ();
    ASSERT_FALSE (unixSocket.opened());
}

/**
 * @brief Test bind method.
 */
TEST_F (UnixStreamSocket, bind)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);

    ASSERT_EQ (unixSocket.bind (_clientpath), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();

    unixSocket.close ();
}

/**
 * @brief Test connect method.
 */
TEST_F (UnixStreamSocket, connect)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);

    ASSERT_EQ (unixSocket.connect (""), -1);

    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.connect (_serverpath), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    unixSocket.close ();
}

/**
 * @brief Test waitConnected method.
 */
TEST_F (UnixStreamSocket, waitConnected)
{
    UnixStream::Socket unixSocket;

    ASSERT_FALSE (unixSocket.waitConnected (_timeout));
    if (unixSocket.connect (_serverpath) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
        ASSERT_TRUE (unixSocket.connecting ());
    }
    ASSERT_TRUE (unixSocket.waitConnected (_timeout)) << join::lastError.message ();
    if (unixSocket.disconnect () == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (unixSocket.waitDisconnected (_timeout)) << join::lastError.message ();
    unixSocket.close ();
}

/**
 * @brief Test disconnect method.
 */
TEST_F (UnixStreamSocket, disconnect)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);

    ASSERT_FALSE (unixSocket.connected ());
    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.connected ());
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_FALSE (unixSocket.connected ());
    unixSocket.close ();
    ASSERT_FALSE (unixSocket.connected ());
}

/**
 * @brief Test waitDisconnected method.
 */
TEST_F (UnixStreamSocket, waitDisconnected)
{
    UnixStream::Socket unixSocket;

    if (unixSocket.connect (_serverpath) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
        ASSERT_TRUE (unixSocket.connecting ());
    }
    ASSERT_TRUE (unixSocket.waitConnected (_timeout)) << join::lastError.message ();
    ASSERT_FALSE (unixSocket.waitDisconnected (_timeout));
    if (unixSocket.disconnect () == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (unixSocket.waitDisconnected (_timeout)) << join::lastError.message ();
    unixSocket.close ();
}

/**
 * @brief Test canRead method.
 */
TEST_F (UnixStreamSocket, canRead)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (unixSocket.canRead (), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.writeExactly (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_GT (unixSocket.canRead (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    unixSocket.close ();
}

/**
 * @brief Test waitReadyRead method.
 */
TEST_F (UnixStreamSocket, waitReadyRead)
{
    UnixStream::Socket unixSocket;
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_FALSE (unixSocket.waitReadyRead (_timeout));
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    if (unixSocket.connect (_serverpath) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (unixSocket.waitConnected (_timeout)) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.writeExactly (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    if (unixSocket.disconnect () == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (unixSocket.waitDisconnected (_timeout)) << join::lastError.message ();
    unixSocket.close ();
}

/**
 * @brief Test read method.
 */
TEST_F (UnixStreamSocket, read)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (unixSocket.read (data, sizeof (data)), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.writeExactly (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_GT (unixSocket.read (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    unixSocket.close ();
}

/**
 * @brief Test readExactly method.
 */
TEST_F (UnixStreamSocket, readExactly)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.writeExactly (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.readExactly (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    unixSocket.close ();
}

/**
 * @brief Test waitReadyWrite method.
 */
TEST_F (UnixStreamSocket, waitReadyWrite)
{
    UnixStream::Socket unixSocket;

    ASSERT_FALSE (unixSocket.waitReadyWrite (_timeout));
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    if (unixSocket.connect (_serverpath) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (unixSocket.waitConnected (_timeout)) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    if (unixSocket.disconnect () == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (unixSocket.waitDisconnected (_timeout)) << join::lastError.message ();
    unixSocket.close ();
}

/**
 * @brief Test write method.
 */
TEST_F (UnixStreamSocket, write)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (unixSocket.write (data, sizeof (data)), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_GT (unixSocket.write (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    unixSocket.close ();
}

/**
 * @brief Test writeExactly method.
 */
TEST_F (UnixStreamSocket, writeExactly)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.writeExactly (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    unixSocket.close ();
}

/**
 * @brief Test setMode method.
 */
TEST_F (UnixStreamSocket, setMode)
{
    UnixStream::Socket unixSocket;

    ASSERT_EQ (unixSocket.setMode (UnixStream::Socket::Blocking), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.setMode (UnixStream::Socket::NonBlocking), 0);
    if (unixSocket.disconnect () == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (unixSocket.waitDisconnected (_timeout)) << join::lastError.message ();
    unixSocket.close ();
}

/**
 * @brief Test setOption method.
 */
TEST_F (UnixStreamSocket, setOption)
{
    UnixStream::Socket unixSocket;

    ASSERT_EQ (unixSocket.setOption (UnixStream::Socket::RcvBuffer, 1500), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (unixSocket.open (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.setOption (UnixStream::Socket::NoDelay, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (unixSocket.setOption (UnixStream::Socket::KeepAlive, 1), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.setOption (UnixStream::Socket::KeepIdle, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (unixSocket.setOption (UnixStream::Socket::KeepIntvl, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (unixSocket.setOption (UnixStream::Socket::KeepCount, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (unixSocket.setOption (UnixStream::Socket::SndBuffer, 1500), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.setOption (UnixStream::Socket::RcvBuffer, 1500), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.setOption (UnixStream::Socket::TimeStamp, 1), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.setOption (UnixStream::Socket::ReuseAddr, 1), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.setOption (UnixStream::Socket::ReusePort, 1), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.setOption (UnixStream::Socket::Broadcast, 1), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.setOption (UnixStream::Socket::Ttl, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (unixSocket.setOption (UnixStream::Socket::MulticastLoop, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (unixSocket.setOption (UnixStream::Socket::MulticastTtl, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (unixSocket.setOption (UnixStream::Socket::PathMtuDiscover, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (unixSocket.setOption (UnixStream::Socket::RcvError, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (unixSocket.setOption (UnixStream::Socket::AuxData, 1), -1);
    ASSERT_EQ (join::lastError, std::errc::operation_not_supported);
    unixSocket.close ();
}

/**
 * @brief Test localEndpoint method.
 */
TEST_F (UnixStreamSocket, localEndpoint)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);

    ASSERT_EQ (unixSocket.localEndpoint (), UnixStream::Endpoint {});
    ASSERT_EQ (unixSocket.bind (_clientpath), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.localEndpoint (), UnixStream::Endpoint (_clientpath)) << join::lastError.message ();
    unixSocket.close ();
}

/**
 * @brief Test mtu method.
 */
TEST_F (UnixStreamSocket, remoteEndpoint)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);

    ASSERT_EQ (unixSocket.remoteEndpoint (), UnixStream::Endpoint {});
    ASSERT_EQ (unixSocket.bind (_clientpath), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.remoteEndpoint (), UnixStream::Endpoint (_serverpath)) << join::lastError.message ();
    unixSocket.close ();
}

/**
 * @brief Test opened method.
 */
TEST_F (UnixStreamSocket, opened)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);

    ASSERT_FALSE (unixSocket.opened ());
    ASSERT_EQ (unixSocket.open (), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.opened ());
    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.opened ());
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_FALSE (unixSocket.opened ());
    unixSocket.close ();
    ASSERT_FALSE (unixSocket.opened ());
}

/**
 * @brief Test connected method.
 */
TEST_F (UnixStreamSocket, connected)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);

    ASSERT_FALSE (unixSocket.connected());
    ASSERT_EQ (unixSocket.open (), 0) << join::lastError.message ();
    ASSERT_FALSE (unixSocket.connected());
    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.connected());
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_FALSE (unixSocket.connected());
    unixSocket.close ();
    ASSERT_FALSE (unixSocket.connected());
}

/**
 * @brief Test encrypted method.
 */
TEST_F (UnixStreamSocket, encrypted)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);

    ASSERT_FALSE (unixSocket.encrypted ());
    ASSERT_EQ (unixSocket.open (), 0) << join::lastError.message ();
    ASSERT_FALSE (unixSocket.encrypted ());
    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_FALSE (unixSocket.encrypted ());
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_FALSE (unixSocket.encrypted ());
    unixSocket.close ();
    ASSERT_FALSE (unixSocket.encrypted ());
}

/**
 * @brief Test family method.
 */
TEST_F (UnixStreamSocket, family)
{
    UnixStream::Socket unixSocket;

    ASSERT_EQ (unixSocket.family (), AF_UNIX);
}

/**
 * @brief Test type method.
 */
TEST_F (UnixStreamSocket, type)
{
    UnixStream::Socket unixSocket;

    ASSERT_EQ (unixSocket.type (), SOCK_STREAM);
}

/**
 * @brief Test protocol method.
 */
TEST_F (UnixStreamSocket, protocol)
{
    UnixStream::Socket unixSocket;

    ASSERT_EQ (unixSocket.protocol (), 0);
}

/**
 * @brief Test handle method.
 */
TEST_F (UnixStreamSocket, handle)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);

    ASSERT_EQ (unixSocket.handle (), -1);
    ASSERT_EQ (unixSocket.open (), 0) << join::lastError.message ();
    ASSERT_GT (unixSocket.handle (), -1);
    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_GT (unixSocket.handle (), -1);
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.handle (), -1);
    unixSocket.close ();
    ASSERT_EQ (unixSocket.handle (), -1);
}

/**
 * @brief Test mtu method.
 */
TEST_F (UnixStreamSocket, mtu)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);

    ASSERT_EQ (unixSocket.mtu (), -1);
    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.mtu (), -1);
    unixSocket.close ();
    ASSERT_EQ (unixSocket.mtu (), -1);
}

/**
 * @brief Test checksum method.
 */
TEST_F (UnixStreamSocket, checksum)
{
    std::string buffer ({'\xD2', '\xB6', '\x69', '\xFD', '\x2E'});

    ASSERT_EQ (UnixStream::Socket::checksum (reinterpret_cast <uint16_t *> (&buffer[0]), buffer.size (), 0), 19349);
}

/**
 * @brief Test lower method.
 */
TEST_F (UnixStreamSocket, lower)
{
    UnixStream::Socket unixSocket1, unixSocket2;

    ASSERT_EQ (unixSocket1.open (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket2.open (), 0) << join::lastError.message ();
    if (unixSocket1.handle() < unixSocket2.handle())
    {
        ASSERT_TRUE (unixSocket1 < unixSocket2);
    }
    else
    {
        ASSERT_TRUE (unixSocket2 < unixSocket1);
    }
    unixSocket1.close ();
    unixSocket2.close ();
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
