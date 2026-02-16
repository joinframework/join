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
#include <join/reactorpool.hpp>
#include <join/cpu.hpp>

// C.
#include <unistd.h>

using join::Reactor;
using join::ReactorPool;

// =========================================================================
//   CLASS     : ReactorPool
//   METHOD    : ReactorPool
// =========================================================================
ReactorPool::ReactorPool ()
{
    const auto& cores = CpuTopology::instance ()->cores ();

    if (cores.empty ())
    {
        _reactors.emplace_back (std::make_unique <Reactor> ());
        return;
    }

    _reactors.reserve (cores.size ());

    for (const auto& core : cores)
    {
        int primary = core.primaryThread ();
        if (primary != -1)
        {
            _reactors.emplace_back (std::make_unique <Reactor> (primary));
        }
    }
}

// =========================================================================
//   CLASS     : ReactorPool
//   METHOD    : addHandler
// =========================================================================
int ReactorPool::addHandler (EventHandler* handler, bool sync) noexcept
{
    if (JOIN_UNLIKELY (handler == nullptr))
    {
        lastError = make_error_code (Errc::InvalidParam);
        return -1;
    }

    if (JOIN_UNLIKELY (handler->handle () < 0))
    {
        lastError = std::make_error_code (std::errc::bad_file_descriptor);
        return -1;
    }

    size_t index = _next.fetch_add (1, std::memory_order_relaxed) % _reactors.size ();
    handler->reactorIndex (static_cast <int> (index));

    int result = _reactors[index]->addHandler (handler, sync);
    if (JOIN_UNLIKELY (result != 0))
    {
        handler->reactorIndex (-1);
    }

    return result;
}

// =========================================================================
//   CLASS     : ReactorPool
//   METHOD    : delHandler
// =========================================================================
int ReactorPool::delHandler (EventHandler* handler, bool sync) noexcept
{
    if (JOIN_UNLIKELY (handler == nullptr))
    {
        lastError = make_error_code (Errc::InvalidParam);
        return -1;
    }

    int index = handler->reactorIndex ();
    if (JOIN_UNLIKELY (index < 0 || index >= static_cast <int> (_reactors.size ())))
    {
        lastError = make_error_code (Errc::NotFound);
        return -1;
    }

    int result = _reactors[index]->delHandler (handler, sync);
    if (JOIN_LIKELY (result == 0))
    {
        handler->reactorIndex (-1);
    }

    return result;
}

// =========================================================================
//   CLASS     : ReactorPool
//   METHOD    : instance
// =========================================================================
ReactorPool* ReactorPool::instance () noexcept
{
    static ReactorPool pool;
    return &pool;
}
