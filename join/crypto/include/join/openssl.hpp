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

#ifndef __JOIN_OPENSSL_HPP__
#define __JOIN_OPENSSL_HPP__

// Libraries.
#include <openssl/bn.h>
#include <openssl/ecdsa.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>
#include <openssl/x509v3.h>
#include <openssl/ssl.h>

// C++.
#include <memory>
#include <vector>
#include <string>

namespace join
{
namespace crypto
{
    /**
     * @brief custom functor for BIGNUM deletion.
     */
    struct BigNumDelete
    {
        constexpr BigNumDelete () noexcept = default;

        void operator ()(BIGNUM* num)
        {
            BN_free (num);
        }
    };

    using BigNumPtr = std::unique_ptr <BIGNUM, BigNumDelete>;

    /**
     * @brief custom functor for ECDSA_SIG deletion.
     */
    struct EcdsaSigDelete
    {
        constexpr EcdsaSigDelete () noexcept = default;

        void operator ()(ECDSA_SIG* ecSig)
        {
            ECDSA_SIG_free (ecSig);
        }
    };

    using EcdsaSigPtr = std::unique_ptr <ECDSA_SIG, EcdsaSigDelete>;

    /**
     * @brief custom functor for EVP_PKEY deletion.
     */
    struct EvpPkeyDelete
    {
        constexpr EvpPkeyDelete () noexcept = default;

        void operator ()(EVP_PKEY* evpPkey)
        {
            EVP_PKEY_free (evpPkey);
        }
    };

    using EvpPkeyPtr = std::unique_ptr <EVP_PKEY, EvpPkeyDelete>;

    /**
     * @brief custom functor for EVP_MD_CTX deletion.
     */
    struct EvpMdCtxDelete
    {
        constexpr EvpMdCtxDelete () noexcept = default;

        void operator ()(EVP_MD_CTX* evpMdCtx)
        {
            EVP_MD_CTX_free (evpMdCtx);
        }
    };

    using EvpMdCtxPtr = std::unique_ptr <EVP_MD_CTX, EvpMdCtxDelete>;

    /**
     * @brief Custom functor for STACK_OF(X509_NAME) deletion.
     */
    struct StackOfX509NameDelete
    {
        constexpr StackOfX509NameDelete () noexcept = default;

        void operator()(STACK_OF (X509_NAME)* stack)
        {
            sk_X509_NAME_pop_free (stack, X509_NAME_free);
        }
    };

    using StackOfX509NamePtr = std::unique_ptr <STACK_OF (X509_NAME), StackOfX509NameDelete>;

    /**
     * @brief custom functor for STACK_OF(GENERAL_NAME) deletion.
     */
    struct StackOfGeneralNameDelete
    {
        constexpr StackOfGeneralNameDelete () noexcept = default;

        void operator ()(STACK_OF (GENERAL_NAME)* stack)
        {
            sk_GENERAL_NAME_pop_free (stack, GENERAL_NAME_free);
        }
    };

    using StackOfGeneralNamePtr = std::unique_ptr <STACK_OF (GENERAL_NAME), StackOfGeneralNameDelete>;

    /**
     * @brief custom functor for SSL handle deletion.
     */
    struct SslDelete
    {
        constexpr SslDelete () noexcept = default;

        void operator ()(SSL* p)
        {
            SSL_free (p);
        }
    };

    using SslPtr = std::unique_ptr <SSL, SslDelete>;

    /**
     * @brief custom functor for SSL context deletion.
     */
    struct SslCtxDelete
    {
        constexpr SslCtxDelete () noexcept = default;

        void operator ()(SSL_CTX* p)
        {
            SSL_CTX_free (p);
        }
    };

    using SslCtxPtr = std::shared_ptr <SSL_CTX>;

#if OPENSSL_VERSION_NUMBER < 0x30000000L
    /**
     * @brief Custom functor for DH key deletion.
     */
    struct DhKeyDelete
    {
        constexpr DhKeyDelete () noexcept = default;

        void operator()(DH* p)
        {
            DH_free (p);
        }
    };

    using DhKeyPtr = std::unique_ptr <DH, DhKeyDelete>;

    /**
     * @brief Custom functor for ECDH key deletion.
     */
    struct EcdhKeyDelete
    {
        constexpr EcdhKeyDelete () noexcept = default;

        void operator()(EC_KEY* p)
        {
            EC_KEY_free (p);
        }
    };

    using EcdhKeyPtr = std::unique_ptr <EC_KEY, EcdhKeyDelete>;
#endif

    /**
     * @brief initialize the OpenSSL libraries.
     * @note the OpenSSL library should be initialized only once
     *       and at the early beginning of the process.
     *       this function will ensure that the libraries initialization
     *       can't be done multiple times even if called in multiple places.
     */
    void initializeOpenSSL ();

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
    const std::string defaultCipher_ = "EECDH+ECDSA+AESGCM:EECDH+aRSA+AESGCM:EECDH+ECDSA+CHACHA20:EECDH+aRSA+CHACHA20:EECDH+ECDSA+AESCCM:"
                                       "EDH+DSS+AESGCM:EDH+aRSA+CHACHA20:EDH+aRSA+AESCCM:-AESCCM8:EECDH+ECDSA+AESCCM8:EDH+aRSA+AESCCM8";

#if OPENSSL_VERSION_NUMBER >= 0x10101000L
    const std::string defaultCipher_1_3_ = "TLS_AES_256_GCM_SHA384:TLS_AES_128_GCM_SHA256:TLS_CHACHA20_POLY1305_SHA256:"
                                           "TLS_AES_128_CCM_SHA256:TLS_AES_128_CCM_8_SHA256";
#endif // OPENSSL_VERSION_NUMBER >= 0x1010101fL

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    const std::string defaultCurve_ = "prime256v1";
#endif // OPENSSL_VERSION_NUMBER >= 0x30000000L
}
}
#else // OPENSSL_VERSION_NUMBER >= 0x10100000L
    const std::string defaultCipher = "EECDH+ECDSA+AESGCM:EECDH+aRSA+AESGCM:EDH+DSS+AESGCM";
}
}

#define ASN1_STRING_get0_data ASN1_STRING_data

int DH_set0_pqg (DH *dh, BIGNUM *p, BIGNUM *q, BIGNUM *g);

#endif // OPENSSL_VERSION_NUMBER >= 0x10100000L

#endif
