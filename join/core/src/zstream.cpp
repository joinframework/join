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
#include <join/zstream.hpp>
#include <join/error.hpp>

using join::Zstreambuf;
using join::Zstream;

// =========================================================================
//   CLASS     : Zstreambuf
//   METHOD    : Zstreambuf
// =========================================================================
Zstreambuf::Zstreambuf (std::streambuf* streambuf, int format, bool own)
: StreambufDecorator (streambuf, own),
  _inflate (std::make_unique <z_stream> ()),
  _deflate (std::make_unique <z_stream> ()),
  _buf (std::make_unique <char []> (4 * _bufsize))
{
    inflateInit2 (_inflate.get (), format);
    deflateInit2 (_deflate.get (), Z_DEFAULT_COMPRESSION, Z_DEFLATED, format, 8, Z_DEFAULT_STRATEGY);
}

// =========================================================================
//   CLASS     : Zstreambuf
//   METHOD    : Zstreambuf
// =========================================================================
Zstreambuf::Zstreambuf (Zstreambuf&& other)
: StreambufDecorator (std::move (other)),
  _inflate (std::move (other._inflate)),
  _deflate (std::move (other._deflate)),
  _buf (std::move (other._buf))
{
}

// =========================================================================
//   CLASS     : Zstreambuf
//   METHOD    : operator=
// =========================================================================
Zstreambuf& Zstreambuf::operator= (Zstreambuf&& other)
{
    StreambufDecorator::operator= (std::move (other));
    _inflate = std::move (other._inflate);
    _deflate = std::move (other._deflate);
    _buf = std::move (other._buf);
    return *this;
}

// =========================================================================
//   CLASS     : Zstreambuf
//   METHOD    : ~Zstreambuf
// =========================================================================
Zstreambuf::~Zstreambuf ()
{
    if (_inflate)
    {
        inflateEnd (_inflate.get ());
    }
    if (_deflate)
    {
        if (_innerbuf)
        {
            overflow ();
        }
        deflateEnd (_deflate.get ());
    }
}

// =========================================================================
//   CLASS     : Zstreambuf
//   METHOD    : underflow
// =========================================================================
Zstreambuf::int_type Zstreambuf::underflow ()
{
    if (eback () == nullptr)
    {
        setg (_buf.get (), _buf.get (), _buf.get ());
    }

    if (this->gptr () == this->egptr ())
    {
        if (_inflate->avail_in == 0)
        {
            if (_instate == Z_STREAM_END)
            {
                _instate = inflateReset (_inflate.get ());
                return traits_type::eof ();
            }

            _inflate->next_in = reinterpret_cast <uint8_t*> (eback () + _bufsize);
            _inflate->avail_in = _innerbuf->sgetn (eback () + _bufsize, _bufsize);
            if (_inflate->avail_in == 0)
            {
                return traits_type::eof ();
            }
        }

        _inflate->next_out = reinterpret_cast <uint8_t*> (eback ());
        _inflate->avail_out = _bufsize;
        _instate = ::inflate (_inflate.get (), Z_NO_FLUSH);
        if ((_instate != Z_OK) && (_instate != Z_STREAM_END))
        {
            join::lastError = make_error_code (Errc::OperationFailed);
            return traits_type::eof ();
        }

        setg (eback (), eback (), eback () + (_bufsize - _inflate->avail_out));
    }

    return traits_type::to_int_type (*gptr ());
}

// =========================================================================
//   CLASS     : Zstreambuf
//   METHOD    : overflow
// =========================================================================
Zstreambuf::int_type Zstreambuf::overflow (int_type c)
{
    if (pbase () == nullptr)
    {
        setp (_buf.get () + (2 * _bufsize), _buf.get () + (3 * _bufsize));
    }

    if ((pptr () == epptr ()) || (c == traits_type::eof ()))
    {
        _deflate->next_in = reinterpret_cast <uint8_t*> (pbase ());
        _deflate->avail_in = pptr () - pbase ();

        while (_deflate->avail_in)
        {
            _deflate->next_out = reinterpret_cast <uint8_t*> (pbase () + _bufsize);
            _deflate->avail_out = _bufsize;

            int res = ::deflate (_deflate.get (), (c == traits_type::eof ()) ? Z_FINISH : Z_NO_FLUSH);
            if ((res != Z_OK) && (res != Z_STREAM_END))
            {
                join::lastError = make_error_code (Errc::OperationFailed);
                return traits_type::eof ();
            }

            std::streamsize deflated = _bufsize - _deflate->avail_out;
            std::streamsize nwrite = _innerbuf->sputn (pbase () + _bufsize, deflated);
            if (nwrite != deflated)
            {
                return traits_type::eof ();
            }
        }

        setp (pbase (), pbase () + _bufsize);

        if (c == traits_type::eof ())
        {
            return traits_type::not_eof (c);
        }
    }

    return sputc (traits_type::to_char_type (c));
}

// =========================================================================
//   CLASS     : Zstreambuf
//   METHOD    : sync
// =========================================================================
Zstreambuf::int_type Zstreambuf::sync ()
{
    if (overflow () == traits_type::eof ())
    {
        return -1;
    }
    deflateReset (_deflate.get ());
    return _innerbuf->pubsync ();
}

// =========================================================================
//   CLASS     : Zstream
//   METHOD    : Zstream
// =========================================================================
Zstream::Zstream (std::iostream& stream, Format format)
: _zbuf (stream.rdbuf (), format)
{
    init (&_zbuf);
}

// =========================================================================
//   CLASS     : Zstream
//   METHOD    : Zstream
// =========================================================================
Zstream::Zstream (Zstream&& other)
: std::iostream (std::move (other)),
  _zbuf (std::move (other._zbuf))
{
    set_rdbuf (&_zbuf);
}

// =========================================================================
//   CLASS     : Zstream
//   METHOD    : operator=
// =========================================================================
Zstream& Zstream::operator=(Zstream&& other)
{
    std::iostream::operator= (std::move (other));
    _zbuf = std::move (other._zbuf);
    return *this;
}
