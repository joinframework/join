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

#ifndef __JOIN_JSONWRITER_HPP__
#define __JOIN_JSONWRITER_HPP__

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
        InvalidComment = 1,     /**< Comment is invalid. */
        InvalidEscaping,        /**< Character escaping is invalid. */
        InvalidEncoding,        /**< Character encoding is invalid. */
        IllegalCharacter,       /**< Illegal character. */
        MissingCurlyBracket,    /**< Missing curly bracket. */
        MissingSquareBracket,   /**< Missing square bracket. */
        MissingQuote,           /**< Missing quote. */
        MissingColon,           /**< Missing colon. */
        MissingComma            /**< Missing comma. */
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
        std::stack <bool> stack_;

        /// indentation.
        size_t indentation_;

        /// tabulation.
        std::string tab_;

        /// is first element.
        bool first_ = true;
    };
}
}

namespace std
{
    /// JSON error code specialization.
    template <> struct is_error_condition_enum <join::sax::JsonErrc> : public true_type {};
}

#endif
