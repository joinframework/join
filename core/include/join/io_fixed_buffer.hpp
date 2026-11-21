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

#ifndef JOIN_CORE_IO_FIXED_BUFFER_HPP
#define JOIN_CORE_IO_FIXED_BUFFER_HPP

// libjoin.
#include <join/slab_allocator.hpp>
#include <join/memory.hpp>
#include <join/utils.hpp>
#include <join/error.hpp>

// Libraries.
#include <liburing.h>

// C++.
#include <system_error>
#include <array>

// C.
#include <sys/uio.h>
#include <cstdint>
#include <cstddef>

namespace join
{
    /**
     * @brief empty registered I/O buffers.
     */
    struct NoFixedBuffer
    {
    };

    /**
     * @brief registered I/O buffers.
     */
    template <size_t Count, size_t... Sizes>
    class IoFixedBuffer final
    {
    public:
        /// arena type.
        using Arena = LocalMem::SlabAllocator<Count, Sizes...>;

        /// first pool header size.
        static constexpr size_t header = sizeof (BasicSlabSegment<std::array{Sizes...}[0]>);

        /// registered area size.
        static constexpr size_t length = Arena::total - header;

        /// io_uring registration limit.
        static_assert (length <= (size_t (1) << 30), "buffer area exceeds the 1 GiB registration limit");

        /**
         * @brief create instance.
         * @throw std::system_error on mapping failure.
         */
        IoFixedBuffer () = default;

        /**
         * @brief create instance and attach it.
         * @param ring io_uring instance.
         * @throw std::system_error on mapping or registration failure.
         */
        explicit IoFixedBuffer (io_uring* ring)
        {
            if (attach (ring) == -1)
            {
                throw std::system_error (lastError, "io_uring_register_buffers failed");
            }
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        IoFixedBuffer (const IoFixedBuffer& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to copy.
         * @return current object.
         */
        IoFixedBuffer& operator= (const IoFixedBuffer& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        IoFixedBuffer (IoFixedBuffer&& other) = delete;

        /**
         * @brief move assignment operator.
         * @param other other object to move.
         * @return current object.
         */
        IoFixedBuffer& operator= (IoFixedBuffer&& other) = delete;

        /**
         * @brief destroy instance.
         */
        ~IoFixedBuffer ()
        {
            detach ();
        }

        /**
         * @brief acquire a buffer.
         * @param size minimum size.
         * @return buffer, nullptr if none.
         */
        char* acquire (size_t size) noexcept
        {
            char* buf = static_cast<char*> (_arena.allocate (size));
            if (JOIN_UNLIKELY (buf == nullptr))
            {
                lastError = make_error_code (Errc::OutOfMemory);
            }
            return buf;
        }

        /**
         * @brief release a buffer.
         * @param buf buffer.
         */
        void release (char* buf) noexcept
        {
            _arena.deallocate (buf);
        }

        /**
         * @brief check if a range is registered.
         * @param ptr range start.
         * @param size range size.
         * @return true if registered.
         */
        bool owns (const void* ptr, size_t size) const noexcept
        {
            auto base = reinterpret_cast<std::uintptr_t> (data ());
            auto addr = reinterpret_cast<std::uintptr_t> (ptr);
            return (addr >= base) && ((addr - base) <= length) && (size <= (length - (addr - base)));
        }

        /**
         * @brief get registered area.
         * @return registered area.
         */
        void* data () const noexcept
        {
            return _arena.getPtr (0);
        }

    private:
        /**
         * @brief register buffers.
         * @param ring io_uring instance.
         * @return 0 on success, -1 on failure.
         */
        int attach (io_uring* ring) noexcept
        {
            if (JOIN_UNLIKELY (_uring != nullptr))
            {
                // LCOV_EXCL_START
                lastError = make_error_code (Errc::InUse);
                return -1;
                // LCOV_EXCL_STOP
            }

            iovec iov = {data (), length};

            int ret = io_uring_register_buffers (ring, &iov, 1);
            if (JOIN_UNLIKELY (ret < 0))
            {
                lastError = std::error_code (-ret, std::system_category ());
                return -1;
            }

            _uring = ring;
            return 0;
        }

        /**
         * @brief unregister buffers.
         * @return 0 on success, -1 on failure.
         */
        int detach () noexcept
        {
            if (_uring == nullptr)
            {
                return 0;
            }

            int ret = io_uring_unregister_buffers (_uring);
            if (JOIN_UNLIKELY (ret < 0))
            {
                // LCOV_EXCL_START
                lastError = std::error_code (-ret, std::system_category ());
                return -1;
                // LCOV_EXCL_STOP
            }

            _uring = nullptr;
            return 0;
        }

        /// buffer arena.
        Arena _arena;

        /// io_uring instance.
        io_uring* _uring = nullptr;

        /// friendship with proactor.
        template <typename>
        friend class BasicProactor;
    };
}

#endif
