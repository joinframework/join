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
#include <join/utils.hpp>

// Libraries.
#include <gtest/gtest.h>

/**
 * @brief test swap.
 */
TEST (Utils, swap)
{
    int8_t int8Val = 12;
    EXPECT_EQ (join::swap (int8Val), 12);

    uint8_t uint8Val = 12;
    EXPECT_EQ (join::swap (uint8Val), 12);

    int16_t int16Val = 12;
    EXPECT_EQ (join::swap (int16Val), 3072);

    uint16_t uint16Val = 12;
    EXPECT_EQ (join::swap (uint16Val), 3072);

    int32_t int32Val = 12;
    EXPECT_EQ (join::swap (int32Val), 201326592);

    uint32_t uint32Val = 12;
    EXPECT_EQ (join::swap (uint32Val), 201326592);

    int64_t int64Val = 12;
    EXPECT_EQ (join::swap (int64Val), 864691128455135232);

    uint64_t uint64Val = 12;
    EXPECT_EQ (join::swap (uint64Val), 864691128455135232);

    float floatVal = 12.0;
    EXPECT_FLOAT_EQ (join::swap (floatVal), 2.305e-41);

    double doubleVal = 12.0;
    EXPECT_DOUBLE_EQ (join::swap (doubleVal), 5.09085e-320);
}

/**
 * @brief Test randomize.
 */
TEST (Utils, randomize)
{
    ASSERT_GT (join::randomize <int> (), 0);
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
