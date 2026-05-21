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

#ifndef JOIN_CORE_FUTEX_HPP
#define JOIN_CORE_FUTEX_HPP

// libjoin.
#include <join/mutex.hpp>

// Linux.
#include <sys/syscall.h>
#include <linux/futex.h>
#include <unistd.h>

// C++.
#include <atomic>

namespace join
{
    /**
     * @brief Futex-based mutex for intra-process locking.
     */
    class Futex
    {
    public:
        /**
         * @brief default constructor.
         */
        Futex () noexcept
        : _futex (0)
        {
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        Futex (const Futex& other) = delete;

        /**
         * @brief copy assignment.
         * @param other other object to copy.
         * @return a reference to the current object.
         */
        Futex& operator= (const Futex& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        Futex (Futex&& other) = delete;

        /**
         * @brief move assignment.
         * @param other other object to move.
         * @return a reference to the current object.
         */
        Futex& operator= (Futex&& other) = delete;

        /**
         * @brief destructor.
         */
        ~Futex () = default;

        /**
         * @brief lock the futex, blocking until it becomes available.
         */
        void lock () noexcept
        {
            uint32_t expected = 0;
            if (_futex.compare_exchange_strong (expected, 1, std::memory_order_acquire, std::memory_order_relaxed))
            {
                return;
            }

            do
            {
                if (expected == 2 ||
                    _futex.compare_exchange_strong (expected, 2, std::memory_order_relaxed, std::memory_order_relaxed))
                {
                    ::syscall (SYS_futex, reinterpret_cast<uint32_t*> (&_futex), FUTEX_WAIT_PRIVATE, 2, nullptr,
                               nullptr, 0);
                }
                expected = 0;
            }
            while (!_futex.compare_exchange_strong (expected, 2, std::memory_order_acquire, std::memory_order_relaxed));
        }

        /**
         * @brief try to lock the futex without blocking.
         * @return true if the lock was acquired, false if it was already locked.
         */
        bool tryLock () noexcept
        {
            uint32_t expected = 0;
            return _futex.compare_exchange_strong (expected, 1, std::memory_order_acquire, std::memory_order_relaxed);
        }

        /**
         * @brief unlock the futex, waking one waiter if any.
         */
        void unlock () noexcept
        {
            if (_futex.exchange (0, std::memory_order_release) == 2)
            {
                ::syscall (SYS_futex, reinterpret_cast<uint32_t*> (&_futex), FUTEX_WAKE_PRIVATE, 1, nullptr, nullptr,
                           0);
            }
        }

        /**
         * @brief get a pointer to the underlying futex word.
         * @return pointer to the atomic futex word.
         */
        std::atomic_uint32_t* handle () noexcept
        {
            return &_futex;
        }

    private:
        /// Futex.
        alignas (4) std::atomic_uint32_t _futex;
    };

    /**
     * @brief Futex-based mutex suitable for inter-process locking via shared memory.
     */
    class SharedFutex
    {
    public:
        /**
         * @brief default constructor.
         */
        SharedFutex () noexcept
        : _futex (0)
        {
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        SharedFutex (const SharedFutex& other) = delete;

        /**
         * @brief copy assignment.
         * @param other other object to copy.
         * @return a reference to the current object.
         */
        SharedFutex& operator= (const SharedFutex& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        SharedFutex (SharedFutex&& other) = delete;

        /**
         * @brief move assignment.
         * @param other other object to move.
         * @return a reference to the current object.
         */
        SharedFutex& operator= (SharedFutex&& other) = delete;

        /**
         * @brief destructor.
         */
        ~SharedFutex () = default;

        /**
         * @brief lock the futex, blocking until it becomes available.
         */
        void lock () noexcept
        {
            uint32_t expected = 0;
            if (_futex.compare_exchange_strong (expected, 1, std::memory_order_acquire, std::memory_order_relaxed))
            {
                return;
            }

            do
            {
                if (expected == 2 ||
                    _futex.compare_exchange_strong (expected, 2, std::memory_order_relaxed, std::memory_order_relaxed))
                {
                    ::syscall (SYS_futex, reinterpret_cast<uint32_t*> (&_futex), FUTEX_WAIT, 2, nullptr, nullptr, 0);
                }
                expected = 0;
            }
            while (!_futex.compare_exchange_strong (expected, 2, std::memory_order_acquire, std::memory_order_relaxed));
        }

        /**
         * @brief try to lock the futex without blocking.
         * @return true if the lock was acquired, false if it was already locked.
         */
        bool tryLock () noexcept
        {
            uint32_t expected = 0;
            return _futex.compare_exchange_strong (expected, 1, std::memory_order_acquire, std::memory_order_relaxed);
        }

        /**
         * @brief unlock the futex, waking one waiter if any.
         */
        void unlock () noexcept
        {
            if (_futex.exchange (0, std::memory_order_release) == 2)
            {
                ::syscall (SYS_futex, reinterpret_cast<uint32_t*> (&_futex), FUTEX_WAKE, 1, nullptr, nullptr, 0);
            }
        }

        /**
         * @brief get a pointer to the underlying futex word.
         * @return pointer to the atomic futex word.
         */
        std::atomic_uint32_t* handle () noexcept
        {
            return &_futex;
        }

    private:
        /// Futex word: 0=unlocked, 1=locked, 2=locked+waiters.
        alignas (4) std::atomic_uint32_t _futex;
    };
}

#endif
