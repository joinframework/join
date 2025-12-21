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

#ifndef __JOIN_SIGNATURE_HPP__
#define __JOIN_SIGNATURE_HPP__

// libjoin.
#include <join/digest.hpp>

// C++.
#include <system_error>
#include <future>

namespace join
{
    /**
     * @brief signature stream buffer.
     */
    class Signaturebuf : public Digestbuf
    {
    public:
        /**
         * @brief create the signature stream buffer instance.
         * @param algo the message digest used.
         */
        Signaturebuf (const std::string& algo);

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        Signaturebuf (const Signaturebuf& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        Signaturebuf& operator= (const Signaturebuf& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        Signaturebuf (Signaturebuf&& other) = default;

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        Signaturebuf& operator= (Signaturebuf&& other) = default;

        /**
         * @brief destroy the signature stream buffer instance.
         */
        virtual ~Signaturebuf () = default;

        /**
         * @brief sign with the given private key.
         * @param privKey path to private key.
         * @return the generated signature on success, an empty bytes array on failure.
         */
        BytesArray sign (const std::string& privKey);

        /**
         * @brief verify signature with the given public key.
         * @param sig the message signature.
         * @param pubKey path to public key.
         * @return true on success, false otherwise.
         */
        bool verify (const BytesArray& sig, const std::string& pubKey);
    };

    /**
     * @brief class used to manage signature.
     */
    class Signature : public std::ostream
    {
    public:
        /**
         * @brief create instance.
         * @param algo the message digest used.
         */
        Signature (Digest::Algorithm algo);

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        Signature (const Signature& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        Signature& operator=(const Signature& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        Signature (Signature&& other);

        /**
         * @brief move assignment operator.
         * @param other other object to assign.
         * @return current object.
         */
        Signature& operator=(Signature&& other);

        /**
         * @brief destroy instance.
         */
        virtual ~Signature () = default;

        /**
         * @brief sign with given private key.
         * @param privKey path to private key.
         * @return the generated signature on success, an empty bytes array on failure.
         */
        BytesArray sign (const std::string& privKey);

        /**
         * @brief sign a data with given private key.
         * @param data data buffer to sign.
         * @param size data buffer size.
         * @param privKey path to private key.
         * @param algo the algorithm used for the signature.
         * @return the generated signature on success, an empty bytes array on failure.
         */
        static BytesArray sign (const char* data, std::streamsize size, const std::string& privKey, Digest::Algorithm algo);

        /**
         * @brief sign a data with given private key.
         * @param data data to sign.
         * @param privKey path to private key.
         * @param algo the algorithm used for the signature.
         * @return the generated signature on success, an empty bytes array on failure.
         */
        static BytesArray sign (const BytesArray& data, const std::string& privKey, Digest::Algorithm algo);

        /**
         * @brief sign a data with given private key.
         * @param data data to sign.
         * @param privKey path to private key.
         * @param algo the algorithm used for the signature.
         * @return the generated signature on success, an empty bytes array on failure.
         */
        static BytesArray sign (const std::string& data, const std::string& privKey, Digest::Algorithm algo);

        /**
         * @brief verify signature.
         * @param signature the message signature.
         * @param pubKey path to public key.
         * @return true on success, false otherwise.
         */
        bool verify (const BytesArray& signature, const std::string& pubKey);

        /**
         * @brief verify data signature.
         * @param data data buffer to verify.
         * @param data data buffer size to verify.
         * @param signature the message signature.
         * @param pubKey path to public key.
         * @param algo the algorithm used for the signature.
         * @return true on success, false otherwise.
         */
        static bool verify (const char* data, std::streamsize size, const BytesArray& signature, const std::string& pubKey, Digest::Algorithm algo);

        /**
         * @brief verify data signature.
         * @param data data to verify.
         * @param signature the message signature.
         * @param pubKey path to public key.
         * @param algo the algorithm used for the signature.
         * @return true on success, false otherwise.
         */
        static bool verify (const BytesArray& data, const BytesArray& signature, const std::string& pubKey, Digest::Algorithm algo);

        /**
         * @brief verify data signature.
         * @param data data to verify.
         * @param signature the message signature.
         * @param pubKey path to public key.
         * @param algo the algorithm used for the signature.
         * @return true on success, false otherwise.
         */
        static bool verify (const std::string& data, const BytesArray& signature, const std::string& pubKey, Digest::Algorithm algo);

    private:
        /// associated signature stream buffer.
        Signaturebuf _sigbuf;
    };
}

#endif
