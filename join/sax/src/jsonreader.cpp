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
#include <join/jsonreader.hpp>

// C.
#include <cstdlib>

#define likely(x)   __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)

using join::sax::Value;
using join::sax::JsonReader;

// =========================================================================
//   CLASS     : JsonReader
//   METHOD    : JsonReader
// =========================================================================
JsonReader::JsonReader (Value& root)
: StreamReader (root)
{
}

// =========================================================================
//   CLASS     : JsonReader
//   METHOD    : read
// =========================================================================
int JsonReader::read (View& document)
{
    if (readValue (document) == 0)
    {
        skipWhitespace (document);

        if (document.peek () == std::char_traits <char>::eof ())
        {
            return 0;
        }

        join::lastError = make_error_code (SaxErrc::ExtraData);
    }

    return -1;
}

// =========================================================================
//   CLASS     : JsonReader
//   METHOD    : readValue
// =========================================================================
int JsonReader::readValue (View& document)
{
    if (skipComment (document) == 0)
    {
        switch (document.peek ())
        {
            case 'n':
                return readNull (document);
            case 't':
                return readTrue (document);
            case 'f':
                return readFalse (document);
            case '"':
                return readString (document);
            case '[':
                return readArray (document);
            case '{':
                return readObject (document);
            default:
                return readNumber (document);
        }
    }

    return -1;
}

// =========================================================================
//   CLASS     : JsonReader
//   METHOD    : readNull
// =========================================================================
int JsonReader::readNull (View& document)
{
    if (unlikely ((document.get () != 'n') || (document.get () != 'u') || (document.get () != 'l') || (document.get () != 'l')))
    {
        join::lastError = make_error_code (SaxErrc::InvalidValue);
        return -1;
    }

    return setNull ();
}

// =========================================================================
//   CLASS     : JsonReader
//   METHOD    : readTrue
// =========================================================================
int JsonReader::readTrue (View& document)
{
    if (unlikely ((document.get () != 't') || (document.get () != 'r') || (document.get () != 'u') || (document.get () != 'e')))
    {
        join::lastError = make_error_code (SaxErrc::InvalidValue);
        return -1;
    }

    return setBool (true);
}

// =========================================================================
//   CLASS     : JsonReader
//   METHOD    : readFalse
// =========================================================================
int JsonReader::readFalse (View& document)
{
    if (unlikely ((document.get () != 'f') || (document.get () != 'a') || (document.get () != 'l') || (document.get () != 's') || (document.get () != 'e')))
    {
        join::lastError = make_error_code (SaxErrc::InvalidValue);
        return -1;
    }

    return setBool (false);
}

// =========================================================================
//   CLASS     : JsonReader
//   METHOD    : pow10
// =========================================================================
const double& JsonReader::pow10 (size_t exponent)
{
    static const double result[] =
    {
        1e0,    1e+1,   1e+2,   1e+3,   1e+4,   1e+5,   1e+6,   1e+7,   1e+8,
        1e+9,   1e+10,  1e+11,  1e+12,  1e+13,  1e+14,  1e+15,  1e+16,  1e+17,
        1e+18,  1e+19,  1e+20,  1e+21,  1e+22,  1e+23,  1e+24,  1e+25,  1e+26,
        1e+27,  1e+28,  1e+29,  1e+30,  1e+31,  1e+32,  1e+33,  1e+34,  1e+35,
        1e+36,  1e+37,  1e+38,  1e+39,  1e+40,  1e+41,  1e+42,  1e+43,  1e+44,
        1e+45,  1e+46,  1e+47,  1e+48,  1e+49,  1e+50,  1e+51,  1e+52,  1e+53,
        1e+54,  1e+55,  1e+56,  1e+57,  1e+58,  1e+59,  1e+60,  1e+61,  1e+62,
        1e+63,  1e+64,  1e+65,  1e+66,  1e+67,  1e+68,  1e+69,  1e+70,  1e+71,
        1e+72,  1e+73,  1e+74,  1e+75,  1e+76,  1e+77,  1e+78,  1e+79,  1e+80,
        1e+81,  1e+82,  1e+83,  1e+84,  1e+85,  1e+86,  1e+87,  1e+88,  1e+89,
        1e+90,  1e+91,  1e+92,  1e+93,  1e+94,  1e+95,  1e+96,  1e+97,  1e+98,
        1e+99,  1e+100, 1e+101, 1e+102, 1e+103, 1e+104, 1e+105, 1e+106, 1e+107,
        1e+108, 1e+109, 1e+110, 1e+111, 1e+112, 1e+113, 1e+114, 1e+115, 1e+116,
        1e+117, 1e+118, 1e+119, 1e+120, 1e+121, 1e+122, 1e+123, 1e+124, 1e+125,
        1e+126, 1e+127, 1e+128, 1e+129, 1e+130, 1e+131, 1e+132, 1e+133, 1e+134,
        1e+135, 1e+136, 1e+137, 1e+138, 1e+139, 1e+140, 1e+141, 1e+142, 1e+143,
        1e+144, 1e+145, 1e+146, 1e+147, 1e+148, 1e+149, 1e+150, 1e+151, 1e+152,
        1e+153, 1e+154, 1e+155, 1e+156, 1e+157, 1e+158, 1e+159, 1e+160, 1e+161,
        1e+162, 1e+163, 1e+164, 1e+165, 1e+166, 1e+167, 1e+168, 1e+169, 1e+170,
        1e+171, 1e+172, 1e+173, 1e+174, 1e+175, 1e+176, 1e+177, 1e+178, 1e+179,
        1e+180, 1e+181, 1e+182, 1e+183, 1e+184, 1e+185, 1e+186, 1e+187, 1e+188,
        1e+189, 1e+190, 1e+191, 1e+192, 1e+193, 1e+194, 1e+195, 1e+196, 1e+197,
        1e+198, 1e+199, 1e+200, 1e+201, 1e+202, 1e+203, 1e+204, 1e+205, 1e+206,
        1e+207, 1e+208, 1e+209, 1e+210, 1e+211, 1e+212, 1e+213, 1e+214, 1e+215,
        1e+216, 1e+217, 1e+218, 1e+219, 1e+220, 1e+221, 1e+222, 1e+223, 1e+224,
        1e+225, 1e+226, 1e+227, 1e+228, 1e+229, 1e+230, 1e+231, 1e+232, 1e+233,
        1e+234, 1e+235, 1e+236, 1e+237, 1e+238, 1e+239, 1e+240, 1e+241, 1e+242,
        1e+243, 1e+244, 1e+245, 1e+246, 1e+247, 1e+248, 1e+249, 1e+250, 1e+251,
        1e+252, 1e+253, 1e+254, 1e+255, 1e+256, 1e+257, 1e+258, 1e+259, 1e+260,
        1e+261, 1e+262, 1e+263, 1e+264, 1e+265, 1e+266, 1e+267, 1e+268, 1e+269,
        1e+270, 1e+271, 1e+272, 1e+273, 1e+274, 1e+275, 1e+276, 1e+277, 1e+278,
        1e+279, 1e+280, 1e+281, 1e+282, 1e+283, 1e+284, 1e+285, 1e+286, 1e+287,
        1e+288, 1e+289, 1e+290, 1e+291, 1e+292, 1e+293, 1e+294, 1e+295, 1e+296,
        1e+297, 1e+298, 1e+299, 1e+300, 1e+301, 1e+302, 1e+303, 1e+304, 1e+305,
        1e+306, 1e+307, 1e+308
    };

    if (exponent > 308)
    {
        throw std::invalid_argument ("exponent is invalid");
    }

    return result [exponent];
}

// =========================================================================
//   CLASS     : JsonReader
//   METHOD    : readNumber
// =========================================================================
int JsonReader::readNumber (View& document)
{
    auto beg = document.data ();
    bool negative = false;

    if (isSign (document.peek ()))
    {
        negative = (document.get () == '-');
    }

    uint64_t max64 = std::numeric_limits <uint64_t>::max ();
    if (negative)
    {
        max64 = static_cast <uint64_t> (std::numeric_limits <int64_t>::max ()) + 1;
    }

    auto digitsStart = document.data ();
    bool isDouble = false;
    uint64_t u = 0;

    if (unlikely (document.getIf ('0')))
    {
        if (unlikely (isDigit (document.peek ())))
        {
            join::lastError = make_error_code (SaxErrc::InvalidValue);
            return -1;
        }

        digitsStart++;
    }
    else if (likely (isDigit (document.peek ())))
    {
        u = document.get () - '0';

        while (likely (isDigit (document.peek ())))
        {
            int digit = document.peek () - '0';

            if (unlikely (u > ((max64 - digit) / 10)))
            {
                isDouble = true;
                break;
            }

            u = (u * 10) + digit;
            document.get ();
        }
    }
    else if (likely (document.getIf ('I') && document.getIf ('n') && document.getIf ('f')))
    {
        if (unlikely (document.getIf ('i') && !((document.get () == 'n') && (document.get () == 'i') && (document.get () == 't') && (document.get () == 'y'))))
        {
            join::lastError = make_error_code (SaxErrc::InvalidValue);
            return -1;
        }

        return setDouble (negative ? -std::numeric_limits <double>::infinity () : std::numeric_limits <double>::infinity ());
    }
    else if (likely (document.getIf ('N') && (document.get () == 'a') && (document.get () == 'N')))
    {
        return setDouble (negative ? -std::numeric_limits <double>::quiet_NaN () : std::numeric_limits <double>::quiet_NaN ());
    }
    else
    {
        join::lastError = make_error_code (SaxErrc::InvalidValue);
        return -1;
    }

    if (isDouble)
    {
        while (likely (isDigit (document.peek ())))
        {
            u = (u * 10) + (document.get () - '0');
        }
    }

    int64_t exponent = 0;
    int digitsOffset = 0;

    if (document.getIf ('.'))
    {
        auto exponentStart = document.data ();
        digitsOffset = 1;
        isDouble = true;

        while (likely (isDigit (document.peek ())))
        {
            u = (u * 10) + (document.get () - '0');

            if (unlikely (u == 0))
            {
                digitsStart++;
            }
        }

        exponent = exponentStart - document.data ();
    }

    int digits = document.data () - digitsStart - digitsOffset;

    if (document.getIf ('e') || document.getIf ('E'))
    {
        bool negativeExp = false;
        isDouble = true;

        if (isSign (document.peek ()))
        {
            negativeExp = (document.get () == '-');
        }

        int exp = 0;

        if (likely (isDigit (document.peek ())))
        {
            exp = (document.get () - '0');

            while (likely (isDigit (document.peek ())))
            {
                int digit = document.peek () - '0';

                if (likely (exp <= ((std::numeric_limits <int>::max () - digit) / 10)))
                {
                    exp = (exp * 10) + digit;
                }

                document.get ();
            }
        }
        else
        {
            join::lastError = make_error_code (SaxErrc::InvalidValue);
            return -1;
        }

        exponent += negativeExp ? -exp : exp;
    }

    if (isDouble)
    {
        if (unlikely ((digits > std::numeric_limits <double>::max_digits10) || (exponent < -308) || (exponent > 308)))
        {
            char* end = nullptr;
            static locale_t locale = newlocale (LC_ALL_MASK, "C", nullptr);
            double d = strtod_l (beg, &end, locale);
            if (end != document.data ())
            {
                join::lastError = make_error_code (SaxErrc::InvalidValue);
                return -1;
            }
            return setDouble (d);
        }

        double d = static_cast <double> (u);

        if (likely (d != 0.0))
        {
            if (exponent >= 0)
            {
                d *= pow10 (exponent);
            }
            else
            {
                d /= pow10 (-exponent);
            }
        }

        return setDouble (negative ? -d : d);
    }

    if (negative)
    {
        return setInt64 (-u);
    }

    return setUint64 (u);
}

// =========================================================================
//   CLASS     : JsonReader
//   METHOD    : readHex
// =========================================================================
int JsonReader::readHex (View& document, uint32_t& u)
{
    for (int i = 0; i < 4; ++i)
    {
        char c = document.get ();

        if (isDigit (c))
        {
            c -= '0';
        }
        else if (isUpperAlpha (c))
        {
            c = c - 'A' + 10;
        }
        else if (isLowerAlpha (c))
        {
            c = c - 'a' + 10;
        }
        else
        {
            join::lastError = make_error_code (JsonErrc::InvalidEncoding);
            return -1;
        }

        u = (u << 4) + c;
    }

    return 0;
}

// =========================================================================
//   CLASS     : JsonReader
//   METHOD    : encodeUtf8
// =========================================================================
void JsonReader::encodeUtf8 (uint32_t codepoint, std::string& output)
{
    if (codepoint < 0x80)
    {
        output.push_back (static_cast <char> (codepoint));
    }
    else if (codepoint < 0x800)
    {
        output.push_back (static_cast <char> (0xC0 | (codepoint >> 6)));
        output.push_back (static_cast <char> (0x80 | (codepoint & 0x3F)));
    }
    else if (codepoint < 0x010000)
    {
        output.push_back (static_cast <char> (0xE0 | (codepoint >> 12)));
        output.push_back (static_cast <char> (0x80 | ((codepoint >> 6) & 0x3F)));
        output.push_back (static_cast <char> (0x80 | (codepoint & 0x3F)));
    }
    else
    {
        output.push_back (static_cast <char> (0xF0 | (codepoint >> 18)));
        output.push_back (static_cast <char> (0x80 | ((codepoint >> 12) & 0x3F)));
        output.push_back (static_cast <char> (0x80 | ((codepoint >> 6) & 0x3F)));
        output.push_back (static_cast <char> (0x80 | (codepoint & 0x3F)));
    }
}

// =========================================================================
//   CLASS     : JsonReader
//   METHOD    : readUnicode
// =========================================================================
int JsonReader::readUnicode (View& document, std::string& output)
{
    uint32_t u = 0;

    if (readHex (document, u) == -1)
    {
        return -1;
    }

    if (u >= 0xD800 && u <= 0xDBFF)
    {
        if ((document.get () != '\\') || (document.get () != 'u'))
        {
            join::lastError = make_error_code (JsonErrc::InvalidEncoding);
            return -1;
        }

        uint32_t v = 0;

        if (readHex (document, v) == -1)
        {
            join::lastError = make_error_code (JsonErrc::InvalidEncoding);
            return -1;
        }

        if (v < 0xDC00 || v > 0xDFFF)
        {
            join::lastError = make_error_code (JsonErrc::InvalidEncoding);
            return -1;
        }

        u = 0x10000 + (((u - 0xD800) << 10) | (v - 0xDC00));
    }

    encodeUtf8 (u, output);

    return 0;
}

// =========================================================================
//   CLASS     : JsonReader
//   METHOD    : readEscaped
// =========================================================================
int JsonReader::readEscaped (View& document, std::string& output)
{
    if (document.getIf ('\\'))
    {
        if (document.getIf ('"'))
        {
            output.push_back ('"');
        }
        else if (document.getIf ('\\'))
        {
            output.push_back ('\\');
        }
        else if (document.getIf ('b'))
        {
            output.push_back ('\b');
        }
        else if (document.getIf ('f'))
        {
            output.push_back ('\f');
        }
        else if (document.getIf ('n'))
        {
            output.push_back ('\n');
        }
        else if (document.getIf ('r'))
        {
            output.push_back ('\r');
        }
        else if (document.getIf ('t'))
        {
            output.push_back ('\t');
        }
        else if (document.getIf ('/'))
        {
            output.push_back ('/');
        }
        else if (document.getIf ('u'))
        {
            if (readUnicode (document, output) == -1)
            {
                return -1;
            }
        }
        else
        {
            join::lastError = make_error_code (JsonErrc::InvalidEscaping);
            return -1;
        }
    }
    else
    {
        join::lastError = make_error_code (JsonErrc::InvalidEscaping);
        return -1;
    }

    return 0;
}

// =========================================================================
//   CLASS     : JsonReader
//   METHOD    : readUtf8
// =========================================================================
int JsonReader::readUtf8 (View& document, std::string& output)
{
    size_t count = 0;

    if (static_cast <uint8_t> (document.peek ()) < 0x80)
    {
        output.push_back (document.get ());
    }
    else if (static_cast <uint8_t> (document.peek ()) < 0xE0)
    {
        output.push_back (document.get ());
        count = 1;
    }
    else if (static_cast <uint8_t> (document.peek ()) < 0xF0)
    {
        output.push_back (document.get ());
        count = 2;
    }
    else if (static_cast <uint8_t> (document.peek ()) < 0xF8)
    {
        output.push_back (document.get ());
        count = 3;
    }
    else
    {
        join::lastError = make_error_code (JsonErrc::InvalidEncoding);
        return -1;
    }

    for (size_t i = 0; i < count; ++i)
    {
        if (static_cast <uint8_t> (document.peek ()) < 0x80 || static_cast <uint8_t> (document.peek ()) > 0xBF)
        {
            join::lastError = make_error_code (JsonErrc::InvalidEncoding);
            return -1;
        }

        output.push_back (document.get ());
    }

    return 0;
}

// =========================================================================
//   CLASS     : JsonReader
//   METHOD    : readStringSlow
// =========================================================================
int JsonReader::readStringSlow (View& document, bool isKey, std::string& output)
{
    for (;;)
    {
        if (unlikely (document.getIf ('"')))
        {
            break;
        }
        else if (unlikely (static_cast <uint8_t> (document.peek ()) == '\\'))
        {
            if (readEscaped (document, output) == -1)
            {
                return -1;
            }
        }
        else if (unlikely (static_cast <uint8_t> (document.peek ()) < 0x20))
        {
            join::lastError = make_error_code (JsonErrc::IllegalCharacter);
            return -1;
        }
        //else if (unlikely (static_cast <uint8_t> (document.peek ()) > 0x7F))
        //{
        //    if (readUtf8 (document, output) == -1)
        //    {
        //        return -1;
        //    }
        //}
        else
        {
            output.push_back (document.get ());
        }
    }

    return isKey ? setKey (output) : setString (output);
}

// =========================================================================
//   CLASS     : JsonReader
//   METHOD    : readString
// =========================================================================
int JsonReader::readString (View& document, bool isKey)
{
    if (unlikely (!document.getIf ('"')))
    {
        join::lastError = make_error_code (JsonErrc::MissingQuote);
        return -1;
    }

    auto start = document.data ();
    size_t len = document.size ();

    for (size_t pos = 0; pos < len; ++pos)
    {
        if (!isPlainText (static_cast <uint8_t> (document[pos])))
        {
            std::string output (start, start + pos);
            document.removePrefix (pos);

            if (document.getIf ('"'))
            {
                return isKey ? setKey (output) : setString (output);
            }

            return readStringSlow (document, isKey, output);
        }
    }

    join::lastError = make_error_code (JsonErrc::IllegalCharacter);
    return false;
}

// =========================================================================
//   CLASS     : JsonReader
//   METHOD    : readArray
// =========================================================================
int JsonReader::readArray (View& document)
{
    if (unlikely (!document.getIf ('[')))
    {
        join::lastError = make_error_code (JsonErrc::MissingSquareBracket);
        return -1;
    }

    if (startArray () == -1)
    {
        return -1;
    }

    if (skipComment (document) == -1)
    {
        return -1;
    }

    if (document.getIf (']'))
    {
        return stopArray ();
    }

    for (;;)
    {
        if (readValue (document) == -1)
        {
            return -1;
        }

        if (skipComment (document) == -1)
        {
            return -1;
        }

        if (document.getIf (','))
        {
            if (skipComment (document) == -1)
            {
                return -1;
            }

            continue;
        }
        else if (document.getIf (']'))
        {
            break;
        }
        else
        {
            join::lastError = make_error_code (JsonErrc::MissingComma);
            return -1;
        }
    }

    return stopArray ();
}

// =========================================================================
//   CLASS     : JsonReader
//   METHOD    : readObject
// =========================================================================
int JsonReader::readObject (View& document)
{
    if (unlikely (!document.getIf ('{')))
    {
        join::lastError = make_error_code (JsonErrc::MissingCurlyBracket);
        return -1;
    }

    if (startObject () == -1)
    {
        return -1;
    }

    if (skipComment (document) == -1)
    {
        return -1;
    }

    if (document.getIf ('}'))
    {
        return stopObject ();
    }

    for (;;)
    {
        if (document.peek () != '"')
        {
            join::lastError = make_error_code (JsonErrc::MissingQuote);
            return -1;
        }

        if (readString (document, true) == -1)
        {
            return -1;
        }

        if (skipComment (document) == -1)
        {
            return -1;
        }

        if (document.get () != ':')
        {
            join::lastError = make_error_code (JsonErrc::MissingColon);
            return -1;
        }

        if (readValue (document) == -1)
        {
            return -1;
        }

        if (skipComment (document) == -1)
        {
            return -1;
        }

        if (document.getIf (','))
        {
            if (skipComment (document) == -1)
            {
                return -1;
            }

            continue;
        }
        else if (document.getIf ('}'))
        {
            break;
        }
        else
        {
            join::lastError = make_error_code (JsonErrc::MissingComma);
            return -1;
        }
    }

    return stopObject ();
}

// =========================================================================
//   CLASS     : JsonReader
//   METHOD    : skipWhitespace
// =========================================================================
void JsonReader::skipWhitespace (View& document)
{
    char current;
    while ((current = document.peek ()) == ' ' || current == '\n' || current == '\r' || current == '\t')
    {
        document.get ();
    }
}

// =========================================================================
//   CLASS     : JsonReader
//   METHOD    : skipComment
// =========================================================================
int JsonReader::skipComment (View& document)
{
    skipWhitespace (document);

    while (document.getIf ('/'))
    {
        if (document.getIf ('*'))
        {
            while ((document.get () != '*') || (document.get () != '/'))
            {
                // ignore comment.
            }
        }
        else if (document.getIf ('/'))
        {
            while (document.get () != '\n')
            {
                // ignore comment.
            }
        }
        else
        {
            join::lastError = make_error_code (JsonErrc::InvalidComment);
            return -1;
        }

        skipWhitespace (document);
    }

    return 0;
}
