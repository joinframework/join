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
#include <join/protocol.hpp>

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
    using join::BasicAsyncConnect<Tcp, Proactor>::_handler;
    using join::BasicAsyncOperation<Tcp, Proactor>::_op;
    using join::BasicAsyncOperation<Tcp, Proactor>::_state;
};

/**
 * @brief exposes the protected interface of the read operation under test.
 */
struct AsyncRead : public join::BasicAsyncRead<Tcp, Proactor>
{
    using join::BasicAsyncRead<Tcp, Proactor>::_handler;
    using join::BasicAsyncRead<Tcp, Proactor>::_msg;
    using join::BasicAsyncRead<Tcp, Proactor>::_iov;
    using join::BasicAsyncOperation<Tcp, Proactor>::_op;
    using join::BasicAsyncOperation<Tcp, Proactor>::_state;
};

/**
 * @brief exposes the protected interface of the write operation under test.
 */
struct AsyncWrite : public join::BasicAsyncWrite<Tcp, Proactor>
{
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
