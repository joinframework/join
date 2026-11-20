/**
 * MIT License
 *
 * Copyright (c) 2026 Mathieu Rabine
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
#include <join/io_fixed_buffer.hpp>

// Libraries.
#include <gtest/gtest.h>

// C++.
#include <system_error>

using join::Errc;
using join::IoFixedBuffer;

/**
 * @brief Test create method.
 */
TEST (IoFixedBuffer, create)
{
    io_uring ring;
    ASSERT_EQ (io_uring_queue_init (8, &ring, 0), 0);

    IoFixedBuffer<4, 256> buffer;
    ASSERT_NE (buffer.data (), nullptr);
    ASSERT_EQ (buffer.attach (&ring), 0) << join::lastError.message ();
    ASSERT_EQ (buffer.detach (), 0) << join::lastError.message ();

    IoFixedBuffer<4, 256> attached (&ring);
    ASSERT_NE (attached.data (), nullptr);
    ASSERT_EQ (attached.attach (&ring), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);

    ASSERT_THROW ((IoFixedBuffer<4, 256> (&ring)), std::system_error);

    ASSERT_EQ (attached.detach (), 0) << join::lastError.message ();

    io_uring_queue_exit (&ring);
}

/**
 * @brief Test attach method.
 */
TEST (IoFixedBuffer, attach)
{
    io_uring ring;
    ASSERT_EQ (io_uring_queue_init (8, &ring, 0), 0);

    IoFixedBuffer<4, 256> buffer;
    ASSERT_EQ (buffer.attach (&ring), 0) << join::lastError.message ();

    ASSERT_EQ (buffer.attach (&ring), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);

    IoFixedBuffer<4, 256> other;
    ASSERT_EQ (other.attach (&ring), -1);
    ASSERT_EQ (join::lastError, std::errc::device_or_resource_busy);

    ASSERT_EQ (buffer.detach (), 0) << join::lastError.message ();

    io_uring_queue_exit (&ring);
}

/**
 * @brief Test detach method.
 */
TEST (IoFixedBuffer, detach)
{
    io_uring ring;
    ASSERT_EQ (io_uring_queue_init (8, &ring, 0), 0);

    IoFixedBuffer<4, 256> buffer;
    ASSERT_EQ (buffer.detach (), 0);

    ASSERT_EQ (buffer.attach (&ring), 0) << join::lastError.message ();
    ASSERT_EQ (buffer.detach (), 0) << join::lastError.message ();
    ASSERT_EQ (buffer.detach (), 0);

    ASSERT_EQ (buffer.attach (&ring), 0) << join::lastError.message ();
    ASSERT_EQ (buffer.detach (), 0) << join::lastError.message ();

    {
        IoFixedBuffer<4, 256> other (&ring);
    }
    ASSERT_EQ (buffer.attach (&ring), 0) << join::lastError.message ();
    ASSERT_EQ (buffer.detach (), 0) << join::lastError.message ();

    io_uring_queue_exit (&ring);
}

/**
 * @brief Test acquire method.
 */
TEST (IoFixedBuffer, acquire)
{
    IoFixedBuffer<4, 256> buffer;
    char* first = buffer.acquire (256);
    ASSERT_NE (first, nullptr);
    char* second = buffer.acquire (1);
    ASSERT_NE (second, nullptr);
    ASSERT_NE (second, first);
    ASSERT_NE (buffer.acquire (256), nullptr);
    ASSERT_NE (buffer.acquire (256), nullptr);

    ASSERT_EQ (buffer.acquire (256), nullptr);
    ASSERT_EQ (join::lastError, Errc::OutOfMemory);

    buffer.release (first);
    join::lastError = {};
    ASSERT_EQ (buffer.acquire (257), nullptr);
    ASSERT_EQ (join::lastError, Errc::OutOfMemory);

    IoFixedBuffer<1, 64, 128> classes;
    char* small = classes.acquire (64);
    ASSERT_NE (small, nullptr);
    char* promoted = classes.acquire (64);
    ASSERT_NE (promoted, nullptr);
    ASSERT_NE (promoted, small);
    join::lastError = {};
    ASSERT_EQ (classes.acquire (64), nullptr);
    ASSERT_EQ (join::lastError, Errc::OutOfMemory);
}

/**
 * @brief Test release method.
 */
TEST (IoFixedBuffer, release)
{
    IoFixedBuffer<4, 256> buffer;
    char* first = buffer.acquire (256);
    ASSERT_NE (first, nullptr);
    ASSERT_NE (buffer.acquire (256), nullptr);
    ASSERT_NE (buffer.acquire (256), nullptr);
    ASSERT_NE (buffer.acquire (256), nullptr);
    ASSERT_EQ (buffer.acquire (256), nullptr);

    buffer.release (first);
    ASSERT_EQ (buffer.acquire (256), first);

    buffer.release (nullptr);
    ASSERT_EQ (buffer.acquire (256), nullptr);
}

/**
 * @brief Test owns method.
 */
TEST (IoFixedBuffer, owns)
{
    ASSERT_EQ ((IoFixedBuffer<4, 256>::length), 1024u);

    IoFixedBuffer<4, 256> buffer;
    char* base = static_cast<char*> (buffer.data ());
    ASSERT_TRUE (buffer.owns (base, IoFixedBuffer<4, 256>::length));
    ASSERT_TRUE (buffer.owns (base + 1023, 1));
    ASSERT_TRUE (buffer.owns (base + 1024, 0));
    ASSERT_FALSE (buffer.owns (base, IoFixedBuffer<4, 256>::length + 1));
    ASSERT_FALSE (buffer.owns (base + 1023, 2));
    ASSERT_FALSE (buffer.owns (base + 1025, 0));
    ASSERT_FALSE (buffer.owns (base - 1, 1));
    ASSERT_FALSE (buffer.owns (nullptr, 1));

    IoFixedBuffer<1, 64, 128> classes;
    ASSERT_NE (classes.acquire (64), nullptr);
    char* last = classes.acquire (128);
    ASSERT_NE (last, nullptr);
    ASSERT_TRUE (classes.owns (last, 128));
    ASSERT_FALSE (classes.owns (last, 129));
    ASSERT_TRUE (classes.owns (classes.data (), IoFixedBuffer<1, 64, 128>::length));
}

/**
 * @brief Test data method.
 */
TEST (IoFixedBuffer, data)
{
    IoFixedBuffer<4, 256> buffer;
    ASSERT_NE (buffer.data (), nullptr);
    ASSERT_EQ (buffer.acquire (256), buffer.data ());
}
