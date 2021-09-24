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
#include <join/socketio.hpp>
#include <join/acceptor.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::Errc;
using join::IpAddress;
using join::Tcp;

/**
 * @brief Class used to test the TCP socket io stream API.
 */
class TcpIoSocket : public ::testing::Test, public Tcp::Acceptor::Observer
{
protected:
    /**
     * @brief Sets up the test fixture.
     */
    void SetUp ()
    {
        ASSERT_EQ (bind ({Tcp::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
        ASSERT_EQ (listen (), 0) << join::lastError.message ();
        ASSERT_EQ (start (), 0) << join::lastError.message ();
    }

    /**
     * @brief Tears down the test fixture.
     */
    void TearDown ()
    {
        ASSERT_EQ (stop (), 0) << join::lastError.message ();
        ASSERT_EQ (close (), 0) << join::lastError.message ();
    }

    /**
     * @brief method called on receive.
     */
    virtual void onReceive () override
    {
        Tcp::Socket sock = accept ();
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
                sock.writeData (buf, nread);
            }
            sock.close ();
        }
    }

    /// host.
    static const std::string _host;

    /// port.
    static const uint16_t _port;

    /// timeout.
    static const int _timeout;
};

const std::string TcpIoSocket::_host = "localhost";
const uint16_t    TcpIoSocket::_port = 5000;
const int         TcpIoSocket::_timeout = 1000;

/**
 * @brief Test default constructor.
 */
TEST_F (TcpIoSocket, defaultConstruct)
{
    Tcp::SocketStream tcpStream;
    ASSERT_TRUE (tcpStream.good ()) << join::lastError.message ();
}

/**
 * @brief Test move constructor.
 */
TEST_F (TcpIoSocket, moveConstruct)
{
    Tcp::SocketStream tmp;
    ASSERT_TRUE (tmp.good ()) << join::lastError.message ();

    Tcp::SocketStream tcpStream (std::move (tmp));
    ASSERT_TRUE (tcpStream.good ()) << join::lastError.message ();
}

/**
 * @brief Test move operatore.
 */
TEST_F (TcpIoSocket, moveAssign)
{
    Tcp::SocketStream tmp;
    ASSERT_TRUE (tmp.good ()) << join::lastError.message ();

    Tcp::SocketStream tcpStream;
    tcpStream = std::move (tmp);
    ASSERT_TRUE (tcpStream.good ()) << join::lastError.message ();
}

/**
 * @brief Test open method.
 */
TEST_F (TcpIoSocket, connect)
{
    Tcp::SocketStream sockStream;
    ASSERT_FALSE (sockStream.socket ().connected ());
    sockStream.connect ({Tcp::Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (sockStream.good ()) << join::lastError.message ();
    ASSERT_TRUE (sockStream.socket ().connected ());
}

/**
 * @brief Test close method.
 */
TEST_F (TcpIoSocket, close)
{
    Tcp::SocketStream sockStream;
    ASSERT_FALSE (sockStream.socket ().connected ());
    sockStream.connect ({Tcp::Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (sockStream.good ()) << join::lastError.message ();
    ASSERT_TRUE (sockStream.socket ().connected ());
    sockStream.close ();
    ASSERT_TRUE (sockStream.good ()) << join::lastError.message ();
    ASSERT_FALSE (sockStream.socket ().connected ());
}

/**
 * @brief Test timeout method.
 */
TEST_F (TcpIoSocket, timeout)
{
    Tcp::SocketStream sockStream;
    ASSERT_NE (sockStream.timeout (), _timeout);
    sockStream.timeout (_timeout);
    ASSERT_EQ (sockStream.timeout (), _timeout);
}

/**
 * @brief Test socket method.
 */
TEST_F (TcpIoSocket, socket)
{
    Tcp::SocketStream sockStream;
    ASSERT_EQ (sockStream.socket ().handle (), -1);
    sockStream.connect ({Tcp::Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (sockStream.good ()) << join::lastError.message ();
    ASSERT_NE (sockStream.socket ().handle (), -1);
    sockStream.close ();
    ASSERT_TRUE (sockStream.good ()) << join::lastError.message ();
    ASSERT_EQ (sockStream.socket ().handle (), -1);
}

/**
 * @brief Test insert operator.
 */
TEST_F (TcpIoSocket, insert)
{
    Tcp::SocketStream sockStream;
    sockStream.connect ({Tcp::Resolver::resolveHost (_host), _port});

    sockStream << "This is a test" << std::endl;
    ASSERT_TRUE (sockStream.good ()) << join::lastError.message ();
}

/**
 * @brief Test put method.
 */
TEST_F (TcpIoSocket, put)
{
    Tcp::SocketStream sockStream;
    sockStream.connect ({Tcp::Resolver::resolveHost (_host), _port});

    sockStream.put ('t');
    sockStream.put ('e');
    sockStream.put ('s');
    sockStream.put ('t');
    ASSERT_TRUE (sockStream.good ()) << join::lastError.message ();
}

/**
 * @brief Test write method.
 */
TEST_F (TcpIoSocket, write)
{
    Tcp::SocketStream sockStream;
    sockStream.connect ({Tcp::Resolver::resolveHost (_host), _port});

    sockStream.write ("test", 4);
    ASSERT_TRUE (sockStream.good ()) << join::lastError.message ();
}

/**
 * @brief Test flush method.
 */
TEST_F (TcpIoSocket, flush)
{
    Tcp::SocketStream sockStream;
    sockStream.connect ({Tcp::Resolver::resolveHost (_host), _port});

    sockStream.put ('t');
    sockStream.flush ();
    sockStream.put ('e');
    sockStream.flush ();
    sockStream.put ('s');
    sockStream.flush ();
    sockStream.put ('t');
    sockStream.flush ();
    ASSERT_TRUE (sockStream.good ()) << join::lastError.message ();
}

/**
 * @brief Test extract method.
 */
TEST_F (TcpIoSocket, extract)
{
    Tcp::SocketStream sockStream;
    sockStream.connect ({Tcp::Resolver::resolveHost (_host), _port});
    sockStream << int (123456789) << std::endl;
    sockStream.flush ();

    int test;
    sockStream >> test;
    ASSERT_EQ (test, 123456789);
}

/**
 * @brief Test get method.
 */
TEST_F (TcpIoSocket, get)
{
    Tcp::SocketStream sockStream;
    sockStream.connect ({Tcp::Resolver::resolveHost (_host), _port});
    sockStream.write ("test", 4);
    sockStream.flush ();

    ASSERT_EQ (sockStream.get (), 't');
    ASSERT_EQ (sockStream.get (), 'e');
    ASSERT_EQ (sockStream.get (), 's');
    ASSERT_EQ (sockStream.get (), 't');
}

/**
 * @brief Test peek method.
 */
TEST_F (TcpIoSocket, peek)
{
    Tcp::SocketStream sockStream;
    sockStream.connect ({Tcp::Resolver::resolveHost (_host), _port});
    sockStream.write ("test", 4);
    sockStream.flush ();

    ASSERT_EQ (sockStream.peek (), 't');
    ASSERT_EQ (sockStream.get (), 't');
    ASSERT_EQ (sockStream.peek (), 'e');
    ASSERT_EQ (sockStream.get (), 'e');
    ASSERT_EQ (sockStream.peek (), 's');
    ASSERT_EQ (sockStream.get (), 's');
    ASSERT_EQ (sockStream.peek (), 't');
    ASSERT_EQ (sockStream.get (), 't');
}

/**
 * @brief Test unget method.
 */
TEST_F (TcpIoSocket, unget)
{
    Tcp::SocketStream sockStream;
    sockStream.connect ({Tcp::Resolver::resolveHost (_host), _port});
    sockStream.write ("test", 4);
    sockStream.flush ();

    ASSERT_EQ (sockStream.get (), 't');
    sockStream.unget ();
    ASSERT_EQ (sockStream.get (), 't');
    ASSERT_EQ (sockStream.get (), 'e');
    sockStream.unget ();
    ASSERT_EQ (sockStream.get (), 'e');
    ASSERT_EQ (sockStream.get (), 's');
    sockStream.unget ();
    ASSERT_EQ (sockStream.get (), 's');
    ASSERT_EQ (sockStream.get (), 't');
    sockStream.unget ();
    ASSERT_EQ (sockStream.get (), 't');
}

/**
 * @brief Test putback method.
 */
TEST_F (TcpIoSocket, putback)
{
    Tcp::SocketStream sockStream;
    sockStream.connect ({Tcp::Resolver::resolveHost (_host), _port});
    sockStream.write ("test", 4);
    sockStream.flush ();

    ASSERT_EQ (sockStream.get (), 't');
    sockStream.putback ('s');
    ASSERT_EQ (sockStream.get (), 's');
    ASSERT_EQ (sockStream.get (), 'e');
    sockStream.putback ('t');
    ASSERT_EQ (sockStream.get (), 't');
    ASSERT_EQ (sockStream.get (), 's');
    sockStream.putback ('e');
    ASSERT_EQ (sockStream.get (), 'e');
    ASSERT_EQ (sockStream.get (), 't');
}

/**
 * @brief Test getline method.
 */
TEST_F (TcpIoSocket, getline)
{
    Tcp::SocketStream sockStream;
    sockStream.connect ({Tcp::Resolver::resolveHost (_host), _port});
    sockStream.write ("test\n", 5);
    sockStream.flush ();

    std::array <char, 32> test = {};
    sockStream.getline (test.data (), test.size (), '\n');
    ASSERT_STREQ (test.data (), "test");
}

/**
 * @brief Test ignore method.
 */
TEST_F (TcpIoSocket, ignore)
{
    Tcp::SocketStream sockStream;
    sockStream.connect ({Tcp::Resolver::resolveHost (_host), _port});
    sockStream.write ("test\n", 5);
    sockStream.flush ();

    sockStream.ignore (std::numeric_limits <std::streamsize>::max (), 'e');
    ASSERT_EQ (sockStream.get (), 's');
    ASSERT_EQ (sockStream.get (), 't');
}

/**
 * @brief Test read method.
 */
TEST_F (TcpIoSocket, read)
{
    Tcp::SocketStream sockStream;
    sockStream.connect ({Tcp::Resolver::resolveHost (_host), _port});
    sockStream.write ("test", 4);
    sockStream.flush ();

    std::array <char, 32> test = {};
    sockStream.read (test.data (), 4);
    ASSERT_STREQ (test.data (), "test");
}

/**
 * @brief Test readsome method.
 */
/*TEST_F (TcpIoSocket, readsome)
{
    Tcp::SocketStream sockStream;
    sockStream.connect ({Tcp::Resolver::resolveHost (_host), _port});
    sockStream.write ("test", 4);
    sockStream.flush ();

    std::array <char, 32> test = {};
    ASSERT_EQ (sockStream.readsome (test.data (), test.size ()), 4);
    ASSERT_STREQ (test.data (), "test");
}*/

/**
 * @brief Test gcount method.
 */
TEST_F (TcpIoSocket, gcount)
{
    Tcp::SocketStream sockStream;
    sockStream.connect ({Tcp::Resolver::resolveHost (_host), _port});
    sockStream.write ("test", 4);
    sockStream.flush ();

    std::array <char, 32> test = {};
    sockStream.read (test.data (), 4);
    ASSERT_EQ (sockStream.gcount (), 4);
}

/**
 * @brief Test sync method.
 */
TEST_F (TcpIoSocket, sync)
{
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
