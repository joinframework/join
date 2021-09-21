/**
 * MIT License
 *
 * Copyright (c) 2021 Mathieu Rabine
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
#include <join/acceptor.hpp>

// Libraries.
#include <gtest/gtest.h>

// C++.
#include <condition_variable>

using join::Errc;
using join::Tcp;

using namespace std::chrono_literals;

/**
 * @brief Class used to test the Observer API.
 */
class Observer : public ::testing::Test, public Tcp::Socket::Observer
{
protected:
    /**
     * @brief Sets up test.
     */
    void SetUp ()
    {
        ASSERT_EQ (open (), 0) << join::lastError.message ();
        ASSERT_EQ (setMode (Tcp::Socket::Blocking), 0) << join::lastError.message ();
    }

    /**
     * @brief Tears down test.
     */
    void TearDown ()
    {
        ASSERT_EQ (close (), 0) << join::lastError.message ();
    }

    /**
     * @brief method called on receive.
     */
    virtual void onReceive () override
    {
        std::lock_guard <std::mutex> lock (mut_);
        data_ = "onReceive";
        cond_.notify_all ();
    }

    /**
     * @brief method called on error.
     */
    virtual void onError () override
    {
        std::lock_guard <std::mutex> lock (mut_);
        BasicObserver::onError ();
        data_ = "onError";
        cond_.notify_all ();
    }

    /**
     * @brief method called on close.
     */
    virtual void onClose () override
    {
        std::lock_guard <std::mutex> lock (mut_);
        BasicObserver::onClose ();
        data_ = "onClose";
        cond_.notify_all ();
    }

    /// host.
    static const std::string host_;

    /// port.
    static const uint16_t port_;

    /// timeout.
    static const int timeout_;

    /// condition variable.
    static std::condition_variable cond_;

    /// condition mutex.
    static std::mutex mut_;

    /// data.
    static std::string data_;
};

const std::string       Observer::host_    = "localhost";
const uint16_t          Observer::port_    = 5000;
const int               Observer::timeout_ = 1000;
std::condition_variable Observer::cond_;
std::mutex              Observer::mut_;
std::string             Observer::data_;

/**
 * @brief Test start method.
 */
TEST_F (Observer, start)
{
    ASSERT_EQ (start (), 0) << join::lastError.message ();

    ASSERT_EQ (start (), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);

    ASSERT_EQ (stop (), 0) << join::lastError.message ();
    ASSERT_EQ (close (), 0) << join::lastError.message ();
    ASSERT_EQ (start (), -1);
}

/**
 * @brief Test stop method.
 */
TEST_F (Observer, stop)
{
    ASSERT_EQ (stop (), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (start (), 0) << join::lastError.message ();
    ASSERT_EQ (stop (), 0) << join::lastError.message ();

    std::this_thread::sleep_for (100ms);
    ASSERT_EQ (start (), 0) << join::lastError.message ();
    ASSERT_EQ (close (), 0) << join::lastError.message ();
    ASSERT_EQ (stop (), -1);
}

/**
 * @brief Test onReceive method.
 */
TEST_F (Observer, onReceive)
{
    Tcp::Acceptor server;
    Tcp::Socket socket;

    ASSERT_EQ (server.bind ({Tcp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_EQ (server.listen (), 0) << join::lastError.message ();
    ASSERT_EQ (connect ({Tcp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_TRUE ((socket = server.accept ()).connected ());
    ASSERT_EQ (start (), 0) << join::lastError.message ();
    ASSERT_EQ (socket.writeData ("onReceive", strlen ("onReceive"), timeout_), 0) << join::lastError.message ();
    {
        std::unique_lock <std::mutex> lk (mut_);
        ASSERT_TRUE (cond_.wait_for (lk, std::chrono::milliseconds (timeout_), [this] () {return data_ == "onReceive";}));
        data_.clear ();
    }
    ASSERT_EQ (stop (), 0) << join::lastError.message ();
    ASSERT_EQ (socket.close (), 0) << join::lastError.message ();
    ASSERT_EQ (server.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test onError method.
 */
TEST_F (Observer, onError)
{
    Tcp::Acceptor server;
    Tcp::Socket socket;

    struct linger sl;
    sl.l_onoff = 1;
    sl.l_linger = 0;

    ASSERT_EQ (server.bind ({Tcp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_EQ (server.listen (), 0) << join::lastError.message ();
    ASSERT_EQ (connect ({Tcp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_TRUE ((socket = server.accept ()).connected ());
    ASSERT_EQ (start (), 0) << join::lastError.message ();
    ASSERT_EQ (setsockopt (socket.handle (), SOL_SOCKET, SO_LINGER, &sl, sizeof (sl)), 0) << strerror (errno);
    ASSERT_EQ (socket.close (), 0) << join::lastError.message ();
    {
        std::unique_lock <std::mutex> lk (mut_);
        ASSERT_TRUE (cond_.wait_for (lk, std::chrono::milliseconds (timeout_), [this] () {return data_ == "onError";}));
        data_.clear ();
    }
    ASSERT_EQ (stop (), 0) << join::lastError.message ();
    ASSERT_EQ (server.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test onClose method.
 */
TEST_F (Observer, onClose)
{
    Tcp::Acceptor server;
    Tcp::Socket socket;

    ASSERT_EQ (server.bind ({Tcp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_EQ (server.listen (), 0) << join::lastError.message ();
    ASSERT_EQ (connect ({Tcp::Resolver::resolveHost (host_), port_}), 0) << join::lastError.message ();
    ASSERT_TRUE ((socket = server.accept ()).connected ());
    ASSERT_EQ (start (), 0) << join::lastError.message ();
    ASSERT_EQ (socket.close (), 0) << join::lastError.message ();
    {
        std::unique_lock <std::mutex> lk (mut_);
        ASSERT_TRUE (cond_.wait_for (lk, std::chrono::milliseconds (timeout_), [this] () {return data_ == "onClose";}));
        data_.clear ();
    }
    ASSERT_EQ (stop (), 0) << join::lastError.message ();
    ASSERT_EQ (server.close (), 0) << join::lastError.message ();
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
   testing::InitGoogleTest (&argc, argv);
   return RUN_ALL_TESTS ();
}
