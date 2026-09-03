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

// =========================================================================
//   CLASS     : IoRingBuffer
//   METHOD    : registerBuffer
// =========================================================================
template <size_t Count, size_t Size>
int join::IoRingBuffer::registerBuffer (uint16_t group, LocalMem::Allocator<Count, Size>& arena)
{
    static_assert (isPow2 (Count), "buffer count must be a power of two");

    if (JOIN_UNLIKELY (_base != nullptr))
    {
        lastError = make_error_code (Errc::InUse);
        return -1;
    }

    _mapping = LocalMem (Count * sizeof (io_uring_buf));

    if (JOIN_UNLIKELY (!arena.reserveAll ()))
    {
        lastError = make_error_code (Errc::InUse);
        return -1;
    }

    io_uring_buf_reg reg = {};
    reg.ring_addr = reinterpret_cast<uint64_t> (_mapping.get ());
    reg.ring_entries = Count;
    reg.bgid = group;

    int ret = io_uring_register_buf_ring (_uring, &reg, 0);
    if (JOIN_UNLIKELY (ret < 0))
    {
        arena.releaseAll ();
        _mapping = LocalMem ();
        lastError = std::error_code (-ret, std::system_category ());
        return -1;
    }

    _ring = static_cast<io_uring_buf_ring*> (_mapping.get ());
    _base = static_cast<char*> (arena.getPtr (0));
    _release = [&arena] () {
        arena.releaseAll ();
    };
    _size = Size;
    _count = Count;
    _group = group;

    io_uring_buf_ring_init (_ring);

    for (uint16_t bid = 0; bid < _count; ++bid)
    {
        io_uring_buf_ring_add (_ring, get (bid), _size, bid, static_cast<int> (_count - 1), bid);
    }

    io_uring_buf_ring_advance (_ring, static_cast<int> (_count));

    return 0;
}

// =========================================================================
//   CLASS     : IoRingBuffer
//   METHOD    : unregisterBuffer
// =========================================================================
inline int join::IoRingBuffer::unregisterBuffer () noexcept
{
    if (_base == nullptr)
    {
        return 0;
    }

    int ret = io_uring_unregister_buf_ring (_uring, _group);
    if (JOIN_UNLIKELY (ret < 0))
    {
        // LCOV_EXCL_START
        lastError = std::error_code (-ret, std::system_category ());
        return -1;
        // LCOV_EXCL_STOP
    }

    _release ();
    _release = nullptr;
    _mapping = LocalMem ();
    _ring = nullptr;
    _base = nullptr;
    _size = 0;
    _count = 0;
    _group = 0;

    return 0;
}

// =========================================================================
//   CLASS     : IoRingBuffer
//   METHOD    : recycle
// =========================================================================
inline void join::IoRingBuffer::recycle (uint16_t bid) noexcept
{
    io_uring_buf_ring_add (_ring, get (bid), _size, bid, static_cast<int> (_count - 1), 0);
    io_uring_buf_ring_advance (_ring, 1);
}
