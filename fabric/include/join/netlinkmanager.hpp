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

#ifndef JOIN_FABRIC_NETLINKMANAGER_HPP
#define JOIN_FABRIC_NETLINKMANAGER_HPP

// libjoin.
#include <join/condition.hpp>
#include <join/socket.hpp>
#include <join/queue.hpp>

// C++.
#include <unordered_map>
#include <functional>
#include <memory>
#include <atomic>

// C.
#include <linux/rtnetlink.h>
#include <linux/netlink.h>
#include <cstdint>
#include <cstddef>

namespace join
{
    /**
     * @brief base class for netlink-based managers.
     */
    class NetlinkManager : private Netlink::Socket
    {
    public:
        /**
         * @brief create instance.
         * @param groups netlink multicast group bitmask to subscribe to.
         * @param reactor event loop reactor.
         */
        NetlinkManager (uint32_t groups, Reactor* reactor = nullptr);

        /**
         * @brief create instance by copy.
         */
        NetlinkManager (const NetlinkManager&) = delete;

        /**
         * @brief create instance by move.
         */
        NetlinkManager (NetlinkManager&&) = delete;

        /**
         * @brief assign instance by copy.
         */
        NetlinkManager& operator= (const NetlinkManager&) = delete;

        /**
         * @brief assign instance by move.
         */
        NetlinkManager& operator= (NetlinkManager&&) = delete;

        /**
         * @brief destroy instance.
         */
        virtual ~NetlinkManager ();

        /**
         * @brief get the event loop reactor.
         * @return pointer to the reactor.
         */
        Reactor* reactor () const noexcept;

    protected:
        /**
         * @brief start listening for netlink events.
         */
        void start ();

        /**
         * @brief stop listening for netlink events.
         */
        void stop ();

        /**
         * @brief send a netlink request, optionally waiting for the ack.
         * @param nlh netlink message to send.
         * @param sync if true, block until NLMSG_ERROR/NLMSG_DONE is received.
         * @param timeout maximum wait duration when sync is true (default: 5 seconds).
         * @return 0 on success, -1 on failure.
         */
        int sendRequest (struct nlmsghdr* nlh, bool sync, std::chrono::milliseconds timeout = std::chrono::seconds (5));

        /**
         * @brief wait for specific netlink response.
         * @param lock mutex previously locked by the calling thread.
         * @param seq sequence number to wait for.
         * @param timeout maximum wait duration.
         * @return 0 on success, -1 on failure.
         */
        template <class Rep, class Period>
        int waitResponse (ScopedLock<Mutex>& lock, uint32_t seq, std::chrono::duration<Rep, Period> timeout)
        {
            auto inserted = _pending.emplace (seq, std::make_unique<PendingRequest> ());
            if (!inserted.second)
            {
                lastError = make_error_code (Errc::OperationFailed);
                return -1;
            }

            if (!inserted.first->second->cond.timedWait (lock, timeout))
            {
                _pending.erase (inserted.first);
                lastError = make_error_code (Errc::TimedOut);
                return -1;
            }

            if (inserted.first->second->error != 0)
            {
                int err = inserted.first->second->error;
                _pending.erase (inserted.first);
                lastError = std::error_code (err, std::generic_category ());
                return -1;
            }

            _pending.erase (inserted.first);
            return 0;
        }

        /**
         * @brief push a job to be executed on the reactor thread.
         * @param func function to execute on the reactor thread.
         * @param args arguments to bind to the function.
         */
        template <typename Func, typename... Args>
        void pushJob (Func&& func, Args&&... args) noexcept
        {
            Job job;
            job.func = std::bind (std::forward<Func> (func), std::forward<Args> (args)...);

            if (_reactor->isReactorThread ())
            {
                job.func ();
                return;
            }

            _jobs.push (&job);

            uint64_t v                     = 1;
            [[maybe_unused]] ssize_t bytes = ::write (_wakeup, &v, sizeof (v));

            Backoff backoff;
            while (!job.done.load (std::memory_order_acquire))
            {
                backoff ();
            }
        }

        /**
         * @brief method called when data are ready to be read on handle.
         * @param fd file descriptor.
         */
        virtual void onReceive (int fd) override final;

        /**
         * @brief dispatch a single RTM_* message to the derived class.
         * @param nlh the netlink message to process.
         */
        virtual void onMessage (struct nlmsghdr* nlh) = 0;

        /**
         * @brief notify a pending synchronous request.
         * @param seq sequence number of the completed request.
         * @param error kernel error code (0 on success, positive errno).
         */
        void notifyRequest (uint32_t seq, int error = 0);

        /**
         * @brief add an attribute to a netlink message.
         * @param nlh netlink message header.
         * @param type attribute type (RTA_*, IFA_*, NDA_*, …).
         * @param data attribute payload.
         * @param alen payload length in bytes.
         */
        static void addAttributes (struct nlmsghdr* nlh, int type, const void* data, int alen);

        /**
         * @brief open a nested attribute block.
         * @param nlh netlink message header.
         * @param type container attribute type.
         * @return pointer to the container rtattr (pass to stopNestedAttributes).
         */
        static struct rtattr* startNestedAttributes (struct nlmsghdr* nlh, int type);

        /**
         * @brief close a nested attribute block.
         * @param nlh netlink message header.
         * @param nested pointer returned by startNestedAttributes.
         * @return current message length.
         */
        static int stopNestedAttributes (struct nlmsghdr* nlh, struct rtattr* nested);

        /**
         * @brief update a value in place and report whether it changed.
         * @param oldVal value to update.
         * @param newVal candidate new value.
         * @return true if the value was changed, false otherwise.
         */
        template <typename T, typename Flag>
        static Flag updateValue (T& oldVal, const T& newVal, Flag changed)
        {
            if (oldVal != newVal)
            {
                oldVal = newVal;
                return changed;
            }
            return static_cast<Flag> (0);
        }

        /// internal buffer size.
        static constexpr size_t _bufferSize = 16384;

        /// internal read buffer.
        std::unique_ptr<char[]> _buffer;

        /// sequence number.
        std::atomic<uint32_t> _seq;

        /// pending synchronous request.
        struct PendingRequest
        {
            Condition cond;
            int error = 0;
        };

        /// synchronous requests indexed by sequence number.
        std::unordered_map<uint32_t, std::unique_ptr<PendingRequest>> _pending;

        /// protection mutex.
        Mutex _syncMutex;

        /**
         * @brief job to be executed on the reactor thread.
         */
        struct Job
        {
            /// function to execute.
            std::function<void ()> func;

            /// set to true when the job has been executed.
            std::atomic<bool> done{false};
        };

        /// job queue size.
        static constexpr size_t _jobQueueSize = 256;

        /// job queue.
        LocalMem::Mpsc::Queue<Job*> _jobs;

        /// eventfd used to wake the reactor thread for pending jobs.
        int _wakeup = -1;

        /// event loop reactor.
        Reactor* _reactor;
    };
}

#endif
