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

#ifndef JOIN_FABRIC_NEIGHBOR_HPP
#define JOIN_FABRIC_NEIGHBOR_HPP

// libjoin.
#include <join/macaddress.hpp>
#include <join/ipaddress.hpp>
#include <join/mutex.hpp>

// C++.
#include <memory>
#include <vector>

// C.
#include <linux/neighbour.h>
#include <cstdint>

namespace join
{
    /// forward declaration.
    class NeighborManager;

    /**
     * @brief key identifying a neighbor cache entry.
     */
    struct NeighborKey
    {
        /// interface index.
        uint32_t _index = 0;

        /// destination address.
        IpAddress _ip;
    };

    /**
     * @brief compare if two neighbor keys are equal.
     * @param lhs left side key.
     * @param rhs right side key.
     * @return true if both keys are equal.
     */
    inline bool operator== (const NeighborKey& lhs, const NeighborKey& rhs) noexcept
    {
        return lhs._index == rhs._index && lhs._ip == rhs._ip;
    }

    /**
     * @brief compare two neighbor keys for ordering.
     * @param lhs left side key.
     * @param rhs right side key.
     * @return true if lhs is less than rhs.
     */
    inline bool operator< (const NeighborKey& lhs, const NeighborKey& rhs) noexcept
    {
        if (lhs._index != rhs._index)
        {
            return lhs._index < rhs._index;
        }
        return lhs._ip < rhs._ip;
    }

    /**
     * @brief class representing a single ARP / NDP neighbor cache entry.
     */
    class Neighbor
    {
    private:
        /**
         * @brief create instance.
         * @param manager owning neighbor manager.
         * @param key key identifying a neighbor cache entry.
         */
        explicit Neighbor (NeighborManager* manager, const NeighborKey& key);

    public:
        using Ptr = std::shared_ptr<Neighbor>;

        /**
         * @brief create instance.
         */
        Neighbor () = delete;

        /**
         * @brief destroy instance.
         */
        ~Neighbor () = default;

        /**
         * @brief get interface index.
         * @return interface index.
         */
        uint32_t index () const noexcept;

        /**
         * @brief get the destination address.
         * @return IP address.
         */
        const IpAddress& ip () const noexcept;

        /**
         * @brief get the address resolved for this entry.
         * @return MAC address.
         */
        MacAddress mac () const;

        /**
         * @brief get the NUD state bitmask.
         * @return NUD state flags.
         */
        uint16_t state () const;

        /**
         * @brief is the entry in NUD_REACHABLE state.
         * @return true if reachable.
         */
        bool isReachable () const noexcept;

        /**
         * @brief is the entry in NUD_STALE state.
         * @return true if stale.
         */
        bool isStale () const noexcept;

        /**
         * @brief is the entry in NUD_PERMANENT state (static).
         * @return true if permanent.
         */
        bool isPermanent () const noexcept;

        /**
         * @brief is the entry in NUD_INCOMPLETE state (resolution ongoing).
         * @return true if incomplete.
         */
        bool isIncomplete () const noexcept;

        /**
         * @brief is the entry in NUD_FAILED state.
         * @return true if failed.
         */
        bool isFailed () const noexcept;

        /**
         * @brief update or replace this neighbor entry in the kernel table.
         * @param macAddress new address to associate.
         * @param state NUD state to set (default: NUD_PERMANENT).
         * @param sync wait for operation completion if true.
         * @return 0 on success, -1 on failure.
         */
        int set (const MacAddress& macAddress, uint16_t state = NUD_PERMANENT, bool sync = false) const;

        /**
         * @brief remove this neighbor entry from the kernel table.
         * @param sync wait for operation completion if true.
         * @return 0 on success, -1 on failure.
         */
        int remove (bool sync = false) const;

    private:
        /// neighbor manager.
        NeighborManager* _manager = nullptr;

        /// interface index.
        uint32_t _index = 0;

        /// destination address.
        IpAddress _ip;

        /// resolved address.
        MacAddress _mac;

        /// NUD state bitmask.
        uint16_t _state = NUD_NONE;

        /// protection mutex.
        mutable Mutex _mutex;

        // friendship with equal operator.
        friend bool operator== (const Neighbor::Ptr& lhs, const Neighbor::Ptr& rhs) noexcept;

        // friendship with less operator.
        friend bool operator< (const Neighbor::Ptr& lhs, const Neighbor::Ptr& rhs) noexcept;

        // friendship with neighbor manager.
        friend class NeighborManager;
    };

    /**
     * @brief compare if two neighbor are equal.
     * @param lhs left side key.
     * @param rhs right side key.
     * @return true if both are equal.
     */
    inline bool operator== (const Neighbor::Ptr& lhs, const Neighbor::Ptr& rhs) noexcept
    {
        if (!lhs && !rhs)
        {
            return true;
        }
        if (!lhs || !rhs)
        {
            return false;
        }
        return lhs->_index == rhs->_index && lhs->_ip == rhs->_ip;
    }

    /**
     * @brief compare two neighbor for ordering.
     * @param lhs left side key.
     * @param rhs right side key.
     * @return true if lhs is less than rhs.
     */
    inline bool operator< (const Neighbor::Ptr& lhs, const Neighbor::Ptr& rhs) noexcept
    {
        if (!lhs)
        {
            return true;
        }
        if (!rhs)
        {
            return false;
        }
        if (lhs->_index != rhs->_index)
        {
            return lhs->_index < rhs->_index;
        }
        return lhs->_ip < rhs->_ip;
    }

    /// list of neighbors.
    using NeighborList = std::vector<Neighbor::Ptr>;
};

namespace std
{
    /**
     * @brief std::hash specialization for NeighborKey.
     */
    template <>
    struct hash<join::NeighborKey>
    {
        size_t operator() (const join::NeighborKey& key) const noexcept
        {
            size_t h = std::hash<uint32_t>{}(key._index);
            h ^= std::hash<join::IpAddress>{}(key._ip) + 0x9e3779b9 + (h << 6) + (h >> 2);
            return h;
        }
    };
}

#endif
