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
#include <join/variant.hpp>
#include <join/error.hpp>

// C++.
#include <functional>
#include <ostream>
#include <string>
#include <vector>
#include <limits>

// C.
#include <cmath>

namespace join
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
     * @brief value class.
     */
    class Value : public Variant <std::nullptr_t, bool, int32_t, uint32_t, int64_t, uint64_t, double, std::string, Array, Object>
    {
    public:
        /**
         * @brief nested value type index.
         */
        enum Index : size_t
        {
            Null        = 0,    /**< index of the std::nullptr_t alternative that can be held by the Value object. */
            Boolean     = 1,    /**< index of the boolean alternative that can be held by the Value object. */
            Integer     = 2,    /**< index of the 32 bits integer alternative that can be held by the Value object. */
            Unsigned    = 3,    /**< index of the 32 bits unsigned integer alternative that can be held by the Value object. */
            Integer64   = 4,    /**< index of the 64 bits integer alternative that can be held by the Value object. */
            Unsigned64  = 5,    /**< index of the 64 bits unsigned integer alternative that can be held by the Value object. */
            Real        = 6,    /**< index of the real alternative that can be held by the Value object. */
            String      = 7,    /**< index of the std::string alternative that can be held by the Value object. */
            ArrayValue  = 8,    /**< index of the Array alternative that can be held by the Value object. */
            ObjectValue = 9,    /**< index of the Object alternative that can be held by the Value object. */
        };

        /// pointer.
        using Ptr = Value*;

        /// inherits parent's constructors.
        using Variant::Variant;

        /// inherits parent's assignment operators.
        using Variant::operator=;

        /**
         * @brief default constructor.
         */
        constexpr Value () = default;

        /**
         * @brief constructs the value with the copy of the contents of a C-style string.
         * @param other c-style string to use as data source.
         */
        constexpr Value (const char* other)
        : Variant (in_place_index_t <String> {}, other)
        {
        }

        /**
         * @brief replaces the contents with a copy of a C-style string.
         * @param other c-style string to use as data source.
         * @return a reference of the current value.
         */
        Value& operator= (const char* other)
        {
            Variant::operator= (Value (in_place_index_t <String> {}, other));
            return *this;
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
        constexpr Value& operator= (const Value& other) = default;

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
        constexpr Value& operator= (Value&& other) = default;

        /**
         * @brief destroy the Value instance.
         */
        virtual ~Value () = default;

        /**
         * @brief check if the variable held by value is a null value.
         * @return true if the variable held by value is null, false otherwise.
         */
        constexpr bool isNull () const
        {
            return index () == Null;
        }

        /**
         * @brief check if the variable held by value is a boolean value.
         * @return true if the variable held by value is a boolean value, false otherwise.
         */
        constexpr bool isBool () const
        {
            return index () == Boolean;
        }

        /**
         * @brief get variable held by value as a boolean value.
         * @return variable held by value as a boolean value.
         * @throw std::bad_cast.
         */
        constexpr bool getBool () const
        {
            switch (index ())
            {
                case Null:
                    return false;

                case Boolean:
                    return get <Boolean> ();

                case Integer:
                    return get <Integer> ();

                case Unsigned:
                    return get <Unsigned> ();

                case Integer64:
                    return get <Integer64> ();

                case Unsigned64:
                    return get <Unsigned64> ();

                case Real:
                    return get <Real> ();

                default:
                    break;
            }

            throw std::bad_cast ();
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
            return index () == Integer || index () == Unsigned || index () == Integer64 || index () == Unsigned64 || index () == Real;
        }

        /**
         * @brief check if the variable held by value is a 8 bits integer value.
         * @return true if the variable held by value is a 8 bits integer value, false otherwise.
         */
        constexpr bool isInt8 () const
        {
            switch (index ())
            {
                case Integer:
                    return get <Integer> () >= static_cast <int32_t> (std::numeric_limits <int8_t>::min ()) &&
                           get <Integer> () <= static_cast <int32_t> (std::numeric_limits <int8_t>::max ());

                case Unsigned:
                    return get <Unsigned> () <= static_cast <uint32_t> (std::numeric_limits <int8_t>::max ());

                case Integer64:
                    return get <Integer64> () >= static_cast <int64_t> (std::numeric_limits <int8_t>::min ()) &&
                           get <Integer64> () <= static_cast <int64_t> (std::numeric_limits <int8_t>::max ());

                case Unsigned64:
                    return get <Unsigned64> () <= static_cast <uint64_t> (std::numeric_limits <int8_t>::max ());

                case Real:
                    return std::trunc (get <Real> ()) == get <Real> () &&
                           get <Real> () >= static_cast <double> (std::numeric_limits <int8_t>::min ()) &&
                           get <Real> () <= static_cast <double> (std::numeric_limits <int8_t>::max ());

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
                switch (index ())
                {
                    case Integer:
                        return static_cast <int8_t> (get <Integer> ());

                    case Unsigned:
                        return static_cast <int8_t> (get <Unsigned> ());

                    case Integer64:
                        return static_cast <int8_t> (get <Integer64> ());

                    case Unsigned64:
                        return static_cast <int8_t> (get <Unsigned64> ());

                    default:
                        return static_cast <int8_t> (get <Real> ());
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
            switch (index ())
            {
                case Integer:
                    return get <Integer> () >= 0 &&
                           get <Integer> () <= static_cast <int32_t> (std::numeric_limits <uint8_t>::max ());

                case Unsigned:
                    return get <Unsigned> () <= static_cast <uint32_t> (std::numeric_limits <uint8_t>::max ());

                case Integer64:
                    return get <Integer64> () >= 0 &&
                           get <Integer64> () <= static_cast <int64_t> (std::numeric_limits <uint8_t>::max ());

                case Unsigned64:
                    return get <Unsigned64> () <= static_cast <uint64_t> (std::numeric_limits <uint8_t>::max ());

                case Real:
                    return std::trunc (get <Real> ()) == get <Real> () &&
                           get <Real> () >= 0 &&
                           get <Real> () <= static_cast <double> (std::numeric_limits <uint8_t>::max ());

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
                switch (index ())
                {
                    case Integer:
                        return static_cast <uint8_t> (get <Integer> ());

                    case Unsigned:
                        return static_cast <uint8_t> (get <Unsigned> ());

                    case Integer64:
                        return static_cast <uint8_t> (get <Integer64> ());

                    case Unsigned64:
                        return static_cast <uint8_t> (get <Unsigned64> ());

                    default:
                        return static_cast <uint8_t> (get <Real> ());
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
            switch (index ())
            {
                case Integer:
                    return get <Integer> () >= static_cast <int32_t> (std::numeric_limits <int16_t>::min ()) &&
                           get <Integer> () <= static_cast <int32_t> (std::numeric_limits <int16_t>::max ());

                case Unsigned:
                    return get <Unsigned> () <= static_cast <uint32_t> (std::numeric_limits <int16_t>::max ());

                case Integer64:
                    return get <Integer64> () >= static_cast <int64_t> (std::numeric_limits <int16_t>::min ()) &&
                           get <Integer64> () <= static_cast <int64_t> (std::numeric_limits <int16_t>::max ());

                case Unsigned64:
                    return get <Unsigned64> () <= static_cast <uint64_t> (std::numeric_limits <int16_t>::max ());

                case Real:
                    return std::trunc (get <Real> ()) == get <Real> () &&
                           get <Real> () >= static_cast <double> (std::numeric_limits <int16_t>::min ()) &&
                           get <Real> () <= static_cast <double> (std::numeric_limits <int16_t>::max ());

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
                switch (index ())
                {
                    case Integer:
                        return static_cast <int16_t> (get <Integer> ());

                    case Unsigned:
                        return static_cast <int16_t> (get <Unsigned> ());

                    case Integer64:
                        return static_cast <int16_t> (get <Integer64> ());

                    case Unsigned64:
                        return static_cast <int16_t> (get <Unsigned64> ());

                    default:
                        return static_cast <int16_t> (get <Real> ());
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
            switch (index ())
            {
                case Integer:
                    return get <Integer> () >= 0 &&
                           get <Integer> () <= static_cast <int32_t> (std::numeric_limits <uint16_t>::max ());

                case Unsigned:
                    return get <Unsigned> () <= static_cast <uint32_t> (std::numeric_limits <uint16_t>::max ());

                case Integer64:
                    return get <Integer64> () >= 0 &&
                           get <Integer64> () <= static_cast <int64_t> (std::numeric_limits <uint16_t>::max ());

                case Unsigned64:
                    return get <Unsigned64> () <= static_cast <uint64_t> (std::numeric_limits <uint16_t>::max ());

                case Real:
                    return std::trunc (get <Real> ()) == get <Real> () &&
                           get <Real> () >= 0 &&
                           get <Real> () <= static_cast <double> (std::numeric_limits <uint16_t>::max ());

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
                switch (index ())
                {
                    case Integer:
                        return static_cast <uint16_t> (get <Integer> ());

                    case Unsigned:
                        return static_cast <uint16_t> (get <Unsigned> ());

                    case Integer64:
                        return static_cast <uint16_t> (get <Integer64> ());

                    case Unsigned64:
                        return static_cast <uint16_t> (get <Unsigned64> ());

                    default:
                        return static_cast <uint16_t> (get <Real> ());
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
            switch (index ())
            {
                case Integer:
                    return true;

                case Unsigned:
                    return get <Unsigned> () <= static_cast <uint32_t> (std::numeric_limits <int32_t>::max ());

                case Integer64:
                    return get <Integer64> () >= static_cast <int64_t> (std::numeric_limits <int32_t>::min ()) &&
                           get <Integer64> () <= static_cast <int64_t> (std::numeric_limits <int32_t>::max ());

                case Unsigned64:
                    return get <Unsigned64> () <= static_cast <uint64_t> (std::numeric_limits <int32_t>::max ());

                case Real:
                    return std::trunc (get <Real> ()) == get <Real> () &&
                           get <Real> () >= static_cast <double> (std::numeric_limits <int32_t>::min ()) &&
                           get <Real> () <= static_cast <double> (std::numeric_limits <int32_t>::max ());

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
            if (isInt ())
            {
                switch (index ())
                {
                    case Integer:
                        return get <Integer> ();

                    case Unsigned:
                        return static_cast <int32_t> (get <Unsigned> ());

                    case Integer64:
                        return static_cast <int32_t> (get <Integer64> ());

                    case Unsigned64:
                        return static_cast <int32_t> (get <Unsigned64> ());

                    default:
                        return static_cast <int32_t> (get <Real> ());
                }
            }

            throw std::bad_cast ();
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
            switch (index ())
            {
                case Integer:
                    return get <Integer> () >= 0;

                case Unsigned:
                    return true;

                case Integer64:
                    return get <Integer64> () >= 0 &&
                           get <Integer64> () <= static_cast <int64_t> (std::numeric_limits <uint32_t>::max ());

                case Unsigned64:
                    return get <Unsigned64> () <= static_cast <uint64_t> (std::numeric_limits <uint32_t>::max ());

                case Real:
                    return std::trunc (get <Real> ()) == get <Real> () &&
                           get <Real> () >= 0 &&
                           get <Real> () <= static_cast <double> (std::numeric_limits <uint32_t>::max ());

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
            if (isUint ())
            {
                switch (index ())
                {
                    case Integer:
                        return static_cast <uint32_t> (get <Integer> ());

                    case Unsigned:
                        return get <Unsigned> ();

                    case Integer64:
                        return static_cast <uint32_t> (get <Integer64> ());

                    case Unsigned64:
                        return static_cast <uint32_t> (get <Unsigned64> ());

                    default:
                        return static_cast <uint32_t> (get <Real> ());
                }
            }

            throw std::bad_cast ();
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
            switch (index ())
            {
                case Integer:
                case Unsigned:
                case Integer64:
                    return true;

                case Unsigned64:
                    return get <Unsigned64> () <= static_cast <uint64_t> (std::numeric_limits <int64_t>::max ());

                case Real:
                    return std::trunc (get <Real> ()) == get <Real> () &&
                           get <Real> () >= static_cast <double> (std::numeric_limits <int64_t>::min ()) &&
                           get <Real> () <  static_cast <double> (std::numeric_limits <int64_t>::max ());

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
            if (isInt64 ())
            {
                switch (index ())
                {
                    case Integer:
                        return static_cast <int64_t> (get <Integer> ());

                    case Unsigned:
                        return static_cast <int64_t> (get <Unsigned> ());

                    case Integer64:
                        return get <Integer64> ();

                    case Unsigned64:
                        return static_cast <int64_t> (get <Unsigned64> ());

                    default:
                        return static_cast <int64_t> (get <Real> ());
                }
            }

            throw std::bad_cast ();
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
            switch (index ())
            {
                case Integer:
                    return get <Integer> () >= 0;

                case Unsigned:
                    return true;

                case Integer64:
                    return get <Integer64> () >= 0;

                case Unsigned64:
                    return true;

                case Real:
                    return std::trunc (get <Real> ()) == get <Real> () &&
                           get <Real> () >= 0 &&
                           get <Real> () < static_cast <double> (std::numeric_limits <uint64_t>::max ());

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
            if (isUint64 ())
            {
                switch (index ())
                {
                    case Integer:
                        return static_cast <uint64_t> (get <Integer> ());

                    case Unsigned:
                        return static_cast <uint64_t> (get <Unsigned> ());

                    case Integer64:
                        return static_cast <uint64_t> (get <Integer64> ());

                    case Unsigned64:
                        return get <Unsigned64> ();

                    default:
                        return static_cast <uint64_t> (get <Real> ());
                }
            }

            throw std::bad_cast ();
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
            switch (index ())
            {
                case Integer:
                    return static_cast <float> (get <Integer> ());

                case Unsigned:
                    return static_cast <float> (get <Unsigned> ());

                case Integer64:
                    return static_cast <float> (get <Integer64> ());

                case Unsigned64:
                    return static_cast <float> (get <Unsigned64> ());

                case Real:
                    return static_cast <float> (get <Real> ());

                default:
                    break;
            }

            throw std::bad_cast ();
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
            switch (index ())
            {
                case Integer:
                    return static_cast <double> (get <Integer> ());

                case Unsigned:
                    return static_cast <double> (get <Unsigned> ());

                case Integer64:
                    return static_cast <double> (get <Integer64> ());

                case Unsigned64:
                    return static_cast <double> (get <Unsigned64> ());

                case Real:
                    return get <Real> ();

                default:
                    break;
            }

            throw std::bad_cast ();
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
            return index () == String;
        }

        /**
         * @brief get variable held by value as a string value.
         * @return variable held by value as a string value.
         * @throw std::bad_cast.
         */
        constexpr std::string& getString () const
        {
            return get <String> ();
        }

        /**
         * @brief explicit conversion function for string value.
         * @return converted string value.
         * @throw std::bad_cast.
         */
        explicit operator const char * () const
        {
            return getString ().c_str ();
        }

        /**
         * @brief check if the variable held by value is an array.
         * @return true if the variable held by value is an array, false otherwise.
         */
        constexpr bool isArray () const
        {
            return index () == ArrayValue;
        }

        /**
         * @brief get variable held by value as an array.
         * @return variable held by value as an array.
         * @throw std::bad_cast.
         */
        constexpr Array& getArray () const
        {
            return get <ArrayValue> ();
        }

        /**
         * @brief check if the variable held by value is an object.
         * @return true if the variable held by value is an object, false otherwise.
         */
        constexpr bool isObject () const
        {
            return index () == ObjectValue;
        }

        /**
         * @brief get variable held by value as an object.
         * @return variable held by value as an object.
         * @throw std::bad_cast.
         */
        constexpr Object& getObject () const
        {
            return get <ObjectValue> ();
        }

        /**
         * @brief returns a reference to the element at specified location pos.
         * @param pos Position of the element to return.
         * @return a reference to the requested element.
         * @throw std::bad_cast.
         */
        Value& at (size_t pos)
        {
            return get <ArrayValue> ().at (pos);
        }

        /**
         * @brief returns a reference to the element at specified location pos.
         * @param pos Position of the element to return.
         * @return a reference to the requested element.
         * @throw std::bad_cast.
         */
        const Value& at (size_t pos) const
        {
            return get <ArrayValue> ().at (pos);
        }

        /**
         * @brief returns a reference to the value that is mapped to a key.
         * @param key the key of the element to find.
         * @return a reference to the mapped element.
         * @throw std::bad_cast.
         */
        Value& at (const std::string& key)
        {
            for (auto& member : get <ObjectValue> ())
            {
                if (member.first == key)
                {
                    return member.second;
                }
            }

            throw std::out_of_range ("invalid key");
        }

        /**
         * @brief returns a reference to the value that is mapped to a key.
         * @param key the key of the element to find.
         * @return a reference to the mapped element.
         * @throw std::bad_cast.
         */
        const Value& at (const std::string& key) const
        {
            for (auto const& member : get <ObjectValue> ())
            {
                if (member.first == key)
                {
                    return member.second;
                }
            }

            throw std::out_of_range ("invalid key");
        }

        /**
         * @brief returns a reference to the element at specified location pos.
         * @param pos Position of the element to return.
         * @return a reference to the requested element.
         * @throw std::bad_cast.
         */
        Value& operator[] (size_t pos)
        {
            return get <ArrayValue> ()[pos];
        }

        /**
         * @brief returns a reference to the element at specified location pos.
         * @param pos Position of the element to return.
         * @return a reference to the requested element.
         * @throw std::bad_cast.
         */
        const Value& operator[] (size_t pos) const
        {
            return get <ArrayValue> ()[pos];
        }

        /**
         * @brief returns a reference to the value that is mapped to a key.
         * @param key the key of the element to find.
         * @return a reference to the mapped element.
         * @throw std::bad_cast.
         */
        Value& operator[] (const std::string& key)
        {
            if (index () == Null)
            {
                set <ObjectValue> ();
            }

            for (auto& member : get <ObjectValue> ())
            {
                if (member.first == key)
                {
                    return member.second;
                }
            }

            get <ObjectValue> ().emplace_back (key, nullptr);

            return get <ObjectValue> ().back ().second;
        }

        /**
         * @brief returns a reference to the value that is mapped to a key.
         * @param key the key of the element to find.
         * @return a reference to the mapped element.
         * @throw std::bad_cast.
         */
        const Value& operator[] (const std::string& key) const
        {
            for (auto& member : get <ObjectValue> ())
            {
                if (member.first == key)
                {
                    return member.second;
                }
            }

            get <ObjectValue> ().emplace_back (key, nullptr);

            return get <ObjectValue> ().back ().second;
        }

        /**
         * @brief check if the nested container is empty.
         * @return true if empty, false otherwise.
         * @throw std::bad_cast.
         */
        bool empty () const
        {
            switch (index ())
            {
                case String:
                    return get <String> ().empty ();

                case ArrayValue:
                    return get <ArrayValue> ().empty ();

                case ObjectValue:
                    return get <ObjectValue> ().empty ();

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
            switch (index ())
            {
                case String:
                    return get <String> ().size ();

                case ArrayValue:
                    return get <ArrayValue> ().size ();

                case ObjectValue:
                    return get <ObjectValue> ().size ();

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
            switch (index ())
            {
                case String:
                    get <String> ().reserve (cap);
                    return;

                case ArrayValue:
                    get <ArrayValue> ().reserve (cap);
                    return;

                case ObjectValue:
                    get <ObjectValue> ().reserve (cap);
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
            switch (index ())
            {
                case String:
                    get <String> ().clear ();
                    return;

                case ArrayValue:
                    get <ArrayValue> ().clear ();
                    return;

                case ObjectValue:
                    get <ObjectValue> ().clear ();
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
            get <ObjectValue> ().push_back (member);
            return get <ObjectValue> ().back ().second;
        }

        /**
         * @brief insert element in the nested container.
         * @param member the element to insert in the nested container.
         * @return a reference to the inserted element.
         * @throw std::bad_cast.
         */
        Value& insert (Member&& member)
        {
            get <ObjectValue> ().emplace_back (std::move (member));
            return get <ObjectValue> ().back ().second;
        }

        /**
         * @brief removes member with the key equivalent to key.
         * @param key key value of the member to remove.
         * @return number of elements removed.
         * @throw std::bad_cast.
         */
        size_t erase (const std::string& key)
        {
            auto beg = get <ObjectValue> ().begin ();
            auto end = get <ObjectValue> ().end ();

            for (auto it = beg; it != end; ++it)
            {
                if (it->first == key)
                {
                    get <ObjectValue> ().erase (it);
                    return 1;
                }
            }

            return 0;
        }

        /**
         * @brief appends element at the end of the nested container.
         * @param value the element to append.
         * @return a reference to the pushed object.
         * @throw std::bad_cast.
         */
        template <typename... Args>
        Value& emplaceBack (Args&&... args)
        {
            if (index () == Null)
            {
                set <ArrayValue> ();
            }
            return get <ArrayValue> ().emplace_back (std::forward <Args> (args)...);
        }

        /**
         * @brief appends element at the end of the nested container.
         * @param value the element to append.
         * @return a reference to the pushed object.
         * @throw std::bad_cast.
         */
        Value& pushBack (const Value& value)
        {
            if (index () == Null)
            {
                set <ArrayValue> ();
            }
            get <ArrayValue> ().push_back (value);
            return get <ArrayValue> ().back ();
        }

        /**
         * @brief appends element at the end of the nested container.
         * @param value the element to append.
         * @return a reference to the pushed object.
         * @throw std::bad_cast.
         */
        Value& pushBack (Value&& value)
        {
            if (index () == Null)
            {
                set <ArrayValue> ();
            }
            get <ArrayValue> ().emplace_back (std::move (value));
            return get <ArrayValue> ().back ();
        }

        /**
         * @brief removes the last element of the nested container.
         * @throw std::bad_cast.
         */
        void popBack ()
        {
            get <ArrayValue> ().pop_back ();
        }

        /**
         * @brief checks if the nested container contains an element at position pos.
         * @return true if the nested container contains an element at position pos, false otherwise.
         * @throw std::bad_cast.
         */
        bool contains (size_t pos) const
        {
            return get <ArrayValue> ().size () > pos;
        }

        /**
         * @brief checks if the nested container contains an element that is mapped to key.
         * @return true if the nested container contains an element mapped to key, false otherwise.
         * @throw std::bad_cast.
         */
        bool contains (const std::string& key) const
        {
            for (auto& member : get <ObjectValue> ())
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

        /**
         * @brief deserialize a document.
         * @param document document to deserialize.
         * @param length the length of the document to deserialize.
         * @return 0 on success, -1 otherwise.
         */
        template <typename Reader>
        int deserialize (const char* document, size_t length)
        {
            Reader reader (*this);
            return reader.deserialize (document, length);
        }

        /**
         * @brief deserialize a document.
         * @param first the first character of the document to deserialize.
         * @param last the last character of the document to deserialize.
         * @return 0 on success, -1 otherwise.
         */
        template <typename Reader>
        int deserialize (const char* first, const char* last)
        {
            Reader reader (*this);
            return reader.deserialize (first, last);
        }

        /**
         * @brief deserialize a document.
         * @param document document to deserialize.
         * @return 0 on success, -1 otherwise.
         */
        template <typename Reader>
        int deserialize (const std::string& document)
        {
            Reader reader (*this);
            return reader.deserialize (document);
        }

        /**
         * @brief deserialize a document.
         * @param document document to deserialize.
         * @return 0 on success, -1 otherwise.
         */
        template <typename Reader>
        int deserialize (std::istream& document)
        {
            Reader reader (*this);
            return reader.deserialize (document);
        }

        /**
         * @brief serialize data.
         * @param document Document where to serialize data.
         * @return 0 on success, -1 otherwise.
         */
        template <typename Writer>
        int serialize (std::ostream& document) const
        {
            Writer writer (document);
            return writer.serialize (*this);
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
        int jsonRead (const std::string& document);

        /**
         * @brief deserialize a json document.
         * @param document json document to deserialize.
         * @return 0 on success, -1 otherwise.
         */
        int jsonRead (std::istream& document);

        /**
         * @brief serialize json data.
         * @param document Document where to serialize json data.
         * @return 0 on success, -1 otherwise.
         */
        int jsonWrite (std::ostream& document, size_t indentation = 0);

        /**
         * @brief serialize canonicalized json data.
         * @param document Document where to serialize canonicalized json data.
         * @return 0 on success, -1 otherwise.
         */
        int jsonCanonicalize (std::ostream& document);

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
        int packRead (const std::string& document);

        /**
         * @brief deserialize a msgpack document.
         * @param document msgpack document to deserialize.
         * @return 0 on success, -1 otherwise.
         */
        int packRead (std::istream& document);

        /**
         * @brief serialize msgpack data.
         * @return 0 on success, -1 otherwise.
         */
        int packWrite (std::ostream& document);

        // friendship with equal operator.
        friend constexpr bool operator== (const Value& lhs, const Value& rhs);

        // friendship with lower operator.
        friend constexpr bool operator< (const Value& lhs, const Value& rhs);
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
                return rhs.getInt64 () >= 0 && uint64_t (rhs.getInt64 ()) == lhs.getUint64 () ;
            }
            else
            {
                return lhs.getDouble () == rhs.getDouble ();
            }
        }

        return lhs.equal (rhs);
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

        return lhs.lower (rhs);
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
