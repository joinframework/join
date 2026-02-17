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
#include <join/thread.hpp>
#include <join/mutex.hpp>

// Libraries.
#include <gtest/gtest.h>

// C++.
#include <thread>

// C.
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

using join::SharedMutex;
using join::ScopedLock;

using namespace std::chrono_literals;

const std::string _name = "/test_mutex";

/**
 * @brief test lock.
 */
TEST (SharedMutex, lock)
{
    SharedMutex* mutex = nullptr;
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

    mutex = static_cast <SharedMutex*> (shm);
    new (mutex) SharedMutex ();

    child = fork ();
    if (child == 0)
    {
        mutex->lock ();
        std::this_thread::sleep_for (15ms);
        _exit (EXIT_SUCCESS);
    }
    else
    {
        EXPECT_NE (child, -1) << strerror (errno);
        if (HasFailure ())
        {
            goto cleanup;
        }
        std::this_thread::sleep_for (5ms);
        auto beg = std::chrono::high_resolution_clock::now ();
        mutex->lock ();
        auto end = std::chrono::high_resolution_clock::now ();
        EXPECT_GT (std::chrono::duration_cast <std::chrono::milliseconds> (end - beg), 5ms);
        mutex->unlock ();
        int status = -1;
        waitpid (child, &status, 0);
        EXPECT_TRUE (WIFEXITED (status));
        EXPECT_EQ (WEXITSTATUS (status), 0);
    }

cleanup:
    if ((shm != nullptr) && (shm != MAP_FAILED))
    {
        mutex->~SharedMutex ();
        EXPECT_NE (::munmap (shm, sizeof (SharedMutex)), -1) << strerror (errno);
    }

    if (fd != -1)
    {
        EXPECT_NE (::close (fd), -1) << strerror (errno);
    }
}

/**
 * @brief test tryLock.
 */
TEST (SharedMutex, tryLock)
{
    SharedMutex* mutex = nullptr;
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

    mutex = static_cast <SharedMutex*> (shm);
    new (mutex) SharedMutex ();

    child = fork ();
    if (child == 0)
    {
        mutex->lock ();
        std::this_thread::sleep_for (15ms);
        _exit (EXIT_SUCCESS);
    }
    else
    {
        EXPECT_NE (child, -1) << strerror (errno);
        if (HasFailure ())
        {
            goto cleanup;
        }
        std::this_thread::sleep_for (5ms);
        EXPECT_FALSE (mutex->tryLock ());
        std::this_thread::sleep_for (15ms);
        EXPECT_TRUE (mutex->tryLock ());
        mutex->unlock ();
        int status = -1;
        waitpid (child, &status, 0);
        EXPECT_TRUE (WIFEXITED (status));
        EXPECT_EQ (WEXITSTATUS (status), 0);
    }

cleanup:
    if ((shm != nullptr) && (shm != MAP_FAILED))
    {
        mutex->~SharedMutex ();
        EXPECT_NE (::munmap (shm, sizeof (SharedMutex)), -1) << strerror (errno);
    }

    if (fd != -1)
    {
        EXPECT_NE (::close (fd), -1) << strerror (errno);
    }
}

/**
 * @brief test scoped lock.
 */
TEST (SharedMutex, scopedLock)
{
    SharedMutex* mutex = nullptr;
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

    mutex = static_cast <SharedMutex*> (shm);
    new (mutex) SharedMutex ();

    child = fork ();
    if (child == 0)
    {
        {
            ScopedLock <SharedMutex> lock (*mutex);
            std::this_thread::sleep_for (15ms);
        }
        _exit (EXIT_SUCCESS);
    }
    else
    {
        EXPECT_NE (child, -1) << strerror (errno);
        if (HasFailure ())
        {
            goto cleanup;
        }
        std::this_thread::sleep_for (5ms);
        auto beg = std::chrono::high_resolution_clock::now ();
        mutex->lock ();
        auto end = std::chrono::high_resolution_clock::now ();
        EXPECT_GT (std::chrono::duration_cast <std::chrono::milliseconds> (end - beg), 5ms);
        mutex->unlock ();
        int status = -1;
        waitpid (child, &status, 0);
        EXPECT_TRUE (WIFEXITED (status));
        EXPECT_EQ (WEXITSTATUS (status), 0);
    }

cleanup:
    if ((shm != nullptr) && (shm != MAP_FAILED))
    {
        mutex->~SharedMutex ();
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
