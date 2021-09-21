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
         * @brief create floating point using double.
         * @param value double value.
         */
        constexpr DiyFp (double value) noexcept
        {
            union
            {
                double d;
                uint64_t u;
            } tmp = { value };

            _mantissa = (tmp.u & _mantissaMask);
            _exponent = (tmp.u & _exponentMask) >> _mantissaSize;

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
         * @brief destroy instance.
         */
        virtual ~DiyFp () = default;

        /**
         * @brief normalize floating point.
         * @return a reference of the current object.
         */
        constexpr DiyFp& normalize () noexcept
        {
            while ((_mantissa & _hiddenBit) == 0)
            {
                _mantissa <<= 1;
                --_exponent;
            }

            _mantissa <<= (_diyMantissaSize - _mantissaSize - 1);
            _exponent -= (_diyMantissaSize - _mantissaSize - 1);

            return *this;
        }

    private:
        /**
         * @brief normalize boundary.
         * @return a reference of the current object.
         */
        constexpr DiyFp& normalizeBoundary () noexcept
        {
            while ((_mantissa & (_hiddenBit << 1)) == 0)
            {
                _mantissa <<= 1;
                --_exponent;
            }

            _mantissa <<= (_diyMantissaSize - _mantissaSize - 2);
            _exponent -= (_diyMantissaSize - _mantissaSize - 2);

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

            if (_mantissa == _hiddenBit)
            {
                minus._mantissa = (_mantissa << 2) - 1;
                minus._exponent = _exponent - 2;
            }
            else
            {
                minus._mantissa = (_mantissa << 1) - 1;
                minus._exponent = _exponent - 1;
            }

            minus._mantissa <<= minus._exponent - plus._exponent;
            minus._exponent = plus._exponent;
        }

        /**
         * @brief minus operator
         * @param rhs floating point.
         * @return a reference of the current object.
         */
        constexpr DiyFp& operator-= (const DiyFp& rhs) noexcept
        {
            _mantissa -= rhs._mantissa;

            return *this;
        }

        /**
         * @brief multiplication operator.
         * @param rhs floating point.
         * @return a reference of the current object.
         */
        constexpr DiyFp& operator*= (const DiyFp& rhs) noexcept
        {
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
    inline DiyFp operator- (const DiyFp& lhs, const DiyFp& rhs)
    {
        return DiyFp (lhs) -= rhs;
    }

    /**
     * @brief multiplication operator.
     * @param lhs floating point.
     * @param rhs floating point.
     * @return a floating point from lhs multiplied by rhs.
     */
    inline DiyFp operator* (const DiyFp& lhs, const DiyFp& rhs)
    {
        return DiyFp (lhs) *= rhs;
    }
}

#endif
