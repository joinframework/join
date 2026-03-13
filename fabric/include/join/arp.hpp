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

#ifndef JOIN_FABRIC_ARP_HPP
#define JOIN_FABRIC_ARP_HPP

// libjoin.
#include <join/neighbormanager.hpp>
#include <join/macaddress.hpp>
#include <join/condition.hpp>
#include <join/socket.hpp>

// C++.
#include <unordered_map>
#include <string>

// C.
#include <linux/filter.h>
#include <net/if_arp.h>
#include <net/if.h>

namespace join
{
    /**
     * @brief ARP protocol class.
     */
    class Arp : public Raw::Socket
    {
    public:
        /**
         * @brief create the Arp instance.
         */
        Arp () = delete;

        /**
         * @brief create the Arp instance.
         * @param interface interface name.
         * @param reactor event loop reactor.
         */
        Arp (const std::string& interface, Reactor* reactor = nullptr);

        /**
         * @brief destroy the Arp instance.
         */
        ~Arp () = default;

        /**
         * @brief get the MAC address for the given IP address using netlink neighbor cache or ARP request.
         * @param ip IP address.
         * @param timeout request timeout.
         * @return the MAC address.
         */
        template <typename Rep, typename Period>
        MacAddress get (const IpAddress& ip, std::chrono::duration<Rep, Period> timeout)
        {
            if (ip.family () != AF_INET)
            {
                lastError = make_error_code (Errc::InvalidParam);
                return {};
            }

            if (ip == IpAddress::ipv4Address (_interface))
            {
                return MacAddress::address (_interface);
            }

            MacAddress mac = cache (ip);

            return mac.isWildcard () ? request (ip, timeout) : mac;
        }

        /**
         * @brief get the MAC address for the given IP address using ARP cache or ARP request.
         * @param ip IP address.
         * @return the MAC address.
         */
        MacAddress get (const IpAddress& ip)
        {
            return get (ip, std::chrono::seconds (5));
        }

        /**
         * @brief discover the MAC address for the given internet layer address.
         * @param interface interface name.
         * @param ip IP address.
         * @param timeout request timeout.
         * @return the MAC address.
         */
        template <typename Rep, typename Period>
        static MacAddress get (const std::string& interface, const IpAddress& ip,
                               std::chrono::duration<Rep, Period> timeout)
        {
            return Arp (interface).get (ip, timeout);
        }

        /**
         * @brief discover the MAC address for the given internet layer address.
         * @param interface interface name.
         * @param ip IP address.
         * @return the MAC address.
         */
        static MacAddress get (const std::string& interface, const IpAddress& ip)
        {
            return get (interface, ip, std::chrono::seconds (5));
        }

        /**
         * @brief get the MAC address for the given IP address using ARP request.
         * @param ip IP address.
         * @param timeout request timeout.
         * @return the MAC address.
         */
        template <typename Rep, typename Period>
        MacAddress request (const IpAddress& ip, std::chrono::duration<Rep, Period> timeout)
        {
            if (ip.family () != AF_INET)
            {
                lastError = make_error_code (Errc::InvalidParam);
                return {};
            }

            if (bind (_interface) == -1 || setOption (Raw::Socket::Broadcast, 1) == -1)
            {
                return {};
            }

            // accept only ARP replies.
            struct sock_filter code[] = {
                {0x28, 0, 0, 0x0000000c}, {0x15, 0, 3, 0x00000806}, {0x28, 0, 0, 0x00000014},
                {0x15, 0, 1, 0x00000002}, {0x6, 0, 0, 0x00040000},  {0x6, 0, 0, 0x00000000},
            };

            struct sock_fprog bpf;
            bpf.len    = 6;
            bpf.filter = code;

            // best effort, validation is done in onReceive anyway.
            ::setsockopt (handle (), SOL_SOCKET, SO_ATTACH_FILTER, &bpf, sizeof (bpf));

            Packet out              = {};
            const MacAddress srcMac = MacAddress::address (_interface);
            const IpAddress srcIp   = IpAddress::ipv4Address (_interface);

            ::memcpy (out.eth.h_dest, MacAddress::broadcast.addr (), ETH_ALEN);
            ::memcpy (out.eth.h_source, srcMac.addr (), ETH_ALEN);
            out.eth.h_proto = ::htons (ETH_P_ARP);

            out.arp.ar_hrd = ::htons (ARPHRD_ETHER);
            out.arp.ar_pro = ::htons (ETH_P_IP);
            out.arp.ar_hln = ETH_ALEN;
            out.arp.ar_pln = 4;
            out.arp.ar_op  = ::htons (ARPOP_REQUEST);
            ::memcpy (out.arp.ar_sha, srcMac.addr (), ETH_ALEN);
            ::memcpy (&out.arp.ar_sip, srcIp.addr (), sizeof (uint32_t));
            ::memcpy (out.arp.ar_tha, MacAddress::wildcard.addr (), ETH_ALEN);
            ::memcpy (&out.arp.ar_tip, ip.addr (), sizeof (uint32_t));

            ScopedLock<Mutex> lock (_syncMutex);

            if (write (reinterpret_cast<const char*> (&out), sizeof (Packet)) == -1)
            {
                close ();
                return {};
            }

            _reactor->addHandler (this);
            MacAddress mac = waitResponse (lock, out.arp.ar_tip, timeout);
            _reactor->delHandler (this);

            close ();
            return mac;
        }

        /**
         * @brief get the MAC address for the given IP address using ARP request.
         * @param ip IP address.
         * @return the MAC address.
         */
        MacAddress request (const IpAddress& ip)
        {
            return request (ip, std::chrono::seconds (5));
        }

        /**
         * @brief get the MAC address for the given IP address using ARP request.
         * @param interface interface name.
         * @param ip IP address.
         * @param timeout request timeout.
         * @return the MAC address.
         */
        template <typename Rep, typename Period>
        static MacAddress request (const std::string& interface, const IpAddress& ip,
                                   std::chrono::duration<Rep, Period> timeout)
        {
            return Arp (interface).request (ip, timeout);
        }

        /**
         * @brief get the MAC address for the given IP address using ARP request.
         * @param interface interface name.
         * @param ip IP address.
         * @return the MAC address.
         */
        static MacAddress request (const std::string& interface, const IpAddress& ip)
        {
            return request (interface, ip, std::chrono::seconds (5));
        }

        /**
         * @brief add entry the MAC address of the given IP address to ARP cache.
         * @param mac MAC address.
         * @param ip IP address.
         * @return 0 on success, -1 on failure.
         */
        int add (const MacAddress& mac, const IpAddress& ip);

        /**
         * @brief add entry the MAC address of the given IP address to ARP cache.
         * @param interface interface name.
         * @param mac MAC address.
         * @param ip IP address.
         * @return 0 on success, -1 on failure.
         */
        static int add (const std::string& interface, const MacAddress& mac, const IpAddress& ip);

        /**
         * @brief remove the MAC address of the given IP address from ARP cache.
         * @param ip IP address.
         * @return 0 on success, -1 on failure.
         */
        int remove (const IpAddress& ip);

        /**
         * @brief remove the MAC address of the given IP address from ARP cache.
         * @param interface interface name.
         * @param ip IP address.
         * @return 0 on success, -1 on failure.
         */
        static int remove (const std::string& interface, const IpAddress& ip);

        /**
         * @brief get the MAC address for the given IP address using ARP cache.
         * @param ip IP address.
         * @return the MAC address.
         */
        MacAddress cache (const IpAddress& ip);

        /**
         * @brief get the MAC address for the given IP address using ARP cache.
         * @param interface interface name.
         * @param ip IP address.
         * @return the MAC address.
         */
        static MacAddress cache (const std::string& interface, const IpAddress& ip);

    private:
        /**
         * @brief arp packet.
         */
        struct __attribute__ ((packed)) ArpPacket
        {
            uint16_t ar_hrd;
            uint16_t ar_pro;
            uint8_t ar_hln;
            uint8_t ar_pln;
            uint16_t ar_op;
            uint8_t ar_sha[ETH_ALEN];
            uint32_t ar_sip;
            uint8_t ar_tha[ETH_ALEN];
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

        /**
         * @brief wait for ARP response.
         * @param lock mutex previously locked by the calling thread.
         * @param tip target IP.
         * @param timeout wait timeout.
         * @return the MAC address.
         */
        template <typename Rep, typename Period>
        MacAddress waitResponse (ScopedLock<Mutex>& lock, uint32_t tip, std::chrono::duration<Rep, Period> timeout)
        {
            auto inserted = _pending.emplace (tip, std::make_unique<PendingRequest> ());
            if (!inserted.second)
            {
                // LCOV_EXCL_START
                lastError = make_error_code (Errc::OperationFailed);
                return {};
                // LCOV_EXCL_STOP
            }

            if (!inserted.first->second->cond.timedWait (lock, timeout))
            {
                _pending.erase (inserted.first);
                lastError = std::make_error_code (std::errc::no_such_device_or_address);
                return {};
            }

            MacAddress mac = inserted.first->second->mac;
            _pending.erase (inserted.first);

            return mac;
        }

        /**
         * @brief method called when data are ready to be read.
         */
        void onReceive () noexcept override;

        /// buffer size.
        static constexpr size_t _bufferSize = 4096;

        /**
         * @brief pending request.
         */
        struct PendingRequest
        {
            Condition cond;
            MacAddress mac;
        };

        /// pending request.
        std::unordered_map<uint32_t, std::unique_ptr<PendingRequest>> _pending;

        /// mutex for synchronous operations.
        Mutex _syncMutex;

        /// interface name.
        const std::string _interface;

        /// event loop reactor.
        Reactor* const _reactor;

        /// neighbor manager
        NeighborManager _neighbors;
    };
}

#endif
