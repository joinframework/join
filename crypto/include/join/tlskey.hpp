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

#ifndef __TLS_KEY_HPP__
#define __TLS_KEY_HPP__

// libjoin.
#include <join/openssl.hpp>
#include <join/base64.hpp>

namespace join
{
    /**
     * @brief manage private and public keys.
     */
    class TlsKey
    {
    public:
        using Handle = EVP_PKEY*;

        /**
         * @brief key type.
         */
        enum Type
        {
            Public,             /**< public key. */
            Private,            /**< private key. */
        };

        /**
         * @brief default constructor.
         */
        TlsKey () = default;

        /**
         * @brief create key using a file.
         * @param keyPath key path.
         * @param keyType specifies whether the key is public or private.
         */
        TlsKey (const std::string& keyPath, Type keyType = Private);

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        TlsKey (const TlsKey& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        TlsKey& operator= (const TlsKey& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        TlsKey (TlsKey&& other);

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        TlsKey& operator= (TlsKey&& other);

        /**
         * @brief destroy the signature stream buffer instance.
         */
        ~TlsKey () = default;

        /**
         * @brief explicit conversion function for boolean value.
         * @return converted boolean value.
         * @throw std::bad_cast.
         */
        explicit operator bool () const;

        /**
         * @brief get a pointer to the native key handle.
         * @return a pointer to the native key handle.
         */
        Handle handle () const;

        /**
         * @brief return the length in bits of the key.
         * @return the length in bits of the key, -1 if none.
         */
        int length ();

        /**
         * @brief swap this key with other.
         * @param other other key to swap with.
         */
        void swap (TlsKey& other);

        /**
         * @brief return the key type.
         * @return the key type.
         */
        Type type () const;

        /**
         * @brief clear the key.
         */
        void clear ();

    private:
        /**
         * @brief read private key.
         * @param path private key path.
         * @param keyType specifies whether the key is public or private.
         * @return a pointer to the native key handle.
         */
        static Handle readKey (const std::string& path, Type keyType = Private);

        /// key type.
        Type _type = Private;

        /// key native handle.
        EvpPkeyPtr _key;
    };
}

#endif
