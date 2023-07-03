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
#include <join/error.hpp>
#include <join/utils.hpp>
#include <join/base64.hpp>

using join::Base64;
using join::BytesArray;
using join::Encoderbuf;
using join::Encoder;
using join::Decoderbuf;
using join::Decoder;

// =========================================================================
//   CLASS     : Encoderbuf
//   METHOD    : Encoderbuf
// =========================================================================
Encoderbuf::Encoderbuf ()
: _buf (std::make_unique <char []> (_bufsize))
{
    if (_buf == nullptr)
    {
        throw std::system_error (make_error_code (Errc::OutOfMemory));
    }
}

// =========================================================================
//   CLASS     : Encoderbuf
//   METHOD    : Encoderbuf
// =========================================================================
Encoderbuf::Encoderbuf (Encoderbuf&& other)
: std::streambuf (std::move (other)),
  _buf (std::move (other._buf)),
  _encodectx (std::move (other._encodectx)),
  _out (std::move (other._out))
{
}

// =========================================================================
//   CLASS     : Encoderbuf
//   METHOD    : operator=
// =========================================================================
Encoderbuf& Encoderbuf::operator= (Encoderbuf&& other)
{
    std::streambuf::operator= (std::move (other));
    _buf = std::move (other._buf);
    _encodectx = std::move (other._encodectx);
    _out = std::move (other._out);
    return *this;
}

// =========================================================================
//   CLASS     : Encoderbuf
//   METHOD    : get
// =========================================================================
std::string Encoderbuf::get ()
{
    if (_buf && (this->overflow (traits_type::eof ()) == traits_type::eof ()))
    {
        _encodectx.reset ();
        return {};
    }

    int oldsz = _out.size ();
    int reqsz = oldsz + 66;

    _out.resize (reqsz);
    EVP_EncodeFinal (_encodectx.get (), reinterpret_cast <uint8_t *> (&_out[oldsz]), &reqsz);
    _out.resize (oldsz + reqsz);

    _encodectx.reset ();

    return join::replaceAll (_out, "\n", "");
}

// =========================================================================
//   CLASS     : Encoderbuf
//   METHOD    : overflow
// =========================================================================
Encoderbuf::int_type Encoderbuf::overflow (int_type c)
{
    if (_encodectx == nullptr)
    {
        _encodectx.reset (EVP_ENCODE_CTX_new ());
        if (_encodectx == nullptr)
        {
            lastError = make_error_code (Errc::OutOfMemory);
            return traits_type::eof ();
        }

        _out.clear ();
        _out.shrink_to_fit ();

        EVP_EncodeInit (_encodectx.get ());
    }

    if (this->pbase () == nullptr)
    {
        this->setp (_buf.get (), _buf.get () + _bufsize);
    }

    if ((this->pptr () < this->epptr ()) && (c != traits_type::eof ()))
    {
        return this->sputc (traits_type::to_char_type (c));
    }

    int pending = this->pptr () - this->pbase ();
    if (pending)
    {
        int oldsz = _out.size ();
        int reqsz = oldsz + ((((EVP_ENCODE_CTX_num (_encodectx.get ()) + pending) / 48) * 65) + 1);

        _out.resize (reqsz);
        EVP_EncodeUpdate (_encodectx.get (), reinterpret_cast <uint8_t *> (&_out[oldsz]), &reqsz, reinterpret_cast <uint8_t *> (this->pbase ()), pending);
        _out.resize (oldsz + reqsz);
    }

    this->setp (this->pbase (), this->pbase () + _bufsize);

    if (c == traits_type::eof ())
    {
        return traits_type::not_eof (c);
    }

    return this->sputc (traits_type::to_char_type (c));
}

// =========================================================================
//   CLASS     : Encoder
//   METHOD    : Encoder
// =========================================================================
Encoder::Encoder ()
{
    this->init (&_encoderbuf);
}

// =========================================================================
//   CLASS     : Encoder
//   METHOD    : Encoder
// =========================================================================
Encoder::Encoder (Encoder&& other)
: std::ostream (std::move (other)),
  _encoderbuf (std::move (other._encoderbuf))
{
    this->set_rdbuf (&_encoderbuf);
}

// =========================================================================
//   CLASS     : Encoder
//   METHOD    : operator=
// =========================================================================
Encoder& Encoder::operator=(Encoder&& other)
{
    std::ostream::operator= (std::move (other));
    _encoderbuf = std::move (other._encoderbuf);
    return *this;
}

// =========================================================================
//   CLASS     : Encoder
//   METHOD    : get
// =========================================================================
std::string Encoder::get ()
{
    std::string encoded = _encoderbuf.get ();
    if (encoded.empty ())
    {
        this->setstate (std::ios_base::failbit);
    }
    return encoded;
}

// =========================================================================
//   CLASS     : Decoderbuf
//   METHOD    : Decoderbuf
// =========================================================================
Decoderbuf::Decoderbuf ()
: _buf (std::make_unique <char []> (_bufsize))
{
    if (_buf == nullptr)
    {
        throw std::system_error (make_error_code (Errc::OutOfMemory));
    }
}

// =========================================================================
//   CLASS     : Decoderbuf
//   METHOD    : Decoderbuf
// =========================================================================
Decoderbuf::Decoderbuf (Decoderbuf&& other)
: std::streambuf (std::move (other)),
  _buf (std::move (other._buf)),
  _decodectx (std::move (other._decodectx)),
  _out (std::move (other._out))
{
}

// =========================================================================
//   CLASS     : Encoderbuf
//   METHOD    : operator=
// =========================================================================
Decoderbuf& Decoderbuf::operator= (Decoderbuf&& other)
{
    std::streambuf::operator= (std::move (other));
    _buf = std::move (other._buf);
    _decodectx = std::move (other._decodectx);
    _out = std::move (other._out);
    return *this;
}

// =========================================================================
//   CLASS     : Decoderbuf
//   METHOD    : get
// =========================================================================
BytesArray Decoderbuf::get ()
{
    if (_buf && (this->overflow (traits_type::eof ()) == traits_type::eof ()))
    {
        _decodectx.reset ();
        return {};
    }

    int oldsz = _out.size ();
    int reqsz = oldsz + 66;

    _out.resize (reqsz);
    EVP_DecodeFinal (_decodectx.get (), reinterpret_cast <uint8_t *> (&_out[oldsz]), &reqsz);
    _out.resize (oldsz + reqsz);

    _decodectx.reset ();

    return _out;
}

// =========================================================================
//   CLASS     : Decoderbuf
//   METHOD    : overflow
// =========================================================================
Decoderbuf::int_type Decoderbuf::overflow (int_type c)
{
    if (_decodectx == nullptr)
    {
        _decodectx.reset (EVP_ENCODE_CTX_new ());
        if (_decodectx == nullptr)
        {
            lastError = make_error_code (Errc::OutOfMemory);
            return traits_type::eof ();
        }

        _out.clear ();
        _out.shrink_to_fit ();

        EVP_DecodeInit (_decodectx.get ());
    }

    if (this->pbase () == nullptr)
    {
        this->setp (_buf.get (), _buf.get () + _bufsize);
    }

    if ((this->pptr () < this->epptr ()) && (c != traits_type::eof ()))
    {
        return this->sputc (traits_type::to_char_type (c));
    }

    int pending = this->pptr () - this->pbase ();
    if (pending)
    {
        int oldsz = _out.size ();
        int reqsz = oldsz + pending;

        _out.resize (reqsz);
        EVP_DecodeUpdate (_decodectx.get (), &_out[oldsz], &reqsz, reinterpret_cast <uint8_t *> (this->pbase ()), pending);
        _out.resize (oldsz + reqsz);
    }

    this->setp (this->pbase (), this->pbase () + _bufsize);

    if (c == traits_type::eof ())
    {
        return traits_type::not_eof (c);
    }

    return this->sputc (traits_type::to_char_type (c));
}

// =========================================================================
//   CLASS     : Decoder
//   METHOD    : Decoder
// =========================================================================
Decoder::Decoder ()
{
    this->init (&_decoderbuf);
}

// =========================================================================
//   CLASS     : Decoder
//   METHOD    : Decoder
// =========================================================================
Decoder::Decoder (Decoder&& other)
: std::ostream (std::move (other)),
  _decoderbuf (std::move (other._decoderbuf))
{
    this->set_rdbuf (&_decoderbuf);
}

// =========================================================================
//   CLASS     : Decoder
//   METHOD    : operator=
// =========================================================================
Decoder& Decoder::operator=(Decoder&& other)
{
    std::ostream::operator= (std::move (other));
    _decoderbuf = std::move (other._decoderbuf);
    return *this;
}

// =========================================================================
//   CLASS     : Decoder
//   METHOD    : get
// =========================================================================
BytesArray Decoder::get ()
{
    BytesArray decoded = _decoderbuf.get ();
    if (decoded.empty ())
    {
        this->setstate (std::ios_base::failbit);
    }
    return decoded;
}

// =========================================================================
//   CLASS     : Base64
//   METHOD    : encode
// =========================================================================
std::string Base64::encode (const char* data, size_t size)
{
    Encoder encoder;
    encoder.write (data, size);
    return encoder.get ();
}

// =========================================================================
//   CLASS     : Base64
//   METHOD    : encode
// =========================================================================
std::string Base64::encode (const std::string& data)
{
    return encode (data.data (), data.size ());
}

// =========================================================================
//   CLASS     : Base64
//   METHOD    : encode
// =========================================================================
std::string Base64::encode (const BytesArray& data)
{
    return encode (reinterpret_cast <const char*> (data.data ()), data.size ());
}

// =========================================================================
//   CLASS     : Base64
//   METHOD    : decode
// =========================================================================
BytesArray Base64::decode (const std::string& data)
{
    Decoder decoder;
    decoder.write (data.data (), data.size ());
    return decoder.get ();
}
