/**
 * MIT License
 *
 * Copyright (c) 2025 Mathieu Rabine
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
 * LIABILITY, WHETHER IN AN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __JOIN_LOOKUP_HPP__
#define __JOIN_LOOKUP_HPP__

#include <array>

#include <cstddef>

namespace join
{
    namespace details
    {
        constexpr char digitPairs[201] = {
            "00010203040506070809"
            "10111213141516171819"
            "20212223242526272829"
            "30313233343536373839"
            "40414243444546474849"
            "50515253545556575859"
            "60616263646566676869"
            "70717273747576777879"
            "80818283848586878889"
            "90919293949596979899"
        };

        struct UnescapedTable
        {
            uint8_t data[256];
            constexpr UnescapedTable () : data {}
            {
                for (int i = 0; i < 256; ++i) { data[i] = 0; }
                for (int i = 0; i < 32; ++i)  { data[i] = 'u'; }
                data['\b'] = 'b';
                data['\t'] = 't';
                data['\n'] = 'n';
                data['\f'] = 'f';
                data['\r'] = 'r';
                data['"']  = '"';
                data['\\'] = '\\';
            }
            constexpr const uint8_t& operator[] (size_t i) const { return data[i]; }
        };
        constexpr UnescapedTable unescapedLookup {};

        struct EscapedTable
        {
            uint8_t data[256];
            constexpr EscapedTable () : data {}
            {
                for (int i = 0; i < 256; ++i)  { data[i] = 0; }
                for (int i = 0; i < 0x20; ++i) { data[i] = 1; }

                data['"']  = 1;
                data['\\'] = 1;
            }
            constexpr const uint8_t& operator[] (size_t i) const { return data[i]; }
        };
        constexpr EscapedTable escapedLookup {};

        struct WhitespaceTable
        {
            uint8_t data[256];
            constexpr WhitespaceTable () : data {}
            {
                for (int i = 0; i < 256; ++i) { data[i] = 0; }
                data['\t'] = 1;
                data['\n'] = 1;
                data['\r'] = 1;
                data[' ']  = 1;
            }
            constexpr const uint8_t& operator[] (size_t i) const { return data[i]; }
        };
        constexpr WhitespaceTable whitespaceLookup {};
    }
}

#endif
