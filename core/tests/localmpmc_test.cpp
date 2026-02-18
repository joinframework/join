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

TEST (LocalMpmc, tryPush)
{
    LocalMem::Mpmc::Queue <uint64_t> queue1 (512);
    uint64_t data = 0;

    LocalMem::Mpmc::Queue <uint64_t> queue2 (std::move (queue1));
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

TEST (LocalMpmc, push)
{
    LocalMem::Mpmc::Queue <uint64_t> queue1 (512);
    uint64_t data = 0;

    LocalMem::Mpmc::Queue <uint64_t> queue2 (std::move (queue1));
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

TEST (LocalMpmc, tryPop)
{
    LocalMem::Mpmc::Queue <uint64_t> queue1 (512);
    uint64_t data = 0;

    LocalMem::Mpmc::Queue <uint64_t> queue2 (std::move (queue1));
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

TEST (LocalMpmc, pop)
{
    LocalMem::Mpmc::Queue <uint64_t> queue1 (512);
    uint64_t data = 0;

    LocalMem::Mpmc::Queue <uint64_t> queue2 (std::move (queue1));
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

TEST (LocalMpmc, pushBenchmark)
{
    const uint64_t capacity = 512;
    const uint64_t num = 1000000;
    const int numProducers = 4;
    const int numConsumers = 4;
    const uint64_t msgPerProducer = num / numProducers;
    const uint64_t msgPerConsumer = num / numConsumers;

    LocalMem::Mpmc::Queue <uint64_t> queue (capacity);
    std::atomic <bool> ready {false};

    std::vector <Thread> consumers;
    std::vector <Thread> producers;

    for (int i = 0; i < numConsumers; ++i)
    {
        consumers.emplace_back ([&, msgPerConsumer] () {
            uint64_t data = 0;
            while (!ready.load (std::memory_order_acquire))
            {
                std::this_thread::yield ();
            }
            for (uint64_t j = 0; j < msgPerConsumer; ++j)
            {
                while (queue.tryPop (data) == -1)
                {
                    std::this_thread::yield ();
                }
            }
        });
    }

    // pre-fill the buffer
    uint64_t prefillData = 0;
    for (uint64_t i = 0; i < capacity; ++i)
    {
        while (queue.tryPush (prefillData) == -1)
        {
            std::this_thread::yield ();
        }
    }
    ready.store (true, std::memory_order_release);
    for (int i = 0; i < numProducers; ++i)
    {
        producers.emplace_back ([&, msgPerProducer] () {
            uint64_t localData = 0;
            while (!ready.load (std::memory_order_acquire))
            {
                std::this_thread::yield ();
            }
            for (uint64_t j = 0; j < msgPerProducer; ++j)
            {
                EXPECT_EQ (queue.push (localData), 0) << join::lastError.message ();
            }
        });
    }

    for (auto& p : producers) p.join ();
    for (auto& c : consumers) c.join ();


    uint64_t drainData = 0;

    // empty pre-filled buffer.
    for (uint64_t i = 0; i < capacity; ++i)
    {
        while (queue.tryPop (drainData) == -1)
        {
            std::this_thread::yield ();
        }
    }
}

TEST (LocalMpmc, popBenchmark)
{
    const uint64_t capacity = 512;
    const uint64_t num = 1000000;
    const int numProducers = 4;
    const int numConsumers = 4;
    const uint64_t msgPerProducer = num / numProducers;
    const uint64_t msgPerConsumer = num / numConsumers;

    LocalMem::Mpmc::Queue <uint64_t> queue (capacity);
    std::atomic <bool> ready {false};

    std::vector <Thread> producers;
    for (int p = 0; p < numProducers; ++p)
    {
        producers.emplace_back ([&queue, &ready, msgPerProducer] () {
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

    // pre-fill the buffer
    uint64_t prefillData = 0;
    for (uint64_t i = 0; i < capacity; ++i)
    {
        while (queue.tryPush (prefillData) == -1)
        {
            std::this_thread::yield ();
        }
    }
    ready.store (true, std::memory_order_release);
    std::vector <Thread> consumers;
    for (int p = 0; p < numConsumers; ++p)
    {
        consumers.emplace_back ([&queue, msgPerConsumer] () {
            uint64_t threadData = 0;
            for (uint64_t i = 0; i < msgPerConsumer; ++i)
            {
                EXPECT_EQ (queue.pop (threadData), 0) << join::lastError.message ();
            }
        });
    }

    for (auto& c : consumers) c.join ();
    for (auto& p : producers) p.join ();

    uint64_t drainData = 0;

    // empty pre-filled buffer.
    for (uint64_t i = 0; i < capacity; ++i)
    {
        while (queue.tryPop (drainData) == -1)
        {
            std::this_thread::yield ();
        }
    }
}

TEST (LocalMpmc, pending)
{
    LocalMem::Mpmc::Queue <uint64_t> queue1 (0);
    uint64_t data = 0;

    ASSERT_EQ (queue1.pending (), 0);
    ASSERT_EQ (queue1.tryPush (data), 0) << join::lastError.message ();
    ASSERT_EQ (queue1.pending (), 1);

    LocalMem::Mpmc::Queue <uint64_t> queue2 (0);
    queue2 = std::move (queue1);

    ASSERT_EQ (queue1.pending (), 0);
    ASSERT_EQ (queue2.pending (), 1);
}

TEST (LocalMpmc, available)
{
    LocalMem::Mpmc::Queue <uint64_t> queue1 (0);
    uint64_t data = 0;

    ASSERT_EQ (queue1.available (), 1);
    ASSERT_EQ (queue1.tryPush (data), 0) << join::lastError.message ();
    ASSERT_EQ (queue1.available (), 0);

    LocalMem::Mpmc::Queue <uint64_t> queue2 (0);
    queue2 = std::move (queue1);

    ASSERT_EQ (queue1.available (), 0);
    ASSERT_EQ (queue2.available (), 0);
}

TEST (LocalMpmc, full)
{
    LocalMem::Mpmc::Queue <uint64_t> queue1 (0);
    uint64_t data = 0;

    ASSERT_FALSE (queue1.full ());
    ASSERT_EQ (queue1.tryPush (data), 0) << join::lastError.message ();
    ASSERT_TRUE (queue1.full ());

    LocalMem::Mpmc::Queue <uint64_t> queue2 (0);
    queue2 = std::move (queue1);

    ASSERT_FALSE (queue1.full ());
    ASSERT_TRUE (queue2.full ());
}

TEST (LocalMpmc, empty)
{
    LocalMem::Mpmc::Queue <uint64_t> queue1 (0);
    uint64_t data = 0;

    ASSERT_TRUE (queue1.empty ());
    ASSERT_EQ (queue1.tryPush (data), 0) << join::lastError.message ();
    ASSERT_FALSE (queue1.empty ());

    LocalMem::Mpmc::Queue <uint64_t> queue2 (0);
    queue2 = std::move (queue1);

    ASSERT_TRUE (queue1.empty ());
    ASSERT_FALSE (queue2.empty ());
}

TEST (LocalMpmc, memory)
{
    LocalMem::Mpmc::Queue <uint64_t> queue (0);
    ASSERT_NE (queue.memory ().get (), nullptr);
    ASSERT_EQ (queue.memory ().mbind (0), 0) << join::lastError.message ();
    ASSERT_EQ (queue.memory ().mlock (), 0) << join::lastError.message ();

    const LocalMem::Mpmc::Queue <uint64_t>& cqueue = queue;
    ASSERT_NE (cqueue.memory ().get (), nullptr);
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
