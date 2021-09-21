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

#ifndef __JOIN_PACK_HPP__
#define __JOIN_PACK_HPP__

// libjoin.
#include <join/utils.hpp>
#include <join/sax.hpp>

// C++.
#include <stdexcept>

namespace join
{
    /**
     * @brief message pack writer class.
     */
    class PackWriter : public StreamWriter
    {
    public:
        /**
         * @brief create instance.
         * @param document MessagePack document to create.
         */
        PackWriter (std::ostream& document)
        : StreamWriter (document)
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
        virtual ~PackWriter () = default;

        /**
         * @brief set null value.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setNull () override
        {
            append (0xc0);
            return 0;
        }

        /**
         * @brief set boolean value.
         * @param value boolean value to set.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setBool (bool value) override
        {
            if (value)
            {
                append (0xc3);
            }
            else
            {
                append (0xc2);
            }
            return 0;
        }

        /**
         * @brief set integer value.
         * @param value integer value to set.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setInt (int32_t value) override
        {
            if (value < -(1 << 15))
            {
                append (0xd2);
                pack (static_cast <uint32_t> (value));
            }
            else if (value < -(1 << 7))
            {
                append (0xd1);
                pack (static_cast <uint16_t> (value));
            }
            else if (value < -(1 << 5))
            {
                append (0xd0);
                pack (static_cast <uint8_t> (value));
            }
            else if (value < (1 << 7))
            {
                append (static_cast <uint8_t> (value));
            }
            else if (value < (1 << 8))
            {
                append (0xcc);
                pack (static_cast <uint8_t> (value));
            }
            else if (value < (1 << 16))
            {
                append (0xcd);
                pack (static_cast <uint16_t> (value));
            }
            else
            {
                append (0xce);
                pack (static_cast <uint32_t> (value));
            }
            return 0;
        }

        /**
         * @brief set unsigned integer value.
         * @param value unsigned integer value to set.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setUint (uint32_t value) override
        {
            if (value < (1 << 7))
            {
                append (static_cast <uint8_t> (value));
            }
            else if (value < (1 << 8))
            {
                append (0xcc);
                pack (static_cast <uint8_t> (value));
            }
            else if (value < (1 << 16))
            {
                append (0xcd);
                pack (static_cast <uint16_t> (value));
            }
            else
            {
                append (0xce);
                pack (value);
            }
            return 0;
        }

        /**
         * @brief set 64 bits integer value.
         * @param value 64 bits integer value to set.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setInt64 (int64_t value) override
        {
            if (value < -(1LL << 31)) 
            {
                append (0xd3);
                pack (static_cast <uint64_t> (value));
            }
            else if (value < -(1LL << 15))
            {
                append (0xd2);
                pack (static_cast <uint32_t> (value));
            }
            else if (value < -(1LL << 7))
            {
                append (0xd1);
                pack (static_cast <uint16_t> (value));
            }
            else if (value < -(1LL << 5))
            {
                append (0xd0);
                pack (static_cast <uint8_t> (value));
            }
            else if (value < (1LL << 7))
            {
                append (static_cast <uint8_t> (value));
            }
            else if (value < (1LL << 8))
            {
                append (0xcc);
                pack (static_cast <uint8_t> (value));
            }
            else if (value < (1LL << 16))
            {
                append (0xcd);
                pack (static_cast <uint16_t> (value));
            }
            else if (value < (1LL << 32))
            {
                append (0xce);
                pack (static_cast <uint32_t> (value));
            }
            else
            {
                append (0xcf);
                pack (static_cast <uint64_t> (value));
            }
            return 0;
        }

        /**
         * @brief set unsigned 64 bits integer value.
         * @param value unsigned 64 bits integer value to set.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setUint64 (uint64_t value) override
        {
            if (value < (1ULL << 7))
            {
                append (static_cast <uint8_t> (value));
            }
            else if (value < (1ULL << 8))
            {
                append (0xcc);
                pack (static_cast <uint8_t> (value));
            }
            else if (value < (1ULL << 16))
            {
                append (0xcd);
                pack (static_cast <uint16_t> (value));
            }
            else if (value < (1ULL << 32))
            {
                append (0xce);
                pack (static_cast <uint32_t> (value));
            }
            else
            {
                append (0xcf);
                pack (value);
            }
            return 0;
        }

        /**
         * @brief set real value.
         * @param value real value to set.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setDouble (double value) override
        {
            append (0xcb);
            pack (value);
            return 0;
        }

        /**
         * @brief set string value.
         * @param value string value to set.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setString (const std::string& value) override
        {
            if (value.size () < 32)
            {
                append (static_cast <char> (0xa0 | value.size ()));
            } 
            else if (value.size () < 256)
            {
                append (0xd9);
                pack (static_cast <uint8_t> (value.size ()));
            }
            else if (value.size () < 65536)
            {
                append (0xda);
                pack (static_cast <uint16_t> (value.size ()));
            }
            else
            {
                append (0xdb);
                pack (static_cast <uint32_t> (value.size ()));
            }
            append (value.c_str (), value.size ());
            return 0;
        }

        /**
         * @brief start array.
         * @param size array size.
         * @return 0 on success, -1 otherwise.
         */
        virtual int startArray (uint32_t size = 0) override
        {
            if (size < 16) 
            {
                append (static_cast <char> (0x90 | size));
            }
            else if (size < 65536)
            {
                append (0xdc);
                pack (static_cast <uint16_t> (size));
            }
            else
            {
                append (0xdd);
                pack (size);
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
            if (size < 16) 
            {
                append (static_cast <char> (0x80 | size));
            } 
            else if (size < 65536) 
            {
                append (0xde);
                pack (static_cast <uint16_t> (size));
            } 
            else 
            {
                append (0xdf);
                pack (size);
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
            return setString (key);
        }

    protected:
        /**
         * @brief pack value into stream.
         * @param value value to pack.
         */
        template <typename Type>
        void pack (Type value)
        {
            append (reinterpret_cast <const char *> (&swap (value)), sizeof (value));
        }
    };

    /**
     * @brief message pack reader class.
     */
    class PackReader : public StreamReader
    {
    public:
        /**
         * @brief default constructor.
         * @param root Value to write.
         */
        PackReader (Value& root)
        : StreamReader (root)
        {
        }

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
        virtual ~PackReader () = default;

        /**
         * @brief deserialize a document.
         * @param document document to parse.
         * @param length The length of the document to parse.
         * @return 0 on success, -1 otherwise.
         */
        int deserialize (const char* document, size_t length) override
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
        int deserialize (const char* first, const char* last) override
        {
            StringView in (first, last);
            return read (in);
        }

        /**
         * @brief deserialize a document.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        int deserialize (const std::string& document) override
        {
            StringView in (document.c_str (), document.size ());
            return read (in);
        }

        /**
         * @brief parse a document.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        int deserialize (std::istream& document) override
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
            if (JOIN_SAX_LIKELY (readValue (document) == 0))
            {
                if (JOIN_SAX_LIKELY (document.peek () == std::char_traits <char>::eof ()))
                {
                    return 0;
                }

                join::lastError = make_error_code (SaxErrc::ExtraData);
            }

            return -1;
        }

        /**
         * @brief parse value.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType>
        int readValue (ViewType& document)
        {
            char head = document.peek ();

            try
            {
                if (isArray (static_cast <uint8_t> (head)))
                {
                    return readArray (document);
                }
                else if (isObject (static_cast <uint8_t> (head)))
                {
                    return readObject (document);
                }
                else if (isNull (static_cast <uint8_t> (head)))
                {
                    return readNull (document);
                }
                else if (isFalse (static_cast <uint8_t> (head)))
                {
                    return readFalse (document);
                }
                else if (isTrue (static_cast <uint8_t> (head)))
                {
                    return readTrue (document);
                }
                else if (isString (static_cast <uint8_t> (head)))
                {
                    return readString (document);
                }
                else if (isBin (static_cast <uint8_t> (head)))
                {
                    return readBin (document);
                }
                else if (isNumber (static_cast <uint8_t> (head)))
                {
                    return readNumber (document);
                }
                else
                {
                    join::lastError = make_error_code (SaxErrc::InvalidValue);
                    return -1;
                }
            }
            catch (...)
            {
                join::lastError = make_error_code (SaxErrc::InvalidValue);
                return -1;
            }

            return 0;
        }

        /**
         * @brief parse a null value.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType>
        int readNull (ViewType& document)
        {
            document.get ();
            return setNull ();
        }

        /**
         * @brief parse a false value.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType>
        int readFalse (ViewType& document)
        {
            document.get ();
            return setBool (false);
        }

        /**
         * @brief parse a true value.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType>
        int readTrue (ViewType& document)
        {
            document.get ();
            return setBool (true);
        }

        /**
         * @brief parse an array value.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType>
        int readArray (ViewType& document)
        {
            uint32_t len = 0;

            if (document.getIf (0xdd))
            {
                len = unpack <uint32_t> (document);
            }
            else if (document.getIf (0xdc))
            {
                len = unpack <uint16_t> (document);
            }
            else
            {
                len = document.get () & ~0x90;
            }

            if (JOIN_SAX_UNLIKELY (startArray (len) == -1))
            {
                return -1;
            }

            while (len)
            {
                if (JOIN_SAX_UNLIKELY (readValue (document) == -1))
                {
                    return -1;
                }

                --len;
            }

            return stopArray ();
        }

        /**
         * @brief parse an object value.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType>
        int readObject (ViewType& document)
        {
            uint32_t len = 0;

            if (document.getIf (0xdf))
            {
                len = unpack <uint32_t> (document);
            }
            else if (document.getIf (0xde))
            {
                len = unpack <uint16_t> (document);
            }
            else
            {
                len = document.get () & ~0x80;
            }

            if (JOIN_SAX_UNLIKELY (startObject (len) == -1))
            {
                return -1;
            }

            while (len)
            {
                if (JOIN_SAX_UNLIKELY (readString (document, true) == -1))
                {
                    return -1;
                }

                if (JOIN_SAX_UNLIKELY (readValue (document) == -1))
                {
                    return -1;
                }

                --len;
            }

            return stopObject ();
        }

        /**
         * @brief parse a string value.
         * @param document document to parse.
         * @param isKey indicate whether the string to parse is a key or not.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType>
        int readString (ViewType& document, bool isKey = false)
        {
            uint32_t len = 0;

            if (document.getIf (0xdb))
            {
                len = unpack <uint32_t> (document);
            }
            else if (document.getIf (0xda))
            {
                len = unpack <uint16_t> (document);
            }
            else if (document.getIf (0xd9))
            {
                len = unpack <uint8_t> (document);
            }
            else
            {
                len = document.get () & ~0xa0;
            }

            std::string output;
            output.resize (len);

            if (JOIN_SAX_UNLIKELY (document.read (&output[0], len) != len))
            {
                join::lastError = make_error_code (SaxErrc::InvalidValue);
                return -1;
            }

            return isKey ? setKey (output) : setString (output);
        }

        /**
         * @brief parse binary data.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType>
        int readBin (ViewType& document)
        {
            uint32_t len = 0;

            if (document.getIf (0xc6))
            {
                len = unpack <uint32_t> (document);
            }
            else if (document.getIf (0xc5))
            {
                len = unpack <uint16_t> (document);
            }
            else if (document.getIf (0xc4))
            {
                len = unpack <uint8_t> (document);
            }
            else
            {
                join::lastError = make_error_code (SaxErrc::InvalidValue);
                return -1;
            }

            std::string output;
            output.resize (len);

            if (JOIN_SAX_UNLIKELY (document.read (&output[0], len) != len))
            {
                join::lastError = make_error_code (SaxErrc::InvalidValue);
                return -1;
            }

            return setString (output);
        }

        /**
         * @brief parse a number value.
         * @param document document to parse.
         * @return 0 on success, -1 otherwise.
         */
        template <typename ViewType>
        int readNumber (ViewType& document)
        {
            if (document.getIf (0xd3))
            {
                return setInt64 (unpack <int64_t> (document));
            }
            else if (document.getIf (0xcf))
            {
                return setUint64 (unpack <uint64_t> (document));
            }
            else if (document.getIf (0xcb))
            {
                return setDouble (unpack <double> (document));
            }
            else if (document.getIf (0xd2))
            {
                return setInt (unpack <int32_t> (document));
            }
            else if (document.getIf (0xce))
            {
                return setUint (unpack <uint32_t> (document));
            }
            else if (document.getIf (0xca))
            {
                return setDouble (unpack <float> (document));
            }
            else if (document.getIf (0xd1))
            {
                return setInt (unpack <int16_t> (document));
            }
            else if (document.getIf (0xcd))
            {
                return setUint (unpack <uint16_t> (document));
            }
            else if (document.getIf (0xd0))
            {
                return setInt (unpack <int8_t> (document));
            }
            else if (document.getIf (0xcc))
            {
                return setUint (unpack <uint8_t> (document));
            }

            return setInt (document.get ());
        }

        /**
         * @brief unpack value from stream.
         * @param document document to parse.
         * @return unpacked value.
         */
        template <typename Type, typename ViewType>
        std::enable_if_t <std::is_arithmetic <Type>::value, Type>
        static unpack (ViewType& document)
        {
            Type value;
            if (JOIN_SAX_UNLIKELY (document.read (reinterpret_cast <char *> (&value), sizeof (value)) != sizeof (value)))
            {
                throw std::range_error ("not enough data to unpack");
            }
            return swap (value);
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
            return ((c <= 0x7f) || (c >= 0xe0)) || (c == 0xd0) || (c == 0xd1) || (c == 0xd2) || (c == 0xd3);
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
