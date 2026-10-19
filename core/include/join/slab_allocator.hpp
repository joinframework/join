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

#ifndef JOIN_CORE_SLAB_ALLOCATOR_HPP
#define JOIN_CORE_SLAB_ALLOCATOR_HPP

// libjoin.
#include <join/backoff.hpp>
#include <join/memory.hpp>
#include <join/utils.hpp>

// C++.
#include <algorithm>
#include <utility>
#include <atomic>
#include <array>
#include <tuple>

// C.
#include <cassert>
#include <cstdint>
#include <cstddef>

namespace join
{
    /**
     * @brief tagged index.
     */
    union alignas (uint64_t) SlabTaggedIndex
    {
        struct
        {
            /// free-list head index.
            uint32_t idx;

            /// generation counter.
            uint32_t gen;
        };

        /// raw value for atomic CAS.
        uint64_t raw;
    };

    /**
     * @brief basic chunk.
     */
    template <size_t Size>
    union alignas (64) BasicSlabChunk
    {
        static_assert (isPow2 (Size), "size must be a power of 2");
        static_assert (Size % alignof (std::max_align_t) == 0, "size must respects maximum alignment requirement");
        static_assert (Size >= sizeof (uint64_t), "size must respects minimum size for storing index");

        /// index of next free chunk.
        std::atomic_uint32_t _next;

        /// chunk data storage.
        uint8_t _data[Size];
    };

    /**
     * @brief segment header.
     */
    template <size_t Size>
    struct BasicSlabSegment
    {
        using Chunk = BasicSlabChunk<Size>;

        /// magic number for initialization detection.
        static constexpr uint64_t MAGIC = 0x9F7E3B2A8D5C4E1B;

        /// null index sentinel (empty free list).
        static constexpr uint32_t NULL_IDX = UINT32_MAX;

        /// initialization state atomic.
        alignas (64) std::atomic_uint64_t _magic;

        /// tagged head.
        alignas (64) std::atomic_uint64_t _head;

        /// flexible array of chunks.
        Chunk _chunks[];
    };

    /**
     * @brief free-list pool operating over a pre-existing memory region.
     */
    template <size_t Count, size_t Size>
    struct BasicSlabPool
    {
        static_assert (Count > 0, "count must be at least 1");
        static_assert (Count <= UINT32_MAX, "count exceeds tagged index capacity (~256 GB)");

        using Chunk = BasicSlabChunk<Size>;
        using Segment = BasicSlabSegment<Size>;

        /// total number of chunks per pool.
        static constexpr size_t _count = Count;

        /// size of each chunk in bytes.
        static constexpr size_t _size = Size;

        /// size in bytes between two consecutive chunks.
        static constexpr size_t _stride = sizeof (Chunk);

        /// total bytes required in the mapped region for this pool.
        static constexpr size_t _total = sizeof (Segment) + _stride * _count;

        /**
         * @brief initialize the pool over an existing memory region.
         * @param ptr pointer to the start of the region.
         */
        explicit BasicSlabPool (void* ptr) noexcept
        : _segment (static_cast<Segment*> (ptr))
        {
            uint64_t expected = 0;

            if (_segment->_magic.compare_exchange_strong (expected, 0xFFFFFFFFFFFFFFFF, std::memory_order_acq_rel))
            {
                releaseAll ();

                _segment->_magic.store (Segment::MAGIC, std::memory_order_release);
            }
            else
            {
                Backoff backoff;
                while (_segment->_magic.load (std::memory_order_acquire) != Segment::MAGIC)
                {
                    backoff ();  // LCOV_EXCL_LINE
                }
            }
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicSlabPool (const BasicSlabPool& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to copy.
         */
        BasicSlabPool& operator= (const BasicSlabPool& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        BasicSlabPool (BasicSlabPool&& other) noexcept
        : _segment (other._segment)
        {
            other._segment = nullptr;
        }

        /**
         * @brief move assignment operator.
         * @param other other object to move.
         * @return this.
         */
        BasicSlabPool& operator= (BasicSlabPool&& other) noexcept
        {
            _segment = other._segment;
            other._segment = nullptr;
            return *this;
        }

        /**
         * @brief destroy instance.
         */
        ~BasicSlabPool () = default;

        /**
         * @brief pop a chunk from the pool.
         * @return pointer to the chunk, or nullptr if exhausted.
         */
        void* pop () noexcept
        {
            if (JOIN_UNLIKELY (_segment == nullptr))
            {
                return nullptr;
            }

            SlabTaggedIndex cur, next;
            cur.raw = _segment->_head.load (std::memory_order_acquire);

            for (;;)
            {
                if (JOIN_UNLIKELY (cur.idx == Segment::NULL_IDX))
                {
                    return nullptr;
                }

                next.idx = _segment->_chunks[cur.idx]._next.load (std::memory_order_relaxed);
                next.gen = cur.gen + 1;

                if (JOIN_LIKELY (_segment->_head.compare_exchange_weak (cur.raw, next.raw, std::memory_order_acq_rel,
                                                                        std::memory_order_acquire)))
                {
                    return &_segment->_chunks[cur.idx];
                }
            }
        }

        /**
         * @brief push a chunk back to the pool.
         * @param p pointer to the chunk to return.
         */
        void push (void* p) noexcept
        {
            SlabTaggedIndex cur, next;
            next.idx = static_cast<uint32_t> (reinterpret_cast<Chunk*> (p) - _segment->_chunks);
            cur.raw = _segment->_head.load (std::memory_order_relaxed);

            for (;;)
            {
                _segment->_chunks[next.idx]._next.store (cur.idx, std::memory_order_relaxed);
                next.gen = cur.gen + 1;

                if (JOIN_LIKELY (_segment->_head.compare_exchange_weak (cur.raw, next.raw, std::memory_order_release,
                                                                        std::memory_order_relaxed)))
                {
                    return;
                }
            }
        }

        /**
         * @brief reserve all chunks at once.
         * @return true if all chunks were free and have been reserved, false otherwise.
         */
        bool reserveAll () noexcept
        {
            SlabTaggedIndex cur, next;
            cur.raw = _segment->_head.load (std::memory_order_acquire);

            uint32_t n = 0;

            for (uint32_t idx = cur.idx; idx != Segment::NULL_IDX;
                 idx = _segment->_chunks[idx]._next.load (std::memory_order_relaxed))
            {
                ++n;
            }

            if (n != _count)
            {
                return false;
            }

            next.idx = Segment::NULL_IDX;
            next.gen = cur.gen + 1;

            return _segment->_head.compare_exchange_strong (cur.raw, next.raw, std::memory_order_acq_rel,
                                                            std::memory_order_acquire);
        }

        /**
         * @brief release all chunks at once.
         */
        void releaseAll () noexcept
        {
            for (uint32_t i = 0; i < _count - 1; ++i)
            {
                _segment->_chunks[i]._next.store (i + 1, std::memory_order_relaxed);
            }
            _segment->_chunks[_count - 1]._next.store (Segment::NULL_IDX, std::memory_order_relaxed);

            SlabTaggedIndex cur, next;
            cur.raw = _segment->_head.load (std::memory_order_relaxed);
            next.idx = 0;
            next.gen = cur.gen + 1;

            _segment->_head.store (next.raw, std::memory_order_release);
        }

        /**
         * @brief check if the pointer belongs to this pool.
         * @param p pointer to check.
         * @return true if the pointer belongs to this pool, false otherwise.
         */
        bool owns (void* p) const noexcept
        {
            auto base = reinterpret_cast<std::uintptr_t> (_segment->_chunks);
            auto end = base + (_count * _stride);
            auto ptr = reinterpret_cast<std::uintptr_t> (p);
            return ((ptr >= base) && (ptr < end));
        }

        /**
         * @brief get the index of a chunk in the pool.
         * @param p pointer to the chunk (must be owned by this pool).
         * @return index of the chunk.
         */
        uint32_t getIndex (void* p) const noexcept
        {
            return static_cast<uint32_t> (reinterpret_cast<Chunk*> (p) - _segment->_chunks);
        }

        /**
         * @brief get the pointer to a chunk by index.
         * @param idx index of the chunk (must be in range).
         * @return pointer to the chunk.
         */
        void* getPtr (uint32_t idx) const noexcept
        {
            return &_segment->_chunks[idx];
        }

        /// pointer into the mapped region.
        Segment* _segment = nullptr;
    };

    /**
     * @brief slab allocator owning a backend and managing one or more pools.
     */
    template <typename Backend, size_t Count, size_t... Sizes>
    class BasicSlabAllocator
    {
        static_assert (sizeof...(Sizes) > 0, "slab allocator must have at least one pool size");
        static_assert (std::ranges::is_sorted (std::array{Sizes...}), "pool sizes must be provided in ascending order");

    public:
        /// total bytes required in the mapped region.
        static constexpr size_t _total = (BasicSlabPool<Count, Sizes>::_total + ...);

        /**
         * @brief create instance.
         * @param args arguments forwarded to the backend.
         */
        template <typename... Args>
        BasicSlabAllocator (Args&&... args)
        : _backend (_total, std::forward<Args> (args)...)
        , _pools (makePools (std::make_index_sequence<sizeof...(Sizes)>{}))
        {
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicSlabAllocator (const BasicSlabAllocator& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to copy.
         */
        BasicSlabAllocator& operator= (const BasicSlabAllocator& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        BasicSlabAllocator (BasicSlabAllocator&& other) noexcept
        : _backend (std::move (other._backend))
        , _pools (std::move (other._pools))
        {
        }

        /**
         * @brief move assignment operator.
         * @param other other object to move.
         * @return this.
         */
        BasicSlabAllocator& operator= (BasicSlabAllocator&& other) noexcept
        {
            _backend = std::move (other._backend);
            _pools = std::move (other._pools);
            return *this;
        }

        /**
         * @brief destroy instance.
         */
        ~BasicSlabAllocator () = default;

        /**
         * @brief allocate memory from the first pool that fits (promotes if exhausted).
         * @param size size of the allocation request in bytes.
         * @return pointer to the allocated memory, or nullptr on failure.
         */
        void* allocate (size_t size) noexcept
        {
            return allocateImplem<0> (size);
        }

        /**
         * @brief allocate memory from the exact pool that fits, without promotion.
         * @param size size of the allocation request in bytes.
         * @return pointer to the allocated memory, or nullptr on failure.
         */
        void* tryAllocate (size_t size) noexcept
        {
            return tryAllocateImplem<0> (size);
        }

        /**
         * @brief return memory to the appropriate pool.
         * @param p pointer to the memory to return.
         */
        void deallocate (void* p) noexcept
        {
            if (p == nullptr)
            {
                return;
            }
            deallocateImplem<0> (p);
        }

        /**
         * @brief check if the slab allocator still owns a memory region.
         * @return true if the slab allocator owns a memory region, false if it has been moved from.
         */
        bool hasBackend () const noexcept
        {
            return _backend.mapped ();
        }

        /**
         * @brief get the index of a chunk in the pool.
         * @param p pointer to the chunk (must be owned by the pool).
         * @return index of the chunk.
         */
        template <size_t I = 0>
        uint32_t getIndex (void* p) const noexcept
        {
            static_assert (I < sizeof...(Sizes), "pool index out of range");
            return std::get<I> (_pools).getIndex (p);
        }

        /**
         * @brief get the pointer to a chunk by index.
         * @param idx index of the chunk (must be in range).
         * @return pointer to the chunk.
         */
        template <size_t I = 0>
        void* getPtr (uint32_t idx) const noexcept
        {
            static_assert (I < sizeof...(Sizes), "pool index out of range");
            return std::get<I> (_pools).getPtr (idx);
        }

        /**
         * @brief reserve all chunks of a pool at once.
         * @return true if all chunks were free and have been reserved, false otherwise.
         */
        template <size_t I = 0>
        bool reserveAll () noexcept
        {
            static_assert (I < sizeof...(Sizes), "pool index out of range");
            return std::get<I> (_pools).reserveAll ();
        }

        /**
         * @brief release all chunks of a pool at once.
         */
        template <size_t I = 0>
        void releaseAll () noexcept
        {
            static_assert (I < sizeof...(Sizes), "pool index out of range");
            std::get<I> (_pools).releaseAll ();
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
        /**
         * @brief compute byte offsets of each pool in the flat memory region.
         * @return array of byte offsets.
         */
        static std::array<size_t, sizeof...(Sizes)> makeOffsets ()
        {
            size_t totals[] = {BasicSlabPool<Count, Sizes>::_total...};
            std::array<size_t, sizeof...(Sizes)> offsets{};
            for (size_t i = 1; i < sizeof...(Sizes); ++i)
            {
                offsets[i] = offsets[i - 1] + totals[i - 1];
            }
            return offsets;
        }

        /**
         * @brief construct each pool at its computed offset in the flat backend region.
         * @param index sequence used to expand the parameter pack with offsets.
         * @return tuple of initialized pools.
         */
        template <size_t... Is>
        std::tuple<BasicSlabPool<Count, Sizes>...> makePools (std::index_sequence<Is...>) noexcept
        {
            auto offsets = makeOffsets ();
            return std::tuple<BasicSlabPool<Count, Sizes>...>{
                BasicSlabPool<Count, Sizes> (static_cast<char*> (_backend.get ()) + offsets[Is])...};
        }

        /**
         * @brief recursive allocate (promotes to I+1 if exhausted).
         */
        template <size_t I>
        void* allocateImplem (size_t size) noexcept
        {
            if constexpr (I < sizeof...(Sizes))
            {
                auto& pool = std::get<I> (_pools);
                if (size <= pool._size)
                {
                    void* p = pool.pop ();
                    if (p != nullptr)
                    {
                        return p;
                    }
                }
                return allocateImplem<I + 1> (size);
            }
            else
            {
                return nullptr;
            }
        }

        /**
         * @brief recursive tryAllocate (no promotion: fails if exact pool is exhausted).
         */
        template <size_t I>
        void* tryAllocateImplem (size_t size) noexcept
        {
            if constexpr (I < sizeof...(Sizes))
            {
                auto& pool = std::get<I> (_pools);
                if (size <= pool._size)
                {
                    return pool.pop ();
                }
                return tryAllocateImplem<I + 1> (size);
            }
            else
            {
                return nullptr;
            }
        }

        /**
         * @brief recursive deallocate.
         */
        template <size_t I>
        void deallocateImplem (void* p) noexcept
        {
            if constexpr (I < sizeof...(Sizes))
            {
                auto& pool = std::get<I> (_pools);
                if (pool.owns (p))
                {
                    pool.push (p);
                    return;
                }
                deallocateImplem<I + 1> (p);
            }
        }

        /// memory backend.
        Backend _backend;

        /// tuple of pools over the backend region.
        std::tuple<BasicSlabPool<Count, Sizes>...> _pools;
    };
}

#endif
