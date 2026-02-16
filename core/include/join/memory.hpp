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

#ifndef __JOIN_MEMORY_HPP__
#define __JOIN_MEMORY_HPP__

// libjoin.
#include <join/error.hpp>
#include <join/utils.hpp>

// C++.
#include <system_error>
#include <stdexcept>
#include <limits>

// C.
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <numaif.h>
#include <numa.h>
#include <fcntl.h>
#include <cstdint>

namespace join
{
    /// forward declarations.
    template <typename Backend, template <typename, typename> class SyncPolicy> struct SyncBinding;
    template <typename, typename> struct Spsc;
    template <typename, typename> struct Mpsc;
    template <typename, typename> struct Mpmc;

    /**
     * @brief local anonymous memory provider.
     */
    class LocalMem
    {
    public:
        using Spsc = SyncBinding <LocalMem, ::join::Spsc>;
        using Mpsc = SyncBinding <LocalMem, ::join::Mpsc>;
        using Mpmc = SyncBinding <LocalMem, ::join::Mpmc>;

        /**
         * @brief allocates a local anonymous memory segment.
         * @param size allocation size in bytes.
         * @param numa NUMA node ID, or -1 for default policy.
         * @throw std::system_error if mmap fails.
         */
        explicit LocalMem (uint64_t size, int numa = -1)
        { 
            long sc = sysconf (_SC_PAGESIZE);
            uint64_t pageSize = (sc > 0) ? static_cast <uint64_t> (sc) : 4096;
            _size = (size + pageSize - 1) & ~(pageSize - 1);

            create (numa);
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        LocalMem (const LocalMem& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to copy.
         * @return this.
         */
        LocalMem& operator= (const LocalMem& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        LocalMem (LocalMem&& other) noexcept 
        : _size (other._size)
        , _ptr (other._ptr)
        {
            other._size = 0;
            other._ptr = nullptr;
        }

        /**
         * @brief move assignment operator.
         * @param other other object to move.
         * @return this.
         */
        LocalMem& operator= (LocalMem&& other) noexcept
        {
            cleanup ();

            _size = other._size;
            _ptr = other._ptr;

            other._size = 0;
            other._ptr = nullptr;

            return *this;
        }

        /**
         * @brief releases the mapped memory.
         */
        ~LocalMem () noexcept
        {
            cleanup ();
        }

        /**
         * @brief get a const pointer to the memory at a given offset.
         * @param offset byte offset from the start of the memory.
         * @return pointer to the mapped memory at the specified offset, or nullptr if not opened.
         * @throws std::runtime_error if memory is not mapped.
         * @throws std::out_of_range if offset is out of bounds.
         */
        inline const void* get (uint64_t offset = 0) const
        {
            if (JOIN_UNLIKELY (_ptr == nullptr))
            {
                throw std::runtime_error ("memory not mapped");
            }

            if (JOIN_UNLIKELY (offset >= _size))
            {
                throw std::out_of_range ("offset out of bounds");
            }

            return static_cast <const char*> (_ptr) + offset;
        }

        /**
         * @brief get a pointer to the memory at a given offset.
         * @param offset byte offset from the start of the memory.
         * @return pointer to the mapped memory at the specified offset, or nullptr if not opened.
         * @throws std::runtime_error if memory is not mapped.
         * @throws std::out_of_range if offset is out of bounds.
         */
        inline void* get (uint64_t offset = 0)
        {
            if (JOIN_UNLIKELY (_ptr == nullptr))
            {
                throw std::runtime_error ("memory not mapped");
            }

            if (JOIN_UNLIKELY (offset >= _size))
            {
                throw std::out_of_range ("offset out of bounds");
            }

            return static_cast <char*> (_ptr) + offset;
        }

    private:
        /**
         * @brief create the memory.
         * @param numa NUMA node ID, or -1 for default policy.
         * @throw std::system_error if mmap fails.
         */
        void create (int numa)
        {
            _ptr = ::mmap (nullptr, _size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
            if ((_ptr == MAP_FAILED) && ((errno == ENOMEM) || (errno == EINVAL)))
            {
                // no hugepages available or no support.
                _ptr = ::mmap (nullptr, _size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            }

            if (_ptr == MAP_FAILED)
            {
                throw std::system_error (errno, std::generic_category (), "mmap failed");
            }

            // apply NUMA policy if requested - not critical if it fails
            if (numa >= 0)
            {
                unsigned long mask = (1UL << numa);
                ::mbind (_ptr, _size, MPOL_BIND, &mask, sizeof (mask) * 8, MPOL_MF_STRICT);
            }

            // pin the memory to RAM - not critical if it fails
            ::mlock (_ptr, _size);
        }

        /**
         * @brief cleanup the memory.
         */
        void cleanup () noexcept
        {
            if ((_ptr != nullptr) && (_ptr != MAP_FAILED))
            {
                ::munlock (_ptr, _size);
                ::munmap (_ptr, _size);
                _ptr = nullptr;
            }

            _size = 0;
        }

        /// memory size.
        uint64_t _size = 0;

        /// pointer to mapped memory.
        void* _ptr = nullptr;
    };

    /**
     * @brief posix shared memory provider.
     */
    class ShmMem
    {
    public:
        using Spsc = SyncBinding <ShmMem, ::join::Spsc>;
        using Mpsc = SyncBinding <ShmMem, ::join::Mpsc>;
        using Mpmc = SyncBinding <ShmMem, ::join::Mpmc>;

        /**
         * @brief creates or opens a named shared memory segment.
         * @param size shared memory size in bytes.
         * @param name shared memory unique name.
         * @param numa NUMA node ID, or -1 for default policy.
         * @throw std::system_error if mmap fails.
         */
        explicit ShmMem (uint64_t size, const std::string& name, int numa = 1)
        : _name (name)
        {
            long sc = sysconf (_SC_PAGESIZE);
            uint64_t pageSize = (sc > 0) ? static_cast <uint64_t> (sc) : 4096;
            _size = (size + pageSize - 1) & ~(pageSize - 1);

            if (_size > static_cast <uint64_t> (std::numeric_limits <off_t>::max ()))
            {
                throw std::overflow_error ("size will overflow");
            }

            create (numa);
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        ShmMem (const ShmMem& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to copy.
         * @return this.
         */
        ShmMem& operator= (const ShmMem& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        ShmMem (ShmMem&& other) noexcept 
        : _size (other._size)
        , _name (std::move (other._name))
        , _ptr (other._ptr)
        , _fd (other._fd)
        {
            other._size = 0;
            other._ptr = nullptr;
            other._fd = -1;
        }

        /**
         * @brief move assignment operator.
         * @param other other object to move.
         * @return this.
         */
        ShmMem& operator= (ShmMem&& other) noexcept
        {
            cleanup ();

            _size = other._size;
            _name = std::move (other._name);
            _ptr = other._ptr;
            _fd = other._fd;

            other._size = 0;
            other._ptr = nullptr;
            other._fd = -1;

            return *this;
        }

        /**
         * @brief unmaps the memory and closes the file descriptor.
         */
        ~ShmMem () noexcept
        {
            cleanup ();
        }

        /**
         * @brief get a const pointer to the shared memory at a given offset.
         * @param offset byte offset from the start of the shared memory.
         * @return pointer to the mapped memory at the specified offset, or nullptr if not opened.
         * @throws std::runtime_error if memory is not mapped.
         * @throws std::out_of_range if offset is out of bounds.
         */
        inline const void* get (uint64_t offset = 0) const
        {
            if (JOIN_UNLIKELY (_ptr == nullptr))
            {
                throw std::runtime_error ("memory not mapped");
            }

            if (JOIN_UNLIKELY (offset >= _size))
            {
                throw std::out_of_range ("offset out of bounds");
            }

            return static_cast <const char*> (_ptr) + offset;
        }

        /**
         * @brief get a pointer to the shared memory at a given offset.
         * @param offset byte offset from the start of the shared memory.
         * @return pointer to the mapped memory at the specified offset, or nullptr if not opened.
         * @throws std::runtime_error if memory is not mapped.
         * @throws std::out_of_range if offset is out of bounds.
         */
        inline void* get (uint64_t offset = 0)
        {
            if (JOIN_UNLIKELY (_ptr == nullptr))
            {
                throw std::runtime_error ("memory not mapped");
            }

            if (JOIN_UNLIKELY (offset >= _size))
            {
                throw std::out_of_range ("offset out of bounds");
            }

            return static_cast <char*> (_ptr) + offset;
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

    private:
        /**
         * @brief create the posix shared memory.
         * @param numa NUMA node ID, or -1 for default policy.
         * @throw std::system_error if mmap fails.
         */
        void create (int numa)
        {
            bool created = true;

            _fd = ::shm_open (_name.c_str (), O_CREAT | O_RDWR | O_EXCL | O_CLOEXEC, 0644);
            if ((_fd == -1) && (errno == EEXIST))
            {
                created = false;
                _fd = ::shm_open (_name.c_str (), O_RDWR | O_CLOEXEC, 0644);
            }

            if (_fd == -1)
            {
                throw std::system_error (errno, std::generic_category (), "shm_open failed");
            }

            if (!created)
            {
                struct stat st;

                if (fstat (_fd, &st) == -1)
                {
                    int err = errno;
                    ::close (_fd);
                    throw std::system_error (err, std::generic_category (), "fstat failed");
                }

                if (static_cast <uint64_t> (st.st_size) != _size)
                {
                    ::close (_fd);
                    throw std::runtime_error ("shared memory size mismatch");
                }
            }
            else
            {
                if (::ftruncate (_fd, _size) == -1)
                {
                    int err = errno;
                    ::close (_fd);
                    throw std::system_error (err, std::generic_category (), "ftruncate failed");
                }
            }

            _ptr = ::mmap (nullptr, _size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_HUGETLB, _fd, 0);
            if ((_ptr == MAP_FAILED) && ((errno == ENOMEM) || (errno == EINVAL)))
            {
                // no hugepages available or no support.
                _ptr = ::mmap (nullptr, _size, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, 0);
            }

            if (_ptr == MAP_FAILED)
            {
                int err = errno;
                ::close (_fd);
                throw std::system_error (err, std::generic_category (), "mmap failed");
            }

            // apply NUMA policy if requested - not critical if it fails
            if (numa >= 0)
            {
                unsigned long mask = (1UL << numa);
                ::mbind (_ptr, _size, MPOL_BIND, &mask, sizeof (mask) * 8, MPOL_MF_STRICT);
            }

            // pin the memory to RAM - not critical if it fails
            ::mlock (_ptr, _size);
        }

        /**
         * @brief cleanup the posix shared memory.
         */
        void cleanup () noexcept
        {
            if ((_ptr != nullptr) && (_ptr != MAP_FAILED))
            {
                ::munlock (_ptr, _size);
                ::munmap (_ptr, _size);
                _ptr = nullptr;
            }

            if (_fd != -1)
            {
                ::close (_fd);
                _fd = -1;
            }

            _name.clear ();
            _size = 0;
        }

        /// shared memory size.
        uint64_t _size = 0;

        /// shared memory name.
        std::string _name;

        /// pointer to mapped shared memory.
        void* _ptr = nullptr;

        /// shared memory file descriptor.
        int _fd = -1;
    };
}

#endif
