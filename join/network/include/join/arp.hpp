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

#ifndef __JOIN_ARP_HPP__
#define __JOIN_ARP_HPP__

// libjoin.
#include <join/macaddress.hpp>

// C++.
#include <string>

// C.
#include <linux/if_arp.h>

namespace join
{
    /**
     * @brief arp protocol class.
     */
    class Arp
    {
    public:
        /**
         * @brief create the Arp instance.
         */
        Arp () = delete;

        /**
         * @brief create the Arp instance.
         * @param interface interface name.
         */
        Arp (const std::string& interface);

        /**
         * @brief create the Arp instance by copy.
         * @param other other object to copy.
         */
        Arp (const Arp& other);

        /**
         * @brief assign the Arp instance by copy.
         * @param other other object to copy.
         * @return a reference to the current object.
         */
        Arp& operator= (const Arp& other);

        /**
         * @brief create the Arp instance by move.
         * @param other other object to move.
         */
        Arp (Arp&& other);

        /**
         * @brief assign the Arp instance by move.
         * @param other other object to move.
         * @return a reference to the current object.
         */
        Arp& operator= (Arp&& other);

        /**
         * @brief destroy the Arp instance.
         */
        virtual ~Arp () = default;

        /**
         * @brief discover the link layer address for the given ip address using arp cache.
         * @param addr ip address.
         * @param interface interface name.
         * @return the discovered link layer address.
         */
        MacAddress cache (const IpAddress& addr);

        /**
         * @brief discover the link layer address for the given ip address using arp request.
         * @param addr ip address.
         * @param interface interface name.
         * @return the dicovered link layer address.
         */
        MacAddress request (const IpAddress& addr);

        /**
         * @brief discover the link layer address for the given internet layer address.
         * @param addr Iternet layer address.
         * @return The dicovered link layer address.
         */
        MacAddress mac (const IpAddress& addr);

        /**
         * @brief discover the link layer address for the given internet layer address.
         * @param addr Iternet layer address.
         * @return The dicovered link layer address.
         */
        static MacAddress mac (const IpAddress& addr, const std::string& interface);

    private:
        /**
         * @brief arp packet.
         */
        struct __attribute__ ((packed)) ArpPacket
        {
            uint16_t ar_hrd;
            uint16_t ar_pro;
            uint8_t  ar_hln;
            uint8_t  ar_pln;
            uint16_t ar_op;
            uint8_t  ar_sha[ETH_ALEN];
            uint32_t ar_sip;
            uint8_t  ar_tha[ETH_ALEN];
            uint32_t ar_tip;
        };

        /**
         * @brief full arp packet.
         */
        struct __attribute__ ((packed)) Packet
        {
            struct ethhdr eth;
            ArpPacket arp;
        };

        /// interface name.
        std::string _interface;
    };
}

#endif
