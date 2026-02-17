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
#include <join/memory.hpp>

// libraries.
#include <gtest/gtest.h>

// C.
#include <sys/resource.h>

using join::LocalMem;

/**
 * @brief class used to test the posix shared memory provider.
 */
class MmapMem : public ::testing::Test
{
protected:
    /**
     * @brief set up the test fixture.
     */
    void SetUp () override
    {
        getrlimit (RLIMIT_MEMLOCK, &_old);
    }

    /**
     * @brief tear down the test fixture.
     */
    void TearDown () override
    {
        setrlimit (RLIMIT_MEMLOCK, &_old);
    }

    /// rlimit.
    rlimit _old {};
};

TEST_F (MmapMem, create)
{
    ASSERT_THROW (LocalMem (0), std::system_error);
    ASSERT_THROW (LocalMem (std::numeric_limits <uint64_t>::max ()), std::system_error);

    LocalMem mem1 (4096);
    ASSERT_NE (mem1.get (), nullptr);
    LocalMem mem2 (std::move (mem1));
    ASSERT_THROW (mem1.get (), std::runtime_error);
    ASSERT_NE (mem2.get (), nullptr);
}

TEST_F (MmapMem, get)
{
    LocalMem mem1 (4096);
    const LocalMem& cmem1 = mem1;

    EXPECT_THROW (mem1.get (std::numeric_limits <uint64_t>::max ()), std::out_of_range);
    EXPECT_THROW (cmem1.get (std::numeric_limits <uint64_t>::max ()), std::out_of_range);

    ASSERT_NE (mem1.get (), nullptr);
    ASSERT_NE (cmem1.get (), nullptr);

    LocalMem mem2 (4096);
    mem2 = std::move (mem1);

    EXPECT_THROW (mem1.get (), std::runtime_error);
    EXPECT_THROW (cmem1.get (), std::runtime_error);
}

TEST_F (MmapMem, mbind)
{
    LocalMem mem (4096);

    ASSERT_EQ (mem.mbind (0), 0) << join::lastError.message ();
    ASSERT_EQ (join::mbind (nullptr, 4096, 0), -1);
    ASSERT_EQ (join::mbind (mem.get (), 4096, 9999), -1);
}

TEST_F (MmapMem, mlock)
{
    LocalMem mem (4096);

    ASSERT_EQ (mem.mlock (), 0) << join::lastError.message ();
    ASSERT_EQ (join::mlock (nullptr, 4096), -1);
    rlimit zero {0, 0};
    setrlimit (RLIMIT_MEMLOCK, &zero);
    EXPECT_EQ (join::mlock (mem.get (), 8192), -1);
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
