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
#include <join/allocator.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::ShmMem;

/**
 * @brief class used to test the posix shared memory provider.
 */
class PosixAlloc : public ::testing::Test
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

    /// shared memory name.
    static const std::string _name;
};

const std::string PosixAlloc::_name = "/test_shm";

/**
 * @brief test move.
 */
TEST_F (PosixAlloc, move)
{
    ShmMem::Allocator<1, 64> allocator1 (_name);
    ShmMem::Allocator<1, 64> allocator2 (_name);

    void* p1 = allocator1.allocate (64);
    ASSERT_NE (p1, nullptr);

    allocator2 = std::move (allocator1);
    void* p2 = allocator2.allocate (64);
    ASSERT_EQ (p2, nullptr);
}

/**
 * @brief test the allocate method.
 */
TEST_F (PosixAlloc, allocate)
{
    ShmMem::Allocator<1, 64, 128> allocator (_name);

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
 * @brief test the tryAllocate method.
 */
TEST_F (PosixAlloc, tryAllocate)
{
    ShmMem::Allocator<1, 64, 128> allocator (_name);

    void* p1 = allocator.tryAllocate (64);
    ASSERT_NE (p1, nullptr);

    void* p2 = allocator.tryAllocate (64);
    ASSERT_EQ (p2, nullptr);

    void* p3 = allocator.allocate (128);
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
 * @brief test the deallocate method.
 */
TEST_F (PosixAlloc, deallocate)
{
    ShmMem::Allocator<1, 64, 128> allocator (_name);

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

/**
 * @brief test the mbind method.
 */
TEST_F (PosixAlloc, mbind)
{
    ShmMem::Allocator<1, 64> allocator (_name);
    ASSERT_EQ (allocator.mbind (0), 0) << join::lastError.message ();
}

/**
 * @brief test the mlock method.
 */
TEST_F (PosixAlloc, mlock)
{
    ShmMem::Allocator<1, 64> allocator (_name);
    ASSERT_EQ (allocator.mlock (), 0) << join::lastError.message ();
}

/**
 * @brief test concurrent access.
 */
TEST_F (PosixAlloc, benchmark)
{
    ShmMem::Allocator<100, 64> allocator (_name);
    const int iterations = 5000;
    const int numProcesses = 4;

    for (int i = 0; i < numProcesses; ++i)
    {
        pid_t pid = fork ();
        ASSERT_GE (pid, 0);

        if (pid == 0)
        {
            for (int j = 0; j < iterations; ++j)
            {
                void* p = allocator.allocate (64);
                if (p)
                {
                    std::this_thread::yield ();
                    allocator.deallocate (p);
                }
            }
            exit (0);
        }
    }

    int status;
    for (int i = 0; i < numProcesses; ++i)
    {
        wait (&status);
        EXPECT_TRUE (WIFEXITED (status) && WEXITSTATUS (status) == 0);
    }

    std::vector<void*> pointers;
    for (int i = 0; i < 100; ++i)
    {
        void* p = allocator.allocate (64);
        ASSERT_NE (p, nullptr);
        pointers.push_back (p);
    }

    for (void* p : pointers)
    {
        allocator.deallocate (p);
    }
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
