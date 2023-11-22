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

#ifndef __JOIN_HMAC_HPP__
#define __JOIN_HMAC_HPP__

// libjoin.
#include <join/digest.hpp>

namespace join
{
    /// forward declaration.
    class Hmac;

    /**
     * @brief HMAC stream buffer.
     */
    class Hmacbuf : public std::streambuf
    {
    public:
        /**
         * @brief create HMAC stream buffer instance.
         * @param algo the message digest algorithm used.
         * @param key key.
         */
        Hmacbuf (const std::string& algo, const std::string& key);

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        Hmacbuf (const Hmacbuf& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        Hmacbuf& operator= (const Hmacbuf& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        Hmacbuf (Hmacbuf&& other);

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        Hmacbuf& operator= (Hmacbuf&& other);

        /**
         * @brief destroy HMAC stream buffer instance.
         */
        virtual ~Hmacbuf () = default;

        /**
         * @brief get keyed-hash message authentication code.
         * @return keyed-hash message authentication code.
         */
        BytesArray finalize ();

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

        /// message digest.
        const EVP_MD* _md;

        /// HMAC key.
        std::string _key;

        /// HMAC context.
        HmacCtxPtr _ctx;
    };

    /**
     * @brief HMAC stream.
     */
    class Hmac : public std::ostream
    {
    public:
        /**
         * @brief default constructor.
         * @param algo the message digest algorithm used.
         * @param key key.
         */
        Hmac (Digest::Algorithm algo, const std::string& key);

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        Hmac (const Hmac& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        Hmac& operator=(const Hmac& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        Hmac (Hmac&& other);

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        Hmac& operator=(Hmac&& other);

        /**
         * @brief destroy HMAC stream instance.
         */
        virtual ~Hmac () = default;

        /**
         * @brief get keyed-hash message authentication code.
         * @return keyed-hash message authentication code.
         */
        BytesArray finalize ();

        /**
         * @brief get MD5 keyed-hash message authentication code.
         * @param message message to hash using keyed-hash.
         * @param size message size.
         * @param key key.
         * @return MD5 keyed-hash message authentication code.
         */
        static BytesArray md5bin (const char* message, std::streamsize size, const std::string& key);

        /**
         * @brief get MD5 keyed-hash message authentication code.
         * @param message message to hash using keyed-hash.
         * @param key key.
         * @return MD5 keyed-hash message authentication code.
         */
        static BytesArray md5bin (const BytesArray& message, const std::string& key);

        /**
         * @brief get MD5 keyed-hash message authentication code.
         * @param message message to hash using keyed-hash.
         * @param key key.
         * @return MD5 keyed-hash message authentication code.
         */
        static BytesArray md5bin (const std::string& message, const std::string& key);

        /**
         * @brief get MD5 keyed-hash message authentication code.
         * @param message message to hash using keyed-hash.
         * @param size message size.
         * @param key key.
         * @return MD5 keyed-hash message authentication code.
         */
        static std::string md5hex (const char* data, std::streamsize size, const std::string& key);

        /**
         * @brief get MD5 keyed-hash message authentication code.
         * @param message message to hash using keyed-hash.
         * @param key key.
         * @return MD5 keyed-hash message authentication code.
         */
        static std::string md5hex (const BytesArray& data, const std::string& key);

        /**
         * @brief get MD5 keyed-hash message authentication code.
         * @param message message to hash using keyed-hash.
         * @param key key.
         * @return MD5 keyed-hash message authentication code.
         */
        static std::string md5hex (const std::string& data, const std::string& key);

        /**
         * @brief get SHA1 keyed-hash message authentication code.
         * @param message message to hash using keyed-hash.
         * @param size message size.
         * @param key key.
         * @return SHA1 keyed-hash message authentication code.
         */
        static BytesArray sha1bin (const char* message, std::streamsize size, const std::string& key);

        /**
         * @brief get SHA1 keyed-hash message authentication code.
         * @param message message to hash using keyed-hash.
         * @param key key.
         * @return SHA1 keyed-hash message authentication code.
         */
        static BytesArray sha1bin (const BytesArray& message, const std::string& key);

        /**
         * @brief get SHA1 keyed-hash message authentication code.
         * @param message message to hash using keyed-hash.
         * @param key key.
         * @return SHA1 keyed-hash message authentication code.
         */
        static BytesArray sha1bin (const std::string& message, const std::string& key);

        /**
         * @brief get SHA1 keyed-hash message authentication code.
         * @param message message to hash using keyed-hash.
         * @param size message size.
         * @param key key.
         * @return SHA1 keyed-hash message authentication code.
         */
        static std::string sha1hex (const char* data, std::streamsize size, const std::string& key);

        /**
         * @brief get SHA1 keyed-hash message authentication code.
         * @param message message to hash using keyed-hash.
         * @param key key.
         * @return SHA1 keyed-hash message authentication code.
         */
        static std::string sha1hex (const BytesArray& data, const std::string& key);

        /**
         * @brief get SHA1 keyed-hash message authentication code.
         * @param message message to hash using keyed-hash.
         * @param key key.
         * @return SHA1 keyed-hash message authentication code.
         */
        static std::string sha1hex (const std::string& data, const std::string& key);

        /**
         * @brief get SHA224 keyed-hash message authentication code.
         * @param message message to hash using keyed-hash.
         * @param size message size.
         * @param key key.
         * @return SHA224 keyed-hash message authentication code.
         */
        static BytesArray sha224bin (const char* message, std::streamsize size, const std::string& key);

        /**
         * @brief get SHA224 keyed-hash message authentication code.
         * @param message message to hash using keyed-hash.
         * @param key key.
         * @return SHA224 keyed-hash message authentication code.
         */
        static BytesArray sha224bin (const BytesArray& message, const std::string& key);

        /**
         * @brief get SHA224 keyed-hash message authentication code.
         * @param message message to hash using keyed-hash.
         * @param key key.
         * @return SHA224 keyed-hash message authentication code.
         */
        static BytesArray sha224bin (const std::string& message, const std::string& key);

        /**
         * @brief get SHA224 keyed-hash message authentication code.
         * @param message message to hash using keyed-hash.
         * @param size message size.
         * @param key key.
         * @return SHA224 keyed-hash message authentication code.
         */
        static std::string sha224hex (const char* data, std::streamsize size, const std::string& key);

        /**
         * @brief get SHA224 keyed-hash message authentication code.
         * @param message message to hash using keyed-hash.
         * @param key key.
         * @return SHA224 keyed-hash message authentication code.
         */
        static std::string sha224hex (const BytesArray& data, const std::string& key);

        /**
         * @brief get SHA224 keyed-hash message authentication code.
         * @param message message to hash using keyed-hash.
         * @param key key.
         * @return SHA224 keyed-hash message authentication code.
         */
        static std::string sha224hex (const std::string& data, const std::string& key);

        /**
         * @brief get SHA256 keyed-hash message authentication code.
         * @param message message to hash using keyed-hash.
         * @param size message size.
         * @param key key.
         * @return SHA256 keyed-hash message authentication code.
         */
        static BytesArray sha256bin (const char* message, std::streamsize size, const std::string& key);

        /**
         * @brief get SHA256 keyed-hash message authentication code.
         * @param message message to hash using keyed-hash.
         * @param key key.
         * @return SHA256 keyed-hash message authentication code.
         */
        static BytesArray sha256bin (const BytesArray& message, const std::string& key);

        /**
         * @brief get SHA256 keyed-hash message authentication code.
         * @param message message to hash using keyed-hash.
         * @param key key.
         * @return SHA256 keyed-hash message authentication code.
         */
        static BytesArray sha256bin (const std::string& message, const std::string& key);

        /**
         * @brief get SHA256 keyed-hash message authentication code.
         * @param message message to hash using keyed-hash.
         * @param size message size.
         * @param key key.
         * @return SHA256 keyed-hash message authentication code.
         */
        static std::string sha256hex (const char* data, std::streamsize size, const std::string& key);

        /**
         * @brief get SHA256 keyed-hash message authentication code.
         * @param message message to hash using keyed-hash.
         * @param key key.
         * @return SHA256 keyed-hash message authentication code.
         */
        static std::string sha256hex (const BytesArray& data, const std::string& key);

        /**
         * @brief get SHA256 keyed-hash message authentication code.
         * @param message message to hash using keyed-hash.
         * @param key key.
         * @return SHA256 keyed-hash message authentication code.
         */
        static std::string sha256hex (const std::string& data, const std::string& key);

        /**
         * @brief get SHA384 keyed-hash message authentication code.
         * @param message message to hash using keyed-hash.
         * @param size message size.
         * @param key key.
         * @return SHA384 keyed-hash message authentication code.
         */
        static BytesArray sha384bin (const char* message, std::streamsize size, const std::string& key);

        /**
         * @brief get SHA384 keyed-hash message authentication code.
         * @param message message to hash using keyed-hash.
         * @param key key.
         * @return SHA384 keyed-hash message authentication code.
         */
        static BytesArray sha384bin (const BytesArray& message, const std::string& key);

        /**
         * @brief get SHA384 keyed-hash message authentication code.
         * @param message message to hash using keyed-hash.
         * @param key key.
         * @return SHA384 keyed-hash message authentication code.
         */
        static BytesArray sha384bin (const std::string& message, const std::string& key);

        /**
         * @brief get SHA384 keyed-hash message authentication code.
         * @param message message to hash using keyed-hash.
         * @param size message size.
         * @param key key.
         * @return SHA384 keyed-hash message authentication code.
         */
        static std::string sha384hex (const char* data, std::streamsize size, const std::string& key);

        /**
         * @brief get SHA384 keyed-hash message authentication code.
         * @param message message to hash using keyed-hash.
         * @param key key.
         * @return SHA384 keyed-hash message authentication code.
         */
        static std::string sha384hex (const BytesArray& data, const std::string& key);

        /**
         * @brief get SHA384 keyed-hash message authentication code.
         * @param message message to hash using keyed-hash.
         * @param key key.
         * @return SHA384 keyed-hash message authentication code.
         */
        static std::string sha384hex (const std::string& data, const std::string& key);

        /**
         * @brief get SHA512 keyed-hash message authentication code.
         * @param message message to hash using keyed-hash.
         * @param size message size.
         * @param key key.
         * @return SHA512 keyed-hash message authentication code.
         */
        static BytesArray sha512bin (const char* message, std::streamsize size, const std::string& key);

        /**
         * @brief get SHA512 keyed-hash message authentication code.
         * @param message message to hash using keyed-hash.
         * @param key key.
         * @return SHA512 keyed-hash message authentication code.
         */
        static BytesArray sha512bin (const BytesArray& message, const std::string& key);

        /**
         * @brief get SHA512 keyed-hash message authentication code.
         * @param message message to hash using keyed-hash.
         * @param key key.
         * @return SHA512 keyed-hash message authentication code.
         */
        static BytesArray sha512bin (const std::string& message, const std::string& key);

        /**
         * @brief get SHA512 keyed-hash message authentication code.
         * @param message message to hash using keyed-hash.
         * @param size message size.
         * @param key key.
         * @return SHA512 keyed-hash message authentication code.
         */
        static std::string sha512hex (const char* data, std::streamsize size, const std::string& key);

        /**
         * @brief get SHA512 keyed-hash message authentication code.
         * @param message message to hash using keyed-hash.
         * @param key key.
         * @return SHA512 keyed-hash message authentication code.
         */
        static std::string sha512hex (const BytesArray& data, const std::string& key);

        /**
         * @brief get SHA512 keyed-hash message authentication code.
         * @param message message to hash using keyed-hash.
         * @param key key.
         * @return SHA512 keyed-hash message authentication code.
         */
        static std::string sha512hex (const std::string& data, const std::string& key);

        /**
         * @brief get SM3 keyed-hash message authentication code.
         * @param message message to hash using keyed-hash.
         * @param size message size.
         * @param key key.
         * @return SM3 keyed-hash message authentication code.
         */
        static BytesArray sm3bin (const char* message, std::streamsize size, const std::string& key);

        /**
         * @brief get SM3 keyed-hash message authentication code.
         * @param message message to hash using keyed-hash.
         * @param key key.
         * @return SM3 keyed-hash message authentication code.
         */
        static BytesArray sm3bin (const BytesArray& message, const std::string& key);

        /**
         * @brief get SM3 keyed-hash message authentication code.
         * @param message message to hash using keyed-hash.
         * @param key key.
         * @return SM3 keyed-hash message authentication code.
         */
        static BytesArray sm3bin (const std::string& message, const std::string& key);

        /**
         * @brief get SM3 keyed-hash message authentication code.
         * @param message message to hash using keyed-hash.
         * @param size message size.
         * @param key key.
         * @return SM3 keyed-hash message authentication code.
         */
        static std::string sm3hex (const char* data, std::streamsize size, const std::string& key);

        /**
         * @brief get SM3 keyed-hash message authentication code.
         * @param message message to hash using keyed-hash.
         * @param key key.
         * @return SM3 keyed-hash message authentication code.
         */
        static std::string sm3hex (const BytesArray& data, const std::string& key);

        /**
         * @brief get SM3 keyed-hash message authentication code.
         * @param message message to hash using keyed-hash.
         * @param key key.
         * @return SM3 keyed-hash message authentication code.
         */
        static std::string sm3hex (const std::string& data, const std::string& key);

    protected:
        /// associated HMAC stream buffer.
        Hmacbuf _hmacbuf;
    };
}

#endif
