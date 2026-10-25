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

// libraries.
#include <gtest/gtest.h>

// C++.
#include <cstring>
#include <vector>

using join::BumpPlain;
using join::HeapMem;

/**
 * @brief test move.
 */
TEST (HeapBump, move)
{
    HeapMem::BumpAllocator<64, BumpPlain> plain1;
    HeapMem::BumpAllocator<64, BumpPlain> plain2;

    char* p1 = static_cast<char*> (plain1.allocate (48));
    ASSERT_NE (p1, nullptr);
    std::memset (p1, 0x5A, 48);
    ASSERT_NE (plain1.allocate (48), nullptr);
    ASSERT_NE (plain2.allocate (48), nullptr);
    ASSERT_NE (plain2.allocate (48), nullptr);

    plain2 = std::move (plain1);

    ASSERT_FALSE (plain1.hasBackend ());
    ASSERT_EQ (plain1.allocate (1), nullptr);
    ASSERT_TRUE (plain2.hasBackend ());
    ASSERT_EQ (p1[47], 0x5A);

    HeapMem::BumpAllocator<64, BumpPlain> plain3 (std::move (plain2));

    ASSERT_FALSE (plain2.hasBackend ());
    ASSERT_TRUE (plain3.hasBackend ());
    ASSERT_EQ (p1[0], 0x5A);
}

/**
 * @brief test allocate.
 */
TEST (HeapBump, allocate)
{
    HeapMem::BumpAllocator<64> allocator;

    ASSERT_NE (allocator.allocate (64), nullptr);
    ASSERT_EQ (allocator.allocate (1), nullptr);

    HeapMem::BumpAllocator<64, BumpPlain> plain;

    std::vector<char*> blocks;
    for (int i = 0; i < 64; ++i)
    {
        char* p = static_cast<char*> (plain.allocate (100));
        ASSERT_NE (p, nullptr);
        std::memset (p, i, 100);
        blocks.push_back (p);
    }

    for (size_t i = 0; i < blocks.size (); ++i)
    {
        ASSERT_EQ (blocks[i][0], static_cast<char> (i));
        ASSERT_EQ (blocks[i][99], static_cast<char> (i));
    }

    char* aligned = static_cast<char*> (plain.allocate (1, 4096));
    ASSERT_NE (aligned, nullptr);
    ASSERT_EQ (reinterpret_cast<uintptr_t> (aligned) % 4096, 0);

    ASSERT_EQ (plain.allocate (SIZE_MAX), nullptr);
    ASSERT_EQ (plain.allocate (SIZE_MAX / 2), nullptr);
}

/**
 * @brief test reset.
 */
TEST (HeapBump, reset)
{
    HeapMem::BumpAllocator<64, BumpPlain> plain;

    ASSERT_NE (plain.allocate (64), nullptr);
    void* p1 = plain.allocate (1);
    ASSERT_NE (p1, nullptr);

    plain.reset ();
    ASSERT_EQ (plain.used (), 0);
    ASSERT_EQ (plain.allocate (1), p1);
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
