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
        MissingComma            /**< missing comma. */
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
        JsonWriter (std::ostream& document, size_t indentation = 0);

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
        virtual int setNull () override;

        /**
         * @brief set boolean value.
         * @param value boolean value to set.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setBool (bool value) override;

        /**
         * @brief set integer value.
         * @param value integer value to set.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setInt (int32_t value) override;

        /**
         * @brief set unsigned integer value.
         * @param value unsigned integer value to set.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setUint (uint32_t value) override;

        /**
         * @brief set 64 bits integer value.
         * @param value 64 bits integer value to set.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setInt64 (int64_t value) override;

        /**
         * @brief set unsigned 64 bits integer value.
         * @param value unsigned 64 bits integer value to set.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setUint64 (uint64_t value) override;

        /**
         * @brief set real value.
         * @param value real value to set.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setDouble (double value) override;

        /**
         * @brief set string value.
         * @param value string value to set.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setString (const std::string& value) override;

        /**
         * @brief start array.
         * @param size array size (ignored).
         * @return 0 on success, -1 otherwise.
         */
        virtual int startArray (uint32_t size = 0) override;

        /**
         * @brief stop array.
         * @return 0 on success, -1 otherwise.
         */
        virtual int stopArray () override;

        /**
         * @brief start object.
         * @param size array size (ignored).
         * @return 0 on success, -1 otherwise.
         */
        virtual int startObject (uint32_t size = 0) override;

        /**
         * @brief set key.
         * @param key object key.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setKey (const std::string& key) override;

        /**
         * @brief stop object.
         * @return 0 on success, -1 otherwise.
         */
        virtual int stopObject () override;

    private:
        /**
         * @brief write integer value.
         * @param value integer value to write.
         */
        void writeInt (int32_t value);

        /**
         * @brief write unsigned integer value.
         * @param value unsigned integer value to write.
         */
        void writeUint (uint32_t value);

        /**
         * @brief write 64 bits integer value.
         * @param value 64 bits integer value to write.
         */
        void writeInt64 (int64_t value);

        /**
         * @brief write 64 bits unsigned integer value.
         * @param value 64 bits unsigned integer value to write.
         */
        void writeUint64 (uint64_t value);

        /**
         * @brief write real value.
         * @param value real value to write.
         */
        void writeDouble (double value);

        /**
         * @brief get UTF8 codepoint.
         * @param cur current character.
         * @param end end of string.
         * @param codepoint calculated UTF8 codepoint.
         * @return 0 on success, -1 otherwise.
         */
        int utf8Codepoint (std::string::const_iterator& cur, std::string::const_iterator& end, uint32_t& codepoint);

        /**
         * @brief escape string value.
         * @param value string value to escape.
         * @return 0 on success, -1 otherwise.
         */
        int writeEscaped (const std::string& value);

        /**
         * @brief write comma.
         */
        void comma ();

        /**
         * @brief write indentation.
         */
        void indent ();

        /**
         * @brief write space.
         */
        void space ();

        /**
         * @brief write end of line.
         */
        void endLine ();

        /**
         * @brief add comma, go to line and indent if in array.
         */
        void array ();

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
        None                = 0,        /**< no read mode set. */
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
        JsonReader (Value& root, JsonReadMode readMode = JsonReadMode::None);

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

    protected:
        /**
         * @brief parse a document.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        virtual int read (join::View& document) override;

        /**
         * @brief parse a JSON value.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        int readValue (join::View& document);

        /**
         * @brief parse a null value.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        int readNull (join::View& document);

        /**
         * @brief parse a true value.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        int readTrue (join::View& document);

        /**
         * @brief parse a false value.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        int readFalse (join::View& document);

        /**
         * @brief Get the value of 10 raised to the power of exponent.
         * @param exponent Power exponent.
         * @return the value of 10 raised to the power of exponent.
         * @throw std::invalid_argument.
         */
        const double& pow10 (size_t exponent);

        /**
         * @brief parse a number value.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        int readNumber (join::View& document);

        /**
         * @brief .
         * @param document document to parse.
         * @param u .
         * @return 0 on success, -1 otherwise.
         */
        int readHex (join::View& document, uint32_t& u);

        /**
         * @brief .
         * @param codepoint UTF8 codepoint.
         * @param output parse output string.
         */
        void encodeUtf8 (uint32_t codepoint, std::string& output);

        /**
         * @brief parse unicode.
         * @param document document to parse.
         * @param output parse output string.
         * @return 0 on success, -1 otherwise.
         */
        int readUnicode (join::View& document, std::string& output);

        /**
         * @brief parse escaped sequence.
         * @param document document to parse.
         * @param output parse output string.
         * @return 0 on success, -1 otherwise.
         */
        int readEscaped (join::View& document, std::string& output);

        /**
         * @brief parse UTF8.
         * @param document document to parse.
         * @param output parse output string.
         * @return 0 on success, -1 otherwise.
         */
        //int readUtf8 (join::View& document, std::string& output);

        /**
         * @brief parse a string value.
         * @param document document to parse.
         * @param isKey indicate whether the string to parse is a key or not.
         * @param output parse output string.
         * @return 0 on success, -1 otherwise.
         */
        int readStringSlow (View& document, bool isKey, std::string& output);

        /**
         * @brief parse a string value.
         * @param document document to parse.
         * @param isKey indicate whether the string to parse is a key or not.
         * @return 0 on success, -1 otherwise.
         */
        int readString (join::View& document, bool isKey = false);

        /**
         * @brief parse an array value.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        int readArray (join::View& document);

        /**
         * @brief parse an object value.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        int readObject (join::View& document);

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
        constexpr void skipWhitespaces (join::View& document)
        {
            for (;;)
            {
                if (!isWhitespace (document.peek ()))
                {
                    break;
                }
                ++document;
            }
        }

        /**
         * @brief skip comments.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        constexpr int skipComments (join::View& document)
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
