/**
 * MIT License
 *
 * Copyright (c) 2022 Mathieu Rabine
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
#include <join/json.hpp>

using join::JsonErrc;
using join::JsonCategory;

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
        case JsonErrc::EndOfFile:
            return "end of file";
        default:
            return "success";
    }
}

// =========================================================================
//   CLASS     :
//   METHOD    : jsonCategory
// =========================================================================
const std::error_category& join::jsonCategory () noexcept
{
    static JsonCategory instance;
    return instance;
}

// =========================================================================
//   CLASS     :
//   METHOD    : make_error_code
// =========================================================================
std::error_code join::make_error_code (JsonErrc code) noexcept
{
    return std::error_code (static_cast <int> (code), jsonCategory ());
}

// =========================================================================
//   CLASS     :
//   METHOD    : make_error_condition
// =========================================================================
std::error_condition join::make_error_condition (JsonErrc code) noexcept
{
    return std::error_condition (static_cast <int> (code), jsonCategory ());
}
