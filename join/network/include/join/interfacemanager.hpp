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

#ifndef __JOIN_INTERFACE_MANAGER_HPP__
#define __JOIN_INTERFACE_MANAGER_HPP__

// libjoin.
#include <join/macaddress.hpp>
#include <join/ipaddress.hpp>
#include <join/condition.hpp>
#include <join/interface.hpp>
#include <join/socket.hpp>

// C++.
#include <string>
#include <memory>
#include <atomic>
#include <map>

// C.
#include <linux/rtnetlink.h>
#include <linux/netlink.h>
#include <cstdint>
#include <cstddef>

namespace join
{
    /**
     * @brief enumeration of interface change type.
     */
    enum ChangeType
    {
        Added               = 1L << 0,
        Deleted             = 1L << 1,
        Modified            = 1L << 2,
        AdminStateChanged   = 1L << 3,
        OperStateChanged    = 1L << 4,
        MacChanged          = 1L << 5,
        NameChanged         = 1L << 6,
        MtuChanged          = 1L << 7,
        KindChanged         = 1L << 8,
        MasterChanged       = 1L << 9,
    };

    /**
     * @brief perform binary AND on ChangeType.
     * @param __a bitset.
     * @param __b other bitset.
     * @return bitset result of binary AND on ChangeType.
     */
    __inline__ ChangeType operator& (ChangeType __a, ChangeType __b)
    { return ChangeType (static_cast <int> (__a) & static_cast <int> (__b)); }

    /**
     * @brief perform binary OR on ChangeType.
     * @param __a bitset.
     * @param __b other bitset.
     * @return bitset result of binary OR on ChangeType.
     */
    __inline__ ChangeType operator| (ChangeType __a, ChangeType __b)
    { return ChangeType (static_cast <int> (__a) | static_cast <int> (__b)); }

    /**
     * @brief perform binary XOR on ChangeType.
     * @param __a bitset.
     * @param __b other bitset.
     * @return bitset result of binary XOR on ChangeType.
     */
    __inline__ ChangeType operator^ (ChangeType __a, ChangeType __b)
    { return ChangeType (static_cast <int> (__a) ^ static_cast <int> (__b)); }

    /**
     * @brief perform binary NOT on ChangeType.
     * @param __a bitset.
     * @return bitset result of binary NOT on ChangeType.
     */
    __inline__ ChangeType operator~ (ChangeType __a)
    { return ChangeType (~static_cast <int> (__a)); }

    /**
     * @brief perform binary AND on ChangeType.
     * @param __a bitset.
     * @param __b other bitset.
     * @return bitset result of binary AND on ChangeType.
     */
    __inline__ const ChangeType& operator&= (ChangeType& __a, ChangeType __b)
    { return __a = __a & __b; }

    /**
     * @brief perform binary OR on ChangeType.
     * @param __a bitset.
     * @param __b other bitset.
     * @return bitset result of binary OR.
     */
    __inline__ const ChangeType& operator|= (ChangeType& __a, ChangeType __b)
    { return __a = __a | __b; }

    /**
     * @brief perform binary XOR on ChangeType.
     * @param __a bitset.
     * @param __b other bitset.
     * @return bitset result of binary XOR on ChangeType.
     */
    __inline__ const ChangeType& operator^= (ChangeType& __a, ChangeType __b)
    { return __a = __a ^ __b; }

    /**
     * @brief link information.
     */
    struct LinkInfo
    {
        uint32_t index;                 /**< interface index. */
        ChangeType flags;               /**< what changed (bitmask). */
    };

    /**
     * @brief address information.
     */
    struct AddressInfo : public LinkInfo
    {
        Interface::Address address;     /**< address changed. */
    };

    /**
     * @brief route information.
     */
    struct RouteInfo : public LinkInfo
    {
         Interface::Route route;        /**< route changed. */
    };

    /**
     * @brief interface manager class.
     */
    class InterfaceManager : private NetLink::Socket
    {
    private:
        using LinkNotify    = std::function <void (const LinkInfo& info)>;
        using AddressNotify = std::function <void (const AddressInfo& info)>;
        using RouteNotify   = std::function <void (const RouteInfo& info)>;

        /**
         * @brief create instance.
         */
        InterfaceManager ();

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

    public:
        /**
         * @brief create the InterfaceManager instance.
         * @return InterfaceManager instance pointer.
         */
        static InterfaceManager* instance ();

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
         * @brief registers a callback to be invoked when a link update occurs.
         * @param cb the callback function to register.
         */
        void addLinkListener (const LinkNotify& cb);

        /**
         * @brief unregisters a previously registered link update callback.
         * @param cb the callback function to remove.
         */
        void removeLinkListener (const LinkNotify& cb);

        /**
         * @brief registers a callback to be invoked when a address update occurs.
         * @param cb the callback function to register.
         */
        void addAddressListener (const AddressNotify& cb);

        /**
         * @brief unregisters a previously registered address update callback.
         * @param cb the callback function to remove.
         */
        void removeAddressListener (const AddressNotify& cb);

        /**
         * @brief registers a callback to be invoked when a route update occurs.
         * @param cb the callback function to register.
         */
        void addRouteListener (const RouteNotify& cb);

        /**
         * @brief unregisters a previously registered route update callback.
         * @param cb the callback function to remove.
         */
        void removeRouteListener (const RouteNotify& cb);

        /**
         * @brief creates a dummy interface.
         * @param interfaceName name of the dummy interface to be created.
         * @param sync wait for completion.
         * @return 0 on success, -1 on failure.
         */
        int createDummyInterface (const std::string& interfaceName, bool sync = false);

        /**
         * @brief creates a point to point interface.
         * @param interfaceName name of the point to point interface to be created.
         * @param sync wait for completion.
         * @return 0 on success, -1 on failure.
         */
        int createPointToPointInterface (const std::string& interfaceName, bool sync = false);

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
        int createVlanInterface (const std::string& interfaceName, uint32_t parentIndex, uint16_t id, uint16_t proto = ETH_P_8021Q, bool sync = false);

        /**
         * @brief creates a VLAN interface.
         * @param interfaceName name of the VLAN interface to be created.
         * @param parentName name of the physical interface to create VLAN on.
         * @param id VLAN id (must be unique on the parent interface).
         * @param proto VLAN protocol type.
         * @param sync wait for completion.
         * @return 0 on success, -1 on failure.
         */
        int createVlanInterface (const std::string& interfaceName, const std::string& parentName, uint16_t id, uint16_t proto = ETH_P_8021Q, bool sync = false);

        /**
         * @brief creates a Virtual Ethernet nterface pair.
         * @param hostName name of the interface to be created in the current namespace.
         * @param peerName name of the interface to be created in the target namespace.
         * @param pid process id of target namespace.
         * @param sync wait for completion.
         * @return 0 on success, -1 on failure.
         */
        int createVethInterface (const std::string& hostName, const std::string& peerName, pid_t* pid, bool sync = false);

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
        int createGreInterface (const std::string& tunnelName, uint32_t parentIndex,
                                const IpAddress& localAddress, const IpAddress& remoteAddress,
                                const uint32_t* ikey, const uint32_t* okey, uint8_t ttl, bool sync = false);

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
                                const uint32_t* ikey, const uint32_t* okey, uint8_t ttl, bool sync = false);

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
         * @brief update value if changed and set flag accordingly.
         * @param oldVal old value.
         * @param newVal new value.
         * @param changed change type.
         * @return changed flags.
         */
        template <typename T>
        ChangeType updateValue (T& oldVal, const T& newVal, ChangeType changed) const
        {
            if (oldVal != newVal)
            {
                oldVal = newVal;
                return changed;
            }
            return static_cast <ChangeType> (0);
        }

        /**
         * @brief add attributes to netlink message.
         * @brief nlh netlink message header.
         * @brief type attribute type.
         * @brief data attribute data.
         * @brief alen attribute data length.
         */
        void addAttributes (struct nlmsghdr *nlh, int type, const void *data, int alen);

        /**
         * @brief start nested attributes.
         * @brief nlh netlink message header.
         * @return nested attributes pointer.
         */
        struct rtattr* startNestedAttributes (struct nlmsghdr *nlh, int type);

        /**
         * @brief Stop nested attributes.
         * @brief nlh netlink message header.
         * @brief nested attributes pointer.
         */
        int stopNestedAttributes (struct nlmsghdr *nlh, struct rtattr *nested);

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
        int addAddress (uint32_t interfaceIndex, const IpAddress& ipAddress, uint32_t prefix, const IpAddress& broadcast = {}, bool sync = false);

        /**
         * @brief remove ip address from interface.
         * @param interfaceIndex interface index.
         * @param ipAddress ip address to delete.
         * @param prefix prefix length.
         * @param broadcast broadcast address.
         * @param sync wait for operation completion if true.
         * @return 0 on success, -1 on failure.
         */
        int removeAddress (uint32_t interfaceIndex, const IpAddress& ipAddress, uint32_t prefix, const IpAddress& broadcast = {}, bool sync = false);

        /**
         * @brief add route to interface.
         * @param interfaceIndex interface index.
         * @param dest destination network.
         * @param gateway gateway address.
         * @param prefix prefix length.
         * @param metric route metric.
         * @param sync wait for operation completion if true.
         * @return 0 on success, -1 on failure.
         */
        int addRoute (uint32_t interfaceIndex, const IpAddress& dest, const IpAddress& gateway, uint32_t prefix, uint32_t metric, bool sync = false);

        /**
         * @brief remove route from interface.
         * @param interfaceIndex interface index.
         * @param dest destination network.
         * @param gateway gateway address.
         * @param prefix prefix length.
         * @param sync wait for operation completion if true.
         * @return 0 on success, -1 on failure.
         */
        int removeRoute (uint32_t interfaceIndex, const IpAddress& dest, const IpAddress& gateway, uint32_t prefix, bool sync = false);

        /**
         * @brief send netlink request.
         * @param nlh netlink message header.
         * @param sync wait for operation completion if true.
         * @return 0 on success, -1 on failure.
         */
        int sendRequest (struct nlmsghdr* nlh, bool sync);

        /**
         * @brief wait for specific netlink response.
         * @param lock mutex previously locked by the calling thread.
         * @param seq sequence number to wait for.
         * @param timeout timeout in milliseconds.
         * @return 0 on success, -1 on failure.
         */
        int waitResponse (ScopedLock& lock, uint32_t seq, uint32_t timeout = 5000);

        /**
         * @brief method called when data are ready to be read on handle.
         */
        virtual void onReceive () override;

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
        void onLinkInfoMessage (Interface::Ptr& iface, struct rtattr* rta, ChangeType& flags);

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
         * @brief notify pending request.
         * @param seq sequence number.
         * @param error error number.
         */
        void notifyRequest (uint32_t seq, int error = 0);

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
         * @param info interface informations.
         */
        Interface::Ptr acquire (LinkInfo& info);

        /// internal buffer size.
        static const size_t _bufferSize = 32768;

        /// internal read buffer.
        std::unique_ptr <char []> _buffer;

        /// interfaces.
        std::map <uint32_t, Interface::Ptr> _interfaces;

        /// protection mutex.
        Mutex _ifMutex;

        /// sequence number.
        std::atomic<uint32_t> _seq;

        /**
         * @brief pending request.
         */
        struct PendingRequest
        {
            Condition cond;
            int error = 0;
        };

        /// pending requests.
        std::map <uint32_t, std::shared_ptr <PendingRequest>> _pending;

        /// mutex for synchronous operations.
        Mutex _syncMutex;

        /// link listener callbacks.
        std::vector <LinkNotify> _linkListeners;

        /// protection mutex.
        Mutex _linkMutex;

        /// address listener callbacks.
        std::vector <AddressNotify> _addressListeners;

        /// protection mutex.
        Mutex _addressMutex;

        /// route listener callbacks.
        std::vector <RouteNotify> _routeListeners;

        /// protection mutex.
        Mutex _routeMutex;

        // friendship with interface.
        friend class Interface;
    };
}

#endif
