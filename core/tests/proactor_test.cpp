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
using join::Proactor;
using join::ProactorThread;
using join::IoOperation;
using join::CompletionHandler;
using join::Tcp;

/**
 * @brief Class used to test Proactor.
 */
class ProactorTest : public CompletionHandler, public ::testing::Test
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
        ProactorThread::proactor ()->submit (op);
        ProactorThread::proactor ()->cancel (op);

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

Tcp::Acceptor ProactorTest::_acceptor;
Tcp::Socket ProactorTest::_client (Tcp::Socket::Blocking);
Tcp::Socket ProactorTest::_server;
std::string ProactorTest::_host = "127.0.0.1";
uint16_t ProactorTest::_port = 5001;
const int ProactorTest::_timeout = 1000;
Condition ProactorTest::_cond;
Mutex ProactorTest::_mut;
IoOperation* ProactorTest::_op = nullptr;
int ProactorTest::_result = 0;
char ProactorTest::_buf[256] = {};

/**
 * @brief Test makeAccept.
 */
TEST_F (ProactorTest, makeAccept)
{
    sockaddr_storage addr = {};
    socklen_t addrlen = sizeof (addr);

    auto op =
        IoOperation::makeAccept (8, reinterpret_cast<sockaddr*> (&addr), &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC, this);

    ASSERT_EQ (op.fd, 8);
    ASSERT_EQ (op.code, static_cast<uint8_t> (IoOperation::Opcode::Accept));
    ASSERT_EQ (op.handler, this);
    ASSERT_EQ (op.data.accept.addr, reinterpret_cast<sockaddr*> (&addr));
    ASSERT_EQ (op.data.accept.addrlen, &addrlen);
    ASSERT_EQ (op.data.accept.flags, SOCK_NONBLOCK | SOCK_CLOEXEC);
}

/**
 * @brief Test makeConnect.
 */
TEST_F (ProactorTest, makeConnect)
{
    sockaddr_storage addr = {};
    socklen_t addrlen = sizeof (addr);

    auto op = IoOperation::makeConnect (8, reinterpret_cast<sockaddr*> (&addr), addrlen, this);

    ASSERT_EQ (op.fd, 8);
    ASSERT_EQ (op.code, static_cast<uint8_t> (IoOperation::Opcode::Connect));
    ASSERT_EQ (op.handler, this);
    ASSERT_EQ (op.data.connect.addr, reinterpret_cast<sockaddr*> (&addr));
    ASSERT_EQ (op.data.connect.addrlen, addrlen);
}

/**
 * @brief Test makeRead.
 */
TEST_F (ProactorTest, makeRead)
{
    auto op = IoOperation::makeRead (8, _buf, sizeof (_buf), this);

    ASSERT_EQ (op.fd, 8);
    ASSERT_EQ (op.code, static_cast<uint8_t> (IoOperation::Opcode::Read));
    ASSERT_EQ (op.handler, this);
    ASSERT_EQ (op.data.rw.buf, _buf);
    ASSERT_EQ (op.data.rw.len, sizeof (_buf));
    ASSERT_EQ (op.data.rw.index, 0);
    ASSERT_EQ (op.data.rw.fixed, false);
}

/**
 * @brief Test makeWrite.
 */
TEST_F (ProactorTest, makeWrite)
{
    auto op = IoOperation::makeWrite (8, _buf, sizeof (_buf), this);

    ASSERT_EQ (op.fd, 8);
    ASSERT_EQ (op.code, static_cast<uint8_t> (IoOperation::Opcode::Write));
    ASSERT_EQ (op.handler, this);
    ASSERT_EQ (op.data.rw.buf, _buf);
    ASSERT_EQ (op.data.rw.len, sizeof (_buf));
    ASSERT_EQ (op.data.rw.index, 0);
    ASSERT_EQ (op.data.rw.fixed, false);
}

/**
 * @brief Test makeReadFixed.
 */
TEST_F (ProactorTest, makeReadFixed)
{
    auto op = IoOperation::makeReadFixed (8, _buf, sizeof (_buf), 6, this);

    ASSERT_EQ (op.fd, 8);
    ASSERT_EQ (op.code, static_cast<uint8_t> (IoOperation::Opcode::ReadFixed));
    ASSERT_EQ (op.handler, this);
    ASSERT_EQ (op.data.rw.buf, _buf);
    ASSERT_EQ (op.data.rw.len, sizeof (_buf));
    ASSERT_EQ (op.data.rw.index, 6);
    ASSERT_EQ (op.data.rw.fixed, true);
}

/**
 * @brief Test makeWriteFixed.
 */
TEST_F (ProactorTest, makeWriteFixed)
{
    auto op = IoOperation::makeWriteFixed (8, _buf, sizeof (_buf), 6, this);

    ASSERT_EQ (op.fd, 8);
    ASSERT_EQ (op.code, static_cast<uint8_t> (IoOperation::Opcode::WriteFixed));
    ASSERT_EQ (op.handler, this);
    ASSERT_EQ (op.data.rw.buf, _buf);
    ASSERT_EQ (op.data.rw.len, sizeof (_buf));
    ASSERT_EQ (op.data.rw.index, 6);
    ASSERT_EQ (op.data.rw.fixed, true);
}

/**
 * @brief Test makeRecvmsg.
 */
TEST_F (ProactorTest, makeRecvmsg)
{
    iovec iov = {};
    msghdr msg = {};
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    auto op = IoOperation::makeRecvmsg (8, &msg, MSG_DONTWAIT, this);

    ASSERT_EQ (op.fd, 8);
    ASSERT_EQ (op.code, static_cast<uint8_t> (IoOperation::Opcode::RecvMsg));
    ASSERT_EQ (op.handler, this);
    ASSERT_EQ (op.data.msg.msg, &msg);
    ASSERT_EQ (op.data.msg.flags, MSG_DONTWAIT);
}

/**
 * @brief Test makeSendmsg.
 */
TEST_F (ProactorTest, makeSendmsg)
{
    const char* payload = "makeSendmsg";
    iovec iov = {.iov_base = const_cast<char*> (payload), .iov_len = strlen (payload)};
    msghdr msg = {};
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    auto op = IoOperation::makeSendmsg (8, &msg, MSG_DONTWAIT, this);

    ASSERT_EQ (op.fd, 8);
    ASSERT_EQ (op.code, static_cast<uint8_t> (IoOperation::Opcode::SendMsg));
    ASSERT_EQ (op.handler, this);
    ASSERT_EQ (op.data.msg.msg, &msg);
    ASSERT_EQ (op.data.msg.flags, MSG_DONTWAIT);
}

/**
 * @brief Test flush.
 */
TEST_F (ProactorTest, flush)
{
    Proactor proactor;
    Thread th ([&proactor] () {
        proactor.run ();
    });

    ASSERT_EQ (_client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    auto op = IoOperation::makeRead (_server.handle (), _buf, sizeof (_buf), this);
    ASSERT_EQ (proactor.submit (&op), 0) << join::lastError.message ();

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
TEST_F (ProactorTest, submit)
{
    Proactor proactor;
    Thread th ([&proactor] () {
        proactor.run ();
    });

    ASSERT_EQ (proactor.submit (nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);

    auto op1 = IoOperation::makeRead (-1, _buf, sizeof (_buf), this);
    ASSERT_EQ (proactor.submit (&op1), -1);
    ASSERT_EQ (join::lastError, std::errc::bad_file_descriptor);

    op1 = IoOperation::makeRead (2048, _buf, sizeof (_buf), this);
    ASSERT_EQ (proactor.submit (&op1), -1);
    ASSERT_EQ (join::lastError, std::errc::bad_file_descriptor);

    ASSERT_EQ (_client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    op1 = IoOperation::makeRead (_server.handle (), _buf, sizeof (_buf), this);
    op1.state = IoOperation::State::Submitted;
    ASSERT_EQ (proactor.submit (&op1), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    op1.state = IoOperation::State::Idle;
    ASSERT_EQ (proactor.submit (&op1), 0) << join::lastError.message ();

    auto op2 = IoOperation::makeRead (_server.handle (), _buf, sizeof (_buf), this);
    ASSERT_EQ (proactor.submit (&op2), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);

    ASSERT_EQ (proactor.cancel (&op1), 0) << join::lastError.message ();
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
TEST_F (ProactorTest, cancel)
{
    Proactor proactor;
    Thread th ([&proactor] () {
        proactor.run ();
    });

    ASSERT_EQ (proactor.cancel (nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);

    auto op1 = IoOperation::makeRead (-1, _buf, sizeof (_buf), this);
    ASSERT_EQ (proactor.cancel (&op1), -1);
    ASSERT_EQ (join::lastError, std::errc::bad_file_descriptor);

    op1 = IoOperation::makeRead (2048, _buf, sizeof (_buf), this);
    ASSERT_EQ (proactor.submit (&op1), -1);
    ASSERT_EQ (join::lastError, std::errc::bad_file_descriptor);

    ASSERT_EQ (_client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    op1 = IoOperation::makeRead (_server.handle (), _buf, sizeof (_buf), this);
    ASSERT_EQ (proactor.cancel (&op1), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (proactor.submit (&op1), 0) << join::lastError.message ();

    auto op2 = IoOperation::makeRead (_server.handle (), _buf, sizeof (_buf), this);
    op2.state = IoOperation::State::Submitted;
    ASSERT_EQ (proactor.cancel (&op2), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);

    ASSERT_EQ (proactor.cancel (&op1), 0) << join::lastError.message ();
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

#ifdef JOIN_HAS_NUMA
/**
 * @brief Test mbind.
 */
TEST_F (ProactorTest, mbind)
{
    Proactor proactor;

    ASSERT_EQ (proactor.mbind (0), 0) << join::lastError.message ();
}
#endif

/**
 * @brief Test mlock.
 */
TEST_F (ProactorTest, mlock)
{
    Proactor proactor;

    ASSERT_EQ (proactor.mlock (), 0) << join::lastError.message ();
}

/**
 * @brief Test async accept.
 */
TEST_F (ProactorTest, asyncAccept)
{
    ASSERT_EQ (ProactorThread::affinity (0), 0) << join::lastError.message ();
    ASSERT_EQ (ProactorThread::affinity (), 0);
    ASSERT_EQ (ProactorThread::priority (1), 0) << join::lastError.message ();
    ASSERT_EQ (ProactorThread::priority (), 1);
#ifdef JOIN_HAS_NUMA
    ASSERT_EQ (ProactorThread::mbind (0), 0) << join::lastError.message ();
#endif
    ASSERT_EQ (ProactorThread::mlock (), 0) << join::lastError.message ();
    ASSERT_GT (ProactorThread::handle (), 0);

    sockaddr_storage addr = {};
    socklen_t addrlen = sizeof (addr);
    auto op = IoOperation::makeAccept (_acceptor.handle (), reinterpret_cast<sockaddr*> (&addr), &addrlen,
                                       SOCK_NONBLOCK | SOCK_CLOEXEC, this);

    ASSERT_EQ (ProactorThread::proactor ()->submit (&op), 0) << join::lastError.message ();
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
 * @brief Test async read.
 */
TEST_F (ProactorTest, asyncRead)
{
    ASSERT_EQ (_client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    ASSERT_EQ (ProactorThread::affinity (0), 0) << join::lastError.message ();
    ASSERT_EQ (ProactorThread::affinity (), 0);
    ASSERT_EQ (ProactorThread::priority (1), 0) << join::lastError.message ();
    ASSERT_EQ (ProactorThread::priority (), 1);
#ifdef JOIN_HAS_NUMA
    ASSERT_EQ (ProactorThread::mbind (0), 0) << join::lastError.message ();
#endif
    ASSERT_EQ (ProactorThread::mlock (), 0) << join::lastError.message ();
    ASSERT_GT (ProactorThread::handle (), 0);

    auto op = IoOperation::makeRead (_server.handle (), _buf, sizeof (_buf), this);

    ASSERT_EQ (ProactorThread::proactor ()->submit (&op), 0) << join::lastError.message ();
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
TEST_F (ProactorTest, asyncWrite)
{
    ASSERT_EQ (_client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    ASSERT_EQ (ProactorThread::affinity (0), 0) << join::lastError.message ();
    ASSERT_EQ (ProactorThread::affinity (), 0);
    ASSERT_EQ (ProactorThread::priority (1), 0) << join::lastError.message ();
    ASSERT_EQ (ProactorThread::priority (), 1);
#ifdef JOIN_HAS_NUMA
    ASSERT_EQ (ProactorThread::mbind (0), 0) << join::lastError.message ();
#endif
    ASSERT_EQ (ProactorThread::mlock (), 0) << join::lastError.message ();
    ASSERT_GT (ProactorThread::handle (), 0);

    const char* msg = "asyncWrite";
    auto op = IoOperation::makeWrite (_server.handle (), msg, strlen (msg), this);

    ASSERT_EQ (ProactorThread::proactor ()->submit (&op), 0) << join::lastError.message ();

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
 * @brief Test async recvmsg.
 */
TEST_F (ProactorTest, asyncRecvmsg)
{
    ASSERT_EQ (_client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    ASSERT_EQ (ProactorThread::affinity (0), 0) << join::lastError.message ();
    ASSERT_EQ (ProactorThread::affinity (), 0);
    ASSERT_EQ (ProactorThread::priority (1), 0) << join::lastError.message ();
    ASSERT_EQ (ProactorThread::priority (), 1);
#ifdef JOIN_HAS_NUMA
    ASSERT_EQ (ProactorThread::mbind (0), 0) << join::lastError.message ();
#endif
    ASSERT_EQ (ProactorThread::mlock (), 0) << join::lastError.message ();
    ASSERT_GT (ProactorThread::handle (), 0);

    iovec iov = {.iov_base = _buf, .iov_len = sizeof (_buf)};
    msghdr msg = {};
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    auto op = IoOperation::makeRecvmsg (_server.handle (), &msg, 0, this);

    ASSERT_EQ (ProactorThread::proactor ()->submit (&op), 0) << join::lastError.message ();
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
TEST_F (ProactorTest, asyncSendmsg)
{
    ASSERT_EQ (_client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    ASSERT_EQ (ProactorThread::affinity (0), 0) << join::lastError.message ();
    ASSERT_EQ (ProactorThread::affinity (), 0);
    ASSERT_EQ (ProactorThread::priority (1), 0) << join::lastError.message ();
    ASSERT_EQ (ProactorThread::priority (), 1);
#ifdef JOIN_HAS_NUMA
    ASSERT_EQ (ProactorThread::mbind (0), 0) << join::lastError.message ();
#endif
    ASSERT_EQ (ProactorThread::mlock (), 0) << join::lastError.message ();
    ASSERT_GT (ProactorThread::handle (), 0);

    const char* payload = "asyncSendmsg";
    iovec iov = {.iov_base = const_cast<char*> (payload), .iov_len = strlen (payload)};
    msghdr msg = {};
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    auto op = IoOperation::makeSendmsg (_server.handle (), &msg, 0, this);

    ASSERT_EQ (ProactorThread::proactor ()->submit (&op), 0) << join::lastError.message ();

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
TEST_F (ProactorTest, onClose)
{
    ASSERT_EQ (_client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    ASSERT_EQ (ProactorThread::affinity (0), 0) << join::lastError.message ();
    ASSERT_EQ (ProactorThread::affinity (), 0);
    ASSERT_EQ (ProactorThread::priority (1), 0) << join::lastError.message ();
    ASSERT_EQ (ProactorThread::priority (), 1);
#ifdef JOIN_HAS_NUMA
    ASSERT_EQ (ProactorThread::mbind (0), 0) << join::lastError.message ();
#endif
    ASSERT_EQ (ProactorThread::mlock (), 0) << join::lastError.message ();
    ASSERT_GT (ProactorThread::handle (), 0);

    auto op = IoOperation::makeRead (_server.handle (), _buf, sizeof (_buf), this);

    ASSERT_EQ (ProactorThread::proactor ()->submit (&op), 0) << join::lastError.message ();
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
 * @brief Test onError.
 */
TEST_F (ProactorTest, onError)
{
    ASSERT_EQ (_client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    ASSERT_EQ (ProactorThread::affinity (0), 0) << join::lastError.message ();
    ASSERT_EQ (ProactorThread::affinity (), 0);
    ASSERT_EQ (ProactorThread::priority (1), 0) << join::lastError.message ();
    ASSERT_EQ (ProactorThread::priority (), 1);
#ifdef JOIN_HAS_NUMA
    ASSERT_EQ (ProactorThread::mbind (0), 0) << join::lastError.message ();
#endif
    ASSERT_EQ (ProactorThread::mlock (), 0) << join::lastError.message ();
    ASSERT_GT (ProactorThread::handle (), 0);

    auto op = IoOperation::makeRead (_server.handle (), _buf, sizeof (_buf), this);

    ASSERT_EQ (ProactorThread::proactor ()->submit (&op), 0) << join::lastError.message ();
    linger sl{.l_onoff = 1, .l_linger = 0};
    ASSERT_EQ (setsockopt (_client.handle (), SOL_SOCKET, SO_LINGER, &sl, sizeof (sl)), 0) << strerror (errno);
    _client.close ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
            return _op == &op && _result == -EIO;
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
