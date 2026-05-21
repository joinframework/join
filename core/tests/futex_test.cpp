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
#include <join/futex.hpp>

// Libraries.
#include <gtest/gtest.h>

// C++.
#include <future>
#include <thread>

using join::Futex;
using join::ScopedLock;

using namespace std::chrono_literals;

/**
 * @brief test lock.
 */
TEST (Futex, lock)
{
    Futex futex;
    auto task = std::async (std::launch::async, [&futex] () {
        futex.lock ();
        std::this_thread::sleep_for (15ms);
        futex.unlock ();
    });
    std::this_thread::sleep_for (5ms);
    auto beg = std::chrono::high_resolution_clock::now ();
    futex.lock ();
    auto end = std::chrono::high_resolution_clock::now ();
    EXPECT_GT (std::chrono::duration_cast<std::chrono::milliseconds> (end - beg), 5ms);
    futex.unlock ();
    task.wait ();
}

/**
 * @brief test tryLock.
 */
TEST (Futex, tryLock)
{
    Futex futex;
    auto task = std::async (std::launch::async, [&futex] () {
        futex.lock ();
        std::this_thread::sleep_for (15ms);
        futex.unlock ();
    });
    std::this_thread::sleep_for (5ms);
    EXPECT_FALSE (futex.tryLock ());
    std::this_thread::sleep_for (15ms);
    EXPECT_TRUE (futex.tryLock ());
    futex.unlock ();
    task.wait ();
}

/**
 * @brief test scoped lock.
 */
TEST (Futex, scopedLock)
{
    Futex futex;
    auto task = std::async (std::launch::async, [&futex] () {
        ScopedLock<Futex> lock (futex);
        std::this_thread::sleep_for (15ms);
    });
    std::this_thread::sleep_for (5ms);
    auto beg = std::chrono::high_resolution_clock::now ();
    futex.lock ();
    auto end = std::chrono::high_resolution_clock::now ();
    EXPECT_GT (std::chrono::duration_cast<std::chrono::milliseconds> (end - beg), 5ms);
    futex.unlock ();
    task.wait ();
}

/**
 * @brief test handle.
 */
TEST (Futex, handle)
{
    Futex futex;
    EXPECT_NE (futex.handle (), nullptr);
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
