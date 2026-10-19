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
#include <join/variant.hpp>
#include <join/error.hpp>

// C++.
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
            Null = 0,        /**< index of the std::nullptr_t alternative. */
            Boolean = 1,     /**< index of the boolean alternative. */
            Integer = 2,     /**< index of the 32 bits integer alternative. */
            Unsigned = 3,    /**< index of the 32 bits unsigned integer alternative. */
            Integer64 = 4,   /**< index of the 64 bits integer alternative. */
            Unsigned64 = 5,  /**< index of the 64 bits unsigned integer alternative. */
            Real = 6,        /**< index of the real alternative. */
            String = 7,      /**< index of the std::string alternative. */
            ArrayValue = 8,  /**< index of the Array alternative. */
            ObjectValue = 9, /**< index of the Object alternative. */
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
        : _data (std::in_place_index<Null>, value)
        {
        }

        /**
         * @brief constructs a boolean value.
         * @param value boolean value.
         */
        constexpr Value (bool value)
        : _data (std::in_place_index<Boolean>, value)
        {
        }

        /**
         * @brief constructs an integer value.
         * @param value integer value.
         */
        constexpr Value (int32_t value)
        : _data (std::in_place_index<Integer>, value)
        {
        }

        /**
         * @brief constructs an unsigned integer value.
         * @param value unsigned integer value.
         */
        constexpr Value (uint32_t value)
        : _data (std::in_place_index<Unsigned>, value)
        {
        }

        /**
         * @brief constructs a 64 bits integer value.
         * @param value 64 bits integer value.
         */
        constexpr Value (int64_t value)
        : _data (std::in_place_index<Integer64>, value)
        {
        }

        /**
         * @brief constructs a 64 bits unsigned integer value.
         * @param value 64 bits unsigned integer value.
         */
        constexpr Value (uint64_t value)
        : _data (std::in_place_index<Unsigned64>, value)
        {
        }

        /**
         * @brief constructs a real value.
         * @param value real value.
         */
        constexpr Value (double value)
        : _data (std::in_place_index<Real>, value)
        {
        }

        /**
         * @brief constructs a string value from a C-style string.
         * @param value string value.
         */
        Value (const char* value)
        : _data (std::in_place_index<String>, value)
        {
        }

        /**
         * @brief constructs a string value.
         * @param value string value.
         */
        Value (const std::string& value)
        : _data (std::in_place_index<String>, value)
        {
        }

        /**
         * @brief constructs a string value.
         * @param value string value.
         */
        Value (std::string&& value)
        : _data (std::in_place_index<String>, std::move (value))
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
            return _data.index () == Null;
        }

        /**
         * @brief check if the variable held by value is a boolean value.
         * @return true if the variable held by value is a boolean value, false otherwise.
         */
        constexpr bool isBool () const
        {
            return _data.index () == Boolean;
        }

        /**
         * @brief get variable held by value as a boolean value.
         * @return variable held by value as a boolean value.
         * @throw std::bad_cast.
         */
        constexpr bool getBool () const
        {
            if (_data.index () == Boolean)
            {
                return _data.get<Boolean> ();
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
            return _data.index () == Integer || _data.index () == Unsigned || _data.index () == Integer64 ||
                   _data.index () == Unsigned64 || _data.index () == Real;
        }

        /**
         * @brief check if the variable held by value is a 8 bits integer value.
         * @return true if the variable held by value is a 8 bits integer value, false otherwise.
         */
        constexpr bool isInt8 () const
        {
            switch (_data.index ())
            {
                case Integer:
                    return _data.get<Integer> () >= static_cast<int32_t> (std::numeric_limits<int8_t>::min ()) &&
                           _data.get<Integer> () <= static_cast<int32_t> (std::numeric_limits<int8_t>::max ());

                case Unsigned:
                    return _data.get<Unsigned> () <= static_cast<uint32_t> (std::numeric_limits<int8_t>::max ());

                case Integer64:
                    return _data.get<Integer64> () >= static_cast<int64_t> (std::numeric_limits<int8_t>::min ()) &&
                           _data.get<Integer64> () <= static_cast<int64_t> (std::numeric_limits<int8_t>::max ());

                case Unsigned64:
                    return _data.get<Unsigned64> () <= static_cast<uint64_t> (std::numeric_limits<int8_t>::max ());

                case Real:
                    return std::trunc (_data.get<Real> ()) == _data.get<Real> () &&
                           _data.get<Real> () >= static_cast<double> (std::numeric_limits<int8_t>::min ()) &&
                           _data.get<Real> () <= static_cast<double> (std::numeric_limits<int8_t>::max ());

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
                    case Integer:
                        return static_cast<int8_t> (_data.get<Integer> ());

                    case Unsigned:
                        return static_cast<int8_t> (_data.get<Unsigned> ());

                    case Integer64:
                        return static_cast<int8_t> (_data.get<Integer64> ());

                    case Unsigned64:
                        return static_cast<int8_t> (_data.get<Unsigned64> ());

                    default:
                        return static_cast<int8_t> (_data.get<Real> ());
                }
            }

            throw std::bad_cast ();
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
                case Integer:
                    return _data.get<Integer> () >= 0 &&
                           _data.get<Integer> () <= static_cast<int32_t> (std::numeric_limits<uint8_t>::max ());

                case Unsigned:
                    return _data.get<Unsigned> () <= static_cast<uint32_t> (std::numeric_limits<uint8_t>::max ());

                case Integer64:
                    return _data.get<Integer64> () >= 0 &&
                           _data.get<Integer64> () <= static_cast<int64_t> (std::numeric_limits<uint8_t>::max ());

                case Unsigned64:
                    return _data.get<Unsigned64> () <= static_cast<uint64_t> (std::numeric_limits<uint8_t>::max ());

                case Real:
                    return std::trunc (_data.get<Real> ()) == _data.get<Real> () && _data.get<Real> () >= 0 &&
                           _data.get<Real> () <= static_cast<double> (std::numeric_limits<uint8_t>::max ());

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
                    case Integer:
                        return static_cast<uint8_t> (_data.get<Integer> ());

                    case Unsigned:
                        return static_cast<uint8_t> (_data.get<Unsigned> ());

                    case Integer64:
                        return static_cast<uint8_t> (_data.get<Integer64> ());

                    case Unsigned64:
                        return static_cast<uint8_t> (_data.get<Unsigned64> ());

                    default:
                        return static_cast<uint8_t> (_data.get<Real> ());
                }
            }

            throw std::bad_cast ();
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
                case Integer:
                    return _data.get<Integer> () >= static_cast<int32_t> (std::numeric_limits<int16_t>::min ()) &&
                           _data.get<Integer> () <= static_cast<int32_t> (std::numeric_limits<int16_t>::max ());

                case Unsigned:
                    return _data.get<Unsigned> () <= static_cast<uint32_t> (std::numeric_limits<int16_t>::max ());

                case Integer64:
                    return _data.get<Integer64> () >= static_cast<int64_t> (std::numeric_limits<int16_t>::min ()) &&
                           _data.get<Integer64> () <= static_cast<int64_t> (std::numeric_limits<int16_t>::max ());

                case Unsigned64:
                    return _data.get<Unsigned64> () <= static_cast<uint64_t> (std::numeric_limits<int16_t>::max ());

                case Real:
                    return std::trunc (_data.get<Real> ()) == _data.get<Real> () &&
                           _data.get<Real> () >= static_cast<double> (std::numeric_limits<int16_t>::min ()) &&
                           _data.get<Real> () <= static_cast<double> (std::numeric_limits<int16_t>::max ());

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
                    case Integer:
                        return static_cast<int16_t> (_data.get<Integer> ());

                    case Unsigned:
                        return static_cast<int16_t> (_data.get<Unsigned> ());

                    case Integer64:
                        return static_cast<int16_t> (_data.get<Integer64> ());

                    case Unsigned64:
                        return static_cast<int16_t> (_data.get<Unsigned64> ());

                    default:
                        return static_cast<int16_t> (_data.get<Real> ());
                }
            }

            throw std::bad_cast ();
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
                case Integer:
                    return _data.get<Integer> () >= 0 &&
                           _data.get<Integer> () <= static_cast<int32_t> (std::numeric_limits<uint16_t>::max ());

                case Unsigned:
                    return _data.get<Unsigned> () <= static_cast<uint32_t> (std::numeric_limits<uint16_t>::max ());

                case Integer64:
                    return _data.get<Integer64> () >= 0 &&
                           _data.get<Integer64> () <= static_cast<int64_t> (std::numeric_limits<uint16_t>::max ());

                case Unsigned64:
                    return _data.get<Unsigned64> () <= static_cast<uint64_t> (std::numeric_limits<uint16_t>::max ());

                case Real:
                    return std::trunc (_data.get<Real> ()) == _data.get<Real> () && _data.get<Real> () >= 0 &&
                           _data.get<Real> () <= static_cast<double> (std::numeric_limits<uint16_t>::max ());

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
                    case Integer:
                        return static_cast<uint16_t> (_data.get<Integer> ());

                    case Unsigned:
                        return static_cast<uint16_t> (_data.get<Unsigned> ());

                    case Integer64:
                        return static_cast<uint16_t> (_data.get<Integer64> ());

                    case Unsigned64:
                        return static_cast<uint16_t> (_data.get<Unsigned64> ());

                    default:
                        return static_cast<uint16_t> (_data.get<Real> ());
                }
            }

            throw std::bad_cast ();
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
                case Integer:
                    return true;

                case Unsigned:
                    return _data.get<Unsigned> () <= static_cast<uint32_t> (std::numeric_limits<int32_t>::max ());

                case Integer64:
                    return _data.get<Integer64> () >= static_cast<int64_t> (std::numeric_limits<int32_t>::min ()) &&
                           _data.get<Integer64> () <= static_cast<int64_t> (std::numeric_limits<int32_t>::max ());

                case Unsigned64:
                    return _data.get<Unsigned64> () <= static_cast<uint64_t> (std::numeric_limits<int32_t>::max ());

                case Real:
                    return std::trunc (_data.get<Real> ()) == _data.get<Real> () &&
                           _data.get<Real> () >= static_cast<double> (std::numeric_limits<int32_t>::min ()) &&
                           _data.get<Real> () <= static_cast<double> (std::numeric_limits<int32_t>::max ());

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
            if (_data.index () == Integer)
            {
                return _data.get<Integer> ();
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
                case Integer:
                    return _data.get<Integer> () >= 0;

                case Unsigned:
                    return true;

                case Integer64:
                    return _data.get<Integer64> () >= 0 &&
                           _data.get<Integer64> () <= static_cast<int64_t> (std::numeric_limits<uint32_t>::max ());

                case Unsigned64:
                    return _data.get<Unsigned64> () <= static_cast<uint64_t> (std::numeric_limits<uint32_t>::max ());

                case Real:
                    return std::trunc (_data.get<Real> ()) == _data.get<Real> () && _data.get<Real> () >= 0 &&
                           _data.get<Real> () <= static_cast<double> (std::numeric_limits<uint32_t>::max ());

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
            if (_data.index () == Unsigned)
            {
                return _data.get<Unsigned> ();
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
                case Integer:
                case Unsigned:
                case Integer64:
                    return true;

                case Unsigned64:
                    return _data.get<Unsigned64> () <= static_cast<uint64_t> (std::numeric_limits<int64_t>::max ());

                case Real:
                    return std::trunc (_data.get<Real> ()) == _data.get<Real> () &&
                           _data.get<Real> () >= static_cast<double> (std::numeric_limits<int64_t>::min ()) &&
                           _data.get<Real> () < static_cast<double> (std::numeric_limits<int64_t>::max ());

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
            if (_data.index () == Integer64)
            {
                return _data.get<Integer64> ();
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
                case Integer:
                    return _data.get<Integer> () >= 0;

                case Unsigned:
                    return true;

                case Integer64:
                    return _data.get<Integer64> () >= 0;

                case Unsigned64:
                    return true;

                case Real:
                    return std::trunc (_data.get<Real> ()) == _data.get<Real> () && _data.get<Real> () >= 0 &&
                           _data.get<Real> () < static_cast<double> (std::numeric_limits<uint64_t>::max ());

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
            if (_data.index () == Unsigned64)
            {
                return _data.get<Unsigned64> ();
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
            if (_data.index () == Real)
            {
                return static_cast<float> (_data.get<Real> ());
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
            if (_data.index () == Real)
            {
                return _data.get<Real> ();
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
            return _data.index () == String;
        }

        /**
         * @brief get variable held by value as a string value.
         * @return variable held by value as a string value.
         * @throw std::bad_cast.
         */
        constexpr const std::string& getString () const
        {
            return _data.get<String> ();
        }

        /**
         * @brief get variable held by value as a string value.
         * @return variable held by value as a string value.
         * @throw std::bad_cast.
         */
        constexpr std::string& getString ()
        {
            return _data.get<String> ();
        }

        /**
         * @brief explicit conversion function for string value.
         * @return converted string value.
         * @throw std::bad_cast.
         */
        explicit operator const char*() const
        {
            return getString ().c_str ();
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
         * @brief returns a reference to the value at position pos or mapped to key.
         * @param key the position or key of the element to find.
         * @return a reference to the requested element.
         * @throw std::bad_cast, std::out_of_range.
         */
        Value& at (const Value& key)
        {
            if (_data.index () == ArrayValue)
            {
                return _data.get<ArrayValue> ().at (key.getUint64 ());
            }

            for (auto& member : _data.get<ObjectValue> ())
            {
                if (member.first == key)
                {
                    return member.second;
                }
            }

            throw std::out_of_range ("invalid key");
        }

        /**
         * @brief returns a reference to the value at position pos or mapped to key.
         * @param key the position or key of the element to find.
         * @return a reference to the requested element.
         * @throw std::bad_cast, std::out_of_range.
         */
        const Value& at (const Value& key) const
        {
            if (_data.index () == ArrayValue)
            {
                return _data.get<ArrayValue> ().at (key.getUint64 ());
            }

            for (auto const& member : _data.get<ObjectValue> ())
            {
                if (member.first == key)
                {
                    return member.second;
                }
            }

            throw std::out_of_range ("invalid key");
        }

        /**
         * @brief returns a reference to the value at position pos or mapped to key.
         * @param key the position or key of the element to find.
         * @return a reference to the requested element.
         * @throw std::bad_cast.
         */
        Value& operator[] (const Value& key)
        {
            if (_data.index () == ArrayValue)
            {
                return _data.get<ArrayValue> ()[key.getUint64 ()];
            }

            if (_data.index () == Null)
            {
                _data.set<ObjectValue> ();
            }

            for (auto& member : _data.get<ObjectValue> ())
            {
                if (member.first == key)
                {
                    return member.second;
                }
            }

            _data.get<ObjectValue> ().emplace_back (key, nullptr);

            return _data.get<ObjectValue> ().back ().second;
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
                case String:
                    return _data.get<String> ().empty ();

                case ArrayValue:
                    return _data.get<ArrayValue> ().empty ();

                case ObjectValue:
                    return _data.get<ObjectValue> ().empty ();

                default:
                    break;
            }

            throw std::bad_cast ();
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
                case String:
                    return _data.get<String> ().size ();

                case ArrayValue:
                    return _data.get<ArrayValue> ().size ();

                case ObjectValue:
                    return _data.get<ObjectValue> ().size ();

                default:
                    break;
            }

            throw std::bad_cast ();
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
                case String:
                    _data.get<String> ().reserve (cap);
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

            throw std::bad_cast ();
        }

        /**
         * @brief erases all elements in the nested container.
         * @throw std::bad_cast.
         */
        void clear ()
        {
            switch (_data.index ())
            {
                case String:
                    _data.get<String> ().clear ();
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

            throw std::bad_cast ();
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
         * @param key key value of the member to remove.
         * @return number of elements removed.
         * @throw std::bad_cast.
         */
        size_t erase (const Value& key)
        {
            auto beg = _data.get<ObjectValue> ().begin ();
            auto end = _data.get<ObjectValue> ().end ();

            for (auto it = beg; it != end; ++it)
            {
                if (it->first == key)
                {
                    _data.get<ObjectValue> ().erase (it);
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
            if (_data.index () == Null)
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
            if (_data.index () == Null)
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
            if (_data.index () == Null)
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
         * @return true if the nested container contains an element mapped to key, false otherwise.
         * @throw std::bad_cast.
         */
        bool contains (const Value& key) const
        {
            if (_data.index () == ArrayValue)
            {
                return _data.get<ArrayValue> ().size () > key.getUint64 ();
            }

            for (auto const& member : _data.get<ObjectValue> ())
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
            Value temp (std::move (*this));
            *this = std::move (other);
            other = std::move (temp);
        }

        // friendship with equal operator.
        friend constexpr bool operator== (const Value& lhs, const Value& rhs);

        // friendship with lower operator.
        friend constexpr bool operator< (const Value& lhs, const Value& rhs);

    private:
        /**
         * @brief convert the value held by the variant to float.
         * @return converted value.
         */
        float convertFloat () const
        {
            switch (_data.index ())
            {
                case Integer:
                    return static_cast<float> (_data.get<Integer> ());

                case Unsigned:
                    return static_cast<float> (_data.get<Unsigned> ());

                case Integer64:
                    return static_cast<float> (_data.get<Integer64> ());

                case Unsigned64:
                    return static_cast<float> (_data.get<Unsigned64> ());

                default:
                    break;
            }

            throw std::bad_cast ();
        }

        /**
         * @brief convert the value held by the variant to bool.
         * @return converted value.
         */
        bool convertBool () const
        {
            switch (_data.index ())
            {
                case Null:
                    return false;

                case Integer:
                    return _data.get<Integer> ();

                case Unsigned:
                    return _data.get<Unsigned> ();

                case Integer64:
                    return _data.get<Integer64> ();

                case Unsigned64:
                    return _data.get<Unsigned64> ();

                case Real:
                    return _data.get<Real> ();

                default:
                    break;
            }

            throw std::bad_cast ();
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
                    case Unsigned:
                        return static_cast<int32_t> (_data.get<Unsigned> ());

                    case Integer64:
                        return static_cast<int32_t> (_data.get<Integer64> ());

                    case Unsigned64:
                        return static_cast<int32_t> (_data.get<Unsigned64> ());

                    default:
                        return static_cast<int32_t> (_data.get<Real> ());
                }
            }

            throw std::bad_cast ();
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
                    case Integer:
                        return static_cast<uint32_t> (_data.get<Integer> ());

                    case Integer64:
                        return static_cast<uint32_t> (_data.get<Integer64> ());

                    case Unsigned64:
                        return static_cast<uint32_t> (_data.get<Unsigned64> ());

                    default:
                        return static_cast<uint32_t> (_data.get<Real> ());
                }
            }

            throw std::bad_cast ();
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
                    case Integer:
                        return static_cast<int64_t> (_data.get<Integer> ());

                    case Unsigned:
                        return static_cast<int64_t> (_data.get<Unsigned> ());

                    case Unsigned64:
                        return static_cast<int64_t> (_data.get<Unsigned64> ());

                    default:
                        return static_cast<int64_t> (_data.get<Real> ());
                }
            }

            throw std::bad_cast ();
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
                    case Integer:
                        return static_cast<uint64_t> (_data.get<Integer> ());

                    case Unsigned:
                        return static_cast<uint64_t> (_data.get<Unsigned> ());

                    case Integer64:
                        return static_cast<uint64_t> (_data.get<Integer64> ());

                    default:
                        return static_cast<uint64_t> (_data.get<Real> ());
                }
            }

            throw std::bad_cast ();
        }

        /**
         * @brief convert the value held by the variant to double.
         * @return converted value.
         */
        double convertDouble () const
        {
            switch (_data.index ())
            {
                case Integer:
                    return static_cast<double> (_data.get<Integer> ());

                case Unsigned:
                    return static_cast<double> (_data.get<Unsigned> ());

                case Integer64:
                    return static_cast<double> (_data.get<Integer64> ());

                case Unsigned64:
                    return static_cast<double> (_data.get<Unsigned64> ());

                default:
                    break;
            }

            throw std::bad_cast ();
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
     * @brief compare if not equal.
     * @param lhs value to compare.
     * @param rhs value to compare to.
     * @return true if not equal.
     */
    constexpr bool operator!= (const Value& lhs, const Value& rhs)
    {
        return !(lhs == rhs);
    }

    /**
     * @brief compare if lower than.
     * @param lhs value to compare.
     * @param rhs value to compare to.
     * @return true if lower than.
     */
    constexpr bool operator< (const Value& lhs, const Value& rhs)
    {
        if (lhs.isNumber () && rhs.isNumber () && (lhs.index () != rhs.index ()))
        {
            if (lhs.isInt64 () && rhs.isUint64 ())
            {
                return lhs.getInt64 () < 0 || uint64_t (lhs.getInt64 ()) < rhs.getUint64 ();
            }
            else if (lhs.isUint64 () && rhs.isInt64 ())
            {
                return rhs.getInt64 () >= 0 && lhs.getUint64 () < uint64_t (rhs.getInt64 ());
            }
            else
            {
                return lhs.getDouble () < rhs.getDouble ();
            }
        }

        return lhs._data < rhs._data;
    }

    /**
     * @brief compare if greater than.
     * @param lhs value to compare.
     * @param rhs value to compare to.
     * @return true if greater than.
     */
    constexpr bool operator> (const Value& lhs, const Value& rhs)
    {
        return rhs < lhs;
    }

    /**
     * @brief compare if lower or equal than.
     * @param lhs value to compare.
     * @param rhs value to compare to.
     * @return true if lower or equal than.
     */
    constexpr bool operator<= (const Value& lhs, const Value& rhs)
    {
        return !(rhs < lhs);
    }

    /**
     * @brief compare if greater or equal than.
     * @param lhs value to compare.
     * @param rhs value to compare to.
     * @return true if greater or equal than.
     */
    constexpr bool operator>= (const Value& lhs, const Value& rhs)
    {
        return !(lhs < rhs);
    }
}

#endif
