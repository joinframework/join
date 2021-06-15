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

// libjoin.
#include <join/jsonwriter.hpp>
#include <join/error.hpp>
#include <join/dtoa.hpp>

using join::sax::JsonErrc;
using join::sax::JsonCategory;
using join::sax::JsonWriter;

// =========================================================================
//   CLASS     : JsonCategory
//   METHOD    : name
// =========================================================================
const char* JsonCategory::name () const noexcept
{
    return "libjoin";
}

// =========================================================================
//   CLASS     : JsonCategory
//   METHOD    : message
// =========================================================================
std::string JsonCategory::message (int code) const
{
    switch (static_cast <JsonErrc> (code))
    {
        case JsonErrc::InvalidComment:
            return "comment is invalid";
        case JsonErrc::InvalidEscaping:
            return "character escaping is invalid";
        case JsonErrc::InvalidEncoding:
            return "character encoding is invalid";
        case JsonErrc::IllegalCharacter:
            return "illegal character";
        case JsonErrc::MissingCurlyBracket:
            return "missing curly bracket";
        case JsonErrc::MissingSquareBracket:
            return "missing square bracket";
        case JsonErrc::MissingQuote:
            return "missing quote";
        case JsonErrc::MissingColon:
            return "missing colon";
        case JsonErrc::MissingComma:
            return "missing comma";
        default:
            return "success";
    }
}

// =========================================================================
//   CLASS     :
//   METHOD    : jsonCategory
// =========================================================================
const std::error_category& join::sax::jsonCategory ()
{
    static JsonCategory instance;
    return instance;
}

// =========================================================================
//   CLASS     :
//   METHOD    : make_error_code
// =========================================================================
std::error_code join::sax::make_error_code (JsonErrc code)
{
    return std::error_code (static_cast <int> (code), jsonCategory ());
}

// =========================================================================
//   CLASS     :
//   METHOD    : make_error_condition
// =========================================================================
std::error_condition join::sax::make_error_condition (JsonErrc code)
{
    return std::error_condition (static_cast <int> (code), jsonCategory ());
}

// =========================================================================
//   CLASS     : JsonWriter
//   METHOD    : JsonWriter
// =========================================================================
JsonWriter::JsonWriter (std::ostream& document, size_t indentation)
: StreamWriter (document),
  indentation_ (indentation)
{
}

// =========================================================================
//   CLASS     : JsonWriter
//   METHOD    : setNull
// =========================================================================
int JsonWriter::setNull ()
{
    array ();
    append ("null", 4);
    first_ = false;
    return 0;
}

// =========================================================================
//   CLASS     : JsonWriter
//   METHOD    : setBool
// =========================================================================
int JsonWriter::setBool (bool value)
{
    array ();
    if (value)
    {
        append ("true", 4);
    }
    else
    {
        append ("false", 5);
    }
    first_ = false;
    return 0;
}

// =========================================================================
//   CLASS     : JsonWriter
//   METHOD    : setInt
// =========================================================================
int JsonWriter::setInt (int32_t value)
{
    array ();
    writeInt (value);
    first_ = false;
    return 0;
}

// =========================================================================
//   CLASS     : JsonWriter
//   METHOD    : setUint
// =========================================================================
int JsonWriter::setUint (uint32_t value)
{
    array ();
    writeUint (value);
    first_ = false;
    return 0;
}

// =========================================================================
//   CLASS     : JsonWriter
//   METHOD    : setInt64
// =========================================================================
int JsonWriter::setInt64 (int64_t value)
{
    array ();
    writeInt64 (value);
    first_ = false;
    return 0;
}

// =========================================================================
//   CLASS     : JsonWriter
//   METHOD    : setUint64
// =========================================================================
int JsonWriter::setUint64 (uint64_t value)
{
    array ();
    writeUint64 (value);
    first_ = false;
    return 0;
}

// =========================================================================
//   CLASS     : JsonWriter
//   METHOD    : setDouble
// =========================================================================
int JsonWriter::setDouble (double value)
{
    array ();
    if (std::isfinite (value))
    {
        writeDouble (value);
    }
    else if (std::isnan (value))
    {
        if (std::signbit (value))
        {
            append ("-NaN", 4);
        }
        else
        {
            append ("NaN", 3);
        }
    }
    else
    {
        if (std::signbit (value))
        {
            append ("-Inf", 4);
        }
        else
        {
            append ("Inf", 3);
        }
    }
    first_ = false;
    return 0;
}

// =========================================================================
//   CLASS     : JsonWriter
//   METHOD    : setString
// =========================================================================
int JsonWriter::setString (const std::string& value)
{
    array ();
    append ('"');
    if (writeEscaped (value) == -1)
    {
        return -1;
    }
    append ('"');
    first_ = false;
    return 0;
}

// =========================================================================
//   CLASS     : JsonWriter
//   METHOD    : startArray
// =========================================================================
int JsonWriter::startArray ([[maybe_unused]] uint32_t size)
{
    array ();
    append ('[');
    tab_.append (indentation_, ' ');
    first_ = true;
    stack_.push (true);
    return 0;
}

// =========================================================================
//   CLASS     : JsonWriter
//   METHOD    : stopArray
// =========================================================================
int JsonWriter::stopArray ()
{
    tab_.erase (tab_.size () - indentation_);
    if (!first_)
    {
        endLine ();
        indent ();
    }
    append (']');
    first_ = false;
    stack_.pop ();
    return 0;
}

// =========================================================================
//   CLASS     : JsonWriter
//   METHOD    : startObject
// =========================================================================
int JsonWriter::startObject ([[maybe_unused]] uint32_t size)
{
    array ();
    append ('{');
    tab_.append (indentation_, ' ');
    first_ = true;
    stack_.push (false);
    return 0;
}

// =========================================================================
//   CLASS     : JsonWriter
//   METHOD    : setKey
// =========================================================================
int JsonWriter::setKey (const std::string& key)
{
    comma ();
    endLine ();
    indent ();
    append ('"');
    if (writeEscaped (key) == -1)
    {
        return -1;
    }
    append ('"');
    append (':');
    space ();
    first_ = true;
    return 0;
}

// =========================================================================
//   CLASS     : JsonWriter
//   METHOD    : stopObject
// =========================================================================
int JsonWriter::stopObject ()
{
    tab_.erase (tab_.size () - indentation_);
    if (!first_)
    {
        endLine ();
        indent ();
    }
    append ('}');
    first_ = false;
    stack_.pop ();
    return 0;
}

// =========================================================================
//   CLASS     : JsonWriter
//   METHOD    : writeInt
// =========================================================================
void JsonWriter::writeInt (int32_t value)
{
    if (value == std::numeric_limits <int32_t>::min ())
    {
        append ('-');
        writeUint64 (static_cast <uint64_t> (std::numeric_limits <int32_t>::max ()) + 1);
    }
    else if (value < 0)
    {
        append ('-');
        writeUint64 (-value);
    }
    else
    {
        writeInt64 (value);
    }
}

// =========================================================================
//   CLASS     : JsonWriter
//   METHOD    : writeUint
// =========================================================================
void JsonWriter::writeUint (uint32_t value)
{
    writeUint64 (static_cast <uint64_t> (value));
}

// =========================================================================
//   CLASS     : JsonWriter
//   METHOD    : writeInt64
// =========================================================================
void JsonWriter::writeInt64 (int64_t value)
{
    if (value == std::numeric_limits <int64_t>::min ())
    {
        append ('-');
        writeUint64 (static_cast <uint64_t> (std::numeric_limits <int64_t>::max ()) + 1);
    }
    else if (value < 0)
    {
        append ('-');
        writeUint64 (-value);
    }
    else
    {
        writeUint64 (value);
    }
}

// =========================================================================
//   CLASS     : JsonWriter
//   METHOD    : writeUint64
// =========================================================================
void JsonWriter::writeUint64 (uint64_t value)
{
    std::stack <char> stack;
    while (value)
    {
        stack.push ((value % 10) + '0');
        value /= 10;
    }
    if (stack.empty ())
    {
        append ('0');
    }
    while (!stack.empty ())
    {
        append (stack.top ());
        stack.pop ();
    }
}

// =========================================================================
//   CLASS     : JsonWriter
//   METHOD    : writeDouble
// =========================================================================
void JsonWriter::writeDouble (double value)
{
    char buf[25];
    char* end = join::sax::dtoa (buf, value);
    append (buf, end - buf);
}

// =========================================================================
//   CLASS     : JsonWriter
//   METHOD    : utf8Codepoint
// =========================================================================
int JsonWriter::utf8Codepoint (std::string::const_iterator& cur, std::string::const_iterator& end, uint32_t& codepoint)
{
    uint8_t u0 = static_cast <uint8_t> (*cur);
    if (u0 < 0x80)
    {
        codepoint = u0;
        return 0;
    }

    if (++cur == end)
    {
        return -1;
    }

    uint8_t u1 = static_cast <uint8_t> (*cur);
    if (u0 < 0xE0)
    {
        codepoint = ((u0 & 0x1F) << 6) | (u1 & 0x3F);
        if (codepoint < 0x80)
        {
            return -1;
        }
        return 0;
    }

    if (++cur == end)
    {
        return -1;
    }

    uint8_t u2 = static_cast <uint8_t> (*cur);
    if (u0 < 0xF0)
    {
        codepoint = ((u0 & 0x0F) << 12) | ((u1 & 0x3F) << 6) | (u2 & 0x3F);
        if ((codepoint > 0xD7FF) && (codepoint < 0xE000))
        {
            return -1;
        }
        if (codepoint < 0x800)
        {
            return -1;
        }
        return 0;
    }

    if (++cur == end)
    {
        return -1;
    }

    uint8_t u3 = static_cast <uint8_t> (*cur);
    if (u0 < 0xF8)
    {
        codepoint = ((u0 & 0x07) << 18) | ((u1 & 0x3F) << 12) | ((u2 & 0x3F) << 6) | (u3 & 0x3F);
        if (codepoint < 0x10000)
        {
            return -1;
        }
        return 0;
    }

    return -1;
}

// =========================================================================
//   CLASS     : JsonWriter
//   METHOD    : writeEscaped
// =========================================================================
int JsonWriter::writeEscaped (const std::string& value)
{
    auto cur = value.cbegin ();
    auto end = value.cend ();

    while (cur != end)
    {
        uint8_t ch = static_cast <uint8_t> (*cur);

        if (ch == '\"')
        {
            append ("\\\"", 2);
        }
        else if (ch == '\\')
        {
            append ("\\\\", 2);
        }
        else if (ch == '\b')
        {
            append ("\\b", 2);
        }
        else if (ch == '\f')
        {
            append ("\\f", 2);
        }
        else if (ch == '\n')
        {
            append ("\\n", 2);
        }
        else if (ch == '\r')
        {
            append ("\\r", 2);
        }
        else if (ch == '\t')
        {
            append ("\\t", 2);
        }
        else if ((ch < 0x20) /*|| (ch > 0x7F)*/)
        {
            uint32_t codepoint = 0;
            char hex[5];

            if (utf8Codepoint (cur, end, codepoint) == -1)
            {
                join::lastError = make_error_code (JsonErrc::InvalidEncoding);
                return -1;
            }

            if (codepoint <= 0xFFFF)
            {
                append ("\\u", 2);
                snprintf (hex, sizeof (hex), "%04x", uint16_t (codepoint));
                append (hex, 4);
            }
            else
            {
                codepoint -= 0x10000;
                append ("\\u", 2);
                snprintf (hex, sizeof (hex), "%04x", uint16_t (0xD800 + ((codepoint >> 10) & 0x3FF)));
                append (hex, 4);
                append ("\\u", 2);
                snprintf (hex, sizeof (hex), "%04x", uint16_t (0xDC00 + (codepoint & 0x3FF)));
                append (hex, 4);
            }
        }
        else
        {
            append (*cur);
        }

        ++cur;
    }

    return 0;
}

// =========================================================================
//   CLASS     : JsonWriter
//   METHOD    : comma
// =========================================================================
void JsonWriter::comma ()
{
    if (!first_)
    {
        append (',');
    }
}

// =========================================================================
//   CLASS     : JsonWriter
//   METHOD    : indent
// =========================================================================
void JsonWriter::indent ()
{
    if (indentation_)
    {
        append (tab_.c_str (), tab_.size ());
    }
}

// =========================================================================
//   CLASS     : JsonWriter
//   METHOD    : space
// =========================================================================
void JsonWriter::space ()
{
    if (indentation_)
    {
        append (' ');
    }
}

// =========================================================================
//   CLASS     : JsonWriter
//   METHOD    : endLine
// =========================================================================
void JsonWriter::endLine ()
{
    if (indentation_)
    {
        append ('\n');
    }
}

// =========================================================================
//   CLASS     : JsonWriter
//   METHOD    : array
// =========================================================================
void JsonWriter::array ()
{
    comma ();
    if (!stack_.empty () && stack_.top ())
    {
        endLine ();
        indent ();
    }
}
