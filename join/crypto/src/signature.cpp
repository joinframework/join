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
#include <join/signature.hpp>
#include <join/error.hpp>
#include <join/utils.hpp>

// Libraries.
#include <openssl/err.h>

// C++.
#include <limits>
#include <iostream>

// C.
#include <cstdio>

using join::Signature;
using join::SigCategory;
using join::BytesArray;

// =========================================================================
//   CLASS     : SigCategory
//   METHOD    : name
// =========================================================================
const char* SigCategory::name () const noexcept
{
    return "libjoin";
}

// =========================================================================
//   CLASS     : SigCategory
//   METHOD    : message
// =========================================================================
std::string SigCategory::message (int code) const
{
    switch (static_cast <SigErrc> (code))
    {
        case SigErrc::InvalidAlgorithm:
            return "invalid algorithm";
        case SigErrc::InvalidPrivateKey:
            return "invalid private key";
        case SigErrc::InvalidPublicKey:
            return "invalid public key";
        case SigErrc::InvalidSignature:
            return "invalid signature";
        default:
            return "success";
    }
}

// =========================================================================
//   CLASS     :
//   METHOD    : getSigCategory
// =========================================================================
const std::error_category& join::getSigCategory ()
{
    static SigCategory instance;
    return instance;
}

// =========================================================================
//   CLASS     :
//   METHOD    : getAlgorithmName
// =========================================================================
std::error_code join::make_error_code (SigErrc code)
{
    return std::error_code (static_cast <int> (code), getSigCategory ());
}

// =========================================================================
//   CLASS     :
//   METHOD    : getAlgorithmName
// =========================================================================
std::error_condition join::make_error_condition (SigErrc code)
{
    return std::error_condition (static_cast <int> (code), getSigCategory ());
}

// =========================================================================
//   CLASS     : Signature
//   METHOD    : sign
// =========================================================================
BytesArray Signature::sign (const std::string& message, const std::string& privateKey, Algorithm algorithm)
{
    return sign (reinterpret_cast <const uint8_t*> (message.data ()), message.size (), privateKey, algorithm);
}

// =========================================================================
//   CLASS     : Signature
//   METHOD    : sign
// =========================================================================
BytesArray Signature::sign (const BytesArray& data, const std::string& privateKey, Algorithm algorithm)
{
    return sign (data.data (), data.size (), privateKey, algorithm);
}

// =========================================================================
//   CLASS     : Signature
//   METHOD    : sign
// =========================================================================
BytesArray Signature::sign (const uint8_t* data, size_t size, const std::string& privateKey, Algorithm algorithm)
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
        lastError = make_error_code (SigErrc::InvalidPrivateKey);
        return {};
    }

    // create digest.
    const EVP_MD *md = EVP_get_digestbyname (algorithmName (algorithm));
    if (!md)
    {
        lastError = make_error_code (SigErrc::InvalidAlgorithm);
        return {};
    }

    // create EVP context.
    EvpMdCtxPtr mdctx (EVP_MD_CTX_new ());
    if (!mdctx)
    {
        lastError = make_error_code (Errc::OutOfMemory);
        return {};
    }

    // initialize.
    int result = EVP_DigestSignInit (mdctx.get (), nullptr, md, nullptr, pkey.get ());
    if (result != 1)
    {
        lastError = make_error_code (SigErrc::InvalidAlgorithm);
        return {};
    }

    size_t siglen = 0;

    // how much space we need to reserve.
    result = EVP_DigestSign (mdctx.get (), nullptr, &siglen, data, size);
    if (result != 1)
    {
        lastError = make_error_code (Errc::OperationFailed);
        return {};
    }

    BytesArray signature;
    signature.resize (siglen);

    // sign.
    result = EVP_DigestSign (mdctx.get (), signature.data (), &siglen, data, size);
    if (result != 1)
    {
        lastError = make_error_code (Errc::OperationFailed);
        return {};
    }

    // need to resize again to fit real used space.
    signature.resize (siglen);

    return signature;
}

// =========================================================================
//   CLASS     : Signature
//   METHOD    : verify
// =========================================================================
bool Signature::verify (const std::string& message, const BytesArray& signature, const std::string& publicKey, Algorithm algorithm)
{
    return verify (reinterpret_cast <const uint8_t*> (message.data ()), message.size (), signature, publicKey, algorithm);
}

// =========================================================================
//   CLASS     : Signature
//   METHOD    : verify
// =========================================================================
bool Signature::verify (const BytesArray& data, const BytesArray& signature, const std::string& publicKey, Algorithm algorithm)
{
    return verify (data.data (), data.size (), signature, publicKey, algorithm);
}

// =========================================================================
//   CLASS     : Signature
//   METHOD    : verify
// =========================================================================
bool Signature::verify (const uint8_t* data, size_t size, const BytesArray &signature, const std::string &publicKey, Algorithm algorithm)
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
        lastError = make_error_code (SigErrc::InvalidPublicKey);
        return false;
    }

    // create digest.
    const EVP_MD *md = EVP_get_digestbyname (algorithmName (algorithm));
    if (!md)
    {
        lastError = make_error_code (SigErrc::InvalidAlgorithm);
        return false;
    }

    // create EVP context.
    EvpMdCtxPtr mdctx (EVP_MD_CTX_new ());
    if (!mdctx)
    {
        lastError = make_error_code (Errc::OutOfMemory);
        return false;
    }

    // initialize.
    int result = EVP_DigestVerifyInit (mdctx.get (), nullptr, md, nullptr, pkey.get ());
    if (result != 1)
    {
        lastError = make_error_code (SigErrc::InvalidAlgorithm);
        return false;
    }

    // verify.
    result = EVP_DigestVerify (mdctx.get (), signature.data (), signature.size (), data, size);
    if (result != 1)
    {
        lastError = make_error_code (SigErrc::InvalidSignature);
        return false;
    }

    return true;
}

// =========================================================================
//   CLASS     : Signature
//   METHOD    : algorithmName
// =========================================================================
const char* Signature::algorithmName (Algorithm algorithm)
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
