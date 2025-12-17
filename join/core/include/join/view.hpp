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
#include <join/utils.hpp>

// C++.
#include <istream>
#include <string>
#include <vector>

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
        using ViewPos = const char*;

        /**
         * @brief default constructor.
         * @param in input string.
         * @param count number of characters.
         */
        constexpr StringView (const char * in, size_t count)
        : _cur (in)
        , _beg (in)
        , _end (in ? in + count : in)
        {
        }

        /**
         * @brief default constructor.
         * @param first pointer to the first character of the string.
         * @param last pointer to the last character of the string.
         */
        constexpr StringView (const char * first, const char * last)
        : _cur (first)
        , _beg (first)
        , _end (last)
        {
        }

        /**
         * @brief default constructor.
         * @param in input string.
         */
        StringView (const char * in)
        : _cur (in)
        , _beg (in)
        , _end (in ? in + std::char_traits <char>::length (in) : in)
        {
        }

        /**
         * @brief copy constructor.
         * @param other object to copy.
         */
        StringView (const StringView& other) = default;

        /**
         * @brief copy assignment.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        StringView& operator= (const StringView& other) = default;

        /**
         * @brief move constructor.
         * @param other object to move.
         */
        StringView (StringView&& other) = default;

        /**
         * @brief move assignment.
         * @param other object to move.
         * @return a reference of the current object.
         */
        StringView& operator=(StringView&& other) = default;

        /**
         * @brief destroy instance.
         */
        ~StringView () = default;

        /**
         * @brief get character without extracting it.
         * @return extracted character.
         */
        inline int peek () const noexcept
        {
            if (JOIN_LIKELY (_cur < _end))
            {
                return static_cast <unsigned char> (*_cur);
            }
            return std::char_traits <char>::eof ();
        }

        /**
         * @brief extracts character.
         * @return extracted character.
         */
        inline int get () noexcept
        {
            if (JOIN_LIKELY (_cur < _end))
            {
                return static_cast <unsigned char> (*_cur++);
            }
            return std::char_traits <char>::eof ();
        }

        /**
         * @brief extracts expected character.
         * @param expected expected character.
         * @return true if extracted, false otherwise.
         */
        inline bool getIf (char expected) noexcept
        {
            if (JOIN_LIKELY (_cur < _end) && (*_cur == expected))
            {
                ++_cur;
                return true;
            }
            return false;
        }

        /**
         * @brief extracts expected character (case insensitive, ASCII-only).
         * @param expected expected character.
         * @return true if extracted, false otherwise.
         */
        inline bool getIfNoCase (char expected) noexcept
        {
            if (JOIN_LIKELY (_cur < _end))
            {
                const char c = *_cur;
                if ((c | 32) == (expected | 32))
                {
                    ++_cur;
                    return true;
                }
            }
            return false;
        }

        /**
         * @brief read characters.
         * @param buf output buffer.
         * @param count number of characters to read.
         * @return number of characters read.
         */
        inline size_t read (char* buf, size_t count) noexcept
        {
            const size_t available = _end - _cur;
            const size_t nread = (count < available) ? count : available;
            std::memcpy (buf, _cur, nread);
            _cur += nread;
            return nread;
        }

        /**
         * @brief read characters until delimiter.
         * @param out output string.
         * @param delim delimiter character.
         * @return number of characters read.
         */
        inline size_t readUntil (std::string& out, char delim)
        {
            const char* start = _cur;
            const char* pos = static_cast <const char*> (std::memchr (_cur, delim, _end - _cur));
            if (pos)
            {
                out.append (_cur, pos - _cur);
                _cur = pos;
            }
            else
            {
                out.append (_cur, _end - _cur);
                _cur = _end;
            }
            return _cur - start;
        }

        /**
         * @brief read characters until predicate returns true.
         * @param pred predicate function.
         * @param out output buffer.
         * @return number of characters read.
         */
        template <typename Predicate>
        inline size_t readUntil (std::string& out, Predicate pred)
        {
            const char* start = _cur;
            while (_cur < _end && !pred (*_cur))
            {
                ++_cur;
            }
            const size_t nread = _cur - start;
            out.append (start, nread);
            return nread;
        }

        /**
         * @brief consume characters until delimiter.
         * @param delim delimiter character.
         * @return number of characters consumed.
         */
        inline size_t consumeUntil (char delim) noexcept
        {
            const char* start = _cur;
            const char* pos = static_cast <const char*> (std::memchr (_cur, delim, _end - _cur));
            if (pos)
            {
                _cur = pos;
            }
            else
            {
                _cur = _end;
            }
            return _cur - start;
        }

        /**
         * @brief consume characters until predicate returns true.
         * @param pred predicate function.
         * @return number of characters consumed.
         */
        template <typename Predicate>
        inline size_t consumeUntil (Predicate pred) noexcept
        {
            const char* start = _cur;
            while (_cur < _end && !pred (*_cur))
            {
                ++_cur;
            }
            return _cur - start;
        }

        /**
         * @brief get input position indicator.
         * @return current position.
         */
        inline ViewPos tell () const noexcept
        {
            return _cur;
        }

        /**
         * @brief seek to the specified position.
         * @param pos position to seek to.
         */
        inline void seek (ViewPos pos) noexcept
        {
            if (JOIN_LIKELY (pos >= _beg && pos <= _end))
            {
                _cur = pos;
            }
            else if (pos < _beg)
            {
                _cur = _beg;
            }
            else
            {
                _cur = _end;
            }
        }

    private:
        /// current position.
        const char * _cur = nullptr;

        /// beginning position.
        const char * _beg = nullptr;

        /// ending position.
        const char * _end = nullptr;
    };

    /**
     * @brief basic stream view.
     */
    template <bool Seekable = true>
    class BasicStreamView
    {
    public:
        using ViewPos = std::streampos;

        /**
         * @brief default constructor.
         * @param in input stream.
         */
        BasicStreamView (std::istream& in)
        : _in (in.rdbuf ())
        {
        }

        /**
         * @brief copy constructor.
         * @param other object to copy.
         */
        BasicStreamView (const BasicStreamView& other) = delete;

        /**
         * @brief copy assignment.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        BasicStreamView& operator= (const BasicStreamView& other) = delete;

        /**
         * @brief move constructor.
         * @param other object to move.
         */
        BasicStreamView (BasicStreamView&& other) = delete;

        /**
         * @brief move assignment.
         * @param other object to move.
         * @return a reference of the current object.
         */
        BasicStreamView& operator=(BasicStreamView&& other) = delete;

        /**
         * @brief destroy instance.
         */
        ~BasicStreamView () = default;

        /**
         * @brief get character without extracting it.
         * @return extracted character.
         */
        inline int peek () const noexcept
        {
            return _in->sgetc ();
        }

        /**
         * @brief extracts character.
         * @return extracted character.
         */
        inline int get () noexcept
        {
            return _in->sbumpc ();
        }

        /**
         * @brief extracts expected character.
         * @param expected expected character.
         * @return true if extracted, false otherwise.
         */
        inline bool getIf (char expected) noexcept
        {
            if (_in->sgetc () == static_cast <int> (static_cast <unsigned char> (expected)))
            {
                _in->sbumpc ();
                return true;
            }
            return false;
        }

        /**
         * @brief extracts expected character (case insensitive, ASCII-only).
         * @param expected expected character.
         * @return true if extracted, false otherwise.
         */
        inline bool getIfNoCase (char expected) noexcept
        {
            const int c = _in->sgetc ();
            if (JOIN_LIKELY (c != std::char_traits <char>::eof ()))
            {
                if ((static_cast <char> (c) | 32) == (expected | 32))
                {
                    _in->sbumpc ();
                    return true;
                }
            }
            return false;
        }

        /**
         * @brief read characters.
         * @param buf output buffer.
         * @param count number of characters to read.
         * @return number of characters read.
         */
        inline size_t read (char* buf, size_t count) noexcept
        {
            return _in->sgetn (buf, count);
        }

        /**
         * @brief read characters until delimiter.
         * @param out output string.
         * @param delim delimiter character.
         * @return number of characters read.
         */
        inline size_t readUntil (std::string& out, char delim)
        {
            size_t nread = 0;
            int c;
            while ((c = _in->sgetc ()) != std::char_traits <char>::eof ())
            {
                const char ch = static_cast <char> (c);
                if (ch == delim)
                {
                    break;
                }
                out.push_back (ch);
                _in->sbumpc ();
                ++nread;
            }
            return nread;
        }

        /**
         * @brief read characters until predicate returns true.
         * @param pred predicate function.
         * @param out output string.
         * @return number of characters read.
         */
        template <typename Predicate>
        inline size_t readUntil (std::string& out, Predicate pred)
        {
            size_t nread = 0;
            int c;
            while ((c = _in->sgetc ()) != std::char_traits <char>::eof ())
            {
                const char ch = static_cast <char> (c);
                if (pred (ch))
                {
                    break;
                }
                out.push_back (ch);
                _in->sbumpc ();
                ++nread;
            }
            return nread;
        }

        /**
         * @brief consume characters until delimiter.
         * @param delim delimiter character.
         * @return number of characters consumed.
         */
        inline size_t consumeUntil (char delim) noexcept
        {
            size_t nread = 0;
            int c;
            while ((c = _in->sgetc ()) != std::char_traits <char>::eof ())
            {
                const char ch = static_cast <char> (c);
                if (ch == delim)
                {
                    break;
                }
                _in->sbumpc ();
                ++nread;
            }
            return nread;
        }

        /**
         * @brief consume characters until predicate returns true.
         * @param pred predicate function.
         * @return number of characters consumed.
         */
        template <typename Predicate>
        inline size_t consumeUntil (Predicate pred) noexcept
        {
            size_t nread = 0;
            int c;
            while ((c = _in->sgetc ()) != std::char_traits <char>::eof ())
            {
                const char ch = static_cast <char> (c);
                if (pred (ch))
                {
                    break;
                }
                _in->sbumpc ();
                ++nread;
            }
            return nread;
        }

        /**
         * @brief get input position indicator.
         * @return current position.
         */
        template <bool S = Seekable>
        inline typename std::enable_if <S, ViewPos>::type tell () const noexcept
        {
            return _in->pubseekoff (0, std::ios::cur, std::ios::in);
        }

        /**
         * @brief seek to the specified position.
         * @param pos position to seek to.
         */
        template <bool S = Seekable>
        inline typename std::enable_if <S, void>::type seek (ViewPos pos) noexcept
        {
            _in->pubseekpos (pos, std::ios::in);
        }

    private:
        /// input stream buffer.
        std::streambuf* _in;
    };

    /**
     * @brief string stream view (seekable).
     */
    using StringStreamView = BasicStreamView <true>;

    /**
     * @brief file stream view (seekable).
     */
    using FileStreamView = BasicStreamView <true>;

    /**
     * @brief stream view (non-seekable, for pipes/network streams).
     */
    using StreamView = BasicStreamView <false>;

    /**
     * @brief trait to determine if a view type is seekable.
     * @tparam ViewType view type to check.
     */
    template <typename ViewType>
    struct is_seekable : std::false_type {};

    /**
     * @brief specialization for StringView (seekable).
     */
    template <>
    struct is_seekable <StringView> : std::true_type {};

    /**
     * @brief specialization for seekable view.
     */
    template <>
    struct is_seekable <BasicStreamView <true>> : std::true_type {};

    /**
     * @brief buffering view adapter
     */
    template <typename ViewType, bool Seekable = is_seekable <ViewType>::value>
    class BufferingView;

    /**
     * @brief buffering view specialization for seekable view.
     */
    template <typename ViewType>
    class BufferingView <ViewType, true>
    {
    public:
        /**
         * @brief default constructor.
         * @param view view.
         */
        explicit BufferingView (ViewType& view)
        : _view (view)
        , _beg (view.tell ())
        {
        }

        /**
         * @brief get character without extracting it.
         * @return extracted character.
         */
        inline int peek () const noexcept
        {
            return _view.peek ();
        }

        /**
         * @brief extracts character.
         * @return extracted character.
         */
        inline int get () noexcept
        {
            return _view.get ();
        }

        /**
         * @brief extracts expected character.
         * @param expected expected character.
         * @return true if extracted, false otherwise.
         */
        inline bool getIf (char expected) noexcept
        {
            return _view.getIf (expected);
        }

        /**
         * @brief extracts expected character (case insensitive, ASCII-only).
         * @param expected expected character.
         * @return true if extracted, false otherwise.
         */
        inline bool getIfNoCase (char expected) noexcept
        {
            return _view.getIfNoCase (expected);
        }

        /**
         * @brief get snapshot.
         * @param out destination.
         */
        inline void snapshot (std::string& out)
        {
            size_t len = _view.tell () - _beg;
            out.resize (len);

            _view.seek (_beg);
            _view.read (&out[0], len);
        }

        /**
         * @brief consume buffered data.
         * @param out destination.
         */
        inline void consume (std::string& out)
        {
            snapshot (out);
            _beg = _view.tell ();
        }

    private:
        /// underlying view.
        ViewType& _view;

        /// start position.
        typename ViewType::ViewPos _beg;
    };

    /**
     * @brief buffering view specialization for non-seekable view.
     */
    template <typename ViewType>
    class BufferingView <ViewType, false>
    {
    public:
        /**
         * @brief default constructor.
         * @param view view.
         */
        explicit BufferingView (ViewType& view)
        : _view (view)
        {
            static thread_local std::vector <char> buffer;
            buffer.clear ();
            buffer.reserve (32);
            _buf = &buffer;
        }

        /**
         * @brief get character without extracting it.
         * @return extracted character.
         */
        inline int peek () const noexcept
        {
            return _view.peek ();
        }

        /**
         * @brief extracts character.
         * @return extracted character.
         */
        inline int get () noexcept
        {
            const int c = _view.get ();
            if (JOIN_LIKELY (c != std::char_traits <char>::eof ()))
            {
                _buf->push_back (static_cast <char> (c));
            }
            return c;
        }

        /**
         * @brief extracts expected character.
         * @param expected expected character.
         * @return true if extracted, false otherwise.
         */
        inline bool getIf (char expected) noexcept
        {
            if (_view.peek () == static_cast <int> (static_cast <unsigned char> (expected)))
            {
                _buf->push_back (static_cast <char> (_view.get ()));
                return true;
            }
            return false;
        }

        /**
         * @brief extracts expected character (case insensitive, ASCII-only).
         * @param expected expected character.
         * @return true if extracted, false otherwise.
         */
        inline bool getIfNoCase (char expected) noexcept
        {
            const int c = _view.peek ();
            if (JOIN_LIKELY (c != std::char_traits <char>::eof ()))
            {
                if ((static_cast <char> (c) | 32) == (expected | 32))
                {
                    _buf->push_back (static_cast <char> (_view.get ()));
                    return true;
                }
            }
            return false;
        }

        /**
         * @brief get snapshot.
         * @param out destination.
         */
        inline void snapshot (std::string& out)
        {
            out.assign (_buf->data (), _buf->size ());
        }

        /**
         * @brief consume buffered data.
         * @param out destination.
         */
        inline void consume (std::string& out)
        {
            snapshot (out);
            _buf->clear ();
        }

    private:
        /// underlying view.
        ViewType& _view;

        /// buffer for consumed data.
        std::vector <char>* _buf = nullptr;
    };
}

#endif
