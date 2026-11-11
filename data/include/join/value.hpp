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
#include <optional>
#include <compare>
#include <ostream>
#include <utility>
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
    /// forward declarations.
    template <typename Allocator = HeapMem::BumpAllocator<4096, BumpPlain>>
    class BasicValue;

    template <typename Value>
    class ValueBuilder;

    /// scalar alternative selected by overload resolution for a type.
    template <typename Type>
    using ScalarType = match_t<Type, std::nullptr_t, bool, int32_t, uint32_t, int64_t, uint64_t, double>;

    /// type implicitly convertible to a single scalar alternative, object pointers excluded.
    template <typename Type>
    concept Scalar = (std::is_null_pointer_v<Type> || !std::convertible_to<Type, const volatile void*>) &&
                     requires { typename ScalarType<Type>; };

    /// string.
    using String = View<const char>;

    /**
     * @brief binary.
     */
    struct Binary
    {
        /**
         * @brief compare binaries.
         * @param other binary to compare to.
         * @return true if equal.
         */
        constexpr bool operator== (const Binary& other) const = default;

        /**
         * @brief three-way compare binaries.
         * @param other binary to compare to.
         * @return ordering of this binary relative to other.
         */
        constexpr std::partial_ordering operator<=> (const Binary& other) const = default;

        /// binary data.
        View<const uint8_t> data;

        /// binary subtype.
        std::optional<int8_t> subtype;
    };

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
         * @brief constructs a scalar value.
         * @param value scalar value.
         */
        template <Scalar Type>
        constexpr BasicValue (Type value)
        : _data (static_cast<ScalarType<Type>> (value))
        {
        }

        /**
         * @brief constructs a scalar value using an allocator owned by the caller.
         * @param value scalar value.
         * @param allocator allocator providing the memory of the value.
         */
        template <Scalar Type>
        BasicValue (Type value, Allocator& allocator)
        : BasicValue (allocator)
        {
            _data = static_cast<ScalarType<Type>> (value);
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
         * @param subtype binary subtype.
         */
        BasicValue (std::span<const uint8_t> value, std::optional<int8_t> subtype = std::nullopt)
        : BasicValue (nullptr)
        {
            _data.template set<BinaryValue> (copyBinary (value, subtype));
        }

        /**
         * @brief constructs a binary value using an allocator owned by the caller.
         * @param value binary value.
         * @param subtype binary subtype.
         * @param allocator allocator providing the memory of the value.
         */
        BasicValue (std::span<const uint8_t> value, std::optional<int8_t> subtype, Allocator& allocator)
        : BasicValue (allocator)
        {
            _data.template set<BinaryValue> (copyBinary (value, subtype));
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
            _data = value;
            return *this;
        }

        /**
         * @brief assign a scalar value.
         * @param value scalar value.
         * @return a reference of the current object.
         */
        template <Scalar Type>
        Value& operator= (Type value)
        {
            reset ();
            _data = static_cast<ScalarType<Type>> (value);
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

            _data.template set<BinaryValue> (copyBinary (value, std::nullopt));
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
         * @brief explicit conversion function for scalar value.
         * @return converted value.
         * @throw std::bad_cast.
         */
        template <typename Type>
            requires std::same_as<Type, bool> || std::same_as<Type, int8_t> || std::same_as<Type, uint8_t> ||
                     std::same_as<Type, int16_t> || std::same_as<Type, uint16_t> || std::same_as<Type, int32_t> ||
                     std::same_as<Type, uint32_t> || std::same_as<Type, int64_t> || std::same_as<Type, uint64_t> ||
                     std::same_as<Type, float> || std::same_as<Type, double>
        explicit operator Type () const
        {
            if constexpr (std::same_as<Type, bool>)
            {
                return getBool ();
            }
            else if constexpr (std::same_as<Type, float>)
            {
                return getFloat ();
            }
            else if constexpr (std::same_as<Type, double>)
            {
                return getDouble ();
            }
            else
            {
                return convertInteger<Type> ();
            }
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
            return isInteger<int8_t> ();
        }

        /**
         * @brief get variable held by value as a 8 bits integer.
         * @return variable held by value as a 8 bits integer.
         * @throw std::bad_cast.
         */
        constexpr int8_t getInt8 () const
        {
            return convertInteger<int8_t> ();
        }

        /**
         * @brief check if the variable held by value is a 8 bits unsigned integer value.
         * @return true if the variable held by value is a 8 bits unsigned integer value, false otherwise.
         */
        constexpr bool isUint8 () const
        {
            return isInteger<uint8_t> ();
        }

        /**
         * @brief get variable held by value as a 8 bits unsigned integer.
         * @return variable held by value as a 8 bits unsigned integer.
         * @throw std::bad_cast.
         */
        constexpr uint8_t getUint8 () const
        {
            return convertInteger<uint8_t> ();
        }

        /**
         * @brief check if the variable held by value is a 16 bits integer value.
         * @return true if the variable held by value is a 16 bits integer value, false otherwise.
         */
        constexpr bool isInt16 () const
        {
            return isInteger<int16_t> ();
        }

        /**
         * @brief get variable held by value as a 16 bits integer.
         * @return variable held by value as a 16 bits integer.
         * @throw std::bad_cast.
         */
        constexpr int16_t getInt16 () const
        {
            return convertInteger<int16_t> ();
        }

        /**
         * @brief check if the variable held by value is a 16 bits unsigned integer value.
         * @return true if the variable held by value is a 16 bits unsigned integer value, false otherwise.
         */
        constexpr bool isUint16 () const
        {
            return isInteger<uint16_t> ();
        }

        /**
         * @brief get variable held by value as a 16 bits unsigned integer.
         * @return variable held by value as a 16 bits unsigned integer.
         * @throw std::bad_cast.
         */
        constexpr uint16_t getUint16 () const
        {
            return convertInteger<uint16_t> ();
        }

        /**
         * @brief check if the variable held by value is a 32 bits integer value.
         * @return true if the variable held by value is a 32 bits integer value, false otherwise.
         */
        constexpr bool isInt () const
        {
            return isInteger<int32_t> ();
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

            return convertInteger<int32_t> ();
        }

        /**
         * @brief check if the variable held by value is a 32 bits unsigned integer value.
         * @return true if the variable held by value is a 32 bits unsigned integer value, false otherwise.
         */
        constexpr bool isUint () const
        {
            return isInteger<uint32_t> ();
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

            return convertInteger<uint32_t> ();
        }

        /**
         * @brief check if the variable held by value is a 64 bits integer value.
         * @return true if the variable held by value is a 64 bits integer value, false otherwise.
         */
        constexpr bool isInt64 () const
        {
            return isInteger<int64_t> ();
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

            return convertInteger<int64_t> ();
        }

        /**
         * @brief check if the variable held by value is a 64 bits unsigned integer value.
         * @return true if the variable held by value is a 64 bits unsigned integer value, false otherwise.
         */
        constexpr bool isUint64 () const
        {
            return isInteger<uint64_t> ();
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

            return convertInteger<uint64_t> ();
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
            return convertReal<float> ();
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

            return convertReal<double> ();
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
            return std::span<const uint8_t> (value.data.data (), value.data.size ());
        }

        /**
         * @brief get the subtype of the binary value held by value.
         * @return subtype of the binary value, std::nullopt if none.
         * @throw std::bad_cast.
         */
        constexpr std::optional<int8_t> getSubtype () const
        {
            return _data.template get<BinaryValue> ().subtype;
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
         * @brief returns a reference to the value at position pos or mapped to key.
         * @param key position or key of the element to find.
         * @return a reference to the requested element.
         * @throw std::bad_cast, std::out_of_range.
         */
        template <Scalar Key>
        Value& at (Key key)
        {
            return const_cast<Value&> (std::as_const (*this).at (key));
        }

        /**
         * @brief returns a reference to the value mapped to key.
         * @param key key of the element to find.
         * @return a reference to the requested element.
         * @throw std::bad_cast, std::out_of_range.
         */
        Value& at (std::string_view key)
        {
            return const_cast<Value&> (std::as_const (*this).at (key));
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
            return const_cast<Value&> (std::as_const (*this).at (key));
        }

        /**
         * @brief returns a reference to the value at position pos or mapped to key.
         * @param key position or key of the element to find.
         * @return a reference to the requested element.
         * @throw std::bad_cast, std::out_of_range.
         */
        template <Scalar Key>
        const Value& at (Key key) const
        {
            if constexpr (std::integral<ScalarType<Key>> && !std::same_as<ScalarType<Key>, bool>)
            {
                if (_data.index () == ArrayValue)
                {
                    return elementAt (static_cast<ScalarType<Key>> (key));
                }
            }

            const Member* member = findMember (key);
            if (member == nullptr)
            {
                throwException<std::out_of_range> ("invalid key");
            }

            return member->second;
        }

        /**
         * @brief returns a reference to the value mapped to key.
         * @param key key of the element to find.
         * @return a reference to the requested element.
         * @throw std::bad_cast, std::out_of_range.
         */
        const Value& at (std::string_view key) const
        {
            const Member* member = findMember (key);
            if (member == nullptr)
            {
                throwException<std::out_of_range> ("invalid key");
            }

            return member->second;
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

            const Member* member = findMember (key);
            if (member == nullptr)
            {
                throwException<std::out_of_range> ("invalid key");
            }

            return member->second;
        }

        /**
         * @brief returns a reference to the value at position pos or mapped to key.
         * @param key position or key of the element to find.
         * @return a reference to the requested element.
         * @throw std::bad_cast.
         */
        template <Scalar Key>
        Value& operator[] (Key key)
        {
            if constexpr (std::integral<ScalarType<Key>> && !std::same_as<ScalarType<Key>, bool>)
            {
                if (_data.index () == ArrayValue)
                {
                    return _data.template get<ArrayValue> ()[static_cast<ScalarType<Key>> (key)];
                }
            }

            if (_data.index () == NullValue)
            {
                _data.template set<ObjectValue> ();
            }

            Member* member = findMember (key);
            if (member == nullptr)
            {
                return appendMember (key);
            }

            return member->second;
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

            Member* member = findMember (key);
            if (member == nullptr)
            {
                return appendMember (key);
            }

            return member->second;
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

            Member* member = findMember (key);
            if (member == nullptr)
            {
                return appendMember (Value (key, *allocator ()), Value (*allocator ()));
            }

            return member->second;
        }

        /**
         * @brief check if the nested container is empty.
         * @return true if empty, false otherwise.
         * @throw std::bad_cast.
         */
        bool empty () const
        {
            return size () == 0;
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

                case BinaryValue:
                    return _data.template get<BinaryValue> ().data.size ();

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
        template <typename Key>
            requires Scalar<Key> || std::same_as<Key, BasicValue>
        size_t erase (const Key& key)
        {
            Member* member = findMember (key);
            if (member == nullptr)
            {
                return 0;
            }

            eraseMember (member);
            return 1;
        }

        /**
         * @brief removes member with the key equivalent to key.
         * @param key key of the member to remove.
         * @return number of elements removed.
         * @throw std::bad_cast.
         */
        size_t erase (std::string_view key)
        {
            Member* member = findMember (key);
            if (member == nullptr)
            {
                return 0;
            }

            eraseMember (member);
            return 1;
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
         * @brief appends a scalar value at the end of the nested container.
         * @param value the element to append.
         * @return a reference to the pushed object.
         * @throw std::bad_cast.
         */
        template <Scalar Type>
        Value& pushBack (Type value)
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
         * @param subtype binary subtype.
         * @return a reference to the pushed object.
         * @throw std::bad_cast.
         */
        Value& pushBack (std::span<const uint8_t> value, std::optional<int8_t> subtype = std::nullopt)
        {
            return pushBack (Value (value, subtype, *allocator ()));
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
         * @brief checks if the nested container contains an element at position pos or mapped to key.
         * @param key position or key of the element to find.
         * @return true if the nested container contains an element at position pos or mapped to key, false otherwise.
         * @throw std::bad_cast.
         */
        template <Scalar Key>
        bool contains (Key key) const
        {
            if constexpr (std::integral<ScalarType<Key>> && !std::same_as<ScalarType<Key>, bool>)
            {
                if (_data.index () == ArrayValue)
                {
                    if constexpr (std::is_signed_v<ScalarType<Key>>)
                    {
                        if (static_cast<ScalarType<Key>> (key) < 0)
                        {
                            return false;
                        }
                    }

                    return static_cast<uint64_t> (static_cast<ScalarType<Key>> (key)) <
                           _data.template get<ArrayValue> ().size ();
                }
            }

            return findMember (key) != nullptr;
        }

        /**
         * @brief checks if the nested container contains an element that is mapped to key.
         * @param key key of the element to find.
         * @return true if the nested container contains an element mapped to key, false otherwise.
         * @throw std::bad_cast.
         */
        bool contains (std::string_view key) const
        {
            return findMember (key) != nullptr;
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

            return findMember (key) != nullptr;
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
         * @brief compare if equal to a scalar value.
         * @param other scalar value to compare to.
         * @return true if equal.
         */
        template <Scalar Type>
        constexpr bool operator== (Type other) const;

        /**
         * @brief three-way compare to a scalar value.
         * @param other scalar value to compare to.
         * @return ordering of this value relative to other.
         */
        template <Scalar Type>
        constexpr std::partial_ordering operator<=> (Type other) const;

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
         * @param subtype binary subtype.
         * @return copied binary data.
         */
        Binary copyBinary (std::span<const uint8_t> value, std::optional<int8_t> subtype)
        {
            uint8_t* data = allocate<uint8_t> (value.size ());
            std::copy_n (value.data (), value.size (), data);
            return Binary{View<const uint8_t> (data, value.size ()), subtype};
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
                    _data.template set<BinaryValue> (copyBinary (other.getBinary (), other.getSubtype ()));
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
         * @brief check if the value matches a scalar key.
         * @param key key to match.
         * @return true if the value matches the key, false otherwise.
         */
        template <Scalar Key>
        constexpr bool matchKey (Key key) const
        {
            using Type = ScalarType<Key>;

            if constexpr (std::same_as<Type, std::nullptr_t>)
            {
                return isNull ();
            }
            else if constexpr (std::same_as<Type, bool>)
            {
                return isBool () && getBool () == static_cast<Type> (key);
            }
            else if constexpr (std::same_as<Type, double>)
            {
                return isDouble () && getDouble () == static_cast<Type> (key);
            }
            else
            {
                return isInteger<Type> () && convertInteger<Type> () == static_cast<Type> (key);
            }
        }

        /**
         * @brief check if the value matches a string key.
         * @param key key to match.
         * @return true if the value matches the key, false otherwise.
         */
        constexpr bool matchKey (std::string_view key) const
        {
            return isString () && getString () == key;
        }

        /**
         * @brief check if the value matches a value key.
         * @param key key to match.
         * @return true if the value matches the key, false otherwise.
         */
        template <typename Key>
            requires std::same_as<Key, BasicValue>
        constexpr bool matchKey (const Key& key) const
        {
            return *this == key;
        }

        /**
         * @brief find the member mapped to key.
         * @param key key of the member to find.
         * @return a pointer to the member, nullptr if not found.
         * @throw std::bad_cast.
         */
        template <typename Key>
        Member* findMember (const Key& key)
        {
            return const_cast<Member*> (std::as_const (*this).findMember (key));
        }

        /**
         * @brief find the member mapped to key.
         * @param key key of the member to find.
         * @return a pointer to the member, nullptr if not found.
         * @throw std::bad_cast.
         */
        template <typename Key>
        const Member* findMember (const Key& key) const
        {
            for (auto const& member : _data.template get<ObjectValue> ())
            {
                if (member.first.matchKey (key))
                {
                    return &member;
                }
            }

            return nullptr;
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
            return const_cast<Value&> (std::as_const (*this).elementAt (pos));
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
                    return handler.setBinary (value.getBinary (), value.getSubtype ());

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

                        case Value::BinaryValue:
                            result = handler.setKey (member.first.getBinary (), member.first.getSubtype ());
                            break;

                        case Value::ArrayValue:
                        case Value::ObjectValue:
                            result = handler.startKey ();
                            if (result == 0)
                            {
                                result = setValue (member.first, handler);
                            }
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
         * @brief convert the value held by the variant to a floating point type.
         * @return converted value.
         * @throw std::bad_cast.
         */
        template <typename Type>
        Type convertReal () const
        {
            switch (_data.index ())
            {
                case IntegerValue:
                    return static_cast<Type> (_data.template get<IntegerValue> ());

                case UnsignedValue:
                    return static_cast<Type> (_data.template get<UnsignedValue> ());

                case Integer64Value:
                    return static_cast<Type> (_data.template get<Integer64Value> ());

                case Unsigned64Value:
                    return static_cast<Type> (_data.template get<Unsigned64Value> ());

                case RealValue:
                    return static_cast<Type> (_data.template get<RealValue> ());

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
         * @brief check if the variable held by value fits in an integer type.
         * @return true if the variable held by value fits in the integer type, false otherwise.
         */
        template <typename Type>
        constexpr bool isInteger () const
        {
            switch (_data.index ())
            {
                case IntegerValue:
                    return std::in_range<Type> (_data.template get<IntegerValue> ());

                case UnsignedValue:
                    return std::in_range<Type> (_data.template get<UnsignedValue> ());

                case Integer64Value:
                    return std::in_range<Type> (_data.template get<Integer64Value> ());

                case Unsigned64Value:
                    return std::in_range<Type> (_data.template get<Unsigned64Value> ());

                case RealValue:
                    return std::trunc (_data.template get<RealValue> ()) == _data.template get<RealValue> () &&
                           _data.template get<RealValue> () >=
                               static_cast<double> (std::numeric_limits<Type>::min ()) &&
                           _data.template get<RealValue> () < std::ldexp (1.0, std::numeric_limits<Type>::digits);

                default:
                    break;
            }

            return false;
        }

        /**
         * @brief convert the value held by the variant to an integer type.
         * @return converted value.
         * @throw std::bad_cast.
         */
        template <typename Type>
        constexpr Type convertInteger () const
        {
            if (isInteger<Type> ())
            {
                switch (_data.index ())
                {
                    case IntegerValue:
                        return static_cast<Type> (_data.template get<IntegerValue> ());

                    case UnsignedValue:
                        return static_cast<Type> (_data.template get<UnsignedValue> ());

                    case Integer64Value:
                        return static_cast<Type> (_data.template get<Integer64Value> ());

                    case Unsigned64Value:
                        return static_cast<Type> (_data.template get<Unsigned64Value> ());

                    default:
                        return static_cast<Type> (_data.template get<RealValue> ());
                }
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
     * @brief compare if equal to a scalar value.
     * @param other scalar value to compare to.
     * @return true if equal.
     */
    template <typename Allocator>
    template <Scalar Type>
    constexpr bool BasicValue<Allocator>::operator== (Type other) const
    {
        return *this == BasicValue (other);
    }

    /**
     * @brief three-way compare to a scalar value.
     * @param other scalar value to compare to.
     * @return ordering of this value relative to other.
     */
    template <typename Allocator>
    template <Scalar Type>
    constexpr std::partial_ordering BasicValue<Allocator>::operator<=> (Type other) const
    {
        return *this <=> BasicValue (other);
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
