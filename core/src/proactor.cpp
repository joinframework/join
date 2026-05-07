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
#include <unistd.h>
#include <cassert>
#include <cerrno>

using join::CompletionHandler;
using join::IoOperation;
using join::Proactor;
using join::ProactorThread;

// =========================================================================
//   CLASS     : IoOperation
//   METHOD    : makeTimeout
// =========================================================================
IoOperation IoOperation::makeTimeout (const __kernel_timespec& ts, uint32_t flags, CompletionHandler* handler) noexcept
{
    IoOperation op;
    op.code = IORING_OP_TIMEOUT;
    op.handler = handler;
    op.data.timeout.ts = ts;
    op.data.timeout.flags = flags;
    return op;
}

// =========================================================================
//   CLASS     : IoOperation
//   METHOD    : makeRead
// =========================================================================
IoOperation IoOperation::makeRead (int fd, void* buf, uint32_t len, CompletionHandler* handler) noexcept
{
    IoOperation op;
    op.code = IORING_OP_READ;
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
//   METHOD    : makeReadFixed
// =========================================================================
IoOperation IoOperation::makeReadFixed (int fd, void* buf, uint32_t len, uint16_t buf_index,
                                        CompletionHandler* handler) noexcept
{
    IoOperation op;
    op.code = IORING_OP_READ_FIXED;
    op.handler = handler;
    op.data.rw.fd = fd;
    op.data.rw.buf = buf;
    op.data.rw.len = len;
    op.data.rw.index = buf_index;
    op.data.rw.fixed = true;
    return op;
}

// =========================================================================
//   CLASS     : IoOperation
//   METHOD    : makeWrite
// =========================================================================
IoOperation IoOperation::makeWrite (int fd, const void* buf, uint32_t len, CompletionHandler* handler) noexcept
{
    IoOperation op;
    op.code = IORING_OP_WRITE;
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
//   METHOD    : makeWriteFixed
// =========================================================================
IoOperation IoOperation::makeWriteFixed (int fd, const void* buf, uint32_t len, uint16_t index,
                                         CompletionHandler* handler) noexcept
{
    IoOperation op;
    op.code = IORING_OP_WRITE_FIXED;
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
//   METHOD    : makeSendmsg
// =========================================================================
IoOperation IoOperation::makeSendmsg (int fd, msghdr* msg, uint32_t flags, CompletionHandler* handler) noexcept
{
    IoOperation op;
    op.code = IORING_OP_SENDMSG;
    op.handler = handler;
    op.data.msg.fd = fd;
    op.data.msg.msg = msg;
    op.data.msg.flags = flags;
    return op;
}

// =========================================================================
//   CLASS     : IoOperation
//   METHOD    : makeRecvmsg
// =========================================================================
IoOperation IoOperation::makeRecvmsg (int fd, msghdr* msg, uint32_t flags, CompletionHandler* handler) noexcept
{
    IoOperation op;
    op.code = IORING_OP_RECVMSG;
    op.handler = handler;
    op.data.msg.fd = fd;
    op.data.msg.msg = msg;
    op.data.msg.flags = flags;
    return op;
}

// =========================================================================
//   CLASS     : IoOperation
//   METHOD    : makeAccept
// =========================================================================
IoOperation IoOperation::makeAccept (int fd, sockaddr* addr, socklen_t* addrlen, uint32_t flags,
                                     CompletionHandler* handler) noexcept
{
    IoOperation op;
    op.code = IORING_OP_ACCEPT;
    op.handler = handler;
    op.data.accept.fd = fd;
    op.data.accept.addr = addr;
    op.data.accept.addrlen = addrlen;
    op.data.accept.flags = flags;
    return op;
}

// =========================================================================
//   CLASS     : IoOperation
//   METHOD    : makeCancel
// =========================================================================
// IoOperation IoOperation::makeCancel (IoOperation* target, CompletionHandler* handler) noexcept
// {
//     IoOperation op;
//     op.code = IORING_OP_ASYNC_CANCEL;
//     op.handler = handler;
//     op.data.cancel.target = target;
//     return op;
// }

// =========================================================================
//   CLASS     : Proactor
//   METHOD    : Proactor
// =========================================================================
Proactor::Proactor ()
: _wakeup (eventfd (0, EFD_NONBLOCK | EFD_CLOEXEC))
, _commands (_queueSize)
{
    if (_wakeup == -1)
    {
        throw std::system_error (errno, std::system_category (), "eventfd failed");
    }

    if (io_uring_queue_init (_ringSize, &_ring, 0) != 0)
    {
        int err = errno;
        ::close (_wakeup);
        throw std::system_error (err, std::system_category (), "io_uring_queue_init failed");
    }

    _wakeupOp = IoOperation::makeRead (_wakeup, &_wakeupBuf, sizeof (_wakeupBuf), nullptr);
}

// =========================================================================
//   CLASS     : Proactor
//   METHOD    : ~Proactor
// =========================================================================
Proactor::~Proactor () noexcept
{
    stop ();

    io_uring_queue_exit (&_ring);

    ::close (_wakeup);
}

// =========================================================================
//   CLASS     : Proactor
//   METHOD    : submit
// =========================================================================
int Proactor::submit (IoOperation* op, bool sync) noexcept
{
    if (JOIN_UNLIKELY (op == nullptr))
    {
        lastError = make_error_code (Errc::InvalidParam);
        return -1;
    }

    if (isProactorThread ())
    {
        return submitOperation (op);
    }

    std::atomic<bool> done{false}, *pdone = nullptr;
    std::atomic<int> errc{0}, *perrc = nullptr;

    if (JOIN_LIKELY (sync))
    {
        pdone = &done;
        perrc = &errc;
    }

    if (JOIN_UNLIKELY (writeCommand ({CommandType::Submit, op, pdone, perrc}) == -1))
    {
        return -1;
    }

    if (JOIN_LIKELY (sync))
    {
        Backoff backoff;
        while (!done.load (std::memory_order_acquire))
        {
            backoff ();
        }

        int err = errc.load (std::memory_order_acquire);
        if (JOIN_UNLIKELY (err != 0))
        {
            lastError = std::make_error_code (static_cast<std::errc> (err));
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
    if (JOIN_UNLIKELY (op == nullptr))
    {
        lastError = make_error_code (Errc::InvalidParam);
        return -1;
    }

    if (isProactorThread ())
    {
        return cancelOperation (op);
    }

    std::atomic<bool> done{false}, *pdone = nullptr;
    std::atomic<int> errc{0}, *perrc = nullptr;

    if (JOIN_LIKELY (sync))
    {
        pdone = &done;
        perrc = &errc;
    }

    if (JOIN_UNLIKELY (writeCommand ({CommandType::Cancel, op, pdone, perrc}) == -1))
    {
        return -1;
    }

    if (JOIN_LIKELY (sync))
    {
        Backoff backoff;
        while (!done.load (std::memory_order_acquire))
        {
            backoff ();
        }

        int err = errc.load (std::memory_order_acquire);
        if (JOIN_UNLIKELY (err != 0))
        {
            lastError = std::make_error_code (static_cast<std::errc> (err));
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
    _threadId.store (pthread_self (), std::memory_order_release);

    _running.store (true, std::memory_order_release);
    rearmWakeup ();
    eventLoop ();

    _threadId.store (0, std::memory_order_release);
}

// =========================================================================
//   CLASS     : Proactor
//   METHOD    : stop
// =========================================================================
void Proactor::stop (bool sync) noexcept
{
    _running.store (false, std::memory_order_release);

    if (isProactorThread ())
    {
        return;
    }

    writeCommand ({CommandType::Stop, nullptr, nullptr, nullptr});

    if (JOIN_LIKELY (sync))
    {
        Backoff backoff;
        while (_threadId.load (std::memory_order_acquire) != 0)
        {
            backoff ();
        }
    }
}

#ifdef JOIN_HAS_NUMA
// =========================================================================
//   CLASS     : Proactor
//   METHOD    : mbind
// =========================================================================
int Proactor::mbind (int numa) const noexcept
{
    return _commands.mbind (numa);
}
#endif

// =========================================================================
//   CLASS     : Proactor
//   METHOD    : mlock
// =========================================================================
int Proactor::mlock () const noexcept
{
    return _commands.mlock ();
}

// =========================================================================
//   CLASS     : Proactor
//   METHOD    : isProactorThread
// =========================================================================
bool Proactor::isProactorThread () const noexcept
{
    return _threadId.load (std::memory_order_acquire) == pthread_self ();
}

// =========================================================================
//   CLASS     : Proactor
//   METHOD    : getSqe
// =========================================================================
io_uring_sqe* Proactor::getSqe () noexcept
{
    io_uring_sqe* sqe = io_uring_get_sqe (&_ring);

    if (JOIN_UNLIKELY (sqe == nullptr))
    {
        io_uring_submit (&_ring);
        sqe = io_uring_get_sqe (&_ring);
    }

    return sqe;
}

// =========================================================================
//   CLASS     : Proactor
//   METHOD    : submitOperation
// =========================================================================
int Proactor::submitOperation (IoOperation* op) noexcept
{
    io_uring_sqe* sqe = getSqe ();

    if (JOIN_UNLIKELY (sqe == nullptr))
    {
        lastError = std::make_error_code (std::errc::resource_unavailable_try_again);
        return -1;
    }

    prepareSqe (*op, sqe);
    op->state = IoOperation::State::Submitted;

    return 0;
}

// =========================================================================
//   CLASS     : Proactor
//   METHOD    : cancelOperation
// =========================================================================
int Proactor::cancelOperation (IoOperation* op) noexcept
{
    if (op->state != IoOperation::State::Submitted)
    {
        return 0;
    }

    io_uring_sqe* sqe = getSqe ();

    if (JOIN_UNLIKELY (sqe == nullptr))
    {
        lastError = std::make_error_code (std::errc::resource_unavailable_try_again);
        return -1;
    }

    io_uring_prep_cancel (sqe, op, 0);
    io_uring_sqe_set_data (sqe, nullptr);
    op->state = IoOperation::State::Cancelling;

    return 0;
}

// =========================================================================
//   CLASS     : Proactor
//   METHOD    : rearmWakeup
// =========================================================================
void Proactor::rearmWakeup () noexcept
{
    io_uring_sqe* sqe = getSqe ();

    if (JOIN_UNLIKELY (sqe == nullptr))
    {
        return;
    }

    _wakeupOp.state = IoOperation::State::Submitted;
    prepareSqe (_wakeupOp, sqe);
}

// =========================================================================
//   CLASS     : Proactor
//   METHOD    : writeCommand
// =========================================================================
int Proactor::writeCommand (const Command& cmd) noexcept
{
    if (_commands.push (cmd) == -1)
    {
        return -1;
    }

    uint64_t value = 1;
    [[maybe_unused]] ssize_t bytes = ::write (_wakeup, &value, sizeof (uint64_t));

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
            break;
    }

    if (JOIN_UNLIKELY (cmd.done))
    {
        if (cmd.errc && (err != 0))
        {
            cmd.errc->store (lastError.value (), std::memory_order_release);
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
    Command cmd;

    while (_commands.tryPop (cmd) == 0)
    {
        processCommand (cmd);
    }

    if (_running.load (std::memory_order_acquire))
    {
        rearmWakeup ();
    }
}

// =========================================================================
//   CLASS     : Proactor
//   METHOD    : dispatchCqe
// =========================================================================
void Proactor::dispatchCqe (io_uring_cqe* cqe) noexcept
{
    auto* op = reinterpret_cast<IoOperation*> (io_uring_cqe_get_data (cqe));

    if (op == &_wakeupOp)
    {
        _wakeupOp.state = IoOperation::State::Idle;
        readCommands ();
        return;
    }

    if (op != nullptr)
    {
        dispatchOperation (op, cqe->res);
    }
}

// =========================================================================
//   CLASS     : Proactor
//   METHOD    : prepareSqe
// =========================================================================
void Proactor::prepareSqe (const IoOperation& op, io_uring_sqe* sqe) const noexcept
{
    assert (sqe != nullptr);

    switch (op.code)
    {
        case IORING_OP_TIMEOUT:
            {
                auto* ts = const_cast<__kernel_timespec*> (&op.data.timeout.ts);
                io_uring_prep_timeout (sqe, ts, 0, op.data.timeout.flags);
            }
            break;

        case IORING_OP_READ:
            io_uring_prep_read (sqe, op.data.rw.fd, op.data.rw.buf, op.data.rw.len, 0);
            break;

        case IORING_OP_READ_FIXED:
            io_uring_prep_read_fixed (sqe, op.data.rw.fd, op.data.rw.buf, op.data.rw.len, 0, op.data.rw.index);
            break;

        case IORING_OP_WRITE:
            io_uring_prep_write (sqe, op.data.rw.fd, op.data.rw.buf, op.data.rw.len, 0);
            break;

        case IORING_OP_WRITE_FIXED:
            io_uring_prep_write_fixed (sqe, op.data.rw.fd, op.data.rw.buf, op.data.rw.len, 0, op.data.rw.index);
            break;

        case IORING_OP_SENDMSG:
            io_uring_prep_sendmsg (sqe, op.data.msg.fd, op.data.msg.msg, op.data.msg.flags);
            break;

        case IORING_OP_RECVMSG:
            io_uring_prep_recvmsg (sqe, op.data.msg.fd, op.data.msg.msg, op.data.msg.flags);
            break;

        case IORING_OP_ACCEPT:
            io_uring_prep_accept (sqe, op.data.accept.fd, op.data.accept.addr, op.data.accept.addrlen,
                                  static_cast<int> (op.data.accept.flags));
            break;

            // case IORING_OP_ASYNC_CANCEL:
            //     io_uring_prep_cancel (sqe, reinterpret_cast<void*> (op.data.cancel.target), 0);
            //     break;

        default:
            break;
    }

    io_uring_sqe_set_data (sqe, static_cast<void*> (const_cast<join::IoOperation*> (&op)));
}

// =========================================================================
//   CLASS     : Proactor
//   METHOD    : dispatchOperation
// =========================================================================
void Proactor::dispatchOperation (IoOperation* op, int result) noexcept
{
    const IoOperation::State prev = op->state;
    op->state = IoOperation::State::Idle;

    if (op->handler == nullptr)
    {
        return;
    }

    if (result == -ECANCELED || prev == IoOperation::State::Cancelling)
    {
        op->handler->onCancel (op, result);
    }
    else
    {
        op->handler->onComplete (op, result);
    }
}

// =========================================================================
//   CLASS     : Proactor
//   METHOD    : eventLoop
// =========================================================================
void Proactor::eventLoop ()
{
    while (_running.load (std::memory_order_acquire))
    {
        io_uring_submit (&_ring);

        io_uring_cqe* cqe = nullptr;
        int ret = io_uring_wait_cqe (&_ring, &cqe);
        if (JOIN_UNLIKELY (ret < 0))
        {
            continue;
        }

        do
        {
            dispatchCqe (cqe);
            io_uring_cqe_seen (&_ring, cqe);
        }
        while (io_uring_peek_cqe (&_ring, &cqe) == 0);
    }
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
