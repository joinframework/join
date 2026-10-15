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
#include <join/bump_allocator.hpp>
#include <join/statistics.hpp>
#include <join/thread.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::ScopedStats;
using join::BumpPlain;
using join::LocalMem;
using join::Thread;
using join::Rdtsc;

/**
 * @brief test move.
 */
TEST (LocalBump, move)
{
    LocalMem::BumpAllocator<64> allocator1;
    LocalMem::BumpAllocator<64> allocator2;

    ASSERT_NE (allocator1.allocate (32), nullptr);

    allocator2 = std::move (allocator1);

    ASSERT_FALSE (allocator1.hasBackend ());
    ASSERT_EQ (allocator1.used (), 0);
    ASSERT_EQ (allocator1.allocate (1), nullptr);
    ASSERT_EQ (allocator2.used (), 32);

    LocalMem::BumpAllocator<64> allocator3 (std::move (allocator2));

    ASSERT_FALSE (allocator2.hasBackend ());
    ASSERT_EQ (allocator2.used (), 0);
    ASSERT_TRUE (allocator3.hasBackend ());
    ASSERT_EQ (allocator3.used (), 32);
    ASSERT_NE (allocator3.allocate (32), nullptr);
    ASSERT_EQ (allocator3.allocate (1), nullptr);

    LocalMem::BumpAllocator<64, BumpPlain> plain1;
    LocalMem::BumpAllocator<64, BumpPlain> plain2;

    ASSERT_NE (plain1.allocate (32), nullptr);

    plain2 = std::move (plain1);

    ASSERT_FALSE (plain1.hasBackend ());
    ASSERT_EQ (plain1.used (), 0);
    ASSERT_EQ (plain1.allocate (1), nullptr);
    ASSERT_EQ (plain2.used (), 32);

    LocalMem::BumpAllocator<64, BumpPlain> plain3 (std::move (plain2));

    ASSERT_FALSE (plain2.hasBackend ());
    ASSERT_EQ (plain2.used (), 0);
    ASSERT_TRUE (plain3.hasBackend ());
    ASSERT_EQ (plain3.used (), 32);
    ASSERT_NE (plain3.allocate (32), nullptr);
    ASSERT_EQ (plain3.allocate (1), nullptr);
}

/**
 * @brief test allocate.
 */
TEST (LocalBump, allocate)
{
    LocalMem::BumpAllocator<256> allocator;

    char* p1 = static_cast<char*> (allocator.allocate (1));
    ASSERT_NE (p1, nullptr);

    char* p2 = static_cast<char*> (allocator.allocate (24));
    ASSERT_NE (p2, nullptr);
    ASSERT_EQ (reinterpret_cast<uintptr_t> (p2) % 8, 0);
    ASSERT_EQ (p2 - p1, 8);

    char* p3 = static_cast<char*> (allocator.allocate (3));
    ASSERT_NE (p3, nullptr);
    ASSERT_EQ (p3 - p2, 24);

    char* p4 = static_cast<char*> (allocator.allocate (64));
    ASSERT_NE (p4, nullptr);
    ASSERT_EQ (reinterpret_cast<uintptr_t> (p4) % 16, 0);
    ASSERT_EQ (p4 - p1, 48);

    ASSERT_EQ (allocator.allocate (SIZE_MAX), nullptr);

    ASSERT_NE (allocator.allocate (144), nullptr);
    ASSERT_EQ (allocator.allocate (1), nullptr);

    LocalMem::BumpAllocator<256> aligned;

    ASSERT_EQ (aligned.allocate (8, 0), nullptr);
    ASSERT_EQ (aligned.allocate (8, 3), nullptr);

    char* a1 = static_cast<char*> (aligned.allocate (1, 1));
    ASSERT_NE (a1, nullptr);

    char* a2 = static_cast<char*> (aligned.allocate (1, 64));
    ASSERT_NE (a2, nullptr);
    ASSERT_EQ (reinterpret_cast<uintptr_t> (a2) % 64, 0);
    ASSERT_EQ (a2 - a1, 64);

    ASSERT_EQ (aligned.allocate (1, 4096), nullptr);

    LocalMem::BumpAllocator<256, BumpPlain> plain;

    char* u1 = static_cast<char*> (plain.allocate (1));
    ASSERT_NE (u1, nullptr);

    char* u2 = static_cast<char*> (plain.allocate (24));
    ASSERT_NE (u2, nullptr);
    ASSERT_EQ (u2 - u1, 8);

    ASSERT_EQ (plain.allocate (8, 3), nullptr);
    ASSERT_EQ (plain.allocate (SIZE_MAX), nullptr);

    char* u3 = static_cast<char*> (plain.allocate (1, 64));
    ASSERT_NE (u3, nullptr);
    ASSERT_EQ (u3 - u1, 64);

    ASSERT_NE (plain.allocate (176), nullptr);
    ASSERT_EQ (plain.allocate (1), nullptr);
}

/**
 * @brief test reset.
 */
TEST (LocalBump, reset)
{
    LocalMem::BumpAllocator<64> allocator;

    void* p1 = allocator.allocate (64);
    ASSERT_NE (p1, nullptr);
    ASSERT_EQ (allocator.allocate (1), nullptr);

    allocator.reset ();
    ASSERT_EQ (allocator.used (), 0);
    ASSERT_EQ (allocator.allocate (64), p1);

    LocalMem::BumpAllocator<64, BumpPlain> plain;

    void* u1 = plain.allocate (64);
    ASSERT_NE (u1, nullptr);
    ASSERT_EQ (plain.allocate (1), nullptr);

    plain.reset ();
    ASSERT_EQ (plain.used (), 0);
    ASSERT_EQ (plain.allocate (64), u1);
}

/**
 * @brief test capacity.
 */
TEST (LocalBump, capacity)
{
    ASSERT_EQ (LocalMem::BumpAllocator<128>::capacity (), 128);
    ASSERT_EQ ((LocalMem::BumpAllocator<128, BumpPlain>::capacity ()), 128);
}

/**
 * @brief test used.
 */
TEST (LocalBump, used)
{
    LocalMem::BumpAllocator<64> allocator;
    ASSERT_EQ (allocator.used (), 0);

    ASSERT_NE (allocator.allocate (1), nullptr);
    ASSERT_EQ (allocator.used (), 1);

    ASSERT_NE (allocator.allocate (8), nullptr);
    ASSERT_EQ (allocator.used (), 16);

    LocalMem::BumpAllocator<64, BumpPlain> plain;
    ASSERT_EQ (plain.used (), 0);

    ASSERT_NE (plain.allocate (1), nullptr);
    ASSERT_EQ (plain.used (), 1);

    ASSERT_NE (plain.allocate (8), nullptr);
    ASSERT_EQ (plain.used (), 16);
}

/**
 * @brief test hasBackend.
 */
TEST (LocalBump, hasBackend)
{
    LocalMem::BumpAllocator<64> allocator;
    ASSERT_TRUE (allocator.hasBackend ());
}

#ifdef JOIN_HAS_NUMA
/**
 * @brief test mbind.
 */
TEST (LocalBump, mbind)
{
    LocalMem::BumpAllocator<64> allocator;
    ASSERT_EQ (allocator.mbind (0), 0) << join::lastError.message ();
}
#endif

/**
 * @brief test mlock.
 */
TEST (LocalBump, mlock)
{
    LocalMem::BumpAllocator<64> allocator;
    ASSERT_EQ (allocator.mlock (), 0) << join::lastError.message ();
}

/**
 * @brief test concurrent access.
 */
TEST (LocalBump, benchmark)
{
    const int iterations = 100000;
    const int numThreads = 4;

    LocalMem::BumpAllocator<numThreads * iterations * 16> allocator;
    allocator.mlock ();

    std::vector<Thread> threads;
    for (int i = 0; i < numThreads - 1; ++i)
    {
        threads.emplace_back ([&] () {
            for (int j = 0; j < iterations; ++j)
            {
                EXPECT_NE (allocator.allocate (16), nullptr);
            }
        });
    }

    Rdtsc::Stats aStats ("allocate");

    for (int i = 0; i < iterations; ++i)
    {
        void* p = nullptr;
        {
            ScopedStats<Rdtsc::Stats> guard (aStats);
            p = allocator.allocate (16);
        }
        EXPECT_NE (p, nullptr);
    }

    for (auto& t : threads)
    {
        t.join ();
    }

    EXPECT_EQ (allocator.used (), allocator.capacity ());
    EXPECT_EQ (allocator.allocate (1), nullptr);

    LocalMem::BumpAllocator<iterations * 16, BumpPlain> plain;
    plain.mlock ();

    Rdtsc::Stats pStats ("allocate plain");

    for (int i = 0; i < iterations; ++i)
    {
        void* p = nullptr;
        {
            ScopedStats<Rdtsc::Stats> guard (pStats);
            p = plain.allocate (16);
        }
        EXPECT_NE (p, nullptr);
    }

    std::cout << join::statsHeader << "\n";
    std::cout << join::kops << join::nsec << std::fixed << std::setprecision (0) << aStats << "\n";
    std::cout << join::kops << join::nsec << std::fixed << std::setprecision (0) << pStats << "\n";
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    Rdtsc::calibrate ();
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
