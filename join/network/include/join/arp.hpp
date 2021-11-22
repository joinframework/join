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
         * @brief destroy the Arp instance.
         */
        virtual ~Arp () = default;

        /**
         * @brief get the MAC address for the given IP address using ARP cache or ARP request.
         * @param ip IP address.
         * @return the MAC address.
         */
        MacAddress get (const IpAddress& ip);

        /**
         * @brief discover the MAC address for the given internet layer address.
         * @param ip IP address.
         * @param interface interface name.
         * @return the MAC address.
         */
        static MacAddress get (const IpAddress& ip, const std::string& interface);

        /**
         * @brief get the MAC address for the given IP address using ARP request.
         * @param ip IP address.
         * @return the MAC address.
         */
        MacAddress request (const IpAddress& ip);

        /**
         * @brief get the MAC address for the given IP address using ARP request.
         * @param ip IP address.
         * @param interface interface name.
         * @return the MAC address.
         */
        static MacAddress request (const IpAddress& ip, const std::string& interface);

        /**
         * @brief add entry the MAC address of the given IP address to ARP cache.
         * @param ip MAC address.
         * @param ip IP address.
         * @return 0 on success, -1 on failure.
         */
        int add (const MacAddress& mac, const IpAddress& ip);

        /**
         * @brief add entry the MAC address of the given IP address to ARP cache.
         * @param ip MAC address.
         * @param ip IP address.
         * @param interface interface name.
         * @return 0 on success, -1 on failure.
         */
        static int add (const MacAddress& mac, const IpAddress& ip, const std::string& interface);

        /**
         * @brief get the MAC address for the given IP address using ARP cache.
         * @param ip IP address.
         * @return the MAC address.
         */
        MacAddress cache (const IpAddress& ip);

        /**
         * @brief get the MAC address for the given IP address using ARP cache.
         * @param ip IP address.
         * @param interface interface name.
         * @return the MAC address.
         */
        static MacAddress cache (const IpAddress& ip, const std::string& interface);

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
