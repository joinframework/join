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

    if (JOIN_UNLIKELY (!arena.reserveAll ()))
    {
        lastError = make_error_code (Errc::InUse);
        return -1;
    }

    _base = static_cast<char*> (arena.getPtr (0));
    _release = [&arena] () {
        arena.releaseAll ();
    };
    _size = Size;
    _count = Count;
    _group = group;
    _free = 0;

    for (uint16_t bid = 0; bid < _count - 1; ++bid)
    {
        *static_cast<uint16_t*> (get (bid)) = bid + 1;
    }

    *static_cast<uint16_t*> (get (_count - 1)) = _none;

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

    _release ();
    _release = nullptr;
    _base = nullptr;
    _size = 0;
    _count = 0;
    _group = 0;
    _free = _none;

    return 0;
}

// =========================================================================
//   CLASS     : IoRingBuffer
//   METHOD    : select
// =========================================================================
inline int join::IoRingBuffer::select () noexcept
{
    if (JOIN_UNLIKELY (_free == _none))
    {
        return -1;
    }

    uint16_t bid = _free;
    _free = *static_cast<uint16_t*> (get (bid));

    return bid;
}

// =========================================================================
//   CLASS     : IoRingBuffer
//   METHOD    : recycle
// =========================================================================
inline void join::IoRingBuffer::recycle (uint16_t bid) noexcept
{
    *static_cast<uint16_t*> (get (bid)) = _free;
    _free = bid;
}
