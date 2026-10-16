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

#ifndef JOIN_DATA_SAX_HPP
#define JOIN_DATA_SAX_HPP

// libjoin.
#include <join/error.hpp>
#include <join/value.hpp>
#include <join/utils.hpp>
#include <join/view.hpp>

// C++.
#include <system_error>
#include <iterator>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// C.
#include <cstring>
#include <cstddef>

namespace join
{
    /// forward declaration.
    class Value;

    /**
     * @brief SAX API generic error codes.
     */
    enum class SaxErrc
    {
        StackOverflow = 1, /**< Stack overflow. */
        InvalidParent,     /**< Parent not an array nor an object. */
        InvalidKey,        /**< Key is invalid. */
        InvalidValue,      /**< Value is invalid. */
        ExtraData          /**< Extra data detected. */
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
    const std::error_category& saxCategory () noexcept;

    /**
     * @brief create an std::error_code object.
     * @param code error code number.
     * @return the created std::error_code object.
     */
    std::error_code make_error_code (SaxErrc code) noexcept;

    /**
     * @brief create an std::error_condition object.
     * @param code error code number.
     * @return the created std::error_condition object.
     */
    std::error_condition make_error_condition (SaxErrc code) noexcept;

    /**
     * @brief stream serializer class.
     */
    template <typename Handler>
    class SaxWriter
    {
        /// handler allowed to use the traversal.
        friend Handler;

    public:
        /**
         * @brief create instance.
         * @param document document to create.
         * @param args arguments forwarded to the handler.
         */
        template <typename... Args>
        explicit SaxWriter (std::ostream& document, Args&&... args)
        : _out (document.rdbuf ())
        , _handler (*this, std::forward<Args> (args)...)
        {
        }

        /**
         * @brief Create the Writer instance by copy.
         * @param other Other object to copy.
         */
        SaxWriter (const SaxWriter& other) = delete;

        /**
         * @brief Assign the Writer instance by copy.
         * @param other Other object to copy.
         */
        SaxWriter& operator= (const SaxWriter& other) = delete;

        /**
         * @brief Create the Writer instance by move.
         * @param other Other object to move.
         */
        SaxWriter (SaxWriter&& other) = delete;

        /**
         * @brief Assign the Writer instance by move.
         * @param other Other object to move.
         */
        SaxWriter& operator= (SaxWriter&& other) = delete;

        /**
         * @brief Destroy the Writer instance.
         */
        ~SaxWriter () = default;

        /**
         * @brief Serialize data.
         * @param value Value to serialize.
         * @return 0 on success, -1 otherwise.
         */
        int serialize (const Value& value)
        {
            _capacity = sizeof (_buffer);
            int result;
            try
            {
                result = setValue (value);
            }
            catch (...)
            {
                flush ();
                _capacity = 0;
                throw;
            }
            flush ();
            _capacity = 0;
            return result;
        }

        /**
         * @brief set null value.
         * @return 0 on success, -1 otherwise.
         */
        int setNull ()
        {
            return _handler.setNull ();
        }

        /**
         * @brief set boolean value.
         * @param value boolean value to set.
         * @return 0 on success, -1 otherwise.
         */
        int setBool (bool value)
        {
            return _handler.setBool (value);
        }

        /**
         * @brief set integer value.
         * @param value integer value to set.
         * @return 0 on success, -1 otherwise.
         */
        int setInt (int32_t value)
        {
            return _handler.setInt (value);
        }

        /**
         * @brief set unsigned integer value.
         * @param value unsigned integer value to set.
         * @return 0 on success, -1 otherwise.
         */
        int setUint (uint32_t value)
        {
            return _handler.setUint (value);
        }

        /**
         * @brief set 64 bits integer value.
         * @param value 64 bits integer value to set.
         * @return 0 on success, -1 otherwise.
         */
        int setInt64 (int64_t value)
        {
            return _handler.setInt64 (value);
        }

        /**
         * @brief set unsigned 64 bits integer value.
         * @param value unsigned 64 bits integer value to set.
         * @return 0 on success, -1 otherwise.
         */
        int setUint64 (uint64_t value)
        {
            return _handler.setUint64 (value);
        }

        /**
         * @brief set real value.
         * @param value real value to set.
         * @return 0 on success, -1 otherwise.
         */
        int setDouble (double value)
        {
            return _handler.setDouble (value);
        }

        /**
         * @brief set string value.
         * @param value string value to set.
         * @return 0 on success, -1 otherwise.
         */
        int setString (const std::string& value)
        {
            return _handler.setString (value);
        }

        /**
         * @brief start array.
         * @param size array size.
         * @return 0 on success, -1 otherwise.
         */
        int startArray (uint32_t size = 0)
        {
            return _handler.startArray (size);
        }

        /**
         * @brief stop array.
         * @return 0 on success, -1 otherwise.
         */
        int stopArray ()
        {
            return _handler.stopArray ();
        }

        /**
         * @brief start object.
         * @param size object size.
         * @return 0 on success, -1 otherwise.
         */
        int startObject (uint32_t size = 0)
        {
            return _handler.startObject (size);
        }

        /**
         * @brief set key.
         * @param key object key.
         * @return 0 on success, -1 otherwise.
         */
        int setKey (const Value& key)
        {
            return _handler.setKey (key);
        }

        /**
         * @brief stop object.
         * @return 0 on success, -1 otherwise.
         */
        int stopObject ()
        {
            return _handler.stopObject ();
        }

    protected:
        /**
         * @brief set value.
         * @param value Value to write.
         * @return 0 on success, -1 otherwise.
         */
        int setValue (const Value& value)
        {
            switch (value.index ())
            {
                case Value::Boolean:
                    return setBool (value.get<Value::Boolean> ());

                case Value::Integer:
                    return setInt (value.get<Value::Integer> ());

                case Value::Unsigned:
                    return setUint (value.get<Value::Unsigned> ());

                case Value::Integer64:
                    return setInt64 (value.get<Value::Integer64> ());

                case Value::Unsigned64:
                    return setUint64 (value.get<Value::Unsigned64> ());

                case Value::Real:
                    return setDouble (value.get<Value::Real> ());

                case Value::String:
                    return setString (value.get<Value::String> ());

                case Value::ArrayValue:
                    return setArray (value.get<Value::ArrayValue> ());

                case Value::ObjectValue:
                    return setObject (value.get<Value::ObjectValue> ());

                default:
                    return setNull ();
            }
        }

        /**
         * @brief set array value.
         * @param value array value to set.
         * @return 0 on success, -1 otherwise.
         */
        int setArray (const Array& array)
        {
            if (startArray (array.size ()) == -1)
            {
                return -1;
            }

            for (auto const& element : array)
            {
                if (setValue (element) == -1)
                {
                    return -1;
                }
            }

            return stopArray ();
        }

        /**
         * @brief set object value.
         * @param value object value to set.
         * @return 0 on success, -1 otherwise.
         */
        int setObject (const Object& object)
        {
            if (startObject (object.size ()) == -1)
            {
                return -1;
            }

            for (const Member& member : _handler.members (object))
            {
                if (setKey (member.first) == -1)
                {
                    return -1;
                }

                if (setValue (member.second) == -1)
                {
                    return -1;
                }
            }

            return stopObject ();
        }

        /**
         * @brief append character to output stream in batch.
         * @param data data to append to output stream.
         */
        inline void append (char data)
        {
            if (JOIN_UNLIKELY (_size == _capacity))
            {
                flush ();
                if (_capacity == 0)
                {
                    _out->sputc (data);
                    return;
                }
            }
            _buffer[_size++] = data;
        }

        /**
         * @brief append data to output stream in batch.
         * @param data data to append to output stream.
         * @param size data size.
         */
        inline void append (const char* data, uint32_t size)
        {
            if (JOIN_UNLIKELY (size > _capacity - _size))
            {
                flush ();
                if (size >= _capacity)
                {
                    _out->sputn (data, size);
                    return;
                }
            }
            std::memcpy (_buffer + _size, data, size);
            _size += size;
        }

        /**
         * @brief flush buffered data to output stream.
         */
        inline void flush ()
        {
            if (_size)
            {
                _out->sputn (_buffer, _size);
                _size = 0;
            }
        }

    protected:
        /// underlying output stream.
        std::streambuf* _out;

        /// handler.
        Handler _handler;

        /// output buffer.
        char _buffer[1024];

        /// output buffer size.
        size_t _size = 0;

        /// output buffer capacity, 0 outside of serialize.
        size_t _capacity = 0;
    };

    /**
     * @brief stream deserializer class.
     */
    template <typename Handler>
    class SaxReader
    {
        /// handler allowed to build the document.
        friend Handler;

    public:
        /**
         * @brief default constructor.
         * @param root Value to write.
         */
        explicit SaxReader (Value& root)
        : _handler (*this)
        , _stack (stack ())
        , _root (root)
        {
        }

        /**
         * @brief copy constructor.
         * @param other object to copy.
         */
        SaxReader (const SaxReader& other) = delete;

        /**
         * @brief copy assignment.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        SaxReader& operator= (const SaxReader& other) = delete;

        /**
         * @brief move constructor.
         * @param other object to move.
         */
        SaxReader (SaxReader&& other) = delete;

        /**
         * @brief move assignment.
         * @param other object to move.
         * @return a reference of the current object.
         */
        SaxReader& operator= (SaxReader&& other) = delete;

        /**
         * @brief destroy instance.
         */
        ~SaxReader () = default;

        /**
         * @brief deserialize a document.
         * @param document document to deserialize.
         * @param length The length of the document to parse.
         * @return 0 on success, -1 otherwise.
         */
        int deserialize (const char* document, size_t length)
        {
            StringView in (document, length);
            return read (in);
        }

        /**
         * @brief deserialize a document.
         * @param first The first character of the document to parse.
         * @param last The last character of the document to parse.
         * @return 0 on success, -1 otherwise.
         */
        int deserialize (const char* first, const char* last)
        {
            StringView in (first, last);
            return read (in);
        }

        /**
         * @brief deserialize a document.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        int deserialize (const std::string& document)
        {
            StringView in (document.c_str (), document.size ());
            return read (in);
        }

        /**
         * @brief deserialize a document.
         * @param document document to deserialize.
         * @return 0 on success, -1 otherwise.
         */
        int deserialize (std::stringstream& document)
        {
            StringStreamView in (document);
            return read (in);
        }

        /**
         * @brief deserialize a document.
         * @param document document to deserialize.
         * @return 0 on success, -1 otherwise.
         */
        int deserialize (std::istringstream& document)
        {
            StringStreamView in (document);
            return read (in);
        }

        /**
         * @brief deserialize a document.
         * @param document document to deserialize.
         * @return 0 on success, -1 otherwise.
         */
        int deserialize (std::fstream& document)
        {
            FileStreamView in (document);
            return read (in);
        }

        /**
         * @brief deserialize a document.
         * @param document document to deserialize.
         * @return 0 on success, -1 otherwise.
         */
        int deserialize (std::ifstream& document)
        {
            FileStreamView in (document);
            return read (in);
        }

        /**
         * @brief deserialize a document.
         * @param document document to deserialize.
         * @return 0 on success, -1 otherwise.
         */
        int deserialize (std::iostream& document)
        {
            StreamView in (document);
            return read (in);
        }

        /**
         * @brief deserialize a document.
         * @param document document to deserialize.
         * @return 0 on success, -1 otherwise.
         */
        int deserialize (std::istream& document)
        {
            StreamView in (document);
            return read (in);
        }

    protected:
        /**
         * @brief parse a document.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType>
        int read (ViewType& document)
        {
            _depth = 0;
            return _handler.read (document);
        }

        /**
         * @brief set null value.
         * @return 0 on success, -1 otherwise.
         */
        int setNull ()
        {
            return emplaceValue<Value::Null> (nullptr);
        }

        /**
         * @brief set boolean value.
         * @param value boolean value to set.
         * @return 0 on success, -1 otherwise.
         */
        int setBool (bool value)
        {
            return emplaceValue<Value::Boolean> (value);
        }

        /**
         * @brief set integer value.
         * @param value integer value to set.
         * @return 0 on success, -1 otherwise.
         */
        int setInt (int32_t value)
        {
            return emplaceValue<Value::Integer> (value);
        }

        /**
         * @brief set unsigned integer value.
         * @param value unsigned integer value to set.
         * @return 0 on success, -1 otherwise.
         */
        int setUint (uint32_t value)
        {
            return emplaceValue<Value::Unsigned> (value);
        }

        /**
         * @brief set 64 bits integer value.
         * @param value 64 bits integer value to set.
         * @return 0 on success, -1 otherwise.
         */
        int setInt64 (int64_t value)
        {
            return emplaceValue<Value::Integer64> (value);
        }

        /**
         * @brief set unsigned 64 bits integer value.
         * @param value unsigned 64 bits integer value to set.
         * @return 0 on success, -1 otherwise.
         */
        int setUint64 (uint64_t value)
        {
            return emplaceValue<Value::Unsigned64> (value);
        }

        /**
         * @brief set real value.
         * @param value real value to set.
         * @return 0 on success, -1 otherwise.
         */
        int setDouble (double value)
        {
            return emplaceValue<Value::Real> (value);
        }

        /**
         * @brief set string value.
         * @param value string value to move.
         * @return 0 on success, -1 otherwise.
         */
        int setString (std::string&& value)
        {
            return emplaceValue<Value::String> (std::move (value));
        }

        /**
         * @brief start array.
         * @param size array size (ignored).
         * @return 0 on success, -1 otherwise.
         */
        int startArray ([[maybe_unused]] uint32_t size = 0)
        {
            if (JOIN_UNLIKELY (_depth == 0))
            {
                _stack.clear ();
            }
            else if (JOIN_UNLIKELY (_depth >= _maxdepth))
            {
                join::lastError = make_error_code (SaxErrc::StackOverflow);
                return -1;
            }

            _starts[_depth++] = _stack.size ();

            return 0;
        }

        /**
         * @brief stop array.
         * @return 0 on success, -1 otherwise.
         */
        int stopArray ()
        {
            if (JOIN_UNLIKELY (_depth == 0))
            {
                return 0;  // LCOV_EXCL_LINE
            }

            auto first = _stack.begin () + _starts[--_depth];
            Array array (std::make_move_iterator (first), std::make_move_iterator (_stack.end ()));
            _stack.erase (first, _stack.end ());

            if (_depth == 0)
            {
                _root = std::move (array);
                return 0;
            }

            _stack.emplace_back (in_place_index_t<Value::ArrayValue>{}, std::move (array));

            return 0;
        }

        /**
         * @brief start object.
         * @param size object size (ignored).
         * @return 0 on success, -1 otherwise.
         */
        int startObject ([[maybe_unused]] uint32_t size = 0)
        {
            if (JOIN_UNLIKELY (_depth == 0))
            {
                _stack.clear ();
            }
            else if (JOIN_UNLIKELY (_depth >= _maxdepth))
            {
                join::lastError = make_error_code (SaxErrc::StackOverflow);
                return -1;
            }

            _starts[_depth++] = _stack.size ();

            return 0;
        }

        /**
         * @brief set key.
         * @param key object key to move.
         * @return 0 on success, -1 otherwise.
         */
        int setKey (Value&& key)
        {
            _stack.emplace_back (std::move (key));
            return 0;
        }

        /**
         * @brief stop object.
         * @return 0 on success, -1 otherwise.
         */
        int stopObject ()
        {
            if (JOIN_UNLIKELY (_depth == 0))
            {
                return 0;  // LCOV_EXCL_LINE
            }

            auto first = _stack.begin () + _starts[--_depth];
            Object object;
            object.reserve ((_stack.end () - first) / 2);
            for (auto it = first; it != _stack.end (); it += 2)
            {
                object.emplace_back (std::move (it[0]), std::move (it[1]));
            }
            _stack.erase (first, _stack.end ());

            if (_depth == 0)
            {
                _root = std::move (object);
                return 0;
            }

            _stack.emplace_back (in_place_index_t<Value::ObjectValue>{}, std::move (object));

            return 0;
        }

        /**
         * @brief construct a value in place in the current container.
         * @param args arguments forwarded to the value constructor.
         * @return 0 on success, -1 otherwise.
         */
        template <size_t I, typename... Args>
        int emplaceValue (Args&&... args)
        {
            if (JOIN_UNLIKELY (_depth == 0))
            {
                join::lastError = make_error_code (SaxErrc::InvalidParent);
                return -1;
            }

            _stack.emplace_back (in_place_index_t<I>{}, std::forward<Args> (args)...);

            return 0;
        }

        /**
         * @brief get the value stack of the calling thread.
         * @return value stack.
         */
        static std::vector<Value>& stack () noexcept
        {
            thread_local std::vector<Value> values;
            return values;
        }

        /// handler.
        Handler _handler;

        /// max stack depth.
        static constexpr size_t _maxdepth = 19;

        /// start of each open container in _stack.
        size_t _starts[_maxdepth] = {};

        /// number of open containers.
        size_t _depth = 0;

        /// stack of the values of the open containers.
        std::vector<Value>& _stack;

        /// root.
        Value& _root;
    };
}

namespace std
{
    /// SAX API generic error code specialization.
    template <>
    struct is_error_condition_enum<join::SaxErrc> : public true_type
    {
    };
}

#endif
