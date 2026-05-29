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

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : BasicProactor
// =========================================================================
inline join::BasicProactor::BasicProactor ()
: _commands (_queueSize)
, _wakeup (eventfd (0, EFD_NONBLOCK | EFD_CLOEXEC))
, _readOps (256, nullptr)
, _writeOps (256, nullptr)
{
    if (_wakeup == -1)
    {
        throw std::system_error (errno, std::system_category (), "eventfd failed");  // LCOV_EXCL_LINE
    }

    _reactor.addHandler (_wakeup, this, true, false, false);
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : ~BasicProactor
// =========================================================================
inline join::BasicProactor::~BasicProactor () noexcept
{
    stop (true);

    ::close (_wakeup);
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : run
// =========================================================================
inline void join::BasicProactor::run ()
{
    _reactor.run ();
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : stop
// =========================================================================
inline void join::BasicProactor::stop (bool sync) noexcept
{
    if (isProactorThread ())
    {
        cancelAllOperations ();
        _reactor.stop (false);
        return;
    }

    if (!_reactor.isRunning ())
    {
        return;
    }

    std::atomic<bool> done{false};

    if (JOIN_LIKELY (sync))
    {
        bool expected = false;
        if (!_stopping.compare_exchange_strong (expected, true, std::memory_order_acq_rel))
        {
            Backoff backoff;
            while (isRunning ())
            {
                backoff ();
            }
            return;
        }
    }

    writeCommand ({CommandType::Stop, nullptr, sync, sync ? &done : nullptr, nullptr});

    if (JOIN_LIKELY (sync))
    {
        Backoff backoff;
        while (!done.load (std::memory_order_acquire))
        {
            backoff ();
        }
        _stopping.store (false, std::memory_order_release);
    }

    _reactor.stop (sync);
}

#ifdef JOIN_HAS_NUMA
// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : mbind
// =========================================================================
inline int join::BasicProactor::mbind (int numa) const noexcept
{
    if (_commands.mbind (numa) == -1)
    {
        return -1;
    }

    return _reactor.mbind (numa);
}
#endif

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : mlock
// =========================================================================
inline int join::BasicProactor::mlock () const noexcept
{
    if (_commands.mlock () == -1)
    {
        return -1;
    }

    return _reactor.mlock ();
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : isRunning
// =========================================================================
inline bool join::BasicProactor::isRunning () const noexcept
{
    return _reactor.isRunning ();
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : isProactorThread
// =========================================================================
inline bool join::BasicProactor::isProactorThread () const noexcept
{
    return _reactor.isReactorThread ();
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : writeCommand
// =========================================================================
inline int join::BasicProactor::writeCommand (const Command& cmd) noexcept
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
//   CLASS     : BasicProactor
//   METHOD    : readCommands
// =========================================================================
inline void join::BasicProactor::readCommands () noexcept
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
//   CLASS     : BasicProactor
//   METHOD    : processCommand
// =========================================================================
inline void join::BasicProactor::processCommand (const Command& cmd) noexcept
{
    int err = 0;

    switch (cmd.type)
    {
        case CommandType::Submit:
            err = submitOperation (cmd.op, cmd.flush);
            break;

        case CommandType::Cancel:
            err = cancelOperation (cmd.op, cmd.flush);
            break;

        case CommandType::Stop:
            cancelAllOperations ();
            break;

        default:
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
//   CLASS     : BasicProactor
//   METHOD    : submitOperation
// =========================================================================
inline int join::BasicProactor::submitOperation (IoOperation* op, [[maybe_unused]] bool flush) noexcept
{
    if (JOIN_UNLIKELY (op == nullptr))
    {
        lastError = make_error_code (Errc::InvalidParam);
        return -1;
    }

    if (JOIN_UNLIKELY (op->fd () < 0))
    {
        lastError = std::make_error_code (std::errc::bad_file_descriptor);
        return -1;
    }

    if (JOIN_UNLIKELY (op->state != IoOperation::State::Idle))
    {
        lastError = make_error_code (Errc::OperationFailed);
        return -1;
    }

    if (JOIN_UNLIKELY (static_cast<IoOperation::Opcode> (op->code) == IoOperation::Opcode::Connect))
    {
        if (JOIN_UNLIKELY (::connect (op->data.connect.fd, op->data.connect.addr, op->data.connect.addrlen) == -1 &&
                           errno != EINPROGRESS))
        {
            lastError = std::error_code (errno, std::system_category ());
            return -1;
        }
    }

    if (JOIN_UNLIKELY (static_cast<size_t> (op->fd ()) >= _readOps.size ()))
    {
        size_t newSize = static_cast<size_t> (op->fd ()) + 1;
        _readOps.resize (newSize, nullptr);
        _writeOps.resize (newSize, nullptr);
    }

    bool isWrite = isWriteOp (op->code);

    if (JOIN_UNLIKELY ((isWrite && (_writeOps[op->fd ()] != nullptr)) ||
                       (!isWrite && (_readOps[op->fd ()] != nullptr))))
    {
        lastError = make_error_code (Errc::InvalidParam);
        return -1;
    }

    if (isWrite)
    {
        _writeOps[op->fd ()] = op;
    }
    else
    {
        _readOps[op->fd ()] = op;
    }

    op->state = IoOperation::State::Submitted;

    return _reactor.addHandler (op->fd (), this, _readOps[op->fd ()] != nullptr, _writeOps[op->fd ()] != nullptr);
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : cancelOperation
// =========================================================================
inline int join::BasicProactor::cancelOperation (IoOperation* op, [[maybe_unused]] bool flush) noexcept
{
    if (JOIN_UNLIKELY (op == nullptr))
    {
        lastError = make_error_code (Errc::InvalidParam);
        return -1;
    }

    if (JOIN_UNLIKELY (op->fd () < 0))
    {
        lastError = std::make_error_code (std::errc::bad_file_descriptor);
        return -1;
    }

    if (JOIN_UNLIKELY (op->state != IoOperation::State::Submitted))
    {
        lastError = make_error_code (Errc::OperationFailed);
        return -1;
    }

    if (JOIN_UNLIKELY (static_cast<size_t> (op->fd ()) >= _readOps.size ()))
    {
        lastError = std::make_error_code (std::errc::bad_file_descriptor);
        return -1;
    }

    op->state = IoOperation::State::Cancelling;
    bool isWrite = isWriteOp (op->code);

    if (JOIN_UNLIKELY ((isWrite && (_writeOps[op->fd ()] != op)) || (!isWrite && (_readOps[op->fd ()] != op))))
    {
        op->state = IoOperation::State::Submitted;
        lastError = make_error_code (Errc::InvalidParam);
        return -1;
    }

    if (isWrite)
    {
        _writeOps[op->fd ()] = nullptr;
    }
    else
    {
        _readOps[op->fd ()] = nullptr;
    }

    int ret = 0;

    if (_readOps[op->fd ()] == nullptr && _writeOps[op->fd ()] == nullptr)
    {
        ret = _reactor.delHandler (op->fd ());
    }
    else
    {
        ret = _reactor.addHandler (op->fd (), this, _readOps[op->fd ()] != nullptr, _writeOps[op->fd ()] != nullptr);
    }

    dispatchOperation (op, -ECANCELED, true);

    return ret;
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : cancelAllOperations
// =========================================================================
inline void join::BasicProactor::cancelAllOperations () noexcept
{
    for (size_t fd = 0; fd < _readOps.size (); ++fd)
    {
        IoOperation* rOp = std::exchange (_readOps[fd], nullptr);
        IoOperation* wOp = std::exchange (_writeOps[fd], nullptr);
        if (rOp || wOp)
        {
            _reactor.delHandler (fd);
        }
        dispatchOperation (rOp, -ECANCELED, true);
        dispatchOperation (wOp, -ECANCELED, true);
    }
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : endOperation
// =========================================================================
inline void join::BasicProactor::endOperation (IoOperation* op, int result, bool cancelled) noexcept
{
    if (JOIN_UNLIKELY (op == nullptr))
    {
        return;  // LCOV_EXCL_LINE
    }

    int fd = op->fd ();

    if (JOIN_UNLIKELY (fd < 0 || static_cast<size_t> (fd) >= _readOps.size ()))
    {
        return;  // LCOV_EXCL_LINE
    }

    if (isWriteOp (op->code))
    {
        _writeOps[fd] = nullptr;
    }
    else
    {
        _readOps[fd] = nullptr;
    }

    if (_readOps[fd] == nullptr && _writeOps[fd] == nullptr)
    {
        _reactor.delHandler (fd);
    }
    else
    {
        _reactor.addHandler (fd, this, _readOps[fd] != nullptr, _writeOps[fd] != nullptr);
    }

    dispatchOperation (op, result, cancelled);
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : isWriteOp
// =========================================================================
inline bool join::BasicProactor::isWriteOp (uint8_t code) noexcept
{
    return code == static_cast<uint8_t> (IoOperation::Opcode::Connect) ||
           code == static_cast<uint8_t> (IoOperation::Opcode::Write) ||
           code == static_cast<uint8_t> (IoOperation::Opcode::WriteFixed) ||
           code == static_cast<uint8_t> (IoOperation::Opcode::SendMsg) ||
           code == static_cast<uint8_t> (IoOperation::Opcode::Send);
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : executeOp
// =========================================================================
inline int join::BasicProactor::executeOp (IoOperation* op) noexcept
{
    for (;;)
    {
        switch (static_cast<IoOperation::Opcode> (op->code))
        {
            case IoOperation::Opcode::Accept:
                {
                    int fd = ::accept4 (op->data.accept.fd, op->data.accept.addr, op->data.accept.addrlen,
                                        op->data.accept.flags);
                    if (JOIN_UNLIKELY ((fd == -1) && (errno == EINTR)))
                    {
                        continue;  // LCOV_EXCL_LINE
                    }
                    return (fd == -1) ? -errno : fd;
                }

            case IoOperation::Opcode::Connect:
                {
                    int err = 0;
                    socklen_t len = sizeof (err);
                    if (JOIN_UNLIKELY (::getsockopt (op->data.connect.fd, SOL_SOCKET, SO_ERROR, &err, &len) == -1))
                    {
                        return -errno;
                    }
                    return JOIN_UNLIKELY (err) ? -err : 0;
                }

            case IoOperation::Opcode::Read:
            case IoOperation::Opcode::ReadFixed:
                {
                    ssize_t n = ::read (op->data.rw.fd, op->data.rw.buf, op->data.rw.len);
                    if (JOIN_UNLIKELY ((n == -1) && (errno == EINTR)))
                    {
                        continue;  // LCOV_EXCL_LINE
                    }
                    return (n == -1) ? -errno : static_cast<int> (n);
                }

            case IoOperation::Opcode::Write:
            case IoOperation::Opcode::WriteFixed:
                {
                    ssize_t n = ::write (op->data.rw.fd, op->data.rw.buf, op->data.rw.len);
                    if (JOIN_UNLIKELY ((n == -1) && (errno == EINTR)))
                    {
                        continue;  // LCOV_EXCL_LINE
                    }
                    return (n == -1) ? -errno : static_cast<int> (n);
                }

            case IoOperation::Opcode::RecvMsg:
                {
                    ssize_t n = ::recvmsg (op->data.msg.fd, op->data.msg.msg, op->data.msg.flags);
                    if (JOIN_UNLIKELY ((n == -1) && (errno == EINTR)))
                    {
                        continue;  // LCOV_EXCL_LINE
                    }
                    return (n == -1) ? -errno : static_cast<int> (n);
                }

            case IoOperation::Opcode::SendMsg:
                {
                    ssize_t n = ::sendmsg (op->data.msg.fd, op->data.msg.msg, op->data.msg.flags);
                    if (JOIN_UNLIKELY ((n == -1) && (errno == EINTR)))
                    {
                        continue;  // LCOV_EXCL_LINE
                    }
                    return (n == -1) ? -errno : static_cast<int> (n);
                }

            case IoOperation::Opcode::Recv:
                {
                    ssize_t n =
                        ::recv (op->data.stream.fd, op->data.stream.buf, op->data.stream.len, op->data.stream.flags);
                    if (JOIN_UNLIKELY ((n == -1) && (errno == EINTR)))
                    {
                        continue;  // LCOV_EXCL_LINE
                    }
                    return (n == -1) ? -errno : static_cast<int> (n);
                }

            case IoOperation::Opcode::Send:
                {
                    ssize_t n =
                        ::send (op->data.stream.fd, op->data.stream.buf, op->data.stream.len, op->data.stream.flags);
                    if (JOIN_UNLIKELY ((n == -1) && (errno == EINTR)))
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
//   CLASS     : BasicProactor
//   METHOD    : onReadable
// =========================================================================
inline void join::BasicProactor::onReadable (int fd) noexcept
{
    if (JOIN_UNLIKELY (fd == _wakeup))
    {
        readCommands ();
        return;
    }

    endOperation (_readOps[fd], executeOp (_readOps[fd]), false);
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : onWriteable
// =========================================================================
inline void join::BasicProactor::onWriteable (int fd) noexcept
{
    endOperation (_writeOps[fd], executeOp (_writeOps[fd]), false);
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : onClose
// =========================================================================
inline void join::BasicProactor::onClose (int fd) noexcept
{
    IoOperation* rOp = std::exchange (_readOps[fd], nullptr);
    IoOperation* wOp = std::exchange (_writeOps[fd], nullptr);
    if (JOIN_LIKELY (rOp || wOp))
    {
        _reactor.delHandler (fd);
    }
    dispatchOperation (rOp, 0, false);
    dispatchOperation (wOp, 0, false);
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : onError
// =========================================================================
inline void join::BasicProactor::onError (int fd) noexcept
{
    IoOperation* rOp = std::exchange (_readOps[fd], nullptr);
    IoOperation* wOp = std::exchange (_writeOps[fd], nullptr);
    if (JOIN_LIKELY (rOp || wOp))
    {
        _reactor.delHandler (fd);
    }
    dispatchOperation (rOp, -ECONNRESET, false);
    dispatchOperation (wOp, -ECONNRESET, false);
}
