/**
 * MIT License
 *
 * Copyright (c) 2022 Mathieu Rabine
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
#include <join/reactor.hpp>
#include <join/acceptor.hpp>
#include <join/condition.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::Errc;
using join::Mutex;
using join::Condition;
using join::ScopedLock;
using join::Reactor;
using join::Tcp;

/**
 * @brief Class used to test Reactor.
 */
class ReactorTest : public join::EventHandler, public ::testing::Test
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
     * @brief method called when data are ready to be read/accepted on handle.
     */
    virtual void onReceive () override
    {
        {
            ScopedLock <Mutex> lock (_mut);
            _server.readExactly (_event, _server.canRead ());
            EventHandler::onReceive ();
        }

        _cond.signal ();
    }

    /**
     * @brief method called when handle is closed.
     */
    virtual void onClose () override
    {
        Reactor::instance ()->delHandler (this);
        _server.close ();

        {
            ScopedLock <Mutex> lock (_mut);
            _event = "onClose";
            EventHandler::onClose ();
        }

        _cond.signal ();
    }

    /**
     * @brief method called when an error occured on handle.
     */
    virtual void onError () override
    {
        Reactor::instance ()->delHandler (this);
        _server.close ();

        {
            ScopedLock <Mutex> lock (_mut);
            _event = "onError";
            EventHandler::onError ();
        }

        _cond.signal ();
    }

    /**
     * @brief get native handle.
     * @return native handle.
     */
    virtual int handle () const noexcept override
    {
        return _server.handle ();
    }

    /// server socket.
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

    /// event.
    static std::string _event;
};

Tcp::Acceptor ReactorTest::_acceptor;
Tcp::Socket   ReactorTest::_client (Tcp::Socket::Blocking);
Tcp::Socket   ReactorTest::_server;
std::string   ReactorTest::_host = "127.0.0.1";
uint16_t      ReactorTest::_port = 5000;
const int     ReactorTest::_timeout = 1000;
Condition     ReactorTest::_cond;
Mutex         ReactorTest::_mut;
std::string   ReactorTest::_event;

/**
 * @brief Test instance.
 */
TEST_F (ReactorTest, instance)
{
    ASSERT_NE (Reactor::instance (), nullptr);
}

/**
 * @brief Test addHandler.
 */
TEST_F (ReactorTest, addHandler)
{
    Reactor reactor;

    // test invalid parameter.
    ASSERT_EQ (reactor.addHandler (nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);

    // test invalid handle.
    ASSERT_EQ (reactor.addHandler (this), -1);
    ASSERT_EQ (join::lastError, std::errc::bad_file_descriptor);

    // connect socket.
    ASSERT_EQ (_client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    // add handler.
    ASSERT_EQ (reactor.addHandler (this), 0) << join::lastError.message ();

    // delete handler
    ASSERT_EQ (reactor.delHandler (this), 0) << join::lastError.message ();
}

/**
 * @brief Test delHandler.
 */
TEST_F (ReactorTest, delHandler)
{
    Reactor reactor;

    // test invalid parameter.
    ASSERT_EQ (reactor.delHandler (nullptr), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);

    // test invalid handle.
    ASSERT_EQ (reactor.delHandler (this), -1);
    ASSERT_EQ (join::lastError, std::errc::bad_file_descriptor);

    // connect socket.
    ASSERT_EQ (_client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    // add handler.
    ASSERT_EQ (reactor.addHandler (this), 0) << join::lastError.message ();

    // delete handler
    ASSERT_EQ (reactor.delHandler (this), 0) << join::lastError.message ();
}

/**
 * @brief Test onReceive.
 */
TEST_F (ReactorTest, onReceive)
{
    // connect socket.
    ASSERT_EQ (_client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    // add handler.
    ASSERT_EQ (Reactor::instance ()->addHandler (this), 0) << join::lastError.message ();

    // write random data.
    ASSERT_EQ (_client.writeExactly ("onReceive", strlen ("onReceive"), _timeout), 0) << join::lastError.message ();

    // wait for the onReceive notification.
    {
        ScopedLock <Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {return _event == "onReceive";}));
        _event.clear ();
    }

    // delete handler.
    ASSERT_EQ (Reactor::instance ()->delHandler (this), 0) << join::lastError.message ();
}

/**
 * @brief Test onClose.
 */
TEST_F (ReactorTest, onClose)
{
    // connect socket.
    ASSERT_EQ (_client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    // add handler.
    ASSERT_EQ (Reactor::instance ()->addHandler (this), 0) << join::lastError.message ();

    // close immediately.
    _client.close ();

    // wait for the onClose notification.
    {
        ScopedLock <Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {return _event == "onClose";}));
        _event.clear ();
    }
}

/**
 * @brief Test onError.
 */
TEST_F (ReactorTest, onError)
{
    // connect socket.
    ASSERT_EQ (_client.connect ({_host, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE ((_server = _acceptor.accept ()).connected ()) << join::lastError.message ();

    // add handler.
    ASSERT_EQ (Reactor::instance ()->addHandler (this), 0) << join::lastError.message ();

    // reset connection.
    struct linger sl { .l_onoff = 1, .l_linger = 0 };
    ASSERT_EQ (setsockopt (_client.handle (), SOL_SOCKET, SO_LINGER, &sl, sizeof(sl)), 0) << strerror (errno);
    _client.close ();

    // wait for the onError notification.
    {
        ScopedLock <Mutex> lock (_mut);
        ASSERT_TRUE (_cond.timedWait (lock, std::chrono::milliseconds (_timeout), [&] () {return _event == "onError";}));
        _event.clear ();
    }
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
   testing::InitGoogleTest (&argc, argv);
   return RUN_ALL_TESTS ();
}
