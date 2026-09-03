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
#include <join/io_ring_buffer.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::Errc;
using join::LocalMem;
using join::IoRingBuffer;

/**
 * @brief Class used to test IoRingBuffer.
 */
class IoRingBufferTest : public ::testing::Test
{
protected:
#ifdef JOIN_HAS_IO_URING
    /**
     * @brief Sets up the test fixture.
     */
    void SetUp () override
    {
        ASSERT_EQ (io_uring_queue_init (8, &_uring, 0), 0);
    }

    /**
     * @brief Tears down the test fixture.
     */
    void TearDown () override
    {
        io_uring_queue_exit (&_uring);
    }
#endif

    /// arena owning the buffers.
    LocalMem::Allocator<4, 256> _arena;

#ifdef JOIN_HAS_IO_URING
    /// io_uring instance.
    io_uring _uring = {};

    /// buffer ring under test.
    IoRingBuffer _ring{&_uring};
#else
    /// buffer ring under test.
    IoRingBuffer _ring;
#endif
};

/**
 * @brief Test registerBuffer.
 */
TEST_F (IoRingBufferTest, registerBuffer)
{
    ASSERT_NE (_arena.tryAllocate (256), nullptr);
    ASSERT_EQ (_ring.registerBuffer (3, _arena), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);

    _arena.releaseAll ();

    ASSERT_EQ (_ring.registerBuffer (3, _arena), 0) << join::lastError.message ();
    ASSERT_EQ (_arena.tryAllocate (256), nullptr);

    ASSERT_EQ (_ring.registerBuffer (3, _arena), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);

#ifdef JOIN_HAS_IO_URING
    LocalMem::Allocator<4, 256> arena;
    IoRingBuffer ring (&_uring);

    ASSERT_EQ (ring.registerBuffer (3, arena), -1);
    ASSERT_NE (arena.tryAllocate (256), nullptr);
#endif
}

/**
 * @brief Test unregisterBuffer.
 */
TEST_F (IoRingBufferTest, unregisterBuffer)
{
    ASSERT_EQ (_ring.unregisterBuffer (), 0);

    ASSERT_EQ (_ring.registerBuffer (3, _arena), 0) << join::lastError.message ();
    ASSERT_EQ (_ring.unregisterBuffer (), 0) << join::lastError.message ();
    ASSERT_NE (_arena.tryAllocate (256), nullptr);
}

#ifndef JOIN_HAS_IO_URING
/**
 * @brief Test select.
 */
TEST_F (IoRingBufferTest, select)
{
    ASSERT_EQ (_ring.registerBuffer (0, _arena), 0) << join::lastError.message ();

    for (uint16_t bid = 0; bid < _ring.count (); ++bid)
    {
        ASSERT_EQ (_ring.select (), bid);
    }

    ASSERT_EQ (_ring.select (), -1);
}

/**
 * @brief Test recycle.
 */
TEST_F (IoRingBufferTest, recycle)
{
    ASSERT_EQ (_ring.registerBuffer (0, _arena), 0) << join::lastError.message ();

    for (uint16_t bid = 0; bid < _ring.count (); ++bid)
    {
        ASSERT_EQ (_ring.select (), bid);
    }
    ASSERT_EQ (_ring.select (), -1);

    _ring.recycle (2);
    ASSERT_EQ (_ring.select (), 2);

    _ring.recycle (1);
    _ring.recycle (3);
    ASSERT_EQ (_ring.select (), 3);
    ASSERT_EQ (_ring.select (), 1);
    ASSERT_EQ (_ring.select (), -1);
}
#endif

/**
 * @brief Test bind.
 */
TEST_F (IoRingBufferTest, bind)
{
    _ring.bind ();
    ASSERT_TRUE (_ring.armed ());

    _ring.bind ();
    _ring.unbind ();
    ASSERT_TRUE (_ring.armed ());

    _ring.unbind ();
}

/**
 * @brief Test unbind.
 */
TEST_F (IoRingBufferTest, unbind)
{
    _ring.bind ();
    _ring.unbind ();
    ASSERT_FALSE (_ring.armed ());
}

/**
 * @brief Test armed.
 */
TEST_F (IoRingBufferTest, armed)
{
    ASSERT_FALSE (_ring.armed ());

    _ring.bind ();
    ASSERT_TRUE (_ring.armed ());

    _ring.unbind ();
    ASSERT_FALSE (_ring.armed ());
}

/**
 * @brief Test get.
 */
TEST_F (IoRingBufferTest, get)
{
    ASSERT_EQ (_ring.registerBuffer (0, _arena), 0) << join::lastError.message ();

    for (uint16_t bid = 0; bid < _ring.count (); ++bid)
    {
        ASSERT_EQ (_ring.get (bid), _arena.getPtr (bid)) << bid;
    }
}

/**
 * @brief Test group.
 */
TEST_F (IoRingBufferTest, group)
{
    ASSERT_EQ (_ring.group (), 0);

    ASSERT_EQ (_ring.registerBuffer (3, _arena), 0) << join::lastError.message ();
    ASSERT_EQ (_ring.group (), 3);

    ASSERT_EQ (_ring.unregisterBuffer (), 0) << join::lastError.message ();
    ASSERT_EQ (_ring.group (), 0);
}

/**
 * @brief Test size.
 */
TEST_F (IoRingBufferTest, size)
{
    ASSERT_EQ (_ring.size (), 0);

    ASSERT_EQ (_ring.registerBuffer (0, _arena), 0) << join::lastError.message ();
    ASSERT_EQ (_ring.size (), 256);

    ASSERT_EQ (_ring.unregisterBuffer (), 0) << join::lastError.message ();
    ASSERT_EQ (_ring.size (), 0);
}

/**
 * @brief Test count.
 */
TEST_F (IoRingBufferTest, count)
{
    ASSERT_EQ (_ring.count (), 0);

    ASSERT_EQ (_ring.registerBuffer (0, _arena), 0) << join::lastError.message ();
    ASSERT_EQ (_ring.count (), 4);

    ASSERT_EQ (_ring.unregisterBuffer (), 0) << join::lastError.message ();
    ASSERT_EQ (_ring.count (), 0);
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
