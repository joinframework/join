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

#ifndef __JOIN_SEMAPHORE_HPP__
#define __JOIN_SEMAPHORE_HPP__

// libjoin.
#include <join/error.hpp>
#include <join/utils.hpp>

// C++.
#include <chrono>

// C.
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <climits>

namespace join
{
    /**
     * @brief class used to protect shared data from being simultaneously accessed by multiple threads.
     */
    class Semaphore
    {
    public:
        /**
         * @brief create an named semaphore.
         * @param value intial value.
         */
        Semaphore (size_t value = 0);

        /**
         * @brief create a named semaphore.
         * @param name semaphore name.
         * @param value intial value.
         * @param oflag control flag.
         * @param mode open mode.
         */
        Semaphore (const std::string& name, size_t value = 0, int oflag = O_CREAT, mode_t mode = 0644);

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        Semaphore (const Semaphore& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        Semaphore& operator= (const Semaphore& other) = delete;

        /**
         * @brief destroy instance.
         */
        ~Semaphore () noexcept;

        /**
         * @brief increments the internal counter and unblocks acquirers.
         */
        void post () noexcept;

        /**
         * @brief decrements the internal counter or blocks until it can
         */
        void wait () noexcept;

        /**
         * @brief tries to decrement the internal counter without blocking.
         * @return true on success, false otherwise.
         */
        bool tryWait () noexcept;

        /**
         * @brief tries to decrement the internal counter, blocking for up to a duration time.
         * @param timeout timeout.
         * @return true on success, false otherwise.
         */
        template <class Rep, class Period>
        bool timedWait (std::chrono::duration <Rep, Period> timeout)
        {
            struct timespec ts = toTimespec (std::chrono::system_clock::now () + timeout);
            if (((_named) ? ::sem_timedwait (_named_handle, &ts) : ::sem_timedwait (&_unnamed_handle, &ts)) == -1)
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                return false;
            }

            return true;
        }

        /**
         * @brief get semaphore value.
         * @return -1 on error, semaphore value otherwise.
         */
        int value () noexcept;

    private:
        /// semaphore handle.
        union {
            sem_t* _named_handle;
            sem_t _unnamed_handle;
        };

        /// is semaphore named.
        bool _named = false;
    };

    /**
     * @brief class used to protect shared data from being simultaneously accessed by multiple process via a shared memory.
     */
    class SharedSemaphore
    {
    public:
        /**
         * @brief create instance.
         * @param value intial value.
         */
        SharedSemaphore (size_t value = 0);

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        SharedSemaphore (const SharedSemaphore& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        SharedSemaphore& operator= (const SharedSemaphore& other) = delete;

        /**
         * @brief destroy instance.
         */
        ~SharedSemaphore () noexcept;

        /**
         * @brief increments the internal counter and unblocks acquirers.
         */
        void post () noexcept;

        /**
         * @brief decrements the internal counter or blocks until it can
         */
        void wait () noexcept;

        /**
         * @brief tries to decrement the internal counter without blocking.
         * @return true on success, false otherwise.
         */
        bool tryWait () noexcept;

        /**
         * @brief tries to decrement the internal counter, blocking for up to a duration time.
         * @param timeout timeout.
         * @return true on success, false otherwise.
         */
        template <class Rep, class Period>
        bool timedWait (std::chrono::duration <Rep, Period> timeout)
        {
            struct timespec ts = toTimespec (std::chrono::system_clock::now () + timeout);
            if (sem_timedwait (&_handle, &ts) == -1)
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                return false;
            }

            return true;
        }

        /**
         * @brief get semaphore value.
         * @return -1 on error, semaphore value otherwise.
         */
        int value () noexcept;

    private:
        /// handle.
        sem_t _handle;
    };
}

#endif
