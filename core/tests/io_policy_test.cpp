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
using join::has_sqpoll;
using join::has_cq_entries;
using join::has_sq_thread_idle;
using join::has_sq_thread_cpu;
using join::has_spin;
using join::is_default;
using join::spin_ns;
using join::has_yield_ns;
using join::yield_ns;
using join::has_sleep_ns;
using join::sleep_ns;
using join::has_tick_ns;
using join::tick_ns;

/**
 * @brief policy pinning the submission queue thread.
 */
struct PinnedPolicy : IoSqpollPolicy
{
    static constexpr uint32_t sqThreadCpu = 0;
};

/**
 * @brief Test has_sqpoll trait.
 */
TEST (IoPolicy, has_sqpoll)
{
    ASSERT_FALSE (has_sqpoll<IoDefaultPolicy>::value);
    ASSERT_FALSE (has_sqpoll<IoHybridPolicy>::value);
    ASSERT_TRUE (has_sqpoll<IoSqpollPolicy>::value);
}

/**
 * @brief Test has_cq_entries trait.
 */
TEST (IoPolicy, has_cq_entries)
{
    ASSERT_FALSE (has_cq_entries<IoDefaultPolicy>::value);
    ASSERT_FALSE (has_cq_entries<IoHybridPolicy>::value);
    ASSERT_FALSE (has_cq_entries<IoSqpollPolicy>::value);
}

/**
 * @brief Test has_sq_thread_idle trait.
 */
TEST (IoPolicy, has_sq_thread_idle)
{
    ASSERT_FALSE (has_sq_thread_idle<IoDefaultPolicy>::value);
    ASSERT_FALSE (has_sq_thread_idle<IoHybridPolicy>::value);
    ASSERT_TRUE (has_sq_thread_idle<IoSqpollPolicy>::value);
}

/**
 * @brief Test has_sq_thread_cpu trait.
 */
TEST (IoPolicy, has_sq_thread_cpu)
{
    ASSERT_FALSE (has_sq_thread_cpu<IoDefaultPolicy>::value);
    ASSERT_FALSE (has_sq_thread_cpu<IoHybridPolicy>::value);
    ASSERT_FALSE (has_sq_thread_cpu<IoSqpollPolicy>::value);
    ASSERT_TRUE (has_sq_thread_cpu<PinnedPolicy>::value);
}

/**
 * @brief Test has_spin trait.
 */
TEST (IoPolicy, has_spin)
{
    ASSERT_FALSE (has_spin<IoDefaultPolicy>::value);
    ASSERT_TRUE (has_spin<IoHybridPolicy>::value);
    ASSERT_TRUE (has_spin<IoSqpollPolicy>::value);
}

/**
 * @brief Test is_default trait.
 */
TEST (IoPolicy, is_default)
{
    ASSERT_TRUE (is_default<IoDefaultPolicy>::value);
    ASSERT_FALSE (is_default<IoHybridPolicy>::value);
    ASSERT_FALSE (is_default<IoSqpollPolicy>::value);
}

/**
 * @brief Test spin_ns trait.
 */
TEST (IoPolicy, spin_ns)
{
    ASSERT_EQ (spin_ns<IoDefaultPolicy>::value, 10'000u);
    ASSERT_EQ (spin_ns<IoHybridPolicy>::value, 10'000u);
    ASSERT_EQ (spin_ns<IoSqpollPolicy>::value, 10'000u);
}

/**
 * @brief Test has_yield_ns trait.
 */
TEST (IoPolicy, has_yield_ns)
{
    ASSERT_FALSE (has_yield_ns<IoDefaultPolicy>::value);
    ASSERT_TRUE (has_yield_ns<IoHybridPolicy>::value);
    ASSERT_TRUE (has_yield_ns<IoSqpollPolicy>::value);
}

/**
 * @brief Test yield_ns trait.
 */
TEST (IoPolicy, yield_ns)
{
    ASSERT_EQ (yield_ns<IoDefaultPolicy>::value, 100'000u);
    ASSERT_EQ (yield_ns<IoHybridPolicy>::value, 100'000u);
    ASSERT_EQ (yield_ns<IoSqpollPolicy>::value, 100'000u);
}

/**
 * @brief Test has_sleep_ns trait.
 */
TEST (IoPolicy, has_sleep_ns)
{
    ASSERT_FALSE (has_sleep_ns<IoDefaultPolicy>::value);
    ASSERT_TRUE (has_sleep_ns<IoHybridPolicy>::value);
    ASSERT_TRUE (has_sleep_ns<IoSqpollPolicy>::value);
}

/**
 * @brief Test sleep_ns trait.
 */
TEST (IoPolicy, sleep_ns)
{
    ASSERT_EQ (sleep_ns<IoDefaultPolicy>::value, 0u);
    ASSERT_EQ (sleep_ns<IoHybridPolicy>::value, 0u);
    ASSERT_EQ (sleep_ns<IoSqpollPolicy>::value, 0u);
}

/**
 * @brief Test has_tick_ns trait.
 */
TEST (IoPolicy, has_tick_ns)
{
    ASSERT_FALSE (has_tick_ns<IoDefaultPolicy>::value);
    ASSERT_TRUE (has_tick_ns<IoHybridPolicy>::value);
    ASSERT_TRUE (has_tick_ns<IoSqpollPolicy>::value);
}

/**
 * @brief Test tick_ns trait.
 */
TEST (IoPolicy, tick_ns)
{
    ASSERT_EQ (tick_ns<IoDefaultPolicy>::value, 1'000u);
    ASSERT_EQ (tick_ns<IoHybridPolicy>::value, 100u);
    ASSERT_EQ (tick_ns<IoSqpollPolicy>::value, 100u);
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
