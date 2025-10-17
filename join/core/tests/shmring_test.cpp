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
#include <join/utils.hpp>
#include <join/shm.hpp>

// Libraries.
#include <gtest/gtest.h>

// C++.
#include <algorithm>

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
    const uint64_t numMessages = 1000000;
    const uint64_t capacity = 1024;
    const uint64_t elementSize = 256;
    char data[elementSize] = {};

    pid_t child = fork ();
    if (child == 0)
    {
        ShmRing::Producer prod (elementSize, capacity);
        prod.open (_name);

        // pre-fill the buffer ("fast path" benchmark).
        for (uint64_t i = 0; i < capacity; ++i)
        {
            prod.push (data);
        }

        for (uint64_t i = 0; i < numMessages; ++i)
        {
            while (prod.push (data) == -1)
            {
                // spin wait.
                std::this_thread::yield ();
            }
        }

        // let the consumer finnish.
        std::this_thread::sleep_for (100ms);
        prod.close ();

        _exit (0);
    }
    else
    {
        EXPECT_NE (child, -1);
        if (HasFailure ())
        {
            goto cleanup;
        }

        // let the producer prefill the buffer.
        std::this_thread::sleep_for (100ms);

        ShmRing::Consumer cons (elementSize, capacity);
        EXPECT_EQ (cons.open (_name), 0) << join::lastError.message ();
        if (HasFailure ())
        {
            goto cleanup;
        }

        std::vector <uint64_t> recvTimestamps;
        recvTimestamps.reserve (numMessages);

        // benchmark
        auto beg = std::chrono::high_resolution_clock::now ();
        for (uint64_t i = 0; i < numMessages; ++i)
        {
            EXPECT_EQ (cons.timedPop (data, 10ms), 0) << join::lastError.message ();
            if (HasFailure ())
            {
                goto cleanup;
            }

            auto recvTime = std::chrono::high_resolution_clock::now ();
            recvTimestamps.push_back (recvTime.time_since_epoch ().count ());
        }
        auto end = std::chrono::high_resolution_clock::now ();

        // consume pre-filled messages.
        for (uint64_t i = 0; i < capacity; ++i)
        {
            EXPECT_EQ (cons.timedPop (data, 10ms), 0) << join::lastError.message ();
            if (HasFailure ())
            {
                goto cleanup;
            }
        }

        EXPECT_EQ (cons.pending(), 0);
        cons.close ();

        // metrics
        auto duration = end - beg;
        double durationSec = std::chrono::duration <double> (duration).count ();
        double durationMs = std::chrono::duration <double, std::milli> (duration).count ();

        double throughputMsg = numMessages / durationSec;
        double bandwidthMBps = (numMessages * elementSize) / (durationSec * 1024 * 1024);
        double bandwidthGbps = (numMessages * elementSize * 8) / (durationSec * 1000 * 1000 * 1000);

        std::vector <uint64_t> interArrivalTimes;
        interArrivalTimes.reserve (numMessages - 1);

        for (uint64_t i = 1; i < numMessages; ++i)
        {
            interArrivalTimes.push_back (recvTimestamps[i] - recvTimestamps[i-1]);
        }
        std::sort (interArrivalTimes.begin (), interArrivalTimes.end ());

        size_t n = interArrivalTimes.size ();
        uint64_t minTime  = interArrivalTimes[0];
        uint64_t maxTime  = interArrivalTimes[n - 1];
        uint64_t p50Time  = interArrivalTimes[n * 50 / 100];
        uint64_t p90Time  = interArrivalTimes[n * 90 / 100];
        uint64_t p95Time  = interArrivalTimes[n * 95 / 100];
        uint64_t p99Time  = interArrivalTimes[n * 99 / 100];
        uint64_t p999Time = interArrivalTimes[n * 999 / 1000];

        uint64_t sumTime = 0;
        for (uint64_t t : interArrivalTimes)
        {
            sumTime += t;
        }
        double avgTime = static_cast <double> (sumTime) / n;

        std::cout << "\n=== ShmRing Benchmark Results ===" << std::endl;
        std::cout << "\nConfiguration:" << std::endl;
        std::cout << "  Messages:      " << numMessages << std::endl;
        std::cout << "  Message size:  " << elementSize << " bytes" << std::endl;
        std::cout << "  Buffer size:   " << capacity << " slots" << std::endl;
        std::cout << "  Total data:    " << std::fixed << std::setprecision(2) << (numMessages * elementSize) / (1024.0 * 1024.0) << " MB" << std::endl;
        std::cout << "\nTiming:" << std::endl;
        std::cout << "  Total time:    " << std::fixed << std::setprecision(3) << durationMs << " ms" << std::endl;
        std::cout << "\nThroughput:" << std::endl;
        std::cout << "  Messages/sec:  " << std::fixed << std::setprecision(0) << throughputMsg << " msg/s" << std::endl;
        std::cout << "\nBandwidth:" << std::endl;
        std::cout << "  " << std::fixed << std::setprecision(2) << bandwidthMBps << " MB/s" << std::endl;
        std::cout << "  " << std::fixed << std::setprecision(2) << bandwidthGbps << " Gb/s" << std::endl;
        std::cout << "\nInter-arrival Time (nanoseconds):" << std::endl;
        std::cout << "  Minimum:       " << minTime << " ns" << std::endl;
        std::cout << "  Average:       " << std::fixed << std::setprecision(2) << avgTime << " ns" << std::endl;
        std::cout << "  Median (p50):  " << p50Time << " ns" << std::endl;
        std::cout << "  p90:           " << p90Time << " ns" << std::endl;
        std::cout << "  p95:           " << p95Time << " ns" << std::endl;
        std::cout << "  p99:           " << p99Time << " ns" << std::endl;
        std::cout << "  p99.9:         " << p999Time << " ns" << std::endl;
        std::cout << "  Maximum:       " << maxTime << " ns" << std::endl;
        std::cout << "\nInter-arrival Time (microseconds):" << std::endl;
        std::cout << "  Average:       " << std::fixed << std::setprecision(2) << avgTime / 1000.0 << " μs" << std::endl;
        std::cout << "  p50:           " << std::fixed << std::setprecision(2) << p50Time / 1000.0 << " μs" << std::endl;
        std::cout << "  p99:           " << std::fixed << std::setprecision(2) << p99Time / 1000.0 << " μs" << std::endl;
        std::cout << std::endl;
    }

cleanup:
    int status;
    waitpid (child, &status, 0);
    ASSERT_TRUE (WIFEXITED (status));
    ASSERT_EQ (WEXITSTATUS (status), 0);
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
