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

#ifndef JOIN_DATA_JSON_HPP
#define JOIN_DATA_JSON_HPP

// libjoin.
#include <join/atodpow.hpp>
#include <join/value.hpp>
#include <join/utf16.hpp>
#include <join/utf8.hpp>
#include <join/dtoa.hpp>
#include <join/probe.hpp>
#include <join/sax.hpp>

// C++.
#include <type_traits>
#include <functional>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <memory>
#include <vector>
#include <locale>

namespace join
{
    namespace details
    {
        constexpr char digitPairs[201] = {
            "00010203040506070809"
            "10111213141516171819"
            "20212223242526272829"
            "30313233343536373839"
            "40414243444546474849"
            "50515253545556575859"
            "60616263646566676869"
            "70717273747576777879"
            "80818283848586878889"
            "90919293949596979899"};

        struct UnescapedTable
        {
            uint8_t data[256];

            constexpr UnescapedTable ()
            : data{}
            {
                for (int i = 0; i < 32; ++i)
                {
                    data[i] = 'u';
                }
                data[static_cast<unsigned char> ('\b')] = 'b';
                data[static_cast<unsigned char> ('\t')] = 't';
                data[static_cast<unsigned char> ('\n')] = 'n';
                data[static_cast<unsigned char> ('\f')] = 'f';
                data[static_cast<unsigned char> ('\r')] = 'r';
                data[static_cast<unsigned char> ('"')] = '"';
                data[static_cast<unsigned char> ('\\')] = '\\';
            }
        };

        constexpr UnescapedTable unescapedLookup{};

        constexpr char hexDigits[17] = "0123456789abcdef";
    }

    struct LocaleDelete
    {
        constexpr LocaleDelete () noexcept = default;

        void operator() (locale_t loc) noexcept
        {
            freelocale (loc);
        }
    };

    using LocalePtr = std::unique_ptr<std::remove_pointer_t<locale_t>, LocaleDelete>;

    /**
     * @brief JSON error codes.
     */
    enum class JsonErrc
    {
        InvalidComment = 1,   /**< comment is invalid. */
        InvalidEscaping,      /**< character escaping is invalid. */
        InvalidEncoding,      /**< character encoding is invalid. */
        IllegalCharacter,     /**< illegal character. */
        MissingCurlyBracket,  /**< missing curly bracket. */
        MissingSquareBracket, /**< missing square bracket. */
        MissingQuote,         /**< missing quote. */
        MissingColon,         /**< missing colon. */
        MissingComma,         /**< missing comma. */
        EndOfFile,            /**< end of file. */
    };

    /**
     * @brief JSON error category.
     */
    class JsonCategory : public std::error_category
    {
    public:
        /**
         * @brief get JSON error category name.
         * @return JSON error category name.
         */
        virtual const char* name () const noexcept;

        /**
         * @brief translate JSON error code to human readable error string.
         * @param code error code.
         * @return human readable error string.
         */
        virtual std::string message (int code) const;
    };

    /**
     * @brief get error category.
     * @return the created std::error_category object.
     */
    const std::error_category& jsonCategory () noexcept;

    /**
     * @brief create an std::error_code object.
     * @param code error code number.
     * @return the created std::error_code object.
     */
    std::error_code make_error_code (JsonErrc code) noexcept;

    /**
     * @brief create an std::error_condition object.
     * @param code error code number.
     * @return the created std::error_condition object.
     */
    std::error_condition make_error_condition (JsonErrc code) noexcept;

    /**
     * @brief JSON serialization mode.
     */
    enum class JsonWriteMode
    {
        None,      /**< no write mode set. */
        Canonical, /**< canonical form. */
    };

    /**
     * @brief JSON writer class.
     */
    template <JsonWriteMode WriteMode = JsonWriteMode::None>
    class BasicJsonWriter
    {
    public:
        /**
         * @brief create instance.
         * @param document JSON document to create.
         * @param indentation number of characters used to indent JSON.
         */
        explicit BasicJsonWriter (std::ostream& document, size_t indentation = 0)
        : _out (document.rdbuf ())
        , _indentation ((WriteMode == JsonWriteMode::Canonical) ? 0 : indentation)
        {
        }

        /**
         * @brief copy constructor.
         * @param other object to copy.
         */
        BasicJsonWriter (const BasicJsonWriter& other) = delete;

        /**
         * @brief copy assignment.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        BasicJsonWriter& operator= (const BasicJsonWriter& other) = delete;

        /**
         * @brief move constructor.
         * @param other object to move.
         */
        BasicJsonWriter (BasicJsonWriter&& other) = delete;

        /**
         * @brief move assignment.
         * @param other object to move.
         * @return a reference of the current object.
         */
        BasicJsonWriter& operator= (BasicJsonWriter&& other) = delete;

        /**
         * @brief destroy instance.
         */
        ~BasicJsonWriter () = default;

        /**
         * @brief set null value.
         * @return 0 on success, -1 otherwise.
         */
        int setNull ()
        {
            array ();
            append ("null", 4);
            endValue ();
            return 0;
        }

        /**
         * @brief set boolean value.
         * @param value boolean value to set.
         * @return 0 on success, -1 otherwise.
         */
        int setBool (bool value)
        {
            array ();
            if (value)
            {
                append ("true", 4);
            }
            else
            {
                append ("false", 5);
            }
            endValue ();
            return 0;
        }

        /**
         * @brief set integer value.
         * @param value integer value to set.
         * @return 0 on success, -1 otherwise.
         */
        int setInt (int32_t value)
        {
            array ();
            writeInt (value);
            endValue ();
            return 0;
        }

        /**
         * @brief set unsigned integer value.
         * @param value unsigned integer value to set.
         * @return 0 on success, -1 otherwise.
         */
        int setUint (uint32_t value)
        {
            array ();
            writeUint (value);
            endValue ();
            return 0;
        }

        /**
         * @brief set 64 bits integer value.
         * @param value 64 bits integer value to set.
         * @return 0 on success, -1 otherwise.
         */
        int setInt64 (int64_t value)
        {
            array ();
            writeInt64 (value);
            endValue ();
            return 0;
        }

        /**
         * @brief set unsigned 64 bits integer value.
         * @param value unsigned 64 bits integer value to set.
         * @return 0 on success, -1 otherwise.
         */
        int setUint64 (uint64_t value)
        {
            array ();
            writeUint64 (value);
            endValue ();
            return 0;
        }

        /**
         * @brief set real value.
         * @param value real value to set.
         * @return 0 on success, -1 otherwise.
         */
        template <JsonWriteMode Mode = WriteMode>
        int setDouble (double value)
            requires (Mode == JsonWriteMode::None)
        {
            array ();
            uint64_t bits;
            memcpy (&bits, &value, sizeof (bits));
            const bool neg = (bits >> 63) != 0;
            const uint64_t exp = (bits >> 52) & 0x7FFULL;
            const uint64_t frac = bits & 0x000FFFFFFFFFFFFFULL;
            if (exp != 0x7FFULL)
            {
                writeDouble (value);
            }
            else if (frac != 0)
            {
                if (neg)
                {
                    append ("-NaN", 4);
                }
                else
                {
                    append ("NaN", 3);
                }
            }
            else
            {
                if (neg)
                {
                    append ("-Inf", 4);
                }
                else
                {
                    append ("Inf", 3);
                }
            }
            endValue ();
            return 0;
        }

        /**
         * @brief set real value in canonical form.
         * @param value real value to set.
         * @return 0 on success, -1 otherwise.
         */
        template <JsonWriteMode Mode = WriteMode>
        int setDouble (double value)
            requires (Mode == JsonWriteMode::Canonical)
        {
            array ();
            if (std::isfinite (value))
            {
                if ((std::trunc (value) == value) && (value >= 0) &&
                    (value < static_cast<double> (std::numeric_limits<uint64_t>::max ())))
                {
                    writeUint64 (static_cast<uint64_t> (value));
                }
                else if ((std::trunc (value) == value) &&
                         (value >= static_cast<double> (std::numeric_limits<int64_t>::min ())) &&
                         (value < static_cast<double> (std::numeric_limits<int64_t>::max ())))
                {
                    writeInt64 (static_cast<int64_t> (value));
                }
                else
                {
                    writeDouble (value);
                }
            }
            else
            {
                append ("null", 4);
            }
            endValue ();
            return 0;
        }

        /**
         * @brief set string value.
         * @param value string value to set.
         * @return 0 on success, -1 otherwise.
         */
        int setString (const std::string& value)
        {
            array ();
            append ('"');
            if (writeEscaped (value) == -1)
            {
                reset ();
                return -1;
            }
            append ('"');
            endValue ();
            return 0;
        }

        /**
         * @brief start array.
         * @param size array size (ignored).
         * @return 0 on success, -1 otherwise.
         */
        int startArray ([[maybe_unused]] uint32_t size = 0)
        {
            if (JOIN_UNLIKELY (_depth == saxMaxDepth))
            {
                join::lastError = make_error_code (SaxErrc::StackOverflow);
                reset ();
                return -1;
            }
            array ();
            append ('[');
            _arrays[_depth >> 6] |= 1ULL << (_depth & 63);
            ++_depth;
            _first = true;
            return 0;
        }

        /**
         * @brief stop array.
         * @return 0 on success, -1 otherwise.
         */
        int stopArray ()
        {
            if (JOIN_UNLIKELY (_depth == 0))
            {
                join::lastError = make_error_code (SaxErrc::InvalidParent);
                reset ();
                return -1;
            }
            --_depth;
            if (!_first)
            {
                endLine ();
                indent ();
            }
            append (']');
            endValue ();
            return 0;
        }

        /**
         * @brief start object.
         * @param size object size (ignored).
         * @return 0 on success, -1 otherwise.
         */
        int startObject ([[maybe_unused]] uint32_t size = 0)
        {
            if (JOIN_UNLIKELY (_depth == saxMaxDepth))
            {
                join::lastError = make_error_code (SaxErrc::StackOverflow);
                reset ();
                return -1;
            }
            array ();
            append ('{');
            _arrays[_depth >> 6] &= ~(1ULL << (_depth & 63));
            ++_depth;
            _first = true;
            return 0;
        }

        /**
         * @brief get object members in writing order.
         * @param object object to write.
         * @return object members in insertion order.
         */
        template <JsonWriteMode Mode = WriteMode>
        const Object& members (const Object& object) const noexcept
            requires (Mode == JsonWriteMode::None)
        {
            return object;
        }

        /**
         * @brief get object members sorted by key in UTF-16 order.
         * @param object object to write.
         * @return sorted object members.
         */
        template <JsonWriteMode Mode = WriteMode>
        std::vector<std::reference_wrapper<const Member>> members (const Object& object) const
            requires (Mode == JsonWriteMode::Canonical)
        {
            std::vector<std::reference_wrapper<const Member>> sorted (object.begin (), object.end ());
            std::sort (sorted.begin (), sorted.end (), [] (const Member& a, const Member& b) {
                if (!a.first.isString () || !b.first.isString ())
                {
                    return !a.first.isString () && b.first.isString ();
                }
                const std::string& sa = a.first.getString ();
                const std::string& sb = b.first.getString ();
                return utf8::compareUtf16Order (sa.data (), sa.data () + sa.size (), sb.data (),
                                                sb.data () + sb.size ()) < 0;
            });
            return sorted;
        }

        /**
         * @brief set key.
         * @param key object key.
         * @return 0 on success, -1 otherwise.
         */
        int setKey (const Value& key)
        {
            if (JOIN_UNLIKELY (_depth == 0))
            {
                join::lastError = make_error_code (SaxErrc::InvalidParent);
                reset ();
                return -1;
            }
            if (JOIN_UNLIKELY (!key.isString ()))
            {
                join::lastError = make_error_code (SaxErrc::InvalidKey);
                reset ();
                return -1;
            }
            comma ();
            endLine ();
            indent ();
            append ('"');
            if (writeEscaped (key.getString ()) == -1)
            {
                reset ();
                return -1;
            }
            append ('"');
            append (':');
            space ();
            _first = true;
            return 0;
        }

        /**
         * @brief stop object.
         * @return 0 on success, -1 otherwise.
         */
        int stopObject ()
        {
            if (JOIN_UNLIKELY (_depth == 0))
            {
                join::lastError = make_error_code (SaxErrc::InvalidParent);
                reset ();
                return -1;
            }
            --_depth;
            if (!_first)
            {
                endLine ();
                indent ();
            }
            append ('}');
            endValue ();
            return 0;
        }

        /**
         * @brief flush buffered data to output stream.
         */
        void flush ()
        {
            if (_size)
            {
                _out->sputn (_buffer, _size);
                _size = 0;
            }
        }

        /**
         * @brief reset the writer state, discarding buffered data.
         */
        void reset () noexcept
        {
            _size = 0;
            _depth = 0;
            _first = true;
        }

    protected:
        /**
         * @brief write integer value.
         * @param value integer value to write.
         */
        void writeInt (int32_t value)
        {
            if (value == std::numeric_limits<int32_t>::min ())
            {
                append ('-');
                writeUint64 (static_cast<uint64_t> (std::numeric_limits<int32_t>::max ()) + 1);
                return;
            }

            if (value < 0)
            {
                append ('-');
                writeUint64 (static_cast<uint64_t> (-value));
                return;
            }

            writeUint64 (static_cast<uint64_t> (value));
        }

        /**
         * @brief write unsigned integer value.
         * @param value unsigned integer value to write.
         */
        void writeUint (uint32_t value)
        {
            writeUint64 (static_cast<uint64_t> (value));
        }

        /**
         * @brief write 64 bits integer value.
         * @param value 64 bits integer value to write.
         */
        void writeInt64 (int64_t value)
        {
            if (value == std::numeric_limits<int64_t>::min ())
            {
                append ('-');
                writeUint64 (static_cast<uint64_t> (std::numeric_limits<int64_t>::max ()) + 1);
                return;
            }

            if (value < 0)
            {
                append ('-');
                writeUint64 (static_cast<uint64_t> (-value));
                return;
            }

            writeUint64 (static_cast<uint64_t> (value));
        }

        /**
         * @brief write 64 bits unsigned integer value.
         * @param value 64 bits unsigned integer value to write.
         */
        void writeUint64 (uint64_t value)
        {
            static constexpr uint64_t pow10[20] = {1ULL,
                                                   10ULL,
                                                   100ULL,
                                                   1000ULL,
                                                   10000ULL,
                                                   100000ULL,
                                                   1000000ULL,
                                                   10000000ULL,
                                                   100000000ULL,
                                                   1000000000ULL,
                                                   10000000000ULL,
                                                   100000000000ULL,
                                                   1000000000000ULL,
                                                   10000000000000ULL,
                                                   100000000000000ULL,
                                                   1000000000000000ULL,
                                                   10000000000000000ULL,
                                                   100000000000000000ULL,
                                                   1000000000000000000ULL,
                                                   10000000000000000000ULL};

            uint64_t high = value / 100000000;
            uint64_t top = high / 100000000;
            uint32_t mid = static_cast<uint32_t> (high - top * 100000000);
            uint32_t low = static_cast<uint32_t> (value - high * 100000000);

            char buffer[40];
            std::memcpy (buffer, &details::digitPairs[(top / 100) * 2], 2);
            std::memcpy (buffer + 2, &details::digitPairs[(top % 100) * 2], 2);
            std::memcpy (buffer + 4, &details::digitPairs[(mid / 1000000) * 2], 2);
            std::memcpy (buffer + 6, &details::digitPairs[(mid / 10000 % 100) * 2], 2);
            std::memcpy (buffer + 8, &details::digitPairs[(mid / 100 % 100) * 2], 2);
            std::memcpy (buffer + 10, &details::digitPairs[(mid % 100) * 2], 2);
            std::memcpy (buffer + 12, &details::digitPairs[(low / 1000000) * 2], 2);
            std::memcpy (buffer + 14, &details::digitPairs[(low / 10000 % 100) * 2], 2);
            std::memcpy (buffer + 16, &details::digitPairs[(low / 100 % 100) * 2], 2);
            std::memcpy (buffer + 18, &details::digitPairs[(low % 100) * 2], 2);

            size_t length = ((64 - __builtin_clzll (value | 1)) * 1233) >> 12;
            length += (value >= pow10[length]) + (value == 0);

            appendDigits (buffer + 20 - length, length);
        }

        /**
         * @brief write real value.
         * @param value real value to write.
         */
        void writeDouble (double value)
        {
            char buf[25];
            char* end = join::dtoa (buf, value);
            if constexpr (WriteMode == JsonWriteMode::Canonical)
            {
                for (char* pos = buf; pos < end; ++pos)
                {
                    append (*pos);
                    if ((*pos == 'e') && (*(pos + 1) != '-'))
                    {
                        append ('+');
                    }
                }
            }
            else
            {
                append (buf, end - buf);
            }
        }

        /**
         * @brief escape string value.
         * @param value string value to escape.
         * @return 0 on success, -1 otherwise.
         */
        template <JsonWriteMode Mode = WriteMode>
        int writeEscaped (const std::string& value)
            requires (Mode == JsonWriteMode::None)
        {
            const char* cur = value.data ();
            const char* end = cur + value.size ();

            while (cur != end)
            {
                const char* beg = cur;

                while (end - cur >= 8)
                {
                    uint64_t word, quote, backslash;
                    std::memcpy (&word, cur, 8);
                    quote = word ^ 0x2222222222222222ULL;
                    backslash = word ^ 0x5C5C5C5C5C5C5C5CULL;
                    uint64_t mask = (((quote - 0x0101010101010101ULL) & ~quote) |
                                     ((backslash - 0x0101010101010101ULL) & ~backslash) |
                                     ((word - 0x2020202020202020ULL) & ~word)) &
                                    0x8080808080808080ULL;
                    if (mask)
                    {
                        cur += __builtin_ctzll (mask) >> 3;
                        break;
                    }
                    cur += 8;
                }

                while (cur != end && details::unescapedLookup.data[static_cast<uint8_t> (*cur)] == 0)
                {
                    ++cur;
                }

                if (cur != beg)
                {
                    append (beg, cur - beg);
                }

                if (cur == end)
                {
                    break;
                }

                uint8_t ch = static_cast<uint8_t> (*cur);
                uint8_t esc = details::unescapedLookup.data[ch];
                if (esc == 'u')
                {
                    char seq[6] = {'\\', 'u', '0', '0', details::hexDigits[ch >> 4], details::hexDigits[ch & 0xF]};
                    append (seq, 6);
                }
                else
                {
                    char escapeSeq[2] = {'\\', static_cast<char> (esc)};
                    append (escapeSeq, 2);
                }

                ++cur;
            }

            return 0;
        }

        /**
         * @brief validate and escape string value.
         * @param value string value to escape.
         * @return 0 on success, -1 otherwise.
         */
        template <JsonWriteMode Mode = WriteMode>
        int writeEscaped (const std::string& value)
            requires (Mode == JsonWriteMode::Canonical)
        {
            if (JOIN_UNLIKELY (utf8::validate (value.data (), value.data () + value.size ()) == -1))
            {
                join::lastError = make_error_code (JsonErrc::InvalidEncoding);
                return -1;
            }
            return writeEscaped<JsonWriteMode::None> (value);
        }

        /**
         * @brief append character to output stream in batch.
         * @param data character to append to output stream.
         */
        inline void append (char data)
        {
            if (JOIN_UNLIKELY (_size == sizeof (_buffer)))
            {
                flush ();
            }
            _buffer[_size++] = data;
        }

        /**
         * @brief append data to output stream in batch.
         * @param data data to append to output stream.
         * @param size data size.
         */
        inline void append (const char* data, uint32_t size)
        {
            if (JOIN_UNLIKELY (size > sizeof (_buffer) - _size))
            {
                flush ();
                if (size >= sizeof (_buffer))
                {
                    _out->sputn (data, size);
                    return;
                }
            }
            std::memcpy (_buffer + _size, data, size);
            _size += size;
        }

        /**
         * @brief append the digits of a number, read 20 bytes at a time.
         * @param digits digits followed by enough readable bytes to read 20.
         * @param length number of digits.
         */
        inline void appendDigits (const char* digits, size_t length)
        {
            if (JOIN_UNLIKELY (sizeof (_buffer) - _size < 20))
            {
                flush ();
            }
            std::memcpy (_buffer + _size, digits, 20);
            _size += length;
        }

        /**
         * @brief write comma.
         */
        inline void comma ()
        {
            if (_depth && !_first)
            {
                append (',');
            }
        }

        /**
         * @brief write end of line.
         */
        inline void endLine ()
        {
            if (_indentation)
            {
                append ('\n');
            }
        }

        /**
         * @brief write indentation.
         */
        inline void indent ()
        {
            if (_indentation)
            {
                static constexpr char spaces[] = "                                                                ";
                size_t count = _depth * _indentation;
                while (count)
                {
                    size_t n = (count < sizeof (spaces) - 1) ? count : sizeof (spaces) - 1;
                    append (spaces, n);
                    count -= n;
                }
            }
        }

        /**
         * @brief write space.
         */
        inline void space ()
        {
            if (_indentation)
            {
                append (' ');
            }
        }

        /**
         * @brief add comma, go to line and indent if in array.
         */
        inline void array ()
        {
            comma ();
            if (_depth && ((_arrays[(_depth - 1) >> 6] >> ((_depth - 1) & 63)) & 1))
            {
                endLine ();
                indent ();
            }
        }

        /**
         * @brief mark the end of a value, flushing the document once complete.
         */
        inline void endValue ()
        {
            _first = false;
            if (_depth == 0)
            {
                flush ();
            }
        }

        /// underlying output stream.
        std::streambuf* _out;

        /// output buffer.
        char _buffer[1024] = {};

        /// output buffer size.
        size_t _size = 0;

        /// array flags of the open containers, one bit per depth.
        uint64_t _arrays[saxMaxDepth / 64] = {};

        /// number of open containers.
        size_t _depth = 0;

        /// indentation.
        size_t _indentation;

        /// is first element.
        bool _first = true;
    };

    /// JSON default writer.
    using JsonWriter = BasicJsonWriter<JsonWriteMode::None>;

    /// JSON canonicalizer, sorting object members when used with Document::serialize.
    using JsonCanonicalizer = BasicJsonWriter<JsonWriteMode::Canonical>;

    /**
     * @brief JSON deserialization mode.
     */
    enum JsonReadMode
    {
        None = 0,                    /**< no read mode set. */
        ParseComments = 1L << 0,     /**< parse comments. */
        ValidateEncoding = 1L << 1,  /**< validate UTF-8 encoding of strings. */
        StopParsingOnDone = 1L << 2, /**< stop parsing on done. */
    };

    /**
     * @brief perform binary AND on JsonReadMode.
     * @param a bitset.
     * @param b other bitset.
     * @return bitset result of binary AND on JsonReadMode.
     */
    constexpr JsonReadMode operator& (JsonReadMode a, JsonReadMode b) noexcept
    {
        return JsonReadMode (static_cast<int> (a) & static_cast<int> (b));
    }

    /**
     * @brief perform binary OR on JsonReadMode.
     * @param a bitset.
     * @param b other bitset.
     * @return bitset result of binary OR on JsonReadMode.
     */
    constexpr JsonReadMode operator| (JsonReadMode a, JsonReadMode b) noexcept
    {
        return JsonReadMode (static_cast<int> (a) | static_cast<int> (b));
    }

    /**
     * @brief perform binary AND on JsonReadMode.
     * @param a bitset.
     * @param b other bitset.
     * @return bitset result of binary AND on JsonReadMode.
     */
    constexpr const JsonReadMode& operator&= (JsonReadMode& a, JsonReadMode b) noexcept
    {
        return a = a & b;
    }

    /**
     * @brief perform binary OR on JsonReadMode.
     * @param a bitset.
     * @param b other bitset.
     * @return bitset result of binary OR.
     */
    constexpr const JsonReadMode& operator|= (JsonReadMode& a, JsonReadMode b) noexcept
    {
        return a = a | b;
    }

    /**
     * @brief JSON reader class.
     */
    template <JsonReadMode ReadMode = JsonReadMode::None>
    class BasicJsonReader
    {
    public:
        /**
         * @brief default constructor.
         */
        BasicJsonReader () = default;

        /**
         * @brief copy constructor.
         * @param other object to copy.
         */
        BasicJsonReader (const BasicJsonReader& other) = delete;

        /**
         * @brief copy assignment.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        BasicJsonReader& operator= (const BasicJsonReader& other) = delete;

        /**
         * @brief move constructor.
         * @param other object to move.
         */
        BasicJsonReader (BasicJsonReader&& other) = delete;

        /**
         * @brief move assignment.
         * @param other object to move.
         * @return a reference of the current object.
         */
        BasicJsonReader& operator= (BasicJsonReader&& other) = delete;

        /**
         * @brief destroy instance.
         */
        ~BasicJsonReader () = default;

        /**
         * @brief deserialize a document.
         * @param document document to parse.
         * @param length The length of the document to parse.
         * @param handler handler receiving the document.
         * @return 0 on success, -1 otherwise.
         */
        template <typename Handler>
        int deserialize (const char* document, size_t length, Handler& handler)
        {
            StringProbe in (document, length);
            return read (in, handler);
        }

        /**
         * @brief deserialize a document.
         * @param first The first character of the document to parse.
         * @param last The last character of the document to parse.
         * @param handler handler receiving the document.
         * @return 0 on success, -1 otherwise.
         */
        template <typename Handler>
        int deserialize (const char* first, const char* last, Handler& handler)
        {
            StringProbe in (first, last);
            return read (in, handler);
        }

        /**
         * @brief deserialize a document.
         * @param document document to parse.
         * @param handler handler receiving the document.
         * @return 0 on success, -1 otherwise.
         */
        template <typename Handler>
        int deserialize (const std::string& document, Handler& handler)
        {
            StringProbe in (document.c_str (), document.size ());
            return read (in, handler);
        }

        /**
         * @brief deserialize a document.
         * @param document document to deserialize.
         * @param handler handler receiving the document.
         * @return 0 on success, -1 otherwise.
         */
        template <typename Handler>
        int deserialize (std::stringstream& document, Handler& handler)
        {
            StringStreamProbe in (document);
            return read (in, handler);
        }

        /**
         * @brief deserialize a document.
         * @param document document to deserialize.
         * @param handler handler receiving the document.
         * @return 0 on success, -1 otherwise.
         */
        template <typename Handler>
        int deserialize (std::istringstream& document, Handler& handler)
        {
            StringStreamProbe in (document);
            return read (in, handler);
        }

        /**
         * @brief deserialize a document.
         * @param document document to deserialize.
         * @param handler handler receiving the document.
         * @return 0 on success, -1 otherwise.
         */
        template <typename Handler>
        int deserialize (std::fstream& document, Handler& handler)
        {
            FileStreamProbe in (document);
            return read (in, handler);
        }

        /**
         * @brief deserialize a document.
         * @param document document to deserialize.
         * @param handler handler receiving the document.
         * @return 0 on success, -1 otherwise.
         */
        template <typename Handler>
        int deserialize (std::ifstream& document, Handler& handler)
        {
            FileStreamProbe in (document);
            return read (in, handler);
        }

        /**
         * @brief deserialize a document.
         * @param document document to deserialize.
         * @param handler handler receiving the document.
         * @return 0 on success, -1 otherwise.
         */
        template <typename Handler>
        int deserialize (std::iostream& document, Handler& handler)
        {
            StreamProbe in (document);
            return read (in, handler);
        }

        /**
         * @brief deserialize a document.
         * @param document document to deserialize.
         * @param handler handler receiving the document.
         * @return 0 on success, -1 otherwise.
         */
        template <typename Handler>
        int deserialize (std::istream& document, Handler& handler)
        {
            StreamProbe in (document);
            return read (in, handler);
        }

    protected:
        /**
         * @brief parse a document, resetting the handler on failure.
         * @param document document to parse.
         * @param handler handler receiving the document.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ProbeType, typename Handler>
        int read (ProbeType& document, Handler& handler)
        {
            try
            {
                if (JOIN_LIKELY (readDocument (document, handler) == 0))
                {
                    return 0;
                }
            }
            catch (...)
            {
                handler.reset ();
                throw;
            }
            handler.reset ();
            return -1;
        }

        /**
         * @brief parse a document.
         * @param document document to parse.
         * @param handler handler receiving the document.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ProbeType, typename Handler>
        int readDocument (ProbeType& document, Handler& handler)
        {
            if (skipWhitespaces (document) != 0)
            {
                return -1;
            }

            if (readValue (document, handler, 0) != 0)
            {
                return -1;
            }

            if (ReadMode & JsonReadMode::StopParsingOnDone)
            {
                return 0;
            }

            if (skipWhitespaces (document) != 0)
            {
                return -1;
            }

            if (document.peek () != std::char_traits<char>::eof ())
            {
                join::lastError = make_error_code (SaxErrc::ExtraData);
                return -1;
            }

            return 0;
        }

        /**
         * @brief parse a JSON value.
         * @param document document to parse.
         * @param handler handler receiving the document.
         * @param depth number of enclosing containers.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ProbeType, typename Handler>
        int readValue (ProbeType& document, Handler& handler, size_t depth)
        {
            int ch = document.peek ();
            switch (ch)
            {
                case '[':
                    document.get ();
                    return readArray (document, handler, depth);
                case '{':
                    document.get ();
                    return readObject (document, handler, depth);
                case '"':
                    document.get ();
                    return readString (document, handler);
                case 'n':
                    document.get ();
                    return readNull (document, handler);
                case 't':
                    document.get ();
                    return readTrue (document, handler);
                case 'f':
                    document.get ();
                    return readFalse (document, handler);
                default:
                    return readNumber (document, handler);
            }
        }

        /**
         * @brief parse a null value.
         * @param document document to parse.
         * @param handler handler receiving the document.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ProbeType, typename Handler>
        int readNull (ProbeType& document, Handler& handler)
        {
            if (JOIN_UNLIKELY ((document.get () != 'u') || (document.get () != 'l') || (document.get () != 'l')))
            {
                join::lastError = make_error_code (SaxErrc::InvalidValue);
                return -1;
            }

            return handler.setNull ();
        }

        /**
         * @brief parse a true value.
         * @param document document to parse.
         * @param handler handler receiving the document.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ProbeType, typename Handler>
        int readTrue (ProbeType& document, Handler& handler)
        {
            if (JOIN_UNLIKELY ((document.get () != 'r') || (document.get () != 'u') || (document.get () != 'e')))
            {
                join::lastError = make_error_code (SaxErrc::InvalidValue);
                return -1;
            }

            return handler.setBool (true);
        }

        /**
         * @brief parse a false value.
         * @param document document to parse.
         * @param handler handler receiving the document.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ProbeType, typename Handler>
        int readFalse (ProbeType& document, Handler& handler)
        {
            if (JOIN_UNLIKELY ((document.get () != 'a') || (document.get () != 'l') || (document.get () != 's') ||
                               (document.get () != 'e')))
            {
                join::lastError = make_error_code (SaxErrc::InvalidValue);
                return -1;
            }

            return handler.setBool (false);
        }

        /**
         * @brief parse an infinity value.
         * @param document document to parse.
         * @param handler handler receiving the document.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ProbeType, typename Handler>
        int readInf (ProbeType& document, Handler& handler, bool negative)
        {
            if (JOIN_UNLIKELY (!(document.getIfNoCase ('n') && document.getIfNoCase ('f'))))
            {
                join::lastError = make_error_code (SaxErrc::InvalidValue);
                return -1;
            }

            if (JOIN_UNLIKELY (document.getIfNoCase ('i') &&
                               !(document.getIfNoCase ('n') && document.getIfNoCase ('i') &&
                                 document.getIfNoCase ('t') && document.getIfNoCase ('y'))))
            {
                join::lastError = make_error_code (SaxErrc::InvalidValue);
                return -1;
            }

            return handler.setDouble (negative ? -std::numeric_limits<double>::infinity ()
                                               : std::numeric_limits<double>::infinity ());
        }

        /**
         * @brief parse a nan value.
         * @param document document to parse.
         * @param handler handler receiving the document.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ProbeType, typename Handler>
        int readNan (ProbeType& document, Handler& handler, bool negative)
        {
            if (JOIN_UNLIKELY (!(document.getIfNoCase ('a') && document.getIfNoCase ('n'))))
            {
                join::lastError = make_error_code (SaxErrc::InvalidValue);
                return -1;
            }

            return handler.setDouble (negative ? -std::numeric_limits<double>::quiet_NaN ()
                                               : std::numeric_limits<double>::quiet_NaN ());
        }

        /**
         * @brief multiply 192 bits unsigned integer by 64 bits unsigned integer.
         * @param hi high 64 bits of the 128-bit multiplicand.
         * @param lo low 64 bits of the 128-bit multiplicand.
         * @param significand 64-bit multiplier.
         * @param high high 64 bits of the 192-bit result.
         * @param middle middle 64 bits of the 192-bit result.
         * @param low low 64 bits of the 192-bit result.
         */
        inline void umul192 (uint64_t hi, uint64_t lo, uint64_t significand, uint64_t& high, uint64_t& middle,
                             uint64_t& low) noexcept
        {
#if defined(__SIZEOF_INT128__)
            __uint128_t h = static_cast<__uint128_t> (hi) * significand;
            __uint128_t l = static_cast<__uint128_t> (lo) * significand;
            __uint128_t s = h + (l >> 64);

            high = static_cast<uint64_t> (s >> 64);
            middle = static_cast<uint64_t> (s);
            low = static_cast<uint64_t> (l);
#else
            uint64_t hi_hi, hi_lo, lo_hi, lo_lo;

            uint64_t m_lo = static_cast<uint32_t> (significand);
            uint64_t m_hi = significand >> 32;
            uint64_t p0 = (hi & 0xFFFFFFFF) * m_lo;
            uint64_t p1 = (hi >> 32) * m_lo;
            uint64_t p2 = (hi & 0xFFFFFFFF) * m_hi;
            uint64_t p3 = (hi >> 32) * m_hi;
            uint64_t carry = (p0 >> 32) + (p1 & 0xFFFFFFFF) + (p2 & 0xFFFFFFFF);
            hi_lo = (carry << 32) | (p0 & 0xFFFFFFFF);
            hi_hi = (carry >> 32) + (p1 >> 32) + (p2 >> 32) + p3;

            p0 = (lo & 0xFFFFFFFF) * m_lo;
            p1 = (lo >> 32) * m_lo;
            p2 = (lo & 0xFFFFFFFF) * m_hi;
            p3 = (lo >> 32) * m_hi;
            carry = (p0 >> 32) + (p1 & 0xFFFFFFFF) + (p2 & 0xFFFFFFFF);
            lo_lo = (carry << 32) | (p0 & 0xFFFFFFFF);
            lo_hi = (carry >> 32) + (p1 >> 32) + (p2 >> 32) + p3;

            low = lo_lo;
            middle = hi_lo + lo_hi;
            high = hi_hi + (middle < hi_lo ? 1 : 0);
#endif
        }

        /**
         * @brief convert double using fast path.
         * @param significand significand digits.
         * @param exponent exponent.
         * @param value converted value.
         * @return true on success, false otherwise.
         */
        inline bool strtodFast (uint64_t significand, int64_t exponent, double& value)
        {
            constexpr double pow10[] = {1e0,  1e1,  1e2,  1e3,  1e4,  1e5,  1e6,  1e7,  1e8,  1e9,  1e10, 1e11,
                                        1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22};

            value = static_cast<double> (significand);

            if (JOIN_LIKELY ((exponent >= -22) && (exponent <= 22) && (value <= 9007199254740991.0)))
            {
                value = (exponent < 0) ? (value / pow10[-exponent]) : (value * pow10[exponent]);
                return true;
            }

            if (JOIN_UNLIKELY ((exponent > 22) && (exponent < (22 + 16))))
            {
                const double scaled = value * pow10[exponent - 22];
                if (scaled <= 9007199254740991.0)
                {
                    value = scaled * pow10[22];
                    return true;
                }
            }

            if (JOIN_UNLIKELY (value == 0.0))
            {
                return true;
            }

            if (JOIN_UNLIKELY (exponent < -325 || exponent > 308))
            {
                return false;
            }

            uint64_t high, middle, low;
            const details::Power& power = details::atodpow[exponent + 325];
            umul192 (power.hi, power.lo, significand, high, middle, low);
            int64_t exp = ((exponent * 217706) >> 16) + 1087;

            int lz;
            if (high != 0)
            {
                lz = __builtin_clzll (high);
                exp -= lz;
            }
            else if (middle != 0)
            {
                lz = __builtin_clzll (middle);
                exp -= lz + 64;
            }
            else
            {
                return false;  // LCOV_EXCL_LINE
            }

            if (JOIN_UNLIKELY (exp <= 0 || exp >= 2047))
            {
                return false;
            }

            if (high == 0)
            {
                high = middle << lz;
                middle = 0;
            }
            else if (lz != 0)
            {
                high = (high << lz) | (middle >> (64 - lz));
                middle <<= lz;
            }

            middle |= (low != 0);

            uint64_t mant = (high >> 11) & 0xFFFFFFFFFFFFF;
            uint64_t bits = (static_cast<uint64_t> (exp) << 52) | mant;
            uint64_t frac = high & 0x7FF;

            if (JOIN_UNLIKELY ((frac == 0x3FF) && ((middle >> 8) == (~0ULL >> 8))))
            {
                return false;
            }

            bool roundUp = (frac > 0x400) | ((frac == 0x400) && ((middle != 0) || (mant & 1)));

            bits += roundUp;
            std::memcpy (&value, &bits, sizeof (double));

            return true;
        }

        /**
         * @brief convert double using strtod.
         * @param num number to convert.
         * @param value converted value.
         * @return true on success, false otherwise.
         */
        inline bool strtodSlow (const std::string& num, double& d)
        {
            static LocalePtr locale (newlocale (LC_ALL_MASK, "C", nullptr));
            char* end = nullptr;
            d = strtod_l (num.c_str (), &end, locale.get ());
            return (end && (*end == '\0'));
        }

        /**
         * @brief parse a number value.
         * @param document document to parse.
         * @param handler handler receiving the document.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ProbeType, typename Handler>
        int readNumber (ProbeType& document, Handler& handler)
        {
            BufferingProbe<ProbeType> probe (document);

            bool negative = probe.getIf ('-');

            uint64_t max64 = std::numeric_limits<uint64_t>::max ();
            if (negative)
            {
                max64 = static_cast<uint64_t> (std::numeric_limits<int64_t>::max ()) + 1;
            }

            uint64_t digits = 0;
            bool isDouble = false;
            uint64_t u = 0;

            if (JOIN_UNLIKELY (probe.getIf ('0')))
            {
                if (JOIN_UNLIKELY (isDigit (probe.peek ())))
                {
                    join::lastError = make_error_code (SaxErrc::InvalidValue);
                    return -1;
                }
            }
            else if (JOIN_LIKELY (isDigit (probe.peek ())))
            {
                digits = probe.readDigits (u, 18);

                while (JOIN_LIKELY (isDigit (probe.peek ())))
                {
                    int digit = probe.peek () - '0';

                    if (JOIN_UNLIKELY (u > ((max64 - digit) / 10)))
                    {
                        isDouble = true;
                        break;
                    }

                    u = (u * 10) + (probe.get () - '0');
                    ++digits;
                }
            }
            else if (JOIN_LIKELY (document.getIfNoCase ('i')))
            {
                return readInf (document, handler, negative);
            }
            else if (JOIN_LIKELY (document.getIfNoCase ('n')))
            {
                return readNan (document, handler, negative);
            }
            else
            {
                join::lastError = make_error_code (SaxErrc::InvalidValue);
                return -1;
            }

            if (isDouble)
            {
                digits += probe.readDigits (u);
            }

            int64_t frac = 0;
            if (probe.getIf ('.'))
            {
                isDouble = true;

                if (JOIN_UNLIKELY (digits == 0))
                {
                    while (probe.getIf ('0'))
                    {
                        --frac;
                    }
                }

                size_t count = probe.readDigits (u);
                digits += count;
                frac -= static_cast<int64_t> (count);
            }

            int64_t exponent = 0;
            if (probe.getIf ('e') || probe.getIf ('E'))
            {
                isDouble = true;

                bool negExp = false;
                if (isSign (probe.peek ()))
                {
                    negExp = (probe.get () == '-');
                }

                if (JOIN_LIKELY (isDigit (probe.peek ())))
                {
                    exponent = (probe.get () - '0');

                    while (JOIN_LIKELY (isDigit (probe.peek ())))
                    {
                        int digit = probe.get () - '0';

                        if (JOIN_LIKELY (exponent <= ((std::numeric_limits<int>::max () - digit) / 10)))
                        {
                            exponent = (exponent * 10) + digit;
                        }
                    }
                }
                else
                {
                    join::lastError = make_error_code (SaxErrc::InvalidValue);
                    return -1;
                }

                if (negExp)
                {
                    exponent = -exponent;
                }
            }

            if (!isDouble)
            {
                return negative ? handler.setInt64 (static_cast<int64_t> (-u)) : handler.setUint64 (u);
            }

            if (JOIN_LIKELY (digits <= 19))
            {
                double d = 0.0;
                if (strtodFast (u, exponent + frac, d))
                {
                    return handler.setDouble (negative ? -d : d);
                }
            }

            std::string number;
            probe.snapshot (number);

            double d = 0.0;
            if (strtodSlow (number, d))
            {
                return handler.setDouble (d);
            }

            join::lastError = make_error_code (SaxErrc::InvalidValue);  // LCOV_EXCL_LINE
            return -1;                                                  // LCOV_EXCL_LINE
        }

        /**
         * @brief parse a 4-digit hexadecimal sequence..
         * @param document document to parse.
         * @param u output variable to store the parsed hexadecimal value.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ProbeType>
        inline int readHex (ProbeType& document, uint32_t& u)
        {
            for (int i = 0; i < 4; ++i)
            {
                char c = document.get ();

                if (isDigit (c))
                {
                    c -= '0';
                }
                else if (isUpperAlpha (c))
                {
                    c = c - 'A' + 10;
                }
                else if (isLowerAlpha (c))
                {
                    c = c - 'a' + 10;
                }
                else
                {
                    join::lastError = make_error_code (JsonErrc::InvalidEncoding);
                    return -1;
                }

                u = (u << 4) + c;
            }

            return 0;
        }

        /**
         * @brief parse unicode.
         * @param document document to parse.
         * @param output parse output string.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ProbeType>
        inline int readUnicode (ProbeType& document, std::string& output)
        {
            uint32_t u = 0;

            if (readHex (document, u) == -1)
            {
                return -1;
            }

            char16_t units[2] = {static_cast<char16_t> (u), 0};
            const char16_t* cur = units;
            const char16_t* end = units + 1;

            if (utf16::isHighSurrogate (u))
            {
                if ((document.get () != '\\') || (document.get () != 'u'))
                {
                    join::lastError = make_error_code (JsonErrc::InvalidEncoding);
                    return -1;
                }

                uint32_t v = 0;

                if (readHex (document, v) == -1)
                {
                    join::lastError = make_error_code (JsonErrc::InvalidEncoding);
                    return -1;
                }

                units[1] = static_cast<char16_t> (v);
                ++end;
            }

            if (utf16::decode (cur, end, u) == -1)
            {
                join::lastError = make_error_code (JsonErrc::InvalidEncoding);
                return -1;
            }

            char buf[4];
            output.append (buf, utf8::encode (u, buf));

            return 0;
        }

        /**
         * @brief parse escaped sequence.
         * @param document document to parse.
         * @param output parse output string.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ProbeType>
        inline int readEscaped (ProbeType& document, std::string& output)
        {
            int ch = document.get ();
            switch (ch)
            {
                case '"':
                    output.push_back ('"');
                    break;
                case '\\':
                    output.push_back ('\\');
                    break;
                case '/':
                    output.push_back ('/');
                    break;
                case 'b':
                    output.push_back ('\b');
                    break;
                case 'f':
                    output.push_back ('\f');
                    break;
                case 'n':
                    output.push_back ('\n');
                    break;
                case 'r':
                    output.push_back ('\r');
                    break;
                case 't':
                    output.push_back ('\t');
                    break;
                case 'u':
                    return readUnicode (document, output);
                default:
                    join::lastError = make_error_code (JsonErrc::InvalidEscaping);
                    return -1;
            }

            return 0;
        }

        /**
         * @brief parse a string value.
         * @param document document to parse.
         * @param handler handler receiving the document.
         * @param isKey indicate whether the element to parse is a key or not.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ProbeType, typename Handler>
        int readString (ProbeType& document, Handler& handler, bool isKey = false)
        {
            thread_local Value output{std::string ()};
            output.clear ();
            output.reserve (64);

            for (;;)
            {
                size_t pos = output.getString ().size ();
                document.readUntilEscaped (output.getString ());

                if (JOIN_UNLIKELY (validateChunk (output.getString (), pos) == -1))
                {
                    return -1;
                }

                int ch = document.peek ();

                if (ch == '"')
                {
                    document.get ();
                    break;
                }

                if (ch == '\\')
                {
                    document.get ();
                    if (readEscaped (document, output.getString ()) == -1)
                    {
                        return -1;
                    }
                    continue;
                }

                if (ch < 0x20)
                {
                    join::lastError = make_error_code (JsonErrc::IllegalCharacter);
                    return -1;
                }
            }

            return isKey ? handler.setKey (Value (output)) : handler.setString (std::string (output.getString ()));
        }

        /**
         * @brief parse an array value.
         * @param document document to parse.
         * @param handler handler receiving the document.
         * @param depth number of enclosing containers.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ProbeType, typename Handler>
        JOIN_ALWAYS_INLINE int readArray (ProbeType& document, Handler& handler, size_t depth)
        {
            if (JOIN_UNLIKELY (depth == saxMaxDepth))
            {
                join::lastError = make_error_code (SaxErrc::StackOverflow);
                return -1;
            }

            if (JOIN_UNLIKELY (handler.startArray () == -1))
            {
                return -1;
            }

            if (JOIN_UNLIKELY (skipWhitespaces (document) == -1))
            {
                return -1;
            }

            if (document.getIf (']'))
            {
                return handler.stopArray ();
            }

            for (;;)
            {
                if (JOIN_UNLIKELY (readValue (document, handler, depth + 1) == -1))
                {
                    return -1;
                }

                int ch = document.get ();

                if (JOIN_UNLIKELY (details::whitespaceLookup.data[static_cast<unsigned char> (ch)]))
                {
                    if (JOIN_UNLIKELY (skipWhitespaces (document) == -1))
                    {
                        return -1;
                    }
                    ch = document.get ();
                }

                if (JOIN_LIKELY (ch == ','))
                {
                    if (JOIN_UNLIKELY (skipWhitespaces (document) == -1))
                    {
                        return -1;
                    }
                    continue;
                }

                if (JOIN_LIKELY (ch == ']'))
                {
                    break;
                }

                join::lastError = make_error_code (JsonErrc::MissingComma);
                return -1;
            }

            return handler.stopArray ();
        }

        /**
         * @brief parse an object value.
         * @param document document to parse.
         * @param handler handler receiving the document.
         * @param depth number of enclosing containers.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ProbeType, typename Handler>
        JOIN_ALWAYS_INLINE int readObject (ProbeType& document, Handler& handler, size_t depth)
        {
            if (JOIN_UNLIKELY (depth == saxMaxDepth))
            {
                join::lastError = make_error_code (SaxErrc::StackOverflow);
                return -1;
            }

            if (JOIN_UNLIKELY (handler.startObject () == -1))
            {
                return -1;
            }

            if (JOIN_UNLIKELY (skipWhitespaces (document) == -1))
            {
                return -1;
            }

            if (document.getIf ('}'))
            {
                return handler.stopObject ();
            }

            for (;;)
            {
                if (JOIN_UNLIKELY (document.get () != '"'))
                {
                    join::lastError = make_error_code (JsonErrc::MissingQuote);
                    return -1;
                }

                if (JOIN_UNLIKELY (readString (document, handler, true) == -1))
                {
                    return -1;
                }

                if (document.peek () != ':')
                {
                    if (JOIN_UNLIKELY (skipWhitespaces (document) == -1))
                    {
                        return -1;
                    }
                }

                if (JOIN_UNLIKELY (document.get () != ':'))
                {
                    join::lastError = make_error_code (JsonErrc::MissingColon);
                    return -1;
                }

                if (JOIN_UNLIKELY (skipWhitespaces (document) == -1))
                {
                    return -1;
                }

                if (JOIN_UNLIKELY (readValue (document, handler, depth + 1) == -1))
                {
                    return -1;
                }

                int ch = document.get ();

                if (JOIN_UNLIKELY (details::whitespaceLookup.data[static_cast<unsigned char> (ch)]))
                {
                    if (JOIN_UNLIKELY (skipWhitespaces (document) == -1))
                    {
                        return -1;
                    }
                    ch = document.get ();
                }

                if (JOIN_LIKELY (ch == ','))
                {
                    if (JOIN_UNLIKELY (skipWhitespaces (document) == -1))
                    {
                        return -1;
                    }
                    continue;
                }

                if (JOIN_LIKELY (ch == '}'))
                {
                    break;
                }

                join::lastError = make_error_code (JsonErrc::MissingComma);
                return -1;
            }

            return handler.stopObject ();
        }

        /**
         * @brief skip whitespaces, and comments when comment parsing is enabled.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ProbeType>
        inline int skipWhitespaces (ProbeType& document)
        {
            if constexpr (ReadMode & JsonReadMode::ParseComments)
            {
                return document.skipWhitespacesAndComments ();
            }
            else
            {
                return document.skipWhitespaces ();
            }
        }

        /**
         * @brief validate the encoding of the last chunk read, when encoding validation is enabled.
         * @param output parse output string.
         * @param pos position of the chunk in the output string.
         * @return 0 on success, -1 otherwise.
         */
        inline int validateChunk ([[maybe_unused]] const std::string& output, [[maybe_unused]] size_t pos) noexcept
        {
            if constexpr (ReadMode & JsonReadMode::ValidateEncoding)
            {
                if (JOIN_UNLIKELY (utf8::validate (output.data () + pos, output.data () + output.size ()) == -1))
                {
                    join::lastError = make_error_code (JsonErrc::InvalidEncoding);
                    return -1;
                }
            }
            return 0;
        }

        /**
         * @brief check if upper case alphanumeric character.
         * @param c character to check.
         * @return true if upper case alphanumeric character, false otherwise.
         */
        constexpr bool isUpperAlpha (char c) noexcept
        {
            return static_cast<unsigned char> (c - 'A') <= 5u;
        }

        /**
         * @brief check if lower case alphanumeric character.
         * @param c character to check.
         * @return true if lower case alphanumeric character, false otherwise.
         */
        constexpr bool isLowerAlpha (char c) noexcept
        {
            return static_cast<unsigned char> (c - 'a') <= 5u;
        }

        /**
         * @brief check if digit.
         * @param c character to check.
         * @return true if digit, false otherwise.
         */
        constexpr bool isDigit (char c) noexcept
        {
            return static_cast<unsigned char> (c - '0') <= 9u;
        }

        /**
         * @brief check if sign.
         * @param c character to check.
         * @return true if sign, false otherwise.
         */
        constexpr bool isSign (char c) noexcept
        {
            return ((c ^ '+') & (c ^ '-')) == 0;
        }
    };

    /// JSON default reader.
    using JsonReader = BasicJsonReader<JsonReadMode::None>;
}

namespace std
{
    /// JSON error code specialization.
    template <>
    struct is_error_condition_enum<join::JsonErrc> : public true_type
    {
    };
}

#endif
