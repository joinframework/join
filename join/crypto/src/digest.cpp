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

// libjoin.
#include <join/utils.hpp>
#include <join/error.hpp>
#include <join/digest.hpp>

// Libraries.
#include <openssl/err.h>

// C++.
#include <limits>
#include <iostream>

// C.
#include <cstdio>

using join::Digest;
using join::DigestCategory;
using join::BytesArray;

// =========================================================================
//   CLASS     : DigestCategory
//   METHOD    : name
// =========================================================================
const char* DigestCategory::name () const noexcept
{
    return "libjoin";
}

// =========================================================================
//   CLASS     : DigestCategory
//   METHOD    : message
// =========================================================================
std::string DigestCategory::message (int code) const
{
    switch (static_cast <DigestErrc> (code))
    {
        case DigestErrc::InvalidAlgorithm:
            return "invalid digest algorithm";
        case DigestErrc::InvalidPrivateKey:
            return "invalid private key";
        case DigestErrc::InvalidPublicKey:
            return "invalid public key";
        case DigestErrc::InvalidSignature:
            return "invalid signature";
        default:
            return "success";
    }
}

// =========================================================================
//   CLASS     :
//   METHOD    : getDigestCategory
// =========================================================================
const std::error_category& join::getDigestCategory ()
{
    static DigestCategory instance;
    return instance;
}

// =========================================================================
//   CLASS     :
//   METHOD    : getAlgorithmName
// =========================================================================
std::error_code join::make_error_code (DigestErrc code)
{
    return std::error_code (static_cast <int> (code), getDigestCategory ());
}

// =========================================================================
//   CLASS     :
//   METHOD    : getAlgorithmName
// =========================================================================
std::error_condition join::make_error_condition (DigestErrc code)
{
    return std::error_condition (static_cast <int> (code), getDigestCategory ());
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sign
// =========================================================================
BytesArray Digest::sign (const std::string& message, const std::string& privateKey, Algorithm algorithm)
{
    return sign (reinterpret_cast <const uint8_t*> (message.data ()), message.size (), privateKey, algorithm);
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sign
// =========================================================================
BytesArray Digest::sign (const BytesArray& data, const std::string& privateKey, Algorithm algorithm)
{
    return sign (data.data (), data.size (), privateKey, algorithm);
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sign
// =========================================================================
BytesArray Digest::sign (const uint8_t* data, size_t size, const std::string& privateKey, Algorithm algorithm)
{
    // open private key file.
    FILE *fkey = fopen (privateKey.c_str (), "r");
    if (!fkey)
    {
        lastError = std::make_error_code (static_cast <std::errc> (errno));
        return {};
    }

    // read private key.
    EvpPkeyPtr pkey (PEM_read_PrivateKey (fkey, nullptr, 0, nullptr));
    fclose (fkey);

    if (!pkey)
    {
        lastError = make_error_code (DigestErrc::InvalidPrivateKey);
        return {};
    }

    // create EVP context.
    EvpMdCtxPtr mdctx (EVP_MD_CTX_new ());
    if (!mdctx)
    {
        lastError = make_error_code (Errc::OutOfMemory);
        return {};
    }

    // create digest.
    const EVP_MD *md = EVP_get_digestbyname (algorithmName (algorithm));
    if (!md)
    {
        lastError = make_error_code (DigestErrc::InvalidAlgorithm);
        return {};
    }

    // initialize digest sign.
    int result = EVP_DigestSignInit (mdctx.get (), nullptr, md, nullptr, pkey.get ());
    if (1 != result)
    {
        lastError = make_error_code (DigestErrc::InvalidPrivateKey);
        return {};
    }

    // update digest sign.
    result = EVP_DigestSignUpdate (mdctx.get (), data, size);
    if (1 != result)
    {
        lastError = make_error_code (Errc::InvalidParam);
        return {};
    }

    size_t siglen = 0;

    // how much space we need to reserve.
    result = EVP_DigestSignFinal (mdctx.get (), nullptr, &siglen);
    if (1 != result)
    {
        lastError = make_error_code (Errc::OperationFailed);
        return {};
    }

    // check buffer size.
    if ((siglen == 0) || (siglen > static_cast <size_t> (std::numeric_limits <int>::max ())))
    {
        lastError = make_error_code (Errc::OperationFailed);
        return {};
    }

    BytesArray signature;

    // reserve buffer space.
    signature.resize (siglen);

    // finalize digest sign.
    result = EVP_DigestSignFinal (mdctx.get (), signature.data (), &siglen);
    if (1 != result)
    {
        lastError = make_error_code (Errc::OperationFailed);
        return {};
    }

    // need to resize again to fit real used space.
    signature.resize (siglen);

    return signature;
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : verifySignature
// =========================================================================
bool Digest::verifySignature (const std::string& message, const BytesArray& signature, const std::string& publicKey, Algorithm algorithm)
{
    return verifySignature (reinterpret_cast <const uint8_t*> (message.data ()), message.size (), signature, publicKey, algorithm);
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : verifySignature
// =========================================================================
bool Digest::verifySignature (const BytesArray& data, const BytesArray& signature, const std::string& publicKey, Algorithm algorithm)
{
    return verifySignature (data.data (), data.size (), signature, publicKey, algorithm);
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : verifySignature
// =========================================================================
bool Digest::verifySignature (const uint8_t* data, size_t size, const BytesArray &signature, const std::string &publicKey, Algorithm algorithm)
{
    // open public key file.
    FILE *fkey = fopen (publicKey.c_str (), "r");
    if (!fkey)
    {
        lastError = std::make_error_code (static_cast <std::errc> (errno));
        return false;
    }

    // read public key.
    EvpPkeyPtr pkey (PEM_read_PUBKEY (fkey, nullptr, 0, nullptr));
    fclose (fkey);

    if (!pkey)
    {
        lastError = make_error_code (DigestErrc::InvalidPublicKey);
        return false;
    }

    // create EVP context.
    EvpMdCtxPtr mdctx (EVP_MD_CTX_new ());
    if (!mdctx)
    {
        lastError = make_error_code (Errc::OutOfMemory);
        return false;
    }

    // create digest.
    const EVP_MD *md = EVP_get_digestbyname (algorithmName (algorithm));
    if (!md)
    {
        lastError = make_error_code (DigestErrc::InvalidAlgorithm);
        return false;
    }

    // initialize digest verify.
    int result = EVP_DigestVerifyInit (mdctx.get (), nullptr, md, nullptr, pkey.get ());
    if (1 != result)
    {
        lastError = make_error_code (DigestErrc::InvalidPublicKey);
        return false;
    }

    // update digest verify.
    result = EVP_DigestVerifyUpdate (mdctx.get (), data, size);
    if (1 != result)
    {
        lastError = make_error_code (Errc::InvalidParam);
        return false;
    }

    // finalize digest verify.
    result = EVP_DigestVerifyFinal (mdctx.get (), signature.data (), signature.size ());
    if (1 != result)
    {
        lastError = make_error_code (DigestErrc::InvalidSignature);
        return false;
    }

    return true;
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : toFlat
// =========================================================================
BytesArray Digest::toFlat (const BytesArray& der)
{
    const unsigned char* p = der.data ();
    EcdsaSigPtr sig (d2i_ECDSA_SIG (nullptr, &p, der.size ()));
    if (!sig)
    {
        lastError = make_error_code (DigestErrc::InvalidSignature);
        return {};
    }

    const BIGNUM *r = nullptr, *s = nullptr;

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
    ECDSA_SIG_get0 (sig.get (), &r, &s);
#else /* OPENSSL_VERSION_NUMBER >= 0x10100000L */
    r = sig->r;
    s = sig->s;
#endif /* OPENSSL_VERSION_NUMBER >= 0x10100000L */
    if (!r || !s)
    {
        lastError = make_error_code (DigestErrc::InvalidSignature);
        return {};
    }

    // how much space do we need to reserve.
    int size = std::max (fixedSize (BN_num_bytes (r)), fixedSize (BN_num_bytes (s)));
    if (size < 0)
    {
        lastError = make_error_code (DigestErrc::InvalidSignature);
        return {};
    }

    BytesArray flat;
    flat.resize (2 * size);

    // need to pad if required.
    if (!BN_bn2binpad (r, reinterpret_cast <unsigned char*> (flat.data ()), size) ||
        !BN_bn2binpad (s, reinterpret_cast <unsigned char*> (flat.data ()) + size, size))
    {
        lastError = make_error_code (Errc::OperationFailed);
        return {};
    }

    return flat;
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : toDer
// =========================================================================
BytesArray Digest::toDer (const BytesArray& flat)
{
    EcdsaSigPtr sig (ECDSA_SIG_new ());
    if (!sig)
    {
        lastError = make_error_code (Errc::OutOfMemory);
        return {};
    }

    BigNumPtr r (BN_bin2bn (flat.data (),  flat.size () / 2, nullptr));
    BigNumPtr s (BN_bin2bn (flat.data () + flat.size () / 2, flat.size () / 2, nullptr));
    if (!r || !s)
    {
        lastError = make_error_code (Errc::OutOfMemory);
        return {};
    }

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
    ECDSA_SIG_set0 (sig.get (), r.release (), s.release ());
#else /* OPENSSL_VERSION_NUMBER >= 0x10100000L */
    BN_free (sig->r);
    sig->r = r.release ();

    BN_free (sig->s);
    sig->s = s.release ();
#endif /* OPENSSL_VERSION_NUMBER >= 0x10100000L */

    // how much space we need.
    int siglen = i2d_ECDSA_SIG (sig.get (), nullptr);
    if (siglen < 0)
    {
        lastError = make_error_code (DigestErrc::InvalidSignature);
        return {};
    }

    BytesArray der;
    der.resize (siglen);

    // convert flat signature to DER encoded signature.
    unsigned char* p = der.data ();
    siglen = i2d_ECDSA_SIG (sig.get (), &p);
    if (siglen < 0)
    {
        lastError = make_error_code (DigestErrc::InvalidSignature);
        return {};
    }

    // need to resize again.
    der.resize (siglen);

    return der;
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : algorithmName
// =========================================================================
const char* Digest::algorithmName (Algorithm algorithm)
{
   switch (algorithm)
   {
      OUT_ENUM (SHA224);
      OUT_ENUM (SHA256);
      OUT_ENUM (SHA384);
      OUT_ENUM (SHA512);
   }

   return "UNKNOWN";
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : getFixedSize
// =========================================================================
int Digest::fixedSize (int numBytes)
{
    // the fixed size for a flat signature integer depends on the chosen ECDSA curve.
    // curve order can be 112, 128, 160, 192, 224, 256, 384 or 521 bits.
    // to determine the flat signature possible integers size we can devide those values by 8.
    // so possible values are only 14, 16, 20, 24, 28, 32, 48, 66.
    return (numBytes > 48) ? 66 : (numBytes > 32) ? 48 : (numBytes > 28) ? 32 :
           (numBytes > 24) ? 28 : (numBytes > 20) ? 24 : (numBytes > 16) ? 20 :
           (numBytes > 14) ? 16 : 14;
}
