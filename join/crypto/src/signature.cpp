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

// libjoin.
#include <join/signature.hpp>
#include <join/tlskey.hpp>
#include <join/error.hpp>
#include <join/utils.hpp>

// Libraries.
#include <openssl/err.h>

// C++.
#include <iostream>
#include <limits>

// C.
#include <cstdint>
#include <cstdio>

using join::Digest;
using join::TlsKey;
using join::Signaturebuf;
using join::Signature;
using join::BytesArray;

// =========================================================================
//   CLASS     : Signaturebuf
//   METHOD    : Signaturebuf
// =========================================================================
Signaturebuf::Signaturebuf (const std::string& algo)
: Digestbuf (algo)
{
}

// =========================================================================
//   CLASS     : Signaturebuf
//   METHOD    : sign
// =========================================================================
BytesArray Signaturebuf::sign (const std::string& privKey)
{
    BytesArray sig;

    if (_buf && (this->overflow (traits_type::eof ()) != traits_type::eof ()))
    {
        TlsKey key (privKey, TlsKey::Private);
        sig.resize (EVP_PKEY_size (key.handle ()));
        uint32_t siglen = 0;

        int ret = EVP_SignFinal (_ctx.get (), &sig[0], &siglen, key.handle ());
        sig.resize (siglen);

        if (ret == 0)
        {
            lastError = make_error_code (DigestErrc::InvalidAlgorithm);
        }
    }

    _ctx.reset ();

    return sig;
}

// =========================================================================
//   CLASS     : Signaturebuf
//   METHOD    : verify
// =========================================================================
bool Signaturebuf::verify (const BytesArray& sig, const std::string& pubKey)
{
    bool result = false;

    if (_buf && (this->overflow (traits_type::eof ()) != traits_type::eof ()))
    {
        TlsKey key (pubKey, TlsKey::Public);

        int ret = EVP_VerifyFinal (_ctx.get (), sig.data (), sig.size (), key.handle ());
        switch (ret)
        {
            case 1:
                result = true;
                break;
            case 0:
                lastError = make_error_code (DigestErrc::InvalidSignature);
                break;
            default:
                lastError = make_error_code (DigestErrc::InvalidAlgorithm);
                break;
        }
    }

    _ctx.reset ();

    return result;
}

// =========================================================================
//   CLASS     : Signature
//   METHOD    : Signature
// =========================================================================
Signature::Signature (Digest::Algorithm algo)
: _sigbuf (Digest::algorithm (algo))
{
    this->init (&_sigbuf);
}

// =========================================================================
//   CLASS     : Signature
//   METHOD    : Signature
// =========================================================================
Signature::Signature (Signature&& other)
: std::ostream (std::move (other)),
  _sigbuf (std::move (other._sigbuf))
{
    this->set_rdbuf (&_sigbuf);
}

// =========================================================================
//   CLASS     : DigSignatureest
//   METHOD    : operator=
// =========================================================================
Signature& Signature::operator=(Signature&& other)
{
    std::ostream::operator= (std::move (other));
    _sigbuf = std::move (other._sigbuf);
    return *this;
}

// =========================================================================
//   CLASS     : Signature
//   METHOD    : sign
// =========================================================================
BytesArray Signature::sign (const std::string& privKey)
{
    return _sigbuf.sign (privKey);
}

// =========================================================================
//   CLASS     : Signature
//   METHOD    : sign
// =========================================================================
BytesArray Signature::sign (const char* data, std::streamsize size, const std::string& privKey, Digest::Algorithm algo)
{
    try
    {
        Signature sig (algo);
        sig.write (data, size);
        return sig.sign (privKey);
    }
    catch (const std::system_error& ex)
    {
        lastError = ex.code ();
        return {};
    }
}

// =========================================================================
//   CLASS     : Signature
//   METHOD    : sign
// =========================================================================
BytesArray Signature::sign (const BytesArray& data, const std::string& privKey, Digest::Algorithm algo)
{
    return sign (reinterpret_cast <const char*> (data.data ()), data.size (), privKey, algo);
}

// =========================================================================
//   CLASS     : Signature
//   METHOD    : sign
// =========================================================================
BytesArray Signature::sign (const std::string& data, const std::string& privKey, Digest::Algorithm algo)
{
    return sign (BytesArray (data.begin (), data.end ()), privKey, algo);
}

// =========================================================================
//   CLASS     : Signature
//   METHOD    : verify
// =========================================================================
bool Signature::verify (const BytesArray& signature, const std::string& pubKey)
{
    return _sigbuf.verify (signature, pubKey);
}

// =========================================================================
//   CLASS     : Signature
//   METHOD    : verify
// =========================================================================
bool Signature::verify (const char* data, std::streamsize size, const BytesArray &signature, const std::string &pubKey, Digest::Algorithm algo)
{
    try
    {
        Signature sig (algo);
        sig.write (data, size);
        return sig.verify (signature, pubKey);
    }
    catch (const std::system_error& ex)
    {
        lastError = ex.code ();
        return {};
    }
}

// =========================================================================
//   CLASS     : Signature
//   METHOD    : verify
// =========================================================================
bool Signature::verify (const BytesArray& data, const BytesArray& signature, const std::string& pubKey, Digest::Algorithm algo)
{
    return verify (reinterpret_cast <const char*> (data.data ()), data.size (), signature, pubKey, algo);
}

// =========================================================================
//   CLASS     : Signature
//   METHOD    : verify
// =========================================================================
bool Signature::verify (const std::string& data, const BytesArray& signature, const std::string& pubKey, Digest::Algorithm algo)
{
    return verify (BytesArray (data.begin (), data.end ()), signature, pubKey, algo);
}
