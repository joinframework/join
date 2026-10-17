/**
 * MIT License
 *
 * Copyright (c) 2026 Mathieu Rabine
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

#ifndef JOIN_DATA_DOCUMENT_HPP
#define JOIN_DATA_DOCUMENT_HPP

// libjoin.
#include <join/value.hpp>
#include <join/utils.hpp>
#include <join/sax.hpp>

// C++.
#include <iterator>
#include <utility>
#include <string>
#include <vector>

// C.
#include <cstddef>

namespace join
{
    /**
     * @brief document class, filled by balanced SAX calls.
     */
    class Document
    {
    public:
        /**
         * @brief default constructor.
         */
        Document () = default;

        /**
         * @brief copy constructor.
         * @param other object to copy.
         */
        Document (const Document& other) = delete;

        /**
         * @brief copy assignment.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        Document& operator= (const Document& other) = delete;

        /**
         * @brief move constructor.
         * @param other object to move.
         */
        Document (Document&& other) = delete;

        /**
         * @brief move assignment.
         * @param other object to move.
         * @return a reference of the current object.
         */
        Document& operator= (Document&& other) = delete;

        /**
         * @brief destroy instance.
         */
        ~Document () = default;

        /**
         * @brief get the root value.
         * @return root value.
         */
        Value& root () noexcept
        {
            return _root;
        }

        /**
         * @brief get the root value.
         * @return root value.
         */
        const Value& root () const noexcept
        {
            return _root;
        }

        /**
         * @brief serialize the document, resetting the handler on failure.
         * @param handler handler receiving the document.
         * @return 0 on success, -1 otherwise.
         */
        template <typename Handler>
        int serialize (Handler& handler) const
        {
            try
            {
                if (JOIN_LIKELY (setValue (_root, handler) == 0))
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
         * @param value string value to set.
         * @return 0 on success, -1 otherwise.
         */
        int setString (const std::string& value)
        {
            return emplaceValue<Value::String> (value);
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
            else if (JOIN_UNLIKELY (_depth >= saxMaxDepth))
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
                join::lastError = make_error_code (SaxErrc::InvalidParent);
                return -1;
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
            else if (JOIN_UNLIKELY (_depth >= saxMaxDepth))
            {
                join::lastError = make_error_code (SaxErrc::StackOverflow);
                return -1;
            }

            _starts[_depth++] = _stack.size ();

            return 0;
        }

        /**
         * @brief get object members in writing order.
         * @param object object to write.
         * @return object members in insertion order.
         */
        const Object& members (const Object& object) const noexcept
        {
            return object;
        }

        /**
         * @brief set key.
         * @param key object key to set.
         * @return 0 on success, -1 otherwise.
         */
        int setKey (const Value& key)
        {
            if (JOIN_UNLIKELY (_depth == 0))
            {
                join::lastError = make_error_code (SaxErrc::InvalidParent);
                return -1;
            }

            _stack.emplace_back (key);
            return 0;
        }

        /**
         * @brief set key.
         * @param key object key to move.
         * @return 0 on success, -1 otherwise.
         */
        int setKey (Value&& key)
        {
            if (JOIN_UNLIKELY (_depth == 0))
            {
                join::lastError = make_error_code (SaxErrc::InvalidParent);
                return -1;
            }

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
                join::lastError = make_error_code (SaxErrc::InvalidParent);
                return -1;
            }

            if (JOIN_UNLIKELY ((_stack.size () - _starts[_depth - 1]) & 1))
            {
                join::lastError = make_error_code (SaxErrc::InvalidKey);
                return -1;
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
         * @brief reset the parsing state, leaving the root untouched.
         */
        void reset () noexcept
        {
            _depth = 0;
            _stack.clear ();
        }

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
                case Value::Boolean:
                    return handler.setBool (value.get<Value::Boolean> ());

                case Value::Integer:
                    return handler.setInt (value.get<Value::Integer> ());

                case Value::Unsigned:
                    return handler.setUint (value.get<Value::Unsigned> ());

                case Value::Integer64:
                    return handler.setInt64 (value.get<Value::Integer64> ());

                case Value::Unsigned64:
                    return handler.setUint64 (value.get<Value::Unsigned64> ());

                case Value::Real:
                    return handler.setDouble (value.get<Value::Real> ());

                case Value::String:
                    return handler.setString (value.get<Value::String> ());

                case Value::ArrayValue:
                    return setArray (value.get<Value::ArrayValue> (), handler);

                case Value::ObjectValue:
                    return setObject (value.get<Value::ObjectValue> (), handler);

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
                if (handler.setKey (member.first) == -1)
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
         * @brief construct a value in place in the current container, or as root.
         * @param args arguments forwarded to the value constructor.
         * @return 0 on success, -1 otherwise.
         */
        template <size_t I, typename... Args>
        int emplaceValue (Args&&... args)
        {
            if (JOIN_UNLIKELY (_depth == 0))
            {
                _root = Value (in_place_index_t<I>{}, std::forward<Args> (args)...);
                return 0;
            }

            _stack.emplace_back (in_place_index_t<I>{}, std::forward<Args> (args)...);

            return 0;
        }

        /// start of each open container in _stack.
        size_t _starts[saxMaxDepth] = {};

        /// number of open containers.
        size_t _depth = 0;

        /// stack of the values of the open containers.
        std::vector<Value> _stack;

        /// root.
        Value _root;
    };
}

#endif
