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

#ifndef JOIN_FABRIC_NETLINK_ENDPOINT_HPP
#define JOIN_FABRIC_NETLINK_ENDPOINT_HPP

// libjoin.
#include <join/endpoint.hpp>

// C.
#include <linux/netfilter/nfnetlink.h>
#include <linux/rtnetlink.h>
#include <linux/netlink.h>

namespace join
{
    /**
     * @brief basic netlink endpoint class.
     */
    template <class Protocol>
    class BasicNetlinkEndpoint : public BasicEndpoint<Protocol>
    {
    public:
        /**
         * @brief default constructor.
         */
        constexpr BasicNetlinkEndpoint () noexcept
        : BasicEndpoint<Protocol> ()
        , _protocol (Protocol ().protocol ())
        {
        }

        /**
         * @brief create instance using socket address.
         * @param addr socket address.
         * @param len socket address length.
         */
        BasicNetlinkEndpoint (const struct sockaddr* addr, socklen_t len) noexcept
        : BasicEndpoint<Protocol> (addr, len)
        , _protocol (Protocol ().protocol ())
        {
        }

        /**
         * @brief create instance using netlink groups.
         * @param protocol netlink protocol.
         * @param pid process id.
         * @param groups netlink groups to set.
         */
        BasicNetlinkEndpoint (const Protocol& protocol, uint32_t pid, uint32_t groups) noexcept
        : BasicEndpoint<Protocol> ()
        , _protocol (protocol.protocol ())
        {
            struct sockaddr_nl* nl = reinterpret_cast<struct sockaddr_nl*> (&this->_addr);
            nl->nl_pid = pid;
            nl->nl_groups = groups;
        }

        /**
         * @brief create instance using netlink groups.
         * @param pid process id.
         * @param groups netlink groups to set.
         */
        BasicNetlinkEndpoint (uint32_t pid, uint32_t groups) noexcept
        : BasicNetlinkEndpoint (Protocol (), pid, groups)
        {
        }

        /**
         * @brief create instance using netlink groups.
         * @param protocol netlink protocol.
         * @param groups netlink groups to set.
         */
        BasicNetlinkEndpoint (const Protocol& protocol, uint32_t groups) noexcept
        : BasicNetlinkEndpoint (protocol, getpid (), groups)
        {
        }

        /**
         * @brief create instance using netlink groups.
         * @param groups netlink groups to set.
         */
        BasicNetlinkEndpoint (uint32_t groups) noexcept
        : BasicNetlinkEndpoint (Protocol (), getpid (), groups)
        {
        }

        /**
         * @brief get endpoint protocol.
         * @return endpoint protocol.
         */
        Protocol protocol () const noexcept
        {
            if (_protocol == NETLINK_NETFILTER)
            {
                return Protocol::nf ();
            }
            return Protocol::rt ();
        }

        /**
         * @brief get socket address length.
         * @return socket address length.
         */
        constexpr socklen_t length () const noexcept
        {
            return sizeof (struct sockaddr_nl);
        }

        /**
         * @brief set process id.
         * @param pid process id.
         */
        void pid (uint32_t pid) noexcept
        {
            reinterpret_cast<struct sockaddr_nl*> (&this->_addr)->nl_pid = pid;
        }

        /**
         * @brief get process id.
         * @return process id.
         */
        uint32_t pid () const noexcept
        {
            return reinterpret_cast<const struct sockaddr_nl*> (&this->_addr)->nl_pid;
        }

        /**
         * @brief set netlink groups.
         * @param groups netlink groups bitmask.
         */
        void groups (uint32_t groups) noexcept
        {
            reinterpret_cast<struct sockaddr_nl*> (&this->_addr)->nl_groups = groups;
        }

        /**
         * @brief get netlink groups.
         * @return netlink groups bitmask.
         */
        uint32_t groups () const noexcept
        {
            return reinterpret_cast<const struct sockaddr_nl*> (&this->_addr)->nl_groups;
        }

        /**
         * @brief get device name (not applicable for netlink).
         * @return empty string.
         */
        std::string device () const
        {
            return std::string ();
        }

    protected:
        /// netlink protocol type.
        int _protocol;
    };

    /**
     * @brief compare if endpoints are equal.
     * @param a endpoint to compare.
     * @param b endpoint to compare to.
     * @return true if endpoints are equal, false otherwise.
     */
    template <class Protocol>
    bool operator== (const BasicNetlinkEndpoint<Protocol>& a, const BasicNetlinkEndpoint<Protocol>& b) noexcept
    {
        return a.pid () == b.pid () && a.groups () == b.groups ();
    }

    /**
     * @brief compare if endpoints are not equal.
     * @param a endpoint to compare.
     * @param b endpoint to compare to.
     * @return true if endpoints are not equal, false otherwise.
     */
    template <class Protocol>
    bool operator!= (const BasicNetlinkEndpoint<Protocol>& a, const BasicNetlinkEndpoint<Protocol>& b) noexcept
    {
        return !(a == b);
    }

    /**
     * @brief compare if endpoint is lower.
     * @param a endpoint to compare.
     * @param b endpoint to compare to.
     * @return true if lower, false otherwise.
     */
    template <class Protocol>
    bool operator< (const BasicNetlinkEndpoint<Protocol>& a, const BasicNetlinkEndpoint<Protocol>& b) noexcept
    {
        if (a.pid () != b.pid ())
        {
            return a.pid () < b.pid ();
        }
        return a.groups () < b.groups ();
    }

    /**
     * @brief compare if endpoint is greater.
     * @param a endpoint to compare.
     * @param b endpoint to compare to.
     * @return true if greater, false otherwise.
     */
    template <class Protocol>
    bool operator> (const BasicNetlinkEndpoint<Protocol>& a, const BasicNetlinkEndpoint<Protocol>& b) noexcept
    {
        return b < a;
    }

    /**
     * @brief compare if endpoint is lower or equal.
     * @param a endpoint to compare.
     * @param b endpoint to compare to.
     * @return true if lower or equal, false otherwise.
     */
    template <class Protocol>
    bool operator<= (const BasicNetlinkEndpoint<Protocol>& a, const BasicNetlinkEndpoint<Protocol>& b) noexcept
    {
        return !(b < a);
    }

    /**
     * @brief compare if endpoint is greater or equal.
     * @param a endpoint to compare.
     * @param b endpoint to compare to.
     * @return true if greater or equal, false otherwise.
     */
    template <class Protocol>
    bool operator>= (const BasicNetlinkEndpoint<Protocol>& a, const BasicNetlinkEndpoint<Protocol>& b) noexcept
    {
        return !(a < b);
    }

    /**
     * @brief push endpoint representation into a stream.
     * @param os output stream.
     * @param endpoint endpoint to push.
     * @return output stream.
     */
    template <class Protocol>
    std::ostream& operator<< (std::ostream& os, const BasicNetlinkEndpoint<Protocol>& endpoint)
    {
        os << "pid=" << endpoint.pid () << ",groups=" << endpoint.groups ();
        return os;
    }
}

#endif
