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
        static constexpr uint32_t spin = 200;
    };

    struct IoSqpollPolicy
    {
        static constexpr uint32_t sqEntries = 1024;
        static constexpr uint32_t flags = IORING_SETUP_SQPOLL;
        static constexpr uint32_t spin = 200;
        static constexpr uint32_t sqThreadIdle = 2000;
        static constexpr uint32_t sqThreadCpu = 0;
    };

    template <typename...>
    using void_t = void;

    template <typename T, typename = void>
    struct has_spin : std::false_type
    {
    };

    template <typename T>
    struct has_spin<T, void_t<decltype (T::spin)>> : std::true_type
    {
    };

    template <typename T, typename = void>
    struct has_sqpoll : std::false_type
    {
    };

    template <typename T>
    struct has_sqpoll<T, void_t<decltype (T::flags)>>
    : std::integral_constant<bool, bool (T::flags& IORING_SETUP_SQPOLL)>
    {
    };

    template <typename T>
    struct is_default : std::integral_constant<bool, !has_spin<T>::value && !has_sqpoll<T>::value>
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
}

#endif
