/**
 * MIT License
 *
 * Copyright (c) 2021 Mathieu Rabine
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
#include <join/threadpool.hpp>
#include <join/utils.hpp>

// Libraries.
#include <gtest/gtest.h>

// C.
#include <unistd.h>

using namespace std::chrono_literals;

using join::ThreadPool;

size_t nthread = std::thread::hardware_concurrency () + 1;

/**
 * @brief test size.
 */
TEST (ThreadPool, size)
{
    ThreadPool pool (8);
    ASSERT_EQ (pool.size (), 8);
}

/**
 * @brief test push.
 */
TEST (ThreadPool, push)
{
    auto elapsed = join::benchmark ([]
    {
        ThreadPool pool;
        for (int i = 0; i < pool.size (); ++i)
        {
            pool.push (usleep, 20000);
        }
    });
    ASSERT_GE (elapsed, 20ms);
}

/**
 * @brief test parallelForEach.
 */
TEST (ThreadPool, parallelForEach)
{
    std::vector <std::function <int (unsigned int)>> todo {usleep, usleep, usleep, usleep, usleep};
    auto elapsed = join::benchmark ([&todo]
    {
        join::parallelForEach (todo.begin (), todo.end (), [] (auto& func)
        {
            func (20000);
        });
    });
    ASSERT_GE (elapsed, 20ms);
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
