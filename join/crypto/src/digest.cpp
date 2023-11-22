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
using join::Digestbuf;
using join::Digest;
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
  _md (other._md),
  _ctx (std::move (other._ctx))
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
    _md = other._md;
    _ctx = std::move (other._ctx);
    return *this;
}

// =========================================================================
//   CLASS     : Digestbuf
//   METHOD    : finalize
// =========================================================================
BytesArray Digestbuf::finalize ()
{
    BytesArray digest;

    if (_buf && (this->overflow (traits_type::eof ()) != traits_type::eof ()))
    {
        digest.resize (EVP_MD_size (_md));
        EVP_DigestFinal_ex (_ctx.get (), &digest[0], nullptr);
    }

    _ctx.reset ();

    return digest;
}

// =========================================================================
//   CLASS     : Digestbuf
//   METHOD    : overflow
// =========================================================================
Digestbuf::int_type Digestbuf::overflow (int_type c)
{
    if (_ctx == nullptr)
    {
        _ctx.reset (EVP_MD_CTX_new ());
        if (_ctx == nullptr)
        {
            lastError = make_error_code (Errc::OutOfMemory);
            return traits_type::eof ();
        }

        EVP_DigestInit_ex (_ctx.get (), _md, nullptr);
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
    if (pending)
    {
        EVP_DigestUpdate (_ctx.get (), this->pbase (), pending);
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
//   METHOD    : finalize
// =========================================================================
BytesArray Digest::finalize ()
{
    BytesArray digest = _digestbuf.finalize ();
    if (digest.empty ())
    {
        this->setstate (std::ios_base::failbit);
    }
    return digest;
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : md5bin
// =========================================================================
BytesArray Digest::md5bin (const char* data, std::streamsize size)
{
    Digest digest (MD5);
    digest.write (data, size);
    return digest.finalize ();
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : md5bin
// =========================================================================
BytesArray Digest::md5bin (const BytesArray& data)
{
    return md5bin (reinterpret_cast <const char*> (data.data ()), data.size ());
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : md5bin
// =========================================================================
BytesArray Digest::md5bin (const std::string& data)
{
    return md5bin (BytesArray (data.begin (), data.end ()));
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : md5hex
// =========================================================================
std::string Digest::md5hex (const char* data, std::streamsize size)
{
    return bin2hex (md5bin (data, size));
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : md5hex
// =========================================================================
std::string Digest::md5hex (const BytesArray& data)
{
    return md5hex (reinterpret_cast <const char*> (data.data ()), data.size ());
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : md5hex
// =========================================================================
std::string Digest::md5hex (const std::string& data)
{
    return md5hex (BytesArray (data.begin (), data.end ()));
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha1bin
// =========================================================================
BytesArray Digest::sha1bin (const char* data, std::streamsize size)
{
    Digest digest (SHA1);
    digest.write (data, size);
    return digest.finalize ();
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha1bin
// =========================================================================
BytesArray Digest::sha1bin (const BytesArray& data)
{
    return sha1bin (reinterpret_cast <const char*> (data.data ()), data.size ());
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha1bin
// =========================================================================
BytesArray Digest::sha1bin (const std::string& data)
{
    return sha1bin (BytesArray (data.begin (), data.end ()));
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha1hex
// =========================================================================
std::string Digest::sha1hex (const char* data, std::streamsize size)
{
    return bin2hex (sha1bin (data, size));
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha1hex
// =========================================================================
std::string Digest::sha1hex (const BytesArray& data)
{
    return sha1hex (reinterpret_cast <const char*> (data.data ()), data.size ());
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha1hex
// =========================================================================
std::string Digest::sha1hex (const std::string& data)
{
    return sha1hex (BytesArray (data.begin (), data.end ()));
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha224bin
// =========================================================================
BytesArray Digest::sha224bin (const char* data, std::streamsize size)
{
    Digest digest (SHA224);
    digest.write (data, size);
    return digest.finalize ();
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha224bin
// =========================================================================
BytesArray Digest::sha224bin (const BytesArray& data)
{
    return sha224bin (reinterpret_cast <const char*> (data.data ()), data.size ());
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha224bin
// =========================================================================
BytesArray Digest::sha224bin (const std::string& data)
{
    return sha224bin (BytesArray (data.begin (), data.end ()));
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha224hex
// =========================================================================
std::string Digest::sha224hex (const char* data, std::streamsize size)
{
    return bin2hex (sha224bin (data, size));
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha224hex
// =========================================================================
std::string Digest::sha224hex (const BytesArray& data)
{
    return sha224hex (reinterpret_cast <const char*> (data.data ()), data.size ());
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha224hex
// =========================================================================
std::string Digest::sha224hex (const std::string& data)
{
    return sha224hex (BytesArray (data.begin (), data.end ()));
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha256bin
// =========================================================================
BytesArray Digest::sha256bin (const char* data, std::streamsize size)
{
    Digest digest (SHA256);
    digest.write (data, size);
    return digest.finalize ();
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha256bin
// =========================================================================
BytesArray Digest::sha256bin (const BytesArray& data)
{
    return sha256bin (reinterpret_cast <const char*> (data.data ()), data.size ());
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha256bin
// =========================================================================
BytesArray Digest::sha256bin (const std::string& data)
{
    return sha256bin (BytesArray (data.begin (), data.end ()));
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha256hex
// =========================================================================
std::string Digest::sha256hex (const char* data, std::streamsize size)
{
    return bin2hex (sha256bin (data, size));
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha256hex
// =========================================================================
std::string Digest::sha256hex (const BytesArray& data)
{
    return sha256hex (reinterpret_cast <const char*> (data.data ()), data.size ());
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha256hex
// =========================================================================
std::string Digest::sha256hex (const std::string& data)
{
    return sha256hex (BytesArray (data.begin (), data.end ()));
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha384bin
// =========================================================================
BytesArray Digest::sha384bin (const char* data, std::streamsize size)
{
    Digest digest (SHA384);
    digest.write (data, size);
    return digest.finalize ();
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha384bin
// =========================================================================
BytesArray Digest::sha384bin (const BytesArray& data)
{
    return sha384bin (reinterpret_cast <const char*> (data.data ()), data.size ());
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha384bin
// =========================================================================
BytesArray Digest::sha384bin (const std::string& data)
{
    return sha384bin (BytesArray (data.begin (), data.end ()));
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha384hex
// =========================================================================
std::string Digest::sha384hex (const char* data, std::streamsize size)
{
    return bin2hex (sha384bin (data, size));
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha384hex
// =========================================================================
std::string Digest::sha384hex (const BytesArray& data)
{
    return sha384hex (reinterpret_cast <const char*> (data.data ()), data.size ());
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha384hex
// =========================================================================
std::string Digest::sha384hex (const std::string& data)
{
    return sha384hex (BytesArray (data.begin (), data.end ()));
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha512bin
// =========================================================================
BytesArray Digest::sha512bin (const char* data, std::streamsize size)
{
    Digest digest (SHA512);
    digest.write (data, size);
    return digest.finalize ();
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha512bin
// =========================================================================
BytesArray Digest::sha512bin (const BytesArray& data)
{
    return sha512bin (reinterpret_cast <const char*> (data.data ()), data.size ());
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha512bin
// =========================================================================
BytesArray Digest::sha512bin (const std::string& data)
{
    return sha512bin (BytesArray (data.begin (), data.end ()));
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha512hex
// =========================================================================
std::string Digest::sha512hex (const char* data, std::streamsize size)
{
    return bin2hex (sha512bin (data, size));
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha512hex
// =========================================================================
std::string Digest::sha512hex (const BytesArray& data)
{
    return sha512hex (reinterpret_cast <const char*> (data.data ()), data.size ());
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sha512hex
// =========================================================================
std::string Digest::sha512hex (const std::string& data)
{
    return sha512hex (BytesArray (data.begin (), data.end ()));
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sm3bin
// =========================================================================
BytesArray Digest::sm3bin (const char* data, std::streamsize size)
{
    Digest digest (SM3);
    digest.write (data, size);
    return digest.finalize ();
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sm3bin
// =========================================================================
BytesArray Digest::sm3bin (const BytesArray& data)
{
    return sm3bin (reinterpret_cast <const char*> (data.data ()), data.size ());
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sm3bin
// =========================================================================
BytesArray Digest::sm3bin (const std::string& data)
{
    return sm3bin (BytesArray (data.begin (), data.end ()));
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sm3hex
// =========================================================================
std::string Digest::sm3hex (const char* data, std::streamsize size)
{
    return bin2hex (sm3bin (data, size));
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sm3hex
// =========================================================================
std::string Digest::sm3hex (const BytesArray& data)
{
    return sm3hex (reinterpret_cast <const char*> (data.data ()), data.size ());
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : sm3hex
// =========================================================================
std::string Digest::sm3hex (const std::string& data)
{
    return sm3hex (BytesArray (data.begin (), data.end ()));
}

// =========================================================================
//   CLASS     : Digest
//   METHOD    : algorithm
// =========================================================================
const char* Digest::algorithm (Algorithm algo)
{
   switch (algo)
   {
        OUT_ENUM (MD5);
        OUT_ENUM (SHA1);
        OUT_ENUM (SHA224);
        OUT_ENUM (SHA256);
        OUT_ENUM (SHA384);
        OUT_ENUM (SHA512);
        OUT_ENUM (SM3);
   }

   return "UNKNOWN";
}
