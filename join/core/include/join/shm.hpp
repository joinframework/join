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

#ifndef __JOIN_SHM_HPP__
#define __JOIN_SHM_HPP__

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
    struct ShmSync
    {
        alignas (64) SharedMutex _mutex;
        SharedCondition _condition;
        alignas (64) std::atomic_ulong _signalCount;
    };

    /**
     * @brief shared memory publisher policy.
     */
    class PublisherPolicy
    {
    public:
        /**
         * @brief create instance.
         */
        PublisherPolicy () noexcept = default;

        /**
         * @brief destroy instance.
         */
        ~PublisherPolicy () noexcept = default;

        /**
         * @brief notify the subscriber.
         * @param sync synchronization primitives.
         * @return 0 on success, -1 on failure.
         */
        int notify (ShmSync* sync) const
        {
            if (sync == nullptr)
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            // fast path
            sync->_signalCount.fetch_add (1, std::memory_order_release);

            // slow path
            sync->_condition.signal ();

            return 0;
        }

        /**
         * @brief get shared memory open flags.
         * @return open flags.
         */
        constexpr int mode () const noexcept
        {
            return O_CREAT | O_EXCL | O_RDWR;
        }

        /**
         * @brief get shared memory maping protection flags.
         * @return protection flags.
         */
        constexpr int protection () const noexcept
        {
            return PROT_READ | PROT_WRITE;
        }
    };

    /**
     * @brief shared memory subscriber policy.
     */
    class SubscriberPolicy
    {
    public:
        /**
         * @brief create instance.
         */
        SubscriberPolicy () noexcept = default;

        /**
         * @brief destroy instance.
         */
        ~SubscriberPolicy () noexcept = default;

        /**
         * @brief wait publisher notification.
         * @param sync synchronization primitives.
         * @return 0 on success, -1 on failure.
         */
        int wait (ShmSync* sync) const
        {
            if (sync == nullptr)
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            // fast path (if already signaled)
            auto expected = sync->_signalCount.load (std::memory_order_acquire);
            if (expected > 0 && sync->_signalCount.compare_exchange_strong (expected, expected - 1, std::memory_order_acquire, std::memory_order_acquire))
            {
                return 0;
            }

            // slow path (wait)
            ScopedLock <SharedMutex> lock (sync->_mutex);

            // re-check after locking the mutex
            expected = sync->_signalCount.load (std::memory_order_relaxed);
            if (expected > 0)
            {
                sync->_signalCount.fetch_sub (1, std::memory_order_relaxed);
                return 0;
            }

            sync->_condition.wait (lock, [&] () { return sync->_signalCount.load (std::memory_order_relaxed) > 0; });
            sync->_signalCount.fetch_sub (1, std::memory_order_relaxed);

            return 0;
        }

        /**
         * @brief wait publisher notification until timeout expire.
         * @param sync synchronization primitives.
         * @param rt relative timeout.
         * @return 0 on success, -1 on failure.
         */
        template <class Rep, class Period>
        int timedWait (ShmSync* sync, std::chrono::duration <Rep, Period> rt) const
        {
            if (sync == nullptr)
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            // fast path (if already signaled)
            auto expected = sync->_signalCount.load (std::memory_order_acquire);
            if (expected > 0 && sync->_signalCount.compare_exchange_strong (expected, expected - 1, std::memory_order_acquire, std::memory_order_acquire))
            {
                return 0;
            }

            // slow path (wait)
            ScopedLock <SharedMutex> lock (sync->_mutex);
            
            // re-check after locking the mutex
            expected = sync->_signalCount.load (std::memory_order_relaxed);
            if (expected > 0)
            {
                sync->_signalCount.fetch_sub (1, std::memory_order_relaxed);
                return 0;
            }

            if (!sync->_condition.timedWait (lock, rt, [&] () { return sync->_signalCount.load (std::memory_order_relaxed) > 0; }))
            {
                return -1;
            }
            sync->_signalCount.fetch_sub (1, std::memory_order_relaxed);

            return 0;
        }

        /**
         * @brief get shared memory open flags.
         * @return open flags.
         */
        constexpr int mode () const noexcept
        {
            return O_RDWR;
        }

        /**
         * @brief get shared memory maping protection flags.
         * @return protection flags.
         */
        constexpr int protection () const noexcept
        {
            return PROT_READ | PROT_WRITE;
        }
    };

    /**
     * @brief shared memory handler base class.
     */
    template <class ShmPolicy>
    class BasicShm
    {
    public:
        /**
         * @brief create instance.
         */
        BasicShm (off_t size = 4096) noexcept
        : _publisher (_policy.mode () & O_CREAT)
        , _userSize (size)
        , _totalSize (size + sizeof (ShmSync))
        {
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicShm (const BasicShm& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicShm& operator= (const BasicShm& other) = delete;

        /**
         * @brief destroy instance.
         */
        ~BasicShm () noexcept
        {
            close ();
        }

        /**
         * @brief open or create the shared memory segment.
         * @param shmName shared memory object name (must start with '/').
         * @return 0 on success, -1 on failure.
         */
        int open (const std::string& shmName)
        {
            if (opened ())
            {
                lastError = make_error_code (Errc::InUse);
                return -1;
            }

            std::string semName = shmName + "_ready";
            _sem = ::sem_open (semName.c_str (), O_CREAT, 0640, 0);
            if (_sem == SEM_FAILED)
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                return -1;
            }

            _semName = semName;

            if (!_publisher)
            {
                // wait for the publisher to be ready.
                if (::sem_wait (_sem) == -1)
                {
                    lastError = std::make_error_code (static_cast <std::errc> (errno));
                    close ();
                    return -1;
                }

                // ensure other subscribers can also open the shared memory.
                ::sem_post (_sem);
            }

            _fd = ::shm_open (shmName.c_str (), _policy.mode () | O_CLOEXEC, 0640);
            if (_fd == -1)
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                close ();
                return -1;
            }

            _shmName = shmName;

            if (_publisher && (::ftruncate (_fd, _totalSize) == -1))
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                close ();
                return -1;
            }

            _ptr = ::mmap (nullptr, _totalSize, _policy.protection (), MAP_SHARED, _fd, 0);
            if (_ptr == MAP_FAILED)
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                close ();
                return -1;
            }

            _sync = static_cast <ShmSync*> (_ptr);
            _data = static_cast <char*> (_ptr) + sizeof (ShmSync);

            if (_publisher)
            {
                new (&_sync->_mutex) SharedMutex ();
                new (&_sync->_condition) SharedCondition ();
                new (&_sync->_signalCount) std::atomic_ulong (0);

                // we are the semaphore owner.
                _semOwner = true;

                // signal that publisher is ready.
                ::sem_post (_sem);
            }

            return 0;
        }

        /**
         * @brief close the shared memory segment.
         */
        void close () noexcept
        {
            if ((_ptr != nullptr) && (_ptr != MAP_FAILED))
            {
                if (_publisher)
                {
                    _sync->_mutex.~SharedMutex ();
                    _sync->_condition.~SharedCondition ();
                }

                ::munmap (_ptr, _totalSize);

                _sync = nullptr;
                _data = nullptr;
                _ptr = nullptr;
            }

            if (_fd != -1)
            {
                ::close (_fd);
                _fd = -1;

                if (_publisher)
                {
                    ::shm_unlink (_shmName.c_str ());
                }
            }

            _shmName.clear ();

            if (_sem != nullptr && _sem != SEM_FAILED)
            {
                ::sem_close (_sem);
                _sem = nullptr;

                if (_publisher && _semOwner)
                {
                    ::sem_unlink (_semName.c_str ());
                }

                _semOwner = false;
            }

            _semName.clear ();
        }

        /**
         * @brief check if opened.
         * @return true if opened, false otherwise.
         */
        bool opened () const noexcept
        {
            return (_fd != -1);
        }

        /**
         * @brief send an event notification to the peer.
         * @return 0 on success, -1 on failure.
         */
        template <typename P = ShmPolicy, typename = typename std::enable_if <std::is_same <P, PublisherPolicy>::value>::type>
        int notify () const
        {
            return _policy.notify (_sync);
        }

        /**
         * @brief wait peer notification event.
         * @return 0 on success, -1 on failure.
         */
        template <typename P = ShmPolicy, typename = typename std::enable_if <std::is_same <P, SubscriberPolicy>::value>::type>
        int wait () const
        {
            return _policy.wait (_sync);
        }

        /**
         * @brief wait peer notification event until timeout expire.
         * @param rt relative timeout.
         * @return 0 on success, -1 on failure.
         */
        template <class Rep, class Period, typename P = ShmPolicy, typename = typename std::enable_if <std::is_same <P, SubscriberPolicy>::value>::type>
        int timedWait (std::chrono::duration <Rep, Period> rt) const
        {
            return _policy.timedWait (_sync, rt);
        }

        /**
         * @brief get a const pointer to the shared memory.
         * @return pointer to the mapped memory region.
         */
        const void* get () const noexcept
        {
            return _data;
        }

        /**
         * @brief get a const pointer to the shared memory.
         * @return pointer to the mapped memory region.
         */
        void* get () noexcept
        {
            return _data;
        }

        /**
         * @brief get the size of the shared memory region.
         * @return shared memory size in bytes.
         */
        off_t size () const noexcept
        {
            return _userSize;
        }

    private:
        /// policy defining behavior (publisher/subscriber).
        ShmPolicy _policy;

        /// is publisher.
        bool _publisher = false;

        /// pointer to mapped shared memory.
        void* _ptr = nullptr;

        /// pointer to synchronization primitives stored in shared memory.
        ShmSync* _sync = nullptr;

        /// pointer to buffer data.
        void* _data = nullptr;

        /// user shared memory size.
        off_t _userSize = 0;

        /// total shared memory size.
        off_t _totalSize = 0;

        /// shared memory descriptor.
        int _fd = -1;

        /// shared memory object name.
        std::string _shmName;

        /// semaphore handle.
        sem_t* _sem = nullptr;

        /// semaphore name.
        std::string _semName;

        /// is semaphore owner.
        bool _semOwner = false;
    };

    /**
     * @brief convenience wrapper for publisher/subscriber shared memory types.
     */
    struct Shm
    {
        using Publisher  = BasicShm <PublisherPolicy>;
        using Subscriber = BasicShm <SubscriberPolicy>;
    };

    /**
     * @brief ring buffer header.
     */
    struct RingHeader
    {
        alignas (64) std::atomic_ulong _head;
        alignas (64) std::atomic_ulong _tail;
        uint64_t _elementSize;
        uint64_t _capacity;
    };

    /**
     * @brief shared memory ring buffer for inter-process communication.
     */
    template <typename ShmType>
    class BasicShmRing
    {
    public:
        /**
         * @brief create instance.
         * @param elementSize size of each element in the ring buffer.
         * @param capacity number of elements in the ring buffer.
         */
        BasicShmRing (uint64_t elementSize = 64, uint64_t capacity = 1024) noexcept
        : _shm (sizeof (RingHeader) + (elementSize * capacity))
        , _elementSize (elementSize)
        , _capacity (capacity)
        {
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicShmRing (const BasicShmRing& other) = delete;

        /**
         * @brief copy assignment.
         * @param other other object to copy.
         * @return a reference to the current object.
         */
        BasicShmRing& operator= (const BasicShmRing& other) = delete;

        /**
         * @brief destroy instance.
         */
        ~BasicShmRing () noexcept
        {
            close ();
        }

        /**
         * @brief open or create the shared memory segment.
         * @param name shared memory object name (must start with '/').
         * @return 0 on success, -1 on failure.
         */
        int open (const std::string& name)
        {
            if (_shm.open (name) == -1)
            {
                return -1;
            }

            _header = static_cast <RingHeader*> (_shm.get ());
            _data = static_cast <char*> (_shm.get ()) + sizeof (RingHeader);

            if (std::is_same <ShmType, Shm::Publisher>::value)
            {
                new (&_header->_head) std::atomic_ulong (0);
                new (&_header->_tail) std::atomic_ulong (0);
                _header->_elementSize = _elementSize;
                _header->_capacity = _capacity;
            }
            else
            {
                if ((_header->_elementSize != _elementSize) || (_header->_capacity != _capacity))
                {
                    lastError = make_error_code (Errc::InvalidParam);
                    close ();
                    return -1;
                }
            }

            return 0;
        }

        /**
         * @brief close the shared memory segment.
         */
        void close () noexcept
        {
            _header = nullptr;
            _data = nullptr;
            _shm.close ();
        }

        /**
         * @brief check if opened.
         * @return true if opened, false otherwise.
         */
        bool opened () const noexcept
        {
            return _shm.opened ();
        }

        /**
         * @brief push element into ring buffer.
         * @param element pointer to element to push.
         * @return 0 on success, -1 otherwise.
         */
        template <typename T = ShmType, typename = typename std::enable_if <std::is_same <T, Shm::Publisher>::value>::type>
        int push (const void* element) noexcept
        {
            if ((_header == nullptr) || (element == nullptr))
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            if (full ())
            {
                lastError = make_error_code (Errc::TemporaryError);
                return -1;
            }

            auto head = _header->_head.load (std::memory_order_acquire);
            auto next = head % _capacity;

            std::memcpy (static_cast <char*> (_data) + (next * _elementSize), element, _elementSize);
            _header->_head.store (head + 1, std::memory_order_release);

            // CRITICAL: The nested shared memory must be notified after updating the head index.
            // This will ensure that the internal signal count is incremented and that any waiting subscribers are woken up.
            // If removed, internal counters may become inconsistent and lead to deadlocks.
            // Fast path is handled inside the notify() method.
            _shm.notify ();

            return 0;
        }

        /**
         * @brief pop element from ring buffer.
         * @param element pointer to output element.
         * @return  0 on success, -1 otherwise.
         */
        template <typename T = ShmType, typename = typename std::enable_if <std::is_same <T, Shm::Subscriber>::value>::type>
        int pop (void* element) noexcept
        {
            if ((_header == nullptr) || (element == nullptr))
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            // CRITICAL: The nested shared memory must be waited on before reading the tail index.
            // This will ensure that the internal signal count is decremented and that we only proceed if there is data to read.
            // If removed, internal counters may become inconsistent and lead to deadlocks.
            // Fast path is handled inside the wait() method.
            if (_shm.wait() == -1)
            {
                return -1;
            }

            auto tail = _header->_tail.load (std::memory_order_acquire);
            auto next = tail % _capacity;

            std::memcpy (element, static_cast <char*> (_data) + (next * _elementSize), _elementSize);
            _header->_tail.store (tail + 1, std::memory_order_release);

            return 0;
        }

        /**
         * @brief pop element from ring buffer.
         * @param element pointer to output element.
         * @param rt relative timeout.
         * @return  0 on success, -1 otherwise.
         */
        template <class Rep, class Period, typename T = ShmType, typename = typename std::enable_if <std::is_same <T, Shm::Subscriber>::value>::type>
        int timedPop (void* element, std::chrono::duration <Rep, Period> rt) noexcept
        {
            if ((_header == nullptr) || (element == nullptr))
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            // CRITICAL: The nested shared memory must be waited on before reading the tail index.
            // This will ensure that the internal signal count is decremented and that we only proceed if there is data to read.
            // If removed, internal counters may become inconsistent and lead to deadlocks.
            // Fast path is handled inside the timedWait() method.
            if (_shm.timedWait (rt) == -1)
            {
                return -1;
            }

            auto tail = _header->_tail.load (std::memory_order_acquire);
            auto next = tail % _capacity;

            std::memcpy (element, static_cast <char*> (_data) + (next * _elementSize), _elementSize);
            _header->_tail.store (tail + 1, std::memory_order_release);

            return 0;
        }

        /**
         * @brief get pending elements to read.
         * @return number of pending elements to read.
         */
        uint64_t pending () const noexcept
        {
            if (_header == nullptr)
            {
                return 0;
            }

            auto head = _header->_head.load (std::memory_order_acquire);
            auto tail = _header->_tail.load (std::memory_order_acquire);

            return head - tail;
        }

        /**
         * @brief get available slots to write.
         * @return number of available slots to write.
         */
        uint64_t available () const noexcept
        {
            if (_header == nullptr)
            {
                return 0;
            }

            return _capacity - pending ();
        }

        /**
         * @brief check if the ring buffer is empty.
         * @return true if empty, false otherwise.
         */
        bool empty () const noexcept
        {
            return pending () == 0;
        }

        /**
         * @brief check if the ring buffer is full.
         * @return true if full, false otherwise.
         */
        bool full () const noexcept
        {
            return available () == 0;
        }

        /**
         * @brief get the capacity of the ring buffer.
         * @return capacity in number of elements.
         */
        uint64_t capacity () const noexcept 
        {
            return _capacity;
        }

        /**
         * @brief get the size of each element in the ring buffer.
         * @return element size in bytes.
         */
        uint64_t elementSize () const noexcept
        {
            return _elementSize;
        }

        /**
         * @brief get a const pointer to the shared memory.
         * @return pointer to the mapped memory region.
         */
        const void* get () const noexcept
        {
            return _data;
        }

        /**
         * @brief get a pointer to the shared memory.
         * @return pointer to the mapped memory region.
         */
        void* get () noexcept
        {
            return _data;
        }

    private:
        /// shared memory object.
        ShmType _shm;

        /// pointer to ring buffer header.
        RingHeader* _header = nullptr;

        /// pointer to ring buffer data.
        void* _data = nullptr;

        /// ring buffer element size.
        uint64_t _elementSize = 0;

        /// ring buffer capacity
        uint64_t _capacity = 0;
    };

    /**
     * @brief convenience wrapper for producer/consumer ring buffer types.
     */
    struct ShmRing
    {
        using Producer = BasicShmRing <Shm::Publisher>;
        using Consumer = BasicShmRing <Shm::Subscriber>;
    };
}

#endif
