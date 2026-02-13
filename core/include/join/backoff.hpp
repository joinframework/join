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

#ifndef __JOIN_BACKOFF_HPP__
#define __JOIN_BACKOFF_HPP__

// C++.
#include <algorithm>
#include <chrono>
#include <thread>

// C.
#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#endif
#include <cstddef>

namespace join
{
    /**
     * @brief adaptive backoff strategy for busy-wait loops.
     */
    class Backoff
    {
    public:
        /**
         * @brief construct a backoff strategy.
         * @param spin number of active spin iterations before yielding (default: 200).
         */
        Backoff (size_t spin = 200)
        : _spin (spin)
        , _count (0)
        {
        }

        /**
         * @brief execute one backoff iteration.
         */
        void operator()() noexcept
        {
            if (_count < _spin)
            {
            #if defined(__x86_64__) || defined(__i386__)
                _mm_pause ();
            #elif defined(__aarch64__) || defined(__arm__)
                __asm__ __volatile__ ("yield" ::: "memory");
            #endif
                ++_count;
            }
            else
            {
                std::this_thread::yield ();
            }
        }

        /**
         * @brief reset backoff to initial state.
         */
        void reset () noexcept
        {
            _count = 0;
        }

    private:
        /// number of spin iterations before yielding.
        size_t _spin;

        /// current iteration count.
        size_t _count;
    };
}

#endif
