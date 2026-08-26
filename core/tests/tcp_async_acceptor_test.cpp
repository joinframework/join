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
using join::IpAddress;
using join::Tcp;

/**
 * @brief Class used to test the unix asynchronous stream acceptor API.
 */
class TcpAsyncAcceptor : public ::testing::Test
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
        _peerConnected = false;
        _peerFamily = -1;
    }

    /**
     * @brief report a completion to the test thread.
     * @param ec error reported by the acceptor.
     * @param peer accepted socket.
     */
    static void onReport (const std::error_code& ec, Tcp::AsyncSocket&& peer)
    {
        ScopedLock<Mutex> lock (_mut);

        _code = ec;
        _peerConnected = peer.connected ();
        _peerFamily = peer.family ();
        ++_completions;
        _cond.signal ();
    }

    /**
     * @brief handler resubmitting an acceptation from within itself.
     * @param ec error reported by the acceptor.
     * @param peer accepted socket.
     */
    static void onAccept (const std::error_code& ec, Tcp::AsyncSocket&& peer)
    {
        if (!ec)
        {
            _current->asyncAccept (onAccept);
        }

        onReport (ec, std::move (peer));
    }

    /**
     * @brief handler closing the acceptor from within itself.
     * @param ec error reported by the acceptor.
     * @param peer accepted socket.
     */
    static void onAcceptAndClose (const std::error_code& ec, Tcp::AsyncSocket&& peer)
    {
        _current->close ();

        onReport (ec, std::move (peer));
    }

    /// acceptor address.
    static const IpAddress _address;

    /// acceptor port.
    static const uint16_t _port;

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

    /// state of the last accepted socket.
    static bool _peerConnected;

    /// address family of the last accepted socket.
    static int _peerFamily;

    /// acceptor used by the resubmitting handler.
    static Tcp::AsyncAcceptor* _current;
};

const IpAddress TcpAsyncAcceptor::_address = "::1";
const uint16_t TcpAsyncAcceptor::_port = 5033;
const int TcpAsyncAcceptor::_timeout = 1000;
Mutex TcpAsyncAcceptor::_mut;
Condition TcpAsyncAcceptor::_cond;
std::error_code TcpAsyncAcceptor::_code;
int TcpAsyncAcceptor::_completions = 0;
bool TcpAsyncAcceptor::_peerConnected = false;
int TcpAsyncAcceptor::_peerFamily = -1;
Tcp::AsyncAcceptor* TcpAsyncAcceptor::_current = nullptr;

/**
 * @brief Test move with an acceptation in flight.
 */
TEST_F (TcpAsyncAcceptor, move)
{
    Tcp::AsyncAcceptor server;
    Tcp::Socket client (Tcp::Socket::Blocking);

    ASSERT_EQ (server.create ({_address, _port}), 0) << join::lastError.message ();

    ASSERT_EQ (server.asyncAccept ([] (const std::error_code& ec, Tcp::AsyncSocket&& peer) {
        onReport (ec, std::move (peer));
    }),
               0)
        << join::lastError.message ();

    ASSERT_EQ (server.asyncAccept (nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);

    Tcp::AsyncAcceptor moved (std::move (server));

    ASSERT_EQ (moved.asyncAccept (nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);

    ASSERT_EQ (client.connect ({_address, _port}), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
        ASSERT_TRUE (_peerConnected);
    }

    Tcp::AsyncAcceptor assigned;
    assigned = std::move (moved);

    ASSERT_TRUE (assigned.opened ());

    ASSERT_EQ (moved.cancelAccept (), 0) << join::lastError.message ();
    ASSERT_FALSE (moved.opened ());
    moved.close ();

    Tcp::AsyncAcceptor chained (std::move (moved));
    ASSERT_FALSE (chained.opened ());

    Tcp::AsyncAcceptor reassigned;
    reassigned = std::move (chained);
    ASSERT_FALSE (reassigned.opened ());

    client.close ();
    assigned.close ();
}

/**
 * @brief Test create method.
 */
TEST_F (TcpAsyncAcceptor, create)
{
    Tcp::AsyncAcceptor server;

    ASSERT_EQ (server.create ({_address, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (server.create ({_address, _port}), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
}

/**
 * @brief Test close method.
 */
TEST_F (TcpAsyncAcceptor, close)
{
    Tcp::AsyncAcceptor server;

    ASSERT_EQ (server.create ({_address, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (server.opened ());
    server.close ();
    ASSERT_FALSE (server.opened ());
}

/**
 * @brief Test asyncAccept method.
 */
TEST_F (TcpAsyncAcceptor, asyncAccept)
{
    Tcp::AsyncAcceptor server;
    Tcp::Socket client (Tcp::Socket::Blocking);

    ASSERT_EQ (server.asyncAccept (nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (server.create ({_address, _port}), 0) << join::lastError.message ();

    ASSERT_EQ (server.asyncAccept ([] (const std::error_code& ec, Tcp::AsyncSocket&& peer) {
        onReport (ec, std::move (peer));
    }),
               0)
        << join::lastError.message ();

    ASSERT_EQ (server.asyncAccept (nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);

    ASSERT_EQ (client.connect ({_address, _port}), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
        ASSERT_TRUE (_peerConnected);
        ASSERT_EQ (_peerFamily, AF_INET6);
    }

    client.close ();
    server.close ();
}

/**
 * @brief Test asyncAccept method resubmitted from its own handler.
 */
TEST_F (TcpAsyncAcceptor, resubmit)
{
    Tcp::AsyncAcceptor server;
    Tcp::Socket client1 (Tcp::Socket::Blocking);
    Tcp::Socket client2 (Tcp::Socket::Blocking);

    _current = &server;

    ASSERT_EQ (server.create ({_address, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (server.asyncAccept (onAccept), 0) << join::lastError.message ();
    ASSERT_EQ (client1.connect ({_address, _port}), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_FALSE (_code) << _code.message ();
    }

    ASSERT_EQ (client2.connect ({_address, _port}), 0) << join::lastError.message ();

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
TEST_F (TcpAsyncAcceptor, discard)
{
    Tcp::AsyncAcceptor server;
    Tcp::Socket client (Tcp::Socket::Blocking);

    ASSERT_EQ (server.create ({_address, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (server.asyncAccept (nullptr), 0) << join::lastError.message ();
    ASSERT_EQ (server.asyncAccept (nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    ASSERT_EQ (client.connect ({_address, _port}), 0) << join::lastError.message ();

    int rearmed = -1;

    for (int i = 0; (i < 100) && (rearmed == -1); ++i)
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (10));
        rearmed = server.asyncAccept (nullptr);
    }

    ASSERT_EQ (rearmed, 0) << join::lastError.message ();

    client.close ();
    server.close ();
}

/**
 * @brief Test acceptor closed from within its own handler.
 */
TEST_F (TcpAsyncAcceptor, closeFromHandler)
{
    Tcp::AsyncAcceptor server;
    Tcp::Socket client (Tcp::Socket::Blocking);

    _current = &server;

    ASSERT_EQ (server.create ({_address, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (server.asyncAccept (onAcceptAndClose), 0) << join::lastError.message ();
    ASSERT_EQ (client.connect ({_address, _port}), 0) << join::lastError.message ();

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
TEST_F (TcpAsyncAcceptor, cancelAccept)
{
    Tcp::AsyncAcceptor server;

    ASSERT_EQ (server.cancelAccept (), 0) << join::lastError.message ();
    ASSERT_EQ (server.create ({_address, _port}), 0) << join::lastError.message ();

    ASSERT_EQ (server.asyncAccept ([] (const std::error_code& ec, Tcp::AsyncSocket&& peer) {
        onReport (ec, std::move (peer));
    }),
               0)
        << join::lastError.message ();

    ASSERT_EQ (server.cancelAccept (), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [] () {
            return _completions >= 1;
        }));
        ASSERT_EQ (_code, std::errc::operation_canceled);
        ASSERT_FALSE (_peerConnected);
    }

    server.close ();
}

/**
 * @brief Test localEndpoint method.
 */
TEST_F (TcpAsyncAcceptor, localEndpoint)
{
    Tcp::AsyncAcceptor server;

    ASSERT_EQ (server.localEndpoint (), Tcp::Endpoint{});
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (server.create ({_address, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (server.localEndpoint ().ip (), _address);
    ASSERT_EQ (server.localEndpoint ().port (), _port);
    server.close ();
}

/**
 * @brief Test opened method.
 */
TEST_F (TcpAsyncAcceptor, opened)
{
    Tcp::AsyncAcceptor server;

    ASSERT_FALSE (server.opened ());
    ASSERT_EQ (server.create ({_address, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (server.opened ());
    server.close ();
    ASSERT_FALSE (server.opened ());
}

/**
 * @brief Test family method.
 */
TEST_F (TcpAsyncAcceptor, family)
{
    Tcp::AsyncAcceptor server;

    ASSERT_EQ (server.create ({_address, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (server.family (), AF_INET6);
    server.close ();
}

/**
 * @brief Test type method.
 */
TEST_F (TcpAsyncAcceptor, type)
{
    Tcp::AsyncAcceptor server;

    ASSERT_EQ (server.create ({_address, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (server.type (), SOCK_STREAM);
    server.close ();
}

/**
 * @brief Test protocol method.
 */
TEST_F (TcpAsyncAcceptor, protocol)
{
    Tcp::AsyncAcceptor server;

    ASSERT_EQ (server.create ({_address, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (server.protocol (), IPPROTO_TCP);
    server.close ();
}

/**
 * @brief Test handle method.
 */
TEST_F (TcpAsyncAcceptor, handle)
{
    Tcp::AsyncAcceptor server;

    ASSERT_EQ (server.handle (), -1);
    ASSERT_EQ (server.create ({_address, _port}), 0) << join::lastError.message ();
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
