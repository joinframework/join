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

#ifndef JOIN_CORE_BUMP_ALLOCATOR_HPP
#define JOIN_CORE_BUMP_ALLOCATOR_HPP

// libjoin.
#include <join/memory.hpp>
#include <join/utils.hpp>

// C++.
#include <utility>
#include <atomic>

// C.
#include <cstdint>
#include <cstddef>

namespace join
{
    /**
     * @brief bump allocator owning a backend of fixed capacity.
     */
    template <typename Backend, size_t Capacity, typename SyncPolicy>
    class BasicBumpAllocator
    {
        static_assert (Capacity > 0, "capacity must be at least 1");

    public:
        /**
         * @brief create instance.
         * @param args arguments forwarded to the backend.
         * @throw std::system_error if the backend cannot be mapped.
         */
        template <typename... Args>
        BasicBumpAllocator (Args&&... args)
        : _backend (Capacity, std::forward<Args> (args)...)
        , _base (static_cast<char*> (_backend.get ()))
        {
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicBumpAllocator (const BasicBumpAllocator& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to copy.
         */
        BasicBumpAllocator& operator= (const BasicBumpAllocator& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        BasicBumpAllocator (BasicBumpAllocator&& other) noexcept
        : _backend (std::move (other._backend))
        , _base (other._base)
        , _cursor (std::move (other._cursor))
        {
            other._base = nullptr;
            other._cursor.store (0);
        }

        /**
         * @brief move assignment operator.
         * @param other other object to move.
         * @return this.
         */
        BasicBumpAllocator& operator= (BasicBumpAllocator&& other) noexcept
        {
            _backend = std::move (other._backend);
            _base = other._base;
            _cursor = std::move (other._cursor);

            other._base = nullptr;
            other._cursor.store (0);

            return *this;
        }

        /**
         * @brief destroy instance.
         */
        ~BasicBumpAllocator () = default;

        /**
         * @brief allocate memory at the cursor, aligned on the largest power of 2 dividing size, up to max_align_t.
         * @param size size of the allocation request in bytes.
         * @return pointer to the allocated memory, or nullptr on failure.
         */
        void* allocate (size_t size) noexcept
        {
            const size_t bits = size | alignof (std::max_align_t);
            return allocate (size, bits & (~bits + 1));
        }

        /**
         * @brief allocate memory at the cursor with an explicit alignment.
         * @param size size of the allocation request in bytes.
         * @param alignment alignment of the allocation, must be a power of 2.
         * @return pointer to the allocated memory, or nullptr on failure.
         */
        void* allocate (size_t size, size_t alignment) noexcept
        {
            if (JOIN_UNLIKELY ((_base == nullptr) || !isPow2 (alignment)))
            {
                return nullptr;
            }

            const uintptr_t base = reinterpret_cast<uintptr_t> (_base);
            uint64_t current = _cursor.load ();
            uint64_t offset;

            do
            {
                offset = ((base + current + alignment - 1) & ~(alignment - 1)) - base;

                if (JOIN_UNLIKELY ((offset > Capacity) || (size > Capacity - offset)))
                {
                    return nullptr;
                }
            }
            while (!_cursor.advance (current, offset + size));

            return _base + offset;
        }

        /**
         * @brief release all allocations at once.
         */
        void reset () noexcept
        {
            _cursor.store (0);
        }

        /**
         * @brief get the capacity.
         * @return capacity in bytes.
         */
        static constexpr size_t capacity () noexcept
        {
            return Capacity;
        }

        /**
         * @brief get the number of bytes consumed, alignment padding included.
         * @return consumed bytes.
         */
        size_t used () const noexcept
        {
            return _cursor.load ();
        }

        /**
         * @brief check if the bump allocator still owns a memory region.
         * @return true if the bump allocator owns a memory region, false if it has been moved from.
         */
        bool hasBackend () const noexcept
        {
            return _backend.mapped ();
        }

#ifdef JOIN_HAS_NUMA
        /**
         * @brief bind memory to a NUMA node.
         * @param numa NUMA node ID.
         * @return 0 on success, -1 on failure.
         */
        int mbind (int numa) const noexcept
        {
            return _backend.mbind (numa);
        }
#endif

        /**
         * @brief lock memory in RAM.
         * @return 0 on success, -1 on failure.
         */
        int mlock () const noexcept
        {
            return _backend.mlock ();
        }

    private:
        /// memory backend.
        Backend _backend;

        /// start of the backend region.
        char* _base = nullptr;

        /// allocation cursor.
        SyncPolicy _cursor;
    };

    /**
     * @brief bump sync policy using a lock-free cursor.
     */
    class BumpAtomic
    {
    public:
        /**
         * @brief default constructor.
         */
        BumpAtomic () noexcept = default;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        BumpAtomic (BumpAtomic&& other) noexcept
        : _value (other._value.load (std::memory_order_relaxed))
        {
        }

        /**
         * @brief move assignment operator.
         * @param other other object to move.
         * @return this.
         */
        BumpAtomic& operator= (BumpAtomic&& other) noexcept
        {
            _value.store (other._value.load (std::memory_order_relaxed), std::memory_order_relaxed);
            return *this;
        }

        /**
         * @brief load the cursor.
         * @return cursor value.
         */
        uint64_t load () const noexcept
        {
            return _value.load (std::memory_order_relaxed);
        }

        /**
         * @brief store the cursor.
         * @param value new cursor value.
         */
        void store (uint64_t value) noexcept
        {
            _value.store (value, std::memory_order_relaxed);
        }

        /**
         * @brief advance the cursor.
         * @param expected expected cursor value, updated on failure.
         * @param desired new cursor value.
         * @return true if the cursor was advanced, false if another thread moved it first.
         */
        bool advance (uint64_t& expected, uint64_t desired) noexcept
        {
            return _value.compare_exchange_weak (expected, desired, std::memory_order_relaxed);
        }

    private:
        /// cursor value.
        alignas (64) std::atomic_uint64_t _value{0};
    };

    /**
     * @brief bump sync policy using a plain cursor, single thread only.
     */
    class BumpPlain
    {
    public:
        /**
         * @brief load the cursor.
         * @return cursor value.
         */
        uint64_t load () const noexcept
        {
            return _value;
        }

        /**
         * @brief store the cursor.
         * @param value new cursor value.
         */
        void store (uint64_t value) noexcept
        {
            _value = value;
        }

        /**
         * @brief advance the cursor.
         * @param expected expected cursor value.
         * @param desired new cursor value.
         * @return always true.
         */
        bool advance ([[maybe_unused]] uint64_t& expected, uint64_t desired) noexcept
        {
            _value = desired;
            return true;
        }

    private:
        /// cursor value.
        uint64_t _value = 0;
    };
}

#endif
