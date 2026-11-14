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

// dconv.
#include <dconv/probe.hpp>

// libjoin.
#include <join/value_builder.hpp>
#include <join/value.hpp>
#include <join/utils.hpp>
#include <join/utf8.hpp>
#include <join/sax.hpp>

// C++.
#include <algorithm>
#include <optional>
#include <sstream>
#include <fstream>
#include <limits>
#include <span>
#include <bit>

// C.
#include <cstring>

namespace join
{
    static_assert (std::numeric_limits<float>::is_iec559 && std::numeric_limits<double>::is_iec559,
                   "MessagePack requires IEEE 754 floating point numbers");

    /**
     * @brief MessagePack serialization mode.
     */
    enum class PackWriteMode
    {
        None,             /**< no write mode set. */
        ValidateEncoding, /**< validate UTF-8 encoding of strings. */
    };

    /**
     * @brief message pack writer class.
     */
    template <PackWriteMode WriteMode = PackWriteMode::None>
    class BasicPackWriter
    {
    public:
        /**
         * @brief create instance.
         * @param document MessagePack document to create.
         */
        explicit BasicPackWriter (std::ostream& document)
        : _out (document.rdbuf ())
        {
        }

        /**
         * @brief copy constructor.
         * @param other object to copy.
         */
        BasicPackWriter (const BasicPackWriter& other) = delete;

        /**
         * @brief copy assignment.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        BasicPackWriter& operator= (const BasicPackWriter& other) = delete;

        /**
         * @brief move constructor.
         * @param other object to move.
         */
        BasicPackWriter (BasicPackWriter&& other) = delete;

        /**
         * @brief move assignment.
         * @param other object to move.
         * @return a reference of the current object.
         */
        BasicPackWriter& operator= (BasicPackWriter&& other) = delete;

        /**
         * @brief destroy instance.
         */
        ~BasicPackWriter () = default;

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
            if (std::bit_cast<uint64_t> (static_cast<double> (f)) == std::bit_cast<uint64_t> (value))
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
        int setString (std::string_view value)
        {
            if (JOIN_UNLIKELY (value.size () > std::numeric_limits<uint32_t>::max ()))
            {
                join::lastError = make_error_code (SaxErrc::InvalidValue);
                reset ();
                return -1;
            }

            if constexpr (WriteMode == PackWriteMode::ValidateEncoding)
            {
                if (JOIN_UNLIKELY (utf8::validate (value.data (), value.data () + value.size ()) == -1))
                {
                    join::lastError = make_error_code (SaxErrc::InvalidEncoding);
                    reset ();
                    return -1;
                }
            }

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
            append (value.data (), static_cast<uint32_t> (value.size ()));
            endValue ();
            return 0;
        }

        /**
         * @brief set binary value.
         * @param value binary value to set.
         * @param subtype binary subtype.
         * @return 0 on success, -1 otherwise.
         */
        int setBinary (std::span<const uint8_t> value, std::optional<int8_t> subtype = std::nullopt)
        {
            if (JOIN_UNLIKELY (value.size () > std::numeric_limits<uint32_t>::max ()))
            {
                join::lastError = make_error_code (SaxErrc::InvalidValue);
                reset ();
                return -1;
            }

            if (subtype)
            {
                switch (value.size ())
                {
                    case 1:
                        append (0xd4);
                        break;

                    case 2:
                        append (0xd5);
                        break;

                    case 4:
                        append (0xd6);
                        break;

                    case 8:
                        append (0xd7);
                        break;

                    case 16:
                        append (0xd8);
                        break;

                    default:
                        if (value.size () < 256)
                        {
                            append (0xc7);
                            pack (static_cast<uint8_t> (value.size ()));
                        }
                        else if (value.size () < 65536)
                        {
                            append (0xc8);
                            pack (static_cast<uint16_t> (value.size ()));
                        }
                        else
                        {
                            append (0xc9);
                            pack (static_cast<uint32_t> (value.size ()));
                        }
                        break;
                }
                pack (*subtype);
            }
            else if (value.size () < 256)
            {
                append (0xc4);
                pack (static_cast<uint8_t> (value.size ()));
            }
            else if (value.size () < 65536)
            {
                append (0xc5);
                pack (static_cast<uint16_t> (value.size ()));
            }
            else
            {
                append (0xc6);
                pack (static_cast<uint32_t> (value.size ()));
            }
            append (reinterpret_cast<const char*> (value.data ()), static_cast<uint32_t> (value.size ()));
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
        template <typename Type>
        const View<Type>& members (const View<Type>& object) const noexcept
        {
            return object;
        }

        /**
         * @brief set scalar key.
         * @param key object key.
         * @return 0 on success, -1 otherwise.
         */
        template <Scalar Key>
        int setKey ([[maybe_unused]] Key key)
        {
            if (JOIN_UNLIKELY (_depth == 0))
            {
                join::lastError = make_error_code (SaxErrc::InvalidParent);
                reset ();
                return -1;
            }

            using Type = ScalarType<Key>;

            if constexpr (std::same_as<Type, std::nullptr_t>)
            {
                return setNull ();
            }
            else if constexpr (std::same_as<Type, bool>)
            {
                return setBool (static_cast<Type> (key));
            }
            else if constexpr (std::same_as<Type, int32_t>)
            {
                return setInt (static_cast<Type> (key));
            }
            else if constexpr (std::same_as<Type, uint32_t>)
            {
                return setUint (static_cast<Type> (key));
            }
            else if constexpr (std::same_as<Type, int64_t>)
            {
                return setInt64 (static_cast<Type> (key));
            }
            else if constexpr (std::same_as<Type, uint64_t>)
            {
                return setUint64 (static_cast<Type> (key));
            }
            else
            {
                return setDouble (static_cast<Type> (key));
            }
        }

        /**
         * @brief set string key.
         * @param key object key.
         * @return 0 on success, -1 otherwise.
         */
        int setKey (std::string_view key)
        {
            if (JOIN_UNLIKELY (_depth == 0))
            {
                join::lastError = make_error_code (SaxErrc::InvalidParent);
                reset ();
                return -1;
            }
            return setString (key);
        }

        /**
         * @brief set binary key.
         * @param key object key.
         * @param subtype binary subtype.
         * @return 0 on success, -1 otherwise.
         */
        int setKey (std::span<const uint8_t> key, std::optional<int8_t> subtype = std::nullopt)
        {
            if (JOIN_UNLIKELY (_depth == 0))
            {
                join::lastError = make_error_code (SaxErrc::InvalidParent);
                reset ();
                return -1;
            }
            return setBinary (key, subtype);
        }

        /**
         * @brief start an array or object key.
         * @return 0 on success, -1 otherwise.
         */
        int startKey ()
        {
            if (JOIN_UNLIKELY (_depth == 0))
            {
                join::lastError = make_error_code (SaxErrc::InvalidParent);
                reset ();
                return -1;
            }
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
            if (JOIN_UNLIKELY (size == 0))
            {
                return;
            }
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

    /// MessagePack writer.
    using PackWriter = BasicPackWriter<PackWriteMode::None>;

    /**
     * @brief MessagePack deserialization mode.
     */
    enum class PackReadMode : uint32_t
    {
        None = 0,                    /**< no read mode set. */
        ValidateEncoding = 1L << 0,  /**< validate UTF-8 encoding of strings. */
        StopParsingOnDone = 1L << 1, /**< stop parsing on done. */
    };

    /**
     * @brief perform binary AND on PackReadMode.
     * @param a bitset.
     * @param b other bitset.
     * @return bitset result of binary AND on PackReadMode.
     */
    constexpr PackReadMode operator& (PackReadMode a, PackReadMode b) noexcept
    {
        return PackReadMode (static_cast<uint32_t> (a) & static_cast<uint32_t> (b));
    }

    /**
     * @brief perform binary OR on PackReadMode.
     * @param a bitset.
     * @param b other bitset.
     * @return bitset result of binary OR on PackReadMode.
     */
    constexpr PackReadMode operator| (PackReadMode a, PackReadMode b) noexcept
    {
        return PackReadMode (static_cast<uint32_t> (a) | static_cast<uint32_t> (b));
    }

    /**
     * @brief perform binary XOR on PackReadMode.
     * @param a bitset.
     * @param b other bitset.
     * @return bitset result of binary XOR on PackReadMode.
     */
    constexpr PackReadMode operator^ (PackReadMode a, PackReadMode b) noexcept
    {
        return PackReadMode (static_cast<uint32_t> (a) ^ static_cast<uint32_t> (b));
    }

    /**
     * @brief perform binary NOT on PackReadMode.
     * @param a bitset.
     * @return bitset result of binary NOT on PackReadMode.
     */
    constexpr PackReadMode operator~(PackReadMode a) noexcept
    {
        return PackReadMode (~static_cast<uint32_t> (a));
    }

    /**
     * @brief perform binary AND assignment on PackReadMode.
     * @param a bitset.
     * @param b other bitset.
     * @return reference to updated bitset.
     */
    constexpr const PackReadMode& operator&= (PackReadMode& a, PackReadMode b) noexcept
    {
        return a = a & b;
    }

    /**
     * @brief perform binary OR assignment on PackReadMode.
     * @param a bitset.
     * @param b other bitset.
     * @return reference to updated bitset.
     */
    constexpr const PackReadMode& operator|= (PackReadMode& a, PackReadMode b) noexcept
    {
        return a = a | b;
    }

    /**
     * @brief perform binary XOR assignment on PackReadMode.
     * @param a bitset.
     * @param b other bitset.
     * @return reference to updated bitset.
     */
    constexpr const PackReadMode& operator^= (PackReadMode& a, PackReadMode b) noexcept
    {
        return a = a ^ b;
    }

    /**
     * @brief message pack reader class.
     */
    template <PackReadMode ReadMode = PackReadMode::None>
    class BasicPackReader
    {
    public:
        /**
         * @brief default constructor.
         */
        BasicPackReader () = default;

        /**
         * @brief copy constructor.
         * @param other object to copy.
         */
        BasicPackReader (const BasicPackReader& other) = delete;

        /**
         * @brief copy assignment.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        BasicPackReader& operator= (const BasicPackReader& other) = delete;

        /**
         * @brief move constructor.
         * @param other object to move.
         */
        BasicPackReader (BasicPackReader&& other) = delete;

        /**
         * @brief move assignment.
         * @param other object to move.
         * @return a reference of the current object.
         */
        BasicPackReader& operator= (BasicPackReader&& other) = delete;

        /**
         * @brief destroy instance.
         */
        ~BasicPackReader () = default;

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
            dconv::StringProbe in (document, length);
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
            dconv::StringProbe in (first, last);
            return read (in, handler);
        }

        /**
         * @brief deserialize a document.
         * @param document document to parse.
         * @param handler handler receiving the document.
         * @return 0 on success, -1 otherwise.
         */
        template <typename Handler>
        int deserialize (std::string_view document, Handler& handler)
        {
            dconv::StringProbe in (document.data (), document.size ());
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
            dconv::StreamProbe in (document);
            return read (in, handler);
        }

    protected:
        /**
         * @brief parse a document into a value.
         * @param document document to parse.
         * @param value value receiving the document.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ProbeType, typename Allocator>
        int read (ProbeType& document, BasicValue<Allocator>& value)
        {
            int result = read (document, ValueBuilder (value));
            if (JOIN_UNLIKELY (result != 0))
            {
                value = nullptr;
            }
            return result;
        }

        /**
         * @brief parse a document, resetting the handler on failure.
         * @param document document to parse.
         * @param handler handler receiving the document.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ProbeType, typename Handler>
        int read (ProbeType& document, Handler&& handler)
        {
            try
            {
                if (JOIN_LIKELY (readValue (document, handler, 0) == 0))
                {
                    if constexpr ((ReadMode & PackReadMode::StopParsingOnDone) != PackReadMode::None)
                    {
                        return 0;
                    }

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
        template <typename ProbeType, typename Handler>
        int readValue (ProbeType& document, Handler& handler, size_t depth)
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
        template <typename ProbeType, typename Handler>
        int readNull (ProbeType& document, Handler& handler, bool isKey = false)
        {
            document.get ();
            return (isKey) ? handler.setKey (nullptr) : handler.setNull ();
        }

        /**
         * @brief parse a false value.
         * @param document document to parse.
         * @param handler handler receiving the document.
         * @param isKey indicate whether the element to parse is a key or not.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ProbeType, typename Handler>
        int readFalse (ProbeType& document, Handler& handler, bool isKey = false)
        {
            document.get ();
            return (isKey) ? handler.setKey (false) : handler.setBool (false);
        }

        /**
         * @brief parse a true value.
         * @param document document to parse.
         * @param handler handler receiving the document.
         * @param isKey indicate whether the element to parse is a key or not.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ProbeType, typename Handler>
        int readTrue (ProbeType& document, Handler& handler, bool isKey = false)
        {
            document.get ();
            return (isKey) ? handler.setKey (true) : handler.setBool (true);
        }

        /**
         * @brief parse an array value.
         * @param document document to parse.
         * @param handler handler receiving the document.
         * @param depth number of enclosing containers.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ProbeType, typename Handler>
        __attribute__ ((noinline)) int readArray (ProbeType& document, Handler& handler, size_t depth)
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
        template <typename ProbeType, typename Handler>
        __attribute__ ((noinline)) int readObject (ProbeType& document, Handler& handler, size_t depth)
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
                if (JOIN_UNLIKELY (readKey (document, handler, depth + 1) == -1))
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
         * @param depth number of enclosing containers.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ProbeType, typename Handler>
        int readKey (ProbeType& document, Handler& handler, size_t depth)
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
            else if (isBin (head))
            {
                return readBin (document, handler, true);
            }
            else if (isNumber (head))
            {
                return readNumber (document, handler, true);
            }
            else if (isArray (head) || isObject (head))
            {
                if (JOIN_UNLIKELY (handler.startKey () == -1))
                {
                    return -1;
                }
                return readValue (document, handler, depth);
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
        template <typename ProbeType, typename Handler>
        int readString (ProbeType& document, Handler& handler, bool isKey = false)
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

            if constexpr (std::is_same_v<ProbeType, dconv::StringProbe>)
            {
                if (JOIN_UNLIKELY (static_cast<size_t> (document.end () - document.tell ()) < len))
                {
                    join::lastError = make_error_code (SaxErrc::Truncated);
                    return -1;
                }

                std::string_view output (document.tell (), len);
                if (JOIN_UNLIKELY (validateString (output) == -1))
                {
                    return -1;
                }
                document.seek (document.tell () + len);
                return isKey ? handler.setKey (output) : handler.setString (output);
            }

            thread_local std::string cache;
            std::string output = std::move (cache);
            output.clear ();
            if (JOIN_UNLIKELY (readBytes (document, len, output) == -1))
            {
                return -1;
            }

            if (JOIN_UNLIKELY (validateString (output) == -1))
            {
                return -1;
            }

            int result = isKey ? handler.setKey (output) : handler.setString (output);
            cache = std::move (output);
            return result;
        }

        /**
         * @brief validate the UTF-8 encoding of a string when required by the read mode.
         * @param value string to validate.
         * @return 0 on success, -1 otherwise.
         */
        static int validateString ([[maybe_unused]] std::string_view value) noexcept
        {
            if constexpr ((ReadMode & PackReadMode::ValidateEncoding) != PackReadMode::None)
            {
                if (JOIN_UNLIKELY (utf8::validate (value.data (), value.data () + value.size ()) == -1))
                {
                    join::lastError = make_error_code (SaxErrc::InvalidEncoding);
                    return -1;
                }
            }
            return 0;
        }

        /**
         * @brief parse binary data.
         * @param document document to parse.
         * @param handler handler receiving the document.
         * @param isKey indicate whether the element to parse is a key or not.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ProbeType, typename Handler>
        int readBin (ProbeType& document, Handler& handler, bool isKey = false)
        {
            uint32_t len = 0;
            std::optional<int8_t> subtype;

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
            else
            {
                if (document.getIf (0xd4))
                {
                    len = 1;
                }
                else if (document.getIf (0xd5))
                {
                    len = 2;
                }
                else if (document.getIf (0xd6))
                {
                    len = 4;
                }
                else if (document.getIf (0xd7))
                {
                    len = 8;
                }
                else if (document.getIf (0xd8))
                {
                    len = 16;
                }
                else if (document.getIf (0xc9))
                {
                    if (JOIN_UNLIKELY (unpack<uint32_t> (document, len) == -1))
                    {
                        return -1;
                    }
                }
                else if (document.getIf (0xc8))
                {
                    if (JOIN_UNLIKELY (unpack<uint16_t> (document, len) == -1))
                    {
                        return -1;
                    }
                }
                else if (document.getIf (0xc7))
                {
                    if (JOIN_UNLIKELY (unpack<uint8_t> (document, len) == -1))
                    {
                        return -1;
                    }
                }

                int8_t type = 0;
                if (JOIN_UNLIKELY (unpack<int8_t> (document, type) == -1))
                {
                    return -1;
                }
                subtype = type;
            }

            if constexpr (std::is_same_v<ProbeType, dconv::StringProbe>)
            {
                if (JOIN_UNLIKELY (static_cast<size_t> (document.end () - document.tell ()) < len))
                {
                    join::lastError = make_error_code (SaxErrc::Truncated);
                    return -1;
                }

                std::span<const uint8_t> output (reinterpret_cast<const uint8_t*> (document.tell ()), len);
                document.seek (document.tell () + len);
                return isKey ? handler.setKey (output, subtype) : handler.setBinary (output, subtype);
            }

            thread_local std::string cache;
            std::string output = std::move (cache);
            output.clear ();
            if (JOIN_UNLIKELY (readBytes (document, len, output) == -1))
            {
                return -1;
            }

            std::span<const uint8_t> bytes (reinterpret_cast<const uint8_t*> (output.data ()), output.size ());
            int result = isKey ? handler.setKey (bytes, subtype) : handler.setBinary (bytes, subtype);
            cache = std::move (output);
            return result;
        }

        /**
         * @brief parse a number value.
         * @param document document to parse.
         * @param handler handler receiving the document.
         * @param isKey indicate whether the element to parse is a key or not.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ProbeType, typename Handler>
        int readNumber (ProbeType& document, Handler& handler, bool isKey = false)
        {
            uint8_t head = document.get ();

            if (head <= 0x7f)
            {
                auto number = uint32_t (head);
                return isKey ? handler.setKey (number) : handler.setUint (number);
            }
            else if (head >= 0xe0)
            {
                auto number = int32_t (static_cast<int8_t> (head));
                return isKey ? handler.setKey (number) : handler.setInt (number);
            }
            else if (head == 0xd3)
            {
                int64_t number;
                if (JOIN_UNLIKELY (unpack<int64_t> (document, number) == -1))
                {
                    return -1;
                }
                return isKey ? handler.setKey (number) : handler.setInt64 (number);
            }
            else if (head == 0xcf)
            {
                uint64_t number;
                if (JOIN_UNLIKELY (unpack<uint64_t> (document, number) == -1))
                {
                    return -1;
                }
                return isKey ? handler.setKey (number) : handler.setUint64 (number);
            }
            else if (head == 0xcb)
            {
                double number;
                if (JOIN_UNLIKELY (unpack<double> (document, number) == -1))
                {
                    return -1;
                }
                return isKey ? handler.setKey (number) : handler.setDouble (number);
            }
            else if (head == 0xd2)
            {
                int32_t number;
                if (JOIN_UNLIKELY (unpack<int32_t> (document, number) == -1))
                {
                    return -1;
                }
                return isKey ? handler.setKey (number) : handler.setInt (number);
            }
            else if (head == 0xce)
            {
                uint32_t number;
                if (JOIN_UNLIKELY (unpack<uint32_t> (document, number) == -1))
                {
                    return -1;
                }
                return isKey ? handler.setKey (number) : handler.setUint (number);
            }
            else if (head == 0xca)
            {
                double number;
                if (JOIN_UNLIKELY (unpack<float> (document, number) == -1))
                {
                    return -1;
                }
                return isKey ? handler.setKey (number) : handler.setDouble (number);
            }
            else if (head == 0xd1)
            {
                int32_t number;
                if (JOIN_UNLIKELY (unpack<int16_t> (document, number) == -1))
                {
                    return -1;
                }
                return isKey ? handler.setKey (number) : handler.setInt (number);
            }
            else if (head == 0xcd)
            {
                uint32_t number;
                if (JOIN_UNLIKELY (unpack<uint16_t> (document, number) == -1))
                {
                    return -1;
                }
                return isKey ? handler.setKey (number) : handler.setUint (number);
            }
            else if (head == 0xd0)
            {
                int32_t number;
                if (JOIN_UNLIKELY (unpack<int8_t> (document, number) == -1))
                {
                    return -1;
                }
                return isKey ? handler.setKey (number) : handler.setInt (number);
            }
            else if (head == 0xcc)
            {
                uint32_t number;
                if (JOIN_UNLIKELY (unpack<uint8_t> (document, number) == -1))
                {
                    return -1;
                }
                return isKey ? handler.setKey (number) : handler.setUint (number);
            }
            else
            {
                // LCOV_EXCL_START
                join::lastError = make_error_code (SaxErrc::InvalidValue);
                return -1;
                // LCOV_EXCL_STOP
            }
        }

        /**
         * @brief read bytes by chunks.
         * @param document document to parse.
         * @param len number of bytes to read.
         * @param output string receiving the bytes.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ProbeType>
        static int readBytes (ProbeType& document, uint32_t len, std::string& output)
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
        template <typename Type, typename ProbeType, typename Output>
        static int unpack (ProbeType& document, Output& output)
            requires std::is_arithmetic_v<Type>
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
            return ((c >= 0xc4) && (c <= 0xc9)) || ((c >= 0xd4) && (c <= 0xd8));
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

    /// MessagePack reader.
    using PackReader = BasicPackReader<PackReadMode::None>;
}

#endif
