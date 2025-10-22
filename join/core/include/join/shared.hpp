/**
 * MIT License
 *
 * Copyright (c) 2025 Mathieu Rabine
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

#ifndef __JOIN_SHARED_HPP__
#define __JOIN_SHARED_HPP__

// libjoin.
#include <join/condition.hpp>
#include <join/error.hpp>

// C++.
#include <utility>
#include <chrono>
#include <atomic>
#include <string>

// C.
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

namespace join
{
    /**
     * @brief synchronization primitives.
     */
    struct SharedSync
    {
        static constexpr uint64_t MAGIC = 0x9F7E3B2A8D5C4E1B; 
        alignas (64) std::atomic_ulong _magic;
        alignas (64) SharedMutex _mutex;
        alignas (64) SharedCondition _notFull;
        alignas (64) SharedCondition _notEmpty;
        alignas (64) std::atomic_ulong _head;
        alignas (64) std::atomic_ulong _tail;
        alignas (64) uint64_t _elementSize;
        alignas (64) uint64_t _capacity;
    };

    /**
     * @brief shared memory segment.
     */
    struct SharedSegment
    {
        alignas (64) SharedSync _sync;
        alignas (max_align_t) uint8_t _data[];
    };

    /**
     * @brief shared memory base class.
     */
    template <typename BufferPolicy>
    class BasicShared
    {
    public:
        /**
         * @brief create instance.
         * @param name shared memory segment name.
         * @param elementSize shared memory segment element size.
         * @param capacity shared memory segment capacity.
         */
        BasicShared (const std::string& name, uint64_t elementSize = 1472, uint64_t capacity = 144)
        : _name (name)
        , _elementSize (elementSize)
        , _capacity (capacity)
        {
            if ((_capacity != 0) && (_elementSize > (std::numeric_limits <uint64_t>::max () / _capacity)))
            {
                throw std::overflow_error ("total size will overflow");
            }

            _userSize = _elementSize * _capacity;
            _totalSize = _userSize + sizeof(SharedSync);

            if (_totalSize > static_cast <uint64_t> (std::numeric_limits <off_t>::max ()))
            {
                throw std::overflow_error ("total size will overflow");
            }
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicShared (const BasicShared& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicShared& operator= (const BasicShared& other) = delete;

        /**
         * @brief destroy the instance.
         */
        virtual ~BasicShared () = default;

        /**
         * @brief open or create the shared memory segment.
         * @return 0 on success, -1 on failure.
         */
        virtual int open ()
        {
            if (this->opened ())
            {
                lastError = make_error_code (Errc::InUse);
                return -1;
            }

            bool created = true;
            this->_fd = ::shm_open (this->_name.c_str (), O_CREAT | O_RDWR | O_EXCL | O_CLOEXEC, 0644);
            if ((this->_fd == -1) && (errno == EEXIST))
            {
                created = false;
                this->_fd = ::shm_open(this->_name.c_str(), O_RDWR | O_CLOEXEC, 0644);
            }

            if (this->_fd == -1)
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                this->close ();
                return -1;
            }

            if (created)
            {
                ::ftruncate (this->_fd, this->_totalSize);
            }

            this->_ptr = ::mmap (nullptr, this->_totalSize, PROT_READ | PROT_WRITE, MAP_SHARED, this->_fd, 0);
            if (this->_ptr == MAP_FAILED)
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                this->close ();
                return -1;
            }

            this->_segment = static_cast <SharedSegment*> (this->_ptr);
            this->_data = this->_segment->_data;

            return 0;
        }

        /**
         * @brief close the shared memory segment.
         */
        void close () noexcept 
        {
            if (this->opened ())
            {
                ::munmap (this->_ptr, this->_totalSize);
                this->_data = nullptr;
                this->_segment = nullptr;
                this->_ptr = nullptr;
            }

            if (this->_fd != -1)
            {
                ::close (this->_fd);
                this->_fd = -1;
            }
        }

        /**
         * @brief check if shared memory segment is opened.
         * @return true if opened, false otherwise.
         */
        bool opened () const noexcept
        {
            return (this->_ptr != nullptr) && (this->_ptr != MAP_FAILED);
        }

        /**
         * @brief get a const pointer to the shared memory.
         * @return pointer to the mapped memory region.
         */
        const void* get () const noexcept
        {
            return this->_data;
        }

        /**
         * @brief get a const pointer to the shared memory.
         * @return pointer to the mapped memory region.
         */
        void* get () noexcept
        {
            return this->_data;
        }

        /**
         * @brief get the element size of the shared memory region.
         * @return shared memory element size in bytes.
         */
        uint64_t elementSize () const noexcept
        {
            return this->_elementSize;
        }

        /**
         * @brief get the capacity of the shared memory region.
         * @return shared memory capacity.
         */
        uint64_t capacity () const noexcept
        {
            return this->_capacity;
        }

        /**
         * @brief get the size of the shared memory region.
         * @return shared memory size in bytes.
         */
        uint64_t size () const noexcept
        {
            return this->_userSize;
        }

    protected:
        /// ring buffer policy.
        BufferPolicy _policy;

        /// shared memory segment name.
        std::string _name;

        /// shared memory segment element size.
        uint64_t _elementSize = 0;

        /// shared memory segment capacity.
        uint64_t _capacity = 0;

        /// user shared memory size.
        uint64_t _userSize = 0;

        /// total shared memory size.
        uint64_t _totalSize = 0;

        /// shared memory segment descriptor.
        int _fd = -1;

        /// pointer to mapped shared memory.
        void* _ptr = nullptr;

        /// shared memory segment.
        SharedSegment* _segment = nullptr;

        /// shared memory segment user data.
        void* _data = nullptr;
    };

    /**
     * @brief shared memory producer.
     */
    template <typename BufferPolicy>
    class BasicProducer : public BasicShared <BufferPolicy>
    {
    public:
        /**
         * @brief create instance.
         * @param name shared memory segment name.
         * @param elementSize shared memory segment element size.
         * @param capacity shared memory segment capacity.
         */
        BasicProducer (const std::string& name, uint64_t elementSize = 1472, uint64_t capacity = 144)
        : BasicShared <BufferPolicy> (name, elementSize, capacity)
        {
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicProducer (const BasicProducer& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicProducer& operator= (const BasicProducer& other) = delete;

        /**
         * @brief destroy the instance.
         */
        ~BasicProducer ()
        {
            this->close ();
        }

        /**
         * @brief open or create the shared memory segment.
         * @return 0 on success, -1 on failure.
         */
        int open () override
        {
            if (BasicShared <BufferPolicy>::open () == -1)
            {
                return -1;
            }

            uint64_t expected = 0;
            if (this->_segment->_sync._magic.compare_exchange_strong (expected, SharedSync::MAGIC, std::memory_order_acq_rel, std::memory_order_acquire))
            {
                new (&this->_segment->_sync._mutex) SharedMutex ();
                new (&this->_segment->_sync._notFull) SharedCondition ();
                new (&this->_segment->_sync._notEmpty) SharedCondition ();
                new (&this->_segment->_sync._elementSize) uint64_t (this->_elementSize);
                new (&this->_segment->_sync._capacity) uint64_t (this->_capacity);
                new (&this->_segment->_sync._head) std::atomic_ulong (0);
                new (&this->_segment->_sync._tail) std::atomic_ulong (0);
            }

            if ((this->_segment->_sync._elementSize != this->_elementSize) ||
                (this->_segment->_sync._capacity != this->_capacity))
            {
                lastError = make_error_code (Errc::InvalidParam);
                this->close ();
                return -1;
            }

            return 0;
        }

        /**
         * @brief try to push element into ring buffer.
         * @param element pointer to element to push.
         * @return 0 on success, -1 otherwise.
         */
        int tryPush (const void* element) noexcept
        {
            return this->_policy.tryPush (this->_segment, element);
        }

        /**
         * @brief push element into the ring buffer.
         * @param element pointer to element to push.
         * @return 0 on success, -1 otherwise.
         */
        int push (const void* element) noexcept
        {
            return this->_policy.push (this->_segment, element);
        }

        /**
         * @brief try to push push element into the ring buffer until timeout expire.
         * @param element pointer to element to push.
         * @param timeout timeout.
         * @return 0 on success, -1 otherwise.
         */
        template <class Rep, class Period>
        int timedPush (const void* element, std::chrono::duration <Rep, Period> timeout) noexcept
        {
            return this->_policy.timedPush (this->_segment, element, timeout);
        }

        /**
         * @brief check if the ring buffer is full.
         * @return true if full, false otherwise.
         */
        bool full () const noexcept
        {
            return this->_policy.full (this->_segment);
        }
    };

    /**
     * @brief shared memory consumer.
     */
    template <typename BufferPolicy>
    class BasicConsumer : public BasicShared <BufferPolicy>
    {
    public:
        /**
         * @brief create instance.
         * @param name shared memory segment name.
         * @param elementSize shared memory segment element size.
         * @param capacity shared memory segment capacity.
         */
        BasicConsumer (const std::string& name, uint64_t elementSize = 1472, uint64_t capacity = 144)
        : BasicShared <BufferPolicy> (name, elementSize, capacity)
        {
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicConsumer (const BasicConsumer& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicConsumer& operator= (const BasicConsumer& other) = delete;

        /**
         * @brief destroy the instance.
         */
        ~BasicConsumer ()
        {
            this->close ();
        }

        /**
         * @brief open or create the shared memory segment.
         * @return 0 on success, -1 on failure.
         */
        int open () override
        {
            if (BasicShared <BufferPolicy>::open () == -1)
            {
                return -1;
            }

            if (this->_segment->_sync._magic.load (std::memory_order_acquire) != SharedSync::MAGIC)
            {
                lastError = make_error_code (Errc::TemporaryError);
                this->close ();
                return -1;
            }

            if ((this->_segment->_sync._elementSize != this->_elementSize) || 
                (this->_segment->_sync._capacity != this->_capacity))
            {
                lastError = make_error_code (Errc::InvalidParam);
                this->close ();
                return -1;
            }

            return 0;
        }

        /**
         * @brief try to pop element from the ring buffer.
         * @param element pointer to output element.
         * @return 0 on success, -1 otherwise.
         */
        int tryPop (void* element) noexcept
        {
            return this->_policy.tryPop (this->_segment, element);
        }

        /**
         * @brief pop element from the ring buffer.
         * @param element pointer to output element.
         * @return 0 on success, -1 otherwise.
         */
        int pop (void* element) noexcept
        {
            return this->_policy.pop (this->_segment, element);
        }

        /**
         * @brief try to pop element from the ring buffer until timeout expire.
         * @param element pointer to output element.
         * @param timeout timeout.
         * @return 0 on success, -1 otherwise.
         */
        template <class Rep, class Period>
        int timedPop (void* element, std::chrono::duration <Rep, Period> timeout) noexcept
        {
            return this->_policy.timedPop (this->_segment, element, timeout);
        }

        /**
         * @brief check if the ring buffer is empty.
         * @return true if empty, false otherwise.
         */
        bool empty () const noexcept
        {
            return this->_policy.empty (this->_segment);
        }
    };

    /**
     * @brief single producer single consumer ring buffer policy.
     */
    class Spsc
    {
    public:
        using Producer = BasicProducer <Spsc>;
        using Consumer = BasicConsumer <Spsc>;

        /**
         * @brief construct the single producer single consumer ring buffer policy by default.
         */
        constexpr Spsc () noexcept = default;

        /**
         * @brief try to push element into the ring buffer.
         * @param segment shared memory segment.
         * @param element pointer to element to push.
         * @return 0 on success, -1 otherwise.
         */
        virtual int tryPush (SharedSegment* segment, const void* element) const noexcept
        {
            if ((segment == nullptr) || (element == nullptr))
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }
            uint64_t head, tail;
            if (full (segment, head, tail))
            {
                lastError = std::make_error_code (static_cast <std::errc> (Errc::TemporaryError));
                return -1;
            }
            auto next = head % segment->_sync._capacity;
            std::memcpy (segment->_data + (next * segment->_sync._elementSize), element, segment->_sync._elementSize);
            segment->_sync._head.store (head + 1, std::memory_order_release);
            segment->_sync._notEmpty.signal ();
            return 0;
        }

        /**
         * @brief push element into the ring buffer.
         * @param segment shared memory segment.
         * @param element pointer to element to push.
         * @return 0 on success, -1 otherwise.
         */
        int push (SharedSegment* segment, const void* element) const noexcept
        {
            if ((segment == nullptr) || (element == nullptr))
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }
            // fast path (brief spin wait).
            constexpr int nspin = 100;
            for (int i = 0; i < nspin; ++i)
            {
                if (tryPush (segment, element) == 0)
                {
                    return 0;
                }
                std::this_thread::yield ();
            }
            // slow path (block).
            ScopedLock <SharedMutex> lock (segment->_sync._mutex);
            segment->_sync._notFull.wait (lock, [&] () { return !full (segment); });
            return tryPush (segment, element);
        }

        /**
         * @brief try to push push element into the ring buffer until timeout expire.
         * @param segment shared memory segment.
         * @param element pointer to element to push.
         * @param timeout timeout.
         * @return 0 on success, -1 otherwise.
         */
        template <class Rep, class Period>
        int timedPush (SharedSegment* segment, const void* element, std::chrono::duration <Rep, Period> timeout) const noexcept
        {
            if ((segment == nullptr) || (element == nullptr))
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }
            // fast path (brief spin wait).
            auto const deadline = std::chrono::steady_clock::now () + timeout;
            constexpr int nspin = 100;
            for (int i = 0; i < nspin; ++i)
            {
                if (tryPush (segment, element) == 0)
                {
                    return 0;
                }
                if (std::chrono::steady_clock::now () >= deadline)
                {
                    lastError = make_error_code (Errc::TimedOut);
                    return -1;
                }
                std::this_thread::yield ();
            }
            // slow path (block).
            ScopedLock <SharedMutex> lock (segment->_sync._mutex);
            auto remaining = deadline - std::chrono::steady_clock::now ();
            if (remaining <= std::chrono::steady_clock::duration::zero ())
            {
                lastError = make_error_code (Errc::TimedOut);
                return -1;
            }
            if (!segment->_sync._notFull.timedWait (lock, remaining, [&] () { return !full (segment); }))
            {
                return -1;
            }
            return tryPush (segment, element);
        }

        /**
         * @brief try to pop element from the ring buffer.
         * @param segment shared memory segment.
         * @param element pointer to output element.
         * @return 0 on success, -1 otherwise.
         */
        virtual int tryPop (SharedSegment* segment, void* element) const noexcept
        {
            if ((segment == nullptr) || (element == nullptr))
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }
            uint64_t head, tail;
            if (empty (segment, head, tail))
            {
                lastError = std::make_error_code (static_cast <std::errc> (Errc::TemporaryError));
                return -1;
            }
            auto next = tail % segment->_sync._capacity;
            std::memcpy (element, segment->_data + (next * segment->_sync._elementSize), segment->_sync._elementSize);
            segment->_sync._tail.store (tail + 1, std::memory_order_release);
            segment->_sync._notFull.signal ();
            return 0;
        }

        /**
         * @brief pop element from the ring buffer.
         * @param segment shared memory segment.
         * @param element pointer to output element.
         * @return 0 on success, -1 otherwise.
         */
        int pop (SharedSegment* segment, void* element) const noexcept
        {
            if ((segment == nullptr) || (element == nullptr))
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }
            // fast path (brief spin wait).
            constexpr int nspin = 100;
            for (int i = 0; i < nspin; ++i)
            {
                if (tryPop (segment, element) == 0)
                {
                    return 0;
                }
                std::this_thread::yield ();
            }
            // slow path (block).
            ScopedLock <SharedMutex> lock (segment->_sync._mutex);
            segment->_sync._notEmpty.wait (lock, [&] () { return !empty (segment); });
            return tryPop (segment, element);
        }

        /**
         * @brief try to pop element from the ring buffer until timeout expire.
         * @param segment shared memory segment.
         * @param element pointer to output element.
         * @param timeout timeout.
         * @return 0 on success, -1 otherwise.
         */
        template <class Rep, class Period>
        int timedPop (SharedSegment* segment, void* element, std::chrono::duration <Rep, Period> timeout) const noexcept
        {
            if ((segment == nullptr) || (element == nullptr))
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }
            // fast path (brief spin wait).
            auto const deadline = std::chrono::steady_clock::now () + timeout;
            constexpr int nspin = 100;
            for (int i = 0; i < nspin; ++i)
            {
                if (tryPop (segment, element) == 0)
                {
                    return 0;
                }
                if (std::chrono::steady_clock::now () >= deadline)
                {
                    lastError = make_error_code (Errc::TimedOut);
                    return -1;
                }
                std::this_thread::yield ();
            }
            // slow path (block).
            ScopedLock <SharedMutex> lock (segment->_sync._mutex);
            auto remaining = deadline - std::chrono::steady_clock::now ();
            if (remaining <= std::chrono::steady_clock::duration::zero ())
            {
                lastError = make_error_code (Errc::TimedOut);
                return -1;
            }
            if (!segment->_sync._notEmpty.timedWait (lock, remaining, [&] () { return !empty (segment); }))
            {
                return -1;
            }
            return tryPop (segment, element);
        }

        /**
         * @brief check if the ring buffer is full.
         * @param segment shared memory segment.
         * @param head buffer head.
         * @param tail buffer tail.
         * @return true if full, false otherwise.
         */
        bool full (SharedSegment* segment, uint64_t& head, uint64_t& tail) const noexcept
        {
            head = segment->_sync._head.load (std::memory_order_relaxed);
            tail = segment->_sync._tail.load (std::memory_order_acquire);
            return (head - tail) == segment->_sync._capacity;
        }

        /**
         * @brief check if the ring buffer is full.
         * @param segment shared memory segment.
         * @return true if full, false otherwise.
         */
        bool full (SharedSegment* segment) const noexcept
        {
            uint64_t head, tail;
            return full (segment, head, tail);
        }

        /**
         * @brief check if the ring buffer is empty.
         * @param segment shared memory segment.
         * @param head buffer head.
         * @param tail buffer tail.
         * @return true if empty, false otherwise.
         */
        bool empty (SharedSegment* segment, uint64_t& head, uint64_t& tail) const noexcept
        {
            tail = segment->_sync._tail.load (std::memory_order_relaxed);
            head = segment->_sync._head.load (std::memory_order_acquire);
            return (head - tail) == 0;
        }

        /**
         * @brief check if the ring buffer is empty.
         * @param segment shared memory segment.
         * @return true if empty, false otherwise.
         */
        bool empty (SharedSegment* segment) const noexcept
        {
            uint64_t head, tail;
            return empty (segment, head, tail);
        }
    };
}

#endif
