/**
 * MIT License
 *
 * Copyright (c) 2026 Mathieu Rabine
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
#include <join/async_acceptor.hpp>
#include <join/condition.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::Errc;
using join::Mutex;
using join::Condition;
using join::ScopedLock;
using join::IpAddress;
using join::Tcp;

/**
 * @brief Class used to test the unix asynchronous stream socket API.
 */
class TcpAsyncStreamSocket : public ::testing::Test
{
protected:
    /**
     * @brief Sets up the test fixture.
     */
    void SetUp () override
    {
        ASSERT_EQ (_server.create ({IpAddress::ipv6Wildcard, _port}), 0) << join::lastError.message ();
        ASSERT_EQ (_server.asyncAccept (onEchoAccept), 0) << join::lastError.message ();

        ScopedLock<Mutex> lock (_mut);

        _code = {};
        _completions = 0;
        _transferred = 0;
        _rearms = 0;
    }

    /**
     * @brief Tears down the test fixture.
     */
    void TearDown () override
    {
        peer ().close ();
        _server.close ();
    }

    /**
     * @brief get the socket accepted by the echo server.
     * @return the socket accepted by the echo server.
     */
    static Tcp::AsyncSocket& peer ()
    {
        static Tcp::AsyncSocket sock;
        return sock;
    }

    /**
     * @brief receive the data to send back.
     * @param ec error reported by the socket.
     * @param size number of bytes read.
     */
    static void onEchoRead (const std::error_code& ec, size_t size)
    {
        if (!ec)
        {
            peer ().asyncWrite (_echobuf, size, onEchoWrite);
        }
    }

    /**
     * @brief send back the data received by the echo server.
     * @param ec error reported by the socket.
     * @param size number of bytes written.
     */
    static void onEchoWrite (const std::error_code& ec, [[maybe_unused]] size_t size)
    {
        if (!ec)
        {
            peer ().asyncRead (_echobuf, sizeof (_echobuf), onEchoRead);
        }
    }

    /**
     * @brief adopt the socket accepted by the echo server.
     * @param ec error reported by the acceptor.
     * @param sock accepted socket.
     */
    static void onEchoAccept (const std::error_code& ec, Tcp::AsyncSocket&& sock)
    {
        if (!ec)
        {
            peer () = std::move (sock);
            peer ().asyncRead (_echobuf, sizeof (_echobuf), onEchoRead);
        }
    }

    /**
     * @brief handler resubmitting a write from within itself.
     * @param ec error reported by the socket.
     * @param size number of bytes written.
     */
    static void onWrite (const std::error_code& ec, size_t size)
    {
        if (!ec && (_rearms > 0))
        {
            --_rearms;
            _current->asyncWrite ("two", 3, onWrite);
        }

        ScopedLock<Mutex> lock (_mut);

        _code = ec;
        _transferred = size;
        ++_completions;
        _cond.signal ();
    }

    /**
     * @brief handler resubmitting a connection from within itself.
     * @param ec error reported by the socket.
     */
    static void onConnect (const std::error_code& ec)
    {
        if (_rearms > 0)
        {
            --_rearms;
            _current->asyncConnect ({_host, _port}, onConnect);
        }

        ScopedLock<Mutex> lock (_mut);

        _code = ec;
        ++_completions;
        _cond.signal ();
    }

    /**
     * @brief handler resubmitting a read from within itself.
     * @param ec error reported by the socket.
     * @param size number of bytes read.
     */
    static void onRead (const std::error_code& ec, size_t size)
    {
        if (!ec && (_rearms > 0))
        {
            --_rearms;
            _current->asyncRead (_buf, sizeof (_buf), onRead);
        }

        ScopedLock<Mutex> lock (_mut);

        _code = ec;
        _transferred = size;
        ++_completions;
        _cond.signal ();
    }

    /// echo server.
    Tcp::AsyncAcceptor _server;

    /// condition mutex.
    static Mutex _mut;

    /// condition variable.
    static Condition _cond;

    /// last reported error.
    static std::error_code _code;

    /// number of completions reported.
    static int _completions;

    /// number of bytes reported by the last completion.
    static size_t _transferred;

    /// read buffer.
    static char _buf[1024];

    /// buffer used by the echo server.
    static char _echobuf[1024];

    /// socket used by the resubmitting handler.
    static Tcp::AsyncSocket* _current;

    /// number of resubmissions left to perform from the read handler.
    static int _rearms;

    /// host.
    static const IpAddress _host;

    /// port.
    static const uint16_t _port;

    /// timeout.
    static const int _timeout;
};

Mutex TcpAsyncStreamSocket::_mut;
Condition TcpAsyncStreamSocket::_cond;
std::error_code TcpAsyncStreamSocket::_code;
int TcpAsyncStreamSocket::_completions = 0;
size_t TcpAsyncStreamSocket::_transferred = 0;
char TcpAsyncStreamSocket::_buf[1024] = {};
char TcpAsyncStreamSocket::_echobuf[1024] = {};
Tcp::AsyncSocket* TcpAsyncStreamSocket::_current = nullptr;
int TcpAsyncStreamSocket::_rearms = 0;
const IpAddress TcpAsyncStreamSocket::_host = "::1";
const uint16_t TcpAsyncStreamSocket::_port = 5034;
const int TcpAsyncStreamSocket::_timeout = 1000;

/**
 * @brief Test open method.
 */
TEST_F (TcpAsyncStreamSocket, open)
{
    Tcp::AsyncSocket client;

    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_EQ (client.open (), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    client.close ();
}

/**
 * @brief Test close method.
 */
TEST_F (TcpAsyncStreamSocket, close)
{
    Tcp::AsyncSocket client;

    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_TRUE (client.opened ());
    client.close ();
    ASSERT_FALSE (client.opened ());
}

/**
 * @brief Test bind method.
 */
TEST_F (TcpAsyncStreamSocket, bind)
{
    Tcp::AsyncSocket client;

    ASSERT_EQ (client.open (Tcp::v6 ()), 0) << join::lastError.message ();
    ASSERT_EQ (client.bind (IpAddress (AF_INET6)), 0) << join::lastError.message ();
    client.close ();
}

/**
 * @brief Test bindToDevice method.
 */
TEST_F (TcpAsyncStreamSocket, bindToDevice)
{
    Tcp::AsyncSocket client;

    ASSERT_EQ (client.bindToDevice ("lo"), -1);
    ASSERT_EQ (client.open (Tcp::v6 ()), 0) << join::lastError.message ();
    ASSERT_EQ (client.bindToDevice ("lo"), 0) << join::lastError.message ();
    ASSERT_EQ (client.bindToDevice ("foo"), -1);
    client.close ();
}

/**
 * @brief Test asyncConnect method.
 */
TEST_F (TcpAsyncStreamSocket, asyncConnect)
{
    Tcp::AsyncSocket client;

    ASSERT_EQ (client.asyncConnect ({_host, _port},
                                    [] (const std::error_code& ec) {
                                        ScopedLock<Mutex> lock (_mut);
                                        _code = ec;
                                        ++_completions;
                                        _cond.signal ();
                                    }),
               0)
        << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
    }

    ASSERT_TRUE (client.connected ());
    ASSERT_EQ (client.remoteEndpoint (), Tcp::Endpoint (_host, _port));
    client.close ();
}

/**
 * @brief Test asyncWrite method.
 */
TEST_F (TcpAsyncStreamSocket, asyncWrite)
{
    Tcp::AsyncSocket client;

    ASSERT_EQ (client.asyncWrite ("hello", 5, nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (client.asyncConnect ({_host, _port},
                                    [] (const std::error_code& ec) {
                                        ScopedLock<Mutex> lock (_mut);
                                        _code = ec;
                                        ++_completions;
                                        _cond.signal ();
                                    }),
               0)
        << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
    }

    ASSERT_EQ (client.asyncWrite ("hello", 5,
                                  [] (const std::error_code& ec, size_t size) {
                                      ScopedLock<Mutex> lock (_mut);
                                      _code = ec;
                                      _transferred = size;
                                      ++_completions;
                                      _cond.signal ();
                                  }),
               0)
        << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 2;
        }));
        ASSERT_FALSE (_code) << _code.message ();
        ASSERT_EQ (_transferred, 5u);
    }

    client.close ();
}

/**
 * @brief Test asyncRead method.
 */
TEST_F (TcpAsyncStreamSocket, asyncRead)
{
    Tcp::AsyncSocket client;

    ASSERT_EQ (client.asyncRead (_buf, sizeof (_buf), nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (client.asyncConnect ({_host, _port},
                                    [] (const std::error_code& ec) {
                                        ScopedLock<Mutex> lock (_mut);
                                        _code = ec;
                                        ++_completions;
                                        _cond.signal ();
                                    }),
               0)
        << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
    }

    ASSERT_EQ (client.asyncRead (_buf, sizeof (_buf),
                                 [] (const std::error_code& ec, size_t size) {
                                     ScopedLock<Mutex> lock (_mut);
                                     _code = ec;
                                     _transferred = size;
                                     ++_completions;
                                     _cond.signal ();
                                 }),
               0)
        << join::lastError.message ();

    ASSERT_EQ (client.asyncRead (_buf, sizeof (_buf), nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);

    ASSERT_EQ (client.asyncWrite ("hello", 5, nullptr), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 2;
        }));
        ASSERT_FALSE (_code) << _code.message ();
        ASSERT_EQ (_transferred, 5u);
        ASSERT_EQ (std::string (_buf, 5), "hello");
    }

    client.close ();
}

/**
 * @brief Test asyncRead method resubmitted from its own handler.
 */
TEST_F (TcpAsyncStreamSocket, resubmit)
{
    Tcp::AsyncSocket client;

    _current = &client;
    _rearms = 1;

    ASSERT_EQ (client.asyncConnect ({_host, _port},
                                    [] (const std::error_code& ec) {
                                        ScopedLock<Mutex> lock (_mut);
                                        _code = ec;
                                        ++_completions;
                                        _cond.signal ();
                                    }),
               0)
        << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
    }

    ASSERT_EQ (client.asyncRead (_buf, sizeof (_buf), onRead), 0) << join::lastError.message ();
    ASSERT_EQ (client.asyncWrite ("one", 3, nullptr), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 2;
        }));
        ASSERT_FALSE (_code) << _code.message ();
        ASSERT_EQ (_transferred, 3u);
    }

    ASSERT_EQ (client.asyncWrite ("two", 3, nullptr), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 3;
        }));
        ASSERT_FALSE (_code) << _code.message ();
        ASSERT_EQ (_transferred, 3u);
        ASSERT_EQ (std::string (_buf, 3), "two");
    }

    _rearms = 1;

    ASSERT_EQ (client.asyncWrite ("three", 5, onWrite), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 5;
        }));
        ASSERT_FALSE (_code) << _code.message ();
    }

    _rearms = 1;

    ASSERT_EQ (client.asyncConnect ({_host, _port}, onConnect), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 7;
        }));
    }

    ASSERT_FALSE (client.writePending ());

    client.close ();
    _current = nullptr;
}

/**
 * @brief Test close from within the write completion handler.
 */
TEST_F (TcpAsyncStreamSocket, closeFromWriteHandler)
{
    Tcp::AsyncSocket client;

    _current = &client;

    ASSERT_EQ (client.asyncConnect ({_host, _port},
                                    [] (const std::error_code& ec) {
                                        ScopedLock<Mutex> lock (_mut);
                                        _code = ec;
                                        ++_completions;
                                        _cond.signal ();
                                    }),
               0)
        << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
    }

    ASSERT_EQ (client.asyncWrite ("hello", 5,
                                  [] (const std::error_code& ec, [[maybe_unused]] size_t size) {
                                      _current->close ();

                                      ScopedLock<Mutex> lock (_mut);
                                      _code = ec;
                                      ++_completions;
                                      _cond.signal ();
                                  }),
               0)
        << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 2;
        }));
        ASSERT_FALSE (_code) << _code.message ();
    }

    ASSERT_FALSE (client.opened ());
    _current = nullptr;
}

/**
 * @brief Test cancelRead method.
 */
TEST_F (TcpAsyncStreamSocket, cancelRead)
{
    Tcp::AsyncSocket client;

    ASSERT_EQ (client.cancelRead (), 0) << join::lastError.message ();

    ASSERT_EQ (client.asyncConnect ({_host, _port},
                                    [] (const std::error_code& ec) {
                                        ScopedLock<Mutex> lock (_mut);
                                        _code = ec;
                                        ++_completions;
                                        _cond.signal ();
                                    }),
               0)
        << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
    }

    ASSERT_EQ (client.asyncRead (_buf, sizeof (_buf),
                                 [] (const std::error_code& ec, size_t size) {
                                     ScopedLock<Mutex> lock (_mut);
                                     _code = ec;
                                     _transferred = size;
                                     ++_completions;
                                     _cond.signal ();
                                 }),
               0)
        << join::lastError.message ();

    ASSERT_TRUE (client.readPending ());
    ASSERT_EQ (client.cancelRead (), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 2;
        }));
        ASSERT_EQ (_code, std::errc::operation_canceled);
    }

    ASSERT_FALSE (client.readPending ());
    client.close ();
}

/**
 * @brief Test cancelWrite method.
 */
TEST_F (TcpAsyncStreamSocket, cancelWrite)
{
    Tcp::AsyncSocket client;

    ASSERT_EQ (client.cancelWrite (), 0) << join::lastError.message ();
    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_EQ (client.cancelWrite (), 0) << join::lastError.message ();
    client.close ();
}

/**
 * @brief Test readPending method.
 */
TEST_F (TcpAsyncStreamSocket, readPending)
{
    Tcp::AsyncSocket client;

    ASSERT_FALSE (client.readPending ());

    ASSERT_EQ (client.asyncConnect ({_host, _port},
                                    [] (const std::error_code& ec) {
                                        ScopedLock<Mutex> lock (_mut);
                                        _code = ec;
                                        ++_completions;
                                        _cond.signal ();
                                    }),
               0)
        << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
    }

    ASSERT_EQ (client.asyncRead (_buf, sizeof (_buf),
                                 [] (const std::error_code&, size_t) {
                                 }),
               0)
        << join::lastError.message ();
    ASSERT_TRUE (client.readPending ());
    client.close ();
    ASSERT_FALSE (client.readPending ());
}

/**
 * @brief Test writePending method.
 */
TEST_F (TcpAsyncStreamSocket, writePending)
{
    Tcp::AsyncSocket client;

    ASSERT_FALSE (client.writePending ());
    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_FALSE (client.writePending ());
    client.close ();
}

/**
 * @brief Test setOption method.
 */
TEST_F (TcpAsyncStreamSocket, setOption)
{
    Tcp::AsyncSocket client;

    ASSERT_EQ (client.setOption (Tcp::Socket::RcvBuffer, 1500), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_EQ (client.setOption (Tcp::Socket::NoDelay, 1), 0) << join::lastError.message ();
    client.close ();
}

/**
 * @brief Test localEndpoint method.
 */
TEST_F (TcpAsyncStreamSocket, localEndpoint)
{
    Tcp::AsyncSocket client;

    ASSERT_EQ (client.localEndpoint (), Tcp::Endpoint{});
    ASSERT_EQ (client.open (Tcp::v6 ()), 0) << join::lastError.message ();
    ASSERT_EQ (client.bind (IpAddress (AF_INET6)), 0) << join::lastError.message ();
    ASSERT_EQ (client.localEndpoint ().ip (), IpAddress (AF_INET6));
    client.close ();
}

/**
 * @brief Test remoteEndpoint method.
 */
TEST_F (TcpAsyncStreamSocket, remoteEndpoint)
{
    Tcp::AsyncSocket client;

    ASSERT_EQ (client.remoteEndpoint (), Tcp::Endpoint{});

    ASSERT_EQ (client.asyncConnect ({_host, _port},
                                    [] (const std::error_code& ec) {
                                        ScopedLock<Mutex> lock (_mut);
                                        _code = ec;
                                        ++_completions;
                                        _cond.signal ();
                                    }),
               0)
        << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
    }

    ASSERT_EQ (client.remoteEndpoint (), Tcp::Endpoint (_host, _port));
    client.close ();
}

/**
 * @brief Test opened method.
 */
TEST_F (TcpAsyncStreamSocket, opened)
{
    Tcp::AsyncSocket client;

    ASSERT_FALSE (client.opened ());
    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_TRUE (client.opened ());
    client.close ();
    ASSERT_FALSE (client.opened ());
}

/**
 * @brief Test connected method.
 */
TEST_F (TcpAsyncStreamSocket, connected)
{
    Tcp::AsyncSocket client;

    ASSERT_FALSE (client.connected ());

    ASSERT_EQ (client.asyncConnect ({_host, _port},
                                    [] (const std::error_code& ec) {
                                        ScopedLock<Mutex> lock (_mut);
                                        _code = ec;
                                        ++_completions;
                                        _cond.signal ();
                                    }),
               0)
        << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
    }

    ASSERT_TRUE (client.connected ());
    client.close ();
    ASSERT_FALSE (client.connected ());
}

/**
 * @brief Test connecting method.
 */
TEST_F (TcpAsyncStreamSocket, connecting)
{
    Tcp::AsyncSocket client;

    ASSERT_FALSE (client.connecting ());
    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_FALSE (client.connecting ());
    client.close ();
}

/**
 * @brief Test canRead method.
 */
TEST_F (TcpAsyncStreamSocket, canRead)
{
    Tcp::AsyncSocket client;

    ASSERT_EQ (client.canRead (), -1);

    ASSERT_EQ (client.asyncConnect ({_host, _port},
                                    [] (const std::error_code& ec) {
                                        ScopedLock<Mutex> lock (_mut);
                                        _code = ec;
                                        ++_completions;
                                        _cond.signal ();
                                    }),
               0)
        << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
    }

    ASSERT_EQ (client.canRead (), 0);
    client.close ();
}

/**
 * @brief Test mtu method.
 */
TEST_F (TcpAsyncStreamSocket, mtu)
{
    Tcp::AsyncSocket client;

    ASSERT_EQ (client.mtu (), -1);

    ASSERT_EQ (client.asyncConnect ({_host, _port},
                                    [] (const std::error_code& ec) {
                                        ScopedLock<Mutex> lock (_mut);
                                        _code = ec;
                                        ++_completions;
                                        _cond.signal ();
                                    }),
               0)
        << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
    }

    ASSERT_NE (client.mtu (), -1) << join::lastError.message ();
    client.close ();
    ASSERT_EQ (client.mtu (), -1);
}

/**
 * @brief Test family method.
 */
TEST_F (TcpAsyncStreamSocket, family)
{
    Tcp::AsyncSocket client;

    ASSERT_EQ (client.open (Tcp::v6 ()), 0) << join::lastError.message ();
    ASSERT_EQ (client.family (), AF_INET6);
    client.close ();
}

/**
 * @brief Test type method.
 */
TEST_F (TcpAsyncStreamSocket, type)
{
    Tcp::AsyncSocket client;

    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_EQ (client.type (), SOCK_STREAM);
    client.close ();
}

/**
 * @brief Test protocol method.
 */
TEST_F (TcpAsyncStreamSocket, protocol)
{
    Tcp::AsyncSocket client;

    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_EQ (client.protocol (), IPPROTO_TCP);
    client.close ();
}

/**
 * @brief Test handle method.
 */
TEST_F (TcpAsyncStreamSocket, handle)
{
    Tcp::AsyncSocket client;

    ASSERT_EQ (client.handle (), -1);
    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_GT (client.handle (), -1);
    client.close ();
    ASSERT_EQ (client.handle (), -1);
}

/**
 * @brief Test socket method.
 */
TEST_F (TcpAsyncStreamSocket, socket)
{
    Tcp::AsyncSocket client;

    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_TRUE (client.socket ().opened ());
    client.close ();
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
