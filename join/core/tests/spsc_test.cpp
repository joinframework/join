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
using join::Spsc;

/**
 * @brief class used to test the single producer single consumer ring buffer.
 */
class SpscBuffer : public ::testing::Test
{
protected:
    /**
     * @brief set up the test fixture.
     */
    void SetUp () override
    {
        ASSERT_EQ (BasicQueue <Spsc>::unlink (_name), 0) << join::lastError.message ();
    }

    /**
     * @brief tear down the test fixture.
     */
    void TearDown () override
    {
        ASSERT_EQ (BasicQueue <Spsc>::unlink (_name), 0) << join::lastError.message ();
    }

    /// shared memory segment name.
    static const std::string _name;
};

const std::string SpscBuffer::_name = "/test_spsc_shm";

TEST_F (SpscBuffer, open)
{
    Spsc::Producer prod1 (_name, 64, 8), prod2 ("", 64, 8), prod3(_name, 128, 16), prod4 (_name, 1, std::numeric_limits <off_t>::max () - sizeof (join::SharedSegment));
    Spsc::Consumer cons1 (_name, 64, 8), cons2 (_name, 128, 16);

    ASSERT_THROW (Spsc::Consumer (_name, 128, std::numeric_limits <uint64_t>::max ()), std::overflow_error);
    ASSERT_THROW (Spsc::Consumer (_name, 1, std::numeric_limits <off_t>::max ()), std::overflow_error);
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
    ASSERT_EQ (prod4.open (), -1);
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

TEST_F (SpscBuffer, tryPush)
{
    Spsc::Producer prod (_name, 64, 8);
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

TEST_F (SpscBuffer, push)
{
    Spsc::Producer prod (_name, 64, 8);
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

TEST_F (SpscBuffer, timedPush)
{
    Spsc::Producer prod (_name, 64, 8);
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

TEST_F (SpscBuffer, tryPop)
{
    Spsc::Producer prod (_name, 64, 8);
    Spsc::Consumer cons (_name, 64, 8);
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

TEST_F (SpscBuffer, pop)
{
    Spsc::Producer prod (_name, 64, 8);
    Spsc::Consumer cons (_name, 64, 8);
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

TEST_F (SpscBuffer, timedPop)
{
    Spsc::Producer prod (_name, 64, 8);
    Spsc::Consumer cons (_name, 64, 8);
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

TEST_F (SpscBuffer, pushBenchmark)
{
    const uint64_t num = 1000000;
    const uint64_t capacity = 144;
    const uint64_t size = 1472;
    char data[size] = {};

    pid_t child = fork ();
    if (child == 0)
    {
        Semaphore sem (_name);
        Spsc::Consumer cons (_name, size, capacity);
        sem.wait ();
        cons.open ();
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
        cons.close ();
        _exit (0);
    }
    else
    {
        EXPECT_NE (child, -1);
        Semaphore sem (_name);
        Spsc::Producer prod (_name, size, capacity);
        EXPECT_EQ (prod.open (), 0) << join::lastError.message ();
        // pre-fill the buffer.
        for (uint64_t i = 0; i < capacity; ++i)
        {
            while (prod.tryPush (data) == -1)
            {
                std::this_thread::yield ();
            }
        }
        sem.post ();
        for (uint64_t i = 0; i < num; ++i)
        {
            EXPECT_EQ (prod.push (data), 0) << join::lastError.message ();
        }
        prod.close ();
    }

    int status;
    waitpid (child, &status, 0);
    ASSERT_TRUE (WIFEXITED (status));
    ASSERT_EQ (WEXITSTATUS (status), 0);
}

TEST_F (SpscBuffer, timedPushBenchmark)
{
    const uint64_t num = 1000000;
    const uint64_t capacity = 144;
    const uint64_t size = 1472;
    char data[size] = {};

    pid_t child = fork ();
    if (child == 0)
    {
        Semaphore sem (_name);
        Spsc::Consumer cons (_name, size, capacity);
        sem.wait ();
        cons.open ();
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
        cons.close ();
        _exit (0);
    }
    else
    {
        EXPECT_NE (child, -1);
        Semaphore sem (_name);
        Spsc::Producer prod (_name, size, capacity);
        EXPECT_EQ (prod.open (), 0) << join::lastError.message ();
        // pre-fill the buffer.
        for (uint64_t i = 0; i < capacity; ++i)
        {
            while (prod.tryPush (data) == -1)
            {
                std::this_thread::yield ();
            }
        }
        sem.post ();
        for (uint64_t i = 0; i < num; ++i)
        {
            EXPECT_EQ (prod.timedPush (data, 1s), 0) << join::lastError.message ();
        }
        prod.close ();
    }

    int status;
    waitpid (child, &status, 0);
    ASSERT_TRUE (WIFEXITED (status));
    ASSERT_EQ (WEXITSTATUS (status), 0);
}

TEST_F (SpscBuffer, popBenchmark)
{
    const uint64_t num = 1000000;
    const uint64_t capacity = 144;
    const uint64_t size = 1472;
    char data[size] = {};

    pid_t child = fork ();
    if (child == 0)
    {
        Semaphore sem (_name);
        Spsc::Producer prod (_name, size, capacity);
        prod.open ();
        sem.post ();
        for (uint64_t i = 0; i < num; ++i)
        {
            while (prod.tryPush (data) == -1)
            {
                std::this_thread::yield ();
            }
        }
        prod.close ();
        _exit (0);
    }
    else
    {
        EXPECT_NE (child, -1);
        Semaphore sem (_name);
        Spsc::Consumer cons (_name, size, capacity);
        sem.wait ();
        EXPECT_EQ (cons.open (), 0) << join::lastError.message ();
        for (uint64_t i = 0; i < num; ++i)
        {
            EXPECT_EQ (cons.pop (data), 0) << join::lastError.message ();
        }
        cons.close ();
    }

    int status;
    waitpid (child, &status, 0);
    ASSERT_TRUE (WIFEXITED (status));
    ASSERT_EQ (WEXITSTATUS (status), 0);
}

TEST_F (SpscBuffer, timedPopBenchmark)
{
    const uint64_t num = 1000000;
    const uint64_t capacity = 144;
    const uint64_t size = 1472;
    char data[size] = {};

    pid_t child = fork ();
    if (child == 0)
    {
        Semaphore sem (_name);
        Spsc::Producer prod (_name, size, capacity);
        prod.open ();
        sem.post ();
        for (uint64_t i = 0; i < num; ++i)
        {
            while (prod.tryPush (data) == -1)
            {
                std::this_thread::yield ();
            }
        }
        prod.close ();
        _exit (0);
    }
    else
    {
        EXPECT_NE (child, -1);
        Semaphore sem (_name);
        Spsc::Consumer cons (_name, size, capacity);
        sem.wait ();
        EXPECT_EQ (cons.open (), 0) << join::lastError.message ();
        for (uint64_t i = 0; i < num; ++i)
        {
            EXPECT_EQ (cons.timedPop (data, 1s), 0) << join::lastError.message ();
        }
        
        cons.close ();
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
