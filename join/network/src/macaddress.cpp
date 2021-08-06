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
#include <join/macaddress.hpp>

// C++.
#include <algorithm>
#include <utility>
#include <iomanip>
#include <sstream>
#include <random>

// C.
#include <linux/if_arp.h>
#include <cstring>
#include <cstdio>

using join::net::IpAddress;
using join::net::MacAddress;

/// Wildcard MAC address.
const MacAddress MacAddress::wildcard = "00:00:00:00:00:00";

/// Broadcast MAC address.
const MacAddress MacAddress::broadcast = "ff:ff:ff:ff:ff:ff";

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : MacAddress
// =========================================================================
MacAddress::MacAddress ()
{
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : MacAddress
// =========================================================================
MacAddress::MacAddress (const MacAddress& address)
: mac_ (address.mac_)
{
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : MacAddress
// =========================================================================
MacAddress::MacAddress (MacAddress&& address)
: mac_ (std::move (address.mac_))
{
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : MacAddress
// =========================================================================
MacAddress::MacAddress (const uint8_t* address, size_t size)
{
    if (size > mac_.size ())
    {
        throw std::invalid_argument ("out of range");
    }

    memcpy (&mac_[0], address, size);
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : MacAddress
// =========================================================================
MacAddress::MacAddress (const uint8_t (&address) [IFHWADDRLEN])
{
    std::copy (std::begin (address), std::end (address), std::begin (mac_));
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : MacAddress
// =========================================================================
MacAddress::MacAddress (std::initializer_list <uint8_t> address)
{
    if (address.size () > mac_.size ())
    {
        throw std::invalid_argument ("out of range");
    }

    std::copy (address.begin (), address.end (), std::begin (mac_));
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : MacAddress
// =========================================================================
MacAddress::MacAddress (const char* address)
{
    if (sscanf (address, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                &mac_[0], &mac_[1], &mac_[2],
                &mac_[3], &mac_[4], &mac_[5]) !=
                static_cast <int> (mac_.size ()))
    {
        throw std::invalid_argument ("invalid MAC address");
    }
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : MacAddress
// =========================================================================
MacAddress::MacAddress (const std::string& address)
: MacAddress (address.c_str ())
{
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : MacAddress
// =========================================================================
MacAddress::MacAddress (const struct sockaddr& address)
{
    if (address.sa_family == ARPHRD_ETHER)
    {
        memcpy (mac_.data (), address.sa_data, mac_.size ());
        return;
    }

    throw std::invalid_argument ("invalid MAC address");
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : family
// =========================================================================
int MacAddress::family () const
{
    return ARPHRD_ETHER;
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : addr
// =========================================================================
const uint8_t* MacAddress::addr () const
{
    return mac_.data ();
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : length
// =========================================================================
socklen_t MacAddress::length () const
{
    return mac_.size ();
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : isWildcard
// =========================================================================
bool MacAddress::isWildcard () const
{
    return (mac_[0] == 0 && mac_[1] == 0 &&
            mac_[2] == 0 && mac_[3] == 0 &&
            mac_[4] == 0 && mac_[5] == 0);
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : isBroadcast
// =========================================================================
bool MacAddress::isBroadcast () const
{
    return (mac_[0] == 0xff && mac_[1] == 0xff &&
            mac_[2] == 0xff && mac_[3] == 0xff &&
            mac_[4] == 0xff && mac_[5] == 0xff);
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : isMacAddress
// =========================================================================
bool MacAddress::isMacAddress (const std::string& address)
{
    try
    {
        MacAddress addr (address);
    }
    catch (const std::exception& ex)
    {
        return false;
    }

    return true;
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : toString
// =========================================================================
std::string MacAddress::toString (CaseConvert caseConvert) const
{
    std::stringstream oss;

    for (unsigned int i = 0u; i < (mac_.size () - 1); ++i)
    {
        oss << std::hex << caseConvert << std::setw (2);
        oss << std::setfill ('0') << static_cast <uint32_t> (mac_[i]) << ':';
    }
    oss << std::hex << caseConvert << std::setw (2);
    oss << std::setfill ('0') << static_cast <uint32_t> (mac_[mac_.size () - 1]);

    return oss.str ();
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : toIpv6
// =========================================================================
IpAddress MacAddress::toIpv6 (const IpAddress& prefix, int len) const
{
    IpAddress address = prefix & IpAddress (len, AF_INET6);

    address[8]  = mac_[0] ^ (1 << 1);
    address[9]  = mac_[1];
    address[10] = mac_[2];
    address[11] = 0xff;
    address[12] = 0xfe;
    address[13] = mac_[3];
    address[14] = mac_[4];
    address[15] = mac_[5];

    return address;
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : toLinkLocalIpv6
// =========================================================================
IpAddress MacAddress::toLinkLocalIpv6 () const
{
    IpAddress prefix (AF_INET6);

    prefix[0] = 0xfe;
    prefix[1] = 0x80;

    return toIpv6 (prefix, 10);
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : toUniqueLocalIpv6
// =========================================================================
IpAddress MacAddress::toUniqueLocalIpv6 () const
{
    IpAddress prefix (AF_INET6);
    std::uniform_int_distribution <int> dist (0, 255);
    std::random_device rnd;

    prefix[0] = 0xfd;
    prefix[1] = static_cast <uint8_t> (dist (rnd));
    prefix[2] = static_cast <uint8_t> (dist (rnd));
    prefix[3] = static_cast <uint8_t> (dist (rnd));
    prefix[4] = static_cast <uint8_t> (dist (rnd));
    prefix[5] = static_cast <uint8_t> (dist (rnd));

    return toIpv6 (prefix, 48);
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : clear
// =========================================================================
void MacAddress::clear ()
{
    memset (mac_.data (), 0, mac_.size ());
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : begin
// =========================================================================
MacAddress::iterator MacAddress::begin ()
{
    return mac_.begin ();
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : begin
// =========================================================================
MacAddress::const_iterator MacAddress::begin () const
{
    return mac_.begin ();
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : cbegin
// =========================================================================
MacAddress::const_iterator MacAddress::cbegin () const
{
    return mac_.cbegin ();
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : end
// =========================================================================
MacAddress::iterator MacAddress::end ()
{
    return mac_.end ();
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : end
// =========================================================================
MacAddress::const_iterator MacAddress::end () const
{
    return mac_.end ();
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : cend
// =========================================================================
MacAddress::const_iterator MacAddress::cend () const
{
    return mac_.cend ();
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : operator=
// =========================================================================
MacAddress& MacAddress::operator= (const MacAddress& address)
{
    mac_ = address.mac_;
    return *this;
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : operator=
// =========================================================================
MacAddress& MacAddress::operator= (MacAddress&& address)
{
    mac_ = std::move (address.mac_);
    return *this;
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : operator=
// =========================================================================
MacAddress& MacAddress::operator= (std::initializer_list <uint8_t> address)
{
    if (address.size () <= mac_.size ())
    {
        mac_.fill (0);
        std::copy (address.begin (), address.end (), std::begin (mac_));
        return *this;
    }

    throw std::invalid_argument ("invalid MAC address");
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : operator+=
// =========================================================================
MacAddress& MacAddress::operator= (const struct sockaddr& address)
{
    if (address.sa_family == ARPHRD_ETHER)
    {
        memcpy (mac_.data (), address.sa_data, mac_.size ());
        return *this;
    }

    throw std::invalid_argument ("invalid MAC address");
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : operator+=
// =========================================================================
MacAddress& MacAddress::operator+= (int value)
{
    for (int pos = mac_.size () - 1; pos >= 0 && value != 0; --pos)
    {
        int val = mac_[pos] + value;
        mac_[pos] = val % 256;
        value = val / 256;
    }

    return *this;
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : operator++
// =========================================================================
MacAddress& MacAddress::operator++ ()
{
    return operator+= (1);
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : operator++
// =========================================================================
MacAddress MacAddress::operator++ (int)
{
    MacAddress tmp (*this);
    operator+= (1);
    return tmp;
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : operator[]
// =========================================================================
uint8_t& MacAddress::operator[] (size_t position)
{
    if (position > mac_.size () - 1)
    {
        throw std::invalid_argument ("position is out of range");
    }

    return mac_[position];
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : operator[]
// =========================================================================
const uint8_t& MacAddress::operator[] (size_t position) const
{
    if (position > mac_.size () - 1)
    {
        throw std::invalid_argument ("position is out of range");
    }

    return mac_[position];
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : operator~
// =========================================================================
MacAddress MacAddress::operator~ () const
{
    MacAddress addr (*this);
    addr.mac_[0] ^= 0xff;
    addr.mac_[1] ^= 0xff;
    addr.mac_[2] ^= 0xff;
    addr.mac_[3] ^= 0xff;
    addr.mac_[4] ^= 0xff;
    addr.mac_[5] ^= 0xff;
    return addr;
}

// =========================================================================
//   CLASS     :
//   METHOD    : operator+
// =========================================================================
MacAddress join::net::operator+ (int value, const MacAddress& a)
{
    MacAddress addr (a);
    addr += (value);
    return addr;
}

// =========================================================================
//   CLASS     :
//   METHOD    : operator+
// =========================================================================
MacAddress join::net::operator+ (const MacAddress& a, int value)
{
    return operator+ (value, a);
}

// =========================================================================
//   CLASS     :
//   METHOD    : operator==
// =========================================================================
bool join::net::operator== (const MacAddress& a, const MacAddress& b)
{
    return memcmp (a.addr (), b.addr (), a.length ()) == 0;
}

// =========================================================================
//   CLASS     :
//   METHOD    : operator!=
// =========================================================================
bool join::net::operator!= (const MacAddress& a, const MacAddress& b)
{
    return !(a == b);
}

// =========================================================================
//   CLASS     :
//   METHOD    : operator<
// =========================================================================
bool join::net::operator< (const MacAddress& a, const MacAddress& b)
{
    return memcmp (a.addr (), b.addr (), a.length ()) < 0;
}

// =========================================================================
//   CLASS     :
//   METHOD    : operator<=
// =========================================================================
bool join::net::operator<= (const MacAddress& a, const MacAddress& b)
{
    return !(b < a);
}

// =========================================================================
//   CLASS     :
//   METHOD    : operator>
// =========================================================================
bool join::net::operator> (const MacAddress& a, const MacAddress& b)
{
    return b < a;
}

// =========================================================================
//   CLASS     :
//   METHOD    : operator>=
// =========================================================================
bool join::net::operator>= (const MacAddress& a, const MacAddress& b)
{
    return !(a < b);
}

// =========================================================================
//   CLASS     :
//   METHOD    : operator&
// =========================================================================
MacAddress join::net::operator& (const MacAddress& a, const MacAddress& b)
{
    MacAddress addr (a);
    addr[0] &= b[0];
    addr[1] &= b[1];
    addr[2] &= b[2];
    addr[3] &= b[3];
    addr[4] &= b[4];
    addr[5] &= b[5];
    return addr;
}

// =========================================================================
//   CLASS     :
//   METHOD    : operator|
// =========================================================================
MacAddress join::net::operator| (const MacAddress& a, const MacAddress& b)
{
    MacAddress addr (a);
    addr[0] |= b[0];
    addr[1] |= b[1];
    addr[2] |= b[2];
    addr[3] |= b[3];
    addr[4] |= b[4];
    addr[5] |= b[5];
    return addr;
}

// =========================================================================
//   CLASS     :
//   METHOD    : operator^
// =========================================================================
MacAddress join::net::operator^ (const MacAddress& a, const MacAddress& b)
{
    MacAddress addr (a);
    addr[0] ^= b[0];
    addr[1] ^= b[1];
    addr[2] ^= b[2];
    addr[3] ^= b[3];
    addr[4] ^= b[4];
    addr[5] ^= b[5];
    return addr;
}

// =========================================================================
//   CLASS     :
//   METHOD    : operator<<
// =========================================================================
std::ostream& join::net::operator<< (std::ostream& out, const MacAddress& a)
{
    out << a.toString ();
    return out;
}