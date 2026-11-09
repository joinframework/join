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

#ifndef JOIN_DATA_VALUE_HPP
#define JOIN_DATA_VALUE_HPP

// libjoin.
#include <join/bump_allocator.hpp>
#include <join/sax_error.hpp>
#include <join/variant.hpp>
#include <join/memory.hpp>
#include <join/utils.hpp>
#include <join/view.hpp>

// C++.
#include <type_traits>
#include <string_view>
#include <stdexcept>
#include <algorithm>
#include <concepts>
#include <compare>
#include <ostream>
#include <memory>
#include <string>
#include <limits>
#include <span>
#include <new>

// C.
#include <cstdint>
#include <cstring>
#include <cmath>

namespace join
{
    /// forward declaration.
    template <typename Allocator = HeapMem::BumpAllocator<4096, BumpPlain>>
    class BasicValue;

    /// forward declaration.
    template <typename Value>
    class ValueBuilder;

    /// string.
    using String = View<const char>;

    /// binary.
    using Binary = View<const uint8_t>;

    /**
     * @brief member.
     */
    template <typename Type>
    struct BasicMember
    {
        /**
         * @brief compare members.
         * @param other member to compare to.
         * @return true if equal.
         */
        constexpr bool operator== (const BasicMember& other) const = default;

        /**
         * @brief three-way compare members.
         * @param other member to compare to.
         * @return ordering of this member relative to other.
         */
        constexpr std::partial_ordering operator<=> (const BasicMember& other) const = default;

        /// member key.
        Type first;

        /// member value.
        Type second;
    };

    /**
     * @brief value class, the root owning or borrowing the allocator shared by its children.
     */
    template <typename Allocator>
    class BasicValue
    {
    public:
        /// value.
        using Value = BasicValue;

        /// array.
        using Array = View<BasicValue>;

        /// member.
        using Member = BasicMember<BasicValue>;

        /// object.
        using Object = View<Member>;

        /**
         * @brief nested value type index.
         */
        enum Index : size_t
        {
            NullValue = 0,       /**< index of the null alternative. */
            BooleanValue = 1,    /**< index of the boolean alternative. */
            IntegerValue = 2,    /**< index of the 32 bits integer alternative. */
            UnsignedValue = 3,   /**< index of the 32 bits unsigned integer alternative. */
            Integer64Value = 4,  /**< index of the 64 bits integer alternative. */
            Unsigned64Value = 5, /**< index of the 64 bits unsigned integer alternative. */
            RealValue = 6,       /**< index of the real alternative. */
            StringValue = 7,     /**< index of the string alternative. */
            BinaryValue = 8,     /**< index of the binary alternative. */
            ArrayValue = 9,      /**< index of the array alternative. */
            ObjectValue = 10,    /**< index of the object alternative. */
        };

        /**
         * @brief default constructor.
         */
        constexpr BasicValue () noexcept = default;

        /**
         * @brief constructs a null value using an allocator owned by the caller.
         * @param allocator allocator providing the memory of the value.
         */
        explicit BasicValue (Allocator& allocator)
        : _allocator (std::addressof (allocator))
        , _owner (false)
        {
        }

        /**
         * @brief constructs a null value.
         * @param value null value.
         */
        constexpr BasicValue (std::nullptr_t value)
        : _data (std::in_place_index<NullValue>, value)
        {
        }

        /**
         * @brief constructs a null value using an allocator owned by the caller.
         * @param value null value.
         * @param allocator allocator providing the memory of the value.
         */
        BasicValue (std::nullptr_t value, Allocator& allocator)
        : BasicValue (allocator)
        {
            _data.template set<NullValue> (value);
        }

        /**
         * @brief constructs a boolean value.
         * @param value boolean value.
         */
        constexpr BasicValue (bool value)
        : _data (std::in_place_index<BooleanValue>, value)
        {
        }

        /**
         * @brief constructs a boolean value using an allocator owned by the caller.
         * @param value boolean value.
         * @param allocator allocator providing the memory of the value.
         */
        BasicValue (bool value, Allocator& allocator)
        : BasicValue (allocator)
        {
            _data.template set<BooleanValue> (value);
        }

        /**
         * @brief constructs a integer value.
         * @param value integer value.
         */
        constexpr BasicValue (int32_t value)
        : _data (std::in_place_index<IntegerValue>, value)
        {
        }

        /**
         * @brief constructs a integer value using an allocator owned by the caller.
         * @param value integer value.
         * @param allocator allocator providing the memory of the value.
         */
        BasicValue (int32_t value, Allocator& allocator)
        : BasicValue (allocator)
        {
            _data.template set<IntegerValue> (value);
        }

        /**
         * @brief constructs a unsigned integer value.
         * @param value unsigned integer value.
         */
        constexpr BasicValue (uint32_t value)
        : _data (std::in_place_index<UnsignedValue>, value)
        {
        }

        /**
         * @brief constructs a unsigned integer value using an allocator owned by the caller.
         * @param value unsigned integer value.
         * @param allocator allocator providing the memory of the value.
         */
        BasicValue (uint32_t value, Allocator& allocator)
        : BasicValue (allocator)
        {
            _data.template set<UnsignedValue> (value);
        }

        /**
         * @brief constructs a 64 bits integer value.
         * @param value 64 bits integer value.
         */
        constexpr BasicValue (int64_t value)
        : _data (std::in_place_index<Integer64Value>, value)
        {
        }

        /**
         * @brief constructs a 64 bits integer value using an allocator owned by the caller.
         * @param value 64 bits integer value.
         * @param allocator allocator providing the memory of the value.
         */
        BasicValue (int64_t value, Allocator& allocator)
        : BasicValue (allocator)
        {
            _data.template set<Integer64Value> (value);
        }

        /**
         * @brief constructs a 64 bits unsigned integer value.
         * @param value 64 bits unsigned integer value.
         */
        constexpr BasicValue (uint64_t value)
        : _data (std::in_place_index<Unsigned64Value>, value)
        {
        }

        /**
         * @brief constructs a 64 bits unsigned integer value using an allocator owned by the caller.
         * @param value 64 bits unsigned integer value.
         * @param allocator allocator providing the memory of the value.
         */
        BasicValue (uint64_t value, Allocator& allocator)
        : BasicValue (allocator)
        {
            _data.template set<Unsigned64Value> (value);
        }

        /**
         * @brief constructs a real value.
         * @param value real value.
         */
        constexpr BasicValue (double value)
        : _data (std::in_place_index<RealValue>, value)
        {
        }

        /**
         * @brief constructs a real value using an allocator owned by the caller.
         * @param value real value.
         * @param allocator allocator providing the memory of the value.
         */
        BasicValue (double value, Allocator& allocator)
        : BasicValue (allocator)
        {
            _data.template set<RealValue> (value);
        }

        /**
         * @brief constructs a string value from a string view.
         * @param value string value.
         */
        BasicValue (std::string_view value)
        : BasicValue (nullptr)
        {
            _data.template set<StringValue> (copyString (value));
        }

        /**
         * @brief constructs a string value from a string view using an allocator owned by the caller.
         * @param value string value.
         * @param allocator allocator providing the memory of the value.
         */
        BasicValue (std::string_view value, Allocator& allocator)
        : BasicValue (allocator)
        {
            _data.template set<StringValue> (copyString (value));
        }

        /**
         * @brief constructs a string value from a C-style string.
         * @param value string value.
         */
        BasicValue (const char* value)
        : BasicValue (std::string_view (value))
        {
        }

        /**
         * @brief constructs a string value from a C-style string using an allocator owned by the caller.
         * @param value string value.
         * @param allocator allocator providing the memory of the value.
         */
        BasicValue (const char* value, Allocator& allocator)
        : BasicValue (std::string_view (value), allocator)
        {
        }

        /**
         * @brief constructs a string value.
         * @param value string value.
         */
        BasicValue (const std::string& value)
        : BasicValue (std::string_view (value))
        {
        }

        /**
         * @brief constructs a binary value.
         * @param value binary value.
         */
        BasicValue (std::span<const uint8_t> value)
        : BasicValue (nullptr)
        {
            _data.template set<BinaryValue> (copyBinary (value));
        }

        /**
         * @brief constructs a binary value using an allocator owned by the caller.
         * @param value binary value.
         * @param allocator allocator providing the memory of the value.
         */
        BasicValue (std::span<const uint8_t> value, Allocator& allocator)
        : BasicValue (allocator)
        {
            _data.template set<BinaryValue> (copyBinary (value));
        }

        /**
         * @brief constructs an array value.
         * @param value array value.
         */
        BasicValue (Array value)
        : BasicValue (nullptr)
        {
            _data.template set<ArrayValue> (copyArray (value));
        }

        /**
         * @brief constructs an array value using an allocator owned by the caller.
         * @param value array value.
         * @param allocator allocator providing the memory of the value.
         */
        BasicValue (Array value, Allocator& allocator)
        : BasicValue (allocator)
        {
            _data.template set<ArrayValue> (copyArray (value));
        }

        /**
         * @brief constructs an object value.
         * @param value object value.
         */
        BasicValue (Object value)
        : BasicValue (nullptr)
        {
            _data.template set<ObjectValue> (copyObject (value));
        }

        /**
         * @brief constructs an object value using an allocator owned by the caller.
         * @param value object value.
         * @param allocator allocator providing the memory of the value.
         */
        BasicValue (Object value, Allocator& allocator)
        : BasicValue (allocator)
        {
            _data.template set<ObjectValue> (copyObject (value));
        }

        /**
         * @brief copy constructor.
         * @param other object to copy.
         */
        BasicValue (const BasicValue& other)
        : BasicValue (nullptr)
        {
            copy (other);
        }

        /**
         * @brief copy assignment.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        Value& operator= (const Value& other)
        {
            if (_owner)
            {
                return *this = BasicValue (other);
            }

            copy (other);

            return *this;
        }

        /**
         * @brief constructs a copy of a value using an allocator owned by the caller.
         * @param other value to copy.
         * @param allocator allocator providing the memory of the value.
         */
        template <typename Other>
            requires std::same_as<Other, BasicValue>
        BasicValue (const Other& other, Allocator& allocator)
        : BasicValue (allocator)
        {
            copy (other);
        }

        /**
         * @brief assign a null value.
         * @param value null value.
         * @return a reference of the current object.
         */
        Value& operator= (std::nullptr_t value)
        {
            reset ();
            _data.template set<NullValue> (value);
            return *this;
        }

        /**
         * @brief assign a boolean value.
         * @param value boolean value.
         * @return a reference of the current object.
         */
        Value& operator= (bool value)
        {
            reset ();
            _data.template set<BooleanValue> (value);
            return *this;
        }

        /**
         * @brief assign a integer value.
         * @param value integer value.
         * @return a reference of the current object.
         */
        Value& operator= (int32_t value)
        {
            reset ();
            _data.template set<IntegerValue> (value);
            return *this;
        }

        /**
         * @brief assign a unsigned integer value.
         * @param value unsigned integer value.
         * @return a reference of the current object.
         */
        Value& operator= (uint32_t value)
        {
            reset ();
            _data.template set<UnsignedValue> (value);
            return *this;
        }

        /**
         * @brief assign a 64 bits integer value.
         * @param value 64 bits integer value.
         * @return a reference of the current object.
         */
        Value& operator= (int64_t value)
        {
            reset ();
            _data.template set<Integer64Value> (value);
            return *this;
        }

        /**
         * @brief assign a 64 bits unsigned integer value.
         * @param value 64 bits unsigned integer value.
         * @return a reference of the current object.
         */
        Value& operator= (uint64_t value)
        {
            reset ();
            _data.template set<Unsigned64Value> (value);
            return *this;
        }

        /**
         * @brief assign a real value.
         * @param value real value.
         * @return a reference of the current object.
         */
        Value& operator= (double value)
        {
            reset ();
            _data.template set<RealValue> (value);
            return *this;
        }

        /**
         * @brief assign a string value from a string view.
         * @param value string value.
         * @return a reference of the current object.
         */
        Value& operator= (std::string_view value)
        {
            if (isString () && (value.size () <= std::max<size_t> (_capacity, getString ().size ())))
            {
                char* data = const_cast<char*> (getString ().data ());
                _capacity = std::max<size_t> (_capacity, getString ().size ());
                std::char_traits<char>::move (data, value.data (), value.size ());
                data[value.size ()] = '\0';
                _data.template set<StringValue> (data, value.size ());
                return *this;
            }

            if (_owner || (_allocator == nullptr))
            {
                return *this = BasicValue (value);
            }

            _data.template set<StringValue> (copyString (value));
            _capacity = 0;

            return *this;
        }

        /**
         * @brief assign a string value from a C-style string.
         * @param value string value.
         * @return a reference of the current object.
         */
        Value& operator= (const char* value)
        {
            return *this = std::string_view (value);
        }

        /**
         * @brief assign a string value.
         * @param value string value.
         * @return a reference of the current object.
         */
        Value& operator= (const std::string& value)
        {
            return *this = std::string_view (value);
        }

        /**
         * @brief assign a binary value.
         * @param value binary value.
         * @return a reference of the current object.
         */
        Value& operator= (std::span<const uint8_t> value)
        {
            if (_owner || (_allocator == nullptr))
            {
                return *this = BasicValue (value);
            }

            _data.template set<BinaryValue> (copyBinary (value));
            _capacity = 0;

            return *this;
        }

        /**
         * @brief assign an array value.
         * @param value array value.
         * @return a reference of the current object.
         */
        Value& operator= (Array value)
        {
            if (_owner || (_allocator == nullptr))
            {
                return *this = BasicValue (value);
            }

            _data.template set<ArrayValue> (copyArray (value));
            _capacity = 0;

            return *this;
        }

        /**
         * @brief assign an object value.
         * @param value object value.
         * @return a reference of the current object.
         */
        Value& operator= (Object value)
        {
            if (_owner || (_allocator == nullptr))
            {
                return *this = BasicValue (value);
            }

            _data.template set<ObjectValue> (copyObject (value));
            _capacity = 0;

            return *this;
        }

        /**
         * @brief move constructor.
         * @param other object to move.
         */
        BasicValue (BasicValue&& other)
        : BasicValue (nullptr)
        {
            if (other._owner)
            {
                _data = other._data;
                _allocator = other._allocator;
                _capacity = other._capacity;
                _owner = other._owner;
                other._data.template set<NullValue> ();
                other._allocator = nullptr;
                other._capacity = 0;
                other._owner = false;
            }
            else
            {
                copy (other);
                other._data.template set<NullValue> ();
                other._capacity = 0;
            }
        }

        /**
         * @brief move assignment.
         * @param other object to move.
         * @return a reference of the current object.
         */
        Value& operator= (Value&& other)
        {
            if (other._owner && (_owner || (_allocator == nullptr)))
            {
                if (_owner)
                {
                    delete _allocator;
                }
                _data = other._data;
                _allocator = other._allocator;
                _capacity = other._capacity;
                _owner = other._owner;
                other._data.template set<NullValue> ();
                other._allocator = nullptr;
                other._capacity = 0;
                other._owner = false;
            }
            else if (_allocator == other._allocator)
            {
                _data = other._data;
                _capacity = other._capacity;
                other._data.template set<NullValue> ();
                other._capacity = 0;
            }
            else
            {
                reset ();
                copy (other);
                other._data.template set<NullValue> ();
                other._capacity = 0;
            }

            return *this;
        }

        /**
         * @brief destroy the Value instance.
         */
        ~BasicValue ()
        {
            if (_owner)
            {
                delete _allocator;
            }
        }

        /**
         * @brief get the index of the alternative in use.
         * @return index of the alternative in use.
         */
        constexpr std::size_t index () const noexcept
        {
            return _data.index ();
        }

        /**
         * @brief check if the variable held by value is a null value.
         * @return true if the variable held by value is null, false otherwise.
         */
        constexpr bool isNull () const
        {
            return _data.index () == NullValue;
        }

        /**
         * @brief check if the variable held by value is a boolean value.
         * @return true if the variable held by value is a boolean value, false otherwise.
         */
        constexpr bool isBool () const
        {
            return _data.index () == BooleanValue;
        }

        /**
         * @brief get variable held by value as a boolean value.
         * @return variable held by value as a boolean value.
         * @throw std::bad_cast.
         */
        constexpr bool getBool () const
        {
            if (_data.index () == BooleanValue)
            {
                return _data.template get<BooleanValue> ();
            }

            return convertBool ();
        }

        /**
         * @brief explicit conversion function for boolean value.
         * @return converted boolean value.
         * @throw std::bad_cast.
         */
        explicit operator bool () const
        {
            return getBool ();
        }

        /**
         * @brief check if the content of the value is true.
         * @return true if the content of the value is true, false otherwise.
         * @throw std::bad_cast.
         */
        constexpr bool isTrue () const
        {
            return getBool ();
        }

        /**
         * @brief check if the content of the value is false.
         * @return true if the content of the value is false, false otherwise.
         * @throw std::bad_cast.
         */
        constexpr bool isFalse () const
        {
            return !getBool ();
        }

        /**
         * @brief check if the variable held by value is a number value.
         * @return true if the variable held by value is a number, false otherwise.
         */
        constexpr bool isNumber () const
        {
            return _data.index () == IntegerValue || _data.index () == UnsignedValue ||
                   _data.index () == Integer64Value || _data.index () == Unsigned64Value || _data.index () == RealValue;
        }

        /**
         * @brief check if the variable held by value is a 8 bits integer value.
         * @return true if the variable held by value is a 8 bits integer value, false otherwise.
         */
        constexpr bool isInt8 () const
        {
            switch (_data.index ())
            {
                case IntegerValue:
                    return _data.template get<IntegerValue> () >=
                               static_cast<int32_t> (std::numeric_limits<int8_t>::min ()) &&
                           _data.template get<IntegerValue> () <=
                               static_cast<int32_t> (std::numeric_limits<int8_t>::max ());

                case UnsignedValue:
                    return _data.template get<UnsignedValue> () <=
                           static_cast<uint32_t> (std::numeric_limits<int8_t>::max ());

                case Integer64Value:
                    return _data.template get<Integer64Value> () >=
                               static_cast<int64_t> (std::numeric_limits<int8_t>::min ()) &&
                           _data.template get<Integer64Value> () <=
                               static_cast<int64_t> (std::numeric_limits<int8_t>::max ());

                case Unsigned64Value:
                    return _data.template get<Unsigned64Value> () <=
                           static_cast<uint64_t> (std::numeric_limits<int8_t>::max ());

                case RealValue:
                    return std::trunc (_data.template get<RealValue> ()) == _data.template get<RealValue> () &&
                           _data.template get<RealValue> () >=
                               static_cast<double> (std::numeric_limits<int8_t>::min ()) &&
                           _data.template get<RealValue> () <=
                               static_cast<double> (std::numeric_limits<int8_t>::max ());

                default:
                    break;
            }

            return false;
        }

        /**
         * @brief get variable held by value as a 8 bits integer.
         * @return variable held by value as a 8 bits integer.
         * @throw std::bad_cast.
         */
        constexpr int8_t getInt8 () const
        {
            if (isInt8 ())
            {
                switch (_data.index ())
                {
                    case IntegerValue:
                        return static_cast<int8_t> (_data.template get<IntegerValue> ());

                    case UnsignedValue:
                        return static_cast<int8_t> (_data.template get<UnsignedValue> ());

                    case Integer64Value:
                        return static_cast<int8_t> (_data.template get<Integer64Value> ());

                    case Unsigned64Value:
                        return static_cast<int8_t> (_data.template get<Unsigned64Value> ());

                    default:
                        return static_cast<int8_t> (_data.template get<RealValue> ());
                }
            }

            throwException<std::bad_cast> ();
        }

        /**
         * @brief explicit conversion function for 8 bits integer value.
         * @return converted char value.
         * @throw std::bad_cast.
         */
        explicit operator int8_t () const
        {
            return getInt8 ();
        }

        /**
         * @brief check if the variable held by value is a 8 bits unsigned integer value.
         * @return true if the variable held by value is a 8 bits unsigned integer value, false otherwise.
         */
        constexpr bool isUint8 () const
        {
            switch (_data.index ())
            {
                case IntegerValue:
                    return _data.template get<IntegerValue> () >= 0 &&
                           _data.template get<IntegerValue> () <=
                               static_cast<int32_t> (std::numeric_limits<uint8_t>::max ());

                case UnsignedValue:
                    return _data.template get<UnsignedValue> () <=
                           static_cast<uint32_t> (std::numeric_limits<uint8_t>::max ());

                case Integer64Value:
                    return _data.template get<Integer64Value> () >= 0 &&
                           _data.template get<Integer64Value> () <=
                               static_cast<int64_t> (std::numeric_limits<uint8_t>::max ());

                case Unsigned64Value:
                    return _data.template get<Unsigned64Value> () <=
                           static_cast<uint64_t> (std::numeric_limits<uint8_t>::max ());

                case RealValue:
                    return std::trunc (_data.template get<RealValue> ()) == _data.template get<RealValue> () &&
                           _data.template get<RealValue> () >= 0 &&
                           _data.template get<RealValue> () <=
                               static_cast<double> (std::numeric_limits<uint8_t>::max ());

                default:
                    break;
            }

            return false;
        }

        /**
         * @brief get variable held by value as a 8 bits unsigned integer.
         * @return variable held by value as a 8 bits unsigned integer.
         * @throw std::bad_cast.
         */
        constexpr uint8_t getUint8 () const
        {
            if (isUint8 ())
            {
                switch (_data.index ())
                {
                    case IntegerValue:
                        return static_cast<uint8_t> (_data.template get<IntegerValue> ());

                    case UnsignedValue:
                        return static_cast<uint8_t> (_data.template get<UnsignedValue> ());

                    case Integer64Value:
                        return static_cast<uint8_t> (_data.template get<Integer64Value> ());

                    case Unsigned64Value:
                        return static_cast<uint8_t> (_data.template get<Unsigned64Value> ());

                    default:
                        return static_cast<uint8_t> (_data.template get<RealValue> ());
                }
            }

            throwException<std::bad_cast> ();
        }

        /**
         * @brief explicit conversion function for 8 bits unsigned integer value.
         * @return converted unsihed char value.
         * @throw std::bad_cast.
         */
        explicit operator uint8_t () const
        {
            return getUint8 ();
        }

        /**
         * @brief check if the variable held by value is a 16 bits integer value.
         * @return true if the variable held by value is a 16 bits integer value, false otherwise.
         */
        constexpr bool isInt16 () const
        {
            switch (_data.index ())
            {
                case IntegerValue:
                    return _data.template get<IntegerValue> () >=
                               static_cast<int32_t> (std::numeric_limits<int16_t>::min ()) &&
                           _data.template get<IntegerValue> () <=
                               static_cast<int32_t> (std::numeric_limits<int16_t>::max ());

                case UnsignedValue:
                    return _data.template get<UnsignedValue> () <=
                           static_cast<uint32_t> (std::numeric_limits<int16_t>::max ());

                case Integer64Value:
                    return _data.template get<Integer64Value> () >=
                               static_cast<int64_t> (std::numeric_limits<int16_t>::min ()) &&
                           _data.template get<Integer64Value> () <=
                               static_cast<int64_t> (std::numeric_limits<int16_t>::max ());

                case Unsigned64Value:
                    return _data.template get<Unsigned64Value> () <=
                           static_cast<uint64_t> (std::numeric_limits<int16_t>::max ());

                case RealValue:
                    return std::trunc (_data.template get<RealValue> ()) == _data.template get<RealValue> () &&
                           _data.template get<RealValue> () >=
                               static_cast<double> (std::numeric_limits<int16_t>::min ()) &&
                           _data.template get<RealValue> () <=
                               static_cast<double> (std::numeric_limits<int16_t>::max ());

                default:
                    break;
            }

            return false;
        }

        /**
         * @brief get variable held by value as a 16 bits integer.
         * @return variable held by value as a 16 bits integer.
         * @throw std::bad_cast.
         */
        constexpr int16_t getInt16 () const
        {
            if (isInt16 ())
            {
                switch (_data.index ())
                {
                    case IntegerValue:
                        return static_cast<int16_t> (_data.template get<IntegerValue> ());

                    case UnsignedValue:
                        return static_cast<int16_t> (_data.template get<UnsignedValue> ());

                    case Integer64Value:
                        return static_cast<int16_t> (_data.template get<Integer64Value> ());

                    case Unsigned64Value:
                        return static_cast<int16_t> (_data.template get<Unsigned64Value> ());

                    default:
                        return static_cast<int16_t> (_data.template get<RealValue> ());
                }
            }

            throwException<std::bad_cast> ();
        }

        /**
         * @brief explicit conversion function for 16 bits integer value.
         * @return converted short value.
         * @throw std::bad_cast.
         */
        explicit operator int16_t () const
        {
            return getInt16 ();
        }

        /**
         * @brief check if the variable held by value is a 16 bits unsigned integer value.
         * @return true if the variable held by value is a 16 bits unsigned integer value, false otherwise.
         */
        constexpr bool isUint16 () const
        {
            switch (_data.index ())
            {
                case IntegerValue:
                    return _data.template get<IntegerValue> () >= 0 &&
                           _data.template get<IntegerValue> () <=
                               static_cast<int32_t> (std::numeric_limits<uint16_t>::max ());

                case UnsignedValue:
                    return _data.template get<UnsignedValue> () <=
                           static_cast<uint32_t> (std::numeric_limits<uint16_t>::max ());

                case Integer64Value:
                    return _data.template get<Integer64Value> () >= 0 &&
                           _data.template get<Integer64Value> () <=
                               static_cast<int64_t> (std::numeric_limits<uint16_t>::max ());

                case Unsigned64Value:
                    return _data.template get<Unsigned64Value> () <=
                           static_cast<uint64_t> (std::numeric_limits<uint16_t>::max ());

                case RealValue:
                    return std::trunc (_data.template get<RealValue> ()) == _data.template get<RealValue> () &&
                           _data.template get<RealValue> () >= 0 &&
                           _data.template get<RealValue> () <=
                               static_cast<double> (std::numeric_limits<uint16_t>::max ());

                default:
                    break;
            }

            return false;
        }

        /**
         * @brief get variable held by value as a 16 bits unsigned integer.
         * @return variable held by value as a 16 bits unsigned integer.
         * @throw std::bad_cast.
         */
        constexpr uint16_t getUint16 () const
        {
            if (isUint16 ())
            {
                switch (_data.index ())
                {
                    case IntegerValue:
                        return static_cast<uint16_t> (_data.template get<IntegerValue> ());

                    case UnsignedValue:
                        return static_cast<uint16_t> (_data.template get<UnsignedValue> ());

                    case Integer64Value:
                        return static_cast<uint16_t> (_data.template get<Integer64Value> ());

                    case Unsigned64Value:
                        return static_cast<uint16_t> (_data.template get<Unsigned64Value> ());

                    default:
                        return static_cast<uint16_t> (_data.template get<RealValue> ());
                }
            }

            throwException<std::bad_cast> ();
        }

        /**
         * @brief explicit conversion function for 16 bits unsigned integer value.
         * @return converted unsigned short value.
         * @throw std::bad_cast.
         */
        explicit operator uint16_t () const
        {
            return getUint16 ();
        }

        /**
         * @brief check if the variable held by value is a 32 bits integer value.
         * @return true if the variable held by value is a 32 bits integer value, false otherwise.
         */
        constexpr bool isInt () const
        {
            switch (_data.index ())
            {
                case IntegerValue:
                    return true;

                case UnsignedValue:
                    return _data.template get<UnsignedValue> () <=
                           static_cast<uint32_t> (std::numeric_limits<int32_t>::max ());

                case Integer64Value:
                    return _data.template get<Integer64Value> () >=
                               static_cast<int64_t> (std::numeric_limits<int32_t>::min ()) &&
                           _data.template get<Integer64Value> () <=
                               static_cast<int64_t> (std::numeric_limits<int32_t>::max ());

                case Unsigned64Value:
                    return _data.template get<Unsigned64Value> () <=
                           static_cast<uint64_t> (std::numeric_limits<int32_t>::max ());

                case RealValue:
                    return std::trunc (_data.template get<RealValue> ()) == _data.template get<RealValue> () &&
                           _data.template get<RealValue> () >=
                               static_cast<double> (std::numeric_limits<int32_t>::min ()) &&
                           _data.template get<RealValue> () <=
                               static_cast<double> (std::numeric_limits<int32_t>::max ());

                default:
                    break;
            }

            return false;
        }

        /**
         * @brief get variable held by value as a 32 bits integer.
         * @return variable held by value as a 32 bits integer.
         * @throw std::bad_cast.
         */
        constexpr int32_t getInt () const
        {
            if (_data.index () == IntegerValue)
            {
                return _data.template get<IntegerValue> ();
            }

            return convertInt ();
        }

        /**
         * @brief explicit conversion function for 32 bits integer value.
         * @return converted integer value.
         * @throw std::bad_cast.
         */
        explicit operator int32_t () const
        {
            return getInt ();
        }

        /**
         * @brief check if the variable held by value is a 32 bits unsigned integer value.
         * @return true if the variable held by value is a 32 bits unsigned integer value, false otherwise.
         */
        constexpr bool isUint () const
        {
            switch (_data.index ())
            {
                case IntegerValue:
                    return _data.template get<IntegerValue> () >= 0;

                case UnsignedValue:
                    return true;

                case Integer64Value:
                    return _data.template get<Integer64Value> () >= 0 &&
                           _data.template get<Integer64Value> () <=
                               static_cast<int64_t> (std::numeric_limits<uint32_t>::max ());

                case Unsigned64Value:
                    return _data.template get<Unsigned64Value> () <=
                           static_cast<uint64_t> (std::numeric_limits<uint32_t>::max ());

                case RealValue:
                    return std::trunc (_data.template get<RealValue> ()) == _data.template get<RealValue> () &&
                           _data.template get<RealValue> () >= 0 &&
                           _data.template get<RealValue> () <=
                               static_cast<double> (std::numeric_limits<uint32_t>::max ());

                default:
                    break;
            }

            return false;
        }

        /**
         * @brief get variable held by value as a 32 bits unsigned integer value.
         * @return variable held by value as a 32 bits unsigned integer value.
         * @throw std::bad_cast.
         */
        constexpr uint32_t getUint () const
        {
            if (_data.index () == UnsignedValue)
            {
                return _data.template get<UnsignedValue> ();
            }

            return convertUint ();
        }

        /**
         * @brief explicit conversion function for 32 bits unsigned integer value.
         * @return converted unsigned integer value.
         * @throw std::bad_cast.
         */
        explicit operator uint32_t () const
        {
            return getUint ();
        }

        /**
         * @brief check if the variable held by value is a 64 bits integer value.
         * @return true if the variable held by value is a 64 bits integer value, false otherwise.
         */
        constexpr bool isInt64 () const
        {
            switch (_data.index ())
            {
                case IntegerValue:
                case UnsignedValue:
                case Integer64Value:
                    return true;

                case Unsigned64Value:
                    return _data.template get<Unsigned64Value> () <=
                           static_cast<uint64_t> (std::numeric_limits<int64_t>::max ());

                case RealValue:
                    return std::trunc (_data.template get<RealValue> ()) == _data.template get<RealValue> () &&
                           _data.template get<RealValue> () >=
                               static_cast<double> (std::numeric_limits<int64_t>::min ()) &&
                           _data.template get<RealValue> () <
                               static_cast<double> (std::numeric_limits<int64_t>::max ());

                default:
                    break;
            }

            return false;
        }

        /**
         * @brief get variable held by value as a 64 bits integer value.
         * @return variable held by value as a 64 bits integer value.
         * @throw std::bad_cast.
         */
        constexpr int64_t getInt64 () const
        {
            if (_data.index () == Integer64Value)
            {
                return _data.template get<Integer64Value> ();
            }

            return convertInt64 ();
        }

        /**
         * @brief explicit conversion function for 64 bits integer value.
         * @return converted integer 64 bits value.
         * @throw std::bad_cast.
         */
        explicit operator int64_t () const
        {
            return getInt64 ();
        }

        /**
         * @brief check if the variable held by value is a 64 bits unsigned integer value.
         * @return true if the variable held by value is a 64 bits unsigned integer value, false otherwise.
         */
        constexpr bool isUint64 () const
        {
            switch (_data.index ())
            {
                case IntegerValue:
                    return _data.template get<IntegerValue> () >= 0;

                case UnsignedValue:
                    return true;

                case Integer64Value:
                    return _data.template get<Integer64Value> () >= 0;

                case Unsigned64Value:
                    return true;

                case RealValue:
                    return std::trunc (_data.template get<RealValue> ()) == _data.template get<RealValue> () &&
                           _data.template get<RealValue> () >= 0 &&
                           _data.template get<RealValue> () <
                               static_cast<double> (std::numeric_limits<uint64_t>::max ());

                default:
                    break;
            }

            return false;
        }

        /**
         * @brief get variable held by value as a 64 bits unsigned integer value.
         * @return variable held by value as a 64 bits unsigned integer value.
         * @throw std::bad_cast.
         */
        constexpr uint64_t getUint64 () const
        {
            if (_data.index () == Unsigned64Value)
            {
                return _data.template get<Unsigned64Value> ();
            }

            return convertUint64 ();
        }

        /**
         * @brief explicit conversion function for 64 bits unsigned integer value.
         * @return converted 64 bits unsigned integer value.
         * @throw std::bad_cast.
         */
        explicit operator uint64_t () const
        {
            return getUint64 ();
        }

        /**
         * @brief check if the variable held by value is a float value.
         * @return true if the variable held by value is a float value, false otherwise.
         */
        constexpr bool isFloat () const
        {
            return isNumber ();
        }

        /**
         * @brief get variable held by value as a float value.
         * @return variable held by value as a float value.
         * @throw std::bad_cast.
         */
        constexpr float getFloat () const
        {
            if (_data.index () == RealValue)
            {
                return static_cast<float> (_data.template get<RealValue> ());
            }

            return convertFloat ();
        }

        /**
         * @brief explicit conversion function for float value.
         * @return converted float value.
         * @throw std::bad_cast.
         */
        explicit operator float () const
        {
            return getFloat ();
        }

        /**
         * @brief check if the variable held by value is a double value.
         * @return true if the variable held by value is a double value, false otherwise.
         */
        constexpr bool isDouble () const
        {
            return isNumber ();
        }

        /**
         * @brief get variable held by value as a double value.
         * @return variable held by value as a double value.
         * @throw std::bad_cast.
         */
        constexpr double getDouble () const
        {
            if (_data.index () == RealValue)
            {
                return _data.template get<RealValue> ();
            }

            return convertDouble ();
        }

        /**
         * @brief explicit conversion function for double value.
         * @return converted double value.
         * @throw std::bad_cast.
         */
        explicit operator double () const
        {
            return getDouble ();
        }

        /**
         * @brief check if the variable held by value is a string value.
         * @return true if the variable held by value is a string value, false otherwise.
         */
        constexpr bool isString () const
        {
            return _data.index () == StringValue;
        }

        /**
         * @brief get variable held by value as a string value.
         * @return view of a null-terminated string.
         * @throw std::bad_cast.
         */
        constexpr std::string_view getString () const
        {
            const String& value = _data.template get<StringValue> ();
            return std::string_view (value.data (), value.size ());
        }

        /**
         * @brief explicit conversion function for string value.
         * @return converted string value.
         * @throw std::bad_cast.
         */
        explicit operator std::string_view () const
        {
            return getString ();
        }

        /**
         * @brief check if the variable held by value is a binary value.
         * @return true if the variable held by value is a binary value, false otherwise.
         */
        constexpr bool isBinary () const
        {
            return _data.index () == BinaryValue;
        }

        /**
         * @brief get variable held by value as a binary value.
         * @return variable held by value as a binary value.
         * @throw std::bad_cast.
         */
        constexpr std::span<const uint8_t> getBinary () const
        {
            const Binary& value = _data.template get<BinaryValue> ();
            return std::span<const uint8_t> (value.data (), value.size ());
        }

        /**
         * @brief check if the variable held by value is an array.
         * @return true if the variable held by value is an array, false otherwise.
         */
        constexpr bool isArray () const
        {
            return _data.index () == ArrayValue;
        }

        /**
         * @brief get variable held by value as an array.
         * @return variable held by value as an array.
         * @throw std::bad_cast.
         */
        constexpr const Array& getArray () const
        {
            return _data.template get<ArrayValue> ();
        }

        /**
         * @brief check if the variable held by value is an object.
         * @return true if the variable held by value is an object, false otherwise.
         */
        constexpr bool isObject () const
        {
            return _data.index () == ObjectValue;
        }

        /**
         * @brief get variable held by value as an object.
         * @return variable held by value as an object.
         * @throw std::bad_cast.
         */
        constexpr const Object& getObject () const
        {
            return _data.template get<ObjectValue> ();
        }

        /**
         * @brief returns a reference to the value mapped to key.
         * @param key key of the element to find.
         * @return a reference to the requested element.
         * @throw std::bad_cast, std::out_of_range.
         */
        Value& at ([[maybe_unused]] std::nullptr_t key)
        {
            for (auto& member : _data.template get<ObjectValue> ())
            {
                if (member.first.isNull ())
                {
                    return member.second;
                }
            }

            throwException<std::out_of_range> ("invalid key");
        }

        /**
         * @brief returns a reference to the value mapped to key.
         * @param key key of the element to find.
         * @return a reference to the requested element.
         * @throw std::bad_cast, std::out_of_range.
         */
        Value& at (bool key)
        {
            for (auto& member : _data.template get<ObjectValue> ())
            {
                if (member.first.isBool () && member.first.getBool () == key)
                {
                    return member.second;
                }
            }

            throwException<std::out_of_range> ("invalid key");
        }

        /**
         * @brief returns a reference to the value at position pos or mapped to key.
         * @param key position or key of the element to find.
         * @return a reference to the requested element.
         * @throw std::bad_cast, std::out_of_range.
         */
        Value& at (int32_t key)
        {
            if (_data.index () == ArrayValue)
            {
                return elementAt (key);
            }

            for (auto& member : _data.template get<ObjectValue> ())
            {
                if (member.first.isInt () && member.first.getInt () == key)
                {
                    return member.second;
                }
            }

            throwException<std::out_of_range> ("invalid key");
        }

        /**
         * @brief returns a reference to the value at position pos or mapped to key.
         * @param key position or key of the element to find.
         * @return a reference to the requested element.
         * @throw std::bad_cast, std::out_of_range.
         */
        Value& at (uint32_t key)
        {
            if (_data.index () == ArrayValue)
            {
                return elementAt (key);
            }

            for (auto& member : _data.template get<ObjectValue> ())
            {
                if (member.first.isUint () && member.first.getUint () == key)
                {
                    return member.second;
                }
            }

            throwException<std::out_of_range> ("invalid key");
        }

        /**
         * @brief returns a reference to the value at position pos or mapped to key.
         * @param key position or key of the element to find.
         * @return a reference to the requested element.
         * @throw std::bad_cast, std::out_of_range.
         */
        Value& at (int64_t key)
        {
            if (_data.index () == ArrayValue)
            {
                return elementAt (key);
            }

            for (auto& member : _data.template get<ObjectValue> ())
            {
                if (member.first.isInt64 () && member.first.getInt64 () == key)
                {
                    return member.second;
                }
            }

            throwException<std::out_of_range> ("invalid key");
        }

        /**
         * @brief returns a reference to the value at position pos or mapped to key.
         * @param key position or key of the element to find.
         * @return a reference to the requested element.
         * @throw std::bad_cast, std::out_of_range.
         */
        Value& at (uint64_t key)
        {
            if (_data.index () == ArrayValue)
            {
                return elementAt (key);
            }

            for (auto& member : _data.template get<ObjectValue> ())
            {
                if (member.first.isUint64 () && member.first.getUint64 () == key)
                {
                    return member.second;
                }
            }

            throwException<std::out_of_range> ("invalid key");
        }

        /**
         * @brief returns a reference to the value mapped to key.
         * @param key key of the element to find.
         * @return a reference to the requested element.
         * @throw std::bad_cast, std::out_of_range.
         */
        Value& at (double key)
        {
            for (auto& member : _data.template get<ObjectValue> ())
            {
                if (member.first.isDouble () && member.first.getDouble () == key)
                {
                    return member.second;
                }
            }

            throwException<std::out_of_range> ("invalid key");
        }

        /**
         * @brief returns a reference to the value mapped to key.
         * @param key key of the element to find.
         * @return a reference to the requested element.
         * @throw std::bad_cast, std::out_of_range.
         */
        Value& at (std::string_view key)
        {
            for (auto& member : _data.template get<ObjectValue> ())
            {
                if (member.first.isString () && member.first.getString () == key)
                {
                    return member.second;
                }
            }

            throwException<std::out_of_range> ("invalid key");
        }

        /**
         * @brief returns a reference to the value mapped to key.
         * @param key key of the element to find.
         * @return a reference to the requested element.
         * @throw std::bad_cast, std::out_of_range.
         */
        Value& at (const char* key)
        {
            return at (std::string_view (key));
        }

        /**
         * @brief returns a reference to the value at position pos or mapped to key.
         * @param key position or key of the element to find.
         * @return a reference to the requested element.
         * @throw std::bad_cast, std::out_of_range.
         */
        template <typename Key>
            requires std::same_as<Key, BasicValue>
        Value& at (const Key& key)
        {
            if (_data.index () == ArrayValue)
            {
                return elementAt (key.getUint64 ());
            }

            for (auto& member : _data.template get<ObjectValue> ())
            {
                if (member.first == key)
                {
                    return member.second;
                }
            }

            throwException<std::out_of_range> ("invalid key");
        }

        /**
         * @brief returns a reference to the value mapped to key.
         * @param key key of the element to find.
         * @return a reference to the requested element.
         * @throw std::bad_cast, std::out_of_range.
         */
        const Value& at ([[maybe_unused]] std::nullptr_t key) const
        {
            for (auto const& member : _data.template get<ObjectValue> ())
            {
                if (member.first.isNull ())
                {
                    return member.second;
                }
            }

            throwException<std::out_of_range> ("invalid key");
        }

        /**
         * @brief returns a reference to the value mapped to key.
         * @param key key of the element to find.
         * @return a reference to the requested element.
         * @throw std::bad_cast, std::out_of_range.
         */
        const Value& at (bool key) const
        {
            for (auto const& member : _data.template get<ObjectValue> ())
            {
                if (member.first.isBool () && member.first.getBool () == key)
                {
                    return member.second;
                }
            }

            throwException<std::out_of_range> ("invalid key");
        }

        /**
         * @brief returns a reference to the value at position pos or mapped to key.
         * @param key position or key of the element to find.
         * @return a reference to the requested element.
         * @throw std::bad_cast, std::out_of_range.
         */
        const Value& at (int32_t key) const
        {
            if (_data.index () == ArrayValue)
            {
                return elementAt (key);
            }

            for (auto const& member : _data.template get<ObjectValue> ())
            {
                if (member.first.isInt () && member.first.getInt () == key)
                {
                    return member.second;
                }
            }

            throwException<std::out_of_range> ("invalid key");
        }

        /**
         * @brief returns a reference to the value at position pos or mapped to key.
         * @param key position or key of the element to find.
         * @return a reference to the requested element.
         * @throw std::bad_cast, std::out_of_range.
         */
        const Value& at (uint32_t key) const
        {
            if (_data.index () == ArrayValue)
            {
                return elementAt (key);
            }

            for (auto const& member : _data.template get<ObjectValue> ())
            {
                if (member.first.isUint () && member.first.getUint () == key)
                {
                    return member.second;
                }
            }

            throwException<std::out_of_range> ("invalid key");
        }

        /**
         * @brief returns a reference to the value at position pos or mapped to key.
         * @param key position or key of the element to find.
         * @return a reference to the requested element.
         * @throw std::bad_cast, std::out_of_range.
         */
        const Value& at (int64_t key) const
        {
            if (_data.index () == ArrayValue)
            {
                return elementAt (key);
            }

            for (auto const& member : _data.template get<ObjectValue> ())
            {
                if (member.first.isInt64 () && member.first.getInt64 () == key)
                {
                    return member.second;
                }
            }

            throwException<std::out_of_range> ("invalid key");
        }

        /**
         * @brief returns a reference to the value at position pos or mapped to key.
         * @param key position or key of the element to find.
         * @return a reference to the requested element.
         * @throw std::bad_cast, std::out_of_range.
         */
        const Value& at (uint64_t key) const
        {
            if (_data.index () == ArrayValue)
            {
                return elementAt (key);
            }

            for (auto const& member : _data.template get<ObjectValue> ())
            {
                if (member.first.isUint64 () && member.first.getUint64 () == key)
                {
                    return member.second;
                }
            }

            throwException<std::out_of_range> ("invalid key");
        }

        /**
         * @brief returns a reference to the value mapped to key.
         * @param key key of the element to find.
         * @return a reference to the requested element.
         * @throw std::bad_cast, std::out_of_range.
         */
        const Value& at (double key) const
        {
            for (auto const& member : _data.template get<ObjectValue> ())
            {
                if (member.first.isDouble () && member.first.getDouble () == key)
                {
                    return member.second;
                }
            }

            throwException<std::out_of_range> ("invalid key");
        }

        /**
         * @brief returns a reference to the value mapped to key.
         * @param key key of the element to find.
         * @return a reference to the requested element.
         * @throw std::bad_cast, std::out_of_range.
         */
        const Value& at (std::string_view key) const
        {
            for (auto const& member : _data.template get<ObjectValue> ())
            {
                if (member.first.isString () && member.first.getString () == key)
                {
                    return member.second;
                }
            }

            throwException<std::out_of_range> ("invalid key");
        }

        /**
         * @brief returns a reference to the value mapped to key.
         * @param key key of the element to find.
         * @return a reference to the requested element.
         * @throw std::bad_cast, std::out_of_range.
         */
        const Value& at (const char* key) const
        {
            return at (std::string_view (key));
        }

        /**
         * @brief returns a reference to the value at position pos or mapped to key.
         * @param key position or key of the element to find.
         * @return a reference to the requested element.
         * @throw std::bad_cast, std::out_of_range.
         */
        template <typename Key>
            requires std::same_as<Key, BasicValue>
        const Value& at (const Key& key) const
        {
            if (_data.index () == ArrayValue)
            {
                return elementAt (key.getUint64 ());
            }

            for (auto const& member : _data.template get<ObjectValue> ())
            {
                if (member.first == key)
                {
                    return member.second;
                }
            }

            throwException<std::out_of_range> ("invalid key");
        }

        /**
         * @brief returns a reference to the value mapped to key.
         * @param key key of the element to find.
         * @return a reference to the requested element.
         * @throw std::bad_cast.
         */
        Value& operator[] (std::nullptr_t key)
        {
            if (_data.index () == NullValue)
            {
                _data.template set<ObjectValue> ();
            }

            for (auto& member : _data.template get<ObjectValue> ())
            {
                if (member.first.isNull ())
                {
                    return member.second;
                }
            }

            return appendMember (key);
        }

        /**
         * @brief returns a reference to the value mapped to key.
         * @param key key of the element to find.
         * @return a reference to the requested element.
         * @throw std::bad_cast.
         */
        Value& operator[] (bool key)
        {
            if (_data.index () == NullValue)
            {
                _data.template set<ObjectValue> ();
            }

            for (auto& member : _data.template get<ObjectValue> ())
            {
                if (member.first.isBool () && member.first.getBool () == key)
                {
                    return member.second;
                }
            }

            return appendMember (key);
        }

        /**
         * @brief returns a reference to the value at position pos or mapped to key.
         * @param key position or key of the element to find.
         * @return a reference to the requested element.
         * @throw std::bad_cast.
         */
        Value& operator[] (int32_t key)
        {
            if (_data.index () == ArrayValue)
            {
                return _data.template get<ArrayValue> ()[key];
            }

            if (_data.index () == NullValue)
            {
                _data.template set<ObjectValue> ();
            }

            for (auto& member : _data.template get<ObjectValue> ())
            {
                if (member.first.isInt () && member.first.getInt () == key)
                {
                    return member.second;
                }
            }

            return appendMember (key);
        }

        /**
         * @brief returns a reference to the value at position pos or mapped to key.
         * @param key position or key of the element to find.
         * @return a reference to the requested element.
         * @throw std::bad_cast.
         */
        Value& operator[] (uint32_t key)
        {
            if (_data.index () == ArrayValue)
            {
                return _data.template get<ArrayValue> ()[key];
            }

            if (_data.index () == NullValue)
            {
                _data.template set<ObjectValue> ();
            }

            for (auto& member : _data.template get<ObjectValue> ())
            {
                if (member.first.isUint () && member.first.getUint () == key)
                {
                    return member.second;
                }
            }

            return appendMember (key);
        }

        /**
         * @brief returns a reference to the value at position pos or mapped to key.
         * @param key position or key of the element to find.
         * @return a reference to the requested element.
         * @throw std::bad_cast.
         */
        Value& operator[] (int64_t key)
        {
            if (_data.index () == ArrayValue)
            {
                return _data.template get<ArrayValue> ()[key];
            }

            if (_data.index () == NullValue)
            {
                _data.template set<ObjectValue> ();
            }

            for (auto& member : _data.template get<ObjectValue> ())
            {
                if (member.first.isInt64 () && member.first.getInt64 () == key)
                {
                    return member.second;
                }
            }

            return appendMember (key);
        }

        /**
         * @brief returns a reference to the value at position pos or mapped to key.
         * @param key position or key of the element to find.
         * @return a reference to the requested element.
         * @throw std::bad_cast.
         */
        Value& operator[] (uint64_t key)
        {
            if (_data.index () == ArrayValue)
            {
                return _data.template get<ArrayValue> ()[key];
            }

            if (_data.index () == NullValue)
            {
                _data.template set<ObjectValue> ();
            }

            for (auto& member : _data.template get<ObjectValue> ())
            {
                if (member.first.isUint64 () && member.first.getUint64 () == key)
                {
                    return member.second;
                }
            }

            return appendMember (key);
        }

        /**
         * @brief returns a reference to the value mapped to key.
         * @param key key of the element to find.
         * @return a reference to the requested element.
         * @throw std::bad_cast.
         */
        Value& operator[] (double key)
        {
            if (_data.index () == NullValue)
            {
                _data.template set<ObjectValue> ();
            }

            for (auto& member : _data.template get<ObjectValue> ())
            {
                if (member.first.isDouble () && member.first.getDouble () == key)
                {
                    return member.second;
                }
            }

            return appendMember (key);
        }

        /**
         * @brief returns a reference to the value mapped to key.
         * @param key key of the element to find.
         * @return a reference to the requested element.
         * @throw std::bad_cast.
         */
        Value& operator[] (std::string_view key)
        {
            if (_data.index () == NullValue)
            {
                _data.template set<ObjectValue> ();
            }

            for (auto& member : _data.template get<ObjectValue> ())
            {
                if (member.first.isString () && member.first.getString () == key)
                {
                    return member.second;
                }
            }

            return appendMember (key);
        }

        /**
         * @brief returns a reference to the value mapped to key.
         * @param key key of the element to find.
         * @return a reference to the requested element.
         * @throw std::bad_cast.
         */
        Value& operator[] (const char* key)
        {
            return operator[] (std::string_view (key));
        }

        /**
         * @brief returns a reference to the value at position pos or mapped to key.
         * @param key position or key of the element to find.
         * @return a reference to the requested element.
         * @throw std::bad_cast.
         */
        template <typename Key>
            requires std::same_as<Key, BasicValue>
        Value& operator[] (const Key& key)
        {
            if (_data.index () == ArrayValue)
            {
                return _data.template get<ArrayValue> ()[key.getUint64 ()];
            }

            if (_data.index () == NullValue)
            {
                _data.template set<ObjectValue> ();
            }

            for (auto& member : _data.template get<ObjectValue> ())
            {
                if (member.first == key)
                {
                    return member.second;
                }
            }

            return appendMember (Value (key, *allocator ()), Value (*allocator ()));
        }

        /**
         * @brief check if the nested container is empty.
         * @return true if empty, false otherwise.
         * @throw std::bad_cast.
         */
        bool empty () const
        {
            switch (_data.index ())
            {
                case StringValue:
                    return _data.template get<StringValue> ().empty ();

                case ArrayValue:
                    return _data.template get<ArrayValue> ().empty ();

                case ObjectValue:
                    return _data.template get<ObjectValue> ().empty ();

                default:
                    break;
            }

            throwException<std::bad_cast> ();
        }

        /**
         * @brief returns the number of elements in the nested container.
         * @return the number of elements in the nested container.
         * @throw std::bad_cast.
         */
        size_t size () const
        {
            switch (_data.index ())
            {
                case StringValue:
                    return _data.template get<StringValue> ().size ();

                case ArrayValue:
                    return _data.template get<ArrayValue> ().size ();

                case ObjectValue:
                    return _data.template get<ObjectValue> ().size ();

                default:
                    break;
            }

            throwException<std::bad_cast> ();
        }

        /**
         * @brief increase the capacity of the nested container or string.
         * @param cap new capacity.
         * @throw std::bad_cast.
         */
        void reserve (size_t cap)
        {
            switch (_data.index ())
            {
                case StringValue:
                    if (cap > std::max<size_t> (_capacity, getString ().size ()))
                    {
                        if (JOIN_UNLIKELY (cap >= std::numeric_limits<uint32_t>::max ()))
                        {
                            throwException<std::bad_alloc> ();
                        }
                        std::string_view str = getString ();
                        char* data = allocate<char> (cap + 1);
                        std::copy_n (str.data (), str.size () + 1, data);
                        _data.template set<StringValue> (data, str.size ());
                        _capacity = static_cast<uint32_t> (cap);
                    }
                    return;

                case ArrayValue:
                    grow<Value> (cap);
                    return;

                case ObjectValue:
                    grow<Member> (cap);
                    return;

                default:
                    break;
            }

            throwException<std::bad_cast> ();
        }

        /**
         * @brief erases all elements in the nested container.
         * @throw std::bad_cast.
         */
        void clear ()
        {
            switch (_data.index ())
            {
                case StringValue:
                    *this = std::string_view ();
                    return;

                case ArrayValue:
                    _data.template set<ArrayValue> (_data.template get<ArrayValue> ().data (), 0);
                    return;

                case ObjectValue:
                    _data.template set<ObjectValue> (_data.template get<ObjectValue> ().data (), 0);
                    return;

                default:
                    break;
            }

            throwException<std::bad_cast> ();
        }

        /**
         * @brief insert element in the nested container.
         * @param member the element to insert in the nested container.
         * @return a reference to the inserted element.
         * @throw std::bad_cast.
         */
        template <typename M>
            requires std::same_as<M, Member>
        Value& insert (const M& member)
        {
            return appendMember (Value (member.first, *allocator ()), Value (member.second, *allocator ()));
        }

        /**
         * @brief insert a member built with the allocator of the tree.
         * @param key key of the member.
         * @param value value of the member.
         * @return a reference to the inserted element.
         * @throw std::bad_cast.
         */
        template <typename K, typename T>
        Value& insert (K&& key, T&& value)
        {
            return appendMember (Value (std::forward<K> (key), *allocator ()),
                                 Value (std::forward<T> (value), *allocator ()));
        }

        /**
         * @brief removes member with the key equivalent to key.
         * @param key key of the member to remove.
         * @return number of elements removed.
         * @throw std::bad_cast.
         */
        size_t erase ([[maybe_unused]] std::nullptr_t key)
        {
            auto beg = _data.template get<ObjectValue> ().begin ();
            auto end = _data.template get<ObjectValue> ().end ();

            for (auto it = beg; it != end; ++it)
            {
                if (it->first.isNull ())
                {
                    eraseMember (it);
                    return 1;
                }
            }

            return 0;
        }

        /**
         * @brief removes member with the key equivalent to key.
         * @param key key of the member to remove.
         * @return number of elements removed.
         * @throw std::bad_cast.
         */
        size_t erase (bool key)
        {
            auto beg = _data.template get<ObjectValue> ().begin ();
            auto end = _data.template get<ObjectValue> ().end ();

            for (auto it = beg; it != end; ++it)
            {
                if (it->first.isBool () && it->first.getBool () == key)
                {
                    eraseMember (it);
                    return 1;
                }
            }

            return 0;
        }

        /**
         * @brief removes member with the key equivalent to key.
         * @param key key of the member to remove.
         * @return number of elements removed.
         * @throw std::bad_cast.
         */
        size_t erase (int32_t key)
        {
            auto beg = _data.template get<ObjectValue> ().begin ();
            auto end = _data.template get<ObjectValue> ().end ();

            for (auto it = beg; it != end; ++it)
            {
                if (it->first.isInt () && it->first.getInt () == key)
                {
                    eraseMember (it);
                    return 1;
                }
            }

            return 0;
        }

        /**
         * @brief removes member with the key equivalent to key.
         * @param key key of the member to remove.
         * @return number of elements removed.
         * @throw std::bad_cast.
         */
        size_t erase (uint32_t key)
        {
            auto beg = _data.template get<ObjectValue> ().begin ();
            auto end = _data.template get<ObjectValue> ().end ();

            for (auto it = beg; it != end; ++it)
            {
                if (it->first.isUint () && it->first.getUint () == key)
                {
                    eraseMember (it);
                    return 1;
                }
            }

            return 0;
        }

        /**
         * @brief removes member with the key equivalent to key.
         * @param key key of the member to remove.
         * @return number of elements removed.
         * @throw std::bad_cast.
         */
        size_t erase (int64_t key)
        {
            auto beg = _data.template get<ObjectValue> ().begin ();
            auto end = _data.template get<ObjectValue> ().end ();

            for (auto it = beg; it != end; ++it)
            {
                if (it->first.isInt64 () && it->first.getInt64 () == key)
                {
                    eraseMember (it);
                    return 1;
                }
            }

            return 0;
        }

        /**
         * @brief removes member with the key equivalent to key.
         * @param key key of the member to remove.
         * @return number of elements removed.
         * @throw std::bad_cast.
         */
        size_t erase (uint64_t key)
        {
            auto beg = _data.template get<ObjectValue> ().begin ();
            auto end = _data.template get<ObjectValue> ().end ();

            for (auto it = beg; it != end; ++it)
            {
                if (it->first.isUint64 () && it->first.getUint64 () == key)
                {
                    eraseMember (it);
                    return 1;
                }
            }

            return 0;
        }

        /**
         * @brief removes member with the key equivalent to key.
         * @param key key of the member to remove.
         * @return number of elements removed.
         * @throw std::bad_cast.
         */
        size_t erase (double key)
        {
            auto beg = _data.template get<ObjectValue> ().begin ();
            auto end = _data.template get<ObjectValue> ().end ();

            for (auto it = beg; it != end; ++it)
            {
                if (it->first.isDouble () && it->first.getDouble () == key)
                {
                    eraseMember (it);
                    return 1;
                }
            }

            return 0;
        }

        /**
         * @brief removes member with the key equivalent to key.
         * @param key key of the member to remove.
         * @return number of elements removed.
         * @throw std::bad_cast.
         */
        size_t erase (std::string_view key)
        {
            auto beg = _data.template get<ObjectValue> ().begin ();
            auto end = _data.template get<ObjectValue> ().end ();

            for (auto it = beg; it != end; ++it)
            {
                if (it->first.isString () && it->first.getString () == key)
                {
                    eraseMember (it);
                    return 1;
                }
            }

            return 0;
        }

        /**
         * @brief removes member with the key equivalent to key.
         * @param key key of the member to remove.
         * @return number of elements removed.
         * @throw std::bad_cast.
         */
        size_t erase (const char* key)
        {
            return erase (std::string_view (key));
        }

        /**
         * @brief removes member with the key equivalent to key.
         * @param key key of the member to remove.
         * @return number of elements removed.
         * @throw std::bad_cast.
         */
        template <typename Key>
            requires std::same_as<Key, BasicValue>
        size_t erase (const Key& key)
        {
            auto beg = _data.template get<ObjectValue> ().begin ();
            auto end = _data.template get<ObjectValue> ().end ();

            for (auto it = beg; it != end; ++it)
            {
                if (it->first == key)
                {
                    eraseMember (it);
                    return 1;
                }
            }

            return 0;
        }

        /**
         * @brief constructs an element in place at the end of the nested container.
         * @param args arguments forwarded to the element constructor.
         * @return a reference to the constructed object.
         * @throw std::bad_cast.
         */
        template <typename... Args>
        Value& emplaceBack (Args&&... args)
        {
            return pushBack (Value (std::forward<Args> (args)..., *allocator ()));
        }

        /**
         * @brief appends a null value at the end of the nested container.
         * @param value the element to append.
         * @return a reference to the pushed object.
         * @throw std::bad_cast.
         */
        Value& pushBack (std::nullptr_t value)
        {
            return pushBack (Value (value, *allocator ()));
        }

        /**
         * @brief appends a boolean value at the end of the nested container.
         * @param value the element to append.
         * @return a reference to the pushed object.
         * @throw std::bad_cast.
         */
        Value& pushBack (bool value)
        {
            return pushBack (Value (value, *allocator ()));
        }

        /**
         * @brief appends a integer value at the end of the nested container.
         * @param value the element to append.
         * @return a reference to the pushed object.
         * @throw std::bad_cast.
         */
        Value& pushBack (int32_t value)
        {
            return pushBack (Value (value, *allocator ()));
        }

        /**
         * @brief appends a unsigned integer value at the end of the nested container.
         * @param value the element to append.
         * @return a reference to the pushed object.
         * @throw std::bad_cast.
         */
        Value& pushBack (uint32_t value)
        {
            return pushBack (Value (value, *allocator ()));
        }

        /**
         * @brief appends a 64 bits integer value at the end of the nested container.
         * @param value the element to append.
         * @return a reference to the pushed object.
         * @throw std::bad_cast.
         */
        Value& pushBack (int64_t value)
        {
            return pushBack (Value (value, *allocator ()));
        }

        /**
         * @brief appends a 64 bits unsigned integer value at the end of the nested container.
         * @param value the element to append.
         * @return a reference to the pushed object.
         * @throw std::bad_cast.
         */
        Value& pushBack (uint64_t value)
        {
            return pushBack (Value (value, *allocator ()));
        }

        /**
         * @brief appends a real value at the end of the nested container.
         * @param value the element to append.
         * @return a reference to the pushed object.
         * @throw std::bad_cast.
         */
        Value& pushBack (double value)
        {
            return pushBack (Value (value, *allocator ()));
        }

        /**
         * @brief appends a string value from a C-style string at the end of the nested container.
         * @param value the element to append.
         * @return a reference to the pushed object.
         * @throw std::bad_cast.
         */
        Value& pushBack (const char* value)
        {
            return pushBack (Value (value, *allocator ()));
        }

        /**
         * @brief appends a string value from a string view at the end of the nested container.
         * @param value the element to append.
         * @return a reference to the pushed object.
         * @throw std::bad_cast.
         */
        Value& pushBack (std::string_view value)
        {
            return pushBack (Value (value, *allocator ()));
        }

        /**
         * @brief appends a binary value at the end of the nested container.
         * @param value the element to append.
         * @return a reference to the pushed object.
         * @throw std::bad_cast.
         */
        Value& pushBack (std::span<const uint8_t> value)
        {
            return pushBack (Value (value, *allocator ()));
        }

        /**
         * @brief appends an array value at the end of the nested container.
         * @param value the element to append.
         * @return a reference to the pushed object.
         * @throw std::bad_cast.
         */
        Value& pushBack (Array value)
        {
            return pushBack (Value (value, *allocator ()));
        }

        /**
         * @brief appends an object value at the end of the nested container.
         * @param value the element to append.
         * @return a reference to the pushed object.
         * @throw std::bad_cast.
         */
        Value& pushBack (Object value)
        {
            return pushBack (Value (value, *allocator ()));
        }

        /**
         * @brief appends a value at the end of the nested container.
         * @param value the element to append.
         * @return a reference to the pushed object.
         * @throw std::bad_cast.
         */
        template <typename Other>
            requires std::same_as<std::remove_cvref_t<Other>, BasicValue>
        Value& pushBack (Other&& value)
        {
            Value element (*allocator ());
            element = std::forward<Other> (value);
            if (_data.index () == NullValue)
            {
                _data.template set<ArrayValue> ();
            }
            Value& slot = append<Value> ();
            ::new (&slot) Value (*_allocator);
            slot = std::move (element);
            return slot;
        }

        /**
         * @brief removes the last element of the nested container.
         * @throw std::bad_cast.
         */
        void popBack ()
        {
            Array& array = _data.template get<ArrayValue> ();
            if (array.empty ())
            {
                return;
            }
            array = Array (array.data (), array.size () - 1);
        }

        /**
         * @brief checks if the nested container contains an element that is mapped to key.
         * @param key key of the element to find.
         * @return true if the nested container contains an element mapped to key, false otherwise.
         * @throw std::bad_cast.
         */
        bool contains ([[maybe_unused]] std::nullptr_t key) const
        {
            for (auto const& member : _data.template get<ObjectValue> ())
            {
                if (member.first.isNull ())
                {
                    return true;
                }
            }

            return false;
        }

        /**
         * @brief checks if the nested container contains an element that is mapped to key.
         * @param key key of the element to find.
         * @return true if the nested container contains an element mapped to key, false otherwise.
         * @throw std::bad_cast.
         */
        bool contains (bool key) const
        {
            for (auto const& member : _data.template get<ObjectValue> ())
            {
                if (member.first.isBool () && member.first.getBool () == key)
                {
                    return true;
                }
            }

            return false;
        }

        /**
         * @brief checks if the nested container contains an element that is mapped to key.
         * @param key position or key of the element to find.
         * @return true if the nested container contains an element mapped to key, false otherwise.
         * @throw std::bad_cast.
         */
        bool contains (int32_t key) const
        {
            if (_data.index () == ArrayValue)
            {
                return key >= 0 && static_cast<uint64_t> (key) < _data.template get<ArrayValue> ().size ();
            }

            for (auto const& member : _data.template get<ObjectValue> ())
            {
                if (member.first.isInt () && member.first.getInt () == key)
                {
                    return true;
                }
            }

            return false;
        }

        /**
         * @brief checks if the nested container contains an element that is mapped to key.
         * @param key position or key of the element to find.
         * @return true if the nested container contains an element mapped to key, false otherwise.
         * @throw std::bad_cast.
         */
        bool contains (uint32_t key) const
        {
            if (_data.index () == ArrayValue)
            {
                return key < _data.template get<ArrayValue> ().size ();
            }

            for (auto const& member : _data.template get<ObjectValue> ())
            {
                if (member.first.isUint () && member.first.getUint () == key)
                {
                    return true;
                }
            }

            return false;
        }

        /**
         * @brief checks if the nested container contains an element that is mapped to key.
         * @param key position or key of the element to find.
         * @return true if the nested container contains an element mapped to key, false otherwise.
         * @throw std::bad_cast.
         */
        bool contains (int64_t key) const
        {
            if (_data.index () == ArrayValue)
            {
                return key >= 0 && static_cast<uint64_t> (key) < _data.template get<ArrayValue> ().size ();
            }

            for (auto const& member : _data.template get<ObjectValue> ())
            {
                if (member.first.isInt64 () && member.first.getInt64 () == key)
                {
                    return true;
                }
            }

            return false;
        }

        /**
         * @brief checks if the nested container contains an element that is mapped to key.
         * @param key position or key of the element to find.
         * @return true if the nested container contains an element mapped to key, false otherwise.
         * @throw std::bad_cast.
         */
        bool contains (uint64_t key) const
        {
            if (_data.index () == ArrayValue)
            {
                return key < _data.template get<ArrayValue> ().size ();
            }

            for (auto const& member : _data.template get<ObjectValue> ())
            {
                if (member.first.isUint64 () && member.first.getUint64 () == key)
                {
                    return true;
                }
            }

            return false;
        }

        /**
         * @brief checks if the nested container contains an element that is mapped to key.
         * @param key key of the element to find.
         * @return true if the nested container contains an element mapped to key, false otherwise.
         * @throw std::bad_cast.
         */
        bool contains (double key) const
        {
            for (auto const& member : _data.template get<ObjectValue> ())
            {
                if (member.first.isDouble () && member.first.getDouble () == key)
                {
                    return true;
                }
            }

            return false;
        }

        /**
         * @brief checks if the nested container contains an element that is mapped to key.
         * @param key key of the element to find.
         * @return true if the nested container contains an element mapped to key, false otherwise.
         * @throw std::bad_cast.
         */
        bool contains (std::string_view key) const
        {
            for (auto const& member : _data.template get<ObjectValue> ())
            {
                if (member.first.isString () && member.first.getString () == key)
                {
                    return true;
                }
            }

            return false;
        }

        /**
         * @brief checks if the nested container contains an element that is mapped to key.
         * @param key key of the element to find.
         * @return true if the nested container contains an element mapped to key, false otherwise.
         * @throw std::bad_cast.
         */
        bool contains (const char* key) const
        {
            return contains (std::string_view (key));
        }

        /**
         * @brief checks if the nested container contains an element at position pos or mapped to key.
         * @param key position or key of the element to find.
         * @return true if the nested container contains an element at position pos or mapped to key, false otherwise.
         * @throw std::bad_cast.
         */
        template <typename Key>
            requires std::same_as<Key, BasicValue>
        bool contains (const Key& key) const
        {
            if (_data.index () == ArrayValue)
            {
                return key.getUint64 () < _data.template get<ArrayValue> ().size ();
            }

            for (auto const& member : _data.template get<ObjectValue> ())
            {
                if (member.first == key)
                {
                    return true;
                }
            }

            return false;
        }

        /**
         * @brief exchanges the contents of the value with those of other.
         * @param other value to exchange the contents with.
         */
        void swap (Value& other)
        {
            if (_allocator == other._allocator)
            {
                std::swap (_data, other._data);
                std::swap (_capacity, other._capacity);
                return;
            }

            Value temp (std::move (*this));
            *this = std::move (other);
            other = std::move (temp);
        }

        /**
         * @brief serialize the value, resetting the handler on failure.
         * @param handler handler receiving the value.
         * @return 0 on success, -1 otherwise.
         */
        template <typename Handler>
        int serialize (Handler& handler) const
        {
            try
            {
                if (JOIN_LIKELY (setValue (*this, handler) == 0))
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
         * @brief deserialize a document into the value.
         * @param args document to deserialize, followed by any reader argument.
         * @return 0 on success, -1 otherwise.
         */
        template <typename Reader, typename... Args>
        int deserialize (Args&&... args)
        {
            Reader reader;
            return reader.deserialize (std::forward<Args> (args)..., *this);
        }

        /**
         * @brief serialize the value to a stream.
         * @param document stream receiving the value.
         * @param args writer arguments.
         * @return 0 on success, -1 otherwise.
         */
        template <typename Writer, typename... Args>
        int serialize (std::ostream& document, Args&&... args) const
        {
            Writer writer (document, std::forward<Args> (args)...);
            return serialize (writer);
        }

        /**
         * @brief compare if equal.
         * @param other value to compare to.
         * @return true if equal.
         */
        template <typename Other>
            requires std::same_as<Other, BasicValue<Allocator>>
        constexpr bool operator== (const Other& other) const;

        /**
         * @brief three-way compare values.
         * @param other value to compare to.
         * @return ordering of this value relative to other.
         */
        template <typename Other>
            requires std::same_as<Other, BasicValue<Allocator>>
        constexpr std::partial_ordering operator<=> (const Other& other) const;

        /**
         * @brief compare if equal to a null value.
         * @param other null value to compare to.
         * @return true if equal.
         */
        constexpr bool operator== (std::nullptr_t other) const;

        /**
         * @brief three-way compare to a null value.
         * @param other null value to compare to.
         * @return ordering of this value relative to other.
         */
        constexpr std::partial_ordering operator<=> (std::nullptr_t other) const;

        /**
         * @brief compare if equal to a boolean value.
         * @param other boolean value to compare to.
         * @return true if equal.
         */
        constexpr bool operator== (bool other) const;

        /**
         * @brief three-way compare to a boolean value.
         * @param other boolean value to compare to.
         * @return ordering of this value relative to other.
         */
        constexpr std::partial_ordering operator<=> (bool other) const;

        /**
         * @brief compare if equal to a integer value.
         * @param other integer value to compare to.
         * @return true if equal.
         */
        constexpr bool operator== (int32_t other) const;

        /**
         * @brief three-way compare to a integer value.
         * @param other integer value to compare to.
         * @return ordering of this value relative to other.
         */
        constexpr std::partial_ordering operator<=> (int32_t other) const;

        /**
         * @brief compare if equal to a unsigned integer value.
         * @param other unsigned integer value to compare to.
         * @return true if equal.
         */
        constexpr bool operator== (uint32_t other) const;

        /**
         * @brief three-way compare to a unsigned integer value.
         * @param other unsigned integer value to compare to.
         * @return ordering of this value relative to other.
         */
        constexpr std::partial_ordering operator<=> (uint32_t other) const;

        /**
         * @brief compare if equal to a 64 bits integer value.
         * @param other 64 bits integer value to compare to.
         * @return true if equal.
         */
        constexpr bool operator== (int64_t other) const;

        /**
         * @brief three-way compare to a 64 bits integer value.
         * @param other 64 bits integer value to compare to.
         * @return ordering of this value relative to other.
         */
        constexpr std::partial_ordering operator<=> (int64_t other) const;

        /**
         * @brief compare if equal to a 64 bits unsigned integer value.
         * @param other 64 bits unsigned integer value to compare to.
         * @return true if equal.
         */
        constexpr bool operator== (uint64_t other) const;

        /**
         * @brief three-way compare to a 64 bits unsigned integer value.
         * @param other 64 bits unsigned integer value to compare to.
         * @return ordering of this value relative to other.
         */
        constexpr std::partial_ordering operator<=> (uint64_t other) const;

        /**
         * @brief compare if equal to a real value.
         * @param other real value to compare to.
         * @return true if equal.
         */
        constexpr bool operator== (double other) const;

        /**
         * @brief three-way compare to a real value.
         * @param other real value to compare to.
         * @return ordering of this value relative to other.
         */
        constexpr std::partial_ordering operator<=> (double other) const;

        /**
         * @brief compare if equal to a C-style string.
         * @param other string to compare to.
         * @return true if equal.
         */
        constexpr bool operator== (const char* other) const;

        /**
         * @brief three-way compare to a C-style string.
         * @param other string to compare to.
         * @return ordering of this value relative to other.
         */
        constexpr std::partial_ordering operator<=> (const char* other) const;

        /**
         * @brief compare if equal to a string view.
         * @param other string to compare to.
         * @return true if equal.
         */
        constexpr bool operator== (std::string_view other) const;

        /**
         * @brief three-way compare to a string view.
         * @param other string to compare to.
         * @return ordering of this value relative to other.
         */
        constexpr std::partial_ordering operator<=> (std::string_view other) const;

        /**
         * @brief compare if equal to an array.
         * @param other array to compare to.
         * @return true if equal.
         */
        constexpr bool operator== (const Array& other) const;

        /**
         * @brief three-way compare to an array.
         * @param other array to compare to.
         * @return ordering of this value relative to other.
         */
        constexpr std::partial_ordering operator<=> (const Array& other) const;

        /**
         * @brief compare if equal to an object.
         * @param other object to compare to.
         * @return true if equal.
         */
        constexpr bool operator== (const Object& other) const;

        /**
         * @brief three-way compare to an object.
         * @param other object to compare to.
         * @return ordering of this value relative to other.
         */
        constexpr std::partial_ordering operator<=> (const Object& other) const;

    private:
        /// variant type holding the value.
        using Data =
            Variant<std::nullptr_t, bool, int32_t, uint32_t, int64_t, uint64_t, double, String, Binary, Array, Object>;

        // friendship with value builder.
        friend class ValueBuilder<BasicValue>;

        /**
         * @brief get the allocator shared by the tree, created when the root has none.
         * @return allocator.
         */
        Allocator* allocator ()
        {
            if (_allocator == nullptr)
            {
                _allocator = new Allocator ();
                _owner = true;
            }
            return _allocator;
        }

        /**
         * @brief allocate uninitialized memory for elements.
         * @param count number of elements.
         * @return pointer to the memory.
         * @throw std::bad_alloc.
         */
        template <typename T>
        T* allocate (size_t count)
        {
            void* data = nullptr;
            if (JOIN_LIKELY (count <= std::min<size_t> (std::numeric_limits<uint32_t>::max (),
                                                        std::numeric_limits<size_t>::max () / sizeof (T))))
            {
                data = allocator ()->allocate (count * sizeof (T));
            }

            if (JOIN_UNLIKELY (data == nullptr))
            {
                throwException<std::bad_alloc> ();
            }

            return static_cast<T*> (data);
        }

        /**
         * @brief reset to null, releasing the memory of the tree when the value owns the allocator.
         */
        void reset () noexcept
        {
            if constexpr (requires { _allocator->reset (); })
            {
                if (_owner)
                {
                    _allocator->reset ();
                }
            }
            _data.template set<NullValue> ();
            _capacity = 0;
        }

        /**
         * @brief copy a string into the allocator.
         * @param value string to copy.
         * @return copied string.
         */
        String copyString (std::string_view value)
        {
            char* data = allocate<char> (value.size () + 1);
            std::copy_n (value.data (), value.size (), data);
            data[value.size ()] = '\0';
            return String (data, value.size ());
        }

        /**
         * @brief copy binary data into the allocator.
         * @param value binary data to copy.
         * @return copied binary data.
         */
        Binary copyBinary (std::span<const uint8_t> value)
        {
            uint8_t* data = allocate<uint8_t> (value.size ());
            std::copy_n (value.data (), value.size (), data);
            return Binary (data, value.size ());
        }

        /**
         * @brief move values of the open containers of a builder into the allocator.
         * @param values values to move.
         * @param count number of values.
         * @return array holding the values.
         */
        Array makeArray (const Data* values, size_t count)
        {
            Value* data = allocate<Value> (count);
            for (size_t i = 0; i < count; ++i)
            {
                ::new (data + i) Value (*_allocator);
                data[i]._data = values[i];
            }
            return Array (data, count);
        }

        /**
         * @brief copy an array into the allocator.
         * @param value array to copy.
         * @return copied array.
         */
        Array copyArray (const Array& value)
        {
            Value* data = allocate<Value> (value.size ());
            for (size_t i = 0; i < value.size (); ++i)
            {
                ::new (data + i) Value (value[i], *_allocator);
            }
            return Array (data, value.size ());
        }

        /**
         * @brief move keys and values of the open containers of a builder into the allocator.
         * @param values keys and values to move, alternated.
         * @param count number of members.
         * @return object holding the members.
         */
        Object makeObject (const Data* values, size_t count)
        {
            Member* data = allocate<Member> (count);
            for (size_t i = 0; i < count; ++i)
            {
                ::new (data + i) Member{Value (*_allocator), Value (*_allocator)};
                data[i].first._data = values[2 * i];
                data[i].second._data = values[2 * i + 1];
            }
            return Object (data, count);
        }

        /**
         * @brief copy an object into the allocator.
         * @param value object to copy.
         * @return copied object.
         */
        Object copyObject (const Object& value)
        {
            Member* data = allocate<Member> (value.size ());
            for (size_t i = 0; i < value.size (); ++i)
            {
                ::new (data + i) Member{Value (value[i].first, *_allocator), Value (value[i].second, *_allocator)};
            }
            return Object (data, value.size ());
        }

        /**
         * @brief copy a value into the allocator.
         * @param other value to copy.
         */
        void copy (const Value& other)
        {
            _capacity = 0;

            switch (other.index ())
            {
                case StringValue:
                    _data.template set<StringValue> (copyString (other.getString ()));
                    break;

                case BinaryValue:
                    _data.template set<BinaryValue> (copyBinary (other.getBinary ()));
                    break;

                case ArrayValue:
                    _data.template set<ArrayValue> (copyArray (other.getArray ()));
                    break;

                case ObjectValue:
                    _data.template set<ObjectValue> (copyObject (other.getObject ()));
                    break;

                default:
                    _data = other._data;
                    break;
            }
        }

        /**
         * @brief move the elements of the nested container to a larger block.
         * @param cap new capacity.
         */
        template <typename T>
        void grow (size_t cap)
        {
            View<T>& view = _data.template get<View<T>> ();
            if (cap > std::max<size_t> (view.size (), _capacity))
            {
                T* data = allocate<T> (cap);
                if (!view.empty ())
                {
                    std::memcpy (static_cast<void*> (data), static_cast<const void*> (view.data ()),
                                 view.size () * sizeof (T));
                }
                view = View<T> (data, view.size ());
                _capacity = static_cast<uint32_t> (cap);
            }
        }

        /**
         * @brief add an uninitialized element at the end of the nested container.
         * @return a reference to the element.
         */
        template <typename T>
        T& append ()
        {
            View<T>& view = _data.template get<View<T>> ();
            if (view.size () >= _capacity)
            {
                grow<T> (std::max<size_t> (view.size () * 2, 1));
            }
            view = View<T> (view.data (), view.size () + 1);
            return view[view.size () - 1];
        }

        /**
         * @brief add a member at the end of the object.
         * @param key key of the member.
         * @param value value of the member.
         * @return a reference to the value of the member.
         */
        Value& appendMember (Value&& key, Value&& value)
        {
            Member& slot = append<Member> ();
            ::new (&slot) Member{Value (*_allocator), Value (*_allocator)};
            slot.first = std::move (key);
            slot.second = std::move (value);
            return slot.second;
        }

        /**
         * @brief add a member with a null value at the end of the object.
         * @param key key of the member.
         * @return a reference to the value of the member.
         */
        template <typename Key>
        Value& appendMember (Key key)
        {
            return appendMember (Value (key, *allocator ()), Value (*allocator ()));
        }

        /**
         * @brief remove a member of the object.
         * @param it member to remove.
         */
        void eraseMember (Member* it)
        {
            Object& object = _data.template get<ObjectValue> ();
            std::memmove (static_cast<void*> (it), static_cast<const void*> (it + 1),
                          (object.end () - it - 1) * sizeof (Member));
            object = Object (object.data (), object.size () - 1);
        }

        /**
         * @brief returns a reference to the array element at position pos.
         * @param pos position of the element.
         * @return a reference to the requested element.
         * @throw std::out_of_range.
         */
        Value& elementAt (size_t pos)
        {
            Array& array = _data.template get<ArrayValue> ();
            if (pos >= array.size ())
            {
                throwException<std::out_of_range> ("invalid key");
            }
            return array[pos];
        }

        /**
         * @brief returns a reference to the array element at position pos.
         * @param pos position of the element.
         * @return a reference to the requested element.
         * @throw std::out_of_range.
         */
        const Value& elementAt (size_t pos) const
        {
            const Array& array = _data.template get<ArrayValue> ();
            if (pos >= array.size ())
            {
                throwException<std::out_of_range> ("invalid key");
            }
            return array[pos];
        }

        /**
         * @brief set value.
         * @param value value to write.
         * @param handler handler receiving the value.
         * @return 0 on success, -1 otherwise.
         */
        template <typename Handler>
        static int setValue (const Value& value, Handler& handler)
        {
            switch (value.index ())
            {
                case Value::BooleanValue:
                    return handler.setBool (value.getBool ());

                case Value::IntegerValue:
                    return handler.setInt (value.getInt ());

                case Value::UnsignedValue:
                    return handler.setUint (value.getUint ());

                case Value::Integer64Value:
                    return handler.setInt64 (value.getInt64 ());

                case Value::Unsigned64Value:
                    return handler.setUint64 (value.getUint64 ());

                case Value::RealValue:
                    return handler.setDouble (value.getDouble ());

                case Value::StringValue:
                    return handler.setString (value.getString ());

                case Value::BinaryValue:
                    return handler.setBinary (value.getBinary ());

                case Value::ArrayValue:
                    return setArray (value.getArray (), handler);

                case Value::ObjectValue:
                    return setObject (value.getObject (), handler);

                default:
                    return handler.setNull ();
            }
        }

        /**
         * @brief set array value.
         * @param array array value to set.
         * @param handler handler receiving the value.
         * @return 0 on success, -1 otherwise.
         */
        template <typename Handler>
        static int setArray (const Array& array, Handler& handler)
        {
            if (handler.startArray (array.size ()) == -1)
            {
                return -1;
            }

            for (auto const& element : array)
            {
                if (setValue (element, handler) == -1)
                {
                    return -1;
                }
            }

            return handler.stopArray ();
        }

        /**
         * @brief set object value.
         * @param object object value to set.
         * @param handler handler receiving the value.
         * @return 0 on success, -1 otherwise.
         */
        template <typename Handler>
        static int setObject (const Object& object, Handler& handler)
        {
            if (handler.startObject (object.size ()) == -1)
            {
                return -1;
            }

            for (const Member& member : handler.members (object))
            {
                int result = -1;

                if (JOIN_LIKELY (member.first.isString ()))
                {
                    result = handler.setKey (member.first.getString ());
                }
                else
                {
                    switch (member.first.index ())
                    {
                        case Value::BooleanValue:
                            result = handler.setKey (member.first.getBool ());
                            break;

                        case Value::IntegerValue:
                            result = handler.setKey (member.first.getInt ());
                            break;

                        case Value::UnsignedValue:
                            result = handler.setKey (member.first.getUint ());
                            break;

                        case Value::Integer64Value:
                            result = handler.setKey (member.first.getInt64 ());
                            break;

                        case Value::Unsigned64Value:
                            result = handler.setKey (member.first.getUint64 ());
                            break;

                        case Value::RealValue:
                            result = handler.setKey (member.first.getDouble ());
                            break;

                        case Value::ArrayValue:
                        case Value::ObjectValue:
                            join::lastError = make_error_code (SaxErrc::InvalidKey);
                            break;

                        default:
                            result = handler.setKey (nullptr);
                            break;
                    }
                }

                if (result == -1)
                {
                    return -1;
                }

                if (setValue (member.second, handler) == -1)
                {
                    return -1;
                }
            }

            return handler.stopObject ();
        }

        /**
         * @brief convert the value held by the variant to float.
         * @return converted value.
         */
        float convertFloat () const
        {
            switch (_data.index ())
            {
                case IntegerValue:
                    return static_cast<float> (_data.template get<IntegerValue> ());

                case UnsignedValue:
                    return static_cast<float> (_data.template get<UnsignedValue> ());

                case Integer64Value:
                    return static_cast<float> (_data.template get<Integer64Value> ());

                case Unsigned64Value:
                    return static_cast<float> (_data.template get<Unsigned64Value> ());

                default:
                    break;
            }

            throwException<std::bad_cast> ();
        }

        /**
         * @brief convert the value held by the variant to bool.
         * @return converted value.
         */
        bool convertBool () const
        {
            switch (_data.index ())
            {
                case NullValue:
                    return false;

                case IntegerValue:
                    return _data.template get<IntegerValue> ();

                case UnsignedValue:
                    return _data.template get<UnsignedValue> ();

                case Integer64Value:
                    return _data.template get<Integer64Value> ();

                case Unsigned64Value:
                    return _data.template get<Unsigned64Value> ();

                case RealValue:
                    return _data.template get<RealValue> ();

                default:
                    break;
            }

            throwException<std::bad_cast> ();
        }

        /**
         * @brief convert the value held by the variant to int32_t.
         * @return converted value.
         */
        int32_t convertInt () const
        {
            if (isInt ())
            {
                switch (_data.index ())
                {
                    case UnsignedValue:
                        return static_cast<int32_t> (_data.template get<UnsignedValue> ());

                    case Integer64Value:
                        return static_cast<int32_t> (_data.template get<Integer64Value> ());

                    case Unsigned64Value:
                        return static_cast<int32_t> (_data.template get<Unsigned64Value> ());

                    default:
                        return static_cast<int32_t> (_data.template get<RealValue> ());
                }
            }

            throwException<std::bad_cast> ();
        }

        /**
         * @brief convert the value held by the variant to uint32_t.
         * @return converted value.
         */
        uint32_t convertUint () const
        {
            if (isUint ())
            {
                switch (_data.index ())
                {
                    case IntegerValue:
                        return static_cast<uint32_t> (_data.template get<IntegerValue> ());

                    case Integer64Value:
                        return static_cast<uint32_t> (_data.template get<Integer64Value> ());

                    case Unsigned64Value:
                        return static_cast<uint32_t> (_data.template get<Unsigned64Value> ());

                    default:
                        return static_cast<uint32_t> (_data.template get<RealValue> ());
                }
            }

            throwException<std::bad_cast> ();
        }

        /**
         * @brief convert the value held by the variant to int64_t.
         * @return converted value.
         */
        int64_t convertInt64 () const
        {
            if (isInt64 ())
            {
                switch (_data.index ())
                {
                    case IntegerValue:
                        return static_cast<int64_t> (_data.template get<IntegerValue> ());

                    case UnsignedValue:
                        return static_cast<int64_t> (_data.template get<UnsignedValue> ());

                    case Unsigned64Value:
                        return static_cast<int64_t> (_data.template get<Unsigned64Value> ());

                    default:
                        return static_cast<int64_t> (_data.template get<RealValue> ());
                }
            }

            throwException<std::bad_cast> ();
        }

        /**
         * @brief convert the value held by the variant to uint64_t.
         * @return converted value.
         */
        uint64_t convertUint64 () const
        {
            if (isUint64 ())
            {
                switch (_data.index ())
                {
                    case IntegerValue:
                        return static_cast<uint64_t> (_data.template get<IntegerValue> ());

                    case UnsignedValue:
                        return static_cast<uint64_t> (_data.template get<UnsignedValue> ());

                    case Integer64Value:
                        return static_cast<uint64_t> (_data.template get<Integer64Value> ());

                    default:
                        return static_cast<uint64_t> (_data.template get<RealValue> ());
                }
            }

            throwException<std::bad_cast> ();
        }

        /**
         * @brief convert the value held by the variant to double.
         * @return converted value.
         */
        double convertDouble () const
        {
            switch (_data.index ())
            {
                case IntegerValue:
                    return static_cast<double> (_data.template get<IntegerValue> ());

                case UnsignedValue:
                    return static_cast<double> (_data.template get<UnsignedValue> ());

                case Integer64Value:
                    return static_cast<double> (_data.template get<Integer64Value> ());

                case Unsigned64Value:
                    return static_cast<double> (_data.template get<Unsigned64Value> ());

                default:
                    break;
            }

            throwException<std::bad_cast> ();
        }

        /// variant holding the value.
        Data _data;

        /// allocator shared by the tree.
        Allocator* _allocator = nullptr;

        /// capacity of the nested container or string, 0 when it has no spare room.
        uint32_t _capacity = 0;

        /// true when the value destroys the allocator.
        bool _owner = false;
    };

    /**
     * @brief compare if equal.
     * @param other value to compare to.
     * @return true if equal.
     */
    template <typename Allocator>
    template <typename Other>
        requires std::same_as<Other, BasicValue<Allocator>>
    constexpr bool BasicValue<Allocator>::operator== (const Other& other) const
    {
        if (isNumber () && other.isNumber () && (index () != other.index ()))
        {
            if (isInt64 () && other.isUint64 ())
            {
                return getInt64 () >= 0 && uint64_t (getInt64 ()) == other.getUint64 ();
            }
            else if (isUint64 () && other.isInt64 ())
            {
                return other.getInt64 () >= 0 && uint64_t (other.getInt64 ()) == getUint64 ();
            }
            else
            {
                return getDouble () == other.getDouble ();
            }
        }

        return _data == other._data;
    }

    /**
     * @brief three-way compare values.
     * @param other value to compare to.
     * @return ordering of this value relative to other.
     */
    template <typename Allocator>
    template <typename Other>
        requires std::same_as<Other, BasicValue<Allocator>>
    constexpr std::partial_ordering BasicValue<Allocator>::operator<=> (const Other& other) const
    {
        if (isNumber () && other.isNumber () && (index () != other.index ()))
        {
            if (isInt64 () && other.isUint64 ())
            {
                if (getInt64 () < 0)
                {
                    return std::partial_ordering::less;
                }
                return uint64_t (getInt64 ()) <=> other.getUint64 ();
            }
            else if (isUint64 () && other.isInt64 ())
            {
                if (other.getInt64 () < 0)
                {
                    return std::partial_ordering::greater;
                }
                return getUint64 () <=> uint64_t (other.getInt64 ());
            }
            else
            {
                return getDouble () <=> other.getDouble ();
            }
        }

        return _data <=> other._data;
    }

    /**
     * @brief compare if equal to a null value.
     * @param other null value to compare to.
     * @return true if equal.
     */
    template <typename Allocator>
    constexpr bool BasicValue<Allocator>::operator== (std::nullptr_t other) const
    {
        return *this == BasicValue (other);
    }

    /**
     * @brief three-way compare to a null value.
     * @param other null value to compare to.
     * @return ordering of this value relative to other.
     */
    template <typename Allocator>
    constexpr std::partial_ordering BasicValue<Allocator>::operator<=> (std::nullptr_t other) const
    {
        return *this <=> BasicValue (other);
    }

    /**
     * @brief compare if equal to a boolean value.
     * @param other boolean value to compare to.
     * @return true if equal.
     */
    template <typename Allocator>
    constexpr bool BasicValue<Allocator>::operator== (bool other) const
    {
        return *this == BasicValue (other);
    }

    /**
     * @brief three-way compare to a boolean value.
     * @param other boolean value to compare to.
     * @return ordering of this value relative to other.
     */
    template <typename Allocator>
    constexpr std::partial_ordering BasicValue<Allocator>::operator<=> (bool other) const
    {
        return *this <=> BasicValue (other);
    }

    /**
     * @brief compare if equal to a integer value.
     * @param other integer value to compare to.
     * @return true if equal.
     */
    template <typename Allocator>
    constexpr bool BasicValue<Allocator>::operator== (int32_t other) const
    {
        return *this == BasicValue (other);
    }

    /**
     * @brief three-way compare to a integer value.
     * @param other integer value to compare to.
     * @return ordering of this value relative to other.
     */
    template <typename Allocator>
    constexpr std::partial_ordering BasicValue<Allocator>::operator<=> (int32_t other) const
    {
        return *this <=> BasicValue (other);
    }

    /**
     * @brief compare if equal to a unsigned integer value.
     * @param other unsigned integer value to compare to.
     * @return true if equal.
     */
    template <typename Allocator>
    constexpr bool BasicValue<Allocator>::operator== (uint32_t other) const
    {
        return *this == BasicValue (other);
    }

    /**
     * @brief three-way compare to a unsigned integer value.
     * @param other unsigned integer value to compare to.
     * @return ordering of this value relative to other.
     */
    template <typename Allocator>
    constexpr std::partial_ordering BasicValue<Allocator>::operator<=> (uint32_t other) const
    {
        return *this <=> BasicValue (other);
    }

    /**
     * @brief compare if equal to a 64 bits integer value.
     * @param other 64 bits integer value to compare to.
     * @return true if equal.
     */
    template <typename Allocator>
    constexpr bool BasicValue<Allocator>::operator== (int64_t other) const
    {
        return *this == BasicValue (other);
    }

    /**
     * @brief three-way compare to a 64 bits integer value.
     * @param other 64 bits integer value to compare to.
     * @return ordering of this value relative to other.
     */
    template <typename Allocator>
    constexpr std::partial_ordering BasicValue<Allocator>::operator<=> (int64_t other) const
    {
        return *this <=> BasicValue (other);
    }

    /**
     * @brief compare if equal to a 64 bits unsigned integer value.
     * @param other 64 bits unsigned integer value to compare to.
     * @return true if equal.
     */
    template <typename Allocator>
    constexpr bool BasicValue<Allocator>::operator== (uint64_t other) const
    {
        return *this == BasicValue (other);
    }

    /**
     * @brief three-way compare to a 64 bits unsigned integer value.
     * @param other 64 bits unsigned integer value to compare to.
     * @return ordering of this value relative to other.
     */
    template <typename Allocator>
    constexpr std::partial_ordering BasicValue<Allocator>::operator<=> (uint64_t other) const
    {
        return *this <=> BasicValue (other);
    }

    /**
     * @brief compare if equal to a real value.
     * @param other real value to compare to.
     * @return true if equal.
     */
    template <typename Allocator>
    constexpr bool BasicValue<Allocator>::operator== (double other) const
    {
        return *this == BasicValue (other);
    }

    /**
     * @brief three-way compare to a real value.
     * @param other real value to compare to.
     * @return ordering of this value relative to other.
     */
    template <typename Allocator>
    constexpr std::partial_ordering BasicValue<Allocator>::operator<=> (double other) const
    {
        return *this <=> BasicValue (other);
    }

    /**
     * @brief compare if equal to a C-style string.
     * @param other string to compare to.
     * @return true if equal.
     */
    template <typename Allocator>
    constexpr bool BasicValue<Allocator>::operator== (const char* other) const
    {
        return *this == std::string_view (other);
    }

    /**
     * @brief three-way compare to a C-style string.
     * @param other string to compare to.
     * @return ordering of this value relative to other.
     */
    template <typename Allocator>
    constexpr std::partial_ordering BasicValue<Allocator>::operator<=> (const char* other) const
    {
        return *this <=> std::string_view (other);
    }

    /**
     * @brief compare if equal to a string view.
     * @param other string to compare to.
     * @return true if equal.
     */
    template <typename Allocator>
    constexpr bool BasicValue<Allocator>::operator== (std::string_view other) const
    {
        return isString () && (getString () == other);
    }

    /**
     * @brief three-way compare to a string view.
     * @param other string to compare to.
     * @return ordering of this value relative to other.
     */
    template <typename Allocator>
    constexpr std::partial_ordering BasicValue<Allocator>::operator<=> (std::string_view other) const
    {
        if (!isString ())
        {
            return index () <=> StringValue;
        }
        return getString () <=> other;
    }

    /**
     * @brief compare if equal to an array.
     * @param other array to compare to.
     * @return true if equal.
     */
    template <typename Allocator>
    constexpr bool BasicValue<Allocator>::operator== (const Array& other) const
    {
        return isArray () && (getArray () == other);
    }

    /**
     * @brief three-way compare to an array.
     * @param other array to compare to.
     * @return ordering of this value relative to other.
     */
    template <typename Allocator>
    constexpr std::partial_ordering BasicValue<Allocator>::operator<=> (const Array& other) const
    {
        if (!isArray ())
        {
            return index () <=> ArrayValue;
        }
        return getArray () <=> other;
    }

    /**
     * @brief compare if equal to an object.
     * @param other object to compare to.
     * @return true if equal.
     */
    template <typename Allocator>
    constexpr bool BasicValue<Allocator>::operator== (const Object& other) const
    {
        return isObject () && (getObject () == other);
    }

    /**
     * @brief three-way compare to an object.
     * @param other object to compare to.
     * @return ordering of this value relative to other.
     */
    template <typename Allocator>
    constexpr std::partial_ordering BasicValue<Allocator>::operator<=> (const Object& other) const
    {
        if (!isObject ())
        {
            return index () <=> ObjectValue;
        }
        return getObject () <=> other;
    }

    /// value using the default allocator.
    using Value = BasicValue<>;

    /// array.
    using Array = View<Value>;

    /// object member.
    using Member = BasicMember<Value>;

    /// object.
    using Object = View<Member>;
}

#endif
