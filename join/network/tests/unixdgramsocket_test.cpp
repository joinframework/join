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
#include <join/socket.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::Errc;
using join::Reactor;
using join::UnixDgram;

/**
 * @brief Class used to test the unix datagram socket API.
 */
class UnixDgramSocket : public UnixDgram::Socket, public ::testing::Test
{
protected:
    /**
     * @brief Sets up the test fixture.
     */
    void SetUp ()
    {
        ASSERT_EQ (this->bind (_serverpath), 0) << join::lastError.message ();
        ASSERT_EQ (Reactor::instance ()->addHandler (this), 0) << join::lastError.message ();
    }

    /**
     * @brief Tears down the test fixture.
     */
    void TearDown ()
    {
        ASSERT_EQ (Reactor::instance ()->delHandler (this), 0) << join::lastError.message ();
        this->close ();
    }

    /**
     * @brief method called when data are ready to be read on handle.
     */
    virtual void onReceive () override
    {
        auto buffer = std::make_unique <char []> (this->canRead ());
        if (buffer)
        {
            UnixDgram::Endpoint from;
            int nread = this->readFrom (buffer.get (), this->canRead (), &from);
            if (nread > 0)
            {
                this->writeTo (buffer.get (), nread, from);
            }
        }
    }

    /// path.
    static const std::string _serverpath;
    static const std::string _clientpath;

    /// timeout.
    static const int _timeout;
};

const std::string UnixDgramSocket::_serverpath = "/tmp/unixserver_test.sock";
const std::string UnixDgramSocket::_clientpath = "/tmp/unixclient_test.sock";
const int         UnixDgramSocket::_timeout = 1000;

/**
 * @brief Test open method.
 */
TEST_F (UnixDgramSocket, open)
{
    UnixDgram::Socket unixSocket;

    ASSERT_EQ (unixSocket.open (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.open (), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    unixSocket.close ();
}

/**
 * @brief Test close method.
 */
TEST_F (UnixDgramSocket, close)
{
    UnixDgram::Socket unixSocket (UnixDgram::Socket::Blocking);

    ASSERT_FALSE (unixSocket.opened ());
    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.opened ());
    unixSocket.close ();
    ASSERT_FALSE (unixSocket.opened ());
}

/**
 * @brief Test bind method.
 */
TEST_F (UnixDgramSocket, bind)
{
    UnixDgram::Socket unixSocket (UnixDgram::Socket::Blocking);

    ASSERT_EQ (unixSocket.bind (_clientpath), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();

    unixSocket.close ();
}

/**
 * @brief Test bindToDevice method.
 */
TEST_F (UnixDgramSocket, bindToDevice)
{
    UnixDgram::Socket unixSocket (UnixDgram::Socket::Blocking);

    ASSERT_EQ (unixSocket.bindToDevice (_clientpath), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();

    unixSocket.close ();
}

/**
 * @brief Test connect method.
 */
TEST_F (UnixDgramSocket, connect)
{
    UnixDgram::Socket unixSocket (UnixDgram::Socket::Blocking);

    ASSERT_EQ (unixSocket.connect (""), -1);

    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.connect (_serverpath), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    unixSocket.close ();
}

/**
 * @brief disconnect method.
 */
TEST_F (UnixDgramSocket, disconnect)
{
    UnixDgram::Socket unixSocket (UnixDgram::Socket::Blocking);

    ASSERT_FALSE (unixSocket.connected ());
    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.connected ());
    ASSERT_EQ (unixSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_FALSE (unixSocket.connected ());
    unixSocket.close ();
    ASSERT_FALSE (unixSocket.connected());
}

/**
 * @brief Test canRead method.
 */
TEST_F (UnixDgramSocket, canRead)
{
    UnixDgram::Socket unixSocket (UnixDgram::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (unixSocket.canRead (), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (unixSocket.bind (_clientpath), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.write (data, sizeof (data)), sizeof (data)) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_GT (unixSocket.canRead (), 0) << join::lastError.message ();
    unixSocket.close ();
}

/**
 * @brief Test waitReadyRead method.
 */
TEST_F (UnixDgramSocket, waitReadyRead)
{
    UnixDgram::Socket unixSocket;
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_FALSE (unixSocket.waitReadyRead (_timeout));
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (unixSocket.bind (_clientpath), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.write (data, sizeof (data)), sizeof (data)) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    unixSocket.close ();
}

/**
 * @brief Test read method.
 */
TEST_F (UnixDgramSocket, read)
{
    UnixDgram::Socket unixSocket (UnixDgram::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (unixSocket.read (data, sizeof (data)), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (unixSocket.bind (_clientpath), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.write (data, sizeof (data)), sizeof (data)) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.read (data, sizeof (data)), sizeof (data)) << join::lastError.message ();
    unixSocket.close ();
}

/**
 * @brief Test readFrom method.
 */
TEST_F (UnixDgramSocket, readFrom)
{
    UnixDgram::Socket unixSocket (UnixDgram::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};
    UnixDgram::Endpoint from;

    ASSERT_EQ (unixSocket.readFrom (data, sizeof (data)), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (unixSocket.bind (_clientpath), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.write (data, sizeof (data)), sizeof (data)) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.readFrom (data, unixSocket.canRead (), &from), sizeof (data)) << join::lastError.message ();
    ASSERT_EQ (from, UnixDgram::Endpoint (_serverpath));
    unixSocket.close ();
}

/**
 * @brief Test waitReadyWrite method.
 */
TEST_F (UnixDgramSocket, waitReadyWrite)
{
    UnixDgram::Socket unixSocket;

    ASSERT_FALSE (unixSocket.waitReadyWrite (_timeout));
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    unixSocket.close ();
}

/**
 * @brief Test write method.
 */
TEST_F (UnixDgramSocket, write)
{
    UnixDgram::Socket unixSocket (UnixDgram::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (unixSocket.write (data, sizeof (data)), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (unixSocket.bind (_clientpath), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.write (data, sizeof (data)), sizeof (data)) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    unixSocket.close ();
}

/**
 * @brief Test writeTo method.
 */
TEST_F (UnixDgramSocket, writeTo)
{
    UnixDgram::Socket unixSocket (UnixDgram::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (unixSocket.bind (_clientpath), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (unixSocket.writeTo (data, sizeof (data), {}), -1);
    ASSERT_EQ (unixSocket.writeTo (data, sizeof (data), _serverpath), sizeof (data)) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.waitReadyRead (_timeout));
    unixSocket.close ();
}

/**
 * @brief Test setMode method.
 */
TEST_F (UnixDgramSocket, setMode)
{
    UnixDgram::Socket unixSocket;

    ASSERT_EQ (unixSocket.open (), 0) << join::lastError.message ();

    int flags = ::fcntl (unixSocket.handle (), F_GETFL, 0);
    ASSERT_TRUE (flags & O_NONBLOCK);

    unixSocket.setMode (UnixDgram::Socket::Blocking);
    flags = ::fcntl (unixSocket.handle (), F_GETFL, 0);
    ASSERT_FALSE (flags & O_NONBLOCK);

    unixSocket.setMode (UnixDgram::Socket::NonBlocking);
    flags = ::fcntl (unixSocket.handle (), F_GETFL, 0);
    ASSERT_TRUE (flags & O_NONBLOCK);

    unixSocket.close ();
}

/**
 * @brief Test setOption method.
 */
TEST_F (UnixDgramSocket, setOption)
{
    UnixDgram::Socket unixSocket;

    ASSERT_EQ (unixSocket.setOption (UnixDgram::Socket::RcvBuffer, 1500), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (unixSocket.open (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.setOption (UnixDgram::Socket::NoDelay, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (unixSocket.setOption (UnixDgram::Socket::KeepAlive, 1), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.setOption (UnixDgram::Socket::KeepIdle, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (unixSocket.setOption (UnixDgram::Socket::KeepIntvl, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (unixSocket.setOption (UnixDgram::Socket::KeepCount, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (unixSocket.setOption (UnixDgram::Socket::SndBuffer, 1500), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.setOption (UnixDgram::Socket::RcvBuffer, 1500), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.setOption (UnixDgram::Socket::TimeStamp, 1), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.setOption (UnixDgram::Socket::ReuseAddr, 1), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.setOption (UnixDgram::Socket::ReusePort, 1), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.setOption (UnixDgram::Socket::Broadcast, 1), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.setOption (UnixDgram::Socket::Ttl, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (unixSocket.setOption (UnixDgram::Socket::MulticastLoop, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (unixSocket.setOption (UnixDgram::Socket::MulticastTtl, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (unixSocket.setOption (UnixDgram::Socket::PathMtuDiscover, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (unixSocket.setOption (UnixDgram::Socket::RcvError, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (unixSocket.setOption (UnixDgram::Socket::AuxData, 1), -1);
    ASSERT_EQ (join::lastError, std::errc::operation_not_supported);
    unixSocket.close ();
}

/**
 * @brief Test localEndpoint method.
 */
TEST_F (UnixDgramSocket, localEndpoint)
{
    UnixDgram::Socket unixSocket (UnixDgram::Socket::Blocking);

    ASSERT_EQ (unixSocket.localEndpoint (), UnixDgram::Endpoint {});
    ASSERT_EQ (unixSocket.bind (_clientpath), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.localEndpoint (), UnixDgram::Endpoint (_clientpath)) << join::lastError.message ();
    unixSocket.close ();
}

/**
 * @brief Test mtu method.
 */
TEST_F (UnixDgramSocket, remoteEndpoint)
{
    UnixDgram::Socket unixSocket (UnixDgram::Socket::Blocking);

    ASSERT_EQ (unixSocket.remoteEndpoint (), UnixDgram::Endpoint {});
    ASSERT_EQ (unixSocket.bind (_clientpath), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.remoteEndpoint (), UnixDgram::Endpoint (_serverpath)) << join::lastError.message ();
    unixSocket.close ();
}

/**
 * @brief Test opened method.
 */
TEST_F (UnixDgramSocket, opened)
{
    UnixDgram::Socket unixSocket (UnixDgram::Socket::Blocking);

    ASSERT_FALSE (unixSocket.opened ());
    ASSERT_EQ (unixSocket.open (), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.opened ());
    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.opened ());
    unixSocket.close ();
    ASSERT_FALSE (unixSocket.opened ());
}

/**
 * @brief Test connected method.
 */
TEST_F (UnixDgramSocket, connected)
{
    UnixDgram::Socket unixSocket (UnixDgram::Socket::Blocking);

    ASSERT_FALSE (unixSocket.opened ());
    ASSERT_EQ (unixSocket.open (), 0) << join::lastError.message ();
    ASSERT_FALSE (unixSocket.connected ());
    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_TRUE (unixSocket.connected ());
    unixSocket.close ();
    ASSERT_FALSE (unixSocket.connected ());
}

/**
 * @brief Test encrypted method.
 */
TEST_F (UnixDgramSocket, encrypted)
{
    UnixDgram::Socket unixSocket (UnixDgram::Socket::Blocking);

    ASSERT_FALSE (unixSocket.opened ());
    ASSERT_EQ (unixSocket.open (), 0) << join::lastError.message ();
    ASSERT_FALSE (unixSocket.encrypted ());
    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_FALSE (unixSocket.encrypted ());
    unixSocket.close ();
    ASSERT_FALSE (unixSocket.encrypted ());
}

/**
 * @brief Test family method.
 */
TEST_F (UnixDgramSocket, family)
{
    UnixDgram::Socket unixSocket;

    ASSERT_EQ (unixSocket.family (), AF_UNIX);
}

/**
 * @brief Test type method.
 */
TEST_F (UnixDgramSocket, type)
{
    UnixDgram::Socket unixSocket;

    ASSERT_EQ (unixSocket.type (), SOCK_DGRAM);
}

/**
 * @brief Test protocol method.
 */
TEST_F (UnixDgramSocket, protocol)
{
    UnixDgram::Socket unixSocket;

    ASSERT_EQ (unixSocket.protocol (), 0);
}

/**
 * @brief Test handle method.
 */
TEST_F (UnixDgramSocket, handle)
{
    UnixDgram::Socket unixSocket (UnixDgram::Socket::Blocking);

    ASSERT_EQ (unixSocket.handle (), -1);
    ASSERT_EQ (unixSocket.open (), 0) << join::lastError.message ();
    ASSERT_GT (unixSocket.handle (), -1);
    unixSocket.close ();
    ASSERT_EQ (unixSocket.handle (), -1);
}

/**
 * @brief Test mtu method.
 */
TEST_F (UnixDgramSocket, mtu)
{
    UnixDgram::Socket unixSocket (UnixDgram::Socket::Blocking);

    ASSERT_EQ (unixSocket.mtu (), -1);
    ASSERT_EQ (unixSocket.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket.mtu (), -1);
    unixSocket.close ();
    ASSERT_EQ (unixSocket.mtu (), -1);
}

/**
 * @brief Test checksum method.
 */
TEST_F (UnixDgramSocket, checksum)
{
    std::string buffer ({'\xD2', '\xB6', '\x69', '\xFD', '\x2E'});

    ASSERT_EQ (UnixDgram::Socket::checksum (reinterpret_cast <uint16_t *> (&buffer[0]), buffer.size (), 0), 19349);
}

/**
 * @brief Test is lower method.
 */
TEST_F (UnixDgramSocket, lower)
{
    UnixDgram::Socket unixSocket1, unixSocket2;

    ASSERT_EQ (unixSocket1.open (), 0) << join::lastError.message ();
    ASSERT_EQ (unixSocket2.open (), 0) << join::lastError.message ();
    if (unixSocket1.handle () < unixSocket2.handle ())
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
