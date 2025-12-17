/**
 * MIT License
 *
 * Copyright (c) 202 Mathieu Rabine
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
#include <join/json.hpp>

// libraries.
#include <gtest/gtest.h>

// C++.
#include <fstream>

using join::Value;

/**
 * @brief test canada parsing.
 */
TEST (Parse, canada)
{
    std::ifstream fs ("data/canada.json");
    ASSERT_TRUE (fs.is_open ());

    Value value;
    ASSERT_EQ (value.jsonRead (fs), 0) << join::lastError.message ();
}

/**
 * @brief test citm catalog parsing.
 */
TEST (Parse, citm_catalog)
{
    std::ifstream fs ("data/citm_catalog.json");
    ASSERT_TRUE (fs.is_open ());

    std::stringstream buffer;
    buffer << fs.rdbuf ();
    std::string json = buffer.str ();

    Value value;
    ASSERT_EQ (value.jsonRead (json), 0) << join::lastError.message ();
}

/**
 * @brief test twitter parsing.
 */
TEST (Parse, twitter)
{
    std::ifstream fs ("data/twitter.json");
    ASSERT_TRUE (fs.is_open ());

    Value value;
    ASSERT_EQ (value.jsonRead (fs), 0) << join::lastError.message ();
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
