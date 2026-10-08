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
#include <join/backoff.hpp>
#include <join/memory.hpp>
#include <join/thread.hpp>
#include <join/queue.hpp>
#include <join/error.hpp>
#include <join/clock.hpp>
#include <join/utils.hpp>

// C++.
#include <functional>
#include <utility>
#include <chrono>
#include <atomic>
#include <limits>
#include <new>

// C.
#include <pthread.h>
#include <cstdint>

namespace join
{
    /**
     * @brief timing wheel.
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
         * @brief create instance.
         * @param args arguments forwarded to the run policy.
         * @throw std::system_error on failure.
         */
        template <typename... Args>
        explicit BasicWheel (Args&&... args)
        : _commands (_queueSize)
        , _generationsMem (Capacity * sizeof (std::atomic_uint32_t))
        , _generations (static_cast<std::atomic_uint32_t*> (_generationsMem.get ()))
        , _runner (std::forward<Args> (args)...)
        {
            _arena.reserveAll ();

            for (uint32_t index = 0; index < Capacity; ++index)
            {
                new (&_generations[index]) std::atomic_uint32_t (0);
                new (_arena.getPtr (index)) Node ();
            }

            _arena.releaseAll ();

            _tick = toTick (ClockPolicy::now ());
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

            for (uint32_t index = 0; index < Capacity; ++index)
            {
                static_cast<Node*> (_arena.getPtr (index))->~Node ();
                _generations[index].~atomic ();
            }
        }

        /**
         * @brief arm a one-shot timer.
         * @param duration delay, rounded up to a tick.
         * @param callback callback.
         * @param args callback arguments.
         * @return timer handle on success, -1 on failure.
         */
        template <class Rep, class Period, typename Func, typename... Args>
        ssize_t setOneShot (std::chrono::duration<Rep, Period> duration, Func&& callback, Args&&... args) noexcept
        {
            return createTimer (ticksFor (duration), 0, std::forward<Func> (callback), std::forward<Args> (args)...);
        }

        /**
         * @brief arm a periodic timer.
         * @param duration interval, rounded up to a tick.
         * @param callback callback.
         * @param args callback arguments.
         * @return timer handle on success, -1 on failure.
         */
        template <class Rep, class Period, typename Func, typename... Args>
        ssize_t setInterval (std::chrono::duration<Rep, Period> duration, Func&& callback, Args&&... args) noexcept
        {
            const uint64_t ticks = ticksFor (duration);
            return createTimer (ticks, ticks, std::forward<Func> (callback), std::forward<Args> (args)...);
        }

        /**
         * @brief cancel a timer.
         * @param id timer handle.
         * @param sync wait until the callback can no longer run.
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

            _runner.flush (*this);

            if (JOIN_UNLIKELY (sync))
            {
                Backoff backoff;
                while (resolve (id) != nullptr)
                {
                    if (JOIN_UNLIKELY (_runner.flush (*this) == -1))
                    {
                        lastError = make_error_code (Errc::OperationFailed);
                        return -1;
                    }

                    backoff ();
                }
            }

            return 0;
        }

        /**
         * @brief check if a timer is armed.
         * @param id timer handle.
         * @return true if the timer is armed.
         */
        bool active (ssize_t id) const noexcept
        {
            return resolve (id) != nullptr;
        }

        /**
         * @brief get the time left.
         * @param id timer handle.
         * @return time left, zero if not armed.
         */
        std::chrono::nanoseconds remaining (ssize_t id) const noexcept
        {
            const Node* node = resolve (id);

            if (JOIN_UNLIKELY (node == nullptr))
            {
                return std::chrono::nanoseconds::zero ();
            }

            const uint64_t tick = toTick (ClockPolicy::now ());
            const uint64_t deadline = node->deadline.load (std::memory_order_acquire);

            if (JOIN_UNLIKELY (resolve (id) == nullptr))
            {
                return std::chrono::nanoseconds::zero ();  // LCOV_EXCL_LINE
            }

            return std::chrono::nanoseconds ((deadline > tick + 1) ? ((deadline - tick - 1) * TickNs) : 0);
        }

        /**
         * @brief get the interval.
         * @param id timer handle.
         * @return interval, zero if one-shot or not armed.
         */
        std::chrono::nanoseconds interval (ssize_t id) const noexcept
        {
            const Node* node = resolve (id);

            if (JOIN_UNLIKELY (node == nullptr))
            {
                return std::chrono::nanoseconds::zero ();
            }

            const uint64_t ticks = node->interval.load (std::memory_order_acquire);

            if (JOIN_UNLIKELY (resolve (id) == nullptr))
            {
                return std::chrono::nanoseconds::zero ();  // LCOV_EXCL_LINE
            }

            return std::chrono::nanoseconds (ticks * TickNs);
        }

        /**
         * @brief check if a timer is one-shot.
         * @param id timer handle.
         * @return true if one-shot.
         */
        bool oneShot (ssize_t id) const noexcept
        {
            const Node* node = resolve (id);

            if (JOIN_UNLIKELY (node == nullptr))
            {
                return false;
            }

            const uint64_t ticks = node->interval.load (std::memory_order_acquire);

            return (resolve (id) != nullptr) && (ticks == 0);
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
         * @brief get the run policy.
         * @return run policy.
         */
        const RunPolicy& runner () const noexcept
        {
            return _runner;
        }

        /**
         * @brief get the tick duration.
         * @return tick duration.
         */
        static constexpr std::chrono::nanoseconds resolution () noexcept
        {
            return std::chrono::nanoseconds (TickNs);
        }

        /**
         * @brief get the capacity.
         * @return capacity.
         */
        static constexpr size_t capacity () noexcept
        {
            return Capacity;
        }

    private:
        /**
         * @brief command type.
         */
        enum class CommandType : uint8_t
        {
            Arm,    /**< link a node. */
            Disarm, /**< unlink and release a node. */
        };

        /**
         * @brief node state.
         */
        enum class State : uint8_t
        {
            Idle,      /**< unlinked. */
            Linked,    /**< linked. */
            Firing,    /**< firing. */
            Cancelled, /**< cancelled while firing. */
        };

        /**
         * @brief timer node.
         */
        struct Node
        {
            /// previous node.
            Node* prev = nullptr;

            /// next node.
            Node* next = nullptr;

            /// expiration tick.
            std::atomic_uint64_t deadline{0};

            /// interval in ticks, zero if one-shot.
            std::atomic_uint64_t interval{0};

            /// slot.
            uint16_t slot = 0;

            /// level.
            uint8_t level = 0;

            /// node state.
            State state = State::Idle;

            /// callback.
            Function<void (), 48> callback;
        };

        static_assert (sizeof (Node) == 128, "node must fill exactly two cache lines");

        /**
         * @brief command.
         */
        struct Command
        {
            CommandType type;  /**< command type. */
            uint32_t place;    /**< node index. */
            uint32_t occupant; /**< node generation. */
        };

        /**
         * @brief process commands and due slots.
         * @return elapsed ticks.
         */
        uint64_t advance () noexcept
        {
            readCommands ();

            if (_armed == 0)
            {
                return 0;
            }

            const uint64_t target = toTick (ClockPolicy::now ());
            const uint64_t start = _tick;

            if (JOIN_UNLIKELY (target < start))
            {
                return 0;  // LCOV_EXCL_LINE
            }

            if (target >= _due)
            {
                uint64_t tick = start;
                uint64_t next;

                while ((next = nextTick (tick)) <= target)
                {
                    tick = next;
                    _tick = tick;

                    if (JOIN_UNLIKELY ((tick & _slotMask) == 0))
                    {
                        cascade (tick);
                    }

                    fire (tick & _slotMask);
                }

                _due = next;
            }

            _tick = target;

            return target - start;
        }

        /**
         * @brief get the time until the next event.
         * @return time, nanoseconds::max () if empty.
         */
        std::chrono::nanoseconds next () const noexcept
        {
            if (_armed == 0)
            {
                return std::chrono::nanoseconds::max ();
            }

            const uint64_t tick = _tick;
            const uint64_t ticks = (_due > tick) ? (_due - tick) : 0;
            const uint64_t limit = static_cast<uint64_t> (std::numeric_limits<int64_t>::max () / 2) / TickNs;

            return std::chrono::nanoseconds (((ticks < limit) ? ticks : limit) * TickNs);
        }

        /**
         * @brief create and arm a timer.
         * @param ticks delay in ticks.
         * @param interval interval in ticks, zero if one-shot.
         * @param callback callback.
         * @param args callback arguments.
         * @return timer handle on success, -1 on failure.
         */
        template <typename Func, typename... Args>
        ssize_t createTimer (uint64_t ticks, uint64_t interval, Func&& callback, Args&&... args) noexcept
        {
            void* ptr = _arena.tryAllocate (sizeof (Node));

            if (JOIN_UNLIKELY (ptr == nullptr))
            {
                lastError = make_error_code (Errc::OutOfMemory);
                return -1;
            }

            Node* node = static_cast<Node*> (ptr);
            node->callback = std::bind (std::forward<Func> (callback), std::forward<Args> (args)...);
            node->interval.store (interval, std::memory_order_release);
            node->state = State::Idle;
            const uint64_t now = toTick (ClockPolicy::now ());
            node->deadline.store (now + ticks + 1, std::memory_order_release);

            const uint32_t place = _arena.getIndex (node);
            const uint32_t occupant = _generations[place].load (std::memory_order_relaxed) & _occupantMask;

            if (JOIN_LIKELY (_runner.isRunnerThread ()))
            {
                if (_armed++ == 0)
                {
                    _tick = now;
                    _due = std::numeric_limits<uint64_t>::max ();
                }

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

            _runner.flush (*this);

            return makeId (occupant, place);
        }

        /**
         * @brief link a node.
         * @param node node to arm.
         * @param sameTick allow the current slot.
         */
        void armTimer (Node* node, bool sameTick = false) noexcept
        {
            const uint64_t tick = _tick;
            const uint64_t deadline = node->deadline.load (std::memory_order_relaxed);
            const uint64_t horizon = tick + ((uint64_t (1) << (_levels * _slotBits)) - 1);
            const uint64_t due = (deadline > tick) ? deadline : (sameTick ? tick : (tick + 1));
            const uint64_t expires = (due < horizon) ? due : horizon;
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
            _occupied[level][slot >> 6] |= uint64_t (1) << (slot & 63);

            const uint64_t event = (expires >> (level * _slotBits)) << (level * _slotBits);

            if (event < _due)
            {
                _due = event;
            }
        }

        /**
         * @brief unlink and release a node.
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
                unlink (node);
                --_armed;
            }

            releaseTimer (node);
        }

        /**
         * @brief free a node.
         * @param node node to release.
         */
        void releaseTimer (Node* node) noexcept
        {
            const uint32_t place = _arena.getIndex (node);

            node->callback.reset ();
            _generations[place].fetch_add (1, std::memory_order_release);
            _arena.deallocate (node);
        }

        /**
         * @brief find the next tick to process.
         * @param tick tick to search from.
         * @return tick, UINT64_MAX if empty.
         */
        uint64_t nextTick (uint64_t tick) const noexcept
        {
            uint64_t next = std::numeric_limits<uint64_t>::max ();

            for (size_t level = 0; level < _levels; ++level)
            {
                const size_t shift = level * _slotBits;
                const uint64_t base = (tick >> shift) + 1;
                const size_t start = base & _slotMask;
                const size_t word = start >> 6;
                uint64_t bits = _occupied[level][word] & (~uint64_t (0) << (start & 63));

                for (size_t i = 0; i <= _slots / 64; ++i)
                {
                    if (bits != 0)
                    {
                        const size_t slot = ((word + i) % (_slots / 64)) * 64 + __builtin_ctzll (bits);
                        const uint64_t candidate = (base + ((slot - start) & _slotMask)) << shift;
                        next = (candidate < next) ? candidate : next;
                        break;
                    }

                    bits = _occupied[level][(word + i + 1) % (_slots / 64)];
                }

                if (next < (((tick >> (shift + _slotBits)) + 1) << (shift + _slotBits)))
                {
                    break;
                }
            }

            return next;
        }

        /**
         * @brief cascade upper levels.
         * @param tick tick.
         */
        void cascade (uint64_t tick) noexcept
        {
            for (size_t level = 1; level < _levels; ++level)
            {
                const size_t slot = (tick >> (level * _slotBits)) & _slotMask;
                Node* node;

                while ((node = _wheel[level][slot]) != nullptr)
                {
                    unlink (node);
                    armTimer (node, true);
                }

                if (slot != 0)
                {
                    break;
                }
            }
        }

        /**
         * @brief fire a level zero slot.
         * @param slot slot to fire.
         */
        void fire (size_t slot) noexcept
        {
            Node* node = _wheel[0][slot];

            while (node != nullptr)
            {
                unlink (node);

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
                    --_armed;
                }

                node = _wheel[0][slot];
            }
        }

        /**
         * @brief unlink a node.
         * @param node node to unlink.
         */
        void unlink (Node* node) noexcept
        {
            if (node->prev != nullptr)
            {
                node->prev->next = node->next;
            }
            else
            {
                _wheel[node->level][node->slot] = node->next;

                if (node->next == nullptr)
                {
                    _occupied[node->level][node->slot >> 6] &= ~(uint64_t (1) << (node->slot & 63));
                }
            }

            if (node->next != nullptr)
            {
                node->next->prev = node->prev;
            }

            node->prev = nullptr;
            node->next = nullptr;
            node->state = State::Idle;
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
         * @brief process pending commands.
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
         * @brief process a command.
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
                    if (_armed++ == 0)
                    {
                        _tick = toTick (ClockPolicy::now ());
                        _due = std::numeric_limits<uint64_t>::max ();
                    }

                    armTimer (node);
                }
                else
                {
                    disarmTimer (node);
                }
            }
        }

        /**
         * @brief resolve a timer handle to its node.
         * @param id timer handle.
         * @return node, nullptr if invalid.
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
         * @brief build a handle.
         * @param occupant node generation.
         * @param place node index.
         * @return handle.
         */
        static constexpr ssize_t makeId (uint32_t occupant, uint32_t place) noexcept
        {
            return (static_cast<ssize_t> (occupant & _occupantMask) << 32) | (static_cast<ssize_t> (place) + 1);
        }

        /**
         * @brief get the node index.
         * @param id timer handle.
         * @return arena index.
         */
        static constexpr uint32_t placeOf (ssize_t id) noexcept
        {
            return static_cast<uint32_t> ((id & 0xFFFFFFFF) - 1);
        }

        /**
         * @brief get the node generation.
         * @param id timer handle.
         * @return generation.
         */
        static constexpr uint32_t occupantOf (ssize_t id) noexcept
        {
            return static_cast<uint32_t> (id >> 32) & _occupantMask;
        }

        /**
         * @brief convert a time point to a tick.
         * @param timePoint time point to convert.
         * @return tick.
         */
        static uint64_t toTick (typename ClockPolicy::TimePoint timePoint) noexcept
        {
            return static_cast<uint64_t> (timePoint.time_since_epoch ().count ()) / TickNs;
        }

        /**
         * @brief convert a duration to ticks, rounded up.
         * @param duration duration to convert.
         * @return ticks, at least one.
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

        /// number of slots per wheel level.
        static constexpr size_t _slots = 256;

        /// number of bits addressing a slot.
        static constexpr size_t _slotBits = 8;

        /// slot index mask.
        static constexpr size_t _slotMask = _slots - 1;

        /// number of wheel levels.
        static constexpr size_t _levels = 6;

        /// generation mask.
        static constexpr uint32_t _occupantMask = 0x7FFFFFFF;

        /// command queue size.
        static constexpr size_t _queueSize = 2 * Capacity;

        /// node arena.
        LocalMem::Allocator<Capacity, sizeof (Node)> _arena;

        /// command queue.
        LocalMem::Mpsc::Queue<Command> _commands;

        /// generation storage.
        LocalMem _generationsMem;

        /// generations.
        std::atomic_uint32_t* const _generations;

        /// armed timers.
        size_t _armed = 0;

        /// occupied slots.
        uint64_t _occupied[_levels][_slots / 64] = {};

        /// next tick to process.
        uint64_t _due = std::numeric_limits<uint64_t>::max ();

        /// slots.
        alignas (64) Node* _wheel[_levels][_slots] = {};

        /// current tick.
        alignas (64) uint64_t _tick = 0;

        /// clock policy, calibrates Rdtsc.
        ClockPolicy _clock;

        /// run policy.
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
         * @brief start spinning.
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
         * @brief stop spinning.
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
         * @brief check if called from the spinning thread.
         * @return true if so.
         */
        bool isRunnerThread () const noexcept
        {
            return pthread_equal (_threadId.load (std::memory_order_acquire), pthread_self ()) != 0;
        }

        /**
         * @brief no-op.
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
         * @brief spin until stopped.
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
}

#endif
