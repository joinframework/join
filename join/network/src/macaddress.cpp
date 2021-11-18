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
#include <join/error.hpp>

// C++.
#include <algorithm>
#include <utility>
#include <iomanip>
#include <sstream>
#include <random>

// C.
#include <linux/if_arp.h>
#include <sys/ioctl.h>
#include <cstring>
#include <cstdio>

using join::IpAddress;
using join::MacAddress;

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
: _mac (address._mac)
{
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : MacAddress
// =========================================================================
MacAddress::MacAddress (MacAddress&& address)
: _mac (std::move (address._mac))
{
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : MacAddress
// =========================================================================
MacAddress::MacAddress (const uint8_t* address, size_t size)
{
    if (size > _mac.size ())
    {
        throw std::out_of_range ("out of range");
    }

    memcpy (&_mac[0], address, size);
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : MacAddress
// =========================================================================
MacAddress::MacAddress (const uint8_t (&address) [IFHWADDRLEN])
{
    std::copy (std::begin (address), std::end (address), std::begin (_mac));
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : MacAddress
// =========================================================================
MacAddress::MacAddress (std::initializer_list <uint8_t> address)
{
    if (address.size () > _mac.size ())
    {
        throw std::out_of_range ("out of range");
    }

    std::copy (address.begin (), address.end (), std::begin (_mac));
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : MacAddress
// =========================================================================
MacAddress::MacAddress (const char* address)
{
    if (sscanf (address, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                &_mac[0], &_mac[1], &_mac[2],
                &_mac[3], &_mac[4], &_mac[5]) !=
                static_cast <int> (_mac.size ()))
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
        memcpy (_mac.data (), address.sa_data, _mac.size ());
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
    return _mac.data ();
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : length
// =========================================================================
socklen_t MacAddress::length () const
{
    return _mac.size ();
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : isWildcard
// =========================================================================
bool MacAddress::isWildcard () const
{
    return (_mac[0] == 0 && _mac[1] == 0 &&
            _mac[2] == 0 && _mac[3] == 0 &&
            _mac[4] == 0 && _mac[5] == 0);
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : isBroadcast
// =========================================================================
bool MacAddress::isBroadcast () const
{
    return (_mac[0] == 0xff && _mac[1] == 0xff &&
            _mac[2] == 0xff && _mac[3] == 0xff &&
            _mac[4] == 0xff && _mac[5] == 0xff);
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

    for (unsigned int i = 0u; i < (_mac.size () - 1); ++i)
    {
        oss << std::hex << caseConvert << std::setw (2);
        oss << std::setfill ('0') << static_cast <uint32_t> (_mac[i]) << ':';
    }
    oss << std::hex << caseConvert << std::setw (2);
    oss << std::setfill ('0') << static_cast <uint32_t> (_mac[_mac.size () - 1]);

    return oss.str ();
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : toIpv6
// =========================================================================
IpAddress MacAddress::toIpv6 (const IpAddress& prefix, int len) const
{
    IpAddress address = prefix & IpAddress (len, AF_INET6);

    address[8]  = _mac[0] ^ (1 << 1);
    address[9]  = _mac[1];
    address[10] = _mac[2];
    address[11] = 0xff;
    address[12] = 0xfe;
    address[13] = _mac[3];
    address[14] = _mac[4];
    address[15] = _mac[5];

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
    memset (_mac.data (), 0, _mac.size ());
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : begin
// =========================================================================
MacAddress::iterator MacAddress::begin ()
{
    return _mac.begin ();
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : begin
// =========================================================================
MacAddress::const_iterator MacAddress::begin () const
{
    return _mac.begin ();
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : cbegin
// =========================================================================
MacAddress::const_iterator MacAddress::cbegin () const
{
    return _mac.cbegin ();
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : end
// =========================================================================
MacAddress::iterator MacAddress::end ()
{
    return _mac.end ();
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : end
// =========================================================================
MacAddress::const_iterator MacAddress::end () const
{
    return _mac.end ();
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : cend
// =========================================================================
MacAddress::const_iterator MacAddress::cend () const
{
    return _mac.cend ();
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : address
// =========================================================================
MacAddress MacAddress::address (const std::string& interface)
{
    int fd = ::socket (AF_INET, SOCK_DGRAM, 0);
    if (fd == -1)
    {
        lastError = std::make_error_code (static_cast <std::errc> (errno));
        return {};
    }

    struct ifreq iface;
    ::memset (&iface, 0, sizeof (iface));
    ::strncpy (iface.ifr_name, interface.c_str (), IFNAMSIZ - 1);

    int result = ::ioctl (fd, SIOCGIFHWADDR, &iface);
    if (result == -1)
    {
        lastError = std::make_error_code (static_cast <std::errc> (errno));
        close (fd);
        return {};
    }
    close (fd);

    return MacAddress (reinterpret_cast <uint8_t*> (iface.ifr_addr.sa_data), ETH_ALEN);
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : operator=
// =========================================================================
MacAddress& MacAddress::operator= (const MacAddress& address)
{
    _mac = address._mac;
    return *this;
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : operator=
// =========================================================================
MacAddress& MacAddress::operator= (MacAddress&& address)
{
    _mac = std::move (address._mac);
    return *this;
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : operator=
// =========================================================================
MacAddress& MacAddress::operator= (std::initializer_list <uint8_t> address)
{
    if (address.size () <= _mac.size ())
    {
        _mac.fill (0);
        std::copy (address.begin (), address.end (), std::begin (_mac));
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
        memcpy (_mac.data (), address.sa_data, _mac.size ());
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
    for (int pos = _mac.size () - 1; pos >= 0 && value != 0; --pos)
    {
        int val = _mac[pos] + value;
        _mac[pos] = val % 256;
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
    if (position > _mac.size () - 1)
    {
        throw std::out_of_range ("position is out of range");
    }

    return _mac[position];
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : operator[]
// =========================================================================
const uint8_t& MacAddress::operator[] (size_t position) const
{
    if (position > _mac.size () - 1)
    {
        throw std::out_of_range ("position is out of range");
    }

    return _mac[position];
}

// =========================================================================
//   CLASS     : MacAddress
//   METHOD    : operator~
// =========================================================================
MacAddress MacAddress::operator~ () const
{
    MacAddress addr (*this);
    addr._mac[0] ^= 0xff;
    addr._mac[1] ^= 0xff;
    addr._mac[2] ^= 0xff;
    addr._mac[3] ^= 0xff;
    addr._mac[4] ^= 0xff;
    addr._mac[5] ^= 0xff;
    return addr;
}

// =========================================================================
//   CLASS     :
//   METHOD    : operator+
// =========================================================================
MacAddress join::operator+ (int value, const MacAddress& a)
{
    MacAddress addr (a);
    addr += (value);
    return addr;
}

// =========================================================================
//   CLASS     :
//   METHOD    : operator+
// =========================================================================
MacAddress join::operator+ (const MacAddress& a, int value)
{
    return operator+ (value, a);
}

// =========================================================================
//   CLASS     :
//   METHOD    : operator==
// =========================================================================
bool join::operator== (const MacAddress& a, const MacAddress& b)
{
    return memcmp (a.addr (), b.addr (), a.length ()) == 0;
}

// =========================================================================
//   CLASS     :
//   METHOD    : operator!=
// =========================================================================
bool join::operator!= (const MacAddress& a, const MacAddress& b)
{
    return !(a == b);
}

// =========================================================================
//   CLASS     :
//   METHOD    : operator<
// =========================================================================
bool join::operator< (const MacAddress& a, const MacAddress& b)
{
    return memcmp (a.addr (), b.addr (), a.length ()) < 0;
}

// =========================================================================
//   CLASS     :
//   METHOD    : operator<=
// =========================================================================
bool join::operator<= (const MacAddress& a, const MacAddress& b)
{
    return !(b < a);
}

// =========================================================================
//   CLASS     :
//   METHOD    : operator>
// =========================================================================
bool join::operator> (const MacAddress& a, const MacAddress& b)
{
    return b < a;
}

// =========================================================================
//   CLASS     :
//   METHOD    : operator>=
// =========================================================================
bool join::operator>= (const MacAddress& a, const MacAddress& b)
{
    return !(a < b);
}

// =========================================================================
//   CLASS     :
//   METHOD    : operator&
// =========================================================================
MacAddress join::operator& (const MacAddress& a, const MacAddress& b)
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
MacAddress join::operator| (const MacAddress& a, const MacAddress& b)
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
MacAddress join::operator^ (const MacAddress& a, const MacAddress& b)
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
std::ostream& join::operator<< (std::ostream& out, const MacAddress& a)
{
    out << a.toString ();
    return out;
}
