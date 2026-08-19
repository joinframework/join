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

#ifndef JOIN_FABRIC_PING_HPP
#define JOIN_FABRIC_PING_HPP

// libjoin.
#include <join/datagram_socket.hpp>
#include <join/ip_address.hpp>
#include <join/condition.hpp>
#include <join/reactor.hpp>

// C++.
#include <unordered_map>
#include <functional>
#include <chrono>
#include <memory>
#include <string>

// C.
#include <netinet/in.h>

namespace join
{
    /**
     * @brief answer to an echo request.
     */
    struct PingAnswer
    {
        /// address that answered.
        IpAddress from;

        /// round trip time.
        std::chrono::microseconds rtt{0};

        /// time to live of the answer.
        int ttl = 0;

        /// ICMP message size of the answer.
        int size = 0;

        /// MTU reported by the answer, zero if none was reported.
        uint32_t mtu = 0;

        /// set if a time exceeded message was received.
        bool expired = false;

        /// human readable form of the answer.
        std::string error;

        /// answer error code.
        std::error_code code;
    };

    /**
     * @brief ICMP echo statistics.
     */
    class PingStats
    {
    public:
        /**
         * @brief create the PingStats instance.
         */
        PingStats () = default;

        /**
         * @brief create instance by copy.
         * @param other other object to copy.
         */
        PingStats (const PingStats& other) = default;

        /**
         * @brief assign instance by copy.
         * @param other other object to copy.
         * @return a reference of the current object.
         */
        PingStats& operator= (const PingStats& other) = default;

        /**
         * @brief create instance by move.
         * @param other other object to move.
         */
        PingStats (PingStats&& other) = default;

        /**
         * @brief assign instance by move.
         * @param other other object to move.
         * @return a reference of the current object.
         */
        PingStats& operator= (PingStats&& other) = default;

        /**
         * @brief destroy the PingStats instance.
         */
        ~PingStats () = default;

        /**
         * @brief get the device the echo requests are sent through.
         * @return device name, empty if not bound to a device.
         */
        const std::string& device () const noexcept
        {
            return _device;
        }

        /**
         * @brief get the source address the echo requests are sent from.
         * @return source address.
         */
        const IpAddress& source () const noexcept
        {
            return _source;
        }

        /**
         * @brief get the destination as requested by the caller.
         * @return destination host name or address.
         */
        const std::string& host () const noexcept
        {
            return _host;
        }

        /**
         * @brief get the resolved destination address.
         * @return destination address.
         */
        const IpAddress& address () const noexcept
        {
            return _address;
        }

        /**
         * @brief get the number of data bytes sent in each echo request.
         * @return number of data bytes.
         */
        int size () const noexcept
        {
            return _size;
        }

        /**
         * @brief get the interval between two echo requests.
         * @return interval between two echo requests.
         */
        std::chrono::milliseconds interval () const noexcept
        {
            return _interval;
        }

        /**
         * @brief get the address that answered the last echo request.
         * @return answering address, wildcard if none answered.
         */
        const IpAddress& from () const noexcept
        {
            return _answer.from;
        }

        /**
         * @brief get the sequence number of the last echo request.
         * @return sequence number.
         */
        uint16_t sequence () const noexcept
        {
            return _sequence;
        }

        /**
         * @brief get the time to live the echo requests are sent with.
         * @return time to live.
         */
        int hop () const noexcept
        {
            return _hop;
        }

        /**
         * @brief get the path MTU discovery setting the echo requests are sent with.
         * @return path MTU discovery setting.
         */
        int df () const noexcept
        {
            return _df;
        }

        /**
         * @brief get the MTU the stack knows for the destination.
         * @return route MTU, zero if unknown.
         */
        int route () const noexcept
        {
            return _route;
        }

        /**
         * @brief get the time to live of the last answer received.
         * @return time to live.
         */
        int ttl () const noexcept
        {
            return _answer.ttl;
        }

        /**
         * @brief get the ICMP message size of the last answer received.
         * @return ICMP message size.
         */
        int packetSize () const noexcept
        {
            return _answer.size;
        }

        /**
         * @brief get the round trip time of the last echo request.
         * @return round trip time, zero if not measurable.
         */
        std::chrono::microseconds rtt () const noexcept
        {
            return _answer.rtt;
        }

        /**
         * @brief check if the last echo request expired before reaching the destination.
         * @return true if a time exceeded message was received.
         */
        bool expired () const noexcept
        {
            return _answer.expired;
        }

        /**
         * @brief get the MTU reported by the last error received.
         * @return reported MTU, zero if none was reported.
         */
        uint32_t mtu () const noexcept
        {
            return _answer.mtu;
        }

        /**
         * @brief get the human readable form of the last error.
         * @return error message, empty if the last request succeeded.
         */
        const std::string& error () const noexcept
        {
            return _answer.error;
        }

        /**
         * @brief get the error code of the last request.
         * @return error code.
         */
        const std::error_code& code () const noexcept
        {
            return _answer.code;
        }

        /**
         * @brief get the number of echo requests sent.
         * @return number of echo requests sent.
         */
        uint32_t sent () const noexcept
        {
            return _sent;
        }

        /**
         * @brief get the number of answers received.
         * @return number of answers received.
         */
        uint32_t received () const noexcept
        {
            return _received;
        }

        /**
         * @brief get the packet loss percentage.
         * @return packet loss percentage.
         */
        double loss () const noexcept
        {
            return _sent ? ((_sent - _received) * 100.0) / _sent : 0.0;
        }

        /**
         * @brief get the minimum round trip time.
         * @return minimum round trip time.
         */
        std::chrono::microseconds min () const noexcept
        {
            return std::chrono::microseconds (_samples ? _min : 0);
        }

        /**
         * @brief get the maximum round trip time.
         * @return maximum round trip time.
         */
        std::chrono::microseconds max () const noexcept
        {
            return std::chrono::microseconds (_max);
        }

        /**
         * @brief get the average round trip time.
         * @return average round trip time.
         */
        std::chrono::microseconds avg () const noexcept
        {
            return std::chrono::microseconds (static_cast<uint64_t> (_mean));
        }

        /**
         * @brief get the round trip time mean deviation.
         * @return round trip time mean deviation.
         */
        std::chrono::microseconds mdev () const noexcept;

        /**
         * @brief get the total elapsed time, request intervals included.
         * @return total elapsed time.
         */
        std::chrono::microseconds total () const noexcept
        {
            return std::chrono::microseconds (static_cast<uint64_t> (_mean * _samples)) +
                   (_interval * (_sent ? _sent - 1 : 0));
        }

        /**
         * @brief reset the round trip time aggregates and the last request outcome.
         */
        void reset () noexcept;

    private:
        /// device name.
        std::string _device;

        /// source address.
        IpAddress _source;

        /// destination as requested by the caller.
        std::string _host;

        /// resolved destination address.
        IpAddress _address;

        /// number of data bytes sent in each echo request.
        int _size = 0;

        /// interval between two echo requests.
        std::chrono::milliseconds _interval{0};

        /// sequence number of the last echo request.
        uint16_t _sequence = 0;

        /// time to live the echo requests are sent with.
        int _hop = 0;

        /// path MTU discovery setting the echo requests are sent with.
        int _df = IP_PMTUDISC_DONT;

        /// MTU the stack knows for the destination.
        int _route = 0;

        /// answer to the last echo request.
        PingAnswer _answer;

        /// number of echo requests sent.
        uint32_t _sent = 0;

        /// number of answers received.
        uint32_t _received = 0;

        /// number of round trip time samples, expired requests included.
        uint32_t _samples = 0;

        /// minimum round trip time in microseconds.
        uint64_t _min = 0;

        /// maximum round trip time in microseconds.
        uint64_t _max = 0;

        /// round trip time running mean in microseconds.
        double _mean = 0.0;

        /// round trip time sum of squared deltas.
        double _m2 = 0.0;

        /// friendship with ping.
        friend class Ping;
    };

    /**
     * @brief ICMP echo client.
     */
    class Ping : public EventHandler
    {
    public:
        /// notification callback definition.
        using PingNotify = std::function<void (const PingStats&)>;

        /// callback called when a sequence is going to start.
        PingNotify onStart;

        /// callback called when an echo request is answered.
        PingNotify onSuccess;

        /// callback called when an echo request failed.
        PingNotify onFailure;

        /// callback called when all the probes of a hop have been sent.
        PingNotify onHop;

        /// callback called when a sequence is going to stop.
        PingNotify onStop;

        /**
         * @brief create the Ping instance.
         * @param ttl maximum number of IP routers the echo requests can go through.
         * @param reactor reactor instance.
         */
        explicit Ping (int ttl = _defaultTtl, Reactor& reactor = ReactorThread::reactor ());

        /**
         * @brief create the Ping instance bound to the given device.
         * @param device device name to bind to.
         * @param ttl maximum number of IP routers the echo requests can go through.
         * @param reactor reactor instance.
         */
        explicit Ping (const std::string& device, int ttl = _defaultTtl, Reactor& reactor = ReactorThread::reactor ());

        /**
         * @brief create instance by copy.
         * @param other other object to copy.
         */
        Ping (const Ping& other) = delete;

        /**
         * @brief assign instance by copy.
         * @param other other object to copy.
         * @return a reference of the current object.
         */
        Ping& operator= (const Ping& other) = delete;

        /**
         * @brief create instance by move.
         * @param other other object to move.
         */
        Ping (Ping&& other) = delete;

        /**
         * @brief assign instance by move.
         * @param other other object to move.
         * @return a reference of the current object.
         */
        Ping& operator= (Ping&& other) = delete;

        /**
         * @brief destroy the Ping instance.
         */
        ~Ping () = default;

        /**
         * @brief send echo requests to the given destination.
         * @param source source address to bind to, wildcard to let the stack choose.
         * @param destination destination address or host name.
         * @param size number of data bytes to send in each echo request.
         * @param count number of echo requests to send.
         * @param interval interval between two echo requests.
         * @param df set the don't fragment flag.
         * @param timeout answer timeout.
         * @return the number of answered echo requests.
         */
        int ping (const IpAddress& source, const std::string& destination, int size = _defaultSize, int count = 1,
                  std::chrono::milliseconds interval = std::chrono::milliseconds (500), int df = IP_PMTUDISC_DONT,
                  std::chrono::milliseconds timeout = std::chrono::seconds (5));

        /**
         * @brief send echo requests to the given destination using the AF_INET address family.
         * @param destination destination address or host name.
         * @param size number of data bytes to send in each echo request.
         * @param count number of echo requests to send.
         * @param interval interval between two echo requests.
         * @param ttl maximum number of IP routers the echo requests can go through.
         * @param df set the don't fragment flag.
         * @param timeout answer timeout.
         * @return the number of answered echo requests.
         */
        static int ping4 (const std::string& destination, int size = _defaultSize, int count = 1,
                          std::chrono::milliseconds interval = std::chrono::milliseconds (500), int ttl = _defaultTtl,
                          int df = IP_PMTUDISC_DONT, std::chrono::milliseconds timeout = std::chrono::seconds (5));

        /**
         * @brief send echo requests to the given destination using the AF_INET6 address family.
         * @param destination destination address or host name.
         * @param size number of data bytes to send in each echo request.
         * @param count number of echo requests to send.
         * @param interval interval between two echo requests.
         * @param ttl maximum number of IP routers the echo requests can go through.
         * @param df set the don't fragment flag.
         * @param timeout answer timeout.
         * @return the number of answered echo requests.
         */
        static int ping6 (const std::string& destination, int size = _defaultSize, int count = 1,
                          std::chrono::milliseconds interval = std::chrono::milliseconds (500), int ttl = _defaultTtl,
                          int df = IP_PMTUDISC_DONT, std::chrono::milliseconds timeout = std::chrono::seconds (5));

        /**
         * @brief discover the path MTU towards the given destination.
         * @param source source address to bind to, wildcard to let the stack choose.
         * @param destination destination address or host name.
         * @param timeout answer timeout.
         * @return the path MTU on success, -1 on failure.
         */
        int pathMtu (const IpAddress& source, const std::string& destination,
                     std::chrono::milliseconds timeout = std::chrono::seconds (5));

        /**
         * @brief discover the route towards the given destination.
         * @param source source address to bind to, wildcard to let the stack choose.
         * @param destination destination address or host name.
         * @param maxHops maximum number of hops to probe.
         * @param probes number of echo requests to send per hop.
         * @param size number of data bytes to send in each echo request.
         * @param df set the don't fragment flag, discovers the MTU of each hop.
         * @param timeout answer timeout.
         * @return the number of hops to the destination, -1 if not reached.
         */
        int trace (const IpAddress& source, const std::string& destination, int maxHops = _defaultMaxHops,
                   int probes = _defaultProbes, int size = _defaultSize, int df = IP_PMTUDISC_DONT,
                   std::chrono::milliseconds timeout = std::chrono::seconds (5));

        /**
         * @brief get the human readable form of an ICMP type and code pair.
         * @param family address family.
         * @param type ICMP message type.
         * @param code ICMP message code.
         * @return human readable message.
         */
        static const char* message (int family, uint8_t type, uint8_t code) noexcept;

    protected:
        /**
         * @brief check if the last failure occurred before the request could be transmitted.
         * @param stats sequence statistics.
         * @param transmitted number of requests sent before the last attempt.
         * @return true if nothing left the host and retrying cannot succeed.
         */
        static bool isFatal (const PingStats& stats, uint32_t transmitted);

        /**
         * @brief resolve the destination held by the statistics and check it is usable.
         * @param stats sequence statistics, receives the resolved address.
         * @return true if the destination resolves to an address of the source address family.
         */
        bool isValidDestination (PingStats& stats);

        /**
         * @brief open a socket, send an echo request and wait for its answer.
         * @param stats sequence statistics, holds the request settings and receives the outcome.
         * @param timeout answer timeout.
         * @return 0 if the destination answered, -1 otherwise.
         */
        int echo (PingStats& stats, std::chrono::milliseconds timeout);

        /**
         * @brief method called when data are ready to be read.
         * @param fd file descriptor.
         */
        void onReadable (int fd) override final;

        /**
         * @brief method called when an error occurred.
         * @param fd file descriptor.
         */
        void onError (int fd) override final;

        /**
         * @brief safe way to notify sequence events.
         * @param function callback to call.
         * @param stats sequence statistics.
         */
        static void notify (const PingNotify& function, const PingStats& stats);

        /**
         * @brief set the outcome of the last request in the statistics.
         * @param stats sequence statistics.
         * @param code error code.
         * @param error human readable error message.
         * @return -1.
         */
        static int fail (PingStats& stats, const std::error_code& code, const std::string& error);

#ifdef DEBUG
        /**
         * @brief default callback called when a sequence is going to start.
         * @param stats sequence statistics.
         */
        static void defaultOnStart (const PingStats& stats);

        /**
         * @brief default callback called when an echo request is answered.
         * @param stats sequence statistics.
         */
        static void defaultOnSuccess (const PingStats& stats);

        /**
         * @brief default callback called when an echo request failed.
         * @param stats sequence statistics.
         */
        static void defaultOnFailure (const PingStats& stats);

        /**
         * @brief default callback called when a sequence is going to stop.
         * @param stats sequence statistics.
         */
        static void defaultOnStop (const PingStats& stats);
#endif

        /// default time to live.
        static constexpr int _defaultTtl = 60;

        /// default number of data bytes sent in each echo request.
        static constexpr int _defaultSize = 56;

        /// default maximum number of hops probed by trace.
        static constexpr int _defaultMaxHops = 30;

        /// default number of echo requests sent per hop by trace.
        static constexpr int _defaultProbes = 3;

        /// ICMP header size.
        static constexpr int _icmpHeaderSize = 8;

        /// receive buffer size.
        static constexpr size_t _bufferSize = 65535;

        /// maximum number of attempts made to reserve a sequence number.
        static constexpr int _maxSequenceAttempts = 8;

        /// maximum number of probes sent by pathMtu.
        static constexpr int _maxMtuProbes = 30;

        /// delay in milliseconds applied before retrying a request that could not be queued.
        static constexpr int _retryDelay = 200;

        /// maximum number of bytes dumped in debug builds.
        static constexpr int _dumpSize = 64;

        /**
         * @brief pending echo request.
         */
        struct PendingRequest
        {
            /// answer notification.
            Condition cond;

            /// time the echo request was sent at.
            std::chrono::steady_clock::time_point sent;

            /// answer received, shared with the statistics.
            PingAnswer answer;

            /// set once the request has been answered.
            bool answered = false;
        };

        /// pending echo requests indexed by sequence number.
        std::unordered_map<uint16_t, std::unique_ptr<PendingRequest>> _pending;

        /// mutex for synchronous operations.
        Mutex _syncMutex;

        /// receive buffer.
        std::unique_ptr<char[]> _buffer;

        /// device name to bind to.
        const std::string _device;

        /// echo request identifier.
        const uint16_t _identity;

        /// default time to live.
        const int _ttl;

        /// event loop reactor.
        Reactor& _reactor;
    };
}

#endif
