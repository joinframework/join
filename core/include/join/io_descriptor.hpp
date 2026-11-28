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

#ifndef JOIN_CORE_IO_DESCRIPTOR_HPP
#define JOIN_CORE_IO_DESCRIPTOR_HPP

// libjoin.
#include <join/io_operation.hpp>
#include <join/reactor.hpp>

// C.
#include <cstdint>
#include <poll.h>

namespace join
{
#ifndef JOIN_HAS_IO_URING
    // forward declarations.
    class BasicProactor;
#endif

    /**
     * @brief proactor descriptor.
     */
    class IoDescriptor final
#ifndef JOIN_HAS_IO_URING
    : private EventHandler
#endif
    {
    public:
        /**
         * @brief create instance.
         * @param fd file descriptor, -1 if none.
         */
        explicit IoDescriptor (int fd = -1) noexcept
        : _handle (fd)
        {
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        IoDescriptor (const IoDescriptor& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to copy.
         * @return current object.
         */
        IoDescriptor& operator= (const IoDescriptor& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        IoDescriptor (IoDescriptor&& other) = delete;

        /**
         * @brief move assignment operator.
         * @param other other object to move.
         * @return current object.
         */
        IoDescriptor& operator= (IoDescriptor&& other) = delete;

#ifndef JOIN_HAS_IO_URING
        /**
         * @brief destroy instance.
         */
        ~IoDescriptor () noexcept;
#endif

        /**
         * @brief set the file descriptor.
         * @param fd file descriptor, -1 if none.
         */
        void handle (int fd) noexcept
        {
            _handle = fd;
        }

        /**
         * @brief get the file descriptor.
         * @return file descriptor, -1 if none.
         */
        int handle () const noexcept
        {
            return _handle;
        }

    private:
#ifndef JOIN_HAS_IO_URING
        /**
         * @brief dispatch the descriptor events.
         * @param fd file descriptor.
         * @param revents events reported by the reactor.
         */
        void onEvent (int fd, uint32_t revents) noexcept override;

        /**
         * @brief get the back of the queue of an operation.
         * @param op operation.
         * @return last queued operation.
         */
        IoOperation*& back (const IoOperation& op) noexcept
        {
            if (isWrite (op))
            {
                return _writeOps;
            }

            return _readOps;
        }

        /**
         * @brief queue an operation.
         * @param op operation to queue.
         * @return true if its queue was empty.
         */
        bool queue (IoOperation& op) noexcept
        {
            IoOperation*& last = back (op);

            if (last != nullptr)
            {
                op.next = last->next;
                last->next = &op;

                if (JOIN_LIKELY (static_cast<IoOperation::Opcode> (op.code) != IoOperation::Opcode::Connect))
                {
                    last = &op;
                }

                return false;
            }

            op.next = &op;
            last = &op;

            return true;
        }

        /**
         * @brief remove an operation from its queue.
         * @param op operation to remove.
         * @return true if the operation was queued.
         */
        bool dequeue (IoOperation& op) noexcept
        {
            if (op.next == nullptr)
            {
                return false;
            }

            IoOperation*& last = back (op);
            IoOperation* prev = last;

            while (prev->next != &op)
            {
                prev = prev->next;
            }

            if (prev == &op)
            {
                last = nullptr;
            }
            else
            {
                prev->next = op.next;

                if (last == &op)
                {
                    last = prev;
                }
            }

            op.next = nullptr;

            return true;
        }

        /**
         * @brief check if an operation requires EPOLLOUT.
         * @param op operation.
         * @return true for Connect, Write, SendMsg, Send and Poll without POLLIN.
         */
        static bool isWrite (const IoOperation& op) noexcept
        {
            switch (static_cast<IoOperation::Opcode> (op.code))
            {
                case IoOperation::Opcode::Poll:
                    return (op.data.poll.events & POLLIN) == 0;
                case IoOperation::Opcode::Connect:
                case IoOperation::Opcode::Write:
                case IoOperation::Opcode::SendMsg:
                case IoOperation::Opcode::SendMsgZc:
                case IoOperation::Opcode::Send:
                case IoOperation::Opcode::SendZc:
                    return true;
                default:
                    return false;
            }
        }

        /**
         * @brief insert the descriptor in a pending list.
         * @param head list head.
         */
        void link (IoDescriptor*& head) noexcept
        {
            _prev = nullptr;
            _next = head;

            if (head != nullptr)
            {
                head->_prev = this;
            }

            head = this;
        }

        /**
         * @brief remove the descriptor from a pending list.
         * @param head list head.
         */
        void unlink (IoDescriptor*& head) noexcept
        {
            if (_prev != nullptr)
            {
                _prev->_next = _next;
            }
            else
            {
                head = _next;
            }

            if (_next != nullptr)
            {
                _next->_prev = _prev;
            }
        }

        /**
         * @brief check if both queues are empty.
         * @return true if both queues are empty.
         */
        bool empty () const noexcept
        {
            return (_readOps == nullptr) && (_writeOps == nullptr);
        }
#endif

        /// file descriptor.
        int _handle = -1;

#ifndef JOIN_HAS_IO_URING
        /// serving proactor.
        BasicProactor* _proactor = nullptr;

        /// read queue.
        IoOperation* _readOps = nullptr;

        /// write queue.
        IoOperation* _writeOps = nullptr;

        /// previous pending descriptor.
        IoDescriptor* _prev = nullptr;

        /// next pending descriptor.
        IoDescriptor* _next = nullptr;

        /// friendship with proactor.
        friend class BasicProactor;
#endif
    };
}

#endif
