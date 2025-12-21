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

#ifndef __JOIN_MUTEX_HPP__
#define __JOIN_MUTEX_HPP__

// C.
#include <pthread.h>

namespace join
{
    /**
     * @brief class used to protect shared data from being simultaneously accessed by multiple threads.
     */
    class Mutex
    {
    public:
        /**
         * @brief default constructor.
         */
        Mutex ();

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        Mutex (const Mutex& other) = delete;

        /**
         * @brief copy assignment.
         * @param other other object to copy.
         * @return a reference to the current object.
         */
        Mutex& operator= (const Mutex& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        Mutex (Mutex&& other) = delete;

        /**
         * @brief move assignment.
         * @param other other object to move.
         * @return a reference to the current object.
         */
        Mutex& operator= (Mutex&& other) = delete;

        /**
         * @brief destroys the mutex object.
         */
        ~Mutex ();

        /**
         * @brief lock the mutex.
         */
        void lock () noexcept;

        /**
         * @brief lock the mutex or return immediatly if the mutex is already locked.
         * @return true if locked, false otherwise.
         */
        bool tryLock () noexcept;

        /**
         * @brief unlock the mutex.
         */
        void unlock () noexcept;

        /**
         * @brief get native handle.
         * @return native handle.
         */
        pthread_mutex_t* handle () noexcept;

    private:
        /// mutex handle.
        pthread_mutex_t _handle;
    };

    /**
     * @brief class used to protect shared data from being simultaneously accessed by multiple threads.
     */
    class RecursiveMutex
    {
    public:
        /**
         * @brief default constructor.
         */
        RecursiveMutex ();

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        RecursiveMutex (const Mutex& other) = delete;

        /**
         * @brief copy assignment.
         * @param other other object to copy.
         * @return a reference to the current object.
         */
        RecursiveMutex& operator= (const RecursiveMutex& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        RecursiveMutex (RecursiveMutex&& other) = delete;

        /**
         * @brief move assignment.
         * @param other other object to move.
         * @return a reference to the current object.
         */
        RecursiveMutex& operator= (RecursiveMutex&& other) = delete;

        /**
         * @brief destroys the mutex object.
         */
        ~RecursiveMutex ();

        /**
         * @brief lock the mutex.
         */
        void lock () noexcept;

        /**
         * @brief lock the mutex or return immediatly if the mutex is already locked.
         * @return true if locked, false otherwise.
         */
        bool tryLock () noexcept;

        /**
         * @brief unlock the mutex.
         */
        void unlock () noexcept;

        /**
         * @brief get native handle.
         * @return native handle.
         */
        pthread_mutex_t* handle () noexcept;

    private:
        /// mutex handle.
        pthread_mutex_t _handle;
    };

    /**
     * @brief class used to protect shared data from being simultaneously accessed by multiple process via a shared memory.
     */
    class SharedMutex
    {
    public:
        /**
         * @brief default constructor.
         */
        SharedMutex ();

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        SharedMutex (const SharedMutex& other) = delete;

        /**
         * @brief copy assignment.
         * @param other other object to copy.
         * @return a reference to the current object.
         */
        SharedMutex& operator= (const SharedMutex& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        SharedMutex (SharedMutex&& other) = delete;

        /**
         * @brief move assignment.
         * @param other other object to move.
         * @return a reference to the current object.
         */
        SharedMutex& operator= (SharedMutex&& other) = delete;

        /**
         * @brief destroys the mutex object.
         */
        ~SharedMutex ();

        /**
         * @brief lock the mutex.
         */
        void lock () noexcept;

        /**
         * @brief lock the mutex or return immediatly if the mutex is already locked.
         * @return true if locked, false otherwise.
         */
        bool tryLock () noexcept;

        /**
         * @brief unlock the mutex.
         */
        void unlock () noexcept;

        /**
         * @brief get native handle.
         * @return native handle.
         */
        pthread_mutex_t* handle () noexcept;

    private:
        /// mutex handle.
        pthread_mutex_t _handle;
    };

    /**
     * @brief class owning a mutex for the duration of a scoped block.
     */
    template <typename Lockable>
    class ScopedLock
    {
    public:
        /**
         * @brief default constructor.
         */
        ScopedLock () = delete;

        /**
         * @brief acquires ownership of the given mutex.
         * @param mutex mutex to acquire ownership of.
         */
        explicit ScopedLock (Lockable& mutex)
        : _mutex (mutex)
        {
            _mutex.lock ();
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        ScopedLock (const ScopedLock& other) = delete;

        /**
         * @brief copy assignment.
         * @param other other object to copy.
         * @return a reference to the current object.
         */
        ScopedLock& operator= (const ScopedLock& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        ScopedLock (ScopedLock&& other) = delete;

        /**
         * @brief move assignment.
         * @param other other object to move.
         * @return a reference to the current object.
         */
        ScopedLock& operator= (ScopedLock&& other) = delete;

        /**
         * @brief releases the ownership of the owned mutex.
         */
        ~ScopedLock ()
        {
            _mutex.unlock ();
        }

        /**
         * @brief get associated mutex.
         * @return associated mutex.
         */
        Lockable* mutex () const
        {
            return &_mutex;
        }

    private:
        /// mutex owned for the duration of a scoped block.
        Lockable& _mutex;
    };
}

#endif
