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
Chunkstreambuf::Chunkstreambuf (std::istream& istream, std::ostream& ostream, std::streamsize chunksize)
: StreambufDecorator (istream, ostream, 2 * chunksize),
  _chunksize (chunksize)
{
}

// =========================================================================
//   CLASS     : Chunkstreambuf
//   METHOD    : Chunkstreambuf
// =========================================================================
Chunkstreambuf::Chunkstreambuf (Chunkstreambuf&& other)
: StreambufDecorator (std::move (other)),
  _chunksize (other._chunksize)
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
    return *this;
}

// =========================================================================
//   CLASS     : Chunkstreambuf
//   METHOD    : ~Chunkstreambuf
// =========================================================================
Chunkstreambuf::~Chunkstreambuf ()
{
    if (_ostream)
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

        if (!join::getline (*_istream, line))
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

        std::streamsize nread = 0;

        while (nread < chunksize)
        {
            _istream->read (eback () + nread, chunksize - nread);
            if (_istream->fail ())
            {
                return traits_type::eof ();
            }
            nread += _istream->gcount ();
        }

        if (!join::getline (*_istream, line))
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

        setg (eback (), eback (), eback () + chunksize);
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
            *_ostream << std::hex << pending << std::dec << "\r\n";
            _ostream->write (pbase (), pending);
            *_ostream << "\r\n";

            if (_ostream->fail ())
            {
                return traits_type::eof ();
            }
        }

        if (c == traits_type::eof ())
        {
            *_ostream << std::hex << std::streamsize (0) << std::dec << "\r\n";
            *_ostream << "\r\n";

            if (_ostream->fail ())
            {
                return traits_type::eof ();
            }

            return traits_type::not_eof (c);
        }

        setp (pbase (), pbase () + _chunksize);
    }

    return sputc (traits_type::to_char_type (c));
}

// =========================================================================
//   CLASS     : Chunkstreambuf
//   METHOD    : sync
// =========================================================================
Chunkstreambuf::int_type Chunkstreambuf::sync ()
{
    return overflow ();
}

// =========================================================================
//   CLASS     : Chunkstream
//   METHOD    : Chunkstream
// =========================================================================
Chunkstream::Chunkstream (std::iostream& stream, std::streamsize chunksize)
: Chunkstream (stream, stream, chunksize)
{
}

// =========================================================================
//   CLASS     : Chunkstream
//   METHOD    : Chunkstream
// =========================================================================
Chunkstream::Chunkstream (std::istream& istream, std::ostream& ostream, std::streamsize chunksize)
: _chunkbuf (istream, ostream, chunksize)
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
