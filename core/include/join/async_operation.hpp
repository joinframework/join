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

// C.
#include <sys/socket.h>
#include <cstddef>

namespace join
{
    /**
     * @brief asynchronous accept operation.
     */
    template <class Protocol, class Proactor>
    struct BasicAsyncAccept
    {
        using Endpoint = typename Protocol::Endpoint;
        using Socket = typename Protocol::Socket;

        /// handler invoked on completion.
        using Handler = Function<void (Socket&&, const std::error_code&)>;

        /// operation submitted to the proactor.
        IoOperation op = {};

        /// handler invoked on completion.
        Handler handler;

        /// remote endpoint.
        Endpoint remote;

        /// remote address length.
        socklen_t remoteLen = sizeof (struct sockaddr_storage);
    };

    /**
     * @brief asynchronous connect operation.
     */
    template <class Protocol, class Proactor>
    struct BasicAsyncConnect
    {
        /// handler invoked on completion.
        using Handler = Function<void (const std::error_code&)>;

        /// operation submitted to the proactor.
        IoOperation op = {};

        /// handler invoked on completion.
        Handler handler;
    };

    /**
     * @brief asynchronous read operation.
     */
    template <class Protocol, class Proactor>
    struct BasicAsyncRead
    {
        /// handler invoked on completion.
        using Handler = Function<void (const std::error_code&, size_t)>;

        /// operation submitted to the proactor.
        IoOperation op = {};

        /// handler invoked on completion.
        Handler handler;

        /// read message header.
        msghdr msg = {};

        /// read scatter gather entry.
        iovec iov = {};

        /// report an empty read as a closed connection.
        bool stream = false;
    };

    /**
     * @brief asynchronous write operation.
     */
    template <class Protocol, class Proactor>
    struct BasicAsyncWrite
    {
        /// handler invoked on completion.
        using Handler = Function<void (const std::error_code&, size_t)>;

        /// operation submitted to the proactor.
        IoOperation op = {};

        /// handler invoked on completion.
        Handler handler;

        /// write message header.
        msghdr msg = {};

        /// write scatter gather entry.
        iovec iov = {};
    };
}

#endif
