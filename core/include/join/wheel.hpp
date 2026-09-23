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

#ifndef JOIN_CORE_WHEEL_HPP
#define JOIN_CORE_WHEEL_HPP

// libjoin.
#include <join/allocator.hpp>
#include <join/function.hpp>
#include <join/proactor.hpp>
#include <join/backoff.hpp>
#include <join/memory.hpp>
#include <join/thread.hpp>
#include <join/queue.hpp>
#include <join/error.hpp>
#include <join/timer.hpp>
#include <join/clock.hpp>
#include <join/utils.hpp>

// C++.
#include <utility>
#include <chrono>
#include <atomic>
#include <new>

// C.
#include <pthread.h>
#include <cstdint>

namespace join
{
    /**
     * @brief base hierarchical timing wheel class.
     */
    template <class ClockPolicy, class RunPolicy, size_t Capacity, uint64_t TickNs>
    class BasicWheel
    {
        static_assert (Capacity > 0, "capacity must be greater than zero");
        static_assert (TickNs > 0, "tick duration must be greater than zero");

        /// friendship with the policy running the wheel.
        friend RunPolicy;

    public:
        /**
         * @brief create instance and start the run policy.
         * @param args arguments forwarded to the run policy.
         * @throw std::system_error if the arena, the command queue or the run policy cannot be created.
         */
        template <typename... Args>
        explicit BasicWheel (Args&&... args)
        : _commands (_queueSize)
        , _generationsMem (Capacity * sizeof (std::atomic_uint32_t))
        , _generations (static_cast<std::atomic_uint32_t*> (_generationsMem.get ()))
        , _runner (std::forward<Args> (args)...)
        {
            for (uint32_t place = 0; place < Capacity; ++place)
            {
                new (&_generations[place]) std::atomic_uint32_t (0);
            }

            _tick.store (toTick (ClockPolicy::now ()), std::memory_order_release);
            _runner.start (*this, resolution ());
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicWheel (const BasicWheel& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to copy.
         * @return current object.
         */
        BasicWheel& operator= (const BasicWheel& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        BasicWheel (BasicWheel&& other) = delete;

        /**
         * @brief move assignment operator.
         * @param other other object to move.
         * @return current object.
         */
        BasicWheel& operator= (BasicWheel&& other) = delete;

        /**
         * @brief destroy instance.
         */
        ~BasicWheel () noexcept
        {
            _runner.stop ();
            readCommands ();

            for (size_t level = 0; level < _levels; ++level)
            {
                for (size_t slot = 0; slot < _slots; ++slot)
                {
                    Node* node = _wheel[level][slot];

                    while (node != nullptr)
                    {
                        Node* next = node->next;
                        node->~Node ();
                        node = next;
                    }
                }
            }

            for (uint32_t place = 0; place < Capacity; ++place)
            {
                _generations[place].~atomic ();
            }
        }

        /**
         * @brief arm a one-shot timer.
         * @param duration timeout duration before the timer expires, rounded up to the next tick.
         * @param callback function to call when the timer expires, captures limited to 48 bytes.
         * @return timer handle on success, -1 on failure.
         */
        template <class Rep, class Period, typename Func>
        ssize_t setOneShot (std::chrono::duration<Rep, Period> duration, Func&& callback) noexcept
        {
            void* ptr = _arena.tryAllocate (sizeof (Node));

            if (JOIN_UNLIKELY (ptr == nullptr))
            {
                lastError = make_error_code (Errc::OutOfMemory);
                return -1;
            }

            Node* node = new (ptr) Node ();
            node->callback = std::forward<Func> (callback);
            node->deadline.store (_tick.load (std::memory_order_acquire) + ticksFor (duration),
                                  std::memory_order_relaxed);

            const uint32_t place = _arena.getIndex (node);
            const uint32_t occupant = _generations[place].load (std::memory_order_relaxed) & _occupantMask;

            if (JOIN_LIKELY (_runner.isRunnerThread ()))
            {
                armTimer (node);
                return makeId (occupant, place);
            }

            if (JOIN_UNLIKELY (writeCommand ({CommandType::Arm, place, occupant}) == -1))
            {
                // LCOV_EXCL_START
                releaseTimer (node);
                return -1;
                // LCOV_EXCL_STOP
            }

            return makeId (occupant, place);
        }

        /**
         * @brief arm a periodic timer.
         * @param duration interval duration between expirations, rounded up to the next tick.
         * @param callback function to call on each expiration, captures limited to 48 bytes.
         * @return timer handle on success, -1 on failure.
         */
        template <class Rep, class Period, typename Func>
        ssize_t setInterval (std::chrono::duration<Rep, Period> duration, Func&& callback) noexcept
        {
            void* ptr = _arena.tryAllocate (sizeof (Node));

            if (JOIN_UNLIKELY (ptr == nullptr))
            {
                lastError = make_error_code (Errc::OutOfMemory);
                return -1;
            }

            const uint64_t ticks = ticksFor (duration);

            Node* node = new (ptr) Node ();
            node->callback = std::forward<Func> (callback);
            node->interval.store (ticks, std::memory_order_relaxed);
            node->deadline.store (_tick.load (std::memory_order_acquire) + ticks, std::memory_order_relaxed);

            const uint32_t place = _arena.getIndex (node);
            const uint32_t occupant = _generations[place].load (std::memory_order_relaxed) & _occupantMask;

            if (JOIN_LIKELY (_runner.isRunnerThread ()))
            {
                armTimer (node);
                return makeId (occupant, place);
            }

            if (JOIN_UNLIKELY (writeCommand ({CommandType::Arm, place, occupant}) == -1))
            {
                // LCOV_EXCL_START
                releaseTimer (node);
                return -1;
                // LCOV_EXCL_STOP
            }

            return makeId (occupant, place);
        }

        /**
         * @brief cancel a timer.
         * @param id timer handle, -1 and Errc::NotFound if it designates an expired timer.
         * @param sync if true, on return the callback is not running and will not be called again.
         * @return 0 on success, -1 on failure.
         */
        int cancel (ssize_t id, bool sync = false) noexcept
        {
            Node* node = resolve (id);

            if (JOIN_UNLIKELY (node == nullptr))
            {
                lastError = make_error_code (Errc::NotFound);
                return -1;
            }

            if (JOIN_LIKELY (_runner.isRunnerThread ()))
            {
                disarmTimer (node);
                return 0;
            }

            if (JOIN_UNLIKELY (writeCommand ({CommandType::Disarm, placeOf (id), occupantOf (id)}) == -1))
            {
                return -1;  // LCOV_EXCL_LINE
            }

            if (JOIN_UNLIKELY (sync))
            {
                if (JOIN_UNLIKELY (_runner.flush (*this) == -1))
                {
                    return -1;  // LCOV_EXCL_LINE
                }

                Backoff backoff;
                while (resolve (id) != nullptr)
                {
                    backoff ();
                }
            }

            return 0;
        }

        /**
         * @brief check if a timer is armed, snapshot when called from another thread.
         * @param id timer handle.
         * @return true if the timer is armed.
         */
        bool active (ssize_t id) const noexcept
        {
            return resolve (id) != nullptr;
        }

        /**
         * @brief get the remaining time until expiration, snapshot when called from another thread.
         * @param id timer handle.
         * @return remaining duration, zero if the timer is not armed.
         */
        std::chrono::nanoseconds remaining (ssize_t id) const noexcept
        {
            const Node* node = resolve (id);

            if (JOIN_UNLIKELY (node == nullptr))
            {
                return std::chrono::nanoseconds::zero ();
            }

            const uint64_t tick = _tick.load (std::memory_order_acquire);
            const uint64_t deadline = node->deadline.load (std::memory_order_acquire);

            return std::chrono::nanoseconds ((deadline > tick) ? ((deadline - tick) * TickNs) : 0);
        }

        /**
         * @brief get the interval of a periodic timer, snapshot when called from another thread.
         * @param id timer handle.
         * @return interval duration, zero if one-shot or not armed.
         */
        std::chrono::nanoseconds interval (ssize_t id) const noexcept
        {
            const Node* node = resolve (id);

            if (JOIN_UNLIKELY (node == nullptr))
            {
                return std::chrono::nanoseconds::zero ();
            }

            return std::chrono::nanoseconds (node->interval.load (std::memory_order_relaxed) * TickNs);
        }

        /**
         * @brief check if a timer is a one-shot timer, snapshot when called from another thread.
         * @param id timer handle.
         * @return true if the timer is a one-shot timer.
         */
        bool oneShot (ssize_t id) const noexcept
        {
            const Node* node = resolve (id);

            return (node != nullptr) && (node->interval.load (std::memory_order_relaxed) == 0);
        }

#ifdef JOIN_HAS_NUMA
        /**
         * @brief bind wheel memory to a NUMA node.
         * @param numa NUMA node ID.
         * @return 0 on success, -1 on failure.
         */
        int mbind (int numa) const noexcept
        {
            if ((_arena.mbind (numa) == -1) || (_commands.mbind (numa) == -1))
            {
                return -1;
            }

            return _generationsMem.mbind (numa);
        }
#endif

        /**
         * @brief lock wheel memory in RAM.
         * @return 0 on success, -1 on failure.
         */
        int mlock () const noexcept
        {
            if ((_arena.mlock () == -1) || (_commands.mlock () == -1))
            {
                return -1;  // LCOV_EXCL_LINE
            }

            return _generationsMem.mlock ();
        }

        /**
         * @brief get the policy running the wheel, const so that only its accessors are reachable.
         * @return const reference to the run policy.
         */
        const RunPolicy& runner () const noexcept
        {
            return _runner;
        }

        /**
         * @brief get the scheduling granularity.
         * @return tick duration.
         */
        static constexpr std::chrono::nanoseconds resolution () noexcept
        {
            return std::chrono::nanoseconds (TickNs);
        }

        /**
         * @brief get the maximum number of concurrently armed timers.
         * @return timer capacity.
         */
        static constexpr size_t capacity () noexcept
        {
            return Capacity;
        }

    private:
        /**
         * @brief command type for the wheel dispatcher.
         */
        enum class CommandType : uint8_t
        {
            Arm,    /**< link a prepared node into the wheel. */
            Disarm, /**< unlink a node from the wheel and release it. */
        };

        /**
         * @brief node state, states are mutually exclusive.
         */
        enum class State : uint8_t
        {
            Idle,      /**< not linked into any slot. */
            Linked,    /**< linked into the slot designated by level and slot. */
            Firing,    /**< detached, its callback is being invoked. */
            Cancelled, /**< cancelled while firing, released once the callback returns. */
        };

        /**
         * @brief timer node, one arena chunk.
         */
        struct Node
        {
            /// previous node in the slot list, nullptr when first.
            Node* prev = nullptr;

            /// next node in the slot list, nullptr when last.
            Node* next = nullptr;

            /// absolute expiration tick.
            std::atomic_uint64_t deadline{0};

            /// interval in ticks, zero for a one-shot timer.
            std::atomic_uint64_t interval{0};

            /// slot the node is linked into.
            uint16_t slot = 0;

            /// wheel level the node is linked into.
            uint8_t level = 0;

            /// node state.
            State state = State::Idle;

            /// function to call on expiration.
            Function<void (), 48> callback;
        };

        static_assert (sizeof (Node) == 128, "node must fill exactly two cache lines");

        /**
         * @brief command for the wheel dispatcher.
         */
        struct Command
        {
            CommandType type;  /**< command type. */
            uint32_t place;    /**< arena index of the target node. */
            uint32_t occupant; /**< generation the handle was issued with. */
        };

        /**
         * @brief convert a time point to its tick number.
         * @param timePoint time point to convert.
         * @return tick number.
         */
        static uint64_t toTick (typename ClockPolicy::TimePoint timePoint) noexcept
        {
            return static_cast<uint64_t> (timePoint.time_since_epoch ().count ()) / TickNs;
        }

        /**
         * @brief convert a duration to a tick count, rounded up so that a timer never expires early.
         * @param duration duration to convert.
         * @return tick count, at least one.
         */
        template <class Rep, class Period>
        static uint64_t ticksFor (std::chrono::duration<Rep, Period> duration) noexcept
        {
            auto ns = std::chrono::duration_cast<std::chrono::nanoseconds> (duration);

            if (ns.count () < 1)
            {
                return 1;
            }

            return (static_cast<uint64_t> (ns.count ()) + TickNs - 1) / TickNs;
        }

        /**
         * @brief pack an arena index and a generation into a timer handle.
         * @param occupant generation of the arena chunk.
         * @param place arena index of the chunk.
         * @return timer handle, always strictly positive.
         */
        static constexpr ssize_t makeId (uint32_t occupant, uint32_t place) noexcept
        {
            return (static_cast<ssize_t> (occupant & _occupantMask) << 32) | (static_cast<ssize_t> (place) + 1);
        }

        /**
         * @brief extract the arena index from a timer handle.
         * @param id timer handle.
         * @return arena index.
         */
        static constexpr uint32_t placeOf (ssize_t id) noexcept
        {
            return static_cast<uint32_t> ((id & 0xFFFFFFFF) - 1);
        }

        /**
         * @brief extract the generation from a timer handle.
         * @param id timer handle.
         * @return generation.
         */
        static constexpr uint32_t occupantOf (ssize_t id) noexcept
        {
            return static_cast<uint32_t> (id >> 32) & _occupantMask;
        }

        /**
         * @brief resolve a timer handle to its node.
         * @param id timer handle.
         * @return node address, nullptr if the handle is stale or invalid.
         */
        Node* resolve (ssize_t id) const noexcept
        {
            if (JOIN_UNLIKELY (id <= 0))
            {
                return nullptr;
            }

            const uint32_t place = placeOf (id);

            if (JOIN_UNLIKELY (place >= Capacity))
            {
                return nullptr;
            }

            if (JOIN_UNLIKELY ((_generations[place].load (std::memory_order_acquire) & _occupantMask) !=
                               occupantOf (id)))
            {
                return nullptr;
            }

            return static_cast<Node*> (_arena.getPtr (place));
        }

        /**
         * @brief link a node into the level and slot matching its remaining time.
         * @param node node to arm.
         * @param sameTick allow a due node to land in the slot about to fire, only safe before fire().
         */
        void armTimer (Node* node, bool sameTick = false) noexcept
        {
            const uint64_t tick = _tick.load (std::memory_order_relaxed);
            const uint64_t deadline = node->deadline.load (std::memory_order_relaxed);
            const uint64_t expires = (deadline > tick) ? deadline : (sameTick ? tick : (tick + 1));
            const uint64_t delta = (expires > tick) ? (expires - tick) : 1;

            size_t level = static_cast<size_t> (63 - __builtin_clzll (delta)) / _slotBits;
            level = (level < _levels) ? level : (_levels - 1);

            const size_t slot = (expires >> (level * _slotBits)) & _slotMask;

            node->level = static_cast<uint8_t> (level);
            node->slot = static_cast<uint16_t> (slot);
            node->state = State::Linked;
            node->prev = nullptr;
            node->next = _wheel[level][slot];

            if (node->next != nullptr)
            {
                node->next->prev = node;
            }

            _wheel[level][slot] = node;
        }

        /**
         * @brief unlink a node if linked and release it, deferred while its callback is running.
         * @param node node to disarm.
         */
        void disarmTimer (Node* node) noexcept
        {
            if (JOIN_UNLIKELY (node->state == State::Firing))
            {
                node->state = State::Cancelled;
                return;
            }

            if (JOIN_UNLIKELY (node->state == State::Cancelled))
            {
                return;
            }

            if (JOIN_LIKELY (node->state == State::Linked))
            {
                if (node->prev != nullptr)
                {
                    node->prev->next = node->next;
                }
                else
                {
                    _wheel[node->level][node->slot] = node->next;
                }

                if (node->next != nullptr)
                {
                    node->next->prev = node->prev;
                }
            }

            releaseTimer (node);
        }

        /**
         * @brief invalidate the handles issued for a node and return it to the arena.
         * @param node node to release.
         */
        void releaseTimer (Node* node) noexcept
        {
            const uint32_t place = _arena.getIndex (node);

            node->~Node ();
            _generations[place].fetch_add (1, std::memory_order_release);
            _arena.deallocate (node);
        }

        /**
         * @brief write a command to the queue.
         * @param cmd command to write.
         * @return 0 on success, -1 on failure.
         */
        int writeCommand (const Command& cmd) noexcept
        {
            if (JOIN_UNLIKELY (_commands.tryPush (cmd) == -1))
            {
                // LCOV_EXCL_START
                lastError = make_error_code (Errc::TemporaryError);
                return -1;
                // LCOV_EXCL_STOP
            }

            return 0;
        }

        /**
         * @brief read and process all pending commands.
         */
        void readCommands () noexcept
        {
            Command cmd;

            while (_commands.tryPop (cmd) == 0)
            {
                processCommand (cmd);
            }
        }

        /**
         * @brief process a single command, ignored if the handle it carries is stale.
         * @param cmd command to process.
         */
        void processCommand (const Command& cmd) noexcept
        {
            if (JOIN_LIKELY ((_generations[cmd.place].load (std::memory_order_acquire) & _occupantMask) ==
                             cmd.occupant))
            {
                Node* node = static_cast<Node*> (_arena.getPtr (cmd.place));

                if (cmd.type == CommandType::Arm)
                {
                    armTimer (node);
                }
                else
                {
                    disarmTimer (node);
                }
            }
        }

        /**
         * @brief re-link every node of an upper level slot at the level matching its remaining time.
         * @param level wheel level to cascade.
         * @param slot slot to cascade.
         */
        void cascade (size_t level, size_t slot) noexcept
        {
            Node* node = _wheel[level][slot];

            while (node != nullptr)
            {
                detach (level, slot, node);
                armTimer (node, true);
                node = _wheel[level][slot];
            }
        }

        /**
         * @brief invoke the callbacks of a level zero slot, re-arming or releasing each node.
         * @param slot slot to fire.
         */
        void fire (size_t slot) noexcept
        {
            Node* node = _wheel[0][slot];

            while (node != nullptr)
            {
                detach (0, slot, node);

                node->state = State::Firing;
                node->callback ();

                const bool cancelled = (node->state == State::Cancelled);
                node->state = State::Idle;

                if (JOIN_LIKELY (!cancelled && (node->interval.load (std::memory_order_relaxed) != 0)))
                {
                    node->deadline.fetch_add (node->interval.load (std::memory_order_relaxed),
                                              std::memory_order_release);
                    armTimer (node);
                }
                else
                {
                    releaseTimer (node);
                }

                node = _wheel[0][slot];
            }
        }

        /**
         * @brief pop a node off the front of a slot list, leaving the list rooted in its slot.
         * @param level wheel level the node is linked into.
         * @param slot slot the node is linked into.
         * @param node node to detach.
         */
        void detach (size_t level, size_t slot, Node* node) noexcept
        {
            _wheel[level][slot] = node->next;

            if (node->next != nullptr)
            {
                node->next->prev = nullptr;
            }

            node->prev = nullptr;
            node->next = nullptr;
            node->state = State::Idle;
        }

        /**
         * @brief drain pending commands, then cascade and fire every tick elapsed since the last call.
         * @return number of ticks processed.
         */
        uint64_t advance () noexcept
        {
            readCommands ();

            const uint64_t target = toTick (ClockPolicy::now ());
            uint64_t tick = _tick.load (std::memory_order_relaxed);
            uint64_t count = 0;

            while (tick < target)
            {
                _tick.store (++tick, std::memory_order_release);

                const size_t slot = tick & _slotMask;

                if (JOIN_UNLIKELY (slot == 0))
                {
                    for (size_t level = 1; level < _levels; ++level)
                    {
                        const size_t i = (tick >> (level * _slotBits)) & _slotMask;

                        cascade (level, i);

                        if (i != 0)
                        {
                            break;
                        }
                    }
                }

                fire (slot);
                ++count;
            }

            return count;
        }

        /// number of slots per wheel level.
        static constexpr size_t _slots = 256;

        /// number of bits addressing a slot.
        static constexpr size_t _slotBits = 8;

        /// slot index mask.
        static constexpr size_t _slotMask = _slots - 1;

        /// number of wheel levels.
        static constexpr size_t _levels = 6;

        /// generation mask, one bit is reserved so that a handle is never negative.
        static constexpr uint32_t _occupantMask = 0x7FFFFFFF;

        /// command queue size, one Arm and one Disarm may be in flight per timer.
        static constexpr size_t _queueSize = 2 * Capacity;

        /// arena backing the timer nodes.
        LocalMem::Allocator<Capacity, sizeof (Node)> _arena;

        /// command queue.
        LocalMem::Mpsc::Queue<Command> _commands;

        /// mmaped region backing the generation counters.
        LocalMem _generationsMem;

        /// pointer into _generationsMem, one counter per arena chunk.
        std::atomic_uint32_t* const _generations;

        /// slot lists, one array of slots per level.
        alignas (64) Node* _wheel[_levels][_slots] = {};

        /// current tick.
        alignas (64) std::atomic_uint64_t _tick{0};

        /// clock policy, triggers calibration for Rdtsc.
        ClockPolicy _clock;

        /// policy running the wheel, started last and stopped first.
        RunPolicy _runner;
    };

    /**
     * @brief wheel run policy spinning on a dedicated thread.
     */
    template <class ClockPolicy>
    class SpinPolicy
    {
    public:
        /**
         * @brief create instance.
         * @param core thread core affinity (-1 no pinning).
         * @param prio thread priority (0 = SCHED_OTHER, 1-99 = SCHED_FIFO).
         */
        explicit SpinPolicy (int core = -1, int prio = 0) noexcept
        : _core (core)
        , _priority (prio)
        {
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        SpinPolicy (const SpinPolicy& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to copy.
         * @return current object.
         */
        SpinPolicy& operator= (const SpinPolicy& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        SpinPolicy (SpinPolicy&& other) = delete;

        /**
         * @brief move assignment operator.
         * @param other other object to move.
         * @return current object.
         */
        SpinPolicy& operator= (SpinPolicy&& other) = delete;

        /**
         * @brief destroy instance.
         */
        ~SpinPolicy () noexcept
        {
            stop ();
        }

        /**
         * @brief start the dedicated thread spinning on the wheel.
         * @param wheel wheel to advance, shall outlive the run policy.
         * @param period ignored, the loop never sleeps.
         * @return 0 on success, -1 on failure.
         * @throw std::system_error if the thread cannot be created.
         */
        template <class Wheel>
        int start (Wheel& wheel, std::chrono::nanoseconds)
        {
            _running.store (true, std::memory_order_release);
            _dispatcher = Thread (_core, _priority, &SpinPolicy::loop<Wheel>, this, &wheel);

            Backoff backoff;
            while (_threadId.load (std::memory_order_acquire) == _invalidThreadId)
            {
                backoff ();
            }

            return 0;
        }

        /**
         * @brief stop the dedicated thread and wait for its termination.
         */
        void stop () noexcept
        {
            _running.store (false, std::memory_order_release);

            if (JOIN_UNLIKELY (isRunnerThread ()))
            {
                return;  // LCOV_EXCL_LINE
            }

            Backoff backoff;
            while (_threadId.load (std::memory_order_acquire) != _invalidThreadId)
            {
                backoff ();
            }
        }

        /**
         * @brief check if the calling thread is the thread running the wheel.
         * @return true if called from the spinning thread.
         */
        bool isRunnerThread () const noexcept
        {
            return pthread_equal (_threadId.load (std::memory_order_acquire), pthread_self ()) != 0;
        }

        /**
         * @brief no-op, the spinning loop picks commands up within one iteration.
         * @param wheel wheel to advance.
         * @return 0.
         */
        template <class Wheel>
        int flush (Wheel&) noexcept
        {
            return 0;
        }

        /**
         * @brief get spinning thread affinity.
         * @return core index, or -1 if not pinned.
         */
        int affinity () const noexcept
        {
            return _dispatcher.affinity ();
        }

        /**
         * @brief get spinning thread scheduling priority.
         * @return current priority.
         */
        int priority () const noexcept
        {
            return _dispatcher.priority ();
        }

        /**
         * @brief get the native handle of the spinning thread.
         * @return pthread_t handle.
         */
        pthread_t handle () const noexcept
        {
            return _dispatcher.handle ();
        }

    private:
        /**
         * @brief spin on the wheel until stop() is called.
         * @param wheel wheel to advance.
         */
        template <class Wheel>
        void loop (Wheel* wheel) noexcept
        {
            _threadId.store (pthread_self (), std::memory_order_release);

            while (JOIN_LIKELY (_running.load (std::memory_order_relaxed)))
            {
                wheel->advance ();
            }

            _threadId.store (_invalidThreadId, std::memory_order_release);
        }

        /// invalid thread id sentinel.
        static constexpr pthread_t _invalidThreadId = static_cast<pthread_t> (-1);

        /// spinning thread id.
        alignas (64) std::atomic<pthread_t> _threadId{_invalidThreadId};

        /// running flag.
        alignas (64) std::atomic_bool _running{false};

        /// thread core affinity.
        int _core = -1;

        /// thread priority.
        int _priority = 0;

        /// spinning thread.
        Thread _dispatcher;
    };

    /**
     * @brief wheel run policy waking the wheel from a periodic timer dispatched by a proactor.
     */
    template <class ClockPolicy>
    class WaitPolicy
    {
    public:
        /**
         * @brief create instance.
         * @param proactor completion dispatcher.
         * @throw std::system_error if the timer cannot be created or submitted.
         */
        explicit WaitPolicy (Proactor& proactor = ProactorThread::proactor ())
        : _timer (proactor)
        , _proactor (proactor)
        {
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        WaitPolicy (const WaitPolicy& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to copy.
         * @return current object.
         */
        WaitPolicy& operator= (const WaitPolicy& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        WaitPolicy (WaitPolicy&& other) = delete;

        /**
         * @brief move assignment operator.
         * @param other other object to move.
         * @return current object.
         */
        WaitPolicy& operator= (WaitPolicy&& other) = delete;

        /**
         * @brief destroy instance.
         */
        ~WaitPolicy () noexcept
        {
            stop ();
        }

        /**
         * @brief start waking the wheel once per period.
         * @param wheel wheel to advance, shall outlive the run policy.
         * @param period tick duration.
         * @return 0 on success, -1 on failure.
         * @throw std::system_error if the timer cannot be armed.
         */
        template <class Wheel>
        int start (Wheel& wheel, std::chrono::nanoseconds period)
        {
            _timer.setInterval (period, [&wheel] () {
                wheel.advance ();
            });

            return 0;
        }

        /**
         * @brief stop waking the wheel.
         */
        void stop () noexcept
        {
            _timer.cancel ();
        }

        /**
         * @brief check if the calling thread is the thread running the wheel.
         * @return true if called from the proactor thread.
         */
        bool isRunnerThread () const noexcept
        {
            return _proactor.isProactorThread ();
        }

        /**
         * @brief advance the wheel from its owner thread without waiting for the next tick.
         * @param wheel wheel to advance.
         * @return 0 on success, -1 on failure.
         */
        template <class Wheel>
        int flush (Wheel& wheel) noexcept
        {
            typename Proactor::InvokeHandler fn = [&wheel] () {
                wheel.advance ();
            };

            return _proactor.invoke (&fn);
        }

    private:
        /// periodic timer waking the wheel.
        BasicTimer<ClockPolicy> _timer;

        /// completion dispatcher.
        Proactor& _proactor;
    };
}

#endif
