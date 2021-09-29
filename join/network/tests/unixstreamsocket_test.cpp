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
class UnixSocket : public ::testing::Test, public UnixStream::Acceptor::Observer
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
        ASSERT_EQ (close (), 0) << join::lastError.message ();
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

const std::string UnixSocket::_serverpath = "/tmp/unixserver_test.sock";
const std::string UnixSocket::_clientpath = "/tmp/unixclient_test.sock";
const int         UnixSocket::_timeout = 1000;

/**
 * @brief Test open method.
 */
TEST_F (UnixSocket, open)
{
    UnixStream::Socket unixSocket;

    ASSERT_EQ (unixSocket.open (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test close method.
 */
TEST_F (UnixSocket, close)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);

    ASSERT_FALSE (unixSocket.opened ());
    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.opened());
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_FALSE (unixSocket.opened());
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
    ASSERT_FALSE (unixSocket.opened());
}

/**
 * @brief Test bind method.
 */
TEST_F (UnixSocket, bind)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);

    ASSERT_EQ (unixSocket.bind (_clientpath), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test connect method.
 */
TEST_F (UnixSocket, connect)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);

    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test waitConnected method.
 */
TEST_F (UnixSocket, waitConnected)
{
    UnixStream::Socket unixSocket;

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
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test disconnect method.
 */
TEST_F (UnixSocket, disconnect)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);

    ASSERT_FALSE (unixSocket.connected ());
    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.connected ());
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_FALSE (unixSocket.connected ());
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
    ASSERT_FALSE (unixSocket.connected ());
}

/**
 * @brief Test waitDisconnected method.
 */
TEST_F (UnixSocket, waitDisconnected)
{
    UnixStream::Socket unixSocket;

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
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test canRead method.
 */
TEST_F (UnixSocket, canRead)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.writeExactly (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_GT (unixSocket.canRead (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test waitReadyRead method.
 */
TEST_F (UnixSocket, waitReadyRead)
{
    UnixStream::Socket unixSocket;
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

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
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test read method.
 */
TEST_F (UnixSocket, read)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.writeExactly (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_GT (unixSocket.read (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test readExactly method.
 */
TEST_F (UnixSocket, readExactly)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.writeExactly (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.readExactly (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test waitReadyWrite method.
 */
TEST_F (UnixSocket, waitReadyWrite)
{
    UnixStream::Socket unixSocket;

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
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test write method.
 */
TEST_F (UnixSocket, write)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_GT (unixSocket.write (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test writeExactly method.
 */
TEST_F (UnixSocket, writeExactly)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.writeExactly (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test setMode method.
 */
TEST_F (UnixSocket, setMode)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);

    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.setMode (UnixStream::Socket::NonBlocking), 0);
    if (unixSocket.disconnect () == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (unixSocket.waitDisconnected (_timeout)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test setOption method.
 */
TEST_F (UnixSocket, setOption)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);

    ASSERT_EQ (unixSocket.setOption (UnixStream::Socket::SndBuffer, 1500), -1);
    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.setOption (UnixStream::Socket::SndBuffer, 1500), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test localEndpoint method.
 */
TEST_F (UnixSocket, localEndpoint)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);

    ASSERT_EQ (unixSocket.bind (_clientpath), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.localEndpoint (), UnixStream::Endpoint (_clientpath)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test mtu method.
 */
TEST_F (UnixSocket, remoteEndpoint)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);

    ASSERT_EQ (unixSocket.bind (_clientpath), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.remoteEndpoint (), UnixStream::Endpoint (_serverpath)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test opened method.
 */
TEST_F (UnixSocket, opened)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);

    ASSERT_FALSE (unixSocket.opened ());
    ASSERT_EQ (unixSocket.open (), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.opened ());
    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.opened ());
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_FALSE (unixSocket.opened ());
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
    ASSERT_FALSE (unixSocket.opened ());
}

/**
 * @brief Test connected method.
 */
TEST_F (UnixSocket, connected)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);

    ASSERT_FALSE (unixSocket.connected());
    ASSERT_EQ (unixSocket.open (), 0) << join::lastError.message ();
    ASSERT_FALSE (unixSocket.connected());
    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.connected());
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_FALSE (unixSocket.connected());
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
    ASSERT_FALSE (unixSocket.connected());
}

/**
 * @brief Test encrypted method.
 */
TEST_F (UnixSocket, encrypted)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);

    ASSERT_FALSE (unixSocket.encrypted ());
    ASSERT_EQ (unixSocket.open (), 0) << join::lastError.message ();
    ASSERT_FALSE (unixSocket.encrypted ());
    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_FALSE (unixSocket.encrypted ());
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_FALSE (unixSocket.encrypted ());
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
    ASSERT_FALSE (unixSocket.encrypted ());
}

/**
 * @brief Test family method.
 */
TEST_F (UnixSocket, family)
{
    UnixStream::Socket unixSocket;

    ASSERT_EQ (unixSocket.family (), AF_UNIX);
}

/**
 * @brief Test type method.
 */
TEST_F (UnixSocket, type)
{
    UnixStream::Socket unixSocket;

    ASSERT_EQ (unixSocket.type (), SOCK_STREAM);
}

/**
 * @brief Test protocol method.
 */
TEST_F (UnixSocket, protocol)
{
    UnixStream::Socket unixSocket;

    ASSERT_EQ (unixSocket.protocol (), 0);
}

/**
 * @brief Test handle method.
 */
TEST_F (UnixSocket, handle)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);

    ASSERT_EQ (unixSocket.handle (), -1);
    ASSERT_EQ (unixSocket.open (), 0) << join::lastError.message ();
    ASSERT_GT (unixSocket.handle (), -1);
    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_GT (unixSocket.handle (), -1);
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.handle (), -1);
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.handle (), -1);
}

/**
 * @brief Test mtu method.
 */
TEST_F (UnixSocket, mtu)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);

    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.mtu (), -1);
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test lower method.
 */
TEST_F (UnixSocket, lower)
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
    ASSERT_EQ (unixSocket1.close (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket2.close (), 0) << join::lastError.message ();
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
