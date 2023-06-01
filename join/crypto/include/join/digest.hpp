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

#ifndef __JOIN_DIGEST_HPP__
#define __JOIN_DIGEST_HPP__

// libjoin.
#include <join/openssl.hpp>
#include <join/base64.hpp>

// C++.
#include <iostream>

namespace join
{
    /**
     * @brief digest error codes.
     */
    enum class DigestErrc
    {
        InvalidAlgorithm = 1,   /**< invalid algorithm. */
        InvalidKey,             /**< invalid key. */
        InvalidSignature        /**< invalid signature. */
    };

    /**
     * @brief digest error category.
     */
    class DigestCategory : public std::error_category
    {
    public:
        /**
         * @brief get digest error category name.
         * @return digest error category name.
         */
        virtual const char* name () const noexcept;

        /**
         * @brief translate digest error code to human readable error string.
         * @param code error code.
         * @return human readable error string.
         */
        virtual std::string message (int code) const;
    };

    /**
     * @brief get error category.
     * @return the created std::error_category object.
     */
    const std::error_category& getDigestCategory ();

    /**
     * @brief create an std::error_code object.
     * @param code error code number.
     * @return the created std::error_code object.
     */
    std::error_code make_error_code (DigestErrc code);

    /**
     * @brief create an std::error_condition object.
     * @param code error code number.
     * @return the created std::error_condition object.
     */
    std::error_condition make_error_condition (DigestErrc code);

    /// forward declaration.
    class Digest;

    /**
     * @brief digest stream buffer.
     */
    class Digestbuf : public std::streambuf
    {
    public:
        /**
         * @brief create digest stream buffer instance.
         * @param algo the message digest used.
         */
        Digestbuf (const std::string& algo);

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        Digestbuf (const Digestbuf& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        Digestbuf& operator= (const Digestbuf& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        Digestbuf (Digestbuf&& other);

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        Digestbuf& operator= (Digestbuf&& other);

        /**
         * @brief destroy digest stream buffer instance.
         */
        virtual ~Digestbuf () = default;

        /**
         * @brief get message digest.
         * @return message digest.
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

        /// message digest context.
        EvpMdCtxPtr _mdctx;

        /// message digest.
        const EVP_MD* _md;
    };

    /**
     * @brief digest stream.
     */
    class Digest : public std::ostream
    {
    public:
        /**
         * @brief algorithm.
         */
        enum Algorithm
        {
            SHA1 = 1,   /**< secure hash algorithm v1 */
            SHA224,     /**< secure hash algorithm v2 with a 224 bits digest */
            SHA256,     /**< secure hash algorithm v2 with a 256 bits digest */
            SHA384,     /**< secure hash algorithm v2 with a 384 bits digest */
            SHA512,     /**< secure hash algorithm v2 with a 512 bits digest */
        };

        /**
         * @brief default constructor.
         * @param algo the message digest used.
         */
        Digest (Algorithm algo);

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        Digest (const Digest& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        Digest& operator=(const Digest& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        Digest (Digest&& other);

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        Digest& operator=(Digest&& other);

        /**
         * @brief destroy digest stream instance.
         */
        virtual ~Digest () = default;

        /**
         * @brief get message digest.
         * @return message digest.
         */
        BytesArray get ();

        /**
         * @brief get SHA1 message digest.
         * @param data data to hash.
         * @param size data size.
         * @return SHA1 message digest.
         */
        static BytesArray sha1 (const char* data, std::streamsize size);

        /**
         * @brief get SHA1 message digest.
         * @param data data to hash.
         * @return SHA1 message digest.
         */
        static BytesArray sha1 (const BytesArray& data);

        /**
         * @brief get SHA1 message digest.
         * @param data data to hash.
         * @return SHA1 message digest.
         */
        static BytesArray sha1 (const std::string& data);

        /**
         * @brief get SHA224 message digest.
         * @param data data to hash.
         * @param size data size.
         * @return SHA224 message digest.
         */
        static BytesArray sha224 (const char* data, std::streamsize size);

        /**
         * @brief get SHA224 message digest.
         * @param data data to hash.
         * @return SHA224 message digest.
         */
        static BytesArray sha224 (const BytesArray& data);

        /**
         * @brief get SHA224 message digest.
         * @param data data to hash.
         * @return SHA224 message digest.
         */
        static BytesArray sha224 (const std::string& data);

        /**
         * @brief get SHA256 message digest.
         * @param data data to hash.
         * @param size data size.
         * @return SHA256 message digest.
         */
        static BytesArray sha256 (const char* data, std::streamsize size);

        /**
         * @brief get SHA256 message digest.
         * @param data data to hash.
         * @return SHA256 message digest.
         */
        static BytesArray sha256 (const BytesArray& data);

        /**
         * @brief get SHA256 message digest.
         * @param data data to hash.
         * @return SHA256 message digest.
         */
        static BytesArray sha256 (const std::string& data);

        /**
         * @brief get SHA384 message digest.
         * @param data data to hash.
         * @param size data size.
         * @return SHA384 message digest.
         */
        static BytesArray sha384 (const char* data, std::streamsize size);

        /**
         * @brief get SHA384 message digest.
         * @param data data to hash.
         * @return SHA384 message digest.
         */
        static BytesArray sha384 (const BytesArray& data);

        /**
         * @brief get SHA384 message digest.
         * @param data data to hash.
         * @return SHA384 message digest.
         */
        static BytesArray sha384 (const std::string& data);

        /**
         * @brief get SHA512 message digest.
         * @param data data to hash.
         * @param size data size.
         * @return SHA512 message digest.
         */
        static BytesArray sha512 (const char* data, std::streamsize size);

        /**
         * @brief get SHA512 message digest.
         * @param data data to hash.
         * @return SHA512 message digest.
         */
        static BytesArray sha512 (const BytesArray& data);

        /**
         * @brief get SHA512 message digest.
         * @param data data to hash.
         * @return SHA512 message digest.
         */
        static BytesArray sha512 (const std::string& data);

        /**
         * @brief convert bytes array to string.
         * @param bin bytes array.
         * @return converted bytes array string.
         */
        static std::string bin2hex (const BytesArray& bin);

        /**
         * @brief get algorithm name.
         * @param algo digest algorithm.
         * @return algorithm name.
         */
        static const char* algorithm (Algorithm algo);

    private:
        /// associated digest stream buffer.
        Digestbuf _digestbuf;
    };
}

namespace std
{
    /// digest error code specialization.
    template <> struct is_error_condition_enum <join::DigestErrc> : public true_type {};
}

#endif
