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

#include <join/value.hpp>
#include <join/json.hpp>
#include <join/pack.hpp>

using join::Value;

using join::JsonReader;
using join::JsonWriter;
using join::JsonCanonicalizer;

using join::PackReader;
using join::PackWriter;

// =========================================================================
//   CLASS     : Value
//   METHOD    : jsonRead
// =========================================================================
int Value::jsonRead (const char* document, size_t length)
{
    JsonReader reader (*this);
    return reader.deserialize (document, length);
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : jsonRead
// =========================================================================
int Value::jsonRead (const char* first, const char* last)
{
    JsonReader reader (*this);
    return reader.deserialize (first, last);
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : jsonRead
// =========================================================================
int Value::jsonRead (const std::string& document)
{
    JsonReader reader (*this);
    return reader.deserialize (document);
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : jsonRead
// =========================================================================
int Value::jsonRead (std::istream& document)
{
    JsonReader reader (*this);
    return reader.deserialize (document);
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : jsonWrite
// =========================================================================
int Value::jsonWrite (std::ostream& document, size_t indentation)
{
    JsonWriter writer (document, indentation);
    return writer.serialize (*this);
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : jsonCanonicalize
// =========================================================================
int Value::jsonCanonicalize (std::ostream& document)
{
    JsonCanonicalizer writer (document);
    return writer.serialize (*this);
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : packRead
// =========================================================================
int Value::packRead (const char* document, size_t length)
{
    PackReader reader (*this);
    return reader.deserialize (document, length);
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : packRead
// =========================================================================
int Value::packRead (const char* first, const char* last)
{
    PackReader reader (*this);
    return reader.deserialize (first, last);
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : packRead
// =========================================================================
int Value::packRead (const std::string& document)
{
    PackReader reader (*this);
    return reader.deserialize (document);
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : packRead
// =========================================================================
int Value::packRead (std::istream& document)
{
    PackReader reader (*this);
    return reader.deserialize (document);
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : packWrite
// =========================================================================
int Value::packWrite (std::ostream& document)
{
    PackWriter writer (document);
    return writer.serialize (*this);
}
