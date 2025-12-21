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

#ifndef __JOIN_MACADDRESS_HPP__
#define __JOIN_MACADDRESS_HPP__

// libjoin.
#include <join/ipaddress.hpp>

// C++
#include <array>
#include <string>
#include <ostream>

// C
#include <unistd.h>
#include <net/if.h>

namespace join
{
    /**
     * @brief MAC address class.
     */
    class MacAddress
    {
    public:
        /// case conversion function.
        using CaseConvert = std::ios_base& (std::ios_base&);

        /// iterator from nested container.
        using iterator = std::array <uint8_t, IFHWADDRLEN>::iterator;

        /// constant iterator from nested container.
        using const_iterator = std::array <uint8_t, IFHWADDRLEN>::const_iterator;

        /**
         * @brief create the MacAddress instance (wilcard address).
         */
        MacAddress ();

        /**
         * @brief create the MacAddress instance by copy.
         * @param address MAC address to copy.
         */
        MacAddress (const MacAddress& address);

        /**
         * @brief create the MacAddress instance by move.
         * @param address MAC address to move.
         */
        MacAddress (MacAddress&& address);

        /**
         * @brief create the MacAddress instance using an array.
         * @param address MAC address array.
         * @param size MAC address array size.
         * @throw invalid_argument if out of range.
         */
        MacAddress (const uint8_t* address, size_t size);

        /**
         * @brief create the MacAddress instance using an array.
         * @param address MAC address array.
         * @throw invalid_argument if out of range.
         */
        MacAddress (const uint8_t (&address) [IFHWADDRLEN]);

        /**
         * @brief create the MacAddress instance using an initialization list.
         * @param address MAC address initialization list.
         * @throw invalid_argument if out of range.
         */
        MacAddress (std::initializer_list <uint8_t> address);

        /**
         * @brief create the MacAddress instance using a sockaddr structure.
         * @param address address structure to use.
         * @throw invalid_argument if address is wrong.
         */
        MacAddress (const struct sockaddr& address);

        /**
         * @brief create the MacAddress instance using a string.
         * @param address MAC address string to parse.
         * @throw invalid_argument if MAC address is invalid.
         */
        MacAddress (const char* address);

        /**
         * @brief create the MacAddress instance using a string.
         * @param address MAC address string to parse.
         * @throw invalid_argument if MAC address is invalid.
         */
        MacAddress (const std::string& address);

        /**
         * @brief destroy the MacAddress instance.
         */
        ~MacAddress () = default;

        /**
         * @brief get address family.
         * @return ARPHRD_ETHER.
         */
        int family () const;

        /**
         * @brief get the internal MAC address array address.
         * @return A pointer to the internal MAC address array.
         */
        const uint8_t* addr () const;

        /**
         * @brief get the size in byte of the internal MAC address array.
         * @return The size in byte of the internal MAC address array.
         */
        socklen_t length () const;

        /**
         * @brief check if MAC address is a wildcard address.
         * @return if wildcard true is returned, false otherwise.
         */
        bool isWildcard () const;

        /**
         * @brief check if MAC address is a broadcast address.
         * @return if broadcast true is returned, false otherwise.
         */
        bool isBroadcast () const;

        /**
         * @brief check if the specified string is a MAC address.
         * @param address string that may contain a MAC address.
         * @return true if the specified string is a MAC address, false otherwise.
         */
        static bool isMacAddress (const std::string& address);

        /**
         * @brief convert internal address array to string.
         * @param _case case conversion function (default: std::nouppercase).
         * @return the internal address array converted to string.
         */
        std::string toString (CaseConvert caseConvert = std::nouppercase) const;

        /**
         * @brief convert MAC address to IPv6 address using prefix.
         * @param prefix prefix address.
         * @param len prefix length.
         * @return IPv6 address.
         */
        IpAddress toIpv6 (const IpAddress& prefix, int len) const;

        /**
         * @brief convert MAC address to a link local IPv6 address.
         * @return IPv6 link local address.
         */
        IpAddress toLinkLocalIpv6 () const;

        /**
         * @brief convert MAC address to a unique local IPv6 address using eui-64.
         * @return IPv6 unique local address.
         */
        IpAddress toUniqueLocalIpv6 () const;

        /**
         * @brief clear MAC address (wilcard address).
         */
        void clear ();

        /**
         * @brief returns an iterator to the first element of the nested container.
         * @return iterator to the first element.
         */
        iterator begin ();

        /**
         * @brief returns an iterator to the first element of the nested container.
         * @return iterator to the first element.
         */
        const_iterator begin () const;

        /**
         * @brief returns an iterator to the first element of the nested container.
         * @return iterator to the first element.
         */
        const_iterator cbegin () const;

        /**
         * @brief return an iterator to the element following the last element of the nested container.
         * @return iterator to the element following the last element.
         */
        iterator end ();

        /**
         * @brief return an iterator to the element following the last element of the nested container.
         * @return iterator to the element following the last element.
         */
        const_iterator end () const;

        /**
         * @brief return an iterator to the element following the last element of the nested container.
         * @return iterator to the element following the last element.
         */
        const_iterator cend () const;

        /**
         * @brief get the specified interface MAC address.
         * @param interface interface name.
         * @return the specified interface MAC address.
         */
        static MacAddress address (const std::string& interface);

        /**
         * @brief assign the MacAddress instance by copy.
         * @param address MAC address to copy.
         * @return a reference of the current object.
         */
        MacAddress& operator= (const MacAddress& address);

        /**
         * @brief assign the MacAddress instance by move.
         * @param address MAC address to move.
         * @return a reference of the current object.
         */
        MacAddress& operator= (MacAddress&& address);

        /**
         * @brief assign the MacAddress using an initialization list.
         * @param address MAC address initialization list.
         * @return a reference of the current object.
         * @throw invalid_argument if out of range.
         */
        MacAddress& operator= (std::initializer_list <uint8_t> address);

        /**
         * @brief assign the MacAddress using a sockaddr structure.
         * @param address address structure to use.
         * @return a reference of the current object.
         * @throw invalid_argument if address is wrong.
         */
        MacAddress& operator= (const struct sockaddr& address);

        /**
         * @brief add the provided value to the current MAC address.
         * @param value value to add to the current MAC address.
         * @return a reference of the current object.
         */
        MacAddress& operator+= (int value);

        /**
         * @brief perform pre-increment operation on MAC address.
         * @return a reference of the current object.
         */
        MacAddress& operator++ ();

        /**
         * @brief perform post-increment operation on MAC address.
         * @return a copy of the original object.
         */
        MacAddress operator++ (int);

        /**
         * @brief returns a reference to the element at the specified location.
         * @param position position of the element to return.
         * @return peference to the requested element.
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

        /**
         * @brief perform NOT operation on MAC address.
         * @return result of NOT operation on MacAddress.
         */
        MacAddress operator~ () const;

        /// wildcard MAC address.
        static const MacAddress wildcard;

        /// broadcast MAC address.
        static const MacAddress broadcast;

    private:
        /// MAC address.
        std::array <uint8_t, IFHWADDRLEN> _mac = {};
    };

    /**
     * @brief add the provided value to the provided MAC address.
     * @param value value to add to MAC address.
     * @param a MAC address.
     * @return result of the addition operation on MacAddress.
     */
    MacAddress operator+ (int value, const MacAddress& a);

    /**
     * @brief add the provided value to the provided MAC address.
     * @param a MAC address.
     * @param value value to add to MAC address.
     * @return result of the addition operation on MacAddress.
     */
    MacAddress operator+ (const MacAddress& a, int value);

    /**
     * @brief compare if two MAC address are equals.
     * @param a MAC address to compare.
     * @param b MAC address to compare to.
     * @return true if equal.
     */
    bool operator== (const MacAddress& a, const MacAddress& b);

    /**
     * @brief compare if two MAC address are different.
     * @param a MAC address to compare.
     * @param b MAC address to compare to.
     * @return true if different.
     */
    bool operator!= (const MacAddress& a, const MacAddress& b);

    /**
     * @brief compare if MAC address is inferior.
     * @param a MAC address to compare.
     * @param b MAC address to compare to.
     * @return true if inferior.
     */
    bool operator< (const MacAddress& a, const MacAddress& b);

    /**
     * @brief compare if MAC address is inferior or equal.
     * @param a MAC address to compare.
     * @param b MAC address to compare to.
     * @return true if inferior or equal.
     */
    bool operator<= (const MacAddress& a, const MacAddress& b);

    /**
     * @brief compare if MAC address is superior.
     * @param a MAC address to compare.
     * @param b MAC address to compare to.
     * @return true if superior.
     */
    bool operator> (const MacAddress& a, const MacAddress& b);

    /**
     * @brief compare if MAC address is superior or equal.
     * @param a MAC address to compare.
     * @param b MAC address to compare to.
     * @return true if superior or equal.
     */
    bool operator>= (const MacAddress& a, const MacAddress& b);

    /**
     * @brief perform AND operation on MAC address.
     * @param a MAC address to compare.
     * @param b MAC address to compare to.
     * @return result of AND operation on MacAddress.
     */
    MacAddress operator& (const MacAddress& a, const MacAddress& b);

    /**
     * @brief perform OR operation on MAC address.
     * @param a MAC address to compare.
     * @param b MAC address to compare to.
     * @return result of OR operation on MacAddress.
     */
    MacAddress operator| (const MacAddress& a, const MacAddress& b);

    /**
     * @brief perform XOR operation on MAC address.
     * @param a MAC address to compare.
     * @param b MAC address to compare to.
     * @return result of XOR operation on MacAddress.
     */
    MacAddress operator^ (const MacAddress& a, const MacAddress& b);

    /**
     * @brief insert MAC address into stream.
     * @param out output stream.
     * @param a MAC address.
     * @return a reference to the output stream.
     */
    std::ostream& operator<< (std::ostream& out, const MacAddress& a);
}

#endif
