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

using namespace std::chrono_literals;

using join::ShmRing;

const std::string _name = "/test_shm";

TEST (ShmRing, open)
{
    ShmRing::Producer prod1 (64, 8);
    ShmRing::Consumer cons1 (64, 8), cons2 (128, 16);

    ASSERT_EQ (prod1.elementSize (), 64);
    ASSERT_EQ (prod1.capacity (), 8);
    ASSERT_FALSE (prod1.opened ());
    ASSERT_EQ (prod1.get (), nullptr);
    ASSERT_EQ (prod1.open (_name), 0) << join::lastError.message ();
    ASSERT_NE (prod1.get (), nullptr);
    ASSERT_TRUE (prod1.opened ());
    ASSERT_EQ (prod1.open (_name), -1);
    ASSERT_TRUE (prod1.opened ());
    ASSERT_EQ (cons1.elementSize (), 64);
    ASSERT_EQ (cons1.capacity (), 8);
    ASSERT_FALSE (cons1.opened ());
    ASSERT_EQ (cons1.get (), nullptr);
    ASSERT_EQ (cons1.open (_name), 0) << join::lastError.message ();
    ASSERT_NE (cons1.get (), nullptr);
    ASSERT_TRUE (cons1.opened ());
    ASSERT_EQ (cons1.open (_name), -1);
    ASSERT_TRUE (cons1.opened ());
    ASSERT_EQ (cons2.elementSize (), 128);
    ASSERT_EQ (cons2.capacity (), 16);
    ASSERT_FALSE (cons2.opened ());
    ASSERT_EQ (cons2.get (), nullptr);
    ASSERT_EQ (cons2.open (_name), -1);
    ASSERT_EQ (cons2.get (), nullptr);
    ASSERT_FALSE (cons2.opened ());
    prod1.close ();
    ASSERT_FALSE (prod1.opened ());
    cons1.close ();
    ASSERT_FALSE (cons1.opened ());
}

TEST (ShmRing, push)
{
    ShmRing::Producer prod (64, 8);
    char data[64] = {};

    ASSERT_EQ (prod.capacity (), 8);
    ASSERT_EQ (prod.available (), 0);
    ASSERT_EQ (prod.pending (), 0);
    ASSERT_EQ (prod.open (_name), 0) << join::lastError.message ();
    ASSERT_EQ (prod.push (nullptr),-1);
    ASSERT_EQ (prod.capacity (), 8);
    ASSERT_EQ (prod.available (), 8);
    ASSERT_EQ (prod.pending (), 0);
    ASSERT_FALSE (prod.full ());
    ASSERT_TRUE (prod.empty ());
    ASSERT_EQ (prod.push (data), 0) << join::lastError.message ();
    ASSERT_EQ (prod.available (), 7);
    ASSERT_EQ (prod.pending (), 1);
    ASSERT_FALSE (prod.full ());
    ASSERT_FALSE (prod.empty ());
    ASSERT_EQ (prod.push (data), 0) << join::lastError.message ();
    ASSERT_EQ (prod.available (), 6);
    ASSERT_EQ (prod.pending (), 2);
    ASSERT_FALSE (prod.full ());
    ASSERT_FALSE (prod.empty ());
    ASSERT_EQ (prod.push (data), 0) << join::lastError.message ();
    ASSERT_EQ (prod.available (), 5);
    ASSERT_EQ (prod.pending (), 3);
    ASSERT_FALSE (prod.full ());
    ASSERT_FALSE (prod.empty ());
    ASSERT_EQ (prod.push (data), 0) << join::lastError.message ();
    ASSERT_EQ (prod.available (), 4);
    ASSERT_EQ (prod.pending (), 4);
    ASSERT_FALSE (prod.full ());
    ASSERT_FALSE (prod.empty ());
    ASSERT_EQ (prod.push (data), 0) << join::lastError.message ();
    ASSERT_EQ (prod.available (), 3);
    ASSERT_EQ (prod.pending (), 5);
    ASSERT_FALSE (prod.full ());
    ASSERT_FALSE (prod.empty ());
    ASSERT_EQ (prod.push (data), 0) << join::lastError.message ();
    ASSERT_EQ (prod.available (), 2);
    ASSERT_EQ (prod.pending (), 6);
    ASSERT_FALSE (prod.full ());
    ASSERT_FALSE (prod.empty ());
    ASSERT_EQ (prod.push (data), 0) << join::lastError.message ();
    ASSERT_EQ (prod.available (), 1);
    ASSERT_EQ (prod.pending (), 7);
    ASSERT_FALSE (prod.full ());
    ASSERT_FALSE (prod.empty ());
    ASSERT_EQ (prod.push (data), 0) << join::lastError.message ();
    ASSERT_EQ (prod.available (), 0);
    ASSERT_EQ (prod.pending (), 8);
    ASSERT_TRUE (prod.full ());
    ASSERT_FALSE (prod.empty ());
    ASSERT_EQ (prod.push (data), -1);
    ASSERT_EQ (prod.available (), 0);
    ASSERT_EQ (prod.pending (), 8);
    ASSERT_TRUE (prod.full ());
    ASSERT_FALSE (prod.empty ());
    prod.close ();
}

TEST (ShmRing, pop)
{
    ShmRing::Producer prod (64, 8);
    ShmRing::Consumer cons (64, 8);
    char data[64] = {};

    ASSERT_EQ (prod.open (_name), 0) << join::lastError.message ();
    ASSERT_EQ (cons.open (_name), 0) << join::lastError.message ();
    ASSERT_EQ (prod.push (data), 0) << join::lastError.message ();
    ASSERT_EQ (prod.push (data), 0) << join::lastError.message ();
    ASSERT_EQ (cons.pop (nullptr), -1);
    ASSERT_EQ (cons.available (), 6);
    ASSERT_EQ (cons.pending (), 2);
    ASSERT_EQ (cons.pop (data), 0) << join::lastError.message ();
    ASSERT_EQ (cons.available (), 7);
    ASSERT_EQ (cons.pending (), 1);
    ASSERT_EQ (prod.push (data), 0) << join::lastError.message ();
    ASSERT_EQ (prod.push (data), 0) << join::lastError.message ();
    ASSERT_EQ (prod.push (data), 0) << join::lastError.message ();
    ASSERT_EQ (prod.push (data), 0) << join::lastError.message ();
    ASSERT_EQ (prod.push (data), 0) << join::lastError.message ();
    ASSERT_EQ (cons.available (), 2);
    ASSERT_EQ (cons.pending (), 6);
    ASSERT_EQ (cons.pop (data), 0) << join::lastError.message ();
    ASSERT_EQ (cons.pop (data), 0) << join::lastError.message ();
    ASSERT_EQ (cons.pop (data), 0) << join::lastError.message ();
    ASSERT_EQ (cons.pop (data), 0) << join::lastError.message ();
    ASSERT_EQ (cons.pop (data), 0) << join::lastError.message ();
    ASSERT_EQ (cons.pop (data), 0) << join::lastError.message ();
    ASSERT_EQ (cons.available (), 8);
    ASSERT_EQ (cons.pending (), 0);
    prod.close ();
    cons.close ();
}

TEST (ShmRing, timedPop)
{
    ShmRing::Producer prod (64, 8);
    ShmRing::Consumer cons (64, 8);
    char data[64] = {};

    ASSERT_EQ (prod.open (_name), 0) << join::lastError.message ();
    ASSERT_EQ (cons.open (_name), 0) << join::lastError.message ();
    ASSERT_EQ (prod.push (data), 0) << join::lastError.message ();
    ASSERT_EQ (prod.push (data), 0) << join::lastError.message ();
    ASSERT_EQ (cons.timedPop (nullptr, 5ms), -1);
    ASSERT_EQ (cons.available (), 6);
    ASSERT_EQ (cons.pending (), 2);
    ASSERT_EQ (cons.timedPop (data, 5ms), 0) << join::lastError.message ();
    ASSERT_EQ (cons.available (), 7);
    ASSERT_EQ (cons.pending (), 1);
    ASSERT_EQ (prod.push (data), 0) << join::lastError.message ();
    ASSERT_EQ (prod.push (data), 0) << join::lastError.message ();
    ASSERT_EQ (prod.push (data), 0) << join::lastError.message ();
    ASSERT_EQ (prod.push (data), 0) << join::lastError.message ();
    ASSERT_EQ (prod.push (data), 0) << join::lastError.message ();
    ASSERT_EQ (cons.available (), 2);
    ASSERT_EQ (cons.pending (), 6);
    ASSERT_EQ (cons.timedPop (data, 5ms), 0) << join::lastError.message ();
    ASSERT_EQ (cons.timedPop (data, 5ms), 0) << join::lastError.message ();
    ASSERT_EQ (cons.timedPop (data, 5ms), 0) << join::lastError.message ();
    ASSERT_EQ (cons.timedPop (data, 5ms), 0) << join::lastError.message ();
    ASSERT_EQ (cons.timedPop (data, 5ms), 0) << join::lastError.message ();
    ASSERT_EQ (cons.timedPop (data, 5ms), 0) << join::lastError.message ();
    ASSERT_EQ (cons.available (), 8);
    ASSERT_EQ (cons.pending (), 0);
    ASSERT_EQ (cons.timedPop (data, 5ms), -1);
    prod.close ();
    cons.close ();
}

TEST (ShmRing, benchmark)
{
    const uint64_t N = 1000000;
    char data[64] = {};

    pid_t pid = fork ();
    ASSERT_NE (pid, -1);

    if (pid == 0)
    {
        ShmRing::Consumer cons (64, 1024);
        ASSERT_EQ (cons.open (_name), 0) << join::lastError.message ();
        for (uint64_t i = 0; i < N; ++i)
        {
            ASSERT_EQ (cons.pop (data), 0) << join::lastError.message ();
        }
        cons.close ();
        _exit (0);
    }
    else
    {
        ShmRing::Producer prod (64, 1024);
        ASSERT_EQ (prod.open (_name), 0) << join::lastError.message ();
        for (uint64_t i = 0; i < 1024; ++i)
        {
            ASSERT_EQ (prod.push (data), 0) << join::lastError.message ();
        }
        auto t0 = std::chrono::high_resolution_clock::now ();
        for (uint64_t i = 0; i < N; ++i)
        {
            while  (prod.full ()) { /*spin wait*/ }
            ASSERT_EQ (prod.push (data), 0) << join::lastError.message ();
        }
        auto t1 = std::chrono::high_resolution_clock::now();

        std::cout << "Benchmark results for " << N << " messages of 64 bytes:" << std::endl;
        std::cout << "Total time:    " << std::chrono::duration_cast <std::chrono::milliseconds> (t1 - t0).count () << " ms" << std::endl;
        std::cout << "Throughput:    " << std::fixed << std::setprecision (0) << N / std::chrono::duration <double> (t1 - t0).count () << " msg/s" << std::endl;
        std::cout << "Latency:       " << std::chrono::duration_cast <std::chrono::nanoseconds> (t1 - t0).count () / double (N) << " ns" << std::endl;

        ASSERT_EQ (prod.pending (), 1024);
        prod.close ();

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
