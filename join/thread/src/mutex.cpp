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
#include <join/mutex.hpp>

using join::Mutex;
using join::ScopedLock;

// =========================================================================
//   CLASS     : Mutex
//   METHOD    : Mutex
// =========================================================================
Mutex::Mutex ()
{
    pthread_mutexattr_init (&_attr);
    pthread_mutexattr_settype (&_attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init (&_handle, &_attr);
}

// =========================================================================
//   CLASS     : Mutex
//   METHOD    : ~Mutex
// =========================================================================
Mutex::~Mutex ()
{
    pthread_mutex_destroy (&_handle);
    pthread_mutexattr_destroy (&_attr);
}

// =========================================================================
//   CLASS     : Mutex
//   METHOD    : lock
// =========================================================================
void Mutex::lock ()
{
    pthread_mutex_lock (&_handle);
}

// =========================================================================
//   CLASS     : Mutex
//   METHOD    : tryLock
// =========================================================================
bool Mutex::tryLock ()
{
    return (pthread_mutex_trylock (&_handle) == 0);
}

// =========================================================================
//   CLASS     : Mutex
//   METHOD    : unlock
// =========================================================================
void Mutex::unlock ()
{
    pthread_mutex_unlock (&_handle);
}

// =========================================================================
//   CLASS     : ScopedLock
//   METHOD    : ScopedLock
// =========================================================================
ScopedLock::ScopedLock (Mutex& mutex)
: _mutex (mutex)
{
    _mutex.lock ();
}

// =========================================================================
//   CLASS     : ScopedLock
//   METHOD    : ~ScopedLock
// =========================================================================
ScopedLock::~ScopedLock ()
{
    _mutex.unlock ();
}
