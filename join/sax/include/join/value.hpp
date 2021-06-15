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

#ifndef __JOIN_VALUE_HPP__
#define __JOIN_VALUE_HPP__

// libjoin.
#include <join/jsonwriter.hpp>
#include <join/jsonreader.hpp>
#include <join/variant.hpp>
#include <join/error.hpp>

// C++.
#include <functional>
#include <string>
#include <vector>

namespace join
{
namespace sax
{
    /// forward declaration.
    class Value;

    /// array.
    using Array = std::vector <Value>;

    /// object member.
    using Member = std::pair <std::string, Value>;

    /// object.
    using Object = std::vector <Member>;

    /**
     * @brief Value.
     */
    class Value : public Variant <std::nullptr_t, bool, int32_t, uint32_t, int64_t, uint64_t, double, std::string, Array, Object>
    {
    public:
        /**
         * @brief Value index.
         */
        enum Index : size_t
        {
            Null        = 0,    /**< Index of the std::nullptr_t alternative that can be held by the Value object. */
            Boolean     = 1,    /**< Index of the boolean alternative that can be held by the Value object. */
            Integer     = 2,    /**< Index of the 32 bits integer alternative that can be held by the Value object. */
            Unsigned    = 3,    /**< Index of the 32 bits unsigned integer alternative that can be held by the Value object. */
            Integer64   = 4,    /**< Index of the 64 bits integer alternative that can be held by the Value object. */
            Unsigned64  = 5,    /**< Index of the 64 bits unsigned integer alternative that can be held by the Value object. */
            Real        = 6,    /**< Index of the real alternative that can be held by the Value object. */
            String      = 7,    /**< Index of the std::string alternative that can be held by the Value object. */
            ArrayValue  = 8,    /**< Index of the Array alternative that can be held by the Value object. */
            ObjectValue = 9,    /**< Index of the Object alternative that can be held by the Value object. */
        };

        /// Pointer.
        using Ptr = Value*;

        /// Inherits parent's constructors.
        using Variant::Variant;

        /// Inherits parent's assignment operators.
        using Variant::operator=;

        /**
         * @brief Default constructor.
         */
        Value () = default;

        /**
         * @brief Constructs the value with the copy of the contents of a C-style string.
         * @param other C-style string to use as data source.
         */
        Value (const char* other);

        /**
         * @brief Replaces the contents with a copy of a C-style string.
         * @param other C-style string to use as data source.
         * @return A reference of the current value.
         */
        Value& operator= (const char* other);

        /**
         * @brief Copy constructor.
         * @param other object to copy.
         */
        Value (const Value& other) = default;

        /**
         * @brief Copy assignment.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        Value& operator= (const Value& other) = default;

        /**
         * @brief Move constructor.
         * @param other object to move.
         */
        Value (Value&& other) = default;

        /**
         * @brief Move assignment.
         * @param other object to move.
         * @return a reference of the current object.
         */
        Value& operator= (Value&& other) = default;

        /**
         * @brief Destroy the Value instance.
         */
        virtual ~Value () = default;

        /**
         * @brief Check if the variable held by value is a null value.
         * @return true if the variable held by value is null, false otherwise.
         */
        bool isNull () const;

        /**
         * @brief Check if the variable held by value is a boolean value.
         * @return true if the variable held by value is a boolean value, false otherwise.
         */
        bool isBool () const;

        /**
         * @brief Explicit conversion function for boolean value.
         * @return Converted boolean value.
         * @throw std::bad_cast.
         */
        explicit operator bool () const;

        /**
         * @brief Get variable held by value as a boolean value.
         * @return Variable held by value as a boolean value.
         * @throw std::bad_cast.
         */
        bool getBool () const;

        /**
         * @brief Check if the content of the value is true.
         * @return True if the content of the value is true, false otherwise.
         * @throw std::bad_cast.
         */
        bool isTrue () const;

        /**
         * @brief Check if the content of the value is false.
         * @return True if the content of the value is false, false otherwise.
         * @throw std::bad_cast.
         */
        bool isFalse () const;

        /**
         * @brief Check if the variable held by value is a number value.
         * @return true if the variable held by value is a number, false otherwise.
         */
        bool isNumber () const;

        /**
         * @brief Check if the variable held by value is a 8 bits integer value.
         * @return true if the variable held by value is a 8 bits integer value, false otherwise.
         */
        bool isInt8 () const;

        /**
         * @brief Get variable held by value as a 8 bits integer.
         * @return Variable held by value as a 8 bits integer.
         * @throw std::bad_cast.
         */
        int8_t getInt8 () const;

        /**
         * @brief Explicit conversion function for 8 bits integer value.
         * @return Converted char value.
         * @throw std::bad_cast.
         */
        explicit operator int8_t () const;

        /**
         * @brief Check if the variable held by value is a 8 bits unsigned integer value.
         * @return true if the variable held by value is a 8 bits unsigned integer value, false otherwise.
         */
        bool isUint8 () const;

        /**
         * @brief Get variable held by value as a 8 bits unsigned integer.
         * @return Variable held by value as a 8 bits unsigned integer.
         * @throw std::bad_cast.
         */
        uint8_t getUint8 () const;

        /**
         * @brief Explicit conversion function for 8 bits unsigned integer value.
         * @return Converted unsihed char value.
         * @throw std::bad_cast.
         */
        explicit operator uint8_t () const;

        /**
         * @brief Check if the variable held by value is a 16 bits integer value.
         * @return true if the variable held by value is a 16 bits integer value, false otherwise.
         */
        bool isInt16 () const;

        /**
         * @brief Get variable held by value as a 16 bits integer.
         * @return Variable held by value as a 16 bits integer.
         * @throw std::bad_cast.
         */
        int16_t getInt16 () const;

        /**
         * @brief Explicit conversion function for 16 bits integer value.
         * @return Converted short value.
         * @throw std::bad_cast.
         */
        explicit operator int16_t () const;

        /**
         * @brief Check if the variable held by value is a 16 bits unsigned integer value.
         * @return true if the variable held by value is a 16 bits unsigned integer value, false otherwise.
         */
        bool isUint16 () const;

        /**
         * @brief Get variable held by value as a 16 bits unsigned integer.
         * @return Variable held by value as a 16 bits unsigned integer.
         * @throw std::bad_cast.
         */
        uint16_t getUint16 () const;

        /**
         * @brief Explicit conversion function for 16 bits unsigned integer value.
         * @return Converted unsigned short value.
         * @throw std::bad_cast.
         */
        explicit operator uint16_t () const;

        /**
         * @brief Check if the variable held by value is a 32 bits integer value.
         * @return true if the variable held by value is a 32 bits integer value, false otherwise.
         */
        bool isInt () const;

        /**
         * @brief Get variable held by value as a 32 bits integer.
         * @return Variable held by value as a 32 bits integer.
         * @throw std::bad_cast.
         */
        int32_t getInt () const;

        /**
         * @brief Explicit conversion function for 32 bits integer value.
         * @return Converted integer value.
         * @throw std::bad_cast.
         */
        explicit operator int32_t () const;

        /**
         * @brief Check if the variable held by value is a 32 bits unsigned integer value.
         * @return true if the variable held by value is a 32 bits unsigned integer value, false otherwise.
         */
        bool isUint () const;

        /**
         * @brief Get variable held by value as a 32 bits unsigned integer value.
         * @return Variable held by value as a 32 bits unsigned integer value.
         * @throw std::bad_cast.
         */
        uint32_t getUint () const;

        /**
         * @brief Explicit conversion function for 32 bits unsigned integer value.
         * @return Converted unsigned integer value.
         * @throw std::bad_cast.
         */
        explicit operator uint32_t () const;

        /**
         * @brief Check if the variable held by value is a 64 bits integer value.
         * @return true if the variable held by value is a 64 bits integer value, false otherwise.
         */
        bool isInt64 () const;

        /**
         * @brief Get variable held by value as a 64 bits integer value.
         * @return Variable held by value as a 64 bits integer value.
         * @throw std::bad_cast.
         */
        int64_t getInt64 () const;

        /**
         * @brief Explicit conversion function for 64 bits integer value.
         * @return Converted integer 64 bits value.
         * @throw std::bad_cast.
         */
        explicit operator int64_t () const;

        /**
         * @brief Check if the variable held by value is a 64 bits unsigned integer value.
         * @return true if the variable held by value is a 64 bits unsigned integer value, false otherwise.
         */
        bool isUint64 () const;

        /**
         * @brief Get variable held by value as a 64 bits unsigned integer value.
         * @return Variable held by value as a 64 bits unsigned integer value.
         * @throw std::bad_cast.
         */
        uint64_t getUint64 () const;

        /**
         * @brief Explicit conversion function for 64 bits unsigned integer value.
         * @return Converted 64 bits unsigned integer value.
         * @throw std::bad_cast.
         */
        explicit operator uint64_t () const;

        /**
         * @brief Check if the variable held by value is a float value.
         * @return true if the variable held by value is a float value, false otherwise.
         */
        bool isFloat () const;

        /**
         * @brief Get variable held by value as a float value.
         * @return Variable held by value as a float value.
         * @throw std::bad_cast.
         */
        float getFloat () const;

        /**
         * @brief Explicit conversion function for float value.
         * @return Converted float value.
         * @throw std::bad_cast.
         */
        explicit operator float () const;

        /**
         * @brief Check if the variable held by value is a double value.
         * @return true if the variable held by value is a double value, false otherwise.
         */
        bool isDouble () const;

        /**
         * @brief Get variable held by value as a double value.
         * @return Variable held by value as a double value.
         * @throw std::bad_cast.
         */
        double getDouble () const;

        /**
         * @brief Explicit conversion function for double value.
         * @return Converted double value.
         * @throw std::bad_cast.
         */
        explicit operator double () const;

        /**
         * @brief Check if the variable held by value is a string value.
         * @return true if the variable held by value is a string value, false otherwise.
         */
        bool isString () const;

        /**
         * @brief Get variable held by value as a string value.
         * @return Variable held by value as a string value.
         * @throw std::bad_cast.
         */
        std::string& getString () const;

        /**
         * @brief Explicit conversion function for string value.
         * @return Converted string value.
         * @throw std::bad_cast.
         */
        explicit operator const char * () const;

        /**
         * @brief Check if the variable held by value is an array.
         * @return true if the variable held by value is an array, false otherwise.
         */
        bool isArray () const;

        /**
         * @brief Get variable held by value as an array.
         * @return Variable held by value as an array.
         * @throw std::bad_cast.
         */
        Array& getArray () const;

        /**
         * @brief Check if the variable held by value is an object.
         * @return True if the variable held by value is an object, false otherwise.
         */
        bool isObject () const;

        /**
         * @brief Get variable held by value as an object.
         * @return Variable held by value as an object.
         * @throw std::bad_cast.
         */
        Object& getObject () const;

        /**
         * @brief Returns a reference to the element at specified location pos.
         * @param pos Position of the element to return.
         * @return Reference to the requested element.
         * @throw std::bad_cast.
         */
        Value& at (size_t pos);

        /**
         * @brief Returns a reference to the element at specified location pos.
         * @param pos Position of the element to return.
         * @return Reference to the requested element.
         * @throw std::bad_cast.
         */
        const Value& at (size_t pos) const;

        /**
         * @brief Returns a reference to the value that is mapped to a key.
         * @param key The key of the element to find.
         * @return Reference to the mapped element.
         * @throw std::bad_cast.
         */
        Value& at (const std::string& key);

        /**
         * @brief Returns a reference to the value that is mapped to a key.
         * @param key The key of the element to find.
         * @return Reference to the mapped element.
         * @throw std::bad_cast.
         */
        const Value& at (const std::string& key) const;

        /**
         * @brief Returns a reference to the element at specified location pos.
         * @param pos Position of the element to return.
         * @return Reference to the requested element.
         * @throw std::bad_cast.
         */
        Value& operator[] (size_t pos);

        /**
         * @brief Returns a reference to the element at specified location pos.
         * @param pos Position of the element to return.
         * @return Reference to the requested element.
         * @throw std::bad_cast.
         */
        const Value& operator[] (size_t pos) const;

        /**
         * @brief Returns a reference to the value that is mapped to a key.
         * @param key The key of the element to find.
         * @return Reference to the mapped element.
         * @throw std::bad_cast.
         */
        Value& operator[] (const std::string& key);

        /**
         * @brief Returns a reference to the value that is mapped to a key.
         * @param key The key of the element to find.
         * @return Reference to the mapped element.
         * @throw std::bad_cast.
         */
        const Value& operator[] (const std::string& key) const;

        /**
         * @brief Check if the nested container is empty.
         * @return True if empty, false otherwise.
         * @throw std::bad_cast.
         */
        bool empty () const;

        /**
         * @brief Returns the number of elements in the nested container.
         * @return The number of elements in the nested container.
         * @throw std::bad_cast.
         */
        size_t size () const;

        /**
         * @brief Increase the capacity of the nested container.
         * @param cap new capacity.
         * @throw std::bad_cast.
         */
        void reserve (size_t cap);

        /**
         * @brief Erases all elements in the nested container.
         * @throw std::bad_cast.
         */
        void clear ();

        /**
         * @brief Insert element in the nested container.
         * @param element The element to insert in the nested container.
         * @return A reference to the inserted element.
         * @throw std::bad_cast.
         */
        Value& insert (const Member& element);

        /**
         * @brief Insert element in the nested container.
         * @param element The element to insert in the nested container.
         * @return A reference to the inserted element.
         * @throw std::bad_cast.
         */
        Value& insert (Member&& element);

        /**
         * @brief Removes member with the key equivalent to key.
         * @param key Key value of the member to remove.
         * @return Number of elements removed.
         * @throw std::bad_cast.
         */
        size_t erase (const std::string& key);

        /**
         * @brief Appends element at the end of the nested container.
         * @param value The element to append.
         * @return a reference to the pushed object.
         * @throw std::bad_cast.
         */
        template <typename... Args>
        Value& emplaceBack (Args&&... args)
        {
            return get <ArrayValue> ().emplace_back (std::forward <Args> (args)...);
        }

        /**
         * @brief Appends element at the end of the nested container.
         * @param value The element to append.
         * @return a reference to the pushed object.
         * @throw std::bad_cast.
         */
        Value& pushBack (const Value& value);

        /**
         * @brief Appends element at the end of the nested container.
         * @param value The element to append.
         * @return a reference to the pushed object.
         * @throw std::bad_cast.
         */
        Value& pushBack (Value&& value);

        /**
         * @brief Removes the last element of the nested container.
         * @throw std::bad_cast.
         */
        void popBack ();

        /**
         * @brief Checks if the nested container contains an element at position pos.
         * @return True if the nested container contains an element at position pos, false otherwise.
         * @throw std::bad_cast.
         */
        bool contains (size_t pos) const;

        /**
         * @brief Checks if the nested container contains an element that is mapped to key.
         * @return True if the nested container contains an element mapped to key, false otherwise.
         * @throw std::bad_cast.
         */
        bool contains (const std::string& key) const;

        /**
         * @brief Exchanges the contents of the value with those of other.
         * @param other Value to exchange the contents with.
         */
        void swap (Value& other);

        /**
         * @brief Deserialize a JSON document.
         * @param document JSON document to parse.
         * @return 0 on success, -1 otherwise.
         */
        int jsonRead (const char* document);

        /**
         * @brief Deserialize a JSON document.
         * @param document JSON document to parse.
         * @param length The length of the JSON document to parse.
         * @return 0 on success, -1 otherwise.
         */
        int jsonRead (const char* document, size_t length);

        /**
         * @brief Deserialize a JSON document.
         * @param first The first character of the JSON document to parse.
         * @param last The last character of the JSON document to parse.
         * @return 0 on success, -1 otherwise.
         */
        int jsonRead (const char* first, const char* last);

        /**
         * @brief Deserialize a JSON document.
         * @param document JSON document to parse.
         * @return 0 on success, -1 otherwise.
         */
        int jsonRead (const std::string& document);

        /**
         * @brief Parse a JSON document.
         * @param document JSON document to parse.
         * @return 0 on success, -1 otherwise.
         */
        int jsonRead (std::istream& document);

        /**
         * @brief Serialize data to JSON format.
         * @param document Document where to serialize data.
         * @param indentation number of characters used to indent JSON.
         * @return 0 on success, -1 otherwise.
         */
        bool jsonWrite (std::ostream& document, size_t indentation = 0) const;

        // friendship with equal operator.
        friend bool operator== (const Value& lhs, const Value& rhs);

        // friendship with not equal operator.
        friend bool operator!= (const Value& lhs, const Value& rhs);

        // friendship with lower operator.
        friend bool operator< (const Value& lhs, const Value& rhs);

        // friendship with greater operator.
        friend bool operator> (const Value& lhs, const Value& rhs);

        // friendship with lower or equal operator.
        friend bool operator<= (const Value& lhs, const Value& rhs);

        // friendship with greater or equal operator.
        friend bool operator>= (const Value& lhs, const Value& rhs);
    };

    /**
     * @brief Compare if equal.
     * @param lhs value to compare.
     * @param rhs value to compare to.
     * @return true if equal.
     */
    bool operator== (const Value& lhs, const Value& rhs);

    /**
     * @brief Compare if not equal.
     * @param lhs value to compare.
     * @param rhs value to compare to.
     * @return true if not equal.
     */
    bool operator!= (const Value& lhs, const Value& rhs);

    /**
     * @brief Compare if lower than.
     * @param lhs value to compare.
     * @param rhs value to compare to.
     * @return true if lower than.
     */
    bool operator< (const Value& lhs, const Value& rhs);

    /**
     * @brief Compare if greater than.
     * @param lhs value to compare.
     * @param rhs value to compare to.
     * @return true if greater than.
     */
    bool operator> (const Value& lhs, const Value& rhs);

    /**
     * @brief Compare if lower or equal than.
     * @param lhs value to compare.
     * @param rhs value to compare to.
     * @return true if lower or equal than.
     */
    bool operator<= (const Value& lhs, const Value& rhs);

    /**
     * @brief Compare if greater or equal than.
     * @param lhs value to compare.
     * @param rhs value to compare to.
     * @return true if greater or equal than.
     */
    bool operator>= (const Value& lhs, const Value& rhs);
}
}

#endif
