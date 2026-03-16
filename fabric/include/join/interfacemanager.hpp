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

#ifndef JOIN_FABRIC_INTERFACEMANAGER_HPP
#define JOIN_FABRIC_INTERFACEMANAGER_HPP

// libjoin.
#include <join/netlinkmanager.hpp>
#include <join/macaddress.hpp>
#include <join/ipaddress.hpp>
#include <join/interface.hpp>

// C++.
#include <functional>
#include <string>

namespace join
{
    /**
     * @brief enumeration of interface change type.
     */
    enum class InterfaceChangeType : uint32_t
    {
        Added             = 1L << 0, /**< interface did not exist before. */
        Deleted           = 1L << 1, /**< interface was removed. */
        Modified          = 1L << 2, /**< interface was updated. */
        AdminStateChanged = 1L << 3, /**< interface admin status was updated. */
        OperStateChanged  = 1L << 4, /**< interface operational status was updated. */
        MacChanged        = 1L << 5, /**< interface MAC address changed. */
        NameChanged       = 1L << 6, /**< interface name changed. */
        MtuChanged        = 1L << 7, /**< interface MTU changed. */
        KindChanged       = 1L << 8, /**< interface kind changed. */
        MasterChanged     = 1L << 9, /**< interface master bridge changed. */
    };

    /**
     * @brief perform binary AND on InterfaceChangeType.
     * @param a bitset.
     * @param b other bitset.
     * @return bitset result of binary AND on InterfaceChangeType.
     */
    inline InterfaceChangeType operator& (InterfaceChangeType a, InterfaceChangeType b)
    {
        return InterfaceChangeType (static_cast<uint32_t> (a) & static_cast<uint32_t> (b));
    }

    /**
     * @brief perform binary OR on InterfaceChangeType.
     * @param a bitset.
     * @param b other bitset.
     * @return bitset result of binary OR on InterfaceChangeType.
     */
    inline InterfaceChangeType operator| (InterfaceChangeType a, InterfaceChangeType b)
    {
        return InterfaceChangeType (static_cast<uint32_t> (a) | static_cast<uint32_t> (b));
    }

    /**
     * @brief perform binary XOR on InterfaceChangeType.
     * @param a bitset.
     * @param b other bitset.
     * @return bitset result of binary XOR on InterfaceChangeType.
     */
    inline InterfaceChangeType operator^ (InterfaceChangeType a, InterfaceChangeType b)
    {
        return InterfaceChangeType (static_cast<uint32_t> (a) ^ static_cast<uint32_t> (b));
    }

    /**
     * @brief perform binary NOT on InterfaceChangeType.
     * @param a bitset.
     * @return bitset result of binary NOT on InterfaceChangeType.
     */
    inline InterfaceChangeType operator~(InterfaceChangeType a)
    {
        return InterfaceChangeType (~static_cast<uint32_t> (a));
    }

    /**
     * @brief perform binary AND on InterfaceChangeType.
     * @param a bitset.
     * @param b other bitset.
     * @return bitset result of binary AND on InterfaceChangeType.
     */
    inline const InterfaceChangeType& operator&= (InterfaceChangeType& a, InterfaceChangeType b)
    {
        return a = a & b;
    }

    /**
     * @brief perform binary OR on InterfaceChangeType.
     * @param a bitset.
     * @param b other bitset.
     * @return bitset result of binary OR.
     */
    inline const InterfaceChangeType& operator|= (InterfaceChangeType& a, InterfaceChangeType b)
    {
        return a = a | b;
    }

    /**
     * @brief perform binary XOR on InterfaceChangeType.
     * @param a bitset.
     * @param b other bitset.
     * @return bitset result of binary XOR on InterfaceChangeType.
     */
    inline const InterfaceChangeType& operator^= (InterfaceChangeType& a, InterfaceChangeType b)
    {
        return a = a ^ b;
    }

    /**
     * @brief link information.
     */
    struct LinkInfo
    {
        Interface::Ptr interface;  /**< interface. */
        InterfaceChangeType flags; /**< what changed (bitmask). */
    };

    /**
     * @brief address information.
     */
    struct AddressInfo : public LinkInfo
    {
        Interface::Address address; /**< address changed. */
    };

    /**
     * @brief route information.
     */
    struct RouteInfo : public LinkInfo
    {
        Interface::Route route; /**< route changed. */
    };

    /**
     * @brief interface manager class.
     */
    class InterfaceManager : public NetlinkManager
    {
    public:
        using LinkNotify    = std::function<void (const LinkInfo& info)>;
        using AddressNotify = std::function<void (const AddressInfo& info)>;
        using RouteNotify   = std::function<void (const RouteInfo& info)>;

        /**
         * @brief create instance.
         * @param reactor event loop reactor.
         */
        InterfaceManager (Reactor* reactor = nullptr);

        /**
         * @brief create instance by copy.
         * @param other other interface to copy.
         */
        InterfaceManager (const InterfaceManager& other) = delete;

        /**
         * @brief create instance by move.
         * @param other other interface to move.
         */
        InterfaceManager (InterfaceManager&& other) = delete;

        /**
         * @brief assign instance by copy.
         * @param other other interface to copy.
         * @return A reference of the current object.
         */
        InterfaceManager& operator= (const InterfaceManager& other) = delete;

        /**
         * @brief assign instance by move.
         * @param other other interface to move.
         * @return A reference of the current object.
         */
        InterfaceManager& operator= (InterfaceManager&& other) = delete;

        /**
         * @brief destroy instance.
         */
        ~InterfaceManager ();

        /**
         * @brief get the a singleton instance.
         * @return reference to the singleton instance.
         */
        static InterfaceManager& instance ();

        /**
         * @brief find interface by index.
         * @param interfaceIndex interface index.
         * @return interface.
         */
        Interface::Ptr findByIndex (uint32_t interfaceIndex);

        /**
         * @brief find interface by name.
         * @param interfaceName interface name.
         * @return interface.
         */
        Interface::Ptr findByName (const std::string& interfaceName);

        /**
         * @brief enumerate all interfaces.
         * @return all interfaces.
         */
        InterfaceList enumerate ();

        /**
         * @brief refresh all data.
         * @return 0 on success, -1 on failure.
         */
        int refresh ();

        /**
         * @brief registers a callback to be invoked when a link update occurs.
         * @param cb the callback function to register.
         * @return unique id for the callback.
         */
        uint64_t addLinkListener (const LinkNotify& cb);

        /**
         * @brief unregisters a previously registered link update callback.
         * @param id unique id of the callback function to remove.
         */
        void removeLinkListener (uint64_t id);

        /**
         * @brief registers a callback to be invoked when a address update occurs.
         * @param cb the callback function to register.
         * @return unique id for the callback.
         */
        uint64_t addAddressListener (const AddressNotify& cb);

        /**
         * @brief unregisters a previously registered address update callback.
         * @param id unique id of the callback function to remove.
         */
        void removeAddressListener (uint64_t id);

        /**
         * @brief registers a callback to be invoked when a route update occurs.
         * @param cb the callback function to register.
         * @return unique id for the callback.
         */
        uint64_t addRouteListener (const RouteNotify& cb);

        /**
         * @brief unregisters a previously registered route update callback.
         * @param id unique id of the callback function to remove.
         */
        void removeRouteListener (uint64_t id);

        /**
         * @brief creates a dummy interface.
         * @param interfaceName name of the dummy interface to be created.
         * @param sync wait for completion.
         * @return 0 on success, -1 on failure.
         */
        int createDummyInterface (const std::string& interfaceName, bool sync = false);

        /**
         * @brief creates a bridge interface.
         * @param interfaceName name of the bridge interface to be created.
         * @param sync wait for completion.
         * @return 0 on success, -1 on failure.
         */
        int createBridgeInterface (const std::string& interfaceName, bool sync = false);

        /**
         * @brief creates a VLAN interface.
         * @param interfaceName name of the VLAN interface to be created.
         * @param parentIndex index of the physical interface to create VLAN on.
         * @param id VLAN id (must be unique on the parent interface).
         * @param proto VLAN protocol type.
         * @param sync wait for completion.
         * @return 0 on success, -1 on failure.
         */
        int createVlanInterface (const std::string& interfaceName, uint32_t parentIndex, uint16_t id,
                                 uint16_t proto = ETH_P_8021Q, bool sync = false);

        /**
         * @brief creates a VLAN interface.
         * @param interfaceName name of the VLAN interface to be created.
         * @param parentName name of the physical interface to create VLAN on.
         * @param id VLAN id (must be unique on the parent interface).
         * @param proto VLAN protocol type.
         * @param sync wait for completion.
         * @return 0 on success, -1 on failure.
         */
        int createVlanInterface (const std::string& interfaceName, const std::string& parentName, uint16_t id,
                                 uint16_t proto = ETH_P_8021Q, bool sync = false);

        /**
         * @brief creates a Virtual Ethernet nterface pair.
         * @param hostName name of the interface to be created in the current namespace.
         * @param peerName name of the interface to be created in the target namespace.
         * @param pid process id of target namespace.
         * @param sync wait for completion.
         * @return 0 on success, -1 on failure.
         */
        int createVethInterface (const std::string& hostName, const std::string& peerName, pid_t* pid = nullptr,
                                 bool sync = false);

        /**
         * @brief creates a a GRE tunnel interface.
         * @param tunnelName name of the GRE tunnel interface to be created.
         * @param parentIndex index of the physical interface to bind the tunnel to.
         * @param localAddress local endpoint IP address for the tunnel.
         * @param remoteAddress remote endpoint IP address for the tunnel.
         * @param ikey optional inbound GRE key.
         * @param okey optional outbound GRE key.
         * @param ttl ttl/hop Limit for encapsulated packets (0-255)
         * @param sync wait for completion.
         * @return 0 on success, -1 on failure.
         */
        int createGreInterface (const std::string& tunnelName, uint32_t parentIndex, const IpAddress& localAddress,
                                const IpAddress& remoteAddress, const uint32_t* ikey = nullptr,
                                const uint32_t* okey = nullptr, uint8_t ttl = 64, bool sync = false);

        /**
         * @brief creates a a GRE tunnel interface.
         * @param tunnelName name of the GRE tunnel interface to be created.
         * @param parentName name of the physical interface to bind the tunnel to.
         * @param localAddress local endpoint IP address for the tunnel.
         * @param remoteAddress remote endpoint IP address for the tunnel.
         * @param ikey optional inbound GRE key.
         * @param okey optional outbound GRE key.
         * @param ttl ttl/hop Limit for encapsulated packets (0-255)
         * @param sync wait for completion.
         * @return 0 on success, -1 on failure.
         */
        int createGreInterface (const std::string& tunnelName, const std::string& parentName,
                                const IpAddress& localAddress, const IpAddress& remoteAddress,
                                const uint32_t* ikey = nullptr, const uint32_t* okey = nullptr, uint8_t ttl = 64,
                                bool sync = false);

        /**
         * @brief deletes the specified network interface.
         * @param interfaceIndex interface index.
         * @param sync wait for completion.
         * @return 0 on success, -1 on failure.
         */
        int removeInterface (uint32_t interfaceIndex, bool sync = false);

        /**
         * @brief deletes the specified network interface.
         * @param interfaceName interface name.
         * @param sync wait for completion.
         * @return 0 on success, -1 on failure.
         */
        int removeInterface (const std::string& interfaceName, bool sync = false);

    private:
        /**
         * @brief Add veth peer info data.
         * @brief nlh netlink message header.
         * @brief peerName peer interface name.
         */
        static void addPeerInfoData (struct nlmsghdr* nlh, const std::string& peerName);

        /**
         * @brief set interface mtu.
         * @param interfaceIndex interface index.
         * @param mtuBytes new mtu value.
         * @param sync wait for operation completion if true.
         * @return 0 on success, -1 on failure.
         */
        int mtu (uint32_t interfaceIndex, uint32_t mtuBytes, bool sync = false);

        /**
         * @brief set interface mac address.
         * @param interfaceIndex interface index.
         * @param macAddress new mac address.
         * @param sync wait for operation completion if true.
         * @return 0 on success, -1 on failure.
         */
        int mac (uint32_t interfaceIndex, const MacAddress& macAddress, bool sync = false);

        /**
         * @brief add interface to bridge.
         * @param interfaceIndex interface index.
         * @param masterIndex bridge index.
         * @param sync wait for operation completion if true.
         * @return 0 on success, -1 on failure.
         */
        int addToBridge (uint32_t interfaceIndex, uint32_t masterIndex, bool sync = false);

        /**
         * @brief remove interface from bridge.
         * @param interfaceIndex interface index.
         * @param sync wait for operation completion if true.
         * @return 0 on success, -1 on failure.
         */
        int removeFromBridge (uint32_t interfaceIndex, bool sync = false);

        /**
         * @brief enable interface.
         * @param interfaceIndex interface index.
         * @param enabled true to enable interface, false otherwise.
         * @param sync wait for operation completion if true.
         * @return 0 on success, -1 on failure.
         */
        int enable (uint32_t interfaceIndex, bool enabled = true, bool sync = false);

        /**
         * @brief add ip address to interface.
         * @param interfaceIndex interface index.
         * @param ipAddress ip address to add.
         * @param prefix prefix length.
         * @param broadcast broadcast address.
         * @param sync wait for operation completion if true.
         * @return 0 on success, -1 on failure.
         */
        int addAddress (uint32_t interfaceIndex, const IpAddress& ipAddress, uint32_t prefix,
                        const IpAddress& broadcast = {}, bool sync = false);

        /**
         * @brief remove ip address from interface.
         * @param interfaceIndex interface index.
         * @param ipAddress ip address to delete.
         * @param prefix prefix length.
         * @param broadcast broadcast address.
         * @param sync wait for operation completion if true.
         * @return 0 on success, -1 on failure.
         */
        int removeAddress (uint32_t interfaceIndex, const IpAddress& ipAddress, uint32_t prefix,
                           const IpAddress& broadcast = {}, bool sync = false);

        /**
         * @brief add route to interface.
         * @param interfaceIndex interface index.
         * @param dest destination network.
         * @param prefix prefix length.
         * @param gateway gateway address.
         * @param metric route metric.
         * @param sync wait for operation completion if true.
         * @return 0 on success, -1 on failure.
         */
        int addRoute (uint32_t interfaceIndex, const IpAddress& dest, uint32_t prefix, const IpAddress& gateway,
                      uint32_t* metric = nullptr, bool sync = false);

        /**
         * @brief remove route from interface.
         * @param interfaceIndex interface index.
         * @param dest destination network.
         * @param prefix prefix length.
         * @param gateway gateway address.
         * @param metric route metric.
         * @param sync wait for operation completion if true.
         * @return 0 on success, -1 on failure.
         */
        int removeRoute (uint32_t interfaceIndex, const IpAddress& dest, uint32_t prefix, const IpAddress& gateway,
                         uint32_t* metric = nullptr, bool sync = false);

        /**
         * @brief dump link data.
         * @param sync wait for completion.
         * @return 0 on success, -1 on failure.
         */
        int dumpLink (bool sync = false);

        /**
         * @brief dump address data.
         * @param sync wait for completion.
         * @return 0 on success, -1 on failure.
         */
        int dumpAddress (bool sync = false);

        /**
         * @brief dump route data.
         * @param sync wait for completion.
         * @return 0 on success, -1 on failure.
         */
        int dumpRoute (bool sync = false);

        /**
         * @brief dispatch a single RTM_* message to the derived class.
         * @param nlh the netlink message to process.
         */
        void onMessage (struct nlmsghdr* nlh) override;

        /**
         * @brief handle link notification.
         * @param nlh netlink message.
         */
        void onLinkMessage (struct nlmsghdr* nlh);

        /**
         * @brief handle link info notification.
         * @param nlh netlink message.
         * @param flags interface change flags.
         */
        void onLinkInfoMessage (Interface::Ptr& iface, struct rtattr* rta, InterfaceChangeType& flags);

        /**
         * @brief handle address notification.
         * @param nlh netlink message.
         */
        void onAddressMessage (struct nlmsghdr* nlh);

        /**
         * @brief handle route notification.
         * @param nlh netlink message.
         */
        void onRouteMessage (struct nlmsghdr* nlh);

        /**
         * @brief notifies all registered link listeners on link update.
         * @param info the updated link information.
         */
        void notifyLinkUpdate (const LinkInfo& info);

        /**
         * @brief notifies all registered address listeners on address update.
         * @param info the updated address information.
         */
        void notifyAddressUpdate (const AddressInfo& info);

        /**
         * @brief notifies all registered route listeners on route update.
         * @param info the updated route information.
         */
        void notifyRouteUpdate (const RouteInfo& info);

        /**
         * @brief acquire interface.
         * @param index interface index.
         * @param info interface informations.
         */
        Interface::Ptr acquire (uint32_t index, LinkInfo& info);

        /// reserved vlan id.
        static constexpr uint16_t reservedVlanId = 0;

        /// max vlan id.
        static constexpr uint16_t maxVlanId = 4094;

        /// interfaces.
        std::unordered_map<uint32_t, Interface::Ptr> _interfaces;

        /// protection mutex.
        Mutex _ifMutex;

        /// link listener callbacks.
        std::unordered_map<uint64_t, LinkNotify> _linkListeners;

        /// address listener callbacks.
        std::unordered_map<uint64_t, AddressNotify> _addressListeners;

        /// route listener callbacks.
        std::unordered_map<uint64_t, RouteNotify> _routeListeners;

        /// listener id counter.
        std::atomic<uint64_t> _listenerCounter{0};

        // friendship with interface.
        friend class Interface;
    };
}

#endif
