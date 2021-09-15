/**
 * MIT License
 *
 * Copyright (c) 2021 Mathieu Rabine
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

// C++.
#include <stdexcept>

// C.
#include <endian.h>
#include <cstdint>
#include <cstddef>

namespace join
{
namespace utils
{
    namespace details
    {
        template <typename T, size_t sz>
        struct _byteswap
        {
            inline T operator() (T val)
            {
                throw std::out_of_range ("data size");
            }
        };

        template <typename T>
        struct _byteswap <T, 1>
        {
            inline T operator() (T val)
            {
                return val;
            }
        };

        template <typename T>
        struct _byteswap <T, 2>
        {
            inline T operator() (T val)
            {
                if (BYTE_ORDER == LITTLE_ENDIAN)
                {
                    return (val >> 8) | (val << 8);
                }
                return val;
            }
        };

        template <typename T>
        struct _byteswap <T, 4>
        {
            inline T operator() (T val)
            {
                if (BYTE_ORDER == LITTLE_ENDIAN)
                {
                    return ((val & 0xff000000) >> 24) |
                           ((val & 0x00ff0000) >> 8 ) |
                           ((val & 0x0000ff00) << 8 ) |
                           ((val & 0x000000ff) << 24);
                }
                return val;
            }
        };

        template <typename T>
        struct _byteswap <T, 8>
        {
            inline T operator() (T val)
            {
                if (BYTE_ORDER == LITTLE_ENDIAN)
                {
                    return ((val & 0xff00000000000000ull) >> 56) |
                           ((val & 0x00ff000000000000ull) >> 40) |
                           ((val & 0x0000ff0000000000ull) >> 24) |
                           ((val & 0x000000ff00000000ull) >> 8 ) |
                           ((val & 0x00000000ff000000ull) << 8 ) |
                           ((val & 0x0000000000ff0000ull) << 24) |
                           ((val & 0x000000000000ff00ull) << 40) |
                           ((val & 0x00000000000000ffull) << 56);
                }
                return val;
            }
        };

        template <>
        struct _byteswap <float, 4>
        {
            inline float operator() (float val)
            {
                if (BYTE_ORDER == LITTLE_ENDIAN)
                {
                    union { float f; uint32_t i; } tmp; tmp.f = val;
                    tmp.i = _byteswap <uint32_t, sizeof (uint32_t)> ()(tmp.i);
                    return tmp.f;
                }
                return val;
            }
        };

        template<>
        struct _byteswap <double, 8>
        {
            inline double operator() (double val)
            {
                if (BYTE_ORDER == LITTLE_ENDIAN)
                {
                    union { double f; uint64_t i; } tmp; tmp.f = val;
                    tmp.i = _byteswap <uint64_t, sizeof (uint64_t)> ()(tmp.i);
                    return tmp.f;
                }
                return val;
            }
        };

        template <class T>
        struct _swap
        {
            inline T operator() (T val)
            {
                return _byteswap <T, sizeof (T)> ()(val);
            }
        };
    }

    /**
     * @brief swaps byte orders.
     * @param val value to swap.
     * @return the swapped value.
     */
    template <class T>
    inline T& swap (T& val)
    {
        val = details::_swap <T> ()(val);
        return val;
    }
}
}
