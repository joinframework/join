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
template <typename Policy>
join::BasicProactor<Policy>::BasicProactor ()
: _commands (_queueSize)
, _wakeup (initWakeup (is_default<Policy>{}))
{
    static_assert (has_spin<Policy>::value || !has_sqpoll<Policy>::value, "spin required for sq poll policy");

    io_uring_params params{};
    params.flags = Policy::flags;
    initCqEntries (params, has_cq_entries<Policy>{});
    initSqThreadIdle (params, has_sq_thread_idle<Policy>{});
    initSqThreadCpu (params, has_sq_thread_cpu<Policy>{});

    if (io_uring_queue_init_params (Policy::sqEntries, &_ring, &params) < 0)
    {
        // LCOV_EXCL_START
        ::close (_wakeup);
        throw std::system_error (errno, std::system_category (), "io_uring_queue_init failed");
        // LCOV_EXCL_STOP
    }

    initWakeupOp (is_default<Policy>{});
    _pendingOps.reserve (Policy::sqEntries);
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : ~BasicProactor
// =========================================================================
template <typename Policy>
join::BasicProactor<Policy>::~BasicProactor () noexcept
{
    stop (true);

    io_uring_queue_exit (&_ring);

    if (_wakeup != -1)
    {
        ::close (_wakeup);
    }
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : flush
// =========================================================================
template <typename Policy>
int join::BasicProactor<Policy>::flush (bool sync) noexcept
{
    if (isProactorThread ())
    {
        if (JOIN_UNLIKELY (io_uring_submit (&_ring) < 0))
        {
            // LCOV_EXCL_START
            lastError = std::error_code (errno, std::system_category ());
            return -1;
            // LCOV_EXCL_STOP
        }
        return 0;
    }

    std::atomic<bool> done{false}, *pdone = nullptr;
    std::error_code errc, *perrc = nullptr;

    if (JOIN_UNLIKELY (sync))
    {
        pdone = &done;
        perrc = &errc;
    }

    if (JOIN_UNLIKELY (writeCommand ({CommandType::Flush, nullptr, false, pdone, perrc, nullptr}) == -1))
    {
        return -1;  // LCOV_EXCL_LINE
    }

    if (JOIN_UNLIKELY (sync))
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
//   CLASS     : BasicProactor
//   METHOD    : run
// =========================================================================
template <typename Policy>
void join::BasicProactor<Policy>::run ()
{
    _threadId.store (pthread_self (), std::memory_order_release);

    _running.store (true, std::memory_order_release);
    eventLoop ();

    _threadId.store (_invalidThreadId, std::memory_order_release);
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : stop
// =========================================================================
template <typename Policy>
void join::BasicProactor<Policy>::stop (bool sync) noexcept
{
    if (isProactorThread ())
    {
        _running.store (false, std::memory_order_release);
        cancelAllOperations ();
        eventLoop ();
        return;
    }

    if (JOIN_UNLIKELY (!isRunning ()))
    {
        return;
    }

    writeCommand ({CommandType::Stop, nullptr, sync, nullptr, nullptr, nullptr});

    if (JOIN_LIKELY (sync))
    {
        waitStopped ();
    }
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : waitStopped
// =========================================================================
template <typename Policy>
void join::BasicProactor<Policy>::waitStopped () const noexcept
{
    Backoff backoff;

    while (_threadId.load (std::memory_order_acquire) != _invalidThreadId)
    {
        backoff ();
    }
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : registerFixedBuffers
// =========================================================================
template <typename Policy>
template <size_t Count, size_t... Sizes>
int join::BasicProactor<Policy>::registerFixedBuffers (LocalMem::Allocator<Count, Sizes...>& arena) noexcept
{
    return registerFixedBuffers (arena, std::make_index_sequence<sizeof...(Sizes)>{});
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : unregisterFixedBuffers
// =========================================================================
template <typename Policy>
int join::BasicProactor<Policy>::unregisterFixedBuffers () noexcept
{
    int ret = io_uring_unregister_buffers (&_ring);
    if (JOIN_UNLIKELY (ret < 0))
    {
        lastError = std::error_code (-ret, std::system_category ());
        return -1;
    }

    return 0;
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : registerBufferRing
// =========================================================================
template <typename Policy>
template <size_t Count, size_t Size>
int join::BasicProactor<Policy>::registerBufferRing (uint16_t group, LocalMem::Allocator<Count, Size>& arena)
{
    if (JOIN_UNLIKELY (_bufferRings.count (group)))
    {
        lastError = make_error_code (Errc::InUse);
        return -1;
    }

    auto it = _bufferRings.emplace (group, &_ring).first;

    if (JOIN_UNLIKELY (it->second.registerBuffer (group, arena) == -1))
    {
        _bufferRings.erase (it);
        return -1;
    }

    return 0;
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : unregisterBufferRing
// =========================================================================
template <typename Policy>
int join::BasicProactor<Policy>::unregisterBufferRing (uint16_t group)
{
    auto it = _bufferRings.find (group);
    if (JOIN_UNLIKELY (it == _bufferRings.end ()))
    {
        lastError = make_error_code (Errc::NotFound);
        return -1;
    }

    if (JOIN_UNLIKELY (it->second.armed ()))
    {
        lastError = make_error_code (Errc::InUse);
        return -1;
    }

    if (JOIN_UNLIKELY (it->second.unregisterBuffer () == -1))
    {
        return -1;  // LCOV_EXCL_LINE
    }

    _bufferRings.erase (it);

    return 0;
}

#ifdef JOIN_HAS_NUMA
// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : mbind
// =========================================================================
template <typename Policy>
int join::BasicProactor<Policy>::mbind (int numa) const noexcept
{
    return _commands.mbind (numa);
}
#endif

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : mlock
// =========================================================================
template <typename Policy>
int join::BasicProactor<Policy>::mlock () const noexcept
{
    return _commands.mlock ();
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : isRunning
// =========================================================================
template <typename Policy>
bool join::BasicProactor<Policy>::isRunning () const noexcept
{
    return _threadId.load (std::memory_order_acquire) != _invalidThreadId;
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : isProactorThread
// =========================================================================
template <typename Policy>
bool join::BasicProactor<Policy>::isProactorThread () const noexcept
{
    return _threadId.load (std::memory_order_acquire) == pthread_self ();
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : initWakeup
// =========================================================================
template <typename Policy>
int join::BasicProactor<Policy>::initWakeup (std::true_type) noexcept
{
    return eventfd (0, EFD_NONBLOCK | EFD_CLOEXEC);
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : initWakeup
// =========================================================================
template <typename Policy>
int join::BasicProactor<Policy>::initWakeup (std::false_type) noexcept
{
    return -1;
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : initWakeupOp
// =========================================================================
template <typename Policy>
void join::BasicProactor<Policy>::initWakeupOp (std::true_type) noexcept
{
    _wakeupOp = IoOperation::makeRead (_wakeup, &_wakeupBuf, sizeof (_wakeupBuf), nullptr);
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : initWakeupOp
// =========================================================================
template <typename Policy>
void join::BasicProactor<Policy>::initWakeupOp (std::false_type) noexcept
{
    // no-op.
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : initCqEntries
// =========================================================================
template <typename Policy>
void join::BasicProactor<Policy>::initCqEntries (io_uring_params&, std::false_type) noexcept
{
    // no-op.
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : initCqEntries
// =========================================================================
template <typename Policy>
void join::BasicProactor<Policy>::initCqEntries (io_uring_params& params, std::true_type) noexcept
{
    params.flags |= IORING_SETUP_CQSIZE;
    params.cq_entries = Policy::cqEntries;
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : initSqThreadIdle
// =========================================================================
template <typename Policy>
void join::BasicProactor<Policy>::initSqThreadIdle (io_uring_params&, std::false_type) noexcept
{
    // no-op.
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : initSqThreadIdle
// =========================================================================
template <typename Policy>
void join::BasicProactor<Policy>::initSqThreadIdle (io_uring_params& params, std::true_type) noexcept
{
    params.sq_thread_idle = Policy::sqThreadIdle;
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : initSqThreadCpu
// =========================================================================
template <typename Policy>
void join::BasicProactor<Policy>::initSqThreadCpu (io_uring_params&, std::false_type) noexcept
{
    // no-op.
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : initSqThreadCpu
// =========================================================================
template <typename Policy>
void join::BasicProactor<Policy>::initSqThreadCpu (io_uring_params& params, std::true_type) noexcept
{
    params.flags |= IORING_SETUP_SQ_AFF;
    params.sq_thread_cpu = Policy::sqThreadCpu;
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : writeCommand
// =========================================================================
template <typename Policy>
int join::BasicProactor<Policy>::writeCommand (const Command& cmd) noexcept
{
    return writeCommand (cmd, is_default<Policy>{});
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : writeCommand
// =========================================================================
template <typename Policy>
int join::BasicProactor<Policy>::writeCommand (const Command& cmd, std::true_type) noexcept
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
        if (JOIN_UNLIKELY (::write (_wakeup, &value, sizeof (uint64_t)) == -1))
        {
            _wakeupState.store (WakeupState::Sleeping, std::memory_order_seq_cst);  // LCOV_EXCL_LINE
        }
    }

    return 0;
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : writeCommand
// =========================================================================
template <typename Policy>
int join::BasicProactor<Policy>::writeCommand (const Command& cmd, std::false_type) noexcept
{
    return _commands.push (cmd);
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : readCommands
// =========================================================================
template <typename Policy>
void join::BasicProactor<Policy>::readCommands () noexcept
{
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
template <typename Policy>
void join::BasicProactor<Policy>::processCommand (const Command& cmd) noexcept
{
    int err = 0;

    switch (cmd.type)
    {
        case CommandType::Submit:
            err = submitOperation (cmd.op, cmd.flush);
            if (JOIN_UNLIKELY ((err == -1) && (cmd.done == nullptr) && (cmd.op != nullptr) &&
                               (cmd.op->state == IoOperation::State::Idle)))
            {
                dispatchOperation (cmd.op, -lastError.value (), false);
            }
            break;

        case CommandType::Cancel:
            err = cancelOperation (cmd.op, cmd.flush);
            break;

        case CommandType::Invoke:
            err = invokeFunction (cmd.fn);
            break;

        case CommandType::Stop:
            _running.store (false, std::memory_order_release);
            cancelAllOperations ();
            if (JOIN_UNLIKELY (cmd.flush))
            {
                io_uring_submit (&_ring);
            }
            break;

        case CommandType::Flush:
            if (JOIN_UNLIKELY (io_uring_submit (&_ring) < 0))
            {
                // LCOV_EXCL_START
                lastError = std::error_code (errno, std::system_category ());
                err = -1;
                // LCOV_EXCL_STOP
            }
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
template <typename Policy>
int join::BasicProactor<Policy>::submitOperation (IoOperation* op, bool flush) noexcept
{
    if (JOIN_UNLIKELY (op == nullptr))
    {
        lastError = make_error_code (std::errc::invalid_argument);
        return -1;
    }

    if (JOIN_UNLIKELY (op->fd () < 0))
    {
        lastError = std::make_error_code (std::errc::bad_file_descriptor);
        return -1;
    }

    if (JOIN_UNLIKELY (op->state != IoOperation::State::Idle))
    {
        lastError = make_error_code (std::errc::device_or_resource_busy);
        return -1;
    }

    IoRingBuffer* ring = nullptr;

    if (JOIN_UNLIKELY (op->multishot && (op->code != static_cast<uint8_t> (IoOperation::Opcode::Accept))))
    {
        auto it = _bufferRings.find (op->group);
        if (JOIN_UNLIKELY (it == _bufferRings.end ()))
        {
            lastError = make_error_code (Errc::NotFound);
            return -1;
        }

        ring = &it->second;
    }

    io_uring_sqe* sqe = getSqe ();
    if (JOIN_UNLIKELY (sqe == nullptr))
    {
        // LCOV_EXCL_START
        lastError = make_error_code (std::errc::no_buffer_space);
        return -1;
        // LCOV_EXCL_STOP
    }

    prepareSqe (sqe, op);
    op->state = IoOperation::State::Submitted;
    op->index = static_cast<uint32_t> (_pendingOps.size ());
    _pendingOps.push_back (op);

    if (JOIN_UNLIKELY (ring != nullptr))
    {
        op->ring = ring;
        ring->bind ();
    }

    if (JOIN_UNLIKELY (flush))
    {
        io_uring_submit (&_ring);
    }

    return 0;
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : cancelOperation
// =========================================================================
template <typename Policy>
int join::BasicProactor<Policy>::cancelOperation (IoOperation* op, bool flush) noexcept
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

    if (JOIN_UNLIKELY (op->index >= _pendingOps.size () || _pendingOps[op->index] != op))
    {
        lastError = make_error_code (Errc::InvalidParam);
        return -1;
    }

    io_uring_sqe* sqe = getSqe ();
    if (JOIN_UNLIKELY (sqe == nullptr))
    {
        // LCOV_EXCL_START
        lastError = make_error_code (Errc::OperationFailed);
        return -1;
        // LCOV_EXCL_STOP
    }

    op->state = IoOperation::State::Cancelling;
    io_uring_prep_cancel (sqe, op, 0);
    io_uring_sqe_set_data (sqe, nullptr);

    if (JOIN_UNLIKELY (flush))
    {
        io_uring_submit (&_ring);
    }

    return 0;
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : cancelAllOperations
// =========================================================================
template <typename Policy>
void join::BasicProactor<Policy>::cancelAllOperations () noexcept
{
    for (IoOperation* op : _pendingOps)
    {
        cancelOperation (op, false);
    }
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : endOperation
// =========================================================================
template <typename Policy>
void join::BasicProactor<Policy>::endOperation (IoOperation* op, int result, bool cancelled) noexcept
{
    if (JOIN_UNLIKELY (op == nullptr))
    {
        return;  // LCOV_EXCL_LINE
    }

    if (JOIN_LIKELY (op->index < _pendingOps.size () && _pendingOps[op->index] == op))
    {
        IoOperation* last = _pendingOps.back ();
        _pendingOps[op->index] = last;
        last->index = op->index;
        _pendingOps.pop_back ();
    }

    dispatchOperation (op, result, cancelled);
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : registerFixedBuffers
// =========================================================================
template <typename Policy>
template <size_t Count, size_t... Sizes, size_t... Is>
int join::BasicProactor<Policy>::registerFixedBuffers (LocalMem::Allocator<Count, Sizes...>& arena,
                                                       std::index_sequence<Is...>) noexcept
{
    iovec iovecs[] = {iovec{arena.template getPtr<Is> (0), Count * Sizes}...};

    int ret = io_uring_register_buffers (&_ring, iovecs, sizeof...(Sizes));
    if (JOIN_UNLIKELY (ret < 0))
    {
        lastError = std::error_code (-ret, std::system_category ());
        return -1;
    }

    return 0;
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : getSqe
// =========================================================================
template <typename Policy>
io_uring_sqe* join::BasicProactor<Policy>::getSqe () noexcept
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
//   CLASS     : BasicProactor
//   METHOD    : prepareSqe
// =========================================================================
template <typename Policy>
void join::BasicProactor<Policy>::prepareSqe (io_uring_sqe* sqe, IoOperation* op) noexcept
{
    switch (static_cast<IoOperation::Opcode> (op->code))
    {
        case IoOperation::Opcode::Accept:
            if (op->multishot)
            {
                io_uring_prep_multishot_accept (sqe, op->data.accept.fd, op->data.accept.addr, op->data.accept.addrlen,
                                                op->data.accept.flags);
            }
            else
            {
                io_uring_prep_accept (sqe, op->data.accept.fd, op->data.accept.addr, op->data.accept.addrlen,
                                      op->data.accept.flags);
            }
            break;

        case IoOperation::Opcode::Connect:
            io_uring_prep_connect (sqe, op->data.connect.fd, op->data.connect.addr, op->data.connect.addrlen);
            break;

        case IoOperation::Opcode::Read:
            io_uring_prep_read (sqe, op->data.rw.fd, op->data.rw.buf, op->data.rw.len, 0);
            break;

        case IoOperation::Opcode::Write:
            io_uring_prep_write (sqe, op->data.rw.fd, op->data.rw.buf, op->data.rw.len, 0);
            break;

        case IoOperation::Opcode::ReadFixed:
            io_uring_prep_read_fixed (sqe, op->data.rw.fd, op->data.rw.buf, op->data.rw.len, 0, op->data.rw.index);
            break;

        case IoOperation::Opcode::WriteFixed:
            io_uring_prep_write_fixed (sqe, op->data.rw.fd, op->data.rw.buf, op->data.rw.len, 0, op->data.rw.index);
            break;

        case IoOperation::Opcode::RecvMsg:
            if (op->multishot)
            {
                op->data.msg.msg->msg_namelen = op->data.msg.namelen;
                op->data.msg.msg->msg_controllen = op->data.msg.controllen;
                op->data.msg.msg->msg_iovlen = 0;
                io_uring_prep_recvmsg_multishot (sqe, op->data.msg.fd, op->data.msg.msg, op->data.msg.flags);
                sqe->flags |= IOSQE_BUFFER_SELECT;
                sqe->buf_group = op->group;
            }
            else
            {
                io_uring_prep_recvmsg (sqe, op->data.msg.fd, op->data.msg.msg, op->data.msg.flags);
            }
            break;

        case IoOperation::Opcode::SendMsg:
            io_uring_prep_sendmsg (sqe, op->data.msg.fd, op->data.msg.msg, op->data.msg.flags);
            break;

        case IoOperation::Opcode::Recv:
            if (op->multishot)
            {
                io_uring_prep_recv_multishot (sqe, op->data.stream.fd, nullptr, 0, op->data.stream.flags);
                sqe->flags |= IOSQE_BUFFER_SELECT;
                sqe->buf_group = op->group;
            }
            else
            {
                io_uring_prep_recv (sqe, op->data.stream.fd, op->data.stream.buf, op->data.stream.len,
                                    op->data.stream.flags);
            }
            break;

        case IoOperation::Opcode::Send:
            io_uring_prep_send (sqe, op->data.stream.fd, op->data.stream.buf, op->data.stream.len,
                                op->data.stream.flags);
            break;

        default:                      // LCOV_EXCL_LINE
            io_uring_prep_nop (sqe);  // LCOV_EXCL_LINE
    }

    io_uring_sqe_set_data (sqe, op);

    if (JOIN_UNLIKELY (op->linked))
    {
        sqe->flags |= IOSQE_IO_LINK;
    }
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : dispatchCqe
// =========================================================================
template <typename Policy>
void join::BasicProactor<Policy>::dispatchCqe (io_uring_cqe* cqe) noexcept
{
    dispatchCqe (cqe, is_default<Policy>{});
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : dispatchCqe
// =========================================================================
template <typename Policy>
void join::BasicProactor<Policy>::dispatchCqe (io_uring_cqe* cqe, std::true_type) noexcept
{
    IoOperation* op = static_cast<IoOperation*> (io_uring_cqe_get_data (cqe));

    if (JOIN_UNLIKELY (op == &_wakeupOp))
    {
        endOperation (op, cqe->res, false);
        if (JOIN_LIKELY (_running.load (std::memory_order_acquire)))
        {
            submitOperation (&_wakeupOp, true);
        }
        return;
    }

    dispatchCqe (cqe, std::false_type{});
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : dispatchCqe
// =========================================================================
template <typename Policy>
void join::BasicProactor<Policy>::dispatchCqe (io_uring_cqe* cqe, std::false_type) noexcept
{
    IoOperation* op = static_cast<IoOperation*> (io_uring_cqe_get_data (cqe));
    if (JOIN_UNLIKELY (op == nullptr))
    {
        return;
    }

    if (JOIN_UNLIKELY (op->state == IoOperation::State::Idle))
    {
        return;  // LCOV_EXCL_LINE
    }

    int result = cqe->res;
    IoRingBuffer* br = nullptr;
    uint16_t bid = 0;

    if ((cqe->flags & IORING_CQE_F_BUFFER) != 0)
    {
        bid = static_cast<uint16_t> (cqe->flags >> IORING_CQE_BUFFER_SHIFT);
        br = op->ring;

        if (op->code == static_cast<uint8_t> (IoOperation::Opcode::RecvMsg))
        {
            msghdr reserved = {};
            reserved.msg_namelen = op->data.msg.namelen;
            reserved.msg_controllen = op->data.msg.controllen;

            io_uring_recvmsg_out* out = io_uring_recvmsg_validate (br->get (bid), result, &reserved);
            if (JOIN_UNLIKELY (out == nullptr))
            {
                result = -EFAULT;  // LCOV_EXCL_LINE
            }
            else
            {
                uint32_t payloadlen = io_uring_recvmsg_payload_length (out, result, &reserved);

                op->data.msg.msg->msg_name = io_uring_recvmsg_name (out);
                op->data.msg.msg->msg_control = io_uring_recvmsg_cmsg_firsthdr (out, &reserved);
                op->data.msg.msg->msg_iov->iov_base = io_uring_recvmsg_payload (out, &reserved);
                op->data.msg.msg->msg_iov->iov_len = payloadlen;
                op->data.msg.msg->msg_iovlen = 1;
                op->data.msg.msg->msg_namelen = out->namelen;
                op->data.msg.msg->msg_controllen = out->controllen;
                op->data.msg.msg->msg_flags = static_cast<int> (out->flags);

                result = static_cast<int> (payloadlen);
            }
        }
        else
        {
            op->data.stream.buf = br->get (bid);
        }
    }

    if ((cqe->flags & IORING_CQE_F_MORE) != 0)
    {
        notifyOperation (op, result, false);
    }
    else
    {
        bool cancelled = (result < 0) && (result == -ECANCELED || op->state == IoOperation::State::Cancelling);
        endOperation (op, result, cancelled);
    }

    if (br != nullptr)
    {
        br->recycle (bid);
    }
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : eventLoop
// =========================================================================
template <typename Policy>
void join::BasicProactor<Policy>::eventLoop () noexcept
{
    eventLoop (has_spin<Policy>{}, has_sqpoll<Policy>{});
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : eventLoop
// =========================================================================
template <typename Policy>
void join::BasicProactor<Policy>::eventLoop (std::false_type, std::false_type) noexcept
{
    if (JOIN_LIKELY (_running.load (std::memory_order_acquire)))
    {
        submitOperation (&_wakeupOp, true);
    }

    Backoff backoff;
    bool running;

    while ((running = _running.load (std::memory_order_acquire)) || !_pendingOps.empty ())
    {
        readCommands ();
        io_uring_submit (&_ring);

        io_uring_cqe* cqe = nullptr;
        if (io_uring_peek_cqe (&_ring, &cqe) == 0)
        {
            backoff.reset ();

            do
            {
                dispatchCqe (cqe);
                io_uring_cqe_seen (&_ring, cqe);
            }
            while (io_uring_peek_cqe (&_ring, &cqe) == 0);

            continue;
        }

        if (!running || !backoff.spinExhausted ())
        {
            backoff ();
            continue;
        }

        _wakeupState.store (WakeupState::Sleeping, std::memory_order_relaxed);
        std::atomic_thread_fence (std::memory_order_seq_cst);
        readCommands ();

        io_uring_submit (&_ring);
        if (JOIN_LIKELY (_running.load (std::memory_order_acquire)))
        {
            if (io_uring_peek_cqe (&_ring, &cqe) != 0)
            {
                io_uring_wait_cqe (&_ring, &cqe);
            }
        }

        _wakeupState.store (WakeupState::Spinning, std::memory_order_seq_cst);
    }
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : eventLoop
// =========================================================================
template <typename Policy>
void join::BasicProactor<Policy>::eventLoop (std::true_type, std::false_type) noexcept
{
    Backoff backoff (Policy::spin);
    bool running;

    while ((running = _running.load (std::memory_order_acquire)) || !_pendingOps.empty ())
    {
        if (JOIN_LIKELY (running))
        {
            readCommands ();
        }
        else
        {
            io_uring_submit (&_ring);  // LCOV_EXCL_LINE
        }

        io_uring_cqe* cqe = nullptr;
        if (JOIN_UNLIKELY (io_uring_peek_cqe (&_ring, &cqe) != 0))
        {
            backoff ();
            continue;
        }

        do
        {
            dispatchCqe (cqe);
            io_uring_cqe_seen (&_ring, cqe);
        }
        while (io_uring_peek_cqe (&_ring, &cqe) == 0);

        backoff.reset ();
    }
}

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : eventLoop
// =========================================================================
template <typename Policy>
void join::BasicProactor<Policy>::eventLoop (std::true_type, std::true_type) noexcept
{
    Backoff backoff (Policy::spin);
    bool running;

    while ((running = _running.load (std::memory_order_acquire)) || !_pendingOps.empty ())
    {
        if (JOIN_LIKELY (running))
        {
            readCommands ();
            // publishes the SQ tail and, in SQPOLL mode, only enters the kernel
            // if the poll thread went to sleep (IORING_SQ_NEED_WAKEUP).
            io_uring_submit (&_ring);
        }
        else
        {
            io_uring_submit (&_ring);  // LCOV_EXCL_LINE
        }

        io_uring_cqe* cqe = nullptr;
        if (JOIN_UNLIKELY (io_uring_peek_cqe (&_ring, &cqe) != 0))
        {
            backoff ();
            continue;
        }

        do
        {
            dispatchCqe (cqe);
            io_uring_cqe_seen (&_ring, cqe);
        }
        while (io_uring_peek_cqe (&_ring, &cqe) == 0);

        backoff.reset ();
    }
}
