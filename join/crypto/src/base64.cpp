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
#include <join/base64.hpp>

using join::crypto::Base64;
using join::crypto::BytesArray;

/// Base 64 encode/decode table.
const std::string Base64::_base64Table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// =========================================================================
//   CLASS     : Base64
//   METHOD    : encode
// =========================================================================
std::string Base64::encode (const std::string& message)
{
    return encode (reinterpret_cast <const uint8_t*> (message.data ()), message.size ());
}

// =========================================================================
//   CLASS     : Base64
//   METHOD    : encode
// =========================================================================
std::string Base64::encode (const BytesArray& data)
{
    return encode (data.data (), data.size ());
}

// =========================================================================
//   CLASS     : Base64
//   METHOD    : encode
// =========================================================================
std::string Base64::encode (const uint8_t* data, size_t size)
{
    const size_t trail = size % 3;
    size_t sz          = size / 3 * 4;
    sz += (trail != 0) ? 4 : 0;

    un32 b64;
    std::string out;
    out.resize (sz);
    size_t i = 0, k = 0;

    while (i < size - trail)
    {
        b64.c[3] = data[i++];
        b64.c[2] = data[i++];
        b64.c[1] = data[i++];
        out[k++] = _base64Table[static_cast <int32_t> ((b64.l & _mask1) >> 26)];
        out[k++] = _base64Table[static_cast <int32_t> ((b64.l & _mask2) >> 20)];
        out[k++] = _base64Table[static_cast <int32_t> ((b64.l & _mask3) >> 14)];
        out[k++] = _base64Table[static_cast <int32_t> ((b64.l & _mask4) >> 8)];
    }
    b64.l = 0;

    switch (trail)
    {
        case 1:
            b64.c[3] = data[i++];
            out[k++] = _base64Table[static_cast <int32_t> ((b64.l & _mask1) >> 26)];
            out[k++] = _base64Table[static_cast <int32_t> ((b64.l & _mask2) >> 20)];
            out[k++] = _fillChar;
            out[k++] = _fillChar;
            break;
        case 2:
            b64.c[3] = data[i++];
            b64.c[2] = data[i++];
            out[k++] = _base64Table[static_cast <int32_t> ((b64.l & _mask1 ) >> 26)];
            out[k++] = _base64Table[static_cast <int32_t> ((b64.l & _mask2 ) >> 20)];
            out[k++] = _base64Table[static_cast <int32_t> ((b64.l & _mask3 ) >> 14)];
            out[k++] = _fillChar;
            break;
    }

    return out;
}

// =========================================================================
//   CLASS     : Base64
//   METHOD    : decode
// =========================================================================
BytesArray Base64::decode (const std::string& data)
{
    if (data.empty() || data.length () % 4 != 0)
    {
        return {};
    }
    // Number of trailing '='
    const size_t trail = (data[data.length () - 1] == _fillChar) ? ((data[data.length () - 2] == _fillChar) ? 2 : 1) : 0;
    // Number of char to decode
    const size_t szin  = (trail == 0) ? data.length () : data.length () - 4;
    // Output string size
    const size_t szout = szin / 4 * 3 + ((trail == 0) ? 0 : ((trail == 1) ? 2 : 1));

    un32 b64;
    BytesArray out;
    out.resize (szout);
    size_t i = 0, k = 0;

    while (i < szin)
    {
        b64.l  = 0;
        b64.l += (static_cast <uint32_t> (_base64Table.find (data[i++], 0))) << 26;
        b64.l += (static_cast <uint32_t> (_base64Table.find (data[i++], 0))) << 20;
        b64.l += (static_cast <uint32_t> (_base64Table.find (data[i++], 0))) << 14;
        b64.l += (static_cast <uint32_t> (_base64Table.find (data[i++], 0))) <<  8;
        out[k++] = b64.c[3];
        out[k++] = b64.c[2];
        out[k++] = b64.c[1];
    }
    b64.l = 0;

    switch (trail)
    {
        case 1:
            b64.l += (static_cast <uint32_t> (_base64Table.find (data[i++], 0))) << 26;
            b64.l += (static_cast <uint32_t> (_base64Table.find (data[i++], 0))) << 20;
            b64.l += (static_cast <uint32_t> (_base64Table.find (data[i++], 0))) << 14;
            out[k++] = b64.c[3];
            out[k++] = b64.c[2];
            break;
        case 2:
            b64.l += (static_cast <uint32_t> (_base64Table.find (data[i++], 0))) << 26;
            b64.l += (static_cast <uint32_t> (_base64Table.find (data[i++], 0))) << 20;
            out[k++] = b64.c[3];
            break;
    }

    return out;
}
