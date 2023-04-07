/**
 * MIT License
 *
 * Copyright (c) 2022 Mathieu Rabine
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

#ifndef __JOIN_SIGNATURE_HPP__
#define __JOIN_SIGNATURE_HPP__

// libjoin.
#include <join/base64.hpp>

// C++.
#include <system_error>
#include <future>

namespace join
{
    /**
     * @brief signature error codes.
     */
    enum class SigErrc
    {
        InvalidAlgorithm = 1,   /**< invalid algorithm. */
        InvalidPrivateKey,      /**< invalid private key. */
        InvalidPublicKey,       /**< invalid public key. */
        InvalidSignature        /**< invalid signature. */
    };

    /**
     * @brief signature error category.
     */
    class SigCategory : public std::error_category
    {
    public:
        /**
         * @brief get signature error category name.
         * @return signature error category name.
         */
        virtual const char* name () const noexcept;

        /**
         * @brief translate signature error code to human readable error string.
         * @param code error code.
         * @return human readable error string.
         */
        virtual std::string message (int code) const;
    };

    /**
     * @brief get error category.
     * @return the created std::error_category object.
     */
    const std::error_category& getSigCategory ();

    /**
     * @brief create an std::error_code object.
     * @param code error code number.
     * @return the created std::error_code object.
     */
    std::error_code make_error_code (SigErrc code);

    /**
     * @brief create an std::error_condition object.
     * @param code error code number.
     * @return the created std::error_condition object.
     */
    std::error_condition make_error_condition (SigErrc code);

    /**
     * @brief class used to manage signature.
     */
    class Signature
    {
    public:
        /**
         * @brief algorithm.
         */
        enum Algorithm : uint16_t
        {
            SHA224,     /**< secure hash algorithm v2 with a 224 bits digest */
            SHA256,     /**< secure hash algorithm v2 with a 256 bits digest */
            SHA384,     /**< secure hash algorithm v2 with a 384 bits digest */
            SHA512      /**< secure hash algorithm v2 with a 512 bits digest */
        };

        /**
         * @brief create instance.
         */
        Signature () = delete;

        /**
         * @brief destroy instance.
         */
        ~Signature () = delete;

        /**
         * @brief sign a data with given private key.
         * @param message message to sign.
         * @param privateKey path to private key.
         * @param algorithm the algorithm used for the signature.
         * @return the generated signature on success, an empty bytes array on failure.
         */
        static BytesArray sign (const std::string& message, const std::string& privateKey, Algorithm algorithm = SHA256);

        /**
         * @brief sign a data with given private key.
         * @param data data to sign.
         * @param privateKey path to private key.
         * @param algorithm the algorithm used for the signature.
         * @return the generated signature on success, an empty bytes array on failure.
         */
        static BytesArray sign (const BytesArray& data, const std::string& privateKey, Algorithm algorithm = SHA256);

        /**
         * @brief sign a data with given private key.
         * @param data data buffer to sign.
         * @param size data buffer size.
         * @param privateKey path to private key.
         * @param algorithm the algorithm used for the signature.
         * @return the generated signature on success, an empty bytes array on failure.
         */
        static BytesArray sign (const uint8_t* data, size_t size, const std::string& privateKey, Algorithm algorithm = SHA256);

        /**
         * @brief verify data signature.
         * @param message message to verify.
         * @param signature the message signature.
         * @param publicKey path to public key.
         * @param algorithm the algorithm used for the signature.
         * @return true on success, false otherwise.
         */
        static bool verify (const std::string& message, const BytesArray& signature, const std::string& publicKey, Algorithm algorithm = SHA256);

        /**
         * @brief verify data signature.
         * @param data data to verify.
         * @param signature the message signature.
         * @param publicKey path to public key.
         * @param algorithm the algorithm used for the signature.
         * @return true on success, false otherwise.
         */
        static bool verify (const BytesArray& data, const BytesArray& signature, const std::string& publicKey, Algorithm algorithm = SHA256);

        /**
         * @brief verify data signature.
         * @param data data buffer to verify.
         * @param data data buffer size to verify.
         * @param signature the message signature.
         * @param publicKey path to public key.
         * @param algorithm the algorithm used for the signature.
         * @return true on success, false otherwise.
         */
        static bool verify (const uint8_t* data, size_t size, const BytesArray& signature, const std::string& publicKey, Algorithm algorithm = SHA256);

        /**
         * @brief convert a DER encoded ECDSA signature to a flat ECDSA signature.
         * @param der the DER encoded ECDSA signature to convert.
         * @return the flat ECDSA signature on success, an empty bytes array on failure.
         */
        static BytesArray toFlat (const BytesArray& der);

        /**
         * @brief convert a flat ECDSA signature to a DER encoded ECDSA signature.
         * @param flat the flat ECDSA signature to convert.
         * @return the DER encoded ECDSA signature on success, an empty bytes array on failure.
         */
        static BytesArray toDer (const BytesArray& flat);

        /**
         * @brief get algorithm name.
         * @param algorithm the signature algorithm.
         * @return algorithm name.
         */
        static const char* algorithmName (Algorithm algorithm);

    private:
        /**
         * @brief get the fixed size for a flat signature integer.
         * @param numBytes number of bytes set.
         * @return the fixed size for a flat signature integer.
         */
        static int fixedSize (int numBytes);
    };
}

namespace std
{
    /// signature error code specialization.
    template <> struct is_error_condition_enum <join::SigErrc> : public true_type {};
}

#endif
