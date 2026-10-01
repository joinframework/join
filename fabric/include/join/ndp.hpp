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

#ifndef JOIN_FABRIC_NDP_HPP
#define JOIN_FABRIC_NDP_HPP

// libjoin.
#include <join/datagram_socket.hpp>
#include <join/ndp_protocol.hpp>
#include <join/ndp_message.hpp>
#include <join/condition.hpp>
#include <join/notifier.hpp>
#include <join/function.hpp>
#include <join/reactor.hpp>
#include <join/error.hpp>

// C++.
#include <system_error>
#include <algorithm>
#include <chrono>
#include <memory>
#include <string>
#include <vector>

// C.
#include <netinet/icmp6.h>
#include <net/if.h>

namespace join
{
    /**
     * @brief carries NDP messages over an ICMPv6 socket.
     */
    template <class Protocol>
    class BasicNdp : public EventHandler
    {
    public:
        using Socket = typename Protocol::Socket;
        using Endpoint = typename Protocol::Endpoint;

        /// neighbor solicitation notification callback.
        using NeighborSolicitationNotify = Function<void (const NeighborSolicitation& solicitation)>;

        /// neighbor advertisement notification callback.
        using NeighborAdvertisementNotify = Function<void (const NeighborAdvertisement& advert)>;

        /**
         * @brief create the BasicNdp instance.
         */
        BasicNdp () = delete;

        /**
         * @brief create the instance bound to the given interface.
         * @param interface interface name.
         * @param accept message type the instance accepts.
         * @param group multicast group the instance joins, wildcard to join none.
         * @param reactor reactor instance.
         * @throw std::system_error if the interface is unknown or the socket could not be set up.
         */
        BasicNdp (const std::string& interface, uint8_t accept, const IpAddress& group, Reactor& reactor)
        : _socket (Protocol::hopLimit)
        , _buffer (std::make_unique<char[]> (Protocol::maxMsgSize))
        , _interface (interface)
        , _index (lookup (interface))
        , _hardware (MacAddress::address (interface))
        , _reactor (reactor)
        {
            if (_socket.open (Protocol::Transport::v6 ()) == -1)
            {
                throw std::system_error (lastError, "ndp socket open failed");  // LCOV_EXCL_LINE
            }

            if (_socket.bindToDevice (_interface) == -1)
            {
                throw std::system_error (lastError, "ndp socket bind to device failed");  // LCOV_EXCL_LINE
            }

            int on = 1;

            if (::setsockopt (_socket.handle (), IPPROTO_IPV6, IPV6_MULTICAST_IF, &_index, sizeof (_index)) == -1)
            {
                // LCOV_EXCL_START
                throw std::system_error (errno, std::generic_category (), "ndp multicast setup failed");
                // LCOV_EXCL_STOP
            }

            if (::setsockopt (_socket.handle (), IPPROTO_IPV6, IPV6_RECVHOPLIMIT, &on, sizeof (on)) == -1)
            {
                // LCOV_EXCL_START
                throw std::system_error (errno, std::generic_category (), "ndp hop limit setup failed");
                // LCOV_EXCL_STOP
            }

            if (::setsockopt (_socket.handle (), IPPROTO_IPV6, IPV6_DONTFRAG, &on, sizeof (on)) == -1)
            {
                // LCOV_EXCL_START
                throw std::system_error (errno, std::generic_category (), "ndp dontfrag setup failed");
                // LCOV_EXCL_STOP
            }

            struct icmp6_filter filter;
            ICMP6_FILTER_SETBLOCKALL (&filter);
            ICMP6_FILTER_SETPASS (accept, &filter);
            ICMP6_FILTER_SETPASS (NdpMessage::NeighborAdvert, &filter);
            ICMP6_FILTER_SETPASS (NdpMessage::NeighborSolicit, &filter);

            if (::setsockopt (_socket.handle (), IPPROTO_ICMPV6, ICMP6_FILTER, &filter, sizeof (filter)) == -1)
            {
                // LCOV_EXCL_START
                throw std::system_error (errno, std::generic_category (), "ndp icmp6 filter setup failed");
                // LCOV_EXCL_STOP
            }

            if (!group.isWildcard ())
            {
                struct ipv6_mreq mreq = {};
                ::memcpy (&mreq.ipv6mr_multiaddr, group.addr (), group.length ());
                mreq.ipv6mr_interface = _index;

                if (::setsockopt (_socket.handle (), IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, &mreq, sizeof (mreq)) == -1)
                {
                    // LCOV_EXCL_START
                    throw std::system_error (errno, std::generic_category (), "ndp multicast membership setup failed");
                    // LCOV_EXCL_STOP
                }
            }
        }

        /**
         * @brief create instance by copy.
         * @param other other object to copy.
         */
        BasicNdp (const BasicNdp& other) = delete;

        /**
         * @brief assign instance by copy.
         * @param other other object to copy.
         * @return a reference of the current object.
         */
        BasicNdp& operator= (const BasicNdp& other) = delete;

        /**
         * @brief create instance by move.
         * @param other other object to move.
         */
        BasicNdp (BasicNdp&& other) = delete;

        /**
         * @brief assign instance by move.
         * @param other other object to move.
         * @return a reference of the current object.
         */
        BasicNdp& operator= (BasicNdp&& other) = delete;

        /**
         * @brief destroy the instance.
         */
        virtual ~BasicNdp () = default;

        /**
         * @brief send a neighbor solicitation and wait for the advertisement of the target.
         * @param target address of the neighbor to resolve.
         * @param advert receives the advertisement.
         * @param timeout maximum wait duration.
         * @return 0 on success, -1 on failure.
         */
        int neighborSolicit (const IpAddress& target, NeighborAdvertisement& advert,
                             std::chrono::milliseconds timeout = std::chrono::seconds (1)) noexcept
        {
            if (_reactor.isReactorThread ())
            {
                lastError = std::make_error_code (std::errc::resource_deadlock_would_occur);
                return -1;
            }

            NeighborSolicitation out;
            out.target = target;
            out.link = _hardware;

            char payload[Protocol::maxMsgSize];
            ssize_t size = _message.serialize (out, payload, sizeof (payload));
            if (size == -1)
            {
                return -1;
            }

            const IpAddress group = IpAddress::ipv6SolicitedNodes | (target & ~IpAddress (104, AF_INET6));

            ScopedLock<Mutex> lock (_neighborMutex);

            NeighborRequest pending;
            pending.target = IpAddress (target.addr (), target.length ());
            pending.advert = &advert;
            _neighborPending.push_back (&pending);

            if (send (payload, static_cast<size_t> (size), group) == -1)
            {
                // LCOV_EXCL_START
                _neighborPending.erase (std::find (_neighborPending.begin (), _neighborPending.end (), &pending));
                return -1;
                // LCOV_EXCL_STOP
            }

            bool answered = pending.cond.timedWait (lock, timeout, [&pending] {
                return pending.done;
            });

            _neighborPending.erase (std::find (_neighborPending.begin (), _neighborPending.end (), &pending));

            if (!answered)
            {
                lastError = make_error_code (Errc::TimedOut);
                return -1;
            }

            return 0;
        }

        /**
         * @brief send a neighbor advertisement, the interface hardware address is used if it carries none.
         * @param advert advertisement to send.
         * @param destination destination address, all the nodes of the link by default.
         * @return 0 on success, -1 on failure.
         */
        int neighborAdvertise (const NeighborAdvertisement& advert,
                               const IpAddress& destination = IpAddress::ipv6AllNodes) noexcept
        {
            char payload[Protocol::maxMsgSize];
            ssize_t size = _message.serialize (advert, payload, sizeof (payload),
                                               advert.link.isWildcard () ? _hardware : advert.link);
            if (size == -1)
            {
                return -1;
            }

            return send (payload, static_cast<size_t> (size), destination);
        }

        /**
         * @brief set the callback called on every neighbor solicitation received.
         * @param cb callback, called from the reactor thread, must not throw nor destroy the instance.
         * @return 0 on success, -1 on failure.
         */
        int setNeighborSolicitationListener (NeighborSolicitationNotify cb) noexcept
        {
            return _neighborSolicitationListener.set (std::move (cb));
        }

        /**
         * @brief unset the neighbor solicitation callback, it is no longer called once this returns.
         * @return 0 on success, -1 on failure.
         */
        int unsetNeighborSolicitationListener () noexcept
        {
            return _neighborSolicitationListener.unset ();
        }

        /**
         * @brief set the callback called on every neighbor advertisement received, solicited or not.
         * @param cb callback, called from the reactor thread, must not throw nor destroy the instance.
         * @return 0 on success, -1 on failure.
         */
        int setNeighborAdvertisementListener (NeighborAdvertisementNotify cb) noexcept
        {
            return _neighborAdvertisementListener.set (std::move (cb));
        }

        /**
         * @brief unset the neighbor advertisement callback, it is no longer called once this returns.
         * @return 0 on success, -1 on failure.
         */
        int unsetNeighborAdvertisementListener () noexcept
        {
            return _neighborAdvertisementListener.unset ();
        }

        /**
         * @brief get the name of the interface the instance is bound to.
         * @return the interface name.
         */
        const std::string& interface () const noexcept
        {
            return _interface;
        }

        /**
         * @brief get the index of the interface the instance is bound to.
         * @return the interface index.
         */
        unsigned int index () const noexcept
        {
            return _index;
        }

        /**
         * @brief get the hardware address of the interface the instance is bound to.
         * @return the hardware address.
         */
        const MacAddress& hardware () const noexcept
        {
            return _hardware;
        }

    protected:
        /**
         * @brief register with the reactor, to be called by the final class once fully constructed.
         * @throw std::system_error if the registration failed.
         */
        void attach ()
        {
            if (_reactor.addHandler (_socket.handle (), this) == -1)
            {
                throw std::system_error (lastError, "ndp reactor registration failed");  // LCOV_EXCL_LINE
            }
        }

        /**
         * @brief unregister from the reactor, to be called by the final class before destruction starts.
         */
        void detach () noexcept
        {
            _reactor.delHandler (_socket.handle ());
        }

        /**
         * @brief write a message on the interface.
         * @param data message to write.
         * @param size message size.
         * @param destination destination address.
         * @return 0 on success, -1 on failure.
         */
        int send (const char* data, size_t size, const IpAddress& destination)
        {
            if (destination.family () != AF_INET6)
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            if (size > Protocol::maxMsgSize)
            {
                // LCOV_EXCL_START
                lastError = make_error_code (Errc::MessageTooLong);
                return -1;
                // LCOV_EXCL_STOP
            }

            Endpoint endpoint (IpAddress (destination.addr (), destination.length (), _index));

            return (_socket.writeTo (data, size, endpoint) == -1) ? -1 : 0;
        }

        /**
         * @brief read a message that passed the checks common to every NDP message.
         * @param data buffer used to store the message.
         * @param maxSize maximum number of bytes to read.
         * @param from address the message came from.
         * @return the message size, -1 if nothing valid was read.
         */
        ssize_t receive (char* data, size_t maxSize, IpAddress& from)
        {
            Endpoint endpoint;
            char control[CMSG_SPACE (sizeof (int))];
            size_t controlSize = sizeof (control);

            ssize_t size = _socket.readFrom (data, maxSize, &endpoint, control, &controlSize);
            if (size <= 0)
            {
                return -1;  // LCOV_EXCL_LINE
            }

            struct msghdr msg = {};
            msg.msg_control = control;
            msg.msg_controllen = controlSize;

            int hop = 0;

            for (struct cmsghdr* cmsg = CMSG_FIRSTHDR (&msg); cmsg != nullptr; cmsg = CMSG_NXTHDR (&msg, cmsg))
            {
                if ((cmsg->cmsg_level == IPPROTO_IPV6) && (cmsg->cmsg_type == IPV6_HOPLIMIT))
                {
                    ::memcpy (&hop, CMSG_DATA (cmsg), sizeof (hop));
                }
            }

            if (hop != Protocol::hopLimit)
            {
                return -1;
            }

            from = endpoint.ip ();

            return size;
        }

        /**
         * @brief hand a received neighbor message to its listener, and an advertisement to the solicitations waiting
         * for it.
         * @param size message size.
         * @param from address the message came from.
         * @return true if the message is a neighbor solicitation or advertisement, false otherwise.
         */
        bool onNeighborMessage (size_t size, const IpAddress& from) noexcept
        {
            const uint8_t type = static_cast<uint8_t> (_buffer[0]);

            if (type == NdpMessage::NeighborSolicit)
            {
                NeighborSolicitation solicitation;
                if ((_message.deserialize (solicitation, _buffer.get (), size) == 0) &&
                    (!from.isWildcard () || solicitation.link.isWildcard ()))
                {
                    solicitation.src = from;
                    _neighborSolicitationListener.notify (solicitation);
                }

                return true;
            }

            if (type != NdpMessage::NeighborAdvert)
            {
                return false;
            }

            NeighborAdvertisement advert;
            if (_message.deserialize (advert, _buffer.get (), size) == 0)
            {
                advert.src = from;

                _neighborAdvertisementListener.notify (advert);

                ScopedLock<Mutex> lock (_neighborMutex);

                for (NeighborRequest* pending : _neighborPending)
                {
                    if (!pending->done && (pending->target == advert.target))
                    {
                        *pending->advert = advert;
                        pending->done = true;
                        pending->cond.signal ();
                    }
                }
            }

            return true;
        }

        /**
         * @brief get the index of the given interface.
         * @param interface interface name.
         * @return the interface index.
         * @throw std::system_error if the interface is unknown.
         */
        static unsigned int lookup (const std::string& interface)
        {
            unsigned int index = ::if_nametoindex (interface.c_str ());
            if (index == 0)
            {
                throw std::system_error (errno, std::generic_category (), "ndp interface lookup failed");
            }

            return index;
        }

        /// underlying socket.
        Socket _socket;

        /// NDP message codec.
        NdpMessage _message;

        /// receive buffer.
        std::unique_ptr<char[]> _buffer;

        /// interface name.
        const std::string _interface;

        /// interface index.
        const unsigned int _index;

        /// hardware address of the interface.
        const MacAddress _hardware;

        /// event loop reactor.
        Reactor& _reactor;

        /**
         * @brief neighbor solicitation waiting for the advertisement of its target.
         */
        struct NeighborRequest
        {
            /// answer notification.
            Condition cond;

            /// address of the solicited neighbor.
            IpAddress target{AF_INET6};

            /// receives the advertisement.
            NeighborAdvertisement* advert = nullptr;

            /// set once the advertisement has been received.
            bool done = false;
        };

        /// neighbor solicitations waiting for an advertisement.
        std::vector<NeighborRequest*> _neighborPending;

        /// neighbor solicitations protection mutex.
        Mutex _neighborMutex;

        /// neighbor solicitation listener.
        Notifier<NeighborSolicitationNotify, Reactor> _neighborSolicitationListener{_reactor};

        /// neighbor advertisement listener.
        Notifier<NeighborAdvertisementNotify, Reactor> _neighborAdvertisementListener{_reactor};
    };

    /**
     * @brief NDP client, solicits routers and receives their advertisements.
     */
    template <class Protocol>
    class BasicNdpClient final : protected BasicNdp<Protocol>
    {
    public:
        using BasicNdp<Protocol>::interface;
        using BasicNdp<Protocol>::hardware;
        using BasicNdp<Protocol>::index;
        using BasicNdp<Protocol>::neighborSolicit;
        using BasicNdp<Protocol>::neighborAdvertise;
        using BasicNdp<Protocol>::setNeighborSolicitationListener;
        using BasicNdp<Protocol>::unsetNeighborSolicitationListener;
        using BasicNdp<Protocol>::setNeighborAdvertisementListener;
        using BasicNdp<Protocol>::unsetNeighborAdvertisementListener;
        using typename BasicNdp<Protocol>::NeighborSolicitationNotify;
        using typename BasicNdp<Protocol>::NeighborAdvertisementNotify;

        /// router advertisement notification callback.
        using RouterAdvertisementNotify = Function<void (const RouterAdvertisement& advert)>;

        /**
         * @brief create the BasicNdpClient instance.
         */
        BasicNdpClient () = delete;

        /**
         * @brief create the instance bound to the given interface and start receiving advertisements.
         * @param interface interface name.
         * @param reactor reactor instance.
         * @throw std::system_error if the interface is unknown or the socket could not be set up.
         */
        explicit BasicNdpClient (const std::string& interface, Reactor& reactor = ReactorThread::reactor ())
        : BasicNdp<Protocol> (interface, NdpMessage::RouterAdvert, IpAddress::ipv6Wildcard, reactor)
        {
            this->attach ();
        }

        /**
         * @brief stop receiving advertisements and destroy the instance.
         */
        ~BasicNdpClient ()
        {
            this->detach ();
        }

        /**
         * @brief send a router solicitation.
         * @return 0 on success, -1 on failure.
         */
        int routerSolicit () noexcept
        {
            RouterSolicitation out;
            out.link = hardware ();

            char payload[Protocol::maxMsgSize];
            ssize_t size = this->_message.serialize (out, payload, sizeof (payload));
            if (size == -1)
            {
                return -1;  // LCOV_EXCL_LINE
            }

            return this->send (payload, static_cast<size_t> (size), IpAddress::ipv6Routers);
        }

        /**
         * @brief send a router solicitation and wait for the first advertisement.
         * @param advert receives the first advertisement.
         * @param timeout maximum wait duration.
         * @return 0 on success, -1 on failure.
         */
        int routerSolicit (RouterAdvertisement& advert,
                           std::chrono::milliseconds timeout = std::chrono::seconds (1)) noexcept
        {
            if (this->_reactor.isReactorThread ())
            {
                lastError = std::make_error_code (std::errc::resource_deadlock_would_occur);
                return -1;
            }

            ScopedLock<Mutex> lock (_routerMutex);

            RouterRequest pending;
            pending.advert = &advert;
            _pending.push_back (&pending);

            if (routerSolicit () == -1)
            {
                // LCOV_EXCL_START
                _pending.erase (std::find (_pending.begin (), _pending.end (), &pending));
                return -1;
                // LCOV_EXCL_STOP
            }

            bool answered = pending.cond.timedWait (lock, timeout, [&pending] {
                return pending.done;
            });

            _pending.erase (std::find (_pending.begin (), _pending.end (), &pending));

            if (!answered)
            {
                lastError = make_error_code (Errc::TimedOut);
                return -1;
            }

            return 0;
        }

        /**
         * @brief set the callback called on every router advertisement received, solicited or not.
         * @param cb callback, called from the reactor thread, must not throw nor destroy the instance.
         * @return 0 on success, -1 on failure.
         */
        int setRouterAdvertisementListener (RouterAdvertisementNotify cb) noexcept
        {
            return _routerAdvertisementListener.set (std::move (cb));
        }

        /**
         * @brief unset the router advertisement callback, it is no longer called once this returns.
         * @return 0 on success, -1 on failure.
         */
        int unsetRouterAdvertisementListener () noexcept
        {
            return _routerAdvertisementListener.unset ();
        }

    private:
        /**
         * @brief decode a router advertisement and hand it to whoever waits for it.
         * @param fd file descriptor.
         */
        void onReadable ([[maybe_unused]] int fd) override final
        {
            IpAddress from;
            ssize_t size = this->receive (this->_buffer.get (), Protocol::maxMsgSize, from);
            if (size == -1)
            {
                return;
            }

            if (this->onNeighborMessage (static_cast<size_t> (size), from))
            {
                return;
            }

            if (!from.isLinkLocal ())
            {
                return;
            }

            RouterAdvertisement advert;
            if (this->_message.deserialize (advert, this->_buffer.get (), static_cast<size_t> (size)) == -1)
            {
                return;
            }

            advert.src = from;

            _routerAdvertisementListener.notify (advert);

            ScopedLock<Mutex> lock (_routerMutex);

            for (RouterRequest* pending : _pending)
            {
                if (!pending->done)
                {
                    *pending->advert = advert;
                    pending->done = true;
                    pending->cond.signal ();
                }
            }
        }

        /**
         * @brief solicitation waiting for an advertisement.
         */
        struct RouterRequest
        {
            /// answer notification.
            Condition cond;

            /// receives the first advertisement.
            RouterAdvertisement* advert = nullptr;

            /// set once an advertisement has been received.
            bool done = false;
        };

        /// solicitations waiting for an advertisement.
        std::vector<RouterRequest*> _pending;

        /// router solicitations protection mutex.
        Mutex _routerMutex;

        /// router advertisement listener.
        Notifier<RouterAdvertisementNotify, Reactor> _routerAdvertisementListener{this->_reactor};
    };

    /**
     * @brief NDP server, receives router solicitations and sends advertisements.
     */
    template <class Protocol>
    class BasicNdpServer final : protected BasicNdp<Protocol>
    {
    public:
        using BasicNdp<Protocol>::interface;
        using BasicNdp<Protocol>::hardware;
        using BasicNdp<Protocol>::index;
        using BasicNdp<Protocol>::neighborSolicit;
        using BasicNdp<Protocol>::neighborAdvertise;
        using BasicNdp<Protocol>::setNeighborSolicitationListener;
        using BasicNdp<Protocol>::unsetNeighborSolicitationListener;
        using BasicNdp<Protocol>::setNeighborAdvertisementListener;
        using BasicNdp<Protocol>::unsetNeighborAdvertisementListener;
        using typename BasicNdp<Protocol>::NeighborSolicitationNotify;
        using typename BasicNdp<Protocol>::NeighborAdvertisementNotify;

        /// router solicitation notification callback.
        using RouterSolicitationNotify = Function<void (const RouterSolicitation& solicitation)>;

        /**
         * @brief create the BasicNdpServer instance.
         */
        BasicNdpServer () = delete;

        /**
         * @brief create the instance bound to the given interface and start receiving solicitations.
         * @param interface interface name.
         * @param reactor reactor instance.
         * @throw std::system_error if the interface is unknown or the socket could not be set up.
         */
        explicit BasicNdpServer (const std::string& interface, Reactor& reactor = ReactorThread::reactor ())
        : BasicNdp<Protocol> (interface, NdpMessage::RouterSolicit, IpAddress::ipv6Routers, reactor)
        {
            this->attach ();
        }

        /**
         * @brief stop receiving solicitations and destroy the instance.
         */
        ~BasicNdpServer ()
        {
            this->detach ();
        }

        /**
         * @brief set the callback called on every router solicitation received.
         * @param cb callback, called from the reactor thread, must not throw nor destroy the instance.
         * @return 0 on success, -1 on failure.
         */
        int setRouterSolicitationListener (RouterSolicitationNotify cb) noexcept
        {
            return _routerSolicitationListener.set (std::move (cb));
        }

        /**
         * @brief unset the router solicitation callback, it is no longer called once this returns.
         * @return 0 on success, -1 on failure.
         */
        int unsetRouterSolicitationListener () noexcept
        {
            return _routerSolicitationListener.unset ();
        }

        /**
         * @brief send a router advertisement.
         * @param advert advertisement to send, the interface hardware address is used if it carries none.
         * @param destination destination address, the solicitor or all the nodes of the link.
         * @return 0 on success, -1 on failure.
         */
        int routerAdvertise (const RouterAdvertisement& advert,
                             const IpAddress& destination = IpAddress::ipv6AllNodes) noexcept
        {
            char payload[Protocol::maxMsgSize];
            ssize_t size = this->_message.serialize (advert, payload, sizeof (payload),
                                                     advert.link.isWildcard () ? hardware () : advert.link);
            if (size == -1)
            {
                return -1;
            }

            return this->send (payload, static_cast<size_t> (size), destination);
        }

        /**
         * @brief answer a router solicitation, to the solicitor or to all the nodes of the link if it has no address.
         * @param advert advertisement to send, the interface hardware address is used if it carries none.
         * @param solicitation solicitation to answer.
         * @return 0 on success, -1 on failure.
         */
        int routerAdvertise (const RouterAdvertisement& advert, const RouterSolicitation& solicitation) noexcept
        {
            return routerAdvertise (advert,
                                    solicitation.src.isWildcard () ? IpAddress::ipv6AllNodes : solicitation.src);
        }

    private:
        /**
         * @brief decode a router solicitation and hand it to the listeners.
         * @param fd file descriptor.
         */
        void onReadable ([[maybe_unused]] int fd) override final
        {
            IpAddress from;
            ssize_t size = this->receive (this->_buffer.get (), Protocol::maxMsgSize, from);
            if (size == -1)
            {
                return;
            }

            if (this->onNeighborMessage (static_cast<size_t> (size), from))
            {
                return;
            }

            RouterSolicitation solicitation;
            if (this->_message.deserialize (solicitation, this->_buffer.get (), static_cast<size_t> (size)) == -1)
            {
                return;
            }

            if (from.isWildcard () && !solicitation.link.isWildcard ())
            {
                return;
            }

            solicitation.src = from;

            _routerSolicitationListener.notify (solicitation);
        }

        /// router solicitation listener.
        Notifier<RouterSolicitationNotify, Reactor> _routerSolicitationListener{this->_reactor};
    };
}

#endif
