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

#ifndef JOIN_DATA_UTF16_HPP
#define JOIN_DATA_UTF16_HPP

// libjoin.
#include <join/utils.hpp>

// C.
#include <cstddef>
#include <cstdint>

namespace join
{
    namespace utf16
    {
        /**
         * @brief check if code unit is a surrogate.
         * @param unit code unit to check.
         * @return true if surrogate, false otherwise.
         */
        constexpr bool isSurrogate (uint32_t unit) noexcept
        {
            return (unit - 0xD800) < 0x800;
        }

        /**
         * @brief check if code unit is a high surrogate.
         * @param unit code unit to check.
         * @return true if high surrogate, false otherwise.
         */
        constexpr bool isHighSurrogate (uint32_t unit) noexcept
        {
            return (unit - 0xD800) < 0x400;
        }

        /**
         * @brief check if code unit is a low surrogate.
         * @param unit code unit to check.
         * @return true if low surrogate, false otherwise.
         */
        constexpr bool isLowSurrogate (uint32_t unit) noexcept
        {
            return (unit - 0xDC00) < 0x400;
        }

        /**
         * @brief get the high surrogate of a supplementary codepoint.
         * @param codepoint codepoint above U+FFFF.
         * @return high surrogate.
         */
        constexpr uint32_t highSurrogate (uint32_t codepoint) noexcept
        {
            return 0xD800 + ((codepoint - 0x10000) >> 10);
        }

        /**
         * @brief get the low surrogate of a supplementary codepoint.
         * @param codepoint codepoint above U+FFFF.
         * @return low surrogate.
         */
        constexpr uint32_t lowSurrogate (uint32_t codepoint) noexcept
        {
            return 0xDC00 + (codepoint & 0x3FF);
        }

        /**
         * @brief combine a surrogate pair into a codepoint.
         * @param high high surrogate.
         * @param low low surrogate.
         * @return codepoint.
         */
        constexpr uint32_t combine (uint32_t high, uint32_t low) noexcept
        {
            return 0x10000 + (((high - 0xD800) << 10) | (low - 0xDC00));
        }

        /**
         * @brief decode one codepoint from UTF-16.
         * @param cur current code unit, must be lower than end, advanced past the sequence on success.
         * @param end end of the input.
         * @param codepoint decoded codepoint.
         * @return 0 on success, -1 otherwise.
         */
        inline int decode (const char16_t*& cur, const char16_t* end, uint32_t& codepoint) noexcept
        {
            uint32_t u0 = cur[0];

            if (JOIN_LIKELY (!isSurrogate (u0)))
            {
                codepoint = u0;
                ++cur;
                return 0;
            }

            if (JOIN_UNLIKELY (!isHighSurrogate (u0) || (end - cur) < 2 || !isLowSurrogate (cur[1])))
            {
                return -1;
            }

            codepoint = combine (u0, cur[1]);
            cur += 2;
            return 0;
        }

        /**
         * @brief encode one codepoint to UTF-16.
         * @param codepoint unicode scalar value to encode.
         * @param out output buffer of at least 2 code units.
         * @return number of code units written.
         */
        inline size_t encode (uint32_t codepoint, char16_t* out) noexcept
        {
            if (JOIN_LIKELY (codepoint < 0x10000))
            {
                out[0] = static_cast<char16_t> (codepoint);
                return 1;
            }

            out[0] = static_cast<char16_t> (highSurrogate (codepoint));
            out[1] = static_cast<char16_t> (lowSurrogate (codepoint));
            return 2;
        }
    }
}

#endif
