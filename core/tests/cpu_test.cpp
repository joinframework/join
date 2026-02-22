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
#include <join/cpu.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::LogicalCpu;
using join::PhysicalCore;
using join::NumaNode;
using join::CpuTopology;

/**
 * @brief test cores.
 */
TEST (CpuTopology, cores)
{
    ASSERT_GE (CpuTopology::instance ()->cores ().size (), 1);

    for (const auto& core : CpuTopology::instance ()->cores ())
    {
        ASSERT_GE (core.primaryThread (), 0);
    }
}

/**
 * @brief test nodes.
 */
TEST (CpuTopology, nodes)
{
    ASSERT_GE (CpuTopology::instance ()->nodes ().size (), 1);
}

/**
 * @brief test dump.
 */
TEST (CpuTopology, dump)
{
    ASSERT_NO_THROW (CpuTopology::instance ()->dump ());
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
   testing::InitGoogleTest (&argc, argv);
   return RUN_ALL_TESTS ();
}
