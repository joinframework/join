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
//   METHOD    : make_error_code
// =========================================================================
std::error_code join::make_error_code (SigErrc code)
{
    return std::error_code (static_cast <int> (code), getSigCategory ());
}

// =========================================================================
//   CLASS     :
//   METHOD    : make_error_condition
// =========================================================================
std::error_condition join::make_error_condition (SigErrc code)
{
    return std::error_condition (static_cast <int> (code), getSigCategory ());
}

// =========================================================================
//   CLASS     : Signature
//   METHOD    : sign
// =========================================================================
BytesArray Signature::sign (const std::string& message, const std::string& privKey, Algorithm algo)
{
    return sign (reinterpret_cast <const uint8_t*> (message.data ()), message.size (), privKey, algo);
}

// =========================================================================
//   CLASS     : Signature
//   METHOD    : sign
// =========================================================================
BytesArray Signature::sign (const BytesArray& data, const std::string& privKey, Algorithm algo)
{
    return sign (data.data (), data.size (), privKey, algo);
}

// =========================================================================
//   CLASS     : Signature
//   METHOD    : sign
// =========================================================================
BytesArray Signature::sign (const uint8_t* data, size_t size, const std::string& privKey, Algorithm algo)
{
    FILE *fkey = fopen (privKey.c_str (), "r");
    if (!fkey)
    {
        lastError = std::make_error_code (static_cast <std::errc> (errno));
        return {};
    }

    EvpPkeyPtr pkey (PEM_read_PrivateKey (fkey, nullptr, 0, nullptr));
    fclose (fkey);

    if (!pkey)
    {
        lastError = make_error_code (SigErrc::InvalidPrivateKey);
        return {};
    }

    const EVP_MD *md = EVP_get_digestbyname (algorithm (algo));
    if (!md)
    {
        lastError = make_error_code (SigErrc::InvalidAlgorithm);
        return {};
    }

    EvpMdCtxPtr mdctx (EVP_MD_CTX_new ());
    size_t siglen = 0;

    if (!EVP_DigestSignInit (mdctx.get (), nullptr, md, nullptr, pkey.get ()) ||
        !EVP_DigestSign (mdctx.get (), nullptr, &siglen, data, size))
    {
        lastError = make_error_code (Errc::OperationFailed);
        return {};
    }

    BytesArray signature;
    signature.resize (siglen);

    EVP_DigestSign (mdctx.get (), signature.data (), &siglen, data, size);
    signature.resize (siglen);

    return signature;
}

// =========================================================================
//   CLASS     : Signature
//   METHOD    : verify
// =========================================================================
bool Signature::verify (const std::string& message, const BytesArray& signature, const std::string& pubKey, Algorithm algo)
{
    return verify (reinterpret_cast <const uint8_t*> (message.data ()), message.size (), signature, pubKey, algo);
}

// =========================================================================
//   CLASS     : Signature
//   METHOD    : verify
// =========================================================================
bool Signature::verify (const BytesArray& data, const BytesArray& signature, const std::string& pubKey, Algorithm algo)
{
    return verify (data.data (), data.size (), signature, pubKey, algo);
}

// =========================================================================
//   CLASS     : Signature
//   METHOD    : verify
// =========================================================================
bool Signature::verify (const uint8_t* data, size_t size, const BytesArray &signature, const std::string &pubKey, Algorithm algo)
{
    FILE *fkey = fopen (pubKey.c_str (), "r");
    if (!fkey)
    {
        lastError = std::make_error_code (static_cast <std::errc> (errno));
        return false;
    }

    EvpPkeyPtr pkey (PEM_read_PUBKEY (fkey, nullptr, 0, nullptr));
    fclose (fkey);

    if (!pkey)
    {
        lastError = make_error_code (SigErrc::InvalidPublicKey);
        return false;
    }

    const EVP_MD *md = EVP_get_digestbyname (algorithm (algo));
    if (!md)
    {
        lastError = make_error_code (SigErrc::InvalidAlgorithm);
        return false;
    }

    EvpMdCtxPtr mdctx (EVP_MD_CTX_new ());

    if (EVP_DigestVerifyInit (mdctx.get (), nullptr, md, nullptr, pkey.get ()) != 1)
    {
        lastError = make_error_code (Errc::OperationFailed);
        return false;
    }

    if (EVP_DigestVerify (mdctx.get (), signature.data (), signature.size (), data, size) != 1)
    {
        lastError = make_error_code (SigErrc::InvalidSignature);
        return false;
    }

    return true;
}

// =========================================================================
//   CLASS     : Signature
//   METHOD    : algorithm
// =========================================================================
const char* Signature::algorithm (Algorithm algo)
{
   switch (algo)
   {
        OUT_ENUM (SHA1);
        OUT_ENUM (SHA224);
        OUT_ENUM (SHA256);
        OUT_ENUM (SHA384);
        OUT_ENUM (SHA512);
        OUT_ENUM (SM3);
   }

   return "UNKNOWN";
}
