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
#include <join/io_descriptor.hpp>
#include <join/proactor.hpp>

// C.
#include <sys/epoll.h>

using join::IoDescriptor;

// =========================================================================
//   CLASS     : IoDescriptor
//   METHOD    : ~IoDescriptor
// =========================================================================
IoDescriptor::~IoDescriptor () noexcept
{
    if ((_proactor != nullptr) && (_proactor->_dispatching == this))
    {
        _proactor->_dispatching = nullptr;
    }
}

// =========================================================================
//   CLASS     : IoDescriptor
//   METHOD    : onEvent
// =========================================================================
void IoDescriptor::onEvent ([[maybe_unused]] int fd, uint32_t revents) noexcept
{
    BasicProactor* proactor = _proactor;
    proactor->_dispatching = this;

    const bool closing = (revents & (EPOLLERR | EPOLLRDHUP | EPOLLHUP)) != 0;

    if (JOIN_UNLIKELY (closing && (_writeOps != nullptr) &&
                       (_writeOps->next->code == static_cast<uint8_t> (IoOperation::Opcode::Connect))))
    {
        proactor->processQueue (*this, true, revents);
    }
    else
    {
        const bool reading = closing && (_readOps != nullptr);

        if (closing || (revents & EPOLLIN))
        {
            proactor->processQueue (*this, false, revents);
        }

        if ((proactor->_dispatching == this) && !reading && (revents & (EPOLLOUT | EPOLLERR | EPOLLHUP)))
        {
            proactor->processQueue (*this, true, revents);
        }
    }

    if (proactor->_dispatching == this)
    {
        proactor->_dispatching = nullptr;

        if (empty ())
        {
            _proactor = nullptr;
        }
    }
}
