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
#include <join/sax.hpp>

// C++.
#include <sstream>

// C.
#include <cstring>

using join::sax::Value;
using join::sax::Array;
using join::sax::Member;
using join::sax::Object;

using join::sax::SaxErrc;
using join::sax::SaxCategory;
using join::sax::SaxHandler;

using join::sax::StreamWriter;
using join::sax::StreamReader;

// =========================================================================
//   CLASS     : SaxCategory
//   METHOD    : name
// =========================================================================
const char* SaxCategory::name () const noexcept
{
    return "libjoin";
}

// =========================================================================
//   CLASS     : SaxCategory
//   METHOD    : message
// =========================================================================
std::string SaxCategory::message (int code) const
{
    switch (static_cast <SaxErrc> (code))
    {
        case SaxErrc::StackOverflow:
            return "stack overflow";
        case SaxErrc::InvalidParent:
            return "parent not an array nor an object";
        case SaxErrc::InvalidValue:
            return "value is invalid";
        case SaxErrc::ExtraData:
            return "extra data detected";
        default:
            return "success";
    }
}

// =========================================================================
//   CLASS     :
//   METHOD    : saxCategory
// =========================================================================
const std::error_category& join::sax::saxCategory ()
{
    static SaxCategory instance;
    return instance;
}

// =========================================================================
//   CLASS     :
//   METHOD    : make_error_code
// =========================================================================
std::error_code join::sax::make_error_code (SaxErrc code)
{
    return std::error_code (static_cast <int> (code), saxCategory ());
}

// =========================================================================
//   CLASS     :
//   METHOD    : make_error_condition
// =========================================================================
std::error_condition join::sax::make_error_condition (SaxErrc code)
{
    return std::error_condition (static_cast <int> (code), saxCategory ());
}

// =========================================================================
//   CLASS     : StreamWriter
//   METHOD    : StreamWriter
// =========================================================================
StreamWriter::StreamWriter (std::ostream& document)
: SaxHandler (),
  outstream_ (document)
{
}

// =========================================================================
//   CLASS     : StreamWriter
//   METHOD    : serialize
// =========================================================================
int StreamWriter::serialize (const Value& value)
{
    switch (value.index ())
    {
        case Value::Null:
            return setNull ();

        case Value::Boolean:
            return setBool (value.get <Value::Boolean> ());

        case Value::Integer:
            return setInt (value.get <Value::Integer> ());

        case Value::Unsigned:
            return setUint (value.get <Value::Unsigned> ());

        case Value::Integer64:
            return setInt64 (value.get <Value::Integer64> ());

        case Value::Unsigned64:
            return setUint64 (value.get <Value::Unsigned64> ());

        case Value::Real:
            return setDouble (value.get <Value::Real> ());

        case Value::String:
            return setString (value.get <Value::String> ());

        case Value::ArrayValue:
            if (startArray (value.size ()) == -1)
            {
                return -1;
            }
            for (auto const& element : value.getArray ())
            {
                if (serialize (element) == -1)
                {
                    return -1;
                }
            }
            return stopArray ();

        case Value::ObjectValue:
            if (startObject (value.size ()) == -1)
            {
                return -1;
            }
            for (auto const& element : value.getObject ())
            {
                if ((setKey (element.first) == -1) || (serialize (element.second) == -1))
                {
                    return -1;
                }
            }
            return stopObject ();

        default:
            join::lastError = make_error_code (SaxErrc::InvalidValue);
            break;
    }

    return -1;
}

// =========================================================================
//   CLASS     : StreamWriter
//   METHOD    : append
// =========================================================================
void StreamWriter::append (const char* data, uint32_t size)
{
    outstream_.write (data, size);
}

// =========================================================================
//   CLASS     : StreamWriter
//   METHOD    : append
// =========================================================================
void StreamWriter::append (char data)
{
    outstream_.put (data);
}

// =========================================================================
//   CLASS     : StreamReader
//   METHOD    : StreamReader
// =========================================================================
StreamReader::StreamReader (Value& root)
: SaxHandler (),
  root_ (root)
{
}

// =========================================================================
//   CLASS     : StreamReader
//   METHOD    : setNull
// =========================================================================
int StreamReader::setNull ()
{
    return setValue (Value (in_place_index_t <Value::Null> {}, nullptr));
}

// =========================================================================
//   CLASS     : StreamReader
//   METHOD    : setBool
// =========================================================================
int StreamReader::setBool (bool value)
{
    return setValue (Value (in_place_index_t <Value::Boolean> {}, value));
}

// =========================================================================
//   CLASS     : StreamReader
//   METHOD    : setInt
// =========================================================================
int StreamReader::setInt (int32_t value)
{
    return setValue (Value (in_place_index_t <Value::Integer> {}, value));
}

// =========================================================================
//   CLASS     : StreamReader
//   METHOD    : setUint
// =========================================================================
int StreamReader::setUint (uint32_t value)
{
    return setValue (Value (in_place_index_t <Value::Unsigned> {}, value));
}

// =========================================================================
//   CLASS     : StreamReader
//   METHOD    : setInt64
// =========================================================================
int StreamReader::setInt64 (int64_t value)
{
    return setValue (Value (in_place_index_t <Value::Integer64> {}, value));
}

// =========================================================================
//   CLASS     : StreamReader
//   METHOD    : setUint64
// =========================================================================
int StreamReader::setUint64 (uint64_t value)
{
    return setValue (Value (in_place_index_t <Value::Unsigned64> {}, value));
}

// =========================================================================
//   CLASS     : StreamReader
//   METHOD    : setDouble
// =========================================================================
int StreamReader::setDouble (double value)
{
    return setValue (Value (in_place_index_t <Value::Real> {}, value));
}

// =========================================================================
//   CLASS     : StreamReader
//   METHOD    : setString
// =========================================================================
int StreamReader::setString (const std::string& value)
{
    return setValue (Value (in_place_index_t <Value::String> {}, value));
}

// =========================================================================
//   CLASS     : StreamReader
//   METHOD    : startArray
// =========================================================================
int StreamReader::startArray (uint32_t size)
{
    Array array;
    // reserve at least 2.
    array.reserve ((size) ? size : 2);

    if (stack_.empty ())
    {
        root_ = std::move (array);
        stack_.push (&root_);
    }
    else
    {
        if (stack_.size () >= maxdepth_)
        {
            join::lastError = make_error_code (SaxErrc::StackOverflow);
            return -1;
        }

        Value::Ptr parent = stack_.top ();

        if (parent->is <Value::ObjectValue> ())
        {
            stack_.push (&parent->insert (Member (curkey_, std::move (array))));
            curkey_.clear ();
        }
        else if (parent->is <Value::ArrayValue> ())
        {
            stack_.push (&parent->pushBack (std::move (array)));
        }
        else
        {
            join::lastError = make_error_code (SaxErrc::InvalidParent);
            return -1;
        }
    }

    return 0;
}

// =========================================================================
//   CLASS     : StreamReader
//   METHOD    : stopArray
// =========================================================================
int StreamReader::stopArray ()
{
    if (stack_.size ())
    {
        stack_.pop ();
    }

    return 0;
}

// =========================================================================
//   CLASS     : StreamReader
//   METHOD    : startObject
// =========================================================================
int StreamReader::startObject (uint32_t size)
{
    Object object;
    // reserve at least 2.
    object.reserve ((size) ? size : 2);

    if (stack_.empty ())
    {
        root_ = std::move (object);
        stack_.push (&root_);
    }
    else
    {
        if (stack_.size () >= maxdepth_)
        {
            join::lastError = make_error_code (SaxErrc::StackOverflow);
            return -1;
        }

        Value::Ptr parent = stack_.top ();

        if (parent->is <Value::ObjectValue> ())
        {
            stack_.push (&parent->insert (Member (curkey_, std::move (object))));
            curkey_.clear ();
        }
        else if (parent->is <Value::ArrayValue> ())
        {
            stack_.push (&parent->pushBack (std::move (object)));
        }
        else
        {
            join::lastError = make_error_code (SaxErrc::InvalidParent);
            return -1;
        }
    }

    return 0;
}

// =========================================================================
//   CLASS     : StreamReader
//   METHOD    : setKey
// =========================================================================
int StreamReader::setKey (const std::string& key)
{
    curkey_ = key;
    return 0;
}

// =========================================================================
//   CLASS     : StreamReader
//   METHOD    : stopObject
// =========================================================================
int StreamReader::stopObject ()
{
    if (stack_.size ())
    {
        stack_.pop ();
    }

    return 0;
}

// =========================================================================
//   CLASS     : StreamReader
//   METHOD    : setValue
// =========================================================================
int StreamReader::setValue (Value&& value)
{
    if (stack_.empty ())
    {
        join::lastError = make_error_code (SaxErrc::InvalidParent);
        return -1;
    }

    Value::Ptr parent = stack_.top ();

    if (parent->is <Value::ObjectValue> ())
    {
        parent->insert (Member (curkey_, std::move (value)));
        curkey_.clear ();
    }
    else if (parent->is <Value::ArrayValue> ())
    {
        parent->pushBack (std::move (value));
    }
    else
    {
        join::lastError = make_error_code (SaxErrc::InvalidParent);
        return -1;
    }

    return 0;
}
