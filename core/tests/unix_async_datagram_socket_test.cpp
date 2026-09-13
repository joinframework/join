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
#include <join/async_datagram_socket.hpp>
#include <join/condition.hpp>

// Libraries.
#include <gtest/gtest.h>

// C.
#include <unistd.h>

using join::Errc;
using join::Mutex;
using join::Condition;
using join::ScopedLock;
using join::UnixDgram;
using join::Thread;
using join::Proactor;
using join::LocalMem;

/**
 * @brief Class used to test the unix asynchronous datagram socket API.
 */
class UnixAsyncDatagramSocket : public ::testing::Test
{
protected:
    /**
     * @brief Sets up the test fixture.
     */
    void SetUp () override
    {
        ::unlink (_serverpath.c_str ());
        ::unlink (_clientpath.c_str ());
        ::unlink (_senderpath.c_str ());

        ASSERT_EQ (server ().bind (_serverpath), 0) << join::lastError.message ();
        ASSERT_NE (server ().asyncReadFrom (_echobuf, sizeof (_echobuf), _echofrom, onEchoRead), -1)
            << join::lastError.message ();

        ScopedLock<Mutex> lock (_mut);

        _code = {};
        _completions = 0;
        _transferred = 0;
        _more = false;
        _rearms = 0;
    }

    /**
     * @brief Tears down the test fixture.
     */
    void TearDown () override
    {
        server ().close ();

        ::unlink (_serverpath.c_str ());
        ::unlink (_clientpath.c_str ());
        ::unlink (_senderpath.c_str ());
    }

    /**
     * @brief get the echo server socket.
     * @return the echo server socket.
     */
    static UnixDgram::AsyncSocket& server ()
    {
        static UnixDgram::AsyncSocket sock;
        return sock;
    }

    /**
     * @brief send back the datagram received by the echo server.
     * @param ec error reported by the socket.
     * @param data buffer holding the data received.
     * @param size number of bytes read.
     * @param from endpoint the datagram was received from.
     * @param more true if the read stays armed.
     */
    static void onEchoRead (const std::error_code& ec, [[maybe_unused]] const char* data, size_t size,
                            [[maybe_unused]] const UnixDgram::Endpoint& from, [[maybe_unused]] bool more)
    {
        if (!ec)
        {
            server ().asyncWriteTo (_echobuf, size, _echofrom, onEchoWrite);
        }
    }

    /**
     * @brief wait for the next datagram to echo.
     * @param ec error reported by the socket.
     * @param size number of bytes written.
     */
    static void onEchoWrite (const std::error_code& ec, [[maybe_unused]] size_t size)
    {
        if (!ec)
        {
            server ().asyncReadFrom (_echobuf, sizeof (_echobuf), _echofrom, onEchoRead);
        }
    }

    /**
     * @brief report a completion to the test thread.
     * @param ec error reported by the socket.
     * @param size number of bytes transferred.
     */
    static void onReport (const std::error_code& ec, size_t size)
    {
        ScopedLock<Mutex> lock (_mut);

        _code = ec;
        _transferred = size;
        ++_completions;
        _cond.signal ();
    }

    /**
     * @brief report a read completion to the test thread.
     * @param ec error reported by the socket.
     * @param data buffer holding the data received.
     * @param size number of bytes read.
     * @param more true if the read stays armed.
     */
    static void onReportRead (const std::error_code& ec, [[maybe_unused]] const char* data, size_t size,
                              [[maybe_unused]] bool more)
    {
        onReport (ec, size);
    }

    /**
     * @brief report a read completion and the endpoint it came from to the test thread.
     * @param ec error reported by the socket.
     * @param data buffer holding the data received.
     * @param size number of bytes read.
     * @param from endpoint the datagram was received from.
     * @param more true if the read stays armed.
     */
    static void onReportFrom (const std::error_code& ec, [[maybe_unused]] const char* data, size_t size,
                              const UnixDgram::Endpoint& from, [[maybe_unused]] bool more)
    {
        {
            ScopedLock<Mutex> lock (_mut);
            _from = from;
        }

        onReport (ec, size);
    }

    /**
     * @brief get the socket receiving the move performed from a handler.
     * @return the socket receiving the move performed from a handler.
     */
    static UnixDgram::AsyncSocket& moved ()
    {
        static UnixDgram::AsyncSocket sock;
        return sock;
    }

    /**
     * @brief handler moving the socket from within itself.
     * @param ec error reported by the socket.
     * @param data buffer holding the data received.
     * @param size number of bytes read.
     * @param from endpoint the datagram was received from.
     * @param more true if the read stays armed.
     */
    static void onReadAndMove (const std::error_code& ec, [[maybe_unused]] const char* data, size_t size,
                               [[maybe_unused]] const UnixDgram::Endpoint& from, [[maybe_unused]] bool more)
    {
        moved () = std::move (*_current);

        onReport (ec, size);
    }

    /**
     * @brief report a multishot completion to the test thread.
     * @param ec error reported by the socket.
     * @param data buffer holding the data received.
     * @param size number of bytes read.
     * @param from endpoint the datagram was received from.
     * @param more true if the read stays armed.
     */
    static void onReportMulti (const std::error_code& ec, const char* data, size_t size,
                               const UnixDgram::Endpoint& from, bool more)
    {
        ScopedLock<Mutex> lock (_mut);

        if (!ec && (size <= sizeof (_buf)))
        {
            ::memcpy (_buf, data, size);
        }

        _code = ec;
        _transferred = size;
        _from = from;
        _more = more;
        ++_completions;
        _cond.signal ();
    }

    /**
     * @brief handler resubmitting a read from within itself.
     * @param ec error reported by the socket.
     * @param data buffer holding the data received.
     * @param size number of bytes read.
     * @param from endpoint the datagram was received from.
     * @param more true if the read stays armed.
     */
    static void onRead (const std::error_code& ec, [[maybe_unused]] const char* data, size_t size,
                        [[maybe_unused]] const UnixDgram::Endpoint& from, [[maybe_unused]] bool more)
    {
        if (!ec && (_rearms > 0))
        {
            --_rearms;
            _current->asyncReadFrom (_buf, sizeof (_buf), _from, onRead);
        }

        onReport (ec, size);
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
            _current->asyncWriteTo ("two", 3, _dest, onWrite);
        }

        onReport (ec, size);
    }

    /**
     * @brief handler closing the socket from within itself.
     * @param ec error reported by the socket.
     * @param size number of bytes written.
     */
    static void onWriteAndClose (const std::error_code& ec, size_t size)
    {
        _current->close ();

        onReport (ec, size);
    }

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

    /// endpoint the last datagram was received from.
    static UnixDgram::Endpoint _from;

    /// last reported multishot state.
    static bool _more;

    /// buffer used by the echo server.
    static char _echobuf[1024];

    /// endpoint the echo server received the last datagram from.
    static UnixDgram::Endpoint _echofrom;

    /// socket used by the resubmitting handler.
    static UnixDgram::AsyncSocket* _current;

    /// number of resubmissions left to perform from a handler.
    static int _rearms;

    /// destination used by the resubmitting write handler.
    static UnixDgram::Endpoint _dest;

    /// echo server path.
    static const std::string _serverpath;

    /// client path.
    static const std::string _clientpath;

    /// path used by the synchronous sender.
    static const std::string _senderpath;

    /// timeout.
    static const std::chrono::milliseconds _timeout;
};

Mutex UnixAsyncDatagramSocket::_mut;
Condition UnixAsyncDatagramSocket::_cond;
std::error_code UnixAsyncDatagramSocket::_code;
int UnixAsyncDatagramSocket::_completions = 0;
size_t UnixAsyncDatagramSocket::_transferred = 0;
char UnixAsyncDatagramSocket::_buf[1024] = {};
UnixDgram::Endpoint UnixAsyncDatagramSocket::_from;
bool UnixAsyncDatagramSocket::_more = false;
char UnixAsyncDatagramSocket::_echobuf[1024] = {};
UnixDgram::Endpoint UnixAsyncDatagramSocket::_echofrom;
UnixDgram::AsyncSocket* UnixAsyncDatagramSocket::_current = nullptr;
int UnixAsyncDatagramSocket::_rearms = 0;
UnixDgram::Endpoint UnixAsyncDatagramSocket::_dest;
const std::string UnixAsyncDatagramSocket::_serverpath = "/tmp/unixasyncdgramserver_test.sock";
const std::string UnixAsyncDatagramSocket::_clientpath = "/tmp/unixasyncdgramclient_test.sock";
const std::string UnixAsyncDatagramSocket::_senderpath = "/tmp/unixasyncdgramsender_test.sock";
const std::chrono::milliseconds UnixAsyncDatagramSocket::_timeout{1000};

/**
 * @brief Test open method.
 */
TEST_F (UnixAsyncDatagramSocket, open)
{
    UnixDgram::AsyncSocket client;

    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_EQ (client.open (), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    client.close ();
}

/**
 * @brief Test close method.
 */
TEST_F (UnixAsyncDatagramSocket, close)
{
    UnixDgram::AsyncSocket client;

    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_TRUE (client.opened ());
    client.close ();
    ASSERT_FALSE (client.opened ());
}

/**
 * @brief Test bind method.
 */
TEST_F (UnixAsyncDatagramSocket, bind)
{
    UnixDgram::AsyncSocket client;

    ASSERT_EQ (client.bind (_clientpath), 0) << join::lastError.message ();
    ASSERT_EQ (client.bind (_clientpath), -1);
    client.close ();
}

/**
 * @brief Test bindToDevice method.
 */
TEST_F (UnixAsyncDatagramSocket, bindToDevice)
{
    UnixDgram::AsyncSocket client;

    ASSERT_EQ (client.bindToDevice (_clientpath), -1);
    client.close ();
}

/**
 * @brief Test connect method.
 */
TEST_F (UnixAsyncDatagramSocket, connect)
{
    UnixDgram::AsyncSocket client;

    ASSERT_EQ (client.connect (""), -1);

    ASSERT_EQ (client.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_TRUE (client.connected ());
    ASSERT_EQ (client.connect (_serverpath), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    client.close ();
}

/**
 * @brief Test disconnect method.
 */
TEST_F (UnixAsyncDatagramSocket, disconnect)
{
    UnixDgram::AsyncSocket client;

    ASSERT_EQ (client.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (client.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_TRUE (client.connected ());
    ASSERT_EQ (client.disconnect (), 0) << join::lastError.message ();
    ASSERT_FALSE (client.connected ());
    client.close ();
}

/**
 * @brief Test asyncWriteTo method.
 */
TEST_F (UnixAsyncDatagramSocket, asyncWriteTo)
{
    Proactor proactor;
    UnixDgram::AsyncSocket client (proactor);
    UnixDgram::Endpoint dest (_serverpath);

    ASSERT_FALSE (client.opened ());
    ASSERT_NE (client.asyncWriteTo ("hello", 5, dest, onReport), -1) << join::lastError.message ();
    ASSERT_TRUE (client.opened ());

    Thread th ([&proactor] () {
        proactor.run ();
    });

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
        ASSERT_EQ (_transferred, 5u);
    }

    client.close ();

    proactor.stop ();
    th.join ();
}

/**
 * @brief Test asyncReadFrom method.
 */
TEST_F (UnixAsyncDatagramSocket, asyncReadFrom)
{
    UnixDgram::AsyncSocket client;

    ASSERT_EQ (client.asyncReadFrom (_buf, sizeof (_buf), _from, nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (client.bind (_clientpath), 0) << join::lastError.message ();
    ASSERT_EQ (client.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_NE (client.asyncReadFrom (_buf, sizeof (_buf), _from, onReportFrom), -1) << join::lastError.message ();

    ASSERT_NE (client.asyncWrite ("hello", 5, nullptr), -1) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
        ASSERT_EQ (_transferred, 5u);
        ASSERT_EQ (std::string (_buf, 5), "hello");
    }

    client.close ();

    ASSERT_EQ (_from, UnixDgram::Endpoint (_serverpath));
}

/**
 * @brief Test asyncWrite method.
 */
TEST_F (UnixAsyncDatagramSocket, asyncWrite)
{
    UnixDgram::AsyncSocket client;

    ASSERT_EQ (client.asyncWrite ("hello", 5, nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (client.bind (_clientpath), 0) << join::lastError.message ();
    ASSERT_EQ (client.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_NE (client.asyncWrite ("hello", 5, onReport), -1) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
        ASSERT_EQ (_transferred, 5u);
    }

    ASSERT_EQ (::close (client.handle ()), 0);

    ASSERT_NE (client.asyncWrite ("hello", 5, onReport), -1) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 2;
        }));
        ASSERT_EQ (_code, std::errc::bad_file_descriptor) << _code.message ();
    }

    client.close ();
}

/**
 * @brief Test move.
 */
TEST_F (UnixAsyncDatagramSocket, move)
{
    UnixDgram::AsyncSocket client1, client3;

    ASSERT_EQ (client1.bind (_clientpath), 0) << join::lastError.message ();
    ASSERT_EQ (client1.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_TRUE (client1.opened ());

    UnixDgram::AsyncSocket client2 (std::move (client1));
    ASSERT_TRUE (client2.opened ());
    ASSERT_FALSE (client1.opened ());

    ASSERT_EQ (client1.asyncReadFrom (_buf, sizeof (_buf), _from, nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (client1.asyncWriteTo ("one", 3, _dest, nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (client1.cancelRead (0), 0) << join::lastError.message ();
    ASSERT_EQ (client1.cancelWrite (0), 0) << join::lastError.message ();
    client1.close ();

    ASSERT_NE (client2.asyncReadFrom (_buf, sizeof (_buf), _from, onRead), -1) << join::lastError.message ();

    client3 = std::move (client2);

    ASSERT_TRUE (client3.opened ());
    ASSERT_FALSE (client2.opened ());

    ASSERT_NE (client3.asyncWrite ("hello", 5, nullptr), -1) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
        ASSERT_EQ (_transferred, 5u);
        ASSERT_EQ (std::string (_buf, 5), "hello");
    }

    client3.close ();

    UnixDgram::AsyncSocket client4;

    ASSERT_EQ (client4.bind (_senderpath), 0) << join::lastError.message ();
    ASSERT_EQ (client4.connect (_serverpath), 0) << join::lastError.message ();

    _current = &client4;

    ASSERT_NE (client4.asyncReadFrom (_buf, sizeof (_buf), _from, onReadAndMove), -1) << join::lastError.message ();
    ASSERT_NE (client4.asyncWrite ("moved", 5, nullptr), -1) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 2;
        }));
        ASSERT_FALSE (_code) << _code.message ();
        ASSERT_EQ (_transferred, 5u);
        ASSERT_EQ (std::string (_buf, 5), "moved");
    }

    ASSERT_TRUE (moved ().opened ());
    ASSERT_FALSE (client4.opened ());

    moved ().close ();
    _current = nullptr;
}

/**
 * @brief Test asyncReadFromMulti method.
 */
TEST_F (UnixAsyncDatagramSocket, asyncReadFromMulti)
{
    UnixDgram::AsyncSocket client;

    ASSERT_EQ (client.asyncReadFromMulti (0, nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (client.bind (_clientpath), 0) << join::lastError.message ();
    ASSERT_EQ (client.connect (_serverpath), 0) << join::lastError.message ();

    LocalMem::Allocator<4, sizeof (_buf)> arena;
    ASSERT_EQ (client.registerBufferRing (0, arena), 0) << join::lastError.message ();

    ssize_t index = client.asyncReadFromMulti (0, onReportMulti);
    ASSERT_NE (index, -1) << join::lastError.message ();

    for (int i = 1; i <= 2; ++i)
    {
        ASSERT_NE (client.asyncWrite ("hello", 5, nullptr), -1) << join::lastError.message ();

        {
            ScopedLock<Mutex> lock (_mut);
            ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [i] () {
                return _completions >= i;
            }));
            ASSERT_FALSE (_code) << _code.message ();
            ASSERT_EQ (_transferred, 5u);
            ASSERT_EQ (std::string (_buf, 5), "hello");
            ASSERT_EQ (_from, UnixDgram::Endpoint (_serverpath));
            ASSERT_TRUE (_more);
        }
    }

    ASSERT_EQ (client.cancelRead (static_cast<size_t> (index)), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 3;
        }));
        ASSERT_EQ (_code, std::errc::operation_canceled);
        ASSERT_FALSE (_more);
    }

#ifdef JOIN_HAS_IO_URING
    for (size_t i = 0; i < UnixDgram::AsyncSocket::_opCount; ++i)
    {
        ASSERT_NE (client.asyncReadFromMulti (0, nullptr), -1) << join::lastError.message ();
    }

    ASSERT_EQ (client.asyncReadFromMulti (0, nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
#endif

    client.close ();

    ASSERT_EQ (client.unregisterBufferRing (0), 0) << join::lastError.message ();
}

/**
 * @brief Test asyncRead method.
 */
TEST_F (UnixAsyncDatagramSocket, asyncRead)
{
    UnixDgram::AsyncSocket client;

    ASSERT_EQ (client.asyncRead (_buf, sizeof (_buf), nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (client.bind (_clientpath), 0) << join::lastError.message ();
    ASSERT_EQ (client.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_NE (client.asyncRead (_buf, sizeof (_buf), onReportRead), -1) << join::lastError.message ();
    ASSERT_NE (client.asyncWrite ("hello", 5, nullptr), -1) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
        ASSERT_EQ (_transferred, 5u);
        ASSERT_EQ (std::string (_buf, 5), "hello");
    }

    ASSERT_EQ (::close (client.handle ()), 0);

    ASSERT_NE (client.asyncRead (_buf, sizeof (_buf), onReportRead), -1) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 2;
        }));
        ASSERT_EQ (_code, std::errc::bad_file_descriptor) << _code.message ();
    }

    client.close ();
}

/**
 * @brief Test asyncReadFrom method resubmitted from its own handler.
 */
TEST_F (UnixAsyncDatagramSocket, resubmit)
{
    UnixDgram::AsyncSocket client;
    UnixDgram::Endpoint dest (_serverpath);

    _current = &client;
    _rearms = 1;

    ASSERT_EQ (client.bind (_clientpath), 0) << join::lastError.message ();
    ASSERT_NE (client.asyncReadFrom (_buf, sizeof (_buf), _from, onRead), -1) << join::lastError.message ();
    ASSERT_NE (client.asyncWriteTo ("one", 3, dest, nullptr), -1) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
    }

    ASSERT_NE (client.asyncWriteTo ("two", 3, dest, nullptr), -1) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 2;
        }));
        ASSERT_FALSE (_code) << _code.message ();
    }

    _dest = UnixDgram::Endpoint (_serverpath);
    _rearms = 1;

    ASSERT_NE (client.asyncWriteTo ("one", 3, _dest, onWrite), -1) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 4;
        }));
        ASSERT_FALSE (_code) << _code.message ();
    }

    client.close ();
    _current = nullptr;
}

/**
 * @brief Test close called from within a write handler.
 */
TEST_F (UnixAsyncDatagramSocket, closeFromWriteHandler)
{
    UnixDgram::AsyncSocket client;
    UnixDgram::Endpoint dest (_serverpath);

    _current = &client;

    ASSERT_NE (client.asyncWriteTo ("hello", 5, dest, onWriteAndClose), -1) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
    }

    ASSERT_FALSE (client.opened ());
    _current = nullptr;
}

/**
 * @brief Test a datagram larger than the supplied buffer.
 */
TEST_F (UnixAsyncDatagramSocket, truncated)
{
    UnixDgram::AsyncSocket client;
    char small[4] = {};

    ASSERT_EQ (client.bind (_clientpath), 0) << join::lastError.message ();
    ASSERT_EQ (client.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_NE (client.asyncReadFrom (small, sizeof (small), _from, onReportFrom), -1) << join::lastError.message ();
    ASSERT_NE (client.asyncWrite ("hello world", 11, nullptr), -1) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_EQ (_code, Errc::MessageTooLong);
    }

    client.close ();
}

/**
 * @brief Test an empty datagram.
 */
TEST_F (UnixAsyncDatagramSocket, empty)
{
    UnixDgram::AsyncSocket client;
    UnixDgram::Socket sender;
    UnixDgram::Endpoint self (_clientpath);

    ASSERT_EQ (client.bind (_clientpath), 0) << join::lastError.message ();
    ASSERT_NE (client.asyncReadFrom (_buf, sizeof (_buf), _from, onReportFrom), -1) << join::lastError.message ();

    ASSERT_EQ (sender.bind (_senderpath), 0) << join::lastError.message ();
    ASSERT_EQ (sender.writeTo ("", 0, self), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
        ASSERT_EQ (_transferred, 0u);
    }

    sender.close ();
    client.close ();
}

/**
 * @brief Test cancelRead method.
 */
TEST_F (UnixAsyncDatagramSocket, cancelRead)
{
    UnixDgram::AsyncSocket client;

    ASSERT_EQ (client.cancelRead (0), 0) << join::lastError.message ();
    ASSERT_EQ (client.bind (_clientpath), 0) << join::lastError.message ();
    ASSERT_NE (client.asyncReadFrom (_buf, sizeof (_buf), _from, onReportFrom), -1) << join::lastError.message ();
    ASSERT_EQ (client.cancelRead (0), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_EQ (_code, std::errc::operation_canceled);
    }

    client.close ();
}

/**
 * @brief Test cancelWrite method.
 */
TEST_F (UnixAsyncDatagramSocket, cancelWrite)
{
    UnixDgram::AsyncSocket client;

    ASSERT_EQ (client.cancelWrite (0), 0) << join::lastError.message ();
    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_EQ (client.cancelWrite (0), 0) << join::lastError.message ();
    client.close ();
}

/**
 * @brief Test setOption method.
 */
TEST_F (UnixAsyncDatagramSocket, setOption)
{
    UnixDgram::AsyncSocket client;

    ASSERT_EQ (client.setOption (UnixDgram::Socket::RcvBuffer, 1500), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_EQ (client.setOption (UnixDgram::Socket::RcvBuffer, 1500), 0) << join::lastError.message ();
    client.close ();
}

/**
 * @brief Test localEndpoint method.
 */
TEST_F (UnixAsyncDatagramSocket, localEndpoint)
{
    UnixDgram::AsyncSocket client;

    ASSERT_EQ (client.bind (_clientpath), 0) << join::lastError.message ();
    ASSERT_EQ (client.localEndpoint (), UnixDgram::Endpoint (_clientpath));
    client.close ();
}

/**
 * @brief Test remoteEndpoint method.
 */
TEST_F (UnixAsyncDatagramSocket, remoteEndpoint)
{
    UnixDgram::AsyncSocket client;

    ASSERT_EQ (client.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_EQ (client.remoteEndpoint (), UnixDgram::Endpoint (_serverpath));
    client.close ();
}

/**
 * @brief Test opened method.
 */
TEST_F (UnixAsyncDatagramSocket, opened)
{
    UnixDgram::AsyncSocket client;

    ASSERT_FALSE (client.opened ());
    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_TRUE (client.opened ());
    client.close ();
    ASSERT_FALSE (client.opened ());
}

/**
 * @brief Test connected method.
 */
TEST_F (UnixAsyncDatagramSocket, connected)
{
    UnixDgram::AsyncSocket client;

    ASSERT_FALSE (client.connected ());
    ASSERT_EQ (client.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_TRUE (client.connected ());
    client.close ();
    ASSERT_FALSE (client.connected ());
}

/**
 * @brief Test canRead method.
 */
TEST_F (UnixAsyncDatagramSocket, canRead)
{
    UnixDgram::AsyncSocket client;

    ASSERT_EQ (client.canRead (), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_EQ (client.canRead (), 0) << join::lastError.message ();
    client.close ();
}

/**
 * @brief Test mtu method.
 */
TEST_F (UnixAsyncDatagramSocket, mtu)
{
    UnixDgram::AsyncSocket client;

    ASSERT_EQ (client.mtu (), -1);
    ASSERT_EQ (client.connect (_serverpath), 0) << join::lastError.message ();
    ASSERT_EQ (client.mtu (), -1);
    client.close ();
}

/**
 * @brief Test ttl method.
 */
TEST_F (UnixAsyncDatagramSocket, ttl)
{
    UnixDgram::AsyncSocket client;

    ASSERT_EQ (client.ttl (), 60);

    UnixDgram::AsyncSocket other (32);

    ASSERT_EQ (other.ttl (), 32);
}

/**
 * @brief Test family method.
 */
TEST_F (UnixAsyncDatagramSocket, family)
{
    UnixDgram::AsyncSocket client;

    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_EQ (client.family (), AF_UNIX);
    client.close ();
}

/**
 * @brief Test type method.
 */
TEST_F (UnixAsyncDatagramSocket, type)
{
    UnixDgram::AsyncSocket client;

    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_EQ (client.type (), SOCK_DGRAM);
    client.close ();
}

/**
 * @brief Test protocol method.
 */
TEST_F (UnixAsyncDatagramSocket, protocol)
{
    UnixDgram::AsyncSocket client;

    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_EQ (client.protocol (), 0);
    client.close ();
}

/**
 * @brief Test handle method.
 */
TEST_F (UnixAsyncDatagramSocket, handle)
{
    UnixDgram::AsyncSocket client;

    ASSERT_EQ (client.handle (), -1);
    ASSERT_EQ (client.open (), 0) << join::lastError.message ();
    ASSERT_GT (client.handle (), -1);
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
