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

#ifndef JOIN_DATA_CBOR_HPP
#define JOIN_DATA_CBOR_HPP

// dconv.
#include <dconv/probe.hpp>

// libjoin.
#include <join/value_builder.hpp>
#include <join/value.hpp>
#include <join/utils.hpp>
#include <join/utf8.hpp>
#include <join/sax.hpp>

// C++.
#include <string_view>
#include <algorithm>
#include <optional>
#include <istream>
#include <ostream>
#include <string>
#include <limits>
#include <span>
#include <bit>

// C.
#include <cstring>
#include <cmath>

namespace join
{
    static_assert (std::numeric_limits<float>::is_iec559 && std::numeric_limits<double>::is_iec559,
                   "CBOR requires IEEE 754 floating point numbers");

    /**
     * @brief CBOR serialization mode.
     */
    enum class CborWriteMode
    {
        Definite,   /**< containers written with their length. */
        Indefinite, /**< containers written with an indefinite length. */
    };

    /**
     * @brief CBOR writer class.
     */
    template <CborWriteMode WriteMode = CborWriteMode::Definite>
    class BasicCborWriter
    {
    public:
        /**
         * @brief create instance.
         * @param document CBOR document to create.
         */
        explicit BasicCborWriter (std::ostream& document)
        : _out (document.rdbuf ())
        {
        }

        /**
         * @brief copy constructor.
         * @param other object to copy.
         */
        BasicCborWriter (const BasicCborWriter& other) = delete;

        /**
         * @brief copy assignment.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        BasicCborWriter& operator= (const BasicCborWriter& other) = delete;

        /**
         * @brief move constructor.
         * @param other object to move.
         */
        BasicCborWriter (BasicCborWriter&& other) = delete;

        /**
         * @brief move assignment.
         * @param other object to move.
         * @return a reference of the current object.
         */
        BasicCborWriter& operator= (BasicCborWriter&& other) = delete;

        /**
         * @brief destroy instance.
         */
        ~BasicCborWriter () = default;

        /**
         * @brief set null value.
         * @return 0 on success, -1 otherwise.
         */
        int setNull ()
        {
            append (0xf6);
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
                append (0xf5);
            }
            else
            {
                append (0xf4);
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
            return setInt64 (value);
        }

        /**
         * @brief set unsigned integer value.
         * @param value unsigned integer value to set.
         * @return 0 on success, -1 otherwise.
         */
        int setUint (uint32_t value)
        {
            writeHead (0x00, value);
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
            if (value < 0)
            {
                writeHead (0x20, ~static_cast<uint64_t> (value));
            }
            else
            {
                writeHead (0x00, static_cast<uint64_t> (value));
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
            writeHead (0x00, value);
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
                append (0xfa);
                pack (f);
            }
            else
            {
                append (0xfb);
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
            writeHead (0x60, value.size ());
            append (value.data (), value.size ());
            endValue ();
            return 0;
        }

        /**
         * @brief set binary value.
         * @param value binary value to set.
         * @param subtype binary subtype, not written.
         * @return 0 on success, -1 otherwise.
         */
        int setBinary (std::span<const uint8_t> value, [[maybe_unused]] std::optional<int8_t> subtype = std::nullopt)
        {
            writeHead (0x40, value.size ());
            append (reinterpret_cast<const char*> (value.data ()), value.size ());
            endValue ();
            return 0;
        }

        /**
         * @brief start array of unknown size.
         * @return 0 on success, -1 otherwise.
         */
        int startArray ()
        {
            static_assert (WriteMode == CborWriteMode::Indefinite, "definite length containers require a size");
            append (0x9f);
            ++_depth;
            return 0;
        }

        /**
         * @brief start array.
         * @param size array size.
         * @return 0 on success, -1 otherwise.
         */
        int startArray ([[maybe_unused]] uint32_t size)
        {
            if constexpr (WriteMode == CborWriteMode::Indefinite)
            {
                append (0x9f);
            }
            else
            {
                writeHead (0x80, size);
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
            if constexpr (WriteMode == CborWriteMode::Indefinite)
            {
                append (0xff);
            }
            --_depth;
            endValue ();
            return 0;
        }

        /**
         * @brief start object of unknown size.
         * @return 0 on success, -1 otherwise.
         */
        int startObject ()
        {
            static_assert (WriteMode == CborWriteMode::Indefinite, "definite length containers require a size");
            append (0xbf);
            ++_depth;
            return 0;
        }

        /**
         * @brief start object.
         * @param size object size.
         * @return 0 on success, -1 otherwise.
         */
        int startObject ([[maybe_unused]] uint32_t size)
        {
            if constexpr (WriteMode == CborWriteMode::Indefinite)
            {
                append (0xbf);
            }
            else
            {
                writeHead (0xa0, size);
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
         * @param subtype binary subtype, not written.
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
            if constexpr (WriteMode == CborWriteMode::Indefinite)
            {
                append (0xff);
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
        inline void append (const char* data, size_t size)
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
         * @brief write a data item head with the shortest argument encoding.
         * @param major major type, already shifted.
         * @param value head argument.
         */
        inline void writeHead (uint8_t major, uint64_t value)
        {
            if (value < 24)
            {
                append (static_cast<char> (major | value));
            }
            else if (value < (1ULL << 8))
            {
                append (static_cast<char> (major | 24));
                pack (static_cast<uint8_t> (value));
            }
            else if (value < (1ULL << 16))
            {
                append (static_cast<char> (major | 25));
                pack (static_cast<uint16_t> (value));
            }
            else if (value < (1ULL << 32))
            {
                append (static_cast<char> (major | 26));
                pack (static_cast<uint32_t> (value));
            }
            else
            {
                append (static_cast<char> (major | 27));
                pack (value);
            }
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

    /// CBOR writer with definite length containers.
    using CborWriter = BasicCborWriter<CborWriteMode::Definite>;

    /// CBOR writer with indefinite length containers.
    using CborStreamWriter = BasicCborWriter<CborWriteMode::Indefinite>;

    /**
     * @brief CBOR deserialization mode.
     */
    enum class CborReadMode : uint32_t
    {
        None = 0,                    /**< no read mode set. */
        ValidateEncoding = 1L << 0,  /**< validate UTF-8 encoding of text strings. */
        StopParsingOnDone = 1L << 1, /**< stop parsing on done. */
    };

    /**
     * @brief perform binary AND on CborReadMode.
     * @param a bitset.
     * @param b other bitset.
     * @return bitset result of binary AND on CborReadMode.
     */
    constexpr CborReadMode operator& (CborReadMode a, CborReadMode b) noexcept
    {
        return CborReadMode (static_cast<uint32_t> (a) & static_cast<uint32_t> (b));
    }

    /**
     * @brief perform binary OR on CborReadMode.
     * @param a bitset.
     * @param b other bitset.
     * @return bitset result of binary OR on CborReadMode.
     */
    constexpr CborReadMode operator| (CborReadMode a, CborReadMode b) noexcept
    {
        return CborReadMode (static_cast<uint32_t> (a) | static_cast<uint32_t> (b));
    }

    /**
     * @brief perform binary XOR on CborReadMode.
     * @param a bitset.
     * @param b other bitset.
     * @return bitset result of binary XOR on CborReadMode.
     */
    constexpr CborReadMode operator^ (CborReadMode a, CborReadMode b) noexcept
    {
        return CborReadMode (static_cast<uint32_t> (a) ^ static_cast<uint32_t> (b));
    }

    /**
     * @brief perform binary NOT on CborReadMode.
     * @param a bitset.
     * @return bitset result of binary NOT on CborReadMode.
     */
    constexpr CborReadMode operator~(CborReadMode a) noexcept
    {
        return CborReadMode (~static_cast<uint32_t> (a));
    }

    /**
     * @brief perform binary AND assignment on CborReadMode.
     * @param a bitset.
     * @param b other bitset.
     * @return reference to updated bitset.
     */
    constexpr const CborReadMode& operator&= (CborReadMode& a, CborReadMode b) noexcept
    {
        return a = a & b;
    }

    /**
     * @brief perform binary OR assignment on CborReadMode.
     * @param a bitset.
     * @param b other bitset.
     * @return reference to updated bitset.
     */
    constexpr const CborReadMode& operator|= (CborReadMode& a, CborReadMode b) noexcept
    {
        return a = a | b;
    }

    /**
     * @brief perform binary XOR assignment on CborReadMode.
     * @param a bitset.
     * @param b other bitset.
     * @return reference to updated bitset.
     */
    constexpr const CborReadMode& operator^= (CborReadMode& a, CborReadMode b) noexcept
    {
        return a = a ^ b;
    }

    /**
     * @brief CBOR reader class.
     */
    template <CborReadMode ReadMode = CborReadMode::None>
    class BasicCborReader
    {
    public:
        /**
         * @brief default constructor.
         */
        BasicCborReader () = default;

        /**
         * @brief copy constructor.
         * @param other object to copy.
         */
        BasicCborReader (const BasicCborReader& other) = delete;

        /**
         * @brief copy assignment.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        BasicCborReader& operator= (const BasicCborReader& other) = delete;

        /**
         * @brief move constructor.
         * @param other object to move.
         */
        BasicCborReader (BasicCborReader&& other) = delete;

        /**
         * @brief move assignment.
         * @param other object to move.
         * @return a reference of the current object.
         */
        BasicCborReader& operator= (BasicCborReader&& other) = delete;

        /**
         * @brief destroy instance.
         */
        ~BasicCborReader () = default;

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
                    if constexpr ((ReadMode & CborReadMode::StopParsingOnDone) != CborReadMode::None)
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
         * @brief parse a data item, skipping its tags.
         * @param document document to parse.
         * @param handler handler receiving the document.
         * @param depth number of enclosing containers.
         * @param isKey indicate whether the element to parse is a key or not.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ProbeType, typename Handler>
        int readValue (ProbeType& document, Handler& handler, size_t depth, bool isKey = false)
        {
            uint8_t head;
            if (JOIN_UNLIKELY (readHead (document, head) == -1))
            {
                return -1;
            }

            while ((head >> 5) == 6)
            {
                uint64_t tag;
                if (JOIN_UNLIKELY (readArgument (document, head & 0x1f, tag) == -1))
                {
                    return -1;
                }
                if (JOIN_UNLIKELY (readHead (document, head) == -1))
                {
                    return -1;
                }
            }

            uint8_t info = head & 0x1f;

            switch (head >> 5)
            {
                case 0:
                    return readUnsigned (document, handler, info, isKey);

                case 1:
                    return readNegative (document, handler, info, isKey);

                case 2:
                    return readString<false> (document, handler, info, isKey);

                case 3:
                    return readString<true> (document, handler, info, isKey);

                case 4:
                    return readArray (document, handler, info, depth, isKey);

                case 5:
                    return readObject (document, handler, info, depth, isKey);

                default:
                    return readSimple (document, handler, info, isKey);
            }
        }

        /**
         * @brief parse an unsigned integer.
         * @param document document to parse.
         * @param handler handler receiving the document.
         * @param info head additional information.
         * @param isKey indicate whether the element to parse is a key or not.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ProbeType, typename Handler>
        int readUnsigned (ProbeType& document, Handler& handler, uint8_t info, bool isKey)
        {
            uint64_t value;
            if (JOIN_UNLIKELY (readArgument (document, info, value) == -1))
            {
                return -1;
            }

            if (value <= std::numeric_limits<uint32_t>::max ())
            {
                auto number = static_cast<uint32_t> (value);
                return isKey ? handler.setKey (number) : handler.setUint (number);
            }

            return isKey ? handler.setKey (value) : handler.setUint64 (value);
        }

        /**
         * @brief parse a negative integer.
         * @param document document to parse.
         * @param handler handler receiving the document.
         * @param info head additional information.
         * @param isKey indicate whether the element to parse is a key or not.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ProbeType, typename Handler>
        int readNegative (ProbeType& document, Handler& handler, uint8_t info, bool isKey)
        {
            uint64_t value;
            if (JOIN_UNLIKELY (readArgument (document, info, value) == -1))
            {
                return -1;
            }

            if (JOIN_UNLIKELY (value > static_cast<uint64_t> (std::numeric_limits<int64_t>::max ())))
            {
                join::lastError = make_error_code (SaxErrc::InvalidValue);
                return -1;
            }

            int64_t number = -1 - static_cast<int64_t> (value);

            if (number >= std::numeric_limits<int32_t>::min ())
            {
                auto small = static_cast<int32_t> (number);
                return isKey ? handler.setKey (small) : handler.setInt (small);
            }

            return isKey ? handler.setKey (number) : handler.setInt64 (number);
        }

        /**
         * @brief parse a byte or text string.
         * @param document document to parse.
         * @param handler handler receiving the document.
         * @param info head additional information.
         * @param isKey indicate whether the element to parse is a key or not.
         * @return 0 on success, -1 otherwise.
         */
        template <bool Text, typename ProbeType, typename Handler>
        int readString (ProbeType& document, Handler& handler, uint8_t info, bool isKey)
        {
            if (info == 31)
            {
                return readChunks<Text> (document, handler, isKey);
            }

            uint64_t len;
            if (JOIN_UNLIKELY (readArgument (document, info, len) == -1))
            {
                return -1;
            }

            if (JOIN_UNLIKELY (len > std::numeric_limits<uint32_t>::max ()))
            {
                join::lastError = make_error_code (SaxErrc::InvalidValue);
                return -1;
            }

            if constexpr (std::is_same_v<ProbeType, dconv::StringProbe>)
            {
                if (JOIN_UNLIKELY (static_cast<size_t> (document.end () - document.tell ()) < len))
                {
                    join::lastError = make_error_code (SaxErrc::Truncated);
                    return -1;
                }

                const char* data = document.tell ();
                if (JOIN_UNLIKELY (validateString<Text> (data, len) == -1))
                {
                    return -1;
                }
                document.seek (data + len);
                return setString<Text> (handler, data, len, isKey);
            }

            thread_local std::string cache;
            std::string output = std::move (cache);
            output.clear ();
            if (JOIN_UNLIKELY (readBytes (document, static_cast<uint32_t> (len), output) == -1))
            {
                return -1;
            }

            if (JOIN_UNLIKELY (validateString<Text> (output.data (), output.size ()) == -1))
            {
                return -1;
            }

            int result = setString<Text> (handler, output.data (), output.size (), isKey);
            cache = std::move (output);
            return result;
        }

        /**
         * @brief parse an indefinite length byte or text string, concatenating its chunks.
         * @param document document to parse.
         * @param handler handler receiving the document.
         * @param isKey indicate whether the element to parse is a key or not.
         * @return 0 on success, -1 otherwise.
         */
        template <bool Text, typename ProbeType, typename Handler>
        int readChunks (ProbeType& document, Handler& handler, bool isKey)
        {
            thread_local std::string cache;
            std::string output = std::move (cache);
            output.clear ();

            while (!document.getIf (0xff))
            {
                uint8_t head;
                if (JOIN_UNLIKELY (readHead (document, head) == -1))
                {
                    return -1;
                }

                if (JOIN_UNLIKELY ((head >> 5) != (Text ? 3 : 2) || (head & 0x1f) == 31))
                {
                    join::lastError = make_error_code (SaxErrc::InvalidValue);
                    return -1;
                }

                uint64_t len;
                if (JOIN_UNLIKELY (readArgument (document, head & 0x1f, len) == -1))
                {
                    return -1;
                }

                if (JOIN_UNLIKELY (len > std::numeric_limits<uint32_t>::max ()))
                {
                    join::lastError = make_error_code (SaxErrc::InvalidValue);
                    return -1;
                }

                size_t pos = output.size ();
                if (JOIN_UNLIKELY (readBytes (document, static_cast<uint32_t> (len), output) == -1))
                {
                    return -1;
                }

                if (JOIN_UNLIKELY (validateString<Text> (output.data () + pos, output.size () - pos) == -1))
                {
                    return -1;
                }
            }

            int result = setString<Text> (handler, output.data (), output.size (), isKey);
            cache = std::move (output);
            return result;
        }

        /**
         * @brief validate the UTF-8 encoding of a text string when required by the read mode.
         * @param data string data.
         * @param len string length.
         * @return 0 on success, -1 otherwise.
         */
        template <bool Text>
        static int validateString ([[maybe_unused]] const char* data, [[maybe_unused]] size_t len) noexcept
        {
            if constexpr (Text && (ReadMode & CborReadMode::ValidateEncoding) != CborReadMode::None)
            {
                if (JOIN_UNLIKELY (utf8::validate (data, data + len) == -1))
                {
                    join::lastError = make_error_code (SaxErrc::InvalidEncoding);
                    return -1;
                }
            }
            return 0;
        }

        /**
         * @brief deliver a byte or text string to the handler.
         * @param handler handler receiving the document.
         * @param data string data.
         * @param len string length.
         * @param isKey indicate whether the element is a key or not.
         * @return 0 on success, -1 otherwise.
         */
        template <bool Text, typename Handler>
        static int setString (Handler& handler, const char* data, size_t len, bool isKey)
        {
            if constexpr (Text)
            {
                std::string_view value (data, len);
                return isKey ? handler.setKey (value) : handler.setString (value);
            }
            else
            {
                std::span<const uint8_t> value (reinterpret_cast<const uint8_t*> (data), len);
                return isKey ? handler.setKey (value) : handler.setBinary (value);
            }
        }

        /**
         * @brief parse an array.
         * @param document document to parse.
         * @param handler handler receiving the document.
         * @param info head additional information.
         * @param depth number of enclosing containers.
         * @param isKey indicate whether the element to parse is a key or not.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ProbeType, typename Handler>
        __attribute__ ((noinline)) int readArray (ProbeType& document, Handler& handler, uint8_t info, size_t depth,
                                                  bool isKey)
        {
            if (JOIN_UNLIKELY (depth == saxMaxDepth))
            {
                join::lastError = make_error_code (SaxErrc::StackOverflow);
                return -1;
            }

            if (isKey && JOIN_UNLIKELY (handler.startKey () == -1))
            {
                return -1;
            }

            if (info == 31)
            {
                if (JOIN_UNLIKELY (handler.startArray () == -1))
                {
                    return -1;
                }

                while (!document.getIf (0xff))
                {
                    if (JOIN_UNLIKELY (readValue (document, handler, depth + 1) == -1))
                    {
                        return -1;
                    }
                }

                return handler.stopArray ();
            }

            uint64_t len;
            if (JOIN_UNLIKELY (readArgument (document, info, len) == -1))
            {
                return -1;
            }

            if (JOIN_UNLIKELY (len > std::numeric_limits<uint32_t>::max ()))
            {
                join::lastError = make_error_code (SaxErrc::InvalidValue);
                return -1;
            }

            if (JOIN_UNLIKELY (handler.startArray (static_cast<uint32_t> (len)) == -1))
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
         * @brief parse a map.
         * @param document document to parse.
         * @param handler handler receiving the document.
         * @param info head additional information.
         * @param depth number of enclosing containers.
         * @param isKey indicate whether the element to parse is a key or not.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ProbeType, typename Handler>
        __attribute__ ((noinline)) int readObject (ProbeType& document, Handler& handler, uint8_t info, size_t depth,
                                                   bool isKey)
        {
            if (JOIN_UNLIKELY (depth == saxMaxDepth))
            {
                join::lastError = make_error_code (SaxErrc::StackOverflow);
                return -1;
            }

            if (isKey && JOIN_UNLIKELY (handler.startKey () == -1))
            {
                return -1;
            }

            if (info == 31)
            {
                if (JOIN_UNLIKELY (handler.startObject () == -1))
                {
                    return -1;
                }

                while (!document.getIf (0xff))
                {
                    if (JOIN_UNLIKELY (readValue (document, handler, depth + 1, true) == -1))
                    {
                        return -1;
                    }

                    if (JOIN_UNLIKELY (document.getIf (0xff)))
                    {
                        join::lastError = make_error_code (SaxErrc::InvalidKey);
                        return -1;
                    }

                    if (JOIN_UNLIKELY (readValue (document, handler, depth + 1) == -1))
                    {
                        return -1;
                    }
                }

                return handler.stopObject ();
            }

            uint64_t len;
            if (JOIN_UNLIKELY (readArgument (document, info, len) == -1))
            {
                return -1;
            }

            if (JOIN_UNLIKELY (len > std::numeric_limits<uint32_t>::max ()))
            {
                join::lastError = make_error_code (SaxErrc::InvalidValue);
                return -1;
            }

            if (JOIN_UNLIKELY (handler.startObject (static_cast<uint32_t> (len)) == -1))
            {
                return -1;
            }

            while (len)
            {
                if (JOIN_UNLIKELY (readValue (document, handler, depth + 1, true) == -1))
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
         * @brief parse a simple value or a floating point number.
         * @param document document to parse.
         * @param handler handler receiving the document.
         * @param info head additional information.
         * @param isKey indicate whether the element to parse is a key or not.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ProbeType, typename Handler>
        int readSimple (ProbeType& document, Handler& handler, uint8_t info, bool isKey)
        {
            switch (info)
            {
                case 20:
                    return isKey ? handler.setKey (false) : handler.setBool (false);

                case 21:
                    return isKey ? handler.setKey (true) : handler.setBool (true);

                case 22:
                case 23:
                    return isKey ? handler.setKey (nullptr) : handler.setNull ();

                case 25:
                    {
                        uint16_t half;
                        if (JOIN_UNLIKELY (unpack<uint16_t> (document, half) == -1))
                        {
                            return -1;
                        }
                        double number = decodeHalf (half);
                        return isKey ? handler.setKey (number) : handler.setDouble (number);
                    }

                case 26:
                    {
                        double number;
                        if (JOIN_UNLIKELY (unpack<float> (document, number) == -1))
                        {
                            return -1;
                        }
                        return isKey ? handler.setKey (number) : handler.setDouble (number);
                    }

                case 27:
                    {
                        double number;
                        if (JOIN_UNLIKELY (unpack<double> (document, number) == -1))
                        {
                            return -1;
                        }
                        return isKey ? handler.setKey (number) : handler.setDouble (number);
                    }

                default:
                    join::lastError = make_error_code (SaxErrc::InvalidValue);
                    return -1;
            }
        }

        /**
         * @brief read a data item head.
         * @param document document to parse.
         * @param head head read.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ProbeType>
        static int readHead (ProbeType& document, uint8_t& head)
        {
            int ch = document.get ();
            if (JOIN_UNLIKELY (ch == std::char_traits<char>::eof ()))
            {
                join::lastError = make_error_code (SaxErrc::Truncated);
                return -1;
            }
            head = static_cast<uint8_t> (ch);
            return 0;
        }

        /**
         * @brief read a data item head argument.
         * @param document document to parse.
         * @param info head additional information.
         * @param value argument read.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ProbeType>
        static int readArgument (ProbeType& document, uint8_t info, uint64_t& value)
        {
            switch (info)
            {
                case 24:
                    return unpack<uint8_t> (document, value);

                case 25:
                    return unpack<uint16_t> (document, value);

                case 26:
                    return unpack<uint32_t> (document, value);

                case 27:
                    return unpack<uint64_t> (document, value);

                default:
                    if (JOIN_UNLIKELY (info > 27))
                    {
                        join::lastError = make_error_code (SaxErrc::InvalidValue);
                        return -1;
                    }
                    value = info;
                    return 0;
            }
        }

        /**
         * @brief decode a half precision floating point number.
         * @param half half precision bits.
         * @return decoded number.
         */
        static double decodeHalf (uint16_t half) noexcept
        {
            int exponent = (half >> 10) & 0x1f;
            int mantissa = half & 0x3ff;
            double value;

            if (exponent == 0)
            {
                value = std::ldexp (mantissa, -24);
            }
            else if (exponent != 31)
            {
                value = std::ldexp (mantissa + 1024, exponent - 25);
            }
            else if (mantissa == 0)
            {
                value = std::numeric_limits<double>::infinity ();
            }
            else
            {
                value = std::numeric_limits<double>::quiet_NaN ();
            }

            return (half & 0x8000) ? -value : value;
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
    };

    /// CBOR reader.
    using CborReader = BasicCborReader<CborReadMode::None>;
}

#endif
