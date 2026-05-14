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
#include <join/proactor.hpp>
#include <join/backoff.hpp>

// C.
#include <sys/eventfd.h>
#include <cstring>
#include <cerrno>

using join::CompletionHandler;
using join::IoOperation;
using join::Proactor;
using join::ProactorThread;

// =========================================================================
//   CLASS     : IoOperation
//   METHOD    : makeAccept
// =========================================================================
IoOperation IoOperation::makeAccept (int fd, sockaddr* addr, socklen_t* addrlen, int flags,
                                     CompletionHandler* handler) noexcept
{
    IoOperation op;
    op.fd = fd;
    op.code = static_cast<uint8_t> (IoOperation::Opcode::Accept);
    op.handler = handler;
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
    op.fd = fd;
    op.code = static_cast<uint8_t> (IoOperation::Opcode::Connect);
    op.handler = handler;
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
    op.fd = fd;
    op.code = static_cast<uint8_t> (IoOperation::Opcode::Read);
    op.handler = handler;
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
    op.fd = fd;
    op.code = static_cast<uint8_t> (IoOperation::Opcode::Write);
    op.handler = handler;
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
    op.fd = fd;
    op.code = static_cast<uint8_t> (IoOperation::Opcode::ReadFixed);
    op.handler = handler;
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
    op.fd = fd;
    op.code = static_cast<uint8_t> (IoOperation::Opcode::WriteFixed);
    op.handler = handler;
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
    op.fd = fd;
    op.code = static_cast<uint8_t> (IoOperation::Opcode::RecvMsg);
    op.handler = handler;
    op.data.msg.msg = msg;
    op.data.msg.flags = flags;
    return op;
}

// =========================================================================
//   CLASS     : IoOperation
//   METHOD    : makeSendmsg
// =========================================================================
IoOperation IoOperation::makeSendmsg (int fd, msghdr* msg, int flags, CompletionHandler* handler) noexcept
{
    IoOperation op;
    op.fd = fd;
    op.code = static_cast<uint8_t> (IoOperation::Opcode::SendMsg);
    op.handler = handler;
    op.data.msg.msg = msg;
    op.data.msg.flags = flags;
    return op;
}

// =========================================================================
//   CLASS     : Proactor
//   METHOD    : Proactor
// =========================================================================
Proactor::Proactor ()
: _wakeup (eventfd (0, EFD_NONBLOCK | EFD_CLOEXEC))
, _commands (_queueSize)
, _readOps (256, nullptr)
, _writeOps (256, nullptr)
{
    if (_wakeup == -1)
    {
        // LCOV_EXCL_START
        throw std::system_error (errno, std::system_category (), "eventfd failed");
        // LCOV_EXCL_STOP
    }

    _reactor.addHandler (_wakeup, this, true, false, false);
}

// =========================================================================
//   CLASS     : Proactor
//   METHOD    : ~Proactor
// =========================================================================
Proactor::~Proactor () noexcept
{
    stop ();

    ::close (_wakeup);
}

// =========================================================================
//   CLASS     : Proactor
//   METHOD    : submit
// =========================================================================
int Proactor::submit (IoOperation* op, bool sync) noexcept
{
    if (isProactorThread ())
    {
        return submitOperation (op);
    }

    std::atomic<bool> done{false}, *pdone = nullptr;
    std::error_code errc, *perrc = nullptr;

    if (JOIN_LIKELY (sync))
    {
        pdone = &done;
        perrc = &errc;
    }

    if (JOIN_UNLIKELY (writeCommand ({CommandType::Submit, op, pdone, perrc}) == -1))
    {
        return -1;  // LCOV_EXCL_LINE
    }

    if (JOIN_LIKELY (sync))
    {
        Backoff backoff;
        while (!done.load (std::memory_order_acquire))
        {
            backoff ();
        }

        if (JOIN_UNLIKELY (errc))
        {
            lastError = errc;
            return -1;
        }
    }

    return 0;
}

// =========================================================================
//   CLASS     : Proactor
//   METHOD    : cancel
// =========================================================================
int Proactor::cancel (IoOperation* op, bool sync) noexcept
{
    if (isProactorThread ())
    {
        return cancelOperation (op);
    }

    std::atomic<bool> done{false}, *pdone = nullptr;
    std::error_code errc, *perrc = nullptr;

    if (JOIN_LIKELY (sync))
    {
        pdone = &done;
        perrc = &errc;
    }

    if (JOIN_UNLIKELY (writeCommand ({CommandType::Cancel, op, pdone, perrc}) == -1))
    {
        return -1;  // LCOV_EXCL_LINE
    }

    if (JOIN_LIKELY (sync))
    {
        Backoff backoff;
        while (!done.load (std::memory_order_acquire))
        {
            backoff ();
        }

        if (JOIN_UNLIKELY (errc))
        {
            lastError = errc;
            return -1;
        }
    }

    return 0;
}

// =========================================================================
//   CLASS     : Proactor
//   METHOD    : run
// =========================================================================
void Proactor::run ()
{
    _running.store (true, std::memory_order_release);
    _reactor.run ();
}

// =========================================================================
//   CLASS     : Proactor
//   METHOD    : stop
// =========================================================================
void Proactor::stop (bool sync) noexcept
{
    bool expected = true;
    if (_running.compare_exchange_strong (expected, false, std::memory_order_acq_rel))
    {
        writeCommand ({CommandType::Stop, nullptr, nullptr, nullptr});
    }

    _reactor.stop (sync);
}

#ifdef JOIN_HAS_NUMA
// =========================================================================
//   CLASS     : Proactor
//   METHOD    : mbind
// =========================================================================
int Proactor::mbind (int numa) const noexcept
{
    return _reactor.mbind (numa);
}
#endif

// =========================================================================
//   CLASS     : Proactor
//   METHOD    : mlock
// =========================================================================
int Proactor::mlock () const noexcept
{
    return _reactor.mlock ();
}

// =========================================================================
//   CLASS     : Proactor
//   METHOD    : isProactorThread
// =========================================================================
bool Proactor::isProactorThread () const noexcept
{
    return _reactor.isReactorThread ();
}

// =========================================================================
//   CLASS     : Proactor
//   METHOD    : submitOperation
// =========================================================================
int Proactor::submitOperation (IoOperation* op) noexcept
{
    if (JOIN_UNLIKELY (op == nullptr))
    {
        lastError = make_error_code (Errc::InvalidParam);
        return -1;
    }

    if (JOIN_UNLIKELY (op->fd < 0))
    {
        lastError = std::make_error_code (std::errc::bad_file_descriptor);
        return -1;
    }

    if (JOIN_UNLIKELY (op->state != IoOperation::State::Idle))
    {
        lastError = make_error_code (Errc::OperationFailed);
        return -1;
    }

    if (JOIN_UNLIKELY (static_cast<size_t> (op->fd) >= _readOps.size ()))
    {
        size_t newSize = static_cast<size_t> (op->fd) + 1;
        _readOps.resize (newSize, nullptr);
        _writeOps.resize (newSize, nullptr);
    }

    bool isWrite = isWriteOp (op->code);

    if (JOIN_UNLIKELY ((isWrite && (_writeOps[op->fd] != nullptr)) || (!isWrite && (_readOps[op->fd] != nullptr))))
    {
        lastError = make_error_code (Errc::InvalidParam);
        return -1;
    }

    if (isWrite)
    {
        _writeOps[op->fd] = op;
    }
    else
    {
        _readOps[op->fd] = op;
    }

    op->state = IoOperation::State::Submitted;

    return _reactor.addHandler (op->fd, this, _readOps[op->fd] != nullptr, _writeOps[op->fd] != nullptr);
}

// =========================================================================
//   CLASS     : Proactor
//   METHOD    : cancelOperation
// =========================================================================
int Proactor::cancelOperation (IoOperation* op) noexcept
{
    if (JOIN_UNLIKELY (op == nullptr))
    {
        lastError = make_error_code (Errc::InvalidParam);
        return -1;
    }

    if (JOIN_UNLIKELY (op->fd < 0 || static_cast<size_t> (op->fd) >= _readOps.size ()))
    {
        lastError = std::make_error_code (std::errc::bad_file_descriptor);
        return -1;
    }

    if (JOIN_UNLIKELY (op->state != IoOperation::State::Submitted))
    {
        lastError = make_error_code (Errc::OperationFailed);
        return -1;
    }

    bool isWrite = isWriteOp (op->code);

    if (JOIN_UNLIKELY ((isWrite && (_writeOps[op->fd] != op)) || (!isWrite && (_readOps[op->fd] != op))))
    {
        lastError = make_error_code (Errc::InvalidParam);
        return -1;
    }

    if (isWrite)
    {
        _writeOps[op->fd] = nullptr;
    }
    else
    {
        _readOps[op->fd] = nullptr;
    }

    op->state = IoOperation::State::Idle;

    if (_readOps[op->fd] == nullptr && _writeOps[op->fd] == nullptr)
    {
        _reactor.delHandler (op->fd);
    }
    else
    {
        _reactor.addHandler (op->fd, this, _readOps[op->fd] != nullptr, _writeOps[op->fd] != nullptr);
    }

    if (op->handler)
    {
        op->handler->onCancel (op, -ECANCELED);
    }

    return 0;
}

// =========================================================================
//   CLASS     : Proactor
//   METHOD    : writeCommand
// =========================================================================
int Proactor::writeCommand (const Command& cmd) noexcept
{
    if (JOIN_UNLIKELY (_commands.push (cmd) == -1))
    {
        return -1;  // LCOV_EXCL_LINE
    }

    uint64_t value = 1;
    if (JOIN_UNLIKELY (::write (_wakeup, &value, sizeof (uint64_t)) == -1))
    {
        // LCOV_EXCL_START
        lastError = std::error_code (errno, std::system_category ());
        return -1;
        // LCOV_EXCL_STOP
    }

    return 0;
}

// =========================================================================
//   CLASS     : Proactor
//   METHOD    : processCommand
// =========================================================================
void Proactor::processCommand (const Command& cmd) noexcept
{
    int err = 0;

    switch (cmd.type)
    {
        case CommandType::Submit:
            err = submitOperation (cmd.op);
            break;

        case CommandType::Cancel:
            err = cancelOperation (cmd.op);
            break;

        case CommandType::Stop:
            for (int fd = 0; fd < static_cast<int> (_readOps.size ()); ++fd)
            {
                if (_readOps[fd] || _writeOps[fd])
                {
                    abortOperations (fd, -ECANCELED);
                }
            }
            break;
    }

    if (JOIN_UNLIKELY (cmd.done))
    {
        if (cmd.errc && (err != 0))
        {
            *cmd.errc = lastError;
        }
        cmd.done->store (true, std::memory_order_release);
    }
}

// =========================================================================
//   CLASS     : Proactor
//   METHOD    : readCommands
// =========================================================================
void Proactor::readCommands () noexcept
{
    uint64_t count;
    if (JOIN_UNLIKELY (::read (_wakeup, &count, sizeof (count)) == -1))
    {
        return;  // LCOV_EXCL_LINE
    }

    Command cmd;
    while (_commands.tryPop (cmd) == 0)
    {
        processCommand (cmd);
    }
}

// =========================================================================
//   CLASS     : Proactor
//   METHOD    : isWriteOp
// =========================================================================
bool Proactor::isWriteOp (uint8_t code) noexcept
{
    return code == static_cast<uint8_t> (IoOperation::Opcode::Connect) ||
           code == static_cast<uint8_t> (IoOperation::Opcode::Write) ||
           code == static_cast<uint8_t> (IoOperation::Opcode::WriteFixed) ||
           code == static_cast<uint8_t> (IoOperation::Opcode::SendMsg);
}

// =========================================================================
//   CLASS     : Proactor
//   METHOD    : executeOperation
// =========================================================================
int Proactor::executeOperation (IoOperation* op) noexcept
{
    for (;;)
    {
        switch (static_cast<IoOperation::Opcode> (op->code))
        {
            case IoOperation::Opcode::Accept:
                {
                    int fd = ::accept4 (op->fd, op->data.accept.addr, op->data.accept.addrlen, op->data.accept.flags);
                    if ((fd == -1) && (errno == EINTR))
                    {
                        continue;  // LCOV_EXCL_LINE
                    }
                    return (fd == -1) ? -errno : fd;
                }

            case IoOperation::Opcode::Connect:
                {
                    int err = ::connect (op->fd, op->data.connect.addr, op->data.connect.addrlen);
                    if (err == -1 && errno == EINTR)
                    {
                        continue;  // LCOV_EXCL_LINE
                    }
                    return err == -1 ? -errno : 0;
                }

            case IoOperation::Opcode::Read:
            case IoOperation::Opcode::ReadFixed:
                {
                    ssize_t n = ::read (op->fd, op->data.rw.buf, op->data.rw.len);
                    if ((n == -1) && (errno == EINTR))
                    {
                        continue;  // LCOV_EXCL_LINE
                    }
                    return (n == -1) ? -errno : static_cast<int> (n);
                }

            case IoOperation::Opcode::Write:
            case IoOperation::Opcode::WriteFixed:
                {
                    ssize_t n = ::write (op->fd, op->data.rw.buf, op->data.rw.len);
                    if ((n == -1) && (errno == EINTR))
                    {
                        continue;  // LCOV_EXCL_LINE
                    }
                    return (n == -1) ? -errno : static_cast<int> (n);
                }

            case IoOperation::Opcode::RecvMsg:
                {
                    ssize_t n = ::recvmsg (op->fd, op->data.msg.msg, op->data.msg.flags);
                    if ((n == -1) && (errno == EINTR))
                    {
                        continue;  // LCOV_EXCL_LINE
                    }
                    return (n == -1) ? -errno : static_cast<int> (n);
                }

            case IoOperation::Opcode::SendMsg:
                {
                    ssize_t n = ::sendmsg (op->fd, op->data.msg.msg, op->data.msg.flags);
                    if ((n == -1) && (errno == EINTR))
                    {
                        continue;  // LCOV_EXCL_LINE
                    }
                    return (n == -1) ? -errno : static_cast<int> (n);
                }

            default:
                return -EINVAL;
        }
    }
}

// =========================================================================
//   CLASS     : Proactor
//   METHOD    : abortOperations
// =========================================================================
void Proactor::abortOperations (int fd, int result) noexcept
{
    if (JOIN_UNLIKELY (fd < 0 || static_cast<size_t> (fd) >= _readOps.size ()))
    {
        return;
    }

    _reactor.delHandler (fd);

    IoOperation* rOp = _readOps[fd];
    _readOps[fd] = nullptr;

    IoOperation* wOp = _writeOps[fd];
    _writeOps[fd] = nullptr;

    for (IoOperation* op : {wOp, rOp})
    {
        if (op)
        {
            op->state = IoOperation::State::Idle;
            if (op->handler)
            {
                op->handler->onComplete (op, result);
            }
        }
    }
}

// =========================================================================
//   CLASS     : Proactor
//   METHOD    : onReadable
// =========================================================================
void Proactor::onReadable (int fd)
{
    if (JOIN_UNLIKELY (fd == _wakeup))
    {
        readCommands ();
        return;
    }

    if (JOIN_UNLIKELY (fd < 0 || static_cast<size_t> (fd) >= _readOps.size ()))
    {
        return;  // LCOV_EXCL_LINE
    }

    IoOperation* op = _readOps[fd];
    _readOps[fd] = nullptr;

    if (_writeOps[fd])
    {
        _reactor.addHandler (fd, this, _readOps[fd] != nullptr, _writeOps[fd] != nullptr);
    }
    else
    {
        _reactor.delHandler (fd);
    }

    if (op)
    {
        op->state = IoOperation::State::Idle;
        if (op->handler)
        {
            op->handler->onComplete (op, executeOperation (op));
        }
    }
}

// =========================================================================
//   CLASS     : Proactor
//   METHOD    : onWriteable
// =========================================================================
void Proactor::onWriteable (int fd)
{
    if (JOIN_UNLIKELY (fd < 0 || static_cast<size_t> (fd) >= _readOps.size ()))
    {
        return;  // LCOV_EXCL_LINE
    }

    IoOperation* op = _writeOps[fd];
    _writeOps[fd] = nullptr;

    if (_readOps[fd])
    {
        _reactor.addHandler (fd, this, _readOps[fd] != nullptr, _writeOps[fd] != nullptr);
    }
    else
    {
        _reactor.delHandler (fd);
    }

    if (op)
    {
        op->state = IoOperation::State::Idle;
        if (op->handler)
        {
            op->handler->onComplete (op, executeOperation (op));
        }
    }
}

// =========================================================================
//   CLASS     : Proactor
//   METHOD    : onClose
// =========================================================================
void Proactor::onClose (int fd)
{
    if (JOIN_UNLIKELY (fd < 0 || static_cast<size_t> (fd) >= _readOps.size ()))
    {
        return;  // LCOV_EXCL_LINE
    }

    abortOperations (fd, -ECONNRESET);
}

// =========================================================================
//   CLASS     : Proactor
//   METHOD    : onError
// =========================================================================
void Proactor::onError (int fd)
{
    if (JOIN_UNLIKELY (fd < 0 || static_cast<size_t> (fd) >= _readOps.size ()))
    {
        return;  // LCOV_EXCL_LINE
    }

    abortOperations (fd, -EIO);
}

// =========================================================================
//   CLASS     : ProactorThread
//   METHOD    : proactor
// =========================================================================
Proactor* ProactorThread::proactor ()
{
    return &instance ()._proactor;
}

// =========================================================================
//   CLASS     : ProactorThread
//   METHOD    : affinity (setter)
// =========================================================================
int ProactorThread::affinity (int core)
{
    return instance ()._dispatcher.affinity (core);
}

// =========================================================================
//   CLASS     : ProactorThread
//   METHOD    : affinity (getter)
// =========================================================================
int ProactorThread::affinity ()
{
    return instance ()._dispatcher.affinity ();
}

// =========================================================================
//   CLASS     : ProactorThread
//   METHOD    : priority (setter)
// =========================================================================
int ProactorThread::priority (int prio)
{
    return instance ()._dispatcher.priority (prio);
}

// =========================================================================
//   CLASS     : ProactorThread
//   METHOD    : priority (getter)
// =========================================================================
int ProactorThread::priority ()
{
    return instance ()._dispatcher.priority ();
}

// =========================================================================
//   CLASS     : ProactorThread
//   METHOD    : handle
// =========================================================================
pthread_t ProactorThread::handle ()
{
    return instance ()._dispatcher.handle ();
}

#ifdef JOIN_HAS_NUMA
// =========================================================================
//   CLASS     : ProactorThread
//   METHOD    : mbind
// =========================================================================
int ProactorThread::mbind (int numa)
{
    return instance ()._proactor.mbind (numa);
}
#endif

// =========================================================================
//   CLASS     : ProactorThread
//   METHOD    : mlock
// =========================================================================
int ProactorThread::mlock ()
{
    return instance ()._proactor.mlock ();
}

// =========================================================================
//   CLASS     : ProactorThread
//   METHOD    : instance
// =========================================================================
ProactorThread& ProactorThread::instance ()
{
    static ProactorThread proactorThread;
    return proactorThread;
}

// =========================================================================
//   CLASS     : ProactorThread
//   METHOD    : ProactorThread
// =========================================================================
ProactorThread::ProactorThread ()
{
    _dispatcher = Thread ([this] () {
        _proactor.run ();
    });
}

// =========================================================================
//   CLASS     : ProactorThread
//   METHOD    : ~ProactorThread
// =========================================================================
ProactorThread::~ProactorThread ()
{
    _proactor.stop ();
    _dispatcher.join ();
}
