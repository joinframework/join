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
#include <join/condition.hpp>
#include <join/proactor.hpp>
#include <join/acceptor.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::Errc;
using join::Mutex;
using join::Condition;
using join::ScopedLock;
using join::Thread;
using join::SqpollProactor;
using join::SqpollProactorThread;
using join::IoOperation;
using join::CompletionHandler;
using join::Tcp;

/**
 * @brief Class used to test SqpollProactor.
 */
class SqpollProactorTest : public CompletionHandler, public ::testing::Test
{
protected:
    /**
     * @brief Sets up the test fixture.
     */
    void SetUp () override
    {
        ASSERT_EQ (_acceptor.create ({_host, _port}), 0) << join::lastError.message ();
    }

    /**
     * @brief Tears down the test fixture.
     */
    void TearDown () override
    {
        _server.close ();
        _client.close ();
        _acceptor.close ();
    }

    /**
     * @brief method called when an operation completes.
     * @param op completed operation.
     * @param result bytes transferred or negative errno.
     */
    void onComplete (IoOperation* op, int result) override
    {
        SqpollProactorThread::proactor ().submit (op, true, true);
        SqpollProactorThread::proactor ().cancel (op, true, true);
        SqpollProactorThread::proactor ().flush (true);

        {
            ScopedLock<Mutex> lock (_mut);
            _result = result;
            _op = op;
            CompletionHandler::onComplete (op, result);
        }

        _cond.signal ();
    }

    /**
     * @brief method called when an operation is cancelled.
     * @param op cancelled operation.
     * @param result negative errno.
     */
    void onCancel (IoOperation* op, int result) override
    {
        SqpollProactorThread::proactor ().submit (op, true, true);
        SqpollProactorThread::proactor ().cancel (op, true, true);
        SqpollProactorThread::proactor ().flush (true);

        {
            ScopedLock<Mutex> lock (_mut);
            _result = result;
            _op = op;
            CompletionHandler::onCancel (op, result);
        }

        _cond.signal ();
    }

    /// server acceptor.
    static Tcp::Acceptor _acceptor;

    /// client socket.
    static Tcp::Socket _client;

    /// server socket.
    static Tcp::Socket _server;

    /// host.
    static std::string _host;

    /// port.
    static uint16_t _port;

    /// timeout.
    static const int _timeout;

    /// condition variable.
    static Condition _cond;

    /// condition mutex.
    static Mutex _mut;

    /// last completed operation.
    static IoOperation* _op;

    /// last operation result.
    static int _result;

    /// read buffer.
    static char _buf[256];
};

Tcp::Acceptor SqpollProactorTest::_acceptor;
Tcp::Socket SqpollProactorTest::_client (Tcp::Socket::Blocking);
Tcp::Socket SqpollProactorTest::_server;
std::string SqpollProactorTest::_host = "127.0.0.1";
uint16_t SqpollProactorTest::_port = 5001;
const int SqpollProactorTest::_timeout = 1000;
Condition SqpollProactorTest::_cond;
Mutex SqpollProactorTest::_mut;
IoOperation* SqpollProactorTest::_op = nullptr;
int SqpollProactorTest::_result = 0;
char SqpollProactorTest::_buf[256] = {};

/**
 * @brief Test stop.
 */
TEST_F (SqpollProactorTest, stop)
{
    SqpollProactor proactor;
    Thread th ([&proactor] () {
        proactor.run ();
    });

    ASSERT_EQ (_client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    auto op = IoOperation::makeRead (_server.handle (), _buf, sizeof (_buf), this);
    ASSERT_EQ (proactor.submit (&op, true, true), 0) << join::lastError.message ();

    proactor.stop ();
    th.join ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_EQ (_op, &op);
        ASSERT_EQ (_result, -ECANCELED);
        _op = nullptr;
        _result = 0;
    }
}

/**
 * @brief Test submit.
 */
TEST_F (SqpollProactorTest, submit)
{
    SqpollProactor proactor;
    Thread th ([&proactor] () {
        proactor.run ();
    });

    ASSERT_EQ (proactor.submit (nullptr, true, true), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);

    auto op1 = IoOperation::makeRead (-1, _buf, sizeof (_buf), this);
    ASSERT_EQ (proactor.submit (&op1, true, true), -1);
    ASSERT_EQ (join::lastError, std::errc::bad_file_descriptor);

    ASSERT_EQ (_client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    op1 = IoOperation::makeRead (_server.handle (), _buf, sizeof (_buf), this);
    op1.state = IoOperation::State::Submitted;
    ASSERT_EQ (proactor.submit (&op1, true, true), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    op1.state = IoOperation::State::Idle;
    ASSERT_EQ (proactor.submit (&op1, true, true), 0) << join::lastError.message ();

#ifndef JOIN_HAS_IO_URING
    auto op2 = IoOperation::makeRead (_server.handle (), _buf, sizeof (_buf), this);
    ASSERT_EQ (proactor.submit (&op2, true, true), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
#endif

    ASSERT_EQ (proactor.cancel (&op1, true, true), 0) << join::lastError.message ();
    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
            return _op == &op1 && _result == -ECANCELED;
        }));
        _op = nullptr;
        _result = 0;
    }

    proactor.stop ();
    th.join ();
}

/**
 * @brief Test cancel.
 */
TEST_F (SqpollProactorTest, cancel)
{
    SqpollProactor proactor;
    Thread th ([&proactor] () {
        proactor.run ();
    });

    ASSERT_EQ (proactor.cancel (nullptr, true, true), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);

    auto op1 = IoOperation::makeRead (-1, _buf, sizeof (_buf), this);
    ASSERT_EQ (proactor.cancel (&op1, true, true), -1);
    ASSERT_EQ (join::lastError, std::errc::bad_file_descriptor);

    ASSERT_EQ (_client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    op1 = IoOperation::makeRead (_server.handle (), _buf, sizeof (_buf), this);
    ASSERT_EQ (proactor.cancel (&op1, true, true), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (proactor.submit (&op1, true, true), 0) << join::lastError.message ();

    auto op2 = IoOperation::makeRead (_server.handle (), _buf, sizeof (_buf), this);
    op2.state = IoOperation::State::Submitted;
    ASSERT_EQ (proactor.cancel (&op2, true, true), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);

    ASSERT_EQ (proactor.cancel (&op1, true, true), 0) << join::lastError.message ();
    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
            return _op == &op1 && _result == -ECANCELED;
        }));
        _op = nullptr;
        _result = 0;
    }

    proactor.stop ();
    th.join ();
}

/**
 * @brief Test flush.
 */
TEST_F (SqpollProactorTest, flush)
{
    SqpollProactor proactor;
    Thread th ([&proactor] () {
        proactor.run ();
    });

    ASSERT_EQ (_client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    auto op = IoOperation::makeRead (_server.handle (), _buf, sizeof (_buf), this);
    ASSERT_EQ (proactor.submit (&op, false, true), 0) << join::lastError.message ();
    ASSERT_EQ (proactor.flush (true), 0) << join::lastError.message ();

    ASSERT_EQ (_client.writeExactly ("flush", strlen ("flush"), _timeout), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
            return _op == &op && _result > 0;
        }));
        ASSERT_EQ (std::string (_buf, _result), "flush");
        _op = nullptr;
        _result = 0;
    }

    proactor.stop ();
    th.join ();
}

/**
 * @brief Test SQE chaining.
 */
TEST_F (SqpollProactorTest, chain)
{
    SqpollProactor proactor;
    Thread th ([&proactor] () {
        proactor.run ();
    });

    ASSERT_EQ (_client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    auto writeOp = IoOperation::makeWrite (_server.handle (), "ping", 4, this, true);
    auto readOp = IoOperation::makeRead (_server.handle (), _buf, sizeof (_buf), this);

    ASSERT_EQ (proactor.submit (&writeOp, false, true), 0) << join::lastError.message ();
    ASSERT_EQ (proactor.submit (&readOp, true, true), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
            return _op == &writeOp && _result == 4;
        }));
        _op = nullptr;
        _result = 0;
    }

    char tmp[4];
    ASSERT_EQ (_client.readExactly (tmp, 4, _timeout), 0) << join::lastError.message ();
    ASSERT_EQ (_client.writeExactly ("pong", 4, _timeout), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
            return _op == &readOp && _result > 0;
        }));
        ASSERT_EQ (std::string (_buf, _result), "pong");
        _op = nullptr;
        _result = 0;
    }

    proactor.stop ();
    th.join ();
}

#ifdef JOIN_HAS_NUMA
/**
 * @brief Test mbind.
 */
TEST_F (SqpollProactorTest, mbind)
{
    SqpollProactor proactor;

    ASSERT_EQ (proactor.mbind (0), 0) << join::lastError.message ();
}
#endif

/**
 * @brief Test mlock.
 */
TEST_F (SqpollProactorTest, mlock)
{
    SqpollProactor proactor;

    ASSERT_EQ (proactor.mlock (), 0) << join::lastError.message ();
}

/**
 * @brief Test isRunning.
 */
TEST_F (SqpollProactorTest, isRunning)
{
    SqpollProactor proactor;

    ASSERT_FALSE (proactor.isRunning ());

    Thread th ([&proactor] () {
        proactor.run ();
    });
    while (!proactor.isRunning ())
    {
    }

    ASSERT_TRUE (proactor.isRunning ());

    proactor.stop ();
    th.join ();

    ASSERT_FALSE (proactor.isRunning ());
}

#ifdef JOIN_HAS_IO_URING
/**
 * @brief Test registerBuffers and unregisterBuffers.
 */
TEST_F (SqpollProactorTest, registerBuffers)
{
    SqpollProactor proactor;

    std::vector<char> buf (4096);
    iovec iov = {buf.data (), buf.size ()};
    std::vector<iovec> iovecs = {iov};

    ASSERT_EQ (proactor.registerBuffers (iovecs), 0) << join::lastError.message ();
    ASSERT_EQ (proactor.registerBuffers (iovecs), -1);
    ASSERT_EQ (proactor.unregisterBuffers (), 0) << join::lastError.message ();
    ASSERT_EQ (proactor.registerBuffers (iovecs), 0) << join::lastError.message ();
    ASSERT_EQ (proactor.unregisterBuffers (), 0) << join::lastError.message ();
    ASSERT_EQ (proactor.unregisterBuffers (), -1);
}
#endif

/**
 * @brief Test async connect.
 */
TEST_F (SqpollProactorTest, asyncConnect)
{
    Tcp::Endpoint endpoint{_host, _port};

    ASSERT_EQ (_client.open (Tcp::v4 ()), 0) << join::lastError.message ();
    _client.setMode (Tcp::Socket::NonBlocking);

    auto op = IoOperation::makeConnect (_client.handle (), endpoint.addr (), endpoint.length (), this);
    ASSERT_EQ (SqpollProactorThread::proactor ().submit (&op, true, true), 0) << join::lastError.message ();

    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
            return _op == &op && _result == 0;
        }));
        _op = nullptr;
        _result = 0;
    }

    _client.setMode (Tcp::Socket::Blocking);
}

/**
 * @brief Test async accept.
 */
TEST_F (SqpollProactorTest, asyncAccept)
{
    ASSERT_EQ (SqpollProactorThread::affinity (0), 0) << join::lastError.message ();
    ASSERT_EQ (SqpollProactorThread::affinity (), 0);
    ASSERT_EQ (SqpollProactorThread::priority (1), 0) << join::lastError.message ();
    ASSERT_EQ (SqpollProactorThread::priority (), 1);
#ifdef JOIN_HAS_NUMA
    ASSERT_EQ (SqpollProactorThread::mbind (0), 0) << join::lastError.message ();
#endif
    ASSERT_EQ (SqpollProactorThread::mlock (), 0) << join::lastError.message ();
    ASSERT_GT (SqpollProactorThread::handle (), 0);

    sockaddr_storage addr = {};
    socklen_t addrlen = sizeof (addr);
    auto op = IoOperation::makeAccept (_acceptor.handle (), reinterpret_cast<sockaddr*> (&addr), &addrlen,
                                       SOCK_NONBLOCK | SOCK_CLOEXEC, this);

    ASSERT_EQ (SqpollProactorThread::proactor ().submit (&op, true, true), 0) << join::lastError.message ();
    ASSERT_EQ (_client.connect ({_host, _port}), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
            return _op == &op && _result >= 0;
        }));
        ::close (_result);
        _op = nullptr;
        _result = 0;
    }
}

/**
 * @brief Test async write.
 */
TEST_F (SqpollProactorTest, asyncWrite)
{
    ASSERT_EQ (_client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    ASSERT_EQ (SqpollProactorThread::affinity (0), 0) << join::lastError.message ();
    ASSERT_EQ (SqpollProactorThread::affinity (), 0);
    ASSERT_EQ (SqpollProactorThread::priority (1), 0) << join::lastError.message ();
    ASSERT_EQ (SqpollProactorThread::priority (), 1);
#ifdef JOIN_HAS_NUMA
    ASSERT_EQ (SqpollProactorThread::mbind (0), 0) << join::lastError.message ();
#endif
    ASSERT_EQ (SqpollProactorThread::mlock (), 0) << join::lastError.message ();
    ASSERT_GT (SqpollProactorThread::handle (), 0);

    const char* msg = "asyncWrite";
    auto op = IoOperation::makeWrite (_server.handle (), msg, strlen (msg), this);

    ASSERT_EQ (SqpollProactorThread::proactor ().submit (&op, true, true), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
            return _op == &op && _result > 0;
        }));
        ASSERT_EQ (_result, static_cast<int> (strlen (msg)));
        _op = nullptr;
        _result = 0;
    }

    char rbuf[256] = {};
    ASSERT_EQ (_client.readExactly (rbuf, strlen (msg), _timeout), 0) << join::lastError.message ();
    ASSERT_EQ (std::string (rbuf, strlen (msg)), msg);
}

/**
 * @brief Test async read.
 */
TEST_F (SqpollProactorTest, asyncRead)
{
    ASSERT_EQ (_client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    ASSERT_EQ (SqpollProactorThread::affinity (0), 0) << join::lastError.message ();
    ASSERT_EQ (SqpollProactorThread::affinity (), 0);
    ASSERT_EQ (SqpollProactorThread::priority (1), 0) << join::lastError.message ();
    ASSERT_EQ (SqpollProactorThread::priority (), 1);
#ifdef JOIN_HAS_NUMA
    ASSERT_EQ (SqpollProactorThread::mbind (0), 0) << join::lastError.message ();
#endif
    ASSERT_EQ (SqpollProactorThread::mlock (), 0) << join::lastError.message ();
    ASSERT_GT (SqpollProactorThread::handle (), 0);

    auto op = IoOperation::makeRead (_server.handle (), _buf, sizeof (_buf), this);

    ASSERT_EQ (SqpollProactorThread::proactor ().submit (&op, true, true), 0) << join::lastError.message ();
    ASSERT_EQ (_client.writeExactly ("asyncRead", strlen ("asyncRead"), _timeout), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
            return _op == &op && _result > 0;
        }));
        ASSERT_EQ (std::string (_buf, _result), "asyncRead");
        _op = nullptr;
        _result = 0;
    }
}

/**
 * @brief Test async write fixed.
 */
TEST_F (SqpollProactorTest, asyncWriteFixed)
{
    ASSERT_EQ (_client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    const char* msg = "asyncWriteFixed";
    std::vector<char> regbuf (msg, msg + strlen (msg));
    iovec iov = {regbuf.data (), regbuf.size ()};
    std::vector<iovec> iovecs = {iov};

    ASSERT_EQ (SqpollProactorThread::proactor ().registerBuffers (iovecs), 0) << join::lastError.message ();

    auto op = IoOperation::makeWriteFixed (_server.handle (), regbuf.data (), regbuf.size (), 0, this);
    ASSERT_EQ (SqpollProactorThread::proactor ().submit (&op, true, true), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
            return _op == &op && _result > 0;
        }));
        ASSERT_EQ (_result, static_cast<int> (strlen (msg)));
        _op = nullptr;
        _result = 0;
    }

    char rbuf[256] = {};
    ASSERT_EQ (_client.readExactly (rbuf, strlen (msg), _timeout), 0) << join::lastError.message ();
    ASSERT_EQ (std::string (rbuf, strlen (msg)), msg);

    ASSERT_EQ (SqpollProactorThread::proactor ().unregisterBuffers (), 0) << join::lastError.message ();
}

/**
 * @brief Test async read fixed.
 */
TEST_F (SqpollProactorTest, asyncReadFixed)
{
    ASSERT_EQ (_client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    std::vector<char> regbuf (sizeof (_buf));
    iovec iov = {regbuf.data (), regbuf.size ()};
    std::vector<iovec> iovecs = {iov};

    ASSERT_EQ (SqpollProactorThread::proactor ().registerBuffers (iovecs), 0) << join::lastError.message ();

    auto op = IoOperation::makeReadFixed (_server.handle (), regbuf.data (), regbuf.size (), 0, this);
    ASSERT_EQ (SqpollProactorThread::proactor ().submit (&op, true, true), 0) << join::lastError.message ();
    ASSERT_EQ (_client.writeExactly ("asyncReadFixed", strlen ("asyncReadFixed"), _timeout), 0)
        << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
            return _op == &op && _result > 0;
        }));
        ASSERT_EQ (std::string (regbuf.data (), _result), "asyncReadFixed");
        _op = nullptr;
        _result = 0;
    }

    ASSERT_EQ (SqpollProactorThread::proactor ().unregisterBuffers (), 0) << join::lastError.message ();
}

/**
 * @brief Test async sendmsg.
 */
TEST_F (SqpollProactorTest, asyncSendmsg)
{
    ASSERT_EQ (_client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    ASSERT_EQ (SqpollProactorThread::affinity (0), 0) << join::lastError.message ();
    ASSERT_EQ (SqpollProactorThread::affinity (), 0);
    ASSERT_EQ (SqpollProactorThread::priority (1), 0) << join::lastError.message ();
    ASSERT_EQ (SqpollProactorThread::priority (), 1);
#ifdef JOIN_HAS_NUMA
    ASSERT_EQ (SqpollProactorThread::mbind (0), 0) << join::lastError.message ();
#endif
    ASSERT_EQ (SqpollProactorThread::mlock (), 0) << join::lastError.message ();
    ASSERT_GT (SqpollProactorThread::handle (), 0);

    const char* payload = "asyncSendmsg";
    iovec iov = {.iov_base = const_cast<char*> (payload), .iov_len = strlen (payload)};
    msghdr msg = {};
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    auto op = IoOperation::makeSendmsg (_server.handle (), &msg, 0, this);

    ASSERT_EQ (SqpollProactorThread::proactor ().submit (&op, true, true), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
            return _op == &op && _result > 0;
        }));
        ASSERT_EQ (_result, static_cast<int> (strlen (payload)));
        _op = nullptr;
        _result = 0;
    }

    char rbuf[256] = {};
    ASSERT_EQ (_client.readExactly (rbuf, strlen (payload), _timeout), 0) << join::lastError.message ();
    ASSERT_EQ (std::string (rbuf, strlen (payload)), payload);
}

/**
 * @brief Test async recvmsg.
 */
TEST_F (SqpollProactorTest, asyncRecvmsg)
{
    ASSERT_EQ (_client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    ASSERT_EQ (SqpollProactorThread::affinity (0), 0) << join::lastError.message ();
    ASSERT_EQ (SqpollProactorThread::affinity (), 0);
    ASSERT_EQ (SqpollProactorThread::priority (1), 0) << join::lastError.message ();
    ASSERT_EQ (SqpollProactorThread::priority (), 1);
#ifdef JOIN_HAS_NUMA
    ASSERT_EQ (SqpollProactorThread::mbind (0), 0) << join::lastError.message ();
#endif
    ASSERT_EQ (SqpollProactorThread::mlock (), 0) << join::lastError.message ();
    ASSERT_GT (SqpollProactorThread::handle (), 0);

    iovec iov = {.iov_base = _buf, .iov_len = sizeof (_buf)};
    msghdr msg = {};
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    auto op = IoOperation::makeRecvmsg (_server.handle (), &msg, 0, this);

    ASSERT_EQ (SqpollProactorThread::proactor ().submit (&op, true, true), 0) << join::lastError.message ();
    ASSERT_EQ (_client.writeExactly ("asyncRecvmsg", strlen ("asyncRecvmsg"), _timeout), 0)
        << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
            return _op == &op && _result > 0;
        }));
        ASSERT_EQ (std::string (_buf, _result), "asyncRecvmsg");
        _op = nullptr;
        _result = 0;
    }
}

/**
 * @brief Test async send.
 */
TEST_F (SqpollProactorTest, asyncSend)
{
    ASSERT_EQ (_client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    const char* msg = "asyncSend";
    auto op = IoOperation::makeSend (_server.handle (), msg, strlen (msg), 0, this);

    ASSERT_EQ (SqpollProactorThread::proactor ().submit (&op, true, true), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
            return _op == &op && _result > 0;
        }));
        ASSERT_EQ (_result, static_cast<int> (strlen (msg)));
        _op = nullptr;
        _result = 0;
    }

    char rbuf[256] = {};
    ASSERT_EQ (_client.readExactly (rbuf, strlen (msg), _timeout), 0) << join::lastError.message ();
    ASSERT_EQ (std::string (rbuf, strlen (msg)), msg);
}

/**
 * @brief Test async recv.
 */
TEST_F (SqpollProactorTest, asyncRecv)
{
    ASSERT_EQ (_client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    auto op = IoOperation::makeRecv (_server.handle (), _buf, sizeof (_buf), 0, this);

    ASSERT_EQ (SqpollProactorThread::proactor ().submit (&op, true, true), 0) << join::lastError.message ();
    ASSERT_EQ (_client.writeExactly ("asyncRecv", strlen ("asyncRecv"), _timeout), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
            return _op == &op && _result > 0;
        }));
        ASSERT_EQ (std::string (_buf, _result), "asyncRecv");
        _op = nullptr;
        _result = 0;
    }
}

/**
 * @brief Test onClose.
 */
TEST_F (SqpollProactorTest, onClose)
{
    ASSERT_EQ (_client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    ASSERT_EQ (SqpollProactorThread::affinity (0), 0) << join::lastError.message ();
    ASSERT_EQ (SqpollProactorThread::affinity (), 0);
    ASSERT_EQ (SqpollProactorThread::priority (1), 0) << join::lastError.message ();
    ASSERT_EQ (SqpollProactorThread::priority (), 1);
#ifdef JOIN_HAS_NUMA
    ASSERT_EQ (SqpollProactorThread::mbind (0), 0) << join::lastError.message ();
#endif
    ASSERT_EQ (SqpollProactorThread::mlock (), 0) << join::lastError.message ();
    ASSERT_GT (SqpollProactorThread::handle (), 0);

    auto op = IoOperation::makeRead (_server.handle (), _buf, sizeof (_buf), this);

    ASSERT_EQ (SqpollProactorThread::proactor ().submit (&op, true, true), 0) << join::lastError.message ();
    _client.close ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
            return _op == &op && _result == 0;
        }));
        _op = nullptr;
        _result = 0;
    }
}

/**
 * @brief Test onError.
 */
TEST_F (SqpollProactorTest, onError)
{
    ASSERT_EQ (_client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    ASSERT_EQ (SqpollProactorThread::affinity (0), 0) << join::lastError.message ();
    ASSERT_EQ (SqpollProactorThread::affinity (), 0);
    ASSERT_EQ (SqpollProactorThread::priority (1), 0) << join::lastError.message ();
    ASSERT_EQ (SqpollProactorThread::priority (), 1);
#ifdef JOIN_HAS_NUMA
    ASSERT_EQ (SqpollProactorThread::mbind (0), 0) << join::lastError.message ();
#endif
    ASSERT_EQ (SqpollProactorThread::mlock (), 0) << join::lastError.message ();
    ASSERT_GT (SqpollProactorThread::handle (), 0);

    auto op = IoOperation::makeRead (_server.handle (), _buf, sizeof (_buf), this);

    ASSERT_EQ (SqpollProactorThread::proactor ().submit (&op, true, true), 0) << join::lastError.message ();
    linger sl{.l_onoff = 1, .l_linger = 0};
    ASSERT_EQ (setsockopt (_client.handle (), SOL_SOCKET, SO_LINGER, &sl, sizeof (sl)), 0) << strerror (errno);
    _client.close ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
            return _op == &op && _result == -ECONNRESET;
        }));
        _op = nullptr;
        _result = 0;
    }
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
