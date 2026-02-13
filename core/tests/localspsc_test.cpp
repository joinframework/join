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
#include <join/thread.hpp>
#include <join/queue.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::LocalMem;
using join::Thread;

TEST (LocalSpsc, tryPush)
{
    LocalMem::Spsc::Queue <uint64_t> queue1 (512);
    uint64_t data = 0;

    LocalMem::Spsc::Queue <uint64_t> queue2 (std::move (queue1));
    ASSERT_EQ (queue1.tryPush (data), -1);
    ASSERT_FALSE (queue2.full ());
    ASSERT_EQ (queue2.available (), 512);
    for (int i = 0; i < 512; ++i)
    {
        ASSERT_EQ (queue2.tryPush (data), 0) << join::lastError.message ();
        ASSERT_EQ (queue2.full (), i == 511);
        ASSERT_EQ (queue2.available (), 511 - i);
    }
    ASSERT_EQ (queue2.tryPush (data), -1);
    ASSERT_TRUE (queue2.full ());
    ASSERT_EQ (queue2.available (), 0);
}

TEST (LocalSpsc, push)
{
    LocalMem::Spsc::Queue <uint64_t> queue1 (512);
    uint64_t data = 0;

    LocalMem::Spsc::Queue <uint64_t> queue2 (std::move (queue1));
    ASSERT_EQ (queue1.push (data), -1);
    ASSERT_FALSE (queue2.full ());
    ASSERT_EQ (queue2.available (), 512);
    for (int i = 0; i < 512; ++i)
    {
        ASSERT_EQ (queue2.push (data), 0) << join::lastError.message ();
        ASSERT_EQ (queue2.full (), i == 511);
        ASSERT_EQ (queue2.available (), 511 - i);
    }
    ASSERT_TRUE (queue2.full ());
    ASSERT_EQ (queue2.available (), 0);
}

TEST (LocalSpsc, tryPop)
{
    LocalMem::Spsc::Queue <uint64_t> queue1 (512);
    uint64_t data = 0;

    LocalMem::Spsc::Queue <uint64_t> queue2 (std::move (queue1));
    ASSERT_EQ (queue1.tryPop (data), -1);
    ASSERT_EQ (queue2.tryPop (data), -1);
    ASSERT_TRUE (queue2.empty ());
    ASSERT_EQ (queue2.pending (), 0);
    ASSERT_EQ (queue2.tryPush (data), 0) << join::lastError.message ();
    ASSERT_FALSE (queue2.empty ());
    ASSERT_EQ (queue2.pending (), 1);
    ASSERT_EQ (queue2.tryPop (data), 0) << join::lastError.message ();
    ASSERT_TRUE (queue2.empty ());
    ASSERT_EQ (queue2.pending (), 0);
    ASSERT_EQ (queue2.tryPop (data), -1);
}

TEST (LocalSpsc, pop)
{
    LocalMem::Spsc::Queue <uint64_t> queue1 (512);
    uint64_t data = 0;

    LocalMem::Spsc::Queue <uint64_t> queue2 (std::move (queue1));
    ASSERT_EQ (queue1.pop (data), -1);
    ASSERT_TRUE (queue2.empty ());
    ASSERT_EQ (queue2.pending (), 0);
    ASSERT_EQ (queue2.tryPush (data), 0) << join::lastError.message ();
    ASSERT_FALSE (queue2.empty ());
    ASSERT_EQ (queue2.pending (), 1);
    ASSERT_EQ (queue2.pop (data), 0) << join::lastError.message ();
    ASSERT_TRUE (queue2.empty ());
    ASSERT_EQ (queue2.pending (), 0);
}

TEST (LocalSpsc, pushBenchmark)
{
    const uint64_t capacity = 512;
    const uint64_t num = 1000000;

    LocalMem::Spsc::Queue<uint64_t> queue(capacity);
    std::atomic <bool> ready{false};

    std::thread consumer ([&] () {
        uint64_t data = 0;

        while (!ready.load(std::memory_order_acquire))
        {
            std::this_thread::yield ();
        }

        for (uint64_t i = 0; i < num; ++i)
        {
            while (queue.tryPop (data) == -1)
            {
                std::this_thread::yield ();
            }
        }

        for (uint64_t i = 0; i < capacity; ++i)
        {
            while (queue.tryPop (data) == -1)
            {
                std::this_thread::yield ();
            }
        }
    });

    uint64_t data = 0;

    // Pre-fill the buffer
    for (uint64_t i = 0; i < capacity; ++i)
    {
        while (queue.tryPush (data) == -1)
        {
            std::this_thread::yield ();
        }
    }

    ready.store (true, std::memory_order_release);

    for (uint64_t i = 0; i < num; ++i)
    {
        EXPECT_EQ (queue.push (data), 0) << join::lastError.message ();
    }

    consumer.join();
}

TEST (LocalSpsc, popBenchmark)
{
    const uint64_t capacity = 512;
    const uint64_t num = 1000000;

    LocalMem::Spsc::Queue <uint64_t> queue (capacity);
    std::atomic <bool> producerReady {false};

    std::thread producer ([&] () {
        uint64_t data = 0;

        producerReady.store (true, std::memory_order_release);

        for (uint64_t i = 0; i < num; ++i)
        {
            while (queue.tryPush (data) == -1)
            {
                std::this_thread::yield ();
            }
        }
    });

    while (!producerReady.load (std::memory_order_acquire))
    {
        std::this_thread::yield ();
    }

    uint64_t data = 0;

    for (uint64_t i = 0; i < num; ++i)
    {
        EXPECT_EQ (queue.pop (data), 0) << join::lastError.message ();
    }

    producer.join ();
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
