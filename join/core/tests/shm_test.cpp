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
    pid_t pid = fork ();
    ASSERT_NE (pid, -1);

    if (pid == 0)
    {
        Shm::Subscriber sub (1024);
        ASSERT_EQ (sub.wait (), -1);
        ASSERT_EQ (sub.timedWait (10ms), -1);
        ASSERT_EQ (sub.open (_name), 0) << join::lastError.message ();
        std::this_thread::sleep_for (5ms);
        ASSERT_EQ (sub.wait (), 0) << join::lastError.message ();
        ASSERT_STREQ (static_cast <char*> (sub.get ()), "Ping");
        ASSERT_EQ (sub.timedWait (10ms), 0) << join::lastError.message ();
        ASSERT_STREQ (static_cast <char*> (sub.get ()), "Pong");
        ASSERT_EQ (sub.wait (), 0) << join::lastError.message ();
        ASSERT_STREQ (static_cast <char*> (sub.get ()), "Ping");
        std::this_thread::sleep_for (10ms);
        ASSERT_EQ (sub.timedWait (10ms), 0) << join::lastError.message ();
        ASSERT_STREQ (static_cast <char*> (sub.get ()), "Pong");
        ASSERT_EQ (sub.timedWait (10ms), -1);
        sub.close ();
        _exit (0);
    }
    else
    {
        Shm::Publisher pub (1024);
        ASSERT_EQ (pub.notify (), -1);
        ASSERT_EQ (pub.open (_name), 0) << join::lastError.message ();
        ::strcpy (static_cast <char *> (pub.get ()), "Ping");
        ASSERT_EQ (pub.notify (), 0) << join::lastError.message ();
        std::this_thread::sleep_for (10ms);
        ::strcpy (static_cast <char *> (pub.get ()), "Pong");
        ASSERT_EQ (pub.notify (), 0) << join::lastError.message ();
        std::this_thread::sleep_for (10ms);
        ::strcpy (static_cast <char *> (pub.get ()), "Ping");
        ASSERT_EQ (pub.notify (), 0) << join::lastError.message ();
        std::this_thread::sleep_for (10ms);
        ::strcpy (static_cast <char *> (pub.get ()), "Pong");
        ASSERT_EQ (pub.notify (), 0) << join::lastError.message ();
        pub.close ();

        int status;
        waitpid (pid, &status, 0);
        ASSERT_TRUE (WIFEXITED (status));
        ASSERT_EQ (WEXITSTATUS (status), 0);
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
