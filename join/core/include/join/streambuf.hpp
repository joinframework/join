/**
 * MIT License
 *
 * Copyright (c) 2023 Mathieu Rabine
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

// C++.
#include <iostream>
#include <memory>

namespace join
{
    /**
     * @brief stream buffer decorator.
     */
    class Streambuf : public std::streambuf
    {
    public:
        /**
         * @brief create the stream buffer decorator instance.
         * @param streambuf concrete stream buffer.
         * @param own is the decorator owning inner stream buffer.
         */
        StreambufDecorator (std::streambuf* streambuf, bool own = false)
        : _innerbuf (streambuf),
          _own (own)
        {
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        Streambuf (const Streambuf& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        Streambuf& operator= (const Streambuf& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        Streambuf (Streambuf&& other)
        : std::streambuf (std::move (other)),
          _innerbuf (other._innerbuf),
          _own (other._own)
        {
            other._innerbuf = nullptr;
            other._own = false;
        }

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        Streambuf& operator= (Streambuf&& other)
        {
            std::streambuf::operator= (std::move (other));
            _innerbuf = other._innerbuf;
            _own = other._own;
            other._innerbuf = nullptr;
            other._own = false;
            return *this;
        }

        /**
         * @brief destroy the stream buffer decorator instance.
         */
        virtual ~StreambufDecorator ()
        {
            if (_own && _innerbuf)
            {
                delete (_innerbuf);
            }
        }

    protected:
        /// concrete stream buffer.
        std::streambuf* _innerbuf;

        /// own inner stream buffer.
        bool _own = false;
    };
}
