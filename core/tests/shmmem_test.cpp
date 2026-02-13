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

using join::ShmMem;

/**
 * @brief class used to test the posix shared memory provider.
 */
class PosixMem : public ::testing::Test
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

const std::string PosixMem::_name = "/test_shm";

TEST_F (PosixMem, create)
{
    ASSERT_THROW (ShmMem (0, _name), std::system_error);
    ASSERT_THROW (ShmMem (1024, ""), std::system_error);
    ASSERT_THROW (ShmMem (std::numeric_limits <uint64_t>::max (), _name), std::system_error);
}

TEST_F (PosixMem, get)
{
    ShmMem mem1 (1024, _name);
    const ShmMem& cmem1 = mem1;

    EXPECT_THROW (mem1.get (std::numeric_limits <uint64_t>::max ()), std::out_of_range);
    EXPECT_THROW (cmem1.get (std::numeric_limits <uint64_t>::max ()), std::out_of_range);

    ASSERT_NE (mem1.get (), nullptr);
    ASSERT_NE (cmem1.get (), nullptr);

    ShmMem mem2 (1024, _name);
    mem2 = std::move (mem1);

    EXPECT_THROW (mem1.get (), std::runtime_error);
    EXPECT_THROW (cmem1.get (), std::runtime_error);
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
