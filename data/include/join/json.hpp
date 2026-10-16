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
#include <join/dtoa.hpp>
#include <join/sax.hpp>
#include <join/utf8.hpp>
#include <join/utf16.hpp>

// C++.
#include <functional>
#include <memory>
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
         * @param writer SAX writer owning the output.
         * @param indentation number of characters used to indent JSON.
         */
        BasicJsonWriter (SaxWriter<BasicJsonWriter>& writer, size_t indentation = 0)
        : _writer (writer)
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
            _writer.append ("null", 4);
            _first = false;
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
                _writer.append ("true", 4);
            }
            else
            {
                _writer.append ("false", 5);
            }
            _first = false;
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
            _first = false;
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
            _first = false;
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
            _first = false;
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
            _first = false;
            return 0;
        }

        /**
         * @brief set real value.
         * @param value real value to set.
         * @return 0 on success, -1 otherwise.
         */
        template <JsonWriteMode Mode = WriteMode>
        typename std::enable_if<Mode == JsonWriteMode::None, int>::type setDouble (double value)
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
                    _writer.append ("-NaN", 4);
                }
                else
                {
                    _writer.append ("NaN", 3);
                }
            }
            else
            {
                if (neg)
                {
                    _writer.append ("-Inf", 4);
                }
                else
                {
                    _writer.append ("Inf", 3);
                }
            }
            _first = false;
            return 0;
        }

        /**
         * @brief set real value in canonical form.
         * @param value real value to set.
         * @return 0 on success, -1 otherwise.
         */
        template <JsonWriteMode Mode = WriteMode>
        typename std::enable_if<Mode == JsonWriteMode::Canonical, int>::type setDouble (double value)
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
                _writer.append ("null", 4);
            }
            _first = false;
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
            _writer.append ('"');
            if (writeEscaped (value) == -1)
            {
                reset ();
                return -1;
            }
            _writer.append ('"');
            _first = false;
            return 0;
        }

        /**
         * @brief start array.
         * @param size array size (ignored).
         * @return 0 on success, -1 otherwise.
         */
        int startArray ([[maybe_unused]] uint32_t size)
        {
            if (JOIN_UNLIKELY (_depth == sizeof (_arrays) * 8))
            {
                join::lastError = make_error_code (SaxErrc::StackOverflow);
                reset ();
                return -1;
            }
            array ();
            _writer.append ('[');
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
            --_depth;
            if (!_first)
            {
                endLine ();
                indent ();
            }
            _writer.append (']');
            _first = false;
            return 0;
        }

        /**
         * @brief start object.
         * @param size array size (ignored).
         * @return 0 on success, -1 otherwise.
         */
        int startObject ([[maybe_unused]] uint32_t size)
        {
            if (JOIN_UNLIKELY (_depth == sizeof (_arrays) * 8))
            {
                join::lastError = make_error_code (SaxErrc::StackOverflow);
                reset ();
                return -1;
            }
            array ();
            _writer.append ('{');
            _arrays[_depth >> 6] &= ~(1ULL << (_depth & 63));
            ++_depth;
            _first = true;
            return 0;
        }

        /**
         * @brief get object members in writing order.
         * @param object object to write.
         * @return object members.
         */
        template <JsonWriteMode Mode = WriteMode>
        typename std::enable_if<Mode == JsonWriteMode::None, const Object&>::type members (
            const Object& object) const noexcept
        {
            return object;
        }

        /**
         * @brief get object members sorted by key in UTF-16 order.
         * @param object object to write.
         * @return sorted object members.
         */
        template <JsonWriteMode Mode = WriteMode>
        typename std::enable_if<Mode == JsonWriteMode::Canonical,
                                std::vector<std::reference_wrapper<const Member>>>::type
        members (const Object& object) const
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
            if (JOIN_UNLIKELY (!key.isString ()))
            {
                join::lastError = make_error_code (SaxErrc::InvalidKey);
                reset ();
                return -1;
            }
            comma ();
            endLine ();
            indent ();
            _writer.append ('"');
            if (writeEscaped (key.getString ()) == -1)
            {
                reset ();
                return -1;
            }
            _writer.append ('"');
            _writer.append (':');
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
            --_depth;
            if (!_first)
            {
                endLine ();
                indent ();
            }
            _writer.append ('}');
            _first = false;
            return 0;
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
                _writer.append ('-');
                writeUint64 (static_cast<uint64_t> (std::numeric_limits<int32_t>::max ()) + 1);
                return;
            }

            if (value < 0)
            {
                _writer.append ('-');
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
                _writer.append ('-');
                writeUint64 (static_cast<uint64_t> (std::numeric_limits<int64_t>::max ()) + 1);
                return;
            }

            if (value < 0)
            {
                _writer.append ('-');
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

            if (JOIN_UNLIKELY (_writer._capacity - _writer._size < 20))
            {
                _writer.flush ();
                if (_writer._capacity == 0)
                {
                    _writer._out->sputn (buffer + 20 - length, length);
                    return;
                }
            }

            std::memcpy (_writer._buffer + _writer._size, buffer + 20 - length, 20);
            _writer._size += length;
        }

        /**
         * @brief write real value.
         * @param value real value to write.
         */
        template <JsonWriteMode Mode = WriteMode>
        typename std::enable_if<Mode == JsonWriteMode::None>::type writeDouble (double value)
        {
            char buf[25];
            char* end = join::dtoa (buf, value);
            _writer.append (buf, end - buf);
        }

        /**
         * @brief write real value in canonical form.
         * @param value real value to write.
         */
        template <JsonWriteMode Mode = WriteMode>
        typename std::enable_if<Mode == JsonWriteMode::Canonical>::type writeDouble (double value) noexcept
        {
            char beg[25];
            char* end = join::dtoa (beg, value);
            for (char* pos = beg; pos < end; ++pos)
            {
                _writer.append (*pos);
                if ((*pos == 'e') && (*(pos + 1) != '-'))
                {
                    _writer.append ('+');
                }
            }
        }

        /**
         * @brief escape string value.
         * @param value string value to escape.
         * @return 0 on success, -1 otherwise.
         */
        template <JsonWriteMode Mode = WriteMode>
        typename std::enable_if<Mode == JsonWriteMode::None, int>::type writeEscaped (const std::string& value)
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
                    _writer.append (beg, cur - beg);
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
                    _writer.append (seq, 6);
                }
                else
                {
                    char escapeSeq[2] = {'\\', static_cast<char> (esc)};
                    _writer.append (escapeSeq, 2);
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
        typename std::enable_if<Mode == JsonWriteMode::Canonical, int>::type writeEscaped (const std::string& value)
        {
            if (JOIN_UNLIKELY (utf8::validate (value.data (), value.data () + value.size ()) == -1))
            {
                join::lastError = make_error_code (JsonErrc::InvalidEncoding);
                return -1;
            }
            return writeEscaped<JsonWriteMode::None> (value);
        }

        /**
         * @brief write comma.
         */
        inline void comma () noexcept
        {
            if (_depth && !_first)
            {
                _writer.append (',');
            }
        }

        /**
         * @brief write indentation.
         */
        inline void indent () noexcept
        {
            if (_indentation)
            {
                static constexpr char spaces[] = "                                                                ";
                size_t count = _depth * _indentation;
                while (count)
                {
                    size_t n = (count < sizeof (spaces) - 1) ? count : sizeof (spaces) - 1;
                    _writer.append (spaces, n);
                    count -= n;
                }
            }
        }

        /**
         * @brief write space.
         */
        inline void space () noexcept
        {
            if (_indentation)
            {
                _writer.append (' ');
            }
        }

        /**
         * @brief write end of line.
         */
        inline void endLine () noexcept
        {
            if (_indentation)
            {
                _writer.append ('\n');
            }
        }

        /**
         * @brief reset the writer state after an error.
         */
        inline void reset () noexcept
        {
            _depth = 0;
            _first = true;
        }

        /**
         * @brief add comma, go to line and indent if in array.
         */
        inline void array () noexcept
        {
            comma ();
            if (_depth && ((_arrays[(_depth - 1) >> 6] >> ((_depth - 1) & 63)) & 1))
            {
                endLine ();
                indent ();
            }
        }

        /// SAX writer owning the output.
        SaxWriter<BasicJsonWriter>& _writer;

        /// array flags of the open containers, one bit per depth.
        uint64_t _arrays[4] = {};

        /// number of open containers.
        size_t _depth = 0;

        /// indentation.
        size_t _indentation;

        /// is first element.
        bool _first = true;
    };

    /// JSON default writer.
    using JsonWriter = BasicJsonWriter<JsonWriteMode::None>;

    /// JSON canonicalizer.
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
         * @brief create instance.
         * @param reader SAX reader building the document.
         */
        BasicJsonReader (SaxReader<BasicJsonReader>& reader)
        : _reader (reader)
        {
        }

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
         * @brief parse a document.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType>
        int read (ViewType& document)
        {
            if (skipWhitespaces (document) != 0)
            {
                return -1;
            }

            if (readValue (document) != 0)
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

    protected:
        /**
         * @brief parse a JSON value.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType>
        int readValue (ViewType& document)
        {
            int ch = document.peek ();
            switch (ch)
            {
                case '[':
                    document.get ();
                    return readArray (document);
                case '{':
                    document.get ();
                    return readObject (document);
                case '"':
                    document.get ();
                    return readString (document);
                case 'n':
                    document.get ();
                    return readNull (document);
                case 't':
                    document.get ();
                    return readTrue (document);
                case 'f':
                    document.get ();
                    return readFalse (document);
                default:
                    return readNumber (document);
            }
        }

        /**
         * @brief parse a null value.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType>
        int readNull (ViewType& document)
        {
            if (JOIN_UNLIKELY ((document.get () != 'u') || (document.get () != 'l') || (document.get () != 'l')))
            {
                join::lastError = make_error_code (SaxErrc::InvalidValue);
                return -1;
            }

            return _reader.setNull ();
        }

        /**
         * @brief parse a true value.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType>
        int readTrue (ViewType& document)
        {
            if (JOIN_UNLIKELY ((document.get () != 'r') || (document.get () != 'u') || (document.get () != 'e')))
            {
                join::lastError = make_error_code (SaxErrc::InvalidValue);
                return -1;
            }

            return _reader.setBool (true);
        }

        /**
         * @brief parse a false value.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType>
        int readFalse (ViewType& document)
        {
            if (JOIN_UNLIKELY ((document.get () != 'a') || (document.get () != 'l') || (document.get () != 's') ||
                               (document.get () != 'e')))
            {
                join::lastError = make_error_code (SaxErrc::InvalidValue);
                return -1;
            }

            return _reader.setBool (false);
        }

        /**
         * @brief parse an infinity value.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType>
        int readInf (ViewType& document, bool negative)
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

            return _reader.setDouble (negative ? -std::numeric_limits<double>::infinity ()
                                               : std::numeric_limits<double>::infinity ());
        }

        /**
         * @brief parse a nan value.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType>
        int readNan (ViewType& document, bool negative)
        {
            if (JOIN_UNLIKELY (!(document.getIfNoCase ('a') && document.getIfNoCase ('n'))))
            {
                join::lastError = make_error_code (SaxErrc::InvalidValue);
                return -1;
            }

            return _reader.setDouble (negative ? -std::numeric_limits<double>::quiet_NaN ()
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

            if (JOIN_UNLIKELY ((exponent > 22) && (exponent < (22 + 16))))
            {
                value *= pow10[exponent - 22];
                exponent = 22;
            }

            if (JOIN_LIKELY ((exponent >= -22) && (exponent <= 22) && (value <= 9007199254740991.0)))
            {
                value = (exponent < 0) ? (value / pow10[-exponent]) : (value * pow10[exponent]);
                return true;
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

            bool roundUp = ((frac > 0x400) | ((frac == 0x400) && ((middle != 0) || (mant & 1))) |
                            ((frac == 0x3FF) && ((middle != 0))));

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
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType>
        int readNumber (ViewType& document)
        {
            BufferingView<ViewType> view (document);

            bool negative = view.getIf ('-');

            uint64_t max64 = std::numeric_limits<uint64_t>::max ();
            if (negative)
            {
                max64 = static_cast<uint64_t> (std::numeric_limits<int64_t>::max ()) + 1;
            }

            uint64_t digits = 0;
            bool isDouble = false;
            uint64_t u = 0;

            if (JOIN_UNLIKELY (view.getIf ('0')))
            {
                if (JOIN_UNLIKELY (isDigit (view.peek ())))
                {
                    join::lastError = make_error_code (SaxErrc::InvalidValue);
                    return -1;
                }
            }
            else if (JOIN_LIKELY (isDigit (view.peek ())))
            {
                digits = view.readDigits (u, 18);

                while (JOIN_LIKELY (isDigit (view.peek ())))
                {
                    int digit = view.peek () - '0';

                    if (JOIN_UNLIKELY (u > ((max64 - digit) / 10)))
                    {
                        isDouble = true;
                        break;
                    }

                    u = (u * 10) + (view.get () - '0');
                    ++digits;
                }
            }
            else if (JOIN_LIKELY (document.getIfNoCase ('i')))
            {
                return readInf (document, negative);
            }
            else if (JOIN_LIKELY (document.getIfNoCase ('n')))
            {
                return readNan (document, negative);
            }
            else
            {
                join::lastError = make_error_code (SaxErrc::InvalidValue);
                return -1;
            }

            if (isDouble)
            {
                digits += view.readDigits (u);
            }

            int64_t frac = 0;
            if (view.getIf ('.'))
            {
                isDouble = true;

                if (JOIN_UNLIKELY (digits == 0))
                {
                    while (view.getIf ('0'))
                    {
                        --frac;
                    }
                }

                size_t count = view.readDigits (u);
                digits += count;
                frac -= static_cast<int64_t> (count);
            }

            int64_t exponent = 0;
            if (view.getIf ('e') || view.getIf ('E'))
            {
                isDouble = true;

                bool negExp = false;
                if (isSign (view.peek ()))
                {
                    negExp = (view.get () == '-');
                }

                if (JOIN_LIKELY (isDigit (view.peek ())))
                {
                    exponent = (view.get () - '0');

                    while (JOIN_LIKELY (isDigit (view.peek ())))
                    {
                        int digit = view.get () - '0';

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
                return negative ? _reader.setInt64 (static_cast<int64_t> (-u)) : _reader.setUint64 (u);
            }

            if (JOIN_LIKELY (digits <= 19))
            {
                double d = 0.0;
                if (strtodFast (u, exponent + frac, d))
                {
                    return _reader.setDouble (negative ? -d : d);
                }
            }

            std::string number;
            view.snapshot (number);

            double d = 0.0;
            if (strtodSlow (number, d))
            {
                return _reader.setDouble (d);
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
        template <typename ViewType>
        inline int readHex (ViewType& document, uint32_t& u)
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
        template <typename ViewType>
        inline int readUnicode (ViewType& document, std::string& output)
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
        template <typename ViewType>
        inline int readEscaped (ViewType& document, std::string& output)
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
         * @param isKey indicate whether the element to parse is a key or not.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType>
        int readString (ViewType& document, bool isKey = false)
        {
            thread_local Value output (in_place_index_t<Value::String>{});
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

            return isKey ? _reader.setKey (Value (output)) : _reader.setString (std::string (output.getString ()));
        }

        /**
         * @brief parse an array value.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType>
        JOIN_ALWAYS_INLINE int readArray (ViewType& document)
        {
            if (JOIN_UNLIKELY (_reader.startArray () == -1))
            {
                return -1;
            }

            if (JOIN_UNLIKELY (skipWhitespaces (document) == -1))
            {
                return -1;
            }

            if (document.getIf (']'))
            {
                return _reader.stopArray ();
            }

            for (;;)
            {
                if (JOIN_UNLIKELY (readValue (document) == -1))
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

            return _reader.stopArray ();
        }

        /**
         * @brief parse an object value.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType>
        JOIN_ALWAYS_INLINE int readObject (ViewType& document)
        {
            if (JOIN_UNLIKELY (_reader.startObject () == -1))
            {
                return -1;
            }

            if (JOIN_UNLIKELY (skipWhitespaces (document) == -1))
            {
                return -1;
            }

            if (document.getIf ('}'))
            {
                return _reader.stopObject ();
            }

            for (;;)
            {
                if (JOIN_UNLIKELY (document.get () != '"'))
                {
                    join::lastError = make_error_code (JsonErrc::MissingQuote);
                    return -1;
                }

                if (JOIN_UNLIKELY (readString (document, true) == -1))
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

                if (JOIN_UNLIKELY (readValue (document) == -1))
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

            return _reader.stopObject ();
        }

        /**
         * @brief skip whitespaces.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType, JsonReadMode Mode = ReadMode>
        inline typename std::enable_if<!(Mode & JsonReadMode::ParseComments), int>::type skipWhitespaces (
            ViewType& document)
        {
            return document.skipWhitespaces ();
        }

        /**
         * @brief skip whitespaces with comment parsing.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType, JsonReadMode Mode = ReadMode>
        inline typename std::enable_if<(Mode & JsonReadMode::ParseComments), int>::type skipWhitespaces (
            ViewType& document)
        {
            return document.skipWhitespacesAndComments ();
        }

        /**
         * @brief validate the encoding of the last chunk read (validation disabled).
         * @param output parse output string.
         * @param pos position of the chunk in the output string.
         * @return 0.
         */
        template <JsonReadMode Mode = ReadMode>
        inline typename std::enable_if<!(Mode & JsonReadMode::ValidateEncoding), int>::type validateChunk (
            [[maybe_unused]] const std::string& output, [[maybe_unused]] size_t pos) noexcept
        {
            return 0;
        }

        /**
         * @brief validate the encoding of the last chunk read.
         * @param output parse output string.
         * @param pos position of the chunk in the output string.
         * @return 0 on success, -1 otherwise.
         */
        template <JsonReadMode Mode = ReadMode>
        inline typename std::enable_if<((Mode & JsonReadMode::ValidateEncoding) != 0), int>::type validateChunk (
            const std::string& output, size_t pos) noexcept
        {
            if (JOIN_UNLIKELY (utf8::validate (output.data () + pos, output.data () + output.size ()) == -1))
            {
                join::lastError = make_error_code (JsonErrc::InvalidEncoding);
                return -1;
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

        /// SAX reader building the document.
        SaxReader<BasicJsonReader>& _reader;
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
