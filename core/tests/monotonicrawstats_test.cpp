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

// libjoin.
#include <join/statistics.hpp>

// Libraries.
#include <gtest/gtest.h>

// C++
#include <thread>

using namespace std::chrono_literals;

using join::ScopedStats;
using join::MonotonicRaw;

/**
 * @brief Test name.
 */
TEST (MonotonicRawStats, name)
{
    MonotonicRaw::Stats stats ("latency");

    EXPECT_EQ (stats.name (), "latency");
}

/**
 * @brief Test start.
 */
TEST (MonotonicRawStats, start)
{
    MonotonicRaw::Stats stats;

    auto beg = stats.start ();
    EXPECT_GE (beg.time_since_epoch ().count (), 0);
}

/**
 * @brief Test stop.
 */
TEST (MonotonicRawStats, stop)
{
    MonotonicRaw::Stats stats;

    auto beg = stats.start ();
    std::this_thread::sleep_for (5ms);
    stats.stop (beg);

    EXPECT_EQ (stats.count (), 1);
}

/**
 * @brief Test reset.
 */
TEST (MonotonicRawStats, reset)
{
    MonotonicRaw::Stats stats;

    auto beg = stats.start ();
    std::this_thread::sleep_for (5ms);
    stats.stop (beg);

    stats.reset ();

    EXPECT_EQ (stats.count (), 0);
    EXPECT_EQ (stats.last (), MonotonicRaw::Stats::Duration (0));
    EXPECT_EQ (stats.min (), MonotonicRaw::Stats::Duration (0));
    EXPECT_EQ (stats.max (), MonotonicRaw::Stats::Duration (0));
    std::chrono::duration<double, std::nano> zero (0.0);
    EXPECT_EQ (stats.mean (), zero);
    EXPECT_EQ (stats.throughput (), 0.0);
    EXPECT_EQ (stats.percentile (50.0), MonotonicRaw::Stats::Duration (0));
    EXPECT_EQ (stats.percentile (99.0), MonotonicRaw::Stats::Duration (0));
}

/**
 * @brief Test count.
 */
TEST (MonotonicRawStats, count)
{
    MonotonicRaw::Stats stats;

    auto beg = stats.start ();
    std::this_thread::sleep_for (5ms);
    stats.stop (beg);

    EXPECT_EQ (stats.count (), 1);
}

/**
 * @brief Test last.
 */
TEST (MonotonicRawStats, last)
{
    MonotonicRaw::Stats stats;
    EXPECT_EQ (stats.last ().count (), 0);

    auto beg = stats.start ();
    std::this_thread::sleep_for (5ms);
    stats.stop (beg);

    EXPECT_GT (stats.last ().count (), 0);
}

/**
 * @brief Test min.
 */
TEST (MonotonicRawStats, min)
{
    MonotonicRaw::Stats stats;
    EXPECT_EQ (stats.min (), MonotonicRaw::Stats::Duration (0));

    auto beg = stats.start ();
    std::this_thread::sleep_for (5ms);
    stats.stop (beg);

    EXPECT_GT (stats.min ().count (), 0);
}

/**
 * @brief Test max.
 */
TEST (MonotonicRawStats, max)
{
    MonotonicRaw::Stats stats;
    EXPECT_EQ (stats.max (), MonotonicRaw::Stats::Duration (0));

    auto beg = stats.start ();
    std::this_thread::sleep_for (5ms);
    stats.stop (beg);

    EXPECT_GT (stats.max ().count (), 0);
}

/**
 * @brief Test mean.
 */
TEST (MonotonicRawStats, mean)
{
    MonotonicRaw::Stats stats;
    std::chrono::duration<double, std::nano> zero (0.0);
    EXPECT_EQ (stats.mean (), zero);

    auto beg = stats.start ();
    std::this_thread::sleep_for (5ms);
    stats.stop (beg);

    EXPECT_GT (stats.mean ().count (), 0);
}

/**
 * @brief Test throughput.
 */
TEST (MonotonicRawStats, throughput)
{
    MonotonicRaw::Stats stats;
    EXPECT_EQ (stats.throughput (), 0.0);

    auto beg = stats.start ();
    std::this_thread::sleep_for (5ms);
    stats.stop (beg);

    EXPECT_GT (stats.throughput (), 0.0);
}

/**
 * @brief Test percentile.
 */
TEST (MonotonicRawStats, percentile)
{
    MonotonicRaw::Stats stats;
    EXPECT_EQ (stats.percentile (0.0), MonotonicRaw::Stats::Duration (0));
    EXPECT_EQ (stats.percentile (50.0), MonotonicRaw::Stats::Duration (0));
    EXPECT_EQ (stats.percentile (99.0), MonotonicRaw::Stats::Duration (0));

    auto beg = stats.start ();
    std::this_thread::sleep_for (5ms);
    stats.stop (beg);

    EXPECT_GT (stats.percentile (50.0).count (), 0);
    EXPECT_GT (stats.percentile (90.0).count (), 0);
    EXPECT_GT (stats.percentile (99.0).count (), 0);
    EXPECT_LE (stats.percentile (50.0), stats.percentile (90.0));
    EXPECT_LE (stats.percentile (90.0), stats.percentile (99.0));
    EXPECT_GE (stats.percentile (99.0), stats.min ());

    stats.reset ();

    for (int i = 1; i <= 10; ++i)
    {
        auto beg = stats.start ();
        std::this_thread::sleep_for (std::chrono::milliseconds (i));
        stats.stop (beg);
    }

    EXPECT_EQ (stats.count (), 10);
    EXPECT_LE (stats.percentile (50.0), stats.percentile (90.0));
    EXPECT_LE (stats.percentile (90.0), stats.percentile (99.0));
    EXPECT_GE (stats.percentile (99.0), stats.percentile (50.0));
    EXPECT_GE (stats.percentile (99.0), stats.min ());
}

/**
 * @brief Test mbind.
 */
TEST (RdtscStats, mbind)
{
    MonotonicRaw::Stats stats;

    ASSERT_EQ (stats.mbind (0), 0) << join::lastError.message ();
    ASSERT_EQ (join::mbind (nullptr, 4096, 0), -1);
}

/**
 * @brief Test mlock.
 */
TEST (RdtscStats, mlock)
{
    MonotonicRaw::Stats stats;

    ASSERT_EQ (stats.mlock (), 0) << join::lastError.message ();
}

/**
 * @brief Test ScopedStats.
 */
TEST (MonotonicRawStats, scoped)
{
    MonotonicRaw::Stats stats;

    {
        ScopedStats<MonotonicRaw::Stats> guard (stats);
        std::this_thread::sleep_for (5ms);
    }

    EXPECT_EQ (stats.count (), 1);
    EXPECT_GT (stats.last ().count (), 0);
}

/**
 * @brief Test statsHeader.
 */
TEST (MonotonicRawStats, statsHeader)
{
    std::ostringstream oss;
    oss << join::statsHeader;

    EXPECT_NE (oss.str ().find ("Metric"), std::string::npos);
    EXPECT_NE (oss.str ().find ("Count"), std::string::npos);
    EXPECT_NE (oss.str ().find ("Throughput"), std::string::npos);
    EXPECT_NE (oss.str ().find ("Min"), std::string::npos);
    EXPECT_NE (oss.str ().find ("Mean"), std::string::npos);
    EXPECT_NE (oss.str ().find ("Max"), std::string::npos);
    EXPECT_NE (oss.str ().find ("P50"), std::string::npos);
    EXPECT_NE (oss.str ().find ("P90"), std::string::npos);
    EXPECT_NE (oss.str ().find ("P99"), std::string::npos);
}

/**
 * @brief Test nsec manipulator.
 */
TEST (MonotonicRawStats, nsec)
{
    MonotonicRaw::Stats stats;
    auto beg = stats.start ();
    std::this_thread::sleep_for (5ms);
    stats.stop (beg);

    std::ostringstream oss;
    oss << join::nsec << stats;

    EXPECT_NE (oss.str ().find ("ns"), std::string::npos);
}

/**
 * @brief Test usec manipulator.
 */
TEST (MonotonicRawStats, usec)
{
    MonotonicRaw::Stats stats;
    auto beg = stats.start ();
    std::this_thread::sleep_for (5ms);
    stats.stop (beg);

    std::ostringstream oss;
    oss << join::usec << stats;

    EXPECT_NE (oss.str ().find ("us"), std::string::npos);
}

/**
 * @brief Test msec manipulator.
 */
TEST (MonotonicRawStats, msec)
{
    MonotonicRaw::Stats stats;
    auto beg = stats.start ();
    std::this_thread::sleep_for (5ms);
    stats.stop (beg);

    std::ostringstream oss;
    oss << join::msec << stats;

    EXPECT_NE (oss.str ().find ("ms"), std::string::npos);
}

/**
 * @brief Test sec manipulator.
 */
TEST (MonotonicRawStats, sec)
{
    MonotonicRaw::Stats stats;
    auto beg = stats.start ();
    std::this_thread::sleep_for (5ms);
    stats.stop (beg);

    std::ostringstream oss;
    oss << join::sec << stats;

    EXPECT_NE (oss.str ().find ("(s)"), std::string::npos);
}

/**
 * @brief Test ops manipulator.
 */
TEST (MonotonicRawStats, ops)
{
    MonotonicRaw::Stats stats;
    auto beg = stats.start ();
    std::this_thread::sleep_for (5ms);
    stats.stop (beg);

    std::ostringstream oss;
    oss << join::ops << stats;

    EXPECT_NE (oss.str ().find ("ops/s"), std::string::npos);
}

/**
 * @brief Test kops manipulator.
 */
TEST (MonotonicRawStats, kops)
{
    MonotonicRaw::Stats stats;
    auto beg = stats.start ();
    std::this_thread::sleep_for (5ms);
    stats.stop (beg);

    std::ostringstream oss;
    oss << join::kops << stats;

    EXPECT_NE (oss.str ().find ("Kops/s"), std::string::npos);
}

/**
 * @brief Test mops manipulator.
 */
TEST (MonotonicRawStats, mops)
{
    MonotonicRaw::Stats stats;
    auto beg = stats.start ();
    std::this_thread::sleep_for (5ms);
    stats.stop (beg);

    std::ostringstream oss;
    oss << join::mops << stats;

    EXPECT_NE (oss.str ().find ("Mops/s"), std::string::npos);
}

/**
 * @brief Test gops manipulator.
 */
TEST (MonotonicRawStats, gops)
{
    MonotonicRaw::Stats stats;
    auto beg = stats.start ();
    std::this_thread::sleep_for (5ms);
    stats.stop (beg);

    std::ostringstream oss;
    oss << join::gops << stats;

    EXPECT_NE (oss.str ().find ("Gops/s"), std::string::npos);
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
