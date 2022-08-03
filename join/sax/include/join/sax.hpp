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
        virtual const char* name () const noexcept
        {
            return "join";
        }

        /**
         * @brief translate SAX API generic error code to human readable error string.
         * @param code error code.
         * @return human readable error string.
         */
        virtual std::string message (int code) const
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
    };

    /**
     * @brief get error category.
     * @return the created std::error_category object.
     */
    const std::error_category& saxCategory ()
    {
        static SaxCategory instance;
        return instance;
    }

    /**
     * @brief create an std::error_code object.
     * @param code error code number.
     * @return the created std::error_code object.
     */
    std::error_code make_error_code (SaxErrc code)
    {
        return std::error_code (static_cast <int> (code), saxCategory ());
    }

    /**
     * @brief create an std::error_condition object.
     * @param code error code number.
     * @return the created std::error_condition object.
     */
    std::error_condition make_error_condition (SaxErrc code)
    {
        return std::error_condition (static_cast <int> (code), saxCategory ());
    }

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
        virtual int stopArray ()
        {
            return 0;
        }

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
        virtual int stopObject ()
        {
            return 0;
        }
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
        StreamWriter (std::ostream& document)
        : SaxHandler (),
          _outstream (document)
        {
        }

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
         * @param value Value to serialize.
         * @return 0 on success, -1 otherwise.
         */
        virtual int serialize (const Value& value)
        {
            switch (value.index ())
            {
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
                    return setArray (value.get <Value::ArrayValue> ());

                case Value::ObjectValue:
                    return setObject (value.get <Value::ObjectValue> ());

                default:
                    return setNull ();
            }
        }

    protected:
        /**
         * @brief set array value.
         * @param value array value to set.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setArray (const Array& array)
        {
            startArray (array.size ());
            for (auto const& element : array)
            {
                serialize (element);
            }
            stopArray ();
            return 0;
        }

        /**
         * @brief set object value.
         * @param value array value to set.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setObject (const Object& object)
        {
            startObject (object.size ());
            for (auto const& member : object)
            {
                setKey (member.first);
                serialize (member.second);
            }
            stopObject ();
            return 0;
        }

        /**
         * @brief append data to output stream and update data size.
         * @param data data to append to output stream.
         * @param size data size.
         */
        void append (const char* data, uint32_t size)
        {
            _outstream.write (data, size);
        }

        /**
         * @brief append data to stream and update data size.
         * @param data data to append to stream.
         */
        void append (char data)
        {
            _outstream.put (data);
        }

        /// output stream.
        std::ostream& _outstream;
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
        StreamReader (Value& root)
        : SaxHandler (),
          _root (root)
        {
        }

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
        virtual int setNull () override
        {
            return setValue (Value (in_place_index_t <Value::Null> {}, nullptr));
        }

        /**
         * @brief set boolean value.
         * @param value boolean value to set.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setBool (bool value) override
        {
            return setValue (Value (in_place_index_t <Value::Boolean> {}, value));
        }

        /**
         * @brief set integer value.
         * @param value integer value to set.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setInt (int32_t value) override
        {
            return setValue (Value (in_place_index_t <Value::Integer> {}, value));
        }

        /**
         * @brief set unsigned integer value.
         * @param value unsigned integer value to set.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setUint (uint32_t value) override
        {
            return setValue (Value (in_place_index_t <Value::Unsigned> {}, value));
        }

        /**
         * @brief set 64 bits integer value.
         * @param value 64 bits integer value to set.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setInt64 (int64_t value) override
        {
            return setValue (Value (in_place_index_t <Value::Integer64> {}, value));
        }

        /**
         * @brief set unsigned 64 bits integer value.
         * @param value unsigned 64 bits integer value to set.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setUint64 (uint64_t value) override
        {
            return setValue (Value (in_place_index_t <Value::Unsigned64> {}, value));
        }

        /**
         * @brief set real value.
         * @param value real value to set.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setDouble (double value) override
        {
            return setValue (Value (in_place_index_t <Value::Real> {}, value));
        }

        /**
         * @brief set string value.
         * @param value string value to set.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setString (const std::string& value) override
        {
            return setValue (Value (in_place_index_t <Value::String> {}, value));
        }

        /**
         * @brief start array.
         * @param size array size.
         * @return 0 on success, -1 otherwise.
         */
        virtual int startArray (uint32_t size = 0) override
        {
            Array array;
            // reserve at least 2.
            array.reserve ((size) ? size : 2);

            if (_stack.empty ())
            {
                _root = std::move (array);
                _stack.push (&_root);
            }
            else
            {
                if (_stack.size () >= _maxdepth)
                {
                    join::lastError = make_error_code (SaxErrc::StackOverflow);
                    return -1;
                }

                Value::Ptr parent = _stack.top ();

                if (parent->is <Value::ObjectValue> ())
                {
                    _stack.push (&parent->insert (Member (_curkey, std::move (array))));
                    _curkey.clear ();
                }
                else
                {
                    _stack.push (&parent->pushBack (std::move (array)));
                }
            }

            return 0;
        }

        /**
         * @brief stop array.
         * @return 0 on success, -1 otherwise.
         */
        virtual int stopArray () override
        {
            if (_stack.size ())
            {
                _stack.pop ();
            }

            return 0;
        }

        /**
         * @brief start object.
         * @param size array size.
         * @return 0 on success, -1 otherwise.
         */
        virtual int startObject (uint32_t size = 0) override
        {
            Object object;
            // reserve at least 2.
            object.reserve ((size) ? size : 2);

            if (_stack.empty ())
            {
                _root = std::move (object);
                _stack.push (&_root);
            }
            else
            {
                if (_stack.size () >= _maxdepth)
                {
                    join::lastError = make_error_code (SaxErrc::StackOverflow);
                    return -1;
                }

                Value::Ptr parent = _stack.top ();

                if (parent->is <Value::ObjectValue> ())
                {
                    _stack.push (&parent->insert (Member (_curkey, std::move (object))));
                    _curkey.clear ();
                }
                else
                {
                    _stack.push (&parent->pushBack (std::move (object)));
                }
            }

            return 0;
        }

        /**
         * @brief set key.
         * @param key object key.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setKey (const std::string& key) override
        {
            _curkey = key;
            return 0;
        }

        /**
         * @brief stop object.
         * @return 0 on success, -1 otherwise.
         */
        virtual int stopObject () override
        {
            if (_stack.size ())
            {
                _stack.pop ();
            }

            return 0;
        }

        /**
         * @brief set value.
         * @param value value to set.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setValue (Value&& value)
        {
            if (_stack.empty ())
            {
                join::lastError = make_error_code (SaxErrc::InvalidParent);
                return -1;
            }

            Value::Ptr parent = _stack.top ();

            if (parent->is <Value::ObjectValue> ())
            {
                parent->insert (Member (_curkey, std::move (value)));
                _curkey.clear ();
            }
            else
            {
                parent->pushBack (std::move (value));
            }

            return 0;
        }

        /// max stack depth.
        static constexpr size_t _maxdepth = 19;

        /// stack.
        std::stack <Value*> _stack;

        /// current key.
        std::string _curkey;

        /// root.
        Value& _root;
    };
}

namespace std
{
    /// SAX API generic error code specialization.
    template <> struct is_error_condition_enum <join::SaxErrc> : public true_type {};
}

#endif
