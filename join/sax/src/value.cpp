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

// libjoin.
#include <join/value.hpp>

// C++.
#include <limits>

// C.
#include <cmath>

using join::sax::Array;
using join::sax::Member;
using join::sax::Object;
using join::sax::Value;

using join::sax::JsonWriter;
using join::sax::JsonReader;

// =========================================================================
//   CLASS     : Value
//   METHOD    : Value
// =========================================================================
Value::Value (const char* other)
: Variant (in_place_index_t <String> {}, other)
{
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : operator=
// =========================================================================
Value& Value::operator= (const char* other)
{
    Variant::operator= (Value (in_place_index_t <String> {}, other));
    return *this;
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : isNull
// =========================================================================
bool Value::isNull () const
{
    return index () == Null;
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : isBool
// =========================================================================
bool Value::isBool () const
{
    return index () == Boolean;
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : getBool
// =========================================================================
bool Value::getBool () const
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

// =========================================================================
//   CLASS     : Value
//   METHOD    : operator bool
// =========================================================================
Value::operator bool () const
{
    return getBool ();
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : isTrue
// =========================================================================
bool Value::isTrue () const
{
    return getBool ();
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : isFalse
// =========================================================================
bool Value::isFalse () const
{
    return !getBool ();
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : isNumber
// =========================================================================
bool Value::isNumber () const
{
    return index () == Integer || index () == Unsigned || index () == Integer64 || index () == Unsigned64 || index () == Real;
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : isInt8
// =========================================================================
bool Value::isInt8 () const
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

// =========================================================================
//   CLASS     : Value
//   METHOD    : getInt8
// =========================================================================
int8_t Value::getInt8 () const
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

            case Real:
                return static_cast <int8_t> (get <Real> ());

            default:
                break;
        }
    }

    throw std::bad_cast ();
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : operator int8_t
// =========================================================================
Value::operator int8_t () const
{
    return getInt8 ();
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : isUint8
// =========================================================================
bool Value::isUint8 () const
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

// =========================================================================
//   CLASS     : Value
//   METHOD    : getUint8
// =========================================================================
uint8_t Value::getUint8 () const
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

            case Real:
                return static_cast <uint8_t> (get <Real> ());

            default:
                break;
        }
    }

    throw std::bad_cast ();
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : operator uint8_t
// =========================================================================
Value::operator uint8_t () const
{
    return getUint8 ();
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : isInt16
// =========================================================================
bool Value::isInt16 () const
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

// =========================================================================
//   CLASS     : Value
//   METHOD    : getInt16
// =========================================================================
int16_t Value::getInt16 () const
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

            case Real:
                return static_cast <int16_t> (get <Real> ());

            default:
                break;
        }
    }

    throw std::bad_cast ();
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : operator int16_t
// =========================================================================
Value::operator int16_t () const
{
    return getInt16 ();
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : isUint16
// =========================================================================
bool Value::isUint16 () const
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

// =========================================================================
//   CLASS     : Value
//   METHOD    : getUint16
// =========================================================================
uint16_t Value::getUint16 () const
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

            case Real:
                return static_cast <uint16_t> (get <Real> ());

            default:
                break;
        }
    }

    throw std::bad_cast ();
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : operator uint16_t
// =========================================================================
Value::operator uint16_t () const
{
    return getUint16 ();
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : isInt
// =========================================================================
bool Value::isInt () const
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

// =========================================================================
//   CLASS     : Value
//   METHOD    : getInt
// =========================================================================
int32_t Value::getInt () const
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

            case Real:
                return static_cast <int32_t> (get <Real> ());

            default:
                break;
        }
    }

    throw std::bad_cast ();
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : operator int32_t
// =========================================================================
Value::operator int32_t () const
{
    return getInt ();
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : isUint
// =========================================================================
bool Value::isUint () const
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

// =========================================================================
//   CLASS     : Value
//   METHOD    : getUint
// =========================================================================
uint32_t Value::getUint () const
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

            case Real:
                return static_cast <uint32_t> (get <Real> ());

            default:
                break;
        }
    }

    throw std::bad_cast ();
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : operator uint32_t
// =========================================================================
Value::operator uint32_t () const
{
    return getUint ();
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : isInt64
// =========================================================================
bool Value::isInt64 () const
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

// =========================================================================
//   CLASS     : Value
//   METHOD    : getInt64
// =========================================================================
int64_t Value::getInt64 () const
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

            case Real:
                return static_cast <int64_t> (get <Real> ());

            default:
                break;
        }
    }

    throw std::bad_cast ();
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : operator int64_t
// =========================================================================
Value::operator int64_t () const
{
    return getInt64 ();
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : isUint64
// =========================================================================
bool Value::isUint64 () const
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

// =========================================================================
//   CLASS     : Value
//   METHOD    : getUint64
// =========================================================================
uint64_t Value::getUint64 () const
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

            case Real:
                return static_cast <uint64_t> (get <Real> ());

            default:
                break;
        }
    }

    throw std::bad_cast ();
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : operator uint64_t
// =========================================================================
Value::operator uint64_t () const
{
    return getUint64 ();
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : isFloat
// =========================================================================
bool Value::isFloat () const
{
    return isNumber ();
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : getFloat
// =========================================================================
float Value::getFloat () const
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

// =========================================================================
//   CLASS     : Value
//   METHOD    : operator float
// =========================================================================
Value::operator float () const
{
    return getFloat ();
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : isDouble
// =========================================================================
bool Value::isDouble () const
{
    return isNumber ();
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : getDouble
// =========================================================================
double Value::getDouble () const
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

// =========================================================================
//   CLASS     : Value
//   METHOD    : operator double
// =========================================================================
Value::operator double () const
{
    return getDouble ();
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : isString
// =========================================================================
bool Value::isString () const
{
    return index () == String;
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : getString
// =========================================================================
std::string& Value::getString () const
{
    return get <String> ();
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : operator const char *
// =========================================================================
Value::operator const char * () const
{
    return getString ().c_str ();
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : isArray
// =========================================================================
bool Value::isArray () const
{
    return index () == ArrayValue;
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : getArray
// =========================================================================
Array& Value::getArray () const
{
    return get <ArrayValue> ();
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : isObject
// =========================================================================
bool Value::isObject () const
{
    return index () == ObjectValue;
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : getObject
// =========================================================================
Object& Value::getObject () const
{
    return get <ObjectValue> ();
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : at
// =========================================================================
Value& Value::at (size_t pos)
{
    return get <ArrayValue> ().at (pos);
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : at
// =========================================================================
const Value& Value::at (size_t pos) const
{
    return get <ArrayValue> ().at (pos);
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : at
// =========================================================================
Value& Value::at (const std::string& key)
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

// =========================================================================
//   CLASS     : Value
//   METHOD    : at
// =========================================================================
const Value& Value::at (const std::string& key) const
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

// =========================================================================
//   CLASS     : Value
//   METHOD    : operator[]
// =========================================================================
Value& Value::operator[] (size_t pos)
{
    return get <ArrayValue> ()[pos];
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : operator[]
// =========================================================================
const Value& Value::operator[] (size_t pos) const
{
    return get <ArrayValue> ()[pos];
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : operator[]
// =========================================================================
Value& Value::operator[] (const std::string& key)
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

// =========================================================================
//   CLASS     : Value
//   METHOD    : operator[]
// =========================================================================
const Value& Value::operator[] (const std::string& key) const
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

// =========================================================================
//   CLASS     : Value
//   METHOD    : empty
// =========================================================================
bool Value::empty () const
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

// =========================================================================
//   CLASS     : Value
//   METHOD    : size
// =========================================================================
size_t Value::size () const
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

// =========================================================================
//   CLASS     : Value
//   METHOD    : reserve
// =========================================================================
void Value::reserve (size_t cap)
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

// =========================================================================
//   CLASS     : Value
//   METHOD    : clear
// =========================================================================
void Value::clear ()
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

// =========================================================================
//   CLASS     : Value
//   METHOD    : insert
// =========================================================================
Value& Value::insert (const Member& member)
{
    get <ObjectValue> ().push_back (member);
    return get <ObjectValue> ().back ().second;
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : insert
// =========================================================================
Value& Value::insert (Member&& member)
{
    get <ObjectValue> ().emplace_back (std::move (member));
    return get <ObjectValue> ().back ().second;
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : erase
// =========================================================================
size_t Value::erase (const std::string& key)
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

// =========================================================================
//   CLASS     : Value
//   METHOD    : pushBack
// =========================================================================
Value& Value::pushBack (const Value& value)
{
    get <ArrayValue> ().push_back (value);
    return get <ArrayValue> ().back ();
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : pushBack
// =========================================================================
Value& Value::pushBack (Value&& value)
{
    get <ArrayValue> ().emplace_back (std::move (value));
    return get <ArrayValue> ().back ();
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : popBack
// =========================================================================
void Value::popBack ()
{
    get <ArrayValue> ().pop_back ();
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : contains
// =========================================================================
bool Value::contains (size_t pos) const
{
    return get <ArrayValue> ().size () > pos;
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : contains
// =========================================================================
bool Value::contains (const std::string& key) const
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

// =========================================================================
//   CLASS     : Value
//   METHOD    : swap
// =========================================================================
void Value::swap (Value& other)
{
    Value temp (std::move (*this));
    *this = std::move (other);
    other = std::move (temp);
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : jsonRead
// =========================================================================
int Value::jsonRead (const char* document)
{
    JsonReader reader (*this);
    return reader.deserialize (document);
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : jsonRead
// =========================================================================
int Value::jsonRead (const char* document, size_t length)
{
    JsonReader reader (*this);
    return reader.deserialize (document, length);
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : jsonRead
// =========================================================================
int Value::jsonRead (const char* first, const char* last)
{
    JsonReader reader (*this);
    return reader.deserialize (first, last);
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : jsonRead
// =========================================================================
int Value::jsonRead (const std::string& document)
{
    JsonReader reader (*this);
    return reader.deserialize (document);
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : jsonRead
// =========================================================================
int Value::jsonRead (std::istream& document)
{
    JsonReader reader (*this);
    return reader.deserialize (document);
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : jsonWrite
// =========================================================================
bool Value::jsonWrite (std::ostream& document, size_t indentation) const
{
    JsonWriter writer (document, indentation);
    return writer.serialize (*this);
}

// =========================================================================
//   CLASS     :
//   METHOD    : operator==
// =========================================================================
bool join::sax::operator== (const Value& lhs, const Value& rhs)
{
    if (lhs.isNumber () && rhs.isNumber () && (lhs.index () != rhs.index ()))
    {
        if (lhs.isDouble () || rhs.isDouble ())
        {
            return lhs.getDouble () == rhs.getDouble ();
        }
        else if (lhs.isInt64 () && rhs.isUint64 ())
        {
            return lhs.getInt64 () >= 0 && uint64_t (lhs.getInt64 ()) == rhs.getUint64 ();
        }
        else if (lhs.isUint64 () && rhs.isInt64 ())
        {
            return rhs.getInt64 () >= 0 && uint64_t (rhs.getInt64 ()) == lhs.getUint64 () ;
        }
    }

    return lhs.equal (rhs);
}

// =========================================================================
//   CLASS     :
//   METHOD    : operator!=
// =========================================================================
bool join::sax::operator!= (const Value& lhs, const Value& rhs)
{
    return !(lhs == rhs);
}

// =========================================================================
//   CLASS     :
//   METHOD    : operator<
// =========================================================================
bool join::sax::operator< (const Value& lhs, const Value& rhs)
{
    if (lhs.isNumber () && rhs.isNumber () && (lhs.index () != rhs.index ()))
    {
        if (lhs.isDouble () || rhs.isDouble ())
        {
            return lhs.getDouble () < rhs.getDouble ();
        }
        else if (lhs.isInt64 () && rhs.isUint64 ())
        {
            return lhs.getInt64 () < 0 || uint64_t (lhs.getInt64 ()) < rhs.getUint64 ();
        }
        else if (lhs.isUint64 () && rhs.isInt64 ())
        {
            return rhs.getInt64 () >= 0 && lhs.getUint64 () < uint64_t (rhs.getInt64 ());
        }
    }

    return lhs.lower (rhs);
}

// =========================================================================
//   CLASS     :
//   METHOD    : operator>
// =========================================================================
bool join::sax::operator> (const Value& lhs, const Value& rhs)
{
    return rhs < lhs;
}

// =========================================================================
//   CLASS     :
//   METHOD    : operator<=
// =========================================================================
bool join::sax::operator<= (const Value& lhs, const Value& rhs)
{
    return !(rhs < lhs);
}

// =========================================================================
//   CLASS     :
//   METHOD    : operator>=
// =========================================================================
bool join::sax::operator>= (const Value& lhs, const Value& rhs)
{
    return !(lhs < rhs);
}
