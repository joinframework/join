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

#ifndef __JOIN_ENDPOINT_HPP__
#define __JOIN_ENDPOINT_HPP__

// libjoin.
#include <join/resolver.hpp>

// C++.
#include <ostream>
#include <sstream>
#include <regex>

// C.
#include <linux/netfilter/nfnetlink.h>
#include <linux/if_packet.h>
#include <linux/rtnetlink.h>
#include <linux/netlink.h>
#include <sys/un.h>
#include <cstring>

namespace join
{
    /**
     * @brief basic endpoint class.
     */
    template <class Protocol>
    class BasicEndpoint
    {
    public:
        /**
         * @brief default constructor.
         */
        constexpr BasicEndpoint () noexcept
        {
            this->_addr.ss_family = Protocol ().family ();
        }

        /**
         * @brief create instance using socket address.
         * @param addr socket address.
         * @param len socket address length.
         */
        BasicEndpoint (const struct sockaddr* addr, socklen_t len) noexcept
        {
            memcpy (&this->_addr, addr, len);
        }

        /**
         * @brief get socket address.
         * @return socket address.
         */
        struct sockaddr* addr () noexcept
        {
            return reinterpret_cast <struct sockaddr*> (&this->_addr);
        }

        /**
         * @brief get socket address.
         * @return socket address.
         */
        const struct sockaddr* addr () const noexcept
        {
            return reinterpret_cast <const struct sockaddr*> (&this->_addr);
        }

    protected:
        /// socket address storage.
        struct sockaddr_storage _addr = {};
    };

    /**
     * @brief basic unix endpoint class.
     */
    template <class Protocol>
    class BasicUnixEndpoint : public BasicEndpoint <Protocol>
    {
    public:
        /**
         * @brief default constructor.
         */
        constexpr BasicUnixEndpoint () noexcept
        : BasicEndpoint <Protocol> ()
        {
        }

        /**
         * @brief create instance using socket address.
         * @param addr socket address.
         * @param len socket address length.
         */
        BasicUnixEndpoint (const struct sockaddr* addr, socklen_t len) noexcept
        : BasicEndpoint <Protocol> (addr, len)
        {
        }

        /**
         * @brief create instance using device name.
         * @param dev device name to set.
         */
        BasicUnixEndpoint (const char* dev) noexcept
        : BasicUnixEndpoint ()
        {
            struct sockaddr_un* sa = reinterpret_cast <struct sockaddr_un*> (&this->_addr);
            strncpy (sa->sun_path, dev, sizeof (sa->sun_path) - 1);
        }

        /**
         * @brief create instance using device name.
         * @param dev device name to set.
         */
        BasicUnixEndpoint (const std::string& dev) noexcept
        : BasicUnixEndpoint <Protocol> (dev.c_str ())
        {
        }

        /**
         * @brief get endpoint protocol.
         * @return endpoint protocol.
         */
        constexpr Protocol protocol () const noexcept
        {
            return Protocol ();
        }

        /**
         * @brief get socket address length.
         * @return socket address length.
         */
        constexpr socklen_t length () const noexcept
        {
            return sizeof (struct sockaddr_un);
        }

        /**
         * @brief set endpoint device name.
         * @param dev device name to set.
         */
        void device (const std::string& dev) noexcept
        {
            struct sockaddr_un* sa = reinterpret_cast <struct sockaddr_un*> (&this->_addr);
            strncpy (sa->sun_path, dev.c_str (), sizeof (sa->sun_path) - 1);
        }

        /**
         * @brief get endpoint device name.
         * @return endpoint device name.
         */
        std::string device () const
        {
            return reinterpret_cast <const struct sockaddr_un*> (&this->_addr)->sun_path;
        }
    };

    /**
     * @brief compare if endpoints are equal.
     * @param a endpoint to compare.
     * @param b endpoint to compare to.
     * @return true if endpoints are equal, false otherwise.
     */
    template <class Protocol>
    bool operator== (const BasicUnixEndpoint <Protocol>& a, const BasicUnixEndpoint <Protocol>& b) noexcept
    {
        return a.device () == b.device ();
    }

    /**
     * @brief compare if endpoints are not equal.
     * @param a endpoint to compare.
     * @param b endpoint to compare to.
     * @return true if endpoints are not equal, false otherwise.
     */
    template <class Protocol>
    bool operator!= (const BasicUnixEndpoint <Protocol>& a, const BasicUnixEndpoint <Protocol>& b) noexcept
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
    bool operator< (const BasicUnixEndpoint <Protocol>& a, const BasicUnixEndpoint <Protocol>& b) noexcept
    {
        return a.device () < b.device ();
    }

    /**
     * @brief compare if endpoint is greater.
     * @param a endpoint to compare.
     * @param b endpoint to compare to.
     * @return true if greater, false otherwise.
     */
    template <class Protocol>
    bool operator> (const BasicUnixEndpoint <Protocol>& a, const BasicUnixEndpoint <Protocol>& b) noexcept
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
    bool operator<= (const BasicUnixEndpoint <Protocol>& a, const BasicUnixEndpoint <Protocol>& b) noexcept
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
    bool operator>= (const BasicUnixEndpoint <Protocol>& a, const BasicUnixEndpoint <Protocol>& b) noexcept
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
    std::ostream& operator<< (std::ostream& os, const BasicUnixEndpoint <Protocol>& endpoint)
    {
        os << endpoint.device ();
        return os;
    }

    /**
     * @brief basic netlink endpoint class.
     */
    template <class Protocol>
    class BasicNetlinkEndpoint : public BasicEndpoint <Protocol>
    {
    public:
        /**
         * @brief default constructor.
         */
        constexpr BasicNetlinkEndpoint () noexcept
        : BasicEndpoint <Protocol> ()
        , _protocol (Protocol ().protocol ())
        {
        }

        /**
         * @brief create instance using socket address.
         * @param addr socket address.
         * @param len socket address length.
         */
        BasicNetlinkEndpoint (const struct sockaddr* addr, socklen_t len) noexcept
        : BasicEndpoint <Protocol> (addr, len)
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
        : BasicEndpoint <Protocol> ()
        , _protocol (protocol.protocol ())
        {
            struct sockaddr_nl* nl = reinterpret_cast <struct sockaddr_nl*> (&this->_addr);
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
            reinterpret_cast <struct sockaddr_nl*> (&this->_addr)->nl_pid = pid;
        }

        /**
         * @brief get process id.
         * @return process id.
         */
        uint32_t pid () const noexcept
        {
            return reinterpret_cast <const struct sockaddr_nl*> (&this->_addr)->nl_pid;
        }

        /**
         * @brief set netlink groups.
         * @param groups netlink groups bitmask.
         */
        void groups (uint32_t groups) noexcept
        {
            reinterpret_cast <struct sockaddr_nl*> (&this->_addr)->nl_groups = groups;
        }

        /**
         * @brief get netlink groups.
         * @return netlink groups bitmask.
         */
        uint32_t groups () const noexcept
        {
            return reinterpret_cast <const struct sockaddr_nl*> (&this->_addr)->nl_groups;
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
    bool operator== (const BasicNetlinkEndpoint <Protocol>& a, const BasicNetlinkEndpoint <Protocol>& b) noexcept
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
    bool operator!= (const BasicNetlinkEndpoint <Protocol>& a, const BasicNetlinkEndpoint <Protocol>& b) noexcept
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
    bool operator< (const BasicNetlinkEndpoint <Protocol>& a, const BasicNetlinkEndpoint <Protocol>& b) noexcept
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
    bool operator> (const BasicNetlinkEndpoint <Protocol>& a, const BasicNetlinkEndpoint <Protocol>& b) noexcept
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
    bool operator<= (const BasicNetlinkEndpoint <Protocol>& a, const BasicNetlinkEndpoint <Protocol>& b) noexcept
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
    bool operator>= (const BasicNetlinkEndpoint <Protocol>& a, const BasicNetlinkEndpoint <Protocol>& b) noexcept
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
    std::ostream& operator<< (std::ostream& os, const BasicNetlinkEndpoint <Protocol>& endpoint)
    {
        os << "pid=" << endpoint.pid () << ",groups=" << endpoint.groups ();
        return os;
    }

    /**
     * @brief basic link layer endpoint class.
     */
    template <class Protocol>
    class BasicLinkLayerEndpoint : public BasicEndpoint <Protocol>
    {
    public:
        /**
         * @brief default constructor.
         */
        BasicLinkLayerEndpoint () noexcept
        : BasicEndpoint <Protocol> ()
        {
            reinterpret_cast <struct sockaddr_ll*> (&this->_addr)->sll_protocol = this->protocol ().protocol ();
        }

        /**
         * @brief create instance using socket address.
         * @param addr socket address.
         * @param len socket address length.
         */
        BasicLinkLayerEndpoint (const struct sockaddr* addr, socklen_t len) noexcept
        : BasicEndpoint <Protocol> (addr, len)
        {
        }

        /**
         * @brief create instance using device name.
         * @param dev device name to set.
         */
        BasicLinkLayerEndpoint (const char* dev) noexcept
        : BasicEndpoint <Protocol> ()
        {
            reinterpret_cast <struct sockaddr_ll*> (&this->_addr)->sll_ifindex = if_nametoindex (dev);
        }

        /**
         * @brief create instance using device name.
         * @param dev device name to set.
         */
        BasicLinkLayerEndpoint (const std::string& dev) noexcept
        : BasicLinkLayerEndpoint <Protocol> (dev.c_str ())
        {
        }

        /**
         * @brief get endpoint protocol.
         * @return endpoint protocol.
         */
        constexpr Protocol protocol () const noexcept
        {
            return Protocol ();
        }

        /**
         * @brief get socket address length.
         * @return socket address length.
         */
        constexpr socklen_t length () const noexcept
        {
            return sizeof (struct sockaddr_ll);
        }

        /**
         * @brief set endpoint device name.
         * @param dev device name to set.
         */
        void device (const std::string& dev) noexcept
        {
            reinterpret_cast <struct sockaddr_ll*> (&this->_addr)->sll_ifindex = if_nametoindex (dev.c_str ());
        }

        /**
         * @brief get endpoint device name.
         * @return endpoint device name.
         */
        std::string device () const
        {
            char ifname[IFNAMSIZ];
            if (if_indextoname (reinterpret_cast <const struct sockaddr_ll*> (&this->_addr)->sll_ifindex, ifname))
            {
                return ifname;
            }
            return {};
        }
    };

    /**
     * @brief compare if endpoints are equal.
     * @param a endpoint to compare.
     * @param b endpoint to compare to.
     * @return true if endpoints are equal, false otherwise.
     */
    template <class Protocol>
    bool operator== (const BasicLinkLayerEndpoint <Protocol>& a, const BasicLinkLayerEndpoint <Protocol>& b) noexcept
    {
        return a.device () == b.device ();
    }

    /**
     * @brief compare if endpoints are not equal.
     * @param a endpoint to compare.
     * @param b endpoint to compare to.
     * @return true if endpoints are not equal, false otherwise.
     */
    template <class Protocol>
    bool operator!= (const BasicLinkLayerEndpoint <Protocol>& a, const BasicLinkLayerEndpoint <Protocol>& b) noexcept
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
    bool operator< (const BasicLinkLayerEndpoint <Protocol>& a, const BasicLinkLayerEndpoint <Protocol>& b) noexcept
    {
        return a.device () < b.device ();
    }

    /**
     * @brief compare if endpoint is greater.
     * @param a endpoint to compare.
     * @param b endpoint to compare to.
     * @return true if greater, false otherwise.
     */
    template <class Protocol>
    bool operator> (const BasicLinkLayerEndpoint <Protocol>& a, const BasicLinkLayerEndpoint <Protocol>& b) noexcept
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
    bool operator<= (const BasicLinkLayerEndpoint <Protocol>& a, const BasicLinkLayerEndpoint <Protocol>& b) noexcept
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
    bool operator>= (const BasicLinkLayerEndpoint <Protocol>& a, const BasicLinkLayerEndpoint <Protocol>& b) noexcept
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
    std::ostream& operator<< (std::ostream& os, const BasicLinkLayerEndpoint <Protocol>& endpoint)
    {
        os << endpoint.device ();
        return os;
    }

    /**
     * @brief basic internet endpoint class.
     */
    template <class Protocol>
    class BasicInternetEndpoint : public BasicEndpoint <Protocol>
    {
    public:
        /**
         * @brief default constructor.
         */
        constexpr BasicInternetEndpoint () noexcept
        : BasicEndpoint <Protocol> ()
        {
        }

        /**
         * @brief create the endpoint instance.
         * @param addr socket address.
         */
        BasicInternetEndpoint (const struct sockaddr* addr, socklen_t len) noexcept
        : BasicEndpoint <Protocol> (addr, len)
        {
        }

        /**
         * @brief create the endpoint instance.
         * @param url endpoint URL.
         */
        BasicInternetEndpoint (const char* url)
        : BasicEndpoint <Protocol> ()
        {
            // regular expression inspired by rfc3986 (see https://www.ietf.org/rfc/rfc3986.txt)
            // ex.
            // 0: https://example.com:8080/foo/bar.html?val=1#frag  # URL
            // 1: https                                             # Scheme
            // 2: example.com                                       # Host
            // 3: 8080                                              # Port
            // 4: /foo/bar.html                                     # Path
            // 5: val=1                                             # Query
            // 6: frag                                              # Fragment
            //                0    1              2                                    3         4            5            6
            std::regex reg (R"(^(?:([^:/?#]+)://)?([a-z0-9\-._~%]+|\[[a-f0-9:.]+\])(?::([0-9]+))?([^?#]*)(?:\?([^#]*))?(?:#(.*))?)");
            std::cmatch match;

            this->_hostname = url;
            uint16_t port = 0;

            if (std::regex_match (url, match, reg))
            {
                this->_hostname = match[2];
                this->_hostname.erase (0, this->_hostname.find_first_not_of ("["));
                this->_hostname.erase (this->_hostname.find_last_not_of ("]") + 1);
                port = (match[3].length ()) ? uint16_t (std::stoi (match[3])) : Resolver::resolveService (match[1]);
            }

            try
            {
                this->ip (this->_hostname);
                this->port (port);
            }
            catch (const std::exception& ex)
            {
                this->ip (Resolver::resolveHost (this->_hostname));
                this->port (port);
            }
        }

        /**
         * @brief create the endpoint instance.
         * @param url endpoint URL.
         */
        BasicInternetEndpoint (const std::string& url)
        : BasicInternetEndpoint <Protocol> (url.c_str ())
        {
        }

        /**
         * @brief create the endpoint instance.
         * @param ip endpoint IP address.
         * @param port endpoint port number.
         */
        BasicInternetEndpoint (const IpAddress& ip, uint16_t port = 0) noexcept
        : BasicEndpoint <Protocol> ()
        {
            if (ip.family () == AF_INET6)
            {
                struct sockaddr_in6* sa = reinterpret_cast <struct sockaddr_in6*> (&this->_addr);
                sa->sin6_family         = AF_INET6;
                sa->sin6_port           = htons (port);
                memcpy (&sa->sin6_addr, ip.addr (), ip.length ());
                sa->sin6_scope_id       = ip.scope ();
            }
            else
            {
                struct sockaddr_in* sa  = reinterpret_cast <struct sockaddr_in*> (&this->_addr);
                sa->sin_family          = AF_INET;
                sa->sin_port            = htons (port);
                memcpy (&sa->sin_addr, ip.addr (), ip.length ());
            }
        }

        /**
         * @brief create the endpoint instance.
         * @param protocol endpoint protocol.
         * @param port endpoint port number.
         */
        BasicInternetEndpoint (const Protocol& protocol, uint16_t port = 0) noexcept
        : BasicEndpoint <Protocol> ()
        {
            if (protocol.family () == AF_INET6)
            {
                struct sockaddr_in6* sa = reinterpret_cast <struct sockaddr_in6*> (&this->_addr);
                sa->sin6_family         = AF_INET6;
                sa->sin6_port           = htons (port);
            }
            else
            {
                struct sockaddr_in* sa  = reinterpret_cast <struct sockaddr_in*> (&this->_addr);
                sa->sin_family          = AF_INET;
                sa->sin_port            = htons (port);
            }
        }

        /**
         * @brief set endpoint hostname.
         * @param hostname hostname to set.
         */
        void hostname (const std::string& hostname) noexcept
        {
            _hostname = hostname;
        }

        /**
         * @brief get the endpoint hostname.
         * @return endpoint hostname.
         */
        const std::string& hostname () const noexcept
        {
            return _hostname;
        }

        /**
         * @brief set endpoint IP address.
         * @param ip ip address to set.
         */
        void ip (const IpAddress& ip) noexcept
        {
            if (ip.family () == AF_INET6)
            {
                struct sockaddr_in6* sa = reinterpret_cast <struct sockaddr_in6*> (&this->_addr);
                sa->sin6_family         = AF_INET6;
                memcpy (&sa->sin6_addr, ip.addr (), ip.length ());
                sa->sin6_scope_id       = ip.scope ();
            }
            else
            {
                struct sockaddr_in* sa  = reinterpret_cast <struct sockaddr_in*> (&this->_addr);
                sa->sin_family          = AF_INET;
                memcpy (&sa->sin_addr, ip.addr (), ip.length ());
            }
        }

        /**
         * @brief get endpoint IP address.
         * @return endpoint IP address.
         */
        IpAddress ip () const noexcept
        {
            return *reinterpret_cast <const struct sockaddr*> (&this->_addr);
        }

        /**
         * @brief set endpoint port number.
         * @param p port number to set.
         */
        void port (uint16_t p) noexcept
        {
            if (this->_addr.ss_family == AF_INET6)
            {
                reinterpret_cast <struct sockaddr_in6*> (&this->_addr)->sin6_port = htons (p);
            }
            else
            {
                reinterpret_cast <struct sockaddr_in*> (&this->_addr)->sin_port = htons (p);
            }
        }

        /**
         * @brief get endpoint port number.
         * @return endpoint port number.
         */
        uint16_t port () const noexcept
        {
            if (this->_addr.ss_family == AF_INET6)
            {
                return ntohs (reinterpret_cast <const struct sockaddr_in6*> (&this->_addr)->sin6_port);
            }
            else
            {
                return ntohs (reinterpret_cast <const struct sockaddr_in*> (&this->_addr)->sin_port);
            }
        }

        /**
         * @brief get endpoint protocol.
         * @return endpoint protocol.
         */
        Protocol protocol () const noexcept
        {
            if (this->_addr.ss_family == AF_INET)
            {
                return Protocol::v4 ();
            }
            return Protocol::v6 ();
        }

        /**
         * @brief get socket address length.
         * @return socket address length.
         */
        socklen_t length () const noexcept
        {
            if (this->_addr.ss_family == AF_INET)
            {
                return sizeof (struct sockaddr_in);
            }
            return sizeof (struct sockaddr_in6);
        }

        /**
         * @brief set endpoint device name.
         * @param dev device name to set.
         */
        void device (const std::string& dev) noexcept
        {
            if (this->_addr.ss_family == AF_INET6)
            {
                reinterpret_cast <struct sockaddr_in6*> (&this->_addr)->sin6_scope_id = if_nametoindex (dev.c_str ());
            }
        }

        /**
         * @brief get endpoint device name.
         * @return endpoint device name.
         */
        std::string device () const
        {
            if (this->_addr.ss_family == AF_INET6)
            {
                char ifname[IFNAMSIZ];
                if (if_indextoname (reinterpret_cast <const struct sockaddr_in6*> (&this->_addr)->sin6_scope_id, ifname))
                {
                    return ifname;
                }
            }
            return {};
        }

    protected:
        /// endpoint hostname.
        std::string _hostname;
    };

    /**
     * @brief compare if equal.
     * @param a endpoint to compare.
     * @param b endpoint to compare to.
     * @return true if endpoints is equal, false otherwise.
     */
    template <class Protocol>
    bool operator== (const BasicInternetEndpoint <Protocol>& a, const BasicInternetEndpoint <Protocol>& b) noexcept
    {
        return a.ip () == b.ip () && a.port () == b.port ();
    }

    /**
     * @brief compare if not equal.
     * @param a endpoint to compare.
     * @param b endpoint to compare to.
     * @return true if endpoints is not equal, false otherwise.
     */
    template <class Protocol>
    bool operator!= (const BasicInternetEndpoint <Protocol>& a, const BasicInternetEndpoint <Protocol>& b) noexcept
    {
        return !(a == b);
    }

    /**
     * @brief compare if lower.
     * @param a endpoint to compare.
     * @param b endpoint to compare to.
     * @return true if lower, false otherwise.
     */
    template <class Protocol>
    bool operator< (const BasicInternetEndpoint <Protocol>& a, const BasicInternetEndpoint <Protocol>& b) noexcept
    {
        return std::tie (a.ip (), a.port ()) < std::tie (b.ip (), b.port ());
    }

    /**
     * @brief compare if greater.
     * @param a endpoint to compare.
     * @param b endpoint to compare to.
     * @return true if greater, false otherwise.
     */
    template <class Protocol>
    bool operator> (const BasicInternetEndpoint <Protocol>& a, const BasicInternetEndpoint <Protocol>& b) noexcept
    {
        return b < a;
    }

    /**
     * @brief compare if lower or equal.
     * @param a endpoint to compare.
     * @param b endpoint to compare to.
     * @return true if lower or equal, false otherwise.
     */
    template <class Protocol>
    bool operator<= (const BasicInternetEndpoint <Protocol>& a, const BasicInternetEndpoint <Protocol>& b) noexcept
    {
        return !(b < a);
    }

    /**
     * @brief compare if greater or equal.
     * @param a endpoint to compare.
     * @param b endpoint to compare to.
     * @return true if greater or equal, false otherwise.
     */
    template <class Protocol>
    bool operator>= (const BasicInternetEndpoint <Protocol>& a, const BasicInternetEndpoint <Protocol>& b) noexcept
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
    std::ostream& operator<< (std::ostream& os, const BasicInternetEndpoint <Protocol>& endpoint)
    {
        if (endpoint.protocol () == Protocol::v6 ())
            os << "[" << endpoint.ip () << "]";
        else
            os << endpoint.ip ();
        if (endpoint.port ())
            os << ":" << endpoint.port ();
        return os;
    }
}

#endif
