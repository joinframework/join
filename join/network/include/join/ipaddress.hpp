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

#ifndef __JOIN_IPADDRESS_HPP__
#define __JOIN_IPADDRESS_HPP__

// C++.
#include <ostream>
#include <string>
#include <memory>
#include <vector>

// C.
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

namespace join
{
    class IpAddress;
    class IpAddressImpl;

    /// List of IP address.
    using IpAddressList = std::vector <IpAddress>;

    /**
     * @brief IPv6, IPv4 address class.
     */
    class IpAddress
    {
    public:
        /**
         * @brief create the IpAddress instance (default: IPv6 wildcard address).
         */
        IpAddress ();

        /**
         * @brief create the IpAddress instance using address family.
         * @param family address family.
         */
        IpAddress (int family);

        /**
         * @brief create the IpAddress instance by copy.
         * @param address address to copy.
         */
        IpAddress (const IpAddress& address);

        /**
         * @brief create the IpAddress instance by move.
         * @param address address to move.
         */
        IpAddress (IpAddress&& address);

        /**
         * @brief create the IpAddress instance using a sockaddr structure.
         * @param address address structure to use.
         * @throw invalid_argument if address is wrong.
         */
        IpAddress (const struct sockaddr& address);

        /**
         * @brief create the IpAddress instance using an in_addr structure or an in6_addr structure.
         * @param address address structure to use.
         * @param length address structure size.
         * @throw invalid_argument if address is wrong.
         */
        IpAddress (const void* address, socklen_t length);

        /**
         * @brief create the IpAddress instance using an in_addr structure or an in6_addr structure.
         * @param address address structure to use.
         * @param length address structure size.
         * @param scope the scope identifier of the address (ignored when IPv4).
         * @throw invalid_argument if address is wrong.
         */
        IpAddress (const void* address, socklen_t length, uint32_t scope);

        /**
         * @brief create the IpAddress instance using a string and address family.
         * @param address address string to use.
         * @param family address family.
         * @throw invalid_argument if address is wrong.
         */
        IpAddress (const std::string& address, int family);

        /**
         * @brief create the IpAddress instance using a string.
         * @param address address string to use.
         * @throw invalid_argument if address is wrong.
         */
        IpAddress (const std::string& address);

        /**
         * @brief create the IpAddress instance using a string and address family.
         * @param address address string to use.
         * @param family address family.
         * @throw invalid_argument if address is wrong.
         */
        IpAddress (const char* address, int family);

        /**
         * @brief create the IpAddress instance using a string.
         * @param address address string to use.
         * @throw invalid_argument if address is wrong.
         */
        IpAddress (const char* address);

        /**
         * @brief create netmask address using prefix length.
         * @param prefix number of bits to create the netmask address from.
         * @param family address family.
         */
        IpAddress (int prefix, int family);

        /**
         * @brief assign the IpAddress instance by copy.
         * @param address address to copy.
         * @return a reference of the current object.
         */
        IpAddress& operator= (const IpAddress& address);

        /**
         * @brief assign the IpAddress instance by move.
         * @param address address to move.
         * @return a reference of the current object.
         */
        IpAddress& operator= (IpAddress&& address);

        /**
         * @brief assign the IpAddress using a sockaddr structure.
         * @param address address structure to use.
         * @return a reference of the current object.
         * @throw invalid_argument if address is wrong.
         */
        IpAddress& operator= (const struct sockaddr& address);

        /**
         * @brief destroy the IpAddress instance.
         */
        ~IpAddress ();

        /**
         * @brief get address family.
         * @return AF_INET6 if IPv6, AF_INET if IPv4.
         */
        int family () const;

        /**
         * @brief get the internal address structure.
         * @return a pointer to an in6_addr structure if IPv6 or to an in_addr structure if IPv4.
         */
        const void* addr () const;

        /**
         * @brief get the size in byte of the internal address structure.
         * @return the size in byte of the internal address structure.
         */
        socklen_t length () const;

        /**
         * @brief get the scope identifier of the address.
         * @return the scope identifier of the address.
         */
        uint32_t scope () const;

        /**
         * @brief get prefix length from netmask address.
         * @return prefix length.
         */
        int prefix () const;

        /**
         * @brief check if IP address is a wildcard address.
         * @return if wildcard true is returned, false otherwise.
         */
        bool isWildcard () const;

        /**
         * @brief check if IP address is a loopback address.
         * @return if loopback true is returned, false otherwise.
         */
        bool isLoopBack () const;

        /**
         * @brief check if IP address is link local.
         * @return if IP address is link local true is returned, false otherwise.
         */
        bool isLinkLocal () const;

        /**
         * @brief check if IP address is site local (deprecated).
         * @return if IP address is site local true is returned, false otherwise.
         */
        bool isSiteLocal () const;

        /**
         * @brief check if IP address is unique local.
         * @return if IP address is unique local true is returned, false otherwise.
         */
        bool isUniqueLocal () const;

        /**
         * @brief check if IP address is unicast.
         * @return if unicast true is returned, false otherwise.
         */
        bool isUnicast () const;

        /**
         * @brief check if IP address is a broadcast address.
         * @param prefix prefix length.
         * @return if broadcast true is returned, false otherwise.
         */
        bool isBroadcast (int prefix = 0) const;

        /**
         * @brief check if IP address is multicast.
         * @return if multicast true is returned, false otherwise.
         */
        bool isMulticast () const;

        /**
         * @brief check if IP address is global.
         * @return if IP address is global true is returned, false otherwise.
         */
        bool isGlobal () const;

        /**
         * @brief check if the specified string is an IP address.
         * @param address string that may contain an IP address.
         * @return true if the specified string is an IP address, false otherwise.
         */
        static bool isIpAddress (const std::string& address);

        /**
         * @brief check if IP address is an IPv6 address.
         * @return if IP address is an IPv6 address true is returned, false otherwise.
         */
        bool isIpv6Address () const;

        /**
         * @brief check if the specified string is an IPv6 address.
         * @param address string that may contain an IPv6 address.
         * @return true if the specified string is an IPv6 address, false otherwise.
         */
        static bool isIpv6Address (const std::string& address);

        /**
         * @brief check if IP address is IPv4 compatible (deprecated).
         * @return if IPv4 compatible true is returned, false otherwise.
         */
        bool isIpv4Compat () const;

        /**
         * @brief check if IP address is IPv4 mapped.
         * @return if IPv4 mapped true is returned, false otherwise.
         */
        bool isIpv4Mapped () const;

        /**
         * @brief check if IP address is an IPv4 address.
         * @return if IP address is an IPv4 address true is returned, false otherwise.
         */
        bool isIpv4Address () const;

        /**
         * @brief check if the specified string is an IPv4 address.
         * @param address string that may contain an IPv4 address.
         * @return true if the specified string is an IPv4 address, false otherwise.
         */
        static bool isIpv4Address (const std::string& address);

        /**
         * @brief convert IP address to an IPv6 address.
         * @return a valid IPv6 address if IP address is an IPv6 address or an IPv4 mapped IPv6 address if IP address is an IPv4 address.
         */
        IpAddress toIpv6 () const;

        /**
         * @brief convert IP address to an IPv4 address.
         * @return a valid IPv4 address if IP address is an IPv4 address or an IPv4 mapped IPv6 address.
         */
        IpAddress toIpv4 () const;

        /**
         * @brief convert internal address structure to string.
         * @return the internal address structure converted to string.
         */
        std::string toString () const;

        /**
         * @brief convert IP address to the in-addr.arpa or ip6.arpa domain name.
         * @return the converted IP address.
         */
        std::string toArpa () const;

        /**
         * @brief clear IP address (wilcard address).
         */
        void clear ();

        /**
         * @brief get the specified interface IPv4 address.
         * @param interface interface name.
         * @return the specified interface IPv4 address.
         */
        static IpAddress ipv4Address (const std::string& interface);

        /**
         * @brief perform NOT operation on IP address.
         * @return result of NOT operation on IpAddress.
         */
        IpAddress operator~ () const;

        /**
         * @brief returns a reference to the element at the specified location.
         * @param position position of the element to return.
         * @return reference to the requested element.
         * @throw invalid_argument if position is out of range.
         */
        uint8_t& operator[] (size_t position);

       /**
        * @brief returns a reference to the element at the specified location.
        * @param position position of the element to return.
        * @return reference to the requested element.
        * @throw invalid_argument if position is out of range.
        */
        const uint8_t& operator[] (size_t position) const;

        /// wildcard IPv6 address.
        static const IpAddress ipv6Wildcard;

        /// all nodes multicast IPv6 address.
        static const IpAddress ipv6AllNodes;

        /// solicited nodes multicast IPv6 address.
        static const IpAddress ipv6SolicitedNodes;

        /// routers multicast IPv6 address.
        static const IpAddress ipv6Routers;

        /// IPv6 length.
        static const socklen_t ipv6Length = 16;

        /// wildcard IPv4 address.
        static const IpAddress ipv4Wildcard;

        /// broadcast IPv4 address.
        static const IpAddress ipv4Broadcast;

        /// IPv4 length.
        static const socklen_t ipv4Length = 4;

    private:
        /// IP address implementation.
        std::unique_ptr <IpAddressImpl> _ip;
    };

    /**
     * @brief compare if two IP address are equals.
     * @param a address to compare.
     * @param b address to compare to.
     * @return true if equal.
     */
    bool operator== (const IpAddress& a, const IpAddress& b);

    /**
     * @brief compare if two IP address are different.
     * @param a address to compare.
     * @param b address to compare to.
     * @return true if different.
     */
    bool operator!= (const IpAddress& a, const IpAddress& b);

    /**
     * @brief compare if IP address is inferior.
     * @param a address to compare.
     * @param b address to compare to.
     * @return true if inferior.
     */
    bool operator< (const IpAddress& a, const IpAddress& b);

    /**
     * @brief compare if IP address is inferior or equal.
     * @param a address to compare.
     * @param b address to compare to.
     * @return true if inferior or equal.
     */
    bool operator<= (const IpAddress& a, const IpAddress& b);

    /**
     * @brief compare if IP address is superior.
     * @param a address to compare.
     * @param b address to compare to.
     * @return true if superior.
     */
    bool operator> (const IpAddress& a, const IpAddress& b);

    /**
     * @brief compare if IP address is superior or equal.
     * @param a address to compare.
     * @param b address to compare to.
     * @return true if superior or equal.
     */
    bool operator>= (const IpAddress& a, const IpAddress& b);

    /**
     * @brief perform AND operation on IP address.
     * @param a address to compare.
     * @param b address to compare to.
     * @return result of AND operation on IpAddress.
     */
    IpAddress operator& (const IpAddress& a, const IpAddress& b);

    /**
     * @brief perform OR operation on IP address.
     * @param a address to compare.
     * @param b address to compare to.
     * @return result of OR operation on IpAddress.
     */
    IpAddress operator| (const IpAddress& a, const IpAddress& b);

    /**
     * @brief perform XOR operation on IP address.
     * @param a address to compare.
     * @param b address to compare to.
     * @return result of XOR operation on IpAddress.
     */
    IpAddress operator^ (const IpAddress& a, const IpAddress& b);

    /**
     * @brief insert address into stream.
     * @param out output stream.
     * @param address IP address.
     * @return a reference to the output stream.
     */
    std::ostream& operator<< (std::ostream& out, const IpAddress& address);
}

#endif
