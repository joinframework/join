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

using join::net::UnixStream;

std::string path = "/tmp/unixacceptor_test.sock";

/**
 * @brief Test open method.
 */
TEST (UnixAcceptor, open)
{
    UnixStream::Acceptor server;

    ASSERT_EQ (server.open (), 0) << join::lastError.message ();
}
/**
 * @brief Test close method.
 */
TEST (UnixAcceptor, close)
{
    UnixStream::Acceptor server;

    ASSERT_EQ (server.open (), 0) << join::lastError.message ();
    ASSERT_EQ (server.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test bind method.
 */
TEST (UnixAcceptor, bind)
{
    UnixStream::Acceptor server;

    ASSERT_EQ (server.open (), 0) << join::lastError.message ();
    ASSERT_EQ (server.bind (path), 0) << join::lastError.message ();
    ASSERT_EQ (server.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test listen method.
 */
TEST (UnixAcceptor, listen)
{
    UnixStream::Acceptor server;

    ASSERT_EQ (server.bind (path), 0) << join::lastError.message ();
    ASSERT_EQ (server.listen (20), 0) << join::lastError.message ();
    ASSERT_EQ (server.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test accept method.
 */
TEST (UnixAcceptor, accept)
{
    UnixStream::Socket clientSocket (UnixStream::Socket::Blocking), serverSocket;
    UnixStream::Acceptor server;

    ASSERT_EQ (server.bind (path), 0) << join::lastError.message ();
    ASSERT_EQ (server.listen (), 0) << join::lastError.message ();
    ASSERT_EQ (clientSocket.connect (path), 0) << join::lastError.message ();
    ASSERT_NO_THROW (serverSocket = server.accept ());
    ASSERT_TRUE (serverSocket.connected ());
    ASSERT_EQ (serverSocket.localEndpoint ().device (), path);
    ASSERT_EQ (clientSocket.close (), 0) << join::lastError.message ();
    ASSERT_EQ (serverSocket.close (), 0) << join::lastError.message ();
    ASSERT_EQ (server.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test localEndpoint method.
 */
TEST (UnixAcceptor, localEndpoint)
{
    UnixStream::Acceptor server;

    ASSERT_EQ (server.open (), 0) << join::lastError.message ();
    ASSERT_EQ (server.bind (path), 0) << join::lastError.message ();
    ASSERT_EQ (server.localEndpoint ().device (), path);
    ASSERT_EQ (server.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test opened method.
 */
TEST (UnixAcceptor, opened)
{
    UnixStream::Acceptor server;

    ASSERT_FALSE (server.opened ());
    ASSERT_EQ (server.open (), 0) << join::lastError.message ();
    ASSERT_TRUE (server.opened ());
    ASSERT_EQ (server.close (), 0) << join::lastError.message ();
    ASSERT_FALSE (server.opened ());
}

/**
 * @brief Test family method.
 */
TEST (UnixAcceptor, family)
{
    UnixStream::Acceptor server;

    ASSERT_EQ (server.bind (path), 0) << join::lastError.message ();
    ASSERT_EQ (server.family (), AF_UNIX);
    ASSERT_EQ (server.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test type method.
 */
TEST (UnixAcceptor, type)
{
    UnixStream::Acceptor server;

    ASSERT_EQ (server.bind (path), 0) << join::lastError.message ();
    ASSERT_EQ (server.type (), SOCK_STREAM);
    ASSERT_EQ (server.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test protocol method.
 */
TEST (UnixAcceptor, protocol)
{
    UnixStream::Acceptor server;

    ASSERT_EQ (server.bind (path), 0) << join::lastError.message ();
    ASSERT_EQ (server.protocol (), 0);
    ASSERT_EQ (server.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test handle method.
 */
TEST (UnixAcceptor, handle)
{
    UnixStream::Acceptor server;

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