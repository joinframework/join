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

namespace join
{
namespace sax
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
                char buf[5];
                buf[0] = 0xd2;
                storeUint32 (&buf[1], static_cast <uint32_t> (value));
                append (buf, sizeof (buf));
            }
            else if (value < -(1 << 7))
            {
                char buf[3];
                buf[0] = 0xd1;
                storeUint16 (&buf[1], static_cast <uint16_t> (value));
                append (buf, sizeof (buf));
            }
            else if (value < -(1 << 5))
            {
                char buf[2];
                buf[0] = 0xd0;
                storeUint8 (&buf[1], static_cast <uint8_t> (value));
                append (buf, sizeof (buf));
            }
            else if (value < (1 << 7))
            {
                char buf;
                storeUint8 (&buf, static_cast <uint8_t> (value));
                append (buf);
            }
            else if (value < (1 << 8))
            {
                char buf[2];
                buf[0] = 0xcc;
                storeUint8 (&buf[1], static_cast <uint8_t> (value));
                append (buf, sizeof (buf));
            }
            else if (value < (1 << 16))
            {
                char buf[3];
                buf[0] = 0xcd;
                storeUint16 (&buf[1], static_cast <uint16_t> (value));
                append (buf, sizeof (buf));
            }
            else
            {
                char buf[5];
                buf[0] = 0xce;
                storeUint32 (&buf[1], static_cast <uint32_t> (value));
                append (buf, sizeof (buf));
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
                char buf;
                storeUint8 (&buf, static_cast <uint8_t> (value));
                append (buf);
            }
            else if (value < (1 << 8))
            {
                char buf[2];
                buf[0] = 0xcc;
                storeUint8 (&buf[1], static_cast <uint8_t> (value));
                append (buf, sizeof (buf));
            }
            else if (value < (1 << 16))
            {
                char buf[3];
                buf[0] = 0xcd;
                storeUint16 (&buf[1], static_cast <uint16_t> (value));
                append (buf, sizeof (buf));
            }
            else
            {
                char buf[5];
                buf[0] = 0xce;
                storeUint32 (&buf[1], value);
                append (buf, sizeof (buf));
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
            if(value < -(1LL << 31)) 
            {
                char buf[9];
                buf[0] = 0xd3;
                storeUint64 (&buf[1], static_cast <uint64_t> (value));
                append (buf, sizeof (buf));
            }
            else if (value < -(1LL << 15))
            {
                char buf[5];
                buf[0] = 0xd2;
                storeUint32 (&buf[1], static_cast <uint32_t> (value));
                append (buf, sizeof (buf));
            }
            else if (value < -(1LL << 7))
            {
                char buf[3];
                buf[0] = 0xd1;
                storeUint16 (&buf[1], static_cast <uint16_t> (value));
                append (buf, sizeof (buf));
            }
            else if (value < -(1LL << 5))
            {
                char buf[2];
                buf[0] = 0xd0;
                storeUint8 (&buf[1], static_cast <uint8_t> (value));
                append (buf, sizeof (buf));
            }
            else if (value < (1LL << 7))
            {
                char buf;
                storeUint8 (&buf, static_cast <uint8_t> (value));
                append (buf);
            }
            else if (value < (1LL << 8))
            {
                char buf[2];
                buf[0] = 0xcc;
                storeUint8 (&buf[1], static_cast <uint8_t> (value));
                append (buf, sizeof (buf));
            }
            else if (value < (1LL << 16))
            {
                char buf[3];
                buf[0] = 0xcd;
                storeUint16 (&buf[1], static_cast <uint16_t> (value));
                append (buf, sizeof (buf));
            }
            else if (value < (1LL << 32))
            {
                char buf[5];
                buf[0] = 0xce;
                storeUint32 (&buf[1], static_cast <uint32_t> (value));
                append (buf, sizeof (buf));
            }
            else
            {
                char buf[9];
                buf[0] = 0xcf;
                storeUint64 (&buf[1], static_cast <uint64_t> (value));
                append (buf, sizeof (buf));
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
                char buf;
                storeUint8 (&buf, static_cast <uint8_t> (value));
                append (buf);
            }
            else if (value < (1ULL << 8))
            {
                char buf[2];
                buf[0] = 0xcc;
                storeUint8 (&buf[1], static_cast <uint8_t> (value));
                append (buf, sizeof (buf));
            }
            else if (value < (1ULL << 16))
            {
                char buf[3];
                buf[0] = 0xcd;
                storeUint16 (&buf[1], static_cast <uint16_t> (value));
                append (buf, sizeof (buf));
            }
            else if (value < (1ULL << 32))
            {
                char buf[5];
                buf[0] = 0xce;
                storeUint32 (&buf[1], static_cast <uint32_t> (value));
                append (buf, sizeof (buf));
            }
            else
            {
                char buf[9];
                buf[0] = 0xcf;
                storeUint64 (&buf[1], value);
                append (buf, sizeof (buf));
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
            char buf[9];
            buf[0] = 0xcb;
            storeReal (&buf[1], value);
            append (buf, sizeof (buf));
            return 0;
        }

        /**
         * @brief set string value.
         * @param value string value to set.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setString (const std::string& value) override
        {
            return setStringData (value.c_str (), value.size ());
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
                append (static_cast <char> (0x90u | size));
            }
            else if (size < 65536)
            {
                char buf[3];
                buf[0] = 0xdc; 
                storeUint16 (&buf[1], static_cast <uint16_t> (size));
                append (buf, sizeof (buf));
            }
            else
            {
                char buf[5];
                buf[0] = 0xdd; 
                storeUint32 (&buf[1], static_cast <uint32_t> (size));
                append (buf, sizeof (buf));
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
                char buf[3];
                buf[0] = 0xde;
                storeUint16 (&buf[1], static_cast <uint16_t> (size));
                append (buf, sizeof (buf));
            } 
            else 
            {
                char buf[5];
                buf[0] = 0xdf;
                storeUint32 (&buf[1], static_cast <uint32_t> (size));
                append (buf, sizeof (buf));
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
            return setStringData (key.c_str (), key.size ());
        }

    private:
        /**
         * @brief set data stored as string.
         * @param data data to set.
         * @param size data size.
         * @return 0 on success, -1 otherwise.
         */
        int setStringData (const char* data, uint32_t size)
        {
            if (size < 32)
            {
                append (static_cast <char> (0xa0 | size));
            } 
            else if (size < 256)
            {
                char buf[2];
                buf[0] = 0xd9;
                storeUint8 (&buf[1], static_cast <uint8_t> (size));
                append (buf, sizeof (buf));
            }
            else if (size < 65536)
            {
                char buf[3];
                buf[0] = 0xda;
                storeUint16 (&buf[1], static_cast <uint16_t> (size));
                append (buf, sizeof (buf));
            }
            else
            {
                char buf[5];
                buf[0] = 0xdb;
                storeUint32 (&buf[1], static_cast <uint32_t> (size));
                append (buf, sizeof (buf));
            }
            append (data, size);
            return 0;
        }

        /**
         * @brief store unsigned 8 bits integer into destination buffer.
         * @param dest destination.
         * @param src source.
         */
        void storeUint8 (char* dest, uint8_t src)
        {
            memcpy (dest, &utils::swap (src), sizeof (src));
        }

        /**
         * @brief store unsigned 16 bits integer into destination buffer.
         * @param dest destination.
         * @param src source.
         */
        void storeUint16 (char* dest, uint16_t src)
        {
            memcpy (dest, &utils::swap (src), sizeof (src));
        }

        /**
         * @brief store unsigned 32 bits integer into destination buffer.
         * @param dest destination.
         * @param src source.
         */
        void storeUint32 (char* dest, uint32_t src)
        {
            memcpy (dest, &utils::swap (src), sizeof (src));
        }

        /**
         * @brief store unsigned 64 bits integer into destination buffer.
         * @param dest destination.
         * @param src source.
         */
        void storeUint64 (char* dest, uint64_t src)
        {
            memcpy (dest, &utils::swap (src), sizeof (src));
        }

        /**
         * @brief store real into destination buffer.
         * @param dest destination.
         * @param src source.
         */
        void storeReal (char* dest, double src)
        {
            memcpy (dest, &utils::swap (src), sizeof (src));
        }
    };
}
}

#endif
