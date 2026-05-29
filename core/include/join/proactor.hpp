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

#ifndef JOIN_CORE_PROACTOR_HPP
#define JOIN_CORE_PROACTOR_HPP

// libjoin.
#include <join/io_operation.hpp>
#ifdef JOIN_HAS_IO_URING
#include <join/io_policy.hpp>
#else
#include <join/reactor.hpp>
#endif
#include <join/backoff.hpp>
#include <join/thread.hpp>
#include <join/queue.hpp>

// C++.
#include <utility>
#include <vector>

// C.
#include <sys/eventfd.h>
#include <cerrno>

namespace join
{
    class CompletionHandler;
#ifdef JOIN_HAS_IO_URING
    template <typename Policy>
    class BasicProactor;
    template <typename Policy>
    class BasicProactorThread;

    using Proactor = BasicProactor<IoDefaultPolicy>;
    using HybridProactor = BasicProactor<IoHybridPolicy>;
    using SqpollProactor = BasicProactor<IoSqpollPolicy>;
    using ProactorThread = BasicProactorThread<IoDefaultPolicy>;
    using HybridProactorThread = BasicProactorThread<IoHybridPolicy>;
    using SqpollProactorThread = BasicProactorThread<IoSqpollPolicy>;
#else
    class BasicProactor;
    class BasicProactorThread;

    using Proactor = BasicProactor;
    using ProactorThread = BasicProactorThread;
#endif
}

/**
 * @brief completion handler interface class.
 */
class join::CompletionHandler
{
    /// friendship with proactor.
#ifdef JOIN_HAS_IO_URING
    template <typename Policy>
    friend class join::BasicProactor;
#else
    friend class join::BasicProactor;
#endif

public:
    /**
     * @brief create instance.
     */
    CompletionHandler () = default;

    /**
     * @brief copy constructor.
     * @param other other object to copy.
     */
    CompletionHandler (const CompletionHandler& other) = default;

    /**
     * @brief copy assignment operator.
     * @param other other object to copy.
     * @return current object.
     */
    CompletionHandler& operator= (const CompletionHandler& other) = default;

    /**
     * @brief move constructor.
     * @param other other object to move.
     */
    CompletionHandler (CompletionHandler&& other) = default;

    /**
     * @brief move assignment operator.
     * @param other other object to move.
     * @return current object.
     */
    CompletionHandler& operator= (CompletionHandler&& other) = default;

    /**
     * @brief destroy instance.
     */
    virtual ~CompletionHandler () = default;

protected:
    /**
     * @brief method called when an operation completes successfully.
     * @param op completed operation.
     * @param result number of bytes transferred, or operation-specific value.
     */
    virtual void onComplete ([[maybe_unused]] IoOperation* op, [[maybe_unused]] int result)
    {
        // do nothing.
    }

    /**
     * @brief method called when an operation is cancelled.
     * @param op cancelled operation.
     * @param result -ECANCELED, or other negative errno on failure.
     */
    virtual void onCancel ([[maybe_unused]] IoOperation* op, [[maybe_unused]] int result)
    {
        // do nothing.
    }
};

/**
 * @brief basic proactor class.
 */
#ifdef JOIN_HAS_IO_URING
template <typename Policy = join::IoDefaultPolicy>
class join::BasicProactor
#else
class join::BasicProactor : public join::EventHandler
#endif
{
public:
    /**
     * @brief initialize the proactor and its I/O backend.
     */
    explicit BasicProactor ();

    /**
     * @brief copy constructor.
     * @param other other object to copy.
     */
    BasicProactor (const BasicProactor& other) = delete;

    /**
     * @brief copy assignment operator.
     * @param other other object to copy.
     * @return current object.
     */
    BasicProactor& operator= (const BasicProactor& other) = delete;

    /**
     * @brief move constructor.
     * @param other other object to move.
     */
    BasicProactor (BasicProactor&& other) = delete;

    /**
     * @brief move assignment operator.
     * @param other other object to move.
     * @return current object.
     */
    BasicProactor& operator= (BasicProactor&& other) = delete;

    /**
     * @brief destroy instance.
     */
    ~BasicProactor () noexcept;

    /**
     * @brief submit an asynchronous operation to the proactor.
     * @param op operation to submit.
     * @param flush call io_uring_submit after pushing the SQE if true (io_uring only).
     * @param sync wait for submission acknowledgment if true (default: false).
     * @return 0 on success, -1 on failure.
     */
    int submit (IoOperation* op, bool flush = false, bool sync = false) noexcept;

    /**
     * @brief cancel an in-flight operation.
     * @param op operation to cancel.
     * @param flush call io_uring_submit after pushing the cancel SQE if true (io_uring only).
     * @param sync wait for cancellation acknowledgment if true (default: false).
     * @return 0 on success, -1 on failure (lastError set).
     */
    int cancel (IoOperation* op, bool flush = false, bool sync = false) noexcept;

#ifdef JOIN_HAS_IO_URING
    /**
     * @brief flush pending submissions to the kernel.
     * @param sync wait for flush acknowledgment if true (default: false).
     * @return 0 on success, -1 on failure (lastError set).
     */
    int flush (bool sync = false) noexcept;
#endif

    /**
     * @brief run the event loop (blocking).
     */
    void run ();

    /**
     * @brief stop the event loop.
     * @param sync wait for loop termination if true.
     */
    void stop (bool sync = true) noexcept;

#ifdef JOIN_HAS_IO_URING
    /**
     * @brief register fixed buffers with the io_uring instance.
     * @param iovecs list of buffers to register.
     * @return 0 on success, -1 on failure.
     */
    int registerBuffers (const std::vector<iovec>& iovecs) noexcept;

    /**
     * @brief unregister previously registered fixed buffers from the io_uring instance.
     * @return 0 on success, -1 on failure.
     */
    int unregisterBuffers () noexcept;
#endif

#ifdef JOIN_HAS_NUMA
    /**
     * @brief bind proactor command queue memory to a NUMA node.
     * @param numa NUMA node ID.
     * @return 0 on success, -1 on failure.
     */
    int mbind (int numa) const noexcept;
#endif

    /**
     * @brief lock proactor command queue memory in RAM.
     * @return 0 on success, -1 on failure.
     */
    int mlock () const noexcept;

    /**
     * @brief check if the event loop is running.
     * @return true if the event loop is running.
     */
    bool isRunning () const noexcept;

    /**
     * @brief check if the calling thread is the proactor thread.
     * @return true if called from the proactor thread.
     */
    bool isProactorThread () const noexcept;

private:
#ifdef JOIN_HAS_IO_URING
    /**
     * @brief create the wakeup eventfd descriptor.
     * @return eventfd descriptor on success, -1 on failure.
     */
    int initWakeup (std::true_type) noexcept;

    /**
     * @brief no-op stub; returns -1 for policies that do not use a wakeup eventfd.
     * @return -1.
     */
    int initWakeup (std::false_type) noexcept;

    /**
     * @brief init wakeup operation.
     */
    void initWakeupOp (std::true_type) noexcept;

    /**
     * @brief no-op stub for policies that do not use a wakeup operation.
     */
    void initWakeupOp (std::false_type) noexcept;

    /**
     * @brief init completion queue entries.
     * @param params parameters.
     */
    void initCqEntries (io_uring_params&, std::false_type) noexcept;

    /**
     * @brief init completion queue entries.
     * @param params parameters.
     */
    void initCqEntries (io_uring_params& params, std::true_type) noexcept;

    /**
     * @brief init sqpoll thread idle timeout.
     * @param params parameters.
     */
    void initSqThreadIdle (io_uring_params&, std::false_type) noexcept;

    /**
     * @brief init sqpoll thread idle timeout.
     * @param params parameters.
     */
    void initSqThreadIdle (io_uring_params& params, std::true_type) noexcept;

    /**
     * @brief init sqpoll thread cpu affinity.
     * @param params parameters.
     */
    void initSqThreadCpu (io_uring_params&, std::false_type) noexcept;

    /**
     * @brief init sqpoll thread cpu affinity.
     * @param params parameters.
     */
    void initSqThreadCpu (io_uring_params& params, std::true_type) noexcept;
#endif

    /**
     * @brief command type for proactor dispatcher.
     */
    enum class CommandType
    {
        Submit, /**< submit an operation. */
        Cancel, /**< cancel an in-flight operation. */
        Stop,   /**< stop the event loop. */
#ifdef JOIN_HAS_IO_URING
        Flush, /**< flush pending submissions to the kernel. */
#endif
    };

    /**
     * @brief command for proactor dispatcher.
     */
    struct alignas (64) Command
    {
        CommandType type;        /**< command type. */
        IoOperation* op;         /**< target operation, or nullptr for Stop/Flush. */
        bool flush;              /**< if true, call io_uring_submit after processing (io_uring only). */
        std::atomic<bool>* done; /**< set to true when the command is processed. */
        std::error_code* errc;   /**< filled with the error code on failure. */
    };

    /**
     * @brief write command to queue and wake dispatcher.
     * @param cmd command to write.
     * @return 0 on success, -1 on failure.
     */
    int writeCommand (const Command& cmd) noexcept;

#ifdef JOIN_HAS_IO_URING
    /**
     * @brief push command to queue and write to the wakeup eventfd to wake the dispatcher.
     * @param cmd command to write.
     * @return 0 on success, -1 on failure.
     */
    int writeCommand (const Command& cmd, std::true_type) noexcept;

    /**
     * @brief push command to queue; the polling event loop drains it without a wakeup signal.
     * @param cmd command to write.
     * @return 0 on success, -1 on failure.
     */
    int writeCommand (const Command& cmd, std::false_type) noexcept;
#endif

    /**
     * @brief read and process all pending commands from queue.
     */
    void readCommands () noexcept;

    /**
     * @brief process a single command.
     * @param cmd command to process.
     */
    void processCommand (const Command& cmd) noexcept;

    /**
     * @brief submit an operation directly to the backend.
     * @param op operation to submit.
     * @param flush if true, flush pending submissions to the kernel after submitting (io_uring only).
     * @return 0 on success, -1 on failure.
     */
    int submitOperation (IoOperation* op, bool flush) noexcept;

    /**
     * @brief cancel an in-flight operation directly in the backend.
     * @param op operation to cancel.
     * @param flush if true, flush pending submissions to the kernel after cancelling (io_uring only).
     * @return 0 on success, -1 on failure.
     */
    int cancelOperation (IoOperation* op, bool flush) noexcept;

    /**
     * @brief cancel all in-flight operations.
     */
    void cancelAllOperations () noexcept;

    /**
     * @brief dispatch completion callback and reset operation state.
     * @param op operation to dispatch, may be nullptr.
     * @param result negative errno or bytes-transferred result.
     * @param cancelled if true, dispatch to onCancel; otherwise to onComplete.
     */
    void dispatchOperation (IoOperation* op, int result, bool cancelled) noexcept;

    /**
     * @brief end an operation, dispatching onCancel or onComplete.
     * @param op operation to end, may be nullptr.
     * @param result negative errno or bytes-transferred result.
     * @param cancelled if true, dispatch to onCancel; otherwise to onComplete.
     */
    void endOperation (IoOperation* op, int result, bool cancelled = false) noexcept;

#ifdef JOIN_HAS_IO_URING
    /**
     * @brief get a free submission queue entry, submitting pending entries if the ring is full.
     * @return pointer to sqe, or nullptr if unavailable.
     */
    io_uring_sqe* getSqe () noexcept;

    /**
     * @brief prepare a submission queue entry for the given operation and attach it as user data.
     * @param sqe submission queue entry.
     * @param op operation to prepare.
     */
    void prepareSqe (io_uring_sqe* sqe, IoOperation* op) noexcept;

    /**
     * @brief re-arm the wakeup eventfd read on the ring.
     */
    void rearmWakeup () noexcept;

    /**
     * @brief dispatch a completion queue entry to the appropriate handler.
     * @param cqe completion queue entry.
     */
    void dispatchCqe (io_uring_cqe* cqe) noexcept;

    /**
     * @brief dispatch a completion queue entry to the appropriate handler.
     * @param cqe completion queue entry.
     */
    void dispatchCqe (io_uring_cqe* cqe, std::true_type) noexcept;

    /**
     * @brief dispatch a completion queue entry to the appropriate handler.
     * @param cqe completion queue entry.
     */
    void dispatchCqe (io_uring_cqe* cqe, std::false_type) noexcept;

    /**
     * @brief run the backend event loop until stop() is called.
     */
    void eventLoop () noexcept;

    /**
     * @brief run the default io_uring event loop (blocking wait with eventfd wakeup) until stop() is called.
     */
    void eventLoop (std::false_type, std::false_type) noexcept;

    /**
     * @brief run the hybrid backend event loop until stop() is called.
     */
    void eventLoop (std::true_type, std::false_type) noexcept;

    /**
     * @brief run the sqpoll backend event loop until stop() is called.
     */
    void eventLoop (std::true_type, std::true_type) noexcept;
#else
    /**
     * @brief return true if opcode requires EPOLLOUT.
     * @param code raw opcode value.
     * @return true for Connect, Write, WriteFixed, SendMsg, Send.
     */
    static bool isWriteOp (uint8_t code) noexcept;

    /**
     * @brief execute the syscall described by op.
     * @param op operation to execute.
     * @return bytes transferred (>= 0) or -errno (< 0).
     */
    static int executeOp (IoOperation* op) noexcept;

    /**
     * @brief method called when data are ready to be read on handle.
     * @param fd file descriptor.
     */
    void onReadable (int fd) noexcept override;

    /**
     * @brief method called when data are ready to be written on handle.
     * @param fd file descriptor.
     */
    void onWriteable (int fd) noexcept override;

    /**
     * @brief method called when handle was closed by the peer.
     * @param fd file descriptor.
     */
    void onClose (int fd) noexcept override;

    /**
     * @brief method called when an error occurred on handle.
     * @param fd file descriptor.
     */
    void onError (int fd) noexcept override;
#endif

    /// command queue size.
    static constexpr size_t _queueSize = 1024;

    /// command queue.
    LocalMem::Mpsc::Queue<Command> _commands;

    /// set to true while a sync stop() is in progress.
    std::atomic<bool> _stopping{false};

    /// eventfd descriptor.
    int _wakeup = -1;

#ifdef JOIN_HAS_IO_URING
    /// buffer for the wakeup eventfd read.
    uint64_t _wakeupBuf = 0;

    /// internal operation used to watch the wakeup eventfd.
    IoOperation _wakeupOp = {};

    /// io_uring instance.
    io_uring _ring = {};

    /// in-flight operations.
    std::vector<IoOperation*> _pendingOps;

    /// invalid thread id sentinel.
    static constexpr pthread_t _invalidThreadId = static_cast<pthread_t> (-1);

    /// proactor thread id.
    std::atomic<pthread_t> _threadId{_invalidThreadId};

    /// running flag.
    std::atomic<bool> _running{false};
#else
    /// pending read operations.
    std::vector<IoOperation*> _readOps;

    /// pending write operations.
    std::vector<IoOperation*> _writeOps;

    /// reactor instance.
    Reactor _reactor;
#endif
};

// =========================================================================
//   CLASS     : BasicProactor
//   METHOD    : submit
// =========================================================================
#ifdef JOIN_HAS_IO_URING
template <typename Policy>
int join::BasicProactor<Policy>::submit (IoOperation* op, bool flush, bool sync) noexcept
#else
inline int join::BasicProactor::submit (IoOperation* op, bool flush, bool sync) noexcept
#endif
{
    if (isProactorThread ())
    {
        return submitOperation (op, flush);
    }

    std::atomic<bool> done{false}, *pdone = nullptr;
    std::error_code errc, *perrc = nullptr;

    if (JOIN_UNLIKELY (sync))
    {
        pdone = &done;
        perrc = &errc;
    }

    if (JOIN_UNLIKELY (writeCommand ({CommandType::Submit, op, flush, pdone, perrc}) == -1))
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
//   METHOD    : cancel
// =========================================================================
#ifdef JOIN_HAS_IO_URING
template <typename Policy>
int join::BasicProactor<Policy>::cancel (IoOperation* op, bool flush, bool sync) noexcept
#else
inline int join::BasicProactor::cancel (IoOperation* op, bool flush, bool sync) noexcept
#endif
{
    if (isProactorThread ())
    {
        return cancelOperation (op, flush);
    }

    std::atomic<bool> done{false}, *pdone = nullptr;
    std::error_code errc, *perrc = nullptr;

    if (JOIN_UNLIKELY (sync))
    {
        pdone = &done;
        perrc = &errc;
    }

    if (JOIN_UNLIKELY (writeCommand ({CommandType::Cancel, op, flush, pdone, perrc}) == -1))
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
//   METHOD    : dispatchOperation
// =========================================================================
#ifdef JOIN_HAS_IO_URING
template <typename Policy>
void join::BasicProactor<Policy>::dispatchOperation (IoOperation* op, int result, bool cancelled) noexcept
#else
inline void join::BasicProactor::dispatchOperation (IoOperation* op, int result, bool cancelled) noexcept
#endif
{
    if (JOIN_UNLIKELY (op == nullptr))
    {
        return;  // LCOV_EXCL_LINE
    }

    if (JOIN_LIKELY (op->handler))
    {
        if (cancelled)
        {
            op->handler->onCancel (op, result);
        }
        else
        {
            op->handler->onComplete (op, result);
        }
    }

    op->state = IoOperation::State::Idle;
}

#ifdef JOIN_HAS_IO_URING
#include "proactor_uring_impl.hpp"
#else
#include "proactor_epoll_impl.hpp"
#endif

/**
 * @brief Convenience class that owns a Proactor running on a dedicated background thread.
 */
#ifdef JOIN_HAS_IO_URING
template <typename Policy = join::IoDefaultPolicy>
class join::BasicProactorThread
#else
class join::BasicProactorThread
#endif
{
public:
    /**
     * @brief get the Proactor instance owned by the singleton ProactorThread.
     * @return reference to the Proactor.
     */
#ifdef JOIN_HAS_IO_URING
    static BasicProactor<Policy>& proactor ()
#else
    static BasicProactor& proactor ()
#endif
    {
        return instance ()._proactor;
    }

    /**
     * @brief set proactor thread affinity.
     * @param core CPU core (-1 to disable pinning).
     * @return 0 on success, -1 on failure.
     */
    static int affinity (int core)
    {
        return instance ()._dispatcher.affinity (core);
    }

    /**
     * @brief get proactor thread affinity.
     * @return core index, or -1 if not pinned.
     */
    static int affinity () noexcept
    {
        return instance ()._dispatcher.affinity ();
    }

    /**
     * @brief set proactor thread scheduling priority.
     * @param prio 0 = SCHED_OTHER, 1-99 = SCHED_FIFO.
     * @return 0 on success, -1 on failure.
     */
    static int priority (int prio)
    {
        return instance ()._dispatcher.priority (prio);
    }

    /**
     * @brief get proactor thread scheduling priority.
     * @return current priority.
     */
    static int priority () noexcept
    {
        return instance ()._dispatcher.priority ();
    }

    /**
     * @brief get the native handle of the proactor thread.
     * @return pthread_t handle.
     */
    static pthread_t handle () noexcept
    {
        return instance ()._dispatcher.handle ();
    }

#ifdef JOIN_HAS_NUMA
    /**
     * @brief bind proactor command queue memory to a NUMA node.
     * @param numa NUMA node ID.
     * @return 0 on success, -1 on failure.
     */
    static int mbind (int numa) noexcept
    {
        return instance ()._proactor.mbind (numa);
    }
#endif

    /**
     * @brief lock proactor command queue memory in RAM.
     * @return 0 on success, -1 on failure.
     */
    static int mlock () noexcept
    {
        return instance ()._proactor.mlock ();
    }

private:
    /**
     * @brief get the singleton ProactorThread instance.
     * @return reference to the singleton ProactorThread.
     */
    static BasicProactorThread& instance ()
    {
        static BasicProactorThread proactorThread;
        return proactorThread;
    }

    /**
     * @brief construct the ProactorThread and start the event loop thread.
     */
    BasicProactorThread ()
    {
        _dispatcher = Thread ([this] () {
            _proactor.run ();
        });
    }

    /**
     * @brief copy constructor.
     * @param other other object to copy.
     */
    BasicProactorThread (const BasicProactorThread&) = delete;

    /**
     * @brief copy assignment operator.
     * @param other other object to copy.
     * @return current object.
     */
    BasicProactorThread& operator= (const BasicProactorThread&) = delete;

    /**
     * @brief move constructor.
     * @param other other object to move.
     */
    BasicProactorThread (BasicProactorThread&&) = delete;

    /**
     * @brief move assignment operator.
     * @param other other object to move.
     * @return current object.
     */
    BasicProactorThread& operator= (BasicProactorThread&&) = delete;

    /**
     * @brief destroy the ProactorThread and cleanly shut down the event loop.
     */
    ~BasicProactorThread ()
    {
        _proactor.stop ();
        _dispatcher.join ();
    }

#ifdef JOIN_HAS_IO_URING
    /// owned Proactor instance.
    BasicProactor<Policy> _proactor;
#else
    /// owned Proactor instance.
    BasicProactor _proactor;
#endif

    /// background dispatcher thread.
    Thread _dispatcher;
};

#endif
