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
using join::UnixStream;
using join::Condition;
using join::Mutex;
using join::Thread;
using join::Proactor;

std::string path = "/tmp/unixasyncacceptor_test.sock";

/**
 * @brief Context for async callbacks.
 */
struct AcceptContext
{
    UnixStream::Socket socket;
    bool called = false;
    Condition cv;
    Mutex mtx;
};

/**
 * @brief Static callback for acceptance.
 */
static void onAccept (UnixStream::Socket&& sock, void* ctx) noexcept
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
TEST (UnixAsyncAcceptor, create)
{
    UnixStream::AsyncAcceptor server;

    ASSERT_EQ (server.create (path), 0) << join::lastError.message ();
    ASSERT_EQ (server.create (path), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
}

/**
 * @brief Test close method.
 */
TEST (UnixAsyncAcceptor, close)
{
    UnixStream::AsyncAcceptor server;

    ASSERT_EQ (server.create (path), 0) << join::lastError.message ();
    ASSERT_TRUE (server.opened ());
    server.close ();
    ASSERT_FALSE (server.opened ());
}

/**
 * @brief Test accept method.
 */
TEST (UnixAsyncAcceptor, accept)
{
    Proactor proactor;
    Thread th ([&proactor] () {
        proactor.run ();
    });

    UnixStream::Socket clientSocket (UnixStream::Socket::Blocking);
    UnixStream::AsyncAcceptor server;
    AcceptContext context;

    ASSERT_EQ (server.accept (nullptr, &context), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (server.accept (onAccept, &context), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (server.create (path), 0) << join::lastError.message ();
    ASSERT_EQ (server.accept (onAccept, &context), 0) << join::lastError.message ();
    ASSERT_EQ (server.accept (onAccept, &context), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    ASSERT_EQ (clientSocket.connect (path), 0) << join::lastError.message ();
    {
        join::ScopedLock<Mutex> lock (context.mtx);
        while (!context.called)
        {
            context.cv.wait (lock);
        }
    }
    ASSERT_TRUE (context.socket.connected ());
    ASSERT_EQ (context.socket.localEndpoint ().device (), path);
    clientSocket.close ();
    context.socket.close ();
    server.close ();

    proactor.stop ();
    th.join ();
}

/**
 * @brief Test cancelAccept method.
 */
TEST (UnixAsyncAcceptor, cancelAccept)
{
    Proactor proactor;
    Thread th ([&proactor] () {
        proactor.run ();
    });

    UnixStream::AsyncAcceptor server;
    AcceptContext context;

    ASSERT_EQ (server.accept (nullptr, &context), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (server.accept (onAccept, &context), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (server.create (path), 0) << join::lastError.message ();
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
TEST (UnixAsyncAcceptor, localEndpoint)
{
    UnixStream::AsyncAcceptor server;

    ASSERT_EQ (server.localEndpoint (), UnixStream::Endpoint{});
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (server.create (path), 0) << join::lastError.message ();
    ASSERT_EQ (server.localEndpoint ().device (), path);
    server.close ();
}

/**
 * @brief Test opened method.
 */
TEST (UnixAsyncAcceptor, opened)
{
    UnixStream::AsyncAcceptor server;

    ASSERT_FALSE (server.opened ());
    ASSERT_EQ (server.create (path), 0) << join::lastError.message ();
    ASSERT_TRUE (server.opened ());
    server.close ();
    ASSERT_FALSE (server.opened ());
}

/**
 * @brief Test family method.
 */
TEST (UnixAsyncAcceptor, family)
{
    UnixStream::AsyncAcceptor server;

    ASSERT_EQ (server.create (path), 0) << join::lastError.message ();
    ASSERT_EQ (server.family (), AF_UNIX);
    server.close ();
}

/**
 * @brief Test type method.
 */
TEST (UnixAsyncAcceptor, type)
{
    UnixStream::AsyncAcceptor server;

    ASSERT_EQ (server.create (path), 0) << join::lastError.message ();
    ASSERT_EQ (server.type (), SOCK_STREAM);
    server.close ();
}

/**
 * @brief Test protocol method.
 */
TEST (UnixAsyncAcceptor, protocol)
{
    UnixStream::AsyncAcceptor server;

    ASSERT_EQ (server.create (path), 0) << join::lastError.message ();
    ASSERT_EQ (server.protocol (), 0);
    server.close ();
}

/**
 * @brief Test handle method.
 */
TEST (UnixAsyncAcceptor, handle)
{
    UnixStream::AsyncAcceptor server;

    ASSERT_EQ (server.handle (), -1);
    ASSERT_EQ (server.create (path), 0) << join::lastError.message ();
    ASSERT_GT (server.handle (), -1);
    server.close ();
    ASSERT_EQ (server.handle (), -1);
}

int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
