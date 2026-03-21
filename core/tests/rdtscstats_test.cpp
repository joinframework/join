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
using join::Rdtsc;

/**
 * @brief Test start.
 */
TEST (RdtscStats, start)
{
    Rdtsc::Stats stats;

    auto beg = stats.start ();
    EXPECT_GE (beg.time_since_epoch ().count (), 0);
}

/**
 * @brief Test stop.
 */
TEST (RdtscStats, stop)
{
    Rdtsc::Stats stats;

    auto beg = stats.start ();
    std::this_thread::sleep_for (5ms);
    stats.stop (beg);

    EXPECT_EQ (stats.count (), 1);
}

/**
 * @brief Test reset.
 */
TEST (RdtscStats, reset)
{
    Rdtsc::Stats stats;

    auto beg = stats.start ();
    std::this_thread::sleep_for (5ms);
    stats.stop (beg);

    stats.reset ();

    EXPECT_EQ (stats.count (), 0);
    EXPECT_EQ (stats.last (), Rdtsc::Stats::Duration (0));
    EXPECT_EQ (stats.min (), Rdtsc::Stats::Duration (0));
    EXPECT_EQ (stats.max (), Rdtsc::Stats::Duration (0));
    std::chrono::duration<double, std::nano> zero (0.0);
    EXPECT_EQ (stats.mean (), zero);
    EXPECT_EQ (stats.throughput (), 0.0);
}

/**
 * @brief Test count.
 */
TEST (RdtscStats, count)
{
    Rdtsc::Stats stats;

    auto beg = stats.start ();
    std::this_thread::sleep_for (5ms);
    stats.stop (beg);

    EXPECT_EQ (stats.count (), 1);
}

/**
 * @brief Test last.
 */
TEST (RdtscStats, last)
{
    Rdtsc::Stats stats;
    EXPECT_EQ (stats.last ().count (), 0);

    auto beg = stats.start ();
    std::this_thread::sleep_for (5ms);
    stats.stop (beg);

    EXPECT_GT (stats.last ().count (), 0);
}

/**
 * @brief Test min.
 */
TEST (RdtscStats, min)
{
    Rdtsc::Stats stats;
    EXPECT_EQ (stats.min (), Rdtsc::Stats::Duration (0));

    auto beg = stats.start ();
    std::this_thread::sleep_for (5ms);
    stats.stop (beg);

    EXPECT_GT (stats.min ().count (), 0);
}

/**
 * @brief Test max.
 */
TEST (RdtscStats, max)
{
    Rdtsc::Stats stats;
    EXPECT_EQ (stats.max (), Rdtsc::Stats::Duration (0));

    auto beg = stats.start ();
    std::this_thread::sleep_for (5ms);
    stats.stop (beg);

    EXPECT_GT (stats.max ().count (), 0);
}

/**
 * @brief Test mean.
 */
TEST (RdtscStats, mean)
{
    Rdtsc::Stats stats;
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
TEST (RdtscStats, throughput)
{
    Rdtsc::Stats stats;
    EXPECT_EQ (stats.throughput (), 0.0);

    auto beg = stats.start ();
    std::this_thread::sleep_for (5ms);
    stats.stop (beg);

    EXPECT_GT (stats.throughput (), 0.0);
}

/**
 * @brief Test name.
 */
TEST (RdtscStats, name)
{
    Rdtsc::Stats stats ("latency");

    EXPECT_EQ (stats.name (), "latency");
}

/**
 * @brief Test ScopedStats.
 */
TEST (RdtscStats, scoped)
{
    Rdtsc::Stats stats;

    {
        ScopedStats<Rdtsc::Stats> guard (stats);
        std::this_thread::sleep_for (5ms);
    }

    EXPECT_EQ (stats.count (), 1);
    EXPECT_GT (stats.last ().count (), 0);
}

/**
 * @brief Test statsHeader.
 */
TEST (RdtscStats, statsHeader)
{
    std::ostringstream oss;
    oss << join::statsHeader;

    EXPECT_NE (oss.str ().find ("Metric"), std::string::npos);
    EXPECT_NE (oss.str ().find ("Count"), std::string::npos);
    EXPECT_NE (oss.str ().find ("Throughput"), std::string::npos);
    EXPECT_NE (oss.str ().find ("Min"), std::string::npos);
    EXPECT_NE (oss.str ().find ("Mean"), std::string::npos);
    EXPECT_NE (oss.str ().find ("Max"), std::string::npos);
}

/**
 * @brief Test nsec manipulator.
 */
TEST (RdtscStats, nsec)
{
    Rdtsc::Stats stats;
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
TEST (RdtscStats, usec)
{
    Rdtsc::Stats stats;
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
TEST (RdtscStats, msec)
{
    Rdtsc::Stats stats;
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
TEST (RdtscStats, sec)
{
    Rdtsc::Stats stats;
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
TEST (RdtscStats, ops)
{
    Rdtsc::Stats stats;
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
TEST (RdtscStats, kops)
{
    Rdtsc::Stats stats;
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
TEST (RdtscStats, mops)
{
    Rdtsc::Stats stats;
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
TEST (RdtscStats, gops)
{
    Rdtsc::Stats stats;
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
