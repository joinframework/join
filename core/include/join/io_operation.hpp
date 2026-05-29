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

// C.
#include <sys/socket.h>
#include <cstdint>

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
            Idle,       /**< operation is not in flight. */
            Submitted,  /**< operation has been submitted and is awaiting completion. */
            Cancelling, /**< operation has been canceled and is awaiting completion. */
        };

        /**
         * @brief operation code.
         */
        enum class Opcode : uint8_t
        {
            Accept,     /**< accept an incoming connection. */
            Connect,    /**< initiate an outgoing connection. */
            Read,       /**< read from a file descriptor. */
            Write,      /**< write to a file descriptor. */
            ReadFixed,  /**< read using a registered buffer. */
            WriteFixed, /**< write using a registered buffer. */
            RecvMsg,    /**< receive a message with ancillary data. */
            SendMsg,    /**< send a message with ancillary data. */
            Recv,       /**< receive data from a socket. */
            Send,       /**< send data on a socket. */
        };

        /**
         * @brief payload for accept.
         */
        struct AcceptData
        {
            /// file descriptor.
            int fd;

            /// peer address.
            sockaddr* addr;

            /// peer address length.
            socklen_t* addrlen;

            /// accept flags.
            int flags;
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
         * @brief payload for connect.
         */
        struct ConnectData
        {
            /// file descriptor.
            int fd;

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
         * @brief payload for read / read fixed / write / write fixed.
         */
        struct RwData
        {
            /// file descriptor.
            int fd;

            /// buffer.
            void* buf;

            /// number of bytes to transfer.
            unsigned long len;

            /// registered buffer index (ignored with reactor backend).
            uint16_t index;

            /// use registered buffer (ignored with reactor backend).
            bool fixed;
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
        static IoOperation makeRead (int fd, void* buf, uint32_t len, CompletionHandler* handler,
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
        static IoOperation makeWrite (int fd, const void* buf, uint32_t len, CompletionHandler* handler,
                                      bool linked = false) noexcept;

        /**
         * @brief build a fixed-buffer read operation.
         * @param fd file descriptor to read from.
         * @param buf destination buffer.
         * @param len number of bytes to read.
         * @param index registered buffer index.
         * @param handler handler to notify on completion.
         * @param linked link this SQE to the next one (io_uring only).
         * @return initialized IoOperation.
         */
        static IoOperation makeReadFixed (int fd, void* buf, uint32_t len, uint16_t index, CompletionHandler* handler,
                                          bool linked = false) noexcept;

        /**
         * @brief build a fixed-buffer write operation.
         * @param fd file descriptor to write to.
         * @param buf source buffer.
         * @param len number of bytes to write.
         * @param index registered buffer index.
         * @param handler handler to notify on completion.
         * @param linked link this SQE to the next one (io_uring only).
         * @return initialized IoOperation.
         */
        static IoOperation makeWriteFixed (int fd, const void* buf, uint32_t len, uint16_t index,
                                           CompletionHandler* handler, bool linked = false) noexcept;

        /**
         * @brief payload for sendmsg / recvmsg.
         */
        struct MsgData
        {
            /// file descriptor.
            int fd;

            /// message header.
            msghdr* msg;

            /// sendmsg / recvmsg flags.
            int flags;
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
         * @brief payload for recv / send.
         */
        struct StreamData
        {
            /// file descriptor.
            int fd;

            /// buffer.
            void* buf;

            /// number of bytes to transfer.
            unsigned long len;

            /// send / recv flags.
            int flags;
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
        static IoOperation makeRecv (int fd, void* buf, uint32_t len, int flags, CompletionHandler* handler,
                                     bool linked = false) noexcept;

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
        static IoOperation makeSend (int fd, const void* buf, uint32_t len, int flags, CompletionHandler* handler,
                                     bool linked = false) noexcept;

        union Data
        {
            AcceptData accept;
            ConnectData connect;
            RwData rw;
            MsgData msg;
            StreamData stream;
        };

        /**
         * @brief return the file descriptor associated with this operation.
         * @return file descriptor, or -1 if opcode is unknown.
         */
        int fd () const noexcept;

        /// operation code.
        uint8_t code = 0;

        /// operation state.
        State state = State::Idle;

        /// index of this operation in the proactor pending ops (io_uring only).
        uint32_t index = 0;

        /// link this SQE to the next one (next executes only if this succeeds, io_uring only).
        bool linked = false;

        /// handler to dispatch to on completion.
        CompletionHandler* handler = nullptr;

        /// operation code specific payload.
        Data data = {};
    };
}

#endif
