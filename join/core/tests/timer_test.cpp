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

using join::Timer;

/**
 * @brief Test move.
 */
TEST (Timer, move)
{
    Timer timer1, timer2;
    int count = 0;

    EXPECT_TRUE (timer1.isOneShot ());
    EXPECT_TRUE (timer2.isOneShot ());
    timer1.setInterval (1ms, [&] { ++count; });
    EXPECT_FALSE (timer1.isOneShot ());
    EXPECT_TRUE (timer2.isOneShot ());
    timer2 = std::move (timer1);
    EXPECT_TRUE (timer1.isOneShot ());
    EXPECT_FALSE (timer2.isOneShot ());
    Timer Timer3 (std::move (timer2));
    EXPECT_TRUE (timer2.isOneShot ());
    EXPECT_FALSE (Timer3.isOneShot ());
}

/**
 * @brief Test setOneShot.
 */
TEST (Timer, setOneShot)
{
    Timer timer;
    int count = 0;

    timer.setOneShot (1ms, [&] { ++count; });
    std::this_thread::sleep_for (3ms);
    EXPECT_EQ (count, 1);
    EXPECT_FALSE (timer.isActive ());
    EXPECT_TRUE (timer.isOneShot ());
    EXPECT_EQ (timer.interval (), 0ms);
}

/**
 * @brief Test setInterval.
 */
TEST (Timer, setInterval)
{
    Timer timer;
    int count = 0;

    timer.setInterval (1ms, [&] { ++count; });
    std::this_thread::sleep_for (3ms);
    EXPECT_GT (count, 1);
    EXPECT_TRUE (timer.isActive ());
    EXPECT_FALSE (timer.isOneShot ());
    EXPECT_EQ (timer.interval (), 1ms);
}

/**
 * @brief Test cancel.
 */
TEST (Timer, cancel)
{
    Timer timer;
    int count1 = 0, count2 = 0;

    timer.setInterval (1ms, [&] { count1++; });
    std::this_thread::sleep_for (3ms);
    timer.cancel ();
    count2 = count1;
    EXPECT_GT (count2, 1);
    std::this_thread::sleep_for (3ms);
    EXPECT_EQ (count1, count2);
}

/**
 * @brief Test interval.
 */
TEST (Timer, interval)
{
    Timer timer;
    int count = 0;

    ASSERT_EQ (timer.interval (), 0ms);
    timer.setInterval (1ms, [&] { ++count; });
    ASSERT_EQ (timer.interval (), 1ms);
    timer.cancel ();
    ASSERT_EQ (timer.interval (), 0ms);
}

/**
 * @brief Test isActive.
 */
TEST (Timer, isActive)
{
    Timer timer;
    int count = 0;

    ASSERT_FALSE (timer.isActive ());
    timer.setInterval (1ms, [&] { ++count; });
    ASSERT_TRUE (timer.isActive ());
    timer.cancel ();
    ASSERT_FALSE (timer.isActive ());
}

/**
 * @brief Test isOneShot.
 */
TEST (Timer, isOneShot)
{
    Timer timer;
    int count = 0;

    ASSERT_TRUE (timer.isOneShot ());
    timer.setInterval (1ms, [&] { ++count; });
    ASSERT_FALSE (timer.isOneShot ());
    timer.cancel ();
    ASSERT_TRUE (timer.isOneShot ());
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
