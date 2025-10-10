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
#include <join/thread.hpp>
#include <join/shm.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::Shm;
using join::Thread;

const std::string _name = "/test_shm";

TEST (Shm, open)
{
    Shm::Server server1, server2 (-sizeof (join::ShmSync) - 1), server3 (-sizeof (join::ShmSync));
    Shm::Client client1;

    ASSERT_EQ (client1.open (_name), -1);
    ASSERT_EQ (server1.open (_name), 0) << join::lastError.message ();
    ASSERT_EQ (server1.open (_name), -1);
    ASSERT_EQ (server2.open (_name), -1);
    ASSERT_EQ (client1.open (_name), 0) << join::lastError.message ();
    ASSERT_EQ (client1.open (_name), -1);
    client1.close ();
    server1.close ();
    ASSERT_EQ (server2.open (_name), -1);
    ASSERT_EQ (server3.open (_name), -1);
}

TEST (Shm, get)
{
    Shm::Server server (1024);
    Shm::Client client (1024);

    ASSERT_EQ (server.get (), nullptr);
    ASSERT_EQ (server.open (_name), 0) << join::lastError.message ();
    ASSERT_NE (server.get (), nullptr);
    ASSERT_EQ (client.get (), nullptr);
    EXPECT_EQ (client.open (_name), 0) << join::lastError.message ();
    ASSERT_NE (client.get (), nullptr);
    client.close ();
    ASSERT_EQ (client.get (), nullptr);
    server.close ();
    ASSERT_EQ (server.get (), nullptr);
}

TEST (Shm, size)
{
    Shm::Server server (1024);
    Shm::Client client (1024);

    ASSERT_EQ (server.size (), 1024);
    ASSERT_EQ (client.size (), 1024);
}

TEST (Shm, notify)
{
    Shm::Server server (1024);
    Shm::Client client (1024);

    Thread th ([&] () {
        std::this_thread::sleep_for (std::chrono::milliseconds (10));
        ASSERT_EQ (client.notify (), -1);
        ASSERT_EQ (client.wait (), -1);
        ASSERT_EQ (client.timedWait (std::chrono::milliseconds (10)), -1);
        ASSERT_EQ (client.open (_name), 0) << join::lastError.message ();
        ASSERT_EQ (client.wait (), 0) << join::lastError.message ();
        ASSERT_STREQ (static_cast <char*> (client.get ()), "Ping");
        ::strcpy (static_cast <char *> (client.get ()), "Pong");
        ASSERT_EQ (client.notify (), 0) << join::lastError.message ();
        ASSERT_EQ (client.timedWait (std::chrono::milliseconds (10)), 0) << join::lastError.message ();
        ASSERT_STREQ (static_cast <char*> (client.get ()), "Ping");
        ::strcpy (static_cast <char *> (client.get ()), "Pong");
        ASSERT_EQ (client.notify (), 0) << join::lastError.message ();
        ASSERT_EQ (client.timedWait (std::chrono::milliseconds (1)), -1);
    });

    ASSERT_EQ (server.notify (), -1);
    ASSERT_EQ (server.wait (), -1);
    ASSERT_EQ (server.timedWait (std::chrono::milliseconds (10)), -1);
    ASSERT_EQ (server.open (_name), 0) << join::lastError.message ();
    ::strcpy (static_cast <char *> (server.get ()), "Ping");
    ASSERT_EQ (server.notify (), 0) << join::lastError.message ();
    ASSERT_EQ (server.wait (), 0) << join::lastError.message ();
    ASSERT_STREQ (static_cast <char*> (server.get ()), "Pong");
    ::strcpy (static_cast <char *> (server.get ()), "Ping");
    ASSERT_EQ (server.notify (), 0) << join::lastError.message ();
    ASSERT_EQ (server.timedWait (std::chrono::milliseconds (10)), 0) << join::lastError.message ();
    ASSERT_STREQ (static_cast <char*> (server.get ()), "Pong");
    ASSERT_EQ (server.timedWait (std::chrono::milliseconds (1)), -1);

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
