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
#include <join/mutex.hpp>

// Libraries.
#include <gtest/gtest.h>

// C++.
#include <future>
#include <thread>

using join::RecursiveMutex;
using join::ScopedLock;

using namespace std::chrono_literals;

/**
 * @brief test lock.
 */
TEST (RecursiveMutex, lock)
{
    RecursiveMutex mutex;
    auto task = std::async (std::launch::async, [&mutex] () {
        mutex.lock ();
        std::this_thread::sleep_for (15ms);
        mutex.unlock ();
    });
    std::this_thread::sleep_for (5ms);
    auto beg = std::chrono::high_resolution_clock::now ();
    mutex.lock ();
    auto end = std::chrono::high_resolution_clock::now ();
    EXPECT_GT (std::chrono::duration_cast <std::chrono::milliseconds> (end - beg), 5ms);
    mutex.unlock ();
    task.wait ();
}

/**
 * @brief test tryLock.
 */
TEST (RecursiveMutex, tryLock)
{
    RecursiveMutex mutex;
    auto task = std::async (std::launch::async, [&mutex] () {
        mutex.lock ();
        std::this_thread::sleep_for (15ms);
        mutex.unlock ();
    });
    std::this_thread::sleep_for (5ms);
    EXPECT_FALSE (mutex.tryLock ());
    std::this_thread::sleep_for (15ms);
    EXPECT_TRUE (mutex.tryLock ());
    mutex.unlock ();
    task.wait ();
}

/**
 * @brief test scoped lock.
 */
TEST (RecursiveMutex, scopedLock)
{
    RecursiveMutex mutex;
    auto task = std::async (std::launch::async, [&mutex] () {
        ScopedLock <RecursiveMutex> lock (mutex);
        std::this_thread::sleep_for (15ms);
    });
    std::this_thread::sleep_for (5ms);
    auto beg = std::chrono::high_resolution_clock::now ();
    mutex.lock ();
    auto end = std::chrono::high_resolution_clock::now ();
    EXPECT_GT (std::chrono::duration_cast <std::chrono::milliseconds> (end - beg), 5ms);
    mutex.unlock ();
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
