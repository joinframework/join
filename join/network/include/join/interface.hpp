/**
 * MIT License
 *
 * Copyright (c) 2025 Mathieu Rabine
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

#ifndef __JOIN_INTERFACE_HPP__
#define __JOIN_INTERFACE_HPP__

// libjoin.
#include <join/macaddress.hpp>
#include <join/ipaddress.hpp>

// C++.
#include <string>
#include <memory>
#include <vector>
#include <tuple>
#include <set>

// C.
#include <cstdint>

namespace join
{
    /// forward declaration.
    class InterfaceManager;

    /**
     * @brief interface class.
     */
    class Interface
    {
    private:
        /**
         * @brief create instance.
         * @param manager interface manager.
         * @param index interface index.
         */
        explicit Interface (std::weak_ptr <InterfaceManager> manager, uint32_t index);

    public:
        using Ptr           = std::shared_ptr <Interface>;
        using Address       = std::tuple <IpAddress, uint32_t, IpAddress>;
        using AddressList   = std::vector <Address>;
        using Route         = std::tuple <IpAddress, uint32_t, IpAddress, uint32_t>;
        using RouteList     = std::vector <Route>;

        /**
         * @brief create instance.
         */
        Interface () = delete;

        /**
         * @brief destroy instance.
         */
        ~Interface () = default;

        /**
         * @brief get interface index.
         * @return interface index.
         */
        uint32_t index () const noexcept;

        /**
         * @brief get master index if bridged.
         * @return master index.
         */
        uint32_t master () const noexcept;

        /**
         * @brief get interface name.
         * @return interface name.
         */
        const std::string& name () const noexcept;

        /**
         * @brief set interface mtu.
         * @param mtuBytes new mtu value.
         * @param sync wait for operation completion if true.
         * @return 0 on success, -1 on failure.
         */
        int mtu (uint32_t mtuBytes, bool sync = false);

        /**
         * @brief get interface mtu.
         * @return interface mtu.
         */
        uint32_t mtu () const noexcept;

        /**
         * @brief get interface kind.
         * @return interface kind.
         */
        const std::string& kind () const noexcept;

        /**
         * @brief set interface mac address.
         * @param macAddress new mac address.
         * @param sync wait for operation completion if true.
         * @return 0 on success, -1 on failure.
         */
        int mac (const MacAddress& macAddress, bool sync = false);

        /**
         * @brief get interface mac address.
         * @return interface mac address.
         */
        const MacAddress& mac () const noexcept;

        /**
         * @brief add address to interface.
         * @param ipAddress ip address to add.
         * @param prefix prefix length.
         * @param broadcast broadcast address .
         * @param sync wait for operation completion if true.
         * @return 0 on success, -1 on failure.
         */
        int addAddress (const IpAddress& ipAddress, uint32_t prefix, const IpAddress& broadcast = {}, bool sync = false);

        /**
         * @brief add address to interface.
         * @param address address to add.
         * @param sync wait for operation completion if true.
         * @return 0 on success, -1 on failure.
         */
        int addAddress (const Address& address, bool sync = false);

        /**
         * @brief remove address from interface.
         * @param ipAddress ip address to delete.
         * @param prefix prefix length.
         * @param broadcast broadcast address .
         * @param sync wait for operation completion if true.
         * @return 0 on success, -1 on failure.
         */
        int removeAddress (const IpAddress& ipAddress, uint32_t prefix, const IpAddress& broadcast = {}, bool sync = false);

        /**
         * @brief remove address from interface.
         * @param address address to delete.
         * @param sync wait for operation completion if true.
         * @return 0 on success, -1 on failure.
         */
        int removeAddress (const Address& address, bool sync = false);

        /**
         * @brief get interface ip addresses.
         * @return interface ip addresses.
         */
        const AddressList& addressList () const noexcept;

        /**
         * @brief check if interface has address stored.
         * @param ipAddress ip address to delete.
         * @return true if interface has address stored, false otherwise
         */
        bool hasAddress (const IpAddress& ipAddress);

        /**
         * @brief add route to interface.
         * @param dest destination network.
         * @param prefix prefix length.
         * @param gateway gateway address.
         * @param metric route metric.
         * @param sync wait for operation completion if true.
         * @return 0 on success, -1 on failure.
         */
        int addRoute (const IpAddress& dest, uint32_t prefix, const IpAddress& gateway = {}, uint32_t metric = 0, bool sync = false);

        /**
         * @brief add route to interface.
         * @param route route.
         * @param sync wait for operation completion if true.
         * @return 0 on success, -1 on failure.
         */
        int addRoute (const Route& route, bool sync = false);

        /**
         * @brief remove route from interface.
         * @param dest destination network.
         * @param prefix prefix length.
         * @param gateway gateway address.
         * @param metric route metric.
         * @param sync wait for operation completion if true.
         * @return 0 on success, -1 on failure.
         */
        int removeRoute (const IpAddress& dest, uint32_t prefix, const IpAddress& gateway = {}, uint32_t metric = 0, bool sync = false);

        /**
         * @brief remove route from interface.
         * @param route route.
         * @param sync wait for operation completion if true.
         * @return 0 on success, -1 on failure.
         */
        int removeRoute (const Route& route, bool sync = false);

        /**
         * @brief get interface routes.
         * @return interface routes.
         */
        const RouteList& routeList () const noexcept;

        /**
         * @brief check if interface has route stored.
         * @param dest destination network.
         * @param prefix prefix length.
         * @param gateway gateway address.
         * @param metric route metric.
         * @return 0 on success, -1 on failure.
         */
        bool hasRoute (const IpAddress& dest, uint32_t prefix, const IpAddress& gateway, uint32_t metric);

        /**
         * @brief check if interface has route stored.
         * @param route route.
         * @return true if interface has route stored, false otherwise
         */
        bool hasRoute (const Route& route);

        /**
         * @brief add interface to bridge.
         * @param masterIndex bridge index.
         * @param sync wait for operation completion if true.
         * @return 0 on success, -1 on failure.
         */
        int addToBridge (uint32_t masterIndex, bool sync = false);

        /**
         * @brief add interface to bridge.
         * @param masterName bridge name.
         * @param sync wait for operation completion if true.
         * @return 0 on success, -1 on failure.
         */
        int addToBridge (const std::string& masterName, bool sync = false);

        /**
         * @brief remove interface from bridge.
         * @param sync wait for operation completion if true.
         * @return 0 on success, -1 on failure.
         */
        int removeFromBridge (bool sync = false);

        /**
         * @brief get interface flags.
         * @return interface flags.
         */
        uint32_t flags () const noexcept;

        /**
         * @brief enable interface.
         * @param enabled true to enable interface, false otherwise.
         * @param sync wait for operation completion if true.
         * @return 0 on success, -1 on failure.
         */
        int enable (bool enabled = true, bool sync = false);

        /**
         * @brief is interface enabled.
         * @return true if enabled.
         */
        bool isEnabled () const noexcept;

        /**
         * @brief is interface running.
         * @return true if running.
         */
        bool isRunning () const noexcept;

        /**
         * @brief is interface a loopback interface.
         * @return true if loopback interface.
         */
        bool isLoopback () const noexcept;

        /**
         * @brief is interface a point to point interface.
         * @return true if point to point interface.
         */
        bool isPointToPoint () const noexcept;

        /**
         * @brief is interface a dummy interface.
         * @return true if dummy interface.
         */
        bool isDummy () const noexcept;

        /**
         * @brief is interface a bridge interface.
         * @return true if bridge interface.
         */
        bool isBridge () const noexcept;

        /**
         * @brief is interface a vlan interface.
         * @return true if vlan interface.
         */
        bool isVlan () const noexcept;

        /**
         * @brief is interface a veth interface.
         * @return true if veth interface.
         */
        bool isVeth () const noexcept;

        /**
         * @brief is interface a gre interface.
         * @return true if gre interface.
         */
        bool isGre () const noexcept;

        /**
         * @brief is interface a tun interface.
         * @return true if tun interface.
         */
        bool isTun () const noexcept;

        /**
         * @brief is interface supporting broadcast.
         * @return true if supporting broadcast.
         */
        bool supportsBroadcast () const noexcept;

        /**
         * @brief is interface supporting multicast.
         * @return true if supporting multicast.
         */
        bool supportsMulticast () const noexcept;

        /**
         * @brief is interface supporting IPv4.
         * @return true if supporting IPv4.
         */
        bool supportsIpv4 ();

        /**
         * @brief is interface supporting IPv6.
         * @return true if supporting IPv6.
         */
        bool supportsIpv6 ();

    private:
        /// interface manager
        std::weak_ptr <InterfaceManager> _manager;

        /// interface index.
        uint32_t _index = 0;

        /// master index.
        uint32_t _master = 0;

        /// interface name.
        std::string _name;

        /// interface kind.
        std::string _kind;

        /// interface mtu.
        uint32_t _mtu = 0;

        /// interface flags.
        uint32_t _flags = 0;

        /// interface mac address.
        MacAddress _mac;

        /// interface addresses.
        AddressList _addresses;

        /// interface routes.
        RouteList _routes;

        /// protection mutex.
        Mutex _mutex;

        // friendship with equal operator.
        friend bool operator== (const Interface::Ptr& lhs, const Interface::Ptr& rhs);

        // friendship with not equal operator.
        friend bool operator< (const Interface::Ptr& lhs, const Interface::Ptr& rhs);

        // friendship with interface manager.
        friend class InterfaceManager;
    };

    /**
     * @brief compare if two interfaces are equals.
     * @param a Interface to compare.
     * @param b Interface to compare to.
     * @return true if equal.
     */
    inline bool operator== (const Interface::Ptr& lhs, const Interface::Ptr& rhs)
    {
        if (!lhs && !rhs)
        {
            return true;
        }
        if (!lhs || !rhs)
        {
            return false;
        }
        return lhs->_index == rhs->_index;
    }

    /**
     * @brief compare if interface is inferior.
     * @param a Interface to compare.
     * @param b Interface to compare to.
     * @return true if inferior.
     */
    inline bool operator< (const Interface::Ptr& lhs, const Interface::Ptr& rhs)
    {
        if (!lhs)
        {
            return true;
        }
        if (!rhs)
        {
            return false;
        }
        return lhs->_index < rhs->_index;
    }

    /// list of interfaces.
    using InterfaceList = std::set <Interface::Ptr>;
}

#endif
