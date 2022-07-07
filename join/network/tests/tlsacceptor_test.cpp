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
class TlsAcceptor : public ::testing::Test
{
public:
    /**
     * @brief set up test case.
     */
    static void SetUpTestCase ()
    {
        std::ofstream rootCertFile (_root);
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
        unlink (_root.c_str ());
        unlink (_cert.c_str ());
        unlink (_key.c_str ());
        unlink (_invalidKey.c_str ());
    }

protected:
    /// host ip address.
    static const IpAddress _hostip;

    /// host name.
    static const std::string _hostname;

    /// port.
    static const uint16_t _port;

    /// root certificate.
    static const std::string _root;

    /// certificate.
    static const std::string _cert;

    /// private key.
    static const std::string _key;

    /// invalid private key.
    static const std::string _invalidKey;
};

const IpAddress   TlsAcceptor::_hostip     = "127.0.0.1";
const std::string TlsAcceptor::_hostname   = "localhost";
const uint16_t    TlsAcceptor::_port       = 5000;
const std::string TlsAcceptor::_root       = "/tmp/tlsserver_test_root.cert";
const std::string TlsAcceptor::_cert       = "/tmp/tlsserver_test.cert";
const std::string TlsAcceptor::_key        = "/tmp/tlsserver_test.key";
const std::string TlsAcceptor::_invalidKey = "/tmp/tlsserver_test_invalid.key";

/**
 * @brief Assign by move.
 */
TEST_F (TlsAcceptor, move)
{
    Tls::Acceptor server1, server2;

    ASSERT_EQ (server1.open (Tls::v6 ()), 0) << join::lastError.message ();

    server2 = std::move (server1);
    ASSERT_TRUE (server2.opened ());

    Tls::Acceptor server3 = std::move (server2);
    ASSERT_TRUE (server3.opened ());
}

/**
 * @brief Test open method.
 */
TEST_F (TlsAcceptor, open)
{
    Tls::Acceptor server;

    ASSERT_EQ (server.open (Tls::v6 ()), 0) << join::lastError.message ();
    ASSERT_EQ (server.open (Tls::v6 ()), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
}

/**
 * @brief Test close method.
 */
TEST_F (TlsAcceptor, close)
{
    Tls::Acceptor server;

    ASSERT_EQ (server.open (Tls::v6 ()), 0) << join::lastError.message ();
}

/**
 * @brief Test bind method.
 */
TEST_F (TlsAcceptor, bind)
{
    Tls::Acceptor server;

    ASSERT_EQ (server.bind ({_hostip, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (server.bind ({_hostip, _port}), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
}

/**
 * @brief Test listen method.
 */
TEST_F (TlsAcceptor, listen)
{
    Tls::Acceptor server;

    ASSERT_EQ (server.listen (20), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (server.bind ({_hostip, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (server.listen (20), 0) << join::lastError.message ();
}

/**
 * @brief Test accept method.
 */
TEST_F (TlsAcceptor, accept)
{
    Tls::Socket clientSocket (Tls::Socket::Blocking);
    Tls::Acceptor server;

    ASSERT_FALSE (server.accept ().connected ());
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (server.bind ({_hostip, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (server.listen (), 0) << join::lastError.message ();
    ASSERT_EQ (clientSocket.connect({_hostip, _port}), 0) << join::lastError.message ();
    Tls::Socket serverSocket = server.accept ();
    ASSERT_TRUE (serverSocket.connected ());
    ASSERT_EQ (serverSocket.localEndpoint ().ip (), _hostip);
    ASSERT_EQ (serverSocket.localEndpoint ().port (), _port);
}

/**
 * @brief Test localEndpoint method.
 */
TEST_F (TlsAcceptor, localEndpoint)
{
    Tls::Acceptor server;

    ASSERT_EQ (server.localEndpoint (), Tls::Endpoint {});
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (server.bind ({_hostip, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (server.localEndpoint ().ip (), _hostip);
    ASSERT_EQ (server.localEndpoint ().port (), _port);
}

/**
 * @brief Test opened method.
 */
TEST_F (TlsAcceptor, opened)
{
    Tls::Acceptor server;

    ASSERT_FALSE (server.opened ());
    ASSERT_EQ (server.open (Tls::v6 ()), 0) << join::lastError.message ();
    ASSERT_TRUE (server.opened ());
    server.close ();
    ASSERT_FALSE (server.opened ());
}

/**
 * @brief Test family method.
 */
TEST_F (TlsAcceptor, family)
{
    Tls::Acceptor server;

    ASSERT_EQ (server.bind ({_hostip, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (server.family (), AF_INET);
}

/**
 * @brief Test type method.
 */
TEST_F (TlsAcceptor, type)
{
    Tls::Acceptor server;

    ASSERT_EQ (server.bind ({_hostip, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (server.type (), SOCK_STREAM);
}

/**
 * @brief Test protocol method.
 */
TEST_F (TlsAcceptor, protocol)
{
    Tls::Acceptor server;

    ASSERT_EQ (server.bind ({_hostip, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (server.protocol (), IPPROTO_TCP);
}

/**
 * @brief Test handle method.
 */
TEST_F (TlsAcceptor, handle)
{
    Tls::Acceptor server;

    ASSERT_EQ (server.handle (), -1);
    ASSERT_EQ (server.open (Tls::v6 ()), 0) << join::lastError.message ();
    ASSERT_GT (server.handle (), -1);
    server.close ();
    ASSERT_EQ (server.handle (), -1);
}

/**
 * @brief Test setCertificate method.
 */
TEST_F (TlsAcceptor, setCertificate)
{
    Tls::Acceptor server;

    ASSERT_EQ (server.setCertificate ("foo"), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (server.setCertificate (_cert), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (server.setCertificate (_cert, "foo"), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (server.setCertificate (_cert, _invalidKey), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (server.setCertificate (_cert, _key), 0) << join::lastError.message ();
}

/**
 * @brief Test setVerify method.
 */
TEST_F (TlsAcceptor, setVerify)
{
    Tls::Acceptor server;

    ASSERT_NO_THROW (server.setVerify (true));
    ASSERT_NO_THROW (server.setVerify (false));
}

/**
 * @brief Test setCaCertificate method.
 */
TEST_F (TlsAcceptor, setCaCertificate)
{
    Tls::Acceptor server;

    ASSERT_EQ (server.setCaCertificate ("foo"), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (server.setCaCertificate (_cert), 0) << join::lastError.message ();
}

/**
 * @brief Test setCipher method.
 */
TEST_F (TlsAcceptor, setCipher)
{
    Tls::Acceptor server;

    ASSERT_EQ (server.setCipher ("foo"), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (server.setCipher (join::crypto::defaultCipher_), 0) << join::lastError.message ();
}

#if OPENSSL_VERSION_NUMBER >= 0x10101000L
/**
 * @brief Test setCipher_1_3 method.
 */
TEST_F (TlsAcceptor, setCipher_1_3)
{
    Tls::Acceptor server;

    ASSERT_EQ (server.setCipher_1_3 ("foo"), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (server.setCipher_1_3 (join::crypto::defaultCipher_1_3_), 0) << join::lastError.message ();
}
#endif

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
/**
 * @brief Test setCurve method.
 */
TEST_F (TlsAcceptor, setCurve)
{
    Tls::Acceptor server;

    ASSERT_EQ (server.setCurve ("foo"), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (server.setCurve (join::crypto::defaultCurve_), 0) << join::lastError.message ();
}
#endif

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    join::crypto::initializeOpenSSL ();
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
