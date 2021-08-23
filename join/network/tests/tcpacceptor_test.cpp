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
using join::net::IpAddress;
using join::net::Tcp;

IpAddress address = "127.0.0.1";
uint16_t  port    = 5000;

/**
 * @brief Test open method.
 */
TEST (TcpAcceptor, open)
{
    Tcp::Acceptor server;

    ASSERT_EQ (server.open (), 0) << join::lastError.message ();
    ASSERT_EQ (server.open (), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
}
/**
 * @brief Test close method.
 */
TEST (TcpAcceptor, close)
{
    Tcp::Acceptor server;

    ASSERT_EQ (server.open (), 0) << join::lastError.message ();
    ASSERT_EQ (server.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test bind method.
 */
TEST (TcpAcceptor, bind)
{
    Tcp::Acceptor server;

    ASSERT_EQ (server.open (), 0) << join::lastError.message ();
    ASSERT_EQ (server.bind ({address, port}), 0) << join::lastError.message ();
    ASSERT_EQ (server.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test listen method.
 */
TEST (TcpAcceptor, listen)
{
    Tcp::Acceptor server;

    ASSERT_EQ (server.listen (20), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (server.bind ({address, port}), 0) << join::lastError.message ();
    ASSERT_EQ (server.listen (20), 0) << join::lastError.message ();
    ASSERT_EQ (server.close (), 0) << join::lastError.message ();
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
    ASSERT_EQ (server.bind ({address, port}), 0) << join::lastError.message ();
    ASSERT_EQ (server.listen (), 0) << join::lastError.message ();
    ASSERT_EQ (clientSocket.connect ({address, port}), 0) << join::lastError.message ();
    Tcp::Socket serverSocket = server.accept ();
    ASSERT_TRUE (serverSocket.connected ());
    ASSERT_EQ (serverSocket.localEndpoint ().ip (), address);
    ASSERT_EQ (serverSocket.localEndpoint ().port (), port);
    ASSERT_EQ (clientSocket.close (), 0) << join::lastError.message ();
    ASSERT_EQ (serverSocket.close (), 0) << join::lastError.message ();
    ASSERT_EQ (server.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test localEndpoint method.
 */
TEST (TcpAcceptor, localEndpoint)
{
    Tcp::Acceptor server;

    ASSERT_EQ (server.localEndpoint (), Tcp::Endpoint {});
    ASSERT_EQ (server.open (), 0) << join::lastError.message ();
    ASSERT_EQ (server.bind ({address, port}), 0) << join::lastError.message ();
    ASSERT_EQ (server.localEndpoint ().ip (), address);
    ASSERT_EQ (server.localEndpoint ().port (), port);
    ASSERT_EQ (server.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test opened method.
 */
TEST (TcpAcceptor, opened)
{
    Tcp::Acceptor server;

    ASSERT_FALSE (server.opened ());
    ASSERT_EQ (server.open (), 0) << join::lastError.message ();
    ASSERT_TRUE (server.opened ());
    ASSERT_EQ (server.close (), 0) << join::lastError.message ();
    ASSERT_FALSE (server.opened ());
}

/**
 * @brief Test family method.
 */
TEST (TcpAcceptor, family)
{
    Tcp::Acceptor server;

    ASSERT_EQ (server.bind ({address, port}), 0) << join::lastError.message ();
    ASSERT_EQ (server.family (), AF_INET);
    ASSERT_EQ (server.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test type method.
 */
TEST (TcpAcceptor, type)
{
    Tcp::Acceptor server;

    ASSERT_EQ (server.bind ({address, port}), 0) << join::lastError.message ();
    ASSERT_EQ (server.type (), SOCK_STREAM);
    ASSERT_EQ (server.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test protocol method.
 */
TEST (TcpAcceptor, protocol)
{
    Tcp::Acceptor server;

    ASSERT_EQ (server.bind ({address, port}), 0) << join::lastError.message ();
    ASSERT_EQ (server.protocol (), IPPROTO_TCP);
    ASSERT_EQ (server.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test handle method.
 */
TEST (TcpAcceptor, handle)
{
    Tcp::Acceptor server;

    ASSERT_EQ (server.handle (), -1);
    ASSERT_EQ (server.open (), 0) << join::lastError.message ();
    ASSERT_GT (server.handle (), -1);
    ASSERT_EQ (server.close (), 0) << join::lastError.message ();
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
