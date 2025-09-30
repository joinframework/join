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
#include <join/error.hpp>
#include <join/ipaddress.hpp>

// C++.
#include <bitset>
#include <sstream>
#include <utility>
#include <exception>

// C.
#include <cstring>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>

namespace join
{
    /**
     * @brief IP address implementation interface.
     */
    class IpAddressImpl
    {
    protected:
        /**
         * @brief create the IpAddressImpl instance.
         */
        IpAddressImpl ()
        {
        }

        /**
         * @brief create the IpAddressImpl instance by copy.
         * @param address address to copy.
         */
        IpAddressImpl (const IpAddressImpl& address) = delete;

    public:
        /**
         * @brief destroy the IpAddressImpl instance.
         */
        virtual ~IpAddressImpl ()
        {
        }

        /**
         * @brief get address family.
         * @return AF_INET6 if IPv6, AF_INET if IPv4.
         */
        virtual int family () const = 0;

        /**
         * @brief get the internal address structure.
         * @return a pointer to an in6_addr structure if IPv6 or to an in_addr structure if IPv4;
         */
        virtual const void* addr () const = 0;

        /**
         * @brief get the size in byte of the internal address structure.
         * @return the size in byte of the internal address structure.
         */
        virtual socklen_t length () const = 0;

        /**
         * @brief get the scope identifier of the address.
         * @return the scope identifier of the address.
         */
        virtual uint32_t scope () const = 0;

        /**
         * @brief get prefix length from netmask address.
         * @return prefix length.
         */
        virtual int prefix () const = 0;

        /**
         * @brief check if IP address is a wildcard address.
         * @return if wildcard true is returned, false otherwise.
         */
        virtual bool isWildcard () const = 0;

        /**
         * @brief check if IP address is a loopback address.
         * @return if loopback true is returned, false otherwise.
         */
        virtual bool isLoopBack () const = 0;

        /**
         * @brief check if IP address is link local.
         * @return if IP address is link local true is returned, false otherwise.
         */
        virtual bool isLinkLocal () const = 0;

        /**
         * @brief check if IP address is site local (deprecated).
         * @return if IP address is site local true is returned, false otherwise.
         */
        virtual bool isSiteLocal () const = 0;

        /**
         * @brief check if IP address is unique local.
         * @return if IP address is unique local true is returned, false otherwise.
         */
        virtual bool isUniqueLocal () const = 0;

        /**
         * @brief check if IP address is a broadcast address.
         * @param prefix prefix length.
         * @return if broadcast true is returned, false otherwise.
         */
        virtual bool isBroadcast (int prefix) const = 0;

        /**
         * @brief check if IP address is multicast.
         * @return if multicast true is returned, false otherwise.
         */
        virtual bool isMulticast () const = 0;

        /**
         * @brief check if IP address is IPv4 compatible (deprecated).
         * @return if IPv4 compatible true is returned, false otherwise.
         */
        virtual bool isIpv4Compat () const = 0;

        /**
         * @brief check if IP address is IPv4 mapped.
         * @return if IPv4 mapped true is returned, false otherwise.
         */
        virtual bool isIpv4Mapped () const = 0;

        /**
         * @brief convert internal address structure to string.
         * @return the internal address structure converted to string.
         */
        virtual std::string toString () const = 0;

        /**
         * @brief convert IP address to the in-addr.arpa or ip6.arpa domain name.
         * @return the converted IP address.
         */
        virtual std::string toArpa () const = 0;

        /**
         * @brief clear IP address.
         */
        virtual void clear () = 0;

        /**
         * @brief returns a reference to the element at the specified location.
         * @param position position of the element to return.
         * @return reference to the requested element.
         * @throw invalid_argument if position is out of range.
         */
        virtual uint8_t& operator[] (size_t position) = 0;
    };

    /**
     * @brief IPv4 address implementation.
     */
    class Ipv4Address : public IpAddressImpl
    {
    public:
        /**
         * @brief create the Ipv4Address instance.
         */
        Ipv4Address ()
        {
            memset (&_addr, 0, sizeof _addr);
        }

        /**
         * @brief create the Ipv4Address instance by copy.
         * @param address address to copy.
         */
        Ipv4Address (const Ipv4Address& address)
        {
            memcpy (&_addr, &address._addr, sizeof _addr);
        }

        /**
         * @brief copy the Ipv4Address instance.
         * @param address address to copy.
         */
        Ipv4Address& operator= (const Ipv4Address& address)
        {
            memcpy (&_addr, &address._addr, sizeof _addr);
            return *this;
        }

        /**
         * @brief create the Ipv4Address instance using IPv4 address structure.
         * @param address address structure to use.
         */
        Ipv4Address (const void * address)
        {
            memcpy (&_addr, address, sizeof _addr);
        }

        /**
         * @brief create netmask address using prefix.
         * @param prefix number of bits to create the netmask address from.
         */
        Ipv4Address (int prefix)
        {
            if (prefix)
            {
                _addr.s_addr = htonl (~((1 << (32 - prefix)) - 1));
            }
            else
            {
                _addr.s_addr = htonl (0);
            }
        }

        /**
         * @brief destroy the Ipv4Address instance.
         */
        ~Ipv4Address ()
        {
        }

        /**
         * @brief get address family.
         * @return AF_INET if IPv4, AF_INET6 if IPv6.
         */
        int family () const
        {
            return AF_INET;
        }

        /**
         * @brief get the internal address structure.
         * @return the internal address structure.
         */
        const void* addr () const
        {
            return &_addr;
        }

        /**
         * @brief get the size in byte of the internal address structure.
         * @return the size in byte of the internal address structure.
         */
        socklen_t length () const
        {
            return sizeof _addr;
        }

        /**
         * @brief get the scope identifier of the address.
         * @return the scope identifier of the address.
         */
        uint32_t scope () const
        {
            return 0;
        }

        /**
         * @brief get prefix length from netmask address.
         * @return prefix length.
         */
        int prefix () const
        {
            return std::bitset <32> (ntohl (_addr.s_addr)).count ();
        }

        /**
         * @brief check if IP address is
         * @return if wildcard true is returned, false otherwise.
         */
        bool isWildcard () const
        {
            return ntohl (_addr.s_addr) == INADDR_ANY;
        }

        /**
         * @brief check if IP address is a loopback address.
         * @return if loopback true is returned, false otherwise.
         */
        bool isLoopBack () const
        {
            return ntohl (_addr.s_addr) == INADDR_LOOPBACK;
        }

        /**
         * @brief check if IP address is link local.
         * @return if IP address is link local true is returned, false otherwise.
         */
        bool isLinkLocal () const
        {
            return (ntohl(_addr.s_addr) & 0xFFFF0000) == 0xA9FE0000;
        }

        /**
         * @brief check if IP address is site local (deprecated).
         * @return if IP address is site local true is returned, false otherwise.
         */
        bool isSiteLocal () const
        {
            return isUniqueLocal ();
        }

        /**
         * @brief check if IP address is unique local.
         * @return if IP address is unique local true is returned, false otherwise.
         */
        bool isUniqueLocal () const
        {
            return (ntohl(_addr.s_addr) & 0xFF000000) == 0x0A000000 ||
                   (ntohl(_addr.s_addr) & 0xFFFF0000) == 0xC0A80000 ||
                   (ntohl(_addr.s_addr) >= 0xAC100000 && ntohl(_addr.s_addr) <= 0xAC1FFFFF);
        }

        /**
         * @brief check if IP address is a broadcast address.
         * @param prefix prefix length.
         * @return if broadcast true is returned, false otherwise.
         */
        bool isBroadcast (int prefix) const
        {
            uint32_t ip = ntohl (_addr.s_addr);

            if (ip == INADDR_BROADCAST)
            {
                return true;
            }

            if (prefix >= 0 && prefix <= 32)
            {
                uint32_t mask = (prefix == 0) ? 0 : (0xFFFFFFFF << (32 - prefix));
                return ip == ((ip & mask) | ~mask);
            }

            return false;
        }

        /**
         * @brief check if IP address is multicast.
         * @return if multicast true is returned, false otherwise.
         */
        bool isMulticast () const
        {
            return IN_MULTICAST (ntohl(_addr.s_addr));
        }

        /**
         * @brief check if IP address is IPv4 compatible (deprecated).
         * @return if IPv4 compatible true is returned, false otherwise.
         */
        bool isIpv4Compat () const
        {
            return true;
        }

        /**
         * @brief check if IP address is IPv4 mapped.
         * @return if IPv4 mapped true is returned, false otherwise.
         */
        bool isIpv4Mapped () const
        {
            return true;
        }

        /**
         * @brief convert internal address structure to string.
         * @return the internal address structure converted to string.
         */
        std::string toString () const
        {
            std::string address;
            char buffer [INET_ADDRSTRLEN];
            if (inet_ntop (family (), addr (), buffer, INET_ADDRSTRLEN) != nullptr)
            {
                address.append (buffer);
            }
            return address;
        }

        /**
         * @brief convert IP address to the in-addr.arpa domain name.
         * @return the converted IP address.
         */
        std::string toArpa () const
        {
            std::stringstream arpa;
            arpa << static_cast <int> ((_addr.s_addr & 0xFF000000) >> 24) << ".";
            arpa << static_cast <int> ((_addr.s_addr & 0x00FF0000) >> 16) << ".";
            arpa << static_cast <int> ((_addr.s_addr & 0x0000FF00) >> 8) << ".";
            arpa << static_cast <int> ((_addr.s_addr & 0x000000FF)) << ".";
            arpa << "in-addr.arpa";
            return arpa.str ();
        }

        /**
         * @brief parse an IPv4 address in string format.
         * @param address address string to use.
         * @param ip IP address output.
         * @return true on success, false otherwise.
         */
        static bool parse (const std::string& address, Ipv4Address& ip)
        {
            if (address.empty () == false)
            {
                struct in_addr addr;
                if (inet_pton (AF_INET, address.c_str (), &addr) == 1)
                {
                    ip = Ipv4Address (&addr);
                    return true;
                }
            }
            return false;
        }

        /**
         * @brief perform AND operation on IP address.
         * @param address address to compare.
         * @return result of AND operation on IpAddress.
         */
        Ipv4Address operator& (const Ipv4Address& address) const
        {
            Ipv4Address addr (*this);
            addr._addr.s_addr &= address._addr.s_addr;
            return addr;
        }

        /**
         * @brief perform OR operation on IP address.
         * @param address Address to compare.
         * @return result of OR operation on IpAddress.
         */
        Ipv4Address operator| (const Ipv4Address& address) const
        {
            Ipv4Address addr (*this);
            addr._addr.s_addr |= address._addr.s_addr;
            return addr;
        }

        /**
         * @brief perform XOR operation on IP address.
         * @param address address to compare.
         * @return result of XOR operation on IpAddress.
         */
        Ipv4Address operator^ (const Ipv4Address& address) const
        {
            Ipv4Address addr (*this);
            addr._addr.s_addr ^= address._addr.s_addr;
            return addr;
        }

        /**
         * @brief perform NOT operation on IP address.
         * @return result of NOT operation on IpAddress.
         */
        Ipv4Address operator~ () const
        {
            Ipv4Address addr (*this);
            addr._addr.s_addr ^= 0xffffffff;
            return addr;
        }

        /**
         * @brief clear IP address (wilcard address).
         */
        void clear ()
        {
            memset (&_addr, 0, sizeof _addr);
        }

        /**
         * @brief returns a reference to the element at the specified location.
         * @param position position of the element to return.
         * @return reference to the requested element.
         * @throw invalid_argument if position is out of range.
         */
        uint8_t& operator[] (size_t position)
        {
            if (position > length () - 1)
            {
                throw std::out_of_range ("position is out of range");
            }

            return *(reinterpret_cast <uint8_t*> (&_addr) + position);
        }

    private:
        /// IPv4 address.
        struct in_addr _addr;
    };

    /**
     * @brief IPv6 address implementation.
     */
    class Ipv6Address : public IpAddressImpl
    {
    public:
        /**
         * @brief create the Ipv6Address instance.
         */
        Ipv6Address ()
        {
            memset (&_addr, 0, sizeof _addr);
        }

        /**
         * @brief create the Ipv6Address instance by copy.
         * @param address address to copy.
         */
        Ipv6Address (const Ipv4Address& address)
        {
            _addr.s6_addr32[0] = 0;
            _addr.s6_addr32[1] = 0;
            _addr.s6_addr32[2] = htonl (0xffff);
            _addr.s6_addr32[3] = reinterpret_cast <const in_addr*> (address.addr ())->s_addr;
        }

        /**
         * @brief create the Ipv6Address instance by copy.
         * @param address address to copy.
         */
        Ipv6Address (const Ipv6Address& address)
        : _scope (address._scope)
        {
            memcpy (&_addr, &address._addr, sizeof _addr);
        }

        /**
         * @brief copy the Ipv6Address instance.
         * @param address address to copy.
         */
        Ipv6Address& operator= (const Ipv6Address& address)
        {
            memcpy (&_addr, &address._addr, sizeof _addr);
            _scope = address._scope;
            return *this;
        }

        /**
         * @brief create the Ipv6Address instance using IPv6 address structure.
         * @param address address structure to use.
         */
        Ipv6Address (const void* address)
        {
            memcpy (&_addr, address, sizeof _addr);
        }

        /**
         * @brief create the Ipv6Address instance using IPv6 address structure.
         * @param address address structure to use.
         * @param scope the scope identifier of the address.
         */
        Ipv6Address (const void* address, uint32_t scope)
        : _scope (scope)
        {
            memcpy (&_addr, address, sizeof _addr);
        }

        /**
         * @brief create netmask address using prefix.
         * @param prefix number of bits to create the netmask address from.
         */
        Ipv6Address (int prefix)
        {
            memset (&_addr, 0, sizeof _addr);

            for (int i = 0; prefix > 0; prefix -= 8, ++i)
            {
                if (prefix >= 8)
                {
                    _addr.s6_addr[i] = 0xff;
                }
                else
                {
                    _addr.s6_addr[i] = (unsigned long) (0xffU << (8 - prefix));
                }
            }
        }

        /**
         * @brief destroy the Ipv6Address instance.
         */
        ~Ipv6Address ()
        {
        }

        /**
         * @brief get address family.
         * @return AF_INET6 if IPv6, AF_INET if IPv4.
         */
        int family () const
        {
            return AF_INET6;
        }

        /**
         * @brief get the internal address structure.
         * @return The internal address structure.
         */
        const void* addr () const
        {
            return &_addr;
        }

        /**
         * @brief get the size in byte of the internal address structure.
         * @return the size in byte of the internal address structure.
         */
        socklen_t length () const
        {
            return sizeof _addr;
        }

        /**
         * @brief get the scope identifier of the address.
         * @return the scope identifier of the address.
         */
        uint32_t scope () const
        {
            return _scope;
        }

        /**
         * @brief get prefix length from netmask address.
         * @return prefix length.
         */
        int prefix () const
        {
            uint32_t bitPos = 128;

            for (int i = 3; i >= 0; --i)
            {
                uint32_t bits = std::bitset <32> (ntohl (_addr.s6_addr32[i])).count ();
                if (bits)
                {
                    return (bitPos - (32 - bits));
                }
                bitPos -= 32;
            }

            return 0;
        }

        /**
         * @brief check if IP address is
         * @return if wildcard true is returned, false otherwise.
         */
        bool isWildcard () const
        {
            return IN6_IS_ADDR_UNSPECIFIED (&_addr);
        }

        /**
         * @brief check if IP address is a loopback address.
         * @return if loopback true is returned, false otherwise.
         */
        bool isLoopBack () const
        {
            return IN6_IS_ADDR_LOOPBACK (&_addr);
        }

        /**
         * @brief check if IP address is link local.
         * @return if IP address is link local true is returned, false otherwise.
         */
        bool isLinkLocal () const
        {
            return IN6_IS_ADDR_LINKLOCAL (&_addr);
        }

        /**
         * @brief check if IP address is site local (deprecated).
         * @return if IP address is site local true is returned, false otherwise.
         */
        bool isSiteLocal () const
        {
            return IN6_IS_ADDR_SITELOCAL (&_addr);
        }

        /**
         * @brief check if IP address is unique local.
         * @return if IP address is unique local true is returned, false otherwise.
         */
        bool isUniqueLocal () const
        {
            return (_addr.__in6_u.__u6_addr32[0] & htonl (0xfe000000)) == htonl (0xfc000000);
        }

        /**
         * @brief check if IP address is a broadcast address.
         * @param prefix prefix length.
         * @return if broadcast true is returned, false otherwise.
         */
        bool isBroadcast (int /*prefix*/) const
        {
            return false;
        }

        /**
         * @brief check if IP address is multicast.
         * @return if multicast true is returned, false otherwise.
         */
        bool isMulticast () const
        {
            return IN6_IS_ADDR_MULTICAST (&_addr);
        }

        /**
         * @brief check if IP address is IPv4 compatible (deprecated).
         * @return if IPv4 compatible true is returned, false otherwise.
         */
        bool isIpv4Compat () const
        {
            return IN6_IS_ADDR_V4COMPAT (&_addr);
        }

        /**
         * @brief check if IP address is IPv4 mapped.
         * @return if IPv4 mapped true is returned, false otherwise.
         */
        bool isIpv4Mapped () const
        {
            return IN6_IS_ADDR_V4MAPPED (&_addr);
        }

        /**
         * @brief convert internal address structure to string.
         * @return the internal address structure converted to string.
         */
        std::string toString () const
        {
            std::string address;
            char buffer [INET6_ADDRSTRLEN];
            if (inet_ntop (family (), addr (), buffer, INET6_ADDRSTRLEN) != nullptr)
            {
                address.append (buffer);
                if (_scope > 0)
                {
                    address.append ("%");
                    char ifname[IFNAMSIZ];
                    if (if_indextoname (_scope, ifname))
                    {
                        address.append (ifname);
                    }
                    else
                    {
                        address.append (std::to_string (_scope));
                    }
                }
            }
            return address;
        }

        /**
         * @brief convert IP address to the ip6.arpa domain name.
         * @return the converted IP address.
         */
        std::string toArpa () const
        {
            std::stringstream arpa;
            size_t i = length ();
            while (i--)
            {
                arpa << std::hex;
                arpa << static_cast <int> ((_addr.s6_addr[i] & 0x0F)) << ".";
                arpa << static_cast <int> ((_addr.s6_addr[i] & 0xF0) >> 4) << ".";
            }
            arpa << "ip6.arpa";
            return arpa.str ();
        }

        /**
         * @brief parse an IPv6 address in string format.
         * @param address address string to use.
         * @param ip IP address output.
         * @return true on success, false otherwise.
         */
        static bool parse (const std::string& address, Ipv6Address& ip)
        {
            std::string unscopedAddress (address);
            uint32_t scope = 0;

            auto pos = address.find ('%');
            if (pos != std::string::npos)
            {
                unscopedAddress.erase (pos);
                auto offset = (address.front () == '[') ? 1 : 0;
                std::string tmp (address, pos + 1, address.size() - pos - offset);
                if (tmp.find_first_not_of ("0123456789") == std::string::npos)
                {
                    scope = std::stoi (tmp);
                }
                else
                {
                    scope = if_nametoindex (tmp.c_str ());
                }
            }

            struct in6_addr addr;
            if (inet_pton (AF_INET6, unscopedAddress.c_str (), &addr) == 1)
            {
                ip = Ipv6Address (&addr, scope);
                return true;
            }

            return false;
        }

        /**
         * @brief perform AND operation on IP address.
         * @param address address to compare.
         * @return result of AND operation on IpAddress.
         */
        Ipv6Address operator& (const Ipv6Address& address) const
        {
            Ipv6Address addr (*this);
            addr._addr.s6_addr32[0] &= address._addr.s6_addr32[0];
            addr._addr.s6_addr32[1] &= address._addr.s6_addr32[1];
            addr._addr.s6_addr32[2] &= address._addr.s6_addr32[2];
            addr._addr.s6_addr32[3] &= address._addr.s6_addr32[3];
            return addr;
        }

        /**
         * @brief perform OR operation on IP address.
         * @param address address to compare.
         * @return result of OR operation on IpAddress.
         */
        Ipv6Address operator| (const Ipv6Address& address) const
        {
            Ipv6Address addr (*this);
            addr._addr.s6_addr32[0] |= address._addr.s6_addr32[0];
            addr._addr.s6_addr32[1] |= address._addr.s6_addr32[1];
            addr._addr.s6_addr32[2] |= address._addr.s6_addr32[2];
            addr._addr.s6_addr32[3] |= address._addr.s6_addr32[3];
            return addr;
        }

        /**
         * @brief perform XOR operation on IP address.
         * @param address address to compare.
         * @return result of XOR operation on IpAddress.
         */
        Ipv6Address operator^ (const Ipv6Address& address) const
        {
            Ipv6Address addr (*this);
            addr._addr.s6_addr32[0] ^= address._addr.s6_addr32[0];
            addr._addr.s6_addr32[1] ^= address._addr.s6_addr32[1];
            addr._addr.s6_addr32[2] ^= address._addr.s6_addr32[2];
            addr._addr.s6_addr32[3] ^= address._addr.s6_addr32[3];
            return addr;
        }

        /**
         * @brief perform NOT operation on IP address.
         * @return result of NOT operation on IpAddress.
         */
        Ipv6Address operator~ () const
        {
            Ipv6Address addr (*this);
            addr._addr.s6_addr32[0] ^= 0xffffffff;
            addr._addr.s6_addr32[1] ^= 0xffffffff;
            addr._addr.s6_addr32[2] ^= 0xffffffff;
            addr._addr.s6_addr32[3] ^= 0xffffffff;
            return addr;
        }

        /**
         * @brief clear IP address (wilcard address).
         */
        void clear ()
        {
            memset (&_addr, 0, sizeof _addr);
        }

        /**
         * @brief returns a reference to the element at the specified location.
         * @param position position of the element to return.
         * @return reference to the requested element.
         * @throw invalid_argument if position is out of range.
         */
        uint8_t& operator[] (size_t position)
        {
            if (position > length () - 1)
            {
                throw std::out_of_range ("position is out of range");
            }

            return _addr.s6_addr[position];
        }

    private:
        /// IPv6 address.
        struct in6_addr _addr;

        /// IPv6 address scope.
        uint32_t _scope = 0;
    };
}

using join::IpAddress;

/// wildcard IPv6 address.
const IpAddress IpAddress::ipv6Wildcard = "::";

/// all nodes multicast IPv6 address.
const IpAddress IpAddress::ipv6AllNodes = "ff02::1";

/// solicited nodes multicast IPv6 address.
const IpAddress IpAddress::ipv6SolicitedNodes = "ff02::1:ff00:0";

/// routers multicast IPv6 address.
const IpAddress IpAddress::ipv6Routers = "ff02::2";

/// wildcard IPv4 address.
const IpAddress IpAddress::ipv4Wildcard = "0.0.0.0";

/// broadcast IPv4 address.
const IpAddress IpAddress::ipv4Broadcast = "255.255.255.255";

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : IpAddress
// =========================================================================
IpAddress::IpAddress ()
{
    _ip = std::make_unique <Ipv4Address> ();
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : IpAddress
// =========================================================================
IpAddress::IpAddress (int family)
{
    if (family == AF_INET6)
    {
        _ip = std::make_unique <Ipv6Address> ();
        return;
    }
    else if (family == AF_INET)
    {
        _ip = std::make_unique <Ipv4Address> ();
        return;
    }

    throw std::invalid_argument ("invalid IP address family");
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : IpAddress
// =========================================================================
IpAddress::IpAddress (const IpAddress& address)
{
    if (address.family () == AF_INET6)
    {
        _ip = std::make_unique <Ipv6Address> (address.addr (), address.scope ());
    }
    else
    {
        _ip = std::make_unique <Ipv4Address> (address.addr ());
    }
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : IpAddress
// =========================================================================
IpAddress::IpAddress (IpAddress&& address)
: _ip (std::move (address._ip))
{
    // reset other attributes.
    address._ip = std::make_unique <Ipv6Address> ();
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : IpAddress
// =========================================================================
IpAddress::IpAddress (const struct sockaddr& address)
{
    if (address.sa_family == AF_INET6)
    {
        const struct sockaddr_in6* sa = reinterpret_cast <const struct sockaddr_in6*> (&address);
        _ip = std::make_unique <Ipv6Address> (&sa->sin6_addr, sa->sin6_scope_id);
        return;
    }
    else if (address.sa_family == AF_INET)
    {
        const struct sockaddr_in* sa = reinterpret_cast <const struct sockaddr_in*> (&address);
        _ip = std::make_unique <Ipv4Address> (&sa->sin_addr);
        return;
    }

    throw std::invalid_argument ("invalid IP address family");
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : IpAddress
// =========================================================================
IpAddress::IpAddress (const void* address, socklen_t length)
{
    if (length == sizeof (struct in6_addr))
    {
        _ip = std::make_unique <Ipv6Address> (address);
        return;
    }
    else if (length == sizeof (struct in_addr))
    {
        _ip = std::make_unique <Ipv4Address> (address);
        return;
    }

    throw std::invalid_argument ("invalid IP address length");
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : IpAddress
// =========================================================================
IpAddress::IpAddress (const void* address, socklen_t length, uint32_t scope)
{
    if (length == sizeof (struct in6_addr))
    {
        _ip = std::make_unique <Ipv6Address> (address, scope);
        return;
    }
    else if (length == sizeof (struct in_addr))
    {
        _ip = std::make_unique <Ipv4Address> (address);
        return;
    }

    throw std::invalid_argument ("invalid IP address length");
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : IpAddress
// =========================================================================
IpAddress::IpAddress (const std::string& address, int family)
: IpAddress (address.c_str (), family)
{
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : IpAddress
// =========================================================================
IpAddress::IpAddress (const std::string& address)
: IpAddress (address.c_str ())
{
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : IpAddress
// =========================================================================
IpAddress::IpAddress (const char* address, int family)
{
    if (family == AF_INET6)
    {
        if (address == nullptr || strcmp (address, "") == 0)
        {
            _ip = std::make_unique <Ipv6Address> ();
            return;
        }

        Ipv6Address addr6;
        if (Ipv6Address::parse (address, addr6) == true)
        {
            _ip = std::make_unique <Ipv6Address> (addr6);
            return;
        }

        Ipv4Address addr4;
        if (Ipv4Address::parse (address, addr4) == true)
        {
            _ip = std::make_unique <Ipv6Address> (addr4);
            return;
        }
    }
    else if (family == AF_INET)
    {
        if (address == nullptr || strcmp (address, "") == 0)
        {
            _ip = std::make_unique <Ipv4Address> ();
            return;
        }

        Ipv4Address addr4;
        if (Ipv4Address::parse (address, addr4) == true)
        {
            _ip = std::make_unique <Ipv4Address> (addr4);
            return;
        }
    }

    throw std::invalid_argument ("invalid IP address");
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : IpAddress
// =========================================================================
IpAddress::IpAddress (const char* address)
{
    if (address == nullptr || strcmp (address, "") == 0)
    {
        _ip = std::make_unique <Ipv6Address> ();
        return;
    }

    Ipv6Address addr6;
    if (Ipv6Address::parse (address, addr6) == true)
    {
        _ip = std::make_unique <Ipv6Address> (addr6);
        return;
    }

    Ipv4Address addr4;
    if (Ipv4Address::parse (address, addr4) == true)
    {
        _ip = std::make_unique <Ipv4Address> (addr4);
        return;
    }

    throw std::invalid_argument ("invalid IP address");
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : IpAddress
// =========================================================================
IpAddress::IpAddress (int prefix, int family)
{
    if (family == AF_INET6)
    {
        if (prefix >= 0 && prefix <= 128)
        {
            _ip = std::make_unique <Ipv6Address> (prefix);
            return;
        }

        throw std::invalid_argument ("invalid prefix length");
    }
    else if (family == AF_INET)
    {
        if (prefix >= 0 && prefix <= 32)
        {
            _ip = std::make_unique <Ipv4Address> (prefix);
            return;
        }

        throw std::invalid_argument ("invalid prefix length");
    }

    throw std::invalid_argument ("invalid IP address family");
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : operator=
// =========================================================================
IpAddress& IpAddress::operator= (const IpAddress& address)
{
    if (address.family () == AF_INET6)
    {
        _ip = std::make_unique <Ipv6Address> (address.addr (), address.scope ());
    }
    else
    {
        _ip = std::make_unique <Ipv4Address> (address.addr ());
    }

    return *this;
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : operator=
// =========================================================================
IpAddress& IpAddress::operator= (IpAddress&& address)
{
    // steal other attributes.
    _ip = std::move (address._ip);

    // reset other attributes.
    address._ip = std::make_unique <Ipv6Address> ();

    return *this;
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : operator=
// =========================================================================
IpAddress& IpAddress::operator= (const struct sockaddr& address)
{
    if (address.sa_family == AF_INET6)
    {
        const struct sockaddr_in6* sa = reinterpret_cast <const struct sockaddr_in6*> (&address);
        _ip = std::make_unique <Ipv6Address> (&sa->sin6_addr, sa->sin6_scope_id);
    }
    else if (address.sa_family == AF_INET)
    {
        const struct sockaddr_in* sa = reinterpret_cast <const struct sockaddr_in*> (&address);
        _ip = std::make_unique <Ipv4Address> (&sa->sin_addr);
    }

    return *this;
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : ~IpAddress
// =========================================================================
IpAddress::~IpAddress ()
{
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : family
// =========================================================================
int IpAddress::family () const
{
    return _ip->family ();
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : addr
// =========================================================================
const void * IpAddress::addr () const
{
    return _ip->addr ();
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : length
// =========================================================================
socklen_t IpAddress::length () const
{
    return _ip->length ();
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : scope
// =========================================================================
uint32_t IpAddress::scope () const
{
    return _ip->scope ();
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : prefix
// =========================================================================
int IpAddress::prefix () const
{
    return _ip->prefix ();
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : isWildcard
// =========================================================================
bool IpAddress::isWildcard () const
{
    return _ip->isWildcard ();
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : isLoopBack
// =========================================================================
bool IpAddress::isLoopBack () const
{
    return _ip->isLoopBack ();
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : isLinkLocal
// =========================================================================
bool IpAddress::isLinkLocal () const
{
    return _ip->isLinkLocal ();
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : isSiteLocal
// =========================================================================
bool IpAddress::isSiteLocal () const
{
    return _ip->isSiteLocal ();
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : isUnicast
// =========================================================================
bool IpAddress::isUnicast () const
{
    return !isWildcard () && !isBroadcast () && !isMulticast ();
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : isUniqueLocal
// =========================================================================
bool IpAddress::isUniqueLocal () const
{
    return _ip->isUniqueLocal ();
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : isBroadcast
// =========================================================================
bool IpAddress::isBroadcast (int prefix) const
{
    return _ip->isBroadcast (prefix);
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : isMulticast
// =========================================================================
bool IpAddress::isMulticast () const
{
    return _ip->isMulticast ();
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : isGlobal
// =========================================================================
bool IpAddress::isGlobal () const
{
    return isUnicast () && !isLoopBack () && !isLinkLocal () && !isSiteLocal () && !isUniqueLocal ();
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : isIpAddress
// =========================================================================
bool IpAddress::isIpAddress (const std::string& address)
{
    try
    {
        IpAddress addr (address);
    }
    catch (const std::exception& ex)
    {
        return false;
    }

    return true;
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : isIpv6Address
// =========================================================================
bool IpAddress::isIpv6Address () const
{
    return _ip->family () == AF_INET6;
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : isIpv6Address
// =========================================================================
bool IpAddress::isIpv6Address (const std::string& address)
{
    try
    {
        return IpAddress (address).isIpv6Address ();
    }
    catch (const std::exception& ex)
    {
    }

    return false;
}


// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : isIpv4Compat
// =========================================================================
bool IpAddress::isIpv4Compat () const
{
    return _ip->isIpv4Compat ();
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : isIpv4Mapped
// =========================================================================
bool IpAddress::isIpv4Mapped () const
{
    return _ip->isIpv4Mapped ();
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : isIpv4Address
// =========================================================================
bool IpAddress::isIpv4Address () const
{
    return _ip->family () == AF_INET;
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : isIpv4Address
// =========================================================================
bool IpAddress::isIpv4Address (const std::string& address)
{
    try
    {
        return IpAddress (address).isIpv4Address ();
    }
    catch (const std::exception& ex)
    {
    }

    return false;
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : toIpv6
// =========================================================================
IpAddress IpAddress::toIpv6 () const
{
    if (family () == AF_INET)
    {
        struct in6_addr address;
        address.s6_addr32[0] = 0;
        address.s6_addr32[1] = 0;
        address.s6_addr32[2] = htonl (0xffff);
        address.s6_addr32[3] = reinterpret_cast <const in_addr*> (addr ())->s_addr;
        return IpAddress (&address, sizeof (struct in6_addr));
    }

    return *this;
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : toIpv4
// =========================================================================
IpAddress IpAddress::toIpv4 () const
{
    if ((family () == AF_INET6) && (isIpv4Compat () || isIpv4Mapped ()))
    {
        struct in_addr address;
        address.s_addr = reinterpret_cast <const in6_addr*> (addr ())->s6_addr32[3];
        return IpAddress (&address, sizeof (struct in_addr));
    }

    return *this;
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : toString
// =========================================================================
std::string IpAddress::toString () const
{
    return _ip->toString ();
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : toArpa
// =========================================================================
std::string IpAddress::toArpa () const
{
    return _ip->toArpa ();
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : clear
// =========================================================================
void IpAddress::clear ()
{
    _ip->clear ();
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : ipv4Address
// =========================================================================
IpAddress IpAddress::ipv4Address (const std::string& interface)
{
    int fd = ::socket (AF_INET, SOCK_DGRAM, 0);
    if (fd != -1)
    {
        struct ifreq iface;
        ::memset (&iface, 0, sizeof (iface));
        ::strncpy (iface.ifr_name, interface.c_str (), IFNAMSIZ - 1);
        iface.ifr_addr.sa_family = AF_INET;

        int result = ::ioctl (fd, SIOCGIFADDR, &iface);
        ::close (fd);

        if (result != -1)
        {
            return IpAddress (iface.ifr_addr);
        }
    }

    lastError = std::make_error_code (static_cast <std::errc> (errno));
    return ipv4Wildcard;
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : operator~
// =========================================================================
IpAddress IpAddress::operator~ () const
{
    if (family () == AF_INET6)
    {
        Ipv6Address first (addr (), scope ());
        Ipv6Address result = ~first;
        return IpAddress (result.addr (), result.length (), result.scope ());
    }
    else
    {
        Ipv4Address first (addr ());
        Ipv4Address result = ~first;
        return IpAddress (result.addr(), result.length ());
    }
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : operator[]
// =========================================================================
uint8_t& IpAddress::operator[] (size_t position)
{
    return _ip->operator[] (position);
}

// =========================================================================
//   CLASS     : IpAddress
//   METHOD    : operator[]
// =========================================================================
const uint8_t& IpAddress::operator[] (size_t position) const
{
    return _ip->operator[] (position);
}

// =========================================================================
//   CLASS     :
//   METHOD    : operator==
// =========================================================================
bool join::operator== (const IpAddress& a, const IpAddress& b)
{
    if (a.length () != b.length ())
    {
        return false;
    }

    if (a.scope () != b.scope ())
    {
        return false;
    }

    return memcmp (a.addr (), b.addr (), a.length ()) == 0;
}

// =========================================================================
//   CLASS     :
//   METHOD    : operator!=
// =========================================================================
bool join::operator!= (const IpAddress& a, const IpAddress& b)
{
    return !(a == b);
}

// =========================================================================
//   CLASS     :
//   METHOD    : operator<
// =========================================================================
bool join::operator< (const IpAddress& a, const IpAddress& b)
{
    if (a.family () == b.family ())
    {
        if (a.scope () != b.scope ())
        {
            return a.scope () < b.scope ();
        }

        return memcmp (a.addr (), b.addr (), a.length ()) < 0;
    }

    return a.length () < b.length ();
}

// =========================================================================
//   CLASS     :
//   METHOD    : operator<=
// =========================================================================
bool join::operator<= (const IpAddress& a, const IpAddress& b)
{
    return !(b < a);
}

// =========================================================================
//   CLASS     :
//   METHOD    : operator>
// =========================================================================
bool join::operator> (const IpAddress& a, const IpAddress& b)
{
    return b < a;
}

// =========================================================================
//   CLASS     :
//   METHOD    : operator>=
// =========================================================================
bool join::operator>= (const IpAddress& a, const IpAddress& b)
{
    return !(a < b);
}

// =========================================================================
//   CLASS     :
//   METHOD    : operator&
// =========================================================================
IpAddress join::operator& (const IpAddress& a, const IpAddress& b)
{
    if (a.family () == b.family ())
    {
        if (a.family () == AF_INET6)
        {
            Ipv6Address first (a.addr (), a.scope ());
            Ipv6Address second (b.addr (), b.scope ());
            Ipv6Address result = first & second;
            return IpAddress (result.addr (), result.length (), result.scope ());
        }
        else if (a.family () == AF_INET)
        {
            Ipv4Address first (a.addr ());
            Ipv4Address second (b.addr ());
            Ipv4Address result = first & second;
            return IpAddress (result.addr(), result.length ());
        }
    }

    throw std::invalid_argument ("invalid IP address family");
}

// =========================================================================
//   CLASS     :
//   METHOD    : operator|
// =========================================================================
IpAddress join::operator| (const IpAddress& a, const IpAddress& b)
{
    if (a.family () == b.family ())
    {
        if (a.family () == AF_INET6)
        {
            Ipv6Address first (a.addr (), a.scope ());
            Ipv6Address second (b.addr (), b.scope ());
            Ipv6Address result = first | second;
            return IpAddress (result.addr (), result.length (), result.scope ());
        }
        else if (a.family () == AF_INET)
        {
            Ipv4Address first (a.addr ());
            Ipv4Address second (b.addr ());
            Ipv4Address result = first | second;
            return IpAddress (result.addr(), result.length ());
        }
    }

    throw std::invalid_argument ("invalid IP address family");
}

// =========================================================================
//   CLASS     :
//   METHOD    : operator^
// =========================================================================
IpAddress join::operator^ (const IpAddress& a, const IpAddress& b)
{
    if (a.family () == b.family ())
    {
        if (a.family () == AF_INET6)
        {
            Ipv6Address first (a.addr (), a.scope ());
            Ipv6Address second (b.addr (), b.scope ());
            Ipv6Address result = first ^ second;
            return IpAddress (result.addr (), result.length (), result.scope ());
        }
        else if (a.family () == AF_INET)
        {
            Ipv4Address first (a.addr ());
            Ipv4Address second (b.addr ());
            Ipv4Address result = first ^ second;
            return IpAddress (result.addr(), result.length ());
        }
    }

    throw std::invalid_argument ("invalid IP address family");
}

// =========================================================================
//   CLASS     :
//   METHOD    : operator<<
// =========================================================================
std::ostream& join::operator<< (std::ostream& out, const IpAddress& address)
{
    out << address.toString ();
    return out;
}
