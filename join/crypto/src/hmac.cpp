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
#include <join/hmac.hpp>
#include <join/error.hpp>

using join::Errc;
using join::DigestErrc;
using join::Digest;
using join::Hmacbuf;
using join::Hmac;
using join::BytesArray;

// =========================================================================
//   CLASS     : Hmacbuf
//   METHOD    : Hmacbuf
// =========================================================================
Hmacbuf::Hmacbuf (const std::string& algo, const std::string& key)
: _buf (std::make_unique <char []> (_bufsize)),
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
  _md (EVP_MAC_fetch (nullptr, algo.c_str (), nullptr)),
#else
  _md (EVP_get_digestbyname (algo.c_str ())),
#endif
  _key (key)
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
//   CLASS     : Hmacbuf
//   METHOD    : Hmacbuf
// =========================================================================
Hmacbuf::Hmacbuf (Hmacbuf&& other)
: std::streambuf (std::move (other)),
  _buf (std::move (other._buf)),
  _md (other._md),
  _ctx (std::move (other._ctx)),
  _key (std::move (other._key))
{
}

// =========================================================================
//   CLASS     : Hmacbuf
//   METHOD    : operator=
// =========================================================================
Hmacbuf& Hmacbuf::operator= (Hmacbuf&& other)
{
    std::streambuf::operator= (std::move (other));
    _buf = std::move (other._buf);
    _md = other._md;
    _ctx = std::move (other._ctx);
    _key = std::move (other._key);
    return *this;
}

// =========================================================================
//   CLASS     : Hmacbuf
//   METHOD    : finalize
// =========================================================================
BytesArray Hmacbuf::finalize ()
{
    BytesArray hmac;
    if (_buf && (this->overflow (traits_type::eof ()) != traits_type::eof ()))
    {
        hmac.resize (EVP_MD_size (_md));
    #if OPENSSL_VERSION_NUMBER >= 0x30000000L
        EVP_MAC_final (_ctx.get (), &hmac[0], nullptr, hmac.size ());
    #else
        HMAC_Final (_ctx.get (), &hmac[0], nullptr);
    #endif
    }
    _ctx.reset ();
    return hmac;
}

// =========================================================================
//   CLASS     : Hmacbuf
//   METHOD    : overflow
// =========================================================================
Hmacbuf::int_type Hmacbuf::overflow (int_type c)
{
    if (_ctx == nullptr)
    {
    #if OPENSSL_VERSION_NUMBER >= 0x30000000L
        _ctx.reset (EVP_MAC_CTX_new (_md));
    #else
        _ctx.reset (HMAC_CTX_new ());
    #endif
        if (_ctx == nullptr)
        {
            lastError = make_error_code (Errc::OutOfMemory);
            return traits_type::eof ();
        }
    #if OPENSSL_VERSION_NUMBER >= 0x30000000L
        EVP_MAC_init (_ctx.get (), _key.c_str (), _key.size (), nullptr);
    #else
        HMAC_Init_ex (_ctx.get (), _key.c_str (), _key.size (), _md, nullptr);
    #endif
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
    #if OPENSSL_VERSION_NUMBER >= 0x30000000L
        EVP_MAC_update (_ctx.get (), reinterpret_cast <uint8_t*> (this->pbase ()), pending);
    #else
        HMAC_Update (_ctx.get (), reinterpret_cast <uint8_t*> (this->pbase ()), pending);
    #endif
    }

    this->setp (this->pbase (), this->pbase () + _bufsize);

    if (c == traits_type::eof ())
    {
        return traits_type::not_eof (c);
    }

    return this->sputc (traits_type::to_char_type (c));
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : Hmac
// =========================================================================
Hmac::Hmac (Digest::Algorithm algo, const std::string& key)
: _hmacbuf (Digest::algorithm (algo), key)
{
    this->init (&_hmacbuf);
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : Hmac
// =========================================================================
Hmac::Hmac (Hmac&& other)
: std::ostream (std::move (other)),
  _hmacbuf (std::move (other._hmacbuf))
{
    this->set_rdbuf (&_hmacbuf);
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : Hmac
// =========================================================================
Hmac& Hmac::operator=(Hmac&& other)
{
    std::ostream::operator= (std::move (other));
    _hmacbuf = std::move (other._hmacbuf);
    return *this;
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : finalize
// =========================================================================
BytesArray Hmac::finalize ()
{
    BytesArray hmac = _hmacbuf.finalize ();
    if (hmac.empty ())
    {
        this->setstate (std::ios_base::failbit);
    }
    return hmac;
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : md5bin
// =========================================================================
BytesArray Hmac::md5bin (const char* message, std::streamsize size, const std::string& key)
{
    Hmac hmac (Digest::MD5, key);
    hmac.write (message, size);
    return hmac.finalize ();
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : md5bin
// =========================================================================
BytesArray Hmac::md5bin (const BytesArray& message, const std::string& key)
{
    return md5bin (reinterpret_cast <const char*> (message.data ()), message.size (), key);
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : md5bin
// =========================================================================
BytesArray Hmac::md5bin (const std::string& message, const std::string& key)
{
    return md5bin (BytesArray (message.begin (), message.end ()), key);
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : md5hex
// =========================================================================
std::string Hmac::md5hex (const char* data, std::streamsize size, const std::string& key)
{
    return bin2hex (md5bin (data, size, key));
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : md5hex
// =========================================================================
std::string Hmac::md5hex (const BytesArray& data, const std::string& key)
{
    return md5hex (reinterpret_cast <const char*> (data.data ()), data.size (), key);
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : md5hex
// =========================================================================
std::string Hmac::md5hex (const std::string& data, const std::string& key)
{
    return md5hex (BytesArray (data.begin (), data.end ()), key);
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : sha1bin
// =========================================================================
BytesArray Hmac::sha1bin (const char* message, std::streamsize size, const std::string& key)
{
    Hmac hmac (Digest::SHA1, key);
    hmac.write (message, size);
    return hmac.finalize ();
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : sha1bin
// =========================================================================
BytesArray Hmac::sha1bin (const BytesArray& message, const std::string& key)
{
    return sha1bin (reinterpret_cast <const char*> (message.data ()), message.size (), key);
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : sha1bin
// =========================================================================
BytesArray Hmac::sha1bin (const std::string& message, const std::string& key)
{
    return sha1bin (BytesArray (message.begin (), message.end ()), key);
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : sha1hex
// =========================================================================
std::string Hmac::sha1hex (const char* data, std::streamsize size, const std::string& key)
{
    return bin2hex (sha1bin (data, size, key));
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : sha1hex
// =========================================================================
std::string Hmac::sha1hex (const BytesArray& data, const std::string& key)
{
    return sha1hex (reinterpret_cast <const char*> (data.data ()), data.size (), key);
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : sha1hex
// =========================================================================
std::string Hmac::sha1hex (const std::string& data, const std::string& key)
{
    return sha1hex (BytesArray (data.begin (), data.end ()), key);
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : sha224bin
// =========================================================================
BytesArray Hmac::sha224bin (const char* message, std::streamsize size, const std::string& key)
{
    Hmac hmac (Digest::SHA224, key);
    hmac.write (message, size);
    return hmac.finalize ();
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : sha224bin
// =========================================================================
BytesArray Hmac::sha224bin (const BytesArray& message, const std::string& key)
{
    return sha224bin (reinterpret_cast <const char*> (message.data ()), message.size (), key);
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : sha224bin
// =========================================================================
BytesArray Hmac::sha224bin (const std::string& message, const std::string& key)
{
    return sha224bin (BytesArray (message.begin (), message.end ()), key);
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : sha224hex
// =========================================================================
std::string Hmac::sha224hex (const char* data, std::streamsize size, const std::string& key)
{
    return bin2hex (sha224bin (data, size, key));
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : sha224hex
// =========================================================================
std::string Hmac::sha224hex (const BytesArray& data, const std::string& key)
{
    return sha224hex (reinterpret_cast <const char*> (data.data ()), data.size (), key);
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : sha224hex
// =========================================================================
std::string Hmac::sha224hex (const std::string& data, const std::string& key)
{
    return sha224hex (BytesArray (data.begin (), data.end ()), key);
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : sha256bin
// =========================================================================
BytesArray Hmac::sha256bin (const char* message, std::streamsize size, const std::string& key)
{
    Hmac hmac (Digest::SHA256, key);
    hmac.write (message, size);
    return hmac.finalize ();
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : sha256bin
// =========================================================================
BytesArray Hmac::sha256bin (const BytesArray& message, const std::string& key)
{
    return sha256bin (reinterpret_cast <const char*> (message.data ()), message.size (), key);
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : sha256bin
// =========================================================================
BytesArray Hmac::sha256bin (const std::string& message, const std::string& key)
{
    return sha256bin (BytesArray (message.begin (), message.end ()), key);
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : sha256hex
// =========================================================================
std::string Hmac::sha256hex (const char* data, std::streamsize size, const std::string& key)
{
    return bin2hex (sha256bin (data, size, key));
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : sha256hex
// =========================================================================
std::string Hmac::sha256hex (const BytesArray& data, const std::string& key)
{
    return sha256hex (reinterpret_cast <const char*> (data.data ()), data.size (), key);
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : sha256hex
// =========================================================================
std::string Hmac::sha256hex (const std::string& data, const std::string& key)
{
    return sha256hex (BytesArray (data.begin (), data.end ()), key);
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : sha384bin
// =========================================================================
BytesArray Hmac::sha384bin (const char* message, std::streamsize size, const std::string& key)
{
    Hmac hmac (Digest::SHA384, key);
    hmac.write (message, size);
    return hmac.finalize ();
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : sha384bin
// =========================================================================
BytesArray Hmac::sha384bin (const BytesArray& message, const std::string& key)
{
    return sha384bin (reinterpret_cast <const char*> (message.data ()), message.size (), key);
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : sha384bin
// =========================================================================
BytesArray Hmac::sha384bin (const std::string& message, const std::string& key)
{
    return sha384bin (BytesArray (message.begin (), message.end ()), key);
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : sha384hex
// =========================================================================
std::string Hmac::sha384hex (const char* data, std::streamsize size, const std::string& key)
{
    return bin2hex (sha384bin (data, size, key));
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : sha384hex
// =========================================================================
std::string Hmac::sha384hex (const BytesArray& data, const std::string& key)
{
    return sha384hex (reinterpret_cast <const char*> (data.data ()), data.size (), key);
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : sha384hex
// =========================================================================
std::string Hmac::sha384hex (const std::string& data, const std::string& key)
{
    return sha384hex (BytesArray (data.begin (), data.end ()), key);
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : sha512bin
// =========================================================================
BytesArray Hmac::sha512bin (const char* message, std::streamsize size, const std::string& key)
{
    Hmac hmac (Digest::SHA512, key);
    hmac.write (message, size);
    return hmac.finalize ();
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : sha512bin
// =========================================================================
BytesArray Hmac::sha512bin (const BytesArray& message, const std::string& key)
{
    return sha512bin (reinterpret_cast <const char*> (message.data ()), message.size (), key);
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : sha512bin
// =========================================================================
BytesArray Hmac::sha512bin (const std::string& message, const std::string& key)
{
    return sha512bin (BytesArray (message.begin (), message.end ()), key);
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : sha512hex
// =========================================================================
std::string Hmac::sha512hex (const char* data, std::streamsize size, const std::string& key)
{
    return bin2hex (sha512bin (data, size, key));
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : sha512hex
// =========================================================================
std::string Hmac::sha512hex (const BytesArray& data, const std::string& key)
{
    return sha512hex (reinterpret_cast <const char*> (data.data ()), data.size (), key);
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : sha512hex
// =========================================================================
std::string Hmac::sha512hex (const std::string& data, const std::string& key)
{
    return sha512hex (BytesArray (data.begin (), data.end ()), key);
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : sm3bin
// =========================================================================
BytesArray Hmac::sm3bin (const char* message, std::streamsize size, const std::string& key)
{
    Hmac hmac (Digest::SM3, key);
    hmac.write (message, size);
    return hmac.finalize ();
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : sm3bin
// =========================================================================
BytesArray Hmac::sm3bin (const BytesArray& message, const std::string& key)
{
    return sm3bin (reinterpret_cast <const char*> (message.data ()), message.size (), key);
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : sm3bin
// =========================================================================
BytesArray Hmac::sm3bin (const std::string& message, const std::string& key)
{
    return sm3bin (BytesArray (message.begin (), message.end ()), key);
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : sm3hex
// =========================================================================
std::string Hmac::sm3hex (const char* data, std::streamsize size, const std::string& key)
{
    return bin2hex (sm3bin (data, size, key));
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : sm3hex
// =========================================================================
std::string Hmac::sm3hex (const BytesArray& data, const std::string& key)
{
    return sm3hex (reinterpret_cast <const char*> (data.data ()), data.size (), key);
}

// =========================================================================
//   CLASS     : Hmac
//   METHOD    : sm3hex
// =========================================================================
std::string Hmac::sm3hex (const std::string& data, const std::string& key)
{
    return sm3hex (BytesArray (data.begin (), data.end ()), key);
}
