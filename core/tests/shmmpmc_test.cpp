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
#include <join/thread.hpp>
#include <join/queue.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::Semaphore;
using join::ShmMem;
using join::Thread;

/**
 * @brief class used to test the multiple producer multiple consumer ring buffer.
 */
class ShmMpmc : public ::testing::Test
{
protected:
    /**
     * @brief set up the test fixture.
     */
    void SetUp () override
    {
        ASSERT_EQ (ShmMem::unlink (_name), 0) << join::lastError.message ();
    }

    /**
     * @brief tear down the test fixture.
     */
    void TearDown () override
    {
        ASSERT_EQ (ShmMem::unlink (_name), 0) << join::lastError.message ();
    }

    /// shared memory segment name.
    static const std::string _name;
};

const std::string ShmMpmc::_name = "/test_mpmc_shm";

TEST_F (ShmMpmc, tryPush)
{
    ShmMem::Mpmc::Queue <uint64_t> prod (512, _name);
    uint64_t data = 0;

    ASSERT_FALSE (prod.full ());
    ASSERT_EQ (prod.available (), 512);
    for (int i = 0; i < 512; ++i)
    {
        ASSERT_EQ (prod.tryPush (data), 0) << join::lastError.message ();
        ASSERT_EQ (prod.full (), i == 511);
        ASSERT_EQ (prod.available (), 511 - i);
    }
    ASSERT_EQ (prod.tryPush (data), -1);
    ASSERT_TRUE (prod.full ());
    ASSERT_EQ (prod.available (), 0);
}

TEST_F (ShmMpmc, push)
{
    ShmMem::Mpmc::Queue <uint64_t> prod (512, _name);
    uint64_t data = 0;

    ASSERT_FALSE (prod.full ());
    ASSERT_EQ (prod.available (), 512);
    for (int i = 0; i < 512; ++i)
    {
        ASSERT_EQ (prod.push (data), 0) << join::lastError.message ();
        ASSERT_EQ (prod.full (), i == 511);
        ASSERT_EQ (prod.available (), 511 - i);
    }
    ASSERT_TRUE (prod.full ());
    ASSERT_EQ (prod.available (), 0);
}

TEST_F (ShmMpmc, tryPop)
{
    ShmMem::Mpmc::Queue <uint64_t> prod (512, _name);
    ShmMem::Mpmc::Queue <uint64_t> cons (512, _name);
    uint64_t data = 0;

    ASSERT_EQ (cons.tryPop (data), -1);
    ASSERT_TRUE (cons.empty ());
    ASSERT_EQ (cons.pending (), 0);
    ASSERT_EQ (prod.tryPush (data), 0) << join::lastError.message ();
    ASSERT_FALSE (cons.empty ());
    ASSERT_EQ (cons.pending (), 1);
    ASSERT_EQ (cons.tryPop (data), 0) << join::lastError.message ();
    ASSERT_TRUE (cons.empty ());
    ASSERT_EQ (cons.pending (), 0);
    ASSERT_EQ (cons.tryPop (data), -1);
}

TEST_F (ShmMpmc, pop)
{
    ShmMem::Mpmc::Queue <uint64_t> prod (512, _name);
    ShmMem::Mpmc::Queue <uint64_t> cons (512, _name);
    uint64_t data = 0;

    ASSERT_TRUE (cons.empty ());
    ASSERT_EQ (cons.pending (), 0);
    ASSERT_EQ (prod.tryPush (data), 0) << join::lastError.message ();
    ASSERT_FALSE (cons.empty ());
    ASSERT_EQ (cons.pending (), 1);
    ASSERT_EQ (cons.pop (data), 0) << join::lastError.message ();
    ASSERT_TRUE (cons.empty ());
    ASSERT_EQ (cons.pending (), 0);
}

TEST_F (ShmMpmc, pushBenchmark)
{
    const uint64_t capacity = 512;
    const uint64_t num = 1000000;
    uint64_t data = 0;

    pid_t child = fork ();
    if (child == 0)
    {
        const int numConsumers = 4;
        std::vector <Thread> consumers;
        const uint64_t msgPerConsumer = num / numConsumers;
        Semaphore sem (_name);
        sem.wait ();
        for (int p = 0; p < numConsumers; ++p)
        {
            consumers.emplace_back([&] () {
                ShmMem::Mpmc::Queue <uint64_t> cons (capacity, _name);
                for (uint64_t i = 0; i < msgPerConsumer; ++i)
                {
                    while (cons.tryPop (data) == -1)
                    {
                        std::this_thread::yield ();
                    }
                }
            });
        }
        for (auto& consumer : consumers)
        {
            consumer.join ();
        }
        // empty pre-filled buffer.
        for (uint64_t i = 0; i < capacity; ++i)
        {
            ShmMem::Mpmc::Queue <uint64_t> cons (capacity, _name);
            while (cons.tryPop (data) == -1)
            {
                std::this_thread::yield ();
            }
        }
        _exit (0);
    }
    else
    {
        EXPECT_NE (child, -1);
        Semaphore sem (_name);
        const int numProducers = 4;
        std::vector <Thread> producers;
        const uint64_t msgPerProducer = num / numProducers;
        // pre-fill the buffer.
        for (uint64_t i = 0; i < capacity; ++i)
        {
            ShmMem::Mpmc::Queue <uint64_t> prod (capacity, _name);
            while (prod.tryPush (data) == -1)
            {
                std::this_thread::yield ();
            }
        }
        sem.post ();
        for (int p = 0; p < numProducers; ++p)
        {
            producers.emplace_back([&] () {
                ShmMem::Mpmc::Queue <uint64_t> prod (capacity, _name);
                for (uint64_t i = 0; i < msgPerProducer; ++i)
                {
                    EXPECT_EQ (prod.push (data), 0) << join::lastError.message ();
                }
            });
        }
        for (auto& producer : producers)
        {
            producer.join ();
        }
    }

    int status;
    waitpid (child, &status, 0);
    ASSERT_TRUE (WIFEXITED (status));
    ASSERT_EQ (WEXITSTATUS (status), 0);
}

TEST_F (ShmMpmc, popBenchmark)
{
    const uint64_t capacity = 512;
    const uint64_t num = 1000000;
    uint64_t data = 0;

    pid_t child = fork ();
    if (child == 0)
    {
        Semaphore sem (_name);
        const int numProducers = 4;
        std::vector <Thread> producers;
        const uint64_t msgPerProducer = num / numProducers;
        // pre-fill the buffer.
        for (uint64_t i = 0; i < capacity; ++i)
        {
            ShmMem::Mpmc::Queue <uint64_t> prod (capacity, _name);
            while (prod.tryPush (data) == -1)
            {
                std::this_thread::yield ();
            }
        }
        sem.post ();
        for (int p = 0; p < numProducers; ++p)
        {
            producers.emplace_back([&] () {
                ShmMem::Mpmc::Queue <uint64_t> prod (capacity, _name);
                for (uint64_t i = 0; i < msgPerProducer; ++i)
                {
                    while (prod.tryPush (data) == -1)
                    {
                        std::this_thread::yield ();
                    }
                }
            });
        }
        for (auto& producer : producers)
        {
            producer.join ();
        }
        _exit (0);
    }
    else
    {
        EXPECT_NE (child, -1);
        const int numConsumers = 4;
        std::vector <Thread> consumers;
        const uint64_t msgPerConsumer = num / numConsumers;
        Semaphore sem (_name);
        sem.wait ();
        for (int p = 0; p < numConsumers; ++p)
        {
            consumers.emplace_back([&] () {
                ShmMem::Mpmc::Queue <uint64_t> cons (capacity, _name);
                for (uint64_t i = 0; i < msgPerConsumer; ++i)
                {
                    EXPECT_EQ (cons.pop (data), 0) << join::lastError.message ();
                }
            });
        }
        for (auto& consumer : consumers)
        {
            consumer.join ();
        }
        // empty pre-filled buffer.
        for (uint64_t i = 0; i < capacity; ++i)
        {
            ShmMem::Mpmc::Queue <uint64_t> cons (capacity, _name);
            while (cons.tryPop (data) == -1)
            {
                std::this_thread::yield ();
            }
        }
    }

    int status;
    waitpid (child, &status, 0);
    ASSERT_TRUE (WIFEXITED (status));
    ASSERT_EQ (WEXITSTATUS (status), 0);
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
