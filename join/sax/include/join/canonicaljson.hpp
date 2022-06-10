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

#ifndef __JOIN_CANONICALJSON_HPP__
#define __JOIN_CANONICALJSON_HPP__

// libjoin.
#include <join/json.hpp>

// C++.
#include <codecvt>
#include <locale>

namespace join
{
    /**
     * @brief JSON canonicalizer class.
     */
    class JsonCanonicalizer : public JsonWriter
    {
    public:
        /**
         * @brief create instance.
         * @param document JSON document to create.
         */
        JsonCanonicalizer (std::ostream& document)
        : JsonWriter (document, 0)
        {
        }

        /**
         * @brief copy constructor.
         * @param other object to copy.
         */
        JsonCanonicalizer (const JsonCanonicalizer& other) = delete;

        /**
         * @brief copy assignment.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        JsonCanonicalizer& operator= (const JsonCanonicalizer& other) = delete;

        /**
         * @brief move constructor.
         * @param other object to move.
         */
        JsonCanonicalizer (JsonCanonicalizer&& other) = delete;

        /**
         * @brief move assignment.
         * @param other object to move.
         * @return a reference of the current object.
         */
        JsonCanonicalizer& operator= (JsonCanonicalizer&& other) = delete;

        /**
         * @brief destroy instance.
         */
        virtual ~JsonCanonicalizer () = default;

        /**
         * @brief set real value.
         * @param value real value to set.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setDouble (double value) override
        {
            array ();
            if (std::isfinite (value))
            {
                if ((std::trunc (value) == value) && 
                    (value >= 0) &&
                    (value < static_cast <double> (std::numeric_limits <uint64_t>::max ())))
                {
                    writeUint64 (static_cast <uint64_t> (value));
                }
                else if ((std::trunc (value) == value) &&
                         (value >= static_cast <double> (std::numeric_limits <int64_t>::min ())) &&
                         (value <  static_cast <double> (std::numeric_limits <int64_t>::max ())))
                {
                    writeInt64 (static_cast <int64_t> (value));
                }
                else
                {
                    writeDouble (value);
                }
            }
            else
            {
                append ("null", 4);
            }
            _first = false;
            return 0;
        }

    protected:
        /**
         * @brief set object value.
         * @param value array value to set.
         * @return 0 on success, -1 otherwise.
         */
        virtual int setObject (const Object& object) override
        {
            if (startObject (object.size ()) == -1)
            {
                return -1;
            }
            std::vector <const Member *> members;
            std::transform (object.begin (), object.end (), std::back_inserter (members), [] (const Member &member) {return &member;});
            std::sort (members.begin (), members.end (), [] (const Member *a, const Member *b) { 
                std::wstring_convert <std::codecvt_utf8_utf16 <char16_t>, char16_t> cvt_utf8_utf16;
                std::u16string wa = cvt_utf8_utf16.from_bytes (a->first.data ());
                std::u16string wb = cvt_utf8_utf16.from_bytes (b->first.data ());
                return wa < wb;
            });
            for (auto const& member : members)
            {
                if ((setKey (member->first) == -1) || (serialize (member->second) == -1))
                {
                    return -1;
                }
            }
            return stopObject ();
        }

        /**
         * @brief write real value.
         * @param value real value to write.
         */
        virtual void writeDouble (double value) override
        {
            char beg[25];
            char* end = join::dtoa (beg, value);
            for (char* pos = beg; pos < end; ++pos)
            {
                append (*pos);
                if ((*pos == 'e') && (*(pos + 1) != '-'))
                {
                    append ('+');
                }
            }
        }
    };
}

#endif
