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
using join::UnixStream;

/**
 * @brief Class used to test the unix asynchronous stream socket API.
 */
class UnixAsyncStreamSocket : public ::testing::Test
{
protected:
    /**
     * @brief Sets up the test fixture.
     */
    void SetUp () override
    {
        ASSERT_EQ (_server.create (_serverpath), 0) << join::lastError.message ();
        ASSERT_EQ (_server.asyncAccept (onEchoAccept), 0) << join::lastError.message ();

        ScopedLock<Mutex> lock (_mut);

        _code = {};
        _completions = 0;
        _transferred = 0;
        _rearms = 0;
    }

    /**
     * @brief Tears down the test suite.
     */
    static void TearDownTestSuite ()
    {
        ::unlink (_serverpath.c_str ());
        ::unlink (_clientpath.c_str ());
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
    static UnixStream::AsyncSocket& peer ()
    {
        static UnixStream::AsyncSocket sock;
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
    static void onEchoAccept (const std::error_code& ec, UnixStream::AsyncSocket&& sock)
    {
        if (!ec)
        {
            peer () = std::move (sock);
            peer ().asyncRead (_echobuf, sizeof (_echobuf), onEchoRead);
        }
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
    UnixStream::AsyncAcceptor _server;

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
    static UnixStream::AsyncSocket* _current;

    /// number of resubmissions left to perform from the read handler.
    static int _rearms;

    /// path.
    static const std::string _serverpath;
    static const std::string _clientpath;

    /// timeout.
    static const int _timeout;
};

Mutex UnixAsyncStreamSocket::_mut;
Condition UnixAsyncStreamSocket::_cond;
std::error_code UnixAsyncStreamSocket::_code;
int UnixAsyncStreamSocket::_completions = 0;
size_t UnixAsyncStreamSocket::_transferred = 0;
char UnixAsyncStreamSocket::_buf[1024] = {};
char UnixAsyncStreamSocket::_echobuf[1024] = {};
UnixStream::AsyncSocket* UnixAsyncStreamSocket::_current = nullptr;
int UnixAsyncStreamSocket::_rearms = 0;
const std::string UnixAsyncStreamSocket::_serverpath = "/tmp/unixasyncserver_test.sock";
const std::string UnixAsyncStreamSocket::_clientpath = "/tmp/unixasyncclient_test.sock";
const int UnixAsyncStreamSocket::_timeout = 1000;

/**
 * @brief Test open method.
 */
TEST_F (UnixAsyncStreamSocket, open)
{
    UnixStream::AsyncSocket client;

    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_EQ (client.open (), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    client.close ();
}

/**
 * @brief Test close method.
 */
TEST_F (UnixAsyncStreamSocket, close)
{
    UnixStream::AsyncSocket client;

    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_TRUE (client.opened ());
    client.close ();
    ASSERT_FALSE (client.opened ());
}

/**
 * @brief Test bind method.
 */
TEST_F (UnixAsyncStreamSocket, bind)
{
    UnixStream::AsyncSocket client;

    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_EQ (client.bind (_clientpath), 0) << join::lastError.message ();
    client.close ();
}

/**
 * @brief Test bindToDevice method.
 */
TEST_F (UnixAsyncStreamSocket, bindToDevice)
{
    UnixStream::AsyncSocket client;

    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_EQ (client.bindToDevice (_clientpath), -1);
    client.close ();
}

/**
 * @brief Test asyncConnect method.
 */
TEST_F (UnixAsyncStreamSocket, asyncConnect)
{
    UnixStream::AsyncSocket client;

    ASSERT_EQ (client.asyncConnect (_serverpath,
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
    ASSERT_EQ (client.remoteEndpoint (), UnixStream::Endpoint (_serverpath));
    client.close ();
}

/**
 * @brief Test asyncWrite method.
 */
TEST_F (UnixAsyncStreamSocket, asyncWrite)
{
    UnixStream::AsyncSocket client;

    ASSERT_EQ (client.asyncWrite ("hello", 5, nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (client.asyncConnect (_serverpath,
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
TEST_F (UnixAsyncStreamSocket, asyncRead)
{
    UnixStream::AsyncSocket client;

    ASSERT_EQ (client.asyncRead (_buf, sizeof (_buf), nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (client.asyncConnect (_serverpath,
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
TEST_F (UnixAsyncStreamSocket, resubmit)
{
    UnixStream::AsyncSocket client;

    _current = &client;
    _rearms = 1;

    ASSERT_EQ (client.asyncConnect (_serverpath,
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

    client.close ();
    _current = nullptr;
}

/**
 * @brief Test cancelRead method.
 */
TEST_F (UnixAsyncStreamSocket, cancelRead)
{
    UnixStream::AsyncSocket client;

    ASSERT_EQ (client.cancelRead (), 0) << join::lastError.message ();

    ASSERT_EQ (client.asyncConnect (_serverpath,
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
TEST_F (UnixAsyncStreamSocket, cancelWrite)
{
    UnixStream::AsyncSocket client;

    ASSERT_EQ (client.cancelWrite (), 0) << join::lastError.message ();
    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_EQ (client.cancelWrite (), 0) << join::lastError.message ();
    client.close ();
}

/**
 * @brief Test readPending method.
 */
TEST_F (UnixAsyncStreamSocket, readPending)
{
    UnixStream::AsyncSocket client;

    ASSERT_FALSE (client.readPending ());

    ASSERT_EQ (client.asyncConnect (_serverpath,
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
TEST_F (UnixAsyncStreamSocket, writePending)
{
    UnixStream::AsyncSocket client;

    ASSERT_FALSE (client.writePending ());
    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_FALSE (client.writePending ());
    client.close ();
}

/**
 * @brief Test setOption method.
 */
TEST_F (UnixAsyncStreamSocket, setOption)
{
    UnixStream::AsyncSocket client;

    ASSERT_EQ (client.setOption (UnixStream::Socket::RcvBuffer, 1500), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_EQ (client.setOption (UnixStream::Socket::KeepAlive, 1), 0) << join::lastError.message ();
    client.close ();
}

/**
 * @brief Test localEndpoint method.
 */
TEST_F (UnixAsyncStreamSocket, localEndpoint)
{
    UnixStream::AsyncSocket client;

    ASSERT_EQ (client.localEndpoint (), UnixStream::Endpoint{});
    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_EQ (client.bind (_clientpath), 0) << join::lastError.message ();
    ASSERT_EQ (client.localEndpoint (), UnixStream::Endpoint (_clientpath));
    client.close ();
}

/**
 * @brief Test remoteEndpoint method.
 */
TEST_F (UnixAsyncStreamSocket, remoteEndpoint)
{
    UnixStream::AsyncSocket client;

    ASSERT_EQ (client.remoteEndpoint (), UnixStream::Endpoint{});

    ASSERT_EQ (client.asyncConnect (_serverpath,
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

    ASSERT_EQ (client.remoteEndpoint (), UnixStream::Endpoint (_serverpath));
    client.close ();
}

/**
 * @brief Test opened method.
 */
TEST_F (UnixAsyncStreamSocket, opened)
{
    UnixStream::AsyncSocket client;

    ASSERT_FALSE (client.opened ());
    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_TRUE (client.opened ());
    client.close ();
    ASSERT_FALSE (client.opened ());
}

/**
 * @brief Test connected method.
 */
TEST_F (UnixAsyncStreamSocket, connected)
{
    UnixStream::AsyncSocket client;

    ASSERT_FALSE (client.connected ());

    ASSERT_EQ (client.asyncConnect (_serverpath,
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
TEST_F (UnixAsyncStreamSocket, connecting)
{
    UnixStream::AsyncSocket client;

    ASSERT_FALSE (client.connecting ());
    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_FALSE (client.connecting ());
    client.close ();
}

/**
 * @brief Test canRead method.
 */
TEST_F (UnixAsyncStreamSocket, canRead)
{
    UnixStream::AsyncSocket client;

    ASSERT_EQ (client.canRead (), -1);

    ASSERT_EQ (client.asyncConnect (_serverpath,
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
TEST_F (UnixAsyncStreamSocket, mtu)
{
    UnixStream::AsyncSocket client;

    ASSERT_EQ (client.mtu (), -1);
    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_EQ (client.mtu (), -1);
    client.close ();
}

/**
 * @brief Test family method.
 */
TEST_F (UnixAsyncStreamSocket, family)
{
    UnixStream::AsyncSocket client;

    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_EQ (client.family (), AF_UNIX);
    client.close ();
}

/**
 * @brief Test type method.
 */
TEST_F (UnixAsyncStreamSocket, type)
{
    UnixStream::AsyncSocket client;

    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_EQ (client.type (), SOCK_STREAM);
    client.close ();
}

/**
 * @brief Test protocol method.
 */
TEST_F (UnixAsyncStreamSocket, protocol)
{
    UnixStream::AsyncSocket client;

    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_EQ (client.protocol (), 0);
    client.close ();
}

/**
 * @brief Test handle method.
 */
TEST_F (UnixAsyncStreamSocket, handle)
{
    UnixStream::AsyncSocket client;

    ASSERT_EQ (client.handle (), -1);
    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_GT (client.handle (), -1);
    client.close ();
    ASSERT_EQ (client.handle (), -1);
}

/**
 * @brief Test socket method.
 */
TEST_F (UnixAsyncStreamSocket, socket)
{
    UnixStream::AsyncSocket client;

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
