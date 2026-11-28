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
#include <join/io_descriptor.hpp>
#include <join/io_operation.hpp>
#include <join/socket.hpp>

// Libraries.
#include <gtest/gtest.h>

// C.
#include <poll.h>

using join::IoOperation;
using join::IoDescriptor;
using join::CompletionHandler;
using join::Tcp;

static const std::string host = "127.0.0.1";
static const uint16_t port = 5001;
static char buffer[256] = {};

/**
 * @brief Test copy.
 */
TEST (IoOperation, copy)
{
    IoDescriptor descriptor (8);

    msghdr msg = {};
    msg.msg_namelen = sizeof (struct sockaddr_storage);
    msg.msg_controllen = CMSG_SPACE (sizeof (int));

    auto op = IoOperation::makeRecvmsgMulti (descriptor, 3, &msg, MSG_DONTWAIT, nullptr);
    op.state.store (IoOperation::State::Submitted);
    op.resume = IoOperation::State::Submitted;
#ifdef JOIN_HAS_IO_URING
    op.index = 4;
    op.result = 5;
#else
    op.next = &op;
#endif
    op.linked = true;
    op.timer = 6;

    IoOperation copy (op);

    ASSERT_EQ (copy.code, op.code);
    ASSERT_EQ (copy.state.load (), op.state.load ());
    ASSERT_EQ (copy.resume, op.resume);
#ifdef JOIN_HAS_IO_URING
    ASSERT_EQ (copy.index, op.index);
    ASSERT_EQ (copy.result, op.result);
#else
    ASSERT_EQ (copy.next, op.next);
#endif
    ASSERT_EQ (copy.linked, op.linked);
    ASSERT_EQ (copy.multishot, op.multishot);
    ASSERT_EQ (copy.data.msg.group, op.data.msg.group);
    ASSERT_EQ (copy.handler, op.handler);
    ASSERT_EQ (copy.descriptor, op.descriptor);
    ASSERT_EQ (copy.data.msg.msg, op.data.msg.msg);
    ASSERT_EQ (copy.ring, op.ring);
    ASSERT_EQ (copy.timer, op.timer);

    IoOperation assigned;
    assigned = op;

    ASSERT_EQ (assigned.code, op.code);
    ASSERT_EQ (assigned.state.load (), IoOperation::State::Idle);
    ASSERT_EQ (assigned.resume, IoOperation::State::Idle);
#ifdef JOIN_HAS_IO_URING
    ASSERT_EQ (assigned.index, op.index);
    ASSERT_EQ (assigned.result, op.result);
#else
    ASSERT_EQ (assigned.next, op.next);
#endif
    ASSERT_EQ (assigned.linked, op.linked);
    ASSERT_EQ (assigned.multishot, op.multishot);
    ASSERT_EQ (assigned.data.msg.group, op.data.msg.group);
    ASSERT_EQ (assigned.handler, op.handler);
    ASSERT_EQ (assigned.descriptor, op.descriptor);
    ASSERT_EQ (assigned.data.msg.msg, op.data.msg.msg);
    ASSERT_EQ (assigned.ring, op.ring);
    ASSERT_EQ (assigned.timer, op.timer);
}

/**
 * @brief Test makePoll.
 */
TEST (IoOperation, makePoll)
{
    IoDescriptor descriptor (8);
    IoDescriptor other (9);

    auto in = IoOperation::makePoll (descriptor, POLLIN, nullptr);

    ASSERT_EQ (in.code, static_cast<uint8_t> (IoOperation::Opcode::Poll));
    ASSERT_FALSE (in.multishot);
    ASSERT_EQ (in.handler, nullptr);
    ASSERT_EQ (in.descriptor, &descriptor);
    ASSERT_EQ (in.data.poll.events, static_cast<uint32_t> (POLLIN));

    auto out = IoOperation::makePoll (other, POLLOUT, nullptr);

    ASSERT_EQ (out.code, static_cast<uint8_t> (IoOperation::Opcode::Poll));
    ASSERT_FALSE (out.multishot);
    ASSERT_EQ (out.handler, nullptr);
    ASSERT_EQ (out.descriptor, &other);
    ASSERT_EQ (out.data.poll.events, static_cast<uint32_t> (POLLOUT));
}

/**
 * @brief Test makePollMulti.
 */
TEST (IoOperation, makePollMulti)
{
    IoDescriptor descriptor (8);
    IoDescriptor other (9);

    auto in = IoOperation::makePollMulti (descriptor, POLLIN, nullptr);

    ASSERT_EQ (in.code, static_cast<uint8_t> (IoOperation::Opcode::Poll));
    ASSERT_TRUE (in.multishot);
    ASSERT_EQ (in.handler, nullptr);
    ASSERT_EQ (in.descriptor, &descriptor);
    ASSERT_EQ (in.data.poll.events, static_cast<uint32_t> (POLLIN));

    auto out = IoOperation::makePollMulti (other, POLLOUT, nullptr);

    ASSERT_EQ (out.code, static_cast<uint8_t> (IoOperation::Opcode::Poll));
    ASSERT_TRUE (out.multishot);
    ASSERT_EQ (out.handler, nullptr);
    ASSERT_EQ (out.descriptor, &other);
    ASSERT_EQ (out.data.poll.events, static_cast<uint32_t> (POLLOUT));
}

/**
 * @brief Test makeAccept.
 */
TEST (IoOperation, makeAccept)
{
    IoDescriptor descriptor (8);

    sockaddr_storage addr = {};
    socklen_t addrlen = sizeof (addr);

    auto op = IoOperation::makeAccept (descriptor, reinterpret_cast<sockaddr*> (&addr), &addrlen,
                                       SOCK_NONBLOCK | SOCK_CLOEXEC, nullptr);

    ASSERT_EQ (op.code, static_cast<uint8_t> (IoOperation::Opcode::Accept));
    ASSERT_EQ (op.handler, nullptr);
    ASSERT_EQ (op.descriptor, &descriptor);
    ASSERT_EQ (op.data.accept.addr, reinterpret_cast<sockaddr*> (&addr));
    ASSERT_EQ (op.data.accept.addrlen, &addrlen);
    ASSERT_EQ (op.flags, SOCK_NONBLOCK | SOCK_CLOEXEC);
}

/**
 * @brief Test makeAcceptMulti.
 */
TEST (IoOperation, makeAcceptMulti)
{
    IoDescriptor descriptor (8);

    auto op = IoOperation::makeAcceptMulti (descriptor, SOCK_NONBLOCK | SOCK_CLOEXEC, nullptr);

    ASSERT_EQ (op.code, static_cast<uint8_t> (IoOperation::Opcode::Accept));
    ASSERT_TRUE (op.multishot);
    ASSERT_EQ (op.handler, nullptr);
    ASSERT_EQ (op.descriptor, &descriptor);
    ASSERT_EQ (op.data.accept.addr, nullptr);
    ASSERT_EQ (op.data.accept.addrlen, nullptr);
    ASSERT_EQ (op.flags, SOCK_NONBLOCK | SOCK_CLOEXEC);
}

/**
 * @brief Test makeConnect.
 */
TEST (IoOperation, makeConnect)
{
    IoDescriptor descriptor (8);

    Tcp::Endpoint endpoint{host, port};

    auto op = IoOperation::makeConnect (descriptor, endpoint.addr (), endpoint.length (), nullptr);

    ASSERT_EQ (op.code, static_cast<uint8_t> (IoOperation::Opcode::Connect));
    ASSERT_EQ (op.handler, nullptr);
    ASSERT_EQ (op.descriptor, &descriptor);
    ASSERT_EQ (op.data.connect.addr, endpoint.addr ());
    ASSERT_EQ (op.data.connect.addrlen, endpoint.length ());
}

/**
 * @brief Test makeRead.
 */
TEST (IoOperation, makeRead)
{
    IoDescriptor descriptor (8);

    auto op = IoOperation::makeRead (descriptor, buffer, sizeof (buffer), nullptr);

    ASSERT_EQ (op.code, static_cast<uint8_t> (IoOperation::Opcode::Read));
    ASSERT_EQ (op.handler, nullptr);
    ASSERT_EQ (op.descriptor, &descriptor);
    ASSERT_EQ (op.data.rw.buf, buffer);
    ASSERT_EQ (op.data.rw.len, sizeof (buffer));
}

/**
 * @brief Test makeWrite.
 */
TEST (IoOperation, makeWrite)
{
    IoDescriptor descriptor (8);

    auto op = IoOperation::makeWrite (descriptor, buffer, sizeof (buffer), nullptr);

    ASSERT_EQ (op.code, static_cast<uint8_t> (IoOperation::Opcode::Write));
    ASSERT_EQ (op.handler, nullptr);
    ASSERT_EQ (op.descriptor, &descriptor);
    ASSERT_EQ (op.data.rw.buf, buffer);
    ASSERT_EQ (op.data.rw.len, sizeof (buffer));
}

/**
 * @brief Test makeRecvmsg.
 */
TEST (IoOperation, makeRecvmsg)
{
    IoDescriptor descriptor (8);

    iovec iov = {};
    msghdr msg = {};
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    auto op = IoOperation::makeRecvmsg (descriptor, &msg, MSG_DONTWAIT, nullptr);

    ASSERT_EQ (op.code, static_cast<uint8_t> (IoOperation::Opcode::RecvMsg));
    ASSERT_EQ (op.handler, nullptr);
    ASSERT_EQ (op.descriptor, &descriptor);
    ASSERT_EQ (op.data.msg.msg, &msg);
    ASSERT_EQ (op.flags, MSG_DONTWAIT);
}

/**
 * @brief Test makeRecvmsgMulti.
 */
TEST (IoOperation, makeRecvmsgMulti)
{
    IoDescriptor descriptor (8);

    msghdr msg = {};
    msg.msg_namelen = sizeof (struct sockaddr_storage);
    msg.msg_controllen = CMSG_SPACE (sizeof (int));

    auto op = IoOperation::makeRecvmsgMulti (descriptor, 3, &msg, MSG_DONTWAIT, nullptr);

    ASSERT_EQ (op.code, static_cast<uint8_t> (IoOperation::Opcode::RecvMsg));
    ASSERT_TRUE (op.multishot);
    ASSERT_EQ (op.data.msg.group, 3);
    ASSERT_EQ (op.handler, nullptr);
    ASSERT_EQ (op.descriptor, &descriptor);
    ASSERT_EQ (op.data.msg.msg, &msg);
    ASSERT_EQ (op.flags, MSG_DONTWAIT);
    ASSERT_EQ (op.data.msg.namelen, sizeof (struct sockaddr_storage));
    ASSERT_EQ (op.data.msg.controllen, CMSG_SPACE (sizeof (int)));
}

/**
 * @brief Test makeSendmsg.
 */
TEST (IoOperation, makeSendmsg)
{
    IoDescriptor descriptor (8);

    const char* payload = "makeSendmsg";
    iovec iov = {.iov_base = const_cast<char*> (payload), .iov_len = strlen (payload)};
    msghdr msg = {};
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    auto op = IoOperation::makeSendmsg (descriptor, &msg, MSG_DONTWAIT, nullptr);

    ASSERT_EQ (op.code, static_cast<uint8_t> (IoOperation::Opcode::SendMsg));
    ASSERT_EQ (op.handler, nullptr);
    ASSERT_EQ (op.descriptor, &descriptor);
    ASSERT_EQ (op.data.msg.msg, &msg);
    ASSERT_EQ (op.flags, MSG_DONTWAIT);
}

/**
 * @brief Test makeSendmsgZc.
 */
TEST (IoOperation, makeSendmsgZc)
{
    IoDescriptor descriptor (8);

    const char* payload = "makeSendmsgZc";
    iovec iov = {.iov_base = const_cast<char*> (payload), .iov_len = strlen (payload)};
    msghdr msg = {};
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    auto op = IoOperation::makeSendmsgZc (descriptor, &msg, MSG_NOSIGNAL, nullptr);

    ASSERT_EQ (op.code, static_cast<uint8_t> (IoOperation::Opcode::SendMsgZc));
    ASSERT_EQ (op.handler, nullptr);
    ASSERT_FALSE (op.linked);
    ASSERT_EQ (op.descriptor, &descriptor);
    ASSERT_EQ (op.data.msg.msg, &msg);
    ASSERT_EQ (op.flags, MSG_NOSIGNAL);

    op = IoOperation::makeSendmsgZc (descriptor, &msg, MSG_NOSIGNAL, nullptr, true);
    ASSERT_TRUE (op.linked);
}

/**
 * @brief Test makeRecv.
 */
TEST (IoOperation, makeRecv)
{
    IoDescriptor descriptor (8);

    auto op = IoOperation::makeRecv (descriptor, buffer, sizeof (buffer), MSG_DONTWAIT, nullptr);

    ASSERT_EQ (op.code, static_cast<uint8_t> (IoOperation::Opcode::Recv));
    ASSERT_EQ (op.handler, nullptr);
    ASSERT_EQ (op.descriptor, &descriptor);
    ASSERT_EQ (op.data.stream.buf, buffer);
    ASSERT_EQ (op.data.stream.len, sizeof (buffer));
    ASSERT_EQ (op.flags, MSG_DONTWAIT);
}

/**
 * @brief Test makeRecvMulti.
 */
TEST (IoOperation, makeRecvMulti)
{
    IoDescriptor descriptor (8);

    auto op = IoOperation::makeRecvMulti (descriptor, 3, MSG_DONTWAIT, nullptr);

    ASSERT_EQ (op.code, static_cast<uint8_t> (IoOperation::Opcode::Recv));
    ASSERT_TRUE (op.multishot);
    ASSERT_EQ (op.data.stream.group, 3);
    ASSERT_EQ (op.handler, nullptr);
    ASSERT_EQ (op.descriptor, &descriptor);
    ASSERT_EQ (op.data.stream.buf, nullptr);
    ASSERT_EQ (op.data.stream.len, 0U);
    ASSERT_EQ (op.flags, MSG_DONTWAIT);
}

/**
 * @brief Test makeSend.
 */
TEST (IoOperation, makeSend)
{
    IoDescriptor descriptor (8);

    const char* payload = "makeSend";

    auto op = IoOperation::makeSend (descriptor, payload, strlen (payload), MSG_DONTWAIT, nullptr);

    ASSERT_EQ (op.code, static_cast<uint8_t> (IoOperation::Opcode::Send));
    ASSERT_EQ (op.handler, nullptr);
    ASSERT_EQ (op.descriptor, &descriptor);
    ASSERT_EQ (op.data.stream.buf, payload);
    ASSERT_EQ (op.data.stream.len, strlen (payload));
    ASSERT_EQ (op.flags, MSG_DONTWAIT);
}

/**
 * @brief Test makeSendZc.
 */
TEST (IoOperation, makeSendZc)
{
    IoDescriptor descriptor (8);

    const char* payload = "makeSendZc";

    auto op = IoOperation::makeSendZc (descriptor, payload, strlen (payload), MSG_NOSIGNAL, nullptr);

    ASSERT_EQ (op.code, static_cast<uint8_t> (IoOperation::Opcode::SendZc));
    ASSERT_EQ (op.handler, nullptr);
    ASSERT_FALSE (op.linked);
    ASSERT_EQ (op.descriptor, &descriptor);
    ASSERT_EQ (op.data.stream.buf, payload);
    ASSERT_EQ (op.data.stream.len, strlen (payload));
    ASSERT_EQ (op.flags, MSG_NOSIGNAL);

    op = IoOperation::makeSendZc (descriptor, payload, strlen (payload), MSG_NOSIGNAL, nullptr, true);
    ASSERT_TRUE (op.linked);
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
