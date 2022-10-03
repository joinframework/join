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

#ifndef __JOIN_BASE64_HPP__
#define __JOIN_BASE64_HPP__

// libjoin.
#include <join/openssl.hpp>

// C++.
#include <string>

namespace join
{
    /// bytes array.
    using BytesArray = std::vector <uint8_t>;

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

    public:
        /**
         * @brief encode data in base 64.
         * @param message the message to encode.
         * @return the string encoded.
         */
        static std::string encode (const std::string& message);

        /**
         * @brief encode data in base 64.
         * @param data the data to encode.
         * @return the string encoded.
         */
        static std::string encode (const BytesArray& data);

        /**
         * @brief encode data in base 64.
         * @param data the data buffer to encode.
         * @param size the data buffer size to encode.
         * @return the string encoded.
         */
        static std::string encode (const uint8_t* data, size_t size);

        /**
         * @brief decode a base64 encoded string.
         * @param data string to decode.
         * @return the string decoded.
         */
        static BytesArray decode (const std::string& data);

    private:
        /// encode table.
        static const std::string _base64Table;

        /// filling character.
        static const char _fillChar = '=';

        /// bytes mask.
        static const uint32_t _mask1 = 0xFC000000;

        /// bytes mask.
        static const uint32_t _mask2 = 0x03F00000;

        /// bytes mask.
        static const uint32_t _mask3 = 0x000FC000;

        /// bytes mask.
        static const uint32_t _mask4 = 0x00003F00;

        /// union.
        typedef union
        {
            uint32_t l;         /**< access the variable as an unsigned integer. */
            char     c[4];      /**< access the variable bytes per bytes. */
        }un32;
    };
}

#endif
