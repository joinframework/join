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

// C++.
#include <string>

// C.
#include <cerrno>

using join::Semaphore;
using join::SharedSemaphore;

// =========================================================================
//   CLASS     : Semaphore
//   METHOD    : Semaphore
// =========================================================================
Semaphore::Semaphore (size_t value)
{
    if (::sem_init (&_unnamed_handle, 0, value) == -1)
    {
        throw std::system_error (errno, std::system_category (), "sem_init failed");
    }
}

// =========================================================================
//   CLASS     : Semaphore
//   METHOD    : Semaphore
// =========================================================================
Semaphore::Semaphore (const std::string& name, size_t value, int oflag, mode_t mode)
: _named (true)
{
    _named_handle = ::sem_open (name.c_str (), oflag, mode, value);
    if (_named_handle == SEM_FAILED)
    {
        throw std::system_error (errno, std::system_category (), "sem_open failed");
    }
}

// =========================================================================
//   CLASS     : Semaphore
//   METHOD    : ~Semaphore
// =========================================================================
Semaphore::~Semaphore () noexcept
{
    if (_named)
    {
        ::sem_close (_named_handle);
    }
    else
    {
        ::sem_destroy (&_unnamed_handle);
    }
}

// =========================================================================
//   CLASS     : Semaphore
//   METHOD    : post
// =========================================================================
void Semaphore::post () noexcept
{
    if (_named)
    {
        ::sem_post (_named_handle);
    }
    else
    {
        ::sem_post (&_unnamed_handle);
    }
}

// =========================================================================
//   CLASS     : Semaphore
//   METHOD    : wait
// =========================================================================
void Semaphore::wait () noexcept
{
    if (_named)
    {
        ::sem_wait (_named_handle);
    }
    else
    {
        ::sem_wait (&_unnamed_handle);
    }
}

// =========================================================================
//   CLASS     : Semaphore
//   METHOD    : tryWait
// =========================================================================
bool Semaphore::tryWait () noexcept
{
    int res = (_named) ? ::sem_trywait (_named_handle) : ::sem_trywait (&_unnamed_handle);
    if (res == -1)
    {
        lastError = std::make_error_code (static_cast <std::errc> (errno));
    }
    return (res == 0);
}

// =========================================================================
//   CLASS     : Semaphore
//   METHOD    : value
// =========================================================================
int Semaphore::value () noexcept
{
    int value = -1;
    if (_named)
    {
        ::sem_getvalue (_named_handle, &value);
    }
    else
    {
        ::sem_getvalue (&_unnamed_handle, &value);
    }
    return value;
}

// =========================================================================
//   CLASS     : SharedSemaphore
//   METHOD    : post
// =========================================================================
SharedSemaphore::SharedSemaphore (size_t value)
{
    if (::sem_init (&_handle, 1, value) == -1)
    {
        throw std::system_error (errno, std::system_category(), "sem_init failed");
    }
}

// =========================================================================
//   CLASS     : SharedSemaphore
//   METHOD    : ~SharedSemaphore
// =========================================================================
SharedSemaphore::~SharedSemaphore () noexcept
{
    ::sem_destroy (&_handle);
}

// =========================================================================
//   CLASS     : SharedSemaphore
//   METHOD    : post
// =========================================================================
void SharedSemaphore::post () noexcept
{
    ::sem_post (&_handle);
}

// =========================================================================
//   CLASS     : SharedSemaphore
//   METHOD    : wait
// =========================================================================
void SharedSemaphore::wait () noexcept
{
    ::sem_wait (&_handle);
}

// =========================================================================
//   CLASS     : SharedSemaphore
//   METHOD    : tryWait
// =========================================================================
bool SharedSemaphore::tryWait () noexcept
{
    if (::sem_trywait (&_handle) == -1)
    {
        lastError = std::make_error_code (static_cast <std::errc> (errno));
        return false;
    }
    return true;
}

// =========================================================================
//   CLASS     : SharedSemaphore
//   METHOD    : value
// =========================================================================
int SharedSemaphore::value () noexcept
{
    int value = -1;
    ::sem_getvalue (&_handle, &value);
    return value;
}
