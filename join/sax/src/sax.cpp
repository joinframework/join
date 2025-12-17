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
#include <join/sax.hpp>

using join::SaxErrc;
using join::SaxCategory;

// =========================================================================
//   CLASS     : SaxCategory
//   METHOD    : name
// =========================================================================
const char* SaxCategory::name () const noexcept
{
    return "libjoin";
}

// =========================================================================
//   CLASS     : SaxCategory
//   METHOD    : message
// =========================================================================
std::string SaxCategory::message (int code) const
{
    switch (static_cast <SaxErrc> (code))
    {
        case SaxErrc::StackOverflow:
            return "stack overflow";
        case SaxErrc::InvalidParent:
            return "parent not an array nor an object";
        case SaxErrc::InvalidValue:
            return "value is invalid";
        case SaxErrc::ExtraData:
            return "extra data detected";
        default:
            return "success";
    }
}

// =========================================================================
//   CLASS     :
//   METHOD    : saxCategory
// =========================================================================
const std::error_category& join::saxCategory () noexcept
{
    static SaxCategory instance;
    return instance;
}

// =========================================================================
//   CLASS     :
//   METHOD    : make_error_code
// =========================================================================
std::error_code join::make_error_code (SaxErrc code) noexcept
{
    return std::error_code (static_cast <int> (code), saxCategory ());
}

// =========================================================================
//   CLASS     :
//   METHOD    : make_error_condition
// =========================================================================
std::error_condition join::make_error_condition (SaxErrc code) noexcept
{
    return std::error_condition (static_cast <int> (code), saxCategory ());
}
