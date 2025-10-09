/**
 * MIT License
 *
 * Copyright (c) 2021 Mathieu Rabine
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
#include <join/error.hpp>
#include <join/condition.hpp>

// Libraries.
#include <gtest/gtest.h>

// C++.
#include <future>
#include <thread>

using join::SharedMutex;
using join::ScopedLock;
using join::SharedCondition;

using namespace std::chrono_literals;

/**
 * @brief test wait.
 */
TEST (SharedCondition, wait)
{
    bool ready = false;
    SharedCondition condition;
    SharedMutex mutex;
    ScopedLock <SharedMutex> lock (mutex);
    auto task = std::async (std::launch::async, [&] () {
        std::this_thread::sleep_for (5ms);
        ScopedLock <SharedMutex> lk (mutex);
        std::this_thread::sleep_for (15ms);
        ready = true;
        condition.signal ();
    });
    auto beg = std::chrono::high_resolution_clock::now ();
    condition.wait (lock, [&ready](){return ready;});
    auto end = std::chrono::high_resolution_clock::now ();
    EXPECT_GE (std::chrono::duration_cast <std::chrono::milliseconds> (end - beg), 5ms);
    task.wait ();
}

/**
 * @brief test timedWait.
 */
TEST (SharedCondition, timedWait)
{
    bool ready = false;
    SharedCondition condition;
    SharedMutex mutex;
    ScopedLock <SharedMutex> lock (mutex);
    auto task = std::async (std::launch::async, [&] () {
        std::this_thread::sleep_for (10ms);
        ScopedLock <SharedMutex> lk (mutex);
        std::this_thread::sleep_for (10ms);
        ready = true;
        condition.broadcast ();
    });
    auto beg = std::chrono::high_resolution_clock::now ();
    EXPECT_FALSE (condition.timedWait (lock, 5ms, [&ready](){return ready;}));
    EXPECT_TRUE (condition.timedWait (lock, 50ms, [&ready](){return ready;})) << join::lastError.message ();
    auto end = std::chrono::high_resolution_clock::now ();
    EXPECT_GE (std::chrono::duration_cast <std::chrono::milliseconds> (end - beg), 5ms);
    task.wait ();
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
   testing::InitGoogleTest (&argc, argv);
   return RUN_ALL_TESTS ();
}
