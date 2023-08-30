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

#ifndef __JOIN_ZSTREAM_HPP__
#define __JOIN_ZSTREAM_HPP__

// libjoin.
#include <join/streambuf.hpp>

// Libraries.
#include <zlib.h>

namespace join
{
    /**
     * @brief zlib stream buffer.
     */
    class Zstreambuf : public StreambufDecorator
    {
    public:
        /**
         * @brief create the zlib stream buffer instance.
         * @param streambuf concrete stream buffer
         * @param format compressed data format.
         * @param own is the decorator owning inner stream buffer.
         */
        Zstreambuf (std::streambuf* streambuf, int format, bool own = false);

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        Zstreambuf (const Zstreambuf& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        Zstreambuf& operator= (const Zstreambuf& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        Zstreambuf (Zstreambuf&& other);

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        Zstreambuf& operator= (Zstreambuf&& other);

        /**
         * @brief destroy the zlib stream buffer instance.
         */
        virtual ~Zstreambuf ();

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

        /// internal buffer size.
        static const std::streamsize _bufsize = 16384;

        /// inflate context.
        std::unique_ptr <z_stream> _inflate;

        /// deflate context.
        std::unique_ptr <z_stream> _deflate;

        /// internal buffer.
        std::unique_ptr <char []> _buf;
    };

    /**
     * @brief zlib stream.
     */
    class Zstream : public std::iostream
    {
    public:
        /**
         * @brief data format.
         */
        enum Format
        {
            Deflate = -MAX_WBITS,           /**< use the deflate compressed data format. */
            Zlib    =  MAX_WBITS,           /**< use the zlib compressed data format. */
            Gzip    =  MAX_WBITS + 16,      /**< use the gzip compressed data format. */
        };

        /**
         * @brief create the zlib stream instance.
         * @param stream concrete stream.
         * @param format compressed data format.
         */
        Zstream (std::iostream& stream, Format format = Zlib);

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        Zstream (const Zstream& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        Zstream& operator=(const Zstream& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        Zstream (Zstream&& other);

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        Zstream& operator=(Zstream&& other);

        /**
         * @brief destroy the zlib stream instance.
         */
        virtual ~Zstream () = default;

    protected:
        /// zlib stream buffer
        Zstreambuf _zbuf;
    };
}

#endif
