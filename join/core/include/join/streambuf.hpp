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
         * @param bufsize internal buffer size.
         */
        Streambuf (std::streambuf& streambuf, std::streamsize bufsize)
        : _buf (std::make_unique <char []> (bufsize)),
          _streambuf (std::addressof (streambuf))
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
          _buf (std::move (other._buf)),
          _streambuf (other._streambuf)
        {
            other._streambuf = nullptr;
        }

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        Streambuf& operator= (Streambuf&& other)
        {
            std::streambuf::operator= (std::move (other));
            _buf = std::move (other._buf);
            _streambuf = other._streambuf;
            other._streambuf = nullptr;
            return *this;
        }

        /**
         * @brief destroy the stream buffer decorator instance.
         */
        virtual ~Streambuf () = default;

    protected:
        /// internal buffer.
        std::unique_ptr <char []> _buf;

        /// concrete stream buffer.
        std::streambuf* _streambuf;
    };
}
