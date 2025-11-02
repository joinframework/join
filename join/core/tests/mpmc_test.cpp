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
#include <join/shared.hpp>
#include <join/utils.hpp>

// Libraries.
#include <gtest/gtest.h>

// C++.
#include <algorithm>

using namespace std::chrono_literals;

using join::BasicQueue;
using join::Semaphore;
using join::Thread;
using join::Mpmc;

/**
 * @brief class used to test the multiple producer multiple consumer ring buffer.
 */
class MpmcBuffer : public ::testing::Test
{
protected:
    /**
     * @brief set up the test fixture.
     */
    void SetUp ()
    {
        ASSERT_EQ (BasicQueue <Mpmc>::unlink (_name), 0) << join::lastError.message ();
    }

    /**
     * @brief tear down the test fixture.
     */
    void TearDown () override
    {
        ASSERT_EQ (BasicQueue <Mpmc>::unlink (_name), 0) << join::lastError.message ();
    }

    /// shared memory segment name.
    static const std::string _name;
};

const std::string MpmcBuffer::_name = "/test_mpmc_shm";

TEST_F (MpmcBuffer, open)
{
    Mpmc::Producer prod1 (_name, 64, 8), prod2 ("", 64, 8), prod3(_name, 128, 16), prod4 (_name, 1, std::numeric_limits <off_t>::max () - sizeof (join::SharedSegment));
    Mpmc::Consumer cons1 (_name, 64, 8), cons2 (_name, 128, 16);

    ASSERT_THROW (Mpmc::Consumer (_name, 128, std::numeric_limits <uint64_t>::max ()), std::overflow_error);
    ASSERT_THROW (Mpmc::Consumer (_name, 1, std::numeric_limits <off_t>::max ()), std::overflow_error);
    ASSERT_EQ (prod1.elementSize (), 64);
    ASSERT_EQ (prod1.capacity (), 8);
    ASSERT_FALSE (prod1.opened ());
    ASSERT_EQ (prod1.size (), 64 * 8);
    ASSERT_EQ (prod1.get (), nullptr);
    ASSERT_EQ (prod1.open (), 0) << join::lastError.message ();
    ASSERT_EQ (prod1.size (), 64 * 8);
    ASSERT_NE (prod1.get (), nullptr);
    ASSERT_TRUE (prod1.opened ());
    ASSERT_EQ (prod1.open (), -1);
    ASSERT_TRUE (prod1.opened ());
    ASSERT_EQ (prod2.open (), -1);
    ASSERT_EQ (prod3.open (), -1);
    ASSERT_EQ (cons1.elementSize (), 64);
    ASSERT_EQ (cons1.capacity (), 8);
    ASSERT_FALSE (cons1.opened ());
    ASSERT_EQ (cons1.get (), nullptr);
    ASSERT_EQ (cons1.open (), 0) << join::lastError.message ();
    ASSERT_NE (cons1.get (), nullptr);
    ASSERT_TRUE (cons1.opened ());
    ASSERT_EQ (cons1.open (), -1);
    ASSERT_TRUE (cons1.opened ());
    ASSERT_EQ (cons2.elementSize (), 128);
    ASSERT_EQ (cons2.capacity (), 16);
    ASSERT_FALSE (cons2.opened ());
    ASSERT_EQ (cons2.get (), nullptr);
    ASSERT_EQ (cons2.open (), -1);
    ASSERT_EQ (cons2.get (), nullptr);
    ASSERT_FALSE (cons2.opened ());
    prod1.close ();
    ASSERT_FALSE (prod1.opened ());
    cons1.close ();
    ASSERT_FALSE (cons1.opened ());
}

TEST_F (MpmcBuffer, tryPush)
{
    Mpmc::Producer prod (_name, 64, 8);
    char data[64] = {};

    ASSERT_EQ (prod.tryPush (data), -1);
    ASSERT_EQ (prod.open (), 0) << join::lastError.message ();
    ASSERT_EQ (prod.tryPush (nullptr), -1);
    ASSERT_FALSE (prod.full ());
    ASSERT_EQ (prod.available (), 8);
    for (int i = 0; i < 8; ++i)
    {
        ASSERT_EQ (prod.tryPush (data), 0) << join::lastError.message ();
        ASSERT_EQ (prod.full (), i == 7);
        ASSERT_EQ (prod.available (), 7 - i);
    }
    ASSERT_EQ (prod.tryPush (data), -1);
    ASSERT_TRUE (prod.full ());
    ASSERT_EQ (prod.available (), 0);
    prod.close ();
}

TEST_F (MpmcBuffer, push)
{
    Mpmc::Producer prod (_name, 64, 8);
    char data[64] = {};

    ASSERT_EQ (prod.push (data), -1);
    ASSERT_EQ (prod.open (), 0) << join::lastError.message ();
    ASSERT_EQ (prod.push (nullptr), -1);
    ASSERT_FALSE (prod.full ());
    ASSERT_EQ (prod.available (), 8);
    for (int i = 0; i < 8; ++i)
    {
        ASSERT_EQ (prod.push (data), 0) << join::lastError.message ();
        ASSERT_EQ (prod.full (), i == 7);
        ASSERT_EQ (prod.available (), 7 - i);
    }
    prod.close ();
}

TEST_F (MpmcBuffer, timedPush)
{
    Mpmc::Producer prod (_name, 64, 8);
    char data[64] = {};

    ASSERT_EQ (prod.timedPush (data, 5ms), -1);
    ASSERT_EQ (prod.open (), 0) << join::lastError.message ();
    ASSERT_EQ (prod.timedPush (nullptr, 5ms), -1);
    ASSERT_FALSE (prod.full ());
    ASSERT_EQ (prod.available (), 8);
    for (int i = 0; i < 8; ++i)
    {
        ASSERT_EQ (prod.timedPush (data, 5ms), 0) << join::lastError.message ();
        ASSERT_EQ (prod.full (), i == 7);
        ASSERT_EQ (prod.available (), 7 - i);
    }
    ASSERT_EQ (prod.timedPush (data, 5ms), -1);
    ASSERT_TRUE (prod.full ());
    ASSERT_EQ (prod.available (), 0);
    prod.close ();
}

TEST_F (MpmcBuffer, tryPop)
{
    Mpmc::Producer prod (_name, 64, 8);
    Mpmc::Consumer cons (_name, 64, 8);
    char data[64] = {};

    ASSERT_EQ (prod.open (), 0) << join::lastError.message ();
    ASSERT_EQ (cons.tryPop (data), -1);
    ASSERT_EQ (cons.open (), 0) << join::lastError.message ();
    ASSERT_EQ (cons.tryPop (nullptr), -1);
    ASSERT_TRUE (cons.empty ());
    ASSERT_EQ (cons.pending (), 0);
    ASSERT_EQ (prod.tryPush (data), 0) << join::lastError.message ();
    ASSERT_FALSE (cons.empty ());
    ASSERT_EQ (cons.pending (), 1);
    ASSERT_EQ (cons.tryPop (data), 0) << join::lastError.message ();
    ASSERT_TRUE (cons.empty ());
    ASSERT_EQ (cons.pending (), 0);
    ASSERT_EQ (cons.tryPop (data), -1);
    cons.close ();
    prod.close ();
}

TEST_F (MpmcBuffer, pop)
{
    Mpmc::Producer prod (_name, 64, 8);
    Mpmc::Consumer cons (_name, 64, 8);
    char data[64] = {};

    ASSERT_EQ (prod.open (), 0) << join::lastError.message ();
    ASSERT_EQ (cons.pop (data), -1);
    ASSERT_EQ (cons.open (), 0) << join::lastError.message ();
    ASSERT_EQ (cons.pop (nullptr), -1);
    ASSERT_TRUE (cons.empty ());
    ASSERT_EQ (cons.pending (), 0);
    ASSERT_EQ (prod.tryPush (data), 0) << join::lastError.message ();
    ASSERT_FALSE (cons.empty ());
    ASSERT_EQ (cons.pending (), 1);
    ASSERT_EQ (cons.pop (data), 0) << join::lastError.message ();
    ASSERT_TRUE (cons.empty ());
    ASSERT_EQ (cons.pending (), 0);
    cons.close ();
    prod.close ();
}

TEST_F (MpmcBuffer, timedPop)
{
    Mpmc::Producer prod (_name, 64, 8);
    Mpmc::Consumer cons (_name, 64, 8);
    char data[64] = {};

    ASSERT_EQ (prod.open (), 0) << join::lastError.message ();
    ASSERT_EQ (cons.timedPop (data, 5ms), -1);
    ASSERT_EQ (cons.open (), 0) << join::lastError.message ();
    ASSERT_EQ (cons.timedPop (nullptr, 5ms), -1);
    ASSERT_TRUE (cons.empty ());
    ASSERT_EQ (cons.pending (), 0);
    ASSERT_EQ (prod.tryPush (data), 0) << join::lastError.message ();
    ASSERT_FALSE (cons.empty ());
    ASSERT_EQ (cons.pending (), 1);
    ASSERT_EQ (cons.timedPop (data, 5ms), 0) << join::lastError.message ();
    ASSERT_TRUE (cons.empty ());
    ASSERT_EQ (cons.pending (), 0);
    ASSERT_EQ (cons.timedPop (data, 5ms), -1);
    cons.close ();
    prod.close ();
}

TEST_F (MpmcBuffer, pushBenchmark)
{
    const uint64_t num = 1000000;
    const uint64_t capacity = 144;
    const uint64_t size = 1472;
    char data[size] = {};

    pid_t child = fork ();
    if (child == 0)
    {
        const int numConsumers = 4;
        std::vector <Thread> consumers;
        const uint64_t msgPerConsumer = num / numConsumers;
        Semaphore sem (_name);
        sem.wait ();
        Mpmc::Consumer cons0 (_name, size, capacity);
        cons0.open ();
        for (int p = 0; p < numConsumers; ++p)
        {
            consumers.emplace_back([&] () {
                Mpmc::Consumer cons (_name, size, capacity);
                cons.open ();
                for (uint64_t i = 0; i < msgPerConsumer; ++i)
                {
                    while (cons.tryPop (data) == -1)
                    {
                        std::this_thread::yield ();
                    }
                }
                cons.close ();
            });
        }
        for (auto& consumer : consumers)
        {
            consumer.join ();
        }
        // empty pre-filled buffer.
        for (uint64_t i = 0; i < capacity; ++i)
        {
            while (cons0.tryPop (data) == -1)
            {
                std::this_thread::yield ();
            }
        }
        cons0.close ();
        _exit (0);
    }
    else
    {
        EXPECT_NE (child, -1);
        Semaphore sem (_name);
        const int numProducers = 4;
        std::vector <Thread> producers;
        const uint64_t msgPerProducer = num / numProducers;
        Mpmc::Producer prod0 (_name, size, capacity);
        EXPECT_EQ (prod0.open (), 0) << join::lastError.message ();
        // pre-fill the buffer.
        for (uint64_t i = 0; i < capacity; ++i)
        {
            while (prod0.tryPush (data) == -1)
            {
                std::this_thread::yield ();
            }
        }
        sem.post ();
        for (int p = 0; p < numProducers; ++p)
        {
            producers.emplace_back([&, p] () {
                Mpmc::Producer prod (_name, size, capacity);
                EXPECT_EQ (prod.open (), 0) << join::lastError.message ();
                for (uint64_t i = 0; i < msgPerProducer; ++i)
                {
                    EXPECT_EQ (prod.push (data), 0) << join::lastError.message ();
                }
                prod.close ();
            });
        }
        for (auto& producer : producers)
        {
            producer.join ();
        }
        prod0.close ();
    }

    int status;
    waitpid (child, &status, 0);
    ASSERT_TRUE (WIFEXITED (status));
    ASSERT_EQ (WEXITSTATUS (status), 0);
}

TEST_F (MpmcBuffer, timedPushBenchmark)
{
    const uint64_t num = 1000000;
    const uint64_t capacity = 144;
    const uint64_t size = 1472;
    char data[size] = {};

    pid_t child = fork ();
    if (child == 0)
    {
        const int numConsumers = 4;
        std::vector <Thread> consumers;
        const uint64_t msgPerConsumer = num / numConsumers;
        Semaphore sem (_name);
        sem.wait ();
        Mpmc::Consumer cons0 (_name, size, capacity);
        cons0.open ();
        for (int p = 0; p < numConsumers; ++p)
        {
            consumers.emplace_back([&] () {
                Mpmc::Consumer cons (_name, size, capacity);
                cons.open ();
                for (uint64_t i = 0; i < msgPerConsumer; ++i)
                {
                    while (cons.tryPop (data) == -1)
                    {
                        std::this_thread::yield ();
                    }
                }
                cons.close ();
            });
        }
        for (auto& consumer : consumers)
        {
            consumer.join ();
        }
        // empty pre-filled buffer.
        for (uint64_t i = 0; i < capacity; ++i)
        {
            while (cons0.tryPop (data) == -1)
            {
                std::this_thread::yield ();
            }
        }
        cons0.close ();
        _exit (0);
    }
    else
    {
        EXPECT_NE (child, -1);
        Semaphore sem (_name);
        const int numProducers = 4;
        std::vector <Thread> producers;
        const uint64_t msgPerProducer = num / numProducers;
        std::vector <uint64_t> sendTimestamps;
        sendTimestamps.resize (num);
        Mpmc::Producer prod0 (_name, size, capacity);
        EXPECT_EQ (prod0.open (), 0) << join::lastError.message ();
        // pre-fill the buffer.
        for (uint64_t i = 0; i < capacity; ++i)
        {
            while (prod0.tryPush (data) == -1)
            {
                std::this_thread::yield ();
            }
        }
        sem.post ();
        for (int p = 0; p < numProducers; ++p)
        {
            producers.emplace_back([&, p] () {
                Mpmc::Producer prod (_name, size, capacity);
                EXPECT_EQ (prod.open (), 0) << join::lastError.message ();
                for (uint64_t i = 0; i < msgPerProducer; ++i)
                {
                    EXPECT_EQ (prod.timedPush (data, 1s), 0) << join::lastError.message ();
                }
                prod.close ();
            });
        }
        for (auto& producer : producers)
        {
            producer.join ();
        }
        prod0.close ();
    }

    int status;
    waitpid (child, &status, 0);
    ASSERT_TRUE (WIFEXITED (status));
    ASSERT_EQ (WEXITSTATUS (status), 0);
}

TEST_F (MpmcBuffer, popBenchmark)
{
    const uint64_t num = 1000000;
    const uint64_t capacity = 144;
    const uint64_t size = 1472;
    char data[size] = {};

    pid_t child = fork ();
    if (child == 0)
    {
        Semaphore sem (_name);
        const int numProducers = 4;
        std::vector <Thread> producers;
        const uint64_t msgPerProducer = num / numProducers;
        Mpmc::Producer prod0 (_name, size, capacity);
        prod0.open ();
        // pre-fill the buffer.
        for (uint64_t i = 0; i < capacity; ++i)
        {
            while (prod0.tryPush (data) == -1)
            {
                std::this_thread::yield ();
            }
        }
        sem.post ();
        for (int p = 0; p < numProducers; ++p)
        {
            producers.emplace_back([&] () {
                Mpmc::Producer prod (_name, size, capacity);
                prod.open ();
                for (uint64_t i = 0; i < msgPerProducer; ++i)
                {
                    while (prod.tryPush (data) == -1)
                    {
                        std::this_thread::yield ();
                    }
                }
                prod.close ();
            });
        }
        for (auto& producer : producers)
        {
            producer.join ();
        }
        prod0.close ();
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
        Mpmc::Consumer cons0 (_name, size, capacity);
        EXPECT_EQ (cons0.open (), 0) << join::lastError.message ();
        for (int p = 0; p < numConsumers; ++p)
        {
            consumers.emplace_back([&, p] () {
                Mpmc::Consumer cons (_name, size, capacity);
                EXPECT_EQ (cons.open (), 0) << join::lastError.message ();
                for (uint64_t i = 0; i < msgPerConsumer; ++i)
                {
                    EXPECT_EQ (cons.pop (data), 0) << join::lastError.message ();
                }
                cons.close ();
            });
        }
        for (auto& consumer : consumers)
        {
            consumer.join ();
        }
        // empty pre-filled buffer.
        for (uint64_t i = 0; i < capacity; ++i)
        {
            while (cons0.tryPop (data) == -1)
            {
                std::this_thread::yield ();
            }
        }
        cons0.close ();
    }

    int status;
    waitpid (child, &status, 0);
    ASSERT_TRUE (WIFEXITED (status));
    ASSERT_EQ (WEXITSTATUS (status), 0);
}

TEST_F (MpmcBuffer, timedPopBenchmark)
{
    const uint64_t num = 1000000;
    const uint64_t capacity = 144;
    const uint64_t size = 1472;
    char data[size] = {};

    pid_t child = fork ();
    if (child == 0)
    {
        Semaphore sem (_name);
        const int numProducers = 4;
        std::vector <Thread> producers;
        const uint64_t msgPerProducer = num / numProducers;
        Mpmc::Producer prod0 (_name, size, capacity);
        prod0.open ();
        // pre-fill the buffer.
        for (uint64_t i = 0; i < capacity; ++i)
        {
            while (prod0.tryPush (data) == -1)
            {
                std::this_thread::yield ();
            }
        }
        sem.post ();
        for (int p = 0; p < numProducers; ++p)
        {
            producers.emplace_back([&] () {
                Mpmc::Producer prod (_name, size, capacity);
                prod.open ();
                for (uint64_t i = 0; i < msgPerProducer; ++i)
                {
                    while (prod.tryPush (data) == -1)
                    {
                        std::this_thread::yield ();
                    }
                }
                prod.close ();
            });
        }
        for (auto& producer : producers)
        {
            producer.join ();
        }
        prod0.close ();
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
        Mpmc::Consumer cons0 (_name, size, capacity);
        EXPECT_EQ (cons0.open (), 0) << join::lastError.message ();
        for (int p = 0; p < numConsumers; ++p)
        {
            consumers.emplace_back([&, p] () {
                Mpmc::Consumer cons (_name, size, capacity);
                EXPECT_EQ (cons.open (), 0) << join::lastError.message ();
                for (uint64_t i = 0; i < msgPerConsumer; ++i)
                {
                    EXPECT_EQ (cons.timedPop (data, 1s), 0) << join::lastError.message ();
                }
                cons.close ();
            });
        }
        for (auto& consumer : consumers)
        {
            consumer.join ();
        }
        // empty pre-filled buffer.
        for (uint64_t i = 0; i < capacity; ++i)
        {
            while (cons0.tryPop (data) == -1)
            {
                std::this_thread::yield ();
            }
        }
        cons0.close ();
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
    ::mlockall (MCL_CURRENT | MCL_FUTURE);
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
