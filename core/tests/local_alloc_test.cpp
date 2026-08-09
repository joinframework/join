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
#include <join/statistics.hpp>
#include <join/allocator.hpp>
#include <join/thread.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::ScopedStats;
using join::Rdtsc;
using join::LocalMem;
using join::Thread;

/**
 * @brief test move.
 */
TEST (LocalAlloc, move)
{
    LocalMem::Allocator<1, 64> allocator1;
    LocalMem::Allocator<1, 64> allocator2;

    void* p1 = allocator1.allocate (64);
    ASSERT_NE (p1, nullptr);

    allocator2 = std::move (allocator1);

    void* p2 = allocator2.allocate (64);
    ASSERT_EQ (p2, nullptr);

    LocalMem::Allocator<1, 64> allocator3 (std::move (allocator2));

    void* p3 = allocator3.allocate (64);
    ASSERT_EQ (p3, nullptr);
}

/**
 * @brief test allocate.
 */
TEST (LocalAlloc, allocate)
{
    LocalMem::Allocator<1, 64, 128> allocator;

    void* p1 = allocator.allocate (64);
    ASSERT_NE (p1, nullptr);

    void* p2 = allocator.allocate (64);
    ASSERT_NE (p2, nullptr);
    ASSERT_NE (p1, p2);

    void* p3 = allocator.allocate (128);
    ASSERT_EQ (p3, nullptr);

    void* p4 = allocator.allocate (64);
    ASSERT_EQ (p4, nullptr);

    allocator.deallocate (p2);
    p3 = allocator.allocate (128);
    ASSERT_NE (p3, nullptr);
    ASSERT_EQ (p3, p2);

    allocator.deallocate (p1);
    allocator.deallocate (p3);

    void* p5 = allocator.allocate (256);
    EXPECT_EQ (p5, nullptr);
}

/**
 * @brief test tryAllocate.
 */
TEST (LocalAlloc, tryAllocate)
{
    LocalMem::Allocator<1, 64, 128> allocator;

    void* p1 = allocator.tryAllocate (64);
    ASSERT_NE (p1, nullptr);

    void* p2 = allocator.tryAllocate (64);
    ASSERT_EQ (p2, nullptr);

    void* p3 = allocator.tryAllocate (128);
    ASSERT_NE (p3, nullptr);
    ASSERT_NE (p1, p3);

    allocator.deallocate (p1);
    void* p4 = allocator.tryAllocate (128);
    ASSERT_EQ (p4, nullptr);

    allocator.deallocate (p3);
    p4 = allocator.tryAllocate (128);
    ASSERT_NE (p4, nullptr);
    ASSERT_EQ (p4, p3);

    allocator.deallocate (p4);

    void* p5 = allocator.tryAllocate (256);
    EXPECT_EQ (p5, nullptr);
}

/**
 * @brief test deallocate.
 */
TEST (LocalAlloc, deallocate)
{
    LocalMem::Allocator<1, 64, 128> allocator;

    void* p1 = allocator.allocate (64);
    ASSERT_NE (p1, nullptr);

    void* p2 = allocator.allocate (128);
    ASSERT_NE (p2, nullptr);

    ASSERT_NO_THROW (allocator.deallocate (nullptr));
    ASSERT_NO_THROW (allocator.deallocate (p2));
    ASSERT_NO_THROW (allocator.deallocate (p1));

    int dummy;
    void* dummyPtr = &dummy;
    EXPECT_NO_THROW (allocator.deallocate (dummyPtr));
}

#ifdef JOIN_HAS_NUMA
/**
 * @brief test mbind.
 */
TEST (LocalAlloc, mbind)
{
    LocalMem::Allocator<1, 64> allocator;
    ASSERT_EQ (allocator.mbind (0), 0) << join::lastError.message ();
}
#endif

/**
 * @brief test mlock.
 */
TEST (LocalAlloc, mlock)
{
    LocalMem::Allocator<1, 64> allocator;
    ASSERT_EQ (allocator.mlock (), 0) << join::lastError.message ();
}

/**
 * @brief test concurrent access.
 */
TEST (LocalAlloc, benchmark)
{
    const int iterations = 100000;
    const int numThreads = 4;

    join::LocalMem::Allocator<iterations, 64> allocator;
    allocator.mlock ();

    std::vector<Thread> threads;
    for (int i = 0; i < numThreads - 1; ++i)
    {
        threads.emplace_back ([&] () {
            for (int j = 0; j < iterations; ++j)
            {
                void* p = allocator.allocate (64);
                if (p)
                {
                    allocator.deallocate (p);
                }
            }
        });
    }

    Rdtsc::Stats aStats ("allocate");
    Rdtsc::Stats dStats ("deallocate");

    for (int i = 0; i < iterations; ++i)
    {
        void* p = nullptr;
        {
            ScopedStats<Rdtsc::Stats> guard (aStats);
            p = allocator.allocate (64);
        }
        EXPECT_NE (p, nullptr);
        if (p)
        {
            ScopedStats<Rdtsc::Stats> guard (dStats);
            allocator.deallocate (p);
        }
    }

    for (auto& t : threads)
    {
        t.join ();
    }

    std::cout << join::statsHeader << "\n";
    std::cout << join::mops << join::usec << std::fixed << std::setprecision (3) << aStats << "\n";
    std::cout << join::mops << join::usec << std::fixed << std::setprecision (3) << dStats << "\n";
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
