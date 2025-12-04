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

#ifndef __JOIN_DIYFP_HPP__
#define __JOIN_DIYFP_HPP__

// C++.
#include <utility>
#include <limits>

// C.
#include <cstdint>
#include <cstring>

namespace join
{
    /**
     * @brief hand made floating point.
     */
    class DiyFp
    {
    public:
        /**
         * @brief default constructor.
         */
        constexpr DiyFp () noexcept = default;

        /**
         * @brief copy constructor.
         * @param other object to copy.
         */
        constexpr DiyFp (const DiyFp& other) noexcept = default;

        /**
         * @brief copy assignment.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        constexpr DiyFp& operator= (const DiyFp& other) noexcept = default;

        /**
         * @brief create floating point using double.
         * @param value double value.
         */
        explicit constexpr DiyFp (double value) noexcept
        {
            uint64_t u64 = 0;
            memcpy (&u64, &value, sizeof (double));

            _mantissa = u64 & _mantissaMask;
            _exponent = static_cast <int> ((u64 & _exponentMask) >> _mantissaSize);

            if (_exponent)
            {
                _mantissa += _hiddenBit;
                _exponent -= _exponentBias;
            }
            else
            {
                _exponent = 1 - _exponentBias;
            }
        }

        /**
         * @brief create floating point using mantissa and exponent.
         * @param mantissa mantissa.
         * @param exponent exponent.
         */
        constexpr DiyFp (uint64_t mantissa, int exponent) noexcept
        : _mantissa (mantissa),
          _exponent (exponent)
        {
        }

        /**
         * @brief destroy instance.
         */
        ~DiyFp () = default;

        /**
         * @brief normalize floating point.
         * @return a reference of the current object.
         */
        inline constexpr DiyFp& normalize () noexcept
        {
            if (_mantissa == 0)
            {
                return *this;
            }

            int shift = __builtin_clzll (_mantissa);
            _mantissa <<= shift;
            _exponent  -= shift;

            return *this;
        }

    private:
        /**
         * @brief normalize boundary.
         * @return a reference of the current object.
         */
        inline constexpr DiyFp& normalizeBoundary () noexcept
        {
            if (_mantissa != 0)
            {
                int shift = __builtin_clzll (_mantissa) - (64 - _mantissaSize - 2);
                if (shift > 0)
                {
                    _mantissa <<= shift;
                    _exponent  -= shift;
                }
            }

            constexpr int shift = _diyMantissaSize - _mantissaSize - 2;
            _mantissa <<= shift;
            _exponent  -= shift;

            return *this;
        }

    public:
        /**
         * @brief get normalized boundaries.
         * @return normalized boundaries.
         */
        constexpr void normalizedBoundaries (DiyFp& minus, DiyFp& plus) const noexcept
        {
            plus._mantissa = (_mantissa << 1) + 1;
            plus._exponent = _exponent - 1;
            plus.normalizeBoundary ();

            const bool special = __builtin_expect (_mantissa == _hiddenBit, 0);
            minus._mantissa = (_mantissa << (special ? 2 : 1)) - 1;
            minus._exponent = _exponent - (special ? 2 : 1);

            const int diff = minus._exponent - plus._exponent;
            minus._mantissa <<= diff;
            minus._exponent = plus._exponent;
        }

        /**
         * @brief minus operator
         * @param rhs floating point.
         * @return a reference of the current object.
         */
        inline constexpr DiyFp& operator-= (const DiyFp& rhs) noexcept
        {
            _mantissa -= rhs._mantissa;
            return *this;
        }

        /**
         * @brief multiplication operator.
         * @param rhs floating point.
         * @return a reference of the current object.
         */
        inline constexpr DiyFp& operator*= (const DiyFp& rhs) noexcept
        {
        #if defined(__SIZEOF_INT128__)
            __uint128_t product = static_cast <__uint128_t> (_mantissa) * static_cast <__uint128_t> (rhs._mantissa);
            _mantissa = static_cast <uint64_t> ((product >> 64) + ((product >> 63) & 1));
        #else
            uint64_t M32 = 0xFFFFFFFFU;

            uint64_t a = _mantissa >> 32;
            uint64_t b = _mantissa & M32;
            uint64_t c = rhs._mantissa >> 32;
            uint64_t d = rhs._mantissa & M32;

            uint64_t ac = a * c;
            uint64_t bc = b * c;
            uint64_t ad = a * d;
            uint64_t bd = b * d;

            uint64_t tmp = (bd >> 32) + (ad & M32) + (bc & M32) + (1U << 31);
            _mantissa = ac + (ad >> 32) + (bc >> 32) + (tmp >> 32);
        #endif
            _exponent += rhs._exponent + 64;

            return *this;
        }

        /// home made double mantissa size.
        static constexpr int _diyMantissaSize = std::numeric_limits <uint64_t>::digits;

        /// double mantissa size.
        static constexpr int _mantissaSize = std::numeric_limits <double>::digits - 1;

        /// exponent bias.
        static constexpr int _exponentBias = 0x3FF + _mantissaSize;

        /// mantissa mask.
        static constexpr uint64_t _mantissaMask = 0x000FFFFFFFFFFFFFLLU;

        /// exponent mask.
        static constexpr uint64_t _exponentMask = 0x7FF0000000000000LLU;

        /// hidden bit.
        static constexpr uint64_t _hiddenBit = 0x0010000000000000LLU;

        /// mantissa.
        uint64_t _mantissa = 0;

        /// exponent.
        int _exponent = 0;
    };

    /**
     * @brief minus operator
     * @param lhs floating point.
     * @param rhs floating point.
     * @return a floating point from lhs minus rhs.
     */
    inline constexpr DiyFp operator- (const DiyFp& lhs, const DiyFp& rhs)
    {
        return DiyFp (lhs) -= rhs;
    }

    /**
     * @brief multiplication operator.
     * @param lhs floating point.
     * @param rhs floating point.
     * @return a floating point from lhs multiplied by rhs.
     */
    inline constexpr DiyFp operator* (const DiyFp& lhs, const DiyFp& rhs)
    {
        return DiyFp (lhs) *= rhs;
    }
}

#endif
