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

#ifndef JOIN_DATA_VALUE_BUILDER_HPP
#define JOIN_DATA_VALUE_BUILDER_HPP

// libjoin.
#include <join/value.hpp>
#include <join/utils.hpp>
#include <join/sax.hpp>

// C++.
#include <string_view>
#include <iterator>
#include <utility>
#include <string>
#include <vector>

// C.
#include <cstddef>

namespace join
{
    /**
     * @brief value builder class, filled by balanced SAX calls.
     */
    class ValueBuilder
    {
    public:
        /**
         * @brief create instance.
         * @param root value receiving the document.
         */
        explicit ValueBuilder (Value& root)
        : _stack (std::move (cache ()))
        , _root (root)
        {
        }

        /**
         * @brief copy constructor.
         * @param other object to copy.
         */
        ValueBuilder (const ValueBuilder& other) = delete;

        /**
         * @brief copy assignment.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        ValueBuilder& operator= (const ValueBuilder& other) = delete;

        /**
         * @brief move constructor.
         * @param other object to move.
         */
        ValueBuilder (ValueBuilder&& other) = delete;

        /**
         * @brief move assignment.
         * @param other object to move.
         * @return a reference of the current object.
         */
        ValueBuilder& operator= (ValueBuilder&& other) = delete;

        /**
         * @brief destroy instance.
         */
        ~ValueBuilder ()
        {
            _stack.clear ();
            cache () = std::move (_stack);
        }

        /**
         * @brief set null value.
         * @return 0 on success, -1 otherwise.
         */
        int setNull ()
        {
            return emplaceValue (nullptr);
        }

        /**
         * @brief set boolean value.
         * @param value boolean value to set.
         * @return 0 on success, -1 otherwise.
         */
        int setBool (bool value)
        {
            return emplaceValue (value);
        }

        /**
         * @brief set integer value.
         * @param value integer value to set.
         * @return 0 on success, -1 otherwise.
         */
        int setInt (int32_t value)
        {
            return emplaceValue (value);
        }

        /**
         * @brief set unsigned integer value.
         * @param value unsigned integer value to set.
         * @return 0 on success, -1 otherwise.
         */
        int setUint (uint32_t value)
        {
            return emplaceValue (value);
        }

        /**
         * @brief set 64 bits integer value.
         * @param value 64 bits integer value to set.
         * @return 0 on success, -1 otherwise.
         */
        int setInt64 (int64_t value)
        {
            return emplaceValue (value);
        }

        /**
         * @brief set unsigned 64 bits integer value.
         * @param value unsigned 64 bits integer value to set.
         * @return 0 on success, -1 otherwise.
         */
        int setUint64 (uint64_t value)
        {
            return emplaceValue (value);
        }

        /**
         * @brief set real value.
         * @param value real value to set.
         * @return 0 on success, -1 otherwise.
         */
        int setDouble (double value)
        {
            return emplaceValue (value);
        }

        /**
         * @brief set string value.
         * @param value string value to set.
         * @return 0 on success, -1 otherwise.
         */
        int setString (std::string_view value)
        {
            return emplaceValue (std::string (value));
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

            _stack.emplace_back (std::move (array));

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

            _stack.emplace_back (std::move (object));

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
         * @brief construct a value in place in the current container, or as root.
         * @param value value of the alternative to construct.
         * @return 0 on success, -1 otherwise.
         */
        template <typename T>
        int emplaceValue (T&& value)
        {
            if (JOIN_UNLIKELY (_depth == 0))
            {
                _root = Value (std::forward<T> (value));
                return 0;
            }

            _stack.emplace_back (std::forward<T> (value));

            return 0;
        }

        /**
         * @brief get the stack kept by the calling thread between documents.
         * @return stack.
         */
        static std::vector<Value>& cache () noexcept
        {
            thread_local std::vector<Value> stack;
            return stack;
        }

        /// start of each open container in _stack.
        size_t _starts[saxMaxDepth] = {};

        /// number of open containers.
        size_t _depth = 0;

        /// stack of the values of the open containers.
        std::vector<Value> _stack;

        /// value receiving the document.
        Value& _root;
    };
}

#endif
