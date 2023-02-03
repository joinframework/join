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

#ifndef __JOIN_CONDITION_HPP__
#define __JOIN_CONDITION_HPP__

// libjoin.
#include <join/error.hpp>
#include <join/mutex.hpp>

// C++.
#include <chrono>

namespace join
{
    /**
     * @brief condition variable class.
     */
    class Condition
    {
    public:
        /**
         * @brief default constructor.
         */
        Condition ();

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        Condition (const Condition& other) = delete;

        /**
         * @brief copy assignment.
         * @param other other object to copy.
         * @return a reference to the current object.
         */
        Condition& operator= (const Condition& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        Condition (Condition&& other) = delete;

        /**
         * @brief move assignment.
         * @param other other object to move.
         * @return a reference to the current object.
         */
        Condition& operator= (Condition&& other) = delete;

        /**
         * @brief destroys the mutex object.
         */
        ~Condition ();

        /**
         * @brief unblocks one of the waiting threads.
         */
        void signal () noexcept;

        /**
         * @brief unblocks all threads currently waiting.
         */
        void broadcast () noexcept;

        /**
         * @brief wait on a condition.
         * @param lock mutex previously locked by the calling thread.
         */
        void wait (ScopedLock& lock);

        /**
         * @brief wait on a condition with predicate.
         * @param lock mutex previously locked by the calling thread.
         * @param pred predicate.
         */
        template <class Predicate>
        void wait (ScopedLock& lock, Predicate pred)
        {
            while (!pred ())
            {
                wait (lock);
            }
        }

        /**
         * @brief wait on a condition until timeout expire.
         * @param lock mutex previously locked by the calling thread.
         * @param rt relative timeout.
         * @return true on success, false on timeout.
         */
        template <class Rep, class Period>
        bool timedWait (ScopedLock& lock, std::chrono::duration <Rep, Period> rt)
        {
            auto tp = std::chrono::steady_clock::now () + rt;
            auto secs = std::chrono::time_point_cast <std::chrono::seconds> (tp);
            auto ns = std::chrono::time_point_cast <std::chrono::nanoseconds> (tp) - std::chrono::time_point_cast <std::chrono::nanoseconds> (secs);
            struct timespec ts = {secs.time_since_epoch ().count (), ns.count ()};
            int err = pthread_cond_timedwait (&_handle, &lock._mutex._handle, &ts);
            if (err != 0)
            {
                lastError = std::make_error_code (static_cast <std::errc> (err));
                return false;
            }
            return true;
        }

        /**
         * @brief wait on a condition with predicate until timeout expire.
         * @param lock mutex previously locked by the calling thread.
         * @param rt relative timeout.
         * @param pred predicate.
         * @return true on success, false on timeout.
         */
        template <class Rep, class Period, class Predicate>
        bool timedWait (ScopedLock& lock, std::chrono::duration <Rep, Period> rt, Predicate pred)
        {
            while (!pred ())
            {
                if (!timedWait (lock, rt))
                {
                    return pred ();
                }
            }
            return true;
        }

    private:
        /// condition attributes.
        pthread_condattr_t _attr;

        /// condition handle.
        pthread_cond_t _handle;
    };
}

#endif
