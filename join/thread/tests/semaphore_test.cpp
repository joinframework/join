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
#include <join/semaphore.hpp>

// Libraries.
#include <gtest/gtest.h>

// C++.
#include <future>
#include <thread>

using join::Semaphore;

using namespace std::chrono_literals;

const std::string _name = "/test_sem";

/**
 * @brief test create.
 */
TEST (Semaphore, create)
{
    ASSERT_THROW (Semaphore (size_t (SEM_VALUE_MAX) + 1), std::system_error);
    ASSERT_THROW (Semaphore ("/"), std::system_error);
}

/**
 * @brief test wait.
 */
TEST (Semaphore, wait)
{
    Semaphore unnamed;
    auto task = std::async (std::launch::async, [&] () {
        std::this_thread::sleep_for (10ms);
        unnamed.post ();
    });
    auto beg = std::chrono::high_resolution_clock::now ();
    unnamed.wait ();
    auto end = std::chrono::high_resolution_clock::now ();
    EXPECT_GT (std::chrono::duration_cast <std::chrono::milliseconds> (end - beg), 5ms);
    task.wait ();

    Semaphore named (_name);
    task = std::async (std::launch::async, [&] () {
        std::this_thread::sleep_for (10ms);
        named.post ();
    });
    beg = std::chrono::high_resolution_clock::now ();
    named.wait ();
    end = std::chrono::high_resolution_clock::now ();
    EXPECT_GT (std::chrono::duration_cast <std::chrono::milliseconds> (end - beg), 5ms);
    task.wait ();
}

/**
 * @brief test tryWait.
 */
TEST (Semaphore, tryWait)
{
    Semaphore unnamed;
    ASSERT_FALSE (unnamed.tryWait ());
    auto task = std::async (std::launch::async, [&] () {
        unnamed.post ();
    });
    std::this_thread::sleep_for (10ms);
    ASSERT_TRUE (unnamed.tryWait ()) << join::lastError.message ();
    task.wait ();

    Semaphore named (_name);
    ASSERT_FALSE (named.tryWait ());
    task = std::async (std::launch::async, [&] () {
        named.post ();
    });
    std::this_thread::sleep_for (10ms);
    ASSERT_TRUE (named.tryWait ()) << join::lastError.message ();
    task.wait ();
}

/**
 * @brief test timedWait.
 */
TEST (Semaphore, timedWait)
{
    Semaphore unnamed;
    ASSERT_FALSE (unnamed.timedWait (10ms));
    auto task = std::async (std::launch::async, [&] () {
        unnamed.post ();
    });
    ASSERT_TRUE (unnamed.timedWait (10ms)) << join::lastError.message ();
    task.wait ();

    Semaphore named (_name);
    ASSERT_FALSE (named.timedWait (10ms));
    task = std::async (std::launch::async, [&] () {
        named.post ();
    });
    ASSERT_TRUE (named.timedWait (10ms)) << join::lastError.message ();
    task.wait ();
}

/**
 * @brief test value.
 */
TEST (Semaphore, value)
{
    Semaphore unnamed;
    ASSERT_EQ (unnamed.value (), 0);
    auto task = std::async (std::launch::async, [&] () {
        unnamed.post ();
    });
    std::this_thread::sleep_for (10ms);
    ASSERT_EQ (unnamed.value (), 1);
    ASSERT_TRUE (unnamed.timedWait (10ms)) << join::lastError.message ();
    ASSERT_EQ (unnamed.value (), 0);
    task.wait ();

    Semaphore named (_name);
    ASSERT_EQ (named.value (), 0);
    task = std::async (std::launch::async, [&] () {
        named.post ();
    });
    std::this_thread::sleep_for (10ms);
    ASSERT_EQ (named.value (), 1);
    ASSERT_TRUE (named.timedWait (10ms)) << join::lastError.message ();
    ASSERT_EQ (named.value (), 0);
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
