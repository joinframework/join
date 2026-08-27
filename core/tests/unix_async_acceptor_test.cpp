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

// C++.
#include <thread>

// Libraries.
#include <gtest/gtest.h>

using join::Errc;
using join::Mutex;
using join::Condition;
using join::ScopedLock;
using join::UnixStream;

/**
 * @brief Class used to test the unix asynchronous stream acceptor API.
 */
class UnixAsyncAcceptor : public ::testing::Test
{
protected:
    /**
     * @brief Sets up the test fixture.
     */
    void SetUp () override
    {
        ScopedLock<Mutex> lock (_mut);

        _code = {};
        _completions = 0;

        peer ().close ();
        spare ().close ();
    }

    /**
     * @brief Tears down the test suite.
     */
    static void TearDownTestSuite ()
    {
        ::unlink (_path.c_str ());
    }

    /**
     * @brief get the socket receiving the accepted connections.
     * @return the socket receiving the accepted connections.
     */
    static UnixStream::AsyncSocket& peer ()
    {
        static UnixStream::AsyncSocket sock;
        return sock;
    }

    /**
     * @brief get the socket receiving the connection accepted by a resubmitted acceptation.
     * @return the socket receiving the connection accepted by a resubmitted acceptation.
     */
    static UnixStream::AsyncSocket& spare ()
    {
        static UnixStream::AsyncSocket sock;
        return sock;
    }

    /**
     * @brief report a completion to the test thread.
     * @param ec error reported by the acceptor.
     */
    static void onReport (const std::error_code& ec)
    {
        ScopedLock<Mutex> lock (_mut);

        _code = ec;
        ++_completions;
        _cond.signal ();
    }

    /**
     * @brief handler resubmitting an acceptation from within itself.
     * @param ec error reported by the acceptor.
     */
    static void onAccept (const std::error_code& ec)
    {
        if (!ec)
        {
            _current->asyncAccept (spare (), onAccept);
        }

        onReport (ec);
    }

    /**
     * @brief handler closing the acceptor from within itself.
     * @param ec error reported by the acceptor.
     */
    static void onAcceptAndClose (const std::error_code& ec)
    {
        _current->close ();

        onReport (ec);
    }

    /// acceptor path.
    static const std::string _path;

    /// completion timeout.
    static const int _timeout;

    /// condition mutex.
    static Mutex _mut;

    /// condition variable.
    static Condition _cond;

    /// last reported error.
    static std::error_code _code;

    /// number of completions reported.
    static int _completions;

    /// acceptor used by the resubmitting handler.
    static UnixStream::AsyncAcceptor* _current;

};

const std::string UnixAsyncAcceptor::_path = "/tmp/unixasyncacceptor_test.sock";
const int UnixAsyncAcceptor::_timeout = 1000;
Mutex UnixAsyncAcceptor::_mut;
Condition UnixAsyncAcceptor::_cond;
std::error_code UnixAsyncAcceptor::_code;
int UnixAsyncAcceptor::_completions = 0;
UnixStream::AsyncAcceptor* UnixAsyncAcceptor::_current = nullptr;

/**
 * @brief Test create method.
 */
TEST_F (UnixAsyncAcceptor, create)
{
    UnixStream::AsyncAcceptor server;

    ASSERT_EQ (server.create (_path), 0) << join::lastError.message ();
    ASSERT_EQ (server.create (_path), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
}

/**
 * @brief Test close method.
 */
TEST_F (UnixAsyncAcceptor, close)
{
    UnixStream::AsyncAcceptor server;

    ASSERT_EQ (server.create (_path), 0) << join::lastError.message ();
    ASSERT_TRUE (server.opened ());
    server.close ();
    ASSERT_FALSE (server.opened ());
}

/**
 * @brief Test asyncAccept method.
 */
TEST_F (UnixAsyncAcceptor, asyncAccept)
{
    UnixStream::AsyncAcceptor server;
    UnixStream::Socket client (UnixStream::Socket::Blocking);

    ASSERT_EQ (server.asyncAccept (peer (), nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (server.create (_path), 0) << join::lastError.message ();

    ASSERT_EQ (server.asyncAccept (peer (), onReport), 0) << join::lastError.message ();

    ASSERT_EQ (server.asyncAccept (peer (), nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);

    ASSERT_EQ (client.connect (_path), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
    }

    ASSERT_TRUE (peer ().connected ());
    ASSERT_EQ (peer ().family (), AF_UNIX);

    ASSERT_EQ (server.asyncAccept (peer (), nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);

    client.close ();
    server.close ();
}

/**
 * @brief Test asyncAccept method resubmitted from its own handler.
 */
TEST_F (UnixAsyncAcceptor, resubmit)
{
    UnixStream::AsyncAcceptor server;
    UnixStream::Socket client1 (UnixStream::Socket::Blocking);
    UnixStream::Socket client2 (UnixStream::Socket::Blocking);

    _current = &server;

    ASSERT_EQ (server.create (_path), 0) << join::lastError.message ();
    ASSERT_EQ (server.asyncAccept (peer (), onAccept), 0) << join::lastError.message ();
    ASSERT_EQ (client1.connect (_path), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
    }

    ASSERT_EQ (client2.connect (_path), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 2;
        }));
        ASSERT_FALSE (_code) << _code.message ();
    }

    client1.close ();
    client2.close ();
    server.close ();
    _current = nullptr;
}

/**
 * @brief Test asyncAccept method without handler.
 */
TEST_F (UnixAsyncAcceptor, discard)
{
    UnixStream::AsyncAcceptor server;
    UnixStream::Socket client (UnixStream::Socket::Blocking);

    ASSERT_EQ (server.create (_path), 0) << join::lastError.message ();
    ASSERT_EQ (server.asyncAccept (peer (), nullptr), 0) << join::lastError.message ();
    ASSERT_EQ (server.asyncAccept (peer (), nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    ASSERT_EQ (client.connect (_path), 0) << join::lastError.message ();

    int rearmed = -1;

    for (int i = 0; (i < 100) && (rearmed == -1); ++i)
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (10));
        rearmed = server.asyncAccept (spare (), nullptr);
    }

    ASSERT_EQ (rearmed, 0) << join::lastError.message ();

    client.close ();
    server.close ();
}

/**
 * @brief Test acceptor closed from within its own handler.
 */
TEST_F (UnixAsyncAcceptor, closeFromHandler)
{
    UnixStream::AsyncAcceptor server;
    UnixStream::Socket client (UnixStream::Socket::Blocking);

    _current = &server;

    ASSERT_EQ (server.create (_path), 0) << join::lastError.message ();
    ASSERT_EQ (server.asyncAccept (peer (), onAcceptAndClose), 0) << join::lastError.message ();
    ASSERT_EQ (client.connect (_path), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
    }

    ASSERT_FALSE (server.opened ());

    client.close ();
    _current = nullptr;
}

/**
 * @brief Test cancelAccept method.
 */
TEST_F (UnixAsyncAcceptor, cancelAccept)
{
    UnixStream::AsyncAcceptor server;

    ASSERT_EQ (server.cancelAccept (), 0) << join::lastError.message ();
    ASSERT_EQ (server.create (_path), 0) << join::lastError.message ();

    ASSERT_EQ (server.asyncAccept (peer (), onReport), 0) << join::lastError.message ();

    ASSERT_EQ (server.cancelAccept (), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_EQ (_code, std::errc::operation_canceled);
        ASSERT_FALSE (peer ().connected ());
    }

    server.close ();
}

/**
 * @brief Test localEndpoint method.
 */
TEST_F (UnixAsyncAcceptor, localEndpoint)
{
    UnixStream::AsyncAcceptor server;

    ASSERT_EQ (server.localEndpoint ().device (), "");
    ASSERT_EQ (server.create (_path), 0) << join::lastError.message ();
    ASSERT_EQ (server.localEndpoint ().device (), _path);
    server.close ();
}

/**
 * @brief Test opened method.
 */
TEST_F (UnixAsyncAcceptor, opened)
{
    UnixStream::AsyncAcceptor server;

    ASSERT_FALSE (server.opened ());
    ASSERT_EQ (server.create (_path), 0) << join::lastError.message ();
    ASSERT_TRUE (server.opened ());
    server.close ();
    ASSERT_FALSE (server.opened ());
}

/**
 * @brief Test family method.
 */
TEST_F (UnixAsyncAcceptor, family)
{
    UnixStream::AsyncAcceptor server;

    ASSERT_EQ (server.create (_path), 0) << join::lastError.message ();
    ASSERT_EQ (server.family (), AF_UNIX);
    server.close ();
}

/**
 * @brief Test type method.
 */
TEST_F (UnixAsyncAcceptor, type)
{
    UnixStream::AsyncAcceptor server;

    ASSERT_EQ (server.create (_path), 0) << join::lastError.message ();
    ASSERT_EQ (server.type (), SOCK_STREAM);
    server.close ();
}

/**
 * @brief Test protocol method.
 */
TEST_F (UnixAsyncAcceptor, protocol)
{
    UnixStream::AsyncAcceptor server;

    ASSERT_EQ (server.create (_path), 0) << join::lastError.message ();
    ASSERT_EQ (server.protocol (), 0);
    server.close ();
}

/**
 * @brief Test handle method.
 */
TEST_F (UnixAsyncAcceptor, handle)
{
    UnixStream::AsyncAcceptor server;

    ASSERT_EQ (server.handle (), -1);
    ASSERT_EQ (server.create (_path), 0) << join::lastError.message ();
    ASSERT_GT (server.handle (), -1);
    server.close ();
    ASSERT_EQ (server.handle (), -1);
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
