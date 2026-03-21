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

#ifndef JOIN_CORE_CLOCK_HPP
#define JOIN_CORE_CLOCK_HPP

// C++.
#include <chrono>
#include <mutex>

// C.
#include <ctime>

namespace join
{
    template <class ClockPolicy>
    class BasicTimer;

    template <class ClockPolicy>
    class BasicStats;

    /**
     * @brief minimal clock type used as a type tag for time_point parameterization.
     */
    struct NanoClock
    {
        using rep                         = int64_t;
        using period                      = std::nano;
        using duration                    = std::chrono::nanoseconds;
        using time_point                  = std::chrono::time_point<NanoClock>;
        static constexpr bool is_steady   = true;
        static time_point now () noexcept = delete;
    };

    /**
     * @brief real-time clock policy.
     */
    class RealTime
    {
    public:
        using Timer = BasicTimer<RealTime>;

        /**
         * @brief default constructor.
         */
        constexpr RealTime () noexcept = default;

        /**
         * @brief get the POSIX clock identifier for this policy.
         * @return CLOCK_REALTIME.
         */
        static constexpr int type () noexcept
        {
            return CLOCK_REALTIME;
        }
    };

    /**
     * @brief monotonic clock policy (stable, adjusted, recommended default).
     */
    class Monotonic
    {
    public:
        using Duration  = std::chrono::nanoseconds;
        using TimePoint = std::chrono::time_point<NanoClock>;
        using Timer     = BasicTimer<Monotonic>;
        using Stats     = BasicStats<Monotonic>;

        /**
         * @brief default constructor.
         */
        constexpr Monotonic () noexcept = default;

        /**
         * @brief get the POSIX clock identifier for this policy.
         * @return CLOCK_MONOTONIC.
         */
        static constexpr int type () noexcept
        {
            return CLOCK_MONOTONIC;
        }

        /**
         * @brief read the current time.
         * @return current time point.
         */
        static TimePoint now () noexcept
        {
            timespec ts{};
            ::clock_gettime (CLOCK_MONOTONIC, &ts);
            return TimePoint (Duration (static_cast<int64_t> (ts.tv_sec) * 1'000'000'000LL + ts.tv_nsec));
        }
    };

    /**
     * @brief raw monotonic clock policy (raw hardware clock, may drift).
     */
    class MonotonicRaw
    {
    public:
        using Duration  = std::chrono::nanoseconds;
        using TimePoint = std::chrono::time_point<NanoClock>;
        using Stats     = BasicStats<MonotonicRaw>;

        /**
         * @brief default constructor.
         */
        constexpr MonotonicRaw () noexcept = default;

        /**
         * @brief get the POSIX clock identifier for this policy.
         * @return CLOCK_MONOTONIC_RAW.
         */
        static constexpr int type () noexcept
        {
            return CLOCK_MONOTONIC_RAW;
        }

        /**
         * @brief read the current time.
         * @return current time point.
         */
        static TimePoint now () noexcept
        {
            timespec ts{};
            ::clock_gettime (CLOCK_MONOTONIC_RAW, &ts);
            return TimePoint (Duration (static_cast<int64_t> (ts.tv_sec) * 1'000'000'000LL + ts.tv_nsec));
        }
    };

    /**
     * @brief rdtsc clock policy (requires invariant TSC and CPU pinning).
     */
    class Rdtsc
    {
    public:
        using Duration  = std::chrono::nanoseconds;
        using TimePoint = std::chrono::time_point<NanoClock>;
        using Stats     = BasicStats<Rdtsc>;

        /**
         * @brief default constructor.
         */
        Rdtsc () noexcept
        {
            calibrate ();
        }

        /**
         * @brief read the current time.
         * @return current time point.
         */
        static TimePoint now () noexcept
        {
            const uint64_t ns = static_cast<uint64_t> ((static_cast<__uint128_t> (readCycles ()) * cycleToNs ()) >> 32);
            return TimePoint (Duration (static_cast<int64_t> (ns)));
        }

    private:
        /**
         * @brief read the CPU cycle counter.
         * @return current cycle count.
         */
        static uint64_t readCycles () noexcept
        {
#if defined(__x86_64__)
            uint32_t lo, hi;
            __asm__ volatile ("rdtsc" : "=a"(lo), "=d"(hi));
            return (static_cast<uint64_t> (hi) << 32) | lo;
#elif defined(__aarch64__)
            uint64_t val;
            __asm__ volatile ("mrs %0, cntvct_el0" : "=r"(val));
            return val;
#else
#error "Rdtsc: unsupported architecture"
#endif
        }

        /**
         * @brief returns the cycle-to-nanosecond multiplier.
         * @return reference to the static multiplier.
         */
        static uint64_t& cycleToNs () noexcept
        {
            static uint64_t value = 0;
            return value;
        }

        /**
         * @brief calibrate the cycle-to-nanosecond multiplier against CLOCK_MONOTONIC.
         */
        static void calibrate () noexcept
        {
            static std::once_flag flag;

            std::call_once (flag, [] {
                timespec t0{}, t1{};
                ::clock_gettime (CLOCK_MONOTONIC, &t0);
                const uint64_t c0 = readCycles ();

                const timespec delay{0, 100'000'000};
                ::nanosleep (&delay, nullptr);

                ::clock_gettime (CLOCK_MONOTONIC, &t1);
                const uint64_t c1 = readCycles ();

                const int64_t ns      = (t1.tv_sec - t0.tv_sec) * 1'000'000'000LL + (t1.tv_nsec - t0.tv_nsec);
                const uint64_t cycles = c1 - c0;

                cycleToNs () = (static_cast<uint64_t> (ns) << 32) / cycles;
            });
        }
    };
}

#endif
