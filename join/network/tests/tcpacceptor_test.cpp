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
using join::IpAddress;
using join::Tcp;

IpAddress address = "::1";
uint16_t  port    = 5000;

/**
 * @brief Assign by move.
 */
TEST (TcpAcceptor, move)
{
    Tcp::Acceptor server1, server2;

    ASSERT_EQ (server1.create ({address, port}), 0) << join::lastError.message ();

    server2 = std::move (server1);
    ASSERT_TRUE (server2.opened ());

    Tcp::Acceptor server3 = std::move (server2);
    ASSERT_TRUE (server3.opened ());
}

/**
 * @brief Test create method.
 */
TEST (TcpAcceptor, create)
{
    Tcp::Acceptor server1, server2;

    ASSERT_EQ (server1.create ({address, port}), 0) << join::lastError.message ();

    ASSERT_EQ (server1.create ({address, port}), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);

    ASSERT_EQ (server2.create ({address, port}), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
}

/**
 * @brief Test close method.
 */
TEST (TcpAcceptor, close)
{
    Tcp::Acceptor server;

    ASSERT_EQ (server.create ({address, port}), 0) << join::lastError.message ();
    ASSERT_TRUE (server.opened ());
    server.close ();
    ASSERT_FALSE (server.opened ());
}

/**
 * @brief Test accept method.
 */
TEST (TcpAcceptor, accept)
{
    Tcp::Socket clientSocket (Tcp::Socket::Blocking);
    Tcp::Acceptor server;

    ASSERT_FALSE (server.accept ().connected ());
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (server.create ({address, port}), 0) << join::lastError.message ();
    ASSERT_EQ (clientSocket.connect ({address, port}), 0) << join::lastError.message ();
    Tcp::Socket serverSocket = server.accept ();
    ASSERT_TRUE (serverSocket.connected ());
    ASSERT_EQ (serverSocket.localEndpoint ().ip (), address);
    ASSERT_EQ (serverSocket.localEndpoint ().port (), port);
    clientSocket.close ();
    serverSocket.close ();
    server.close ();
}

/**
 * @brief Test acceptStream method.
 */
TEST (TcpAcceptor, acceptStream)
{
    Tcp::Socket clientSocket (Tcp::Socket::Blocking);
    Tcp::Acceptor server;

    ASSERT_FALSE (server.acceptStream ().connected ());
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (server.create ({address, port}), 0) << join::lastError.message ();
    ASSERT_EQ (clientSocket.connect ({address, port}), 0) << join::lastError.message ();
    Tcp::Stream serverStream = server.acceptStream ();
    ASSERT_TRUE (serverStream.connected ());
    ASSERT_EQ (serverStream.socket ().localEndpoint ().ip (), address);
    ASSERT_EQ (serverStream.socket ().localEndpoint ().port (), port);
    clientSocket.close ();
    serverStream.close ();
    server.close ();
}

/**
 * @brief Test localEndpoint method.
 */
TEST (TcpAcceptor, localEndpoint)
{
    Tcp::Acceptor server;

    ASSERT_EQ (server.localEndpoint (), Tcp::Endpoint {});
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (server.create ({address, port}), 0) << join::lastError.message ();
    ASSERT_EQ (server.localEndpoint ().ip (), address);
    ASSERT_EQ (server.localEndpoint ().port (), port);
    server.close ();
}

/**
 * @brief Test opened method.
 */
TEST (TcpAcceptor, opened)
{
    Tcp::Acceptor server;

    ASSERT_FALSE (server.opened ());
    ASSERT_EQ (server.create ({address, port}), 0) << join::lastError.message ();
    ASSERT_TRUE (server.opened ());
    server.close ();
    ASSERT_FALSE (server.opened ());
}

/**
 * @brief Test family method.
 */
TEST (TcpAcceptor, family)
{
    Tcp::Acceptor server;

    ASSERT_EQ (server.create ({address, port}), 0) << join::lastError.message ();
    ASSERT_EQ (server.family (), address.family ());
    server.close ();
}

/**
 * @brief Test type method.
 */
TEST (TcpAcceptor, type)
{
    Tcp::Acceptor server;

    ASSERT_EQ (server.create ({address, port}), 0) << join::lastError.message ();
    ASSERT_EQ (server.type (), SOCK_STREAM);
    server.close ();
}

/**
 * @brief Test protocol method.
 */
TEST (TcpAcceptor, protocol)
{
    Tcp::Acceptor server;

    ASSERT_EQ (server.create ({address, port}), 0) << join::lastError.message ();
    ASSERT_EQ (server.protocol (), IPPROTO_TCP);
    server.close ();
}

/**
 * @brief Test handle method.
 */
TEST (TcpAcceptor, handle)
{
    Tcp::Acceptor server;

    ASSERT_EQ (server.handle (), -1);
    ASSERT_EQ (server.create ({address, port}), 0) << join::lastError.message ();
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
