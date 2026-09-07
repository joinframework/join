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
#include <join/async_operation.hpp>
#include <join/datagram_socket.hpp>
#include <join/protocol.hpp>

// Libraries.
#include <gtest/gtest.h>

// C++.
#include <memory>
#include <chrono>
#include <thread>

using join::Errc;
using join::BasicAsyncOperation;
using join::BasicAsyncRead;
using join::BasicAsyncWrite;
using join::IoOperation;
using join::Udp;

/**
 * @brief engine stub reporting whether the caller runs on the proactor thread.
 */
struct Engine
{
    bool proactor = false;

    bool isProactorThread () const noexcept
    {
        return proactor;
    }
};

/**
 * @brief exposes the protected interface of the operation under test.
 */
struct AsyncOperation : public BasicAsyncOperation<Udp>
{
    using BasicAsyncOperation<Udp>::op;
    using BasicAsyncOperation<Udp>::state;
};

/**
 * @brief exposes the protected interface of the read operation under test.
 */
struct AsyncRead : public BasicAsyncRead<Udp>
{
    using BasicAsyncRead<Udp>::onComplete;
    using BasicAsyncRead<Udp>::onCancel;
    using BasicAsyncRead<Udp>::handler;
    using BasicAsyncRead<Udp>::msg;
    using BasicAsyncRead<Udp>::iov;
    using BasicAsyncRead<Udp>::stream;
    using BasicAsyncOperation<Udp>::op;
    using BasicAsyncOperation<Udp>::state;
};

/**
 * @brief exposes the protected interface of the write operation under test.
 */
struct AsyncWrite : public BasicAsyncWrite<Udp>
{
    using BasicAsyncWrite<Udp>::onComplete;
    using BasicAsyncWrite<Udp>::onCancel;
    using BasicAsyncWrite<Udp>::handler;
    using BasicAsyncWrite<Udp>::connectHandler;
    using BasicAsyncWrite<Udp>::msg;
    using BasicAsyncWrite<Udp>::iov;
    using BasicAsyncWrite<Udp>::remote;
    using BasicAsyncWrite<Udp>::socket;
    using BasicAsyncOperation<Udp>::op;
    using BasicAsyncOperation<Udp>::state;
};

/**
 * @brief Test operator new.
 */
TEST (AsyncOperation, new)
{
    std::unique_ptr<AsyncOperation> operation (new AsyncOperation ());
    ASSERT_NE (operation, nullptr);
    ASSERT_EQ (reinterpret_cast<uintptr_t> (operation.get ()) % alignof (AsyncOperation), 0);

    std::unique_ptr<AsyncRead> read (new AsyncRead ());
    ASSERT_EQ (reinterpret_cast<uintptr_t> (read.get ()) % alignof (AsyncRead), 0);

    std::unique_ptr<AsyncWrite> write (new AsyncWrite ());
    ASSERT_EQ (reinterpret_cast<uintptr_t> (write.get ()) % alignof (AsyncWrite), 0);
}

/**
 * @brief Test operator delete.
 */
TEST (AsyncOperation, delete)
{
    AsyncOperation* operation = new AsyncOperation ();
    ASSERT_NO_THROW (delete operation);

    BasicAsyncOperation<Udp>* read = new AsyncRead ();
    ASSERT_NO_THROW (delete read);

    BasicAsyncOperation<Udp>* write = new AsyncWrite ();
    ASSERT_NO_THROW (delete write);
}

/**
 * @brief Test reserve.
 */
TEST (AsyncOperation, reserve)
{
    Engine engine;
    AsyncOperation operation;

    ASSERT_EQ (operation.reserve (engine), 0) << join::lastError.message ();
    ASSERT_EQ (operation.state.load (), AsyncOperation::Pending);

    ASSERT_EQ (operation.reserve (engine), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    ASSERT_EQ (operation.state.load (), AsyncOperation::Pending);

    operation.state.store (AsyncOperation::Dispatching);
    ASSERT_EQ (operation.reserve (engine), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    ASSERT_EQ (operation.state.load (), AsyncOperation::Dispatching);

    engine.proactor = true;
    ASSERT_EQ (operation.reserve (engine), 0) << join::lastError.message ();
    ASSERT_EQ (operation.state.load (), AsyncOperation::Pending);

    operation.state.store (AsyncOperation::Closing);
    ASSERT_EQ (operation.reserve (engine), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
}

/**
 * @brief Test release.
 */
TEST (AsyncOperation, release)
{
    AsyncOperation operation;

    operation.state.store (AsyncOperation::Pending);
    operation.release ();
    ASSERT_EQ (operation.state.load (), AsyncOperation::Idle);

    operation.state.store (AsyncOperation::Closing);
    operation.release ();
    ASSERT_EQ (operation.state.load (), AsyncOperation::Idle);
}

/**
 * @brief Test drain.
 */
TEST (AsyncOperation, drain)
{
    int cancelled = 0;

    AsyncOperation operation;

    auto cancel = [&operation, &cancelled] () {
        ++cancelled;
        operation.state.store (AsyncOperation::Dispatching);
    };

    operation.drain (cancel);
    ASSERT_EQ (operation.state.load (), AsyncOperation::Closing);
    ASSERT_EQ (cancelled, 0);

    operation.state.store (AsyncOperation::Pending);

    std::thread waiter ([&operation] () {
        while (operation.state.load () == AsyncOperation::Pending)
        {
            std::this_thread::yield ();
        }

        std::this_thread::sleep_for (std::chrono::milliseconds (2));
        operation.state.store (AsyncOperation::Idle);
    });

    operation.drain (cancel);
    waiter.join ();

    ASSERT_EQ (operation.state.load (), AsyncOperation::Closing);
    ASSERT_EQ (cancelled, 1);
}

/**
 * @brief Test dispatch.
 */
TEST (AsyncOperation, dispatch)
{
    AsyncOperation::State observed = AsyncOperation::Idle;
    bool rearm = false;

    AsyncOperation operation;

    auto handler = [&operation, &observed, &rearm] () {
        observed = operation.state.load ();

        if (rearm)
        {
            operation.state.store (AsyncOperation::Pending);
        }
    };

    operation.dispatch (handler);
    ASSERT_EQ (observed, AsyncOperation::Dispatching);
    ASSERT_EQ (operation.state.load (), AsyncOperation::Idle);

    rearm = true;
    operation.dispatch (handler);
    ASSERT_EQ (operation.state.load (), AsyncOperation::Pending);
}

/**
 * @brief Test op.
 */
TEST (AsyncOperation, op)
{
    AsyncOperation operation;

    ASSERT_EQ (operation.op.handler, nullptr);
    ASSERT_EQ (operation.op.state, IoOperation::State::Idle);
}

/**
 * @brief Test state.
 */
TEST (AsyncOperation, state)
{
    AsyncOperation operation;

    ASSERT_EQ (operation.state.load (), AsyncOperation::Idle);

    operation.state.store (AsyncOperation::Pending);
    ASSERT_EQ (operation.state.load (), AsyncOperation::Pending);
}

/**
 * @brief Test onComplete.
 */
TEST (AsyncRead, onComplete)
{
    std::error_code code;
    size_t size = 0;
    int calls = 0;

    auto report = [&code, &size, &calls] (const std::error_code& ec, size_t transferred) {
        code = ec;
        size = transferred;
        ++calls;
    };

    AsyncRead read;

    read.handler = report;
    read.onComplete (&read.op, 16);
    ASSERT_EQ (calls, 1);
    ASSERT_FALSE (code) << code.message ();
    ASSERT_EQ (size, 16);

    read.handler = report;
    read.onComplete (&read.op, -EAGAIN);
    ASSERT_EQ (calls, 2);
    ASSERT_EQ (code, std::errc::resource_unavailable_try_again) << code.message ();
    ASSERT_EQ (size, 0);

    read.msg.msg_flags = MSG_TRUNC;
    read.handler = report;
    read.onComplete (&read.op, 8);
    ASSERT_EQ (calls, 3);
    ASSERT_EQ (code, Errc::MessageTooLong) << code.message ();
    ASSERT_EQ (size, 8);

    read.msg.msg_flags = 0;
    read.stream = true;
    read.handler = report;
    read.onComplete (&read.op, 4);
    ASSERT_EQ (calls, 4);
    ASSERT_FALSE (code) << code.message ();
    ASSERT_EQ (size, 4);

    read.handler = report;
    read.onComplete (&read.op, 0);
    ASSERT_EQ (calls, 5);
    ASSERT_EQ (code, Errc::ConnectionClosed) << code.message ();
    ASSERT_EQ (size, 0);

    read.msg.msg_flags = MSG_TRUNC;
    read.handler = report;
    read.onComplete (&read.op, 2);
    ASSERT_EQ (calls, 6);
    ASSERT_FALSE (code) << code.message ();
    ASSERT_EQ (size, 2);

    read.handler = report;
    read.onComplete (&read.op, -ECONNRESET);
    ASSERT_EQ (calls, 7);
    ASSERT_EQ (code, std::errc::connection_reset) << code.message ();
    ASSERT_EQ (size, 0);

    ASSERT_NO_THROW (read.onComplete (&read.op, 1));
    ASSERT_EQ (calls, 7);
}

/**
 * @brief Test onCancel.
 */
TEST (AsyncRead, onCancel)
{
    std::error_code code;
    size_t size = 1;
    int calls = 0;

    auto report = [&code, &size, &calls] (const std::error_code& ec, size_t transferred) {
        code = ec;
        size = transferred;
        ++calls;
    };

    AsyncRead read;

    read.handler = report;
    read.onCancel (&read.op, -ECANCELED);
    ASSERT_EQ (calls, 1);
    ASSERT_EQ (code, std::errc::operation_canceled) << code.message ();
    ASSERT_EQ (size, 0);

    ASSERT_NO_THROW (read.onCancel (&read.op, -ECANCELED));
    ASSERT_EQ (calls, 1);
}

/**
 * @brief Test onComplete.
 */
TEST (AsyncWrite, onComplete)
{
    std::error_code code;
    size_t size = 0;
    int calls = 0;

    auto report = [&code, &size, &calls] (const std::error_code& ec, size_t transferred) {
        code = ec;
        size = transferred;
        ++calls;
    };

    auto reportConnect = [&code, &calls] (const std::error_code& ec) {
        code = ec;
        ++calls;
    };

    AsyncWrite write;

    write.handler = report;
    write.onComplete (&write.op, 32);
    ASSERT_EQ (calls, 1);
    ASSERT_FALSE (code) << code.message ();
    ASSERT_EQ (size, 32);

    write.handler = report;
    write.onComplete (&write.op, -EPIPE);
    ASSERT_EQ (calls, 2);
    ASSERT_EQ (code, std::errc::broken_pipe) << code.message ();
    ASSERT_EQ (size, 0);

    ASSERT_NO_THROW (write.onComplete (&write.op, 1));
    ASSERT_EQ (calls, 2);

    Udp::Socket sock;
    ASSERT_EQ (sock.open (), 0) << join::lastError.message ();

    write.socket = &sock;
    write.op.code = static_cast<uint8_t> (IoOperation::Opcode::Connect);

    write.connectHandler = reportConnect;
    write.onComplete (&write.op, 0);
    ASSERT_EQ (calls, 3);
    ASSERT_FALSE (code) << code.message ();
    ASSERT_TRUE (sock.connected ());

    write.connectHandler = reportConnect;
    write.onComplete (&write.op, -ECONNREFUSED);
    ASSERT_EQ (calls, 4);
    ASSERT_EQ (code, std::errc::connection_refused) << code.message ();
    ASSERT_FALSE (sock.opened ());

    ASSERT_NO_THROW (write.onComplete (&write.op, 0));
    ASSERT_EQ (calls, 4);
}

/**
 * @brief Test onCancel.
 */
TEST (AsyncWrite, onCancel)
{
    std::error_code code;
    size_t size = 1;
    int calls = 0;

    AsyncWrite write;

    write.handler = [&code, &size, &calls] (const std::error_code& ec, size_t transferred) {
        code = ec;
        size = transferred;
        ++calls;
    };
    write.onCancel (&write.op, -ECANCELED);
    ASSERT_EQ (calls, 1);
    ASSERT_EQ (code, std::errc::operation_canceled) << code.message ();
    ASSERT_EQ (size, 0);

    ASSERT_NO_THROW (write.onCancel (&write.op, -ECANCELED));
    ASSERT_EQ (calls, 1);
}

/**
 * @brief Test remote.
 */
TEST (AsyncWrite, remote)
{
    AsyncWrite write;

    ASSERT_EQ (write.remote, Udp::Endpoint ());

    write.remote = Udp::Endpoint ("127.0.0.1", 5000);
    ASSERT_EQ (write.remote.ip (), "127.0.0.1");
    ASSERT_EQ (write.remote.port (), 5000);
}
