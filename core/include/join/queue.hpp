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

#ifndef JOIN_CORE_QUEUE_HPP
#define JOIN_CORE_QUEUE_HPP

// libjoin.
#include <join/backoff.hpp>
#include <join/memory.hpp>
#include <join/utils.hpp>

// C++.
#include <type_traits>
#include <stdexcept>
#include <algorithm>
#include <atomic>

// C.
#include <sys/types.h>

namespace join
{
    /**
     * @brief queue synchronization primitives.
     */
    struct QueueSync
    {
        /// magic number for initialization detection.
        static constexpr uint64_t MAGIC = 0x9F7E3B2A8D5C4E1B;

        /// initialization state atomic.
        alignas (64) std::atomic_uint64_t _magic;

        /// write position.
        alignas (64) std::atomic_uint64_t _head;

        /// read position.
        alignas (64) std::atomic_uint64_t _tail;

        /// total queue capacity (power of 2).
        alignas (64) uint64_t _capacity;

        /// bit mask for fast modulo.
        alignas (64) uint64_t _mask;
    };

    /**
     * @brief lightweight queue slot used by SPSC.
     */
    template <typename Type>
    struct QueueSlotLight
    {
        /// stored element data.
        Type data;

        /// padding to prevent false sharing.
        char _padding[(64 - (sizeof (Type) % 64)) % 64];
    };

    /**
     * @brief full queue slot used by MPSC/MPMC.
     */
    template <typename Type>
    struct QueueSlotFull
    {
        /// sequence number for synchronization.
        alignas (64) std::atomic_uint64_t _seq;

        /// stored element data.
        Type data;

        /// padding to prevent false sharing.
        char _padding[(64 - ((sizeof (std::atomic_uint64_t) + sizeof (Type)) % 64)) % 64];
    };

    /**
     * @brief queue memory segment.
     */
    template <typename Type, typename Slot>
    struct QueueSegment
    {
        /// synchronization primitives.
        alignas (64) QueueSync _sync;

        /// flexible array of queue slots.
        Slot _elements[];
    };

    // forward declarations.
    template <typename Type, typename Backend>
    struct Spsc;

    /**
     * @brief primary trait: all sync policies need sequence numbers by default.
     * @tparam SyncPolicy instantiated sync policy type.
     */
    template <typename SyncPolicy>
    struct needs_seq : std::true_type
    {
    };

    /**
     * @brief specialization for SPSC: no sequence number needed.
     */
    template <typename Type, typename Backend>
    struct needs_seq<Spsc<Type, Backend>> : std::false_type
    {
    };

    /**
     * @brief queue base class.
     */
    template <typename Type, typename Backend, typename SyncPolicy>
    class BasicQueue
    {
        static_assert (std::is_trivially_copyable<Type>::value, "type must be trivially copyable");
        static_assert (std::is_trivially_destructible<Type>::value, "type must be trivially destructible");

    public:
        using ValueType = Type;
        using Slot =
            typename std::conditional<needs_seq<SyncPolicy>::value, QueueSlotFull<Type>, QueueSlotLight<Type>>::type;
        using Segment = QueueSegment<Type, Slot>;

        /**
         * @brief create instance.
         * @param capacity queue capacity.
         * @param args backend args.
         */
        template <typename... Args>
        explicit BasicQueue (uint64_t capacity, Args&&... args)
        : _capacity (roundPow2 (capacity))
        , _elementSize (sizeof (Slot))
        , _totalSize (sizeof (QueueSync) + (_capacity * _elementSize))
        , _backend (_totalSize, std::forward<Args> (args)...)
        , _segment (static_cast<Segment*> (_backend.get ()))
        {
            uint64_t expected = 0;

            if (_segment->_sync._magic.compare_exchange_strong (expected, 0xFFFFFFFFFFFFFFFF,
                                                                std::memory_order_acq_rel))
            {
                _segment->_sync._head.store (0, std::memory_order_relaxed);
                _segment->_sync._tail.store (0, std::memory_order_relaxed);
                _segment->_sync._capacity = _capacity;
                _segment->_sync._mask     = _capacity - 1;

                initSlots<needs_seq<SyncPolicy>::value> ();

                _segment->_sync._magic.store (QueueSync::MAGIC, std::memory_order_release);
            }
            else
            {
                Backoff backoff;
                while (_segment->_sync._magic.load (std::memory_order_acquire) != QueueSync::MAGIC)
                {
                    backoff ();  // LCOV_EXCL_LINE
                }
            }

            if (_segment->_sync._capacity != _capacity)
            {
                throw std::runtime_error ("capacity mismatch");
            }
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicQueue (const BasicQueue& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to copy.
         * @return this.
         */
        BasicQueue& operator= (const BasicQueue& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        BasicQueue (BasicQueue&& other) = delete;

        /**
         * @brief move assignment operator.
         * @param other other object to move.
         * @return this.
         */
        BasicQueue& operator= (BasicQueue&& other) = delete;

        /**
         * @brief destroy queue instance.
         */
        ~BasicQueue () noexcept = default;

        /**
         * @brief try to push element into ring buffer.
         * @param element element to push.
         * @return 0 on success, -1 otherwise.
         */
        int tryPush (const Type& element) noexcept
        {
            return SyncPolicy::tryPush (_segment, element);
        }

        /**
         * @brief try to push multiple elements into the ring buffer.
         * @param elements pointer to the first element.
         * @param size number of elements to push.
         * @return number of elements successfully pushed, -1 otherwise.
         */
        ssize_t tryPush (const Type* elements, size_t size) noexcept
        {
            return SyncPolicy::tryPush (_segment, elements, size);
        }

        /**
         * @brief push element into the ring buffer.
         * @param element element to push.
         * @return 0 on success, -1 otherwise.
         */
        int push (const Type& element) noexcept
        {
            Backoff backoff;

            while (tryPush (element) == -1)
            {
                if (JOIN_UNLIKELY (lastError != Errc::TemporaryError))
                {
                    return -1;
                }

                backoff ();
            }

            return 0;
        }

        /**
         * @brief push multiple elements into the ring buffer.
         * @param elements pointer to the first element.
         * @param size number of elements to push.
         * @return 0 on success, -1 otherwise.
         */
        int push (const Type* elements, size_t size) noexcept
        {
            Backoff backoff;
            uint64_t pushed = 0;

            while (pushed < size)
            {
                ssize_t n = tryPush (elements + pushed, size - pushed);
                if (n == -1)
                {
                    if (JOIN_UNLIKELY (lastError != Errc::TemporaryError))
                    {
                        return -1;
                    }

                    backoff ();
                }
                else
                {
                    pushed += static_cast<uint64_t> (n);
                }
            }

            return 0;
        }

        /**
         * @brief try to pop element from the ring buffer.
         * @param element output element.
         * @return 0 on success, -1 otherwise.
         */
        int tryPop (Type& element) noexcept
        {
            return SyncPolicy::tryPop (_segment, element);
        }

        /**
         * @brief try to pop multiple elements from the ring buffer.
         * @param elements pointer to the output buffer.
         * @param size maximum number of elements to pop.
         * @return number of elements successfully popped, -1 otherwise.
         */
        ssize_t tryPop (Type* elements, size_t size) noexcept
        {
            return SyncPolicy::tryPop (_segment, elements, size);
        }

        /**
         * @brief pop element from the ring buffer.
         * @param element output element.
         * @return 0 on success, -1 otherwise.
         */
        int pop (Type& element) noexcept
        {
            Backoff backoff;

            while (tryPop (element) == -1)
            {
                if (JOIN_UNLIKELY (lastError != Errc::TemporaryError))
                {
                    return -1;
                }

                backoff ();
            }

            return 0;
        }

        /**
         * @brief pop multiple elements from the ring buffer.
         * @param elements pointer to the output buffer.
         * @param size number of elements to pop.
         * @return 0 on success, -1 otherwise.
         */
        int pop (Type* elements, size_t size) noexcept
        {
            Backoff backoff;
            uint64_t popped = 0;

            while (popped < size)
            {
                ssize_t n = tryPop (elements + popped, size - popped);
                if (n == -1)
                {
                    if (JOIN_UNLIKELY (lastError != Errc::TemporaryError))
                    {
                        return -1;
                    }

                    backoff ();
                }
                else
                {
                    popped += static_cast<uint64_t> (n);
                }
            }

            return 0;
        }

        /**
         * @brief get the number of pending elements for reading.
         * @return number of elements pending in the ring buffer.
         */
        uint64_t pending () const noexcept
        {
            if (JOIN_UNLIKELY (_segment == nullptr))
            {
                return 0;  // LCOV_EXCL_LINE
            }
            auto head = _segment->_sync._head.load (std::memory_order_acquire);
            auto tail = _segment->_sync._tail.load (std::memory_order_relaxed);
            return head - tail;
        }

        /**
         * @brief get the number of available slots for writing.
         * @return number of slots available in the ring buffer.
         */
        uint64_t available () const noexcept
        {
            if (JOIN_UNLIKELY (_segment == nullptr))
            {
                return 0;  // LCOV_EXCL_LINE
            }
            return _segment->_sync._capacity - pending ();
        }

        /**
         * @brief check if the ring buffer is full.
         * @return true if full, false otherwise.
         */
        bool full () const noexcept
        {
            if (JOIN_UNLIKELY (_segment == nullptr))
            {
                return false;  // LCOV_EXCL_LINE
            }
            return pending () == _segment->_sync._capacity;
        }

        /**
         * @brief check if the ring buffer is empty.
         * @return true if empty, false otherwise.
         */
        bool empty () const noexcept
        {
            if (JOIN_UNLIKELY (_segment == nullptr))
            {
                return true;  // LCOV_EXCL_LINE
            }
            return pending () == 0;
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
         * @brief round up to next power of 2.
         * @param v input value.
         * @return smallest power of 2 >= v.
         */
        static constexpr uint64_t roundPow2 (uint64_t v) noexcept
        {
            if (v == 0)
            {
                return 1;
            }
            v--;
            v |= v >> 1;
            v |= v >> 2;
            v |= v >> 4;
            v |= v >> 8;
            v |= v >> 16;
            v |= v >> 32;
            return v + 1;
        }

        /**
         * @brief initialize slots for policies that do not require sequence numbers (SPSC).
         */
        template <bool NeedsSeq, typename std::enable_if<!NeedsSeq>::type* = nullptr>
        void initSlots () noexcept
        {
            // nothing to initialize.
        }

        /**
         * @brief initialize sequence numbers for policies that require sequence numbers (MPSC/MPMC).
         */
        template <bool NeedsSeq, typename std::enable_if<NeedsSeq>::type* = nullptr>
        void initSlots () noexcept
        {
            for (uint64_t i = 0; i < _capacity; ++i)
            {
                _segment->_elements[i]._seq.store (i, std::memory_order_relaxed);
            }
        }

        /// memory segment capacity.
        const uint64_t _capacity = 0;

        /// memory segment element size.
        const uint64_t _elementSize = 0;

        /// total memory size.
        const uint64_t _totalSize = 0;

        /// memory segment backend.
        Backend _backend;

        /// shared memory segment.
        Segment* _segment = nullptr;
    };

    /**
     * @brief single producer single consumer ring buffer.
     */
    template <typename Type, typename Backend>
    struct Spsc
    {
        using Queue   = BasicQueue<Type, Backend, Spsc>;
        using Segment = QueueSegment<Type, QueueSlotLight<Type>>;

        /**
         * @brief try to push element into the ring buffer.
         * @param segment shared memory segment.
         * @param element element to push.
         * @return 0 on success, -1 otherwise.
         */
        static int tryPush (Segment* segment, const Type& element) noexcept
        {
            if (JOIN_UNLIKELY (segment == nullptr))
            {
                // LCOV_EXCL_START
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
                // LCOV_EXCL_STOP
            }

            auto& sync    = segment->_sync;
            uint64_t head = sync._head.load (std::memory_order_relaxed);
            uint64_t tail = sync._tail.load (std::memory_order_acquire);

            if (JOIN_UNLIKELY ((head - tail) == sync._capacity))
            {
                lastError = make_error_code (Errc::TemporaryError);
                return -1;
            }

            segment->_elements[head & sync._mask].data = element;
            sync._head.store (head + 1, std::memory_order_release);

            return 0;
        }

        /**
         * @brief try to push multiple elements into the ring buffer.
         * @param segment shared memory segment.
         * @param elements pointer to the first element.
         * @param size number of elements to push.
         * @return number of elements successfully pushed, -1 otherwise.
         */
        static ssize_t tryPush (Segment* segment, const Type* elements, size_t size) noexcept
        {
            if (JOIN_UNLIKELY (segment == nullptr || elements == nullptr || size == 0))
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            auto& sync       = segment->_sync;
            uint64_t head    = sync._head.load (std::memory_order_relaxed);
            uint64_t tail    = sync._tail.load (std::memory_order_acquire);
            uint64_t toWrite = std::min (static_cast<uint64_t> (size), sync._capacity - (head - tail));

            if (JOIN_UNLIKELY (toWrite == 0))
            {
                lastError = make_error_code (Errc::TemporaryError);
                return -1;
            }

            for (uint64_t i = 0; i < toWrite; ++i)
            {
                segment->_elements[(head + i) & sync._mask].data = elements[i];
            }

            sync._head.store (head + toWrite, std::memory_order_release);

            return static_cast<ssize_t> (toWrite);
        }

        /**
         * @brief try to pop element from the ring buffer.
         * @param segment shared memory segment.
         * @param element output element.
         * @return 0 on success, -1 otherwise.
         */
        static int tryPop (Segment* segment, Type& element) noexcept
        {
            if (JOIN_UNLIKELY (segment == nullptr))
            {
                // LCOV_EXCL_START
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
                // LCOV_EXCL_STOP
            }

            auto& sync    = segment->_sync;
            uint64_t tail = sync._tail.load (std::memory_order_relaxed);
            uint64_t head = sync._head.load (std::memory_order_acquire);

            if (JOIN_UNLIKELY (head == tail))
            {
                lastError = make_error_code (Errc::TemporaryError);
                return -1;
            }

            element = segment->_elements[tail & sync._mask].data;
            sync._tail.store (tail + 1, std::memory_order_release);

            return 0;
        }

        /**
         * @brief try to pop multiple elements from the ring buffer.
         * @param segment shared memory segment.
         * @param elements pointer to the output buffer.
         * @param size maximum number of elements to pop.
         * @return number of elements successfully popped, -1 otherwise.
         */
        static ssize_t tryPop (Segment* segment, Type* elements, size_t size) noexcept
        {
            if (JOIN_UNLIKELY (segment == nullptr || elements == nullptr || size == 0))
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            auto& sync      = segment->_sync;
            uint64_t tail   = sync._tail.load (std::memory_order_relaxed);
            uint64_t head   = sync._head.load (std::memory_order_acquire);
            uint64_t toRead = std::min (static_cast<uint64_t> (size), head - tail);

            if (JOIN_UNLIKELY (toRead == 0))
            {
                lastError = make_error_code (Errc::TemporaryError);
                return -1;
            }

            for (uint64_t i = 0; i < toRead; ++i)
            {
                elements[i] = segment->_elements[(tail + i) & sync._mask].data;
            }

            sync._tail.store (tail + toRead, std::memory_order_release);

            return static_cast<ssize_t> (toRead);
        }
    };

    /**
     * @brief multiple producer single consumer ring buffer.
     */
    template <typename Type, typename Backend>
    struct Mpsc
    {
        using Queue   = BasicQueue<Type, Backend, Mpsc>;
        using Segment = QueueSegment<Type, QueueSlotFull<Type>>;

        /**
         * @brief try to push element into the ring buffer.
         * @param segment shared memory segment.
         * @param element element to push.
         * @return 0 on success, -1 otherwise.
         */
        static int tryPush (Segment* segment, const Type& element) noexcept
        {
            if (JOIN_UNLIKELY (segment == nullptr))
            {
                // LCOV_EXCL_START
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
                // LCOV_EXCL_STOP
            }

            auto& sync    = segment->_sync;
            uint64_t head = sync._head.load (std::memory_order_relaxed);
            Backoff backoff;

            for (;;)
            {
                auto* slot   = &segment->_elements[head & sync._mask];
                uint64_t seq = slot->_seq.load (std::memory_order_acquire);

                if (seq == head)
                {
                    if (JOIN_LIKELY (sync._head.compare_exchange_weak (head, head + 1, std::memory_order_acquire,
                                                                       std::memory_order_relaxed)))
                    {
                        slot->data = element;
                        slot->_seq.store (head + 1, std::memory_order_release);
                        return 0;
                    }
                }
                else if (JOIN_UNLIKELY (seq < head))
                {
                    lastError = make_error_code (Errc::TemporaryError);
                    return -1;
                }
                else
                {
                    backoff ();
                    head = sync._head.load (std::memory_order_relaxed);
                }
            }
        }

        /**
         * @brief try to push multiple elements into the ring buffer.
         * @param segment shared memory segment.
         * @param elements pointer to the first element.
         * @param size number of elements to push.
         * @return number of elements successfully pushed, -1 otherwise.
         */
        static ssize_t tryPush (Segment* segment, const Type* elements, size_t size) noexcept
        {
            if (JOIN_UNLIKELY (segment == nullptr || elements == nullptr || size == 0))
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            Backoff backoff;
            auto& sync    = segment->_sync;
            uint64_t head = sync._head.load (std::memory_order_relaxed);

            for (;;)
            {
                uint64_t tail    = sync._tail.load (std::memory_order_acquire);
                uint64_t toWrite = std::min (static_cast<uint64_t> (size), sync._capacity - (head - tail));

                if (JOIN_UNLIKELY (toWrite == 0))
                {
                    lastError = make_error_code (Errc::TemporaryError);
                    return -1;
                }

                if (JOIN_LIKELY (sync._head.compare_exchange_weak (head, head + toWrite, std::memory_order_acquire,
                                                                   std::memory_order_relaxed)))
                {
                    for (uint64_t i = 0; i < toWrite; ++i)
                    {
                        auto* slot = &segment->_elements[(head + i) & sync._mask];
                        slot->data = elements[i];
                        slot->_seq.store (head + i + 1, std::memory_order_release);
                    }

                    return static_cast<ssize_t> (toWrite);
                }

                backoff ();
            }
        }

        /**
         * @brief try to pop element from the ring buffer.
         * @param segment shared memory segment.
         * @param element output element.
         * @return 0 on success, -1 otherwise.
         */
        static int tryPop (Segment* segment, Type& element) noexcept
        {
            if (JOIN_UNLIKELY (segment == nullptr))
            {
                // LCOV_EXCL_START
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
                // LCOV_EXCL_STOP
            }

            auto& sync    = segment->_sync;
            uint64_t tail = sync._tail.load (std::memory_order_relaxed);
            auto* slot    = &segment->_elements[tail & sync._mask];
            uint64_t seq  = slot->_seq.load (std::memory_order_acquire);

            if (JOIN_UNLIKELY (seq != tail + 1))
            {
                lastError = make_error_code (Errc::TemporaryError);
                return -1;
            }

            element = slot->data;
            slot->_seq.store (tail + sync._capacity, std::memory_order_release);
            sync._tail.store (tail + 1, std::memory_order_release);

            return 0;
        }

        /**
         * @brief try to pop multiple elements from the ring buffer.
         * @param segment shared memory segment.
         * @param elements pointer to the output buffer.
         * @param size maximum number of elements to pop.
         * @return number of elements successfully popped, -1 otherwise.
         */
        static ssize_t tryPop (Segment* segment, Type* elements, size_t size) noexcept
        {
            if (JOIN_UNLIKELY (segment == nullptr || elements == nullptr || size == 0))
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            auto& sync      = segment->_sync;
            uint64_t tail   = sync._tail.load (std::memory_order_relaxed);
            uint64_t head   = sync._head.load (std::memory_order_acquire);
            uint64_t toRead = std::min (static_cast<uint64_t> (size), head - tail);
            uint64_t popped = 0;

            for (uint64_t i = 0; i < toRead; ++i)
            {
                auto* slot = &segment->_elements[(tail + i) & sync._mask];

                if (JOIN_UNLIKELY (slot->_seq.load (std::memory_order_acquire) != tail + i + 1))
                {
                    break;
                }

                elements[i] = slot->data;
                slot->_seq.store (tail + i + sync._capacity, std::memory_order_release);
                ++popped;
            }

            if (JOIN_LIKELY (popped > 0))
            {
                sync._tail.store (tail + popped, std::memory_order_release);
                return static_cast<ssize_t> (popped);
            }

            lastError = make_error_code (Errc::TemporaryError);
            return -1;
        }
    };

    /**
     * @brief multiple producer multiple consumer ring buffer.
     */
    template <typename Type, typename Backend>
    struct Mpmc
    {
        using Queue   = BasicQueue<Type, Backend, Mpmc>;
        using Segment = QueueSegment<Type, QueueSlotFull<Type>>;

        /**
         * @brief try to push element into the ring buffer.
         * @param segment shared memory segment.
         * @param element element to push.
         * @return 0 on success, -1 otherwise.
         */
        static int tryPush (Segment* segment, const Type& element) noexcept
        {
            return Mpsc<Type, Backend>::tryPush (segment, element);
        }

        /**
         * @brief try to push multiple elements into the ring buffer.
         * @param segment shared memory segment.
         * @param elements pointer to the first element.
         * @param size number of elements to push.
         * @return number of elements successfully pushed, -1 otherwise.
         */
        static ssize_t tryPush (Segment* segment, const Type* elements, size_t size) noexcept
        {
            return Mpsc<Type, Backend>::tryPush (segment, elements, size);
        }

        /**
         * @brief try to pop element from the ring buffer.
         * @param segment shared memory segment.
         * @param element output element.
         * @return 0 on success, -1 otherwise.
         */
        static int tryPop (Segment* segment, Type& element) noexcept
        {
            if (JOIN_UNLIKELY (segment == nullptr))
            {
                // LCOV_EXCL_START
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
                // LCOV_EXCL_STOP
            }

            auto& sync    = segment->_sync;
            uint64_t tail = sync._tail.load (std::memory_order_relaxed);
            Backoff backoff;

            for (;;)
            {
                auto* slot   = &segment->_elements[tail & sync._mask];
                uint64_t seq = slot->_seq.load (std::memory_order_acquire);

                if (seq == (tail + 1))
                {
                    if (JOIN_LIKELY (sync._tail.compare_exchange_weak (tail, tail + 1, std::memory_order_acquire,
                                                                       std::memory_order_relaxed)))
                    {
                        element = slot->data;
                        slot->_seq.store (tail + sync._capacity, std::memory_order_release);
                        return 0;
                    }
                }
                else if (JOIN_UNLIKELY (seq < (tail + 1)))
                {
                    lastError = make_error_code (Errc::TemporaryError);
                    return -1;
                }
                else
                {
                    backoff ();
                    tail = sync._tail.load (std::memory_order_relaxed);
                }
            }
        }

        /**
         * @brief try to pop multiple elements from the ring buffer.
         * @param segment shared memory segment.
         * @param elements pointer to the output buffer.
         * @param size maximum number of elements to pop.
         * @return number of elements successfully popped, -1 otherwise.
         */
        static ssize_t tryPop (Segment* segment, Type* elements, size_t size) noexcept
        {
            if (JOIN_UNLIKELY (segment == nullptr || elements == nullptr || size == 0))
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            Backoff backoff;
            auto& sync    = segment->_sync;
            uint64_t tail = sync._tail.load (std::memory_order_relaxed);

            for (;;)
            {
                uint64_t head   = sync._head.load (std::memory_order_acquire);
                uint64_t toRead = std::min (static_cast<uint64_t> (size), head - tail);

                if (JOIN_UNLIKELY (toRead == 0))
                {
                    lastError = make_error_code (Errc::TemporaryError);
                    return -1;
                }

                uint64_t ready = 0;
                for (; ready < toRead; ++ready)
                {
                    if (JOIN_UNLIKELY (segment->_elements[(tail + ready) & sync._mask]._seq.load (
                                           std::memory_order_acquire) != tail + ready + 1))
                    {
                        break;
                    }
                }

                if (JOIN_UNLIKELY (ready == 0))
                {
                    lastError = make_error_code (Errc::TemporaryError);
                    return -1;
                }

                if (JOIN_LIKELY (sync._tail.compare_exchange_weak (tail, tail + ready, std::memory_order_acquire,
                                                                   std::memory_order_relaxed)))
                {
                    for (uint64_t i = 0; i < ready; ++i)
                    {
                        auto* slot  = &segment->_elements[(tail + i) & sync._mask];
                        elements[i] = slot->data;
                        slot->_seq.store (tail + i + sync._capacity, std::memory_order_release);
                    }

                    return static_cast<ssize_t> (ready);
                }

                backoff ();
            }
        }
    };

    /**
     * @brief helper to bind backend and synchronization policy.
     */
    template <typename Backend, template <typename, typename> class SyncPolicy>
    struct SyncBinding
    {
        /// queue type alias combining backend and policy.
        template <typename Type>
        using Queue = BasicQueue<Type, Backend, SyncPolicy<Type, Backend>>;
    };
}

#endif
