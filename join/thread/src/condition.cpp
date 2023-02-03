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

// C.
#include <ctime>

using join::ScopedLock;
using join::Condition;

// =========================================================================
//   CLASS     : Condition
//   METHOD    : Condition
// =========================================================================
Condition::Condition ()
{
    pthread_condattr_init (&_attr);
    pthread_condattr_setclock (&_attr, CLOCK_MONOTONIC);
    pthread_cond_init (&_handle, &_attr);
}

// =========================================================================
//   CLASS     : Condition
//   METHOD    : ~Condition
// =========================================================================
Condition::~Condition ()
{
    pthread_cond_destroy (&_handle);
    pthread_condattr_destroy (&_attr);
}

// =========================================================================
//   CLASS     : Condition
//   METHOD    : signal
// =========================================================================
void Condition::signal () noexcept
{
    pthread_cond_signal (&_handle);
}

// =========================================================================
//   CLASS     : Condition
//   METHOD    : broadcast
// =========================================================================
void Condition::broadcast () noexcept
{
    pthread_cond_broadcast (&_handle);
}

// =========================================================================
//   CLASS     : Condition
//   METHOD    : wait
// =========================================================================
void Condition::wait (ScopedLock& lock)
{
    pthread_cond_wait (&_handle, &lock._mutex._handle);
}
