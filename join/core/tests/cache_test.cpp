/**
 * MIT License
 *
 * Copyright (c) 2022 Mathieu Rabine
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
#include <join/cache.hpp>

// Libraries.
#include <gtest/gtest.h>

// C++.
#include <iostream>
#include <fstream>

// C.
#include <unistd.h>

using join::Cache;

/**
 * @brief Class used to test cache.
 */
class CacheTest : public ::testing::Test
{
public:
    /**
     * @brief Set up test case.
     */
    static void SetUpTestCase ()
    {
        std::ofstream outFile;

        outFile.open (file);
        outFile << fileContent << std::endl;
        outFile.close ();

        outFile.open (otherFile);
        outFile << otherFileContent << std::endl;
        outFile.close ();
    }

    /**
     * @brief Tear down test case.
     */
    static void TearDownTestCase ()
    {
        remove (file.c_str ());
        remove (otherFile.c_str ());
    }

protected:
    /**
     * @brief Set up test.
     */
    virtual void SetUp ()
    {
        ASSERT_NE (nullptr, cache.get (file));
        ASSERT_NE (nullptr, cache.get (otherFile));
    }

    /**
     * @brief Tear down test.
     */
    virtual void TearDown ()
    {
        ASSERT_NO_THROW (cache.clear ());
    }

    /// server instance.
    static Cache cache;

    /// file name.
    static const std::string file;

    /// file content.
    static const std::string fileContent;

    /// other file name.
    static const std::string otherFile;

    /// other file content.
    static const std::string otherFileContent;
};

Cache             CacheTest::cache;
const std::string CacheTest::file             = "/tmp/cache_test";
const std::string CacheTest::fileContent      = "test string";
const std::string CacheTest::otherFile        = "/tmp/cache_other_test";
const std::string CacheTest::otherFileContent = "other test string";

/**
 * @brief Test get method.
 */
TEST_F (CacheTest, get)
{
    struct stat sbuf;

    void* fi = cache.get (file);
    ASSERT_NE (fi, nullptr);
    ASSERT_EQ (stat (file.c_str (), &sbuf), 0);
    ASSERT_STREQ (std::string (reinterpret_cast <char const*> (fi), sbuf.st_size - 1).c_str (), fileContent.c_str ());

    fi = cache.get (otherFile);
    ASSERT_NE (fi, nullptr);
    ASSERT_EQ (stat (otherFile.c_str (), &sbuf), 0);
    ASSERT_STREQ (std::string (reinterpret_cast <char const*> (fi), sbuf.st_size - 1).c_str (), otherFileContent.c_str ());
}

/**
 * @brief Test remove method.
 */
TEST_F (CacheTest, remove)
{
    EXPECT_NO_THROW (cache.remove (file));
    EXPECT_EQ (1, cache.size ());
}

/**
 * @brief Test clear method.
 */
TEST_F (CacheTest, clear)
{
    EXPECT_NO_THROW (cache.clear ());
    EXPECT_EQ (0, cache.size ());
}

/**
 * @brief Test size method.
 */
TEST_F (CacheTest, size)
{
    EXPECT_EQ (2, cache.size ());
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
