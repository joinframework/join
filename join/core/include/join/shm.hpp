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
#include <join/acceptor.hpp>
#include <join/reactor.hpp>
#include <join/error.hpp>

// C++.
#include <string>
#include <array>

// C.
#include <sys/eventfd.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>

namespace join
{
    /**
     * @brief synchronization primitives.
     */
    struct ShmSync
    {
        /// protection mutex.
        SharedMutex _mutex;

        /// server condition.
        SharedCondition _serverCond;
        bool _serverSignaled = false;

        /// client condition.
        SharedCondition _clientCond;
        bool _clientSignaled = false;
    };

    /**
     * @brief shared memory server policy.
     */
    class ServerPolicy
    {
    public:
        /**
         * @brief create instance.
         */
        ServerPolicy () noexcept = default;

        /**
         * @brief destroy instance.
         */
        ~ServerPolicy () noexcept = default;

        /**
         * @brief notify the client via eventfd.
         * @param sync synchronization primitives.
         * @return 0 on success, -1 on failure.
         */
        int notify (ShmSync* sync)
        {
            if (sync == nullptr)
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            ScopedLock <SharedMutex> lock (sync->_mutex);
            sync->_clientSignaled = true;
            sync->_clientCond.signal ();

            return 0;
        }

        /**
         * @brief wait client notification via eventfd.
         * @param sync synchronization primitives.
         * @return 0 on success, -1 on failure.
         */
        int wait (ShmSync* sync)
        {
            if (sync == nullptr)
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            ScopedLock <SharedMutex> lock (sync->_mutex);
            sync->_serverCond.wait (lock, [&] () { return sync->_serverSignaled;});
            sync->_serverSignaled = false;

            return 0;
        }

        /**
         * @brief get open flags for shared memory.
         * @return open flags.
         */
        constexpr int flag () const noexcept
        {
            return O_CREAT | O_EXCL | O_RDWR;
        }
    };

    /**
     * @brief shared memory client policy.
     */
    class ClientPolicy
    {
    public:
        /**
         * @brief create instance.
         */
        ClientPolicy () noexcept = default;

        /**
         * @brief destroy instance.
         */
        ~ClientPolicy () noexcept = default;

        /**
         * @brief notify the client via eventfd.
         * @param sync synchronization primitives.
         * @return 0 on success, -1 on failure.
         */
        int notify (ShmSync* sync)
        {
            if (sync == nullptr)
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            ScopedLock <SharedMutex> lock (sync->_mutex);
            sync->_serverSignaled = true;
            sync->_serverCond.signal ();

            return 0;
        }

        /**
         * @brief wait client notification via eventfd.
         * @param sync synchronization primitives.
         * @return 0 on success, -1 on failure.
         */
        int wait (ShmSync* sync)
        {
            if (sync == nullptr)
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            ScopedLock <SharedMutex> lock (sync->_mutex);
            sync->_clientCond.wait (lock, [&] () { return sync->_clientSignaled;});
            sync->_clientSignaled = false;

            return 0;
        }

        /**
         * @brief get open flags for shared memory.
         * @return open flags.
         */
        constexpr int flag () const noexcept
        {
            return O_RDWR;
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
        : _userSize (size)
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
         * @param name shared memory object name (must start with '/').
         * @return 0 on success, -1 on failure.
         */
        int open (const std::string& name)
        {
            if (opened ())
            {
                lastError = make_error_code (Errc::InUse);
                return -1;
            }

            _fd = ::shm_open (name.c_str (), _policy.flag () | O_CLOEXEC, 0640);
            if (_fd == -1)
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                return -1;
            }

            _name = name;

            if ((_policy.flag () & O_CREAT) && (::ftruncate (_fd, _totalSize) == -1))
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                close ();
                return -1;
            }

            _ptr = ::mmap (nullptr, _totalSize, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, 0);
            if (_ptr == MAP_FAILED)
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                close ();
                return -1;
            }

            _sync = static_cast <ShmSync*> (_ptr);

            if (_policy.flag () & O_CREAT)
            {
                new (&_sync->_mutex) SharedMutex ();
                new (&_sync->_serverCond) SharedCondition ();
                new (&_sync->_clientCond) SharedCondition ();
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
                if (_policy.flag () & O_CREAT)
                {
                    _sync->_mutex.~SharedMutex ();
                    _sync->_serverCond.~SharedCondition ();
                    _sync->_clientCond.~SharedCondition ();
                }

                ::munmap (_ptr, _totalSize);

                _sync = nullptr;
                _ptr = nullptr;
            }

            if (_fd != -1)
            {
                ::close (_fd);
                _fd = -1;

                if (_policy.flag () & O_CREAT)
                {
                    ::shm_unlink (_name.c_str ());
                }
            }

            _name.clear ();
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
        int notify ()
        {
            return _policy.notify (_sync);
        }

        /**
         * @brief wait peer notification event.
         * @return 0 on success, -1 on failure.
         */
        int wait ()
        {
            return _policy.wait (_sync);
        }

        /**
         * @brief get a const pointer to the shared memory.
         * @return pointer to the mapped memory region.
         */
        const void* get () const noexcept
        {
            if (!_ptr)
            {
                return nullptr;
            }

            return static_cast <const char*> (_ptr) + sizeof (ShmSync);
        }

        /**
         * @brief get a const pointer to the shared memory.
         * @return pointer to the mapped memory region.
         */
        void* get () noexcept
        {
            if (!_ptr)
            {
                return nullptr;
            }

            return static_cast <char*> (_ptr) + sizeof (ShmSync);
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
        /// policy defining behavior (server/client).
        ShmPolicy _policy;

        /// pointer to mapped shared memory.
        void* _ptr = nullptr;

        /// pointer to synchronization primitives stored in shared memory.
        ShmSync* _sync = nullptr;

        /// user shared memory size.
        off_t _userSize = 0;

        /// total shared memory size.
        off_t _totalSize = 0;

        /// shared memory descriptor.
        int _fd = -1;

        /// shared memory object name.
        std::string _name;
    };

    /**
     * @brief convenience wrapper for server/client shared memory types.
     */
    struct Shm
    {
        using Server = BasicShm <ServerPolicy>;
        using Client = BasicShm <ClientPolicy>;
    };
}

#endif
