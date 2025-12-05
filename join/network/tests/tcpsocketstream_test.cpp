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
#include <join/reactor.hpp>
#include <join/acceptor.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::Errc;
using join::IpAddress;
using join::Resolver;
using join::Reactor;
using join::Tcp;

/**
 * @brief Class used to test the TCP socket stream API.
 */
class TcpSocketStream : public Tcp::Acceptor, public ::testing::Test
{
protected:
    /**
     * @brief Sets up the test fixture.
     */
    void SetUp () override
    {
        ASSERT_EQ (this->create ({IpAddress::ipv6Wildcard, _port}), 0) << join::lastError.message ();
        ASSERT_EQ (Reactor::instance ()->addHandler (this), 0) << join::lastError.message ();
    }

    /**
     * @brief Tears down the test fixture.
     */
    void TearDown () override
    {
        ASSERT_EQ (Reactor::instance ()->delHandler (this), 0) << join::lastError.message ();
        this->close ();
    }

    /**
     * @brief method called when data are ready to be read on handle.
     */
    virtual void onReceive () override
    {
        Tcp::Socket sock = this->accept ();
        if (sock.connected ())
        {
            char buf[1024];
            for (;;)
            {
                // echo received data.
                int nread = sock.read (buf, sizeof (buf));
                if (nread == -1)
                {
                    if (join::lastError == Errc::TemporaryError)
                    {
                        if (sock.waitReadyRead (_timeout))
                            continue;
                    }
                    break;
                }
                sock.writeExactly (buf, nread);
            }
            sock.close ();
        }
    }

    /// timeout.
    static const int _timeout;

    /// host.
    static const std::string _host;

    /// port.
    static const uint16_t _port;
    static const uint16_t _invalid_port;
};

const int         TcpSocketStream::_timeout = 1000;
const std::string TcpSocketStream::_host = "localhost";
const uint16_t    TcpSocketStream::_port = 5000;
const uint16_t    TcpSocketStream::_invalid_port = 5032;

/**
 * @brief Test default constructor.
 */
TEST_F (TcpSocketStream, defaultConstruct)
{
    Tcp::Stream tcpStream;
    ASSERT_TRUE (tcpStream.good ()) << join::lastError.message ();
}

/**
 * @brief Test move constructor.
 */
TEST_F (TcpSocketStream, moveConstruct)
{
    Tcp::Stream tmp;
    ASSERT_TRUE (tmp.good ()) << join::lastError.message ();
    Tcp::Stream tcpStream (std::move (tmp));
    ASSERT_TRUE (tcpStream.good ()) << join::lastError.message ();
}

/**
 * @brief Test move operatore.
 */
TEST_F (TcpSocketStream, moveAssign)
{
    Tcp::Stream tmp;
    ASSERT_TRUE (tmp.good ()) << join::lastError.message ();
    Tcp::Stream tcpStream;
    tcpStream = std::move (tmp);
    ASSERT_TRUE (tcpStream.good ()) << join::lastError.message ();
}

/**
 * @brief Test bind method.
 */
TEST_F (TcpSocketStream, bind)
{
    Tcp::Stream tcpStream;
    tcpStream.connect ({Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (tcpStream.good ()) << join::lastError.message ();
    tcpStream.bind (_host);
    ASSERT_TRUE (tcpStream.fail ());
    tcpStream.clear ();
    tcpStream.disconnect ();
    ASSERT_TRUE (tcpStream.good ()) << join::lastError.message ();
    tcpStream.bind (_host);
    ASSERT_TRUE (tcpStream.good ()) << join::lastError.message ();
    tcpStream.connect ({Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (tcpStream.good ()) << join::lastError.message ();
    tcpStream.disconnect ();
    ASSERT_TRUE (tcpStream.good ()) << join::lastError.message ();
    tcpStream.close ();
}

/**
 * @brief Test connect method.
 */
TEST_F (TcpSocketStream, connect)
{
    Tcp::Stream tcpStream;
    tcpStream.connect ({"255.255.255.255", _port});
    ASSERT_TRUE (tcpStream.fail ());
    tcpStream.clear ();
    tcpStream.connect ({Resolver::resolveHost (_host), _invalid_port});
    ASSERT_TRUE (tcpStream.fail ());
    tcpStream.clear ();
    tcpStream.connect ({Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (tcpStream.good ()) << join::lastError.message ();
    tcpStream.close ();
}

/**
 * @brief Test disconnect method.
 */
TEST_F (TcpSocketStream, disconnect)
{
    Tcp::Stream tcpStream;
    ASSERT_FALSE (tcpStream.connected ());
    tcpStream.connect ({Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (tcpStream.good ()) << join::lastError.message ();
    ASSERT_TRUE (tcpStream.connected ());
    tcpStream.disconnect ();
    ASSERT_TRUE (tcpStream.good ()) << join::lastError.message ();
    ASSERT_FALSE (tcpStream.connected ());
    tcpStream.close ();
    ASSERT_FALSE (tcpStream.connected ());
}

/**
 * @brief Test close method.
 */
TEST_F (TcpSocketStream, close)
{
    Tcp::Stream tcpStream;
    tcpStream.connect ({Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (tcpStream.good ()) << join::lastError.message ();
    tcpStream.close ();
    ASSERT_TRUE (tcpStream.good ()) << join::lastError.message ();
}

/**
 * @brief Test localEndpoint method.
 */
TEST_F (TcpSocketStream, localEndpoint)
{
    Tcp::Stream tcpStream;
    ASSERT_EQ (tcpStream.localEndpoint (), Tcp::Endpoint {});
    ASSERT_EQ (tcpStream.socket ().bind ({Resolver::resolveHost (_host), uint16_t (_port + 1)}), 0) << join::lastError.message ();
    tcpStream.connect ({Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (tcpStream.good ()) << join::lastError.message ();
    ASSERT_TRUE (tcpStream.connected ());
    ASSERT_EQ (tcpStream.localEndpoint (), Tcp::Endpoint (Resolver::resolveHost (_host), uint16_t (_port + 1))) << join::lastError.message ();
    tcpStream.close ();
}

/**
 * @brief Test remoteEndpoint method.
 */
TEST_F (TcpSocketStream, remoteEndpoint)
{
    Tcp::Stream tcpStream;
    ASSERT_EQ (tcpStream.remoteEndpoint (), Tcp::Endpoint {});
    tcpStream.connect ({Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (tcpStream.good ()) << join::lastError.message ();
    ASSERT_TRUE (tcpStream.connected ());
    ASSERT_EQ (tcpStream.remoteEndpoint (), Tcp::Endpoint (Resolver::resolveHost (_host), _port)) << join::lastError.message ();
    tcpStream.close ();
}

/**
 * @brief Test opened method.
 */
TEST_F (TcpSocketStream, opened)
{
    Tcp::Stream tcpStream;
    ASSERT_FALSE (tcpStream.opened ());
    tcpStream.connect ({Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (tcpStream.good ()) << join::lastError.message ();
    ASSERT_TRUE (tcpStream.opened ());
    tcpStream.disconnect ();
    ASSERT_TRUE (tcpStream.good ()) << join::lastError.message ();
    ASSERT_FALSE (tcpStream.opened ());
    tcpStream.close ();
    ASSERT_FALSE (tcpStream.opened ());
}

/**
 * @brief Test connected method.
 */
TEST_F (TcpSocketStream, connected)
{
    Tcp::Stream tcpStream;
    ASSERT_FALSE (tcpStream.connected ());
    tcpStream.connect ({Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (tcpStream.good ()) << join::lastError.message ();
    ASSERT_TRUE (tcpStream.connected ());
    tcpStream.close ();
    ASSERT_TRUE (tcpStream.good ()) << join::lastError.message ();
    ASSERT_FALSE (tcpStream.connected ());
}

/**
 * @brief Test encrypted method.
 */
TEST_F (TcpSocketStream, encrypted)
{
    Tcp::Stream tcpStream;
    ASSERT_FALSE (tcpStream.encrypted ());
    tcpStream.connect ({Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (tcpStream.good ()) << join::lastError.message ();
    ASSERT_TRUE (tcpStream.connected ());
    ASSERT_FALSE (tcpStream.encrypted ());
    tcpStream.close ();
    ASSERT_TRUE (tcpStream.good ()) << join::lastError.message ();
    ASSERT_FALSE (tcpStream.connected ());
    ASSERT_FALSE (tcpStream.encrypted ());
}

/**
 * @brief Test timeout method.
 */
TEST_F (TcpSocketStream, timeout)
{
    Tcp::Stream tcpStream;
    ASSERT_NE (tcpStream.timeout (), _timeout);
    tcpStream.timeout (_timeout);
    ASSERT_EQ (tcpStream.timeout (), _timeout);
}

/**
 * @brief Test socket method.
 */
TEST_F (TcpSocketStream, socket)
{
    Tcp::Stream tcpStream;
    ASSERT_EQ (tcpStream.socket ().handle (), -1);
    tcpStream.connect ({Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (tcpStream.good ()) << join::lastError.message ();
    ASSERT_NE (tcpStream.socket ().handle (), -1);
    tcpStream.close ();
    ASSERT_TRUE (tcpStream.good ()) << join::lastError.message ();
    ASSERT_EQ (tcpStream.socket ().handle (), -1);
}

/**
 * @brief Test insert operator.
 */
TEST_F (TcpSocketStream, insert)
{
    Tcp::Stream tcpStream;
    tcpStream << "test" << std::endl;
    ASSERT_TRUE (tcpStream.fail ());
    ASSERT_EQ (join::lastError, Errc::ConnectionClosed);
    tcpStream.clear ();
    tcpStream.connect ({Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (tcpStream.good ()) << join::lastError.message ();
    tcpStream << "test" << std::endl;
    tcpStream.flush ();
    ASSERT_TRUE (tcpStream.socket ().waitReadyRead (_timeout));
    ASSERT_TRUE (tcpStream.good ()) << join::lastError.message ();
    tcpStream.close ();
}

/**
 * @brief Test put method.
 */
TEST_F (TcpSocketStream, put)
{
    Tcp::Stream tcpStream;
    tcpStream.put ('t');
    ASSERT_TRUE (tcpStream.fail ());
    ASSERT_EQ (join::lastError, Errc::ConnectionClosed);
    tcpStream.clear ();
    tcpStream.connect ({Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (tcpStream.good ()) << join::lastError.message ();
    tcpStream.put ('t');
    tcpStream.put ('e');
    tcpStream.put ('s');
    tcpStream.put ('t');
    tcpStream.flush ();
    ASSERT_TRUE (tcpStream.socket ().waitReadyRead (_timeout));
    ASSERT_TRUE (tcpStream.good ()) << join::lastError.message ();
    tcpStream.close ();
}

/**
 * @brief Test write method.
 */
TEST_F (TcpSocketStream, write)
{
    Tcp::Stream tcpStream;
    tcpStream.write ("test", 4);
    ASSERT_TRUE (tcpStream.fail ());
    ASSERT_EQ (join::lastError, Errc::ConnectionClosed);
    tcpStream.clear ();
    tcpStream.connect ({Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (tcpStream.good ()) << join::lastError.message ();
    tcpStream.write ("test", 4);
    tcpStream.flush ();
    ASSERT_TRUE (tcpStream.socket ().waitReadyRead (_timeout));
    ASSERT_TRUE (tcpStream.good ()) << join::lastError.message ();
    tcpStream.close ();
}

/**
 * @brief Test flush method.
 */
TEST_F (TcpSocketStream, flush)
{
    Tcp::Stream tcpStream;
    tcpStream.connect ({Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (tcpStream.good ()) << join::lastError.message ();
    tcpStream.put ('t');
    tcpStream.flush ();
    tcpStream.put ('e');
    tcpStream.flush ();
    tcpStream.put ('s');
    tcpStream.flush ();
    tcpStream.put ('t');
    tcpStream.flush ();
    ASSERT_TRUE (tcpStream.socket ().waitReadyRead (_timeout));
    ASSERT_TRUE (tcpStream.good ()) << join::lastError.message ();
    tcpStream.close ();
}

/**
 * @brief Test extract method.
 */
TEST_F (TcpSocketStream, extract)
{
    int test;
    Tcp::Stream tcpStream;
    tcpStream >> test;
    ASSERT_TRUE (tcpStream.fail ());
    ASSERT_EQ (join::lastError, Errc::ConnectionClosed);
    tcpStream.clear ();
    tcpStream.connect ({Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (tcpStream.good ()) << join::lastError.message ();
    tcpStream << int (123456789) << std::endl;
    tcpStream.flush ();
    tcpStream >> test;
    ASSERT_EQ (test, 123456789);
    tcpStream.close ();
}

/**
 * @brief Test get method.
 */
TEST_F (TcpSocketStream, get)
{
    Tcp::Stream tcpStream;
    tcpStream.get ();
    ASSERT_TRUE (tcpStream.fail ());
    ASSERT_EQ (join::lastError, Errc::ConnectionClosed);
    tcpStream.clear ();
    tcpStream.connect ({Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (tcpStream.good ()) << join::lastError.message ();
    tcpStream.write ("test", 4);
    tcpStream.flush ();
    ASSERT_EQ (tcpStream.get (), 't');
    ASSERT_EQ (tcpStream.get (), 'e');
    ASSERT_EQ (tcpStream.get (), 's');
    ASSERT_EQ (tcpStream.get (), 't');
    tcpStream.close ();
}

/**
 * @brief Test peek method.
 */
TEST_F (TcpSocketStream, peek)
{
    Tcp::Stream tcpStream;
    tcpStream.connect ({Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (tcpStream.good ()) << join::lastError.message ();
    tcpStream.write ("test", 4);
    tcpStream.flush ();
    ASSERT_EQ (tcpStream.peek (), 't');
    ASSERT_EQ (tcpStream.get (), 't');
    ASSERT_EQ (tcpStream.peek (), 'e');
    ASSERT_EQ (tcpStream.get (), 'e');
    ASSERT_EQ (tcpStream.peek (), 's');
    ASSERT_EQ (tcpStream.get (), 's');
    ASSERT_EQ (tcpStream.peek (), 't');
    ASSERT_EQ (tcpStream.get (), 't');
    tcpStream.close ();
}

/**
 * @brief Test getline method.
 */
TEST_F (TcpSocketStream, getline)
{
    Tcp::Stream tcpStream;
    tcpStream.connect ({Resolver::resolveHost (_host), _port});
    tcpStream.write ("test\n", 5);
    tcpStream.flush ();
    std::array <char, 32> test = {};
    tcpStream.getline (test.data (), test.size (), '\n');
    ASSERT_STREQ (test.data (), "test");
    tcpStream.close ();
}

/**
 * @brief Test ignore method.
 */
TEST_F (TcpSocketStream, ignore)
{
    Tcp::Stream tcpStream;
    tcpStream.connect ({Resolver::resolveHost (_host), _port});
    tcpStream.write ("test\n", 5);
    tcpStream.flush ();
    tcpStream.ignore (std::numeric_limits <std::streamsize>::max (), 'e');
    ASSERT_EQ (tcpStream.get (), 's');
    ASSERT_EQ (tcpStream.get (), 't');
    tcpStream.close ();
}

/**
 * @brief Test read method.
 */
TEST_F (TcpSocketStream, read)
{
    Tcp::Stream tcpStream;
    tcpStream.connect ({Resolver::resolveHost (_host), _port});
    tcpStream.write ("test", 4);
    tcpStream.flush ();
    std::array <char, 32> test = {};
    tcpStream.read (test.data (), 4);
    ASSERT_STREQ (test.data (), "test");
    tcpStream.close ();
}

/**
 * @brief Test readsome method.
 */
TEST_F (TcpSocketStream, DISABLED_readsome)
{
    Tcp::Stream tcpStream;
    tcpStream.connect ({Resolver::resolveHost (_host), _port});
    tcpStream.write ("test", 4);
    tcpStream.flush ();
    std::array <char, 32> test = {};
    ASSERT_EQ (tcpStream.readsome (test.data (), test.size ()), 4);
    ASSERT_STREQ (test.data (), "test");
    tcpStream.close ();
}

/**
 * @brief Test gcount method.
 */
TEST_F (TcpSocketStream, gcount)
{
    Tcp::Stream tcpStream;
    tcpStream.connect ({Resolver::resolveHost (_host), _port});
    tcpStream.write ("test", 4);
    tcpStream.flush ();
    std::array <char, 32> test = {};
    tcpStream.read (test.data (), 4);
    ASSERT_EQ (tcpStream.gcount (), 4);
    tcpStream.close ();
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
