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
#include <join/reactor.hpp>
#include <join/thread.hpp>
#include <join/queue.hpp>

// C++.
#include <vector>

// C.
#include <sys/socket.h>

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
         * @brief operation code.
         */
        enum class Opcode : uint8_t
        {
            Accept,
            Read,
            Write,
            ReadFixed,
            WriteFixed,
            RecvMsg,
            SendMsg,
        };

        /**
         * @brief payload for accept.
         */
        struct AcceptData
        {
            /// peer address output buffer.
            sockaddr* addr;

            /// peer address length.
            socklen_t* addrlen;

            /// accept flags.
            int flags;
        };

        /**
         * @brief payload for read / read fixed / write / write fixed.
         */
        struct RwData
        {
            /// buffer.
            void* buf;

            /// number of bytes to proceed.
            unsigned long len;

            /// registered buffer index (ignored with reactor backend).
            uint16_t index;

            /// use registered buffer (ignored with reactor backend).
            bool fixed;
        };

        /**
         * @brief payload for sendmsg / recvmsg.
         */
        struct MsgData
        {
            /// message header.
            msghdr* msg;

            /// send / recv flags.
            int flags;
        };

        union Data
        {
            AcceptData accept;
            RwData rw;
            MsgData msg;
        };

        /**
         * @brief build an accept operation.
         * @param fd listening socket file descriptor.
         * @param addr peer address output buffer.
         * @param addrlen peer address length output.
         * @param flags accept flags.
         * @param handler handler to notify on completion.
         * @return initialized IoOperation.
         */
        static IoOperation makeAccept (int fd, sockaddr* addr, socklen_t* addrlen, int flags,
                                       CompletionHandler* handler) noexcept;

        /**
         * @brief build a regular read operation.
         * @param fd file descriptor to read from.
         * @param buf destination buffer.
         * @param len number of bytes to read.
         * @param handler handler to notify on completion.
         * @return initialized IoOperation.
         */
        static IoOperation makeRead (int fd, void* buf, uint32_t len, CompletionHandler* handler) noexcept;

        /**
         * @brief build a regular write operation.
         * @param fd file descriptor to write to.
         * @param buf source buffer.
         * @param len number of bytes to write.
         * @param handler handler to notify on completion.
         * @return initialized IoOperation.
         */
        static IoOperation makeWrite (int fd, const void* buf, uint32_t len, CompletionHandler* handler) noexcept;

        /**
         * @brief build a regular read operation.
         * @param fd file descriptor to read from.
         * @param buf destination buffer.
         * @param len number of bytes to read.
         * @param index registered buffer index.
         * @param handler handler to notify on completion.
         * @return initialized IoOperation.
         */
        static IoOperation makeReadFixed (int fd, void* buf, uint32_t len, uint16_t index,
                                          CompletionHandler* handler) noexcept;

        /**
         * @brief build a regular write operation.
         * @param fd file descriptor to write to.
         * @param buf source buffer.
         * @param len number of bytes to write.
         * @param index registered buffer index.
         * @param handler handler to notify on completion.
         * @return initialized IoOperation.
         */
        static IoOperation makeWriteFixed (int fd, const void* buf, uint32_t len, uint16_t index,
                                           CompletionHandler* handler) noexcept;

        /**
         * @brief build a receive-message operation.
         * @param fd socket file descriptor.
         * @param msg message header.
         * @param flags recv flags.
         * @param handler handler to notify on completion.
         * @return initialized IoOperation.
         */
        static IoOperation makeRecvmsg (int fd, msghdr* msg, int flags, CompletionHandler* handler) noexcept;

        /**
         * @brief build a send-message operation.
         * @param fd socket file descriptor.
         * @param msg message header.
         * @param flags send flags.
         * @param handler handler to notify on completion.
         * @return initialized IoOperation.
         */
        static IoOperation makeSendmsg (int fd, msghdr* msg, int flags, CompletionHandler* handler) noexcept;

        /// file descriptor.
        int fd;

        /// operation code.
        uint8_t code = 0;

        /// operation state.
        State state = State::Idle;

        /// handler to dispatch to on completion.
        CompletionHandler* handler = nullptr;

        /// operation code specific payload.
        Data data = {};
    };

    /**
     * @brief Proactor class.
     */
    class Proactor : public EventHandler
    {
    public:
        /**
         * @brief default constructor.
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
         * @brief cancel an in-flight operation.
         * @param op operation to cancel.
         * @param sync block until the cancellation has been acknowledged.
         * @return 0 on success, -1 on failure (lastError set).
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
         * @brief bind reactor command queue memory to a NUMA node.
         * @param numa NUMA node ID.
         * @return 0 on success, -1 on failure.
         */
        int mbind (int numa) const noexcept;
#endif

        /**
         * @brief lock reactor command queue memory in RAM.
         * @return 0 on success, -1 on failure.
         */
        int mlock () const noexcept;

        /**
         * @brief check if the calling thread is the proactor thread.
         * @return true if called from the reactor thread.
         */
        bool isProactorThread () const noexcept;

    protected:
        /// queue size.
        static constexpr size_t _queueSize = 1024;

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
            std::error_code* errc;
        };

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
         * @brief return true if opcode requires EPOLLOUT.
         * @param code raw opcode value.
         * @return true for Write, WriteFixed, SendMsg.
         */
        static bool isWriteOp (uint8_t code) noexcept;

        /**
         * @brief execute the syscall described by op.
         * @param op operation to execute.
         * @return bytes transferred (>= 0) or -errno (< 0).
         */
        static int executeOperation (IoOperation* op) noexcept;

        /**
         * @brief abort both pending operations for fd with the given result.
         * @param fd file descriptor.
         * @param result negative errno to deliver to handlers.
         */
        void abortOperations (int fd, int result) noexcept;

        /**
         * @brief method called when data are ready to be read on handle.
         * @param fd file descriptor.
         */
        void onReadable (int fd) override;

        /**
         * @brief method called when data are ready to be written on handle.
         * @param fd file descriptor.
         */
        void onWriteable (int fd) override;

        /**
         * @brief method called when handle was closed by the peer.
         * @param fd file descriptor.
         */
        void onClose (int fd) override;

        /**
         * @brief method called when an error occurred on handle.
         * @param fd file descriptor.
         */
        void onError (int fd) override;

        /// eventfd descriptor.
        int _wakeup = -1;

        /// command queue.
        LocalMem::Mpsc::Queue<Command> _commands;

        /// running flag.
        std::atomic<bool> _running{false};

        /// pending read operations.
        std::vector<IoOperation*> _readOps;

        /// pending write operations.
        std::vector<IoOperation*> _writeOps;

        /// reactor instance.
        Reactor _reactor;
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
         * @param core CPU core (-1 to disable pinning).
         * @return 0 on success, -1 on failure.
         */
        static int affinity (int core);

        /**
         * @brief get proactor thread affinity.
         * @return core index, or -1 if not pinned.
         */
        static int affinity ();

        /**
         * @brief set proactor thread scheduling priority.
         * @param prio 0 = SCHED_OTHER, 1-99 = SCHED_FIFO.
         * @return 0 on success, -1 on failure.
         */
        static int priority (int prio);

        /**
         * @brief get proactor thread scheduling priority.
         * @return current priority.
         */
        static int priority ();

        /**
         * @brief get the native handle of the proactor thread.
         * @return pthread_t handle.
         */
        static pthread_t handle ();

#ifdef JOIN_HAS_NUMA
        /**
         * @brief bind reactor command queue memory to a NUMA node.
         * @param numa NUMA node ID.
         * @return 0 on success, -1 on failure.
         */
        static int mbind (int numa);
#endif

        /**
         * @brief lock reactor command queue memory in RAM.
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
        ProactorThread (const ProactorThread&) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to copy.
         * @return current object.
         */
        ProactorThread& operator= (const ProactorThread&) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        ProactorThread (ProactorThread&&) = delete;

        /**
         * @brief move assignment operator.
         * @param other other object to move.
         * @return current object.
         */
        ProactorThread& operator= (ProactorThread&&) = delete;

        /**
         * @brief destroy the ProactorThread and cleanly shut down the event loop.
         */
        ~ProactorThread ();

        /// owned Proactor instance.
        Proactor _proactor;

        /// background dispatcher thread.
        Thread _dispatcher;
    };
}

#endif
