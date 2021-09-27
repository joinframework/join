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
using join::Unix;
/**
 * @brief Class used to test the unix stream socket API.
 */
class UnixSocket : public ::testing::Test, public Unix::Acceptor::Observer
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
        Unix::Socket sock = accept ();
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
                sock.writeData (buf, nread);
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
    Unix::Socket unixSocket;

    ASSERT_EQ (unixSocket.open (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test close method.
 */
TEST_F (UnixSocket, close)
{
    Unix::Socket unixSocket (Unix::Socket::Blocking);

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
    Unix::Socket unixSocket (Unix::Socket::Blocking);

    ASSERT_EQ (unixSocket.bind (_clientpath), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test connect method.
 */
TEST_F (UnixSocket, connect)
{
    Unix::Socket unixSocket (Unix::Socket::Blocking);

    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test waitConnected method.
 */
TEST_F (UnixSocket, waitConnected)
{
    Unix::Socket unixSocket;

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
    Unix::Socket unixSocket (Unix::Socket::Blocking);

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
    Unix::Socket unixSocket;

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
    Unix::Socket unixSocket (Unix::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.writeData (data, sizeof (data)), 0) << join::lastError.message ();
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
    Unix::Socket unixSocket;
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    if (unixSocket.connect (_serverpath) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (unixSocket.waitConnected (_timeout)) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.writeData (data, sizeof (data)), 0) << join::lastError.message ();
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
    Unix::Socket unixSocket (Unix::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.writeData (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_GT (unixSocket.read (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test readChar method.
 */
TEST_F (UnixSocket, readChar)
{
    Unix::Socket unixSocket (Unix::Socket::Blocking);
    char data;

    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.writeData ("b", 1), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.readChar (data), 0) << join::lastError.message ();
    ASSERT_EQ (data, 'b');
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test readLine method.
 */
TEST_F (UnixSocket, readLine)
{
    Unix::Socket unixSocket (Unix::Socket::Blocking);
    std::string data;

    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.writeData ("readLine\n", strlen ("readLine\n")), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.readLine (data, 1024), 0) << join::lastError.message ();
    ASSERT_EQ (data, "readLine");
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test readData method.
 */
TEST_F (UnixSocket, readData)
{
    Unix::Socket unixSocket (Unix::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.writeData (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.readData (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test waitReadyWrite method.
 */
TEST_F (UnixSocket, waitReadyWrite)
{
    Unix::Socket unixSocket;

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
    Unix::Socket unixSocket (Unix::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_GT (unixSocket.write (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test writeData method.
 */
TEST_F (UnixSocket, writeData)
{
    Unix::Socket unixSocket (Unix::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.writeData (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test setMode method.
 */
TEST_F (UnixSocket, setMode)
{
    Unix::Socket unixSocket (Unix::Socket::Blocking);

    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.setMode (Unix::Socket::NonBlocking), 0);
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
    Unix::Socket unixSocket (Unix::Socket::Blocking);

    ASSERT_EQ (unixSocket.setOption (Unix::Socket::SndBuffer, 1500), -1);
    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.setOption (Unix::Socket::SndBuffer, 1500), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test localEndpoint method.
 */
TEST_F (UnixSocket, localEndpoint)
{
    Unix::Socket unixSocket (Unix::Socket::Blocking);

    ASSERT_EQ (unixSocket.bind (_clientpath), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.localEndpoint (), Unix::Endpoint (_clientpath)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test mtu method.
 */
TEST_F (UnixSocket, remoteEndpoint)
{
    Unix::Socket unixSocket (Unix::Socket::Blocking);

    ASSERT_EQ (unixSocket.bind (_clientpath), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.remoteEndpoint (), Unix::Endpoint (_serverpath)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test opened method.
 */
TEST_F (UnixSocket, opened)
{
    Unix::Socket unixSocket (Unix::Socket::Blocking);

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
    Unix::Socket unixSocket (Unix::Socket::Blocking);

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
    Unix::Socket unixSocket (Unix::Socket::Blocking);

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
    Unix::Socket unixSocket;

    ASSERT_EQ (unixSocket.family (), AF_UNIX);
}

/**
 * @brief Test type method.
 */
TEST_F (UnixSocket, type)
{
    Unix::Socket unixSocket;

    ASSERT_EQ (unixSocket.type (), SOCK_STREAM);
}

/**
 * @brief Test protocol method.
 */
TEST_F (UnixSocket, protocol)
{
    Unix::Socket unixSocket;

    ASSERT_EQ (unixSocket.protocol (), 0);
}

/**
 * @brief Test handle method.
 */
TEST_F (UnixSocket, handle)
{
    Unix::Socket unixSocket (Unix::Socket::Blocking);

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
    Unix::Socket unixSocket (Unix::Socket::Blocking);

    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.mtu (), -1);
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test lower method.
 */
TEST_F (UnixSocket, lower)
{
    Unix::Socket unixSocket1, unixSocket2;

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
