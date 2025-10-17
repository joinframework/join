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

using namespace std::chrono_literals;

using join::Shm;

const std::string _name = "/test_shm";

TEST (Shm, open)
{
    Shm::Publisher pub1, pub2 (-sizeof (join::ShmSync) - 1), pub3 (-sizeof (join::ShmSync));
    Shm::Subscriber sub1;

    ASSERT_EQ (pub1.open (_name), 0) << join::lastError.message ();
    ASSERT_EQ (pub1.open (_name), -1);
    ASSERT_EQ (pub2.open (_name), -1);
    ASSERT_EQ (sub1.open (_name), 0) << join::lastError.message ();
    ASSERT_EQ (sub1.open (_name), -1);
    sub1.close ();
    pub1.close ();
    ASSERT_EQ (pub2.open (_name), -1);
    ASSERT_EQ (pub3.open (_name), -1);
}

TEST (Shm, get)
{
    Shm::Publisher pub (1024);
    Shm::Subscriber sub (1024);

    ASSERT_EQ (pub.get (), nullptr);
    ASSERT_EQ (pub.open (_name), 0) << join::lastError.message ();
    ASSERT_NE (pub.get (), nullptr);
    ASSERT_EQ (sub.get (), nullptr);
    EXPECT_EQ (sub.open (_name), 0) << join::lastError.message ();
    ASSERT_NE (sub.get (), nullptr);
    sub.close ();
    ASSERT_EQ (sub.get (), nullptr);
    pub.close ();
    ASSERT_EQ (pub.get (), nullptr);
}

TEST (Shm, size)
{
    Shm::Publisher pub (1024);
    Shm::Subscriber sub (1024);

    ASSERT_EQ (pub.size (), 1024);
    ASSERT_EQ (sub.size (), 1024);
}

TEST (Shm, notify)
{
    pid_t child = fork ();
    if (child == 0)
    {
        Shm::Subscriber sub (1024);
        sub.open (_name);
        sub.timedWait (10ms);
        sub.timedWait (10ms);
        sub.timedWait (10ms);
        sub.timedWait (10ms);
        sub.close ();
        _exit (EXIT_SUCCESS);
    }
    else
    {
        EXPECT_NE (child, -1) << strerror (errno);
        Shm::Publisher pub (1024);
        EXPECT_EQ (pub.notify (), -1);
        EXPECT_EQ (pub.open (_name), 0) << join::lastError.message ();
        ::strcpy (static_cast <char *> (pub.get ()), "Ping");
        EXPECT_EQ (pub.notify (), 0) << join::lastError.message ();
        std::this_thread::sleep_for (5ms);
        ::strcpy (static_cast <char *> (pub.get ()), "Pong");
        EXPECT_EQ (pub.notify (), 0) << join::lastError.message ();
        std::this_thread::sleep_for (5ms);
        ::strcpy (static_cast <char *> (pub.get ()), "Ping");
        EXPECT_EQ (pub.notify (), 0) << join::lastError.message ();
        std::this_thread::sleep_for (5ms);
        ::strcpy (static_cast <char *> (pub.get ()), "Pong");
        EXPECT_EQ (pub.notify (), 0) << join::lastError.message ();
        int status = -1;
        waitpid (child, &status, 0);
        EXPECT_TRUE (WIFEXITED (status));
        EXPECT_EQ (WEXITSTATUS (status), 0);
        pub.close ();
    }
}

TEST (Shm, wait)
{
    pid_t child = fork ();
    if (child == 0)
    {
        Shm::Publisher pub (1024);
        pub.open (_name);
        ::strcpy (static_cast <char *> (pub.get ()), "Ping");
        pub.notify ();
        std::this_thread::sleep_for (10ms);
        ::strcpy (static_cast <char *> (pub.get ()), "Pong");
        pub.notify ();
        std::this_thread::sleep_for (10ms);
        ::strcpy (static_cast <char *> (pub.get ()), "Ping");
        pub.notify ();
        std::this_thread::sleep_for (10ms);
        ::strcpy (static_cast <char *> (pub.get ()), "Pong");
        pub.notify ();
        pub.close ();
        _exit (EXIT_SUCCESS);
    }
    else
    {
        EXPECT_NE (child, -1) << strerror (errno);
        Shm::Subscriber sub (1024);
        EXPECT_EQ (sub.wait (), -1);
        EXPECT_EQ (sub.timedWait (10ms), -1);
        EXPECT_EQ (sub.open (_name), 0) << join::lastError.message ();
        std::this_thread::sleep_for (5ms);
        EXPECT_EQ (sub.wait (), 0) << join::lastError.message ();
        EXPECT_STREQ (static_cast <char*> (sub.get ()), "Ping");
        EXPECT_EQ (sub.timedWait (10ms), 0) << join::lastError.message ();
        EXPECT_STREQ (static_cast <char*> (sub.get ()), "Pong");
        EXPECT_EQ (sub.wait (), 0) << join::lastError.message ();
        EXPECT_STREQ (static_cast <char*> (sub.get ()), "Ping");
        std::this_thread::sleep_for (10ms);
        EXPECT_EQ (sub.timedWait (10ms), 0) << join::lastError.message ();
        EXPECT_STREQ (static_cast <char*> (sub.get ()), "Pong");
        EXPECT_EQ (sub.timedWait (10ms), -1);
        int status = -1;
        waitpid (child, &status, 0);
        EXPECT_TRUE (WIFEXITED (status));
        EXPECT_EQ (WEXITSTATUS (status), 0);
        sub.close ();
    }
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
