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
    class StreambufDecorator : public std::streambuf
    {
    public:
        /**
         * @brief create the stream buffer decorator instance.
         * @param istream concrete input stream.
         * @param ostream concrete output stream.
         * @param bufsize internal buffer size.
         */
        StreambufDecorator (std::istream& istream, std::ostream& ostream, std::streamsize bufsize)
        : _buf (std::make_unique <char []> (bufsize)),
          _istream (std::addressof (istream)),
          _ostream (std::addressof (ostream))
        {
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        StreambufDecorator (const StreambufDecorator& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        StreambufDecorator& operator= (const StreambufDecorator& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        StreambufDecorator (StreambufDecorator&& other)
        : std::streambuf (std::move (other)),
          _buf (std::move (other._buf)),
          _istream (other._istream),
          _ostream (other._ostream)
        {
            other._istream = nullptr;
            other._ostream = nullptr;
        }

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        StreambufDecorator& operator= (StreambufDecorator&& other)
        {
            std::streambuf::operator= (std::move (other));
            _buf = std::move (other._buf);
            _istream = other._istream;
            _istream = other._istream;
            other._istream = nullptr;
            other._ostream = nullptr;
            return *this;
        }

        /**
         * @brief destroy the stream buffer decorator instance.
         */
        virtual ~StreambufDecorator () = default;

    protected:
        /// internal buffer.
        std::unique_ptr <char []> _buf;

        /// concrete input stream.
        std::istream* _istream;

        /// concrete output stream.
        std::ostream* _ostream;
    };
}
