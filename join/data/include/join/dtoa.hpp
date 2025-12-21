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

#ifndef __JOIN_DTOA_HPP__
#define __JOIN_DTOA_HPP__

// libjoin.
#include <join/dtoapow.hpp>

// C.
#include <cstring>
#include <cmath>

namespace join
{
    namespace details
    {
        inline char* writeExponent (char* buffer, int k)
        {
            *buffer = '-';
            buffer += (k < 0);
            k = (k < 0) ? -k : k;
            if (k >= 100)
            {
                *buffer++ = '0' + k / 100;
                k %= 100;
                *buffer++ = '0' + k / 10;
                k %= 10;
            }
            else if (k >= 10)
            {
                *buffer++ = '0' + k / 10;
                k %= 10;
            }
            *buffer++ = '0' + k;
            return buffer;
        }

        inline char* prettify (char* buffer, int length, int k)
        {
            int kk = length + k;

            if ((length <= kk) && (kk <= 21))
            {
                memset (buffer + length, '0', kk - length);
                buffer[kk] = '.';
                buffer[kk + 1] = '0';
                return &buffer[kk + 2];
            }
            else if ((0 < kk) && (kk <= 21))
            {
                memmove (&buffer[kk + 1], &buffer[kk], length - kk);
                buffer[kk] = '.';
                return &buffer[length + 1];
            }
            else if ((-6 < kk) && (kk <= 0))
            {
                int offset = 2 - kk;
                memmove (&buffer[offset], &buffer[0], length);
                buffer[0] = '0';
                buffer[1] = '.';
                memset (&buffer[2], '0', offset - 2);
                return &buffer[length + offset];
            }
            else if (length == 1)
            {
                buffer[1] = 'e';
                return writeExponent (&buffer[2], kk - 1);
            }
            else
            {
                memmove (&buffer[2], &buffer[1], length - 1);
                buffer[1] = '.';
                buffer[length + 1] = 'e';
                return writeExponent (&buffer[length + 2], kk - 1);
            }
        }

        inline void grisuRound (char* buffer, int length, uint64_t delta, uint64_t rest, uint64_t ten_kappa, uint64_t wp_w)
        {
            while (rest < wp_w && delta - rest >= ten_kappa && (rest + ten_kappa < wp_w || wp_w - rest > rest + ten_kappa - wp_w))
            {
                --buffer[length - 1];
                rest += ten_kappa;
            }
        }

        inline size_t digitsCount (uint32_t n)
        {
            if (n < 10) return 1;
            if (n < 100) return 2;
            if (n < 1000) return 3;
            if (n < 10000) return 4;
            if (n < 100000) return 5;
            if (n < 1000000) return 6;
            if (n < 10000000) return 7;
            if (n < 100000000) return 8;
            if (n < 1000000000) return 9;
            return 10;
        }

        inline void digitsGen (DiyFp W, DiyFp Mp, uint64_t delta, char* buffer, int& length, int& k)
        {
            static const uint32_t kPow10[] = {1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000};
            DiyFp one (static_cast <uint64_t> (1) << -Mp._exponent, Mp._exponent);
            DiyFp wp_w = Mp - W;
            uint32_t p1 = static_cast <uint32_t> (Mp._mantissa >> -one._exponent);
            uint64_t p2 = Mp._mantissa & (one._mantissa - 1);
            int kappa = static_cast <int> (digitsCount (p1));
            length = 0;

            while (kappa > 0)
            {
                uint32_t d = 0;
                switch (kappa)
                {
                    case 10: d = p1 / 1000000000; p1 %= 1000000000; break;
                    case  9: d = p1 / 100000000;  p1 %= 100000000;  break;
                    case  8: d = p1 / 10000000;   p1 %= 10000000;   break;
                    case  7: d = p1 / 1000000;    p1 %= 1000000;    break;
                    case  6: d = p1 / 100000;     p1 %= 100000;     break;
                    case  5: d = p1 / 10000;      p1 %= 10000;      break;
                    case  4: d = p1 / 1000;       p1 %= 1000;       break;
                    case  3: d = p1 / 100;        p1 %= 100;        break;
                    case  2: d = p1 / 10;         p1 %= 10;         break;
                    case  1: d = p1;              p1  = 0;          break;
                    default:                                        break;
                }
                if (d || length)
                {
                    buffer[length++] = '0' + d;
                }
                --kappa;
                uint64_t tmp = (static_cast <uint64_t> (p1) << -one._exponent) + p2;
                if (tmp <= delta)
                {
                    k += kappa;
                    grisuRound (buffer, length, delta, tmp, static_cast <uint64_t> (kPow10[kappa]) << -one._exponent, wp_w._mantissa);
                    return;
                }
            }

            uint64_t unit = 1;
            for (;;)
            {
                p2 *= 10; delta *= 10; unit *= 10;
                char d = static_cast <char> (p2 >> -one._exponent);
                if (d || length)
                {
                    buffer[length++] = '0' + d;
                }
                p2 &= one._mantissa - 1;
                --kappa;
                if (p2 < delta)
                {
                    k += kappa;
                    grisuRound (buffer, length, delta, p2, one._mantissa, wp_w._mantissa * unit);
                    return;
                }
            }
        }

        inline int kComputation (int exp, int alpha)
        {
            return static_cast <int> (::ceil ((alpha - exp + 63) * 0.30102999566398114));
        }

        inline void grisu2 (char* buffer, int& length, int& k, double value)
        {
            DiyFp val (value), minus, plus;
            val.normalizedBoundaries (minus, plus);

            int mk = kComputation (plus._exponent + 64, -59);
            const DiyFp& c_mk = dtoapow[mk + 343];

            minus *= c_mk;
            plus  *= c_mk;

            ++minus._mantissa;
            --plus._mantissa;

            k = -mk;

            digitsGen (val.normalize () * c_mk, plus, plus._mantissa - minus._mantissa, buffer, length, k);
        }
    }

    /**
     * @brief double to string conversion.
     * @param buffer buffer to write the string representation to.
     * @param value value to convert.
     * @return end position.
     */
    inline char* dtoa (char* buffer, double value)
    {
        uint64_t bits;
        memcpy (&bits, &value, sizeof(double));
        bool is_negative = (bits >> 63) != 0;

        *buffer = '-';
        buffer += is_negative;
        value = is_negative ? -value : value;

        if (value == 0.0)
        {
            memcpy (buffer, "0.0", 3);
            return buffer + 3;
        }

        int length = 0, k = 0;
        details::grisu2 (buffer, length, k, value);
        return details::prettify (buffer, length, k);
    }
}

#endif
