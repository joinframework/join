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
#include <join/neighbormanager.hpp>

using join::NeighborKey;
using join::Neighbor;
using join::NeighborManager;
using join::MacAddress;
using join::IpAddress;

// =========================================================================
//   CLASS     : Neighbor
//   METHOD    : Neighbor
// =========================================================================
Neighbor::Neighbor (NeighborManager* manager, const NeighborKey& key)
: _manager (manager)
, _index (key._index)
, _ip (key._ip)
{
}

// =========================================================================
//   CLASS     : Neighbor
//   METHOD    : index
// =========================================================================
uint32_t Neighbor::index () const noexcept
{
    return _index;
}

// =========================================================================
//   CLASS     : Neighbor
//   METHOD    : ip
// =========================================================================
const IpAddress& Neighbor::ip () const noexcept
{
    return _ip;
}

// =========================================================================
//   CLASS     : Neighbor
//   METHOD    : mac
// =========================================================================
MacAddress Neighbor::mac () const
{
    ScopedLock<Mutex> lock (_mutex);
    return _mac;
}

// =========================================================================
//   CLASS     : Neighbor
//   METHOD    : state
// =========================================================================
uint16_t Neighbor::state () const
{
    ScopedLock<Mutex> lock (_mutex);
    return _state;
}

// =========================================================================
//   CLASS     : Neighbor
//   METHOD    : isReachable
// =========================================================================
bool Neighbor::isReachable () const noexcept
{
    return (state () & NUD_REACHABLE) != 0;
}

// =========================================================================
//   CLASS     : Neighbor
//   METHOD    : isStale
// =========================================================================
bool Neighbor::isStale () const noexcept
{
    return (state () & NUD_STALE) != 0;
}

// =========================================================================
//   CLASS     : Neighbor
//   METHOD    : isPermanent
// =========================================================================
bool Neighbor::isPermanent () const noexcept
{
    return (state () & NUD_PERMANENT) != 0;
}

// =========================================================================
//   CLASS     : Neighbor
//   METHOD    : isIncomplete
// =========================================================================
bool Neighbor::isIncomplete () const noexcept
{
    return (state () & NUD_INCOMPLETE) != 0;
}

// =========================================================================
//   CLASS     : Neighbor
//   METHOD    : isFailed
// =========================================================================
bool Neighbor::isFailed () const noexcept
{
    return (state () & NUD_FAILED) != 0;
}

// =========================================================================
//   CLASS     : Neighbor
//   METHOD    : set
// =========================================================================
int Neighbor::set (const MacAddress& macAddress, uint16_t state, bool sync) const
{
    return _manager->setNeighbor (_index, _ip, macAddress, state, sync);
}

// =========================================================================
//   CLASS     : Neighbor
//   METHOD    : remove
// =========================================================================
int Neighbor::remove (bool sync) const
{
    return _manager->removeNeighbor (_index, _ip, sync);
}
