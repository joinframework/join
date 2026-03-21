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

#ifndef JOIN_CORE_STATISTICS_HPP
#define JOIN_CORE_STATISTICS_HPP

// libjoin.
#include <join/clock.hpp>

// C++.
#include <iomanip>
#include <ostream>
#include <sstream>
#include <limits>
#include <atomic>

// C.
#include <cstdint>

namespace join
{
    /**
     * @brief lock-free, multi-producer-safe performance statistics collector.
     */
    template <class ClockPolicy>
    class BasicStats
    {
    public:
        using Duration  = typename ClockPolicy::Duration;
        using TimePoint = typename ClockPolicy::TimePoint;

        /**
         * @brief default constructor.
         * @param name metric name.
         */
        BasicStats (const std::string& name = {})
        : _name (name)
        {
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicStats (const BasicStats& other) = delete;

        /**
         * @brief copy assignment.
         * @param other other object to copy.
         * @return a reference to the current object.
         */
        BasicStats& operator= (const BasicStats& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        BasicStats (BasicStats&& other) = delete;

        /**
         * @brief move assignment.
         * @param other other object to move.
         * @return a reference to the current object.
         */
        BasicStats& operator= (BasicStats&& other) = delete;

        /**
         * @brief destructor.
         */
        ~BasicStats () noexcept = default;

        /**
         * @brief get metric name.
         * @return metric name.
         */
        const std::string& name () const noexcept
        {
            return _name;
        }

        /**
         * @brief mark the beginning of a measured interval.
         * @return time point to be passed to the matching stop() call.
         */
        TimePoint start () const noexcept
        {
            return _clock.now ();
        }

        /**
         * @brief mark the end of a measured interval and update all aggregates.
         * @param start time point returned by the matching start() call.
         */
        void stop (TimePoint start) noexcept
        {
            record (std::chrono::duration_cast<Duration> (_clock.now () - start));
        }

        /**
         * @brief reset all accumulators to their initial state.
         */
        void reset () noexcept
        {
            _count.store (0, std::memory_order_relaxed);
            _last.store (0, std::memory_order_relaxed);
            _min.store (std::numeric_limits<uint64_t>::max (), std::memory_order_relaxed);
            _max.store (0, std::memory_order_relaxed);
            _sum.store (0, std::memory_order_relaxed);
        }

        /**
         * @brief total number of completed intervals.
         * @return sample count.
         */
        uint64_t count () const noexcept
        {
            return _count.load (std::memory_order_acquire);
        }

        /**
         * @brief duration of the most recently completed interval.
         * @return last measured duration.
         */
        Duration last () const noexcept
        {
            return Duration (_last.load (std::memory_order_relaxed));
        }

        /**
         * @brief minimum duration observed across all completed intervals.
         * @return minimum measured duration, or zero if no interval has been recorded.
         */
        Duration min () const noexcept
        {
            if (_count.load (std::memory_order_acquire) == 0)
            {
                return Duration (0);
            }
            return Duration (_min.load (std::memory_order_relaxed));
        }

        /**
         * @brief maximum duration observed across all completed intervals.
         * @return maximum measured duration, or zero if no interval has been recorded.
         */
        Duration max () const noexcept
        {
            if (_count.load (std::memory_order_acquire) == 0)
            {
                return Duration (0);
            }
            return Duration (_max.load (std::memory_order_relaxed));
        }

        /**
         * @brief arithmetic mean of all completed intervals.
         * @return mean measured duration, or zero if no interval has been recorded.
         */
        std::chrono::duration<double, std::nano> mean () const noexcept
        {
            const auto count = _count.load (std::memory_order_acquire);
            if (count == 0)
            {
                return std::chrono::duration<double, std::nano> (0.0);
            }
            return std::chrono::duration<double, std::nano> (
                static_cast<double> (_sum.load (std::memory_order_relaxed)) / static_cast<double> (count));
        }

        /**
         * @brief operations per second.
         * @return throughput in ops/s.
         */
        double throughput () const noexcept
        {
            const auto count = _count.load (std::memory_order_acquire);
            const auto sum   = _sum.load (std::memory_order_acquire);

            if (count == 0 || sum == 0)
            {
                return 0.0;
            }

            return (static_cast<double> (count) * 1e9) / static_cast<double> (sum);
        }

    private:
        /**
         * @brief atomically update all accumulators with a new sample.
         * @param elapsed elapsed duration to record.
         */
        void record (Duration elapsed) noexcept
        {
            const uint64_t ns = static_cast<uint64_t> (elapsed.count ());

            _sum.fetch_add (ns, std::memory_order_relaxed);
            _last.store (ns, std::memory_order_relaxed);

            auto prev = _min.load (std::memory_order_relaxed);
            while (ns < prev &&
                   !_min.compare_exchange_weak (prev, ns, std::memory_order_relaxed, std::memory_order_relaxed))
                ;

            prev = _max.load (std::memory_order_relaxed);
            while (ns > prev &&
                   !_max.compare_exchange_weak (prev, ns, std::memory_order_relaxed, std::memory_order_relaxed))
                ;

            _count.fetch_add (1, std::memory_order_release);
        }

        /// number of completed intervals.
        alignas (64) std::atomic_uint64_t _count{0};

        /// duration of the most recent interval (nanoseconds).
        alignas (64) std::atomic_uint64_t _last{0};

        /// minimum duration observed (nanoseconds).
        alignas (64) std::atomic_uint64_t _min{std::numeric_limits<uint64_t>::max ()};

        /// maximum duration observed (nanoseconds).
        alignas (64) std::atomic_uint64_t _max{0};

        /// running sum of all durations (nanoseconds).
        alignas (64) std::atomic_uint64_t _sum{0};

        /// metric name.
        std::string _name;

        /// clock policy.
        ClockPolicy _clock;
    };

    namespace details
    {
        /// width of the metric name column.
        constexpr int colMetric = 24;

        /// width of the sample count column.
        constexpr int colCount = 14;

        /// width of the throughput column.
        constexpr int colThroughput = 20;

        /// width of the latency columns.
        constexpr int colLatency = 16;

        /// total table width.
        constexpr int colTotal = colMetric + colCount + colThroughput + colLatency * 3;

        /**
         * @brief returns the xalloc index used to store the latency scale on a stream.
         * @return stream storage index.
         */
        inline int latencyScaleIndex () noexcept
        {
            static int idx = std::ios_base::xalloc ();
            return idx;
        }

        /**
         * @brief returns the xalloc index used to store the throughput scale on a stream.
         * @return stream storage index.
         */
        inline int throughputScaleIndex () noexcept
        {
            static int idx = std::ios_base::xalloc ();
            return idx;
        }
    }

    /**
     * @brief print the statistics table header to a stream.
     * @param out output stream.
     * @return reference to out.
     */
    inline std::ostream& statsHeader (std::ostream& out)
    {
        out << std::left << std::setw (details::colMetric) << "Metric" << std::right << std::setw (details::colCount)
            << "Count" << std::setw (details::colThroughput) << "Throughput" << std::setw (details::colLatency) << "Min"
            << std::setw (details::colLatency) << "Mean" << std::setw (details::colLatency) << "Max" << "\n"
            << std::string (details::colTotal, '-');

        return out;
    }

    /**
     * @brief set latency display unit to nanoseconds.
     * @param out output stream.
     * @return reference to out.
     */
    inline std::ostream& nsec (std::ostream& out)
    {
        out.iword (details::latencyScaleIndex ()) = 1;
        return out;
    }

    /**
     * @brief set latency display unit to microseconds.
     * @param out output stream.
     * @return reference to out.
     */
    inline std::ostream& usec (std::ostream& out)
    {
        out.iword (details::latencyScaleIndex ()) = 1'000;
        return out;
    }

    /**
     * @brief set latency display unit to milliseconds.
     * @param out output stream.
     * @return reference to out.
     */
    inline std::ostream& msec (std::ostream& out)
    {
        out.iword (details::latencyScaleIndex ()) = 1'000'000;
        return out;
    }

    /**
     * @brief set latency display unit to seconds.
     * @param out output stream.
     * @return reference to out.
     */
    inline std::ostream& sec (std::ostream& out)
    {
        out.iword (details::latencyScaleIndex ()) = 1'000'000'000;
        return out;
    }

    /**
     * @brief set throughput display unit to ops/sec.
     * @param out output stream.
     * @return reference to out.
     */
    inline std::ostream& ops (std::ostream& out)
    {
        out.iword (details::throughputScaleIndex ()) = 1;
        return out;
    }

    /**
     * @brief set throughput display unit to Kops/sec.
     * @param out output stream.
     * @return reference to out.
     */
    inline std::ostream& kops (std::ostream& out)
    {
        out.iword (details::throughputScaleIndex ()) = 1'000;
        return out;
    }

    /**
     * @brief set throughput display unit to Mops/sec.
     * @param out output stream.
     * @return reference to out.
     */
    inline std::ostream& mops (std::ostream& out)
    {
        out.iword (details::throughputScaleIndex ()) = 1'000'000;
        return out;
    }

    /**
     * @brief set throughput display unit to Gops/sec.
     * @param out output stream.
     * @return reference to out.
     */
    inline std::ostream& gops (std::ostream& out)
    {
        out.iword (details::throughputScaleIndex ()) = 1'000'000'000;
        return out;
    }

    /**
     * @brief stream insertion operator for statistics.
     * @param out destination stream.
     * @param statistics statistics to print.
     * @return reference to out.
     */
    template <class ClockPolicy>
    inline std::ostream& operator<< (std::ostream& out, const BasicStats<ClockPolicy>& statistics)
    {
        // latency scale.
        const long lscale = [&] {
            const long s = out.iword (details::latencyScaleIndex ());
            return s == 0 ? 1L : s;
        }();

        const double dlscale = static_cast<double> (lscale);
        const char* lunit    = "ns";
        if (lscale == 1'000'000'000)
        {
            lunit = "s";
        }
        else if (lscale == 1'000'000)
        {
            lunit = "ms";
        }
        else if (lscale == 1'000)
        {
            lunit = "µs";
        }

        const long tscale = [&] {
            const long s = out.iword (details::throughputScaleIndex ());
            return s == 0 ? 1L : s;
        }();

        // throughput scale.
        const double dtscale = static_cast<double> (tscale);
        const char* tunit    = "ops/s";
        if (tscale == 1'000'000'000)
        {
            tunit = "Gops/s";
        }
        else if (tscale == 1'000'000)
        {
            tunit = "Mops/s";
        }
        else if (tscale == 1'000)
        {
            tunit = "Kops/s";
        }

        // format statistics.
        const auto count = statistics.count ();
        const auto min   = statistics.min ();
        const auto mean  = statistics.mean ();
        const auto max   = statistics.max ();
        const auto thr   = statistics.throughput ();

        std::ostringstream oss;
        oss << std::fixed << std::setprecision (out.precision ()) << thr / dtscale << " (" << tunit << ")";
        auto printLatCol = [&] (double v) {
            std::ostringstream ss;
            ss << std::fixed << std::setprecision (out.precision ()) << v << " (" << lunit << ")";
            out << std::setw (details::colLatency) << ss.str ();
        };
        out << std::left << std::setw (details::colMetric) << statistics.name () << std::right
            << std::setw (details::colCount) << count << std::setw (details::colThroughput) << oss.str ();
        printLatCol (min.count () / dlscale);
        printLatCol (mean.count () / dlscale);
        printLatCol (max.count () / dlscale);

        return out;
    }

    /**
     * @brief RAII guard that automatically calls start() on construction and stop() on destruction.
     */
    template <typename Statistics>
    class ScopedStats
    {
    public:
        using TimePoint = typename Statistics::TimePoint;

        /**
         * @brief construct the guard and immediately call start() on stats.
         * @param stats statistics collector whose interval is to be bracketed.
         */
        explicit ScopedStats (Statistics& stats) noexcept
        : _statistics (stats)
        , _start (_statistics.start ())
        {
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        ScopedStats (const ScopedStats& other) = delete;

        /**
         * @brief copy assignment.
         * @param other other object to copy.
         * @return a reference to the current object.
         */
        ScopedStats& operator= (const ScopedStats& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        ScopedStats (ScopedStats&& other) = delete;

        /**
         * @brief move assignment.
         * @param other other object to move.
         * @return a reference to the current object.
         */
        ScopedStats& operator= (ScopedStats&& other) = delete;

        /**
         * @brief destructor.
         */
        ~ScopedStats () noexcept
        {
            _statistics.stop (_start);
        }

    private:
        /// statistics collector.
        Statistics& _statistics;

        /// timestamp captured at construction.
        TimePoint _start;
    };
}

#endif
