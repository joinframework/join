/**
 * MIT License
 *
 * Copyright (c) 2022 Mathieu Rabine
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

#ifndef JOIN_CORE_REACTOR_HPP
#define JOIN_CORE_REACTOR_HPP

// libjoin.
#include <join/thread.hpp>
#include <join/queue.hpp>

// C++.
#include <unordered_map>
#include <unordered_set>
#include <atomic>

// C.
#include <sys/epoll.h>

namespace join
{
    /**
     * @brief Event handler interface class.
     */
    class EventHandler
    {
    public:
        /**
         * @brief create instance.
         */
        EventHandler () = default;

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        EventHandler (const EventHandler& other) = default;

        /**
         * @brief copy assignment operator.
         * @param other other object to copy.
         * @return current object.
         */
        EventHandler& operator= (const EventHandler& other) = default;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        EventHandler (EventHandler&& other) = default;

        /**
         * @brief move assignment operator.
         * @param other other object to move.
         * @return current object.
         */
        EventHandler& operator= (EventHandler&& other) = default;

        /**
         * @brief destroy instance.
         */
        virtual ~EventHandler () = default;

    protected:
        /**
         * @brief method called when data are ready to be read on handle.
         * @param fd file descriptor.
         */
        virtual void onReceive ([[maybe_unused]] int fd)
        {
            // do nothing.
        }

        /**
         * @brief method called when handle is closed.
         * @param fd file descriptor.
         */
        virtual void onClose ([[maybe_unused]] int fd)
        {
            // do nothing.
        }

        /**
         * @brief method called when an error occurred on handle.
         * @param fd file descriptor.
         */
        virtual void onError ([[maybe_unused]] int fd)
        {
            // do nothing.
        }

        /// friendship with reactor.
        friend class Reactor;
    };

    /**
     * @brief Reactor class.
     */
    class Reactor
    {
    public:
        /**
         * @brief default constructor.
         */
        Reactor ();

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        Reactor (const Reactor& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to copy.
         * @return current object.
         */
        Reactor& operator= (const Reactor& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        Reactor (Reactor&& other) = delete;

        /**
         * @brief move assignment operator.
         * @param other other object to move.
         * @return current object.
         */
        Reactor& operator= (Reactor&& other) = delete;

        /**
         * @brief destroy instance.
         */
        ~Reactor () noexcept;

        /**
         * @brief add handler to reactor.
         * @param fd file descriptor.
         * @param handler handler pointer.
         * @param sync wait for operation completion if true.
         * @return 0 on success, -1 on failure.
         */
        int addHandler (int fd, EventHandler* handler, bool sync = true) noexcept;

        /**
         * @brief delete handler from reactor.
         * @param fd file descriptor.
         * @param sync wait for operation completion if true.
         * @return 0 on success, -1 on failure.
         */
        int delHandler (int fd, bool sync = true) noexcept;

        /**
         * @brief run the event loop (blocking).
         */
        void run ();

        /**
         * @brief stop the event loop.
         * @param sync wait for loop termination if true.
         */
        void stop (bool sync = true) noexcept;

        /**
         * @brief bind command queue memory to a NUMA node.
         * @param numa NUMA node ID.
         * @return 0 on success, -1 on failure.
         */
        int mbind (int numa) const noexcept;

        /**
         * @brief lock command queue memory in RAM.
         * @return 0 on success, -1 on failure.
         */
        int mlock () const noexcept;

    private:
        /// deleted handlers reserve size.
        static constexpr size_t _deletedReserve = 64;

        /// queue size.
        static constexpr size_t _queueSize = 1024;

        /// max events
        static constexpr size_t _maxEvents = 1024;

        /**
         * @brief Command type for reactor dispatcher.
         */
        enum class CommandType
        {
            Add,
            Del,
            Stop
        };

        /**
         * @brief Command for reactor dispatcher.
         */
        struct alignas (64) Command
        {
            CommandType type;
            int fd;
            EventHandler* handler;
            std::atomic<bool>* done;
            std::atomic<int>* errc;
        };

        /**
         * @brief register handler with epoll.
         * @param fd file descriptor.
         * @param handler handler pointer.
         * @return 0 on success, -1 on failure.
         */
        int registerHandler (int fd, EventHandler* handler) noexcept;

        /**
         * @brief unregister handler from epoll.
         * @param fd file descriptor.
         * @return 0 on success, -1 on failure.
         */
        int unregisterHandler (int fd) noexcept;

        /**
         * @brief write command to queue and wake dispatcher.
         * @param cmd command to write.
         * @return 0 on success, -1 on failure.
         */
        int writeCommand (const Command& cmd) noexcept;

        /**
         * @brief process a single command.
         * @param cmd command to process.
         */
        void processCommand (const Command& cmd) noexcept;

        /**
         * @brief read and process all pending commands from queue.
         */
        void readCommands () noexcept;

        /**
         * @brief dispatch a single event to its handler.
         * @param event epoll event.
         */
        void dispatchEvent (const epoll_event& event);

        /**
         * @brief main event loop running in dispatcher thread.
         */
        void eventLoop ();

        /**
         * @brief check if handler is active.
         * @param fd file descriptor.
         * @return true if handler is active, false otherwise.
         */
        bool isActive (int fd) const noexcept;

        /// eventfd descriptor.
        int _wakeup = -1;

        /// epoll descriptor.
        int _epoll = -1;

        /// command queue
        LocalMem::Mpsc::Queue<Command> _commands;

        /// registered handlers
        std::unordered_map<int, EventHandler*> _handlers;

        /// deleted handlers.
        std::unordered_set<int> _deleted;

        /// running flag for dispatcher thread.
        std::atomic<bool> _running{false};

        /// event loop thread ID.
        std::atomic<pthread_t> _threadId{0};
    };

    /**
     * @brief Convenience class that owns a Reactor running on a dedicated background thread.
     */
    class ReactorThread
    {
    public:
        /**
         * @brief get the global Reactor instance.
         * @return reference to the singleton Reactor.
         */
        static Reactor* reactor ();

        /**
         * @brief set reactor thread affinity.
         * @param core thread core affinity (-1 to disable pinning).
         * @return 0 on success, -1 on failure.
         */
        static int affinity (int core);

        /**
         * @brief get reactor thread affinity.
         * @return affinity or -1 if not pinned.
         */
        static int affinity ();

        /**
         * @brief set reactor thread priority.
         * @param prio thread priority (0 = SCHED_OTHER, 1-99 = SCHED_FIFO).
         * @return 0 on success, -1 on failure.
         */
        static int priority (int prio);

        /**
         * @brief get reactor thread priority.
         * @return priority.
         */
        static int priority ();

        /**
         * @brief get the handle of the reactor thread.
         * @return reactor thread handle.
         */
        static pthread_t handle ();

        /**
         * @brief bind command queue memory to a NUMA node.
         * @param numa NUMA node ID.
         * @return 0 on success, -1 on failure.
         */
        static int mbind (int numa);

        /**
         * @brief lock command queue memory in RAM.
         * @return 0 on success, -1 on failure.
         */
        static int mlock ();

    private:
        /**
         * @brief get the singleton ReactorThread instance.
         * @return reference to the singleton ReactorThread.
         */
        static ReactorThread& instance ();

        /**
         * @brief construct the ReactorThread and start the event loop thread.
         */
        ReactorThread ();

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        ReactorThread (const ReactorThread& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to copy.
         * @return current object.
         */
        ReactorThread& operator= (const ReactorThread& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        ReactorThread (ReactorThread&& other) noexcept = delete;

        /**
         * @brief move assignment operator.
         * @param other other object to move.
         * @return current object.
         */
        ReactorThread& operator= (ReactorThread&& other) noexcept = delete;

        /**
         * @brief destroy the ReactorThread and cleanly shut down the event loop.
         */
        ~ReactorThread ();

        /// Reactor instance.
        Reactor _reactor;

        /// Background thread.
        Thread _dispatcher;
    };
}

#endif
