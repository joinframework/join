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
#include <join/async_acceptor.hpp>

// Libraries.
#include <gtest/gtest.h>

// C++.
#include <memory>
#include <chrono>
#include <thread>

using join::Errc;
using join::IoOperation;
using join::Tcp;

/**
 * @brief proactor stub reporting whether the caller runs on the proactor thread.
 */
struct Proactor
{
    bool proactor = false;

    bool isProactorThread () const noexcept
    {
        return proactor;
    }

    int submit ([[maybe_unused]] IoOperation* op, [[maybe_unused]] bool flush = false,
                [[maybe_unused]] bool sync = false) noexcept
    {
        return 0;
    }

    int cancel ([[maybe_unused]] IoOperation* op, [[maybe_unused]] bool flush = false,
                [[maybe_unused]] bool sync = false) noexcept
    {
        return 0;
    }
};

/**
 * @brief exposes the protected interface of the operation under test.
 */
struct AsyncOperation : public join::BasicAsyncOperation<Tcp, Proactor>
{
    using join::BasicAsyncOperation<Tcp, Proactor>::_op;
    using join::BasicAsyncOperation<Tcp, Proactor>::_state;
};

/**
 * @brief exposes the protected interface of the accept operation under test.
 */
struct AsyncAccept : public join::BasicAsyncAccept<Tcp, Proactor>
{
    using join::BasicAsyncAccept<Tcp, Proactor>::onComplete;
    using join::BasicAsyncAccept<Tcp, Proactor>::onCancel;
    using join::BasicAsyncAccept<Tcp, Proactor>::_handler;
    using join::BasicAsyncAccept<Tcp, Proactor>::_remote;
    using join::BasicAsyncAccept<Tcp, Proactor>::_remoteLen;
    using join::BasicAsyncAccept<Tcp, Proactor>::_peer;
    using join::BasicAsyncOperation<Tcp, Proactor>::_op;
    using join::BasicAsyncOperation<Tcp, Proactor>::_state;
};

/**
 * @brief exposes the protected interface of the connect operation under test.
 */
struct AsyncConnect : public join::BasicAsyncConnect<Tcp, Proactor>
{
    using join::BasicAsyncConnect<Tcp, Proactor>::onComplete;
    using join::BasicAsyncConnect<Tcp, Proactor>::onCancel;
    using join::BasicAsyncConnect<Tcp, Proactor>::_handler;
    using join::BasicAsyncConnect<Tcp, Proactor>::_socket;
    using join::BasicAsyncOperation<Tcp, Proactor>::_op;
    using join::BasicAsyncOperation<Tcp, Proactor>::_state;
};

/**
 * @brief exposes the protected interface of the read operation under test.
 */
struct AsyncRead : public join::BasicAsyncRead<Tcp, Proactor>
{
    using join::BasicAsyncRead<Tcp, Proactor>::onComplete;
    using join::BasicAsyncRead<Tcp, Proactor>::onCancel;
    using join::BasicAsyncRead<Tcp, Proactor>::_handler;
    using join::BasicAsyncRead<Tcp, Proactor>::_msg;
    using join::BasicAsyncRead<Tcp, Proactor>::_iov;
    using join::BasicAsyncRead<Tcp, Proactor>::_stream;
    using join::BasicAsyncOperation<Tcp, Proactor>::_op;
    using join::BasicAsyncOperation<Tcp, Proactor>::_state;
};

/**
 * @brief exposes the protected interface of the write operation under test.
 */
struct AsyncWrite : public join::BasicAsyncWrite<Tcp, Proactor>
{
    using join::BasicAsyncWrite<Tcp, Proactor>::onComplete;
    using join::BasicAsyncWrite<Tcp, Proactor>::onCancel;
    using join::BasicAsyncWrite<Tcp, Proactor>::_handler;
    using join::BasicAsyncWrite<Tcp, Proactor>::_msg;
    using join::BasicAsyncWrite<Tcp, Proactor>::_iov;
    using join::BasicAsyncOperation<Tcp, Proactor>::_op;
    using join::BasicAsyncOperation<Tcp, Proactor>::_state;
};

/**
 * @brief Test operator new.
 */
TEST (AsyncOperation, new)
{
    std::unique_ptr<AsyncOperation> operation (new AsyncOperation ());
    ASSERT_NE (operation, nullptr);
    ASSERT_EQ (reinterpret_cast<uintptr_t> (operation.get ()) % alignof (AsyncOperation), 0);
    ASSERT_EQ (operation->_op.handler, nullptr);
    ASSERT_EQ (operation->_op.state, IoOperation::State::Idle);
    ASSERT_EQ (operation->_state.load (), AsyncOperation::Idle);

    std::unique_ptr<AsyncAccept> accept (new AsyncAccept ());
    ASSERT_EQ (reinterpret_cast<uintptr_t> (accept.get ()) % alignof (AsyncAccept), 0);
    ASSERT_FALSE (accept->_handler);
    ASSERT_EQ (accept->_remote, Tcp::Endpoint ());
    ASSERT_EQ (accept->_remoteLen, sizeof (struct sockaddr_storage));
    ASSERT_EQ (accept->_peer, nullptr);

    std::unique_ptr<AsyncConnect> connect (new AsyncConnect ());
    ASSERT_EQ (reinterpret_cast<uintptr_t> (connect.get ()) % alignof (AsyncConnect), 0);
    ASSERT_FALSE (connect->_handler);

    std::unique_ptr<AsyncRead> read (new AsyncRead ());
    ASSERT_EQ (reinterpret_cast<uintptr_t> (read.get ()) % alignof (AsyncRead), 0);
    ASSERT_FALSE (read->_handler);
    ASSERT_EQ (read->_msg.msg_iovlen, 0);
    ASSERT_EQ (read->_iov.iov_len, 0);

    std::unique_ptr<AsyncWrite> write (new AsyncWrite ());
    ASSERT_EQ (reinterpret_cast<uintptr_t> (write.get ()) % alignof (AsyncWrite), 0);
    ASSERT_FALSE (write->_handler);
    ASSERT_EQ (write->_msg.msg_iovlen, 0);
    ASSERT_EQ (write->_iov.iov_len, 0);
}

/**
 * @brief Test operator delete.
 */
TEST (AsyncOperation, delete)
{
    ASSERT_NO_THROW (delete new AsyncOperation ());
    ASSERT_NO_THROW (delete new AsyncAccept ());
    ASSERT_NO_THROW (delete new AsyncConnect ());
    ASSERT_NO_THROW (delete new AsyncRead ());
    ASSERT_NO_THROW (delete new AsyncWrite ());
}

/**
 * @brief Test reserve.
 */
TEST (AsyncOperation, reserve)
{
    Proactor proactor;
    AsyncOperation operation;

    ASSERT_EQ (operation.reserve (proactor), 0) << join::lastError.message ();
    ASSERT_EQ (operation._state.load (), AsyncOperation::Pending);

    ASSERT_EQ (operation.reserve (proactor), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    ASSERT_EQ (operation._state.load (), AsyncOperation::Pending);

    operation._state.store (AsyncOperation::Dispatching);
    ASSERT_EQ (operation.reserve (proactor), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    ASSERT_EQ (operation._state.load (), AsyncOperation::Dispatching);

    proactor.proactor = true;
    ASSERT_EQ (operation.reserve (proactor), 0) << join::lastError.message ();
    ASSERT_EQ (operation._state.load (), AsyncOperation::Pending);

    operation._state.store (AsyncOperation::Closing);
    ASSERT_EQ (operation.reserve (proactor), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    ASSERT_EQ (operation._state.load (), AsyncOperation::Closing);
}

/**
 * @brief Test release.
 */
TEST (AsyncOperation, release)
{
    AsyncOperation operation;

    operation._state.store (AsyncOperation::Pending);
    operation.release ();
    ASSERT_EQ (operation._state.load (), AsyncOperation::Idle);

    operation._state.store (AsyncOperation::Closing);
    operation.release ();
    ASSERT_EQ (operation._state.load (), AsyncOperation::Idle);
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
        operation._state.store (AsyncOperation::Dispatching);
    };

    operation.drain (cancel);
    ASSERT_EQ (operation._state.load (), AsyncOperation::Closing);
    ASSERT_EQ (cancelled, 0);

    operation._state.store (AsyncOperation::Pending);

    std::thread waiter ([&operation] () {
        while (operation._state.load () == AsyncOperation::Pending)
        {
            std::this_thread::yield ();
        }

        std::this_thread::sleep_for (std::chrono::milliseconds (2));
        operation._state.store (AsyncOperation::Idle);
    });

    operation.drain (cancel);
    waiter.join ();

    ASSERT_EQ (operation._state.load (), AsyncOperation::Closing);
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
        observed = operation._state.load ();

        if (rearm)
        {
            operation._state.store (AsyncOperation::Pending);
        }
    };

    operation.dispatch (handler);
    ASSERT_EQ (observed, AsyncOperation::Dispatching);
    ASSERT_EQ (operation._state.load (), AsyncOperation::Idle);

    rearm = true;
    operation.dispatch (handler);
    ASSERT_EQ (operation._state.load (), AsyncOperation::Pending);
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

    read._handler = report;
    read.onComplete (&read._op, 16);
    ASSERT_EQ (calls, 1);
    ASSERT_FALSE (code) << code.message ();
    ASSERT_EQ (size, 16);

    read._handler = report;
    read.onComplete (&read._op, -EAGAIN);
    ASSERT_EQ (calls, 2);
    ASSERT_EQ (code, std::errc::resource_unavailable_try_again) << code.message ();
    ASSERT_EQ (size, 0);

    read._msg.msg_flags = MSG_TRUNC;
    read._handler = report;
    read.onComplete (&read._op, 8);
    ASSERT_EQ (calls, 3);
    ASSERT_EQ (code, Errc::MessageTooLong) << code.message ();
    ASSERT_EQ (size, 8);

    read._msg.msg_flags = 0;
    read._stream = true;
    read._handler = report;
    read.onComplete (&read._op, 4);
    ASSERT_EQ (calls, 4);
    ASSERT_FALSE (code) << code.message ();
    ASSERT_EQ (size, 4);

    read._handler = report;
    read.onComplete (&read._op, 0);
    ASSERT_EQ (calls, 5);
    ASSERT_EQ (code, Errc::ConnectionClosed) << code.message ();
    ASSERT_EQ (size, 0);

    read._msg.msg_flags = MSG_TRUNC;
    read._handler = report;
    read.onComplete (&read._op, 2);
    ASSERT_EQ (calls, 6);
    ASSERT_FALSE (code) << code.message ();
    ASSERT_EQ (size, 2);

    ASSERT_NO_THROW (read.onComplete (&read._op, 1));
    ASSERT_EQ (calls, 6);
}

/**
 * @brief Test onCancel.
 */
TEST (AsyncRead, onCancel)
{
    std::error_code code;
    size_t size = 1;
    int calls = 0;

    AsyncRead read;

    read._handler = [&code, &size, &calls] (const std::error_code& ec, size_t transferred) {
        code = ec;
        size = transferred;
        ++calls;
    };
    read.onCancel (&read._op, -ECANCELED);
    ASSERT_EQ (calls, 1);
    ASSERT_EQ (code, std::errc::operation_canceled) << code.message ();
    ASSERT_EQ (size, 0);

    ASSERT_NO_THROW (read.onCancel (&read._op, -ECANCELED));
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

    AsyncWrite write;

    write._handler = report;
    write.onComplete (&write._op, 32);
    ASSERT_EQ (calls, 1);
    ASSERT_FALSE (code) << code.message ();
    ASSERT_EQ (size, 32);

    write._handler = report;
    write.onComplete (&write._op, -EPIPE);
    ASSERT_EQ (calls, 2);
    ASSERT_EQ (code, std::errc::broken_pipe) << code.message ();
    ASSERT_EQ (size, 0);

    ASSERT_NO_THROW (write.onComplete (&write._op, 1));
    ASSERT_EQ (calls, 2);
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

    write._handler = [&code, &size, &calls] (const std::error_code& ec, size_t transferred) {
        code = ec;
        size = transferred;
        ++calls;
    };
    write.onCancel (&write._op, -ECANCELED);
    ASSERT_EQ (calls, 1);
    ASSERT_EQ (code, std::errc::operation_canceled) << code.message ();
    ASSERT_EQ (size, 0);

    ASSERT_NO_THROW (write.onCancel (&write._op, -ECANCELED));
    ASSERT_EQ (calls, 1);
}

/**
 * @brief Test onComplete.
 */
TEST (AsyncConnect, onComplete)
{
    std::error_code code;
    int calls = 0;

    auto report = [&code, &calls] (const std::error_code& ec) {
        code = ec;
        ++calls;
    };

    Tcp::Socket sock;
    ASSERT_EQ (sock.open (Tcp::v6 ()), 0) << join::lastError.message ();

    AsyncConnect connect;
    connect._socket = &sock;

    connect._handler = report;
    connect.onComplete (&connect._op, 0);
    ASSERT_EQ (calls, 1);
    ASSERT_FALSE (code) << code.message ();
    ASSERT_TRUE (sock.connected ());

    connect._handler = report;
    connect.onComplete (&connect._op, -ECONNREFUSED);
    ASSERT_EQ (calls, 2);
    ASSERT_EQ (code, std::errc::connection_refused) << code.message ();
    ASSERT_FALSE (sock.opened ());

    ASSERT_NO_THROW (connect.onComplete (&connect._op, 0));
    ASSERT_EQ (calls, 2);
}

/**
 * @brief Test onCancel.
 */
TEST (AsyncConnect, onCancel)
{
    std::error_code code;
    int calls = 0;

    Tcp::Socket sock;
    ASSERT_EQ (sock.open (Tcp::v6 ()), 0) << join::lastError.message ();

    AsyncConnect connect;
    connect._socket = &sock;

    connect._handler = [&code, &calls] (const std::error_code& ec) {
        code = ec;
        ++calls;
    };
    connect.onCancel (&connect._op, -ECANCELED);
    ASSERT_EQ (calls, 1);
    ASSERT_EQ (code, std::errc::operation_canceled) << code.message ();
    ASSERT_FALSE (sock.opened ());

    ASSERT_NO_THROW (connect.onCancel (&connect._op, -ECANCELED));
    ASSERT_EQ (calls, 1);
}

/**
 * @brief Test onComplete.
 */
TEST (AsyncAccept, onComplete)
{
    std::error_code code;
    int calls = 0;

    auto report = [&code, &calls] (const std::error_code& ec) {
        code = ec;
        ++calls;
    };

    Proactor proactor;
    join::BasicAsyncStreamSocket<Tcp, Proactor> peer (proactor);

    AsyncAccept accept;

    accept._handler = report;
    accept._peer = &peer;
    accept.onComplete (&accept._op, -ECONNABORTED);
    ASSERT_EQ (calls, 1);
    ASSERT_EQ (code, std::errc::connection_aborted) << code.message ();
    ASSERT_EQ (accept._peer, nullptr);
    ASSERT_FALSE (peer.opened ());

    int fd = ::socket (AF_INET6, SOCK_STREAM, 0);
    ASSERT_NE (fd, -1);

    accept._handler = report;
    accept._peer = &peer;
    accept._remote = Tcp::Endpoint ("::1", 5000);
    accept.onComplete (&accept._op, fd);
    ASSERT_EQ (calls, 2);
    ASSERT_FALSE (code) << code.message ();
    ASSERT_EQ (accept._peer, nullptr);
    ASSERT_TRUE (peer.connected ());
    ASSERT_EQ (peer.remoteEndpoint (), Tcp::Endpoint ("::1", 5000));

    peer.close ();
}

/**
 * @brief Test onCancel.
 */
TEST (AsyncAccept, onCancel)
{
    std::error_code code;
    int calls = 0;

    Proactor proactor;
    join::BasicAsyncStreamSocket<Tcp, Proactor> peer (proactor);

    AsyncAccept accept;

    accept._handler = [&code, &calls] (const std::error_code& ec) {
        code = ec;
        ++calls;
    };
    accept._peer = &peer;
    accept.onCancel (&accept._op, -ECANCELED);
    ASSERT_EQ (calls, 1);
    ASSERT_EQ (code, std::errc::operation_canceled) << code.message ();
    ASSERT_EQ (accept._peer, nullptr);

    ASSERT_NO_THROW (accept.onCancel (&accept._op, -ECANCELED));
    ASSERT_EQ (calls, 1);
}
