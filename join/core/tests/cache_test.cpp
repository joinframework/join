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
#include <fcntl.h>

using join::Cache;

/**
 * @brief Class used to test cache.
 */
class CacheTest : public ::testing::Test
{
protected:
    /**
     * @brief Set up test.
     */
    virtual void SetUp ()
    {
        ASSERT_TRUE (writeFile (path, content));
        ASSERT_TRUE (writeFile (other, otherContent));
        ASSERT_NE (nullptr, cache.get (path));
        ASSERT_NE (nullptr, cache.get (other));
    }

    /**
     * @brief Tear down test.
     */
    virtual void TearDown ()
    {
        ASSERT_NO_THROW (cache.clear ());
        remove (other.c_str ());
        remove (path.c_str ());
    }

    /**
     * @brief Create file.
     * @param filepath path.
     * @param content content to write.
     * @return true on success, false otherwise.
     */
    bool writeFile (const std::string& filepath, const std::string& content)
    {
        int dst = ::open (filepath.c_str (), O_CREAT | O_TRUNC | O_WRONLY, 0644);
        if (dst == -1)
        {
            return false;
        }
        if (::write (dst, content.c_str (), content.size ()) != ssize_t (content.size ()))
        {
            ::close (dst);
            return false;
        }
        ::fsync (dst);
        ::close (dst);
        return true;
    }

    /// server instance.
    static Cache cache;

    /// file base.
    static const std::string base;

    /// file name without extension.
    static const std::string stem;

    /// file extension.
    static const std::string ext;

    /// file name.
    static const std::string name;

    /// file path.
    static const std::string path;

    /// file content.
    static const std::string content;

    /// bad file path.
    static const std::string bad;

    /// other file path.
    static const std::string other;

    /// other file content.
    static const std::string otherContent;
};

Cache             CacheTest::cache;
const std::string CacheTest::base             = "/tmp/";
const std::string CacheTest::stem             = "join_cache_test";
const std::string CacheTest::ext              = "txt";
const std::string CacheTest::name             = stem + "." + ext;
const std::string CacheTest::path             = base + name;
const std::string CacheTest::content          = "test string";
const std::string CacheTest::bad              = base + stem + ".bad";
const std::string CacheTest::other            = base + stem + ".other";
const std::string CacheTest::otherContent     = "other test string";

/**
 * @brief Test get method.
 */
TEST_F (CacheTest, get)
{
    ASSERT_EQ (cache.get (bad), nullptr);

    struct stat sbuf;
    ASSERT_EQ (stat (base.c_str (), &sbuf), 0) << strerror (errno);
    ASSERT_EQ (cache.get (bad, &sbuf), nullptr);

    ASSERT_EQ (stat (base.c_str (), &sbuf), 0) << strerror (errno);
    ASSERT_EQ (cache.get (base, &sbuf), nullptr);

    ASSERT_EQ (stat (path.c_str (), &sbuf), 0) << strerror (errno);
    char* data = reinterpret_cast <char*> (cache.get (path, &sbuf));
    ASSERT_NE (data, nullptr);
    ASSERT_EQ (std::string (data, sbuf.st_size), content);

    ASSERT_EQ (stat (other.c_str (), &sbuf), 0) << strerror (errno);
    data = reinterpret_cast <char*> (cache.get (other));
    ASSERT_NE (data, nullptr);
    ASSERT_EQ (std::string (data, sbuf.st_size), otherContent);

    ASSERT_TRUE (writeFile (path, otherContent));
    ASSERT_EQ (stat (path.c_str (), &sbuf), 0) << strerror (errno);
    data = reinterpret_cast <char*> (cache.get (path, &sbuf));
    ASSERT_NE (data, nullptr);
    ASSERT_EQ (std::string (data, sbuf.st_size), otherContent);
}

/**
 * @brief Test remove method.
 */
TEST_F (CacheTest, remove)
{
    EXPECT_NO_THROW (cache.remove (path));
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
