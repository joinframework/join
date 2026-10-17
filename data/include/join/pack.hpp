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

#ifndef JOIN_DATA_PACK_HPP
#define JOIN_DATA_PACK_HPP

// libjoin.
#include <join/value.hpp>
#include <join/utils.hpp>
#include <join/view.hpp>
#include <join/sax.hpp>

// C++.
#include <algorithm>
#include <sstream>
#include <fstream>

// C.
#include <cstring>

namespace join
{
    /**
     * @brief message pack writer class.
     */
    class PackWriter
    {
    public:
        /**
         * @brief create instance.
         * @param document MessagePack document to create.
         */
        explicit PackWriter (std::ostream& document)
        : _out (document.rdbuf ())
        {
        }

        /**
         * @brief copy constructor.
         * @param other object to copy.
         */
        PackWriter (const PackWriter& other) = delete;

        /**
         * @brief copy assignment.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        PackWriter& operator= (const PackWriter& other) = delete;

        /**
         * @brief move constructor.
         * @param other object to move.
         */
        PackWriter (PackWriter&& other) = delete;

        /**
         * @brief move assignment.
         * @param other object to move.
         * @return a reference of the current object.
         */
        PackWriter& operator= (PackWriter&& other) = delete;

        /**
         * @brief destroy instance.
         */
        ~PackWriter () = default;

        /**
         * @brief set null value.
         * @return 0 on success, -1 otherwise.
         */
        int setNull ()
        {
            append (0xc0);
            endValue ();
            return 0;
        }

        /**
         * @brief set boolean value.
         * @param value boolean value to set.
         * @return 0 on success, -1 otherwise.
         */
        int setBool (bool value)
        {
            if (value)
            {
                append (0xc3);
            }
            else
            {
                append (0xc2);
            }
            endValue ();
            return 0;
        }

        /**
         * @brief set integer value.
         * @param value integer value to set.
         * @return 0 on success, -1 otherwise.
         */
        int setInt (int32_t value)
        {
            if (value < -(1 << 15))
            {
                append (0xd2);
                pack (static_cast<uint32_t> (value));
            }
            else if (value < -(1 << 7))
            {
                append (0xd1);
                pack (static_cast<uint16_t> (value));
            }
            else if (value < -(1 << 5))
            {
                append (0xd0);
                pack (static_cast<uint8_t> (value));
            }
            else if (value < (1 << 7))
            {
                append (static_cast<uint8_t> (value));
            }
            else if (value < (1 << 8))
            {
                append (0xcc);
                pack (static_cast<uint8_t> (value));
            }
            else if (value < (1 << 16))
            {
                append (0xcd);
                pack (static_cast<uint16_t> (value));
            }
            else
            {
                append (0xce);
                pack (static_cast<uint32_t> (value));
            }
            endValue ();
            return 0;
        }

        /**
         * @brief set unsigned integer value.
         * @param value unsigned integer value to set.
         * @return 0 on success, -1 otherwise.
         */
        int setUint (uint32_t value)
        {
            if (value < (1 << 7))
            {
                append (static_cast<uint8_t> (value));
            }
            else if (value < (1 << 8))
            {
                append (0xcc);
                pack (static_cast<uint8_t> (value));
            }
            else if (value < (1 << 16))
            {
                append (0xcd);
                pack (static_cast<uint16_t> (value));
            }
            else
            {
                append (0xce);
                pack (value);
            }
            endValue ();
            return 0;
        }

        /**
         * @brief set 64 bits integer value.
         * @param value 64 bits integer value to set.
         * @return 0 on success, -1 otherwise.
         */
        int setInt64 (int64_t value)
        {
            if (value < -(1LL << 31))
            {
                append (0xd3);
                pack (static_cast<uint64_t> (value));
            }
            else if (value < -(1LL << 15))
            {
                append (0xd2);
                pack (static_cast<uint32_t> (value));
            }
            else if (value < -(1LL << 7))
            {
                append (0xd1);
                pack (static_cast<uint16_t> (value));
            }
            else if (value < -(1LL << 5))
            {
                append (0xd0);
                pack (static_cast<uint8_t> (value));
            }
            else if (value < (1LL << 7))
            {
                append (static_cast<uint8_t> (value));
            }
            else if (value < (1LL << 8))
            {
                append (0xcc);
                pack (static_cast<uint8_t> (value));
            }
            else if (value < (1LL << 16))
            {
                append (0xcd);
                pack (static_cast<uint16_t> (value));
            }
            else if (value < (1LL << 32))
            {
                append (0xce);
                pack (static_cast<uint32_t> (value));
            }
            else
            {
                append (0xcf);
                pack (static_cast<uint64_t> (value));
            }
            endValue ();
            return 0;
        }

        /**
         * @brief set unsigned 64 bits integer value.
         * @param value unsigned 64 bits integer value to set.
         * @return 0 on success, -1 otherwise.
         */
        int setUint64 (uint64_t value)
        {
            if (value < (1ULL << 7))
            {
                append (static_cast<uint8_t> (value));
            }
            else if (value < (1ULL << 8))
            {
                append (0xcc);
                pack (static_cast<uint8_t> (value));
            }
            else if (value < (1ULL << 16))
            {
                append (0xcd);
                pack (static_cast<uint16_t> (value));
            }
            else if (value < (1ULL << 32))
            {
                append (0xce);
                pack (static_cast<uint32_t> (value));
            }
            else
            {
                append (0xcf);
                pack (value);
            }
            endValue ();
            return 0;
        }

        /**
         * @brief set real value.
         * @param value real value to set.
         * @return 0 on success, -1 otherwise.
         */
        int setDouble (double value)
        {
            float f = static_cast<float> (value);
            if (std::isfinite (value) && static_cast<double> (f) == value)
            {
                append (0xca);
                pack (f);
            }
            else
            {
                append (0xcb);
                pack (value);
            }
            endValue ();
            return 0;
        }

        /**
         * @brief set string value.
         * @param value string value to set.
         * @return 0 on success, -1 otherwise.
         */
        int setString (const std::string& value)
        {
            if (value.size () < 32)
            {
                append (static_cast<char> (0xa0 | value.size ()));
            }
            else if (value.size () < 256)
            {
                append (0xd9);
                pack (static_cast<uint8_t> (value.size ()));
            }
            else if (value.size () < 65536)
            {
                append (0xda);
                pack (static_cast<uint16_t> (value.size ()));
            }
            else
            {
                append (0xdb);
                pack (static_cast<uint32_t> (value.size ()));
            }
            append (value.c_str (), static_cast<uint32_t> (value.size ()));
            endValue ();
            return 0;
        }

        /**
         * @brief start array.
         * @param size array size.
         * @return 0 on success, -1 otherwise.
         */
        int startArray (uint32_t size)
        {
            if (size < 16)
            {
                append (static_cast<char> (0x90 | size));
            }
            else if (size < 65536)
            {
                append (0xdc);
                pack (static_cast<uint16_t> (size));
            }
            else
            {
                append (0xdd);
                pack (size);
            }
            ++_depth;
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
                reset ();
                return -1;
            }
            --_depth;
            endValue ();
            return 0;
        }

        /**
         * @brief start object.
         * @param size object size.
         * @return 0 on success, -1 otherwise.
         */
        int startObject (uint32_t size)
        {
            if (size < 16)
            {
                append (static_cast<char> (0x80 | size));
            }
            else if (size < 65536)
            {
                append (0xde);
                pack (static_cast<uint16_t> (size));
            }
            else
            {
                append (0xdf);
                pack (size);
            }
            ++_depth;
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
         * @param key object key.
         * @return 0 on success, -1 otherwise.
         */
        int setKey (const Value& key)
        {
            if (JOIN_UNLIKELY (_depth == 0))
            {
                join::lastError = make_error_code (SaxErrc::InvalidParent);
                reset ();
                return -1;
            }
            if (JOIN_UNLIKELY (key.isArray () || key.isObject ()))
            {
                join::lastError = make_error_code (SaxErrc::InvalidKey);
                reset ();
                return -1;
            }
            switch (key.index ())
            {
                case Value::Boolean:
                    return setBool (key.get<Value::Boolean> ());

                case Value::Integer:
                    return setInt (key.get<Value::Integer> ());

                case Value::Unsigned:
                    return setUint (key.get<Value::Unsigned> ());

                case Value::Integer64:
                    return setInt64 (key.get<Value::Integer64> ());

                case Value::Unsigned64:
                    return setUint64 (key.get<Value::Unsigned64> ());

                case Value::Real:
                    return setDouble (key.get<Value::Real> ());

                case Value::String:
                    return setString (key.get<Value::String> ());

                default:
                    return setNull ();
            }
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
                reset ();
                return -1;
            }
            --_depth;
            endValue ();
            return 0;
        }

        /**
         * @brief flush buffered data to output stream.
         */
        void flush ()
        {
            if (_size)
            {
                _out->sputn (_buffer, _size);
                _size = 0;
            }
        }

        /**
         * @brief reset the writer state, discarding buffered data.
         */
        void reset () noexcept
        {
            _size = 0;
            _depth = 0;
        }

    protected:
        /**
         * @brief append character to output stream in batch.
         * @param data character to append to output stream.
         */
        inline void append (char data)
        {
            if (JOIN_UNLIKELY (_size == sizeof (_buffer)))
            {
                flush ();
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
            if (JOIN_UNLIKELY (size > sizeof (_buffer) - _size))
            {
                flush ();
                if (size >= sizeof (_buffer))
                {
                    _out->sputn (data, size);
                    return;
                }
            }
            std::memcpy (_buffer + _size, data, size);
            _size += size;
        }

        /**
         * @brief pack value into stream.
         * @param value value to pack.
         */
        template <typename Type>
        void pack (Type value)
        {
            append (reinterpret_cast<const char*> (&swap (value)), sizeof (value));
        }

        /**
         * @brief mark the end of a value, flushing the document once complete.
         */
        inline void endValue ()
        {
            if (_depth == 0)
            {
                flush ();
            }
        }

        /// underlying output stream.
        std::streambuf* _out;

        /// output buffer.
        char _buffer[1024] = {};

        /// output buffer size.
        size_t _size = 0;

        /// number of open containers.
        size_t _depth = 0;
    };

    /**
     * @brief message pack reader class.
     */
    class PackReader
    {
    public:
        /**
         * @brief default constructor.
         */
        PackReader () = default;

        /**
         * @brief copy constructor.
         * @param other object to copy.
         */
        PackReader (const PackReader& other) = delete;

        /**
         * @brief copy assignment.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        PackReader& operator= (const PackReader& other) = delete;

        /**
         * @brief move constructor.
         * @param other object to move.
         */
        PackReader (PackReader&& other) = delete;

        /**
         * @brief move assignment.
         * @param other object to move.
         * @return a reference of the current object.
         */
        PackReader& operator= (PackReader&& other) = delete;

        /**
         * @brief destroy instance.
         */
        ~PackReader () = default;

        /**
         * @brief deserialize a document.
         * @param document document to parse.
         * @param length The length of the document to parse.
         * @param handler handler receiving the document.
         * @return 0 on success, -1 otherwise.
         */
        template <typename Handler>
        int deserialize (const char* document, size_t length, Handler& handler)
        {
            StringView in (document, length);
            return read (in, handler);
        }

        /**
         * @brief deserialize a document.
         * @param first The first character of the document to parse.
         * @param last The last character of the document to parse.
         * @param handler handler receiving the document.
         * @return 0 on success, -1 otherwise.
         */
        template <typename Handler>
        int deserialize (const char* first, const char* last, Handler& handler)
        {
            StringView in (first, last);
            return read (in, handler);
        }

        /**
         * @brief deserialize a document.
         * @param document document to parse.
         * @param handler handler receiving the document.
         * @return 0 on success, -1 otherwise.
         */
        template <typename Handler>
        int deserialize (const std::string& document, Handler& handler)
        {
            StringView in (document.c_str (), document.size ());
            return read (in, handler);
        }

        /**
         * @brief deserialize a document.
         * @param document document to deserialize.
         * @param handler handler receiving the document.
         * @return 0 on success, -1 otherwise.
         */
        template <typename Handler>
        int deserialize (std::stringstream& document, Handler& handler)
        {
            StringStreamView in (document);
            return read (in, handler);
        }

        /**
         * @brief deserialize a document.
         * @param document document to deserialize.
         * @param handler handler receiving the document.
         * @return 0 on success, -1 otherwise.
         */
        template <typename Handler>
        int deserialize (std::istringstream& document, Handler& handler)
        {
            StringStreamView in (document);
            return read (in, handler);
        }

        /**
         * @brief deserialize a document.
         * @param document document to deserialize.
         * @param handler handler receiving the document.
         * @return 0 on success, -1 otherwise.
         */
        template <typename Handler>
        int deserialize (std::fstream& document, Handler& handler)
        {
            FileStreamView in (document);
            return read (in, handler);
        }

        /**
         * @brief deserialize a document.
         * @param document document to deserialize.
         * @param handler handler receiving the document.
         * @return 0 on success, -1 otherwise.
         */
        template <typename Handler>
        int deserialize (std::ifstream& document, Handler& handler)
        {
            FileStreamView in (document);
            return read (in, handler);
        }

        /**
         * @brief deserialize a document.
         * @param document document to deserialize.
         * @param handler handler receiving the document.
         * @return 0 on success, -1 otherwise.
         */
        template <typename Handler>
        int deserialize (std::iostream& document, Handler& handler)
        {
            StreamView in (document);
            return read (in, handler);
        }

        /**
         * @brief deserialize a document.
         * @param document document to deserialize.
         * @param handler handler receiving the document.
         * @return 0 on success, -1 otherwise.
         */
        template <typename Handler>
        int deserialize (std::istream& document, Handler& handler)
        {
            StreamView in (document);
            return read (in, handler);
        }

    protected:
        /**
         * @brief parse a document, resetting the handler on failure.
         * @param document document to parse.
         * @param handler handler receiving the document.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType, typename Handler>
        int read (ViewType& document, Handler& handler)
        {
            try
            {
                if (JOIN_LIKELY (readValue (document, handler, 0) == 0))
                {
                    if (JOIN_LIKELY (document.peek () == std::char_traits<char>::eof ()))
                    {
                        return 0;
                    }

                    join::lastError = make_error_code (SaxErrc::ExtraData);
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
         * @brief parse value.
         * @param document document to parse.
         * @param handler handler receiving the document.
         * @param depth number of enclosing containers.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType, typename Handler>
        int readValue (ViewType& document, Handler& handler, size_t depth)
        {
            int ch = document.peek ();
            if (JOIN_UNLIKELY (ch == std::char_traits<char>::eof ()))
            {
                join::lastError = make_error_code (SaxErrc::Truncated);
                return -1;
            }

            uint8_t head = static_cast<uint8_t> (ch);

            if (isArray (head))
            {
                return readArray (document, handler, depth);
            }
            else if (isObject (head))
            {
                return readObject (document, handler, depth);
            }
            else if (isNull (head))
            {
                return readNull (document, handler);
            }
            else if (isFalse (head))
            {
                return readFalse (document, handler);
            }
            else if (isTrue (head))
            {
                return readTrue (document, handler);
            }
            else if (isString (head))
            {
                return readString (document, handler);
            }
            else if (isBin (head))
            {
                return readBin (document, handler);
            }
            else if (isNumber (head))
            {
                return readNumber (document, handler);
            }
            else
            {
                join::lastError = make_error_code (SaxErrc::InvalidValue);
                return -1;
            }
        }

        /**
         * @brief parse a null value.
         * @param document document to parse.
         * @param handler handler receiving the document.
         * @param isKey indicate whether the element to parse is a key or not.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType, typename Handler>
        int readNull (ViewType& document, Handler& handler, bool isKey = false)
        {
            document.get ();
            return (isKey) ? handler.setKey (Value (in_place_index_t<Value::Null>{}, nullptr)) : handler.setNull ();
        }

        /**
         * @brief parse a false value.
         * @param document document to parse.
         * @param handler handler receiving the document.
         * @param isKey indicate whether the element to parse is a key or not.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType, typename Handler>
        int readFalse (ViewType& document, Handler& handler, bool isKey = false)
        {
            document.get ();
            return (isKey) ? handler.setKey (Value (in_place_index_t<Value::Boolean>{}, false))
                           : handler.setBool (false);
        }

        /**
         * @brief parse a true value.
         * @param document document to parse.
         * @param handler handler receiving the document.
         * @param isKey indicate whether the element to parse is a key or not.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType, typename Handler>
        int readTrue (ViewType& document, Handler& handler, bool isKey = false)
        {
            document.get ();
            return (isKey) ? handler.setKey (Value (in_place_index_t<Value::Boolean>{}, true)) : handler.setBool (true);
        }

        /**
         * @brief parse an array value.
         * @param document document to parse.
         * @param handler handler receiving the document.
         * @param depth number of enclosing containers.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType, typename Handler>
        int readArray (ViewType& document, Handler& handler, size_t depth)
        {
            if (JOIN_UNLIKELY (depth == saxMaxDepth))
            {
                join::lastError = make_error_code (SaxErrc::StackOverflow);
                return -1;
            }

            uint32_t len = 0;

            if (document.getIf (0xdd))
            {
                if (JOIN_UNLIKELY (unpack<uint32_t> (document, len) == -1))
                {
                    return -1;
                }
            }
            else if (document.getIf (0xdc))
            {
                if (JOIN_UNLIKELY (unpack<uint16_t> (document, len) == -1))
                {
                    return -1;
                }
            }
            else
            {
                len = static_cast<uint8_t> (document.get ()) & ~0x90u;
            }

            if (JOIN_UNLIKELY (handler.startArray (len) == -1))
            {
                return -1;
            }

            while (len)
            {
                if (JOIN_UNLIKELY (readValue (document, handler, depth + 1) == -1))
                {
                    return -1;
                }

                --len;
            }

            return handler.stopArray ();
        }

        /**
         * @brief parse an object value.
         * @param document document to parse.
         * @param handler handler receiving the document.
         * @param depth number of enclosing containers.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType, typename Handler>
        int readObject (ViewType& document, Handler& handler, size_t depth)
        {
            if (JOIN_UNLIKELY (depth == saxMaxDepth))
            {
                join::lastError = make_error_code (SaxErrc::StackOverflow);
                return -1;
            }

            uint32_t len = 0;

            if (document.getIf (0xdf))
            {
                if (JOIN_UNLIKELY (unpack<uint32_t> (document, len) == -1))
                {
                    return -1;
                }
            }
            else if (document.getIf (0xde))
            {
                if (JOIN_UNLIKELY (unpack<uint16_t> (document, len) == -1))
                {
                    return -1;
                }
            }
            else
            {
                len = static_cast<uint8_t> (document.get ()) & ~0x80u;
            }

            if (JOIN_UNLIKELY (handler.startObject (len) == -1))
            {
                return -1;
            }

            while (len)
            {
                if (JOIN_UNLIKELY (readKey (document, handler) == -1))
                {
                    return -1;
                }

                if (JOIN_UNLIKELY (readValue (document, handler, depth + 1) == -1))
                {
                    return -1;
                }

                --len;
            }

            return handler.stopObject ();
        }

        /**
         * @brief parse a key of any type.
         * @param document document to parse.
         * @param handler handler receiving the document.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType, typename Handler>
        int readKey (ViewType& document, Handler& handler)
        {
            int ch = document.peek ();
            if (JOIN_UNLIKELY (ch == std::char_traits<char>::eof ()))
            {
                join::lastError = make_error_code (SaxErrc::Truncated);
                return -1;
            }

            uint8_t head = static_cast<uint8_t> (ch);

            if (isNull (head))
            {
                return readNull (document, handler, true);
            }
            else if (isFalse (head))
            {
                return readFalse (document, handler, true);
            }
            else if (isTrue (head))
            {
                return readTrue (document, handler, true);
            }
            else if (isString (head))
            {
                return readString (document, handler, true);
            }
            else if (isNumber (head))
            {
                return readNumber (document, handler, true);
            }
            else
            {
                join::lastError = make_error_code (SaxErrc::InvalidValue);
                return -1;
            }
        }

        /**
         * @brief parse a string value.
         * @param document document to parse.
         * @param handler handler receiving the document.
         * @param isKey indicate whether the element to parse is a key or not.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType, typename Handler>
        int readString (ViewType& document, Handler& handler, bool isKey = false)
        {
            uint32_t len = 0;

            if (document.getIf (0xdb))
            {
                if (JOIN_UNLIKELY (unpack<uint32_t> (document, len) == -1))
                {
                    return -1;
                }
            }
            else if (document.getIf (0xda))
            {
                if (JOIN_UNLIKELY (unpack<uint16_t> (document, len) == -1))
                {
                    return -1;
                }
            }
            else if (document.getIf (0xd9))
            {
                if (JOIN_UNLIKELY (unpack<uint8_t> (document, len) == -1))
                {
                    return -1;
                }
            }
            else
            {
                len = static_cast<uint8_t> (document.get ()) & ~0xa0u;
            }

            std::string output;
            if (JOIN_UNLIKELY (readBytes (document, len, output) == -1))
            {
                return -1;
            }

            return isKey ? handler.setKey (Value (in_place_index_t<Value::String>{}, std::move (output)))
                         : handler.setString (std::move (output));
        }

        /**
         * @brief parse binary data.
         * @param document document to parse.
         * @param handler handler receiving the document.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType, typename Handler>
        int readBin (ViewType& document, Handler& handler)
        {
            uint32_t len = 0;

            if (document.getIf (0xc6))
            {
                if (JOIN_UNLIKELY (unpack<uint32_t> (document, len) == -1))
                {
                    return -1;
                }
            }
            else if (document.getIf (0xc5))
            {
                if (JOIN_UNLIKELY (unpack<uint16_t> (document, len) == -1))
                {
                    return -1;
                }
            }
            else if (document.getIf (0xc4))
            {
                if (JOIN_UNLIKELY (unpack<uint8_t> (document, len) == -1))
                {
                    return -1;
                }
            }

            std::string output;
            if (JOIN_UNLIKELY (readBytes (document, len, output) == -1))
            {
                return -1;
            }

            return handler.setString (std::move (output));
        }

        /**
         * @brief parse a number value.
         * @param document document to parse.
         * @param handler handler receiving the document.
         * @param isKey indicate whether the element to parse is a key or not.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType, typename Handler>
        int readNumber (ViewType& document, Handler& handler, bool isKey = false)
        {
            uint8_t head = document.get ();

            if (head <= 0x7f)
            {
                auto number = uint32_t (head);
                return isKey ? handler.setKey (Value (in_place_index_t<Value::Unsigned>{}, number))
                             : handler.setUint (number);
            }
            else if (head >= 0xe0)
            {
                auto number = int32_t (static_cast<int8_t> (head));
                return isKey ? handler.setKey (Value (in_place_index_t<Value::Integer>{}, number))
                             : handler.setInt (number);
            }
            else if (head == 0xd3)
            {
                int64_t number;
                if (JOIN_UNLIKELY (unpack<int64_t> (document, number) == -1))
                {
                    return -1;
                }
                return isKey ? handler.setKey (Value (in_place_index_t<Value::Integer64>{}, number))
                             : handler.setInt64 (number);
            }
            else if (head == 0xcf)
            {
                uint64_t number;
                if (JOIN_UNLIKELY (unpack<uint64_t> (document, number) == -1))
                {
                    return -1;
                }
                return isKey ? handler.setKey (Value (in_place_index_t<Value::Unsigned64>{}, number))
                             : handler.setUint64 (number);
            }
            else if (head == 0xcb)
            {
                double number;
                if (JOIN_UNLIKELY (unpack<double> (document, number) == -1))
                {
                    return -1;
                }
                return isKey ? handler.setKey (Value (in_place_index_t<Value::Real>{}, number))
                             : handler.setDouble (number);
            }
            else if (head == 0xd2)
            {
                int32_t number;
                if (JOIN_UNLIKELY (unpack<int32_t> (document, number) == -1))
                {
                    return -1;
                }
                return isKey ? handler.setKey (Value (in_place_index_t<Value::Integer>{}, number))
                             : handler.setInt (number);
            }
            else if (head == 0xce)
            {
                uint32_t number;
                if (JOIN_UNLIKELY (unpack<uint32_t> (document, number) == -1))
                {
                    return -1;
                }
                return isKey ? handler.setKey (Value (in_place_index_t<Value::Unsigned>{}, number))
                             : handler.setUint (number);
            }
            else if (head == 0xca)
            {
                double number;
                if (JOIN_UNLIKELY (unpack<float> (document, number) == -1))
                {
                    return -1;
                }
                return isKey ? handler.setKey (Value (in_place_index_t<Value::Real>{}, number))
                             : handler.setDouble (number);
            }
            else if (head == 0xd1)
            {
                int32_t number;
                if (JOIN_UNLIKELY (unpack<int16_t> (document, number) == -1))
                {
                    return -1;
                }
                return isKey ? handler.setKey (Value (in_place_index_t<Value::Integer>{}, number))
                             : handler.setInt (number);
            }
            else if (head == 0xcd)
            {
                uint32_t number;
                if (JOIN_UNLIKELY (unpack<uint16_t> (document, number) == -1))
                {
                    return -1;
                }
                return isKey ? handler.setKey (Value (in_place_index_t<Value::Unsigned>{}, number))
                             : handler.setUint (number);
            }
            else if (head == 0xd0)
            {
                int32_t number;
                if (JOIN_UNLIKELY (unpack<int8_t> (document, number) == -1))
                {
                    return -1;
                }
                return isKey ? handler.setKey (Value (in_place_index_t<Value::Integer>{}, number))
                             : handler.setInt (number);
            }
            else if (head == 0xcc)
            {
                uint32_t number;
                if (JOIN_UNLIKELY (unpack<uint8_t> (document, number) == -1))
                {
                    return -1;
                }
                return isKey ? handler.setKey (Value (in_place_index_t<Value::Unsigned>{}, number))
                             : handler.setUint (number);
            }
            else
            {
                join::lastError = make_error_code (SaxErrc::InvalidValue);  // LCOV_EXCL_LINE
                return -1;                                                  // LCOV_EXCL_LINE
            }
        }

        /**
         * @brief read bytes by chunks.
         * @param document document to parse.
         * @param len number of bytes to read.
         * @param output string receiving the bytes.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType>
        static int readBytes (ViewType& document, uint32_t len, std::string& output)
        {
            while (len)
            {
                size_t pos = output.size ();
                size_t chunk = std::min<size_t> (len, 65536);
                output.resize (pos + chunk);
                if (JOIN_UNLIKELY (document.read (&output[pos], chunk) != chunk))
                {
                    join::lastError = make_error_code (SaxErrc::Truncated);
                    return -1;
                }
                len -= chunk;
            }
            return 0;
        }

        /**
         * @brief unpack value from stream.
         * @param document document to parse.
         * @param output unpacked value.
         * @return 0 on success, -1 otherwise.
         */
        template <typename Type, typename ViewType, typename Output>
        static std::enable_if_t<std::is_arithmetic<Type>::value, int> unpack (ViewType& document, Output& output)
        {
            Type value;
            if (JOIN_UNLIKELY (document.read (reinterpret_cast<char*> (&value), sizeof (value)) != sizeof (value)))
            {
                join::lastError = make_error_code (SaxErrc::Truncated);
                return -1;
            }
            output = static_cast<Output> (swap (value));
            return 0;
        }

        /**
         * @brief check if null.
         * @param c character to check.
         * @return true if null, false otherwise.
         */
        constexpr bool isNull (uint8_t c)
        {
            return (c == 0xc0);
        }

        /**
         * @brief check if false.
         * @param c character to check.
         * @return true if false, false otherwise.
         */
        constexpr bool isFalse (uint8_t c)
        {
            return (c == 0xc2);
        }

        /**
         * @brief check if true.
         * @param c character to check.
         * @return true if true, false otherwise.
         */
        constexpr bool isTrue (uint8_t c)
        {
            return (c == 0xc3);
        }

        /**
         * @brief check if 32 bits integer.
         * @param c character to check.
         * @return true if 32 bits integer, false otherwise.
         */
        constexpr bool isInt (uint8_t c)
        {
            return ((c <= 0x7f) || (c >= 0xe0)) || (c == 0xd0) || (c == 0xd1) || (c == 0xd2);
        }

        /**
         * @brief check if unsigned 32 bits integer.
         * @param c character to check.
         * @return true if unsigned 32 bits integer, false otherwise.
         */
        constexpr bool isUint (uint8_t c)
        {
            return (c == 0xcc) || (c == 0xcd) || (c == 0xce);
        }

        /**
         * @brief check if 64 bits integer.
         * @param c character to check.
         * @return true if 64 bits integer, false otherwise.
         */
        constexpr bool isInt64 (uint8_t c)
        {
            return (c == 0xd3);
        }

        /**
         * @brief check if unsigned 64 bits integer.
         * @param c character to check.
         * @return true if unsigned 64 bits integer, false otherwise.
         */
        constexpr bool isUint64 (uint8_t c)
        {
            return (c == 0xcf);
        }

        /**
         * @brief check if real.
         * @param c character to check.
         * @return true if real, false otherwise.
         */
        constexpr bool isReal (uint8_t c)
        {
            return (c == 0xca) || (c == 0xcb);
        }

        /**
         * @brief check if number.
         * @param c character to check.
         * @return true if number, false otherwise.
         */
        constexpr bool isNumber (uint8_t c)
        {
            return isInt (c) || isUint (c) || isInt64 (c) || isUint64 (c) || isReal (c);
        }

        /**
         * @brief check if string.
         * @param c character to check.
         * @return true if string, false otherwise.
         */
        constexpr bool isString (uint8_t c)
        {
            return ((c >= 0xa0) && (c <= 0xbf)) || (c == 0xd9) || (c == 0xda) || (c == 0xdb);
        }

        /**
         * @brief check if binary data.
         * @param c character to check.
         * @return true if binary data, false otherwise.
         */
        constexpr bool isBin (uint8_t c)
        {
            return (c == 0xc4) || (c == 0xc5) || (c == 0xc6);
        }

        /**
         * @brief check if array.
         * @param c character to check.
         * @return true if array, false otherwise.
         */
        constexpr bool isArray (uint8_t c)
        {
            return ((c >= 0x90) && (c <= 0x9f)) || (c == 0xdc) || (c == 0xdd);
        }

        /**
         * @brief check if object.
         * @param c character to check.
         * @return true if object, false otherwise.
         */
        constexpr bool isObject (uint8_t c)
        {
            return ((c >= 0x80) && (c <= 0x8f)) || (c == 0xde) || (c == 0xdf);
        }
    };
}

#endif
