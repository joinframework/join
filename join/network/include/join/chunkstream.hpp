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

#ifndef __JOIN_CHUNKTREAM_HPP__
#define __JOIN_CHUNKTREAM_HPP__

// libjoin.
#include <join/streambuf.hpp>

namespace join
{
    /**
     * @brief chunk stream buffer.
     */
    class Chunkstreambuf : public StreambufDecorator
    {
    public:
        /**
         * @brief create the chunk stream buffer instance.
         * @param streambuf concrete stream buffer.
         * @param chunksize chunk size.
         * @param own is the decorator owning inner stream buffer.
         */
        Chunkstreambuf (std::streambuf* streambuf, std::streamsize chunksize = 2048, bool own = false);

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        Chunkstreambuf (const Chunkstreambuf& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        Chunkstreambuf& operator= (const Chunkstreambuf& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        Chunkstreambuf (Chunkstreambuf&& other);

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        Chunkstreambuf& operator= (Chunkstreambuf&& other);

        /**
         * @brief destroy the chunk stream buffer instance.
         */
        virtual ~Chunkstreambuf ();

    protected:
        /**
         * @brief reads characters from the associated input sequence to the get area.
         * @return the value of the character pointed to by the get pointer after the call on success, EOF otherwise.
         */
        virtual int_type underflow () override;

        /**
         * @brief writes characters to the associated output sequence from the put area.
         * @param c the character to store in the put area.
         * @return EOF on failure, some other value on success.
         */
        virtual int_type overflow (int_type c = traits_type::eof ()) override;

        /**
         * @brief synchronizes the buffers with the associated character sequence.
         * @return EOF on failure, some other value on success.
         */
        virtual int_type sync () override;

        /// chunk size.
        std::streamsize _chunksize;

        /// internal buffer.
        std::unique_ptr <char []> _buf;
    };

    /**
     * @brief chunk stream.
     */
    class Chunkstream : public std::iostream
    {
    public:
        /**
         * @brief create the chunk stream instance.
         * @param stream concrete stream.
         * @param chunksize chunk size.
         */
        Chunkstream (std::iostream& stream, std::streamsize chunksize = 2048);

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        Chunkstream (const Chunkstream& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        Chunkstream& operator=(const Chunkstream& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        Chunkstream (Chunkstream&& other);

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        Chunkstream& operator=(Chunkstream&& other);

        /**
         * @brief destroy the chunk stream instance.
         */
        virtual ~Chunkstream () = default;

    protected:
        /// chunkstream buffer.
        Chunkstreambuf _chunkbuf;
    };
}

#endif
