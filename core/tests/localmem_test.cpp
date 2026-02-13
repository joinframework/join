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

using join::LocalMem;

TEST (LocalMem, create)
{
    ASSERT_THROW (LocalMem (0), std::system_error);
    ASSERT_THROW (LocalMem (std::numeric_limits <uint64_t>::max ()), std::system_error);
}

TEST (LocalMem, get)
{
    LocalMem mem1 (1024);
    const LocalMem& cmem1 = mem1;

    EXPECT_THROW (mem1.get (std::numeric_limits <uint64_t>::max ()), std::out_of_range);
    EXPECT_THROW (cmem1.get (std::numeric_limits <uint64_t>::max ()), std::out_of_range);

    ASSERT_NE (mem1.get (), nullptr);
    ASSERT_NE (cmem1.get (), nullptr);

    LocalMem mem2 (1024);
    mem2 = std::move (mem1);

    EXPECT_THROW (mem1.get (), std::runtime_error);
    EXPECT_THROW (cmem1.get (), std::runtime_error);
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
