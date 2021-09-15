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
namespace sax
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
            char* end = join::sax::dtoa (buf, value);
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
         * @return 0 on success, -1 otherwise.
         */
        int deserialize (const char* document) override
        {
            StringView in (document);
            return read (in);
        }

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
         * @brief Get the value of 10 raised to the power of exponent.
         * @param exponent Power exponent.
         * @return the value of 10 raised to the power of exponent.
         * @throw std::invalid_argument.
         */
        const double& pow10 (size_t exponent)
        {
            static const double result[] =
            {
                1e0,    1e+1,   1e+2,   1e+3,   1e+4,   1e+5,   1e+6,   1e+7,   1e+8,
                1e+9,   1e+10,  1e+11,  1e+12,  1e+13,  1e+14,  1e+15,  1e+16,  1e+17,
                1e+18,  1e+19,  1e+20,  1e+21,  1e+22,  1e+23,  1e+24,  1e+25,  1e+26,
                1e+27,  1e+28,  1e+29,  1e+30,  1e+31,  1e+32,  1e+33,  1e+34,  1e+35,
                1e+36,  1e+37,  1e+38,  1e+39,  1e+40,  1e+41,  1e+42,  1e+43,  1e+44,
                1e+45,  1e+46,  1e+47,  1e+48,  1e+49,  1e+50,  1e+51,  1e+52,  1e+53,
                1e+54,  1e+55,  1e+56,  1e+57,  1e+58,  1e+59,  1e+60,  1e+61,  1e+62,
                1e+63,  1e+64,  1e+65,  1e+66,  1e+67,  1e+68,  1e+69,  1e+70,  1e+71,
                1e+72,  1e+73,  1e+74,  1e+75,  1e+76,  1e+77,  1e+78,  1e+79,  1e+80,
                1e+81,  1e+82,  1e+83,  1e+84,  1e+85,  1e+86,  1e+87,  1e+88,  1e+89,
                1e+90,  1e+91,  1e+92,  1e+93,  1e+94,  1e+95,  1e+96,  1e+97,  1e+98,
                1e+99,  1e+100, 1e+101, 1e+102, 1e+103, 1e+104, 1e+105, 1e+106, 1e+107,
                1e+108, 1e+109, 1e+110, 1e+111, 1e+112, 1e+113, 1e+114, 1e+115, 1e+116,
                1e+117, 1e+118, 1e+119, 1e+120, 1e+121, 1e+122, 1e+123, 1e+124, 1e+125,
                1e+126, 1e+127, 1e+128, 1e+129, 1e+130, 1e+131, 1e+132, 1e+133, 1e+134,
                1e+135, 1e+136, 1e+137, 1e+138, 1e+139, 1e+140, 1e+141, 1e+142, 1e+143,
                1e+144, 1e+145, 1e+146, 1e+147, 1e+148, 1e+149, 1e+150, 1e+151, 1e+152,
                1e+153, 1e+154, 1e+155, 1e+156, 1e+157, 1e+158, 1e+159, 1e+160, 1e+161,
                1e+162, 1e+163, 1e+164, 1e+165, 1e+166, 1e+167, 1e+168, 1e+169, 1e+170,
                1e+171, 1e+172, 1e+173, 1e+174, 1e+175, 1e+176, 1e+177, 1e+178, 1e+179,
                1e+180, 1e+181, 1e+182, 1e+183, 1e+184, 1e+185, 1e+186, 1e+187, 1e+188,
                1e+189, 1e+190, 1e+191, 1e+192, 1e+193, 1e+194, 1e+195, 1e+196, 1e+197,
                1e+198, 1e+199, 1e+200, 1e+201, 1e+202, 1e+203, 1e+204, 1e+205, 1e+206,
                1e+207, 1e+208, 1e+209, 1e+210, 1e+211, 1e+212, 1e+213, 1e+214, 1e+215,
                1e+216, 1e+217, 1e+218, 1e+219, 1e+220, 1e+221, 1e+222, 1e+223, 1e+224,
                1e+225, 1e+226, 1e+227, 1e+228, 1e+229, 1e+230, 1e+231, 1e+232, 1e+233,
                1e+234, 1e+235, 1e+236, 1e+237, 1e+238, 1e+239, 1e+240, 1e+241, 1e+242,
                1e+243, 1e+244, 1e+245, 1e+246, 1e+247, 1e+248, 1e+249, 1e+250, 1e+251,
                1e+252, 1e+253, 1e+254, 1e+255, 1e+256, 1e+257, 1e+258, 1e+259, 1e+260,
                1e+261, 1e+262, 1e+263, 1e+264, 1e+265, 1e+266, 1e+267, 1e+268, 1e+269,
                1e+270, 1e+271, 1e+272, 1e+273, 1e+274, 1e+275, 1e+276, 1e+277, 1e+278,
                1e+279, 1e+280, 1e+281, 1e+282, 1e+283, 1e+284, 1e+285, 1e+286, 1e+287,
                1e+288, 1e+289, 1e+290, 1e+291, 1e+292, 1e+293, 1e+294, 1e+295, 1e+296,
                1e+297, 1e+298, 1e+299, 1e+300, 1e+301, 1e+302, 1e+303, 1e+304, 1e+305,
                1e+306, 1e+307, 1e+308
            };

            return result [exponent];
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
            bool negative = false;

            if (isSign (document.peek ()))
            {
                negative = (document.get () == '-');
            }

            uint64_t max64 = std::numeric_limits <uint64_t>::max ();
            if (negative)
            {
                max64 = static_cast <uint64_t> (std::numeric_limits <int64_t>::max ()) + 1;
            }

            size_t digitsStart = document.tell ();
            bool isDouble = false;
            uint64_t u = 0;

            if (JOIN_SAX_UNLIKELY (document.getIf ('0')))
            {
                if (JOIN_SAX_UNLIKELY (isDigit (document.peek ())))
                {
                    join::lastError = make_error_code (SaxErrc::InvalidValue);
                    return -1;
                }

                digitsStart++;
            }
            else if (JOIN_SAX_LIKELY (isDigit (document.peek ())))
            {
                u = document.get () - '0';

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
                }
            }
            else if (JOIN_SAX_LIKELY (document.getIf ('I') && document.getIf ('n') && document.getIf ('f')))
            {
                if (JOIN_SAX_UNLIKELY (document.getIf ('i') && !((document.get () == 'n') && (document.get () == 'i') && (document.get () == 't') && (document.get () == 'y'))))
                {
                    join::lastError = make_error_code (SaxErrc::InvalidValue);
                    return -1;
                }

                return setDouble (negative ? -std::numeric_limits <double>::infinity () : std::numeric_limits <double>::infinity ());
            }
            else if (JOIN_SAX_LIKELY (document.getIf ('N') && (document.get () == 'a') && (document.get () == 'N')))
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
                }
            }

            int64_t exponent = 0;
            int digitsOffset = 0;

            if (document.getIf ('.'))
            {
                size_t exponentStart = document.tell ();
                digitsOffset = 1;
                isDouble = true;

                while (JOIN_SAX_LIKELY (isDigit (document.peek ())))
                {
                    u = (u * 10) + (document.get () - '0');

                    if (JOIN_SAX_UNLIKELY (u == 0))
                    {
                        ++digitsStart;
                    }
                }

                exponent = exponentStart - document.tell ();
            }

            int digits = document.tell () - digitsStart - digitsOffset;

            if (document.getIf ('e') || document.getIf ('E'))
            {
                bool negativeExp = false;
                isDouble = true;

                if (isSign (document.peek ()))
                {
                    negativeExp = (document.get () == '-');
                }

                int exp = 0;

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

                exponent += negativeExp ? -exp : exp;
            }

            if (isDouble)
            {
                if (JOIN_SAX_UNLIKELY ((digits > std::numeric_limits <double>::max_digits10) || (exponent < -308) || (exponent > 308)))
                {
                    std::string number;
                    size_t len = document.tell () - beg;

                    number.resize (len);
                    document.rewind (len);
                    document.read (&number[0], len);

                    char* end = nullptr;
                    static locale_t locale = newlocale (LC_ALL_MASK, "C", nullptr);
                    double d = strtod_l (number.data (), &end, locale);
                    if (end == number.data ())
                    {
                        join::lastError = make_error_code (SaxErrc::InvalidValue);
                        return -1;
                    }

                    return setDouble (d);
                }

                double d = static_cast <double> (u);

                if (JOIN_SAX_LIKELY (d != 0.0))
                {
                    if (exponent >= 0)
                    {
                        d *= pow10 (exponent);
                    }
                    else
                    {
                        d /= pow10 (-exponent);
                    }
                }

                return setDouble (negative ? -d : d);
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
}

namespace std
{
    /// JSON error code specialization.
    template <> struct is_error_condition_enum <join::sax::JsonErrc> : public true_type {};
}

#endif
