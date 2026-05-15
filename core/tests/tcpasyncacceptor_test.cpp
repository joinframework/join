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
#include <join/asyncacceptor.hpp>
#include <join/condition.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::Errc;
using join::IpAddress;
using join::Tcp;
using join::Condition;
using join::Mutex;
using join::Thread;
using join::Proactor;

IpAddress address = "::1";
uint16_t port = 5000;

/**
 * @brief Context for async callbacks.
 */
struct AcceptContext
{
    Tcp::Socket socket;
    bool called = false;
    Condition cv;
    Mutex mtx;
};

/**
 * @brief Static callback for acceptance.
 */
static void onAccept (Tcp::Socket&& sock, void* ctx) noexcept
{
    auto* context = static_cast<AcceptContext*> (ctx);
    join::ScopedLock<Mutex> lock (context->mtx);
    context->socket = std::move (sock);
    context->called = true;
    context->cv.signal ();
}

/**
 * @brief Test create method.
 */
TEST (TcpAsyncAcceptor, create)
{
    Tcp::AsyncAcceptor server1, server2;

    ASSERT_EQ (server1.create ({address, port}), 0) << join::lastError.message ();
    ASSERT_EQ (server1.create ({address, port}), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    ASSERT_EQ (server2.create ({address, port}), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
}

/**
 * @brief Test close method.
 */
TEST (TcpAsyncAcceptor, close)
{
    Tcp::AsyncAcceptor server;

    ASSERT_EQ (server.create ({address, port}), 0) << join::lastError.message ();
    ASSERT_TRUE (server.opened ());
    server.close ();
    ASSERT_FALSE (server.opened ());
}

/**
 * @brief Test accept method.
 */
TEST (TcpAsyncAcceptor, accept)
{
    Proactor proactor;
    Thread th ([&proactor] () {
        proactor.run ();
    });

    Tcp::Socket clientSocket (Tcp::Socket::Blocking);
    Tcp::AsyncAcceptor server;
    AcceptContext context;

    ASSERT_EQ (server.accept (nullptr, &context), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (server.accept (onAccept, &context), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (server.create ({address, port}), 0) << join::lastError.message ();
    ASSERT_EQ (server.accept (onAccept, &context), 0) << join::lastError.message ();
    ASSERT_EQ (server.accept (onAccept, &context), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    ASSERT_EQ (clientSocket.connect ({address, port}), 0) << join::lastError.message ();
    {
        join::ScopedLock<Mutex> lock (context.mtx);
        while (!context.called)
        {
            context.cv.wait (lock);
        }
    }
    ASSERT_TRUE (context.socket.connected ());
    ASSERT_EQ (context.socket.localEndpoint ().ip (), address);
    ASSERT_EQ (context.socket.localEndpoint ().port (), port);
    clientSocket.close ();
    context.socket.close ();
    server.close ();

    proactor.stop ();
    th.join ();
}

/**
 * @brief Test cancelAccept method.
 */
TEST (TcpAsyncAcceptor, cancelAccept)
{
    Proactor proactor;
    Thread th ([&proactor] () {
        proactor.run ();
    });

    Tcp::AsyncAcceptor server;
    AcceptContext context;

    ASSERT_EQ (server.accept (nullptr, &context), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (server.accept (onAccept, &context), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (server.create ({address, port}), 0) << join::lastError.message ();
    ASSERT_EQ (server.accept (onAccept, &context), 0) << join::lastError.message ();
    ASSERT_EQ (server.accept (onAccept, &context), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    ASSERT_EQ (server.cancelAccept (), 0) << join::lastError.message ();
    {
        join::ScopedLock<Mutex> lock (context.mtx);
        while (!context.called)
        {
            context.cv.wait (lock);
        }
    }
    ASSERT_FALSE (context.socket.connected ());
    server.close ();

    proactor.stop ();
    th.join ();
}

/**
 * @brief Test localEndpoint method.
 */
TEST (TcpAsyncAcceptor, localEndpoint)
{
    Tcp::AsyncAcceptor server;

    ASSERT_EQ (server.localEndpoint (), Tcp::Endpoint{});
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (server.create ({address, port}), 0) << join::lastError.message ();
    ASSERT_EQ (server.localEndpoint ().ip (), address);
    ASSERT_EQ (server.localEndpoint ().port (), port);
    server.close ();
}

/**
 * @brief Test opened method.
 */
TEST (TcpAsyncAcceptor, opened)
{
    Tcp::AsyncAcceptor server;

    ASSERT_FALSE (server.opened ());
    ASSERT_EQ (server.create ({address, port}), 0) << join::lastError.message ();
    ASSERT_TRUE (server.opened ());
    server.close ();
    ASSERT_FALSE (server.opened ());
}

/**
 * @brief Test family method.
 */
TEST (TcpAsyncAcceptor, family)
{
    Tcp::AsyncAcceptor server;

    ASSERT_EQ (server.create ({address, port}), 0) << join::lastError.message ();
    ASSERT_EQ (server.family (), address.family ());
    server.close ();
}

/**
 * @brief Test type method.
 */
TEST (TcpAsyncAcceptor, type)
{
    Tcp::AsyncAcceptor server;

    ASSERT_EQ (server.create ({address, port}), 0) << join::lastError.message ();
    ASSERT_EQ (server.type (), SOCK_STREAM);
    server.close ();
}

/**
 * @brief Test protocol method.
 */
TEST (TcpAsyncAcceptor, protocol)
{
    Tcp::AsyncAcceptor server;

    ASSERT_EQ (server.create ({address, port}), 0) << join::lastError.message ();
    ASSERT_EQ (server.protocol (), IPPROTO_TCP);
    server.close ();
}

/**
 * @brief Test handle method.
 */
TEST (TcpAsyncAcceptor, handle)
{
    Tcp::AsyncAcceptor server;

    ASSERT_EQ (server.handle (), -1);
    ASSERT_EQ (server.create ({address, port}), 0) << join::lastError.message ();
    ASSERT_GT (server.handle (), -1);
    server.close ();
    ASSERT_EQ (server.handle (), -1);
}

int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
