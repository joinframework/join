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

#ifndef __JOIN_JSONREADER_HPP__
#define __JOIN_JSONREADER_HPP__

// libjoin.
#include <join/jsonwriter.hpp>
#include <join/view.hpp>

namespace join
{
namespace sax
{
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
        JsonReader (Value& root);

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
         * @brief Parse a document.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        virtual int read (join::View& document) override;

        /**
         * @brief Parse a JSON value.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        int readValue (join::View& document);

        /**
         * @brief Parse a null value.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        int readNull (join::View& document);

        /**
         * @brief Parse a true value.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        int readTrue (join::View& document);

        /**
         * @brief Parse a false value.
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
         * @brief Parse a number value.
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
         * @param output Parse output string.
         */
        void encodeUtf8 (uint32_t codepoint, std::string& output);

        /**
         * @brief Parse unicode.
         * @param document document to parse.
         * @param output Parse output string.
         * @return 0 on success, -1 otherwise.
         */
        int readUnicode (join::View& document, std::string& output);

        /**
         * @brief Parse escaped sequence.
         * @param document document to parse.
         * @param output Parse output string.
         * @return 0 on success, -1 otherwise.
         */
        int readEscaped (join::View& document, std::string& output);

        /**
         * @brief Parse UTF8.
         * @param document document to parse.
         * @param output Parse output string.
         * @return 0 on success, -1 otherwise.
         */
        int readUtf8 (join::View& document, std::string& output);

        /**
         * @brief Parse a string value.
         * @param document document to parse.
         * @param isKey indicate whether the string to parse is a key or not.
         * @param output Parse output string.
         * @return 0 on success, -1 otherwise.
         */
        int readStringSlow (View& document, bool isKey, std::string& output);

        /**
         * @brief Parse a string value.
         * @param document document to parse.
         * @param isKey indicate whether the string to parse is a key or not.
         * @return 0 on success, -1 otherwise.
         */
        int readString (join::View& document, bool isKey = false);

        /**
         * @brief Parse an array value.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        int readArray (join::View& document);

        /**
         * @brief Parse an object value.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        int readObject (join::View& document);

        /**
         * @brief Skip whitespace.
         * @param document document to parse.
         */
        void skipWhitespace (join::View& document);

        /**
         * @brief Skip comment.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        int skipComment (join::View& document);

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
    };
}
}

#endif
