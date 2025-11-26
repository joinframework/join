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

#ifndef __JOIN_VIEW_HPP__
#define __JOIN_VIEW_HPP__

// libjoin.
#include <join/utils.hpp>

// C++.
#include <istream>
#include <string>

// C.
#include <cstring>
#include <cstddef>

namespace join
{
    /**
     * @brief string view.
     */
    class StringView
    {
    public:
        /**
         * @brief default constructor.
         * @param in input string.
         * @param count number of characters.
         */
        constexpr StringView (const char * in, size_t count)
        : _pos (in)
        , _end (in ? in + count : in)
        {
        }

        /**
         * @brief default constructor.
         * @param first pointer to the first character of the string.
         * @param last pointer to the last character of the string.
         */
        constexpr StringView (const char * first, const char * last)
        : _pos (first)
        , _end (last)
        {
        }

        /**
         * @brief default constructor.
         * @param in input string.
         */
        StringView (const char * in)
        : _pos (in)
        , _end (in ? in + std::char_traits <char>::length (in) : in)
        {
        }

        /**
         * @brief copy constructor.
         * @param other object to copy.
         */
        StringView (const StringView& other) = default;

        /**
         * @brief copy assignment.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        StringView& operator= (const StringView& other) = default;

        /**
         * @brief move constructor.
         * @param other object to move.
         */
        StringView (StringView&& other) = default;

        /**
         * @brief move assignment.
         * @param other object to move.
         * @return a reference of the current object.
         */
        StringView& operator=(StringView&& other) = default;

        /**
         * @brief destroy instance.
         */
        ~StringView () = default;

        /**
         * @brief get character without extracting it.
         * @return extracted character.
         */
        inline int peek () const noexcept
        {
            if (JOIN_LIKELY (_pos < _end))
            {
                return static_cast <unsigned char> (*_pos);
            }
            return std::char_traits <char>::eof ();
        }

        /**
         * @brief extracts character.
         * @return extracted character.
         */
        inline int get () noexcept
        {
            if (JOIN_LIKELY (_pos < _end))
            {
                return static_cast <unsigned char> (*_pos++);
            }
            return std::char_traits <char>::eof ();
        }

        /**
         * @brief extracts expected character.
         * @param expected expected character.
         * @return true if extracted, false otherwise.
         */
        inline bool getIf (char expected) noexcept
        {
            if (JOIN_LIKELY (_pos < _end) && (*_pos == expected))
            {
                ++_pos;
                return true;
            }
            return false;
        }

        /**
         * @brief extracts expected character (case insensitive, ASCII-only).
         * @param expected expected character.
         * @return true if extracted, false otherwise.
         */
        inline bool getIfNoCase (char expected) noexcept
        {
            if (JOIN_LIKELY (_pos < _end))
            {
                const char c = *_pos;
                if ((c | 32) == (expected | 32))
                {
                    ++_pos;
                    return true;
                }
            }
            return false;
        }

        /**
         * @brief read characters.
         * @param buf output buffer.
         * @param count number of characters to read.
         * @return number of characters read.
         */
        inline size_t read (char* buf, size_t count) noexcept
        {
            const size_t available = _end - _pos;
            const size_t nread = (count < available) ? count : available;
            std::memcpy (buf, _pos, nread);
            _pos += nread;
            return nread;
        }

    private:
        /// current position.
        const char * _pos = nullptr;

        /// end position.
        const char * _end = nullptr;
    };

    /**
     * @brief stream view.
     */
    class StreamView
    {
    public:
        /**
         * @brief default constructor.
         * @param in input stream.
         */
        StreamView (std::istream& in)
        : _in (in.rdbuf ())
        {
        }

        /**
         * @brief copy constructor.
         * @param other object to copy.
         */
        StreamView (const StreamView& other) = delete;

        /**
         * @brief copy assignment.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        StreamView& operator= (const StreamView& other) = delete;

        /**
         * @brief move constructor.
         * @param other object to move.
         */
        StreamView (StreamView&& other) = delete;

        /**
         * @brief move assignment.
         * @param other object to move.
         * @return a reference of the current object.
         */
        StreamView& operator=(StreamView&& other) = delete;

        /**
         * @brief destroy instance.
         */
        ~StreamView () = default;

        /**
         * @brief get character without extracting it.
         * @return extracted character.
         */
        inline int peek () const noexcept
        {
            return _in->sgetc ();
        }

        /**
         * @brief extracts character.
         * @return extracted character.
         */
        inline int get () noexcept
        {
            return _in->sbumpc ();
        }

        /**
         * @brief extracts expected character.
         * @param expected expected character.
         * @return true if extracted, false otherwise.
         */
        inline bool getIf (char expected) noexcept
        {
            if (_in->sgetc () == static_cast <int> (static_cast <unsigned char> (expected)))
            {
                _in->sbumpc ();
                return true;
            }
            return false;
        }

        /**
         * @brief extracts expected character (case insensitive, ASCII-only).
         * @param expected expected character.
         * @return true if extracted, false otherwise.
         */
        inline bool getIfNoCase (char expected) noexcept
        {
            const int c = _in->sgetc ();
            if (JOIN_LIKELY (c != std::char_traits <char>::eof ()))
            {
                if ((static_cast <char> (c) | 32) == (expected | 32))
                {
                    _in->sbumpc ();
                    return true;
                }
            }
            return false;
        }

        /**
         * @brief read characters.
         * @param buf output buffer.
         * @param count number of characters to read.
         * @return number of characters read.
         */
        inline size_t read (char* buf, size_t count) noexcept
        {
            return _in->sgetn (buf, count);
        }

    private:
        /// input stream buffer.
        std::streambuf* _in;
    };
}

#endif
