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

#ifndef JOIN_CORE_IO_POLICY_HPP
#define JOIN_CORE_IO_POLICY_HPP

// C.
#include <liburing.h>
#include <cstdint>

namespace join
{
    struct IoDefaultPolicy
    {
        static constexpr uint32_t sqEntries = 1024;
        static constexpr uint32_t flags = 0;
    };

    struct IoHybridPolicy
    {
        static constexpr uint32_t sqEntries = 1024;
        static constexpr uint32_t flags = 0;
        static constexpr uint64_t spinNs = 10'000;
        static constexpr uint64_t yieldNs = 100'000;
        static constexpr uint64_t sleepNs = 0;
        static constexpr uint64_t tickNs = 100;
    };

    struct IoSqpollPolicy
    {
        static constexpr uint32_t sqEntries = 1024;
        static constexpr uint32_t flags = IORING_SETUP_SQPOLL;
        static constexpr uint32_t sqThreadIdle = 2000;
        static constexpr uint64_t spinNs = 10'000;
        static constexpr uint64_t yieldNs = 100'000;
        static constexpr uint64_t sleepNs = 0;
        static constexpr uint64_t tickNs = 100;
    };

    template <typename T>
    inline constexpr bool has_sqpoll_v = (T::flags & IORING_SETUP_SQPOLL) != 0;

    template <typename T>
    inline constexpr bool has_cq_entries_v = requires { T::cqEntries; };

    template <typename T>
    inline constexpr bool has_sq_thread_idle_v = requires { T::sqThreadIdle; };

    template <typename T>
    inline constexpr bool has_sq_thread_cpu_v = requires { T::sqThreadCpu; };

    template <typename T>
    inline constexpr bool has_spin_v = requires { T::spinNs; };

    template <typename T>
    inline constexpr bool is_default_v = !has_spin_v<T> && !has_sqpoll_v<T>;

    template <typename T>
    inline constexpr uint64_t spin_ns_v = 10'000;

    template <typename T>
        requires has_spin_v<T>
    inline constexpr uint64_t spin_ns_v<T> = T::spinNs;

    template <typename T>
    inline constexpr bool has_yield_ns_v = requires { T::yieldNs; };

    template <typename T>
    inline constexpr uint64_t yield_ns_v = 100'000;

    template <typename T>
        requires has_yield_ns_v<T>
    inline constexpr uint64_t yield_ns_v<T> = T::yieldNs;

    template <typename T>
    inline constexpr bool has_sleep_ns_v = requires { T::sleepNs; };

    template <typename T>
    inline constexpr uint64_t sleep_ns_v = 0;

    template <typename T>
        requires has_sleep_ns_v<T>
    inline constexpr uint64_t sleep_ns_v<T> = T::sleepNs;

    template <typename T>
    inline constexpr bool has_tick_ns_v = requires { T::tickNs; };

    template <typename T>
    inline constexpr uint64_t tick_ns_v = 1'000;

    template <typename T>
        requires has_tick_ns_v<T>
    inline constexpr uint64_t tick_ns_v<T> = T::tickNs;
}

#endif
