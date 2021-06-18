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

#ifndef __JOIN_SAX_HPP__
#define __JOIN_SAX_HPP__

// libjoin.
#include <join/error.hpp>
#include <join/value.hpp>
#include <join/view.hpp>

// C++.
#include <system_error>
#include <iostream>
#include <string>
#include <stack>

// C.
#include <cstddef>

#define JOIN_SAX_LIKELY(x)   __builtin_expect(!!(x), 1)
#define JOIN_SAX_UNLIKELY(x) __builtin_expect(!!(x), 0)

namespace join
{
namespace sax
{
    /// forward declaration.
    class Value;

    /**
     * @brief SAX API generic error codes.
     */
    enum class SaxErrc
    {
        StackOverflow = 1,      /**< Stack overflow. */
        InvalidParent,          /**< Parent not an array nor an object. */
        InvalidValue,           /**< Value is invalid. */
        ExtraData               /**< Extra data detected. */
    };

    /**
     * @brief SAX API generic error category.
     */
    class SaxCategory : public std::error_category
    {
    public:
        /**
         * @brief get SAX API generic error category name.
         * @return deserializer generic error category name.
         */
        virtual const char* name () const noexcept;

        /**
         * @brief translate SAX API generic error code to human readable error string.
         * @param code error code.
         * @return human readable error string.
         */
        virtual std::string message (int code) const;
    };

    /**
     * @brief get error category.
     * @return the created std::error_category object.
     */
    const std::error_category& saxCategory ();

    /**
     * @brief create an std::error_code object.
     * @param code error code number.
     * @return the created std::error_code object.
     */
    std::error_code make_error_code (SaxErrc code);

    /**
     * @brief create an std::error_condition object.
     * @param code error code number.
     * @return the created std::error_condition object.
     */
    std::error_condition make_error_condition (SaxErrc code);

    /**
     * @brief SAX API interface class.
     */
    class SaxHandler
    {
    public:
        /**
         * @brief default constructor.
         */
        SaxHandler () = default;

        /**
         * @brief copy constructor.
         * @param other object to copy.
         */
        SaxHandler (const SaxHandler& other) = default;

        /**
         * @brief copy assignment.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        SaxHandler& operator= (const SaxHandler& other) = default;

        /**
         * @brief move constructor.
         * @param other object to move.
         */
        SaxHandler (SaxHandler&& other) = default;

        /**
         * @brief move assignment.
         * @param other object to move.
         * @return a reference of the current object.
         */
        SaxHandler& operator= (SaxHandler&& other) = default;

        /**
         * @brief destroy instance.
         */
        virtual ~SaxHandler () = default;

        /**
         * @brief set null value.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setNull () = 0;

        /**
         * @brief set boolean value.
         * @param value boolean value to set.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setBool (bool value) = 0;

        /**
         * @brief set integer value.
         * @param value integer value to set.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setInt (int32_t value) = 0;

        /**
         * @brief set unsigned integer value.
         * @param value unsigned integer value to set.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setUint (uint32_t value) = 0;

        /**
         * @brief set 64 bits integer value.
         * @param value 64 bits integer value to set.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setInt64 (int64_t value) = 0;

        /**
         * @brief set unsigned 64 bits integer value.
         * @param value unsigned 64 bits integer value to set.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setUint64 (uint64_t value) = 0;

        /**
         * @brief set real value.
         * @param value real value to set.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setDouble (double value) = 0;

        /**
         * @brief set string value.
         * @param value string value to set.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setString (const std::string& value) = 0;

        /**
         * @brief start array.
         * @param size array size.
         * @return 0 on success, -1 otherwise.
         */
        virtual int startArray (uint32_t size = 0) = 0;

        /**
         * @brief stop array.
         * @return 0 on success, -1 otherwise.
         */
        virtual int stopArray () = 0;

        /**
         * @brief start object.
         * @param size array size.
         * @return 0 on success, -1 otherwise.
         */
        virtual int startObject (uint32_t size = 0) = 0;

        /**
         * @brief set key.
         * @param key object key.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setKey (const std::string& key) = 0;

        /**
         * @brief stop object.
         * @return 0 on success, -1 otherwise.
         */
        virtual int stopObject () = 0;
    };

    /**
     * @brief stream serializer abstract class.
     */
    class StreamWriter : public SaxHandler
    {
    public:
        /**
         * @brief create instance.
         * @param document document to create.
         */
        StreamWriter (std::ostream& document);

        /**
         * @brief Create the Writer instance by copy.
         * @param other Other object to copy.
         */
        StreamWriter (const StreamWriter& other) = delete;

        /**
         * @brief Assign the Writer instance by copy.
         * @param other Other object to copy.
         */
        StreamWriter& operator= (const StreamWriter& other) = delete;

        /**
         * @brief Create the Writer instance by move.
         * @param other Other object to move.
         */
        StreamWriter (StreamWriter&& other) = delete;

        /**
         * @brief Assign the Writer instance by move.
         * @param other Other object to move.
         */
        StreamWriter& operator= (StreamWriter&& other) = delete;

        /**
         * @brief Destroy the Writer instance.
         */
        virtual ~StreamWriter () = default;

        /**
         * @brief Serialize data.
         * @param value Value object to serialize.
         * @return 0 on success, -1 otherwise.
         */
        int serialize (const Value& value);

    protected:
        /**
         * @brief append data to output stream and update data size.
         * @param data data to append to output stream.
         * @param size data size.
         */
        void append (const char* data, uint32_t size);

        /**
         * @brief append data to stream and update data size.
         * @param data data to append to stream.
         */
        void append (char data);

        /// output stream.
        std::ostream& outstream_;
    };

    /**
     * @brief stream deserializer abstract class.
     */
    class StreamReader : protected SaxHandler
    {
    public:
        /**
         * @brief default constructor.
         * @param root Value to write.
         */
        StreamReader (Value& root);

        /**
         * @brief copy constructor.
         * @param other object to copy.
         */
        StreamReader (const StreamReader& other) = delete;

        /**
         * @brief copy assignment.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        StreamReader& operator= (const StreamReader& other) = delete;

        /**
         * @brief move constructor.
         * @param other object to move.
         */
        StreamReader (StreamReader&& other) = delete;

        /**
         * @brief move assignment.
         * @param other object to move.
         * @return a reference of the current object.
         */
        StreamReader& operator= (StreamReader&& other) = delete;

        /**
         * @brief destroy instance.
         */
        virtual ~StreamReader () = default;

        /**
         * @brief Deserialize a document.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        virtual int deserialize (const char* document) = 0;

        /**
         * @brief Deserialize a document.
         * @param document document to parse.
         * @param length The length of the document to parse.
         * @return 0 on success, -1 otherwise.
         */
        virtual int deserialize (const char* document, size_t length) = 0;

        /**
         * @brief Deserialize a document.
         * @param first The first character of the document to parse.
         * @param last The last character of the document to parse.
         * @return 0 on success, -1 otherwise.
         */
        virtual int deserialize (const char* first, const char* last) = 0;

        /**
         * @brief Deserialize a document.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        virtual int deserialize (const std::string& document) = 0;

        /**
         * @brief Parse a document.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        virtual int deserialize (std::istream& document) = 0;

    protected:
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
         * @param size array size.
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
         * @param size array size.
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

        /**
         * @brief set value.
         * @param value value to set.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setValue (Value&& value);

        /// max stack depth.
        static constexpr size_t maxdepth_ = 19;

        /// stack.
        std::stack <Value*> stack_;

        /// current key.
        std::string curkey_;

        /// root.
        Value& root_;
    };
}
}

namespace std
{
    /// SAX API generic error code specialization.
    template <> struct is_error_condition_enum <join::sax::SaxErrc> : public true_type {};
}

#endif
