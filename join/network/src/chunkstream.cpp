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
#include <join/chunkstream.hpp>
#include <join/utils.hpp>

// C++.
#include <sstream>

using join::Chunkstreambuf;
using join::Chunkstream;

// =========================================================================
//   CLASS     : Chunkstreambuf
//   METHOD    : Chunkstreambuf
// =========================================================================
Chunkstreambuf::Chunkstreambuf (std::streambuf* streambuf, bool own)
: StreambufDecorator (streambuf, own),
  _buf (std::make_unique <char []> (2 * _chunksize))
{
}

// =========================================================================
//   CLASS     : Chunkstreambuf
//   METHOD    : Chunkstreambuf
// =========================================================================
Chunkstreambuf::Chunkstreambuf (std::streambuf* streambuf, std::streamsize chunksize, bool own)
: StreambufDecorator (streambuf, own),
  _chunksize (chunksize),
  _buf (std::make_unique <char []> (2 * _chunksize))
{
}

// =========================================================================
//   CLASS     : Chunkstreambuf
//   METHOD    : Chunkstreambuf
// =========================================================================
Chunkstreambuf::Chunkstreambuf (Chunkstreambuf&& other)
: StreambufDecorator (std::move (other)),
  _chunksize (other._chunksize),
  _buf (std::move (other._buf))
{
}

// =========================================================================
//   CLASS     : Chunkstreambuf
//   METHOD    : operator=
// =========================================================================
Chunkstreambuf& Chunkstreambuf::operator= (Chunkstreambuf&& other)
{
    StreambufDecorator::operator= (std::move (other));
    _chunksize = other._chunksize;
    _buf = std::move (other._buf);
    return *this;
}

// =========================================================================
//   CLASS     : Chunkstreambuf
//   METHOD    : ~Chunkstreambuf
// =========================================================================
Chunkstreambuf::~Chunkstreambuf ()
{
    if ((pptr () - pbase ()) && (_innerbuf != nullptr))
    {
        overflow ();
    }
}

// =========================================================================
//   CLASS     : Chunkstreambuf
//   METHOD    : underflow
// =========================================================================
Chunkstreambuf::int_type Chunkstreambuf::underflow ()
{
    if (eback () == nullptr)
    {
        setg (_buf.get (), _buf.get (), _buf.get ());
    }

    if (this->gptr () == this->egptr ())
    {
        std::string line;

        if (!join::getline (*_innerbuf, line))
        {
            return traits_type::eof ();
        }

        auto pos = line.find (";");
        if (pos != std::string::npos)
        {
            line.resize (pos);
        }

        std::stringstream ss;
        ss << std::hex << line;

        std::streamsize chunksize = 0;
        if (!(ss >> chunksize))
        {
            join::lastError = make_error_code (Errc::InvalidParam);
            return traits_type::eof ();
        }

        if (chunksize > _chunksize)
        {
            join::lastError = make_error_code (Errc::MessageTooLong);
            return traits_type::eof ();
        }

        std::streamsize sz = _innerbuf->sgetn (eback (), chunksize);
        if (sz != chunksize)
        {
            return traits_type::eof ();
        }

        setg (eback (), eback (), eback () + chunksize);

        if (!join::getline (*_innerbuf, line))
        {
            return traits_type::eof ();
        }

        if (!line.empty ())
        {
            join::lastError = make_error_code (Errc::InvalidParam);
            return traits_type::eof ();
        }

        if (!chunksize)
        {
            return traits_type::eof ();
        }
    }

    return traits_type::to_int_type (*gptr ());
}

// =========================================================================
//   CLASS     : Chunkstreambuf
//   METHOD    : overflow
// =========================================================================
Chunkstreambuf::int_type Chunkstreambuf::overflow (int_type c)
{
    if (pbase () == nullptr)
    {
        setp (_buf.get () + _chunksize, _buf.get () + (2 * _chunksize));
    }

    if ((pptr () == epptr ()) || (c == traits_type::eof ()))
    {
        std::streamsize pending = pptr () - pbase ();
        if (pending)
        {
            std::stringstream oss;
            oss << std::hex << pending << std::dec << "\r\n";
            int res  = (_innerbuf->sputn (oss.str ().c_str (), oss.str ().size ()) == std::streamsize (oss.str ().size ()));
            res     &= (_innerbuf->sputn (pbase (), pending) == pending);
            res     &= (_innerbuf->sputn ("\r\n", 2) == 2);
            if (!res)
            {
                return traits_type::eof ();
            }

            setp (pbase (), pbase () + _chunksize);
        }

        if (c == traits_type::eof ())
        {
            std::streamsize sz = _innerbuf->sputn ("0\r\n\r\n", 5);
            if (sz != 5)
            {
                return traits_type::eof ();
            }

            return traits_type::not_eof (c);
        }
    }

    return sputc (traits_type::to_char_type (c));
}

// =========================================================================
//   CLASS     : Chunkstreambuf
//   METHOD    : sync
// =========================================================================
Chunkstreambuf::int_type Chunkstreambuf::sync ()
{
    if (overflow () == traits_type::eof ())
    {
        return -1;
    }
    return _innerbuf->pubsync ();
}

// =========================================================================
//   CLASS     : Chunkstream
//   METHOD    : Chunkstream
// =========================================================================
Chunkstream::Chunkstream (std::iostream& stream)
: _chunkbuf (stream.rdbuf ())
{
    init (&_chunkbuf);
}

// =========================================================================
//   CLASS     : Chunkstream
//   METHOD    : Chunkstream
// =========================================================================
Chunkstream::Chunkstream (std::iostream& stream, std::streamsize chunksize)
: _chunkbuf (stream.rdbuf (), chunksize)
{
    init (&_chunkbuf);
}

// =========================================================================
//   CLASS     : Chunkstream
//   METHOD    : Chunkstream
// =========================================================================
Chunkstream::Chunkstream (Chunkstream&& other)
: std::iostream (std::move (other)),
  _chunkbuf (std::move (other._chunkbuf))
{
    set_rdbuf (&_chunkbuf);
}

// =========================================================================
//   CLASS     : Chunkstream
//   METHOD    : operator=
// =========================================================================
Chunkstream& Chunkstream::operator=(Chunkstream&& other)
{
    std::iostream::operator= (std::move (other));
    _chunkbuf = std::move (other._chunkbuf);
    return *this;
}
