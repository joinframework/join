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
 * @brief Class used to test the TLS socket stream API.
 */
class TlsSocketStream : public ::testing::Test, public Tls::Acceptor::Observer
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
            certFile << "MIIDljCCAzygAwIBAgIUR3ZIuKMt0BdaOZQnPwhSMR9qzfYwCgYIKoZIzj0EAwIw" << std::endl;
            certFile << "gcQxCzAJBgNVBAYTAkZSMRIwEAYDVQQIDAlPY2NpdGFuaWUxEDAOBgNVBAcMB0Nh" << std::endl;
            certFile << "c3RyZXMxFzAVBgNVBAoMDkpvaW4gRnJhbWV3b3JrMS0wKwYDVQQLDCRKb2luIEZy" << std::endl;
            certFile << "YW1ld29yayBDZXJ0aWZpY2F0ZSBBdXRob3JpdHkxHTAbBgNVBAMMFGNhLmpvaW5m" << std::endl;
            certFile << "cmFtZXdvcmsubmV0MSgwJgYJKoZIhvcNAQkBFhlzdXBwb3J0QGpvaW5mcmFtZXdv" << std::endl;
            certFile << "cmsubmV0MB4XDTIyMDcwNjEzMzMwN1oXDTMyMDcwMzEzMzMwN1owgacxCzAJBgNV" << std::endl;
            certFile << "BAYTAkZSMRIwEAYDVQQIDAlPY2NpdGFuaWUxEDAOBgNVBAcMB0Nhc3RyZXMxFzAV" << std::endl;
            certFile << "BgNVBAoMDkpvaW4gRnJhbWV3b3JrMRswGQYDVQQLDBJKb2luIEZyYW1ld29yayBE" << std::endl;
            certFile << "ZXYxEjAQBgNVBAMMCWxvY2FsaG9zdDEoMCYGCSqGSIb3DQEJARYZc3VwcG9ydEBq" << std::endl;
            certFile << "b2luZnJhbWV3b3JrLm5ldDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB" << std::endl;
            certFile << "AM4RD6B4SXS4ERBDNm3aDHYYN4CteBbsOAtDtI4Muw8e+Rs0BhIU+WwisSJhUuuw" << std::endl;
            certFile << "YAM+KUEyk9vt74TgnYTNklZYVBxSJvKAmaHmB/irPlgzvA/BS3IJZ1kw9UM0Bhfs" << std::endl;
            certFile << "FIy+8gKMAwscRHIyfB7hygSYnsbYP/P73K3ARpNKB6Izi4vKIfDdN3I3CKJafZ+o" << std::endl;
            certFile << "AcOoE3rrIkoFVTDLzd0VKrE0r3Xxvn7O1UXK26ZAN2kL40uo/DR2PeyB0GI4sj1B" << std::endl;
            certFile << "QYlWhji3Ss9UnpisEwxnk8bxQVrE/AnqpOUGIZ8ql0Hw9fZ0or1csBMOgq1AwBXQ" << std::endl;
            certFile << "jAzUeBYE0m0ys7Zb9r3YOE8CAwEAAaNcMFowCwYDVR0PBAQDAgXgMB0GA1UdJQQW" << std::endl;
            certFile << "MBQGCCsGAQUFBwMBBggrBgEFBQcDAjAsBgNVHREEJTAjgglsb2NhbGhvc3SHBH8A" << std::endl;
            certFile << "AAGHEAAAAAAAAAAAAAAAAAAAAAAwCgYIKoZIzj0EAwIDSAAwRQIhAIu+0oI0enGS" << std::endl;
            certFile << "zjEfoHwMzUtdtY7BYKQiftsxYFRcxenXAiB98gEYH4LO17ZxZSDYhsCQleshuJ0D" << std::endl;
            certFile << "bQZplxED8CqeNQ==" << std::endl;
            certFile << "-----END CERTIFICATE-----" << std::endl;
            certFile.close ();
        }

        std::ofstream keyFile (_key);
        if (keyFile.is_open ())
        {
            keyFile << "-----BEGIN RSA PRIVATE KEY-----" << std::endl;
            keyFile << "MIIEpAIBAAKCAQEAzhEPoHhJdLgREEM2bdoMdhg3gK14Fuw4C0O0jgy7Dx75GzQG" << std::endl;
            keyFile << "EhT5bCKxImFS67BgAz4pQTKT2+3vhOCdhM2SVlhUHFIm8oCZoeYH+Ks+WDO8D8FL" << std::endl;
            keyFile << "cglnWTD1QzQGF+wUjL7yAowDCxxEcjJ8HuHKBJiextg/8/vcrcBGk0oHojOLi8oh" << std::endl;
            keyFile << "8N03cjcIolp9n6gBw6gTeusiSgVVMMvN3RUqsTSvdfG+fs7VRcrbpkA3aQvjS6j8" << std::endl;
            keyFile << "NHY97IHQYjiyPUFBiVaGOLdKz1SemKwTDGeTxvFBWsT8Ceqk5QYhnyqXQfD19nSi" << std::endl;
            keyFile << "vVywEw6CrUDAFdCMDNR4FgTSbTKztlv2vdg4TwIDAQABAoIBAQC0p5JqnWnQkNos" << std::endl;
            keyFile << "xq/+CG5qTfrCrdGdTwQnI/kzm4eWzxGWvrofuhGcsqFWQbp/dAYIccObK+sioWsd" << std::endl;
            keyFile << "tAmEdvC3EALVPVR1vzZxEAinAgHLM7fInC43UHUxZVFv1DkPWeH+LhxfDT5RzDtZ" << std::endl;
            keyFile << "Xlcgf9QqyV5Rdx5CGOkzzmBRGlKs6CyzuN80vYpmciK2ool9M7EXQe2CFvOMsNDW" << std::endl;
            keyFile << "2k36Ybg7PNarJOhGTkuOG/WjLuP4+k8cctF5JuZYorbtZP7lk0UiJ+MjShttk10f" << std::endl;
            keyFile << "brH8Jc6DCxXebv5nehtecE6QvPPdvJm9rIb8AOfyisN7cvLecNPduz0Cxu6xk4hN" << std::endl;
            keyFile << "BwscwPIZAoGBAP6EZPvmNBLKourDwoeMBvPjP1dWmmNDAjSbQINWdthgnQYo0fMH" << std::endl;
            keyFile << "sYE7T1/sCohGNVafEsMDwuwSNnljHA7J2kDteZYzWae99xxO7Bcjr4cg0DmT2Knv" << std::endl;
            keyFile << "Gm5gG/yjhgCbnyDO6XRdi39ZwVk6Hay0SIHZLYisSXjx11B0r6XeNoqVAoGBAM9E" << std::endl;
            keyFile << "Z2dKxRfJZix0M0D7YW9acxhrI/tWG4Pkti6bqxfbUtXMzrjgFTuj03qyjpZU/oQy" << std::endl;
            keyFile << "NTugq2ih0q628sWUH71l0x7V9yGdTh2wZ5vL9EF9QlCG2fEcn9/KsjiwrtsoJ9Ft" << std::endl;
            keyFile << "pdmMrDsYOL3Tp1PEm9yZnEqyMcrSnHaUB67d26JTAoGAbVODaSymG5hNSNiT29OL" << std::endl;
            keyFile << "PQHVOHfr0016SgySNphSbnl5maa5IFKiradDXimvEIBP8whbb8dS2EKugY/QAo40" << std::endl;
            keyFile << "IQWg36LpFQOlfNRt1zat9DZlGwZl4ADj8pt4ChpXujUesmIOp7xy6l4sjl5HVuMN" << std::endl;
            keyFile << "7jDSvU18NeZ0HYwx0ubTuM0CgYBBdm5eTlw/rgmKQs0pWfwlKmEttjEwIbshBiyQ" << std::endl;
            keyFile << "PfRk3Y2lH0GvXH74Tj7uAtVMH94fLKhpg85/hpS/P+MfijAYJr/ufk/GmyNf9yZS" << std::endl;
            keyFile << "K7GiuYgnXOAa6hqImUF+7Dbd2ynwWHxIYMjJBVZuhhnUOEWuAApAAVX+pFRsk0Z1" << std::endl;
            keyFile << "8XZ8JwKBgQC2FHE/YXJb+xl9yHYs+skn8pBqMT+S/2f8vc6bfUdlGOR42FkbMoG2" << std::endl;
            keyFile << "RQi4as4mW6bv34u/H9l4/M+ay+wV2C9JvB4pbwEMSCw2J3VDwFlXEDjpaxToT2X1" << std::endl;
            keyFile << "bIishrH1ur2h7C3ZpNuv0zfl8+IiA/diTmqQC8/iIUG7DsQukJ8Uyg==" << std::endl;
            keyFile << "-----END RSA PRIVATE KEY-----" << std::endl;
            keyFile.close ();
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

    /// timeout.
    static const int _timeout;

    /// host.
    static const std::string _host;

    /// port.
    static const uint16_t _port;
    static const uint16_t _invalid_port;

    /// root certificate.
    static const std::string _rootcert;

    /// certificate.
    static const std::string _cert;

    /// private key.
    static const std::string _key;
};

const int         TlsSocketStream::_timeout = 1000;
const std::string TlsSocketStream::_host = "localhost";
const uint16_t    TlsSocketStream::_port = 5000;
const uint16_t    TlsSocketStream::_invalid_port = 5032;
const std::string TlsSocketStream::_rootcert = "/tmp/tlssocket_test_root.cert";
const std::string TlsSocketStream::_cert = "/tmp/tlssocket_test.cert";
const std::string TlsSocketStream::_key = "/tmp/tlssocket_test.key";

/**
 * @brief Test default constructor.
 */
TEST_F (TlsSocketStream, defaultConstruct)
{
    Tls::Stream tlsStream;
    ASSERT_TRUE (tlsStream.good ()) << join::lastError.message ();
}

/**
 * @brief Test move constructor.
 */
TEST_F (TlsSocketStream, moveConstruct)
{
    Tls::Stream tmp;
    ASSERT_TRUE (tmp.good ()) << join::lastError.message ();
    Tls::Stream tlsStream (std::move (tmp));
    ASSERT_TRUE (tlsStream.good ()) << join::lastError.message ();
}

/**
 * @brief Test move operatore.
 */
TEST_F (TlsSocketStream, moveAssign)
{
    Tls::Stream tmp;
    ASSERT_TRUE (tmp.good ()) << join::lastError.message ();
    Tls::Stream tlsStream;
    tlsStream = std::move (tmp);
    ASSERT_TRUE (tlsStream.good ()) << join::lastError.message ();
}

/**
 * @brief Test connect method.
 */
TEST_F (TlsSocketStream, connect)
{
    Tls::Stream tlsStream;
    tlsStream.connect ({Tls::Resolver::resolveHost (_host), _invalid_port});
    ASSERT_TRUE (tlsStream.fail ());
    tlsStream.clear ();
    tlsStream.connect ({Tls::Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (tlsStream.good ()) << join::lastError.message ();
    tlsStream.close ();
}

/**
 * @brief Test startEncryption method.
 */
TEST_F (TlsSocketStream, startEncryption)
{
    Tls::Stream tlsStream;
    tlsStream.connect ({Tls::Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (tlsStream.good ()) << join::lastError.message ();
    tlsStream.startEncryption ();
    ASSERT_TRUE (tlsStream.good ()) << join::lastError.message ();
    tlsStream.close ();
}

/**
 * @brief Test connectEncrypted method.
 */
TEST_F (TlsSocketStream, connectEncrypted)
{
    Tls::Stream tlsStream;
    tlsStream.connect ({Tls::Resolver::resolveHost (_host), _invalid_port});
    ASSERT_TRUE (tlsStream.fail ());
    tlsStream.clear ();
    tlsStream.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (tlsStream.good ()) << join::lastError.message ();
    tlsStream.close ();
}

/**
 * @brief Test close method.
 */
TEST_F (TlsSocketStream, close)
{
    Tls::Stream tlsStream;
    tlsStream.connect ({Tls::Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (tlsStream.good ()) << join::lastError.message ();
    tlsStream.close ();
    ASSERT_TRUE (tlsStream.good ()) << join::lastError.message ();
}

/**
 * @brief Test connected method.
 */
TEST_F (TlsSocketStream, connected)
{
    Tls::Stream tlsStream;
    ASSERT_FALSE (tlsStream.connected ());
    tlsStream.connect ({Tls::Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (tlsStream.good ()) << join::lastError.message ();
    ASSERT_TRUE (tlsStream.connected ());
    tlsStream.close ();
    ASSERT_TRUE (tlsStream.good ()) << join::lastError.message ();
    ASSERT_FALSE (tlsStream.connected ());
}

/**
 * @brief Test encrypted method.
 */
TEST_F (TlsSocketStream, encrypted)
{
    Tls::Stream tlsStream;
    ASSERT_FALSE (tlsStream.encrypted ());
    tlsStream.connect ({Tls::Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (tlsStream.good ()) << join::lastError.message ();
    ASSERT_FALSE (tlsStream.encrypted ());
    tlsStream.startEncryption ();
    ASSERT_TRUE (tlsStream.good ()) << join::lastError.message ();
    ASSERT_TRUE (tlsStream.encrypted ());
    tlsStream.close ();
    ASSERT_TRUE (tlsStream.good ()) << join::lastError.message ();
    ASSERT_FALSE (tlsStream.encrypted ());
    tlsStream.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (tlsStream.good ()) << join::lastError.message ();
    ASSERT_TRUE (tlsStream.encrypted ());
    tlsStream.close ();
}

/**
 * @brief Test timeout method.
 */
TEST_F (TlsSocketStream, timeout)
{
    Tls::Stream tlsStream;
    ASSERT_NE (tlsStream.timeout (), _timeout);
    tlsStream.timeout (_timeout);
    ASSERT_EQ (tlsStream.timeout (), _timeout);
}

/**
 * @brief Test socket method.
 */
TEST_F (TlsSocketStream, socket)
{
    Tls::Stream tlsStream;
    ASSERT_EQ (tlsStream.socket ().handle (), -1);
    tlsStream.connect ({Tls::Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (tlsStream.good ()) << join::lastError.message ();
    ASSERT_NE (tlsStream.socket ().handle (), -1);
    tlsStream.close ();
    ASSERT_TRUE (tlsStream.good ()) << join::lastError.message ();
    ASSERT_EQ (tlsStream.socket ().handle (), -1);
}

/**
 * @brief Test insert operator.
 */
TEST_F (TlsSocketStream, insert)
{
    Tls::Stream tlsStream;
    tlsStream << "test" << std::endl;
    ASSERT_TRUE (tlsStream.fail ());
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    tlsStream.clear ();
    tlsStream.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (tlsStream.good ()) << join::lastError.message ();
    tlsStream << "test" << std::endl;
    ASSERT_TRUE (tlsStream.socket ().waitReadyRead (_timeout));
    ASSERT_TRUE (tlsStream.good ()) << join::lastError.message ();
    tlsStream.close ();
}

/**
 * @brief Test put method.
 */
TEST_F (TlsSocketStream, put)
{
    Tls::Stream tlsStream;
    tlsStream.put ('t');
    ASSERT_TRUE (tlsStream.fail ());
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    tlsStream.clear ();
    tlsStream.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (tlsStream.good ()) << join::lastError.message ();
    tlsStream.put ('t');
    tlsStream.put ('e');
    tlsStream.put ('s');
    tlsStream.put ('t');
    ASSERT_TRUE (tlsStream.socket ().waitReadyRead (_timeout));
    ASSERT_TRUE (tlsStream.good ()) << join::lastError.message ();
    tlsStream.close ();
}

/**
 * @brief Test write method.
 */
TEST_F (TlsSocketStream, write)
{
    Tls::Stream tlsStream;
    tlsStream.write ("test", 4);
    ASSERT_TRUE (tlsStream.fail ());
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    tlsStream.clear ();
    tlsStream.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (tlsStream.good ()) << join::lastError.message ();
    tlsStream.write ("test", 4);
    ASSERT_TRUE (tlsStream.socket ().waitReadyRead (_timeout));
    ASSERT_TRUE (tlsStream.good ()) << join::lastError.message ();
    tlsStream.close ();
}

/**
 * @brief Test flush method.
 */
TEST_F (TlsSocketStream, flush)
{
    Tls::Stream tlsStream;
    tlsStream.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (tlsStream.good ()) << join::lastError.message ();
    tlsStream.put ('t');
    tlsStream.flush ();
    tlsStream.put ('e');
    tlsStream.flush ();
    tlsStream.put ('s');
    tlsStream.flush ();
    tlsStream.put ('t');
    tlsStream.flush ();
    ASSERT_TRUE (tlsStream.socket ().waitReadyRead (_timeout));
    ASSERT_TRUE (tlsStream.good ()) << join::lastError.message ();
    tlsStream.close ();
}

/**
 * @brief Test extract method.
 */
TEST_F (TlsSocketStream, extract)
{
    int test;
    Tls::Stream tlsStream;
    tlsStream >> test;
    ASSERT_TRUE (tlsStream.fail ());
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    tlsStream.clear ();
    tlsStream.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (tlsStream.good ()) << join::lastError.message ();
    tlsStream << int (123456789) << std::endl;
    tlsStream.flush ();
    tlsStream >> test;
    ASSERT_EQ (test, 123456789);
    tlsStream.close ();
}

/**
 * @brief Test get method.
 */
TEST_F (TlsSocketStream, get)
{
    Tls::Stream tlsStream;
    tlsStream.get ();
    ASSERT_TRUE (tlsStream.fail ());
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    tlsStream.clear ();
    tlsStream.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (tlsStream.good ()) << join::lastError.message ();
    tlsStream.write ("test", 4);
    tlsStream.flush ();
    ASSERT_EQ (tlsStream.get (), 't');
    ASSERT_EQ (tlsStream.get (), 'e');
    ASSERT_EQ (tlsStream.get (), 's');
    ASSERT_EQ (tlsStream.get (), 't');
    tlsStream.close ();
}

/**
 * @brief Test peek method.
 */
TEST_F (TlsSocketStream, peek)
{
    Tls::Stream tlsStream;
    tlsStream.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port});
    ASSERT_TRUE (tlsStream.good ()) << join::lastError.message ();
    tlsStream.write ("test", 4);
    tlsStream.flush ();
    ASSERT_EQ (tlsStream.peek (), 't');
    ASSERT_EQ (tlsStream.get (), 't');
    ASSERT_EQ (tlsStream.peek (), 'e');
    ASSERT_EQ (tlsStream.get (), 'e');
    ASSERT_EQ (tlsStream.peek (), 's');
    ASSERT_EQ (tlsStream.get (), 's');
    ASSERT_EQ (tlsStream.peek (), 't');
    ASSERT_EQ (tlsStream.get (), 't');
    tlsStream.close ();
}

/**
 * @brief Test unget method.
 */
TEST_F (TlsSocketStream, unget)
{
    Tls::Stream tlsStream;
    tlsStream.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port});
    tlsStream.write ("test", 4);
    tlsStream.flush ();
    ASSERT_EQ (tlsStream.get (), 't');
    tlsStream.unget ();
    ASSERT_EQ (tlsStream.get (), 't');
    ASSERT_EQ (tlsStream.get (), 'e');
    tlsStream.unget ();
    ASSERT_EQ (tlsStream.get (), 'e');
    ASSERT_EQ (tlsStream.get (), 's');
    tlsStream.unget ();
    ASSERT_EQ (tlsStream.get (), 's');
    ASSERT_EQ (tlsStream.get (), 't');
    tlsStream.unget ();
    ASSERT_EQ (tlsStream.get (), 't');
    tlsStream.close ();
}

/**
 * @brief Test putback method.
 */
TEST_F (TlsSocketStream, putback)
{
    Tls::Stream tlsStream;
    tlsStream.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port});
    tlsStream.write ("test", 4);
    tlsStream.flush ();
    ASSERT_EQ (tlsStream.get (), 't');
    tlsStream.putback ('s');
    ASSERT_EQ (tlsStream.get (), 's');
    ASSERT_EQ (tlsStream.get (), 'e');
    tlsStream.putback ('t');
    ASSERT_EQ (tlsStream.get (), 't');
    ASSERT_EQ (tlsStream.get (), 's');
    tlsStream.putback ('e');
    ASSERT_EQ (tlsStream.get (), 'e');
    ASSERT_EQ (tlsStream.get (), 't');
    tlsStream.close ();
}

/**
 * @brief Test getline method.
 */
TEST_F (TlsSocketStream, getline)
{
    Tls::Stream tlsStream;
    tlsStream.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port});
    tlsStream.write ("test\n", 5);
    tlsStream.flush ();
    std::array <char, 32> test = {};
    tlsStream.getline (test.data (), test.size (), '\n');
    ASSERT_STREQ (test.data (), "test");
    tlsStream.close ();
}

/**
 * @brief Test ignore method.
 */
TEST_F (TlsSocketStream, ignore)
{
    Tls::Stream tlsStream;
    tlsStream.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port});
    tlsStream.write ("test\n", 5);
    tlsStream.flush ();
    tlsStream.ignore (std::numeric_limits <std::streamsize>::max (), 'e');
    ASSERT_EQ (tlsStream.get (), 's');
    ASSERT_EQ (tlsStream.get (), 't');
    tlsStream.close ();
}

/**
 * @brief Test read method.
 */
TEST_F (TlsSocketStream, read)
{
    Tls::Stream tlsStream;
    tlsStream.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port});
    tlsStream.write ("test", 4);
    tlsStream.flush ();
    std::array <char, 32> test = {};
    tlsStream.read (test.data (), 4);
    ASSERT_STREQ (test.data (), "test");
    tlsStream.close ();
}

/**
 * @brief Test readsome method.
 */
/*TEST_F (TlsSocketStream, readsome)
{
    Tls::Stream tlsStream;
    tlsStream.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port});
    tlsStream.write ("test", 4);
    tlsStream.flush ();
    std::array <char, 32> test = {};
    ASSERT_EQ (tlsStream.readsome (test.data (), test.size ()), 4);
    ASSERT_STREQ (test.data (), "test");
    tlsStream.close ();
}*/

/**
 * @brief Test gcount method.
 */
TEST_F (TlsSocketStream, gcount)
{
    Tls::Stream tlsStream;
    tlsStream.connectEncrypted ({Tls::Resolver::resolveHost (_host), _port});
    tlsStream.write ("test", 4);
    tlsStream.flush ();
    std::array <char, 32> test = {};
    tlsStream.read (test.data (), 4);
    ASSERT_EQ (tlsStream.gcount (), 4);
    tlsStream.close ();
}

/**
 * @brief Test sync method.
 */
/*TEST_F (TlsSocketStream, sync)
{
}*/

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    join::crypto::initializeOpenSSL ();
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
