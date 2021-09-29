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

using join::Errc;
using join::IpAddress;
using join::Tcp;

/**
 * @brief Class used to test the TCP socket io stream API.
 */
class TcpSocketStream : public ::testing::Test, public Tcp::Acceptor::Observer
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
};

const int         TcpSocketStream::_timeout = 1000;
const std::string TcpSocketStream::_host = "localhost";
const uint16_t    TcpSocketStream::_port = 5000;

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
 * @brief Test open method.
 */
TEST_F (TcpSocketStream, connect)
{
    Tcp::Stream sockStream;
    ASSERT_FALSE (sockStream.socket ().connected ());
    sockStream.connect ({Tcp::Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (sockStream.good ()) << join::lastError.message ();
    ASSERT_TRUE (sockStream.socket ().connected ());
}

/**
 * @brief Test close method.
 */
TEST_F (TcpSocketStream, close)
{
    Tcp::Stream sockStream;
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
TEST_F (TcpSocketStream, timeout)
{
    Tcp::Stream sockStream;
    ASSERT_NE (sockStream.timeout (), _timeout);
    sockStream.timeout (_timeout);
    ASSERT_EQ (sockStream.timeout (), _timeout);
}

/**
 * @brief Test socket method.
 */
TEST_F (TcpSocketStream, socket)
{
    Tcp::Stream sockStream;
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
TEST_F (TcpSocketStream, insert)
{
    Tcp::Stream sockStream;
    sockStream << "test" << std::endl;
    ASSERT_TRUE (sockStream.fail ());
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    sockStream.clear ();
    sockStream.connect ({Tcp::Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (sockStream.good ()) << join::lastError.message ();
    sockStream << "test" << std::endl;
    ASSERT_TRUE (sockStream.socket ().waitReadyRead (_timeout));
    ASSERT_TRUE (sockStream.good ()) << join::lastError.message ();
}

/**
 * @brief Test put method.
 */
TEST_F (TcpSocketStream, put)
{
    Tcp::Stream sockStream;
    sockStream.put ('t');
    ASSERT_TRUE (sockStream.fail ());
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    sockStream.clear ();
    sockStream.connect ({Tcp::Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (sockStream.good ()) << join::lastError.message ();
    sockStream.put ('t');
    sockStream.put ('e');
    sockStream.put ('s');
    sockStream.put ('t');
    ASSERT_TRUE (sockStream.socket ().waitReadyRead (_timeout));
    ASSERT_TRUE (sockStream.good ()) << join::lastError.message ();
}

/**
 * @brief Test write method.
 */
TEST_F (TcpSocketStream, write)
{
    Tcp::Stream sockStream;
    sockStream.write ("test", 4);
    ASSERT_TRUE (sockStream.fail ());
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    sockStream.clear ();
    sockStream.connect ({Tcp::Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (sockStream.good ()) << join::lastError.message ();
    sockStream.write ("test", 4);
    ASSERT_TRUE (sockStream.socket ().waitReadyRead (_timeout));
    ASSERT_TRUE (sockStream.good ()) << join::lastError.message ();
}

/**
 * @brief Test flush method.
 */
TEST_F (TcpSocketStream, flush)
{
    Tcp::Stream sockStream;
    sockStream.connect ({Tcp::Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (sockStream.good ()) << join::lastError.message ();
    sockStream.put ('t');
    sockStream.flush ();
    sockStream.put ('e');
    sockStream.flush ();
    sockStream.put ('s');
    sockStream.flush ();
    sockStream.put ('t');
    sockStream.flush ();
    ASSERT_TRUE (sockStream.socket ().waitReadyRead (_timeout));
    ASSERT_TRUE (sockStream.good ()) << join::lastError.message ();
}

/**
 * @brief Test extract method.
 */
TEST_F (TcpSocketStream, extract)
{
    int test;
    Tcp::Stream sockStream;
    sockStream >> test;
    ASSERT_TRUE (sockStream.fail ());
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    sockStream.clear ();
    sockStream.connect ({Tcp::Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (sockStream.good ()) << join::lastError.message ();
    sockStream << int (123456789) << std::endl;
    sockStream.flush ();
    sockStream >> test;
    ASSERT_EQ (test, 123456789);
}

/**
 * @brief Test get method.
 */
TEST_F (TcpSocketStream, get)
{
    Tcp::Stream sockStream;
    sockStream.get ();
    ASSERT_TRUE (sockStream.fail ());
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    sockStream.clear ();
    sockStream.connect ({Tcp::Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (sockStream.good ()) << join::lastError.message ();
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
TEST_F (TcpSocketStream, peek)
{
    Tcp::Stream sockStream;
    sockStream.connect ({Tcp::Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (sockStream.good ()) << join::lastError.message ();
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
TEST_F (TcpSocketStream, unget)
{
    Tcp::Stream sockStream;
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
TEST_F (TcpSocketStream, putback)
{
    Tcp::Stream sockStream;
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
TEST_F (TcpSocketStream, getline)
{
    Tcp::Stream sockStream;
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
TEST_F (TcpSocketStream, ignore)
{
    Tcp::Stream sockStream;
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
TEST_F (TcpSocketStream, read)
{
    Tcp::Stream sockStream;
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
/*TEST_F (TcpSocketStream, readsome)
{
    Tcp::Stream sockStream;
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
TEST_F (TcpSocketStream, gcount)
{
    Tcp::Stream sockStream;
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
TEST_F (TcpSocketStream, sync)
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
