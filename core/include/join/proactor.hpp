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
#include <join/thread.hpp>
#include <join/queue.hpp>

// C.
#include <sys/socket.h>
#include <liburing.h>

namespace join
{
    // forward declaration.
    struct IoOperation;

    /**
     * @brief completion handler interface class.
     */
    class CompletionHandler
    {
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

        /// friendship with proactor.
        friend class Proactor;
    };

    /**
     * @brief Describes a single asynchronous operation submitted to the Proactor.
     */
    struct alignas (64) IoOperation
    {
        /**
         * @brief operation lifecycle state.
         */
        enum class State : uint8_t
        {
            Idle,
            Submitted,
            Cancelling,
        };

        /**
         * @brief payload for IORING_OP_TIMEOUT.
         */
        struct TimeoutData
        {
            /// absolute or relative kernel timespec.
            __kernel_timespec ts;

            /// 0 or IORING_TIMEOUT_ABS.
            uint32_t flags;
        };

        /**
         * @brief payload for IORING_OP_READ / IORING_OP_READ_FIXED / IORING_OP_WRITE / IORING_OP_WRITE_FIXED.
         */
        struct RwData
        {
            /// file descriptor.
            int fd;

            /// buffer.
            void* buf;

            /// number of bytes to proceed.
            uint32_t len;

            /// registered buffer index (FIXED only, ignored otherwise).
            uint16_t index;

            /// use FIXED if true.
            bool fixed;
        };

        /**
         * @brief payload for IORING_OP_SENDMSG / IORING_OP_RECVMSG.
         */
        struct MsgData
        {
            /// file descriptor.
            int fd;

            /// message header.
            msghdr* msg;

            /// send/recv flags.
            uint32_t flags;
        };

        /**
         * @brief payload for IORING_OP_ACCEPT.
         */
        struct AcceptData
        {
            /// listening file descriptor.
            int fd;

            /// peer address output buffer.
            sockaddr* addr;

            /// peer address length.
            socklen_t* addrlen;

            /// accept flags.
            uint32_t flags;
        };

        /**
         * @brief payload for IORING_OP_ASYNC_CANCEL.
         */
        // struct CancelData
        // {
        //     /// operation to cancel, identified by its address as user_data.
        //     IoOperation* target;
        // };

        union Data
        {
            TimeoutData timeout;
            RwData rw;
            MsgData msg;
            AcceptData accept;
            // CancelData cancel;
        };

        /**
         * @brief build a relative or absolute timeout operation.
         * @param ts kernel timespec (relative duration or absolute deadline).
         * @param flags 0 for relative, IORING_TIMEOUT_ABS for absolute.
         * @param handler handler to notify on completion.
         * @return initialized IoOperation.
         */
        static IoOperation makeTimeout (const __kernel_timespec& ts, uint32_t flags,
                                        CompletionHandler* handler) noexcept;

        /**
         * @brief build a regular (non-registered) read operation.
         * @param fd file descriptor to read from.
         * @param buf destination buffer.
         * @param len number of bytes to read.
         * @param handler handler to notify on completion.
         * @return initialized IoOperation.
         */
        static IoOperation makeRead (int fd, void* buf, uint32_t len, CompletionHandler* handler) noexcept;

        /**
         * @brief build a fixed-buffer read operation (requires registered buffers).
         * @param fd file descriptor to read from.
         * @param buf destination buffer (must belong to a registered region).
         * @param len number of bytes to read.
         * @param index index of the registered buffer.
         * @param handler handler to notify on completion.
         * @return initialized IoOperation.
         */
        static IoOperation makeReadFixed (int fd, void* buf, uint32_t len, uint16_t index,
                                          CompletionHandler* handler) noexcept;

        /**
         * @brief build a regular (non-registered) write operation.
         * @param fd file descriptor to write to.
         * @param buf source buffer.
         * @param len number of bytes to write.
         * @param handler handler to notify on completion.
         * @return initialized IoOperation.
         */
        static IoOperation makeWrite (int fd, const void* buf, uint32_t len, CompletionHandler* handler) noexcept;

        /**
         * @brief build a fixed-buffer write operation (requires registered buffers).
         * @param fd file descriptor to write to.
         * @param buf source buffer (must belong to a registered region).
         * @param len number of bytes to write.
         * @param index index of the registered buffer.
         * @param handler handler to notify on completion.
         * @return initialized IoOperation.
         */
        static IoOperation makeWriteFixed (int fd, const void* buf, uint32_t len, uint16_t index,
                                           CompletionHandler* handler) noexcept;
        /**
         * @brief build a send-message operation.
         * @param fd socket file descriptor.
         * @param msg message header.
         * @param flags send flags.
         * @param handler handler to notify on completion.
         * @return initialized IoOperation.
         */
        static IoOperation makeSendmsg (int fd, msghdr* msg, uint32_t flags, CompletionHandler* handler) noexcept;

        /**
         * @brief build a receive-message operation.
         * @param fd socket file descriptor.
         * @param msg message header.
         * @param flags recv flags.
         * @param handler handler to notify on completion.
         * @return initialized IoOperation.
         */
        static IoOperation makeRecvmsg (int fd, msghdr* msg, uint32_t flags, CompletionHandler* handler) noexcept;

        /**
         * @brief build an accept operation.
         * @param fd listening socket file descriptor.
         * @param addr peer address output buffer (may be nullptr).
         * @param addrlen peer address length output (may be nullptr).
         * @param flags accept flags (e.g. SOCK_NONBLOCK | SOCK_CLOEXEC).
         * @param handler handler to notify on completion.
         * @return initialized IoOperation.
         */
        static IoOperation makeAccept (int fd, sockaddr* addr, socklen_t* addrlen, uint32_t flags,
                                       CompletionHandler* handler) noexcept;

        /**
         * @brief build a cancellation operation targeting another in-flight operation.
         * @param target operation to cancel.
         * @param handler handler to notify when the cancellation itself completes.
         * @return initialized IoOperation.
         */
        // static IoOperation makeCancel (IoOperation* target, CompletionHandler* handler) noexcept;

        /// io_uring opcode (IORING_OP_TIMEOUT, IORING_OP_READ, ...).
        uint8_t code = 0;

        /// state.
        State state = State::Idle;

        /// handler to dispatch to on completion or cancellation.
        CompletionHandler* handler = nullptr;

        /// opcode-specific payload.
        Data data = {};
    };

    /**
     * @brief Proactor class.
     */
    class Proactor
    {
    public:
        /**
         * @brief create instance.
         */
        Proactor ();

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        Proactor (const Proactor& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to copy.
         * @return current object.
         */
        Proactor& operator= (const Proactor& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        Proactor (Proactor&& other) = delete;

        /**
         * @brief move assignment operator.
         * @param other other object to move.
         * @return current object.
         */
        Proactor& operator= (Proactor&& other) = delete;

        /**
         * @brief destroy instance.
         */
        ~Proactor () noexcept;

        /**
         * @brief submit an asynchronous operation to the proactor.
         * @param op operation to submit.
         * @param sync wait for submission acknowledgment if true.
         * @return 0 on success, -1 on failure.
         */
        int submit (IoOperation* op, bool sync = true) noexcept;

        /**
         * @brief cancel an in-flight asynchronous operation.
         * @param op operation to cancel.
         * @param sync wait for cancellation acknowledgment if true.
         * @return 0 on success, -1 on failure.
         */
        int cancel (IoOperation* op, bool sync = true) noexcept;

        /**
         * @brief run the event loop (blocking).
         */
        void run ();

        /**
         * @brief stop the event loop.
         * @param sync wait for loop termination if true.
         */
        void stop (bool sync = true) noexcept;

#ifdef JOIN_HAS_NUMA
        /**
         * @brief bind command queue memory to a NUMA node.
         * @param numa NUMA node ID.
         * @return 0 on success, -1 on failure.
         */
        int mbind (int numa) const noexcept;
#endif

        /**
         * @brief lock command queue memory in RAM.
         * @return 0 on success, -1 on failure.
         */
        int mlock () const noexcept;

        /**
         * @brief check if the calling thread is the proactor thread.
         * @return true if called from the proactor thread.
         */
        bool isProactorThread () const noexcept;

    private:
        /// queue size.
        static constexpr size_t _queueSize = 1024;

        /// io_uring submission/completion ring size.
        static constexpr size_t _ringSize = 1024;

        /**
         * @brief Command type for proactor dispatcher.
         */
        enum class CommandType
        {
            Submit,
            Cancel,
            Stop
        };

        /**
         * @brief Command for proactor dispatcher.
         */
        struct alignas (64) Command
        {
            CommandType type;
            IoOperation* op;
            std::atomic<bool>* done;
            std::atomic<int>* errc;
        };

        /**
         * @brief get a free SQE from the ring, flushing if necessary.
         * @return pointer to an SQE, or nullptr on failure.
         */
        io_uring_sqe* getSqe () noexcept;

        /**
         * @brief submit op directly onto the ring.
         * @param op operation to submit.
         * @return 0 on success, -1 on failure.
         */
        int submitOperation (IoOperation* op) noexcept;

        /**
         * @brief cancel op directly on the ring.
         * @param op operation to cancel.
         * @return 0 on success, -1 on failure.
         */
        int cancelOperation (IoOperation* op) noexcept;

        /**
         * @brief (re)submit the internal wakeup read on _wakeup eventfd.
         */
        void rearmWakeup () noexcept;

        /**
         * @brief write command to queue and wake dispatcher.
         * @param cmd command to write.
         * @return 0 on success, -1 on failure.
         */
        int writeCommand (const Command& cmd) noexcept;

        /**
         * @brief process a single command.
         * @param cmd command to process.
         */
        void processCommand (const Command& cmd) noexcept;

        /**
         * @brief read and process all pending commands from queue.
         */
        void readCommands () noexcept;

        /**
         * @brief dispatch a single CQE to its completion handler.
         * @param cqe completed queue entry.
         */
        void dispatchCqe (io_uring_cqe* cqe) noexcept;

        /**
         * @brief fill an SQE from an operation.
         * @param op source operation.
         * @param sqe submission queue entry to populate.
         */
        void prepareSqe (const IoOperation& op, io_uring_sqe* sqe) const noexcept;

        /**
         * @brief dispatch a completed operation to its handler.
         * @param op operation whose CQE has arrived.
         * @param result cqe->res.
         */
        void dispatchOperation (IoOperation* op, int result) noexcept;

        /**
         * @brief main event loop running in dispatcher thread.
         */
        void eventLoop ();

        /// eventfd descriptor.
        int _wakeup = -1;

        /// read buffer for the wakeup eventfd.
        uint64_t _wakeupBuf = 0;

        /// internal IoOperation that keeps a persistent READ on _wakeup in the ring.
        IoOperation _wakeupOp;

        /// io_uring instance.
        io_uring _ring = {};

        /// command queue.
        LocalMem::Mpsc::Queue<Command> _commands;

        /// running flag for dispatcher thread.
        std::atomic<bool> _running{false};

        /// event loop thread ID.
        std::atomic<pthread_t> _threadId{0};
    };

    /**
     * @brief Convenience class that owns a Proactor running on a dedicated background thread.
     */
    class ProactorThread
    {
    public:
        /**
         * @brief get the global Proactor instance.
         * @return pointer to the singleton Proactor.
         */
        static Proactor* proactor ();

        /**
         * @brief set proactor thread affinity.
         * @param core thread core affinity (-1 to disable pinning).
         * @return 0 on success, -1 on failure.
         */
        static int affinity (int core);

        /**
         * @brief get proactor thread affinity.
         * @return affinity or -1 if not pinned.
         */
        static int affinity ();

        /**
         * @brief set proactor thread priority.
         * @param prio thread priority (0 = SCHED_OTHER, 1-99 = SCHED_FIFO).
         * @return 0 on success, -1 on failure.
         */
        static int priority (int prio);

        /**
         * @brief get proactor thread priority.
         * @return priority.
         */
        static int priority ();

        /**
         * @brief get the handle of the proactor thread.
         * @return proactor thread handle.
         */
        static pthread_t handle ();

#ifdef JOIN_HAS_NUMA
        /**
         * @brief bind command queue memory to a NUMA node.
         * @param numa NUMA node ID.
         * @return 0 on success, -1 on failure.
         */
        static int mbind (int numa);
#endif

        /**
         * @brief lock command queue memory in RAM.
         * @return 0 on success, -1 on failure.
         */
        static int mlock ();

    private:
        /**
         * @brief get the singleton ProactorThread instance.
         * @return reference to the singleton ProactorThread.
         */
        static ProactorThread& instance ();

        /**
         * @brief construct the ProactorThread and start the event loop thread.
         */
        ProactorThread ();

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        ProactorThread (const ProactorThread& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to copy.
         * @return current object.
         */
        ProactorThread& operator= (const ProactorThread& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        ProactorThread (ProactorThread&& other) noexcept = delete;

        /**
         * @brief move assignment operator.
         * @param other other object to move.
         * @return current object.
         */
        ProactorThread& operator= (ProactorThread&& other) noexcept = delete;

        /**
         * @brief destroy the ProactorThread and cleanly shut down the event loop.
         */
        ~ProactorThread ();

        /// Proactor instance.
        Proactor _proactor;

        /// Background thread.
        Thread _dispatcher;
    };
}

#endif
