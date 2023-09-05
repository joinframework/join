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

// C++.
#include <stdexcept>
#include <istream>

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
        StringView (const char* in, size_t count)
        : _buf (in),
          _len (count)
        {
        }

        /**
         * @brief default constructor.
         * @param in input string.
         */
        StringView (const char * in)
        : _buf (in),
          _len (strlen (in))
        {
        }

        /**
         * @brief default constructor.
         * @param first pointer to the first character of the string.
         * @param last pointer to the last character of the string.
         */
        StringView (const char * first, const char * last)
        : _buf (first),
          _len (last - first)
        {
        }

        /**
         * @brief copy constructor.
         * @param other object to copy.
         */
        StringView (const StringView& other) = delete;

        /**
         * @brief copy assignment.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        StringView& operator= (const StringView& other) = delete;

        /**
         * @brief move constructor.
         * @param other object to move.
         */
        StringView (StringView&& other) = delete;

        /**
         * @brief move assignment.
         * @param other object to move.
         * @return a reference of the current object.
         */
        StringView& operator=(StringView&& other) = delete;

        /**
         * @brief destroy instance.
         */
        virtual ~StringView () = default;

        /**
         * @brief get character without extracting it.
         * @return extracted character.
         */
        int peek () const noexcept
        {
            if (_len)
            {
                return _buf[_pos];
            }
            return std::char_traits <char>::eof ();
        }

        /**
         * @brief extracts character.
         * @return extracted character.
         */
        int get () noexcept
        {
            if (_len)
            {
                --_len;
                return _buf[_pos++];
            }
            return std::char_traits <char>::eof ();
        }

        /**
         * @brief extracts expected character.
         * @param expected expected character.
         * @return true if extracted, false otherwise.
         */
        bool getIf (char expected) noexcept
        {
            if (_len && (_buf[_pos] == expected))
            {
                --_len;
                ++_pos;
                return true;
            }
            return false;
        }

        /**
         * @brief extracts expected character (case insensitive).
         * @param expected expected character.
         * @return true if extracted, false otherwise.
         */
        bool getIfNoCase (char expected) noexcept
        {
            if (_len && (((_buf[_pos] ^ expected) == 0) ||
                         ((_buf[_pos] ^ expected) == 32)))
            {
                --_len;
                ++_pos;
                return true;
            }
            return false;
        }

        /**
         * @brief .
         * @param buf .
         * @param count .
         */
        size_t read (char* buf, size_t count)
        {
            count = std::min (_len, count);
            ::memcpy (buf, &_buf[_pos], count);
            _pos += count;
            _len -= count;
            return count;
        }

    private:
        /// input buffer start pointer.
        const char * _buf = nullptr;

        /// string remaining size.
        size_t _len = 0;

        /// current position.
        size_t _pos = 0;
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
        : _in (std::addressof (in))
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
        virtual ~StreamView () = default;

        /**
         * @brief get character without extracting it.
         * @return extracted character.
         */
        int peek () const noexcept
        {
            return _in->peek ();
        }

        /**
         * @brief extracts character.
         * @return extracted character.
         */
        int get () noexcept
        {
            return _in->get ();
        }

        /**
         * @brief extracts expected character.
         * @param expected expected character.
         * @return true if extracted, false otherwise.
         */
        bool getIf (char expected) noexcept
        {
            if (!_in->eof () && (static_cast <char> (_in->peek ()) == expected))
            {
                _in->get ();
                return true;
            }
            return false;
        }

        /**
         * @brief extracts expected character (case insensitive).
         * @param expected expected character.
         * @return true if extracted, false otherwise.
         */
        bool getIfNoCase (char expected) noexcept
        {
            if (!_in->eof () && (((static_cast <char> (_in->peek ()) ^ expected) == 0) ||
                                 ((static_cast <char> (_in->peek ()) ^ expected) == 32)))
            {
                _in->get ();
                return true;
            }
            return false;
        }

        /**
         * @brief .
         * @param buf .
         * @param count .
         */
        size_t read (char* buf, size_t count)
        {
            _in->read (buf, count);
            return _in->gcount ();
        }

    private:
        /// input stream.
        std::istream* _in;
    };
}

#endif
