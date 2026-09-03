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

#ifndef JOIN_CORE_IO_RING_BUFFER_HPP
#define JOIN_CORE_IO_RING_BUFFER_HPP

// libjoin.
#include <join/allocator.hpp>
#include <join/error.hpp>
#include <join/function.hpp>
#include <join/memory.hpp>
#include <join/utils.hpp>

// C++.
#include <atomic>

// Libraries.
#ifdef JOIN_HAS_IO_URING
#include <liburing.h>
#endif

namespace join
{
    /**
     * @brief pool of buffers provided to the I/O backend.
     */
    class IoRingBuffer
    {
    public:
#ifdef JOIN_HAS_IO_URING
        /**
         * @brief create instance.
         * @param ring io_uring instance to register with.
         */
        explicit IoRingBuffer (io_uring* ring) noexcept
        : _uring (ring)
        {
        }
#else
        /**
         * @brief create instance.
         */
        IoRingBuffer () noexcept = default;
#endif

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        IoRingBuffer (const IoRingBuffer& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to copy.
         * @return this.
         */
        IoRingBuffer& operator= (const IoRingBuffer& other) = delete;

        /**
         * @brief destroy instance.
         */
        ~IoRingBuffer ()
        {
            unregisterBuffer ();
        }

        /**
         * @brief provide the arena buffers to the backend.
         * @param group buffer group id.
         * @param arena arena owning the buffers, reserved until unregistered, must outlive this object.
         * @return 0 on success, -1 on failure.
         * @throw std::system_error if the descriptor ring cannot be mapped.
         */
        template <size_t Count, size_t Size>
        int registerBuffer (uint16_t group, LocalMem::Allocator<Count, Size>& arena);

        /**
         * @brief withdraw the buffers and give them back to the arena.
         * @return 0 on success, -1 on failure.
         */
        int unregisterBuffer () noexcept;

#ifndef JOIN_HAS_IO_URING
        /**
         * @brief take a free buffer.
         * @return buffer index, or -1 if none is available.
         */
        int select () noexcept;
#endif

        /**
         * @brief give a buffer back.
         * @param bid buffer index.
         */
        void recycle (uint16_t bid) noexcept;

        /**
         * @brief bind an operation to this buffer ring.
         */
        void bind () noexcept
        {
            _armed.fetch_add (1, std::memory_order_relaxed);
        }

        /**
         * @brief unbind an operation from this buffer ring.
         */
        void unbind () noexcept
        {
            _armed.fetch_sub (1, std::memory_order_release);
        }

        /**
         * @brief check whether operations are still bound to this buffer ring.
         * @return true if at least one operation is bound.
         */
        bool armed () const noexcept
        {
            return _armed.load (std::memory_order_acquire) != 0;
        }

        /**
         * @brief get the address of a buffer.
         * @param bid buffer index.
         * @return pointer to the buffer.
         */
        void* get (uint16_t bid) const noexcept
        {
            return _base + (bid * _size);
        }

        /**
         * @brief get the buffer group id.
         * @return buffer group id.
         */
        uint16_t group () const noexcept
        {
            return _group;
        }

        /**
         * @brief get the size of a buffer.
         * @return size of a buffer in bytes.
         */
        uint32_t size () const noexcept
        {
            return _size;
        }

        /**
         * @brief get the number of buffers.
         * @return number of buffers.
         */
        uint16_t count () const noexcept
        {
            return _count;
        }

    private:
        /// no free buffer sentinel.
        static constexpr uint16_t _none = UINT16_MAX;

#ifdef JOIN_HAS_IO_URING
        /// io_uring instance.
        io_uring* _uring = nullptr;

        /// mapping owning the descriptor ring.
        LocalMem _mapping;

        /// descriptor ring shared with the kernel.
        io_uring_buf_ring* _ring = nullptr;
#else
        /// index of the first free buffer.
        uint16_t _free = _none;
#endif

        /// base address of the buffer area.
        char* _base = nullptr;

        /// give the buffers back to the arena.
        Function<void ()> _release;

        /// size of one buffer.
        uint32_t _size = 0;

        /// number of buffers.
        uint16_t _count = 0;

        /// buffer group id.
        uint16_t _group = 0;

        /// number of operations bound to this buffer ring.
        std::atomic<uint32_t> _armed{0};
    };
}

#ifdef JOIN_HAS_IO_URING
#include "io_ring_buffer_uring.hpp"
#else
#include "io_ring_buffer_epoll.hpp"
#endif

#endif
