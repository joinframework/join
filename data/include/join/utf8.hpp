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

#ifndef JOIN_DATA_UTF8_HPP
#define JOIN_DATA_UTF8_HPP

// libjoin.
#include <join/utils.hpp>

// C++.
#include <algorithm>

// C.
#include <cstring>
#include <cstddef>
#include <cstdint>

namespace join
{
    namespace utf8
    {
        namespace details
        {
            /**
             * @brief UTF-8 well-formed multibyte sequences indexed by lead byte.
             */
            struct alignas (64) SequenceTable
            {
                uint8_t data[256]; /**< length in bits 0-2, 0 if not a multibyte lead, range in bits 3-5. */
                uint8_t lower[8];  /**< lowest allowed second byte per range. */
                uint8_t span[8];   /**< allowed second byte span per range. */

                /**
                 * @brief create the lookup table.
                 */
                constexpr SequenceTable ()
                : data{}
                , lower{0x80, 0xA0, 0x80, 0x90, 0x80}
                , span{0x3F, 0x1F, 0x1F, 0x2F, 0x0F}
                {
                    for (unsigned i = 0xC2; i < 0xE0; ++i)
                    {
                        data[i] = 2;
                    }
                    for (unsigned i = 0xE1; i < 0xF0; ++i)
                    {
                        data[i] = 3;
                    }
                    for (unsigned i = 0xF1; i < 0xF4; ++i)
                    {
                        data[i] = 4;
                    }
                    data[0xE0] = 3 | (1 << 3);
                    data[0xED] = 3 | (2 << 3);
                    data[0xF0] = 4 | (3 << 3);
                    data[0xF4] = 4 | (4 << 3);
                }
            };

            /**
             * @brief UTF-8 multibyte sequences lookup table.
             */
            constexpr SequenceTable sequenceLookup{};

            /**
             * @brief rank of UTF-8 bytes in UTF-16 code unit order.
             */
            struct alignas (64) OrderTable
            {
                uint8_t data[256]; /**< rank indexed by byte. */

                /**
                 * @brief create the lookup table.
                 */
                constexpr OrderTable ()
                : data{}
                {
                    for (unsigned i = 0; i < 0xEE; ++i)
                    {
                        data[i] = static_cast<uint8_t> (i);
                    }
                    for (unsigned i = 0xF0; i < 0x100; ++i)
                    {
                        data[i] = static_cast<uint8_t> (i - 2);
                    }
                    data[0xEE] = 0xFE;
                    data[0xEF] = 0xFF;
                }
            };

            /**
             * @brief UTF-16 code unit order lookup table.
             */
            constexpr OrderTable orderLookup{};

            /**
             * @brief mask of the high bit of each byte of a 64 bits word.
             */
            constexpr uint64_t highBits = 0x8080808080808080ULL;

            /**
             * @brief get the length of a multibyte sequence.
             * @param cur lead byte of the sequence.
             * @param end end of the input.
             * @return sequence length, 0 if the sequence is ill-formed.
             */
            inline uint32_t sequenceLength (const uint8_t* cur, const uint8_t* end) noexcept
            {
                uint32_t entry = sequenceLookup.data[cur[0]];
                uint32_t n = entry & 7;

                if (JOIN_UNLIKELY (n == 0 || static_cast<size_t> (end - cur) < n))
                {
                    return 0;
                }

                uint32_t range = entry >> 3;
                uint32_t bad = static_cast<uint8_t> (cur[1] - sequenceLookup.lower[range]) > sequenceLookup.span[range];

                if (JOIN_LIKELY (end - cur >= 4))
                {
                    bad |= (n >= 3 ? (cur[2] & 0xC0) ^ 0x80 : 0) | (n == 4 ? (cur[3] & 0xC0) ^ 0x80 : 0);
                }
                else
                {
                    for (uint32_t i = 2; i < n; ++i)
                    {
                        bad |= (cur[i] & 0xC0) ^ 0x80;
                    }
                }

                return bad ? 0 : n;
            }

            /**
             * @brief validate UTF-8 input that may contain multibyte sequences.
             * @param cur first byte of the input.
             * @param end end of the input.
             * @return 0 on success, -1 otherwise.
             */
            inline int validateMixed (const uint8_t* cur, const uint8_t* end) noexcept
            {
                while (cur < end)
                {
                    if (JOIN_LIKELY (end - cur >= 8))
                    {
                        uint64_t word;
                        std::memcpy (&word, cur, 8);
                        if (JOIN_LIKELY ((word & highBits) == 0))
                        {
                            cur += 8;
                            continue;
                        }
                    }

                    if (*cur < 0x80)
                    {
                        ++cur;
                        continue;
                    }

                    do
                    {
                        uint32_t n = sequenceLength (cur, end);
                        if (JOIN_UNLIKELY (n == 0))
                        {
                            return -1;
                        }
                        cur += n;
                    }
                    while (cur < end && *cur >= 0x80);
                }

                return 0;
            }
        }

        /**
         * @brief decode one codepoint from UTF-8.
         * @param cur current byte, must be lower than end, advanced past the sequence on success.
         * @param end end of the input.
         * @param codepoint decoded codepoint.
         * @return 0 on success, -1 otherwise.
         */
        inline int decode (const char*& cur, const char* end, uint32_t& codepoint) noexcept
        {
            const uint8_t* p = reinterpret_cast<const uint8_t*> (cur);

            if (JOIN_LIKELY (p[0] < 0x80))
            {
                codepoint = p[0];
                ++cur;
                return 0;
            }

            uint32_t n = details::sequenceLength (p, reinterpret_cast<const uint8_t*> (end));
            if (JOIN_UNLIKELY (n == 0))
            {
                return -1;
            }

            uint32_t cp = p[0] & (0xFF >> (n + 1));
            for (uint32_t i = 1; i < n; ++i)
            {
                cp = (cp << 6) | (p[i] & 0x3F);
            }

            codepoint = cp;
            cur += n;
            return 0;
        }

        /**
         * @brief encode one codepoint to UTF-8.
         * @param codepoint unicode scalar value to encode.
         * @param out output buffer of at least 4 bytes.
         * @return number of bytes written.
         */
        inline size_t encode (uint32_t codepoint, char* out) noexcept
        {
            if (JOIN_LIKELY (codepoint < 0x80))
            {
                out[0] = static_cast<char> (codepoint);
                return 1;
            }

            if (codepoint < 0x800)
            {
                out[0] = static_cast<char> (0xC0 | (codepoint >> 6));
                out[1] = static_cast<char> (0x80 | (codepoint & 0x3F));
                return 2;
            }

            if (codepoint < 0x10000)
            {
                out[0] = static_cast<char> (0xE0 | (codepoint >> 12));
                out[1] = static_cast<char> (0x80 | ((codepoint >> 6) & 0x3F));
                out[2] = static_cast<char> (0x80 | (codepoint & 0x3F));
                return 3;
            }

            out[0] = static_cast<char> (0xF0 | (codepoint >> 18));
            out[1] = static_cast<char> (0x80 | ((codepoint >> 12) & 0x3F));
            out[2] = static_cast<char> (0x80 | ((codepoint >> 6) & 0x3F));
            out[3] = static_cast<char> (0x80 | (codepoint & 0x3F));
            return 4;
        }

        /**
         * @brief validate UTF-8 input.
         * @param first first byte of the input.
         * @param last end of the input.
         * @return 0 on success, -1 otherwise.
         */
        inline int validate (const char* first, const char* last) noexcept
        {
            const uint8_t* cur = reinterpret_cast<const uint8_t*> (first);
            const uint8_t* end = reinterpret_cast<const uint8_t*> (last);
            uint64_t word;

            if (end - cur < 8)
            {
                uint32_t acc = 0;
                for (const uint8_t* p = cur; p < end; ++p)
                {
                    acc |= *p;
                }
                return JOIN_LIKELY (acc < 0x80) ? 0 : details::validateMixed (cur, end);
            }

            while (end - cur >= 8)
            {
                std::memcpy (&word, cur, 8);
                if (JOIN_UNLIKELY (word & details::highBits))
                {
                    return details::validateMixed (cur, end);
                }
                cur += 8;
            }

            std::memcpy (&word, end - 8, 8);
            return JOIN_LIKELY ((word & details::highBits) == 0) ? 0 : details::validateMixed (cur, end);
        }

        /**
         * @brief compare two UTF-8 inputs in UTF-16 code unit order.
         * @param first1 first byte of the first input.
         * @param last1 end of the first input.
         * @param first2 first byte of the second input.
         * @param last2 end of the second input.
         * @return negative value if the first input is ordered before the second, 0 if equal, positive value otherwise.
         */
        inline int compareUtf16Order (const char* first1, const char* last1, const char* first2,
                                      const char* last2) noexcept
        {
            auto mis = std::mismatch (first1, last1, first2, last2);

            if (mis.first == last1)
            {
                return (mis.second == last2) ? 0 : -1;
            }

            if (mis.second == last2)
            {
                return 1;
            }

            return static_cast<int> (details::orderLookup.data[static_cast<uint8_t> (*mis.first)]) -
                   static_cast<int> (details::orderLookup.data[static_cast<uint8_t> (*mis.second)]);
        }
    }
}

#endif
