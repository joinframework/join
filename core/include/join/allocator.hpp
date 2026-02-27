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

#ifndef __JOIN_ALLOCATOR_HPP__
#define __JOIN_ALLOCATOR_HPP__

// libjoin.
#include <join/backoff.hpp>
#include <join/memory.hpp>
#include <join/utils.hpp>

// C++.
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
    union alignas (uint64_t) TaggedIndex
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
    union alignas (std::max_align_t) BasicChunk
    {
        static_assert ((Size & (Size - 1)) == 0, "size must be a power of 2");
        static_assert (Size % alignof (std::max_align_t) == 0, "size must respects maximum alignment requirement");
        static_assert (Size >= sizeof (uint64_t), "size must respects minimum size for storing index");

        /// index of next free chunk.
        uint32_t _next;

        /// chunk data storage.
        uint8_t _data[Size];
    };

    /**
     * @brief segment header.
     */
    template <size_t Size>
    struct BasicSegment
    {
        using Chunk = BasicChunk<Size>;

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
    struct BasicPool
    {
        static_assert (Count > 0, "count must be at least 1");
        static_assert (Count <= UINT32_MAX, "count exceeds tagged index capacity (~256 GB)");

        using Chunk   = BasicChunk<Size>;
        using Segment = BasicSegment<Size>;

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
        explicit BasicPool (void* ptr) noexcept
        : _segment (static_cast<Segment*> (ptr))
        {
            uint64_t expected = 0;

            if (_segment->_magic.compare_exchange_strong (expected, 0xFFFFFFFFFFFFFFFF, std::memory_order_acq_rel))
            {
                for (uint32_t i = 0; i < _count - 1; ++i)
                {
                    _segment->_chunks[i]._next = i + 1;
                }
                _segment->_chunks[_count - 1]._next = Segment::NULL_IDX;

                TaggedIndex head;
                head.idx = 0;
                head.gen = 0;
                _segment->_head.store (head.raw, std::memory_order_relaxed);

                _segment->_magic.store (Segment::MAGIC, std::memory_order_release);
            }
            else
            {
                Backoff backoff;
                while (_segment->_magic.load (std::memory_order_acquire) != Segment::MAGIC)
                {
                    backoff ();
                }
            }
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicPool (const BasicPool& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to copy.
         */
        BasicPool& operator= (const BasicPool& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        BasicPool (BasicPool&& other) noexcept
        : _segment (other._segment)
        {
            other._segment = nullptr;
        }

        /**
         * @brief move assignment operator.
         * @param other other object to move.
         * @return this.
         */
        BasicPool& operator= (BasicPool&& other) noexcept
        {
            _segment       = other._segment;
            other._segment = nullptr;
            return *this;
        }

        /**
         * @brief destroy instance.
         */
        ~BasicPool () = default;

        /**
         * @brief pop a chunk from the pool.
         * @return pointer to the chunk, or nullptr if exhausted.
         */
        void* pop () noexcept
        {
            TaggedIndex cur, next;
            cur.raw = _segment->_head.load (std::memory_order_acquire);

            for (;;)
            {
                if (JOIN_UNLIKELY (cur.idx == Segment::NULL_IDX))
                {
                    return nullptr;
                }

                next.idx = _segment->_chunks[cur.idx]._next;
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
            TaggedIndex cur, next;
            next.idx = static_cast<uint32_t> (reinterpret_cast<Chunk*> (p) - _segment->_chunks);
            cur.raw  = _segment->_head.load (std::memory_order_relaxed);

            for (;;)
            {
                _segment->_chunks[next.idx]._next = cur.idx;
                next.gen                          = cur.gen + 1;

                if (JOIN_LIKELY (_segment->_head.compare_exchange_weak (cur.raw, next.raw, std::memory_order_release,
                                                                        std::memory_order_relaxed)))
                {
                    return;
                }
            }
        }

        /**
         * @brief check if the pointer belongs to this pool.
         * @param p pointer to check.
         * @return true if the pointer belongs to this pool, false otherwise.
         */
        bool owns (void* p) const noexcept
        {
            auto base = reinterpret_cast<std::uintptr_t> (_segment->_chunks);
            auto end  = base + (_count * _stride);
            auto ptr  = reinterpret_cast<std::uintptr_t> (p);
            return ((ptr >= base) && (ptr < end));
        }

        /// pointer into the mapped region.
        Segment* _segment = nullptr;
    };

    /**
     * @brief total size computation.
     */
    template <size_t Count, size_t... Sizes>
    struct TotalSize;

    /**
     * @brief total size computation (recursive case).
     */
    template <size_t Count, size_t First, size_t... Rest>
    struct TotalSize<Count, First, Rest...>
    {
        static constexpr size_t value = BasicPool<Count, First>::_total + TotalSize<Count, Rest...>::value;
    };

    /**
     * @brief total size computation (base case).
     */
    template <size_t Count, size_t Last>
    struct TotalSize<Count, Last>
    {
        static constexpr size_t value = BasicPool<Count, Last>::_total;
    };

    /**
     * @brief is sequence sorted.
     */
    template <size_t... Sizes>
    struct Sorted;

    /**
     * @brief is sequence sorted (recursive case).
     */
    template <size_t First, size_t Second, size_t... Rest>
    struct Sorted<First, Second, Rest...>
    : std::integral_constant<bool, (First <= Second) && Sorted<Second, Rest...>::value>
    {
    };

    /**
     * @brief is sequence sorted (base case).
     */
    template <size_t Last>
    struct Sorted<Last> : std::true_type
    {
    };

    /**
     * @brief is sequence sorted (empty case).
     */
    template <>
    struct Sorted<> : std::true_type
    {
    };

    /**
     * @brief memory arena owning backend and managing one or more pools.
     */
    template <typename Backend, size_t Count, size_t... Sizes>
    class BasicArena
    {
        static_assert (sizeof...(Sizes) > 0, "arena must have at least one pool size");
        static_assert (Sorted<Sizes...>::value, "pool sizes must be provided in ascending order");

    public:
        /// total bytes required in the mapped region.
        static constexpr size_t _total = TotalSize<Count, Sizes...>::value;

        /**
         * @brief create instance.
         * @param args arguments forwarded to the backend.
         */
        template <typename... Args>
        BasicArena (Args&&... args)
        : _backend (_total, std::forward<Args> (args)...)
        , _pools (makePools (std::make_index_sequence<sizeof...(Sizes)>{}))
        {
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicArena (const BasicArena& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to copy.
         */
        BasicArena& operator= (const BasicArena& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        BasicArena (BasicArena&& other) noexcept
        : _backend (std::move (other._backend))
        , _pools (std::move (other._pools))
        {
        }

        /**
         * @brief move assignment operator.
         * @param other other object to move.
         * @return this.
         */
        BasicArena& operator= (BasicArena&& other) noexcept
        {
            _backend = std::move (other._backend);
            _pools   = std::move (other._pools);
            return *this;
        }

        /**
         * @brief destroy instance.
         */
        ~BasicArena () = default;

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
         * @brief bind memory to a NUMA node.
         * @param numa NUMA node ID.
         * @return 0 on success, -1 on failure.
         */
        int mbind (int numa) const noexcept
        {
            return _backend.mbind (numa);
        }

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
            size_t totals[] = {BasicPool<Count, Sizes>::_total...};
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
        std::tuple<BasicPool<Count, Sizes>...> makePools (std::index_sequence<Is...>) noexcept
        {
            auto offsets = makeOffsets ();
            return std::tuple<BasicPool<Count, Sizes>...>{
                BasicPool<Count, Sizes> (static_cast<char*> (_backend.get ()) + offsets[Is])...};
        }

        /**
         * @brief recursive allocate (promotes to I+1 if exhausted).
         */
        template <size_t I>
        typename std::enable_if<(I < sizeof...(Sizes)), void*>::type allocateImplem (size_t size) noexcept
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

        /**
         * @brief base case (no suitable or available pool).
         */
        template <size_t I>
        typename std::enable_if<(I >= sizeof...(Sizes)), void*>::type allocateImplem (size_t) noexcept
        {
            return nullptr;
        }

        /**
         * @brief recursive tryAllocate (no promotion: fails if exact pool is exhausted).
         */
        template <size_t I>
        typename std::enable_if<(I < sizeof...(Sizes)), void*>::type tryAllocateImplem (size_t size) noexcept
        {
            auto& pool = std::get<I> (_pools);
            if (size <= pool._size)
            {
                return pool.pop ();
            }
            return tryAllocateImplem<I + 1> (size);
        }

        /**
         * @brief base case (no pool fits the requested size).
         */
        template <size_t I>
        typename std::enable_if<(I >= sizeof...(Sizes)), void*>::type tryAllocateImplem (size_t) noexcept
        {
            return nullptr;
        }

        /**
         * @brief recursive deallocate.
         */
        template <size_t I>
        typename std::enable_if<(I < sizeof...(Sizes))>::type deallocateImplem (void* p) noexcept
        {
            auto& pool = std::get<I> (_pools);
            if (pool.owns (p))
            {
                pool.push (p);
                return;
            }
            deallocateImplem<I + 1> (p);
        }

        /**
         * @brief base case (pointer does not belong to any pool).
         */
        template <size_t I>
        typename std::enable_if<(I >= sizeof...(Sizes))>::type deallocateImplem (void*) noexcept
        {
        }

        /// memory backend.
        Backend _backend;

        /// tuple of pools over the backend region.
        std::tuple<BasicPool<Count, Sizes>...> _pools;
    };
}

#endif
