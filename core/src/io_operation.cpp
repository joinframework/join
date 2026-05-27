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
#include <join/proactor.hpp>

using join::IoOperation;
using join::CompletionHandler;

// =========================================================================
//   CLASS     : IoOperation
//   METHOD    : makeAccept
// =========================================================================
IoOperation IoOperation::makeAccept (int fd, sockaddr* addr, socklen_t* addrlen, int flags,
                                     CompletionHandler* handler) noexcept
{
    IoOperation op;
    op.code = static_cast<uint8_t> (IoOperation::Opcode::Accept);
    op.handler = handler;
    op.data.accept.fd = fd;
    op.data.accept.addr = addr;
    op.data.accept.addrlen = addrlen;
    op.data.accept.flags = flags;
    return op;
}

// =========================================================================
//   CLASS     : IoOperation
//   METHOD    : makeConnect
// =========================================================================
IoOperation IoOperation::makeConnect (int fd, const sockaddr* addr, socklen_t addrlen,
                                      CompletionHandler* handler) noexcept
{
    IoOperation op;
    op.code = static_cast<uint8_t> (IoOperation::Opcode::Connect);
    op.handler = handler;
    op.data.connect.fd = fd;
    op.data.connect.addr = addr;
    op.data.connect.addrlen = addrlen;
    return op;
}

// =========================================================================
//   CLASS     : IoOperation
//   METHOD    : makeRead
// =========================================================================
IoOperation IoOperation::makeRead (int fd, void* buf, uint32_t len, CompletionHandler* handler) noexcept
{
    IoOperation op;
    op.code = static_cast<uint8_t> (IoOperation::Opcode::Read);
    op.handler = handler;
    op.data.rw.fd = fd;
    op.data.rw.buf = buf;
    op.data.rw.len = len;
    op.data.rw.index = 0;
    op.data.rw.fixed = false;
    return op;
}

// =========================================================================
//   CLASS     : IoOperation
//   METHOD    : makeWrite
// =========================================================================
IoOperation IoOperation::makeWrite (int fd, const void* buf, uint32_t len, CompletionHandler* handler) noexcept
{
    IoOperation op;
    op.code = static_cast<uint8_t> (IoOperation::Opcode::Write);
    op.handler = handler;
    op.data.rw.fd = fd;
    op.data.rw.buf = const_cast<void*> (buf);
    op.data.rw.len = len;
    op.data.rw.index = 0;
    op.data.rw.fixed = false;
    return op;
}

// =========================================================================
//   CLASS     : IoOperation
//   METHOD    : makeReadFixed
// =========================================================================
IoOperation IoOperation::makeReadFixed (int fd, void* buf, uint32_t len, uint16_t index,
                                        CompletionHandler* handler) noexcept
{
    IoOperation op;
    op.code = static_cast<uint8_t> (IoOperation::Opcode::ReadFixed);
    op.handler = handler;
    op.data.rw.fd = fd;
    op.data.rw.buf = buf;
    op.data.rw.len = len;
    op.data.rw.index = index;
    op.data.rw.fixed = true;
    return op;
}

// =========================================================================
//   CLASS     : IoOperation
//   METHOD    : makeWriteFixed
// =========================================================================
IoOperation IoOperation::makeWriteFixed (int fd, const void* buf, uint32_t len, uint16_t index,
                                         CompletionHandler* handler) noexcept
{
    IoOperation op;
    op.code = static_cast<uint8_t> (IoOperation::Opcode::WriteFixed);
    op.handler = handler;
    op.data.rw.fd = fd;
    op.data.rw.buf = const_cast<void*> (buf);
    op.data.rw.len = len;
    op.data.rw.index = index;
    op.data.rw.fixed = true;
    return op;
}

// =========================================================================
//   CLASS     : IoOperation
//   METHOD    : makeRecvmsg
// =========================================================================
IoOperation IoOperation::makeRecvmsg (int fd, msghdr* msg, int flags, CompletionHandler* handler) noexcept
{
    IoOperation op;
    op.code = static_cast<uint8_t> (IoOperation::Opcode::RecvMsg);
    op.handler = handler;
    op.data.msg.fd = fd;
    op.data.msg.msg = msg;
    op.data.msg.flags = flags;
    return op;
}

// =========================================================================
//   CLASS     : IoOperation
//   METHOD    : makeSendmsg
// =========================================================================
IoOperation IoOperation::makeSendmsg (int fd, const msghdr* msg, int flags, CompletionHandler* handler) noexcept
{
    IoOperation op;
    op.code = static_cast<uint8_t> (IoOperation::Opcode::SendMsg);
    op.handler = handler;
    op.data.msg.fd = fd;
    op.data.msg.msg = const_cast<msghdr*> (msg);
    op.data.msg.flags = flags;
    return op;
}

// =========================================================================
//   CLASS     : IoOperation
//   METHOD    : makeRecv
// =========================================================================
IoOperation IoOperation::makeRecv (int fd, void* buf, uint32_t len, int flags, CompletionHandler* handler) noexcept
{
    IoOperation op;
    op.code = static_cast<uint8_t> (IoOperation::Opcode::Recv);
    op.handler = handler;
    op.data.stream.fd = fd;
    op.data.stream.buf = buf;
    op.data.stream.len = len;
    op.data.stream.flags = flags;
    return op;
}

// =========================================================================
//   CLASS     : IoOperation
//   METHOD    : makeSend
// =========================================================================
IoOperation IoOperation::makeSend (int fd, const void* buf, uint32_t len, int flags,
                                   CompletionHandler* handler) noexcept
{
    IoOperation op;
    op.code = static_cast<uint8_t> (IoOperation::Opcode::Send);
    op.handler = handler;
    op.data.stream.fd = fd;
    op.data.stream.buf = const_cast<void*> (buf);
    op.data.stream.len = len;
    op.data.stream.flags = flags;
    return op;
}

// =========================================================================
//   CLASS     : IoOperation
//   METHOD    : fd
// =========================================================================
int IoOperation::fd () const noexcept
{
    switch (static_cast<Opcode> (code))
    {
        case Opcode::Accept:
            return data.accept.fd;
        case Opcode::Connect:
            return data.connect.fd;
        case Opcode::Read:
        case Opcode::ReadFixed:
        case Opcode::Write:
        case Opcode::WriteFixed:
            return data.rw.fd;
        case Opcode::RecvMsg:
        case Opcode::SendMsg:
            return data.msg.fd;
        case Opcode::Recv:
        case Opcode::Send:
            return data.stream.fd;
        default:
            return -1;
    }
}
