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

// C.
#include <cstring>
#include <cstddef>

namespace join
{
    /**
     * @brief char array view.
     */
    class View
    {
    public:
        /**
         * @brief default constructor.
         * @param s pointer to a character array.
         * @param count number of characters in the sequence.
         */
        constexpr View (const char * s, size_t count)
        : _ptr (s),
          _len (count)
        {
        }

        /**
         * @brief default constructor.
         * @param s pointer to a character array.
         */
        constexpr View (const char * s)
        : _ptr (s),
          _len (strlen (s))
        {
        }

        /**
         * @brief default constructor.
         * @param first pointer to the first character of the sequence.
         * @param last pointer to the last character of the sequence.
         */
        constexpr View (const char * first, const char * last)
        : _ptr (first),
          _len (last - first)
        {
        }

        /**
         * @brief copy constructor.
         * @param other object to copy.
         */
        constexpr View (const View& other) noexcept = default;

        /**
         * @brief copy assignment.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        constexpr View& operator= (const View& other) noexcept = default;

        /**
         * @brief destroy instance.
         */
        virtual ~View () = default;

        /**
         * @brief returns a pointer to the first character of a view.
         * @return a pointer to the first character of a view.
         */
        constexpr const char * data () const noexcept
        {
            return _ptr;
        }

        /**
         * @brief returns the number of characters in the view.
         * @return the number of characters in the view.
         */
        constexpr size_t size () const noexcept
        {
            return _len;
        }

        /**
         * @brief get character without extracting it.
         * @return extracted character.
         */
        constexpr int peek () const noexcept
        {
            if (_len)
            {
                return *_ptr;
            }
            return std::char_traits <char>::eof ();
        }

        /**
         * @brief extracts character.
         * @return extracted character.
         */
        constexpr int get () noexcept
        {
            if (_len)
            {
                --_len;
                return *_ptr++;
            }
            return std::char_traits <char>::eof ();
        }

        /**
         * @brief extracts expected character.
         * @param expected expected character.
         * @return true if extracted, false otherwise.
         */
        constexpr bool getIf (char expected) noexcept
        {
            if (_len && (*_ptr == expected))
            {
                ++_ptr;
                --_len;
                return true;
            }
            return false;
        }

        /**
         * @brief moves the start of the view forward by n characters.
         * @param n number of characters to remove from the start of the view.
         */
        constexpr void removePrefix (size_t n)
        {
            _ptr += n;
            _len -= n;
        }

        /**
         * @brief returns a reference to the element at the specified location pos.
         * @param pos position of the element to return.
         * @return reference to the requested element.
         * @throw std::bad_cast.
         */
        const char& operator[] (size_t pos) const
        {
            return _ptr[pos];
        }

    protected:
        /// view start pointer.
        const char * _ptr;

        /// view size.
        size_t _len;
    };
}

#endif
