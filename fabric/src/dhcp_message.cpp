/**
 * MIT License
 *
 * Copyright (c) 2026 Mathieu Rabine
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
#include <join/dhcp_message.hpp>

using join::IpAddress;
using join::MacAddress;
using join::ByteList;
using join::WordList;
using join::IpList;
using join::DhcpOption;
using join::DhcpMessage;

// =========================================================================
//   CLASS     : DhcpMessage
//   METHOD    : writeHead
// =========================================================================
bool DhcpMessage::writeHead (char*& cur, const char* end, uint8_t code, size_t size)
{
    if (size > maxOptionSize)
    {
        lastError = make_error_code (Errc::MessageTooLong);
        return false;
    }

    uint8_t head[2] = {code, static_cast<uint8_t> (size)};

    return writeBytes (cur, end, head, sizeof (head));
}

// =========================================================================
//   CLASS     : DhcpMessage
//   METHOD    : serialize
// =========================================================================
int DhcpMessage::serialize (const DhcpOption& options, char*& cur, const char* end) const
{
    bool ok = true;

    for (auto const& option : options)
    {
        const DhcpOption::Info* info = DhcpOption::describe (option.first);
        if (info == nullptr)
        {
            continue;  // LCOV_EXCL_LINE
        }

        switch (info->type)
        {
            case DhcpOption::Byte:
                {
                    uint8_t value = option.second.get<uint8_t> ();
                    ok &= writeHead (cur, end, option.first, sizeof (value));
                    ok &= writeBytes (cur, end, &value, sizeof (value));
                    break;
                }

            case DhcpOption::Word:
                {
                    uint16_t value = htons (option.second.get<uint16_t> ());
                    ok &= writeHead (cur, end, option.first, sizeof (value));
                    ok &= writeBytes (cur, end, &value, sizeof (value));
                    break;
                }

            case DhcpOption::Long:
                {
                    uint32_t value = htonl (option.second.get<uint32_t> ());
                    ok &= writeHead (cur, end, option.first, sizeof (value));
                    ok &= writeBytes (cur, end, &value, sizeof (value));
                    break;
                }

            case DhcpOption::SLong:
                {
                    uint32_t value = htonl (static_cast<uint32_t> (option.second.get<int32_t> ()));
                    ok &= writeHead (cur, end, option.first, sizeof (value));
                    ok &= writeBytes (cur, end, &value, sizeof (value));
                    break;
                }

            case DhcpOption::Text:
                {
                    const std::string& value = option.second.get<std::string> ();
                    ok &= writeHead (cur, end, option.first, value.size ());
                    ok &= writeBytes (cur, end, value.data (), value.size ());
                    break;
                }

            case DhcpOption::Ip:
                {
                    const IpAddress& value = option.second.get<IpAddress> ();
                    ok &= writeHead (cur, end, option.first, IpAddress::ipv4Length);
                    ok &= writeBytes (cur, end, value.addr (), IpAddress::ipv4Length);
                    break;
                }

            case DhcpOption::Mac:
                {
                    const MacAddress& value = option.second.get<MacAddress> ();
                    const uint8_t htype = _ethernet;
                    ok &= writeHead (cur, end, option.first, sizeof (_ethernet) + ETH_ALEN);
                    ok &= writeBytes (cur, end, &htype, sizeof (htype));
                    ok &= writeBytes (cur, end, value.addr (), ETH_ALEN);
                    break;
                }

            case DhcpOption::Bytes:
                {
                    const ByteList& value = option.second.get<ByteList> ();
                    ok &= writeHead (cur, end, option.first, value.size ());
                    ok &= writeBytes (cur, end, value.data (), value.size ());
                    break;
                }

            case DhcpOption::Words:
                {
                    const WordList& value = option.second.get<WordList> ();
                    ok &= writeHead (cur, end, option.first, value.size () * sizeof (uint16_t));
                    for (auto const& element : value)
                    {
                        uint16_t word = htons (element);
                        ok &= writeBytes (cur, end, &word, sizeof (word));
                    }
                    break;
                }

            case DhcpOption::Ips:
                {
                    const IpList& value = option.second.get<IpList> ();
                    ok &= writeHead (cur, end, option.first, value.size () * IpAddress::ipv4Length);
                    for (auto const& element : value)
                    {
                        ok &= writeBytes (cur, end, element.addr (), IpAddress::ipv4Length);
                    }
                    break;
                }
        }
    }

    const uint8_t last = DhcpOption::End;
    ok &= writeBytes (cur, end, &last, sizeof (last));

    return ok ? 0 : -1;
}

// =========================================================================
//   CLASS     : DhcpMessage
//   METHOD    : deserialize
// =========================================================================
int DhcpMessage::deserialize (DhcpOption& options, const char* cur, const char* end) const
{
    for (;;)
    {
        if (cur == end)
        {
            return 0;
        }

        const uint8_t code = static_cast<uint8_t> (*cur++);

        if (code == DhcpOption::Pad)
        {
            continue;
        }

        if (code == DhcpOption::End)
        {
            return 0;
        }

        uint8_t size = 0;
        if (!readBytes (cur, end, &size, sizeof (size)))
        {
            return -1;
        }

        uint8_t payload[maxOptionSize];
        if (!readBytes (cur, end, payload, size))
        {
            return -1;
        }

        const DhcpOption::Info* info = DhcpOption::describe (code);
        if (info == nullptr)
        {
            continue;
        }

        switch (info->type)
        {
            case DhcpOption::Byte:
                if (size == sizeof (uint8_t))
                {
                    options.insert (code, payload[0]);
                }
                break;

            case DhcpOption::Word:
                if (size == sizeof (uint16_t))
                {
                    options.insert (code, static_cast<uint16_t> ((payload[0] << 8) | payload[1]));
                }
                break;

            case DhcpOption::Long:
            case DhcpOption::SLong:
                if (size == sizeof (uint32_t))
                {
                    uint32_t value = (static_cast<uint32_t> (payload[0]) << 24) |
                                     (static_cast<uint32_t> (payload[1]) << 16) |
                                     (static_cast<uint32_t> (payload[2]) << 8) | static_cast<uint32_t> (payload[3]);

                    if (info->type == DhcpOption::Long)
                    {
                        options.insert (code, value);
                    }
                    else
                    {
                        options.insert (code, static_cast<int32_t> (value));
                    }
                }
                break;

            case DhcpOption::Text:
                options.insert (code, std::string (payload, payload + size));
                break;

            case DhcpOption::Ip:
                if (size == IpAddress::ipv4Length)
                {
                    options.insert (code, IpAddress (payload, IpAddress::ipv4Length));
                }
                break;

            case DhcpOption::Mac:
                if ((size == sizeof (_ethernet) + ETH_ALEN) && (payload[0] == _ethernet))
                {
                    options.insert (code, MacAddress (payload + sizeof (_ethernet), ETH_ALEN));
                }
                break;

            case DhcpOption::Bytes:
                options.insert (code, ByteList (payload, payload + size));
                break;

            case DhcpOption::Words:
                if ((size % sizeof (uint16_t)) == 0)
                {
                    WordList words;
                    for (size_t i = 0; i < size; i += sizeof (uint16_t))
                    {
                        words.push_back (static_cast<uint16_t> ((payload[i] << 8) | payload[i + 1]));
                    }
                    options.insert (code, words);
                }
                break;

            case DhcpOption::Ips:
                if ((size % IpAddress::ipv4Length) == 0)
                {
                    IpList addresses;
                    for (size_t i = 0; i < size; i += IpAddress::ipv4Length)
                    {
                        addresses.push_back (IpAddress (payload + i, IpAddress::ipv4Length));
                    }
                    options.insert (code, addresses);
                }
                break;
        }
    }
}
