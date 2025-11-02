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
#include <join/shared.hpp>
#include <join/utils.hpp>

// Libraries.
#include <gtest/gtest.h>

// C++.
#include <algorithm>

using namespace std::chrono_literals;

using join::SharedMemory;

/**
 * @brief class used to test the single producer single consumer ring buffer.
 */
class SharedMem : public ::testing::Test
{
protected:
    /**
     * @brief set up the test fixture.
     */
    void SetUp () override
    {
        ASSERT_EQ (SharedMemory::unlink (_name), 0) << join::lastError.message ();
    }

    /**
     * @brief tear down the test fixture.
     */
    void TearDown () override
    {
        ASSERT_EQ (SharedMemory::unlink (_name), 0) << join::lastError.message ();
    }

    /// shared memory segment name.
    static const std::string _name;
};

const std::string SharedMem::_name = "/test_shm";

TEST_F (SharedMem, open)
{
    SharedMemory shm (_name, 1024);

    ASSERT_THROW (SharedMemory (_name, std::numeric_limits <uint64_t>::max ()), std::overflow_error);
    ASSERT_EQ (shm.size (), 1024);
    ASSERT_FALSE (shm.opened ());
    ASSERT_THROW (shm.get (std::numeric_limits <uint64_t>::max ()), std::out_of_range);
    ASSERT_EQ (shm.get (), nullptr);
    ASSERT_EQ (shm.open (), 0) << join::lastError.message ();
    ASSERT_EQ (shm.size (), 1024);
    ASSERT_NE (shm.get (), nullptr);
    ASSERT_TRUE (shm.opened ());
    ASSERT_EQ (shm.open (), -1);
    ASSERT_TRUE (shm.opened ());
    shm.close ();
    ASSERT_FALSE (shm.opened ());
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
