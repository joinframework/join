/**
 * MIT License
 *
 * Copyright (c) 2025 Mathieu Rabine
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
#include <join/shm.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::Shm;

class ShmTest : public ::testing::Test
{
protected:
    /**
     * @brief Tears down the test fixture.
     */
    void TearDown () override
    {
        server.close ();
        client.close ();
    }

    Shm::Server server {1024};
    Shm::Client client {1024};

    /// name.
    static const std::string _name;
};

const std::string ShmTest::_name = "/test_shm";

TEST_F (ShmTest, open)
{
    ASSERT_EQ (server.open (_name, false), 0) << join::lastError.message ();
    EXPECT_EQ (client.open (_name, false), 0) << join::lastError.message ();
    client.close ();
    server.close ();
}

TEST_F (ShmTest, get)
{
    ASSERT_EQ (server.get (), nullptr);
    ASSERT_EQ (server.open (_name, false), 0) << join::lastError.message ();
    ASSERT_NE (server.get (), nullptr);
    ASSERT_EQ (client.get (), nullptr);
    EXPECT_EQ (client.open (_name, false), 0) << join::lastError.message ();
    ASSERT_NE (client.get (), nullptr);
    client.close ();
    ASSERT_EQ (client.get (), nullptr);
    server.close ();
    ASSERT_EQ (server.get (), nullptr);
}

TEST_F (ShmTest, size)
{
    ASSERT_EQ (server.size (), 1024);
    ASSERT_EQ (client.size (), 1024);
}

TEST_F (ShmTest, notify)
{
    std::thread th ([&] () {
        std::this_thread::sleep_for (std::chrono::milliseconds (10));
        EXPECT_EQ (client.open (_name, true), 0);
        EXPECT_EQ (client.wait (), 0) << join::lastError.message ();
        EXPECT_STREQ (static_cast <char*> (client.get ()), "Ping");
        ::strcpy (static_cast <char *> (client.get ()), "Pong");
        EXPECT_EQ (client.notify (), 0) << join::lastError.message ();
    });

    EXPECT_EQ (server.open (_name, true), 0);
    ::strcpy (static_cast <char *> (server.get ()), "Ping");
    EXPECT_EQ (server.notify (), 0) << join::lastError.message ();
    EXPECT_EQ (server.wait (), 0) << join::lastError.message ();
    EXPECT_STREQ (static_cast <char*> (server.get ()), "Pong");

    th.join ();

    client.close ();
    server.close ();
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
