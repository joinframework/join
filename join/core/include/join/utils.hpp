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

#ifndef __JOIN_UTILS_HPP__
#define __JOIN_UTILS_HPP__

// libjoin.
#include <join/error.hpp>

// C++.
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <random>
#include <limits>
#include <chrono>

// C.
#include <endian.h>
#include <cstdint>
#include <cstddef>
#include <cstring>

#define OUT_ENUM(a) case a : return #a

namespace join
{
    namespace details
    {
        template <typename Type, size_t sz>
        struct _byteswap
        {
            __inline__ Type operator() (Type val)
            {
                throw std::out_of_range ("data size");
            }
        };

        template <typename Type>
        struct _byteswap <Type, 1>
        {
            __inline__ Type operator() (Type val)
            {
                return val;
            }
        };

        template <typename Type>
        struct _byteswap <Type, 2>
        {
            __inline__ Type operator() (Type val)
            {
                if (BYTE_ORDER == LITTLE_ENDIAN)
                {
                    return (val >> 8) | (val << 8);
                }
                return val;
            }
        };

        template <typename Type>
        struct _byteswap <Type, 4>
        {
            __inline__ Type operator() (Type val)
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

        template <typename Type>
        struct _byteswap <Type, 8>
        {
            __inline__ Type operator() (Type val)
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
            __inline__ float operator() (float val)
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
            __inline__ double operator() (double val)
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

        template <class Type>
        struct _swap
        {
            __inline__ Type operator() (Type val)
            {
                return _byteswap <Type, sizeof (Type)> ()(val);
            }
        };

        struct lessNoCase
        {
            bool operator () (const std::string& a, const std::string& b) const noexcept
            {
                return ::strcasecmp (a.c_str (), b.c_str ()) < 0;
            }
        };
    }

    /**
     * @brief swaps byte orders.
     * @param val value to swap.
     * @return the swapped value.
     */
    template <class Type>
    __inline__ Type& swap (Type& val)
    {
        val = details::_swap <Type> ()(val);
        return val;
    }

    /**
     * @brief case insensitive string comparison.
     * @param a string to compare.
     * @param b string to compare to.
     * @return true if equals, false otharwise.
     */
    __inline__ bool compareNoCase (const std::string& a, const std::string& b)
    {
        return ((a.size () == b.size ()) && 
            std::equal (a.begin (), a.end (), b.begin (), [] (char c1, char c2) {return std::toupper (c1) == std::toupper (c2);}));
    }

    /**
     * @brief trim left.
     * @param s string to trim.
     * @return trimed string.
     */
    __inline__ std::string& trimLeft (std::string& s)
    {
        return s.erase (0, s.find_first_not_of ("\f\t\v\r\n "));
    }

    /**
     * @brief trim right.
     * @param s string to trim.
     * @return trimed string.
     */
    __inline__ std::string& trimRight (std::string& s)
    {
        return s.erase (s.find_last_not_of (" \f\t\v\r\n") + 1);
    }

    /**
     * @brief trim.
     * @param s string to trim.
     * @return trimed string.
     */
    __inline__ std::string& trim (std::string& s)
    {
        return trimLeft (trimRight (s));
    }

    /**
     * @brief replace all occurrences of a substring.
     * @param str string to scan.
     * @param toReplace string to replace.
     * @param by string to put instead of the "toReplace" substring.
     * @return a reference to the string.
     */
    __inline__ std::string& replaceAll (std::string& str, const std::string &toReplace, const std::string &by)
    {
        size_t pos = 0;

        while ((pos = str.find (toReplace, pos)) != std::string::npos)
        {
            str.replace (pos, toReplace.length (), by);
            pos += by.length ();
        }

        return str;
    }

    /**
     * @brief read HTTP line (delimiter "\r\n").
     * @param in input stream.
     * @param line line read.
     * @param max max characters to read.
     * @return input stream.
     */
    __inline__ std::istream& getline (std::istream& in, std::string& line, std::streamsize max = 1024)
    {
        line.clear ();

        while (max--)
        {
            char ch = in.get ();
            if (in.fail ())
            {
                return in;
            }

            if (ch == '\r')
            {
                continue;
            }

            if (ch == '\n')
            {
                return in;
            }

            line.push_back (ch);
        }

        join::lastError = make_error_code (Errc::MessageTooLong);
        in.setstate (std::ios_base::failbit);

        return in;
    }

    /**
     * @brief dump data to standard output stream.
     * @param data data to dump.
     * @param size number of data bytes.
     */
    __inline__ void dump (const void* data, unsigned long size, std::ostream& out = std::cout)
    {
        unsigned char *buf = (unsigned char *) data;

        for (int i = 0; i < int (size); i += 16)
        {
            out << std::hex << std::uppercase << std::setw (8);
            out << std::setfill ('0') << i << std::dec << ":";

            for (int j = 0; j < 16; ++j)
            {
                if (j % 4 == 0)
                    out << std::dec << " ";

                if (i + j < int (size))
                {
                    out << std::hex << std::uppercase << std::setw (2);
                    out << std::setfill ('0') << static_cast <int> (buf[i + j]);
                }
                else
                    out << std::dec << "  ";
            }

            out << std::dec << " ";

            for (int j = 0; j < 16; ++j)
            {
                if (i + j < int (size))
                {
                    if (isprint (buf[i + j]))
                        out << buf[i + j];
                    else
                        out << ".";
                }
            }

            out << std::endl;
        }

        out << std::endl;
    }

    /**
     * @brief create a random number.
     * @return random number.
     */
    template <typename Type>
    std::enable_if_t <std::numeric_limits <Type>::is_integer, Type>
    randomize ()
    {
        std::random_device rnd;
        std::uniform_int_distribution <Type> dist {};
        return dist (rnd);
    }

    /**
     * @brief benchmark function call.
     * @param function function to benchmark.
     * @param arguments function arguments.
     * @return time elapsed in milliseconds.
     */
    template <class Func, class... Args>
    std::chrono::milliseconds benchmark (Func&& func, Args&&... args)
    {
        auto beg = std::chrono::high_resolution_clock::now ();
        func (std::forward <Args> (args)...);
        auto end = std::chrono::high_resolution_clock::now ();
        return std::chrono::duration_cast <std::chrono::milliseconds> (end - beg);
    }
}

#endif
