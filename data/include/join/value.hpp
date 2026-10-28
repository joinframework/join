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
#include <join/sax_error.hpp>
#include <join/variant.hpp>
#include <join/utils.hpp>

// C++.
#include <string_view>
#include <stdexcept>
#include <compare>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <limits>

// C.
#include <cstdint>
#include <cmath>

namespace join
{
    /// forward declaration.
    class Value;

    /// array.
    using Array = std::vector<Value>;

    /// object member.
    using Member = std::pair<Value, Value>;

    /// object.
    using Object = std::vector<Member>;

    /**
     * @brief value class.
     */
    class Value
    {
        /// variant type holding the value.
        using Data =
            Variant<std::nullptr_t, bool, int32_t, uint32_t, int64_t, uint64_t, double, std::string, Array, Object>;

    public:
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
            ArrayValue = 8,      /**< index of the array alternative. */
            ObjectValue = 9,     /**< index of the object alternative. */
        };

        /**
         * @brief default constructor.
         */
        constexpr Value () = default;

        /**
         * @brief constructs a null value.
         * @param value null value.
         */
        constexpr Value (std::nullptr_t value)
        : _data (std::in_place_index<NullValue>, value)
        {
        }

        /**
         * @brief constructs a boolean value.
         * @param value boolean value.
         */
        constexpr Value (bool value)
        : _data (std::in_place_index<BooleanValue>, value)
        {
        }

        /**
         * @brief constructs an integer value.
         * @param value integer value.
         */
        constexpr Value (int32_t value)
        : _data (std::in_place_index<IntegerValue>, value)
        {
        }

        /**
         * @brief constructs an unsigned integer value.
         * @param value unsigned integer value.
         */
        constexpr Value (uint32_t value)
        : _data (std::in_place_index<UnsignedValue>, value)
        {
        }

        /**
         * @brief constructs a 64 bits integer value.
         * @param value 64 bits integer value.
         */
        constexpr Value (int64_t value)
        : _data (std::in_place_index<Integer64Value>, value)
        {
        }

        /**
         * @brief constructs a 64 bits unsigned integer value.
         * @param value 64 bits unsigned integer value.
         */
        constexpr Value (uint64_t value)
        : _data (std::in_place_index<Unsigned64Value>, value)
        {
        }

        /**
         * @brief constructs a real value.
         * @param value real value.
         */
        constexpr Value (double value)
        : _data (std::in_place_index<RealValue>, value)
        {
        }

        /**
         * @brief constructs a string value from a C-style string.
         * @param value string value.
         */
        Value (const char* value)
        : _data (std::in_place_index<StringValue>, value)
        {
        }

        /**
         * @brief constructs a string value from a string view.
         * @param value string value.
         */
        Value (std::string_view value)
        : _data (std::in_place_index<StringValue>, value)
        {
        }

        /**
         * @brief constructs a string value.
         * @param value string value.
         */
        Value (const std::string& value)
        : _data (std::in_place_index<StringValue>, value)
        {
        }

        /**
         * @brief constructs a string value.
         * @param value string value.
         */
        Value (std::string&& value)
        : _data (std::in_place_index<StringValue>, std::move (value))
        {
        }

        /**
         * @brief constructs an array value.
         * @param value array value.
         */
        Value (const Array& value)
        : _data (std::in_place_index<ArrayValue>, value)
        {
        }

        /**
         * @brief constructs an array value.
         * @param value array value.
         */
        Value (Array&& value)
        : _data (std::in_place_index<ArrayValue>, std::move (value))
        {
        }

        /**
         * @brief constructs an object value.
         * @param value object value.
         */
        Value (const Object& value)
        : _data (std::in_place_index<ObjectValue>, value)
        {
        }

        /**
         * @brief constructs an object value.
         * @param value object value.
         */
        Value (Object&& value)
        : _data (std::in_place_index<ObjectValue>, std::move (value))
        {
        }

        /**
         * @brief copy constructor.
         * @param other object to copy.
         */
        constexpr Value (const Value& other) = default;

        /**
         * @brief copy assignment.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        Value& operator= (const Value& other) = default;

        /**
         * @brief move constructor.
         * @param other object to move.
         */
        constexpr Value (Value&& other) = default;

        /**
         * @brief move assignment.
         * @param other object to move.
         * @return a reference of the current object.
         */
        Value& operator= (Value&& other) = default;

        /**
         * @brief destroy the Value instance.
         */
        ~Value () = default;

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
                return _data.get<BooleanValue> ();
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
                    return _data.get<IntegerValue> () >= static_cast<int32_t> (std::numeric_limits<int8_t>::min ()) &&
                           _data.get<IntegerValue> () <= static_cast<int32_t> (std::numeric_limits<int8_t>::max ());

                case UnsignedValue:
                    return _data.get<UnsignedValue> () <= static_cast<uint32_t> (std::numeric_limits<int8_t>::max ());

                case Integer64Value:
                    return _data.get<Integer64Value> () >= static_cast<int64_t> (std::numeric_limits<int8_t>::min ()) &&
                           _data.get<Integer64Value> () <= static_cast<int64_t> (std::numeric_limits<int8_t>::max ());

                case Unsigned64Value:
                    return _data.get<Unsigned64Value> () <= static_cast<uint64_t> (std::numeric_limits<int8_t>::max ());

                case RealValue:
                    return std::trunc (_data.get<RealValue> ()) == _data.get<RealValue> () &&
                           _data.get<RealValue> () >= static_cast<double> (std::numeric_limits<int8_t>::min ()) &&
                           _data.get<RealValue> () <= static_cast<double> (std::numeric_limits<int8_t>::max ());

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
                        return static_cast<int8_t> (_data.get<IntegerValue> ());

                    case UnsignedValue:
                        return static_cast<int8_t> (_data.get<UnsignedValue> ());

                    case Integer64Value:
                        return static_cast<int8_t> (_data.get<Integer64Value> ());

                    case Unsigned64Value:
                        return static_cast<int8_t> (_data.get<Unsigned64Value> ());

                    default:
                        return static_cast<int8_t> (_data.get<RealValue> ());
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
                    return _data.get<IntegerValue> () >= 0 &&
                           _data.get<IntegerValue> () <= static_cast<int32_t> (std::numeric_limits<uint8_t>::max ());

                case UnsignedValue:
                    return _data.get<UnsignedValue> () <= static_cast<uint32_t> (std::numeric_limits<uint8_t>::max ());

                case Integer64Value:
                    return _data.get<Integer64Value> () >= 0 &&
                           _data.get<Integer64Value> () <= static_cast<int64_t> (std::numeric_limits<uint8_t>::max ());

                case Unsigned64Value:
                    return _data.get<Unsigned64Value> () <=
                           static_cast<uint64_t> (std::numeric_limits<uint8_t>::max ());

                case RealValue:
                    return std::trunc (_data.get<RealValue> ()) == _data.get<RealValue> () &&
                           _data.get<RealValue> () >= 0 &&
                           _data.get<RealValue> () <= static_cast<double> (std::numeric_limits<uint8_t>::max ());

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
                        return static_cast<uint8_t> (_data.get<IntegerValue> ());

                    case UnsignedValue:
                        return static_cast<uint8_t> (_data.get<UnsignedValue> ());

                    case Integer64Value:
                        return static_cast<uint8_t> (_data.get<Integer64Value> ());

                    case Unsigned64Value:
                        return static_cast<uint8_t> (_data.get<Unsigned64Value> ());

                    default:
                        return static_cast<uint8_t> (_data.get<RealValue> ());
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
                    return _data.get<IntegerValue> () >= static_cast<int32_t> (std::numeric_limits<int16_t>::min ()) &&
                           _data.get<IntegerValue> () <= static_cast<int32_t> (std::numeric_limits<int16_t>::max ());

                case UnsignedValue:
                    return _data.get<UnsignedValue> () <= static_cast<uint32_t> (std::numeric_limits<int16_t>::max ());

                case Integer64Value:
                    return _data.get<Integer64Value> () >=
                               static_cast<int64_t> (std::numeric_limits<int16_t>::min ()) &&
                           _data.get<Integer64Value> () <= static_cast<int64_t> (std::numeric_limits<int16_t>::max ());

                case Unsigned64Value:
                    return _data.get<Unsigned64Value> () <=
                           static_cast<uint64_t> (std::numeric_limits<int16_t>::max ());

                case RealValue:
                    return std::trunc (_data.get<RealValue> ()) == _data.get<RealValue> () &&
                           _data.get<RealValue> () >= static_cast<double> (std::numeric_limits<int16_t>::min ()) &&
                           _data.get<RealValue> () <= static_cast<double> (std::numeric_limits<int16_t>::max ());

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
                        return static_cast<int16_t> (_data.get<IntegerValue> ());

                    case UnsignedValue:
                        return static_cast<int16_t> (_data.get<UnsignedValue> ());

                    case Integer64Value:
                        return static_cast<int16_t> (_data.get<Integer64Value> ());

                    case Unsigned64Value:
                        return static_cast<int16_t> (_data.get<Unsigned64Value> ());

                    default:
                        return static_cast<int16_t> (_data.get<RealValue> ());
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
                    return _data.get<IntegerValue> () >= 0 &&
                           _data.get<IntegerValue> () <= static_cast<int32_t> (std::numeric_limits<uint16_t>::max ());

                case UnsignedValue:
                    return _data.get<UnsignedValue> () <= static_cast<uint32_t> (std::numeric_limits<uint16_t>::max ());

                case Integer64Value:
                    return _data.get<Integer64Value> () >= 0 &&
                           _data.get<Integer64Value> () <= static_cast<int64_t> (std::numeric_limits<uint16_t>::max ());

                case Unsigned64Value:
                    return _data.get<Unsigned64Value> () <=
                           static_cast<uint64_t> (std::numeric_limits<uint16_t>::max ());

                case RealValue:
                    return std::trunc (_data.get<RealValue> ()) == _data.get<RealValue> () &&
                           _data.get<RealValue> () >= 0 &&
                           _data.get<RealValue> () <= static_cast<double> (std::numeric_limits<uint16_t>::max ());

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
                        return static_cast<uint16_t> (_data.get<IntegerValue> ());

                    case UnsignedValue:
                        return static_cast<uint16_t> (_data.get<UnsignedValue> ());

                    case Integer64Value:
                        return static_cast<uint16_t> (_data.get<Integer64Value> ());

                    case Unsigned64Value:
                        return static_cast<uint16_t> (_data.get<Unsigned64Value> ());

                    default:
                        return static_cast<uint16_t> (_data.get<RealValue> ());
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
                    return _data.get<UnsignedValue> () <= static_cast<uint32_t> (std::numeric_limits<int32_t>::max ());

                case Integer64Value:
                    return _data.get<Integer64Value> () >=
                               static_cast<int64_t> (std::numeric_limits<int32_t>::min ()) &&
                           _data.get<Integer64Value> () <= static_cast<int64_t> (std::numeric_limits<int32_t>::max ());

                case Unsigned64Value:
                    return _data.get<Unsigned64Value> () <=
                           static_cast<uint64_t> (std::numeric_limits<int32_t>::max ());

                case RealValue:
                    return std::trunc (_data.get<RealValue> ()) == _data.get<RealValue> () &&
                           _data.get<RealValue> () >= static_cast<double> (std::numeric_limits<int32_t>::min ()) &&
                           _data.get<RealValue> () <= static_cast<double> (std::numeric_limits<int32_t>::max ());

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
                return _data.get<IntegerValue> ();
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
                    return _data.get<IntegerValue> () >= 0;

                case UnsignedValue:
                    return true;

                case Integer64Value:
                    return _data.get<Integer64Value> () >= 0 &&
                           _data.get<Integer64Value> () <= static_cast<int64_t> (std::numeric_limits<uint32_t>::max ());

                case Unsigned64Value:
                    return _data.get<Unsigned64Value> () <=
                           static_cast<uint64_t> (std::numeric_limits<uint32_t>::max ());

                case RealValue:
                    return std::trunc (_data.get<RealValue> ()) == _data.get<RealValue> () &&
                           _data.get<RealValue> () >= 0 &&
                           _data.get<RealValue> () <= static_cast<double> (std::numeric_limits<uint32_t>::max ());

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
                return _data.get<UnsignedValue> ();
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
                    return _data.get<Unsigned64Value> () <=
                           static_cast<uint64_t> (std::numeric_limits<int64_t>::max ());

                case RealValue:
                    return std::trunc (_data.get<RealValue> ()) == _data.get<RealValue> () &&
                           _data.get<RealValue> () >= static_cast<double> (std::numeric_limits<int64_t>::min ()) &&
                           _data.get<RealValue> () < static_cast<double> (std::numeric_limits<int64_t>::max ());

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
                return _data.get<Integer64Value> ();
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
                    return _data.get<IntegerValue> () >= 0;

                case UnsignedValue:
                    return true;

                case Integer64Value:
                    return _data.get<Integer64Value> () >= 0;

                case Unsigned64Value:
                    return true;

                case RealValue:
                    return std::trunc (_data.get<RealValue> ()) == _data.get<RealValue> () &&
                           _data.get<RealValue> () >= 0 &&
                           _data.get<RealValue> () < static_cast<double> (std::numeric_limits<uint64_t>::max ());

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
                return _data.get<Unsigned64Value> ();
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
                return static_cast<float> (_data.get<RealValue> ());
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
                return _data.get<RealValue> ();
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
         * @return variable held by value as a string value.
         * @throw std::bad_cast.
         */
        constexpr std::string_view getString () const
        {
            return _data.get<StringValue> ();
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
            return _data.get<ArrayValue> ();
        }

        /**
         * @brief get variable held by value as an array.
         * @return variable held by value as an array.
         * @throw std::bad_cast.
         */
        constexpr Array& getArray ()
        {
            return _data.get<ArrayValue> ();
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
            return _data.get<ObjectValue> ();
        }

        /**
         * @brief get variable held by value as an object.
         * @return variable held by value as an object.
         * @throw std::bad_cast.
         */
        constexpr Object& getObject ()
        {
            return _data.get<ObjectValue> ();
        }

        /**
         * @brief returns a reference to the value mapped to key.
         * @param key key of the element to find.
         * @return a reference to the requested element.
         * @throw std::bad_cast, std::out_of_range.
         */
        Value& at ([[maybe_unused]] std::nullptr_t key)
        {
            for (auto& member : _data.get<ObjectValue> ())
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
            for (auto& member : _data.get<ObjectValue> ())
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
                return _data.get<ArrayValue> ().at (key);
            }

            for (auto& member : _data.get<ObjectValue> ())
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
                return _data.get<ArrayValue> ().at (key);
            }

            for (auto& member : _data.get<ObjectValue> ())
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
                return _data.get<ArrayValue> ().at (key);
            }

            for (auto& member : _data.get<ObjectValue> ())
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
                return _data.get<ArrayValue> ().at (key);
            }

            for (auto& member : _data.get<ObjectValue> ())
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
            for (auto& member : _data.get<ObjectValue> ())
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
            for (auto& member : _data.get<ObjectValue> ())
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
         * @brief returns a reference to the value mapped to key.
         * @param key key of the element to find.
         * @return a reference to the requested element.
         * @throw std::bad_cast, std::out_of_range.
         */
        const Value& at ([[maybe_unused]] std::nullptr_t key) const
        {
            for (auto const& member : _data.get<ObjectValue> ())
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
            for (auto const& member : _data.get<ObjectValue> ())
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
                return _data.get<ArrayValue> ().at (key);
            }

            for (auto const& member : _data.get<ObjectValue> ())
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
                return _data.get<ArrayValue> ().at (key);
            }

            for (auto const& member : _data.get<ObjectValue> ())
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
                return _data.get<ArrayValue> ().at (key);
            }

            for (auto const& member : _data.get<ObjectValue> ())
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
                return _data.get<ArrayValue> ().at (key);
            }

            for (auto const& member : _data.get<ObjectValue> ())
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
            for (auto const& member : _data.get<ObjectValue> ())
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
            for (auto const& member : _data.get<ObjectValue> ())
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
         * @brief returns a reference to the value mapped to key.
         * @param key key of the element to find.
         * @return a reference to the requested element.
         * @throw std::bad_cast.
         */
        Value& operator[] (std::nullptr_t key)
        {
            if (_data.index () == NullValue)
            {
                _data.set<ObjectValue> ();
            }

            for (auto& member : _data.get<ObjectValue> ())
            {
                if (member.first.isNull ())
                {
                    return member.second;
                }
            }

            _data.get<ObjectValue> ().emplace_back (key, nullptr);

            return _data.get<ObjectValue> ().back ().second;
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
                _data.set<ObjectValue> ();
            }

            for (auto& member : _data.get<ObjectValue> ())
            {
                if (member.first.isBool () && member.first.getBool () == key)
                {
                    return member.second;
                }
            }

            _data.get<ObjectValue> ().emplace_back (key, nullptr);

            return _data.get<ObjectValue> ().back ().second;
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
                return _data.get<ArrayValue> ()[key];
            }

            if (_data.index () == NullValue)
            {
                _data.set<ObjectValue> ();
            }

            for (auto& member : _data.get<ObjectValue> ())
            {
                if (member.first.isInt () && member.first.getInt () == key)
                {
                    return member.second;
                }
            }

            _data.get<ObjectValue> ().emplace_back (key, nullptr);

            return _data.get<ObjectValue> ().back ().second;
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
                return _data.get<ArrayValue> ()[key];
            }

            if (_data.index () == NullValue)
            {
                _data.set<ObjectValue> ();
            }

            for (auto& member : _data.get<ObjectValue> ())
            {
                if (member.first.isUint () && member.first.getUint () == key)
                {
                    return member.second;
                }
            }

            _data.get<ObjectValue> ().emplace_back (key, nullptr);

            return _data.get<ObjectValue> ().back ().second;
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
                return _data.get<ArrayValue> ()[key];
            }

            if (_data.index () == NullValue)
            {
                _data.set<ObjectValue> ();
            }

            for (auto& member : _data.get<ObjectValue> ())
            {
                if (member.first.isInt64 () && member.first.getInt64 () == key)
                {
                    return member.second;
                }
            }

            _data.get<ObjectValue> ().emplace_back (key, nullptr);

            return _data.get<ObjectValue> ().back ().second;
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
                return _data.get<ArrayValue> ()[key];
            }

            if (_data.index () == NullValue)
            {
                _data.set<ObjectValue> ();
            }

            for (auto& member : _data.get<ObjectValue> ())
            {
                if (member.first.isUint64 () && member.first.getUint64 () == key)
                {
                    return member.second;
                }
            }

            _data.get<ObjectValue> ().emplace_back (key, nullptr);

            return _data.get<ObjectValue> ().back ().second;
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
                _data.set<ObjectValue> ();
            }

            for (auto& member : _data.get<ObjectValue> ())
            {
                if (member.first.isDouble () && member.first.getDouble () == key)
                {
                    return member.second;
                }
            }

            _data.get<ObjectValue> ().emplace_back (key, nullptr);

            return _data.get<ObjectValue> ().back ().second;
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
                _data.set<ObjectValue> ();
            }

            for (auto& member : _data.get<ObjectValue> ())
            {
                if (member.first.isString () && member.first.getString () == key)
                {
                    return member.second;
                }
            }

            _data.get<ObjectValue> ().emplace_back (key, nullptr);

            return _data.get<ObjectValue> ().back ().second;
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
         * @brief check if the nested container is empty.
         * @return true if empty, false otherwise.
         * @throw std::bad_cast.
         */
        bool empty () const
        {
            switch (_data.index ())
            {
                case StringValue:
                    return _data.get<StringValue> ().empty ();

                case ArrayValue:
                    return _data.get<ArrayValue> ().empty ();

                case ObjectValue:
                    return _data.get<ObjectValue> ().empty ();

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
                    return _data.get<StringValue> ().size ();

                case ArrayValue:
                    return _data.get<ArrayValue> ().size ();

                case ObjectValue:
                    return _data.get<ObjectValue> ().size ();

                default:
                    break;
            }

            throwException<std::bad_cast> ();
        }

        /**
         * @brief increase the capacity of the nested container.
         * @param cap new capacity.
         * @throw std::bad_cast.
         */
        void reserve (size_t cap)
        {
            switch (_data.index ())
            {
                case StringValue:
                    _data.get<StringValue> ().reserve (cap);
                    return;

                case ArrayValue:
                    _data.get<ArrayValue> ().reserve (cap);
                    return;

                case ObjectValue:
                    _data.get<ObjectValue> ().reserve (cap);
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
                    _data.get<StringValue> ().clear ();
                    return;

                case ArrayValue:
                    _data.get<ArrayValue> ().clear ();
                    return;

                case ObjectValue:
                    _data.get<ObjectValue> ().clear ();
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
        Value& insert (const Member& member)
        {
            _data.get<ObjectValue> ().push_back (member);
            return _data.get<ObjectValue> ().back ().second;
        }

        /**
         * @brief insert element in the nested container.
         * @param member the element to insert in the nested container.
         * @return a reference to the inserted element.
         * @throw std::bad_cast.
         */
        Value& insert (Member&& member)
        {
            _data.get<ObjectValue> ().emplace_back (std::move (member));
            return _data.get<ObjectValue> ().back ().second;
        }

        /**
         * @brief removes member with the key equivalent to key.
         * @param key key of the member to remove.
         * @return number of elements removed.
         * @throw std::bad_cast.
         */
        size_t erase ([[maybe_unused]] std::nullptr_t key)
        {
            auto beg = _data.get<ObjectValue> ().begin ();
            auto end = _data.get<ObjectValue> ().end ();

            for (auto it = beg; it != end; ++it)
            {
                if (it->first.isNull ())
                {
                    _data.get<ObjectValue> ().erase (it);
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
            auto beg = _data.get<ObjectValue> ().begin ();
            auto end = _data.get<ObjectValue> ().end ();

            for (auto it = beg; it != end; ++it)
            {
                if (it->first.isBool () && it->first.getBool () == key)
                {
                    _data.get<ObjectValue> ().erase (it);
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
            auto beg = _data.get<ObjectValue> ().begin ();
            auto end = _data.get<ObjectValue> ().end ();

            for (auto it = beg; it != end; ++it)
            {
                if (it->first.isInt () && it->first.getInt () == key)
                {
                    _data.get<ObjectValue> ().erase (it);
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
            auto beg = _data.get<ObjectValue> ().begin ();
            auto end = _data.get<ObjectValue> ().end ();

            for (auto it = beg; it != end; ++it)
            {
                if (it->first.isUint () && it->first.getUint () == key)
                {
                    _data.get<ObjectValue> ().erase (it);
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
            auto beg = _data.get<ObjectValue> ().begin ();
            auto end = _data.get<ObjectValue> ().end ();

            for (auto it = beg; it != end; ++it)
            {
                if (it->first.isInt64 () && it->first.getInt64 () == key)
                {
                    _data.get<ObjectValue> ().erase (it);
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
            auto beg = _data.get<ObjectValue> ().begin ();
            auto end = _data.get<ObjectValue> ().end ();

            for (auto it = beg; it != end; ++it)
            {
                if (it->first.isUint64 () && it->first.getUint64 () == key)
                {
                    _data.get<ObjectValue> ().erase (it);
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
            auto beg = _data.get<ObjectValue> ().begin ();
            auto end = _data.get<ObjectValue> ().end ();

            for (auto it = beg; it != end; ++it)
            {
                if (it->first.isDouble () && it->first.getDouble () == key)
                {
                    _data.get<ObjectValue> ().erase (it);
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
            auto beg = _data.get<ObjectValue> ().begin ();
            auto end = _data.get<ObjectValue> ().end ();

            for (auto it = beg; it != end; ++it)
            {
                if (it->first.isString () && it->first.getString () == key)
                {
                    _data.get<ObjectValue> ().erase (it);
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
         * @brief constructs an element in place at the end of the nested container.
         * @param args arguments forwarded to the element constructor.
         * @return a reference to the constructed object.
         * @throw std::bad_cast.
         */
        template <typename... Args>
        Value& emplaceBack (Args&&... args)
        {
            if (_data.index () == NullValue)
            {
                _data.set<ArrayValue> ();
            }
            _data.get<ArrayValue> ().emplace_back (std::forward<Args> (args)...);
            return _data.get<ArrayValue> ().back ();
        }

        /**
         * @brief appends element at the end of the nested container.
         * @param value the element to append.
         * @return a reference to the pushed object.
         * @throw std::bad_cast.
         */
        Value& pushBack (const Value& value)
        {
            if (_data.index () == NullValue)
            {
                _data.set<ArrayValue> ();
            }
            _data.get<ArrayValue> ().push_back (value);
            return _data.get<ArrayValue> ().back ();
        }

        /**
         * @brief appends element at the end of the nested container.
         * @param value the element to append.
         * @return a reference to the pushed object.
         * @throw std::bad_cast.
         */
        Value& pushBack (Value&& value)
        {
            if (_data.index () == NullValue)
            {
                _data.set<ArrayValue> ();
            }
            _data.get<ArrayValue> ().emplace_back (std::move (value));
            return _data.get<ArrayValue> ().back ();
        }

        /**
         * @brief removes the last element of the nested container.
         * @throw std::bad_cast.
         */
        void popBack ()
        {
            _data.get<ArrayValue> ().pop_back ();
        }

        /**
         * @brief checks if the nested container contains an element that is mapped to key.
         * @param key key of the element to find.
         * @return true if the nested container contains an element mapped to key, false otherwise.
         * @throw std::bad_cast.
         */
        bool contains ([[maybe_unused]] std::nullptr_t key) const
        {
            for (auto const& member : _data.get<ObjectValue> ())
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
            for (auto const& member : _data.get<ObjectValue> ())
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
                return key >= 0 && static_cast<uint64_t> (key) < _data.get<ArrayValue> ().size ();
            }

            for (auto const& member : _data.get<ObjectValue> ())
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
                return key < _data.get<ArrayValue> ().size ();
            }

            for (auto const& member : _data.get<ObjectValue> ())
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
                return key >= 0 && static_cast<uint64_t> (key) < _data.get<ArrayValue> ().size ();
            }

            for (auto const& member : _data.get<ObjectValue> ())
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
                return key < _data.get<ArrayValue> ().size ();
            }

            for (auto const& member : _data.get<ObjectValue> ())
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
            for (auto const& member : _data.get<ObjectValue> ())
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
            for (auto const& member : _data.get<ObjectValue> ())
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
         * @brief exchanges the contents of the value with those of other.
         * @param other value to exchange the contents with.
         */
        void swap (Value& other)
        {
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
         * @brief deserialize a json document.
         * @param document json document to deserialize.
         * @param length the length of the json document to deserialize.
         * @return 0 on success, -1 otherwise.
         */
        int jsonRead (const char* document, size_t length);

        /**
         * @brief deserialize a json document.
         * @param first the first character of the json document to deserialize.
         * @param last the last character of the document to deserialize.
         * @return 0 on success, -1 otherwise.
         */
        int jsonRead (const char* first, const char* last);

        /**
         * @brief deserialize a json document.
         * @param document json document to deserialize.
         * @return 0 on success, -1 otherwise.
         */
        int jsonRead (std::string_view document);

        /**
         * @brief deserialize a json document.
         * @param document json document to deserialize.
         * @return 0 on success, -1 otherwise.
         */
        int jsonRead (std::stringstream& document);

        /**
         * @brief deserialize a json document.
         * @param document json document to deserialize.
         * @return 0 on success, -1 otherwise.
         */
        int jsonRead (std::istringstream& document);

        /**
         * @brief deserialize a json document.
         * @param document json document to deserialize.
         * @return 0 on success, -1 otherwise.
         */
        int jsonRead (std::fstream& document);

        /**
         * @brief deserialize a json document.
         * @param document json document to deserialize.
         * @return 0 on success, -1 otherwise.
         */
        int jsonRead (std::ifstream& document);

        /**
         * @brief deserialize a json document.
         * @param document json document to deserialize.
         * @return 0 on success, -1 otherwise.
         */
        int jsonRead (std::iostream& document);

        /**
         * @brief deserialize a json document.
         * @param document json document to deserialize.
         * @return 0 on success, -1 otherwise.
         */
        int jsonRead (std::istream& document);

        /**
         * @brief serialize json data.
         * @param document Document where to serialize json data.
         * @param indentation number of spaces per indentation level.
         * @return 0 on success, -1 otherwise.
         */
        int jsonWrite (std::ostream& document, size_t indentation = 0) const;

        /**
         * @brief serialize canonicalized json data.
         * @param document Document where to serialize canonicalized json data.
         * @return 0 on success, -1 otherwise.
         */
        int jsonCanonicalize (std::ostream& document) const;

        /**
         * @brief deserialize a msgpack document.
         * @param document msgpack document to deserialize.
         * @param length the length of the msgpack document to deserialize.
         * @return 0 on success, -1 otherwise.
         */
        int packRead (const char* document, size_t length);

        /**
         * @brief deserialize a msgpack document.
         * @param first the first character of the msgpack document to deserialize.
         * @param last the last character of the document to deserialize.
         * @return 0 on success, -1 otherwise.
         */
        int packRead (const char* first, const char* last);

        /**
         * @brief deserialize a msgpack document.
         * @param document msgpack document to deserialize.
         * @return 0 on success, -1 otherwise.
         */
        int packRead (std::string_view document);

        /**
         * @brief deserialize a msgpack document.
         * @param document msgpack document to deserialize.
         * @return 0 on success, -1 otherwise.
         */
        int packRead (std::stringstream& document);

        /**
         * @brief deserialize a msgpack document.
         * @param document msgpack document to deserialize.
         * @return 0 on success, -1 otherwise.
         */
        int packRead (std::istringstream& document);

        /**
         * @brief deserialize a msgpack document.
         * @param document msgpack document to deserialize.
         * @return 0 on success, -1 otherwise.
         */
        int packRead (std::fstream& document);

        /**
         * @brief deserialize a msgpack document.
         * @param document msgpack document to deserialize.
         * @return 0 on success, -1 otherwise.
         */
        int packRead (std::ifstream& document);

        /**
         * @brief deserialize a msgpack document.
         * @param document msgpack document to deserialize.
         * @return 0 on success, -1 otherwise.
         */
        int packRead (std::iostream& document);

        /**
         * @brief deserialize a msgpack document.
         * @param document msgpack document to deserialize.
         * @return 0 on success, -1 otherwise.
         */
        int packRead (std::istream& document);

        /**
         * @brief serialize msgpack data.
         * @param document Document where to serialize msgpack data.
         * @return 0 on success, -1 otherwise.
         */
        int packWrite (std::ostream& document) const;

        // friendship with equal operator.
        friend constexpr bool operator== (const Value& lhs, const Value& rhs);

        // friendship with three-way comparison operator.
        friend constexpr std::partial_ordering operator<=> (const Value& lhs, const Value& rhs);

    private:
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

                    case Value::StringValue:
                        result = handler.setKey (member.first.getString ());
                        break;

                    case Value::ArrayValue:
                    case Value::ObjectValue:
                        join::lastError = make_error_code (SaxErrc::InvalidKey);
                        break;

                    default:
                        result = handler.setKey (nullptr);
                        break;
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
                    return static_cast<float> (_data.get<IntegerValue> ());

                case UnsignedValue:
                    return static_cast<float> (_data.get<UnsignedValue> ());

                case Integer64Value:
                    return static_cast<float> (_data.get<Integer64Value> ());

                case Unsigned64Value:
                    return static_cast<float> (_data.get<Unsigned64Value> ());

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
                    return _data.get<IntegerValue> ();

                case UnsignedValue:
                    return _data.get<UnsignedValue> ();

                case Integer64Value:
                    return _data.get<Integer64Value> ();

                case Unsigned64Value:
                    return _data.get<Unsigned64Value> ();

                case RealValue:
                    return _data.get<RealValue> ();

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
                        return static_cast<int32_t> (_data.get<UnsignedValue> ());

                    case Integer64Value:
                        return static_cast<int32_t> (_data.get<Integer64Value> ());

                    case Unsigned64Value:
                        return static_cast<int32_t> (_data.get<Unsigned64Value> ());

                    default:
                        return static_cast<int32_t> (_data.get<RealValue> ());
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
                        return static_cast<uint32_t> (_data.get<IntegerValue> ());

                    case Integer64Value:
                        return static_cast<uint32_t> (_data.get<Integer64Value> ());

                    case Unsigned64Value:
                        return static_cast<uint32_t> (_data.get<Unsigned64Value> ());

                    default:
                        return static_cast<uint32_t> (_data.get<RealValue> ());
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
                        return static_cast<int64_t> (_data.get<IntegerValue> ());

                    case UnsignedValue:
                        return static_cast<int64_t> (_data.get<UnsignedValue> ());

                    case Unsigned64Value:
                        return static_cast<int64_t> (_data.get<Unsigned64Value> ());

                    default:
                        return static_cast<int64_t> (_data.get<RealValue> ());
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
                        return static_cast<uint64_t> (_data.get<IntegerValue> ());

                    case UnsignedValue:
                        return static_cast<uint64_t> (_data.get<UnsignedValue> ());

                    case Integer64Value:
                        return static_cast<uint64_t> (_data.get<Integer64Value> ());

                    default:
                        return static_cast<uint64_t> (_data.get<RealValue> ());
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
                    return static_cast<double> (_data.get<IntegerValue> ());

                case UnsignedValue:
                    return static_cast<double> (_data.get<UnsignedValue> ());

                case Integer64Value:
                    return static_cast<double> (_data.get<Integer64Value> ());

                case Unsigned64Value:
                    return static_cast<double> (_data.get<Unsigned64Value> ());

                default:
                    break;
            }

            throwException<std::bad_cast> ();
        }

        /// variant holding the value.
        Data _data;
    };

    /**
     * @brief compare if equal.
     * @param lhs value to compare.
     * @param rhs value to compare to.
     * @return true if equal.
     */
    constexpr bool operator== (const Value& lhs, const Value& rhs)
    {
        if (lhs.isNumber () && rhs.isNumber () && (lhs.index () != rhs.index ()))
        {
            if (lhs.isInt64 () && rhs.isUint64 ())
            {
                return lhs.getInt64 () >= 0 && uint64_t (lhs.getInt64 ()) == rhs.getUint64 ();
            }
            else if (lhs.isUint64 () && rhs.isInt64 ())
            {
                return rhs.getInt64 () >= 0 && uint64_t (rhs.getInt64 ()) == lhs.getUint64 ();
            }
            else
            {
                return lhs.getDouble () == rhs.getDouble ();
            }
        }

        return lhs._data == rhs._data;
    }

    /**
     * @brief three-way compare values.
     * @param lhs value to compare.
     * @param rhs value to compare to.
     * @return ordering of lhs relative to rhs.
     */
    constexpr std::partial_ordering operator<=> (const Value& lhs, const Value& rhs)
    {
        if (lhs.isNumber () && rhs.isNumber () && (lhs.index () != rhs.index ()))
        {
            if (lhs.isInt64 () && rhs.isUint64 ())
            {
                if (lhs.getInt64 () < 0)
                {
                    return std::partial_ordering::less;
                }
                return uint64_t (lhs.getInt64 ()) <=> rhs.getUint64 ();
            }
            else if (lhs.isUint64 () && rhs.isInt64 ())
            {
                if (rhs.getInt64 () < 0)
                {
                    return std::partial_ordering::greater;
                }
                return lhs.getUint64 () <=> uint64_t (rhs.getInt64 ());
            }
            else
            {
                return lhs.getDouble () <=> rhs.getDouble ();
            }
        }

        return lhs._data <=> rhs._data;
    }
}

#endif
