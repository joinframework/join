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

// C++.
#include <sstream>
#include <thread>

using join::Errc;

using namespace std::chrono_literals;

/**
 * @brief test swap.
 */
TEST (Utils, swap)
{
    int8_t int8Val = 12;
    ASSERT_EQ (join::swap (int8Val), 12);

    uint8_t uint8Val = 12;
    ASSERT_EQ (join::swap (uint8Val), 12);

    int16_t int16Val = 12;
    ASSERT_EQ (join::swap (int16Val), 3072);

    uint16_t uint16Val = 12;
    ASSERT_EQ (join::swap (uint16Val), 3072);

    int32_t int32Val = 12;
    ASSERT_EQ (join::swap (int32Val), 201326592);

    uint32_t uint32Val = 12;
    ASSERT_EQ (join::swap (uint32Val), 201326592);

    int64_t int64Val = 12;
    ASSERT_EQ (join::swap (int64Val), 864691128455135232);

    uint64_t uint64Val = 12;
    ASSERT_EQ (join::swap (uint64Val), 864691128455135232);

    float floatVal = 12.0;
    ASSERT_FLOAT_EQ (join::swap (floatVal), 2.305e-41);

    double doubleVal = 12.0;
    ASSERT_DOUBLE_EQ (join::swap (doubleVal), 5.09085e-320);
}

/**
 * @brief Test compareNoCase.
 */
TEST (Utils, compareNoCase)
{
    ASSERT_TRUE  (join::compareNoCase ("One", "ONE"));
    ASSERT_TRUE  (join::compareNoCase ("TWO", "two"));
    ASSERT_TRUE  (join::compareNoCase ("Three", "ThReE"));
    ASSERT_FALSE (join::compareNoCase ("Four", "4"));
}

/**
 * @brief Test trimLeft.
 */
TEST (Utils, trimLeft)
{
    std::string str ("\f\t\v\r\n trim \f\t\v\r\n");
    ASSERT_EQ (join::trimLeft (str), "trim \f\t\v\r\n");
}

/**
 * @brief Test trimRight.
 */
TEST (Utils, trimRight)
{
    std::string str ("\f\t\v\r\n trim \f\t\v\r\n");
    ASSERT_EQ (join::trimRight (str), "\f\t\v\r\n trim");
}

/**
 * @brief Test trim.
 */
TEST (Utils, trim)
{
    std::string str ("\f\t\v\r\n trim \f\t\v\r\n");
    ASSERT_EQ (join::trim (str), "trim");
}

/**
 * @brief Test replaceAll.
 */
TEST (Utils, replaceAll)
{
    std::string str ("replace all other by other");
    ASSERT_EQ (join::replaceAll (str, "other", "OTHER"), "replace all OTHER by OTHER");
}

/**
 * @brief Test split.
 */
TEST (Utils, split)
{
    std::string str ("this=>is=>a=>string=>with=>delimiters");
    auto tokens = join::split (str, "=>");
    ASSERT_EQ (tokens.size (), 6);
    ASSERT_EQ (tokens[0], "this");
    ASSERT_EQ (tokens[1], "is");
    ASSERT_EQ (tokens[2], "a");
    ASSERT_EQ (tokens[3], "string");
    ASSERT_EQ (tokens[4], "with");
    ASSERT_EQ (tokens[5], "delimiters");
}

/**
 * @brief Test rsplit.
 */
TEST (Utils, rsplit)
{
    std::string str ("this=>is=>a=>string=>with=>delimiters");
    auto tokens = join::rsplit (str, "=>");
    ASSERT_EQ (tokens.size (), 6);
    ASSERT_EQ (tokens[0], "delimiters");
    ASSERT_EQ (tokens[1], "with");
    ASSERT_EQ (tokens[2], "string");
    ASSERT_EQ (tokens[3], "a");
    ASSERT_EQ (tokens[4], "is");
    ASSERT_EQ (tokens[5], "this");
}

/**
 * @brief Test getline.
 */
TEST (Utils, getline)
{
    std::stringstream stream;
    std::string line;

    stream.clear ();
    stream.str ("no end line");
    ASSERT_FALSE (join::getline (stream, line));

    stream.clear ();
    stream.str ("too long\r\n");
    ASSERT_FALSE (join::getline (stream, line, 1));
    ASSERT_EQ (join::lastError, Errc::MessageTooLong);

    stream.clear ();
    stream.str ("ok\r\n");
    ASSERT_TRUE (join::getline (stream, line));
    ASSERT_EQ (line, "ok");
}

/**
 * @brief Test dump.
 */
TEST (Utils, dump)
{
    std::stringstream out;
    std::string str ("☺this is a test☺");
    join::dump (str.c_str (), str.size (), out);
    ASSERT_EQ (out.str (), "00000000: E298BA74 68697320 69732061 20746573 ...this is a tes\n"
                           "00000010: 74E298BA                            t...\n\n");
}

/**
 * @brief Test randomize.
 */
TEST (Utils, randomize)
{
    ASSERT_GT (join::randomize <int> (), 0);
}

/**
 * @brief Test benchmark.
 */
TEST (Utils, benchmark)
{
    auto elapsed = join::benchmark ([]
    {
        std::this_thread::sleep_for (10ms);
    });
    ASSERT_GE (elapsed, 10ms);
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
