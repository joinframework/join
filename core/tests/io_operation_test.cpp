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
#include <join/io_operation.hpp>
#include <join/socket.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::IoOperation;
using join::CompletionHandler;
using join::Tcp;

static const std::string host = "127.0.0.1";
static const uint16_t port = 5001;
static char buffer[256] = {};

/**
 * @brief Test makeAccept.
 */
TEST (IoOperation, makeAccept)
{
    sockaddr_storage addr = {};
    socklen_t addrlen = sizeof (addr);

    auto op = IoOperation::makeAccept (8, reinterpret_cast<sockaddr*> (&addr), &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC,
                                       nullptr);

    ASSERT_EQ (op.code, static_cast<uint8_t> (IoOperation::Opcode::Accept));
    ASSERT_EQ (op.handler, nullptr);
    ASSERT_EQ (op.data.accept.fd, 8);
    ASSERT_EQ (op.data.accept.addr, reinterpret_cast<sockaddr*> (&addr));
    ASSERT_EQ (op.data.accept.addrlen, &addrlen);
    ASSERT_EQ (op.data.accept.flags, SOCK_NONBLOCK | SOCK_CLOEXEC);
}

/**
 * @brief Test makeConnect.
 */
TEST (IoOperation, makeConnect)
{
    Tcp::Endpoint endpoint{host, port};

    auto op = IoOperation::makeConnect (8, endpoint.addr (), endpoint.length (), nullptr);

    ASSERT_EQ (op.code, static_cast<uint8_t> (IoOperation::Opcode::Connect));
    ASSERT_EQ (op.handler, nullptr);
    ASSERT_EQ (op.data.connect.fd, 8);
    ASSERT_EQ (op.data.connect.addr, endpoint.addr ());
    ASSERT_EQ (op.data.connect.addrlen, endpoint.length ());
}

/**
 * @brief Test makeRead.
 */
TEST (IoOperation, makeRead)
{
    auto op = IoOperation::makeRead (8, buffer, sizeof (buffer), nullptr);

    ASSERT_EQ (op.code, static_cast<uint8_t> (IoOperation::Opcode::Read));
    ASSERT_EQ (op.handler, nullptr);
    ASSERT_EQ (op.data.rw.fd, 8);
    ASSERT_EQ (op.data.rw.buf, buffer);
    ASSERT_EQ (op.data.rw.len, sizeof (buffer));
    ASSERT_EQ (op.data.rw.index, 0);
    ASSERT_EQ (op.data.rw.fixed, false);
}

/**
 * @brief Test makeWrite.
 */
TEST (IoOperation, makeWrite)
{
    auto op = IoOperation::makeWrite (8, buffer, sizeof (buffer), nullptr);

    ASSERT_EQ (op.code, static_cast<uint8_t> (IoOperation::Opcode::Write));
    ASSERT_EQ (op.handler, nullptr);
    ASSERT_EQ (op.data.rw.fd, 8);
    ASSERT_EQ (op.data.rw.buf, buffer);
    ASSERT_EQ (op.data.rw.len, sizeof (buffer));
    ASSERT_EQ (op.data.rw.index, 0);
    ASSERT_EQ (op.data.rw.fixed, false);
}

/**
 * @brief Test makeReadFixed.
 */
TEST (IoOperation, makeReadFixed)
{
    auto op = IoOperation::makeReadFixed (8, buffer, sizeof (buffer), 6, nullptr);

    ASSERT_EQ (op.code, static_cast<uint8_t> (IoOperation::Opcode::ReadFixed));
    ASSERT_EQ (op.handler, nullptr);
    ASSERT_EQ (op.data.rw.fd, 8);
    ASSERT_EQ (op.data.rw.buf, buffer);
    ASSERT_EQ (op.data.rw.len, sizeof (buffer));
    ASSERT_EQ (op.data.rw.index, 6);
    ASSERT_EQ (op.data.rw.fixed, true);
}

/**
 * @brief Test makeWriteFixed.
 */
TEST (IoOperation, makeWriteFixed)
{
    auto op = IoOperation::makeWriteFixed (8, buffer, sizeof (buffer), 6, nullptr);

    ASSERT_EQ (op.code, static_cast<uint8_t> (IoOperation::Opcode::WriteFixed));
    ASSERT_EQ (op.handler, nullptr);
    ASSERT_EQ (op.data.rw.fd, 8);
    ASSERT_EQ (op.data.rw.buf, buffer);
    ASSERT_EQ (op.data.rw.len, sizeof (buffer));
    ASSERT_EQ (op.data.rw.index, 6);
    ASSERT_EQ (op.data.rw.fixed, true);
}

/**
 * @brief Test makeRecvmsg.
 */
TEST (IoOperation, makeRecvmsg)
{
    iovec iov = {};
    msghdr msg = {};
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    auto op = IoOperation::makeRecvmsg (8, &msg, MSG_DONTWAIT, nullptr);

    ASSERT_EQ (op.code, static_cast<uint8_t> (IoOperation::Opcode::RecvMsg));
    ASSERT_EQ (op.handler, nullptr);
    ASSERT_EQ (op.data.msg.fd, 8);
    ASSERT_EQ (op.data.msg.msg, &msg);
    ASSERT_EQ (op.data.msg.flags, MSG_DONTWAIT);
}

/**
 * @brief Test makeSendmsg.
 */
TEST (IoOperation, makeSendmsg)
{
    const char* payload = "makeSendmsg";
    iovec iov = {.iov_base = const_cast<char*> (payload), .iov_len = strlen (payload)};
    msghdr msg = {};
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    auto op = IoOperation::makeSendmsg (8, &msg, MSG_DONTWAIT, nullptr);

    ASSERT_EQ (op.code, static_cast<uint8_t> (IoOperation::Opcode::SendMsg));
    ASSERT_EQ (op.handler, nullptr);
    ASSERT_EQ (op.data.msg.fd, 8);
    ASSERT_EQ (op.data.msg.msg, &msg);
    ASSERT_EQ (op.data.msg.flags, MSG_DONTWAIT);
}

/**
 * @brief Test makeRecv.
 */
TEST (IoOperation, makeRecv)
{
    auto op = IoOperation::makeRecv (8, buffer, sizeof (buffer), MSG_DONTWAIT, nullptr);

    ASSERT_EQ (op.code, static_cast<uint8_t> (IoOperation::Opcode::Recv));
    ASSERT_EQ (op.handler, nullptr);
    ASSERT_EQ (op.data.stream.fd, 8);
    ASSERT_EQ (op.data.stream.buf, buffer);
    ASSERT_EQ (op.data.stream.len, sizeof (buffer));
    ASSERT_EQ (op.data.stream.flags, MSG_DONTWAIT);
}

/**
 * @brief Test makeSend.
 */
TEST (IoOperation, makeSend)
{
    const char* payload = "makeSend";

    auto op = IoOperation::makeSend (8, payload, strlen (payload), MSG_DONTWAIT, nullptr);

    ASSERT_EQ (op.code, static_cast<uint8_t> (IoOperation::Opcode::Send));
    ASSERT_EQ (op.handler, nullptr);
    ASSERT_EQ (op.data.stream.fd, 8);
    ASSERT_EQ (op.data.stream.buf, payload);
    ASSERT_EQ (op.data.stream.len, strlen (payload));
    ASSERT_EQ (op.data.stream.flags, MSG_DONTWAIT);
}

/**
 * @brief Test fd.
 */
TEST (IoOperation, fd)
{
    IoOperation op;

    op.code = static_cast<uint8_t> (IoOperation::Opcode::Accept);
    op.data.accept.fd = 0;
    ASSERT_EQ (op.fd (), 0);

    op.code = static_cast<uint8_t> (IoOperation::Opcode::Connect);
    op.data.connect.fd = 1;
    ASSERT_EQ (op.fd (), 1);

    op.code = static_cast<uint8_t> (IoOperation::Opcode::Read);
    op.data.rw.fd = 2;
    ASSERT_EQ (op.fd (), 2);

    op.code = static_cast<uint8_t> (IoOperation::Opcode::ReadFixed);
    op.data.rw.fd = 3;
    ASSERT_EQ (op.fd (), 3);

    op.code = static_cast<uint8_t> (IoOperation::Opcode::Write);
    op.data.rw.fd = 4;
    ASSERT_EQ (op.fd (), 4);

    op.code = static_cast<uint8_t> (IoOperation::Opcode::WriteFixed);
    op.data.rw.fd = 5;
    ASSERT_EQ (op.fd (), 5);

    op.code = static_cast<uint8_t> (IoOperation::Opcode::RecvMsg);
    op.data.msg.fd = 6;
    ASSERT_EQ (op.fd (), 6);

    op.code = static_cast<uint8_t> (IoOperation::Opcode::SendMsg);
    op.data.msg.fd = 7;
    ASSERT_EQ (op.fd (), 7);

    op.code = static_cast<uint8_t> (IoOperation::Opcode::Recv);
    op.data.stream.fd = 8;
    ASSERT_EQ (op.fd (), 8);

    op.code = static_cast<uint8_t> (IoOperation::Opcode::Send);
    op.data.stream.fd = 9;
    ASSERT_EQ (op.fd (), 9);

    op.code = 255;
    ASSERT_EQ (op.fd (), -1);
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
