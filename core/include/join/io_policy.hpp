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

    template <typename...>
    using void_t = void;

    template <typename T, typename = void>
    struct has_sqpoll : std::false_type
    {
    };

    template <typename T>
    struct has_sqpoll<T, void_t<decltype (T::flags)>>
    : std::integral_constant<bool, bool (T::flags& IORING_SETUP_SQPOLL)>
    {
    };

    template <typename T, typename = void>
    struct has_cq_entries : std::false_type
    {
    };

    template <typename T>
    struct has_cq_entries<T, void_t<decltype (T::cqEntries)>> : std::true_type
    {
    };

    template <typename T, typename = void>
    struct has_sq_thread_idle : std::false_type
    {
    };

    template <typename T>
    struct has_sq_thread_idle<T, void_t<decltype (T::sqThreadIdle)>> : std::true_type
    {
    };

    template <typename T, typename = void>
    struct has_sq_thread_cpu : std::false_type
    {
    };

    template <typename T>
    struct has_sq_thread_cpu<T, void_t<decltype (T::sqThreadCpu)>> : std::true_type
    {
    };

    template <typename T, typename = void>
    struct has_spin : std::false_type
    {
    };

    template <typename T>
    struct has_spin<T, void_t<decltype (T::spinNs)>> : std::true_type
    {
    };

    template <typename T>
    struct is_default : std::integral_constant<bool, !has_spin<T>::value && !has_sqpoll<T>::value>
    {
    };

    template <typename T, bool = has_spin<T>::value>
    struct spin_ns : std::integral_constant<uint64_t, 10'000>
    {
    };

    template <typename T>
    struct spin_ns<T, true> : std::integral_constant<uint64_t, T::spinNs>
    {
    };

    template <typename T, typename = void>
    struct has_yield_ns : std::false_type
    {
    };

    template <typename T>
    struct has_yield_ns<T, void_t<decltype (T::yieldNs)>> : std::true_type
    {
    };

    template <typename T, bool = has_yield_ns<T>::value>
    struct yield_ns : std::integral_constant<uint64_t, 100'000>
    {
    };

    template <typename T>
    struct yield_ns<T, true> : std::integral_constant<uint64_t, T::yieldNs>
    {
    };

    template <typename T, typename = void>
    struct has_sleep_ns : std::false_type
    {
    };

    template <typename T>
    struct has_sleep_ns<T, void_t<decltype (T::sleepNs)>> : std::true_type
    {
    };

    template <typename T, bool = has_sleep_ns<T>::value>
    struct sleep_ns : std::integral_constant<uint64_t, 0>
    {
    };

    template <typename T>
    struct sleep_ns<T, true> : std::integral_constant<uint64_t, T::sleepNs>
    {
    };

    template <typename T, typename = void>
    struct has_tick_ns : std::false_type
    {
    };

    template <typename T>
    struct has_tick_ns<T, void_t<decltype (T::tickNs)>> : std::true_type
    {
    };

    template <typename T, bool = has_tick_ns<T>::value>
    struct tick_ns : std::integral_constant<uint64_t, 1'000>
    {
    };

    template <typename T>
    struct tick_ns<T, true> : std::integral_constant<uint64_t, T::tickNs>
    {
    };
}

#endif
