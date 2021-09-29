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
#include <fstream>

using join::Errc;
using join::IpAddress;
using join::Tls;

/**
 * @brief Class used to test the TLS socket API.
 */
class TlsSocket : public ::testing::Test, public Tls::Acceptor::Observer
{
public:
    /**
     * @brief set up test case.
     */
    static void SetUpTestCase ()
    {
        std::ofstream certFile (_cert);
        if (certFile.is_open ())
        {
            certFile << "-----BEGIN CERTIFICATE-----" << std::endl;
            certFile << "MIIEWzCCA0OgAwIBAgIUapEGKsCfGBcP5xrCN27wWmfz+V0wDQYJKoZIhvcNAQEL" << std::endl;
            certFile << "BQAwgbkxCzAJBgNVBAYTAkNBMRIwEAYDVQQIDAlPY2NpdGFuaWUxEDAOBgNVBAcM" << std::endl;
            certFile << "B0Nhc3RyZXMxFzAVBgNVBAoMDkpvaW4gRnJhbWV3b3JrMSMwIQYDVQQLDBpKb2lu" << std::endl;
            certFile << "IEZyYW1ld29yayBEZXZlbG9wbWVudDEcMBoGA1UEAwwTKi5qb2luZnJhbWV3b3Jr" << std::endl;
            certFile << "Lm5ldDEoMCYGCSqGSIb3DQEJARYZc3VwcG9ydEBqb2luZnJhbWV3b3JrLm5ldDAe" << std::endl;
            certFile << "Fw0yMTA4MDYxMjQ2MDFaFw0zMTA4MDQxMjQ2MDFaMIG5MQswCQYDVQQGEwJDQTES" << std::endl;
            certFile << "MBAGA1UECAwJT2NjaXRhbmllMRAwDgYDVQQHDAdDYXN0cmVzMRcwFQYDVQQKDA5K" << std::endl;
            certFile << "b2luIEZyYW1ld29yazEjMCEGA1UECwwaSm9pbiBGcmFtZXdvcmsgRGV2ZWxvcG1l" << std::endl;
            certFile << "bnQxHDAaBgNVBAMMEyouam9pbmZyYW1ld29yay5uZXQxKDAmBgkqhkiG9w0BCQEW" << std::endl;
            certFile << "GXN1cHBvcnRAam9pbmZyYW1ld29yay5uZXQwggEiMA0GCSqGSIb3DQEBAQUAA4IB" << std::endl;
            certFile << "DwAwggEKAoIBAQDEYr3c/DO15l64KTHVxNmIS2pxUzK6TpZafKHoaSJ/UaelwVLv" << std::endl;
            certFile << "L+VsqxXC7Iy/Vl/6YAQjGWP9fPAsH22VORV5Ox/dJfeqUYaR1SlICMA3D9xDCJfK" << std::endl;
            certFile << "Y9Ibs7VQsnJfo0ReI4/o8g+7LOjLAkzu5IXS58tkYBad8UGCYft9EuZeaYkDxDBw" << std::endl;
            certFile << "J9In8fo75mIQcgaZBt3CRU7OQIc8uG8lvxxf3pvW5jCEARgCnfnsfmIdH7xjk/Mv" << std::endl;
            certFile << "NVG7UFpvQWGG3xhLcK3LAbXutaBmXgtwBYgSNbDhtOn17+oLnGumv53yAC5H5gsw" << std::endl;
            certFile << "a37PyuCuk8MXQcSqhVfkwnOEAG9IOkmNToorAgMBAAGjWTBXMAkGA1UdEwQCMAAw" << std::endl;
            certFile << "CwYDVR0PBAQDAgXgMB0GA1UdJQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjAeBgNV" << std::endl;
            certFile << "HREEFzAVghMqLmpvaW5mcmFtZXdvcmsubmV0MA0GCSqGSIb3DQEBCwUAA4IBAQCT" << std::endl;
            certFile << "Q6vsUYRH3lns4kXtLt5ktZ6tE1Lo0W5zzdb2O27Joxj8qAF2kI/1Yhwv4B+lsI7l" << std::endl;
            certFile << "frVNj0hSwQLOyaslVIrVC65rhaqVwUZG7xklN4XjrUSWopSElPu/Eav4J4Sr9sQ7" << std::endl;
            certFile << "54zBUrubX8UVlPUxxD01R/p3sGAAU/Cak+CfYqKT8qJ+hKYOrMoLZji69WZto8dC" << std::endl;
            certFile << "D67+SDYArhdpqqlp+Us79RU9j7JC8DTHvtRTQjgZzYCP2XNaBd/1S2i2aOSzhkrI" << std::endl;
            certFile << "hSzDFAXnaJBJmlgDZFX+J9mt1BFIgADu0X1LA1WtotSjEnR88w8mw4QfMyWSmwbI" << std::endl;
            certFile << "aWkihk7D9M51Epy7r89d" << std::endl;
            certFile << "-----END CERTIFICATE-----" << std::endl;
            certFile.close ();
        }

        std::ofstream keyFile (_key);
        if (keyFile.is_open ())
        {
            keyFile << "-----BEGIN PRIVATE KEY-----" << std::endl;
            keyFile << "MIIEwAIBADANBgkqhkiG9w0BAQEFAASCBKowggSmAgEAAoIBAQDEYr3c/DO15l64" << std::endl;
            keyFile << "KTHVxNmIS2pxUzK6TpZafKHoaSJ/UaelwVLvL+VsqxXC7Iy/Vl/6YAQjGWP9fPAs" << std::endl;
            keyFile << "H22VORV5Ox/dJfeqUYaR1SlICMA3D9xDCJfKY9Ibs7VQsnJfo0ReI4/o8g+7LOjL" << std::endl;
            keyFile << "Akzu5IXS58tkYBad8UGCYft9EuZeaYkDxDBwJ9In8fo75mIQcgaZBt3CRU7OQIc8" << std::endl;
            keyFile << "uG8lvxxf3pvW5jCEARgCnfnsfmIdH7xjk/MvNVG7UFpvQWGG3xhLcK3LAbXutaBm" << std::endl;
            keyFile << "XgtwBYgSNbDhtOn17+oLnGumv53yAC5H5gswa37PyuCuk8MXQcSqhVfkwnOEAG9I" << std::endl;
            keyFile << "OkmNToorAgMBAAECggEBAJxaBNE4la7ff0/dtEh+VbZWXKNm2r9LrBbUnU7czVNS" << std::endl;
            keyFile << "XkfwV6gImP7gYw3yqMf466b55LFRGLOLUee/Cc7BSOBg+yhlfv3BtILMk/Y+yVHN" << std::endl;
            keyFile << "Nc7Eu5ytxmy5scRSng6YfOj6JSwP9AhQY/KknT2hAQ597we2HKuYlVBJ1CCq+G/2" << std::endl;
            keyFile << "7W42JJpNVGFSr1hzIWCboKaQFOSQxDJ1l5TuRBjnFqAJbU86RPXe7o4O8oUe04I+" << std::endl;
            keyFile << "OVUp/H3cVc9AHrtIyCwZBp6SMMkA5Mi/95q7efk1eO23Sxei05UnMSwMmTJWmOZb" << std::endl;
            keyFile << "ox0OYLhPL4stO/nKDcxTO9F7cT2S4UOI4GKTvsj9FekCgYEA9NjHsum4Qw3zLIuD" << std::endl;
            keyFile << "EVGKj93lxhFlrrw7IShy9aK++liQwYDZlfFIEShAjtTlEimzYBkDEW/Z3JpNFYCc" << std::endl;
            keyFile << "yELfixkpu9betFV43PQL4mYY53QWijuHryAsoh9+LG6WhobFNIF6UxbUxsBsQG+3" << std::endl;
            keyFile << "h0z+s+EHlLNsjr+6K7RDSYHwR4cCgYEAzVTYPO93h0wVjlPRJ8JWriXGjEwugFg7" << std::endl;
            keyFile << "fIc3/eZPQCSsZaUCdh457g5jwSjVUbE6Tsg4ucpsByoUgWV6UDSSVkWAUYxtZdPp" << std::endl;
            keyFile << "93SkKXobP0Ow8KKa8LMjvl2xLA4te0TNDkpG+LLnVhy9l9D3lW7AxCfg/EiB+61s" << std::endl;
            keyFile << "vr2TPzIYST0CgYEA0OcUm4Xl/PAuBleSIHid7Fm67d9yDj/zgzrrusB8iUR/XVn+" << std::endl;
            keyFile << "S3xtrJXXXwYt2+1QajFs+Leh/iJ4mlv9aLAF4rw5a1bM9LfoQzzF1widPv1BLffU" << std::endl;
            keyFile << "sIqXCkwsbuR78ZOTNlUydSoJqPoMYn3u7LW4qujYevsLKFBVG2zV7CW9SIkCgYEA" << std::endl;
            keyFile << "yKG/zQLcU4qHgZPqyiO9mNp2gD4sLeKI9awWjPQOuglbntI2bn4ocrECzgn9Lp6y" << std::endl;
            keyFile << "76n6q6fcGMAIW0tx0Y8c8kllEcnaV2ZStiX2BkU2/IFDEk6AgPM52Ngpt3d+/3UB" << std::endl;
            keyFile << "vJXddUMyglUA+KgKU6xtlP22fCzJPcd+e343s8daLa0CgYEAvaaiBtBMXvbC6Xf1" << std::endl;
            keyFile << "BDorGaJAIzFkIfcs3itmt9WfW3Um2ROyEwtoTUsPKPdrQ2iAzDvr3TwETUwCo7z+" << std::endl;
            keyFile << "CPLuvz7HrJ1ug6Dm63JlZOWinmPzMoHz/cRn4UlA3gliZ8HEnBKwkZQ6eOV4Mm/U" << std::endl;
            keyFile << "9KoeJmE/4GgO4jrjoF3hZPBtn6I=" << std::endl;
            keyFile << "-----END PRIVATE KEY-----" << std::endl;
            keyFile.close ();
        }
    }

    /**
     * @brief tear down test case.
     */
    static void TearDownTestCase ()
    {
        unlink (_cert.c_str ());
        unlink (_key.c_str ());
    }

protected:
    /**
     * @brief Sets up the test fixture.
     */
    void SetUp ()
    {
        ASSERT_EQ (setCertificate (_cert, _key), 0) << join::lastError.message ();
        ASSERT_EQ (setCipher (join::crypto::defaultCipher_), 0) << join::lastError.message ();
        ASSERT_EQ (bind ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
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
        Tls::Socket sock = accept ();
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

    /// certificate.
    static const std::string _cert;

    /// private key.
    static const std::string _key;
};

const std::string TlsSocket::_host = "localhost";
const uint16_t    TlsSocket::_port = 5000;
const int         TlsSocket::_timeout = 1000;
const std::string TlsSocket::_cert = "/tmp/tlssocket_test.cert";
const std::string TlsSocket::_key = "/tmp/tlssocket_test.key";

/**
 * @brief Test open method.
 */
TEST_F (TlsSocket, open)
{
    Tls::Socket tlsSocket;

    ASSERT_EQ (tlsSocket.open (Tls::v4 ()), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.close (), 0) << join::lastError.message ();

    ASSERT_EQ (tlsSocket.open (Tls::v6 ()), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test close method.
 */
TEST_F (TlsSocket, close)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);

    ASSERT_FALSE (tlsSocket.opened ());
    ASSERT_EQ (tlsSocket.connect ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (tlsSocket.opened());
    ASSERT_EQ (tlsSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_FALSE (tlsSocket.opened());
    ASSERT_EQ (tlsSocket.close (), 0) << join::lastError.message ();
    ASSERT_FALSE (tlsSocket.opened());
}

/**
 * @brief Test bind method.
 */
TEST_F (TlsSocket, bind)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);

    ASSERT_FALSE (tlsSocket.opened ());
    ASSERT_EQ (tlsSocket.bind ({Tls::Resolver::resolveHost (_host)}), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.connect ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test connect method.
 */
TEST_F (TlsSocket, connect)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);

    ASSERT_EQ (tlsSocket.connect ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.close (), 0) << join::lastError.message ();

    ASSERT_EQ (tlsSocket.connect (Tls::Resolver::resolve (_host + ":" + std::to_string (_port))), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test waitConnected method.
 */
TEST_F (TlsSocket, waitConnected)
{
    Tls::Socket tlsSocket;

    if (tlsSocket.connect ({Tls::Resolver::resolveHost (_host), _port}) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
        ASSERT_TRUE (tlsSocket.connecting ());
    }
    ASSERT_TRUE (tlsSocket.waitConnected (_timeout)) << join::lastError.message ();
    if (tlsSocket.disconnect () == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (tlsSocket.waitDisconnected (_timeout)) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test connectEncrypted method.
 */
TEST_F (TlsSocket, connectEncrypted)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);

    ASSERT_EQ (tlsSocket.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test startEncryption method.
 */
TEST_F (TlsSocket, startEncryption)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);

    ASSERT_EQ (tlsSocket.connect ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.startEncryption (), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test waitEncrypted method.
 */
TEST_F (TlsSocket, waitEncrypted)
{
    Tls::Socket tlsSocket (Tls::Socket::NonBlocking);

    if (tlsSocket.connect ({Tls::Resolver::resolveHost (_host), _port}) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (tlsSocket.waitConnected (_timeout)) << join::lastError.message ();
    if (tlsSocket.startEncryption () == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (tlsSocket.waitEncrypted (_timeout)) << join::lastError.message ();
    if (tlsSocket.disconnect () == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (tlsSocket.waitDisconnected (_timeout)) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.close (), 0) << join::lastError.message ();

    if (tlsSocket.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port}) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (tlsSocket.waitEncrypted (_timeout)) << join::lastError.message ();
    if (tlsSocket.disconnect () == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (tlsSocket.waitDisconnected (_timeout)) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test disconnect method.
 */
TEST_F (TlsSocket, disconnect)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);

    ASSERT_FALSE (tlsSocket.connected ());
    ASSERT_EQ (tlsSocket.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (tlsSocket.connected ());
    ASSERT_EQ (tlsSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_FALSE (tlsSocket.connected ());
    ASSERT_EQ (tlsSocket.close (), 0) << join::lastError.message ();
    ASSERT_FALSE (tlsSocket.connected ());
}

/**
 * @brief Test waitDisconnected method.
 */
TEST_F (TlsSocket, waitDisconnected)
{
    Tls::Socket tlsSocket (Tls::Socket::NonBlocking);

    if (tlsSocket.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port}) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (tlsSocket.waitEncrypted (_timeout)) << join::lastError.message ();
    if (tlsSocket.disconnect () == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (tlsSocket.waitDisconnected (_timeout)) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test waitReadyRead method.
 */
TEST_F (TlsSocket, waitReadyRead)
{
    Tls::Socket tlsSocket;
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    if (tlsSocket.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port}) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (tlsSocket.waitEncrypted (_timeout)) << join::lastError.message ();
    ASSERT_TRUE (tlsSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.writeData (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (tlsSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    if (tlsSocket.disconnect () == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (tlsSocket.waitDisconnected (_timeout)) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test canRead method.
 */
TEST_F (TlsSocket, canRead)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (tlsSocket.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (tlsSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.writeData (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (tlsSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.read (data, 1), 1) << join::lastError.message ();
    ASSERT_GT (tlsSocket.canRead (), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test read method.
 */
TEST_F (TlsSocket, read)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (tlsSocket.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (tlsSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.writeData (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (tlsSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_GT (tlsSocket.read (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test readChar method.
 */
TEST_F (TlsSocket, readChar)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);
    char data;

    ASSERT_EQ (tlsSocket.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (tlsSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.writeData ("b", 1), 0) << join::lastError.message ();
    ASSERT_TRUE (tlsSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.readChar (data), 0) << join::lastError.message ();
    ASSERT_EQ (data, 'b');
    ASSERT_EQ (tlsSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test readLine method.
 */
TEST_F (TlsSocket, readLine)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);
    std::string data;

    ASSERT_EQ (tlsSocket.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (tlsSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.writeData ("readLine\n", strlen ("readLine\n")), 0) << join::lastError.message ();
    ASSERT_TRUE (tlsSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.readLine (data, 1024), 0) << join::lastError.message ();
    ASSERT_EQ (data, "readLine");
    ASSERT_EQ (tlsSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test readData method.
 */
TEST_F (TlsSocket, readData)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (tlsSocket.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (tlsSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.writeData (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (tlsSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.readData (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test waitReadyWrite method.
 */
TEST_F (TlsSocket, waitReadyWrite)
{
    Tls::Socket tlsSocket;

    if (tlsSocket.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port}) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (tlsSocket.waitEncrypted (_timeout)) << join::lastError.message ();
    ASSERT_TRUE (tlsSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    if (tlsSocket.disconnect () == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (tlsSocket.waitDisconnected (_timeout)) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test write method.
 */
TEST_F (TlsSocket, write)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (tlsSocket.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (tlsSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_GT (tlsSocket.write (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (tlsSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test writeData method.
 */
TEST_F (TlsSocket, writeData)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (tlsSocket.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (tlsSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.writeData (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (tlsSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test setMode method.
 */
TEST_F (TlsSocket, setMode)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);

    ASSERT_EQ (tlsSocket.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setMode (Tls::Socket::NonBlocking), 0) << join::lastError.message ();
    if (tlsSocket.disconnect () == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (tlsSocket.waitDisconnected (_timeout)) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test setOption method.
 */
TEST_F (TlsSocket, setOption)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);

    ASSERT_EQ (tlsSocket.setOption (Tls::Socket::NoDelay, 1), -1);
    ASSERT_EQ (tlsSocket.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setOption (Tls::Socket::NoDelay, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test localEndpoint method.
 */
TEST_F (TlsSocket, localEndpoint)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);

    ASSERT_EQ (tlsSocket.bind ({Tls::Resolver::resolveHost (_host), uint16_t (_port + 1)}), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.connect ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.localEndpoint (), Tls::Endpoint (Tls::Resolver::resolveHost (_host), uint16_t (_port + 1))) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test remoteEndpoint method.
 */
TEST_F (TlsSocket, remoteEndpoint)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);

    ASSERT_EQ (tlsSocket.bind ({Tls::Resolver::resolveHost (_host), uint16_t (_port + 1)}), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.connect ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.remoteEndpoint (), Tls::Endpoint (Tls::Resolver::resolveHost (_host), _port)) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test opened method.
 */
TEST_F (TlsSocket, opened)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);

    ASSERT_FALSE (tlsSocket.opened ());
    ASSERT_EQ (tlsSocket.open (Tls::Resolver::resolveHost (_host).family ()), 0) << join::lastError.message ();
    ASSERT_TRUE (tlsSocket.opened ());
    ASSERT_EQ (tlsSocket.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (tlsSocket.opened ());
    ASSERT_EQ (tlsSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_FALSE (tlsSocket.opened ());
    ASSERT_EQ (tlsSocket.close (), 0) << join::lastError.message ();
    ASSERT_FALSE (tlsSocket.opened ());
}

/**
 * @brief Test connected method.
 */
TEST_F (TlsSocket, connected)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);

    ASSERT_FALSE (tlsSocket.connected ());
    ASSERT_EQ (tlsSocket.open (Tls::Resolver::resolveHost (_host).family ()), 0) << join::lastError.message ();
    ASSERT_FALSE (tlsSocket.connected ());
    ASSERT_EQ (tlsSocket.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (tlsSocket.connected ());
    ASSERT_EQ (tlsSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_FALSE (tlsSocket.connected ());
    ASSERT_EQ (tlsSocket.close (), 0) << join::lastError.message ();
    ASSERT_FALSE (tlsSocket.connected ());
}

/**
 * @brief Test encrypted method.
 */
TEST_F (TlsSocket, encrypted)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);

    ASSERT_FALSE (tlsSocket.encrypted ());
    ASSERT_EQ (tlsSocket.open (Tls::Resolver::resolveHost (_host).family ()), 0) << join::lastError.message ();
    ASSERT_FALSE (tlsSocket.encrypted ());
    ASSERT_EQ (tlsSocket.connect ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_FALSE (tlsSocket.encrypted ());
    ASSERT_EQ (tlsSocket.startEncryption (), 0) << join::lastError.message ();
    ASSERT_TRUE (tlsSocket.encrypted ());
    ASSERT_EQ (tlsSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_FALSE (tlsSocket.encrypted ());
    ASSERT_EQ (tlsSocket.close (), 0) << join::lastError.message ();
    ASSERT_FALSE (tlsSocket.encrypted ());
}

/**
 * @brief Test family method.
 */
TEST_F (TlsSocket, family)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);

    ASSERT_EQ (tlsSocket.family (), AF_INET);

    ASSERT_EQ (tlsSocket.bind (IpAddress (AF_INET6)), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.family (), AF_INET6);
    ASSERT_EQ (tlsSocket.close (), 0) << join::lastError.message ();

    ASSERT_EQ (tlsSocket.bind (IpAddress (AF_INET)), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.family (), AF_INET);
    ASSERT_EQ (tlsSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test type method.
 */
TEST_F (TlsSocket, type)
{
    Tls::Socket tlsSocket;

    ASSERT_EQ (tlsSocket.type (), SOCK_STREAM);
}

/**
 * @brief Test protocol method.
 */
TEST_F (TlsSocket, protocol)
{
    Tls::Socket tlsSocket;

    ASSERT_EQ (tlsSocket.protocol (), IPPROTO_TCP);
}

/**
 * @brief Test handle method.
 */
TEST_F (TlsSocket, handle)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);

    ASSERT_EQ (tlsSocket.handle (), -1);
    ASSERT_EQ (tlsSocket.open (Tls::Resolver::resolveHost (_host).family ()), 0) << join::lastError.message ();
    ASSERT_GT (tlsSocket.handle (), -1);
    ASSERT_EQ (tlsSocket.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_GT (tlsSocket.handle (), -1);
    ASSERT_EQ (tlsSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.handle (), -1);
    ASSERT_EQ (tlsSocket.close (), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.handle (), -1);
}

/**
 * @brief Test mtu method.
 */
TEST_F (TlsSocket, mtu)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);

    ASSERT_EQ (tlsSocket.mtu (), -1) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_NE (tlsSocket.mtu (), -1) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.mtu (), -1);
    ASSERT_EQ (tlsSocket.close (), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.mtu (), -1);
}

/**
 * @brief Test setCertificate method.
 */
TEST_F (TlsSocket, setCertificate)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);

    ASSERT_EQ (tlsSocket.setCertificate (_cert, _key), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setCertificate (_cert, _key), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setCertificate (_cert, _key), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test setCaCertificate method.
 */
TEST_F (TlsSocket, setCaCertificate)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);

    ASSERT_EQ (tlsSocket.setCaCertificate (_cert), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setCaCertificate (_cert), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setCaCertificate (_cert), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test setCipher method.
 */
TEST_F (TlsSocket, setCipher)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);

    ASSERT_EQ (tlsSocket.setCipher (join::crypto::defaultCipher_), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setCipher (join::crypto::defaultCipher_), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setCipher (join::crypto::defaultCipher_), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test is lower method.
 */
TEST_F (TlsSocket, isLower)
{
    Tls::Socket tlsSocket1, tlsSocket2;

    ASSERT_EQ (tlsSocket1.open (Tls::Resolver::resolveHost (_host).family ()), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket2.open (Tls::Resolver::resolveHost (_host).family ()), 0) << join::lastError.message ();
    if (tlsSocket1.handle () < tlsSocket2.handle ())
    {
        ASSERT_TRUE (tlsSocket1 < tlsSocket2);
    }
    else
    {
        ASSERT_TRUE (tlsSocket2 < tlsSocket1);
    }
    ASSERT_EQ (tlsSocket1.close (), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket2.close (), 0) << join::lastError.message ();
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    join::crypto::initializeOpenSSL ();
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}

