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

#ifndef __JOIN_QUEUE_HPP__
#define __JOIN_QUEUE_HPP__

// libjoin.
#include <join/backoff.hpp>
#include <join/memory.hpp>
#include <join/utils.hpp>

// C++.
#include <type_traits>
#include <stdexcept>
#include <atomic>

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
     * @brief queue slot.
     */
    template <typename Type>
    struct QueueSlot
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
    template <typename Type>
    struct QueueSegment
    {
        /// synchronization primitives.
        alignas (64) QueueSync _sync;

        /// flexible array of queue slots.
        QueueSlot <Type> _elements[];
    };

    /**
     * @brief queue base class.
     */
    template <typename Type, typename Backend, typename SyncPolicy>
    class BasicQueue
    {
        static_assert (std::is_trivially_copyable <Type>::value, "type must be trivially copyable");
        static_assert (std::is_trivially_destructible <Type>::value, "type must be trivially destructible");

    public:
        using ValueType = Type;

        /**
         * @brief create instance.
         * @param capacity queue capacity.
         * @param args backend args.
         */
        template <typename... Args>
        BasicQueue (uint64_t capacity, Args&&... args)
        : _capacity (roundPow2 (capacity))
        , _elementSize (sizeof (QueueSlot <Type>))
        , _totalSize (sizeof (QueueSync) + (_capacity * _elementSize))
        , _backend (_totalSize, std::forward <Args> (args)...)
        , _segment (static_cast <QueueSegment <Type>*> (_backend.get ()))
        {
            uint64_t expected = 0;

            if (_segment->_sync._magic.compare_exchange_strong (expected, 0xFFFFFFFFFFFFFFFF, std::memory_order_acq_rel))
            {
                _segment->_sync._head.store (0, std::memory_order_relaxed);
                _segment->_sync._tail.store (0, std::memory_order_relaxed);
                _segment->_sync._capacity = _capacity;
                _segment->_sync._mask = _capacity - 1;

                for (uint64_t i = 0; i < _capacity; ++i)
                {
                    _segment->_elements[i]._seq.store (i, std::memory_order_relaxed);
                }

                _segment->_sync._magic.store (QueueSync::MAGIC, std::memory_order_release);
            }
            else
            {
                while (_segment->_sync._magic.load (std::memory_order_acquire) != QueueSync::MAGIC)
                {
                    std::this_thread::yield ();
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
        BasicQueue (BasicQueue&& other) noexcept
        : _capacity (other._capacity)
        , _elementSize (other._elementSize)
        , _totalSize (other._totalSize)
        , _backend (std::move (other._backend))
        , _policy (std::move (other._policy))
        , _segment (other._segment)
        {
            other._capacity = 0;
            other._elementSize = 0;
            other._totalSize = 0;
            other._segment = nullptr;
        }

        /**
         * @brief move assignment operator.
         * @param other other object to move.
         * @return this.
         */
        BasicQueue& operator= (BasicQueue&& other) noexcept
        {
            _capacity = other._capacity;
            _elementSize = other._elementSize;
            _totalSize = other._totalSize;
            _backend = std::move (other._backend);
            _policy = std::move (other._policy);
            _segment = other._segment;

            other._capacity = 0;
            other._elementSize = 0;
            other._totalSize = 0;
            other._segment = nullptr;

            return *this;
        }

        /**
         * @brief destroy queue instance.
         */
        ~BasicQueue () = default;

        /**
         * @brief try to push element into ring buffer.
         * @param element element to push.
         * @return 0 on success, -1 otherwise.
         */
        int tryPush (const Type& element) noexcept
        {
            return _policy.tryPush (_segment, element);
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
         * @brief try to pop element from the ring buffer.
         * @param element output element.
         * @return 0 on success, -1 otherwise.
         */
        int tryPop (Type& element) noexcept
        {
            return _policy.tryPop (_segment, element);
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
         * @brief get the number of pending elements for reading.
         * @return number of elements pending in the ring buffer.
         */
        uint64_t pending () const noexcept
        {
            if (_segment == nullptr)
            {
                return 0;
            }
            auto head = _segment->_sync._head.load (std::memory_order_acquire);
            auto tail = _segment->_sync._tail.load (std::memory_order_acquire);
            return head - tail;
        }

        /**
         * @brief get the number of available slots for writing.
         * @return number of slots available in the ring buffer.
         */
        uint64_t available () const noexcept
        {
            if (_segment == nullptr)
            {
                return 0;
            }
            return _segment->_sync._capacity - pending ();
        }

        /**
         * @brief check if the ring buffer is full.
         * @return true if full, false otherwise.
         */
        bool full () const noexcept
        {
            if (_segment == nullptr)
            {
                return false;
            }
            return pending () == _segment->_sync._capacity;
        }

        /**
         * @brief check if the ring buffer is empty.
         * @return true if empty, false otherwise.
         */
        bool empty () const noexcept
        {
            if (_segment == nullptr)
            {
                return true;
            }
            return pending () == 0;
        }

    protected:
        /**
         * @brief round up to next power of 2.
         * @param v input value.
         * @return smallest power of 2 >= v.
         */
        static uint64_t roundPow2 (uint64_t v) noexcept
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

        /// memory segment capacity.
        uint64_t _capacity = 0;

        /// memory segment element size.
        uint64_t _elementSize = 0;

        /// total memory size.
        uint64_t _totalSize = 0;

        /// memory segment backend.
        Backend _backend;

        /// memory segment sync policy.
        SyncPolicy _policy;

        /// shared memory segment.
        QueueSegment <Type>* _segment = nullptr;
    };

    /**
     * @brief single producer single consumer ring buffer.
     */
    template <typename Type, typename Backend>
    struct Spsc
    {
        using Queue = BasicQueue <Type, Backend, Spsc>;

        /**
         * @brief try to push element into the ring buffer.
         * @param segment shared memory segment.
         * @param element element to push.
         * @return 0 on success, -1 otherwise.
         */
        static int tryPush (QueueSegment <Type>* segment, const Type& element) noexcept
        {
            if (JOIN_UNLIKELY (segment == nullptr))
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            auto& sync = segment->_sync;
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
         * @brief try to pop element from the ring buffer.
         * @param segment shared memory segment.
         * @param element output element.
         * @return 0 on success, -1 otherwise.
         */
        static int tryPop (QueueSegment <Type>* segment, Type& element) noexcept
        {
            if (JOIN_UNLIKELY (segment == nullptr))
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            auto& sync = segment->_sync;
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
    };

    /**
     * @brief multiple producer single consumer ring buffer.
     */
    template <typename Type, typename Backend>
    struct Mpsc
    {
        using Queue = BasicQueue <Type, Backend, Mpsc>;

        /**
         * @brief try to push element into the ring buffer.
         * @param segment shared memory segment.
         * @param element element to push.
         * @return 0 on success, -1 otherwise.
         */
        static int tryPush (QueueSegment <Type>* segment, const Type& element) noexcept
        {
            if (JOIN_UNLIKELY (segment == nullptr))
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            auto& sync = segment->_sync;
            uint64_t head = sync._head.load (std::memory_order_relaxed);

            for (;;)
            {
                auto* slot = &segment->_elements[head & sync._mask];
                uint64_t seq = slot->_seq.load (std::memory_order_acquire);

                if (seq == head)
                {
                    if (JOIN_LIKELY (sync._head.compare_exchange_weak (head, head + 1, std::memory_order_acquire)))
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
                    head = sync._head.load (std::memory_order_relaxed);
                }
            }
        }

        /**
         * @brief try to pop element from the ring buffer.
         * @param segment shared memory segment.
         * @param element output element.
         * @return 0 on success, -1 otherwise.
         */
        static int tryPop (QueueSegment <Type>* segment, Type& element) noexcept
        {
            if (JOIN_UNLIKELY (segment == nullptr))
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            auto& sync = segment->_sync;
            uint64_t tail = sync._tail.load (std::memory_order_relaxed);
            auto* slot = &segment->_elements[tail & sync._mask];
            uint64_t seq = slot->_seq.load (std::memory_order_acquire);

            if (JOIN_UNLIKELY (seq != tail + 1))
            {
                lastError = make_error_code (Errc::TemporaryError);
                return -1;
            }

            slot->_seq.store (tail + sync._capacity, std::memory_order_release);
            element = slot->data;
            sync._tail.store (tail + 1, std::memory_order_release);

            return 0;
        }
    };

    /**
     * @brief multiple producer multiple consumer ring buffer.
     */
    template <typename Type, typename Backend>
    struct Mpmc
    {
        using Queue = BasicQueue <Type, Backend, Mpmc>;

        /**
         * @brief try to push element into the ring buffer.
         * @param segment shared memory segment.
         * @param element element to push.
         * @return 0 on success, -1 otherwise.
         */
        static int tryPush (QueueSegment <Type>* segment, const Type& element) noexcept
        {
            return Mpsc <Type, Backend>::tryPush (segment, element);
        }

        /**
         * @brief try to pop element from the ring buffer.
         * @param segment shared memory segment.
         * @param element output element.
         * @return 0 on success, -1 otherwise.
         */
        static int tryPop (QueueSegment <Type>* segment, Type& element) noexcept
        {
            if (JOIN_UNLIKELY (segment == nullptr))
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            auto& sync = segment->_sync;
            uint64_t tail = sync._tail.load (std::memory_order_relaxed);

            for (;;)
            {
                auto* slot = &segment->_elements[tail & sync._mask];
                uint64_t seq = slot->_seq.load (std::memory_order_acquire);

                if (seq == (tail + 1))
                {
                    Type temp = slot->data;

                    if (JOIN_LIKELY (sync._tail.compare_exchange_weak (tail, tail + 1, std::memory_order_acquire)))
                    {
                        element = temp;
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
                    tail = sync._tail.load (std::memory_order_relaxed);
                }
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
        template <typename Type> using Queue = BasicQueue <Type, Backend, SyncPolicy <Type, Backend>>;
    };
}

#endif
