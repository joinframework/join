/**
 * MIT License
 *
 * Copyright (c) 2025 Mathieu Rabine
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
#include <join/timer.hpp>

// Libraries.
#include <gtest/gtest.h>

// C++.
#include <thread>
#include <chrono>
#include <atomic>

// C.
#include <sys/resource.h>

using namespace std::chrono_literals;

using join::Monotonic;

/**
 * @brief Test setOneShot.
 */
TEST (MonotonicTimer, setOneShot)
{
    Monotonic::Timer timer;
    int count = 0;

    timer.setOneShot (50ms, [&] {
        ++count;
    });
    std::this_thread::sleep_for (250ms);
    EXPECT_EQ (count, 1);
    EXPECT_FALSE (timer.active ());
    EXPECT_TRUE (timer.oneShot ());
    EXPECT_EQ (timer.interval (), 0ms);

    timer.setOneShot (std::chrono::steady_clock::now () + 50ms, [&] {
        ++count;
    });
    std::this_thread::sleep_for (250ms);
    EXPECT_EQ (count, 2);
    EXPECT_FALSE (timer.active ());
    EXPECT_TRUE (timer.oneShot ());
    EXPECT_EQ (timer.interval (), 0ms);

    timer.setOneShot (0ms, [&] {
        ++count;
    });
    std::this_thread::sleep_for (250ms);
    EXPECT_EQ (count, 3);
    EXPECT_FALSE (timer.active ());
    EXPECT_TRUE (timer.oneShot ());
    EXPECT_EQ (timer.interval (), 0ms);
}

/**
 * @brief Test setInterval.
 */
TEST (MonotonicTimer, setInterval)
{
    Monotonic::Timer timer;
    int count = 0;

    timer.setInterval (50ms, [&] {
        ++count;
    });
    std::this_thread::sleep_for (250ms);
    EXPECT_GT (count, 1);
    EXPECT_TRUE (timer.active ());
    EXPECT_FALSE (timer.oneShot ());
    EXPECT_EQ (timer.interval (), 50ms);

    timer.cancel ();

    Monotonic::Timer once;
    int fired = 0;

    once.setInterval (0ms, [&] {
        ++fired;
    });
    std::this_thread::sleep_for (250ms);
    EXPECT_EQ (fired, 1);
    EXPECT_FALSE (once.active ());
}

/**
 * @brief Test cancel.
 */
TEST (MonotonicTimer, cancel)
{
    Monotonic::Timer timer;
    int count1 = 0, count2 = 0;

    timer.setInterval (50ms, [&] {
        count1++;
    });
    std::this_thread::sleep_for (250ms);
    timer.cancel ();
    count2 = count1;
    EXPECT_GT (count2, 1);
    std::this_thread::sleep_for (250ms);
    EXPECT_EQ (count1, count2);

    Monotonic::Timer slow;
    std::atomic<int> ran{0};

    slow.setInterval (50ms, [&ran] {
        ran++;
        std::this_thread::sleep_for (300ms);
    });
    while (ran.load () == 0)
    {
    }
    slow.cancel ();
    EXPECT_GE (ran.load (), 1);

    Monotonic::Timer suicidal;
    std::atomic<int> fired{0};

    suicidal.setInterval (50ms, [&suicidal, &fired] {
        fired++;
        suicidal.cancel ();
    });
    std::this_thread::sleep_for (250ms);
    EXPECT_GE (fired.load (), 1);
    EXPECT_FALSE (suicidal.active ());
}

/**
 * @brief Test active.
 */
TEST (MonotonicTimer, active)
{
    Monotonic::Timer timer;
    int count = 0;

    ASSERT_FALSE (timer.active ());
    timer.setInterval (50ms, [&] {
        ++count;
    });
    ASSERT_TRUE (timer.active ());
    timer.cancel ();
    ASSERT_FALSE (timer.active ());
}

/**
 * @brief Test remaining.
 */
TEST (MonotonicTimer, remaining)
{
    Monotonic::Timer timer;

    timer.setOneShot (200ms, [] {
    });
    auto t1 = timer.remaining ();
    std::this_thread::sleep_for (60ms);
    auto t2 = timer.remaining ();
    EXPECT_GT (t2.count (), 0);
    EXPECT_LT (t2.count (), t1.count ());
    std::this_thread::sleep_for (200ms);
    auto t3 = timer.remaining ();
    EXPECT_EQ (t3.count (), 0);  // remaining time is zero

    timer.setInterval (200ms, [] {
    });
    t1 = timer.remaining ();
    std::this_thread::sleep_for (60ms);
    t2 = timer.remaining ();
    EXPECT_GT (t2.count (), 0);
    EXPECT_LT (t2.count (), t1.count ());
    std::this_thread::sleep_for (200ms);
    t3 = timer.remaining ();
    EXPECT_GT (t3.count (), 0);  // next interval has started
}

/**
 * @brief Test interval.
 */
TEST (MonotonicTimer, interval)
{
    Monotonic::Timer timer;
    int count = 0;

    ASSERT_EQ (timer.interval (), 0ms);
    timer.setInterval (50ms, [&] {
        ++count;
    });
    ASSERT_EQ (timer.interval (), 50ms);
    timer.cancel ();
    ASSERT_EQ (timer.interval (), 0ms);
}

/**
 * @brief Test oneShot.
 */
TEST (MonotonicTimer, oneShot)
{
    Monotonic::Timer timer;
    int count = 0;

    ASSERT_TRUE (timer.oneShot ());
    timer.setInterval (50ms, [&] {
        ++count;
    });
    ASSERT_FALSE (timer.oneShot ());
    timer.cancel ();
    ASSERT_TRUE (timer.oneShot ());
}

/**
 * @brief Test type.
 */
TEST (MonotonicTimer, type)
{
    Monotonic::Timer timer;

    ASSERT_EQ (timer.type (), CLOCK_MONOTONIC);
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
