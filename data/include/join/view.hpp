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
    namespace details
    {
        struct alignas(64) EscapedTable
        {
            uint8_t data[256];

            constexpr EscapedTable () : data {}
            {
                for (unsigned i = 0; i < 0x20; ++i) { data[i] = 1; }
                data['"']  = 1;
                data['\\'] = 1;
            }
        };

        constexpr EscapedTable escapedLookup {};

        struct alignas(64) WhitespaceTable
        {
            uint8_t data[256];

            constexpr WhitespaceTable () : data {}
            {
                data['\t'] = 1;
                data['\n'] = 1;
                data['\r'] = 1;
                data[' ']  = 1;
            }
        };

        constexpr WhitespaceTable whitespaceLookup {};
    }

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
         * @brief read characters until escaped.
         * @param out output buffer.
         */
        inline void readUntilEscaped (std::string& out) noexcept
        {
            const char* beg = _cur;
            const char* cur = _cur;
            const char* end = _end;

            while (cur < end && !details::escapedLookup.data[static_cast <unsigned char> (*cur)])
            {
                ++cur;
            }

            out.append (beg, static_cast <size_t> (cur - beg));
            _cur = cur;
        }

        /**
         * @brief skip whitespaces.
         * @return 0 on success, -1 otherwise.
         */
        inline int skipWhitespaces () noexcept
        {
            const char* cur = _cur;
            const char* end = _end;

            while (cur < end && details::whitespaceLookup.data[static_cast <unsigned char> (*cur)])
            {
                ++cur;
            }

            _cur = cur;

            return 0;
        }

        /**
         * @brief skip whitespaces and comments.
         * @return 0 on success, -1 otherwise.
         */
        inline int skipWhitespacesAndComments () noexcept
        {
            const char* cur = _cur;
            const char* end = _end;

            while (cur < end)
            {
                while (cur < end && details::whitespaceLookup.data[static_cast <unsigned char> (*cur)])
                {
                    ++cur;
                }

                if (cur >= end || *cur != '/')
                {
                    break;
                }

                if (++cur >= end)
                {
                    return -1;
                }

                if (*cur == '*')
                {
                    ++cur;
                    bool closed = false;
                    while (cur < end)
                    {
                        if (*cur == '*' && (cur + 1 < end) && *(cur + 1) == '/')
                        {
                            cur += 2;
                            closed = true;
                            break;
                        }
                        ++cur;
                    }
                    if (!closed)
                    {
                        return -1;
                    }
                }
                else if (*cur == '/')
                {
                    ++cur;
                    const char* p = static_cast <const char*> (memchr (cur, '\n', end - cur));
                    cur = p ? p : end;
                }
                else
                {
                    return -1;
                }
            }

            _cur = cur;

            return 0;
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
         * @brief read characters until escaped.
         * @param out output buffer.
         */
        inline void readUntilEscaped (std::string& out) noexcept
        {
            int c;
            while ((c = _in->sgetc ()) != std::char_traits <char>::eof () && !details::escapedLookup.data[static_cast <unsigned char> (c)])
            {
                out.push_back (static_cast <char> (c));
                _in->sbumpc ();
            }
        }

        /**
         * @brief skip whitespaces.
         * @return 0 on success, -1 otherwise.
         */
        inline int skipWhitespaces () noexcept
        {
            int c;
            while ((c = _in->sgetc ()) != std::char_traits <char>::eof () && details::whitespaceLookup.data[static_cast <unsigned char> (c)])
            {
                _in->sbumpc ();
            }

            return 0;
        }

        /**
         * @brief skip whitespaces and comments.
         * @return 0 on success, -1 otherwise.
         */
        inline int skipWhitespacesAndComments () noexcept
        {
            int c;

            while ((c = _in->sgetc ()) != std::char_traits <char>::eof ())
            {
                while ((c = _in->sgetc ()) != std::char_traits <char>::eof () && details::whitespaceLookup.data[static_cast <unsigned char> (c)])
                {
                    _in->sbumpc ();
                }

                if (c != '/')
                {
                    break;
                }

                _in->sbumpc ();
                c = _in->sgetc ();

                if (c == std::char_traits <char>::eof ())
                {
                    return -1;
                }

                if (c == '*')
                {
                    _in->sbumpc ();
                    bool closed = false;

                    while ((c = _in->sbumpc ()) != std::char_traits <char>::eof ())
                    {
                        if (c == '*' && _in->sgetc () == '/')
                        {
                            _in->sbumpc ();
                            closed = true;
                            break;
                        }
                    }

                    if (!closed)
                    {
                        return -1;
                    }
                }
                else if (c == '/')
                {
                    _in->sbumpc ();

                    while ((c = _in->sbumpc ()) != std::char_traits <char>::eof () && c != '\n')
                    {
                    }
                }
                else
                {
                    return -1;
                }
            }

            return 0;
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
