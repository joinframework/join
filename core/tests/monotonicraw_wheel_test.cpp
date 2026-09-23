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
#include <join/wheel.hpp>

// Libraries.
#include <gtest/gtest.h>

// C++.
#include <thread>
#include <chrono>
#include <atomic>

using namespace std::chrono_literals;

using join::Errc;
using join::MonotonicRaw;

using Wheel = MonotonicRaw::Wheel<64>;

/**
 * @brief Test setOneShot.
 */
TEST (MonotonicRawWheel, setOneShot)
{
    Wheel wheel;
    std::atomic<int> count{0};

    ASSERT_GT (wheel.setOneShot (50us,
                                 [&count] () {
                                     ++count;
                                 }),
               0);

    ASSERT_GT (wheel.setOneShot (50ms,
                                 [&count] () {
                                     ++count;
                                 }),
               0);

    ASSERT_GT (wheel.setOneShot (0ms,
                                 [&count] () {
                                     ++count;
                                 }),
               0);

    std::this_thread::sleep_for (200ms);
    EXPECT_EQ (count, 3);

    ASSERT_GT (wheel.setOneShot (10ms,
                                 [&wheel, &count] () {
                                     ASSERT_GT (wheel.setOneShot (10ms,
                                                                  [&count] () {
                                                                      ++count;
                                                                  }),
                                                0);
                                 }),
               0);

    std::this_thread::sleep_for (200ms);
    EXPECT_EQ (count, 4);

    for (size_t i = 0; i < wheel.capacity (); ++i)
    {
        ASSERT_GT (wheel.setOneShot (1h,
                                     [] () {
                                     }),
                   0);
    }
    ASSERT_EQ (wheel.setOneShot (1h,
                                 [] () {
                                 }),
               -1);
    ASSERT_EQ (join::lastError, Errc::OutOfMemory);
}

/**
 * @brief Test setInterval.
 */
TEST (MonotonicRawWheel, setInterval)
{
    Wheel wheel;
    std::atomic<int> count{0};

    ssize_t id = wheel.setInterval (1ms, [&count] () {
        ++count;
    });
    ASSERT_GT (id, 0);

    std::this_thread::sleep_for (200ms);
    ASSERT_EQ (wheel.cancel (id, true), 0);

    const int fired = count;
    EXPECT_GT (fired, 50);

    std::this_thread::sleep_for (100ms);
    EXPECT_EQ (count, fired);

    std::atomic<int> chained{0};
    std::atomic<ssize_t> inner{-1};
    ASSERT_GT (wheel.setOneShot (1ms,
                                 [&wheel, &inner, &chained] () {
                                     inner = wheel.setInterval (1ms, [&chained] () {
                                         ++chained;
                                     });
                                 }),
               0);

    std::this_thread::sleep_for (200ms);
    EXPECT_GT (chained, 0);
    ASSERT_GT (inner.load (), 0);
    ASSERT_EQ (wheel.cancel (inner, true), 0);

    for (size_t i = 0; i < wheel.capacity (); ++i)
    {
        ASSERT_GT (wheel.setInterval (1h,
                                      [] () {
                                      }),
                   0);
    }
    ASSERT_EQ (wheel.setInterval (1h,
                                  [] () {
                                  }),
               -1);
    ASSERT_EQ (join::lastError, Errc::OutOfMemory);
}

/**
 * @brief Test cancel.
 */
TEST (MonotonicRawWheel, cancel)
{
    Wheel wheel;
    std::atomic<int> count{0};

    ssize_t id = wheel.setOneShot (100ms, [&count] () {
        ++count;
    });
    ASSERT_GT (id, 0);
    ASSERT_EQ (wheel.cancel (id), 0);

    id = wheel.setOneShot (100ms, [&count] () {
        ++count;
    });
    ASSERT_GT (id, 0);
    ASSERT_EQ (wheel.cancel (id, true), 0);

    std::this_thread::sleep_for (200ms);
    EXPECT_EQ (count, 0);

    ASSERT_EQ (wheel.cancel (id), -1);
    ASSERT_EQ (join::lastError, Errc::NotFound);
    ASSERT_EQ (wheel.cancel (0), -1);
    ASSERT_EQ (join::lastError, Errc::NotFound);
    ASSERT_EQ (wheel.cancel (-1), -1);
    ASSERT_EQ (join::lastError, Errc::NotFound);

    std::atomic<int> sameSlot{0};
    ssize_t first = wheel.setOneShot (10ms, [&sameSlot] () {
        ++sameSlot;
    });
    ssize_t middle = wheel.setOneShot (10ms, [&sameSlot] () {
        ++sameSlot;
    });
    ssize_t last = wheel.setOneShot (10ms, [&sameSlot] () {
        ++sameSlot;
    });
    ASSERT_GT (first, 0);
    ASSERT_GT (middle, 0);
    ASSERT_GT (last, 0);
    ASSERT_EQ (wheel.cancel (middle, true), 0);

    std::this_thread::sleep_for (200ms);
    EXPECT_EQ (sameSlot, 2);

    MonotonicRaw::Wheel<64, 10'000'000> coarse;
    std::atomic<int> siblings{0};

    for (int i = 0; i < 4; ++i)
    {
        ASSERT_GT (coarse.setOneShot (50ms,
                                      [&siblings] () {
                                          ++siblings;
                                      }),
                   0);
    }

    std::atomic<ssize_t> twice{-1};
    twice = coarse.setOneShot (50ms, [&coarse, &twice] () {
        const ssize_t id = twice.load ();

        if (id > 0)
        {
            coarse.cancel (id);
            coarse.cancel (id);
        }
    });
    ASSERT_GT (twice.load (), 0);

    std::this_thread::sleep_for (300ms);
    EXPECT_EQ (siblings, 4);

    std::atomic<ssize_t> self{-1};
    self = wheel.setInterval (1ms, [&wheel, &self, &count] () {
        const ssize_t id = self.load ();

        if (id > 0)
        {
            ++count;
            wheel.cancel (id);
        }
    });
    ASSERT_GT (self.load (), 0);

    std::this_thread::sleep_for (100ms);
    EXPECT_EQ (count, 1);
    EXPECT_FALSE (wheel.active (self));
}

/**
 * @brief Test active.
 */
TEST (MonotonicRawWheel, active)
{
    Wheel wheel;

    ssize_t id = wheel.setOneShot (1h, [] () {
    });
    ASSERT_GT (id, 0);
    EXPECT_TRUE (wheel.active (id));

    ASSERT_EQ (wheel.cancel (id, true), 0);
    EXPECT_FALSE (wheel.active (id));

    EXPECT_FALSE (wheel.active (0));
    EXPECT_FALSE (wheel.active (-1));
    EXPECT_FALSE (wheel.active (static_cast<ssize_t> (wheel.capacity ()) + 1));
}

/**
 * @brief Test remaining.
 */
TEST (MonotonicRawWheel, remaining)
{
    Wheel wheel;

    ssize_t id = wheel.setOneShot (1h, [] () {
    });
    ASSERT_GT (id, 0);
    EXPECT_GT (wheel.remaining (id), 0ms);
    EXPECT_LE (wheel.remaining (id), 1h);

    ASSERT_EQ (wheel.cancel (id, true), 0);
    EXPECT_EQ (wheel.remaining (id), 0ms);
    EXPECT_EQ (wheel.remaining (0), 0ms);
}

/**
 * @brief Test interval.
 */
TEST (MonotonicRawWheel, interval)
{
    Wheel wheel;

    ssize_t periodic = wheel.setInterval (20ms, [] () {
    });
    ASSERT_GT (periodic, 0);
    EXPECT_EQ (wheel.interval (periodic), 20ms);

    ssize_t once = wheel.setOneShot (1h, [] () {
    });
    ASSERT_GT (once, 0);
    EXPECT_EQ (wheel.interval (once), 0ms);

    ssize_t rounded = wheel.setInterval (1500ns, [] () {
    });
    ASSERT_GT (rounded, 0);
    EXPECT_EQ (wheel.interval (rounded), 2us);
    ASSERT_EQ (wheel.cancel (rounded, true), 0);

    ASSERT_EQ (wheel.cancel (periodic, true), 0);
    EXPECT_EQ (wheel.interval (periodic), 0ms);
    EXPECT_EQ (wheel.interval (0), 0ms);
}

/**
 * @brief Test oneShot.
 */
TEST (MonotonicRawWheel, oneShot)
{
    Wheel wheel;

    ssize_t once = wheel.setOneShot (1h, [] () {
    });
    ASSERT_GT (once, 0);
    EXPECT_TRUE (wheel.oneShot (once));

    ssize_t periodic = wheel.setInterval (20ms, [] () {
    });
    ASSERT_GT (periodic, 0);
    EXPECT_FALSE (wheel.oneShot (periodic));

    ASSERT_EQ (wheel.cancel (once, true), 0);
    EXPECT_FALSE (wheel.oneShot (once));
    EXPECT_FALSE (wheel.oneShot (0));
}

/**
 * @brief Test runner.
 */
TEST (MonotonicRawWheel, runner)
{
    Wheel unpinned;

    EXPECT_FALSE (unpinned.runner ().isRunnerThread ());
    EXPECT_EQ (unpinned.runner ().affinity (), -1);
    EXPECT_EQ (unpinned.runner ().priority (), 0);
    EXPECT_NE (unpinned.runner ().handle (), pthread_t ());

    Wheel pinned (0, 0);

    EXPECT_EQ (pinned.runner ().affinity (), 0);
    EXPECT_EQ (pinned.runner ().priority (), 0);

    std::atomic<bool> onRunner{false}, done{false};
    ASSERT_GT (pinned.setOneShot (1ms,
                                  [&pinned, &onRunner, &done] () {
                                      onRunner = pinned.runner ().isRunnerThread ();
                                      done = true;
                                  }),
               0);

    std::this_thread::sleep_for (100ms);
    ASSERT_TRUE (done);
    EXPECT_TRUE (onRunner);
}

/**
 * @brief Test mlock.
 */
TEST (MonotonicRawWheel, mlock)
{
    Wheel wheel;

    ASSERT_EQ (wheel.mlock (), 0) << join::lastError.message ();
}

#ifdef JOIN_HAS_NUMA
/**
 * @brief Test mbind.
 */
TEST (MonotonicRawWheel, mbind)
{
    Wheel wheel;

    ASSERT_EQ (wheel.mbind (0), 0) << join::lastError.message ();
    ASSERT_EQ (wheel.mbind (999), -1);
}
#endif

/**
 * @brief Test resolution.
 */
TEST (MonotonicRawWheel, resolution)
{
    ASSERT_EQ (Wheel::resolution (), 1us);
    ASSERT_EQ ((MonotonicRaw::Wheel<64, 100>::resolution ()), 100ns);
}

/**
 * @brief Test capacity.
 */
TEST (MonotonicRawWheel, capacity)
{
    ASSERT_EQ (Wheel::capacity (), 64u);
    ASSERT_EQ ((MonotonicRaw::Wheel<128>::capacity ()), 128u);
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
