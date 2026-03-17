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

#ifndef JOIN_FABRIC_ROUTEMANAGER_HPP
#define JOIN_FABRIC_ROUTEMANAGER_HPP

// libjoin.
#include <join/netlinkmanager.hpp>
#include <join/route.hpp>

// C++.
#include <functional>

namespace join
{
    /**
     * @brief enumeration of route change types.
     */
    enum class RouteChangeType : uint32_t
    {
        Added           = 1L << 0, /**< entry did not exist before. */
        Deleted         = 1L << 1, /**< entry was removed. */
        Modified        = 1L << 2, /**< entry was updated. */
        GatewayChanged  = 1L << 3, /**< gateway address changed. */
        MetricChanged   = 1L << 4, /**< metric changed. */
        TypeChanged     = 1L << 5, /**< type changed. */
        ScopeChanged    = 1L << 6, /**< scope changed. */
        ProtocolChanged = 1L << 7, /**< protocol changed. */
    };

    /**
     * @brief perform binary AND on RouteChangeType.
     * @param a bitset.
     * @param b other bitset.
     * @return bitset result of binary AND on RouteChangeType.
     */
    inline RouteChangeType operator& (RouteChangeType a, RouteChangeType b)
    {
        return RouteChangeType (static_cast<uint32_t> (a) & static_cast<uint32_t> (b));
    }

    /**
     * @brief perform binary OR on RouteChangeType.
     * @param a bitset.
     * @param b other bitset.
     * @return bitset result of binary OR on RouteChangeType.
     */
    inline RouteChangeType operator| (RouteChangeType a, RouteChangeType b)
    {
        return RouteChangeType (static_cast<uint32_t> (a) | static_cast<uint32_t> (b));
    }

    /**
     * @brief perform binary XOR on RouteChangeType.
     * @param a bitset.
     * @param b other bitset.
     * @return bitset result of binary XOR on RouteChangeType.
     */
    inline RouteChangeType operator^ (RouteChangeType a, RouteChangeType b)
    {
        return RouteChangeType (static_cast<uint32_t> (a) ^ static_cast<uint32_t> (b));
    }

    /**
     * @brief perform binary NOT on RouteChangeType.
     * @param a bitset.
     * @return bitset result of binary NOT on RouteChangeType.
     */
    inline RouteChangeType operator~(RouteChangeType a)
    {
        return RouteChangeType (~static_cast<uint32_t> (a));
    }

    /**
     * @brief perform binary AND assignment on RouteChangeType.
     * @param a bitset.
     * @param b other bitset.
     * @return reference to updated bitset.
     */
    inline const RouteChangeType& operator&= (RouteChangeType& a, RouteChangeType b)
    {
        return a = a & b;
    }

    /**
     * @brief perform binary OR assignment on RouteChangeType.
     * @param a bitset.
     * @param b other bitset.
     * @return reference to updated bitset.
     */
    inline const RouteChangeType& operator|= (RouteChangeType& a, RouteChangeType b)
    {
        return a = a | b;
    }

    /**
     * @brief perform binary XOR assignment on RouteChangeType.
     * @param a bitset.
     * @param b other bitset.
     * @return reference to updated bitset.
     */
    inline const RouteChangeType& operator^= (RouteChangeType& a, RouteChangeType b)
    {
        return a = a ^ b;
    }

    /**
     * @brief route change notification payload.
     */
    struct RouteInfo
    {
        Route::Ptr route;      /**< route. */
        RouteChangeType flags; /**< what changed (bitmask). */
    };

    /**
     * @brief kernel routing table manager class.
     */
    class RouteManager : public NetlinkManager
    {
    public:
        using RouteNotify = std::function<void (const RouteInfo& info)>;

        /**
         * @brief create instance.
         * @param reactor event loop reactor (uses ReactorThread if nullptr).
         */
        explicit RouteManager (Reactor* reactor = nullptr);

        /**
         * @brief create instance by copy.
         */
        RouteManager (const RouteManager&) = delete;

        /**
         * @brief create instance by move.
         */
        RouteManager (RouteManager&&) = delete;

        /**
         * @brief assign instance by copy.
         */
        RouteManager& operator= (const RouteManager&) = delete;

        /**
         * @brief assign instance by move.
         */
        RouteManager& operator= (RouteManager&&) = delete;

        /**
         * @brief destroy instance.
         */
        ~RouteManager ();

        /**
         * @brief get the singleton instance.
         * @return reference to the singleton instance.
         */
        static RouteManager& instance ();

        /**
         * @brief find a route entry by interface index, destination and prefix.
         * @param index interface index.
         * @param dest destination network address.
         * @param prefix prefix length.
         * @return route pointer, or nullptr if not found.
         */
        Route::Ptr findByIndex (uint32_t index, const IpAddress& dest, uint32_t prefix);

        /**
         * @brief find a route entry by interface name, destination and prefix.
         * @param interfaceName interface name.
         * @param dest destination network address.
         * @param prefix prefix length.
         * @return route pointer, or nullptr if not found.
         */
        Route::Ptr findByName (const std::string& interfaceName, const IpAddress& dest, uint32_t prefix);

        /**
         * @brief enumerate all cached route entries.
         * @return snapshot of the current route list.
         */
        RouteList enumerate ();

        /**
         * @brief enumerate all cached route entries for a given interface.
         * @param index interface index.
         * @return snapshot of route entries on that interface.
         */
        RouteList enumerate (uint32_t index);

        /**
         * @brief enumerate all cached route entries for a given interface.
         * @param interfaceName interface name.
         * @return snapshot of route entries on that interface.
         */
        RouteList enumerate (const std::string& interfaceName);

        /**
         * @brief refresh all data.
         * @return 0 on success, -1 on failure.
         */
        int refresh ();

        /**
         * @brief register a callback invoked on any route table change.
         * @param cb callback to register.
         * @return unique id for the callback.
         */
        uint64_t addRouteListener (const RouteNotify& cb);

        /**
         * @brief unregister a previously registered callback.
         * @param id unique id of the callback function to remove.
         */
        void removeRouteListener (uint64_t id);

        /**
         * @brief add a route entry in the kernel table.
         * @param index interface index.
         * @param dest destination network address.
         * @param prefix prefix length.
         * @param gateway gateway address.
         * @param metric route metric.
         * @param sync wait for completion.
         * @return 0 on success, -1 on failure.
         */
        int addRoute (uint32_t index, const IpAddress& dest, uint32_t prefix, const IpAddress& gateway = {},
                      uint32_t metric = 0, bool sync = false);

        /**
         * @brief add a route entry in the kernel table.
         * @param interfaceName interface name.
         * @param dest destination network address.
         * @param prefix prefix length.
         * @param gateway gateway address.
         * @param metric route metric.
         * @param sync wait for completion.
         * @return 0 on success, -1 on failure.
         */
        int addRoute (const std::string& interfaceName, const IpAddress& dest, uint32_t prefix,
                      const IpAddress& gateway = {}, uint32_t metric = 0, bool sync = false);

        /**
         * @brief remove a route entry from the kernel table.
         * @param index interface index.
         * @param dest destination network address.
         * @param prefix prefix length.
         * @param sync wait for completion.
         * @return 0 on success, -1 on failure.
         */
        int removeRoute (uint32_t index, const IpAddress& dest, uint32_t prefix, bool sync = false);

        /**
         * @brief remove a route entry from the kernel table.
         * @param interfaceName interface name.
         * @param dest destination network address.
         * @param prefix prefix length.
         * @param sync wait for completion.
         * @return 0 on success, -1 on failure.
         */
        int removeRoute (const std::string& interfaceName, const IpAddress& dest, uint32_t prefix, bool sync = false);

        /**
         * @brief flush all route entries for a given interface.
         * @param index interface index.
         * @param sync wait for completion.
         * @return 0 on success, -1 on failure.
         */
        int flushRoutes (uint32_t index, bool sync = false);

        /**
         * @brief flush all route entries for a given interface.
         * @param interfaceName interface name.
         * @param sync wait for completion.
         * @return 0 on success, -1 on failure.
         */
        int flushRoutes (const std::string& interfaceName, bool sync = false);

    private:
        /**
         * @brief update or replace a route entry in the kernel table.
         * @param index interface index.
         * @param dest destination network address.
         * @param prefix prefix length.
         * @param gateway gateway address.
         * @param metric route metric.
         * @param sync wait for kernel acknowledgement if true.
         * @return 0 on success, -1 on failure.
         */
        int setRoute (uint32_t index, const IpAddress& dest, uint32_t prefix, const IpAddress& gateway, uint32_t metric,
                      bool sync);

        /**
         * @brief dump the full kernel route table.
         * @param sync wait for completion.
         * @return 0 on success, -1 on failure.
         */
        int dumpRoutes (bool sync = false);

        /**
         * @brief dispatch a single RTM_* message to the derived class.
         * @param nlh the netlink message to process.
         */
        void onMessage (struct nlmsghdr* nlh) override;

        /**
         * @brief handle a route add / update / delete notification.
         * @param nlh netlink message.
         */
        void onRouteMessage (struct nlmsghdr* nlh);

        /**
         * @brief notify all registered listeners.
         * @param info change description.
         */
        void notifyRouteUpdate (const RouteInfo& info);

        /**
         * @brief acquire route entry.
         * @param index interface index.
         * @param dest destination network address.
         * @param prefix prefix length.
         * @param info route entry information.
         * @return shared pointer to the Route.
         */
        Route::Ptr acquire (uint32_t index, const IpAddress& dest, uint32_t prefix, RouteInfo& info);

        /// route cache.
        std::unordered_map<RouteKey, Route::Ptr> _routes;

        /// protection mutex for the cache.
        Mutex _routeMutex;

        /// route listener callbacks.
        std::unordered_map<uint64_t, RouteNotify> _routeListeners;

        /// listener id counter.
        std::atomic<uint64_t> _listenerCounter{0};

        // friendship with route.
        friend class Route;
    };
}

#endif
