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
#include <join/semaphore.hpp>
#include <join/thread.hpp>
#include <join/shared.hpp>
#include <join/utils.hpp>

// Libraries.
#include <gtest/gtest.h>

// C++.
#include <algorithm>

using namespace std::chrono_literals;

using join::Semaphore;
using join::Mpmc;

/**
 * @brief class used to test the multiple producer multiple consumer ring buffer.
 */
class MpmcBuffer : public ::testing::Test
{
protected:
    /**
     * @brief sets up the test fixture.
     */
    void SetUp ()
    {
        ::shm_unlink (_name.c_str ());
    }

    /**
     * @brief print benchmark metrics.
     * @param size element size.
     * @param capacity capacity.
     * @param num number of messages.
     * @param duration benchmark duration.
     * @param timestamps receive timestamps.
     */
    void metrics (uint64_t size, uint64_t capacity, uint64_t num, std::chrono::nanoseconds duration, const std::vector <uint64_t>& timestamps)
    {
        double durationSec = std::chrono::duration <double> (duration).count ();
        double durationMs = std::chrono::duration <double, std::milli> (duration).count ();

        double throughputMsg = num / durationSec;
        double bandwidthMBps = (num * size) / (durationSec * 1024 * 1024);
        double bandwidthGbps = (num * size * 8) / (durationSec * 1000 * 1000 * 1000);

        std::vector <uint64_t> sortedTimestamps = timestamps;
        std::sort (sortedTimestamps.begin (), sortedTimestamps.end ());

        std::vector <uint64_t> interArrivalTimes;
        interArrivalTimes.reserve (num - 1);

        for (uint64_t i = 1; i < num; ++i)
        {
            interArrivalTimes.push_back (sortedTimestamps[i] - sortedTimestamps[i-1]);
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

        std::cout << "\nConfiguration:" << std::endl;
        std::cout << "  Messages:      " << num << std::endl;
        std::cout << "  Message size:  " << size << " bytes" << std::endl;
        std::cout << "  Buffer size:   " << capacity << " slots" << std::endl;
        std::cout << "  Total data:    " << std::fixed << std::setprecision(2) << (num * size) / (1024.0 * 1024.0) << " MB" << std::endl;
        std::cout << "\nTiming:" << std::endl;
        std::cout << "  Total time:    " << std::fixed << std::setprecision(3) << durationMs << " ms" << std::endl;
        std::cout << "\nThroughput:" << std::endl;
        std::cout << "  Messages/sec:  " << std::fixed << std::setprecision(0) << throughputMsg << " msg/s" << std::endl;
        std::cout << "\nBandwidth:" << std::endl;
        std::cout << "  " << std::fixed << std::setprecision(2) << bandwidthMBps << " MB/s" << std::endl;
        std::cout << "  " << std::fixed << std::setprecision(2) << bandwidthGbps << " Gb/s" << std::endl;
        std::cout << "\nInter Time (nanoseconds):" << std::endl;
        std::cout << "  Minimum:       " << minTime << " ns" << std::endl;
        std::cout << "  Average:       " << std::fixed << std::setprecision(2) << avgTime << " ns" << std::endl;
        std::cout << "  Median (p50):  " << p50Time << " ns" << std::endl;
        std::cout << "  p90:           " << p90Time << " ns" << std::endl;
        std::cout << "  p95:           " << p95Time << " ns" << std::endl;
        std::cout << "  p99:           " << p99Time << " ns" << std::endl;
        std::cout << "  p99.9:         " << p999Time << " ns" << std::endl;
        std::cout << "  Maximum:       " << maxTime << " ns" << std::endl;
        std::cout << "\nInter Time (microseconds):" << std::endl;
        std::cout << "  Average:       " << std::fixed << std::setprecision(2) << avgTime / 1000.0 << " μs" << std::endl;
        std::cout << "  p50:           " << std::fixed << std::setprecision(2) << p50Time / 1000.0 << " μs" << std::endl;
        std::cout << "  p99:           " << std::fixed << std::setprecision(2) << p99Time / 1000.0 << " μs" << std::endl;
        std::cout << std::endl;
    }

    /// shared memory segment name.
    static const std::string _name;
};

const std::string MpmcBuffer::_name = "/test_mpmc_shm";

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    ::mlockall (MCL_CURRENT | MCL_FUTURE);
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
