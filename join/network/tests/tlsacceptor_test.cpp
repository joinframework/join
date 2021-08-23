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
using join::net::IpAddress;
using join::net::Tcp;

/**
 * @brief Class used to test the TLS socket API.
 */
class TlsServerTest : public ::testing::Test
{
public:
    /**
     * @brief set up test case.
     */
    static void SetUpTestCase ()
    {
        std::ofstream certFile (cert);
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

        std::ofstream validKeyFile (validKey);
        if (validKeyFile.is_open ())
        {
            validKeyFile << "-----BEGIN PRIVATE KEY-----" << std::endl;
            validKeyFile << "MIIEwAIBADANBgkqhkiG9w0BAQEFAASCBKowggSmAgEAAoIBAQDEYr3c/DO15l64" << std::endl;
            validKeyFile << "KTHVxNmIS2pxUzK6TpZafKHoaSJ/UaelwVLvL+VsqxXC7Iy/Vl/6YAQjGWP9fPAs" << std::endl;
            validKeyFile << "H22VORV5Ox/dJfeqUYaR1SlICMA3D9xDCJfKY9Ibs7VQsnJfo0ReI4/o8g+7LOjL" << std::endl;
            validKeyFile << "Akzu5IXS58tkYBad8UGCYft9EuZeaYkDxDBwJ9In8fo75mIQcgaZBt3CRU7OQIc8" << std::endl;
            validKeyFile << "uG8lvxxf3pvW5jCEARgCnfnsfmIdH7xjk/MvNVG7UFpvQWGG3xhLcK3LAbXutaBm" << std::endl;
            validKeyFile << "XgtwBYgSNbDhtOn17+oLnGumv53yAC5H5gswa37PyuCuk8MXQcSqhVfkwnOEAG9I" << std::endl;
            validKeyFile << "OkmNToorAgMBAAECggEBAJxaBNE4la7ff0/dtEh+VbZWXKNm2r9LrBbUnU7czVNS" << std::endl;
            validKeyFile << "XkfwV6gImP7gYw3yqMf466b55LFRGLOLUee/Cc7BSOBg+yhlfv3BtILMk/Y+yVHN" << std::endl;
            validKeyFile << "Nc7Eu5ytxmy5scRSng6YfOj6JSwP9AhQY/KknT2hAQ597we2HKuYlVBJ1CCq+G/2" << std::endl;
            validKeyFile << "7W42JJpNVGFSr1hzIWCboKaQFOSQxDJ1l5TuRBjnFqAJbU86RPXe7o4O8oUe04I+" << std::endl;
            validKeyFile << "OVUp/H3cVc9AHrtIyCwZBp6SMMkA5Mi/95q7efk1eO23Sxei05UnMSwMmTJWmOZb" << std::endl;
            validKeyFile << "ox0OYLhPL4stO/nKDcxTO9F7cT2S4UOI4GKTvsj9FekCgYEA9NjHsum4Qw3zLIuD" << std::endl;
            validKeyFile << "EVGKj93lxhFlrrw7IShy9aK++liQwYDZlfFIEShAjtTlEimzYBkDEW/Z3JpNFYCc" << std::endl;
            validKeyFile << "yELfixkpu9betFV43PQL4mYY53QWijuHryAsoh9+LG6WhobFNIF6UxbUxsBsQG+3" << std::endl;
            validKeyFile << "h0z+s+EHlLNsjr+6K7RDSYHwR4cCgYEAzVTYPO93h0wVjlPRJ8JWriXGjEwugFg7" << std::endl;
            validKeyFile << "fIc3/eZPQCSsZaUCdh457g5jwSjVUbE6Tsg4ucpsByoUgWV6UDSSVkWAUYxtZdPp" << std::endl;
            validKeyFile << "93SkKXobP0Ow8KKa8LMjvl2xLA4te0TNDkpG+LLnVhy9l9D3lW7AxCfg/EiB+61s" << std::endl;
            validKeyFile << "vr2TPzIYST0CgYEA0OcUm4Xl/PAuBleSIHid7Fm67d9yDj/zgzrrusB8iUR/XVn+" << std::endl;
            validKeyFile << "S3xtrJXXXwYt2+1QajFs+Leh/iJ4mlv9aLAF4rw5a1bM9LfoQzzF1widPv1BLffU" << std::endl;
            validKeyFile << "sIqXCkwsbuR78ZOTNlUydSoJqPoMYn3u7LW4qujYevsLKFBVG2zV7CW9SIkCgYEA" << std::endl;
            validKeyFile << "yKG/zQLcU4qHgZPqyiO9mNp2gD4sLeKI9awWjPQOuglbntI2bn4ocrECzgn9Lp6y" << std::endl;
            validKeyFile << "76n6q6fcGMAIW0tx0Y8c8kllEcnaV2ZStiX2BkU2/IFDEk6AgPM52Ngpt3d+/3UB" << std::endl;
            validKeyFile << "vJXddUMyglUA+KgKU6xtlP22fCzJPcd+e343s8daLa0CgYEAvaaiBtBMXvbC6Xf1" << std::endl;
            validKeyFile << "BDorGaJAIzFkIfcs3itmt9WfW3Um2ROyEwtoTUsPKPdrQ2iAzDvr3TwETUwCo7z+" << std::endl;
            validKeyFile << "CPLuvz7HrJ1ug6Dm63JlZOWinmPzMoHz/cRn4UlA3gliZ8HEnBKwkZQ6eOV4Mm/U" << std::endl;
            validKeyFile << "9KoeJmE/4GgO4jrjoF3hZPBtn6I=" << std::endl;
            validKeyFile << "-----END PRIVATE KEY-----" << std::endl;
            validKeyFile.close ();
        }

        std::ofstream invalidKeyFile (invalidKey);
        if (invalidKeyFile.is_open ())
        {
            invalidKeyFile << "-----BEGIN PRIVATE KEY-----" << std::endl;
            invalidKeyFile << "MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQDDxlMyrT43szju" << std::endl;
            invalidKeyFile << "+rmpgEJoVEvMEgxrnJM3mfw0OLgCierBgaX1BQZ8gZ+n2M/slN5MZMFskKm4PRen" << std::endl;
            invalidKeyFile << "XJOno6Vk0SNQuoq9qypnA4bKUeo13mw+T8IBJ4Xw5oEy90vuTmGMZdFdiebcOFDo" << std::endl;
            invalidKeyFile << "R4iFCjCvc2sLFPqwCEccp34zCj/aTKsqir6zpcFiDriH/GwIDedn9yzb/DtiJGx1" << std::endl;
            invalidKeyFile << "yZc9/Hf3WTwQDyZtLStz2IaslUovA4Zk2dIJ8JX4RtVLRVnvRRViHMqNGI4tRcyW" << std::endl;
            invalidKeyFile << "B1Uoxo5QSKFDVo/MyPdyGYpk6jbzasDYuvB8d8iKdL6w2tf8RG4A9hZ6PNU4fAoB" << std::endl;
            invalidKeyFile << "G6Lx0WDRAgMBAAECggEBAI5lTlZp0/tHjH2mOCylcafYyFjiN8590EZ85ghFyPFv" << std::endl;
            invalidKeyFile << "xySAXGlfAlzFG49GD9YQYbBHCLkib4/mP7Inj+47BS3TQFCTnh5exTSwR/YYprYP" << std::endl;
            invalidKeyFile << "1cNs17tnN/EEd6zOWpg2wpUP6byTdStwesi4XwAFbWvwE0e4Nq6bFr4sxYnyRodM" << std::endl;
            invalidKeyFile << "PtfCJKHWmD3CRNGB6WNKhbeQAHk0tZLw0v8aDOhywYqvdvQt+zZ1lBRZxgTrnLtz" << std::endl;
            invalidKeyFile << "3rYeCGFDrTEkvRVVIgZAzRMmlSoP3S6xHza62e1ukyGhutAlfNIF5y0Z+PcOefQU" << std::endl;
            invalidKeyFile << "oANlOB0hSSkCV/J1lNVtkUPNn60eTLVIDRn12CG5ZVECgYEA5jkU8u0DWTcbmqAC" << std::endl;
            invalidKeyFile << "8ZnnMMzc24gR4cNDEb6gukv8fSYYvYRKOQrLCp5dkoSGitAeD8/W2tF0jvdJyqsg" << std::endl;
            invalidKeyFile << "4bB9tpq5kIuV1lxRud24E8gklVrFSWaO1v4mwkZ/7bxy4X5BwwmtwyCQ+y3WRlBR" << std::endl;
            invalidKeyFile << "K7x0eXIn1KcUfNnNd2wM/dQxWc8CgYEA2bHYzdaZgJG/p17BPpjAFrnrjSYAkaW4" << std::endl;
            invalidKeyFile << "W0C+5IlXggRPHV0bAjKq8DXAldudvmi12QBlS+f4TvxAhpYptvnJAaeuoF0gHr5s" << std::endl;
            invalidKeyFile << "cpK0hIHGz5rw0Ast8vuaJUHvYM3OAgeWp/eJy+IG8sVeFXLVCQN07AWrzYzLMJZs" << std::endl;
            invalidKeyFile << "ZT6eoIVaY18CgYB3aqBtKXeVLTqKR/l+7H+5FmpxDPGJk0kUyTaBq6x6Feq7UwVU" << std::endl;
            invalidKeyFile << "9T5AccjDmS5Yjn8pYHtn4UF+btK952oU5wEuWK9sRJsiJ5zUticnS6d+OSkgW6w3" << std::endl;
            invalidKeyFile << "5HiiSQNOg3Nrd5wQ2NJi5l+NikcdDcgs0YHxBL+YVFoSML+c9FUAzLIUjQKBgHKJ" << std::endl;
            invalidKeyFile << "KOl7vD2x8WUs7H1zvrT2UuFrmicHSwzv7r+tJxZoD/wCDYDFibxie5Yc+0zAKNUo" << std::endl;
            invalidKeyFile << "aSNbxABCpexpuz7jAwfU4oLLHGgwhjxtxX5mDWUTPxoml6FdEUgC/eAvx/C3b2Om" << std::endl;
            invalidKeyFile << "UKCtpjw67XpZdY6NfAnmHfNC3XEN47ANF0abguf/AoGBAIPXgCpAQjuIBpvK1Vlj" << std::endl;
            invalidKeyFile << "KdKKVD/3MROxnbR2GJjq0eqLWvdjoGIWXkQH+Pf1AjJzLgXTK2qHyT4ICiGXZEJs" << std::endl;
            invalidKeyFile << "WboYgRhGfX43ipeK3THMNsKdzgMX0KQNb0V/o95c6jZYySxSDINI+jgCTmf0b3/+" << std::endl;
            invalidKeyFile << "eQ2feElAKjfblTZKlEoeryL3" << std::endl;
            invalidKeyFile << "-----END PRIVATE KEY-----" << std::endl;
            invalidKeyFile.close ();
        }
    }

    /**
     * @brief tear down test case.
     */
    static void TearDownTestCase ()
    {
        unlink (cert.c_str ());
        unlink (validKey.c_str ());
        unlink (invalidKey.c_str ());
    }

protected:
    /// host.
    static const IpAddress address;

    /// port.
    static const uint16_t port;

    /// certificate.
    static const std::string cert;

    /// valid private key.
    static const std::string validKey;

    /// invalid private key.
    static const std::string invalidKey;
};

const IpAddress   TlsServerTest::address    = "127.0.0.1";
const uint16_t    TlsServerTest::port       = 5000;
const std::string TlsServerTest::cert       = "/tmp/tlsserver_test.cert";
const std::string TlsServerTest::validKey   = "/tmp/tlsserver_test_valid.key";
const std::string TlsServerTest::invalidKey = "/tmp/tlsserver_test_invalid.key";

/**
 * @brief Assign by move.
 */
TEST_F (TlsServerTest, move)
{
    Tcp::TlsAcceptor server1, server2;

    ASSERT_EQ (server1.open (), 0) << join::lastError.message ();

    server2 = std::move (server1);
    ASSERT_TRUE (server2.opened ());

    Tcp::TlsAcceptor server3 = std::move (server2);
    ASSERT_TRUE (server3.opened ());
}

/**
 * @brief Test open method.
 */
TEST_F (TlsServerTest, open)
{
    Tcp::TlsAcceptor server;

    ASSERT_EQ (server.open (), 0) << join::lastError.message ();
    ASSERT_EQ (server.open (), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
}

/**
 * @brief Test close method.
 */
TEST_F (TlsServerTest, close)
{
    Tcp::TlsAcceptor server;

    ASSERT_EQ (server.open (), 0) << join::lastError.message ();
    ASSERT_EQ (server.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test bind method.
 */
TEST_F (TlsServerTest, bind)
{
    Tcp::TlsAcceptor server;

    ASSERT_EQ (server.open (), 0) << join::lastError.message ();
    ASSERT_EQ (server.bind ({address, port}), 0) << join::lastError.message ();
    ASSERT_EQ (server.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test listen method.
 */
TEST_F (TlsServerTest, listen)
{
    Tcp::TlsAcceptor server;

    ASSERT_EQ (server.listen (20), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (server.bind ({address, port}), 0) << join::lastError.message ();
    ASSERT_EQ (server.listen (20), 0) << join::lastError.message ();
    ASSERT_EQ (server.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test accept method.
 */
TEST_F (TlsServerTest, accept)
{
    Tcp::TlsSocket clientSocket (Tcp::TlsSocket::Blocking);
    Tcp::TlsAcceptor server;

    ASSERT_FALSE (server.accept ().connected ());
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (server.bind ({address, port}), 0) << join::lastError.message ();
    ASSERT_EQ (server.listen (), 0) << join::lastError.message ();
    ASSERT_EQ (clientSocket.connect({address, port}), 0) << join::lastError.message ();
    Tcp::TlsSocket serverSocket = server.accept ();
    ASSERT_TRUE (serverSocket.connected ());
    ASSERT_EQ (serverSocket.localEndpoint ().ip (), address);
    ASSERT_EQ (serverSocket.localEndpoint ().port (), port);
    ASSERT_EQ (clientSocket.close (), 0) << join::lastError.message ();
    ASSERT_EQ (serverSocket.close (), 0) << join::lastError.message ();
    ASSERT_EQ (server.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test localEndpoint method.
 */
TEST_F (TlsServerTest, localEndpoint)
{
    Tcp::TlsAcceptor server;

    ASSERT_EQ (server.localEndpoint (), Tcp::Endpoint {});
    ASSERT_EQ (server.open (), 0) << join::lastError.message ();
    ASSERT_EQ (server.bind ({address, port}), 0) << join::lastError.message ();
    ASSERT_EQ (server.localEndpoint ().ip (), address);
    ASSERT_EQ (server.localEndpoint ().port (), port);
    ASSERT_EQ (server.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test opened method.
 */
TEST_F (TlsServerTest, opened)
{
    Tcp::TlsAcceptor server;

    ASSERT_FALSE (server.opened ());
    ASSERT_EQ (server.open (), 0) << join::lastError.message ();
    ASSERT_TRUE (server.opened ());
    ASSERT_EQ (server.close (), 0) << join::lastError.message ();
    ASSERT_FALSE (server.opened ());
}

/**
 * @brief Test family method.
 */
TEST_F (TlsServerTest, family)
{
    Tcp::TlsAcceptor server;

    ASSERT_EQ (server.bind ({address, port}), 0) << join::lastError.message ();
    ASSERT_EQ (server.family (), AF_INET);
    ASSERT_EQ (server.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test type method.
 */
TEST_F (TlsServerTest, type)
{
    Tcp::TlsAcceptor server;

    ASSERT_EQ (server.bind ({address, port}), 0) << join::lastError.message ();
    ASSERT_EQ (server.type (), SOCK_STREAM);
    ASSERT_EQ (server.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test protocol method.
 */
TEST_F (TlsServerTest, protocol)
{
    Tcp::TlsAcceptor server;

    ASSERT_EQ (server.bind ({address, port}), 0) << join::lastError.message ();
    ASSERT_EQ (server.protocol (), IPPROTO_TCP);
    ASSERT_EQ (server.close (), 0) << join::lastError.message ();
}

/**
 * @brief Test handle method.
 */
TEST_F (TlsServerTest, handle)
{
    Tcp::TlsAcceptor server;

    ASSERT_EQ (server.handle (), -1);
    ASSERT_EQ (server.open (), 0) << join::lastError.message ();
    ASSERT_GT (server.handle (), -1);
    ASSERT_EQ (server.close (), 0) << join::lastError.message ();
    ASSERT_EQ (server.handle (), -1);
}

/**
 * @brief Test setCertificate method.
 */
TEST_F (TlsServerTest, setCertificate)
{
    Tcp::TlsAcceptor server;

    ASSERT_EQ (server.setCertificate ("foo"), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (server.setCertificate (cert, "foo"), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (server.setCertificate (cert, invalidKey), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (server.setCertificate (cert, validKey), 0) << join::lastError.message ();
}

/**
 * @brief Test setCaCertificate method.
 */
TEST_F (TlsServerTest, setCaCertificate)
{
    Tcp::TlsAcceptor server;

    ASSERT_EQ (server.setCaCertificate ("foo"), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (server.setCaCertificate (cert), 0) << join::lastError.message ();
}

/**
 * @brief Test setCipher method.
 */
TEST_F (TlsServerTest, setCipher)
{
    Tcp::TlsAcceptor server;

    ASSERT_EQ (server.setCipher ("foo"), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (server.setCipher (join::crypto::defaultCipher_), 0) << join::lastError.message ();
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
