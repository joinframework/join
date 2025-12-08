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

#ifndef __JOIN_JSON_HPP__
#define __JOIN_JSON_HPP__

// libjoin.
#include <join/atodpow.hpp>
#include <join/lookup.hpp>
#include <join/dtoa.hpp>
#include <join/sax.hpp>

// C++.
#include <codecvt>
#include <memory>
#include <locale>

namespace join
{
    /**
     * @brief JSON error codes.
     */
    enum class JsonErrc
    {
        InvalidComment = 1,     /**< comment is invalid. */
        InvalidEscaping,        /**< character escaping is invalid. */
        InvalidEncoding,        /**< character encoding is invalid. */
        IllegalCharacter,       /**< illegal character. */
        MissingCurlyBracket,    /**< missing curly bracket. */
        MissingSquareBracket,   /**< missing square bracket. */
        MissingQuote,           /**< missing quote. */
        MissingColon,           /**< missing colon. */
        MissingComma,           /**< missing comma. */
        EndOfFile,              /**< end of file. */
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
    const std::error_category& jsonCategory ();

    /**
     * @brief create an std::error_code object.
     * @param code error code number.
     * @return the created std::error_code object.
     */
    std::error_code make_error_code (JsonErrc code);

    /**
     * @brief create an std::error_condition object.
     * @param code error code number.
     * @return the created std::error_condition object.
     */
    std::error_condition make_error_condition (JsonErrc code);

    /**
     * @brief JSON writer class.
     */
    class JsonWriter : public StreamWriter
    {
    public:
        /**
         * @brief create instance.
         * @param document JSON document to create.
         * @param indentation number of characters used to indent JSON.
         */
        JsonWriter (std::ostream& document, size_t indentation = 0)
        : StreamWriter (document)
        , _indentation (indentation)
        {
        }

        /**
         * @brief copy constructor.
         * @param other object to copy.
         */
        JsonWriter (const JsonWriter& other) = delete;

        /**
         * @brief copy assignment.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        JsonWriter& operator= (const JsonWriter& other) = delete;

        /**
         * @brief move constructor.
         * @param other object to move.
         */
        JsonWriter (JsonWriter&& other) = delete;

        /**
         * @brief move assignment.
         * @param other object to move.
         * @return a reference of the current object.
         */
        JsonWriter& operator= (JsonWriter&& other) = delete;

        /**
         * @brief destroy instance.
         */
        virtual ~JsonWriter () = default;

        /**
         * @brief set null value.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setNull () override
        {
            array ();
            append ("null", 4);
            _first = false;
            return 0;
        }

        /**
         * @brief set boolean value.
         * @param value boolean value to set.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setBool (bool value) override
        {
            array ();
            append (value ? "true" : "false", value ? 4 : 5);
            _first = false;
            return 0;
        }

        /**
         * @brief set integer value.
         * @param value integer value to set.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setInt (int32_t value) override
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
        virtual int setUint (uint32_t value) override
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
        virtual int setInt64 (int64_t value) override
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
        virtual int setUint64 (uint64_t value) override
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
        virtual int setDouble (double value) override
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
                append (neg ? "-NaN" : "NaN", neg ? 4 : 3);
            }
            else
            {
                append (neg ? "-Inf" : "Inf", neg ? 4 : 3);
            }
            _first = false;
            return 0;
        }

        /**
         * @brief set string value.
         * @param value string value to set.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setString (const std::string& value) override
        {
            array ();
            append ('"');
            if (writeEscaped (value) == -1)
            {
                return -1;
            }
            append ('"');
            _first = false;
            return 0;
        }

        /**
         * @brief start array.
         * @param size array size (ignored).
         * @return 0 on success, -1 otherwise.
         */
        virtual int startArray ([[maybe_unused]] uint32_t size = 0) override
        {
            array ();
            append ('[');
            _tab.append (_indentation, ' ');
            _first = true;
            _stack.push (true);
            return 0;
        }

        /**
         * @brief stop array.
         * @return 0 on success, -1 otherwise.
         */
        virtual int stopArray () override
        {
            _tab.erase (_tab.size () - _indentation);
            if (!_first)
            {
                endLine ();
                indent ();
            }
            append (']');
            _first = false;
            _stack.pop ();
            return 0;
        }

        /**
         * @brief start object.
         * @param size array size (ignored).
         * @return 0 on success, -1 otherwise.
         */
        virtual int startObject ([[maybe_unused]] uint32_t size = 0) override
        {
            array ();
            append ('{');
            _tab.append (_indentation, ' ');
            _first = true;
            _stack.push (false);
            return 0;
        }

        /**
         * @brief set key.
         * @param key object key.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setKey (const std::string& key) override
        {
            comma ();
            endLine ();
            indent ();
            append ('"');
            if (writeEscaped (key) == -1)
            {
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
        virtual int stopObject () override
        {
            _tab.erase (_tab.size () - _indentation);
            if (!_first)
            {
                endLine ();
                indent ();
            }
            append ('}');
            _first = false;
            _stack.pop ();
            return 0;
        }

    protected:
        /**
         * @brief write integer value.
         * @param value integer value to write.
         */
        virtual void writeInt (int32_t value)
        {
            if (value == std::numeric_limits <int32_t>::min ())
            {
                append ('-');
                writeUint64 (static_cast <uint64_t> (std::numeric_limits <int32_t>::max ()) + 1);
                return;
            }

            if (value < 0)
            {
                append ('-');
                writeUint64 (static_cast <uint64_t> (-value));
                return;
            }

            writeUint64 (static_cast <uint64_t> (value));
        }

        /**
         * @brief write unsigned integer value.
         * @param value unsigned integer value to write.
         */
        virtual void writeUint (uint32_t value)
        {
            writeUint64 (static_cast <uint64_t> (value));
        }

        /**
         * @brief write 64 bits integer value.
         * @param value 64 bits integer value to write.
         */
        virtual void writeInt64 (int64_t value)
        {
            if (value == std::numeric_limits <int64_t>::min ())
            {
                append ('-');
                writeUint64 (static_cast <uint64_t> (std::numeric_limits <int64_t>::max ()) + 1);
                return;
            }

            if (value < 0)
            {
                append ('-');
                writeUint64 (static_cast <uint64_t> (-value));
                return;
            }

            writeUint64 (static_cast <uint64_t> (value));
        }

        /**
         * @brief write 64 bits unsigned integer value.
         * @param value 64 bits unsigned integer value to write.
         */
        virtual void writeUint64 (uint64_t value) noexcept
        {
            if (value == 0)
            {
                append ('0');
                return;
            }

            char buffer[20];
            char* ptr = buffer + 20;

            while (value >= 100)
            {
                uint64_t r = value % 100; 
                value /= 100;
                ptr -= 2;
                std::memcpy (ptr, &details::digitPairs[r * 2], 2);
            }

            if (value >= 10)
            {
                ptr -= 2;
                std::memcpy (ptr, &details::digitPairs[value * 2], 2);
            }
            else
            {
                *--ptr = '0' + static_cast <char> (value);
            }

            size_t length = (buffer + 20) - ptr;
            append (ptr, length);
        }

        /**
         * @brief write real value.
         * @param value real value to write.
         */
        virtual void writeDouble (double value) noexcept
        {
            char buf[25];
            char* end = join::dtoa (buf, value);
            append (buf, end - buf);
        }

        /**
         * @brief get UTF8 codepoint.
         * @param cur current character.
         * @param end end of string.
         * @param codepoint calculated UTF8 codepoint.
         * @return 0 on success, -1 otherwise.
         */
        virtual int utf8Codepoint (std::string::const_iterator& cur, std::string::const_iterator& end, uint32_t& codepoint) noexcept
        {
            uint8_t u0 = static_cast <uint8_t> (*cur);
            if (u0 < 0x80)
            {
                codepoint = u0;
                return 0;
            }

            if (++cur == end)
            {
                return -1;
            }

            uint8_t u1 = static_cast <uint8_t> (*cur);
            if (u0 < 0xE0)
            {
                codepoint = ((u0 & 0x1F) << 6) | (u1 & 0x3F);
                if (codepoint < 0x80)
                {
                    return -1;
                }
                return 0;
            }

            if (++cur == end)
            {
                return -1;
            }

            uint8_t u2 = static_cast <uint8_t> (*cur);
            if (u0 < 0xF0)
            {
                codepoint = ((u0 & 0x0F) << 12) | ((u1 & 0x3F) << 6) | (u2 & 0x3F);
                if ((codepoint > 0xD7FF) && (codepoint < 0xE000))
                {
                    return -1;
                }
                if (codepoint < 0x800)
                {
                    return -1;
                }
                return 0;
            }

            if (++cur == end)
            {
                return -1;
            }

            uint8_t u3 = static_cast <uint8_t> (*cur);
            if (u0 < 0xF8)
            {
                codepoint = ((u0 & 0x07) << 18) | ((u1 & 0x3F) << 12) | ((u2 & 0x3F) << 6) | (u3 & 0x3F);
                if (codepoint < 0x10000)
                {
                    return -1;
                }
                return 0;
            }

            return -1;
        }

        /**
         * @brief escape string value.
         * @param value string value to escape.
         * @return 0 on success, -1 otherwise.
         */
        virtual int writeEscaped(const std::string& value) noexcept
        {
            auto cur = value.cbegin ();
            auto end = value.cend ();

            while (cur != end)
            {
                auto beg = cur;

                while (cur != end && details::escapeLookup[static_cast <uint8_t> (*cur)] == 0)
                {
                    ++cur;
                }

                if (cur != beg)
                {
                    append (&(*beg), cur - beg);
                }

                if (cur == end)
                {
                    break;
                }

                uint8_t ch = static_cast <uint8_t> (*cur);
                uint8_t esc = details::escapeLookup[ch];
                if (esc == 'u')
                {
                    uint32_t codepoint = 0;
                    char hex[5];

                    if (utf8Codepoint (cur, end, codepoint) == -1)
                    {
                        join::lastError = make_error_code (JsonErrc::InvalidEncoding);
                        return -1;
                    }

                    if (codepoint <= 0xFFFF)
                    {
                        append ("\\u", 2);
                        snprintf (hex, sizeof (hex), "%04x", uint16_t (codepoint));
                        append (hex, 4);
                    }
                    else
                    {
                        codepoint -= 0x10000;
                        append ("\\u", 2);
                        snprintf (hex, sizeof (hex), "%04x", uint16_t (0xD800 + ((codepoint >> 10) & 0x3FF)));
                        append (hex, 4);
                        append ("\\u", 2);
                        snprintf (hex, sizeof (hex), "%04x", uint16_t (0xDC00 + (codepoint & 0x3FF)));
                        append (hex, 4);
                    }
                }
                else
                {
                    char escapeSeq[2] = {'\\', static_cast <char> (esc)};
                    append (escapeSeq, 2);
                }

                ++cur;
            }

            return 0;
        }

        /**
         * @brief write comma.
         */
        inline void comma () noexcept
        {
            if (!_stack.empty () && !_first)
            {
                append (',');
            }
        }

        /**
         * @brief write indentation.
         */
        inline void indent () noexcept
        {
            if (_indentation)
            {
                append (_tab.c_str (), _tab.size ());
            }
        }

        /**
         * @brief write space.
         */
        inline void space () noexcept
        {
            if (_indentation)
            {
                append (' ');
            }
        }

        /**
         * @brief write end of line.
         */
        inline void endLine () noexcept
        {
            if (_indentation)
            {
                append ('\n');
            }
        }

        /**
         * @brief add comma, go to line and indent if in array.
         */
        inline void array () noexcept
        {
            comma ();
            if (!_stack.empty () && _stack.top ())
            {
                endLine ();
                indent ();
            }
        }

        /// array stack.
        std::stack <bool> _stack;

        /// indentation.
        size_t _indentation;

        /// tabulation.
        std::string _tab;

        /// is first element.
        bool _first = true;
    };

    /**
     * @brief JSON canonicalizer class.
     */
    class JsonCanonicalizer : public JsonWriter
    {
    public:
        /**
         * @brief create instance.
         * @param document JSON document to create.
         */
        JsonCanonicalizer (std::ostream& document)
        : JsonWriter (document, 0)
        {
        }

        /**
         * @brief copy constructor.
         * @param other object to copy.
         */
        JsonCanonicalizer (const JsonCanonicalizer& other) = delete;

        /**
         * @brief copy assignment.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        JsonCanonicalizer& operator= (const JsonCanonicalizer& other) = delete;

        /**
         * @brief move constructor.
         * @param other object to move.
         */
        JsonCanonicalizer (JsonCanonicalizer&& other) = delete;

        /**
         * @brief move assignment.
         * @param other object to move.
         * @return a reference of the current object.
         */
        JsonCanonicalizer& operator= (JsonCanonicalizer&& other) = delete;

        /**
         * @brief destroy instance.
         */
        virtual ~JsonCanonicalizer () = default;

        /**
         * @brief set real value.
         * @param value real value to set.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setDouble (double value) override
        {
            array ();
            if (std::isfinite (value))
            {
                if ((std::trunc (value) == value) &&
                    (value >= 0) &&
                    (value < static_cast <double> (std::numeric_limits <uint64_t>::max ())))
                {
                    writeUint64 (static_cast <uint64_t> (value));
                }
                else if ((std::trunc (value) == value) &&
                         (value >= static_cast <double> (std::numeric_limits <int64_t>::min ())) &&
                         (value <  static_cast <double> (std::numeric_limits <int64_t>::max ())))
                {
                    writeInt64 (static_cast <int64_t> (value));
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
            _first = false;
            return 0;
        }

    protected:
        /**
         * @brief set object value.
         * @param value array value to set.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setObject (const Object& object) override
        {
            startObject (object.size ());
            std::vector <const Member *> members;
            std::transform (object.begin (), object.end (), std::back_inserter (members), [] (const Member &member) {return &member;});
            std::sort (members.begin (), members.end (), [] (const Member *a, const Member *b) {
                std::wstring_convert <std::codecvt_utf8_utf16 <char16_t>, char16_t> cvt_utf8_utf16;
                std::u16string wa = cvt_utf8_utf16.from_bytes (a->first.data ());
                std::u16string wb = cvt_utf8_utf16.from_bytes (b->first.data ());
                return wa < wb;
            });
            for (auto const& member : members)
            {
                setKey (member->first);
                serialize (member->second);
            }
            stopObject ();
            return 0;
        }

        /**
         * @brief write real value.
         * @param value real value to write.
         */
        virtual void writeDouble (double value) noexcept override
        {
            char beg[25];
            char* end = join::dtoa (beg, value);
            for (char* pos = beg; pos < end; ++pos)
            {
                append (*pos);
                if ((*pos == 'e') && (*(pos + 1) != '-'))
                {
                    append ('+');
                }
            }
        }
    };

    struct LocaleDelete
    {
        constexpr LocaleDelete () noexcept = default;

        void operator () (locale_t loc) noexcept
        {
            freelocale (loc);
        }
    };

    using LocalePtr = std::unique_ptr <std::remove_pointer_t <locale_t>, LocaleDelete>;

    /**
     * @brief JSON deserialization mode.
     */
    enum JsonReadMode
    {
        None                = 0,       /**< no read mode set. */
        ParseComments       = 1L << 0, /**< parse comments. */
        ValidateEncoding    = 1L << 1, /**< validate encoding. */
        StopParsingOnDone   = 1L << 2, /**< stop parsing on done. */
    };

    /**
     * @brief perform binary AND on JsonReadMode.
     * @param a bitset.
     * @param b other bitset.
     * @return bitset result of binary AND on JsonReadMode.
     */
    inline constexpr JsonReadMode operator& (JsonReadMode a, JsonReadMode b) noexcept
    { return JsonReadMode (static_cast <int> (a) & static_cast <int> (b)); }

    /**
     * @brief perform binary OR on JsonReadMode.
     * @param a bitset.
     * @param b other bitset.
     * @return bitset result of binary OR on JsonReadMode.
     */
    inline constexpr JsonReadMode operator| (JsonReadMode a, JsonReadMode b) noexcept
    { return JsonReadMode (static_cast <int> (a) | static_cast <int> (b)); }

    /**
     * @brief perform binary AND on JsonReadMode.
     * @param a bitset.
     * @param b other bitset.
     * @return bitset result of binary AND on JsonReadMode.
     */
    inline constexpr const JsonReadMode& operator&= (JsonReadMode& a, JsonReadMode b) noexcept
    { return a = a & b; }

    /**
     * @brief perform binary OR on JsonReadMode.
     * @param a bitset.
     * @param b other bitset.
     * @return bitset result of binary OR.
     */
    inline constexpr const JsonReadMode& operator|= (JsonReadMode& a, JsonReadMode b) noexcept
    { return a = a | b; }

    /**
     * @brief JSON reader class.
     */
    class JsonReader : public StreamReader
    {
    public:
        /**
         * @brief default constructor.
         * @param root Value to write.
         */
        JsonReader (Value& root)
        : StreamReader (root)
        {
        }

        /**
         * @brief copy constructor.
         * @param other object to copy.
         */
        JsonReader (const JsonReader& other) = delete;

        /**
         * @brief copy assignment.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        JsonReader& operator= (const JsonReader& other) = delete;

        /**
         * @brief move constructor.
         * @param other object to move.
         */
        JsonReader (JsonReader&& other) = delete;

        /**
         * @brief move assignment.
         * @param other object to move.
         * @return a reference of the current object.
         */
        JsonReader& operator= (JsonReader&& other) = delete;

        /**
         * @brief destroy instance.
         */
        virtual ~JsonReader () = default;

        /**
         * @brief Deserialize a document.
         * @param document document to parse.
         * @param length The length of the document to parse.
         * @return 0 on success, -1 otherwise.
         */
        template <JsonReadMode ReadMode = JsonReadMode::None>
        int deserialize (const char* document, size_t length)
        {
            StringView in (document, length);
            return read <ReadMode> (in);
        }

        /**
         * @brief Deserialize a document.
         * @param document document to parse.
         * @param length The length of the document to parse.
         * @return 0 on success, -1 otherwise.
         */
        int deserialize (const char* document, size_t length) override
        {
            return deserialize <> (document, length);
        }

        /**
         * @brief Deserialize a document.
         * @param first The first character of the document to parse.
         * @param last The last character of the document to parse.
         * @return 0 on success, -1 otherwise.
         */
        template <JsonReadMode ReadMode = JsonReadMode::None>
        int deserialize (const char* first, const char* last)
        {
            StringView in (first, last);
            return read <ReadMode> (in);
        }

        /**
         * @brief Deserialize a document.
         * @param first The first character of the document to parse.
         * @param last The last character of the document to parse.
         * @return 0 on success, -1 otherwise.
         */
        int deserialize (const char* first, const char* last) override
        {
            return deserialize <> (first, last);
        }

        /**
         * @brief Deserialize a document.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        template <JsonReadMode ReadMode = JsonReadMode::None>
        int deserialize (const std::string& document)
        {
            StringView in (document.c_str (), document.size ());
            return read <ReadMode> (in);
        }

        /**
         * @brief Deserialize a document.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        int deserialize (const std::string& document) override
        {
            return deserialize <> (document);
        }

        /**
         * @brief Parse a document.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        template <JsonReadMode ReadMode = JsonReadMode::None>
        int deserialize (std::istream& document)
        {
            StreamView in (document);
            return read <ReadMode> (in);
        }

        /**
         * @brief Parse a document.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        int deserialize (std::istream& document) override
        {
            return deserialize <> (document);
        }

    protected:
        /**
         * @brief parse a document.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        template <JsonReadMode ReadMode, typename ViewType>
        int read (ViewType& document)
        {
            if (readValue <ReadMode> (document) != 0)
            {
                return -1;
            }

            if (ReadMode & JsonReadMode::StopParsingOnDone)
            {
                return 0;
            }

            skipWhitespaces (document);

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
         * @return 0 on success, -1 otherwise.
         */
        template <JsonReadMode ReadMode, typename ViewType>
        int readValue (ViewType& document)
        {
            if (skipComments <ReadMode> (document) != 0)
            {
                return -1;
            }

            int ch = document.peek ();
            switch (ch)
            {
                case '[':
                    document.get ();
                    return readArray <ReadMode> (document);
                case '{':
                    document.get ();
                    return readObject <ReadMode> (document);
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

            return setNull ();
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

            return setBool (true);
        }

        /**
         * @brief parse a false value.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType>
        int readFalse (ViewType& document)
        {
            if (JOIN_UNLIKELY ((document.get () != 'a') || (document.get () != 'l') || (document.get () != 's') || (document.get () != 'e')))
            {
                join::lastError = make_error_code (SaxErrc::InvalidValue);
                return -1;
            }

            return setBool (false);
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

            if (JOIN_UNLIKELY (document.getIfNoCase ('i') && !(document.getIfNoCase ('n') && document.getIfNoCase ('i') && document.getIfNoCase ('t') && document.getIfNoCase ('y'))))
            {
                join::lastError = make_error_code (SaxErrc::InvalidValue);
                return -1;
            }

            return setDouble (negative ? -std::numeric_limits <double>::infinity () : std::numeric_limits <double>::infinity ());
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

            return setDouble (negative ? -std::numeric_limits <double>::quiet_NaN () : std::numeric_limits <double>::quiet_NaN ());
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
        inline void umul192 (uint64_t hi, uint64_t lo, uint64_t significand, uint64_t& high, uint64_t& middle, uint64_t& low) noexcept
        {
        #if defined(__SIZEOF_INT128__)
            __uint128_t h = static_cast <__uint128_t> (hi) * significand;
            __uint128_t l = static_cast <__uint128_t> (lo) * significand;
            __uint128_t s = h + (l >> 64);

            high = static_cast <uint64_t> (s >> 64);
            middle = static_cast <uint64_t> (s);
            low = static_cast <uint64_t> (l);
        #else
            uint64_t hi_hi, hi_lo, lo_hi, lo_lo;

            uint64_t m_lo = static_cast <uint32_t> (significand);
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
        inline bool strtodFast (uint64_t significand, int64_t exponent, double &value) noexcept
        {
            value = static_cast <double> (significand);

            if (JOIN_UNLIKELY ((exponent > 22) && (exponent < (22 + 16))))
            {
                value *= details::pow10[exponent - 22];
                exponent = 22;
            }

            if (JOIN_LIKELY ((exponent >= -22) && (exponent <= 22) && (value <= 9007199254740991.0)))
            {
                value = (exponent < 0) ? (value / details::pow10[-exponent]) : (value * details::pow10[exponent]);
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
                return false;
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
            uint64_t bits = (static_cast <uint64_t> (exp) << 52) | mant;
            uint64_t frac = high & 0x7FF;

            bool roundUp = ((frac >  0x400) |
                           ((frac == 0x400) && ((middle != 0) || (mant & 1))) |
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
        int readNumber (ViewType& document) noexcept
        {
            size_t beg = document.tell ();
            bool negative = document.getIf ('-');

            uint64_t max64 = std::numeric_limits <uint64_t>::max ();
            if (negative)
            {
                max64 = static_cast <uint64_t> (std::numeric_limits <int64_t>::max ()) + 1;
            }

            uint64_t digits = 0;
            bool isDouble = false;
            uint64_t u = 0;

            if (JOIN_UNLIKELY (document.getIf ('0')))
            {
                if (JOIN_UNLIKELY (isDigit (document.peek ())))
                {
                    join::lastError = make_error_code (SaxErrc::InvalidValue);
                    return -1;
                }
            }
            else if (JOIN_LIKELY (isDigit (document.peek ())))
            {
                u = document.get () - '0';
                ++digits;

                while (JOIN_LIKELY (isDigit (document.peek ())))
                {
                    int digit = document.peek () - '0';

                    if (JOIN_UNLIKELY (u > ((max64 - digit) / 10)))
                    {
                        isDouble = true;
                        break;
                    }

                    u = (u * 10) + (document.get () - '0');
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
                while (JOIN_LIKELY (isDigit (document.peek ())))
                {
                    u = (u * 10) + (document.get () - '0');
                    ++digits;
                }
            }

            int64_t frac = 0;
            if (document.getIf ('.'))
            {
                isDouble = true;

                while (JOIN_LIKELY (isDigit (document.peek ())))
                {
                    u = (u * 10) + (document.get () - '0');
                    if (JOIN_LIKELY (u || digits))
                    {
                        ++digits;
                    }
                    --frac;
                }
            }

            int64_t exponent = 0;
            if (document.getIf ('e') || document.getIf ('E'))
            {
                isDouble = true;

                bool negExp = false;
                if (isSign (document.peek ()))
                {
                    negExp = (document.get () == '-');
                }

                if (JOIN_LIKELY (isDigit (document.peek ())))
                {
                    exponent = (document.get () - '0');

                    while (JOIN_LIKELY (isDigit (document.peek ())))
                    {
                        int digit = document.get () - '0';

                        if (JOIN_LIKELY (exponent <= ((std::numeric_limits <int>::max () - digit) / 10)))
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
                return negative ? setInt64 (-static_cast <int64_t> (u)) : setUint64 (u);
            }

            if (JOIN_LIKELY (digits <= 19))
            {
                double d = 0.0;
                if (strtodFast (u, exponent + frac, d))
                {
                    return setDouble (negative ? -d : d);
                }
            }

            size_t len = document.tell () - beg;

            std::string number;
            number.resize (len);

            document.rewind (len);
            document.read (&number[0], len);

            double d = 0.0;
            if (strtodSlow (number, d))
            {
                return setDouble (d);
            }

            join::lastError = make_error_code (SaxErrc::InvalidValue);
            return -1;
        }

        /**
         * @brief parse a 4-digit hexadecimal sequence..
         * @param document document to parse.
         * @param u output variable to store the parsed hexadecimal value.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType>
        inline int readHex (ViewType& document, uint32_t& u) noexcept
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
         * @brief encode a Unicode codepoint to UTF-8.
         * @param codepoint unicode codepoint to encode.
         * @param output parse output string.
         */
        inline void encodeUtf8 (uint32_t codepoint, std::string& output) noexcept
        {
            if (codepoint < 0x80)
            {
                output.push_back (static_cast <char> (codepoint));
            }
            else if (codepoint < 0x800)
            {
                char buf[2];
                buf[0] = static_cast <char> (0xC0 | (codepoint >> 6));
                buf[1] = static_cast <char> (0x80 | (codepoint & 0x3F));
                output.append (buf, 2);
            }
            else if (codepoint < 0x10000)
            {
                char buf[3];
                buf[0] = static_cast <char> (0xE0 | (codepoint >> 12));
                buf[1] = static_cast <char> (0x80 | ((codepoint >> 6) & 0x3F));
                buf[2] = static_cast <char> (0x80 | (codepoint & 0x3F));
                output.append (buf, 3);
            }
            else
            {
                char buf[4];
                buf[0] = static_cast <char> (0xF0 | (codepoint >> 18));
                buf[1] = static_cast <char> (0x80 | ((codepoint >> 12) & 0x3F));
                buf[2] = static_cast <char> (0x80 | ((codepoint >> 6) & 0x3F));
                buf[3] = static_cast <char> (0x80 | (codepoint & 0x3F));
                output.append (buf, 4);
            }
        }

        /**
         * @brief parse unicode.
         * @param document document to parse.
         * @param output parse output string.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType>
        inline int readUnicode (ViewType& document, std::string& output) noexcept
        {
            uint32_t u = 0;

            if (readHex (document, u) == -1)
            {
                return -1;
            }

            if (u >= 0xDC00 && u <= 0xDFFF)
            {
                join::lastError = make_error_code (JsonErrc::InvalidEncoding);
                return -1;
            }

            if (u >= 0xD800 && u <= 0xDBFF)
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

                if (v < 0xDC00 || v > 0xDFFF)
                {
                    join::lastError = make_error_code (JsonErrc::InvalidEncoding);
                    return -1;
                }

                u = 0x10000 + (((u - 0xD800) << 10) | (v - 0xDC00));
            }

            if (u > 0x10FFFF)
            {
                join::lastError = make_error_code (JsonErrc::InvalidEncoding);
                return -1;
            }

            encodeUtf8 (u, output);

            return 0;
        }

        /**
         * @brief parse escaped sequence.
         * @param document document to parse.
         * @param output parse output string.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType>
        inline int readEscaped (ViewType& document, std::string& output) noexcept
        {
            if (JOIN_UNLIKELY (!document.getIf ('\\')))
            {
                join::lastError = make_error_code (JsonErrc::InvalidEscaping);
                return -1;
            }

            int ch = document.get ();
            switch (ch)
            {
                case '"':  output.push_back ('"');  break;
                case '\\': output.push_back ('\\'); break;
                case '/':  output.push_back ('/');  break;
                case 'b':  output.push_back ('\b'); break;
                case 'f':  output.push_back ('\f'); break;
                case 'n':  output.push_back ('\n'); break;
                case 'r':  output.push_back ('\r'); break;
                case 't':  output.push_back ('\t'); break;
                case 'u': 
                    return readUnicode (document, output);
                default:
                    join::lastError = make_error_code (JsonErrc::InvalidEscaping);
                    return -1;
            }

            return 0;
        }

        /**
         * @brief parse UTF8.
         * @param document document to parse.
         * @param output parse output string.
         * @return 0 on success, -1 otherwise.
         */
        /*template <typename ViewType>
        inline int readUtf8 (ViewType& document, std::string& output) noexcept
        {
            size_t count = 0;

            if (static_cast <uint8_t> (document.peek ()) < 0x80)
            {
                output.push_back (document.get ());
            }
            else if (static_cast <uint8_t> (document.peek ()) < 0xE0)
            {
                output.push_back (document.get ());
                count = 1;
            }
            else if (static_cast <uint8_t> (document.peek ()) < 0xF0)
            {
                output.push_back (document.get ());
                count = 2;
            }
            else if (static_cast <uint8_t> (document.peek ()) < 0xF8)
            {
                output.push_back (document.get ());
                count = 3;
            }
            else
            {
                join::lastError = make_error_code (JsonErrc::InvalidEncoding);
                return -1;
            }

            for (size_t i = 0; i < count; ++i)
            {
                if (static_cast <uint8_t> (document.peek ()) < 0x80 || static_cast <uint8_t> (document.peek ()) > 0xBF)
                {
                    join::lastError = make_error_code (JsonErrc::InvalidEncoding);
                    return -1;
                }

                output.push_back (document.get ());
            }

            return 0;
        }*/

        /**
         * @brief parse a string value.
         * @param document document to parse.
         * @param isKey indicate whether the string to parse is a key or not.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType>
        int readString (ViewType& document, bool isKey = false)
        {
            thread_local std::string output;

            output.clear ();
            output.reserve (64);

            char buffer[256];

            for (;;)
            {
                size_t bufIdx = 0;

                while (bufIdx < 256)
                {
                    int ch = document.peek ();
                    if (JOIN_UNLIKELY (ch == std::char_traits <char>::eof ()))
                    {
                        join::lastError = make_error_code (JsonErrc::EndOfFile);
                        return -1;
                    }

                    uint8_t uch = static_cast <uint8_t> (ch);
                    if (JOIN_UNLIKELY (uch < 0x20 || uch == 0x22 || uch == 0x5C))
                    {
                        break;
                    }
                    document.get ();
                    buffer[bufIdx++] = static_cast <char> (ch);
                }

                if (bufIdx > 0)
                {
                    output.append (buffer, bufIdx);
                }

                int ch = document.peek ();

                if (JOIN_LIKELY (ch == '"'))
                {
                    document.get ();
                    break;
                }
                else if (JOIN_UNLIKELY (ch == '\\'))
                {
                    if (readEscaped (document, output) == -1)
                    {
                        return -1;
                    }
                }
                /*else if (JOIN_UNLIKELY (static_cast <uint8_t> (ch) > 0x7F))
                {
                    if (readUtf8 (document, output) == -1)
                    {
                        return -1;
                    }
                }*/
                else if (JOIN_UNLIKELY (ch < 0x20))
                {
                    join::lastError = make_error_code (JsonErrc::IllegalCharacter);
                    return -1;
                }
                else if (JOIN_UNLIKELY (ch == std::char_traits <char>::eof ()))
                {
                    join::lastError = make_error_code (JsonErrc::EndOfFile);
                    return -1;
                }
            }
            
            return isKey ? setKey (output) : setString (output);
        }

        /**
         * @brief parse an array value.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        template <JsonReadMode ReadMode, typename ViewType>
        int readArray (ViewType& document)
        {
            if (JOIN_UNLIKELY (startArray () == -1))
            {
                return -1;
            }

            if (JOIN_UNLIKELY (skipComments <ReadMode> (document) == -1))
            {
                return -1;
            }

            if (document.getIf (']'))
            {
                return stopArray ();
            }

            for (;;)
            {
                if (JOIN_UNLIKELY (readValue <ReadMode> (document) == -1))
                {
                    return -1;
                }

                if (JOIN_UNLIKELY (skipComments <ReadMode> (document) == -1))
                {
                    return -1;
                }

                int ch = document.peek ();

                if (JOIN_LIKELY (ch == ',' || ch == ']'))
                {
                    document.get ();

                    if (ch == ']')
                    {
                        break;
                    }

                    if (JOIN_UNLIKELY (skipComments <ReadMode> (document) == -1))
                    {
                        return -1;
                    }
                }
                else
                {
                    join::lastError = make_error_code (JsonErrc::MissingComma);
                    return -1;
                }
            }

            return stopArray ();
        }

        /**
         * @brief parse an object value.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        template <JsonReadMode ReadMode, typename ViewType>
        int readObject (ViewType& document)
        {
            if (JOIN_UNLIKELY (startObject () == -1))
            {
                return -1;
            }

            if (JOIN_UNLIKELY (skipComments <ReadMode> (document) == -1))
            {
                return -1;
            }

            if (document.getIf ('}'))
            {
                return stopObject ();
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

                if (JOIN_UNLIKELY (skipComments <ReadMode> (document) == -1))
                {
                    return -1;
                }

                if (JOIN_UNLIKELY (document.get () != ':'))
                {
                    join::lastError = make_error_code (JsonErrc::MissingColon);
                    return -1;
                }

                if (JOIN_UNLIKELY (skipComments <ReadMode> (document) == -1))
                {
                    return -1;
                }

                if (JOIN_UNLIKELY (readValue <ReadMode> (document) == -1))
                {
                    return -1;
                }

                if (JOIN_UNLIKELY (skipComments <ReadMode> (document) == -1))
                {
                    return -1;
                }

                int ch = document.peek ();

                if (JOIN_LIKELY (ch == ',' || ch == '}'))
                {
                    document.get ();

                    if (ch == '}')
                    {
                        break;
                    }

                    if (JOIN_UNLIKELY (skipComments <ReadMode> (document) == -1))
                    {
                        return -1;
                    }
                }
                else
                {
                    join::lastError = make_error_code (JsonErrc::MissingComma);
                    return -1;
                }
            }

            return stopObject ();
        }

        /**
         * @brief check if whitespace.
         * @param c character to check.
         * @return true if whitespace, false otherwise.
         */
        inline constexpr bool isWhitespace (char c) noexcept
        {
            return details::whitespaceLookup[static_cast <unsigned char> (c)];
        }

        /**
         * @brief skip whitespaces.
         * @param document document to parse.
         */
        template <typename ViewType>
        inline constexpr void skipWhitespaces (ViewType& document) noexcept
        {
            while (isWhitespace (document.peek ()))
            {
                document.get ();
            }
        }

        /**
         * @brief skip comments.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        template <JsonReadMode ReadMode, typename ViewType>
        inline constexpr int skipComments (ViewType& document) noexcept
        {
            if (!(ReadMode & JsonReadMode::ParseComments))
            {
                skipWhitespaces (document);
                return 0;
            }

            skipWhitespaces (document);

            while (JOIN_UNLIKELY (document.getIf ('/')))
            {
                if (document.getIf ('*'))
                {
                    while ((document.get () != '*') || (document.get () != '/'))
                    {
                        // ignore comment.
                    }
                }
                else if (JOIN_LIKELY (document.getIf ('/')))
                {
                    while (document.get () != '\n')
                    {
                        // ignore comment.
                    }
                }
                else
                {
                    join::lastError = make_error_code (JsonErrc::InvalidComment);
                    return -1;
                }

                skipWhitespaces (document);
            }

            return 0;
        }

        /**
         * @brief check if upper case alphanumeric character.
         * @param c character to check.
         * @return true if upper case alphanumeric character, false otherwise.
         */
        inline constexpr bool isUpperAlpha (char c) noexcept
        {
            return static_cast <unsigned char> (c - 'A') <= 5u;
        }

        /**
         * @brief check if lower case alphanumeric character.
         * @param c character to check.
         * @return true if lower case alphanumeric character, false otherwise.
         */
        inline constexpr bool isLowerAlpha (char c) noexcept
        {
            return static_cast <unsigned char> (c - 'a') <= 5u;
        }

        /**
         * @brief check if digit.
         * @param c character to check.
         * @return true if digit, false otherwise.
         */
        inline constexpr bool isDigit (char c) noexcept
        {
            return static_cast <unsigned char> (c - '0') <= 9u;
        }

        /**
         * @brief check if sign.
         * @param c character to check.
         * @return true if sign, false otherwise.
         */
        inline constexpr bool isSign (char c) noexcept
        {
            return ((c ^ '+') & (c ^ '-')) == 0;
        }
    };
}

namespace std
{
    /// JSON error code specialization.
    template <> struct is_error_condition_enum <join::JsonErrc> : public true_type {};
}

#endif
