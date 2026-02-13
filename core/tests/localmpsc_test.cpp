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

TEST (LocalMpsc, tryPush)
{
    LocalMem::Mpsc::Queue <uint64_t> queue1 (512);
    uint64_t data = 0;

    LocalMem::Mpsc::Queue <uint64_t> queue2 (std::move (queue1));
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

TEST (LocalMpsc, push)
{
    LocalMem::Mpsc::Queue <uint64_t> queue1 (512);
    uint64_t data = 0;

    LocalMem::Mpsc::Queue <uint64_t> queue2 (std::move (queue1));
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

TEST (LocalMpsc, tryPop)
{
    LocalMem::Mpsc::Queue <uint64_t> queue1 (512);
    uint64_t data = 0;

    LocalMem::Mpsc::Queue <uint64_t> queue2 (std::move (queue1));
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

TEST (LocalMpsc, pop)
{
    LocalMem::Mpsc::Queue <uint64_t> queue1 (512);
    uint64_t data = 0;

    LocalMem::Mpsc::Queue <uint64_t> queue2 (std::move (queue1));
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

TEST (LocalMpsc, pushBenchmark)
{
    const uint64_t capacity = 512;
    const uint64_t num = 1000000;

    LocalMem::Mpsc::Queue <uint64_t> queue (capacity);
    std::atomic <bool> ready {false};

    Thread consumer ([&] () {
        uint64_t data = 0;
        while (!ready.load (std::memory_order_acquire))
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
        // empty pre-filled buffer
        for (uint64_t i = 0; i < capacity; ++i)
        {
            while (queue.tryPop (data) == -1)
            {
                std::this_thread::yield ();
            }
        }
    });

    const int numProducers = 4;
    std::vector <Thread> producers;
    const uint64_t msgPerProducer = num / numProducers;
    uint64_t data = 0;
    // pre-fill the buffer
    for (uint64_t i = 0; i < capacity; ++i)
    {
        while (queue.tryPush (data) == -1)
        {
            std::this_thread::yield ();
        }
    }
    ready.store (true, std::memory_order_release);
    for (int p = 0; p < numProducers; ++p)
    {
        producers.emplace_back ([&] () {
            for (uint64_t i = 0; i < msgPerProducer; ++i)
            {
                EXPECT_EQ (queue.push (data), 0) << join::lastError.message ();
            }
        });
    }
    for (auto& p : producers)
    {
        p.join ();
    }
    consumer.join ();
}

TEST (LocalMpsc, popBenchmark)
{
    const uint64_t capacity = 512;
    const uint64_t num = 1000000;
    const int numProducers = 4;
    const uint64_t msgPerProducer = num / numProducers;

    LocalMem::Mpsc::Queue <uint64_t> queue (capacity);
    std::atomic <bool> ready {false};

    std::vector <Thread> producers;
    for (int p = 0; p < numProducers; ++p)
    {
        producers.emplace_back ([&queue, msgPerProducer, &ready] () {
            uint64_t threadData = 0;

            while (!ready.load (std::memory_order_acquire))
            {
                std::this_thread::yield ();
            }
            for (uint64_t i = 0; i < msgPerProducer; ++i)
            {
                while (queue.tryPush (threadData) == -1)
                {
                    std::this_thread::yield ();
                }
            }
        });
    }

    uint64_t data = 0;
    ready.store (true, std::memory_order_release);
    for (uint64_t i = 0; i < num; ++i)
    {
        EXPECT_EQ (queue.pop (data), 0) << join::lastError.message ();
    }
    for (auto& p : producers)
    {
        p.join ();
    }
}

TEST (LocalMpsc, pending)
{
    LocalMem::Mpsc::Queue <uint64_t> queue1 (0);
    uint64_t data = 0;

    ASSERT_EQ (queue1.pending (), 0);
    ASSERT_EQ (queue1.tryPush (data), 0) << join::lastError.message ();
    ASSERT_EQ (queue1.pending (), 1);

    LocalMem::Mpsc::Queue <uint64_t> queue2 (0);
    queue2 = std::move (queue1);

    ASSERT_EQ (queue1.pending (), 0);
    ASSERT_EQ (queue2.pending (), 1);
}

TEST (LocalMpsc, available)
{
    LocalMem::Mpsc::Queue <uint64_t> queue1 (0);
    uint64_t data = 0;

    ASSERT_EQ (queue1.available (), 1);
    ASSERT_EQ (queue1.tryPush (data), 0) << join::lastError.message ();
    ASSERT_EQ (queue1.available (), 0);

    LocalMem::Mpsc::Queue <uint64_t> queue2 (0);
    queue2 = std::move (queue1);

    ASSERT_EQ (queue1.available (), 0);
    ASSERT_EQ (queue2.available (), 0);
}

TEST (LocalMpsc, full)
{
    LocalMem::Mpsc::Queue <uint64_t> queue1 (0);
    uint64_t data = 0;

    ASSERT_FALSE (queue1.full ());
    ASSERT_EQ (queue1.tryPush (data), 0) << join::lastError.message ();
    ASSERT_TRUE (queue1.full ());

    LocalMem::Mpsc::Queue <uint64_t> queue2 (0);
    queue2 = std::move (queue1);

    ASSERT_FALSE (queue1.full ());
    ASSERT_TRUE (queue2.full ());
}

TEST (LocalMpsc, empty)
{
    LocalMem::Mpsc::Queue <uint64_t> queue1 (0);
    uint64_t data = 0;

    ASSERT_TRUE (queue1.empty ());
    ASSERT_EQ (queue1.tryPush (data), 0) << join::lastError.message ();
    ASSERT_FALSE (queue1.empty ());

    LocalMem::Mpsc::Queue <uint64_t> queue2 (0);
    queue2 = std::move (queue1);

    ASSERT_TRUE (queue1.empty ());
    ASSERT_FALSE (queue2.empty ());
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
