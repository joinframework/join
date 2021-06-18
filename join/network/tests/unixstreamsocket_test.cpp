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
using join::net::UnixStream;
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
        ASSERT_EQ (bind (serverpath_), 0) << join::lastError.message ();
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
        UnixStream::Socket socket = accept ();
        if (socket.connected ())
        {
            char buf[1024];
            for (;;)
            {
                // echo received data.
                int nread = socket.read (buf, sizeof (buf));
                if (nread == -1)
                {
                    if (join::lastError == Errc::TemporaryError)
                    {
                        if (socket.waitReadyRead (timeout_))
                            continue;
                    }
                    break;
                }
                socket.writeData (buf, nread);
            }
            socket.close ();
        }
    }

    /// path.
    static const std::string serverpath_;
    static const std::string clientpath_;

    /// timeout.
    static const int timeout_;
};

const std::string UnixStreamSocket::serverpath_ = "/tmp/unixserver_test.sock";
const std::string UnixStreamSocket::clientpath_ = "/tmp/unixclient_test.sock";
const int         UnixStreamSocket::timeout_ = 1000;

/**
 * @brief Test open method.
 */
TEST_F (UnixStreamSocket, open)
{
    UnixStream::Socket unixSocket;

    ASSERT_EQ (unixSocket.open (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test close method.
 */
TEST_F (UnixStreamSocket, close)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);

    ASSERT_FALSE (unixSocket.opened ());
    ASSERT_EQ (unixSocket.connect (serverpath_), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.opened());
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_FALSE (unixSocket.opened());
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
    ASSERT_FALSE (unixSocket.opened());
}

/**
 * @brief Test bind method.
 */
TEST_F (UnixStreamSocket, bind)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);

    ASSERT_EQ (unixSocket.bind (clientpath_), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test connect method.
 */
TEST_F (UnixStreamSocket, connect)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);

    ASSERT_EQ (unixSocket.connect (serverpath_), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test waitConnected method.
 */
TEST_F (UnixStreamSocket, waitConnected)
{
    UnixStream::Socket unixSocket;

    if (unixSocket.connect (serverpath_) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
        ASSERT_TRUE (unixSocket.connecting ());
    }
    ASSERT_TRUE (unixSocket.waitConnected (timeout_)) << join::lastError.message ();
    if (unixSocket.disconnect () == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (unixSocket.waitDisconnected (timeout_)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test disconnect method.
 */
TEST_F (UnixStreamSocket, disconnect)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);

    ASSERT_FALSE (unixSocket.connected ());
    ASSERT_EQ (unixSocket.connect (serverpath_), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.connected ());
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_FALSE (unixSocket.connected ());
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
    ASSERT_FALSE (unixSocket.connected ());
}

/**
 * @brief Test waitDisconnected method.
 */
TEST_F (UnixStreamSocket, waitDisconnected)
{
    UnixStream::Socket unixSocket;

    if (unixSocket.connect (serverpath_) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
        ASSERT_TRUE (unixSocket.connecting ());
    }
    ASSERT_TRUE (unixSocket.waitConnected (timeout_)) << join::lastError.message ();
    if (unixSocket.disconnect () == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (unixSocket.waitDisconnected (timeout_)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test canRead method.
 */
TEST_F (UnixStreamSocket, canRead)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (unixSocket.connect (serverpath_), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyWrite (timeout_)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.writeData (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyRead (timeout_)) << join::lastError.message ();
    ASSERT_GT (unixSocket.canRead (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test waitReadyRead method.
 */
TEST_F (UnixStreamSocket, waitReadyRead)
{
    UnixStream::Socket unixSocket;
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    if (unixSocket.connect (serverpath_) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (unixSocket.waitConnected (timeout_)) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyWrite (timeout_)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.writeData (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyRead (timeout_)) << join::lastError.message ();
    if (unixSocket.disconnect () == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (unixSocket.waitDisconnected (timeout_)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test read method.
 */
TEST_F (UnixStreamSocket, read)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (unixSocket.connect (serverpath_), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyWrite (timeout_)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.writeData (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyRead (timeout_)) << join::lastError.message ();
    ASSERT_GT (unixSocket.read (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test readChar method.
 */
TEST_F (UnixStreamSocket, readChar)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);
    char data;

    ASSERT_EQ (unixSocket.connect (serverpath_), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyWrite (timeout_)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.writeData ("b", 1), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyRead (timeout_)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.readChar (data), 0) << join::lastError.message ();
    ASSERT_EQ (data, 'b');
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test readLine method.
 */
TEST_F (UnixStreamSocket, readLine)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);
    std::string data;

    ASSERT_EQ (unixSocket.connect (serverpath_), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyWrite (timeout_)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.writeData ("readLine\n", strlen ("readLine\n")), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyRead (timeout_)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.readLine (data, 1024), 0) << join::lastError.message ();
    ASSERT_EQ (data, "readLine");
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test readData method.
 */
TEST_F (UnixStreamSocket, readData)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (unixSocket.connect (serverpath_), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyWrite (timeout_)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.writeData (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyRead (timeout_)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.readData (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test waitReadyWrite method.
 */
TEST_F (UnixStreamSocket, waitReadyWrite)
{
    UnixStream::Socket unixSocket;

    if (unixSocket.connect (serverpath_) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (unixSocket.waitConnected (timeout_)) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyWrite (timeout_)) << join::lastError.message ();
    if (unixSocket.disconnect () == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (unixSocket.waitDisconnected (timeout_)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test write method.
 */
TEST_F (UnixStreamSocket, write)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (unixSocket.connect (serverpath_), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyWrite (timeout_)) << join::lastError.message ();
    ASSERT_GT (unixSocket.write (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyRead (timeout_)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test writeData method.
 */
TEST_F (UnixStreamSocket, writeData)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (unixSocket.connect (serverpath_), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyWrite (timeout_)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.writeData (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyRead (timeout_)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test setMode method.
 */
TEST_F (UnixStreamSocket, setMode)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);

    ASSERT_EQ (unixSocket.connect (serverpath_), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.setMode (UnixStream::Socket::NonBlocking), 0);
    if (unixSocket.disconnect () == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (unixSocket.waitDisconnected (timeout_)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test setOption method.
 */
TEST_F (UnixStreamSocket, setOption)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);

    ASSERT_EQ (unixSocket.setOption (UnixStream::Socket::SndBuffer, 1500), -1);
    ASSERT_EQ (unixSocket.connect (serverpath_), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.setOption (UnixStream::Socket::SndBuffer, 1500), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test localEndpoint method.
 */
TEST_F (UnixStreamSocket, localEndpoint)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);

    ASSERT_EQ (unixSocket.bind (clientpath_), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.connect (serverpath_), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.localEndpoint (), UnixStream::Endpoint (clientpath_)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test mtu method.
 */
TEST_F (UnixStreamSocket, remoteEndpoint)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);

    ASSERT_EQ (unixSocket.bind (clientpath_), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.connect (serverpath_), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.remoteEndpoint (), UnixStream::Endpoint (serverpath_)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
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
    ASSERT_EQ (unixSocket.connect (serverpath_), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.opened ());
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_FALSE (unixSocket.opened ());
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
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
    ASSERT_EQ (unixSocket.connect (serverpath_), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.connected());
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_FALSE (unixSocket.connected());
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
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
    ASSERT_EQ (unixSocket.connect (serverpath_), 0) << join::lastError.message ();
    ASSERT_FALSE (unixSocket.encrypted ());
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_FALSE (unixSocket.encrypted ());
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
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
    ASSERT_EQ (unixSocket.connect (serverpath_), 0) << join::lastError.message ();
    ASSERT_GT (unixSocket.handle (), -1);
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.handle (), -1);
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.handle (), -1);
}

/**
 * @brief Test mtu method.
 */
TEST_F (UnixStreamSocket, mtu)
{
    UnixStream::Socket unixSocket (UnixStream::Socket::Blocking);

    ASSERT_EQ (unixSocket.connect (serverpath_), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.mtu (), -1);
    ASSERT_EQ (unixSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test randomize method.
 */
TEST_F (UnixStreamSocket, randomize)
{
    ASSERT_GT (UnixStream::Socket::randomize <int> (), 0);
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