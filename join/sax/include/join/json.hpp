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
#include <join/dtoa.hpp>
#include <join/sax.hpp>

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
        virtual const char* name () const noexcept
        {
            return "join";
        }

        /**
         * @brief translate JSON error code to human readable error string.
         * @param code error code.
         * @return human readable error string.
         */
        virtual std::string message (int code) const
        {
            switch (static_cast <JsonErrc> (code))
            {
                case JsonErrc::InvalidComment:
                    return "comment is invalid";
                case JsonErrc::InvalidEscaping:
                    return "character escaping is invalid";
                case JsonErrc::InvalidEncoding:
                    return "character encoding is invalid";
                case JsonErrc::IllegalCharacter:
                    return "illegal character";
                case JsonErrc::MissingCurlyBracket:
                    return "missing curly bracket";
                case JsonErrc::MissingSquareBracket:
                    return "missing square bracket";
                case JsonErrc::MissingQuote:
                    return "missing quote";
                case JsonErrc::MissingColon:
                    return "missing colon";
                case JsonErrc::MissingComma:
                    return "missing comma";
                case JsonErrc::EndOfFile:
                    return "end of file";
                default:
                    return "success";
            }
        }
    };

    /**
     * @brief get error category.
     * @return the created std::error_category object.
     */
    const std::error_category& jsonCategory ()
    {
        static JsonCategory instance;
        return instance;
    }

    /**
     * @brief create an std::error_code object.
     * @param code error code number.
     * @return the created std::error_code object.
     */
    std::error_code make_error_code (JsonErrc code)
    {
        return std::error_code (static_cast <int> (code), jsonCategory ());
    }

    /**
     * @brief create an std::error_condition object.
     * @param code error code number.
     * @return the created std::error_condition object.
     */
    std::error_condition make_error_condition (JsonErrc code)
    {
        return std::error_condition (static_cast <int> (code), jsonCategory ());
    }

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
        : StreamWriter (document),
          _indentation (indentation)
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
            if (value)
            {
                append ("true", 4);
            }
            else
            {
                append ("false", 5);
            }
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
            if (std::isfinite (value))
            {
                writeDouble (value);
            }
            else if (std::isnan (value))
            {
                if (std::signbit (value))
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
                if (std::signbit (value))
                {
                    append ("-Inf", 4);
                }
                else
                {
                    append ("Inf", 3);
                }
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

    private:
        /**
         * @brief write integer value.
         * @param value integer value to write.
         */
        void writeInt (int32_t value)
        {
            if (value == std::numeric_limits <int32_t>::min ())
            {
                append ('-');
                writeUint64 (static_cast <uint64_t> (std::numeric_limits <int32_t>::max ()) + 1);
            }
            else if (value < 0)
            {
                append ('-');
                writeUint64 (-value);
            }
            else
            {
                writeInt64 (value);
            }
        }

        /**
         * @brief write unsigned integer value.
         * @param value unsigned integer value to write.
         */
        void writeUint (uint32_t value)
        {
            writeUint64 (static_cast <uint64_t> (value));
        }

        /**
         * @brief write 64 bits integer value.
         * @param value 64 bits integer value to write.
         */
        void writeInt64 (int64_t value)
        {
            if (value == std::numeric_limits <int64_t>::min ())
            {
                append ('-');
                writeUint64 (static_cast <uint64_t> (std::numeric_limits <int64_t>::max ()) + 1);
            }
            else if (value < 0)
            {
                append ('-');
                writeUint64 (-value);
            }
            else
            {
                writeUint64 (value);
            }
        }

        /**
         * @brief write 64 bits unsigned integer value.
         * @param value 64 bits unsigned integer value to write.
         */
        void writeUint64 (uint64_t value)
        {
            std::stack <char> stack;
            while (value)
            {
                stack.push ((value % 10) + '0');
                value /= 10;
            }
            if (stack.empty ())
            {
                append ('0');
            }
            while (!stack.empty ())
            {
                append (stack.top ());
                stack.pop ();
            }
        }

        /**
         * @brief write real value.
         * @param value real value to write.
         */
        void writeDouble (double value)
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
        int utf8Codepoint (std::string::const_iterator& cur, std::string::const_iterator& end, uint32_t& codepoint)
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
        int writeEscaped (const std::string& value)
        {
            auto cur = value.cbegin ();
            auto end = value.cend ();

            while (cur != end)
            {
                uint8_t ch = static_cast <uint8_t> (*cur);

                if (ch == '\"')
                {
                    append ("\\\"", 2);
                }
                else if (ch == '\\')
                {
                    append ("\\\\", 2);
                }
                else if (ch == '\b')
                {
                    append ("\\b", 2);
                }
                else if (ch == '\f')
                {
                    append ("\\f", 2);
                }
                else if (ch == '\n')
                {
                    append ("\\n", 2);
                }
                else if (ch == '\r')
                {
                    append ("\\r", 2);
                }
                else if (ch == '\t')
                {
                    append ("\\t", 2);
                }
                else if ((ch < 0x20) /*|| (ch > 0x7F)*/)
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
                    append (*cur);
                }

                ++cur;
            }

            return 0;
        }

        /**
         * @brief write comma.
         */
        void comma ()
        {
            if (!_stack.empty () && !_first)
            {
                append (',');
            }
        }

        /**
         * @brief write indentation.
         */
        void indent ()
        {
            if (_indentation)
            {
                append (_tab.c_str (), _tab.size ());
            }
        }

        /**
         * @brief write space.
         */
        void space ()
        {
            if (_indentation)
            {
                append (' ');
            }
        }

        /**
         * @brief write end of line.
         */
        void endLine ()
        {
            if (_indentation)
            {
                append ('\n');
            }
        }

        /**
         * @brief add comma, go to line and indent if in array.
         */
        void array ()
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
     * @brief JSON parsing mode.
     */
    enum JsonReadMode
    {
        None,                           /**< no read mode set. */
        ParseComments       = 1L << 0,  /**< parse comments. */
        ValidateEncoding    = 1L << 1,  /**< validate encoding. */
    };

    /**
     * @brief perform binary AND on JsonReadMode.
     * @param a bitset.
     * @param b other bitset.
     * @return bitset result of binary AND on JsonReadMode.
     */
    constexpr JsonReadMode operator& (JsonReadMode a, JsonReadMode b)
    { return JsonReadMode (static_cast <int> (a) & static_cast <int> (b)); }

    /**
     * @brief perform binary OR on JsonReadMode.
     * @param a bitset.
     * @param b other bitset.
     * @return bitset result of binary OR on JsonReadMode.
     */
    constexpr JsonReadMode operator| (JsonReadMode a, JsonReadMode b)
    { return JsonReadMode (static_cast <int> (a) | static_cast <int> (b)); }

    /**
     * @brief perform binary AND on JsonReadMode.
     * @param a bitset.
     * @param b other bitset.
     * @return bitset result of binary AND on JsonReadMode.
     */
    constexpr const JsonReadMode& operator&= (JsonReadMode& a, JsonReadMode b)
    { return a = a & b; }

    /**
     * @brief perform binary OR on JsonReadMode.
     * @param a bitset.
     * @param b other bitset.
     * @return bitset result of binary OR.
     */
    constexpr const JsonReadMode& operator|= (JsonReadMode& a, JsonReadMode b)
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
         * @param readMode JSON parsing mode.
         */
        JsonReader (Value& root, JsonReadMode readMode = JsonReadMode::None)
        : StreamReader (root),
          _mode (readMode)
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
        int deserialize (const char* document, size_t length) override
        {
            StringView in (document, length);
            return read (in);
        }

        /**
         * @brief Deserialize a document.
         * @param first The first character of the document to parse.
         * @param last The last character of the document to parse.
         * @return 0 on success, -1 otherwise.
         */
        int deserialize (const char* first, const char* last) override
        {
            StringView in (first, last);
            return read (in);
        }

        /**
         * @brief Deserialize a document.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        int deserialize (const std::string& document) override
        {
            StringView in (document.c_str (), document.size ());
            return read (in);
        }

        /**
         * @brief Parse a document.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        int deserialize (std::istream& document) override
        {
            StreamView in (document);
            return read (in);
        }

    protected:
        /**
         * @brief parse a document.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType>
        int read (ViewType& document)
        {
            if (readValue (document) == 0)
            {
                skipWhitespaces (document);

                if (document.peek () == std::char_traits <char>::eof ())
                {
                    return 0;
                }

                join::lastError = make_error_code (SaxErrc::ExtraData);
            }

            return -1;
        }

        /**
         * @brief parse a JSON value.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType>
        int readValue (ViewType& document)
        {
            if (skipComments (document) == 0)
            {
                switch (document.peek ())
                {
                    case '[':
                        document.get ();
                        return readArray (document);
                    case '{':
                        document.get ();
                        return readObject (document);
                    case 'n':
                        document.get ();
                        return readNull (document);
                    case 't':
                        document.get ();
                        return readTrue (document);
                    case 'f':
                        document.get ();
                        return readFalse (document);
                    case '"':
                        document.get ();
                        return readString (document);
                    default:
                        return readNumber (document);
                }
            }

            return -1;
        }

        /**
         * @brief parse a null value.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType>
        int readNull (ViewType& document)
        {
            if (JOIN_SAX_UNLIKELY ((document.get () != 'u') || (document.get () != 'l') || (document.get () != 'l')))
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
            if (JOIN_SAX_UNLIKELY ((document.get () != 'r') || (document.get () != 'u') || (document.get () != 'e')))
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
            if (JOIN_SAX_UNLIKELY ((document.get () != 'a') || (document.get () != 'l') || (document.get () != 's') || (document.get () != 'e')))
            {
                join::lastError = make_error_code (SaxErrc::InvalidValue);
                return -1;
            }

            return setBool (false);
        }

        /**
         * @brief convert double using fast path.
         * @param significand significand digitd.
         * @param exponent exponent.
         * @param digits number of digit.
         * @param value converted value.
         * @return true on success, false otherwise.
         */
        bool strtodFast (uint64_t significand, int64_t exponent, uint64_t digits, double &value)
        {
            static const double pow10[] = {
                1e0,  1e1,  1e2,  1e3,  1e4,  1e5,  1e6,  1e7,  1e8,  1e9,  1e10,
                1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19, 1e20, 1e21,
                1e22
            };

            if (JOIN_SAX_LIKELY ((exponent >= -325) && (exponent <= 308) && (digits <= 19)))
            {
                value = static_cast <double> (significand);

                if (JOIN_SAX_UNLIKELY (value == 0))
                {
                    value = 0.0;
                    return true;
                }

                if ((exponent > 22) && (exponent < (22 + 16)))
                {
                    value *= pow10[exponent - 22];
                    exponent = 22;
                }

                if ((exponent >= -22) && (exponent <= 22) && (value <= 9007199254740991.0))
                {
                    if (exponent < 0)
                    {
                        value /= pow10[-exponent];
                    }
                    else
                    {
                        value *= pow10[exponent];
                    }

                    return true;
                }
            }

            return false;
        }

        /**
         * @brief parse a number value.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType>
        int readNumber (ViewType& document)
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

            if (JOIN_SAX_UNLIKELY (document.getIf ('0')))
            {
                if (JOIN_SAX_UNLIKELY (isDigit (document.peek ())))
                {
                    join::lastError = make_error_code (SaxErrc::InvalidValue);
                    return -1;
                }
            }
            else if (JOIN_SAX_LIKELY (isDigit (document.peek ())))
            {
                u = document.get () - '0';
                ++digits;

                while (JOIN_SAX_LIKELY (isDigit (document.peek ())))
                {
                    int digit = document.peek () - '0';

                    if (JOIN_SAX_UNLIKELY (u > ((max64 - digit) / 10)))
                    {
                        isDouble = true;
                        break;
                    }

                    u = (u * 10) + digit;
                    document.get ();
                    ++digits;
                }
            }
            else if (JOIN_SAX_LIKELY (document.getIfNoCase ('i') && document.getIfNoCase ('n') && document.getIfNoCase ('f')))
            {
                if (JOIN_SAX_UNLIKELY (document.getIfNoCase ('i') && !(document.getIfNoCase ('n') && document.getIfNoCase ('i') && document.getIfNoCase ('t') && document.getIfNoCase ('y'))))
                {
                    join::lastError = make_error_code (SaxErrc::InvalidValue);
                    return -1;
                }

                return setDouble (negative ? -std::numeric_limits <double>::infinity () : std::numeric_limits <double>::infinity ());
            }
            else if (JOIN_SAX_LIKELY (document.getIfNoCase ('n') && document.getIfNoCase ('a') && document.getIfNoCase ('N')))
            {
                return setDouble (negative ? -std::numeric_limits <double>::quiet_NaN () : std::numeric_limits <double>::quiet_NaN ());
            }
            else
            {
                join::lastError = make_error_code (SaxErrc::InvalidValue);
                return -1;
            }

            if (isDouble)
            {
                while (JOIN_SAX_LIKELY (isDigit (document.peek ())))
                {
                    u = (u * 10) + (document.get () - '0');
                    ++digits;
                }
            }

            int64_t frac = 0;

            if (document.getIf ('.'))
            {
                isDouble = true;

                while (JOIN_SAX_LIKELY (isDigit (document.peek ())))
                {
                    u = (u * 10) + (document.get () - '0');
                    if (JOIN_SAX_LIKELY (u || digits))
                    {
                        ++digits;
                    }
                    --frac;
                }
            }

            int64_t exp = 0;

            if (document.getIf ('e') || document.getIf ('E'))
            {
                bool negExp = false;
                isDouble = true;

                if (isSign (document.peek ()))
                {
                    negExp = (document.get () == '-');
                }

                if (JOIN_SAX_LIKELY (isDigit (document.peek ())))
                {
                    exp = (document.get () - '0');

                    while (JOIN_SAX_LIKELY (isDigit (document.peek ())))
                    {
                        int digit = document.peek () - '0';

                        if (JOIN_SAX_LIKELY (exp <= ((std::numeric_limits <int>::max () - digit) / 10)))
                        {
                            exp = (exp * 10) + digit;
                        }

                        document.get ();
                    }
                }
                else
                {
                    join::lastError = make_error_code (SaxErrc::InvalidValue);
                    return -1;
                }

                if (negExp)
                {
                    exp = -exp;
                }
            }

            int64_t exponent = exp + frac;

            if (isDouble)
            {
                double d = 0.0;

                if (strtodFast (u, exponent, digits, d))
                {
                    return setDouble (negative ? -d : d);
                }

                size_t len = document.tell () - beg;
                std::string number;

                number.resize (len);
                document.rewind (len);
                document.read (&number[0], len);

                char* end = nullptr;
                static locale_t locale = newlocale (LC_ALL_MASK, "C", nullptr);
                d = strtod_l (number.data (), &end, locale);
                if (end == number.data ())
                {
                    join::lastError = make_error_code (SaxErrc::InvalidValue);
                    return -1;
                }

                return setDouble (d);
            }

            if (negative)
            {
                return setInt64 (-u);
            }

            return setUint64 (u);
        }

        /**
         * @brief .
         * @param document document to parse.
         * @param u .
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType>
        int readHex (ViewType& document, uint32_t& u)
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
         * @brief .
         * @param codepoint UTF8 codepoint.
         * @param output parse output string.
         */
        void encodeUtf8 (uint32_t codepoint, std::string& output)
        {
            if (codepoint < 0x80)
            {
                output.push_back (static_cast <char> (codepoint));
            }
            else if (codepoint < 0x800)
            {
                output.push_back (static_cast <char> (0xC0 | (codepoint >> 6)));
                output.push_back (static_cast <char> (0x80 | (codepoint & 0x3F)));
            }
            else if (codepoint < 0x010000)
            {
                output.push_back (static_cast <char> (0xE0 | (codepoint >> 12)));
                output.push_back (static_cast <char> (0x80 | ((codepoint >> 6) & 0x3F)));
                output.push_back (static_cast <char> (0x80 | (codepoint & 0x3F)));
            }
            else
            {
                output.push_back (static_cast <char> (0xF0 | (codepoint >> 18)));
                output.push_back (static_cast <char> (0x80 | ((codepoint >> 12) & 0x3F)));
                output.push_back (static_cast <char> (0x80 | ((codepoint >> 6) & 0x3F)));
                output.push_back (static_cast <char> (0x80 | (codepoint & 0x3F)));
            }
        }

        /**
         * @brief parse unicode.
         * @param document document to parse.
         * @param output parse output string.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType>
        int readUnicode (ViewType& document, std::string& output)
        {
            uint32_t u = 0;

            if (readHex (document, u) == -1)
            {
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
        int readEscaped (ViewType& document, std::string& output)
        {
            if (document.getIf ('\\'))
            {
                if (document.getIf ('"'))
                {
                    output.push_back ('"');
                }
                else if (document.getIf ('\\'))
                {
                    output.push_back ('\\');
                }
                else if (document.getIf ('b'))
                {
                    output.push_back ('\b');
                }
                else if (document.getIf ('f'))
                {
                    output.push_back ('\f');
                }
                else if (document.getIf ('n'))
                {
                    output.push_back ('\n');
                }
                else if (document.getIf ('r'))
                {
                    output.push_back ('\r');
                }
                else if (document.getIf ('t'))
                {
                    output.push_back ('\t');
                }
                else if (document.getIf ('/'))
                {
                    output.push_back ('/');
                }
                else if (document.getIf ('u'))
                {
                    if (readUnicode (document, output) == -1)
                    {
                        return -1;
                    }
                }
                else
                {
                    join::lastError = make_error_code (JsonErrc::InvalidEscaping);
                    return -1;
                }
            }
            else
            {
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
        int readUtf8 (ViewType& document, std::string& output)
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
         * @param output parse output string.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType>
        int readStringSlow (ViewType& document, bool isKey, std::string& output)
        {
            for (;;)
            {
                if (JOIN_SAX_UNLIKELY (document.getIf ('"')))
                {
                    break;
                }
                else if (JOIN_SAX_UNLIKELY (static_cast <uint8_t> (document.peek ()) == '\\'))
                {
                    if (readEscaped (document, output) == -1)
                    {
                        return -1;
                    }
                }
                /*else if (JOIN_SAX_UNLIKELY (static_cast <uint8_t> (document.peek ()) > 0x7F))
                {
                    if (readUtf8 (document, output) == -1)
                    {
                        return -1;
                    }
                }*/
                else if (JOIN_SAX_UNLIKELY (static_cast <uint8_t> (document.peek ()) < 0x20))
                {
                    join::lastError = make_error_code (JsonErrc::IllegalCharacter);
                    return -1;
                }
                else if (JOIN_SAX_UNLIKELY (document.peek () == std::char_traits <char>::eof ()))
                {
                    join::lastError = make_error_code (JsonErrc::EndOfFile);
                    return -1;
                }
                else
                {
                    output.push_back (document.get ());
                }
            }

            return isKey ? setKey (output) : setString (output);
        }

        /**
         * @brief parse a string value.
         * @param document document to parse.
         * @param isKey indicate whether the string to parse is a key or not.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType>
        int readString (ViewType& document, bool isKey = false)
        {
            size_t beg = document.tell ();

            for (;;)
            {
                if (!isPlainText (static_cast <uint8_t> (document.peek ())))
                {
                    if (JOIN_SAX_UNLIKELY (document.peek () == std::char_traits <char>::eof ()))
                    {
                        break;
                    }

                    std::string output;
                    size_t len = document.tell () - beg;

                    output.resize (len);
                    document.rewind (len);
                    document.read (&output[0], len);

                    if (JOIN_SAX_LIKELY (document.getIf ('"')))
                    {
                        return isKey ? setKey (output) : setString (output);
                    }

                    return readStringSlow (document, isKey, output);
                }

                document.get ();
            }

            join::lastError = make_error_code (JsonErrc::MissingQuote);
            return false;
        }

        /**
         * @brief parse an array value.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType>
        int readArray (ViewType& document)
        {
            if (JOIN_SAX_UNLIKELY (startArray () == -1))
            {
                return -1;
            }

            if (JOIN_SAX_UNLIKELY (skipComments (document) == -1))
            {
                return -1;
            }

            if (document.getIf (']'))
            {
                return stopArray ();
            }

            for (;;)
            {
                if (JOIN_SAX_UNLIKELY (readValue (document) == -1))
                {
                    return -1;
                }

                if (JOIN_SAX_UNLIKELY (skipComments (document) == -1))
                {
                    return -1;
                }

                if (document.getIf (','))
                {
                    if (JOIN_SAX_UNLIKELY (skipComments (document) == -1))
                    {
                        return -1;
                    }
                }
                else if (document.getIf (']'))
                {
                    break;
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
        template <typename ViewType>
        int readObject (ViewType& document)
        {
            if (JOIN_SAX_UNLIKELY (startObject () == -1))
            {
                return -1;
            }

            if (JOIN_SAX_UNLIKELY (skipComments (document) == -1))
            {
                return -1;
            }

            if (document.getIf ('}'))
            {
                return stopObject ();
            }

            for (;;)
            {
                if (JOIN_SAX_UNLIKELY (document.get () != '"'))
                {
                    join::lastError = make_error_code (JsonErrc::MissingQuote);
                    return -1;
                }

                if (JOIN_SAX_UNLIKELY (readString (document, true) == -1))
                {
                    return -1;
                }

                if (JOIN_SAX_UNLIKELY (skipComments (document) == -1))
                {
                    return -1;
                }

                if (JOIN_SAX_UNLIKELY (document.get () != ':'))
                {
                    join::lastError = make_error_code (JsonErrc::MissingColon);
                    return -1;
                }

                if (JOIN_SAX_UNLIKELY (readValue (document) == -1))
                {
                    return -1;
                }

                if (JOIN_SAX_UNLIKELY (skipComments (document) == -1))
                {
                    return -1;
                }

                if (document.getIf (','))
                {
                    if (JOIN_SAX_UNLIKELY (skipComments (document) == -1))
                    {
                        return -1;
                    }
                }
                else if (document.getIf ('}'))
                {
                    break;
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
        constexpr bool isWhitespace (char c)
        {
            return ((c == ' ') || (c == '\n') || (c == '\r') || (c == '\t'));
        }

        /**
         * @brief skip whitespaces.
         * @param document document to parse.
         */
        template <typename ViewType>
        constexpr void skipWhitespaces (ViewType& document)
        {
            for (;;)
            {
                if (!isWhitespace (document.peek ()))
                {
                    break;
                }
                document.get ();
            }
        }

        /**
         * @brief skip comments.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType>
        constexpr int skipComments (ViewType& document)
        {
            skipWhitespaces (document);

            if (_mode & JsonReadMode::ParseComments)
            {
                while (JOIN_SAX_UNLIKELY (document.getIf ('/')))
                {
                    if (document.getIf ('*'))
                    {
                        while ((document.get () != '*') || (document.get () != '/'))
                        {
                            // ignore comment.
                        }
                    }
                    else if (document.getIf ('/'))
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
            }

            return 0;
        }

        /**
         * @brief check if plain text.
         * @param c character to check.
         * @return true if plain text, false otherwise.
         */
        constexpr bool isPlainText (uint8_t c)
        {
            return (c >= 0x20) && (c <= 0x7F) && (c != 0x5C) && (c != 0x22);
        }

        /**
         * @brief check if upper case alphanumeric character.
         * @param c character to check.
         * @return true if upper case alphanumeric character, false otherwise.
         */
        constexpr bool isUpperAlpha (char c)
        {
            return (c >= 'A') && (c <= 'F');
        }

        /**
         * @brief check if lower case alphanumeric character.
         * @param c character to check.
         * @return true if lower case alphanumeric character, false otherwise.
         */
        constexpr bool isLowerAlpha (char c)
        {
            return (c >= 'a') && (c <= 'f');
        }

        /**
         * @brief check if digit.
         * @param c character to check.
         * @return true if digit, false otherwise.
         */
        constexpr bool isDigit (char c)
        {
            return (c >= '0') && (c <= '9');
        }

        /**
         * @brief check if sign.
         * @param c character to check.
         * @return true if sign, false otherwise.
         */
        constexpr bool isSign (char c)
        {
            return (c == '+') || (c == '-');
        }

        /// JSON parsing mode.
        JsonReadMode _mode = JsonReadMode::None;
    };
}

namespace std
{
    /// JSON error code specialization.
    template <> struct is_error_condition_enum <join::JsonErrc> : public true_type {};
}

#endif
