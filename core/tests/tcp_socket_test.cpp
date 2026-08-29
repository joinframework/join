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
#include <join/reactor.hpp>

// Libraries.
#include <gtest/gtest.h>

// C++.
#include <vector>
#include <thread>

using join::Errc;
using join::IpAddress;
using join::ReactorThread;
using join::EventHandler;
using join::Tcp;

/**
 * @brief Class used to test the TCP socket API.
 */
class TcpSocket : public EventHandler, public ::testing::Test
{
protected:
    /**
     * @brief Sets up the test fixture.
     */
    void SetUp () override
    {
        ASSERT_EQ (_server.create ({IpAddress::ipv6Wildcard, _port}), 0) << join::lastError.message ();
        ASSERT_EQ (ReactorThread::reactor ().addHandler (_server.handle (), this), 0) << join::lastError.message ();
    }

    /**
     * @brief Tears down the test fixture.
     */
    void TearDown () override
    {
        ASSERT_EQ (ReactorThread::reactor ().delHandler (_server.handle ()), 0) << join::lastError.message ();
        _server.close ();
    }

    /**
     * @brief method called when data are ready to be read on handle.
     * @param fd file descriptor.
     */
    virtual void onReadable ([[maybe_unused]] int fd) override
    {
        Tcp::Socket sock = _server.accept ();
        if (sock.connected ())
        {
            char buf[1024];
            for (;;)
            {
                // echo received data.
                ssize_t nread = sock.read (buf, sizeof (buf));
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

    /// server.
    Tcp::Acceptor _server;

    /// host.
    static const std::string _hostv4;
    static const std::string _hostv6;

    /// port.
    static const uint16_t _port;

    /// ports of the acceptors used to test the operation deadlines.
    static const uint16_t _stallport;
    static const uint16_t _stallport2;

    /// timeout.
    static const std::chrono::milliseconds _timeout;
};

const std::string TcpSocket::_hostv4 = "127.0.0.1";
const std::string TcpSocket::_hostv6 = "::1";
const uint16_t TcpSocket::_port = 5000;
const uint16_t TcpSocket::_stallport = 5002;
const uint16_t TcpSocket::_stallport2 = 5003;
const std::chrono::milliseconds TcpSocket::_timeout{1000};

/**
 * @brief Test move.
 */
TEST_F (TcpSocket, move)
{
    Tcp::Socket tcpSocket1 (Tcp::Socket::Blocking), tcpSocket2;

    ASSERT_EQ (tcpSocket1.connect ({_hostv4, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket1.connected ());
    tcpSocket2 = std::move (tcpSocket1);
    ASSERT_TRUE (tcpSocket2.connected ());
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
    ASSERT_EQ (tcpSocket.connect ({_hostv4, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.opened ());
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.opened ());
    tcpSocket.close ();
    ASSERT_FALSE (tcpSocket.opened ());
}

/**
 * @brief Test bind method.
 */
TEST_F (TcpSocket, bind)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);

    ASSERT_EQ (tcpSocket.connect ({_hostv4, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.bind (_hostv4), -1);
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    tcpSocket.close ();

    ASSERT_EQ (tcpSocket.bind (_hostv4), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.connect ({_hostv4, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    tcpSocket.close ();
}

/**
 * @brief Test bindToDevice method.
 */
TEST_F (TcpSocket, bindToDevice)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);

    ASSERT_EQ (tcpSocket.bindToDevice ("lo"), -1);
    ASSERT_EQ (tcpSocket.connect ({_hostv4, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.bindToDevice ("lo"), -1);
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    tcpSocket.close ();

    ASSERT_EQ (tcpSocket.open (Tcp::v6 ()), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.bindToDevice ("lo"), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.connect ({_hostv6, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.bindToDevice ("foo"), -1);
    tcpSocket.close ();
}

/**
 * @brief Test connect method.
 */
TEST_F (TcpSocket, connect)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);

    ASSERT_EQ (tcpSocket.connect ({"255.255.255.255", _port}), -1);

    ASSERT_EQ (tcpSocket.connect ({_hostv4, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.connect ({_hostv4, _port}), -1);
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
    if (tcpSocket.connect ({_hostv4, _port}) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
        ASSERT_TRUE (tcpSocket.connecting ());
    }
    ASSERT_TRUE (tcpSocket.waitConnected (_timeout)) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitConnected ()) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitConnected (std::chrono::steady_clock::now () + _timeout)) << join::lastError.message ();
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
    ASSERT_EQ (tcpSocket.connect ({_hostv4, _port}), 0) << join::lastError.message ();
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

    ASSERT_TRUE (tcpSocket.waitDisconnected (_timeout)) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitDisconnected ()) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitDisconnected (std::chrono::steady_clock::now () + _timeout))
        << join::lastError.message ();
    if (tcpSocket.connect ({_hostv4, _port}) == -1)
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
    char data[] = {0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (tcpSocket.canRead (), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (tcpSocket.connect ({_hostv4, _port}), 0) << join::lastError.message ();
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
    char data[] = {0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_FALSE (tcpSocket.waitReadyRead (_timeout));
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    if (tcpSocket.connect ({_hostv4, _port}) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (tcpSocket.waitConnected (_timeout)) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.writeExactly (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitReadyRead ()) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitReadyRead (std::chrono::steady_clock::now () + _timeout)) << join::lastError.message ();
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
    char data[] = {0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (tcpSocket.read (data, sizeof (data)), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (tcpSocket.connect ({_hostv4, _port}), 0) << join::lastError.message ();
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
    char data[] = {0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (tcpSocket.readExactly (data, sizeof (data)), -1);
    ASSERT_EQ (tcpSocket.connect ({_hostv4, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.writeExactly (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.readExactly (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    tcpSocket.close ();

    Tcp::Acceptor stall;
    Tcp::Socket dribbler;

    ASSERT_EQ (stall.create ({IpAddress (_hostv4), _stallport}), 0) << join::lastError.message ();

    if (dribbler.connect ({_hostv4, _stallport}) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError);
        ASSERT_TRUE (dribbler.waitConnected (_timeout)) << join::lastError.message ();
    }

    Tcp::Socket peer = stall.accept ();
    ASSERT_TRUE (peer.connected ());

    std::thread sender ([&peer] () {
        for (int i = 0; i < 8; ++i)
        {
            std::this_thread::sleep_for (std::chrono::milliseconds (100));
            peer.writeExactly ("x", 1);
        }
    });

    char slow[8] = {};
    auto beg = std::chrono::steady_clock::now ();

    int result = dribbler.readExactly (slow, sizeof (slow), std::chrono::milliseconds (250));
    std::error_code error = join::lastError;
    auto elapsed = std::chrono::steady_clock::now () - beg;

    dribbler.setMode (Tcp::Socket::Blocking);
    int blocking = dribbler.readExactly (slow, sizeof (slow), std::chrono::milliseconds (250));
    std::error_code blockingError = join::lastError;
    bool blockingWait = dribbler.waitDisconnected (std::chrono::milliseconds (250));
    std::error_code blockingWaitError = join::lastError;

    sender.join ();
    dribbler.close ();
    peer.close ();
    stall.close ();

    ASSERT_EQ (result, -1);
    ASSERT_EQ (error, Errc::TimedOut);
    ASSERT_LT (elapsed, std::chrono::milliseconds (700));
    ASSERT_EQ (blocking, -1);
    ASSERT_EQ (blockingError, Errc::OperationFailed);
    ASSERT_FALSE (blockingWait);
    ASSERT_EQ (blockingWaitError, Errc::OperationFailed);
}

/**
 * @brief Test wait method.
 */
TEST_F (TcpSocket, wait)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);

    ASSERT_EQ (tcpSocket.wait (true, false), -1);
    ASSERT_EQ (join::lastError, std::errc::bad_file_descriptor);

    ASSERT_EQ (tcpSocket.connect ({_hostv4, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.wait (false, true), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    tcpSocket.close ();
}

/**
 * @brief Test waitFor method.
 */
TEST_F (TcpSocket, waitFor)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);

    ASSERT_EQ (tcpSocket.waitFor (true, false, _timeout), -1);
    ASSERT_EQ (join::lastError, std::errc::bad_file_descriptor);

    ASSERT_EQ (tcpSocket.connect ({_hostv4, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.waitFor (false, true, _timeout), 0) << join::lastError.message ();

    auto beg = std::chrono::steady_clock::now ();

    ASSERT_EQ (tcpSocket.waitFor (true, false, std::chrono::milliseconds (100)), -1);
    ASSERT_EQ (join::lastError, Errc::TimedOut);
    ASSERT_GE (std::chrono::steady_clock::now () - beg, std::chrono::milliseconds (100));

    ASSERT_EQ (tcpSocket.waitFor (true, false, std::chrono::nanoseconds::zero ()), -1);
    ASSERT_EQ (join::lastError, Errc::TimedOut);

    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    tcpSocket.close ();
}

/**
 * @brief Test waitUntil method.
 */
TEST_F (TcpSocket, waitUntil)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);

    ASSERT_EQ (tcpSocket.waitUntil (true, false, Tcp::Socket::TimePoint::max ()), -1);
    ASSERT_EQ (join::lastError, std::errc::bad_file_descriptor);

    ASSERT_EQ (tcpSocket.connect ({_hostv4, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.waitUntil (false, true, Tcp::Socket::TimePoint::max ()), 0) << join::lastError.message ();

    ASSERT_EQ (tcpSocket.waitUntil (true, false, std::chrono::steady_clock::now ()), -1);
    ASSERT_EQ (join::lastError, Errc::TimedOut);

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
    if (tcpSocket.connect ({_hostv4, _port}) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (tcpSocket.waitConnected (_timeout)) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitReadyWrite ()) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitReadyWrite (std::chrono::steady_clock::now () + _timeout)) << join::lastError.message ();
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
    char data[] = {0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (tcpSocket.write (data, sizeof (data)), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (tcpSocket.connect ({_hostv4, _port}), 0) << join::lastError.message ();
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
    char data[] = {0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (tcpSocket.writeExactly (data, sizeof (data)), -1);
    ASSERT_EQ (tcpSocket.connect ({_hostv4, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.writeExactly (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    tcpSocket.close ();

    Tcp::Acceptor stall;
    Tcp::Socket sender;

    ASSERT_EQ (stall.create ({IpAddress (_hostv4), _stallport2}), 0) << join::lastError.message ();

    if (sender.connect ({_hostv4, _stallport2}) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError);
        ASSERT_TRUE (sender.waitConnected (_timeout)) << join::lastError.message ();
    }

    Tcp::Socket peer = stall.accept ();
    ASSERT_TRUE (peer.connected ());

    ASSERT_EQ (sender.setOption (Tcp::Socket::SndBuffer, 4096), 0) << join::lastError.message ();
    ASSERT_EQ (peer.setOption (Tcp::Socket::RcvBuffer, 4096), 0) << join::lastError.message ();

    std::vector<char> bulk (1024 * 1024, 'x');
    auto beg = std::chrono::steady_clock::now ();

    ASSERT_EQ (sender.writeExactly (bulk.data (), bulk.size (), std::chrono::milliseconds (250)), -1);
    ASSERT_EQ (join::lastError, Errc::TimedOut);
    ASSERT_LT (std::chrono::steady_clock::now () - beg, std::chrono::milliseconds (700));

    sender.setMode (Tcp::Socket::Blocking);
    ASSERT_EQ (sender.writeExactly (bulk.data (), bulk.size (), std::chrono::milliseconds (250)), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (sender.writeExactly (bulk.data (), bulk.size (), std::chrono::steady_clock::now ()), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    sender.close ();
    peer.close ();
    stall.close ();
}

/**
 * @brief Test setMode method.
 */
TEST_F (TcpSocket, setMode)
{
    Tcp::Socket tcpSocket;

    ASSERT_EQ (tcpSocket.open (), 0) << join::lastError.message ();

    int flags = ::fcntl (tcpSocket.handle (), F_GETFL, 0);
    ASSERT_TRUE (flags & O_NONBLOCK);

    tcpSocket.setMode (Tcp::Socket::Blocking);
    flags = ::fcntl (tcpSocket.handle (), F_GETFL, 0);
    ASSERT_FALSE (flags & O_NONBLOCK);

    tcpSocket.setMode (Tcp::Socket::NonBlocking);
    flags = ::fcntl (tcpSocket.handle (), F_GETFL, 0);
    ASSERT_TRUE (flags & O_NONBLOCK);

    tcpSocket.close ();
}

/**
 * @brief Test mode method.
 */
TEST_F (TcpSocket, mode)
{
    Tcp::Socket tcpSocket;

    ASSERT_EQ (tcpSocket.mode (), Tcp::Socket::NonBlocking);
    tcpSocket.setMode (Tcp::Socket::Blocking);
    ASSERT_EQ (tcpSocket.mode (), Tcp::Socket::Blocking);
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

    ASSERT_EQ (tcpSocket.localEndpoint (), Tcp::Endpoint{});
    ASSERT_EQ (tcpSocket.bind ({_hostv4, uint16_t (_port + 1)}), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.connect ({_hostv4, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.localEndpoint (), Tcp::Endpoint (_hostv4, uint16_t (_port + 1))) << join::lastError.message ();
    tcpSocket.close ();
}

/**
 * @brief Test remoteEndpoint method.
 */
TEST_F (TcpSocket, remoteEndpoint)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);

    ASSERT_EQ (tcpSocket.remoteEndpoint (), Tcp::Endpoint{});
    ASSERT_EQ (tcpSocket.bind ({_hostv4, uint16_t (_port + 1)}), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.connect ({_hostv4, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.remoteEndpoint (), Tcp::Endpoint (_hostv4, _port)) << join::lastError.message ();
    tcpSocket.close ();
}

/**
 * @brief Test opened method.
 */
TEST_F (TcpSocket, opened)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);

    ASSERT_FALSE (tcpSocket.opened ());
    ASSERT_EQ (tcpSocket.open (Tcp{IpAddress (_hostv4).family ()}), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.opened ());
    ASSERT_EQ (tcpSocket.connect ({_hostv4, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.opened ());
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.opened ());
    tcpSocket.close ();
    ASSERT_FALSE (tcpSocket.opened ());
}

/**
 * @brief Test connected method.
 */
TEST_F (TcpSocket, connected)
{
    Tcp::Socket tcpSocket (Tcp::Socket::Blocking);

    ASSERT_FALSE (tcpSocket.connected ());
    ASSERT_EQ (tcpSocket.open (Tcp{IpAddress (_hostv4).family ()}), 0) << join::lastError.message ();
    ASSERT_FALSE (tcpSocket.connected ());
    ASSERT_EQ (tcpSocket.connect ({_hostv4, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (tcpSocket.connected ());
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_FALSE (tcpSocket.connected ());
    tcpSocket.close ();
    ASSERT_FALSE (tcpSocket.connected ());
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
    ASSERT_EQ (tcpSocket.open (Tcp{IpAddress (_hostv4).family ()}), 0) << join::lastError.message ();
    ASSERT_GT (tcpSocket.handle (), -1);
    ASSERT_EQ (tcpSocket.connect ({_hostv4, _port}), 0) << join::lastError.message ();
    ASSERT_GT (tcpSocket.handle (), -1);
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_GT (tcpSocket.handle (), -1);
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
    ASSERT_EQ (tcpSocket.connect ({"127.0.0.1", _port}), 0) << join::lastError.message ();
    ASSERT_NE (tcpSocket.mtu (), -1) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_NE (tcpSocket.mtu (), -1) << join::lastError.message ();
    tcpSocket.close ();
    ASSERT_EQ (tcpSocket.mtu (), -1);

    ASSERT_EQ (tcpSocket.mtu (), -1);
    ASSERT_EQ (tcpSocket.connect ({"::1", _port}), 0) << join::lastError.message ();
    ASSERT_NE (tcpSocket.mtu (), -1) << join::lastError.message ();
    ASSERT_EQ (tcpSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_NE (tcpSocket.mtu (), -1) << join::lastError.message ();
    tcpSocket.close ();
    ASSERT_EQ (tcpSocket.mtu (), -1);
}

/**
 * @brief Test lower method.
 */
TEST_F (TcpSocket, lower)
{
    Tcp::Socket tcpSocket1, tcpSocket2;

    ASSERT_EQ (tcpSocket1.open (Tcp{IpAddress (_hostv4).family ()}), 0) << join::lastError.message ();
    ASSERT_EQ (tcpSocket2.open (Tcp{IpAddress (_hostv4).family ()}), 0) << join::lastError.message ();
    if (tcpSocket1.handle () < tcpSocket2.handle ())
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
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
