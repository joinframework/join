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
#include <join/thread.hpp>
#include <join/mutex.hpp>

// Linux.
#include <sys/syscall.h>
#include <linux/futex.h>
#include <unistd.h>
#include <cerrno>

// C++.
#include <cstdlib>
#include <atomic>

#if defined(__SANITIZE_THREAD__)
#define JOIN_TSAN 1
#elif defined(__has_feature)
#if __has_feature(thread_sanitizer)
#define JOIN_TSAN 1
#endif
#endif

#ifdef JOIN_TSAN
#include <sanitizer/tsan_interface.h>
#define JOIN_TSAN_ACQUIRE(addr) __tsan_acquire (addr)
#define JOIN_TSAN_RELEASE(addr) __tsan_release (addr)
#else
#define JOIN_TSAN_ACQUIRE(addr)
#define JOIN_TSAN_RELEASE(addr)
#endif

namespace join
{
    /**
     * @brief Futex-based mutex for intra-process locking.
     */
    template <class LockPolicy>
    class BasicFutex
    {
    public:
        /**
         * @brief default constructor.
         */
        BasicFutex () noexcept
        : _futex (0)
        {
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicFutex (const BasicFutex& other) = delete;

        /**
         * @brief copy assignment.
         * @param other other object to copy.
         * @return a reference to the current object.
         */
        BasicFutex& operator= (const BasicFutex& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        BasicFutex (BasicFutex&& other) = delete;

        /**
         * @brief move assignment.
         * @param other other object to move.
         * @return a reference to the current object.
         */
        BasicFutex& operator= (BasicFutex&& other) = delete;

        /**
         * @brief destructor.
         */
        ~BasicFutex () = default;

        /**
         * @brief lock the futex, blocking until it becomes available.
         */
        void lock () noexcept
        {
            LockPolicy::lock (_futex, FUTEX_PRIVATE_FLAG);
        }

        /**
         * @brief try to lock the futex without blocking.
         * @return true if the lock was acquired, false if it was already locked.
         */
        bool tryLock () noexcept
        {
            return LockPolicy::tryLock (_futex);
        }

        /**
         * @brief unlock the futex.
         */
        void unlock () noexcept
        {
            LockPolicy::unlock (_futex, FUTEX_PRIVATE_FLAG);
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
        /// futex word.
        alignas (4) std::atomic_uint32_t _futex;
    };

    /**
     * @brief Futex-based mutex suitable for inter-process locking via shared memory.
     */
    template <class LockPolicy>
    class BasicSharedFutex
    {
    public:
        /**
         * @brief default constructor.
         */
        BasicSharedFutex () noexcept
        : _futex (0)
        {
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicSharedFutex (const BasicSharedFutex& other) = delete;

        /**
         * @brief copy assignment.
         * @param other other object to copy.
         * @return a reference to the current object.
         */
        BasicSharedFutex& operator= (const BasicSharedFutex& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        BasicSharedFutex (BasicSharedFutex&& other) = delete;

        /**
         * @brief move assignment.
         * @param other other object to move.
         * @return a reference to the current object.
         */
        BasicSharedFutex& operator= (BasicSharedFutex&& other) = delete;

        /**
         * @brief destructor.
         */
        ~BasicSharedFutex () = default;

        /**
         * @brief lock the futex, blocking until it becomes available.
         */
        void lock () noexcept
        {
            LockPolicy::lock (_futex, 0);
        }

        /**
         * @brief try to lock the futex without blocking.
         * @return true if the lock was acquired, false if it was already locked.
         */
        bool tryLock () noexcept
        {
            return LockPolicy::tryLock (_futex);
        }

        /**
         * @brief unlock the futex.
         */
        void unlock () noexcept
        {
            LockPolicy::unlock (_futex, 0);
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
        /// futex word.
        alignas (4) std::atomic_uint32_t _futex;
    };

    /**
     * @brief no priority inheritance policy.
     */
    class PrioNone
    {
    public:
        /**
         * @brief lock the futex.
         * @param futex futex word.
         * @param flags futex flags.
         */
        static void lock (std::atomic_uint32_t& futex, int flags) noexcept
        {
            uint32_t expected = 0;
            if (futex.compare_exchange_strong (expected, 1, std::memory_order_acquire, std::memory_order_relaxed))
            {
                return;
            }

            do
            {
                if (expected == 2 ||
                    futex.compare_exchange_strong (expected, 2, std::memory_order_relaxed, std::memory_order_relaxed))
                {
                    ::syscall (SYS_futex, reinterpret_cast<uint32_t*> (&futex), FUTEX_WAIT | flags, 2, nullptr, nullptr,
                               0);
                }
                expected = 0;
            }
            while (!futex.compare_exchange_strong (expected, 2, std::memory_order_acquire, std::memory_order_relaxed));
        }

        /**
         * @brief try to lock the futex.
         * @param futex futex word.
         * @return true if locked, false otherwise.
         */
        static bool tryLock (std::atomic_uint32_t& futex) noexcept
        {
            uint32_t expected = 0;
            return futex.compare_exchange_strong (expected, 1, std::memory_order_acquire, std::memory_order_relaxed);
        }

        /**
         * @brief unlock the futex.
         * @param futex futex word.
         * @param flags futex flags.
         */
        static void unlock (std::atomic_uint32_t& futex, int flags) noexcept
        {
            if (futex.exchange (0, std::memory_order_release) == 2)
            {
                ::syscall (SYS_futex, reinterpret_cast<uint32_t*> (&futex), FUTEX_WAKE | flags, 1, nullptr, nullptr, 0);
            }
        }
    };

    /**
     * @brief priority inheritance policy.
     */
    class PrioInherit
    {
    public:
        /**
         * @brief lock the futex.
         * @param futex futex word.
         * @param flags futex flags.
         */
        static void lock (std::atomic_uint32_t& futex, int flags) noexcept
        {
            uint32_t expected = 0;
            if (futex.compare_exchange_strong (expected, static_cast<uint32_t> (Thread::currentId ()),
                                               std::memory_order_acquire, std::memory_order_relaxed))
            {
                return;
            }

            while (::syscall (SYS_futex, reinterpret_cast<uint32_t*> (&futex), FUTEX_LOCK_PI | flags, 0, nullptr,
                              nullptr, 0) == -1)
            {
                // LCOV_EXCL_START
                if (errno != EAGAIN)
                {
                    std::abort ();
                }
                // LCOV_EXCL_STOP
            }

            JOIN_TSAN_ACQUIRE (&futex);
        }

        /**
         * @brief try to lock the futex.
         * @param futex futex word.
         * @return true if locked, false otherwise.
         */
        static bool tryLock (std::atomic_uint32_t& futex) noexcept
        {
            uint32_t expected = 0;
            return futex.compare_exchange_strong (expected, static_cast<uint32_t> (Thread::currentId ()),
                                                  std::memory_order_acquire, std::memory_order_relaxed);
        }

        /**
         * @brief unlock the futex.
         * @param futex futex word.
         * @param flags futex flags.
         */
        static void unlock (std::atomic_uint32_t& futex, int flags) noexcept
        {
            uint32_t expected = static_cast<uint32_t> (Thread::currentId ());
            if (futex.compare_exchange_strong (expected, 0, std::memory_order_release, std::memory_order_relaxed))
            {
                return;
            }

            JOIN_TSAN_RELEASE (&futex);
            ::syscall (SYS_futex, reinterpret_cast<uint32_t*> (&futex), FUTEX_UNLOCK_PI | flags, 0, nullptr, nullptr,
                       0);
        }
    };

    using Futex = BasicFutex<PrioNone>;
    using PiFutex = BasicFutex<PrioInherit>;

    using SharedFutex = BasicSharedFutex<PrioNone>;
    using SharedPiFutex = BasicSharedFutex<PrioInherit>;
}

#endif
