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

// C.
#include <sys/resource.h>

using namespace std::chrono_literals;

using join::Monotonic;

/**
 * @brief Test move.
 */
TEST (MonotonicTimer, move)
{
    Monotonic::Timer timer1, timer2;
    int count = 0;

    EXPECT_TRUE (timer1.oneShot ());
    EXPECT_TRUE (timer2.oneShot ());
    timer1.setInterval (10ms, [&] { ++count; });
    EXPECT_FALSE (timer1.oneShot ());
    EXPECT_TRUE (timer2.oneShot ());
    timer2 = std::move (timer1);
    EXPECT_TRUE (timer1.oneShot ());
    EXPECT_FALSE (timer2.oneShot ());
    Monotonic::Timer Timer3 (std::move (timer2));
    EXPECT_TRUE (timer2.oneShot ());
    EXPECT_FALSE (Timer3.oneShot ());
}

/**
 * @brief Test setOneShot.
 */
TEST (MonotonicTimer, setOneShot)
{
    Monotonic::Timer timer;
    int count = 0;

    timer.setOneShot (10ms, [&] { ++count; });
    std::this_thread::sleep_for (35ms);
    EXPECT_EQ (count, 1);
    EXPECT_FALSE (timer.active ());
    EXPECT_TRUE (timer.oneShot ());
    EXPECT_EQ (timer.interval (), 0ms);

    timer.setOneShot (std::chrono::steady_clock::now () + 10ms, [&] { ++count; });
    std::this_thread::sleep_for (35ms);
    EXPECT_EQ (count, 2);
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

    timer.setInterval (10ms, [&] { ++count; });
    std::this_thread::sleep_for (35ms);
    EXPECT_GT (count, 1);
    EXPECT_TRUE (timer.active ());
    EXPECT_FALSE (timer.oneShot ());
    EXPECT_EQ (timer.interval (), 10ms);
}

/**
 * @brief Test cancel.
 */
TEST (MonotonicTimer, cancel)
{
    Monotonic::Timer timer;
    int count1 = 0, count2 = 0;

    timer.setInterval (10ms, [&] { count1++; });
    std::this_thread::sleep_for (35ms);
    timer.cancel ();
    count2 = count1;
    EXPECT_GT (count2, 1);
    std::this_thread::sleep_for (35ms);
    EXPECT_EQ (count1, count2);
}

/**
 * @brief Test active.
 */
TEST (MonotonicTimer, active)
{
    Monotonic::Timer timer;
    int count = 0;

    ASSERT_FALSE (timer.active ());
    timer.setInterval (10ms, [&] { ++count; });
    ASSERT_TRUE (timer.active ());
    timer.cancel ();
    ASSERT_FALSE (timer.active ());
}

/**
 * @brief Test remaining.
 */
TEST(MonotonicTimer, remaining)
{
    Monotonic::Timer timer;

    timer.setOneShot (20ms, [] {});
    auto t1 = timer.remaining ();
    std::this_thread::sleep_for (15ms);
    auto t2 = timer.remaining ();
    EXPECT_GT (t2.count (), 0);
    EXPECT_LT (t2.count (), t1.count ());
    std::this_thread::sleep_for (15ms);
    auto t3 = timer.remaining ();
    EXPECT_EQ (t3.count (), 0); // remaining time is zero

    timer.setInterval (20ms, [] {});
    t1 = timer.remaining ();
    std::this_thread::sleep_for (15ms);
    t2 = timer.remaining ();
    EXPECT_GT (t2.count (), 0);
    EXPECT_LT (t2.count (), t1.count ());
    std::this_thread::sleep_for (15ms);
    t3 = timer.remaining ();
    EXPECT_GT (t3.count (), 0); // next interval has started
}

/**
 * @brief Test interval.
 */
TEST (MonotonicTimer, interval)
{
    Monotonic::Timer timer;
    int count = 0;

    ASSERT_EQ (timer.interval (), 0ms);
    timer.setInterval (10ms, [&] { ++count; });
    ASSERT_EQ (timer.interval (), 10ms);
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
    timer.setInterval (10ms, [&] { ++count; });
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
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
