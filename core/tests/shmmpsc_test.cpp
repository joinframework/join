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
 * @brief class used to test the multiple producer single consumer ring buffer.
 */
class ShmMpsc : public ::testing::Test
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

const std::string ShmMpsc::_name = "/test_mpsc_shm";

TEST_F (ShmMpsc, create)
{
    ShmMem::Mpsc::Queue <uint64_t> prod1 (0, _name);
    ASSERT_THROW (ShmMem::Mpsc::Queue <uint64_t> (2, _name), std::runtime_error);
}

TEST_F (ShmMpsc, tryPush)
{
    ShmMem::Mpsc::Queue <uint64_t> prod1 (512, _name);
    uint64_t data = 0;

    ShmMem::Mpsc::Queue <uint64_t> prod2 (std::move (prod1));
    ASSERT_EQ (prod1.tryPush (data), -1);
    ASSERT_FALSE (prod2.full ());
    ASSERT_EQ (prod2.available (), 512);
    for (int i = 0; i < 512; ++i)
    {
        ASSERT_EQ (prod2.tryPush (data), 0) << join::lastError.message ();
        ASSERT_EQ (prod2.full (), i == 511);
        ASSERT_EQ (prod2.available (), 511 - i);
    }
    ASSERT_EQ (prod2.tryPush (data), -1);
    ASSERT_TRUE (prod2.full ());
    ASSERT_EQ (prod2.available (), 0);
}

TEST_F (ShmMpsc, push)
{
    ShmMem::Mpsc::Queue <uint64_t> prod1 (512, _name);
    uint64_t data = 0;

    ShmMem::Mpsc::Queue <uint64_t> prod2 (std::move (prod1));
    ASSERT_EQ (prod1.push (data), -1);
    ASSERT_FALSE (prod2.full ());
    ASSERT_EQ (prod2.available (), 512);
    for (int i = 0; i < 512; ++i)
    {
        ASSERT_EQ (prod2.push (data), 0) << join::lastError.message ();
        ASSERT_EQ (prod2.full (), i == 511);
        ASSERT_EQ (prod2.available (), 511 - i);
    }
    ASSERT_TRUE (prod2.full ());
    ASSERT_EQ (prod2.available (), 0);
}

TEST_F (ShmMpsc, tryPop)
{
    ShmMem::Mpsc::Queue <uint64_t> prod (512, _name);
    ShmMem::Mpsc::Queue <uint64_t> cons1 (512, _name);
    uint64_t data = 0;

    ShmMem::Mpsc::Queue <uint64_t> cons2 (std::move (cons1));
    ASSERT_EQ (cons1.tryPop (data), -1);
    ASSERT_EQ (cons2.tryPop (data), -1);
    ASSERT_TRUE (cons2.empty ());
    ASSERT_EQ (cons2.pending (), 0);
    ASSERT_EQ (prod.tryPush (data), 0) << join::lastError.message ();
    ASSERT_FALSE (cons2.empty ());
    ASSERT_EQ (cons2.pending (), 1);
    ASSERT_EQ (cons2.tryPop (data), 0) << join::lastError.message ();
    ASSERT_TRUE (cons2.empty ());
    ASSERT_EQ (cons2.pending (), 0);
    ASSERT_EQ (cons2.tryPop (data), -1);
}

TEST_F (ShmMpsc, pop)
{
    ShmMem::Mpsc::Queue <uint64_t> prod (512, _name);
    ShmMem::Mpsc::Queue <uint64_t> cons1 (512, _name);
    uint64_t data = 0;

    ShmMem::Mpsc::Queue <uint64_t> cons2 (std::move (cons1));
    ASSERT_EQ (cons1.pop (data), -1);
    ASSERT_TRUE (cons2.empty ());
    ASSERT_EQ (cons2.pending (), 0);
    ASSERT_EQ (prod.tryPush (data), 0) << join::lastError.message ();
    ASSERT_FALSE (cons2.empty ());
    ASSERT_EQ (cons2.pending (), 1);
    ASSERT_EQ (cons2.pop (data), 0) << join::lastError.message ();
    ASSERT_TRUE (cons2.empty ());
    ASSERT_EQ (cons2.pending (), 0);
}

TEST_F (ShmMpsc, pushBenchmark)
{
    const uint64_t capacity = 512;
    const uint64_t num = 1000000;
    uint64_t data = 0;

    pid_t child = fork ();
    if (child == 0)
    {
        Semaphore sem (_name);
        sem.wait ();
        ShmMem::Mpsc::Queue <uint64_t> cons (capacity, _name);
        for (uint64_t i = 0; i < num; ++i)
        {
            while (cons.tryPop (data) == -1)
            {
                std::this_thread::yield ();
            }
        }
        // empty pre-filled buffer.
        for (uint64_t i = 0; i < capacity; ++i)
        {
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
            ShmMem::Mpsc::Queue <uint64_t> prod (capacity, _name);
            while (prod.tryPush (data) == -1)
            {
                std::this_thread::yield ();
            }
        }
        sem.post ();
        for (int p = 0; p < numProducers; ++p)
        {
            producers.emplace_back([&] () {
                ShmMem::Mpsc::Queue <uint64_t> prod (capacity, _name);
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

TEST_F (ShmMpsc, popBenchmark)
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
        ShmMem::Mpsc::Queue <uint64_t> prod0 (capacity, _name);
        sem.post ();
        for (int p = 0; p < numProducers; ++p)
        {
            producers.emplace_back([&] () {
                ShmMem::Mpsc::Queue <uint64_t> prod (capacity, _name);
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
        Semaphore sem (_name);
        sem.wait ();
        ShmMem::Mpsc::Queue <uint64_t> cons (capacity, _name);
        for (uint64_t i = 0; i < num; ++i)
        {
            EXPECT_EQ (cons.pop (data), 0) << join::lastError.message ();
        }
    }

    int status;
    waitpid (child, &status, 0);
    ASSERT_TRUE (WIFEXITED (status));
    ASSERT_EQ (WEXITSTATUS (status), 0);
}

TEST_F (ShmMpsc, pending)
{
    ShmMem::Mpsc::Queue <uint64_t> prod1 (0, _name);
    uint64_t data = 0;

    ASSERT_EQ (prod1.pending (), 0);
    ASSERT_EQ (prod1.tryPush (data), 0) << join::lastError.message ();
    ASSERT_EQ (prod1.pending (), 1);

    ShmMem::Mpsc::Queue <uint64_t> prod2 (0, _name);
    prod2 = std::move (prod1);

    ASSERT_EQ (prod1.pending (), 0);
    ASSERT_EQ (prod2.pending (), 1);
}

TEST_F (ShmMpsc, available)
{
    ShmMem::Mpsc::Queue <uint64_t> prod1 (0, _name);
    uint64_t data = 0;

    ASSERT_EQ (prod1.available (), 1);
    ASSERT_EQ (prod1.tryPush (data), 0) << join::lastError.message ();
    ASSERT_EQ (prod1.available (), 0);

    ShmMem::Mpsc::Queue <uint64_t> prod2 (0, _name);
    prod2 = std::move (prod1);

    ASSERT_EQ (prod1.available (), 0);
    ASSERT_EQ (prod2.available (), 0);
}

TEST_F (ShmMpsc, full)
{
    ShmMem::Mpsc::Queue <uint64_t> prod1 (0, _name);
    uint64_t data = 0;

    ASSERT_FALSE (prod1.full ());
    ASSERT_EQ (prod1.tryPush (data), 0) << join::lastError.message ();
    ASSERT_TRUE (prod1.full ());

    ShmMem::Mpsc::Queue <uint64_t> prod2 (0, _name);
    prod2 = std::move (prod1);

    ASSERT_FALSE (prod1.full ());
    ASSERT_TRUE (prod2.full ());
}

TEST_F (ShmMpsc, empty)
{
    ShmMem::Mpsc::Queue <uint64_t> prod1 (0, _name);
    uint64_t data = 0;

    ASSERT_TRUE (prod1.empty ());
    ASSERT_EQ (prod1.tryPush (data), 0) << join::lastError.message ();
    ASSERT_FALSE (prod1.empty ());

    ShmMem::Mpsc::Queue <uint64_t> prod2 (0, _name);
    prod2 = std::move (prod1);

    ASSERT_TRUE (prod1.empty ());
    ASSERT_FALSE (prod2.empty ());
}

TEST_F (ShmMpsc, memory)
{
    ShmMem::Mpsc::Queue <uint64_t> queue (0, _name);
    ASSERT_NE (queue.memory ().get (), nullptr);
    ASSERT_EQ (queue.memory ().mbind (0), 0) << join::lastError.message ();
    ASSERT_EQ (queue.memory ().mlock (), 0) << join::lastError.message ();

    const ShmMem::Mpsc::Queue <uint64_t>& cqueue = queue;
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
