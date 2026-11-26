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

#ifndef JOIN_CORE_IO_OPERATION_HPP
#define JOIN_CORE_IO_OPERATION_HPP

// libjoin.
#include <join/io_ring_buffer.hpp>

// C++.
#include <atomic>

// C.
#include <sys/socket.h>
#include <sys/types.h>
#include <cstdint>
#include <cstddef>

namespace join
{
    // forward declaration.
    class CompletionHandler;

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
            Idle,      /**< operation is not in flight. */
            Submitted, /**< operation is in flight. */
            Busy,      /**< a completion is in progress. */
            Suspended, /**< operation has been suspended. */
        };

        /**
         * @brief operation code.
         */
        enum class Opcode : uint8_t
        {
            Poll,      /**< wait for readiness on a file descriptor. */
            Accept,    /**< accept an incoming connection. */
            Connect,   /**< initiate an outgoing connection. */
            Read,      /**< read from a file descriptor. */
            Write,     /**< write to a file descriptor. */
            RecvMsg,   /**< receive a message with ancillary data. */
            SendMsg,   /**< send a message with ancillary data. */
            SendMsgZc, /**< send a message with ancillary data without copy. */
            Recv,      /**< receive data from a socket. */
            Send,      /**< send data on a socket. */
            SendZc,    /**< send data on a socket without copy. */
        };

        /**
         * @brief default constructor.
         */
        IoOperation () noexcept
        {
            state.store (State::Idle, std::memory_order_release);
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        IoOperation (const IoOperation& other) noexcept;

        /**
         * @brief copy assignment operator.
         * @param other other object to copy.
         * @return current object.
         */
        IoOperation& operator= (const IoOperation& other) noexcept;

        /**
         * @brief payload for poll.
         */
        struct PollData
        {
            /// requested events.
            uint32_t events;
        };

        /**
         * @brief build a poll operation.
         * @param fd file descriptor to watch.
         * @param events requested events, POLLIN or POLLOUT.
         * @param handler handler to notify on completion.
         * @return initialized IoOperation.
         */
        static IoOperation makePoll (int fd, uint32_t events, CompletionHandler* handler) noexcept;

        /**
         * @brief build a multishot poll operation, staying armed until cancelled or failed.
         * @param fd file descriptor to watch.
         * @param events requested events, POLLIN or POLLOUT.
         * @param handler handler to notify on completion.
         * @return initialized IoOperation.
         */
        static IoOperation makePollMulti (int fd, uint32_t events, CompletionHandler* handler) noexcept;

        /**
         * @brief payload for accept.
         */
        struct AcceptData
        {
            /// peer address.
            sockaddr* addr;

            /// peer address length.
            socklen_t* addrlen;
        };

        /**
         * @brief build an accept operation.
         * @param fd listening socket file descriptor.
         * @param addr peer address.
         * @param addrlen peer address length.
         * @param flags accept flags.
         * @param handler handler to notify on completion.
         * @return initialized IoOperation.
         */
        static IoOperation makeAccept (int fd, sockaddr* addr, socklen_t* addrlen, int flags,
                                       CompletionHandler* handler) noexcept;

        /**
         * @brief build a multishot accept operation.
         * @param fd listening socket file descriptor.
         * @param flags accept flags.
         * @param handler handler to notify on completion.
         * @return initialized IoOperation.
         */
        static IoOperation makeAcceptMulti (int fd, int flags, CompletionHandler* handler) noexcept;

        /**
         * @brief payload for connect.
         */
        struct ConnectData
        {
            /// remote address.
            const sockaddr* addr;

            /// address length.
            socklen_t addrlen;
        };

        /**
         * @brief build a connect operation.
         * @param fd socket file descriptor.
         * @param addr remote address.
         * @param addrlen remote address length.
         * @param handler handler to notify on completion.
         * @return initialized IoOperation.
         */
        static IoOperation makeConnect (int fd, const sockaddr* addr, socklen_t addrlen,
                                        CompletionHandler* handler) noexcept;

        /**
         * @brief payload for read / write.
         */
        struct RwData
        {
            /// buffer.
            void* buf;

            /// number of bytes to transfer.
            uint32_t len;
        };

        /**
         * @brief build a regular read operation.
         * @param fd file descriptor to read from.
         * @param buf destination buffer.
         * @param len number of bytes to read.
         * @param handler handler to notify on completion.
         * @param linked link this SQE to the next one (io_uring only).
         * @return initialized IoOperation.
         */
        static IoOperation makeRead (int fd, void* buf, size_t len, CompletionHandler* handler,
                                     bool linked = false) noexcept;

        /**
         * @brief build a regular write operation.
         * @param fd file descriptor to write to.
         * @param buf source buffer.
         * @param len number of bytes to write.
         * @param handler handler to notify on completion.
         * @param linked link this SQE to the next one (io_uring only).
         * @return initialized IoOperation.
         */
        static IoOperation makeWrite (int fd, const void* buf, size_t len, CompletionHandler* handler,
                                      bool linked = false) noexcept;

        /**
         * @brief payload for sendmsg / recvmsg.
         */
        struct MsgData
        {
            /// message header.
            msghdr* msg;

            /// reserved name area length.
            uint16_t namelen;

            /// reserved control area length.
            uint16_t controllen;

            /// provided buffer group.
            uint16_t group;
        };

        /**
         * @brief build a receive-message operation.
         * @param fd socket file descriptor.
         * @param msg message header.
         * @param flags recv flags.
         * @param handler handler to notify on completion.
         * @param linked link this SQE to the next one (io_uring only).
         * @return initialized IoOperation.
         */
        static IoOperation makeRecvmsg (int fd, msghdr* msg, int flags, CompletionHandler* handler,
                                        bool linked = false) noexcept;

        /**
         * @brief build a multishot receive-message operation.
         * @param fd socket file descriptor.
         * @param group provided buffer group, the selected buffer is only valid during the callback.
         * @param msg message header.
         * @param flags recv flags.
         * @param handler handler to notify on completion.
         * @return initialized IoOperation.
         */
        static IoOperation makeRecvmsgMulti (int fd, uint16_t group, msghdr* msg, int flags,
                                             CompletionHandler* handler) noexcept;

        /**
         * @brief build a send-message operation.
         * @param fd socket file descriptor.
         * @param msg message header.
         * @param flags send flags.
         * @param handler handler to notify on completion.
         * @param linked link this SQE to the next one (io_uring only).
         * @return initialized IoOperation.
         */
        static IoOperation makeSendmsg (int fd, const msghdr* msg, int flags, CompletionHandler* handler,
                                        bool linked = false) noexcept;

        /**
         * @brief build a zero-copy send-message operation.
         * @param fd socket file descriptor.
         * @param msg message header.
         * @param flags send flags.
         * @param handler handler to notify on completion.
         * @param linked link this SQE to the next one (io_uring only).
         * @return initialized IoOperation.
         */
        static IoOperation makeSendmsgZc (int fd, const msghdr* msg, int flags, CompletionHandler* handler,
                                          bool linked = false) noexcept;

        /**
         * @brief payload for recv / send.
         */
        struct StreamData
        {
            /// buffer.
            void* buf;

            /// number of bytes to transfer.
            uint32_t len;

            /// provided buffer group.
            uint16_t group;
        };

        /**
         * @brief build a receive operation.
         * @param fd socket file descriptor.
         * @param buf destination buffer.
         * @param len number of bytes to receive.
         * @param flags recv flags.
         * @param handler handler to notify on completion.
         * @param linked link this SQE to the next one (io_uring only).
         * @return initialized IoOperation.
         */
        static IoOperation makeRecv (int fd, void* buf, size_t len, int flags, CompletionHandler* handler,
                                     bool linked = false) noexcept;

        /**
         * @brief build a multishot receive operation.
         * @param fd socket file descriptor.
         * @param group provided buffer group, the selected buffer is only valid during the callback.
         * @param flags recv flags.
         * @param handler handler to notify on completion.
         * @return initialized IoOperation.
         */
        static IoOperation makeRecvMulti (int fd, uint16_t group, int flags, CompletionHandler* handler) noexcept;

        /**
         * @brief build a send operation.
         * @param fd socket file descriptor.
         * @param buf source buffer.
         * @param len number of bytes to send.
         * @param flags send flags.
         * @param handler handler to notify on completion.
         * @param linked link this SQE to the next one (io_uring only).
         * @return initialized IoOperation.
         */
        static IoOperation makeSend (int fd, const void* buf, size_t len, int flags, CompletionHandler* handler,
                                     bool linked = false) noexcept;

        /**
         * @brief build a zero-copy send operation.
         * @param fd socket file descriptor.
         * @param buf source buffer.
         * @param len number of bytes to send.
         * @param flags send flags.
         * @param handler handler to notify on completion.
         * @param linked link this SQE to the next one (io_uring only).
         * @return initialized IoOperation.
         */
        static IoOperation makeSendZc (int fd, const void* buf, size_t len, int flags, CompletionHandler* handler,
                                       bool linked = false) noexcept;

        union Data
        {
            PollData poll;
            AcceptData accept;
            ConnectData connect;
            RwData rw;
            MsgData msg;
            StreamData stream;
        };

#ifdef JOIN_HAS_IO_URING
        /// index of this operation in the proactor pending ops.
        uint32_t index = 0;

        /// zero-copy send result.
        int result = 0;
#else
        /// next operation queued on the same descriptor and direction.
        IoOperation* next = nullptr;
#endif

        /// operation code.
        uint8_t code = 0;

        /// operation state.
        std::atomic<State> state;

        /// state to restore when the current hold is released.
        State resume{State::Idle};

        /// link this SQE to the next one (next executes only if this succeeds, io_uring only).
        bool linked : 1 = false;

        /// rearm the operation after each completion.
        bool multishot : 1 = false;

        /// true while a multishot operation stays armed.
        bool more : 1 = false;

        /// accept / send / recv flags.
        int flags = 0;

        /// handler to dispatch to on completion.
        CompletionHandler* handler = nullptr;

        /// file descriptor.
        int fd = -1;

        /// operation code specific payload.
        Data data = {};

        /// buffer ring bound to this operation.
        IoRingBuffer* ring = nullptr;

        /// timer id.
        ssize_t timer = 0;
    };

    static_assert (offsetof (IoOperation, state) >= sizeof (uint32_t),
                   "state must survive the arena free list link stored in the first bytes of a released chunk");

    static_assert (sizeof (IoOperation) == 64, "an operation must fit in one cache line");
}

#endif
