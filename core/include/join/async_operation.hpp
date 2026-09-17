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

#ifndef JOIN_CORE_ASYNC_OPERATION_HPP
#define JOIN_CORE_ASYNC_OPERATION_HPP

// libjoin.
#include <join/io_operation.hpp>
#include <join/function.hpp>
#include <join/protocol.hpp>

// C++.
#include <system_error>
#include <algorithm>

// C.
#include <sys/socket.h>
#include <cstddef>

namespace join
{
    /**
     * @brief asynchronous wait operation.
     */
    template <class Protocol, class Proactor>
    struct BasicAsyncWait
    {
        /// handler invoked on completion.
        using Wait = Function<void (const std::error_code&, bool), 16>;

        /// operation submitted to the proactor.
        IoOperation op = {};

        /// handler invoked on completion.
        Wait waitHandler;
    };

    /**
     * @brief asynchronous accept operation.
     */
    template <class Protocol, class Proactor>
    struct BasicAsyncAccept
    {
        using Endpoint = typename Protocol::Endpoint;
        using Socket = typename Protocol::Socket;

        /// handler invoked on completion.
        using Accept = Function<void (Socket&&, const std::error_code&, bool), 16>;

        /// operation submitted to the proactor.
        IoOperation op = {};

        /// handler invoked on completion.
        Accept acceptHandler;

        /// remote endpoint.
        Endpoint remote;

        /// remote address length.
        socklen_t remoteLen = sizeof (struct sockaddr_storage);
    };

    /**
     * @brief asynchronous read operation.
     */
    template <class Protocol, class Proactor>
    struct BasicAsyncRead
    {
        using Endpoint = typename Protocol::Endpoint;

        /// handler invoked on completion.
        using Read = Function<void (const std::error_code&, const char*, size_t, bool), 16>;

        /// handler invoked on completion, reporting the endpoint the data are coming from.
        using ReadFrom = Function<void (const std::error_code&, const char*, size_t, const Endpoint&, bool), 16>;

        /// operation submitted to the proactor.
        IoOperation op = {};

        /// handler invoked on completion.
        Read readHandler;

        /// handler invoked on completion, reporting the endpoint the data are coming from.
        ReadFrom readFromHandler;

        /// read message header.
        msghdr msg = {};

        /// read scatter gather entry.
        iovec iov = {};
    };

    /**
     * @brief asynchronous write operation.
     */
    template <class Protocol, class Proactor>
    struct BasicAsyncWrite
    {
        /// handler invoked on connection completion.
        using Connect = Function<void (const std::error_code&), 16>;

        /// handler invoked on completion.
        using Write = Function<void (const std::error_code&, size_t), 16>;

        /// operation submitted to the proactor.
        IoOperation op = {};

        /// handler invoked on connection completion.
        Connect connectHandler;

        /// handler invoked on completion.
        Write writeHandler;

        /// write message header.
        msghdr msg = {};

        /// write scatter gather entry.
        iovec iov = {};
    };

    /**
     * @brief asynchronous operation traits.
     */
    template <class... Ops>
    struct BasicAsyncOpTraits
    {
        static_assert (sizeof...(Ops) > 0, "traits must describe at least one operation");

        /// size of the largest asynchronous operation.
        static constexpr size_t maxSize = std::max ({sizeof (Ops)...});
    };

    /**
     * @brief asynchronous socket operation traits.
     */
    template <class Protocol, class Proactor>
    using AsyncOp = BasicAsyncOpTraits<BasicAsyncWait<Protocol, Proactor>, BasicAsyncRead<Protocol, Proactor>,
                                       BasicAsyncWrite<Protocol, Proactor>>;
}

#endif
