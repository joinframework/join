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

/**
 * @brief test default construction.
 */
TEST (Thread, defaultConstruct)
{
    Thread th;
    EXPECT_FALSE (th.joinable ());
}

/**
 * @brief test move construction.
 */
TEST (Thread, moveConstruct)
{
    Thread th1;
    EXPECT_FALSE (th1.joinable ());
    Thread th2 (std::move (th1));
    EXPECT_FALSE (th2.joinable ());
    Thread th3 ([](){std::this_thread::sleep_for (10ms);});
    EXPECT_TRUE (th3.joinable ());
    Thread th4 (std::move (th3));
    EXPECT_FALSE (th3.joinable ());
    EXPECT_TRUE (th4.joinable ());
    th4.join ();
}

/**
 * @brief test move assignment.
 */
TEST (Thread, moveAssign)
{
    Thread th1, th2, th3, th4;
    EXPECT_FALSE (th1.joinable ());
    th2 = std::move (th1);
    EXPECT_FALSE (th2.joinable ());
    th3 = Thread ([](){std::this_thread::sleep_for (10ms);});
    EXPECT_TRUE (th3.joinable ());
    th4 = std::move (th3);
    EXPECT_FALSE (th3.joinable ());
    EXPECT_TRUE (th4.joinable ());
    th4.join ();
}

/**
 * @brief test joinable.
 */
TEST (Thread, joinable)
{
    Thread th ([](){std::this_thread::sleep_for (10ms);});
    EXPECT_TRUE (th.joinable ());
    th.join();
    EXPECT_FALSE (th.joinable ());
}

/**
 * @brief test running.
 */
TEST (Thread, running)
{
    Thread th ([](){std::this_thread::sleep_for (10ms);});
    EXPECT_TRUE (th.running ());
    std::this_thread::sleep_for (15ms);
    EXPECT_FALSE (th.running ());
    th.join();
    EXPECT_FALSE (th.running ());
}

/**
 * @brief test tryJoin.
 */
TEST (Thread, tryJoin)
{
    Thread th ([](){std::this_thread::sleep_for (10ms);});
    EXPECT_FALSE (th.tryJoin ());
    std::this_thread::sleep_for (15ms);
    EXPECT_TRUE (th.tryJoin ());
    th.join();
    EXPECT_TRUE (th.tryJoin ());
}

/**
 * @brief test cancel.
 */
TEST (Thread, cancel)
{
    Thread th ([](){std::this_thread::sleep_for (100ms);});
    EXPECT_TRUE (th.joinable ());
    th.cancel ();
    EXPECT_FALSE (th.joinable ());
}

/**
 * @brief test swap.
 */
TEST (Thread, swap)
{
    Thread th1, th2 ([](){std::this_thread::sleep_for (10ms);});;
    EXPECT_FALSE (th1.joinable ());
    EXPECT_TRUE (th2.joinable ());
    th1.swap (th2);
    EXPECT_TRUE (th1.joinable ());
    EXPECT_FALSE (th2.joinable ());
    th1.join ();
}

/**
 * @brief test handle.
 */
TEST (Thread, handle)
{
    Thread th ([](){std::this_thread::sleep_for (10ms);});;
    EXPECT_TRUE (th.joinable ());
    EXPECT_NO_THROW (th.handle ());
    th.join ();
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
   testing::InitGoogleTest (&argc, argv);
   return RUN_ALL_TESTS ();
}
