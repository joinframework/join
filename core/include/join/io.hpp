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

#ifndef JOIN_CORE_IO_HPP
#define JOIN_CORE_IO_HPP

#ifdef JOIN_HAS_IO_URING
#include <join/proactor.hpp>
#else
#include <join/reactor.hpp>
#endif

namespace join
{
#ifdef JOIN_HAS_IO_URING
    /// completion handler.
    using IoHandler = CompletionHandler;

    /// proactor engine.
    using IoEngine = Proactor;

    /// default thread managing the proactor.
    using IoThread = ProactorThread;
#else
    /// event handler.
    using IoHandler = EventHandler;

    /// reactor engine.
    using IoEngine = Reactor;

    /// default thread managing the reactor.
    using IoThread = ReactorThread;
#endif

    /**
     * @brief Get the default I/O engine.
     */
    inline IoEngine* defaultIoEngine ()
    {
#ifdef JOIN_HAS_IO_URING
        return IoThread::proactor ();
#else
        return IoThread::reactor ();
#endif
    }
}

#endif
