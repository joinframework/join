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
#include <join/condition.hpp>
#include <join/thread.hpp>
#include <join/error.hpp>

// Libraries.
#include <gtest/gtest.h>

// C.
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

using join::SharedCondition;
using join::SharedMutex;
using join::ScopedLock;

using namespace std::chrono_literals;

const std::string _name = "/test_condition";

struct ConditionSync
{
    alignas (64) SharedMutex mutex;
    SharedCondition condition;
    alignas (64) bool ready;
};

/**
 * @brief test wait.
 */
TEST (SharedCondition, wait)
{
    ConditionSync* sync = nullptr;
    void* shm = nullptr;
    pid_t child = -1;

    int fd = ::shm_open (_name.c_str (), O_CREAT | O_RDWR, 0644);
    ASSERT_NE (fd, -1) << strerror (errno);
    EXPECT_NE (::ftruncate (fd, sizeof (SharedMutex)), -1) << strerror (errno);
    if (HasFailure ())
    {
        goto cleanup;
    }
    shm = ::mmap (nullptr, sizeof (SharedMutex), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    EXPECT_NE (shm, nullptr) << strerror (errno);
    if (HasFailure ())
    {
        goto cleanup;
    }

    sync = static_cast <ConditionSync*> (shm);
    new (&sync->condition) SharedCondition ();
    new (&sync->mutex) SharedMutex ();
    new (&sync->ready) bool (false);

    child = fork ();
    if (child == 0)
    {
        std::this_thread::sleep_for (5ms);
        ScopedLock <SharedMutex> lk (sync->mutex);
        std::this_thread::sleep_for (15ms);
        sync->ready = true;
        sync->condition.signal ();
        _exit (EXIT_SUCCESS);
    }
    else
    {
        EXPECT_NE (child, -1) << strerror (errno);
        if (HasFailure ())
        {
            goto cleanup;
        }
        ScopedLock <SharedMutex> lock (sync->mutex);
        auto beg = std::chrono::high_resolution_clock::now ();
        sync->condition.wait (lock, [&] () {return sync->ready;});
        auto end = std::chrono::high_resolution_clock::now ();
        EXPECT_GE (std::chrono::duration_cast <std::chrono::milliseconds> (end - beg), 5ms);
        int status = -1;
        waitpid (child, &status, 0);
        EXPECT_TRUE (WIFEXITED (status));
        EXPECT_EQ (WEXITSTATUS (status), 0);
    }

cleanup:
    if ((shm != nullptr) && (shm != MAP_FAILED))
    {
        sync->condition.~SharedCondition ();
        sync->mutex.~SharedMutex ();
        EXPECT_NE (::munmap (shm, sizeof (SharedMutex)), -1) << strerror (errno);
    }

    if (fd != -1)
    {
        EXPECT_NE (::close (fd), -1) << strerror (errno);
    }
}

/**
 * @brief test timedWait.
 */
TEST (SharedCondition, timedWait)
{
    ConditionSync* sync = nullptr;
    void* shm = nullptr;
    pid_t child = -1;

    int fd = ::shm_open (_name.c_str (), O_CREAT | O_RDWR, 0644);
    ASSERT_NE (fd, -1) << strerror (errno);
    EXPECT_NE (::ftruncate (fd, sizeof (SharedMutex)), -1) << strerror (errno);
    if (HasFailure ())
    {
        goto cleanup;
    }
    shm = ::mmap (nullptr, sizeof (SharedMutex), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    EXPECT_NE (shm, nullptr) << strerror (errno);
    if (HasFailure ())
    {
        goto cleanup;
    }

    sync = static_cast <ConditionSync*> (shm);
    new (&sync->condition) SharedCondition ();
    new (&sync->mutex) SharedMutex ();
    new (&sync->ready) bool (false);

    child = fork ();
    if (child == 0)
    {
        std::this_thread::sleep_for (10ms);
        ScopedLock <SharedMutex> lk (sync->mutex);
        std::this_thread::sleep_for (10ms);
        sync->ready = true;
        sync->condition.broadcast ();
        _exit (EXIT_SUCCESS);
    }
    else
    {
        EXPECT_NE (child, -1) << strerror (errno);
        if (HasFailure ())
        {
            goto cleanup;
        }
        ScopedLock <SharedMutex> lock (sync->mutex);
        auto beg = std::chrono::high_resolution_clock::now ();
        EXPECT_FALSE (sync->condition.timedWait (lock, 5ms, [&](){return sync->ready;}));
        EXPECT_TRUE (sync->condition.timedWait (lock, 50ms, [&](){return sync->ready;})) << join::lastError.message ();
        auto end = std::chrono::high_resolution_clock::now ();
        EXPECT_GE (std::chrono::duration_cast <std::chrono::milliseconds> (end - beg), 5ms);
        int status = -1;
        waitpid (child, &status, 0);
        EXPECT_TRUE (WIFEXITED (status));
        EXPECT_EQ (WEXITSTATUS (status), 0);
    }

cleanup:
    if ((shm != nullptr) && (shm != MAP_FAILED))
    {
        sync->condition.~SharedCondition ();
        sync->mutex.~SharedMutex ();
        EXPECT_NE (::munmap (shm, sizeof (SharedMutex)), -1) << strerror (errno);
    }

    if (fd != -1)
    {
        EXPECT_NE (::close (fd), -1) << strerror (errno);
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
