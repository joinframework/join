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

#define _XOPEN_SOURCE 700

// libjoin.
#include <join/mutex.hpp>

// C.
#include <cerrno>

using join::Mutex;
using join::SharedMutex;
using join::ScopedLock;

// =========================================================================
//   CLASS     : Mutex
//   METHOD    : Mutex
// =========================================================================
Mutex::Mutex ()
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init (&attr);
    pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init (&_handle, &attr);
    pthread_mutexattr_destroy (&attr);
}

// =========================================================================
//   CLASS     : Mutex
//   METHOD    : ~Mutex
// =========================================================================
Mutex::~Mutex ()
{
    pthread_mutex_destroy (&_handle);
}

// =========================================================================
//   CLASS     : Mutex
//   METHOD    : lock
// =========================================================================
void Mutex::lock () noexcept
{
    pthread_mutex_lock (&_handle);
}

// =========================================================================
//   CLASS     : Mutex
//   METHOD    : tryLock
// =========================================================================
bool Mutex::tryLock () noexcept
{
    return (pthread_mutex_trylock (&_handle) == 0);
}

// =========================================================================
//   CLASS     : Mutex
//   METHOD    : unlock
// =========================================================================
void Mutex::unlock () noexcept
{
    pthread_mutex_unlock (&_handle);
}

// =========================================================================
//   CLASS     : Mutex
//   METHOD    : handle
// =========================================================================
pthread_mutex_t* Mutex::handle () noexcept
{
    return &_handle;
}

// =========================================================================
//   CLASS     : SharedMutex
//   METHOD    : SharedMutex
// =========================================================================
SharedMutex::SharedMutex ()
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init (&attr);
    pthread_mutexattr_setpshared (&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutexattr_setrobust (&attr, PTHREAD_MUTEX_ROBUST);
    pthread_mutex_init (&_handle, &attr);
    pthread_mutexattr_destroy (&attr);
}

// =========================================================================
//   CLASS     : SharedMutex
//   METHOD    : ~SharedMutex
// =========================================================================
SharedMutex::~SharedMutex ()
{
    pthread_mutex_destroy (&_handle);
}

// =========================================================================
//   CLASS     : SharedMutex
//   METHOD    : lock
// =========================================================================
void SharedMutex::lock () noexcept
{
    if (pthread_mutex_lock (&_handle) == EOWNERDEAD)
    {
        pthread_mutex_consistent (&_handle);
    }
}

// =========================================================================
//   CLASS     : SharedMutex
//   METHOD    : tryLock
// =========================================================================
bool SharedMutex::tryLock () noexcept
{
    int result = pthread_mutex_trylock (&_handle);
    if (result == EOWNERDEAD)
    {
        pthread_mutex_consistent (&_handle);
        return true;
    }
    return (result == 0);
}

// =========================================================================
//   CLASS     : SharedMutex
//   METHOD    : unlock
// =========================================================================
void SharedMutex::unlock () noexcept
{
    pthread_mutex_unlock (&_handle);
}

// =========================================================================
//   CLASS     : SharedMutex
//   METHOD    : handle
// =========================================================================
pthread_mutex_t* SharedMutex::handle () noexcept
{
    return &_handle;
}
