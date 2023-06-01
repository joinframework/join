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
#include <join/digest.hpp>
#include <join/error.hpp>
#include <join/utils.hpp>

using join::Errc;
using join::DigestErrc;
using join::DigestCategory;
using join::Digest;
using join::Digestbuf;
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
            return "invalid algorithm";
        case DigestErrc::InvalidKey:
            return "invalid key";
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
//   METHOD    : make_error_code
// =========================================================================
std::error_code join::make_error_code (DigestErrc code)
{
    return std::error_code (static_cast <int> (code), getDigestCategory ());
}

// =========================================================================
//   CLASS     :
//   METHOD    : make_error_condition
// =========================================================================
std::error_condition join::make_error_condition (DigestErrc code)
{
    return std::error_condition (static_cast <int> (code), getDigestCategory ());
}

// =========================================================================
//   CLASS     : Digestbuf
//   METHOD    : Digestbuf
// =========================================================================
Digestbuf::Digestbuf (const std::string& algo)
: _buf (std::make_unique <char []> (_bufsize)),
  _md (EVP_get_digestbyname (algo.c_str ()))
{
    if (_buf == nullptr)
    {
        throw std::system_error (make_error_code (Errc::OutOfMemory));
    }

    if (_md == nullptr)
    {
        throw std::system_error (make_error_code (DigestErrc::InvalidAlgorithm));
    }
}

// =========================================================================
//   CLASS     : Digestbuf
//   METHOD    : Digestbuf
// =========================================================================
Digestbuf::Digestbuf (Digestbuf&& other)
: std::streambuf (std::move (other)),
  _buf (std::move (other._buf)),
  _mdctx (std::move (other._mdctx)),
  _md (other._md)
{
}

// =========================================================================
//   CLASS     : Digestbuf
//   METHOD    : operator=
// =========================================================================
Digestbuf& Digestbuf::operator= (Digestbuf&& other)
{
    std::streambuf::operator= (std::move (other));
    _buf = std::move (other._buf);
    _mdctx = std::move (other._mdctx);
    _md = other._md;
    return *this;
}

// =========================================================================
//   CLASS     : Digestbuf
//   METHOD    : get
// =========================================================================
BytesArray Digestbuf::get ()
{
    if (_buf && (this->overflow (traits_type::eof ()) == traits_type::eof ()))
    {
        _mdctx.reset ();
        return {};
    }

    BytesArray digest;
    digest.resize (EVP_MD_size (_md));

    if (!EVP_DigestFinal_ex (_mdctx.get (), &digest[0], nullptr))
    {
        lastError = make_error_code (Errc::OperationFailed);
        _mdctx.reset ();
        return {};
    }

    _mdctx.reset ();

    return digest;
}

// =========================================================================
//   CLASS     : Digestbuf
//   METHOD    : overflow
// =========================================================================
Digestbuf::int_type Digestbuf::overflow (int_type c)
{
    if (_mdctx == nullptr)
    {
        _mdctx.reset (EVP_MD_CTX_new ());
        if (_mdctx == nullptr)
        {
            lastError = make_error_code (Errc::OutOfMemory);
            return traits_type::eof ();
        }

        if (!EVP_DigestInit_ex (_mdctx.get (), _md, nullptr))
        {
            lastError = make_error_code (Errc::OperationFailed);
            return traits_type::eof ();
        }
    }

    if (this->pbase () == nullptr)
    {
        this->setp (_buf.get (), _buf.get () + _bufsize);
    }

    if ((this->pptr () < this->epptr ()) && (c != traits_type::eof ()))
    {
        return this->sputc (traits_type::to_char_type (c));
    }

    std::streamsize pending = this->pptr () - this->pbase ();

    if (pending && !EVP_DigestUpdate (_mdctx.get (), this->pbase (), pending))
    {
        lastError = make_error_code (Errc::OperationFailed);
        return traits_type::eof ();
    }

    this->setp (this->pbase (), this->pbase () + _bufsize);

    if (c == traits_type::eof ())
    {
        return traits_type::not_eof (c);
    }

    return this->sputc (traits_type::to_char_type (c));
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : Digest
// =========================================================================
Digest::Digest (Algorithm algo)
: _digestbuf (algorithm (algo))
{
    this->init (&_digestbuf);
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : Digest
// =========================================================================
Digest::Digest (Digest&& other)
: std::ostream (std::move (other)),
  _digestbuf (std::move (other._digestbuf))
{
    this->set_rdbuf (&_digestbuf);
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : Digest
// =========================================================================
Digest& Digest::operator=(Digest&& other)
{
    std::ostream::operator= (std::move (other));
    _digestbuf = std::move (other._digestbuf);
    return *this;
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : get
// =========================================================================
BytesArray Digest::get ()
{
    BytesArray digest = _digestbuf.get ();
    if (digest.empty ())
    {
        this->setstate (std::ios_base::failbit);
    }
    return digest;
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha1
// =========================================================================
BytesArray Digest::sha1 (const char* data, std::streamsize size)
{
    Digest digest (SHA1);
    digest.write (data, size);
    return digest.get ();
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha1
// =========================================================================
BytesArray Digest::sha1 (const BytesArray& data)
{
    return sha1 (reinterpret_cast <const char*> (data.data ()), data.size ());
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha1
// =========================================================================
BytesArray Digest::sha1 (const std::string& data)
{
    return sha1 (BytesArray (data.begin (), data.end ()));
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha224
// =========================================================================
BytesArray Digest::sha224 (const char* data, std::streamsize size)
{
    Digest digest (SHA224);
    digest.write (data, size);
    return digest.get ();
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha224
// =========================================================================
BytesArray Digest::sha224 (const BytesArray& data)
{
    return sha224 (reinterpret_cast <const char*> (data.data ()), data.size ());
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha224
// =========================================================================
BytesArray Digest::sha224 (const std::string& data)
{
    return sha224 (BytesArray (data.begin (), data.end ()));
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha256
// =========================================================================
BytesArray Digest::sha256 (const char* data, std::streamsize size)
{
    Digest digest (SHA256);
    digest.write (data, size);
    return digest.get ();
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha256
// =========================================================================
BytesArray Digest::sha256 (const BytesArray& data)
{
    return sha256 (reinterpret_cast <const char*> (data.data ()), data.size ());
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha256
// =========================================================================
BytesArray Digest::sha256 (const std::string& data)
{
    return sha256 (BytesArray (data.begin (), data.end ()));
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha384
// =========================================================================
BytesArray Digest::sha384 (const char* data, std::streamsize size)
{
    Digest digest (SHA384);
    digest.write (data, size);
    return digest.get ();
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha384
// =========================================================================
BytesArray Digest::sha384 (const BytesArray& data)
{
    return sha384 (reinterpret_cast <const char*> (data.data ()), data.size ());
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha384
// =========================================================================
BytesArray Digest::sha384 (const std::string& data)
{
    return sha384 (BytesArray (data.begin (), data.end ()));
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha512
// =========================================================================
BytesArray Digest::sha512 (const char* data, std::streamsize size)
{
    Digest digest (SHA512);
    digest.write (data, size);
    return digest.get ();
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha512
// =========================================================================
BytesArray Digest::sha512 (const BytesArray& data)
{
    return sha512 (reinterpret_cast <const char*> (data.data ()), data.size ());
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha512
// =========================================================================
BytesArray Digest::sha512 (const std::string& data)
{
    return sha512 (BytesArray (data.begin (), data.end ()));
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : bin2hex
// =========================================================================
std::string Digest::bin2hex (const BytesArray& digest)
{
    std::stringstream oss;
    for (size_t i = 0; i < digest.size (); ++i)
    {
        oss << std::hex << std::setw (2) << std::setfill ('0') << static_cast <uint32_t> (digest[i]);
    }
    return oss.str ();
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : algorithm
// =========================================================================
const char* Digest::algorithm (Algorithm algo)
{
   switch (algo)
   {
        OUT_ENUM (SHA1);
        OUT_ENUM (SHA224);
        OUT_ENUM (SHA256);
        OUT_ENUM (SHA384);
        OUT_ENUM (SHA512);
   }

   return "UNKNOWN";
}
