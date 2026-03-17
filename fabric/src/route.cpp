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
#include <join/routemanager.hpp>

using join::RouteKey;
using join::Route;
using join::RouteManager;
using join::IpAddress;

// =========================================================================
//   CLASS     : Route
//   METHOD    : Route
// =========================================================================
Route::Route (RouteManager* manager, const RouteKey& key)
: _manager (manager)
, _index (key._index)
, _dest (key._dest)
, _prefix (key._prefix)
{
}

// =========================================================================
//   CLASS     : Route
//   METHOD    : index
// =========================================================================
uint32_t Route::index () const noexcept
{
    return _index;
}

// =========================================================================
//   CLASS     : Route
//   METHOD    : dest
// =========================================================================
const IpAddress& Route::dest () const noexcept
{
    return _dest;
}

// =========================================================================
//   CLASS     : Route
//   METHOD    : prefix
// =========================================================================
uint32_t Route::prefix () const noexcept
{
    return _prefix;
}

// =========================================================================
//   CLASS     : Route
//   METHOD    : gateway
// =========================================================================
IpAddress Route::gateway () const
{
    ScopedLock<Mutex> lock (_mutex);
    return _gateway;
}

// =========================================================================
//   CLASS     : Route
//   METHOD    : metric
// =========================================================================
uint32_t Route::metric () const
{
    ScopedLock<Mutex> lock (_mutex);
    return _metric;
}

// =========================================================================
//   CLASS     : Route
//   METHOD    : type
// =========================================================================
uint8_t Route::type () const
{
    ScopedLock<Mutex> lock (_mutex);
    return _type;
}

// =========================================================================
//   CLASS     : Route
//   METHOD    : scope
// =========================================================================
uint8_t Route::scope () const
{
    ScopedLock<Mutex> lock (_mutex);
    return _scope;
}

// =========================================================================
//   CLASS     : Route
//   METHOD    : protocol
// =========================================================================
uint8_t Route::protocol () const
{
    ScopedLock<Mutex> lock (_mutex);
    return _protocol;
}

// =========================================================================
//   CLASS     : Route
//   METHOD    : isUnicast
// =========================================================================
bool Route::isUnicast () const noexcept
{
    return type () == RTN_UNICAST;
}

// =========================================================================
//   CLASS     : Route
//   METHOD    : isBlackhole
// =========================================================================
bool Route::isBlackhole () const noexcept
{
    return type () == RTN_BLACKHOLE;
}

// =========================================================================
//   CLASS     : Route
//   METHOD    : isUnreachable
// =========================================================================
bool Route::isUnreachable () const noexcept
{
    return type () == RTN_UNREACHABLE;
}

// =========================================================================
//   CLASS     : Route
//   METHOD    : isProhibit
// =========================================================================
bool Route::isProhibit () const noexcept
{
    return type () == RTN_PROHIBIT;
}

// =========================================================================
//   CLASS     : Route
//   METHOD    : isLocal
// =========================================================================
bool Route::isLocal () const noexcept
{
    return type () == RTN_LOCAL;
}

// =========================================================================
//   CLASS     : Route
//   METHOD    : isScopeUniverse
// =========================================================================
bool Route::isScopeUniverse () const noexcept
{
    return scope () == RT_SCOPE_UNIVERSE;
}

// =========================================================================
//   CLASS     : Route
//   METHOD    : isScopeLink
// =========================================================================
bool Route::isScopeLink () const noexcept
{
    return scope () == RT_SCOPE_LINK;
}

// =========================================================================
//   CLASS     : Route
//   METHOD    : isScopeHost
// =========================================================================
bool Route::isScopeHost () const noexcept
{
    return scope () == RT_SCOPE_HOST;
}

// =========================================================================
//   CLASS     : Route
//   METHOD    : isStatic
// =========================================================================
bool Route::isStatic () const noexcept
{
    return protocol () == RTPROT_STATIC;
}

// =========================================================================
//   CLASS     : Route
//   METHOD    : isKernel
// =========================================================================
bool Route::isKernel () const noexcept
{
    return protocol () == RTPROT_KERNEL;
}

// =========================================================================
//   CLASS     : Route
//   METHOD    : isDhcp
// =========================================================================
bool Route::isDhcp () const noexcept
{
    return protocol () == RTPROT_DHCP;
}

// =========================================================================
//   CLASS     : Route
//   METHOD    : isBoot
// =========================================================================
bool Route::isBoot () const noexcept
{
    return protocol () == RTPROT_BOOT;
}

// =========================================================================
//   CLASS     : Route
//   METHOD    : set
// =========================================================================
int Route::set (const IpAddress& gateway, uint32_t metric, bool sync) const
{
    return _manager->setRoute (_index, _dest, _prefix, gateway, metric, sync);
}

// =========================================================================
//   CLASS     : Route
//   METHOD    : remove
// =========================================================================
int Route::remove (bool sync) const
{
    return _manager->removeRoute (_index, _dest, _prefix, sync);
}
