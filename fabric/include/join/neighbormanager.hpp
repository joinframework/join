/**
 * MIT License
 *
 * Copyright (c) 2026 Mathieu Rabine
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

#ifndef JOIN_FABRIC_NEIGHBORMANAGER_HPP
#define JOIN_FABRIC_NEIGHBORMANAGER_HPP

// libjoin.
#include <join/netlinkmanager.hpp>
#include <join/neighbor.hpp>

// C++.
#include <functional>

namespace join
{
    /**
     * @brief enumeration of neighbor change types.
     */
    enum class NeighborChangeType : uint32_t
    {
        Added        = 1L << 0, /**< entry did not exist before. */
        Deleted      = 1L << 1, /**< entry was removed. */
        Modified     = 1L << 2, /**< entry was updated. */
        MacChanged   = 1L << 3, /**< MAC address changed. */
        StateChanged = 1L << 4, /**< NUD state changed. */
    };

    /**
     * @brief perform binary AND on NeighborChangeType.
     * @param a bitset.
     * @param b other bitset.
     * @return bitset result of binary AND on NeighborChangeType.
     */
    inline NeighborChangeType operator& (NeighborChangeType a, NeighborChangeType b)
    {
        return NeighborChangeType (static_cast<uint32_t> (a) & static_cast<uint32_t> (b));
    }

    /**
     * @brief perform binary OR on NeighborChangeType.
     * @param a bitset.
     * @param b other bitset.
     * @return bitset result of binary OR on NeighborChangeType.
     */
    inline NeighborChangeType operator| (NeighborChangeType a, NeighborChangeType b)
    {
        return NeighborChangeType (static_cast<uint32_t> (a) | static_cast<uint32_t> (b));
    }

    /**
     * @brief perform binary XOR on NeighborChangeType.
     * @param a bitset.
     * @param b other bitset.
     * @return bitset result of binary XOR on NeighborChangeType.
     */
    inline NeighborChangeType operator^ (NeighborChangeType a, NeighborChangeType b)
    {
        return NeighborChangeType (static_cast<uint32_t> (a) ^ static_cast<uint32_t> (b));
    }

    /**
     * @brief perform binary NOT on NeighborChangeType.
     * @param a bitset.
     * @return bitset result of binary NOT on NeighborChangeType.
     */
    inline NeighborChangeType operator~(NeighborChangeType a)
    {
        return NeighborChangeType (~static_cast<uint32_t> (a));
    }

    /**
     * @brief perform binary AND on NeighborChangeType.
     * @param a bitset.
     * @param b other bitset.
     * @return bitset result of binary AND on NeighborChangeType.
     */
    inline const NeighborChangeType& operator&= (NeighborChangeType& a, NeighborChangeType b)
    {
        return a = a & b;
    }

    /**
     * @brief perform binary OR on NeighborChangeType.
     * @param a bitset.
     * @param b other bitset.
     * @return bitset result of binary OR.
     */
    inline const NeighborChangeType& operator|= (NeighborChangeType& a, NeighborChangeType b)
    {
        return a = a | b;
    }

    /**
     * @brief perform binary XOR on NeighborChangeType.
     * @param a bitset.
     * @param b other bitset.
     * @return bitset result of binary XOR on NeighborChangeType.
     */
    inline const NeighborChangeType& operator^= (NeighborChangeType& a, NeighborChangeType b)
    {
        return a = a ^ b;
    }

    /**
     * @brief neighbor change notification payload.
     */
    struct NeighborInfo
    {
        Neighbor::Ptr neighbor;   /**< neighbor. */
        NeighborChangeType flags; /**< what changed (bitmask). */
    };

    /**
     * @brief ARP / NDP neighbor manager class.
     */
    class NeighborManager : public NetlinkManager
    {
    public:
        using NeighborNotify = std::function<void (const NeighborInfo& info)>;

        /**
         * @brief create instance.
         * @param reactor event loop reactor (uses ReactorThread if nullptr).
         */
        explicit NeighborManager (Reactor* reactor = nullptr);

        /**
         * @brief create instance by copy.
         * @param other other interface to copy.
         */
        NeighborManager (const NeighborManager& other) = delete;

        /**
         * @brief create instance by move.
         * @param other other interface to move.
         */
        NeighborManager (NeighborManager&& other) = delete;

        /**
         * @brief assign instance by copy.
         * @param other other interface to copy.
         * @return A reference of the current object.
         */
        NeighborManager& operator= (const NeighborManager& other) = delete;

        /**
         * @brief assign instance by move.
         * @param other other interface to move.
         * @return A reference of the current object.
         */
        NeighborManager& operator= (NeighborManager&& other) = delete;

        /**
         * @brief destroy instance.
         */
        ~NeighborManager ();

        /**
         * @brief get the a singleton instance.
         * @return reference to the singleton instance.
         */
        static NeighborManager& instance ();

        /**
         * @brief find a neighbor entry by interface index and IP address.
         * @param index interface index.
         * @param ipAddress destination address.
         * @return neighbor pointer, or nullptr if not found.
         */
        Neighbor::Ptr findByIndex (uint32_t index, const IpAddress& ipAddress);

        /**
         * @brief find a neighbor entry by interface name and IP address.
         * @param interfaceName interface name.
         * @param ipAddress destination address.
         * @return neighbor pointer, or nullptr if not found.
         */
        Neighbor::Ptr findByName (const std::string& interfaceName, const IpAddress& ipAddress);

        /**
         * @brief enumerate all cached neighbor entries.
         * @return snapshot of the current neighbor list.
         */
        NeighborList enumerate ();

        /**
         * @brief enumerate all cached neighbor entries for a given interface.
         * @param index interface index.
         * @return snapshot of neighbor entries on that interface.
         */
        NeighborList enumerate (uint32_t index);

        /**
         * @brief enumerate all cached neighbor entries for a given interface.
         * @param interfaceName interface name.
         * @return snapshot of neighbor entries on that interface.
         */
        NeighborList enumerate (const std::string& interfaceName);

        /**
         * @brief refresh all data.
         * @return 0 on success, -1 on failure.
         */
        int refresh ();

        /**
         * @brief register a callback invoked on any neighbor table change.
         * @param cb callback to register.
         * @return unique id for the callback.
         */
        uint64_t addNeighborListener (const NeighborNotify& cb);

        /**
         * @brief unregister a previously registered callback.
         * @param id unique id of the callback function to remove.
         */
        void removeNeighborListener (uint64_t id);

        /**
         * @brief add or replace a neighbor entry in the kernel table.
         * @param index interface index.
         * @param ipAddress destination address.
         * @param macAddress address to associate.
         * @param state NUD state (default: NUD_PERMANENT).
         * @param sync wait for completion.
         * @return 0 on success, -1 on failure.
         */
        int addNeighbor (uint32_t index, const IpAddress& ipAddress, const MacAddress& macAddress,
                         uint16_t state = NUD_PERMANENT, bool sync = false);

        /**
         * @brief add or replace a neighbor entry in the kernel table.
         * @param interfaceName interface name.
         * @param ipAddress destination address.
         * @param macAddress address to associate.
         * @param state NUD state (default: NUD_PERMANENT).
         * @param sync wait for completion.
         * @return 0 on success, -1 on failure.
         */
        int addNeighbor (const std::string& interfaceName, const IpAddress& ipAddress, const MacAddress& macAddress,
                         uint16_t state = NUD_PERMANENT, bool sync = false);

        /**
         * @brief remove a neighbor entry from the kernel table.
         * @param index interface index.
         * @param ipAddress destination address.
         * @param sync wait for completion.
         * @return 0 on success, -1 on failure.
         */
        int removeNeighbor (uint32_t index, const IpAddress& ipAddress, bool sync = false);

        /**
         * @brief remove a neighbor entry from the kernel table.
         * @param interfaceName interface name.
         * @param ipAddress destination address.
         * @param sync wait for completion.
         * @return 0 on success, -1 on failure.
         */
        int removeNeighbor (const std::string& interfaceName, const IpAddress& ipAddress, bool sync = false);

        /**
         * @brief flush all neighbor entries for a given interface.
         * @param index interface index.
         * @param sync wait for completion.
         * @return 0 on success, -1 on failure.
         */
        int flushNeighbors (uint32_t index, bool sync = false);

        /**
         * @brief flush all neighbor entries for a given interface.
         * @param interfaceName interface name.
         * @param sync wait for completion.
         * @return 0 on success, -1 on failure.
         */
        int flushNeighbors (const std::string& interfaceName, bool sync = false);

    private:
        /**
         * @brief update or replace a neighbor entry.
         * @param index interface index.
         * @param ipAddress  layer-3 destination address.
         * @param macAddress new layer-2 address.
         * @param state new NUD state.
         * @param sync wait for kernel acknowledgement if true.
         * @return 0 on success, -1 on failure.
         */
        int setNeighbor (uint32_t index, const IpAddress& ipAddress, const MacAddress& macAddress, uint16_t state,
                         bool sync);

        /**
         * @brief dump the full kernel neighbor table.
         * @param sync wait for completion.
         * @return 0 on success, -1 on failure.
         */
        int dumpNeighbors (bool sync = false);

        /**
         * @brief dispatch a single RTM_* message to the derived class.
         * @param nlh the netlink message to process.
         */
        void onMessage (struct nlmsghdr* nlh) override;

        /**
         * @brief handle a neighbor add / update / delete notification.
         * @param nlh netlink message.
         */
        void onNeighborMessage (struct nlmsghdr* nlh);

        /**
         * @brief notify all registered listeners.
         * @param info change description.
         */
        void notifyNeighborUpdate (const NeighborInfo& info);

        /**
         * @brief acquire neighbor entry.
         * @param index interface index.
         * @param ipAddress destination address.
         * @param info neighbor entry informations.
         * @return shared pointer to the Neighbor.
         */
        Neighbor::Ptr acquire (uint32_t index, const IpAddress& ipAddress, NeighborInfo& info);

        /// neighbor cache.
        std::unordered_map<NeighborKey, Neighbor::Ptr> _neighbors;

        /// protection mutex for the cache.
        Mutex _neighMutex;

        /// neighbor listener callbacks.
        std::unordered_map<uint64_t, NeighborNotify> _neighborListeners;

        /// listener id counter.
        uint64_t _listenerCounter = 0;

        /// protection mutex for listeners.
        Mutex _listenerMutex;

        // friendship with neighbor.
        friend class Neighbor;
    };
}

#endif
