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

#ifndef JOIN_FABRIC_ROUTE_HPP
#define JOIN_FABRIC_ROUTE_HPP

// libjoin.
#include <join/ipaddress.hpp>
#include <join/mutex.hpp>

// C++.
#include <memory>
#include <vector>

// C.
#include <linux/rtnetlink.h>
#include <cstdint>

namespace join
{
    /// forward declaration.
    class RouteManager;

    /**
     * @brief key identifying a routing table entry.
     */
    struct RouteKey
    {
        /// interface index.
        uint32_t _index = 0;

        /// destination network address.
        IpAddress _dest;

        /// prefix length.
        uint32_t _prefix = 0;
    };

    /**
     * @brief compare if two route keys are equal.
     * @param lhs left side key.
     * @param rhs right side key.
     * @return true if both keys are equal.
     */
    inline bool operator== (const RouteKey& lhs, const RouteKey& rhs) noexcept
    {
        return lhs._index == rhs._index && lhs._dest == rhs._dest && lhs._prefix == rhs._prefix;
    }

    /**
     * @brief compare two route keys for ordering.
     * @param lhs left side key.
     * @param rhs right side key.
     * @return true if lhs is less than rhs.
     */
    inline bool operator< (const RouteKey& lhs, const RouteKey& rhs) noexcept
    {
        if (lhs._index != rhs._index)
        {
            return lhs._index < rhs._index;
        }
        if (lhs._dest != rhs._dest)
        {
            return lhs._dest < rhs._dest;
        }
        return lhs._prefix < rhs._prefix;
    }

    /**
     * @brief class representing a single kernel routing table entry.
     */
    class Route
    {
    private:
        /**
         * @brief create instance.
         * @param manager owning route manager.
         * @param key key identifying a routing table entry.
         */
        explicit Route (RouteManager* manager, const RouteKey& key);

    public:
        using Ptr  = std::shared_ptr<Route>;
        using List = std::vector<Ptr>;

        /**
         * @brief create instance.
         */
        Route () = delete;

        /**
         * @brief destroy instance.
         */
        ~Route () = default;

        /**
         * @brief get interface index.
         * @return interface index.
         */
        uint32_t index () const noexcept;

        /**
         * @brief get the destination network address.
         * @return destination IP address.
         */
        const IpAddress& dest () const noexcept;

        /**
         * @brief get the prefix length.
         * @return prefix length.
         */
        uint32_t prefix () const noexcept;

        /**
         * @brief get the gateway address.
         * @return gateway IP address.
         */
        IpAddress gateway () const;

        /**
         * @brief get the route metric.
         * @return metric value.
         */
        uint32_t metric () const;

        /**
         * @brief get the route type.
         * @return route type.
         */
        uint8_t type () const;

        /**
         * @brief get the route scope.
         * @return route scope.
         */
        uint8_t scope () const;

        /**
         * @brief get the route protocol.
         * @return route protocol.
         */
        uint8_t protocol () const;

        /**
         * @brief check if route is a standard unicast route.
         * @return true if type is RTN_UNICAST.
         */
        bool isUnicast () const noexcept;

        /**
         * @brief check if route silently drops packets.
         * @return true if type is RTN_BLACKHOLE.
         */
        bool isBlackhole () const noexcept;

        /**
         * @brief check if route returns ICMP unreachable.
         * @return true if type is RTN_UNREACHABLE.
         */
        bool isUnreachable () const noexcept;

        /**
         * @brief check if route returns ICMP prohibited.
         * @return true if type is RTN_PROHIBIT.
         */
        bool isProhibit () const noexcept;

        /**
         * @brief check if route is a local route.
         * @return true if type is RTN_LOCAL.
         */
        bool isLocal () const noexcept;

        /**
         * @brief check if route is universe-scoped.
         * @return true if scope is RT_SCOPE_UNIVERSE.
         */
        bool isScopeUniverse () const noexcept;

        /**
         * @brief check if route is link-scoped.
         * @return true if scope is RT_SCOPE_LINK.
         */
        bool isScopeLink () const noexcept;

        /**
         * @brief check if route is host-scoped.
         * @return true if scope is RT_SCOPE_HOST.
         */
        bool isScopeHost () const noexcept;

        /**
         * @brief check if route was added statically.
         * @return true if protocol is RTPROT_STATIC.
         */
        bool isStatic () const noexcept;

        /**
         * @brief check if route was added by the kernel.
         * @return true if protocol is RTPROT_KERNEL.
         */
        bool isKernel () const noexcept;

        /**
         * @brief check if route was added by a DHCP client.
         * @return true if protocol is RTPROT_DHCP.
         */
        bool isDhcp () const noexcept;

        /**
         * @brief check if route was added at boot time.
         * @return true if protocol is RTPROT_BOOT.
         */
        bool isBoot () const noexcept;

        /**
         * @brief update this route entry in the kernel table.
         * @param gateway new gateway address.
         * @param metric new metric value.
         * @param sync wait for operation completion if true.
         * @return 0 on success, -1 on failure.
         */
        int set (const IpAddress& gateway, uint32_t metric = 0, bool sync = false) const;

        /**
         * @brief remove this route entry from the kernel table.
         * @param sync wait for operation completion if true.
         * @return 0 on success, -1 on failure.
         */
        int remove (bool sync = false) const;

    private:
        /// route manager.
        RouteManager* _manager = nullptr;

        /// interface index.
        uint32_t _index = 0;

        /// destination network address.
        IpAddress _dest;

        /// prefix length.
        uint32_t _prefix = 0;

        /// gateway address.
        IpAddress _gateway;

        /// route metric.
        uint32_t _metric = 0;

        /// route type.
        uint8_t _type = RTN_UNICAST;

        /// route scope.
        uint8_t _scope = RT_SCOPE_UNIVERSE;

        /// route protocol.
        uint8_t _protocol = RTPROT_STATIC;

        /// protection mutex.
        mutable Mutex _mutex;

        // friendship with equal operator.
        friend bool operator== (const Route::Ptr& lhs, const Route::Ptr& rhs) noexcept;

        // friendship with less operator.
        friend bool operator< (const Route::Ptr& lhs, const Route::Ptr& rhs) noexcept;

        // friendship with route manager.
        friend class RouteManager;
    };

    /**
     * @brief compare if two routes are equal.
     * @param lhs left side route.
     * @param rhs right side route.
     * @return true if both are equal.
     */
    inline bool operator== (const Route::Ptr& lhs, const Route::Ptr& rhs) noexcept
    {
        if (!lhs && !rhs)
        {
            return true;
        }
        if (!lhs || !rhs)
        {
            return false;
        }
        return lhs->_index == rhs->_index && lhs->_dest == rhs->_dest && lhs->_prefix == rhs->_prefix;
    }

    /**
     * @brief compare two routes for ordering.
     * @param lhs left side route.
     * @param rhs right side route.
     * @return true if lhs is less than rhs.
     */
    inline bool operator< (const Route::Ptr& lhs, const Route::Ptr& rhs) noexcept
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
        if (lhs->_dest != rhs->_dest)
        {
            return lhs->_dest < rhs->_dest;
        }
        return lhs->_prefix < rhs->_prefix;
    }

    /// list of routes.
    using RouteList = std::vector<Route::Ptr>;

}

namespace std
{
    /**
     * @brief std::hash specialization for RouteKey.
     */
    template <>
    struct hash<join::RouteKey>
    {
        size_t operator() (const join::RouteKey& key) const noexcept
        {
            size_t h = std::hash<uint32_t>{}(key._index);
            h ^= std::hash<join::IpAddress>{}(key._dest) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= std::hash<uint32_t>{}(key._prefix) + 0x9e3779b9 + (h << 6) + (h >> 2);
            return h;
        }
    };
}

#endif
