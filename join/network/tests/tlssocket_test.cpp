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
        std::ofstream rootCertFile (_rootcert);
        if (rootCertFile.is_open ())
        {
            rootCertFile << "-----BEGIN CERTIFICATE-----" << std::endl;
            rootCertFile << "MIIChjCCAisCFBuHxbqMUGyl7OQUQcoRg3pOBJF+MAoGCCqGSM49BAMCMIHEMQsw" << std::endl;
            rootCertFile << "CQYDVQQGEwJGUjESMBAGA1UECAwJT2NjaXRhbmllMRAwDgYDVQQHDAdDYXN0cmVz" << std::endl;
            rootCertFile << "MRcwFQYDVQQKDA5Kb2luIEZyYW1ld29yazEtMCsGA1UECwwkSm9pbiBGcmFtZXdv" << std::endl;
            rootCertFile << "cmsgQ2VydGlmaWNhdGUgQXV0aG9yaXR5MR0wGwYDVQQDDBRjYS5qb2luZnJhbWV3" << std::endl;
            rootCertFile << "b3JrLm5ldDEoMCYGCSqGSIb3DQEJARYZc3VwcG9ydEBqb2luZnJhbWV3b3JrLm5l" << std::endl;
            rootCertFile << "dDAeFw0yMjA3MDUxNjMxMTZaFw0zMjA3MDIxNjMxMTZaMIHEMQswCQYDVQQGEwJG" << std::endl;
            rootCertFile << "UjESMBAGA1UECAwJT2NjaXRhbmllMRAwDgYDVQQHDAdDYXN0cmVzMRcwFQYDVQQK" << std::endl;
            rootCertFile << "DA5Kb2luIEZyYW1ld29yazEtMCsGA1UECwwkSm9pbiBGcmFtZXdvcmsgQ2VydGlm" << std::endl;
            rootCertFile << "aWNhdGUgQXV0aG9yaXR5MR0wGwYDVQQDDBRjYS5qb2luZnJhbWV3b3JrLm5ldDEo" << std::endl;
            rootCertFile << "MCYGCSqGSIb3DQEJARYZc3VwcG9ydEBqb2luZnJhbWV3b3JrLm5ldDBZMBMGByqG" << std::endl;
            rootCertFile << "SM49AgEGCCqGSM49AwEHA0IABASk0zCrKtXQi0Ycx+Anx+VWv8gncbPmNQ1yutii" << std::endl;
            rootCertFile << "gQjP2mF9NIqlxpcKNuE/6DDnfSzCEDhFyvGiK0NJ1C3RBowwCgYIKoZIzj0EAwID" << std::endl;
            rootCertFile << "SQAwRgIhAIFqdbxTb5kRjy4UY0N205ZEhHSMK89p2oUyn4iNbXH2AiEAtmV1UyRX" << std::endl;
            rootCertFile << "DIAGr/F+1SwQMPoJzSQxZ7NdxjNgW286e9Q=" << std::endl;
            rootCertFile << "-----END CERTIFICATE-----" << std::endl;
            rootCertFile.close ();
        }

        std::ofstream certFile (_cert);
        if (certFile.is_open ())
        {
            certFile << "-----BEGIN CERTIFICATE-----" << std::endl;
            certFile << "MIIDfjCCAySgAwIBAgIUR3ZIuKMt0BdaOZQnPwhSMR9qzfcwCgYIKoZIzj0EAwIw" << std::endl;
            certFile << "gcQxCzAJBgNVBAYTAkZSMRIwEAYDVQQIDAlPY2NpdGFuaWUxEDAOBgNVBAcMB0Nh" << std::endl;
            certFile << "c3RyZXMxFzAVBgNVBAoMDkpvaW4gRnJhbWV3b3JrMS0wKwYDVQQLDCRKb2luIEZy" << std::endl;
            certFile << "YW1ld29yayBDZXJ0aWZpY2F0ZSBBdXRob3JpdHkxHTAbBgNVBAMMFGNhLmpvaW5m" << std::endl;
            certFile << "cmFtZXdvcmsubmV0MSgwJgYJKoZIhvcNAQkBFhlzdXBwb3J0QGpvaW5mcmFtZXdv" << std::endl;
            certFile << "cmsubmV0MB4XDTIyMDcwNzExMTcxOVoXDTMyMDcwNDExMTcxOVowgacxCzAJBgNV" << std::endl;
            certFile << "BAYTAkZSMRIwEAYDVQQIDAlPY2NpdGFuaWUxEDAOBgNVBAcMB0Nhc3RyZXMxFzAV" << std::endl;
            certFile << "BgNVBAoMDkpvaW4gRnJhbWV3b3JrMRswGQYDVQQLDBJKb2luIEZyYW1ld29yayBE" << std::endl;
            certFile << "ZXYxEjAQBgNVBAMMCWxvY2FsaG9zdDEoMCYGCSqGSIb3DQEJARYZc3VwcG9ydEBq" << std::endl;
            certFile << "b2luZnJhbWV3b3JrLm5ldDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB" << std::endl;
            certFile << "ALUPbMjTtlIlUnQ5uRlkp40SB/n+bE8inQmcBhz8t+LPzIcDPhtuERlaDMoZyTbY" << std::endl;
            certFile << "25O9X2H1Z3MWxYpVYVcdZ7c4PFzRQyp+2NrabcK/OBbcalt0ehCsTC6qPujCP5a9" << std::endl;
            certFile << "Ey89KkSVPHKggsY74Choc2+dd1cg4O8kR5Zpgan8P9OfvUQMlnMYWh9S80QQgYk6" << std::endl;
            certFile << "qxBk1iQ/UXz147u/ykJg8/+T35CIZEzQozS3Ax/mRNcpZmyQZsAZZHwr6sd5Bcy5" << std::endl;
            certFile << "JTTUD0qnEqhwhsDSS6FWsx8AbcI4k33O3tyBxOMhJjVh7T9wlUSpvIBxMldGZcLx" << std::endl;
            certFile << "8LIlllbHKCVpNFxP8BRh678CAwEAAaNEMEIwCwYDVR0PBAQDAgXgMB0GA1UdJQQW" << std::endl;
            certFile << "MBQGCCsGAQUFBwMBBggrBgEFBQcDAjAUBgNVHREEDTALgglsb2NhbGhvc3QwCgYI" << std::endl;
            certFile << "KoZIzj0EAwIDSAAwRQIhAONSuYMLQJl7M2Y0nSx6JPn5/I4Kz2k/6qH7T9Ln7oSw" << std::endl;
            certFile << "AiAUhSoORaOfISLsOJm6mcoKHyodjrh8lTncqtNau8UQiQ==" << std::endl;
            certFile << "-----END CERTIFICATE-----" << std::endl;
            certFile.close ();
        }

        std::ofstream keyFile (_key);
        if (keyFile.is_open ())
        {
            keyFile << "-----BEGIN RSA PRIVATE KEY-----" << std::endl;
            keyFile << "MIIEogIBAAKCAQEAtQ9syNO2UiVSdDm5GWSnjRIH+f5sTyKdCZwGHPy34s/MhwM+" << std::endl;
            keyFile << "G24RGVoMyhnJNtjbk71fYfVncxbFilVhVx1ntzg8XNFDKn7Y2tptwr84FtxqW3R6" << std::endl;
            keyFile << "EKxMLqo+6MI/lr0TLz0qRJU8cqCCxjvgKGhzb513VyDg7yRHlmmBqfw/05+9RAyW" << std::endl;
            keyFile << "cxhaH1LzRBCBiTqrEGTWJD9RfPXju7/KQmDz/5PfkIhkTNCjNLcDH+ZE1ylmbJBm" << std::endl;
            keyFile << "wBlkfCvqx3kFzLklNNQPSqcSqHCGwNJLoVazHwBtwjiTfc7e3IHE4yEmNWHtP3CV" << std::endl;
            keyFile << "RKm8gHEyV0ZlwvHwsiWWVscoJWk0XE/wFGHrvwIDAQABAoIBADsT9WLx8aOMGgNi" << std::endl;
            keyFile << "ckse1KzrHT8tDRolKShhiH0c2MNhGlNTc4hvuZSUFWg8UbNbvwnN2LmV4VNkMsCY" << std::endl;
            keyFile << "a1KJfbYC229NuKS9DvUaIIZS7nJHKUz+DHY0QgdX9X7tIuKEfyYCxy/pofek+AbG" << std::endl;
            keyFile << "srV8n23j63S6faqKI/GoK27Tp9/cytlkFMdwJw184j4/fIdObjbfcpPl4RFLMyYi" << std::endl;
            keyFile << "Nn2xmxdtdCe4ii1eQznkeveR2W3MG1qwVmYE9EtgqDJ4Shb7HqpOGn0U/rLE/4em" << std::endl;
            keyFile << "Mb9LB0n/5cluDAVSfbOcNq7qDL97G3t4EItXClfFs54TS6jC1N6suyLcA/s169mM" << std::endl;
            keyFile << "LvezZIECgYEA2n1ZxjUxoupfFTmqHzzRdzXN6IxcPuiE0TQf0RJg67vSViTwvxqi" << std::endl;
            keyFile << "0fq47DH64Ddc7FYrXwtU7EShzMOvRvGvT1y0WkOcXZkHln430Q8D+eRL40OCO0sk" << std::endl;
            keyFile << "XKFXJ0Qt0ZRheRRwnRwWXK+O0/YByE37Xo2OLh+ODFeQVfV+MxQ4YlkCgYEA1CUM" << std::endl;
            keyFile << "GGmDtHUrOiLloefo/ihihe1oCot712wS8Ns9gkQ6dGJXjP2YNqXV/5uwYp4CMQcG" << std::endl;
            keyFile << "32fSbLhj3HgM09aqkeBmI2pEsz5LUTjcpBI31BTJ/yFJxNwCfMt5BqVbxkrna/LV" << std::endl;
            keyFile << "r4Wr63duooZYUs0wEBDX1yAOaG5vVrLEOmqJi9cCgYBJN7x/6Y+KstPOPxa9hRoE" << std::endl;
            keyFile << "wukPxwmaozzvBnKrE2ebV8OyzopG4Fwz1btmm5Eg1iyznmTGYvvAeXdnMSRpt9Ax" << std::endl;
            keyFile << "3jUqEoIx83hRfFakHk4Mze6IaDUaPaIf0IluRTC9jOXlN25Nl/3+bW6FpWkPgOJ9" << std::endl;
            keyFile << "2x4MYtKtXCKoS65Q27PRcQKBgCeGJ3S8487RLZaNPd8mR+BiQcjV7FZaml4OQpEh" << std::endl;
            keyFile << "J5qHf6SDVfBsptk3mGEg7oHREnNz7bHds/SRfflrXGhzTCAVgLIEcHw8lOezAliH" << std::endl;
            keyFile << "FGOs491mpEnK+6OofQnh8PFGr0sLWQh0jQ03mSMqiHYUyGmV2cC4tjZ6rL+oWqrP" << std::endl;
            keyFile << "goTNAoGAeKnJv2PlUpBMIR9avUAviQg3dfM7mGuJO9wNVgAJOF07CDSBg1SHmYYj" << std::endl;
            keyFile << "/K84Uw/MQPwV17uJDU7HAkjf4fRSMRuUsxpwQp7arQpJVOz4+zYXr+6KZe2QcNsq" << std::endl;
            keyFile << "eoL8FLs4H7DorbXuwOVruWXB61IYCuc/kK4rlPWFSXStEgbWvhY=" << std::endl;
            keyFile << "-----END RSA PRIVATE KEY-----" << std::endl;
            keyFile.close ();
        }

        std::ofstream invalidKeyFile (_invalidKey);
        if (invalidKeyFile.is_open ())
        {
            invalidKeyFile << "-----BEGIN RSA PRIVATE KEY-----" << std::endl;
            invalidKeyFile << "MIIEowIBAAKCAQEA2Q0DOyG039uVMuxNnZ5fpfOcvXXOTguST1QR6eLVkdG7OKpM" << std::endl;
            invalidKeyFile << "nc9K597jx1syT1q+SwFcykMtvWxCfD8BR7bcLILeO6z+HlRfvjOhUiHaX/KCaTN8" << std::endl;
            invalidKeyFile << "l7OJOgmUlL0FhQ1SXxw7KCSGd+rgu1iHwjFDDkj/tG24ashdmNt+DYdeoJu2mzgw" << std::endl;
            invalidKeyFile << "tEASfG9VjqBR7ni4Hg/sRpwXvEK5nI1JSLyZbcPCxGlBRdB8hMdny/VW+SBwKD2/" << std::endl;
            invalidKeyFile << "ivpVJLulw2oniSIcCCcr9d+ERY4XrO71UsiACwPxfdEtbG0KrZfpK91k7vl64DHM" << std::endl;
            invalidKeyFile << "CeTQPKRZm+LDKOUfv/eTF9F6GY4Dpw2LMwLM5QIDAQABAoIBABjV91etzK+Mxa61" << std::endl;
            invalidKeyFile << "AVCWzaUEkhvPvhKKGmy/VulnTj7IO98JBYlNLeoIRBIMql4QKRQWDNMMCtDQ8W6c" << std::endl;
            invalidKeyFile << "Gv5kux7QvrMfYViBGQ9/gucN/pnZ+vgkrw4AuiQM8pZuZpJJ6vH9HfvC6iwQkTR+" << std::endl;
            invalidKeyFile << "tdIPpvecfL3djCuTz7ns66iKo9ZGpRE6emTBynr8og/oqD8Vw5bW+JJ+AJ3IqZf4" << std::endl;
            invalidKeyFile << "NslNist7d5FZ5N/+nxWyBUcFglP7bZzb/raOVc/flrYIeDy72asnWOYbDTPzMyH1" << std::endl;
            invalidKeyFile << "dfaox6QKZtA5NdO9x4aHHGgAz8BTgqs7LvxPwoH+XF1dDCsb3kIeQxHTfcc1opMw" << std::endl;
            invalidKeyFile << "atxpgwECgYEA8Zq/7Z3tKcBlMz4XNKWWvaDxhBUIS62tGeLJ2spLRFvkL1ixnjcK" << std::endl;
            invalidKeyFile << "72YWOwDpoINEWa8AhAhM6afE9VxrupSGg+C9uALaJ8HTWTP6u6/F8sbsYaoWHyA/" << std::endl;
            invalidKeyFile << "k/8/nFEr43ciKUjBhMHB42vYidAgiOvDVXc+/k7HIMQfl/vyp32ecEECgYEA5fu9" << std::endl;
            invalidKeyFile << "ePLh55TYbXe8SCL0hsZcC8Q/ioT/0GJ6uevGb0lw3XAa+HC6//upu90T7ZOIqysc" << std::endl;
            invalidKeyFile << "aAqln7ZEeCfvXI/3YJyJ2RWatD+2itECbd0WV2/JflO/OAzDSSFvpxxmwIzccIeA" << std::endl;
            invalidKeyFile << "UNuNcQGD8HDwFzU+sULvF82yuwMt1syPd/mns6UCgYAviqP5vfnNHW7MhotKcMsY" << std::endl;
            invalidKeyFile << "xXLA6uKXAbXuQhI2W1g0O2DLcEiDOZGNSilVsvhF/Y6VlzoiwP9hewHmxijsrg1K" << std::endl;
            invalidKeyFile << "Jg8vBmCnMhzEkNXl2NC61SnujemMdmwMU03RFKfuOqMePJLX7MiaV75kX/AHAV2O" << std::endl;
            invalidKeyFile << "k8hxgk7sw6rz3UACdVWYAQKBgHUu5ScoksS+Cd0VQmF7Nh8qGSKBt2KsS/BxDVmI" << std::endl;
            invalidKeyFile << "ck6oHBMomQV340CliaHIjuvh3aRhzhKRQjzz0UVsC8GdNY4LlQ2AvZgUUr2+q78x" << std::endl;
            invalidKeyFile << "BL4+nmt43pj/n822dL6wcQaxf2zzDgWlKReojwLHeP5KSgxmL49wZx51CzlEd+HI" << std::endl;
            invalidKeyFile << "2pNlAoGBAObdC7woN7jEfdfYz1BhUpmBsIRqW2yLA1DnlK9lfgs2i1w7spzAh2hV" << std::endl;
            invalidKeyFile << "djPiKj5vZdcrbaa+SBAnZbFTHyXmAbKbO/iZpSromaZYyCK8NktJu/YxpWZmjnRF" << std::endl;
            invalidKeyFile << "2xOadRGCav5fTGzCN/ADLgIo4gIAI2o/UnV/MdaSAdHyIeSrxBAb" << std::endl;
            invalidKeyFile << "-----END RSA PRIVATE KEY-----" << std::endl;
            invalidKeyFile.close ();
        }
    }

    /**
     * @brief tear down test case.
     */
    static void TearDownTestCase ()
    {
        unlink (_rootcert.c_str ());
        unlink (_cert.c_str ());
        unlink (_key.c_str ());
        unlink (_invalidKey.c_str ());
    }

protected:
    /**
     * @brief Sets up the test fixture.
     */
    void SetUp ()
    {
        ASSERT_EQ (setCertificate (_cert, _key), 0) << join::lastError.message ();
        ASSERT_EQ (setCipher (join::crypto::defaultCipher_), 0) << join::lastError.message ();
    #if OPENSSL_VERSION_NUMBER >= 0x10101000L
        ASSERT_EQ (setCipher_1_3 (join::crypto::defaultCipher_1_3_), 0) << join::lastError.message ();
    #endif
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
        close ();
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
                sock.writeExactly (buf, nread);
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

    /// root certificate.
    static const std::string _rootcert;

    /// certificate.
    static const std::string _cert;

    /// private key.
    static const std::string _key;

    /// invalid private key.
    static const std::string _invalidKey;
};

const std::string TlsSocket::_host = "localhost";
const uint16_t    TlsSocket::_port = 5000;
const int         TlsSocket::_timeout = 1000;
const std::string TlsSocket::_rootcert = "/tmp/tlssocket_test_root.cert";
const std::string TlsSocket::_cert = "/tmp/tlssocket_test.cert";
const std::string TlsSocket::_key = "/tmp/tlssocket_test.key";
const std::string TlsSocket::_invalidKey = "/tmp/tlssocket_test_invalid.key";

/**
 * @brief Test construct method.
 */
TEST_F (TlsSocket, construct)
{
    ASSERT_THROW (Tls::Socket (nullptr, Tls::Socket::ClientMode), std::invalid_argument);
}

/**
 * @brief Test open method.
 */
TEST_F (TlsSocket, open)
{
    Tls::Socket tlsSocket;

    ASSERT_EQ (tlsSocket.open (Tls::v4 ()), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.open (Tls::v4 ()), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    tlsSocket.close ();

    ASSERT_EQ (tlsSocket.open (Tls::v6 ()), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.open (Tls::v6 ()), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    tlsSocket.close ();
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
    tlsSocket.close ();
    ASSERT_FALSE (tlsSocket.opened());
}

/**
 * @brief Test bind method.
 */
TEST_F (TlsSocket, bind)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);

    ASSERT_EQ (tlsSocket.connect ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.bind ({Tls::Resolver::resolveHost (_host)}), -1);
    ASSERT_EQ (tlsSocket.disconnect (), 0) << join::lastError.message ();

    ASSERT_EQ (tlsSocket.bind ({Tls::Resolver::resolveHost (_host)}), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.connect ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.disconnect (), 0) << join::lastError.message ();

    tlsSocket.close ();
}

/**
 * @brief Test connect method.
 */
TEST_F (TlsSocket, connect)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);

    ASSERT_EQ (tlsSocket.connect ({"255.255.255.255", _port}), -1);

    ASSERT_EQ (tlsSocket.connect ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.connect ({Tls::Resolver::resolveHost (_host), _port}), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    ASSERT_EQ (tlsSocket.disconnect (), 0) << join::lastError.message ();
    tlsSocket.close ();

    ASSERT_EQ (tlsSocket.connect (Tls::Resolver::resolve (_host + ":" + std::to_string (_port))), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.connect (Tls::Resolver::resolve (_host + ":" + std::to_string (_port))), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    ASSERT_EQ (tlsSocket.disconnect (), 0) << join::lastError.message ();
    tlsSocket.close ();
}

/**
 * @brief Test waitConnected method.
 */
TEST_F (TlsSocket, waitConnected)
{
    Tls::Socket tlsSocket;

    ASSERT_FALSE (tlsSocket.waitConnected (_timeout));
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
    tlsSocket.close ();
}

/**
 * @brief Test connectEncrypted method.
 */
TEST_F (TlsSocket, connectEncrypted)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);

    ASSERT_EQ (tlsSocket.connectEncrypted ({"255.255.255.255", _port}), -1);

    ASSERT_EQ (tlsSocket.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.disconnect (), 0) << join::lastError.message ();
    tlsSocket.close ();
}

/**
 * @brief Test startEncryption method.
 */
TEST_F (TlsSocket, startEncryption)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);

    ASSERT_EQ (tlsSocket.startEncryption (), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (tlsSocket.connect ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.startEncryption (), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.startEncryption (), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.disconnect (), 0) << join::lastError.message ();
    tlsSocket.close ();
}

/**
 * @brief Test waitEncrypted method.
 */
TEST_F (TlsSocket, waitEncrypted)
{
    Tls::Socket tlsSocket (Tls::Socket::NonBlocking);

    ASSERT_EQ (tlsSocket.open (), 0) << join::lastError.message ();
    ASSERT_FALSE (tlsSocket.waitEncrypted (_timeout));
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
    tlsSocket.close ();

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
    tlsSocket.close ();
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
    tlsSocket.close ();
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
    ASSERT_FALSE (tlsSocket.waitDisconnected (_timeout));
    if (tlsSocket.disconnect () == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (tlsSocket.waitDisconnected (_timeout)) << join::lastError.message ();
    tlsSocket.close ();
}

/**
 * @brief Test waitReadyRead method.
 */
TEST_F (TlsSocket, waitReadyRead)
{
    Tls::Socket tlsSocket;
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_FALSE (tlsSocket.waitReadyRead (_timeout));
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    if (tlsSocket.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port}) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (tlsSocket.waitEncrypted (_timeout)) << join::lastError.message ();
    ASSERT_TRUE (tlsSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.writeExactly (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (tlsSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    if (tlsSocket.disconnect () == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (tlsSocket.waitDisconnected (_timeout)) << join::lastError.message ();
    tlsSocket.close ();
}

/**
 * @brief Test canRead method.
 */
TEST_F (TlsSocket, canRead)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (tlsSocket.canRead (), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (tlsSocket.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (tlsSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.writeExactly (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (tlsSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.read (data, 1), 1) << join::lastError.message ();
    ASSERT_GT (tlsSocket.canRead (), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.disconnect (), 0) << join::lastError.message ();
    tlsSocket.close ();
}

/**
 * @brief Test read method.
 */
TEST_F (TlsSocket, read)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (tlsSocket.read (data, sizeof (data)), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (tlsSocket.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (tlsSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.writeExactly (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (tlsSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_GT (tlsSocket.read (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.disconnect (), 0) << join::lastError.message ();
    tlsSocket.close ();
}

/**
 * @brief Test readExactly method.
 */
TEST_F (TlsSocket, readExactly)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (tlsSocket.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (tlsSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.writeExactly (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (tlsSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.readExactly (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.disconnect (), 0) << join::lastError.message ();
    tlsSocket.close ();
}

/**
 * @brief Test waitReadyWrite method.
 */
TEST_F (TlsSocket, waitReadyWrite)
{
    Tls::Socket tlsSocket;

    ASSERT_FALSE (tlsSocket.waitReadyWrite (_timeout));
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
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
    tlsSocket.close ();
}

/**
 * @brief Test write method.
 */
TEST_F (TlsSocket, write)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (tlsSocket.write (data, sizeof (data)), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (tlsSocket.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (tlsSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_GT (tlsSocket.write (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (tlsSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.disconnect (), 0) << join::lastError.message ();
    tlsSocket.close ();
}

/**
 * @brief Test writeExactly method.
 */
TEST_F (TlsSocket, writeExactly)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);
    char data [] = { 0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (tlsSocket.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (tlsSocket.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.writeExactly (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (tlsSocket.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.disconnect (), 0) << join::lastError.message ();
    tlsSocket.close ();
}

/**
 * @brief Test setMode method.
 */
TEST_F (TlsSocket, setMode)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);

    ASSERT_EQ (tlsSocket.setMode (Tls::Socket::Blocking), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setMode (Tls::Socket::NonBlocking), 0) << join::lastError.message ();
    if (tlsSocket.disconnect () == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (tlsSocket.waitDisconnected (_timeout)) << join::lastError.message ();
    tlsSocket.close ();
}

/**
 * @brief Test setOption method.
 */
TEST_F (TlsSocket, setOption)
{
    Tls::Socket tlsSocket;

    ASSERT_EQ (tlsSocket.setOption (Tls::Socket::RcvBuffer, 1500), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (tlsSocket.open (), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setOption (Tls::Socket::NoDelay, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setOption (Tls::Socket::KeepAlive, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setOption (Tls::Socket::KeepIdle, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setOption (Tls::Socket::KeepIntvl, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setOption (Tls::Socket::KeepCount, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setOption (Tls::Socket::SndBuffer, 1500), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setOption (Tls::Socket::RcvBuffer, 1500), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setOption (Tls::Socket::TimeStamp, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setOption (Tls::Socket::ReuseAddr, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setOption (Tls::Socket::ReusePort, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setOption (Tls::Socket::Broadcast, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setOption (Tls::Socket::Ttl, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setOption (Tls::Socket::MulticastLoop, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setOption (Tls::Socket::MulticastTtl, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (tlsSocket.setOption (Tls::Socket::PathMtuDiscover, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setOption (Tls::Socket::RcvError, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setOption (Tls::Socket::AuxData, 1), -1);
    ASSERT_EQ (join::lastError, std::errc::no_protocol_option);
    tlsSocket.close ();

    ASSERT_EQ (tlsSocket.open (Tls::v6 ()), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setOption (Tls::Socket::NoDelay, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setOption (Tls::Socket::KeepAlive, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setOption (Tls::Socket::KeepIdle, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setOption (Tls::Socket::KeepIntvl, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setOption (Tls::Socket::KeepCount, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setOption (Tls::Socket::SndBuffer, 1500), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setOption (Tls::Socket::RcvBuffer, 1500), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setOption (Tls::Socket::TimeStamp, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setOption (Tls::Socket::ReuseAddr, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setOption (Tls::Socket::ReusePort, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setOption (Tls::Socket::Broadcast, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setOption (Tls::Socket::Ttl, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setOption (Tls::Socket::MulticastLoop, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setOption (Tls::Socket::MulticastTtl, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (tlsSocket.setOption (Tls::Socket::PathMtuDiscover, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setOption (Tls::Socket::RcvError, 1), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setOption (Tls::Socket::AuxData, 1), -1);
    ASSERT_EQ (join::lastError, std::errc::no_protocol_option);
    tlsSocket.close ();
}

/**
 * @brief Test localEndpoint method.
 */
TEST_F (TlsSocket, localEndpoint)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);

    ASSERT_EQ (tlsSocket.localEndpoint (), Tls::Endpoint {});
    ASSERT_EQ (tlsSocket.bind ({Tls::Resolver::resolveHost (_host), uint16_t (_port + 1)}), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.connect ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.localEndpoint (), Tls::Endpoint (Tls::Resolver::resolveHost (_host), uint16_t (_port + 1))) << join::lastError.message ();
    tlsSocket.close ();
}

/**
 * @brief Test remoteEndpoint method.
 */
TEST_F (TlsSocket, remoteEndpoint)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);

    ASSERT_EQ (tlsSocket.remoteEndpoint (), Tls::Endpoint {});
    ASSERT_EQ (tlsSocket.bind ({Tls::Resolver::resolveHost (_host), uint16_t (_port + 1)}), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.connect ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.remoteEndpoint (), Tls::Endpoint (Tls::Resolver::resolveHost (_host), _port)) << join::lastError.message ();
    tlsSocket.close ();
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
    tlsSocket.close ();
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
    tlsSocket.close ();
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
    tlsSocket.close ();
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
    tlsSocket.close ();

    ASSERT_EQ (tlsSocket.bind (IpAddress (AF_INET)), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.family (), AF_INET);
    tlsSocket.close ();
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
    tlsSocket.close ();
    ASSERT_EQ (tlsSocket.handle (), -1);
}

/**
 * @brief Test mtu method.
 */
TEST_F (TlsSocket, mtu)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);

    ASSERT_EQ (tlsSocket.mtu (), -1);
    ASSERT_EQ (tlsSocket.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_NE (tlsSocket.mtu (), -1) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.mtu (), -1);
    tlsSocket.close ();
    ASSERT_EQ (tlsSocket.mtu (), -1);
}

/**
 * @brief Test checksum method.
 */
TEST_F (TlsSocket, checksum)
{
    std::string buffer ({'\xD2', '\xB6', '\x69', '\xFD', '\x2E'});

    ASSERT_EQ (Tls::Socket::checksum (reinterpret_cast <uint16_t *> (&buffer[0]), buffer.size (), 0), 19349);
}

/**
 * @brief Test setCertificate method.
 */
TEST_F (TlsSocket, setCertificate)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);

    ASSERT_EQ (tlsSocket.setCertificate ("foo"), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (tlsSocket.setCertificate (_cert), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (tlsSocket.setCertificate (_cert, "foo"), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (tlsSocket.setCertificate (_cert, _invalidKey), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (tlsSocket.setCertificate (_cert, _key), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setCertificate (_cert, _key), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setCertificate (_cert, _key), 0) << join::lastError.message ();
    tlsSocket.close ();
}

/**
 * @brief Test setCaCertificate method.
 */
TEST_F (TlsSocket, setCaCertificate)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);

    ASSERT_EQ (tlsSocket.setCaCertificate ("foo"), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (tlsSocket.setCaCertificate (_cert), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setCaCertificate (_cert), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setCaCertificate (_cert), 0) << join::lastError.message ();
    tlsSocket.close ();
}

/**
 * @brief Test setVerify method.
 */
TEST_F (TlsSocket, setVerify)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);

    tlsSocket.setVerify (false);
    ASSERT_EQ (tlsSocket.connectEncrypted (Tls::Resolver::resolve (_host + ":" + std::to_string ( _port))), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.disconnect (), 0) << join::lastError.message ();
    tlsSocket.setVerify (true, 0);
    ASSERT_EQ (tlsSocket.connectEncrypted (Tls::Resolver::resolve (_host + ":" + std::to_string ( _port))), -1);
    ASSERT_EQ (tlsSocket.setCaCertificate (_rootcert), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.connectEncrypted (Tls::Resolver::resolve (_host + ":" + std::to_string ( _port))), -1);
    tlsSocket.setVerify (true, 1);
    ASSERT_EQ (tlsSocket.connectEncrypted (Tls::Resolver::resolve (_host + ":" + std::to_string ( _port))), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.connectEncrypted (Tls::Resolver::resolve ("127.0.0.1:" + std::to_string ( _port))), -1);
    tlsSocket.close ();
}

/**
 * @brief Test setCipher method.
 */
TEST_F (TlsSocket, setCipher)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);

    ASSERT_EQ (tlsSocket.setCipher ("foo"), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (tlsSocket.setCipher (join::crypto::defaultCipher_), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setCipher (join::crypto::defaultCipher_), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setCipher (join::crypto::defaultCipher_), 0) << join::lastError.message ();
    tlsSocket.close ();
}

#if OPENSSL_VERSION_NUMBER >= 0x10101000L
/**
 * @brief Test setCipher_1_3 method.
 */
TEST_F (TlsSocket, setCipher_1_3)
{
    Tls::Socket tlsSocket (Tls::Socket::Blocking);

    ASSERT_EQ (tlsSocket.setCipher_1_3 ("foo"), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (tlsSocket.setCipher_1_3 (join::crypto::defaultCipher_1_3_), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setCipher_1_3 (join::crypto::defaultCipher_1_3_), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.disconnect (), 0) << join::lastError.message ();
    ASSERT_EQ (tlsSocket.setCipher_1_3 (join::crypto::defaultCipher_1_3_), 0) << join::lastError.message ();
    tlsSocket.close ();
}
#endif

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
    tlsSocket1.close ();
    tlsSocket2.close ();
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
