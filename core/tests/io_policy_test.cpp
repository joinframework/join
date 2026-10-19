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

/**
 * @brief policy pinning the submission queue thread.
 */
struct PinnedPolicy : join::IoSqpollPolicy
{
    static constexpr uint32_t sqThreadCpu = 0;
};

/**
 * @brief Test has_sqpoll_v trait.
 */
TEST (IoPolicy, has_sqpoll_v)
{
    ASSERT_FALSE (join::has_sqpoll_v<join::IoDefaultPolicy>);
    ASSERT_FALSE (join::has_sqpoll_v<join::IoHybridPolicy>);
    ASSERT_TRUE (join::has_sqpoll_v<join::IoSqpollPolicy>);
}

/**
 * @brief Test has_cq_entries_v trait.
 */
TEST (IoPolicy, has_cq_entries_v)
{
    ASSERT_FALSE (join::has_cq_entries_v<join::IoDefaultPolicy>);
    ASSERT_FALSE (join::has_cq_entries_v<join::IoHybridPolicy>);
    ASSERT_FALSE (join::has_cq_entries_v<join::IoSqpollPolicy>);
}

/**
 * @brief Test has_sq_thread_idle_v trait.
 */
TEST (IoPolicy, has_sq_thread_idle_v)
{
    ASSERT_FALSE (join::has_sq_thread_idle_v<join::IoDefaultPolicy>);
    ASSERT_FALSE (join::has_sq_thread_idle_v<join::IoHybridPolicy>);
    ASSERT_TRUE (join::has_sq_thread_idle_v<join::IoSqpollPolicy>);
}

/**
 * @brief Test has_sq_thread_cpu_v trait.
 */
TEST (IoPolicy, has_sq_thread_cpu_v)
{
    ASSERT_FALSE (join::has_sq_thread_cpu_v<join::IoDefaultPolicy>);
    ASSERT_FALSE (join::has_sq_thread_cpu_v<join::IoHybridPolicy>);
    ASSERT_FALSE (join::has_sq_thread_cpu_v<join::IoSqpollPolicy>);
    ASSERT_TRUE (join::has_sq_thread_cpu_v<PinnedPolicy>);
}

/**
 * @brief Test has_spin_v trait.
 */
TEST (IoPolicy, has_spin_v)
{
    ASSERT_FALSE (join::has_spin_v<join::IoDefaultPolicy>);
    ASSERT_TRUE (join::has_spin_v<join::IoHybridPolicy>);
    ASSERT_TRUE (join::has_spin_v<join::IoSqpollPolicy>);
}

/**
 * @brief Test is_default_v trait.
 */
TEST (IoPolicy, is_default_v)
{
    ASSERT_TRUE (join::is_default_v<join::IoDefaultPolicy>);
    ASSERT_FALSE (join::is_default_v<join::IoHybridPolicy>);
    ASSERT_FALSE (join::is_default_v<join::IoSqpollPolicy>);
}

/**
 * @brief Test spin_ns_v trait.
 */
TEST (IoPolicy, spin_ns_v)
{
    ASSERT_EQ (join::spin_ns_v<join::IoDefaultPolicy>, 10'000u);
    ASSERT_EQ (join::spin_ns_v<join::IoHybridPolicy>, 10'000u);
    ASSERT_EQ (join::spin_ns_v<join::IoSqpollPolicy>, 10'000u);
}

/**
 * @brief Test has_yield_ns_v trait.
 */
TEST (IoPolicy, has_yield_ns_v)
{
    ASSERT_FALSE (join::has_yield_ns_v<join::IoDefaultPolicy>);
    ASSERT_TRUE (join::has_yield_ns_v<join::IoHybridPolicy>);
    ASSERT_TRUE (join::has_yield_ns_v<join::IoSqpollPolicy>);
}

/**
 * @brief Test yield_ns_v trait.
 */
TEST (IoPolicy, yield_ns_v)
{
    ASSERT_EQ (join::yield_ns_v<join::IoDefaultPolicy>, 100'000u);
    ASSERT_EQ (join::yield_ns_v<join::IoHybridPolicy>, 100'000u);
    ASSERT_EQ (join::yield_ns_v<join::IoSqpollPolicy>, 100'000u);
}

/**
 * @brief Test has_sleep_ns_v trait.
 */
TEST (IoPolicy, has_sleep_ns_v)
{
    ASSERT_FALSE (join::has_sleep_ns_v<join::IoDefaultPolicy>);
    ASSERT_TRUE (join::has_sleep_ns_v<join::IoHybridPolicy>);
    ASSERT_TRUE (join::has_sleep_ns_v<join::IoSqpollPolicy>);
}

/**
 * @brief Test sleep_ns_v trait.
 */
TEST (IoPolicy, sleep_ns_v)
{
    ASSERT_EQ (join::sleep_ns_v<join::IoDefaultPolicy>, 0u);
    ASSERT_EQ (join::sleep_ns_v<join::IoHybridPolicy>, 0u);
    ASSERT_EQ (join::sleep_ns_v<join::IoSqpollPolicy>, 0u);
}

/**
 * @brief Test has_tick_ns_v trait.
 */
TEST (IoPolicy, has_tick_ns_v)
{
    ASSERT_FALSE (join::has_tick_ns_v<join::IoDefaultPolicy>);
    ASSERT_TRUE (join::has_tick_ns_v<join::IoHybridPolicy>);
    ASSERT_TRUE (join::has_tick_ns_v<join::IoSqpollPolicy>);
}

/**
 * @brief Test tick_ns_v trait.
 */
TEST (IoPolicy, tick_ns_v)
{
    ASSERT_EQ (join::tick_ns_v<join::IoDefaultPolicy>, 1'000u);
    ASSERT_EQ (join::tick_ns_v<join::IoHybridPolicy>, 100u);
    ASSERT_EQ (join::tick_ns_v<join::IoSqpollPolicy>, 100u);
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
