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

#ifndef JOIN_DATA_PROBE_HPP
#define JOIN_DATA_PROBE_HPP

// libjoin.
#include <join/utils.hpp>

// C++.
#include <istream>
#include <string>
#include <vector>
#include <limits>

// C.
#include <cstring>
#include <cstddef>

namespace join
{
    namespace details
    {
        struct alignas (64) EscapedTable
        {
            uint8_t data[256];

            constexpr EscapedTable ()
            : data{}
            {
                for (unsigned i = 0; i < 0x20; ++i)
                {
                    data[i] = 1;
                }
                data[static_cast<unsigned char> ('"')] = 1;
                data[static_cast<unsigned char> ('\\')] = 1;
            }
        };

        constexpr EscapedTable escapedLookup{};

        struct alignas (64) WhitespaceTable
        {
            uint8_t data[256];

            constexpr WhitespaceTable ()
            : data{}
            {
                data[static_cast<unsigned char> ('\t')] = 1;
                data[static_cast<unsigned char> ('\n')] = 1;
                data[static_cast<unsigned char> ('\r')] = 1;
                data[static_cast<unsigned char> (' ')] = 1;
            }
        };

        constexpr WhitespaceTable whitespaceLookup{};
    }

    /**
     * @brief string probe.
     */
    class StringProbe
    {
    public:
        using ProbePos = const char*;

        /**
         * @brief default constructor.
         * @param in input string.
         * @param count number of characters.
         */
        constexpr StringProbe (const char* in, size_t count)
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
        constexpr StringProbe (const char* first, const char* last)
        : _cur (first)
        , _beg (first)
        , _end (last)
        {
        }

        /**
         * @brief default constructor.
         * @param in input string.
         */
        StringProbe (const char* in)
        : _cur (in)
        , _beg (in)
        , _end (in ? in + std::char_traits<char>::length (in) : in)
        {
        }

        /**
         * @brief copy constructor.
         * @param other object to copy.
         */
        StringProbe (const StringProbe& other) = default;

        /**
         * @brief copy assignment.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        StringProbe& operator= (const StringProbe& other) = default;

        /**
         * @brief move constructor.
         * @param other object to move.
         */
        StringProbe (StringProbe&& other) = default;

        /**
         * @brief move assignment.
         * @param other object to move.
         * @return a reference of the current object.
         */
        StringProbe& operator= (StringProbe&& other) = default;

        /**
         * @brief destroy instance.
         */
        ~StringProbe () = default;

        /**
         * @brief get character without extracting it.
         * @return extracted character.
         */
        inline int peek () const noexcept
        {
            if (JOIN_LIKELY (_cur < _end))
            {
                return static_cast<unsigned char> (*_cur);
            }
            return std::char_traits<char>::eof ();
        }

        /**
         * @brief extracts character.
         * @return extracted character.
         */
        inline int get () noexcept
        {
            if (JOIN_LIKELY (_cur < _end))
            {
                return static_cast<unsigned char> (*_cur++);
            }
            return std::char_traits<char>::eof ();
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
         * @brief read consecutive digits.
         * @param value value the digits are appended to.
         * @param max maximum number of digits to read.
         * @return number of digits read.
         */
        inline size_t readDigits (uint64_t& value, size_t max = std::numeric_limits<size_t>::max ()) noexcept
        {
            const char* cur = _cur;
            const char* end = (static_cast<size_t> (_end - _cur) > max) ? _cur + max : _end;
            uint64_t word;

            while ((end - cur >= 8) &&
                   (std::memcpy (&word, cur, 8),
                    ((word & 0xF0F0F0F0F0F0F0F0ULL) |
                     (((word + 0x0606060606060606ULL) & 0xF0F0F0F0F0F0F0F0ULL) >> 4)) == 0x3333333333333333ULL))
            {
                word = ((word & 0x0F0F0F0F0F0F0F0FULL) * 2561) >> 8;
                word = ((word & 0x00FF00FF00FF00FFULL) * 6553601) >> 16;
                word = ((word & 0x0000FFFF0000FFFFULL) * 42949672960001ULL) >> 32;
                value = (value * 100000000) + word;
                cur += 8;
            }

            while ((cur < end) && (static_cast<unsigned char> (*cur - '0') < 10))
            {
                value = (value * 10) + (*cur++ - '0');
            }

            size_t count = cur - _cur;
            _cur = cur;
            return count;
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

            while (end - cur >= 8)
            {
                uint64_t word, quote, backslash;
                std::memcpy (&word, cur, 8);
                quote = word ^ 0x2222222222222222ULL;
                backslash = word ^ 0x5C5C5C5C5C5C5C5CULL;
                uint64_t mask =
                    (((quote - 0x0101010101010101ULL) & ~quote) | ((backslash - 0x0101010101010101ULL) & ~backslash) |
                     ((word - 0x2020202020202020ULL) & ~word)) &
                    0x8080808080808080ULL;
                if (mask)
                {
                    cur += __builtin_ctzll (mask) >> 3;
                    break;
                }
                cur += 8;
            }

            while (cur < end && !details::escapedLookup.data[static_cast<unsigned char> (*cur)])
            {
                ++cur;
            }

            out.append (beg, static_cast<size_t> (cur - beg));
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

            while (cur < end && details::whitespaceLookup.data[static_cast<unsigned char> (*cur)])
            {
                ++cur;

                uint64_t word;
                while ((end - cur >= 8) && (std::memcpy (&word, cur, 8), word == 0x2020202020202020ULL))
                {
                    cur += 8;
                }
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
                while (cur < end && details::whitespaceLookup.data[static_cast<unsigned char> (*cur)])
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
                    const char* p = static_cast<const char*> (memchr (cur, '\n', end - cur));
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
        inline ProbePos tell () const noexcept
        {
            return _cur;
        }

        /**
         * @brief seek to the specified position.
         * @param pos position to seek to.
         */
        inline void seek (ProbePos pos) noexcept
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
        const char* _cur = nullptr;

        /// beginning position.
        const char* _beg = nullptr;

        /// ending position.
        const char* _end = nullptr;
    };

    /**
     * @brief basic stream probe.
     */
    template <bool Seekable = true>
    class BasicStreamProbe
    {
    public:
        using ProbePos = std::streampos;

        /**
         * @brief default constructor.
         * @param in input stream.
         */
        BasicStreamProbe (std::istream& in)
        : _in (in.rdbuf ())
        {
        }

        /**
         * @brief copy constructor.
         * @param other object to copy.
         */
        BasicStreamProbe (const BasicStreamProbe& other) = delete;

        /**
         * @brief copy assignment.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        BasicStreamProbe& operator= (const BasicStreamProbe& other) = delete;

        /**
         * @brief move constructor.
         * @param other object to move.
         */
        BasicStreamProbe (BasicStreamProbe&& other) = delete;

        /**
         * @brief move assignment.
         * @param other object to move.
         * @return a reference of the current object.
         */
        BasicStreamProbe& operator= (BasicStreamProbe&& other) = delete;

        /**
         * @brief destroy instance.
         */
        ~BasicStreamProbe () = default;

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
            if (_in->sgetc () == static_cast<int> (static_cast<unsigned char> (expected)))
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
            if (JOIN_LIKELY (c != std::char_traits<char>::eof ()))
            {
                if ((static_cast<char> (c) | 32) == (expected | 32))
                {
                    _in->sbumpc ();
                    return true;
                }
            }
            return false;
        }

        /**
         * @brief read consecutive digits.
         * @param value value the digits are appended to.
         * @param max maximum number of digits to read.
         * @return number of digits read.
         */
        inline size_t readDigits (uint64_t& value, size_t max = std::numeric_limits<size_t>::max ()) noexcept
        {
            size_t count = 0;
            while ((count < max) && (static_cast<unsigned> (peek () - '0') < 10))
            {
                value = (value * 10) + (get () - '0');
                ++count;
            }
            return count;
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
            while ((c = _in->sgetc ()) != std::char_traits<char>::eof () &&
                   !details::escapedLookup.data[static_cast<unsigned char> (c)])
            {
                out.push_back (static_cast<char> (c));
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
            while ((c = _in->sgetc ()) != std::char_traits<char>::eof () &&
                   details::whitespaceLookup.data[static_cast<unsigned char> (c)])
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

            while ((c = _in->sgetc ()) != std::char_traits<char>::eof ())
            {
                while ((c = _in->sgetc ()) != std::char_traits<char>::eof () &&
                       details::whitespaceLookup.data[static_cast<unsigned char> (c)])
                {
                    _in->sbumpc ();
                }

                if (c != '/')
                {
                    break;
                }

                _in->sbumpc ();
                c = _in->sgetc ();

                if (c == std::char_traits<char>::eof ())
                {
                    return -1;
                }

                if (c == '*')
                {
                    _in->sbumpc ();
                    bool closed = false;

                    while ((c = _in->sbumpc ()) != std::char_traits<char>::eof ())
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

                    while ((c = _in->sbumpc ()) != std::char_traits<char>::eof () && c != '\n')
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
        inline ProbePos tell () const noexcept
            requires Seekable
        {
            return _in->pubseekoff (0, std::ios::cur, std::ios::in);
        }

        /**
         * @brief seek to the specified position.
         * @param pos position to seek to.
         */
        inline void seek (ProbePos pos) noexcept
            requires Seekable
        {
            _in->pubseekpos (pos, std::ios::in);
        }

    private:
        /// input stream buffer.
        std::streambuf* _in;
    };

    /**
     * @brief string stream probe (seekable).
     */
    using StringStreamProbe = BasicStreamProbe<true>;

    /**
     * @brief file stream probe (seekable).
     */
    using FileStreamProbe = BasicStreamProbe<true>;

    /**
     * @brief stream probe (non-seekable, for pipes/network streams).
     */
    using StreamProbe = BasicStreamProbe<false>;

    /**
     * @brief trait to determine if a probe type is seekable.
     */
    template <typename ProbeType>
    inline constexpr bool is_seekable_v = false;

    /**
     * @brief specialization for StringProbe (seekable).
     */
    template <>
    inline constexpr bool is_seekable_v<StringProbe> = true;

    /**
     * @brief specialization for seekable probe.
     */
    template <>
    inline constexpr bool is_seekable_v<BasicStreamProbe<true>> = true;

    /**
     * @brief buffering probe adapter
     */
    template <typename ProbeType, bool Seekable = is_seekable_v<ProbeType>>
    class BufferingProbe;

    /**
     * @brief buffering probe specialization for seekable probe.
     */
    template <typename ProbeType>
    class BufferingProbe<ProbeType, true>
    {
    public:
        /**
         * @brief default constructor.
         * @param probe probe.
         */
        explicit BufferingProbe (ProbeType& probe)
        : _probe (probe)
        , _beg (probe.tell ())
        {
        }

        /**
         * @brief get character without extracting it.
         * @return extracted character.
         */
        inline int peek () const noexcept
        {
            return _probe.peek ();
        }

        /**
         * @brief extracts character.
         * @return extracted character.
         */
        inline int get () noexcept
        {
            return _probe.get ();
        }

        /**
         * @brief extracts expected character.
         * @param expected expected character.
         * @return true if extracted, false otherwise.
         */
        inline bool getIf (char expected) noexcept
        {
            return _probe.getIf (expected);
        }

        /**
         * @brief extracts expected character (case insensitive, ASCII-only).
         * @param expected expected character.
         * @return true if extracted, false otherwise.
         */
        inline bool getIfNoCase (char expected) noexcept
        {
            return _probe.getIfNoCase (expected);
        }

        /**
         * @brief read consecutive digits.
         * @param value value the digits are appended to.
         * @param max maximum number of digits to read.
         * @return number of digits read.
         */
        inline size_t readDigits (uint64_t& value, size_t max = std::numeric_limits<size_t>::max ()) noexcept
        {
            return _probe.readDigits (value, max);
        }

        /**
         * @brief get snapshot.
         * @param out destination.
         */
        inline void snapshot (std::string& out)
        {
            size_t len = _probe.tell () - _beg;
            out.resize (len);

            _probe.seek (_beg);
            _probe.read (&out[0], len);
        }

        /**
         * @brief consume buffered data.
         * @param out destination.
         */
        inline void consume (std::string& out)
        {
            snapshot (out);
            _beg = _probe.tell ();
        }

    private:
        /// underlying probe.
        ProbeType& _probe;

        /// start position.
        typename ProbeType::ProbePos _beg;
    };

    /**
     * @brief buffering probe specialization for non-seekable probe.
     */
    template <typename ProbeType>
    class BufferingProbe<ProbeType, false>
    {
    public:
        /**
         * @brief default constructor.
         * @param probe probe.
         */
        explicit BufferingProbe (ProbeType& probe)
        : _probe (probe)
        {
            static thread_local std::vector<char> buffer;
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
            return _probe.peek ();
        }

        /**
         * @brief extracts character.
         * @return extracted character.
         */
        inline int get () noexcept
        {
            const int c = _probe.get ();
            if (JOIN_LIKELY (c != std::char_traits<char>::eof ()))
            {
                _buf->push_back (static_cast<char> (c));
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
            if (_probe.peek () == static_cast<int> (static_cast<unsigned char> (expected)))
            {
                _buf->push_back (static_cast<char> (_probe.get ()));
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
            const int c = _probe.peek ();
            if (JOIN_LIKELY (c != std::char_traits<char>::eof ()))
            {
                if ((static_cast<char> (c) | 32) == (expected | 32))
                {
                    _buf->push_back (static_cast<char> (_probe.get ()));
                    return true;
                }
            }
            return false;
        }

        /**
         * @brief read consecutive digits.
         * @param value value the digits are appended to.
         * @param max maximum number of digits to read.
         * @return number of digits read.
         */
        inline size_t readDigits (uint64_t& value, size_t max = std::numeric_limits<size_t>::max ()) noexcept
        {
            size_t count = 0;
            while ((count < max) && (static_cast<unsigned> (peek () - '0') < 10))
            {
                value = (value * 10) + (get () - '0');
                ++count;
            }
            return count;
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
        /// underlying probe.
        ProbeType& _probe;

        /// buffer for consumed data.
        std::vector<char>* _buf = nullptr;
    };
}

#endif
