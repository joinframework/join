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
using join::LocalMem;

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
        auto& proactor = HybridProactorThread::proactor ();
        proactor.cancel (&_readOp, true, true);
        proactor.cancel (&_writeOp, true, true);
        proactor.cancel (&_spareOp, true, true);
        proactor.cancel (&_invalidOp, true, true);
        proactor.cancel (&_resubmitOp, true, true);

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
        if (_resubmits > 0)
        {
            --_resubmits;
            _resubmitted = HybridProactorThread::proactor ().submit (op, true, true);
            _rejected = HybridProactorThread::proactor ().submit (op, true, true);
            HybridProactorThread::proactor ().flush (true);
        }

        {
            ScopedLock<Mutex> lock (_mut);
            if ((op->ring != nullptr) && (result > 0))
            {
                ::memcpy (_buf, op->data.stream.buf, result);
            }
            _result = result;
            _op = op;
            ++_completions;
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
        if (_resubmits > 0)
        {
            --_resubmits;
            _resubmitted = HybridProactorThread::proactor ().submit (op, true, true);
            _rejected = HybridProactorThread::proactor ().submit (op, true, true);
            HybridProactorThread::proactor ().flush (true);
        }

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
    static const std::chrono::milliseconds _timeout;

    /// condition variable.
    static Condition _cond;

    /// condition mutex.
    static Mutex _mut;

    /// last completed operation.
    static IoOperation* _op;

    /// read side operation submitted by the tests.
    static IoOperation _readOp;

    /// write side operation submitted by the tests.
    static IoOperation _writeOp;

    /// spare operation for the tests submitting two operations on the same handle.
    static IoOperation _spareOp;

    /// operation on an invalid handle.
    static IoOperation _invalidOp;

    /// operation resubmitted from a handler.
    static IoOperation _resubmitOp;

    /// number of resubmissions left to perform from a handler.
    static int _resubmits;

    /// result of the resubmission performed from a handler.
    static int _resubmitted;

    /// result of the rejected resubmission performed from a handler.
    static int _rejected;

    /// last operation result.
    static int _result;

    /// number of completions received.
    static int _completions;

    /// read buffer.
    static char _buf[256];
};

Tcp::Acceptor HybridProactorTest::_acceptor;
Tcp::Socket HybridProactorTest::_client (Tcp::Socket::NonBlocking);
Tcp::Socket HybridProactorTest::_server;
std::string HybridProactorTest::_host = "127.0.0.1";
uint16_t HybridProactorTest::_port = 5001;
const std::chrono::milliseconds HybridProactorTest::_timeout{1000};
Condition HybridProactorTest::_cond;
Mutex HybridProactorTest::_mut;
IoOperation* HybridProactorTest::_op = nullptr;
IoOperation HybridProactorTest::_readOp = {};
IoOperation HybridProactorTest::_writeOp = {};
IoOperation HybridProactorTest::_spareOp = {};
IoOperation HybridProactorTest::_invalidOp = {};
IoOperation HybridProactorTest::_resubmitOp = {};
int HybridProactorTest::_resubmits = 0;
int HybridProactorTest::_resubmitted = 0;
int HybridProactorTest::_rejected = 0;
int HybridProactorTest::_result = 0;
int HybridProactorTest::_completions = 0;
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

    if (_client.connect ({_host, _port}) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (_client.waitConnected (_timeout)) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    _readOp = IoOperation::makeRead (_server.handle (), _buf, sizeof (_buf), this);
    ASSERT_EQ (proactor.submit (&_readOp, true, true), 0) << join::lastError.message ();

    proactor.stop ();
    th.join ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_EQ (_op, &_readOp);
        ASSERT_EQ (_result, -ECANCELED);
        _op = nullptr;
        _result = 0;
    }

    for (int i = 0; i < 32; ++i)
    {
        HybridProactor concurrent;
        Thread loop ([&concurrent] () {
            concurrent.run ();
        });
        while (!concurrent.isRunning ())
        {
        }

        std::atomic<bool> go{false};
        auto stopper = [&concurrent, &go] () {
            while (!go.load (std::memory_order_acquire))
            {
            }
            concurrent.stop ();
        };

        Thread first (stopper);
        Thread second (stopper);
        Thread third (stopper);
        Thread fourth (stopper);

        go.store (true, std::memory_order_release);

        first.join ();
        second.join ();
        third.join ();
        fourth.join ();

        ASSERT_FALSE (concurrent.isRunning ());

        loop.join ();
    }

    Thread orphan;
    {
        HybridProactor dying;
        orphan = Thread ([&dying] () {
            dying.run ();
        });
        while (!dying.isRunning ())
        {
        }
    }
    orphan.join ();
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

    ASSERT_EQ (proactor.submit (nullptr, true, true), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);

    _readOp = IoOperation::makeRead (-1, _buf, sizeof (_buf), this);
    ASSERT_EQ (proactor.submit (&_readOp, true, true), -1);
    ASSERT_EQ (join::lastError, std::errc::bad_file_descriptor);

    if (_client.connect ({_host, _port}) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (_client.waitConnected (_timeout)) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    _readOp = IoOperation::makeRead (_server.handle (), _buf, sizeof (_buf), this);
    _readOp.state = IoOperation::State::Submitted;
    ASSERT_EQ (proactor.submit (&_readOp, true, true), -1);
    ASSERT_EQ (join::lastError, std::errc::device_or_resource_busy);

    _readOp.state = IoOperation::State::Idle;
    ASSERT_EQ (proactor.submit (&_readOp, true, true), 0) << join::lastError.message ();

    _invalidOp = IoOperation::makeRead (-1, _buf, sizeof (_buf), this);
    ASSERT_EQ (proactor.submit (&_invalidOp, true, false), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
            return _op == &_invalidOp && _result == -EBADF;
        }));
        _op = nullptr;
        _result = 0;
    }

#ifndef JOIN_HAS_IO_URING
    _spareOp = IoOperation::makeRead (_server.handle (), _buf, sizeof (_buf), this);
    ASSERT_EQ (proactor.submit (&_spareOp, true, true), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);

    ASSERT_EQ (proactor.submit (&_spareOp, true, false), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
            return _op == &_spareOp && _result == -EINVAL;
        }));
        _op = nullptr;
        _result = 0;
    }
#endif

    ASSERT_EQ (proactor.cancel (&_readOp, true, true), 0) << join::lastError.message ();
    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
            return _op == &_readOp && _result == -ECANCELED;
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

    ASSERT_EQ (proactor.cancel (nullptr, true, true), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);

    _readOp = IoOperation::makeRead (-1, _buf, sizeof (_buf), this);
    ASSERT_EQ (proactor.cancel (&_readOp, true, true), -1);
    ASSERT_EQ (join::lastError, std::errc::bad_file_descriptor);

    if (_client.connect ({_host, _port}) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (_client.waitConnected (_timeout)) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    _readOp = IoOperation::makeRead (_server.handle (), _buf, sizeof (_buf), this);
    ASSERT_EQ (proactor.cancel (&_readOp, true, true), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (proactor.submit (&_readOp, true, true), 0) << join::lastError.message ();

    _spareOp = IoOperation::makeRead (_server.handle (), _buf, sizeof (_buf), this);
    _spareOp.state = IoOperation::State::Submitted;
    ASSERT_EQ (proactor.cancel (&_spareOp, true, true), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);

    ASSERT_EQ (proactor.cancel (&_readOp, true, true), 0) << join::lastError.message ();
    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
            return _op == &_readOp && _result == -ECANCELED;
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

    if (_client.connect ({_host, _port}) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (_client.waitConnected (_timeout)) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    _readOp = IoOperation::makeRead (_server.handle (), _buf, sizeof (_buf), this);
    ASSERT_EQ (proactor.submit (&_readOp, false, true), 0) << join::lastError.message ();
    ASSERT_EQ (proactor.flush (true), 0) << join::lastError.message ();

    ASSERT_EQ (_client.writeExactly ("flush", strlen ("flush"), _timeout), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
            return _op == &_readOp && _result > 0;
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

    if (_client.connect ({_host, _port}) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (_client.waitConnected (_timeout)) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    _writeOp = IoOperation::makeWrite (_server.handle (), "ping", 4, this, true);
    _readOp = IoOperation::makeRead (_server.handle (), _buf, sizeof (_buf), this);

    ASSERT_EQ (proactor.submit (&_writeOp, false, true), 0) << join::lastError.message ();
    ASSERT_EQ (proactor.submit (&_readOp, true, true), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
            return _op == &_writeOp && _result == 4;
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
            return _op == &_readOp && _result > 0;
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

/**
 * @brief Test waitStopped.
 */
TEST_F (HybridProactorTest, waitStopped)
{
    HybridProactor proactor;

    proactor.waitStopped ();

    Thread th ([&proactor] () {
        proactor.run ();
    });
    while (!proactor.isRunning ())
    {
    }

    proactor.stop (false);
    proactor.waitStopped ();

    ASSERT_FALSE (proactor.isRunning ());

    th.join ();
}

#ifdef JOIN_HAS_IO_URING
/**
 * @brief Test registerFixedBuffers and unregisterFixedBuffers.
 */
TEST_F (HybridProactorTest, registerFixedBuffers)
{
    HybridProactor proactor;

    LocalMem::Allocator<1, 1024, 4096> arena;

    ASSERT_EQ (proactor.registerFixedBuffers (arena), 0) << join::lastError.message ();
    ASSERT_EQ (proactor.registerFixedBuffers (arena), -1);
    ASSERT_EQ (proactor.unregisterFixedBuffers (), 0) << join::lastError.message ();
    ASSERT_EQ (proactor.registerFixedBuffers (arena), 0) << join::lastError.message ();
    ASSERT_EQ (proactor.unregisterFixedBuffers (), 0) << join::lastError.message ();
    ASSERT_EQ (proactor.unregisterFixedBuffers (), -1);
}
#endif

/**
 * @brief Test registerBufferRing and unregisterBufferRing.
 */
TEST_F (HybridProactorTest, registerBufferRing)
{
    HybridProactor proactor;

    LocalMem::Allocator<4, 256> arena;

    ASSERT_EQ (proactor.unregisterBufferRing (0), -1);
    ASSERT_EQ (join::lastError, Errc::NotFound);

    ASSERT_EQ (proactor.registerBufferRing (0, arena), 0) << join::lastError.message ();
    ASSERT_EQ (proactor.registerBufferRing (0, arena), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    ASSERT_EQ (arena.tryAllocate (256), nullptr);

    ASSERT_EQ (proactor.unregisterBufferRing (0), 0) << join::lastError.message ();
    ASSERT_NE (arena.tryAllocate (256), nullptr);
}

/**
 * @brief Test async connect.
 */
TEST_F (HybridProactorTest, asyncConnect)
{
    Tcp::Endpoint endpoint{_host, _port};

    ASSERT_EQ (_client.open (Tcp::v4 ()), 0) << join::lastError.message ();

    _readOp = IoOperation::makeConnect (_client.handle (), endpoint.addr (), endpoint.length (), this);
    ASSERT_EQ (HybridProactorThread::proactor ().submit (&_readOp, true, true), 0) << join::lastError.message ();

    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
            return _op == &_readOp && _result == 0;
        }));
        _op = nullptr;
        _result = 0;
    }
}

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
    _readOp = IoOperation::makeAccept (_acceptor.handle (), reinterpret_cast<sockaddr*> (&addr), &addrlen,
                                       SOCK_NONBLOCK | SOCK_CLOEXEC, this);

    ASSERT_EQ (HybridProactorThread::proactor ().submit (&_readOp, true, true), 0) << join::lastError.message ();
    if (_client.connect ({_host, _port}) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (_client.waitConnected (_timeout)) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
            return _op == &_readOp && _result >= 0;
        }));
        ::close (_result);
        _op = nullptr;
        _result = 0;
    }
}

/**
 * @brief Test multishot accept.
 */
TEST_F (HybridProactorTest, asyncAcceptMulti)
{
    _completions = 0;
    _readOp = IoOperation::makeAcceptMulti (_acceptor.handle (), SOCK_NONBLOCK | SOCK_CLOEXEC, this);

    ASSERT_EQ (HybridProactorThread::proactor ().submit (&_readOp, true, true), 0) << join::lastError.message ();

    for (int i = 0; i < 2; ++i)
    {
        Tcp::Socket client (Tcp::Socket::NonBlocking);

        if (client.connect ({_host, _port}) == -1)
        {
            ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
        }
        ASSERT_TRUE (client.waitConnected (_timeout)) << join::lastError.message ();

        {
            ScopedLock<Mutex> lock (_mut);
            ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
                return (_op == &_readOp) && (_completions == (i + 1));
            }));
            ASSERT_GE (_result, 0) << join::lastError.message ();
            ::close (_result);
        }

        client.close ();
    }

    ASSERT_EQ (HybridProactorThread::proactor ().submit (&_readOp, true, true), -1);

    ASSERT_EQ (HybridProactorThread::proactor ().cancel (&_readOp, true, true), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
            return _op == &_readOp && _result == -ECANCELED;
        }));
        _op = nullptr;
        _result = 0;
    }
}

/**
 * @brief Test async write.
 */
TEST_F (HybridProactorTest, asyncWrite)
{
    if (_client.connect ({_host, _port}) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (_client.waitConnected (_timeout)) << join::lastError.message ();
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
    _writeOp = IoOperation::makeWrite (_server.handle (), msg, strlen (msg), this);

    ASSERT_EQ (HybridProactorThread::proactor ().submit (&_writeOp, true, true), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
            return _op == &_writeOp && _result > 0;
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
TEST_F (HybridProactorTest, asyncRead)
{
    if (_client.connect ({_host, _port}) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (_client.waitConnected (_timeout)) << join::lastError.message ();
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

    _readOp = IoOperation::makeRead (_server.handle (), _buf, sizeof (_buf), this);

    ASSERT_EQ (HybridProactorThread::proactor ().submit (&_readOp, true, true), 0) << join::lastError.message ();
    ASSERT_EQ (_client.writeExactly ("asyncRead", strlen ("asyncRead"), _timeout), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
            return _op == &_readOp && _result > 0;
        }));
        ASSERT_EQ (std::string (_buf, _result), "asyncRead");
        _op = nullptr;
        _result = 0;
    }
}

/**
 * @brief Test async write fixed.
 */
TEST_F (HybridProactorTest, asyncWriteFixed)
{
    if (_client.connect ({_host, _port}) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (_client.waitConnected (_timeout)) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    const char* msg = "asyncWriteFixed";
    LocalMem::Allocator<4, sizeof (_buf)> arena;
    arena.tryAllocate (sizeof (_buf));
    void* regbuf = arena.tryAllocate (sizeof (_buf));
    ::memcpy (regbuf, msg, strlen (msg));

    ASSERT_EQ (HybridProactorThread::proactor ().registerFixedBuffers (arena), 0) << join::lastError.message ();

    _writeOp = IoOperation::makeWriteFixed (_server.handle (), regbuf, strlen (msg), 0, this);
    ASSERT_EQ (HybridProactorThread::proactor ().submit (&_writeOp, true, true), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
            return _op == &_writeOp && _result > 0;
        }));
        ASSERT_EQ (_result, static_cast<int> (strlen (msg)));
        _op = nullptr;
        _result = 0;
    }

    char rbuf[256] = {};
    ASSERT_EQ (_client.readExactly (rbuf, strlen (msg), _timeout), 0) << join::lastError.message ();
    ASSERT_EQ (std::string (rbuf, strlen (msg)), msg);

    ASSERT_EQ (HybridProactorThread::proactor ().unregisterFixedBuffers (), 0) << join::lastError.message ();
}

/**
 * @brief Test async read fixed.
 */
TEST_F (HybridProactorTest, asyncReadFixed)
{
    if (_client.connect ({_host, _port}) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (_client.waitConnected (_timeout)) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    LocalMem::Allocator<4, 64, sizeof (_buf)> arena;
    arena.tryAllocate (sizeof (_buf));
    void* regbuf = arena.tryAllocate (sizeof (_buf));

    ASSERT_EQ (HybridProactorThread::proactor ().registerFixedBuffers (arena), 0) << join::lastError.message ();

    _readOp = IoOperation::makeReadFixed (_server.handle (), regbuf, sizeof (_buf), 1, this);
    ASSERT_EQ (HybridProactorThread::proactor ().submit (&_readOp, true, true), 0) << join::lastError.message ();
    ASSERT_EQ (_client.writeExactly ("asyncReadFixed", strlen ("asyncReadFixed"), _timeout), 0)
        << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
            return _op == &_readOp && _result > 0;
        }));
        ASSERT_EQ (std::string (static_cast<char*> (regbuf), _result), "asyncReadFixed");
        _op = nullptr;
        _result = 0;
    }

    ASSERT_EQ (HybridProactorThread::proactor ().unregisterFixedBuffers (), 0) << join::lastError.message ();
}

/**
 * @brief Test async sendmsg.
 */
TEST_F (HybridProactorTest, asyncSendmsg)
{
    if (_client.connect ({_host, _port}) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (_client.waitConnected (_timeout)) << join::lastError.message ();
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
    _writeOp = IoOperation::makeSendmsg (_server.handle (), &msg, 0, this);

    ASSERT_EQ (HybridProactorThread::proactor ().submit (&_writeOp, true, true), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
            return _op == &_writeOp && _result > 0;
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
TEST_F (HybridProactorTest, asyncRecvmsg)
{
    if (_client.connect ({_host, _port}) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (_client.waitConnected (_timeout)) << join::lastError.message ();
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
    _readOp = IoOperation::makeRecvmsg (_server.handle (), &msg, 0, this);

    ASSERT_EQ (HybridProactorThread::proactor ().submit (&_readOp, true, true), 0) << join::lastError.message ();
    ASSERT_EQ (_client.writeExactly ("asyncRecvmsg", strlen ("asyncRecvmsg"), _timeout), 0)
        << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
            return _op == &_readOp && _result > 0;
        }));
        ASSERT_EQ (std::string (_buf, _result), "asyncRecvmsg");
        _op = nullptr;
        _result = 0;
    }
}

/**
 * @brief Test async send.
 */
TEST_F (HybridProactorTest, asyncSend)
{
    if (_client.connect ({_host, _port}) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (_client.waitConnected (_timeout)) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    const char* msg = "asyncSend";
    _writeOp = IoOperation::makeSend (_server.handle (), msg, strlen (msg), 0, this);

    ASSERT_EQ (HybridProactorThread::proactor ().submit (&_writeOp, true, true), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
            return _op == &_writeOp && _result > 0;
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
TEST_F (HybridProactorTest, asyncRecv)
{
    if (_client.connect ({_host, _port}) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (_client.waitConnected (_timeout)) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    _readOp = IoOperation::makeRecv (_server.handle (), _buf, sizeof (_buf), 0, this);

    ASSERT_EQ (HybridProactorThread::proactor ().submit (&_readOp, true, true), 0) << join::lastError.message ();
    ASSERT_EQ (_client.writeExactly ("asyncRecv", strlen ("asyncRecv"), _timeout), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
            return _op == &_readOp && _result > 0;
        }));
        ASSERT_EQ (std::string (_buf, _result), "asyncRecv");
        _op = nullptr;
        _result = 0;
    }
}

/**
 * @brief Test multishot receive.
 */
TEST_F (HybridProactorTest, asyncRecvMulti)
{
    if (_client.connect ({_host, _port}) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (_client.waitConnected (_timeout)) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    LocalMem::Allocator<4, sizeof (_buf)> arena;
    ASSERT_EQ (HybridProactorThread::proactor ().registerBufferRing (0, arena), 0) << join::lastError.message ();

    _completions = 0;
    _readOp = IoOperation::makeRecvMulti (_server.handle (), 0, 0, this);

    ASSERT_EQ (HybridProactorThread::proactor ().submit (&_readOp, true, true), 0) << join::lastError.message ();

    const char* msg = "asyncRecvMulti";

    for (int i = 0; i < 2; ++i)
    {
        ASSERT_EQ (_client.writeExactly (msg, strlen (msg), _timeout), 0) << join::lastError.message ();

        {
            ScopedLock<Mutex> lock (_mut);
            ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
                return (_op == &_readOp) && (_completions == (i + 1));
            }));
            ASSERT_EQ (_result, static_cast<int> (strlen (msg)));
            ASSERT_EQ (std::string (_buf, _result), msg);
        }
    }

    ASSERT_EQ (HybridProactorThread::proactor ().unregisterBufferRing (0), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);

    ASSERT_EQ (HybridProactorThread::proactor ().cancel (&_readOp, true, true), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
            return _op == &_readOp && _result == -ECANCELED;
        }));
        _op = nullptr;
        _result = 0;
    }

    ASSERT_EQ (HybridProactorThread::proactor ().unregisterBufferRing (0), 0) << join::lastError.message ();
}

/**
 * @brief Test onClose.
 */
TEST_F (HybridProactorTest, onClose)
{
    if (_client.connect ({_host, _port}) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (_client.waitConnected (_timeout)) << join::lastError.message ();
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

    _readOp = IoOperation::makeRead (_server.handle (), _buf, sizeof (_buf), this);

    ASSERT_EQ (HybridProactorThread::proactor ().submit (&_readOp, true, true), 0) << join::lastError.message ();
    _client.close ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
            return _op == &_readOp && _result == 0;
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
    if (_client.connect ({_host, _port}) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (_client.waitConnected (_timeout)) << join::lastError.message ();
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

    _readOp = IoOperation::makeRead (_server.handle (), _buf, sizeof (_buf), this);

    ASSERT_EQ (HybridProactorThread::proactor ().submit (&_readOp, true, true), 0) << join::lastError.message ();
    linger sl{.l_onoff = 1, .l_linger = 0};
    ASSERT_EQ (setsockopt (_client.handle (), SOL_SOCKET, SO_LINGER, &sl, sizeof (sl)), 0) << strerror (errno);
    _client.close ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
            return _op == &_readOp && _result == -ECONNRESET;
        }));
        _op = nullptr;
        _result = 0;
    }
}

/**
 * @brief Test resubmission from handler.
 */
TEST_F (HybridProactorTest, resubmit)
{
    const char* msg = "resubmit";

    if (_client.connect ({_host, _port}) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (_client.waitConnected (_timeout)) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    _resubmits = 1;
    _resubmitted = 0;
    _rejected = 0;

    _resubmitOp = IoOperation::makeRead (_server.handle (), _buf, sizeof (_buf), this);
    ASSERT_EQ (HybridProactorThread::proactor ().submit (&_resubmitOp, true, true), 0) << join::lastError.message ();
    ASSERT_EQ (_client.writeExactly (msg, strlen (msg)), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
            return _op == &_resubmitOp && _result > 0;
        }));
        _op = nullptr;
        _result = 0;
    }

    ASSERT_EQ (_resubmitted, 0);
    ASSERT_EQ (_rejected, -1);

    ASSERT_EQ (HybridProactorThread::proactor ().cancel (&_resubmitOp, true, true), 0) << join::lastError.message ();

    {
        ScopedLock<Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {
            return _op == &_resubmitOp && _result == -ECANCELED;
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
