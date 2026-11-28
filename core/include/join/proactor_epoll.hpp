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

// C.
#include <poll.h>

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : BasicProactor
// =========================================================================
inline join::BasicProactor::BasicProactor ()
: _commands (_queueSize)
, _wakeup (eventfd (0, EFD_NONBLOCK | EFD_CLOEXEC))
{
    if (_wakeup.handle () == -1)
    {
        throw std::system_error (errno, std::system_category (), "eventfd failed");  // LCOV_EXCL_LINE
    }

    _reactor.addHandler (_wakeup.handle (), this, true, false, false);
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : ~BasicProactor
// =========================================================================
inline join::BasicProactor::~BasicProactor () noexcept
{
    stop (true);

    ::close (_wakeup.handle ());
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : run
// =========================================================================
inline void join::BasicProactor::run ()
{
    _stopping = false;
    _wakeupState.store (WakeupState::Sleeping, std::memory_order_seq_cst);
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
        _stopping = true;
        cancelAllOperations (true);
        _reactor.stop (false);
        return;
    }

    if (JOIN_UNLIKELY (!isRunning ()))
    {
        return;
    }

    writeCommand ({CommandType::Stop, nullptr, true, nullptr, nullptr, nullptr});

    if (JOIN_LIKELY (sync))
    {
        waitStopped ();
    }
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : waitStopped
// =========================================================================
inline void join::BasicProactor::waitStopped () const noexcept
{
    _reactor.waitStopped ();
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : registerBufferRing
// =========================================================================
template <size_t Count, size_t Size>
int join::BasicProactor::registerBufferRing (uint16_t group, LocalMem::SlabAllocator<Count, Size>& arena)
{
    int result = 0;
    std::error_code errc;

    InvokeHandler fn = [this, group, &arena, &result, &errc] () {
        if (JOIN_UNLIKELY (_bufferRings.count (group)))
        {
            lastError = make_error_code (Errc::InUse);
            errc = lastError;
            result = -1;
            return;
        }

        if (JOIN_UNLIKELY (_bufferRings[group].registerBuffer (group, arena) == -1))
        {
            errc = lastError;
            _bufferRings.erase (group);
            result = -1;
            return;
        }
    };

    if (JOIN_UNLIKELY (!isRunning ()))
    {
        lastError = make_error_code (Errc::OperationFailed);
        return -1;
    }

    if (JOIN_UNLIKELY (invoke (&fn) == -1))
    {
        return -1;  // LCOV_EXCL_LINE
    }

    if (JOIN_UNLIKELY (result == -1))
    {
        lastError = errc;
        return -1;
    }

    return 0;
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : registerBufferRing
// =========================================================================
template <size_t Count, size_t Size>
int join::BasicProactor::registerBufferRing (LocalMem::SlabAllocator<Count, Size>& arena)
{
    int result = 0;
    std::error_code errc;

    InvokeHandler fn = [this, &arena, &result, &errc] () {
        uint32_t group = 0;

        while ((group <= UINT16_MAX) && _bufferRings.count (static_cast<uint16_t> (group)))
        {
            ++group;
        }

        if (JOIN_UNLIKELY (group > UINT16_MAX))
        {
            // LCOV_EXCL_START
            lastError = make_error_code (Errc::OutOfMemory);
            errc = lastError;
            result = -1;
            return;
            // LCOV_EXCL_STOP
        }

        if (JOIN_UNLIKELY (_bufferRings[static_cast<uint16_t> (group)].registerBuffer (static_cast<uint16_t> (group),
                                                                                       arena) == -1))
        {
            errc = lastError;
            _bufferRings.erase (static_cast<uint16_t> (group));
            result = -1;
            return;
        }

        result = static_cast<int> (group);
    };

    if (JOIN_UNLIKELY (!isRunning ()))
    {
        lastError = make_error_code (Errc::OperationFailed);
        return -1;
    }

    if (JOIN_UNLIKELY (invoke (&fn) == -1))
    {
        return -1;  // LCOV_EXCL_LINE
    }

    if (JOIN_UNLIKELY (result == -1))
    {
        lastError = errc;
    }

    return result;
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : unregisterBufferRing
// =========================================================================
inline int join::BasicProactor::unregisterBufferRing (uint16_t group)
{
    int result = 0;
    std::error_code errc;

    InvokeHandler fn = [this, group, &result, &errc] () {
        auto it = _bufferRings.find (group);
        if (JOIN_UNLIKELY (it == _bufferRings.end ()))
        {
            lastError = make_error_code (Errc::NotFound);
            errc = lastError;
            result = -1;
            return;
        }

        if (JOIN_UNLIKELY (it->second.armed ()))
        {
            lastError = make_error_code (Errc::InUse);
            errc = lastError;
            result = -1;
            return;
        }

        if (JOIN_UNLIKELY (it->second.unregisterBuffer () == -1))
        {
            // LCOV_EXCL_START
            errc = lastError;
            result = -1;
            return;
            // LCOV_EXCL_STOP
        }

        _bufferRings.erase (it);
    };

    if (JOIN_UNLIKELY (!isRunning ()))
    {
        lastError = make_error_code (Errc::OperationFailed);
        return -1;
    }

    if (JOIN_UNLIKELY (invoke (&fn) == -1))
    {
        return -1;  // LCOV_EXCL_LINE
    }

    if (JOIN_UNLIKELY (result == -1))
    {
        lastError = errc;
        return -1;
    }

    return 0;
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
        return -1;  // LCOV_EXCL_LINE
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
//   METHOD    : wheel
// =========================================================================
inline join::BasicProactor::Wheel& join::BasicProactor::wheel () noexcept
{
    return _reactor.wheel ();
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

    // pairs with the fence in the event loop: an acquire load would let the push above be
    // reordered after it, the loop would sleep and the wakeup would be lost.
    std::atomic_thread_fence (std::memory_order_seq_cst);

    WakeupState state = _wakeupState.load (std::memory_order_relaxed);
    if (state != WakeupState::Sleeping)
    {
        return 0;
    }

    if (_wakeupState.compare_exchange_strong (state, WakeupState::Waking, std::memory_order_seq_cst))
    {
        uint64_t value = 1;
        if (JOIN_UNLIKELY (::write (_wakeup.handle (), &value, sizeof (uint64_t)) == -1))
        {
            _wakeupState.store (WakeupState::Sleeping, std::memory_order_seq_cst);  // LCOV_EXCL_LINE
        }
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
    [[maybe_unused]] ssize_t nread = ::read (_wakeup.handle (), &count, sizeof (count));
    _wakeupState.store (WakeupState::Sleeping, std::memory_order_relaxed);
    std::atomic_thread_fence (std::memory_order_seq_cst);

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
            err = submitOperation (*cmd.op, cmd.flush, cmd.deadline);
            if (JOIN_UNLIKELY ((err != 0) && (cmd.done == nullptr) && (cmd.op->state == IoOperation::State::Idle)))
            {
                dispatchOperation (*cmd.op, -lastError.default_error_condition ().value (), false);
            }
            break;

        case CommandType::Cancel:
            err = cancelOperation (*cmd.op, cmd.flush);
            break;

        case CommandType::Invoke:
            err = invokeFunction (cmd.fn);
            break;

        case CommandType::Stop:
            _stopping = true;
            cancelAllOperations (cmd.flush);
            _reactor.stop (false);
            break;

        default:    // LCOV_EXCL_LINE
            break;  // LCOV_EXCL_LINE
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
inline int join::BasicProactor::submitOperation (IoOperation& op, [[maybe_unused]] bool flush,
                                                 TimePoint deadline) noexcept
{
    if (JOIN_UNLIKELY (_stopping))
    {
        lastError = std::make_error_code (std::errc::operation_canceled);
        return -1;
    }

    Backoff backoff;
    while (JOIN_UNLIKELY (op.state.load (std::memory_order_acquire) == IoOperation::State::Suspended))
    {
        backoff ();  // LCOV_EXCL_LINE
    }

    IoOperation::State expected = IoOperation::State::Idle;
    if (!op.state.compare_exchange_strong (expected, IoOperation::State::Submitted, std::memory_order_acquire,
                                           std::memory_order_relaxed) &&
        (expected == IoOperation::State::Busy))
    {
        op.state.store (IoOperation::State::Submitted, std::memory_order_release);
    }

    if (JOIN_UNLIKELY (op.descriptor == nullptr))
    {
        resetOperation (op);
        lastError = std::make_error_code (std::errc::bad_file_descriptor);
        return -1;
    }

    const int fd = op.descriptor->handle ();

    if (JOIN_UNLIKELY (fd < 0))
    {
        resetOperation (op);
        lastError = std::make_error_code (std::errc::bad_file_descriptor);
        return -1;
    }

    IoRingBuffer* ring = nullptr;

    if (JOIN_UNLIKELY (op.multishot && ((op.code == static_cast<uint8_t> (IoOperation::Opcode::RecvMsg)) ||
                                        (op.code == static_cast<uint8_t> (IoOperation::Opcode::Recv)))))
    {
        const uint16_t group =
            (op.code == static_cast<uint8_t> (IoOperation::Opcode::RecvMsg)) ? op.data.msg.group : op.data.stream.group;
        auto it = _bufferRings.find (group);
        if (JOIN_UNLIKELY (it == _bufferRings.end ()))
        {
            resetOperation (op);
            lastError = make_error_code (Errc::NotFound);
            return -1;
        }

        ring = &it->second;
    }

    if (JOIN_UNLIKELY (op.next != nullptr))
    {
        lastError = make_error_code (std::errc::device_or_resource_busy);
        return -1;
    }

    if (JOIN_UNLIKELY ((deadline != TimePoint::max ()) && (armOperation (op, deadline) == -1)))
    {
        resetOperation (op);
        return -1;
    }

    if (JOIN_UNLIKELY (static_cast<IoOperation::Opcode> (op.code) == IoOperation::Opcode::Connect))
    {
        if (JOIN_UNLIKELY (::connect (fd, op.data.connect.addr, op.data.connect.addrlen) == -1 && errno != EINPROGRESS))
        {
            lastError = std::error_code (errno, std::system_category ());
            resetOperation (op);
            return -1;
        }
    }

    IoDescriptor& descriptor = *op.descriptor;

    if (JOIN_UNLIKELY (ring != nullptr))
    {
        op.ring = ring;
        ring->bind ();
    }

    const bool idle = descriptor.empty ();

    if (!descriptor.queue (op))
    {
        return 0;
    }

    if (idle)
    {
        descriptor._proactor = this;
    }

    int err = _reactor.addHandler (fd, &descriptor, descriptor._readOps != nullptr, descriptor._writeOps != nullptr);
    if (JOIN_UNLIKELY (err == -1))
    {
        descriptor.dequeue (op);

        if (JOIN_UNLIKELY (ring != nullptr))
        {
            op.ring->unbind ();
            op.ring = nullptr;
        }

        resetOperation (op);
        return err;
    }

    if (idle)
    {
        descriptor.link (_descriptors);
    }

    return err;
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : cancelOperation
// =========================================================================
inline int join::BasicProactor::cancelOperation (IoOperation& op, [[maybe_unused]] bool flush) noexcept
{
    if (JOIN_UNLIKELY (op.descriptor == nullptr))
    {
        lastError = std::make_error_code (std::errc::bad_file_descriptor);
        return -1;
    }

    const int fd = op.descriptor->handle ();

    if (JOIN_UNLIKELY (fd < 0))
    {
        lastError = std::make_error_code (std::errc::bad_file_descriptor);
        return -1;
    }

    if (JOIN_UNLIKELY (op.state != IoOperation::State::Submitted))
    {
        lastError = make_error_code (Errc::OperationFailed);
        return -1;
    }

    IoDescriptor& descriptor = *op.descriptor;

    if (JOIN_UNLIKELY (!descriptor.dequeue (op)))
    {
        lastError = make_error_code (Errc::InvalidParam);
        return -1;
    }

    int ret = 0;

    if (descriptor.empty ())
    {
        ret = _reactor.delHandler (fd);
        descriptor.unlink (_descriptors);

        if (_dispatching != &descriptor)
        {
            descriptor._proactor = nullptr;
        }
    }
    else if (descriptor.back (op) == nullptr)
    {
        ret = _reactor.addHandler (fd, &descriptor, descriptor._readOps != nullptr, descriptor._writeOps != nullptr);
    }

    dispatchOperation (op, -ECANCELED, true);

    return ret;
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : cancelAllOperations
// =========================================================================
inline void join::BasicProactor::cancelAllOperations ([[maybe_unused]] bool flush) noexcept
{
    while (_descriptors != nullptr)
    {
        IoDescriptor* descriptor = _descriptors;
        _dispatching = descriptor;

        for (IoOperation** ops : {&descriptor->_readOps, &descriptor->_writeOps})
        {
            while ((_dispatching == descriptor) && (*ops != nullptr))
            {
                endOperation (*(*ops)->next, -ECANCELED, true);
            }
        }

        if (_dispatching == descriptor)
        {
            _dispatching = nullptr;
            descriptor->_proactor = nullptr;
        }
    }
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : endOperation
// =========================================================================
inline void join::BasicProactor::endOperation (IoOperation& op, int result, bool cancelled) noexcept
{
    IoDescriptor& descriptor = *op.descriptor;

    if (descriptor.dequeue (op))
    {
        const int fd = descriptor.handle ();

        if (descriptor.empty ())
        {
            _reactor.delHandler (fd);
            descriptor.unlink (_descriptors);

            if (_dispatching != &descriptor)
            {
                descriptor._proactor = nullptr;
            }
        }
        else if (descriptor.back (op) == nullptr)
        {
            _reactor.addHandler (fd, &descriptor, descriptor._readOps != nullptr, descriptor._writeOps != nullptr);
        }
    }

    dispatchOperation (op, result, cancelled);
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : expireOperation
// =========================================================================
inline void join::BasicProactor::expireOperation (IoOperation& op) noexcept
{
    op.timer = -op.timer;

    endOperation (op, -ETIMEDOUT, true);
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : onEvent
// =========================================================================
inline void join::BasicProactor::onEvent ([[maybe_unused]] int fd, [[maybe_unused]] uint32_t revents) noexcept
{
    readCommands ();
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : processQueue
// =========================================================================
inline void join::BasicProactor::processQueue (IoDescriptor& descriptor, bool write, uint32_t revents) noexcept
{
    IoOperation* last = write ? descriptor._writeOps : descriptor._readOps;
    if (last == nullptr)
    {
        return;
    }

    size_t count = 1;
    for (IoOperation* it = last->next; it != last; it = it->next)
    {
        ++count;
    }

    IoOperation* op = last->next;
    bool consumed = false;

    for (size_t i = 0; (op != nullptr) && (i < count); ++i)
    {
        IoOperation* following = (op == last) ? nullptr : op->next;
        bool poll = (op->code == static_cast<uint8_t> (IoOperation::Opcode::Poll));

        if (poll || !consumed)
        {
            consumed = consumed || !poll;
            processOperation (*op, revents);
        }

        if (JOIN_UNLIKELY (_dispatching != &descriptor))
        {
            return;
        }

        last = write ? descriptor._writeOps : descriptor._readOps;
        op = nullptr;

        if ((following != nullptr) && (last != nullptr))
        {
            for (IoOperation* it = last->next;; it = it->next)
            {
                if (it == following)
                {
                    op = following;
                    break;
                }

                if (it == last)
                {
                    break;
                }
            }
        }
    }
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : processOperation
// =========================================================================
inline void join::BasicProactor::processOperation (IoOperation& op, uint32_t revents) noexcept
{
    IoOperation::State current = op.state.load (std::memory_order_acquire);
    Backoff backoff;

    while (JOIN_UNLIKELY (current == IoOperation::State::Suspended))
    {
        backoff ();
        current = op.state.load (std::memory_order_acquire);
    }

    if (JOIN_UNLIKELY (current == IoOperation::State::Idle))
    {
        return;  // LCOV_EXCL_LINE
    }

    IoRingBuffer* br = nullptr;
    uint16_t bid = 0;

    if (op.ring != nullptr)
    {
        br = op.ring;
        bid = static_cast<uint16_t> (br->select ());

        if (op.code == static_cast<uint8_t> (IoOperation::Opcode::RecvMsg))
        {
            uint32_t reserved = op.data.msg.namelen + op.data.msg.controllen;
            if (JOIN_UNLIKELY (reserved >= br->size ()))
            {
                endOperation (op, -EFAULT, false);
                br->recycle (bid);
                return;
            }

            char* base = static_cast<char*> (br->get (bid));

            op.data.msg.msg->msg_name = base;
            op.data.msg.msg->msg_namelen = op.data.msg.namelen;
            op.data.msg.msg->msg_control = base + op.data.msg.namelen;
            op.data.msg.msg->msg_controllen = op.data.msg.controllen;
            op.data.msg.msg->msg_iov->iov_base = base + reserved;
            op.data.msg.msg->msg_iov->iov_len = br->size () - reserved;
            op.data.msg.msg->msg_iovlen = 1;
            op.data.msg.msg->msg_flags = 0;
        }
        else
        {
            op.data.stream.buf = br->get (bid);
            op.data.stream.len = br->size ();
        }
    }

    int result = executeOperation (op, revents);

    if (JOIN_UNLIKELY ((result == -EAGAIN) && !(revents & EPOLLERR) &&
                       (op.code != static_cast<uint8_t> (IoOperation::Opcode::Connect))))
    {
        if (br != nullptr)
        {
            br->recycle (bid);
        }
        return;
    }

    if ((br != nullptr) && (result >= 0) && (op.code == static_cast<uint8_t> (IoOperation::Opcode::RecvMsg)))
    {
        op.data.msg.msg->msg_iov->iov_len = static_cast<size_t> (result);

        if (op.data.msg.msg->msg_controllen < sizeof (cmsghdr))
        {
            op.data.msg.msg->msg_control = nullptr;
        }
    }

    const bool failed = (op.code == static_cast<uint8_t> (IoOperation::Opcode::Poll)) &&
                        ((revents & (EPOLLERR | EPOLLRDHUP | EPOLLHUP)) != 0);

    if (op.multishot && !failed &&
        ((result > 0) || ((result == 0) && (op.code == static_cast<uint8_t> (IoOperation::Opcode::Accept)))))
    {
        op.more = true;
        notifyOperation (op, result, false);
    }
    else
    {
        endOperation (op, result, false);
    }

    if (br != nullptr)
    {
        br->recycle (bid);
    }
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : executeOperation
// =========================================================================
inline int join::BasicProactor::executeOperation (IoOperation& op, uint32_t revents) noexcept
{
    for (;;)
    {
        switch (static_cast<IoOperation::Opcode> (op.code))
        {
            case IoOperation::Opcode::Poll:
                return static_cast<int> (revents & (op.data.poll.events | POLLERR | POLLHUP | POLLRDHUP));

            case IoOperation::Opcode::Accept:
                {
                    int fd =
                        ::accept4 (op.descriptor->handle (), op.data.accept.addr, op.data.accept.addrlen, op.flags);
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
                    if (JOIN_UNLIKELY (::getsockopt (op.descriptor->handle (), SOL_SOCKET, SO_ERROR, &err, &len) == -1))
                    {
                        return -errno;  // LCOV_EXCL_LINE
                    }
                    return JOIN_UNLIKELY (err) ? -err : 0;
                }

            case IoOperation::Opcode::Read:
                {
                    ssize_t n = ::read (op.descriptor->handle (), op.data.rw.buf, op.data.rw.len);
                    if (JOIN_UNLIKELY ((n == -1) && (errno == EINTR)))
                    {
                        continue;  // LCOV_EXCL_LINE
                    }
                    return (n == -1) ? -errno : static_cast<int> (n);
                }

            case IoOperation::Opcode::Write:
                {
                    ssize_t n = ::write (op.descriptor->handle (), op.data.rw.buf, op.data.rw.len);
                    if (JOIN_UNLIKELY ((n == -1) && (errno == EINTR)))
                    {
                        continue;  // LCOV_EXCL_LINE
                    }
                    return (n == -1) ? -errno : static_cast<int> (n);
                }

            case IoOperation::Opcode::RecvMsg:
                {
                    ssize_t n = ::recvmsg (op.descriptor->handle (), op.data.msg.msg, op.flags);
                    if (JOIN_UNLIKELY ((n == -1) && (errno == EINTR)))
                    {
                        continue;  // LCOV_EXCL_LINE
                    }
                    return (n == -1) ? -errno : static_cast<int> (n);
                }

            case IoOperation::Opcode::SendMsg:
            case IoOperation::Opcode::SendMsgZc:
                {
                    ssize_t n = ::sendmsg (op.descriptor->handle (), op.data.msg.msg, op.flags);
                    if (JOIN_UNLIKELY ((n == -1) && (errno == EINTR)))
                    {
                        continue;  // LCOV_EXCL_LINE
                    }
                    return (n == -1) ? -errno : static_cast<int> (n);
                }

            case IoOperation::Opcode::Recv:
                {
                    ssize_t n = ::recv (op.descriptor->handle (), op.data.stream.buf, op.data.stream.len, op.flags);
                    if (JOIN_UNLIKELY ((n == -1) && (errno == EINTR)))
                    {
                        continue;  // LCOV_EXCL_LINE
                    }
                    return (n == -1) ? -errno : static_cast<int> (n);
                }

            case IoOperation::Opcode::Send:
            case IoOperation::Opcode::SendZc:
                {
                    ssize_t n = ::send (op.descriptor->handle (), op.data.stream.buf, op.data.stream.len, op.flags);
                    if (JOIN_UNLIKELY ((n == -1) && (errno == EINTR)))
                    {
                        continue;  // LCOV_EXCL_LINE
                    }
                    return (n == -1) ? -errno : static_cast<int> (n);
                }

            default:             // LCOV_EXCL_LINE
                return -EINVAL;  // LCOV_EXCL_LINE
        }
    }
}
