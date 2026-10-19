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
#include <join/io_policy.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::IoDefaultPolicy;
using join::IoHybridPolicy;
using join::IoSqpollPolicy;
using join::has_sqpoll_v;
using join::has_cq_entries_v;
using join::has_sq_thread_idle_v;
using join::has_sq_thread_cpu_v;
using join::has_spin_v;
using join::is_default_v;
using join::spin_ns_v;
using join::has_yield_ns_v;
using join::yield_ns_v;
using join::has_sleep_ns_v;
using join::sleep_ns_v;
using join::has_tick_ns_v;
using join::tick_ns_v;

/**
 * @brief policy pinning the submission queue thread.
 */
struct PinnedPolicy : IoSqpollPolicy
{
    static constexpr uint32_t sqThreadCpu = 0;
};

/**
 * @brief Test has_sqpoll_v trait.
 */
TEST (IoPolicy, has_sqpoll_v)
{
    ASSERT_FALSE (has_sqpoll_v<IoDefaultPolicy>);
    ASSERT_FALSE (has_sqpoll_v<IoHybridPolicy>);
    ASSERT_TRUE (has_sqpoll_v<IoSqpollPolicy>);
}

/**
 * @brief Test has_cq_entries_v trait.
 */
TEST (IoPolicy, has_cq_entries_v)
{
    ASSERT_FALSE (has_cq_entries_v<IoDefaultPolicy>);
    ASSERT_FALSE (has_cq_entries_v<IoHybridPolicy>);
    ASSERT_FALSE (has_cq_entries_v<IoSqpollPolicy>);
}

/**
 * @brief Test has_sq_thread_idle_v trait.
 */
TEST (IoPolicy, has_sq_thread_idle_v)
{
    ASSERT_FALSE (has_sq_thread_idle_v<IoDefaultPolicy>);
    ASSERT_FALSE (has_sq_thread_idle_v<IoHybridPolicy>);
    ASSERT_TRUE (has_sq_thread_idle_v<IoSqpollPolicy>);
}

/**
 * @brief Test has_sq_thread_cpu_v trait.
 */
TEST (IoPolicy, has_sq_thread_cpu_v)
{
    ASSERT_FALSE (has_sq_thread_cpu_v<IoDefaultPolicy>);
    ASSERT_FALSE (has_sq_thread_cpu_v<IoHybridPolicy>);
    ASSERT_FALSE (has_sq_thread_cpu_v<IoSqpollPolicy>);
    ASSERT_TRUE (has_sq_thread_cpu_v<PinnedPolicy>);
}

/**
 * @brief Test has_spin_v trait.
 */
TEST (IoPolicy, has_spin_v)
{
    ASSERT_FALSE (has_spin_v<IoDefaultPolicy>);
    ASSERT_TRUE (has_spin_v<IoHybridPolicy>);
    ASSERT_TRUE (has_spin_v<IoSqpollPolicy>);
}

/**
 * @brief Test is_default_v trait.
 */
TEST (IoPolicy, is_default_v)
{
    ASSERT_TRUE (is_default_v<IoDefaultPolicy>);
    ASSERT_FALSE (is_default_v<IoHybridPolicy>);
    ASSERT_FALSE (is_default_v<IoSqpollPolicy>);
}

/**
 * @brief Test spin_ns_v trait.
 */
TEST (IoPolicy, spin_ns_v)
{
    ASSERT_EQ (spin_ns_v<IoDefaultPolicy>, 10'000u);
    ASSERT_EQ (spin_ns_v<IoHybridPolicy>, 10'000u);
    ASSERT_EQ (spin_ns_v<IoSqpollPolicy>, 10'000u);
}

/**
 * @brief Test has_yield_ns_v trait.
 */
TEST (IoPolicy, has_yield_ns_v)
{
    ASSERT_FALSE (has_yield_ns_v<IoDefaultPolicy>);
    ASSERT_TRUE (has_yield_ns_v<IoHybridPolicy>);
    ASSERT_TRUE (has_yield_ns_v<IoSqpollPolicy>);
}

/**
 * @brief Test yield_ns_v trait.
 */
TEST (IoPolicy, yield_ns_v)
{
    ASSERT_EQ (yield_ns_v<IoDefaultPolicy>, 100'000u);
    ASSERT_EQ (yield_ns_v<IoHybridPolicy>, 100'000u);
    ASSERT_EQ (yield_ns_v<IoSqpollPolicy>, 100'000u);
}

/**
 * @brief Test has_sleep_ns_v trait.
 */
TEST (IoPolicy, has_sleep_ns_v)
{
    ASSERT_FALSE (has_sleep_ns_v<IoDefaultPolicy>);
    ASSERT_TRUE (has_sleep_ns_v<IoHybridPolicy>);
    ASSERT_TRUE (has_sleep_ns_v<IoSqpollPolicy>);
}

/**
 * @brief Test sleep_ns_v trait.
 */
TEST (IoPolicy, sleep_ns_v)
{
    ASSERT_EQ (sleep_ns_v<IoDefaultPolicy>, 0u);
    ASSERT_EQ (sleep_ns_v<IoHybridPolicy>, 0u);
    ASSERT_EQ (sleep_ns_v<IoSqpollPolicy>, 0u);
}

/**
 * @brief Test has_tick_ns_v trait.
 */
TEST (IoPolicy, has_tick_ns_v)
{
    ASSERT_FALSE (has_tick_ns_v<IoDefaultPolicy>);
    ASSERT_TRUE (has_tick_ns_v<IoHybridPolicy>);
    ASSERT_TRUE (has_tick_ns_v<IoSqpollPolicy>);
}

/**
 * @brief Test tick_ns_v trait.
 */
TEST (IoPolicy, tick_ns_v)
{
    ASSERT_EQ (tick_ns_v<IoDefaultPolicy>, 1'000u);
    ASSERT_EQ (tick_ns_v<IoHybridPolicy>, 100u);
    ASSERT_EQ (tick_ns_v<IoSqpollPolicy>, 100u);
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
