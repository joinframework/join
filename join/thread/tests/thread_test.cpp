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
#include <join/thread.hpp>

// Libraries.
#include <gtest/gtest.h>

// C++.
#include <thread>

using join::Thread;

using namespace std::chrono_literals;

std::chrono::milliseconds delay = 100ms;

/**
 * @brief task to execute.
 */
void task ()
{
    std::this_thread::sleep_for (delay);
}

/**
 * @brief test default construction.
 */
TEST (Thread, defaultConstruct)
{
    Thread th;
    ASSERT_FALSE (th.joinable ());
    ASSERT_FALSE (th.running ());
}

/**
 * @brief test move construction.
 */
TEST (Thread, moveConstruct)
{
    Thread th1;
    ASSERT_FALSE (th1.joinable ());
    ASSERT_FALSE (th1.running ());

    Thread th2 (std::move (th1));
    ASSERT_FALSE (th2.joinable ());
    ASSERT_FALSE (th2.running ());

    Thread th3 (task);
    ASSERT_TRUE (th3.joinable ());
    ASSERT_TRUE (th3.running ());

    Thread th4 (std::move (th3));
    ASSERT_TRUE (th4.joinable ());
    ASSERT_TRUE (th4.running ());
    ASSERT_FALSE (th3.joinable ());
    ASSERT_FALSE (th3.running ());

    ASSERT_TRUE (th4.cancel ());
    ASSERT_FALSE (th4.joinable ());
}

/**
 * @brief test move assignment.
 */
TEST (Thread, moveAssign)
{
    Thread th1, th2, th3, th4;
    ASSERT_FALSE (th1.joinable ());
    ASSERT_FALSE (th1.running ());

    th2 = std::move (th1);
    ASSERT_FALSE (th2.joinable ());
    ASSERT_FALSE (th2.running ());

    th3 = Thread (task);
    ASSERT_TRUE (th3.joinable ());
    ASSERT_TRUE (th3.running ());

    th4 = std::move (th3);
    ASSERT_TRUE (th4.joinable ());
    ASSERT_TRUE (th4.running ());
    ASSERT_FALSE (th3.joinable ());
    ASSERT_FALSE (th3.running ());

    ASSERT_TRUE (th4.cancel ());
    ASSERT_FALSE (th4.joinable ());
}

/**
 * @brief test joinable.
 */
TEST (Thread, joinable)
{
    Thread th (task);
    ASSERT_TRUE (th.joinable ());
    std::this_thread::sleep_for (2*delay);
    ASSERT_TRUE (th.joinable ());
    th.join();
    ASSERT_FALSE (th.joinable ());
}

/**
 * @brief test running.
 */
TEST (Thread, running)
{
    Thread th (task);
    ASSERT_TRUE (th.running ());
    std::this_thread::sleep_for (2*delay);
    ASSERT_FALSE (th.running ());
    th.join();
    ASSERT_FALSE (th.running ());
    ASSERT_FALSE (th.joinable ());
}

/**
 * @brief test join.
 */
TEST (Thread, join)
{
    Thread th (task);
    ASSERT_TRUE (th.joinable ());
    th.join();
    ASSERT_FALSE (th.joinable ());
}

/**
 * @brief test tryJoin.
 */
TEST (Thread, tryJoin)
{
    Thread th (task);
    ASSERT_TRUE (th.joinable ());
    ASSERT_FALSE (th.tryJoin ());
    std::this_thread::sleep_for (2*delay);
    ASSERT_TRUE (th.joinable ());
    ASSERT_TRUE (th.tryJoin ());
    ASSERT_FALSE (th.joinable ());
}

/**
 * @brief test cancel.
 */
TEST (Thread, cancel)
{
    Thread th (task);
    ASSERT_TRUE (th.joinable ());
    ASSERT_TRUE (th.cancel ());
    ASSERT_FALSE (th.joinable ());
}

/**
 * @brief test swap.
 */
TEST (Thread, swap)
{
    Thread th1;
    ASSERT_FALSE (th1.joinable ());
    ASSERT_FALSE (th1.running ());

    Thread th2 (task);
    ASSERT_TRUE (th2.joinable ());
    ASSERT_TRUE (th2.running ());

    th1.swap (th2);
    ASSERT_TRUE (th1.joinable ());
    ASSERT_TRUE (th1.running ());
    ASSERT_FALSE (th2.joinable ());
    ASSERT_FALSE (th2.running ());

    ASSERT_TRUE (th1.cancel ());
    ASSERT_FALSE (th1.joinable ());
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
   testing::InitGoogleTest (&argc, argv);
   return RUN_ALL_TESTS ();
}
