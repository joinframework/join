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

#ifndef __JOIN_BASE64_HPP__
#define __JOIN_BASE64_HPP__

// libjoin.
#include <join/openssl.hpp>

// C++.
#include <iostream>
#include <iomanip>
#include <string>

namespace join
{
    /// bytes array.
    using BytesArray = std::vector <uint8_t>;

    /**
     * @brief convert bytes array to string.
     * @param bin bytes array.
     * @return converted bytes array string.
     */
    __inline__ std::string bin2hex (const BytesArray& bin)
    {
        std::stringstream oss;
        for (size_t i = 0; i < bin.size (); ++i)
        {
            oss << std::hex << std::setw (2) << std::setfill ('0') << static_cast <uint32_t> (bin[i]);
        }
        return oss.str ();
    }

    /**
     * @brief encoder stream buffer.
     */
    class Encoderbuf : public std::streambuf
    {
    public:
        /**
         * @brief create encoder stream buffer instance.
         */
        Encoderbuf ();

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        Encoderbuf (const Encoderbuf& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        Encoderbuf& operator= (const Encoderbuf& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        Encoderbuf (Encoderbuf&& other);

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        Encoderbuf& operator= (Encoderbuf&& other);

        /**
         * @brief destroy encoder stream buffer instance.
         */
        virtual ~Encoderbuf () = default;

        /**
         * @brief get encoded string.
         * @return encoded string.
         */
        std::string get ();

    protected:
        /**
         * @brief writes characters to the associated output sequence from the put area.
         * @param c the character to store in the put area.
         * @return EOF on failure, some other value on success.
         */
        virtual int_type overflow (int_type c = traits_type::eof ()) override;

        /// internal buffer size.
        static const std::streamsize _bufsize = 256;

        /// internal buffer.
        std::unique_ptr <char []> _buf;

        /// encode context.
        EvpEncodeCtxPtr _encodectx;

        /// output buffer.
        std::string _out;
    };

    /**
     * @brief encoder.
     */
    class Encoder : public std::ostream
    {
    public:
        /**
         * @brief default constructor.
         */
        Encoder ();

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        Encoder (const Encoder& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        Encoder& operator=(const Encoder& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        Encoder (Encoder&& other);

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        Encoder& operator=(Encoder&& other);

        /**
         * @brief destroy instance.
         */
        virtual ~Encoder () = default;

        /**
         * @brief get encoded string.
         * @return encoded string.
         */
        std::string get ();

    protected:
        /// associated encoder stream buffer.
        Encoderbuf _encoderbuf;
    };

    /**
     * @brief decoder stream buffer.
     */
    class Decoderbuf : public std::streambuf
    {
    public:
        /**
         * @brief create decoder stream buffer instance.
         */
        Decoderbuf ();

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        Decoderbuf (const Decoderbuf& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        Decoderbuf& operator= (const Decoderbuf& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        Decoderbuf (Decoderbuf&& other);

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        Decoderbuf& operator= (Decoderbuf&& other);

        /**
         * @brief destroy encoder stream buffer instance.
         */
        virtual ~Decoderbuf () = default;

        /**
         * @brief get decoded data.
         * @return decoded data.
         */
        BytesArray get ();

    protected:
        /**
         * @brief writes characters to the associated output sequence from the put area.
         * @param c the character to store in the put area.
         * @return EOF on failure, some other value on success.
         */
        virtual int_type overflow (int_type c = traits_type::eof ()) override;

        /// internal buffer size.
        static const std::streamsize _bufsize = 256;

        /// internal buffer.
        std::unique_ptr <char []> _buf;

        /// decode context.
        EvpEncodeCtxPtr _decodectx;

        /// output buffer.
        BytesArray _out;
    };

    /**
     * @brief decoder.
     */
    class Decoder : public std::ostream
    {
    public:
        /**
         * @brief default constructor.
         */
        Decoder ();

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        Decoder (const Decoder& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        Decoder& operator=(const Decoder& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        Decoder (Decoder&& other);

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        Decoder& operator=(Decoder&& other);

        /**
         * @brief destroy instance.
         */
        virtual ~Decoder () = default;

        /**
         * @brief get decoded data.
         * @return decoded data.
         */
        BytesArray get ();

    protected:
        /// associated decoder stream buffer.
        Decoderbuf _decoderbuf;
    };

    /**
     * @brief base64 encode, decode class.
     */
    class Base64
    {
    public:
        /**
         * @brief create the Base64 instance.
         */
        Base64 () = delete;

        /**
         * @brief destroy the Base64 instance.
         */
        ~Base64 () = delete;

        /**
         * @brief encode data.
         * @param data the data buffer to encode.
         * @param size the data buffer size to encode.
         * @return the encoded string.
         */
        static std::string encode (const char* data, size_t size);

        /**
         * @brief encode data.
         * @param data the data to encode.
         * @return the encoded string.
         */
        static std::string encode (const std::string& data);

        /**
         * @brief encode data.
         * @param data the data to encode.
         * @return the encoded string.
         */
        static std::string encode (const BytesArray& data);

        /**
         * @brief decode a base64 encoded string.
         * @param data string to decode.
         * @return the string decoded.
         */
        static BytesArray decode (const std::string& data);
    };
}

#endif
