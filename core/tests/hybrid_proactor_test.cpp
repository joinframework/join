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
using join::HybridProactor;
using join::HybridProactorThread;
using join::IoOperation;
using join::CompletionHandler;
using join::Tcp;

/**
 * @brief Class used to test HybridProactor.
 */
class HybridProactorTest : public CompletionHandler, public ::testing::Test
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
        HybridProactorThread::proactor ().submit (op, true);
        HybridProactorThread::proactor ().cancel (op, true);

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
        HybridProactorThread::proactor ().submit (op, true);
        HybridProactorThread::proactor ().cancel (op, true);

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

Tcp::Acceptor HybridProactorTest::_acceptor;
Tcp::Socket HybridProactorTest::_client (Tcp::Socket::Blocking);
Tcp::Socket HybridProactorTest::_server;
std::string HybridProactorTest::_host = "127.0.0.1";
uint16_t HybridProactorTest::_port = 5001;
const int HybridProactorTest::_timeout = 1000;
Condition HybridProactorTest::_cond;
Mutex HybridProactorTest::_mut;
IoOperation* HybridProactorTest::_op = nullptr;
int HybridProactorTest::_result = 0;
char HybridProactorTest::_buf[256] = {};

/**
 * @brief Test stop.
 */
TEST_F (HybridProactorTest, stop)
{
    HybridProactor proactor;
    Thread th ([&proactor] () {
        proactor.run ();
    });

    ASSERT_EQ (_client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    auto op = IoOperation::makeRead (_server.handle (), _buf, sizeof (_buf), this);
    ASSERT_EQ (proactor.submit (&op, true), 0) << join::lastError.message ();

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
TEST_F (HybridProactorTest, submit)
{
    HybridProactor proactor;
    Thread th ([&proactor] () {
        proactor.run ();
    });

    ASSERT_EQ (proactor.submit (nullptr, true), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);

    auto op1 = IoOperation::makeRead (-1, _buf, sizeof (_buf), this);
    ASSERT_EQ (proactor.submit (&op1, true), -1);
    ASSERT_EQ (join::lastError, std::errc::bad_file_descriptor);

    ASSERT_EQ (_client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    op1 = IoOperation::makeRead (_server.handle (), _buf, sizeof (_buf), this);
    op1.state = IoOperation::State::Submitted;
    ASSERT_EQ (proactor.submit (&op1, true), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    op1.state = IoOperation::State::Idle;
    ASSERT_EQ (proactor.submit (&op1, true), 0) << join::lastError.message ();

#ifndef JOIN_HAS_IO_URING
    auto op2 = IoOperation::makeRead (_server.handle (), _buf, sizeof (_buf), this);
    ASSERT_EQ (proactor.submit (&op2, true), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
#endif

    ASSERT_EQ (proactor.cancel (&op1, true), 0) << join::lastError.message ();
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
TEST_F (HybridProactorTest, cancel)
{
    HybridProactor proactor;
    Thread th ([&proactor] () {
        proactor.run ();
    });

    ASSERT_EQ (proactor.cancel (nullptr, true), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);

    auto op1 = IoOperation::makeRead (-1, _buf, sizeof (_buf), this);
    ASSERT_EQ (proactor.cancel (&op1, true), -1);
    ASSERT_EQ (join::lastError, std::errc::bad_file_descriptor);

    ASSERT_EQ (_client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    op1 = IoOperation::makeRead (_server.handle (), _buf, sizeof (_buf), this);
    ASSERT_EQ (proactor.cancel (&op1, true), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (proactor.submit (&op1, true), 0) << join::lastError.message ();

    auto op2 = IoOperation::makeRead (_server.handle (), _buf, sizeof (_buf), this);
    op2.state = IoOperation::State::Submitted;
    ASSERT_EQ (proactor.cancel (&op2, true), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);

    ASSERT_EQ (proactor.cancel (&op1, true), 0) << join::lastError.message ();
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
TEST_F (HybridProactorTest, flush)
{
    HybridProactor proactor;
    Thread th ([&proactor] () {
        proactor.run ();
    });

    ASSERT_EQ (_client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    auto op = IoOperation::makeRead (_server.handle (), _buf, sizeof (_buf), this);
    ASSERT_EQ (proactor.submit (&op, false), 0) << join::lastError.message ();
    ASSERT_EQ (proactor.flush (), 0) << join::lastError.message ();

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
TEST_F (HybridProactorTest, chain)
{
    HybridProactor proactor;
    Thread th ([&proactor] () {
        proactor.run ();
    });

    ASSERT_EQ (_client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    auto writeOp = IoOperation::makeWrite (_server.handle (), "ping", 4, this);
    auto readOp = IoOperation::makeRead (_server.handle (), _buf, sizeof (_buf), this);
    writeOp.linked = true;

    ASSERT_EQ (proactor.submit (&writeOp, false), 0) << join::lastError.message ();
    ASSERT_EQ (proactor.submit (&readOp, true), 0) << join::lastError.message ();

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
TEST_F (HybridProactorTest, mbind)
{
    HybridProactor proactor;

    ASSERT_EQ (proactor.mbind (0), 0) << join::lastError.message ();
}
#endif

/**
 * @brief Test mlock.
 */
TEST_F (HybridProactorTest, mlock)
{
    HybridProactor proactor;

    ASSERT_EQ (proactor.mlock (), 0) << join::lastError.message ();
}

/**
 * @brief Test isRunning.
 */
TEST_F (HybridProactorTest, isRunning)
{
    HybridProactor proactor;

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
TEST_F (HybridProactorTest, registerBuffers)
{
    HybridProactor proactor;

    std::vector<char> buf (4096);
    iovec iov = {buf.data (), buf.size ()};
    std::vector<iovec> iovecs = {iov};

    ASSERT_EQ (proactor.registerBuffers (iovecs), 0) << join::lastError.message ();
    ASSERT_EQ (proactor.registerBuffers (iovecs), -1);
    ASSERT_EQ (proactor.unregisterBuffers (), 0) << join::lastError.message ();
    ASSERT_EQ (proactor.registerBuffers (iovecs), 0) << join::lastError.message ();
    ASSERT_EQ (proactor.unregisterBuffers (), 0) << join::lastError.message ();
}
#endif

/**
 * @brief Test async accept.
 */
TEST_F (HybridProactorTest, asyncAccept)
{
    ASSERT_EQ (HybridProactorThread::affinity (0), 0) << join::lastError.message ();
    ASSERT_EQ (HybridProactorThread::affinity (), 0);
    ASSERT_EQ (HybridProactorThread::priority (1), 0) << join::lastError.message ();
    ASSERT_EQ (HybridProactorThread::priority (), 1);
#ifdef JOIN_HAS_NUMA
    ASSERT_EQ (HybridProactorThread::mbind (0), 0) << join::lastError.message ();
#endif
    ASSERT_EQ (HybridProactorThread::mlock (), 0) << join::lastError.message ();
    ASSERT_GT (HybridProactorThread::handle (), 0);

    sockaddr_storage addr = {};
    socklen_t addrlen = sizeof (addr);
    auto op = IoOperation::makeAccept (_acceptor.handle (), reinterpret_cast<sockaddr*> (&addr), &addrlen,
                                       SOCK_NONBLOCK | SOCK_CLOEXEC, this);

    ASSERT_EQ (HybridProactorThread::proactor ().submit (&op, true), 0) << join::lastError.message ();
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
 * @brief Test async connect.
 */
TEST_F (HybridProactorTest, asyncConnect)
{
    Tcp::Endpoint endpoint{_host, _port};

    ASSERT_EQ (_client.open (Tcp::v4 ()), 0) << join::lastError.message ();
    _client.setMode (Tcp::Socket::NonBlocking);

    auto op = IoOperation::makeConnect (_client.handle (), endpoint.addr (), endpoint.length (), this);
    ASSERT_EQ (HybridProactorThread::proactor ().submit (&op, true), 0) << join::lastError.message ();

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
 * @brief Test async read.
 */
TEST_F (HybridProactorTest, asyncRead)
{
    ASSERT_EQ (_client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    ASSERT_EQ (HybridProactorThread::affinity (0), 0) << join::lastError.message ();
    ASSERT_EQ (HybridProactorThread::affinity (), 0);
    ASSERT_EQ (HybridProactorThread::priority (1), 0) << join::lastError.message ();
    ASSERT_EQ (HybridProactorThread::priority (), 1);
#ifdef JOIN_HAS_NUMA
    ASSERT_EQ (HybridProactorThread::mbind (0), 0) << join::lastError.message ();
#endif
    ASSERT_EQ (HybridProactorThread::mlock (), 0) << join::lastError.message ();
    ASSERT_GT (HybridProactorThread::handle (), 0);

    auto op = IoOperation::makeRead (_server.handle (), _buf, sizeof (_buf), this);

    ASSERT_EQ (HybridProactorThread::proactor ().submit (&op, true), 0) << join::lastError.message ();
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
 * @brief Test async write.
 */
TEST_F (HybridProactorTest, asyncWrite)
{
    ASSERT_EQ (_client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    ASSERT_EQ (HybridProactorThread::affinity (0), 0) << join::lastError.message ();
    ASSERT_EQ (HybridProactorThread::affinity (), 0);
    ASSERT_EQ (HybridProactorThread::priority (1), 0) << join::lastError.message ();
    ASSERT_EQ (HybridProactorThread::priority (), 1);
#ifdef JOIN_HAS_NUMA
    ASSERT_EQ (HybridProactorThread::mbind (0), 0) << join::lastError.message ();
#endif
    ASSERT_EQ (HybridProactorThread::mlock (), 0) << join::lastError.message ();
    ASSERT_GT (HybridProactorThread::handle (), 0);

    const char* msg = "asyncWrite";
    auto op = IoOperation::makeWrite (_server.handle (), msg, strlen (msg), this);

    ASSERT_EQ (HybridProactorThread::proactor ().submit (&op, true), 0) << join::lastError.message ();

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
 * @brief Test async read and write.
 */
TEST_F (HybridProactorTest, asyncReadWrite)
{
    ASSERT_EQ (_client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    const char* msg = "asyncReadWrite";
    auto readOp = IoOperation::makeRead (_server.handle (), _buf, sizeof (_buf), this);
    auto writeOp = IoOperation::makeWrite (_server.handle (), msg, strlen (msg), this);

    ASSERT_EQ (HybridProactorThread::proactor ().submit (&readOp, true), 0) << join::lastError.message ();
    ASSERT_EQ (HybridProactorThread::proactor ().submit (&writeOp, true), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
            return _op == &writeOp && _result > 0;
        }));
        ASSERT_EQ (_result, static_cast<int> (strlen (msg)));
        _op = nullptr;
        _result = 0;
    }

    ASSERT_EQ (_client.writeExactly (msg, strlen (msg), _timeout), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
            return _op == &readOp && _result > 0;
        }));
        ASSERT_EQ (std::string (_buf, _result), msg);
        _op = nullptr;
        _result = 0;
    }

    char rbuf[256] = {};
    ASSERT_EQ (_client.readExactly (rbuf, strlen (msg), _timeout), 0) << join::lastError.message ();
    ASSERT_EQ (std::string (rbuf, strlen (msg)), msg);
}

/**
 * @brief Test async recvmsg.
 */
TEST_F (HybridProactorTest, asyncRecvmsg)
{
    ASSERT_EQ (_client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    ASSERT_EQ (HybridProactorThread::affinity (0), 0) << join::lastError.message ();
    ASSERT_EQ (HybridProactorThread::affinity (), 0);
    ASSERT_EQ (HybridProactorThread::priority (1), 0) << join::lastError.message ();
    ASSERT_EQ (HybridProactorThread::priority (), 1);
#ifdef JOIN_HAS_NUMA
    ASSERT_EQ (HybridProactorThread::mbind (0), 0) << join::lastError.message ();
#endif
    ASSERT_EQ (HybridProactorThread::mlock (), 0) << join::lastError.message ();
    ASSERT_GT (HybridProactorThread::handle (), 0);

    iovec iov = {.iov_base = _buf, .iov_len = sizeof (_buf)};
    msghdr msg = {};
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    auto op = IoOperation::makeRecvmsg (_server.handle (), &msg, 0, this);

    ASSERT_EQ (HybridProactorThread::proactor ().submit (&op, true), 0) << join::lastError.message ();
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
 * @brief Test async sendmsg.
 */
TEST_F (HybridProactorTest, asyncSendmsg)
{
    ASSERT_EQ (_client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    ASSERT_EQ (HybridProactorThread::affinity (0), 0) << join::lastError.message ();
    ASSERT_EQ (HybridProactorThread::affinity (), 0);
    ASSERT_EQ (HybridProactorThread::priority (1), 0) << join::lastError.message ();
    ASSERT_EQ (HybridProactorThread::priority (), 1);
#ifdef JOIN_HAS_NUMA
    ASSERT_EQ (HybridProactorThread::mbind (0), 0) << join::lastError.message ();
#endif
    ASSERT_EQ (HybridProactorThread::mlock (), 0) << join::lastError.message ();
    ASSERT_GT (HybridProactorThread::handle (), 0);

    const char* payload = "asyncSendmsg";
    iovec iov = {.iov_base = const_cast<char*> (payload), .iov_len = strlen (payload)};
    msghdr msg = {};
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    auto op = IoOperation::makeSendmsg (_server.handle (), &msg, 0, this);

    ASSERT_EQ (HybridProactorThread::proactor ().submit (&op, true), 0) << join::lastError.message ();

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
 * @brief Test onClose.
 */
TEST_F (HybridProactorTest, onClose)
{
    ASSERT_EQ (_client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    ASSERT_EQ (HybridProactorThread::affinity (0), 0) << join::lastError.message ();
    ASSERT_EQ (HybridProactorThread::affinity (), 0);
    ASSERT_EQ (HybridProactorThread::priority (1), 0) << join::lastError.message ();
    ASSERT_EQ (HybridProactorThread::priority (), 1);
#ifdef JOIN_HAS_NUMA
    ASSERT_EQ (HybridProactorThread::mbind (0), 0) << join::lastError.message ();
#endif
    ASSERT_EQ (HybridProactorThread::mlock (), 0) << join::lastError.message ();
    ASSERT_GT (HybridProactorThread::handle (), 0);

    auto op = IoOperation::makeRead (_server.handle (), _buf, sizeof (_buf), this);

    ASSERT_EQ (HybridProactorThread::proactor ().submit (&op, true), 0) << join::lastError.message ();
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
TEST_F (HybridProactorTest, onError)
{
    ASSERT_EQ (_client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    ASSERT_EQ (HybridProactorThread::affinity (0), 0) << join::lastError.message ();
    ASSERT_EQ (HybridProactorThread::affinity (), 0);
    ASSERT_EQ (HybridProactorThread::priority (1), 0) << join::lastError.message ();
    ASSERT_EQ (HybridProactorThread::priority (), 1);
#ifdef JOIN_HAS_NUMA
    ASSERT_EQ (HybridProactorThread::mbind (0), 0) << join::lastError.message ();
#endif
    ASSERT_EQ (HybridProactorThread::mlock (), 0) << join::lastError.message ();
    ASSERT_GT (HybridProactorThread::handle (), 0);

    auto op = IoOperation::makeRead (_server.handle (), _buf, sizeof (_buf), this);

    ASSERT_EQ (HybridProactorThread::proactor ().submit (&op, true), 0) << join::lastError.message ();
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
