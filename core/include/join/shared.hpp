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
#include <join/error.hpp>

// C++.
#include <utility>
#include <thread>
#include <atomic>
#include <chrono>
#include <memory>
#include <string>

// C.
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

namespace join
{
    /**
     * @brief shared memory class.
     */
    class SharedMemory
    {
    public:
        /**
         * @brief create instance.
         * @param name shared memory name.
         * @param size shared memory size.
         */
        SharedMemory (const std::string& name, uint64_t size)
        : _name (name)
        , _size (size)
        {
            if (_size > static_cast <uint64_t> (std::numeric_limits <off_t>::max ()))
            {
                throw std::overflow_error ("size will overflow");
            }
        }

        /**
         * @brief destroy the instance.
         */
        virtual ~SharedMemory ()
        {
            this->close ();
        }

        /**
         * @brief open or create the shared memory.
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

            if (created && (::ftruncate (this->_fd, this->_size) == -1))
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                this->close ();
                return -1;
            }

            this->_ptr = ::mmap (nullptr, this->_size, PROT_READ | PROT_WRITE, MAP_SHARED, this->_fd, 0);
            if (this->_ptr == MAP_FAILED)
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                this->close ();
                return -1;
            }

            return 0;
        }

        /**
         * @brief check if shared memory is opened.
         * @return true if opened, false otherwise.
         */
        bool opened () const noexcept
        {
            return (this->_ptr != nullptr) && (this->_ptr != MAP_FAILED);
        }

        /**
         * @brief close the shared memory.
         */
        virtual void close () noexcept 
        {
            if (this->opened ())
            {
                ::munmap (this->_ptr, this->_size);
            }

            if (this->_fd != -1)
            {
                ::close (this->_fd);
                this->_fd = -1;
            }

            this->_ptr = nullptr;
        }

        /**
         * @brief get a const pointer to the shared memory at a given offset.
         * @param offset byte offset from the start of the shared memory.
         * @return pointer to the mapped memory at the specified offset, or nullptr if not opened.
         * @throws std::out_of_range if offset is out of bounds.
         */
        virtual const void* get (uint64_t offset = 0) const
        {
            if (offset >= this->_size)
            {
                throw std::out_of_range ("offset out of bounds");
            }

            if (this->_ptr == nullptr)
            {
                return nullptr;
            }

            return static_cast <const char*> (this->_ptr) + offset;
        }

        /**
         * @brief get a pointer to the shared memory at a given offset.
         * @param offset byte offset from the start of the shared memory.
         * @return pointer to the mapped memory at the specified offset, or nullptr if not opened.
         * @throws std::out_of_range if offset is out of bounds.
         */
        virtual void* get (uint64_t offset = 0)
        {
            if (offset >= this->_size)
            {
                throw std::out_of_range ("offset out of bounds");
            }

            if (this->_ptr == nullptr)
            {
                return nullptr;
            }

            return static_cast <char*> (this->_ptr) + offset;
        }

        /**
         * @brief get the size of the shared memory.
         * @return shared memory size in bytes.
         */
        virtual uint64_t size () const noexcept
        {
            return this->_size;
        }

        /**
         * @brief destroy synchronization primitives and unlink the shared memory segment.
         * @param name shared memory segment name.
         * @return 0 on success, -1 on failure.
         */
        static int unlink (const std::string& name) noexcept
        {
            if (::shm_unlink (name.c_str ()) == -1)
            {
                if (errno == ENOENT)
                {
                    return 0;
                }
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                return -1;
            }

            return 0;
        }

    protected:
        /// shared memory name.
        std::string _name;

        /// shared memory size.
        uint64_t _size = 0;

        /// pointer to mapped shared memory.
        void* _ptr = nullptr;

        /// shared memory file descriptor.
        int _fd = -1;
    };

    /**
     * @brief synchronization primitives.
     */
    struct SharedSync
    {
        static constexpr uint64_t MAGIC = 0x9F7E3B2A8D5C4E1B; 
        alignas (64) std::atomic_uint64_t _magic;
        alignas (64) std::atomic_uint64_t _head;
        alignas (64) std::atomic_uint64_t _tail;
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
    template <typename Policy>
    class BasicQueue : public SharedMemory
    {
    public:
        /**
         * @brief create instance.
         * @param name shared memory segment name.
         * @param elementSize shared memory segment element size.
         * @param capacity shared memory segment capacity.
         */
        BasicQueue (const std::string& name, uint64_t elementSize = 1472, uint64_t capacity = 144)
        : SharedMemory (name, (elementSize * capacity) + sizeof (SharedSync))
        , _elementSize (elementSize)
        , _capacity (capacity)
        {
            if ((_capacity != 0) && (_elementSize > (std::numeric_limits <uint64_t>::max () / _capacity)))
            {
                throw std::overflow_error ("size will overflow");
            }

            _userSize = _elementSize * _capacity;
            _totalSize = _userSize + sizeof (SharedSync);
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicQueue (const BasicQueue& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicQueue& operator= (const BasicQueue& other) = delete;

        /**
         * @brief destroy the instance.
         */
        virtual ~BasicQueue ()
        {
            this->close ();
        }

        /**
         * @brief open or create the shared memory segment.
         * @return 0 on success, -1 on failure.
         */
        int open () override
        {
            if (SharedMemory::open () != 0)
            {
                return -1;
            }

            this->_segment = static_cast <SharedSegment*> (this->_ptr);

            uint64_t expected = 0;
            if (this->_segment->_sync._magic.compare_exchange_strong (expected, 0xFFFFFFFFFFFFFFFF, std::memory_order_acquire, std::memory_order_acquire))
            {
                new (&this->_segment->_sync._head) std::atomic_uint64_t (0);
                new (&this->_segment->_sync._tail) std::atomic_uint64_t (0);
                this->_segment->_sync._elementSize = this->_elementSize;
                this->_segment->_sync._capacity = this->_capacity;

                this->_segment->_sync._magic.store (SharedSync::MAGIC, std::memory_order_release);
            }
            else
            {
                while (this->_segment->_sync._magic.load (std::memory_order_acquire) != SharedSync::MAGIC)
                {
                    std::this_thread::yield ();
                }
            }

            if ((this->_segment->_sync._elementSize != this->_elementSize) || (this->_segment->_sync._capacity != this->_capacity))
            {
                lastError = make_error_code (Errc::InvalidParam);
                this->close ();
                return -1;
            }

            return 0;
        }

        /**
         * @brief close the shared memory segment.
         */
        void close () noexcept override
        {
            this->_segment = nullptr;
            SharedMemory::close ();
        }

        /**
         * @brief get a const pointer to the shared memory data region at a given offset.
         * @param offset byte offset from the start of the data region.
         * @return pointer to the data region at the specified offset, or nullptr if not opened.
         * @throws std::out_of_range if offset is out of bounds.
         */
        const void* get (uint64_t offset = 0) const override
        {
            if (offset > this->_userSize)
            {
                throw std::out_of_range ("offset out of bounds");
            }

            return SharedMemory::get (sizeof (SharedSync) + offset);
        }

        /**
         * @brief get a pointer to the shared memory data region at a given offset.
         * @param offset byte offset from the start of the data region.
         * @return pointer to the data region at the specified offset, or nullptr if not opened.
         * @throws std::out_of_range if offset is out of bounds.
         */
        void* get (uint64_t offset = 0) override
        {
            if (offset > this->_userSize)
            {
                throw std::out_of_range ("offset out of bounds");
            }

            return SharedMemory::get (sizeof (SharedSync) + offset);
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
        uint64_t size () const noexcept override
        {
            return this->_userSize;
        }

    protected:
        /// shared memory segment policy.
        Policy _policy;

        /// shared memory segment element size.
        uint64_t _elementSize = 0;

        /// shared memory segment capacity.
        uint64_t _capacity = 0;

        /// total shared memory size.
        uint64_t _totalSize = 0;

        /// user shared memory size.
        uint64_t _userSize = 0;

        /// shared memory segment.
        SharedSegment* _segment = nullptr;
    };

    /**
     * @brief shared memory producer.
     */
    template <typename Policy>
    class BasicProducer : public BasicQueue <Policy>
    {
    public:
        /**
         * @brief create instance.
         * @param name shared memory segment name.
         * @param elementSize shared memory segment element size.
         * @param capacity shared memory segment capacity.
         */
        BasicProducer (const std::string& name, uint64_t elementSize = 1472, uint64_t capacity = 144)
        : BasicQueue <Policy> (name, elementSize, capacity)
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
        ~BasicProducer () = default;

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
         * @brief try to push element into the ring buffer until timeout expire.
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
         * @brief get the number of available slots for writing.
         * @return number of slots available in the ring buffer.
         */
        uint64_t available () const noexcept
        {
            return this->_policy.available (this->_segment);
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
    template <typename Policy>
    class BasicConsumer : public BasicQueue <Policy>
    {
    public:
        /**
         * @brief create instance.
         * @param name shared memory segment name.
         * @param elementSize shared memory segment element size.
         * @param capacity shared memory segment capacity.
         */
        BasicConsumer (const std::string& name, uint64_t elementSize = 1472, uint64_t capacity = 144)
        : BasicQueue <Policy> (name, elementSize, capacity)
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
        ~BasicConsumer () = default;

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
         * @brief get the number of pending elements for reading.
         * @return number of elements pending in the ring buffer.
         */
        uint64_t pending () const noexcept
        {
            return this->_policy.pending (this->_segment);
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
     * @brief bidirectional shared memory communication endpoint.
     */
    template <typename OutboundPolicy, typename InboundPolicy>
    class BasicEndpoint
    {
    public:
        using Outbound = typename OutboundPolicy::Producer;
        using Inbound  = typename InboundPolicy::Consumer;

        /**
         * @brief endpoint side identifier.
         */
        enum Side
        {
            A,      /**< side A acts as outbound producer and inbound consumer. */
            B,      /**< side B acts as inbound producer and outbound consumer. */
        };

        /**
         * @brief create instance.
         * @param side role of this endpoint.
         * @param name channel name.
         * @param elementSize the size of each data element in the ring buffers.
         * @param capacity the maximum number of elements the ring buffers can hold.
         */
        BasicEndpoint (Side side, const std::string& name, uint64_t elementSize = 1472, uint64_t capacity = 144)
        : _side (side)
        , _name (name)
        {
            if (this->_side == Side::A)
            {
                this->_out = std::make_unique <Outbound> (this->_name + "_AB", elementSize, capacity);
                this->_in  = std::make_unique <Inbound>  (this->_name + "_BA", elementSize, capacity);
            }
            else
            {
                this->_out = std::make_unique <Outbound> (this->_name + "_BA", elementSize, capacity);
                this->_in  = std::make_unique <Inbound>  (this->_name + "_AB", elementSize, capacity);
            }
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicEndpoint (const BasicEndpoint& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to copy.
         * @return assigned object.
         */
        BasicEndpoint& operator= (const BasicEndpoint& other) = delete;

        /**
         * @brief destroy the instance.
         */
        ~BasicEndpoint ()
        {
            this->close();
        }

        /**
         * @brief open the channel endpoint.
         * @return 0 on success, -1 on failure
         */
        int open ()
        {
            if (this->opened ())
            {
                lastError = make_error_code (Errc::InUse);
                return -1;
            }

            if ((this->_out->open () == -1) || (this->_in->open () == -1))
            {
                this->close ();
                return -1;
            }

            return 0;
        }

        /**
         * @brief close the channel endpoint.
         */
        void close ()
        {
            if (this->_out)
            {
                this->_out->close ();
            }

            if (this->_in)
            {
                this->_in->close ();
            }
        }

        /**
         * @brief check if the endpoint is open.
         * @return true if both outbound and inbound queues are opened.
         */
        bool opened () const
        {
            return this->_out && this->_out->opened () && this->_in && this->_in->opened ();
        }

        /**
         * @brief try to send a message to the peer (non blocking).
         * @param element pointer to the element to send
         * @return 0 on success, -1 on failure.
         */
        int trySend (const void* element)
        {
            return this->_out->tryPush (element);
        }

        /**
         * @brief send a message to the peer (blocking).
         * @param element pointer to the element to send
         * @return 0 on success, -1 on failure.
         */
        int send (const void* element)
        {
            return this->_out->push (element);
        }

        /**
         * @brief send a message to the peer with timeout (blocking).
         * @param element pointer to the element to send
         * @param timeout maximum time to wait
         * @return 0 on success, -1 on failure.
         */
        template <class Rep, class Period>
        int timedSend (const void* element, std::chrono::duration <Rep, Period> timeout)
        {
            return this->_out->timedPush (element, timeout);
        }

        /**
         * @brief try to receive a message from the peer (non blocking).
         * @param element pointer to buffer for received element.
         * @return 0 on success, -1 on failure.
         */
        int tryReceive (void* element)
        {
            return this->_in->tryPop (element);
        }

        /**
         * @brief receive a message from the peer (blocking).
         * @param element pointer to buffer for received element.
         * @return 0 on success, -1 on failure.
         */
        int receive (void* element)
        {
            return this->_in->pop (element);
        }

        /**
         * @brief receive a message from the peer with timeout (blocking).
         * @param element pointer to buffer for received element.
         * @param timeout maximum time to wait
         * @return 0 on success, -1 on failure.
         */
        template <class Rep, class Period>
        int timedReceive (void* element, std::chrono::duration <Rep, Period> timeout)
        {
            return this->_in->timedPop (element, timeout);
        }

        /**
         * @brief get number of available slots for sending.
         * @return number of free slots in the outbound queue.
         */
        uint64_t available () const
        {
            return this->_out->available ();
        }

        /**
         * @brief check if outbound queue is full.
         * @return true if cannot send without blocking.
         */
        bool full () const
        {
            return this->_out->full ();
        }

        /**
         * @brief get number of pending messages.
         * @return number of messages waiting in inbound queue.
         */
        uint64_t pending () const
        {
            return this->_in->pending ();
        }

        /**
         * @brief check if inbound queue is empty.
         * @return true if no messages available.
         */
        bool empty () const
        {
            return this->_in->empty ();
        }

        /**
         * @brief get the side this endpoint represents.
         * @return Side::A or Side::B.
         */
        Side side () const
        {
            return this->_side;
        }

        /**
         * @brief get the channel name.
         * @return channel name.
         */
        const std::string& name () const
        {
            return this->_name;
        }

        /**
         * @brief get the element size.
         * @return size of each message element in bytes.
         */
        uint64_t elementSize () const
        {
            return this->_in->elementSize ();
        }

        /**
         * @brief get the buffer capacity.
         * @return number of elements each buffer can hold.
         */
        uint64_t capacity () const
        {
            return this->_in->capacity ();
        }

    private:
        /// side of this endpoint (A or B).
        Side _side;

        /// channel name.
        std::string _name;

        /// outbound channel.
        std::unique_ptr <Outbound> _out;

        /// inbound channel.
        std::unique_ptr <Inbound> _in;
    };

    /**
     * @brief single producer single consumer ring buffer policy.
     */
    class Spsc
    {
    public:
        using Producer = BasicProducer <Spsc>;
        using Consumer = BasicConsumer <Spsc>;
        using Endpoint = BasicEndpoint <Spsc, Spsc>;

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
            uint64_t tail = segment->_sync._tail.load (std::memory_order_acquire);
            uint64_t head = segment->_sync._head.load (std::memory_order_relaxed);
            if ((head - tail) == segment->_sync._capacity)
            {
                lastError = make_error_code (Errc::TemporaryError);
                return -1;
            }
            auto next = head % segment->_sync._capacity;
            std::memcpy (segment->_data + (next * segment->_sync._elementSize), element, segment->_sync._elementSize);
            segment->_sync._head.store (head + 1, std::memory_order_release);
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
            while (tryPush (segment, element) == -1)
            {
                if (lastError != Errc::TemporaryError)
                {
                    return -1;
                }
                std::this_thread::yield ();
            }
            return 0;
        }

        /**
         * @brief try to push element into the ring buffer until timeout expire.
         * @param segment shared memory segment.
         * @param element pointer to element to push.
         * @param timeout timeout.
         * @return 0 on success, -1 otherwise.
         */
        template <class Rep, class Period>
        int timedPush (SharedSegment* segment, const void* element, std::chrono::duration <Rep, Period> timeout) const noexcept
        {
            auto const deadline = std::chrono::steady_clock::now () + timeout;
            while (tryPush (segment, element) == -1)
            {
                if (lastError != Errc::TemporaryError)
                {
                    return -1;
                }
                if (std::chrono::steady_clock::now () >= deadline)
                {
                    lastError = make_error_code (Errc::TimedOut);
                    return -1;
                }
                std::this_thread::yield ();
            }
            return 0;
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
            uint64_t head = segment->_sync._head.load (std::memory_order_acquire);
            uint64_t tail = segment->_sync._tail.load (std::memory_order_relaxed);
            if ((head - tail) == 0)
            {
                lastError = make_error_code (Errc::TemporaryError);
                return -1;
            }
            auto next = tail % segment->_sync._capacity;
            std::memcpy (element, segment->_data + (next * segment->_sync._elementSize), segment->_sync._elementSize);
            segment->_sync._tail.store (tail + 1, std::memory_order_release);
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
            while (tryPop (segment, element) == -1)
            {
                if (lastError != Errc::TemporaryError)
                {
                    return -1;
                }
                std::this_thread::yield ();
            }
            return 0;
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
            auto const deadline = std::chrono::steady_clock::now () + timeout;
            while (tryPop (segment, element) == -1)
            {
                if (lastError != Errc::TemporaryError)
                {
                    return -1;
                }
                if (std::chrono::steady_clock::now () >= deadline)
                {
                    lastError = make_error_code (Errc::TimedOut);
                    return -1;
                }
                std::this_thread::yield ();
            }
            return 0;
        }

        /**
         * @brief get the number of pending elements for reading.
         * @param segment shared memory segment.
         * @return number of elements pending in the ring buffer.
         */
        uint64_t pending (SharedSegment* segment) const noexcept
        {
            auto head = segment->_sync._head.load (std::memory_order_acquire);
            auto tail = segment->_sync._tail.load (std::memory_order_acquire);
            return head - tail;
        }

        /**
         * @brief get the number of available slots for writing.
         * @param segment shared memory segment.
         * @return number of slots available in the ring buffer.
         */
        uint64_t available (SharedSegment* segment) const noexcept
        {
            return segment->_sync._capacity - pending (segment);
        }

        /**
         * @brief check if the ring buffer is full.
         * @param segment shared memory segment.
         * @return true if full, false otherwise.
         */
        bool full (SharedSegment* segment) const noexcept
        {
            return pending (segment) == segment->_sync._capacity;
        }

        /**
         * @brief check if the ring buffer is empty.
         * @param segment shared memory segment.
         * @return true if empty, false otherwise.
         */
        bool empty (SharedSegment* segment) const noexcept
        {
            return pending (segment) == 0;
        }
    };

    /**
     * @brief multiple producer single consumer ring buffer policy.
     */
    class Mpsc : public Spsc
    {
    public:
        using Producer = BasicProducer <Mpsc>;
        using Consumer = BasicConsumer <Mpsc>;
        using Endpoint = BasicEndpoint <Mpsc, Mpsc>;

        /**
         * @brief construct the multiple producer single consumer ring buffer policy by default.
         */
        constexpr Mpsc () noexcept = default;

        /**
         * @brief try to push element into the ring buffer (lock-free for multiple producers).
         * @param segment shared memory segment.
         * @param element pointer to element to push.
         * @return 0 on success, -1 otherwise.
         */
        int tryPush (SharedSegment* segment, const void* element) const noexcept override
        {
            if ((segment == nullptr) || (element == nullptr))
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }
            uint64_t head, tail;
            do
            {
                tail = segment->_sync._tail.load (std::memory_order_acquire);
                head = segment->_sync._head.load (std::memory_order_relaxed);
                if ((head - tail) == segment->_sync._capacity)
                {
                    lastError = make_error_code (Errc::TemporaryError);
                    return -1;
                }
            }
            while (!segment->_sync._head.compare_exchange_weak (head, head + 1, std::memory_order_acquire, std::memory_order_relaxed));
            auto slot = head % segment->_sync._capacity;
            std::memcpy (segment->_data + (slot * segment->_sync._elementSize), element, segment->_sync._elementSize);
            std::atomic_thread_fence (std::memory_order_release);
            return 0;
        }
    };

    /**
     * @brief multiple producer multiple consumer ring buffer policy.
     */
    class Mpmc : public Mpsc
    {
    public:
        using Producer = BasicProducer <Mpmc>;
        using Consumer = BasicConsumer <Mpmc>;
        using Endpoint = BasicEndpoint <Mpmc, Mpmc>;

        /**
         * @brief construct the multiple producer multiple consumer ring buffer policy by default.
         */
        constexpr Mpmc () noexcept = default;

        /**
         * @brief try to pop element from the ring buffer (lock-free for multiple consumers).
         * @param segment shared memory segment.
         * @param element pointer to output element.
         * @return 0 on success, -1 otherwise.
         */
        int tryPop (SharedSegment* segment, void* element) const noexcept override
        {
            if ((segment == nullptr) || (element == nullptr))
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }
            uint64_t head, tail;
            do
            {
                head = segment->_sync._head.load (std::memory_order_acquire);
                tail = segment->_sync._tail.load (std::memory_order_relaxed);
                if ((head - tail) == 0)
                {
                    lastError = make_error_code (Errc::TemporaryError);
                    return -1;
                }
            }
            while (!segment->_sync._tail.compare_exchange_weak (tail, tail + 1, std::memory_order_acquire, std::memory_order_relaxed));
            auto slot = tail % segment->_sync._capacity;
            std::memcpy (element, segment->_data + (slot * segment->_sync._elementSize), segment->_sync._elementSize);
            std::atomic_thread_fence (std::memory_order_release);
            return 0;
        }
    };
}

#endif
