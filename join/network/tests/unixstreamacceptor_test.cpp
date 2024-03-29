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
using join::UnixStream;

std::string path = "/tmp/unixacceptor_test.sock";

/**
 * @brief Assign by move.
 */
TEST (UnixAcceptor, move)
{
    UnixStream::Acceptor server1, server2;

    ASSERT_EQ (server1.create (path), 0) << join::lastError.message ();

    server2 = std::move (server1);
    ASSERT_TRUE (server2.opened ());

    UnixStream::Acceptor server3 = std::move (server2);
    ASSERT_TRUE (server3.opened ());
}

/**
 * @brief Test create method.
 */
TEST (UnixAcceptor, create)
{
    UnixStream::Acceptor server;

    ASSERT_EQ (server.create (path), 0) << join::lastError.message ();
    ASSERT_EQ (server.create (path), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
}
/**
 * @brief Test close method.
 */
TEST (UnixAcceptor, close)
{
    UnixStream::Acceptor server;

    ASSERT_EQ (server.create (path), 0) << join::lastError.message ();
    ASSERT_TRUE (server.opened ());
    server.close ();
    ASSERT_FALSE (server.opened ());
}

/**
 * @brief Test accept method.
 */
TEST (UnixAcceptor, accept)
{
    UnixStream::Socket clientSocket (UnixStream::Socket::Blocking);
    UnixStream::Acceptor server;

    ASSERT_FALSE (server.accept ().connected ());
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (server.create (path), 0) << join::lastError.message ();
    ASSERT_EQ (clientSocket.connect (path), 0) << join::lastError.message ();
    UnixStream::Socket serverSocket = server.accept ();
    ASSERT_TRUE (serverSocket.connected ());
    ASSERT_EQ (serverSocket.localEndpoint ().device (), path);
    clientSocket.close ();
    serverSocket.close ();
    server.close ();
}

/**
 * @brief Test acceptStream method.
 */
TEST (UnixAcceptor, acceptStream)
{
    UnixStream::Socket clientSocket (UnixStream::Socket::Blocking);
    UnixStream::Acceptor server;

    ASSERT_FALSE (server.acceptStream ().connected ());
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (server.create (path), 0) << join::lastError.message ();
    ASSERT_EQ (clientSocket.connect (path), 0) << join::lastError.message ();
    UnixStream::Stream serverStream = server.acceptStream ();
    ASSERT_TRUE (serverStream.connected ());
    ASSERT_EQ (serverStream.socket ().localEndpoint ().device (), path);
    clientSocket.close ();
    serverStream.close ();
    server.close ();
}

/**
 * @brief Test localEndpoint method.
 */
TEST (UnixAcceptor, localEndpoint)
{
    UnixStream::Acceptor server;

    ASSERT_EQ (server.localEndpoint (), UnixStream::Endpoint {});
    ASSERT_EQ (server.create (path), 0) << join::lastError.message ();
    ASSERT_EQ (server.localEndpoint ().device (), path);
    server.close ();
}

/**
 * @brief Test opened method.
 */
TEST (UnixAcceptor, opened)
{
    UnixStream::Acceptor server;

    ASSERT_FALSE (server.opened ());
    ASSERT_EQ (server.create (path), 0) << join::lastError.message ();
    ASSERT_TRUE (server.opened ());
    server.close ();
    ASSERT_FALSE (server.opened ());
}

/**
 * @brief Test family method.
 */
TEST (UnixAcceptor, family)
{
    UnixStream::Acceptor server;

    ASSERT_EQ (server.create (path), 0) << join::lastError.message ();
    ASSERT_EQ (server.family (), AF_UNIX);
    server.close ();
}

/**
 * @brief Test type method.
 */
TEST (UnixAcceptor, type)
{
    UnixStream::Acceptor server;

    ASSERT_EQ (server.create (path), 0) << join::lastError.message ();
    ASSERT_EQ (server.type (), SOCK_STREAM);
    server.close ();
}

/**
 * @brief Test protocol method.
 */
TEST (UnixAcceptor, protocol)
{
    UnixStream::Acceptor server;

    ASSERT_EQ (server.create (path), 0) << join::lastError.message ();
    ASSERT_EQ (server.protocol (), 0);
    server.close ();
}

/**
 * @brief Test handle method.
 */
TEST (UnixAcceptor, handle)
{
    UnixStream::Acceptor server;

    ASSERT_EQ (server.handle (), -1);
    ASSERT_EQ (server.create (path), 0) << join::lastError.message ();
    ASSERT_GT (server.handle (), -1);
    server.close ();
    ASSERT_EQ (server.handle (), -1);
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
