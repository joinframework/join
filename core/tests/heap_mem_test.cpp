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

using join::HeapMem;

/**
 * @brief test create.
 */
TEST (HeapMem, create)
{
    ASSERT_THROW (HeapMem (0), std::bad_alloc);
    ASSERT_THROW (HeapMem (UINT64_MAX), std::bad_alloc);

    HeapMem mem1 (join::pageSize ());
    ASSERT_NE (mem1.get (), nullptr);
    HeapMem mem2 (std::move (mem1));
    ASSERT_THROW (mem1.get (), std::runtime_error);
    ASSERT_NE (mem2.get (), nullptr);

    HeapMem mem3 (4 * join::pageSize () + 1);
    ASSERT_NE (mem3.get (), nullptr);
}

/**
 * @brief test empty.
 */
TEST (HeapMem, empty)
{
    HeapMem mem1;
    const HeapMem& cmem1 = mem1;

    ASSERT_FALSE (mem1.mapped ());
    ASSERT_THROW (mem1.get (), std::runtime_error);
    ASSERT_THROW (cmem1.get (), std::runtime_error);

    mem1 = HeapMem (join::pageSize ());
    ASSERT_TRUE (mem1.mapped ());
    ASSERT_NE (mem1.get (), nullptr);
    ASSERT_NE (cmem1.get (), nullptr);
}

/**
 * @brief test get.
 */
TEST (HeapMem, get)
{
    HeapMem mem1 (join::pageSize ());
    const HeapMem& cmem1 = mem1;

    EXPECT_THROW (mem1.get (join::pageSize ()), std::out_of_range);
    EXPECT_THROW (cmem1.get (join::pageSize ()), std::out_of_range);

    ASSERT_EQ (static_cast<char*> (mem1.get (1)), static_cast<char*> (mem1.get ()) + 1);
    ASSERT_EQ (static_cast<const char*> (cmem1.get (1)), static_cast<const char*> (cmem1.get ()) + 1);

    HeapMem mem2 (join::pageSize ());
    mem2 = std::move (mem1);

    EXPECT_THROW (mem1.get (), std::runtime_error);
    EXPECT_THROW (cmem1.get (), std::runtime_error);
    ASSERT_NE (mem2.get (), nullptr);
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
