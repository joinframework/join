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
    return deserialize <JsonReader> (document, length);
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : jsonRead
// =========================================================================
int Value::jsonRead (const char* first, const char* last)
{
    return deserialize <JsonReader> (first, last);
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : jsonRead
// =========================================================================
int Value::jsonRead (const std::string& document)
{
    return deserialize <JsonReader> (document);
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : jsonRead
// =========================================================================
int Value::jsonRead (std::istringstream& document)
{
    return deserialize <JsonReader> (document);
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : jsonRead
// =========================================================================
int Value::jsonRead (std::ifstream& document)
{
    return deserialize <JsonReader> (document);
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : jsonRead
// =========================================================================
int Value::jsonRead (std::istream& document)
{
    return deserialize <JsonReader> (document);
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : jsonWrite
// =========================================================================
int Value::jsonWrite (std::ostream& document, size_t indentation) const
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
    return serialize <JsonCanonicalizer> (document);
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : packRead
// =========================================================================
int Value::packRead (const char* document, size_t length)
{
    return deserialize <PackReader> (document, length);
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : packRead
// =========================================================================
int Value::packRead (const char* first, const char* last)
{
    return deserialize <PackReader> (first, last);
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : packRead
// =========================================================================
int Value::packRead (const std::string& document)
{
    return deserialize <PackReader> (document);
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : packRead
// =========================================================================
int Value::packRead (std::istringstream& document)
{
    return deserialize <PackReader> (document);
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : packRead
// =========================================================================
int Value::packRead (std::ifstream& document)
{
    return deserialize <PackReader> (document);
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : packRead
// =========================================================================
int Value::packRead (std::istream& document)
{
    return deserialize <PackReader> (document);
}

// =========================================================================
//   CLASS     : Value
//   METHOD    : packWrite
// =========================================================================
int Value::packWrite (std::ostream& document) const
{
    return serialize <PackWriter> (document);
}
