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
#include <join/tlskey.hpp>
#include <join/digest.hpp>
#include <join/error.hpp>
#include <join/utils.hpp>

using join::DigestErrc;
using join::BytesArray;
using join::Base64;
using join::TlsKey;

// =========================================================================
//   CLASS     : TlsKey
//   METHOD    : TlsKey
// =========================================================================
TlsKey::TlsKey (const std::string& keyPath, Type keyType)
: _type (keyType),
  _key (readKey (keyPath, keyType))
{
    if (_key == nullptr)
    {
        throw std::system_error (lastError);
    }
}

// =========================================================================
//   CLASS     : TlsKey
//   METHOD    : TlsKey
// =========================================================================
TlsKey::TlsKey (TlsKey&& other)
: _type (other._type),
  _key (std::move (other._key))
{
}

// =========================================================================
//   CLASS     : TlsKey
//   METHOD    : operator=
// =========================================================================
TlsKey& TlsKey::operator= (TlsKey&& other)
{
    _type = other._type;
    _key = std::move (other._key);
    return *this;
}

// =========================================================================
//   CLASS     : TlsKey
//   METHOD    : operator bool
// =========================================================================
TlsKey::operator bool () const
{
    return !!_key;
}

// =========================================================================
//   CLASS     : TlsKey
//   METHOD    : handle
// =========================================================================
TlsKey::Handle TlsKey::handle () const
{
    return _key.get ();
}

// =========================================================================
//   CLASS     : TlsKey
//   METHOD    : length
// =========================================================================
int TlsKey::length ()
{
    return (_key) ? EVP_PKEY_bits (_key.get ()) : -1;
}

// =========================================================================
//   CLASS     : TlsKey
//   METHOD    : swap
// =========================================================================
void TlsKey::swap (TlsKey& other)
{
    TlsKey tmp (std::move (*this));
    *this = std::move (other);
    other = std::move (tmp);
}

// =========================================================================
//   CLASS     : TlsKey
//   METHOD    : type
// =========================================================================
TlsKey::Type TlsKey::type () const
{
    return _type;
}

// =========================================================================
//   CLASS     : TlsKey
//   METHOD    : clear
// =========================================================================
void TlsKey::clear ()
{
    _type = Private;
    _key.reset ();
}

// =========================================================================
//   CLASS     : TlsKey
//   METHOD    : readKey
// =========================================================================
TlsKey::Handle TlsKey::readKey (const std::string& path, Type keyType)
{
    FILE* fp = fopen (path.c_str (), "r");
    if (fp == nullptr)
    {
        lastError = std::error_code (errno, std::generic_category ());
        return nullptr;
    }
    Handle pkey = (keyType == Public) ? PEM_read_PUBKEY (fp, nullptr, nullptr, nullptr)
                                      : PEM_read_PrivateKey (fp, nullptr, nullptr, nullptr);
    fclose (fp);
    if (pkey == nullptr)
    {
        lastError = make_error_code (DigestErrc::InvalidKey);
    }
    return pkey;
}
