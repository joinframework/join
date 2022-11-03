/**
 * MIT License
 *
 * Copyright (c) 2021 Mathieu Rabine
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

#ifndef __JOIN_VIEW_HPP__
#define __JOIN_VIEW_HPP__

// libjoin.
#include <join/httpclient.hpp>

// C++.
#include <stdexcept>
#include <istream>

// C.
#include <cstring>
#include <cstddef>

namespace join
{
    /**
     * @brief string view.
     */
    class StringView
    {
    public:
        /**
         * @brief default constructor.
         * @param in input string.
         * @param count number of characters.
         */
        StringView (const char* in, size_t count)
        : _buf (in),
          _len (count)
        {
        }

        /**
         * @brief default constructor.
         * @param in input string.
         */
        StringView (const char * in)
        : _buf (in),
          _len (strlen (in))
        {
        }

        /**
         * @brief default constructor.
         * @param first pointer to the first character of the string.
         * @param last pointer to the last character of the string.
         */
        StringView (const char * first, const char * last)
        : _buf (first),
          _len (last - first)
        {
        }

        /**
         * @brief copy constructor.
         * @param other object to copy.
         */
        StringView (const StringView& other) = delete;

        /**
         * @brief copy assignment.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        StringView& operator= (const StringView& other) = delete;

        /**
         * @brief move constructor.
         * @param other object to move.
         */
        StringView (StringView&& other) = delete;

        /**
         * @brief move assignment.
         * @param other object to move.
         * @return a reference of the current object.
         */
        StringView& operator=(StringView&& other) = delete;

        /**
         * @brief destroy instance.
         */
        virtual ~StringView () = default;

        /**
         * @brief get character without extracting it.
         * @return extracted character.
         */
        int peek () noexcept
        {
            if (_len)
            {
                return _buf[_pos];
            }
            return std::char_traits <char>::eof ();
        }

        /**
         * @brief extracts character.
         * @return extracted character.
         */
        int get () noexcept
        {
            if (_len)
            {
                --_len;
                return _buf[_pos++];
            }
            return std::char_traits <char>::eof ();
        }

        /**
         * @brief extracts expected character.
         * @param expected expected character.
         * @return true if extracted, false otherwise.
         */
        bool getIf (char expected) noexcept
        {
            if (_len && (_buf[_pos] == expected))
            {
                --_len;
                ++_pos;
                return true;
            }
            return false;
        }

        /**
         * @brief extracts expected character (case insensitive).
         * @param expected expected character.
         * @return true if extracted, false otherwise.
         */
        bool getIfNoCase (char expected) noexcept
        {
            if (_len && (((_buf[_pos] ^ expected) == 0) ||
                         ((_buf[_pos] ^ expected) == 32)))
            {
                --_len;
                ++_pos;
                return true;
            }
            return false;
        }

        /**
         * @brief .
         * @param buf .
         * @param count .
         */
        size_t read (char* buf, size_t count)
        {
            count = std::min (_len, count);
            ::memcpy (buf, &_buf[_pos], count);
            _pos += count;
            _len -= count;
            return count;
        }

    protected:
        /// input buffer start pointer.
        const char * _buf = nullptr;

        /// string remaining size.
        size_t _len = 0;

        /// current position.
        size_t _pos = 0;
    };

    /**
     * @brief stream view.
     */
    class StreamView
    {
    public:
        /**
         * @brief default constructor.
         * @param in input stream.
         */
        StreamView (std::istream& in)
        : _in (std::addressof (in))
        {
        }

        /**
         * @brief copy constructor.
         * @param other object to copy.
         */
        StreamView (const StreamView& other) = delete;

        /**
         * @brief copy assignment.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        StreamView& operator= (const StreamView& other) = delete;

        /**
         * @brief move constructor.
         * @param other object to move.
         */
        StreamView (StreamView&& other) = delete;

        /**
         * @brief move assignment.
         * @param other object to move.
         * @return a reference of the current object.
         */
        StreamView& operator=(StreamView&& other) = delete;

        /**
         * @brief destroy instance.
         */
        virtual ~StreamView () = default;

        /**
         * @brief get character without extracting it.
         * @return extracted character.
         */
        virtual int peek () noexcept
        {
            return _in->peek ();
        }

        /**
         * @brief extracts character.
         * @return extracted character.
         */
        virtual int get () noexcept
        {
            return _in->get ();
        }

        /**
         * @brief extracts expected character.
         * @param expected expected character.
         * @return true if extracted, false otherwise.
         */
        bool getIf (char expected) noexcept
        {
            if (!_in->eof () && (static_cast <char> (peek ()) == expected))
            {
                get ();
                return true;
            }
            return false;
        }

        /**
         * @brief extracts expected character (case insensitive).
         * @param expected expected character.
         * @return true if extracted, false otherwise.
         */
        bool getIfNoCase (char expected) noexcept
        {
            if (!_in->eof () && (((static_cast <char> (peek ()) ^ expected) == 0) ||
                                 ((static_cast <char> (peek ()) ^ expected) == 32)))
            {
                get ();
                return true;
            }
            return false;
        }

        /**
         * @brief .
         * @param buf .
         * @param count .
         */
        virtual size_t read (char* buf, size_t count)
        {
            _in->read (buf, count);
            return _in->gcount ();
        }

    protected:
        /// input stream.
        std::istream* _in;
    };

    /**
     * @brief HTTP stream view.
     */
    class HttpStreamView : public StreamView
    {
    public:
        /**
         * @brief default constructor.
         * @param in input stream.
         */
        HttpStreamView (HttpClient& in)
        : StreamView (in)
        {
        }

        /**
         * @brief copy constructor.
         * @param other object to copy.
         */
        HttpStreamView (const HttpStreamView& other) = delete;

        /**
         * @brief copy assignment.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        HttpStreamView& operator= (const HttpStreamView& other) = delete;

        /**
         * @brief move constructor.
         * @param other object to move.
         */
        HttpStreamView (HttpStreamView&& other) = delete;

        /**
         * @brief move assignment.
         * @param other object to move.
         * @return a reference of the current object.
         */
        HttpStreamView& operator=(HttpStreamView&& other) = delete;

        /**
         * @brief destroy instance.
         */
        virtual ~HttpStreamView () = default;

        /**
         * @brief get character without extracting it.
         * @return extracted character.
         */
        virtual int peek () noexcept override
        {
            if (reinterpret_cast <HttpClient*> (_in)->encoding () == "chunked")
            {
                return peekchunked ();
            }

            return _in->peek ();
        }

        /**
         * @brief extracts character.
         * @return extracted character.
         */
        virtual int get () noexcept override
        {
            if (reinterpret_cast <HttpClient*> (_in)->encoding () == "chunked")
            {
                return getchunked ();
            }

            return _in->get ();
        }

        /**
         * @brief reads count characters from the input sequence.
         * @param buf pointer to a char array.
         * @param count maximum number of characters to read.
         * @return the number of characters successfully read.
         */
        virtual size_t read (char* buf, size_t count) override
        {
            if (reinterpret_cast <HttpClient*> (_in)->encoding () == "chunked")
            {
                return readchunked (buf, count);
            }

            _in->read (buf, count);
            return _in->gcount ();
        }

    protected:
        /**
         * @brief read chunk size.
         * @param chunksize chunk size. 
         * @return true on success, false otherwise.
         */
        bool readchunksize (size_t& chunksize) const
        {
            char* end = nullptr;
            std::string line;

            if (HttpMessage::getline (*_in, line, 4096))
            {
                chunksize = strtol (line.c_str (), &end, 16);
            }

            return (*end == '\0');
        }

        /**
         * @brief get character without extracting it.
         * @return extracted character.
         */
        int peekchunked () noexcept
        {
            while (!_blocksize)
            {
                if (!readchunksize (_blocksize))
                {
                    return std::char_traits <char>::eof ();
                }
            }

            return _in->peek ();
        }

        /**
         * @brief extracts character.
         * @return extracted character.
         */
        int getchunked () noexcept
        {
            char ch;
            if (readchunked (&ch, 1) != 1)
            {
                return std::char_traits <char>::eof ();
            }
            return ch;
        }

        /**
         * @brief reads count characters from the input sequence.
         * @param buf pointer to a char array.
         * @param count maximum number of characters to read.
         * @return the number of characters successfully read. 
         */
        size_t readchunked (char* buf, size_t count)
        {
            size_t nread = 0;
            std::string line;

            while (nread < count)
            {
                if (!_blocksize && !readchunksize (_blocksize))
                {
                    break;
                }

                size_t remaining = count - nread;
                _in->read (buf + nread, std::min (remaining, _blocksize));
                if (_in->fail ())
                {
                    break;
                }

                nread      += _in->gcount ();
                _blocksize -= _in->gcount ();
            }

            return nread;
        }

        /// block size.
        size_t _blocksize = 0;
    };
}

#endif
