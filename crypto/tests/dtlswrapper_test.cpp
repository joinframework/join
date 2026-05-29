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
#include <join/tlswrapper.hpp>
#include <join/reactor.hpp>
#include <join/socket.hpp>

// Libraries.
#include <gtest/gtest.h>

// C++.
#include <fstream>

using join::Errc;
using join::IpAddress;
using join::ReactorThread;
using join::EventHandler;
using join::Udp;
using join::TlsContext;
using join::TlsWrapper;

/**
 * @brief Class used to test the datagram TLS wrapper API.
 */
class DtlsSocket : public EventHandler, public ::testing::Test
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

        mkdir (_certPath.c_str (), 0777);
        std::ofstream certFile (_certFile);
        if (certFile.is_open ())
        {
            certFile << "-----BEGIN CERTIFICATE-----" << std::endl;
            certFile << "MIIDgDCCAyagAwIBAgIUR3ZIuKMt0BdaOZQnPwhSMR9qzfgwCgYIKoZIzj0EAwIw" << std::endl;
            certFile << "gcQxCzAJBgNVBAYTAkZSMRIwEAYDVQQIDAlPY2NpdGFuaWUxEDAOBgNVBAcMB0Nh" << std::endl;
            certFile << "c3RyZXMxFzAVBgNVBAoMDkpvaW4gRnJhbWV3b3JrMS0wKwYDVQQLDCRKb2luIEZy" << std::endl;
            certFile << "YW1ld29yayBDZXJ0aWZpY2F0ZSBBdXRob3JpdHkxHTAbBgNVBAMMFGNhLmpvaW5m" << std::endl;
            certFile << "cmFtZXdvcmsubmV0MSgwJgYJKoZIhvcNAQkBFhlzdXBwb3J0QGpvaW5mcmFtZXdv" << std::endl;
            certFile << "cmsubmV0MB4XDTIyMDcwNzEyMTIxMFoXDTMyMDcwNDEyMTIxMFowgagxCzAJBgNV" << std::endl;
            certFile << "BAYTAkZSMRIwEAYDVQQIDAlPY2NpdGFuaWUxEDAOBgNVBAcMB0Nhc3RyZXMxFzAV" << std::endl;
            certFile << "BgNVBAoMDkpvaW4gRnJhbWV3b3JrMRswGQYDVQQLDBJKb2luIEZyYW1ld29yayBE" << std::endl;
            certFile << "ZXYxEzARBgNVBAMMCmxvY2FsaG9zdC4xKDAmBgkqhkiG9w0BCQEWGXN1cHBvcnRA" << std::endl;
            certFile << "am9pbmZyYW1ld29yay5uZXQwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIB" << std::endl;
            certFile << "AQDSNtw5zEoJFPf6Rl0Y1n8BQfE0YTPCELvFAeioUfj8CAnUleHL9pwAEFg6kgoG" << std::endl;
            certFile << "hvwto5/yWGPUqNNfe3xbFTJcHgMhgtjqy5H6sYDkTi3kYIIMBfTHr8NI7HWE8Nz1" << std::endl;
            certFile << "qU1snjtERnkoLilIZf/2BojNVMtHC1H316WbMicXS0v7HQo3lv6PYSana9Q9ow9O" << std::endl;
            certFile << "2/FiW5qq1eOhI1ZedRanX+bl0jHWCd3WsI87+5bTaQrfetdHTOmav6O17Iq9FiTh" << std::endl;
            certFile << "Sg9fbM3s2Hw15kI+mws029dhcwXs5sYY+NgtrQwjR5qH+54BdUaPwQfl/KyulfEl" << std::endl;
            certFile << "TJykJ+3w6MorxUr55F68uBNbAgMBAAGjRTBDMAsGA1UdDwQEAwIF4DAdBgNVHSUE" << std::endl;
            certFile << "FjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwFQYDVR0RBA4wDIIKbG9jYWxob3N0LjAK" << std::endl;
            certFile << "BggqhkjOPQQDAgNIADBFAiA120ufIbhcw7BJQ1L6WudDdW2mHrVXvdgeOzVGgz1d" << std::endl;
            certFile << "iAIhAMm/sWI3yzb2IMPffxWKYusWEQE2hZvs24ESSC/ZZ0s+" << std::endl;
            certFile << "-----END CERTIFICATE-----" << std::endl;
            certFile.close ();
        }

        [[maybe_unused]] int result;
        result = std::system (std::string ("/usr/bin/c_rehash " + _certPath).c_str ());

        std::ofstream keyFile (_key);
        if (keyFile.is_open ())
        {
            keyFile << "-----BEGIN RSA PRIVATE KEY-----" << std::endl;
            keyFile << "MIIEowIBAAKCAQEA0jbcOcxKCRT3+kZdGNZ/AUHxNGEzwhC7xQHoqFH4/AgJ1JXh" << std::endl;
            keyFile << "y/acABBYOpIKBob8LaOf8lhj1KjTX3t8WxUyXB4DIYLY6suR+rGA5E4t5GCCDAX0" << std::endl;
            keyFile << "x6/DSOx1hPDc9alNbJ47REZ5KC4pSGX/9gaIzVTLRwtR99elmzInF0tL+x0KN5b+" << std::endl;
            keyFile << "j2Emp2vUPaMPTtvxYluaqtXjoSNWXnUWp1/m5dIx1gnd1rCPO/uW02kK33rXR0zp" << std::endl;
            keyFile << "mr+jteyKvRYk4UoPX2zN7Nh8NeZCPpsLNNvXYXMF7ObGGPjYLa0MI0eah/ueAXVG" << std::endl;
            keyFile << "j8EH5fysrpXxJUycpCft8OjKK8VK+eRevLgTWwIDAQABAoIBAAzdlK7o5OMXaHHl" << std::endl;
            keyFile << "2o7Jme5Oxd9pz4wiEAvnqQCcO7vZFhjvr2kXR8btOSkkhP6PRmHYsNJZPIroZj9i" << std::endl;
            keyFile << "xGKisnlW0OQ9KN995ApO0M+oRUDD81GfD7Mk+7O73Rls0GksmnN6X7A3C/U8lgQ7" << std::endl;
            keyFile << "UeYR0k+Wz/YiKDsd9KHB+QiA8D6HFQ9I8Y2P97KOcYnxXZfSwNm+ENNU3wShZOl2" << std::endl;
            keyFile << "ZYJJ4DE+5m2SwZ6g8b5Zre4cDbOduwuz/jXzjy2tAZBlTS4DVpYlhd14z+ssUWiu" << std::endl;
            keyFile << "AdS/nqSF7Obj0TRhoGNfrkisFzV4itavQ5DKGj/6hjueIJVLteUOzcCeg26YosNy" << std::endl;
            keyFile << "QzZSjOECgYEA7y3InEoh93/4HZCZmdwN8KfZtqirX0t966FntgAT8RkIs+KvNS8B" << std::endl;
            keyFile << "m3RfNLa/EuDt5zTmHRGx+oeN+17i9QQjKWcR0NnJ6aSZbvJByj3yKxLF9XVllzp/" << std::endl;
            keyFile << "vHSSyB264RoKIrWmFN6cCO4u4h9ZPY75pASWBCDMdnGK8axAcqAnlqsCgYEA4P+Y" << std::endl;
            keyFile << "FF9RW4rhrVU4dpXSfcr6vOwqfp9F9vhTVL0JS/SLOFoJNNpS9Rnq3pVLEuKyCphd" << std::endl;
            keyFile << "3nk9VFfoRygmMaGBvwGaXZPPvosoaIUgOdTt7KIfSHPichBEVxRuWCrtTGGkG0ok" << std::endl;
            keyFile << "s/RPHhvxZE267vsVj1PktK8Yr5Ba0AL2ycztNhECgYB5OAwHYe8LIBlg6otelk+e" << std::endl;
            keyFile << "W4OU9rE8L+eWx4vniuyQce6eNNI1syguYHFsJv56E/OfDYlezDwWzCLidnmyUjF7" << std::endl;
            keyFile << "51f5MJgLyTdWKoO7e1/EAtS/jYs6dRSOL8rAj4jKU0c1xjhxNU2BnS23vsmc0Fyn" << std::endl;
            keyFile << "iwd4+iKGGQ+hYnqbXZ4S1wKBgD/3an0gPDkSWua0e8D7B0TMGEztt4cYMQPtxYMp" << std::endl;
            keyFile << "2yLE+2+h6UwlZcBZBfUR7K4J1SQ9/THqtgzskRTpzTH/AKwVAJXqF/3MAkj00Byg" << std::endl;
            keyFile << "9KN50/r9NzvGdCdtn5FhYuV8PPOlOJoQsw2UVCR4FNUsfQyqhTL5NMN0/tx0e0UU" << std::endl;
            keyFile << "BbyBAoGBANu5ifByauVELH8UEl5rXRu1S9iAVV+Bc5jboXwc4VxJtEyomGJ7+YdL" << std::endl;
            keyFile << "5c9LFV+STUp7CE12uSXQZTQM0tEjPinLntRinNzu9tIHR1vy7FZHEwMFIgB4VTY7" << std::endl;
            keyFile << "ALRYv1/QpTuywpNUFRS15JkfGNf5JIkrUEWLgkX3OVCBsRGHUugy" << std::endl;
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
        unlink (_certFile.c_str ());
        rmdir (_certPath.c_str ());
        unlink (_key.c_str ());
        unlink (_invalidKey.c_str ());
    }

protected:
    /**
     * @brief Sets up the test fixture.
     */
    void SetUp () override
    {
        ASSERT_EQ (_tlsContext.setCertificate (_certFile, _key), 0);
        ASSERT_EQ (_tlsContext.setCipher (join::defaultCipher), 0);
        ASSERT_EQ (_tlsContext.setCipher_1_3 (join::defaultCipher_1_3), 0);

        ASSERT_EQ (_socket.bind ({IpAddress::ipv6Wildcard, _port}), 0) << join::lastError.message ();
        ASSERT_EQ (ReactorThread::reactor ().addHandler (_socket.handle (), this), 0);
    }

    /**
     * @brief Tears down the test fixture.
     */
    void TearDown () override
    {
        ASSERT_EQ (ReactorThread::reactor ().delHandler (_socket.handle ()), 0) << join::lastError.message ();
        _socket.close ();
    }

    /**
     * @brief method called when data are ready to be read on handle.
     * @param fd file descriptor.
     */
    virtual void onReadable ([[maybe_unused]] int fd) override
    {
        _socket.waitHandshake (_timeout);

        char buffer[65536];
        Udp::Endpoint from;
        int nread = _socket.readFrom (buffer, sizeof (buffer), &from);
        if (nread > 0)
        {
            _socket.writeTo (buffer, nread, from);
        }

        _socket.waitShutdown (_timeout);
    }

    /// TLS context serveur.
    TlsContext _tlsContext{TlsContext::Role::DtlsServer};

    /// socket.
    TlsWrapper<Udp::Socket> _socket{_tlsContext, Udp::Socket::Blocking};

    /// host.
    static const std::string _hostv4;
    static const std::string _hostv6;

    /// port.
    static const uint16_t _port;

    /// timeout.
    static const int _timeout;

    /// root certificate.
    static const std::string _rootcert;

    /// certificate path.
    static const std::string _certPath;

    /// certificate file.
    static const std::string _certFile;

    /// private key.
    static const std::string _key;

    /// invalid private key.
    static const std::string _invalidKey;
};

const std::string DtlsSocket::_hostv4 = "127.0.0.1";
const std::string DtlsSocket::_hostv6 = "::1";
const uint16_t DtlsSocket::_port = 5000;
const int DtlsSocket::_timeout = 1000;
const std::string DtlsSocket::_rootcert = "/tmp/tlssocket_test_root.cert";
const std::string DtlsSocket::_certPath = "/tmp/certs";
const std::string DtlsSocket::_certFile = _certPath + "/tlssocket_test.cert";
const std::string DtlsSocket::_key = "/tmp/tlssocket_test.key";
const std::string DtlsSocket::_invalidKey = "/tmp/tlssocket_test_invalid.key";

/**
 * @brief Test move.
 */
TEST_F (DtlsSocket, move)
{
    TlsContext ctx (TlsContext::Role::DtlsClient);
    TlsWrapper<Udp::Socket> dtls1 (ctx, Udp::Socket::Blocking), dtls2 (ctx);

    ASSERT_EQ (dtls1.bind ({_hostv4, uint16_t (_port + 1)}), 0) << join::lastError.message ();
    ASSERT_EQ (dtls1.connect ({_hostv4, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (dtls1.connected ());
    ASSERT_EQ (dtls1.handshake (), 0) << join::lastError.message ();
    ASSERT_TRUE (dtls1.encrypted ());
    dtls2 = std::move (dtls1);
    ASSERT_TRUE (dtls2.connected ());
    ASSERT_TRUE (dtls2.encrypted ());
    ASSERT_EQ (dtls2.shutdown (), 0) << join::lastError.message ();
    ASSERT_EQ (dtls2.disconnect (), 0) << join::lastError.message ();
    dtls2.close ();
}

/**
 * @brief Test open method.
 */
TEST_F (DtlsSocket, open)
{
    TlsContext ctx (TlsContext::Role::DtlsClient);
    TlsWrapper<Udp::Socket> dtls (ctx);

    ASSERT_EQ (dtls.open (Udp::v4 ()), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.open (Udp::v4 ()), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    dtls.close ();

    ASSERT_EQ (dtls.open (Udp::v6 ()), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.open (Udp::v6 ()), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    dtls.close ();
}

/**
 * @brief Test close method.
 */
TEST_F (DtlsSocket, close)
{
    TlsContext ctx (TlsContext::Role::DtlsClient);
    TlsWrapper<Udp::Socket> dtls (ctx, Udp::Socket::Blocking);

    ASSERT_FALSE (dtls.opened ());
    ASSERT_EQ (dtls.connect ({_hostv4, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (dtls.opened ());
    ASSERT_EQ (dtls.disconnect (), 0) << join::lastError.message ();
    ASSERT_TRUE (dtls.opened ());
    dtls.close ();
    ASSERT_FALSE (dtls.opened ());
}

/**
 * @brief Test bind method.
 */
TEST_F (DtlsSocket, bind)
{
    TlsContext ctx (TlsContext::Role::DtlsClient);
    TlsWrapper<Udp::Socket> dtls (ctx, Udp::Socket::Blocking);

    ASSERT_EQ (dtls.connect ({_hostv4, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.bind ({_hostv4, uint16_t (_port + 1)}), -1);
    ASSERT_EQ (dtls.disconnect (), 0) << join::lastError.message ();
    dtls.close ();

    ASSERT_EQ (dtls.bind ({_hostv4, uint16_t (_port + 1)}), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.connect ({_hostv4, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.disconnect (), 0) << join::lastError.message ();
    dtls.close ();
}

/**
 * @brief Test bindToDevice method.
 */
TEST_F (DtlsSocket, bindToDevice)
{
    TlsContext ctx (TlsContext::Role::DtlsClient);
    TlsWrapper<Udp::Socket> dtls (ctx, Udp::Socket::Blocking);

    ASSERT_EQ (dtls.bindToDevice ("lo"), -1);

    ASSERT_EQ (dtls.connect ({_hostv4, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.bindToDevice ("lo"), -1);
    ASSERT_EQ (dtls.disconnect (), 0) << join::lastError.message ();
    dtls.close ();

    ASSERT_EQ (dtls.open (Udp::v6 ()), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.bindToDevice ("lo"), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.connect ({_hostv6, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.disconnect (), 0) << join::lastError.message ();
    dtls.close ();

    ASSERT_EQ (dtls.bindToDevice ("foo"), -1);
}

/**
 * @brief Test connect method.
 */
TEST_F (DtlsSocket, connect)
{
    TlsContext ctx (TlsContext::Role::DtlsClient);
    TlsWrapper<Udp::Socket> dtls (ctx, Udp::Socket::Blocking);

    ASSERT_EQ (dtls.connect ({"255.255.255.255", _port}), -1);

    ASSERT_EQ (dtls.connect ({_hostv4, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.connect ({_hostv4, _port}), -1);
    ASSERT_EQ (join::lastError, Errc::InUse);
    ASSERT_EQ (dtls.disconnect (), 0) << join::lastError.message ();
    dtls.close ();
}

/**
 * @brief Test waitConnected method.
 */
// TEST_F (DtlsSocket, waitConnected)
// {
//     TlsContext ctx (TlsContext::Role::DtlsClient);
//     TlsWrapper<Udp::Socket> dtls (ctx);

//     ASSERT_FALSE (dtls.waitConnected (_timeout));
//     if (dtls.connect ({_hostv4, _port}) == -1)
//     {
//         ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
//         ASSERT_TRUE (dtls.connecting ());
//     }
//     ASSERT_TRUE (dtls.waitConnected (_timeout)) << join::lastError.message ();
//     if (dtls.disconnect () == -1)
//     {
//         ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
//     }
//     ASSERT_TRUE (dtls.waitDisconnected (_timeout)) << join::lastError.message ();
//     dtls.close ();
// }

/**
 * @brief Test handshake method.
 */
TEST_F (DtlsSocket, handshake)
{
    TlsContext ctx (TlsContext::Role::DtlsClient);
    TlsWrapper<Udp::Socket> dtls (ctx, Udp::Socket::Blocking);

    ASSERT_EQ (dtls.handshake (), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (dtls.open (Udp::v4 ()), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.handshake (), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (dtls.bind ({_hostv4, uint16_t (_port + 1)}), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.handshake (), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (dtls.connect ({_hostv4, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.handshake (), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.handshake (), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.shutdown (), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.shutdown (), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.disconnect (), 0) << join::lastError.message ();
    dtls.close ();
}

/**
 * @brief Test waitHandshake method.
 */
TEST_F (DtlsSocket, waitHandshake)
{
    TlsContext ctx (TlsContext::Role::DtlsClient);
    TlsWrapper<Udp::Socket> dtls (ctx, Udp::Socket::NonBlocking);

    ASSERT_EQ (dtls.open (Udp::v6 ()), 0) << join::lastError.message ();
    ASSERT_FALSE (dtls.waitHandshake (_timeout));
    if (dtls.connect ({_hostv6, _port}) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (dtls.waitConnected (_timeout)) << join::lastError.message ();
    if (dtls.handshake () == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (dtls.waitHandshake (_timeout)) << join::lastError.message ();
    ASSERT_TRUE (dtls.waitHandshake (_timeout)) << join::lastError.message ();
    if (dtls.shutdown () == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (dtls.waitShutdown (_timeout)) << join::lastError.message ();
    if (dtls.disconnect () == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (dtls.waitDisconnected (_timeout)) << join::lastError.message ();
    dtls.close ();
}

/**
 * @brief Test disconnect method.
 */
TEST_F (DtlsSocket, disconnect)
{
    TlsContext ctx (TlsContext::Role::DtlsClient);
    TlsWrapper<Udp::Socket> dtls (ctx, Udp::Socket::Blocking);

    ASSERT_FALSE (dtls.connected ());
    ASSERT_EQ (dtls.connect ({_hostv4, _port}), 0) << join::lastError.message ();
    ASSERT_TRUE (dtls.connected ());
    ASSERT_EQ (dtls.disconnect (), 0) << join::lastError.message ();
    ASSERT_FALSE (dtls.connected ());
    dtls.close ();
    ASSERT_FALSE (dtls.connected ());
}

/**
 * @brief Test waitDisconnected method.
 */
TEST_F (DtlsSocket, waitDisconnected)
{
    TlsContext ctx (TlsContext::Role::DtlsClient);
    TlsWrapper<Udp::Socket> dtls (ctx, Udp::Socket::NonBlocking);

    ASSERT_TRUE (dtls.waitDisconnected (_timeout)) << join::lastError.message ();
    if (dtls.connect ({_hostv4, _port}) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (dtls.waitConnected (_timeout)) << join::lastError.message ();
    ASSERT_FALSE (dtls.waitDisconnected (_timeout));
    if (dtls.disconnect () == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (dtls.waitDisconnected (_timeout)) << join::lastError.message ();
    dtls.close ();
}

/**
 * @brief Test waitReadyRead method.
 */
TEST_F (DtlsSocket, waitReadyRead)
{
    TlsContext ctx (TlsContext::Role::DtlsClient);
    TlsWrapper<Udp::Socket> dtls (ctx, Udp::Socket::NonBlocking);
    char data[] = {0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_FALSE (dtls.waitReadyRead (_timeout));
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (dtls.bind ({_hostv4, uint16_t (_port + 1)}), 0) << join::lastError.message ();
    if (dtls.connect ({_hostv4, _port}) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (dtls.waitConnected (_timeout)) << join::lastError.message ();
    if (dtls.handshake () == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (dtls.waitHandshake (_timeout)) << join::lastError.message ();
    ASSERT_TRUE (dtls.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (dtls.writeExactly (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (dtls.waitReadyRead (_timeout)) << join::lastError.message ();
    if (dtls.shutdown () == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (dtls.waitShutdown (_timeout)) << join::lastError.message ();
    if (dtls.disconnect () == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (dtls.waitDisconnected (_timeout)) << join::lastError.message ();
    dtls.close ();
}

/**
 * @brief Test read method.
 */
TEST_F (DtlsSocket, read)
{
    TlsContext ctx (TlsContext::Role::DtlsClient);
    TlsWrapper<Udp::Socket> dtls (ctx, Udp::Socket::Blocking);
    char data[] = {0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (dtls.read (data, sizeof (data)), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (dtls.bind ({_hostv4, uint16_t (_port + 1)}), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.connect ({_hostv4, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.handshake (), 0) << join::lastError.message ();
    ASSERT_TRUE (dtls.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (dtls.writeExactly (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (dtls.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_GT (dtls.read (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.shutdown (), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.disconnect (), 0) << join::lastError.message ();
    dtls.close ();
}

TEST_F (DtlsSocket, readExactly)
{
    TlsContext ctx (TlsContext::Role::DtlsClient);
    TlsWrapper<Udp::Socket> dtls (ctx, Udp::Socket::Blocking);
    char data[] = {0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (dtls.readExactly (data, sizeof (data)), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (dtls.bind ({_hostv4, uint16_t (_port + 1)}), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.connect ({_hostv4, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.handshake (), 0) << join::lastError.message ();
    ASSERT_TRUE (dtls.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (dtls.writeExactly (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (dtls.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_EQ (dtls.readExactly (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.shutdown (), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.disconnect (), 0) << join::lastError.message ();
    dtls.close ();
}

/**
 * @brief Test waitReadyWrite method.
 */
TEST_F (DtlsSocket, waitReadyWrite)
{
    TlsContext ctx (TlsContext::Role::DtlsClient);
    TlsWrapper<Udp::Socket> dtls (ctx, Udp::Socket::NonBlocking);

    ASSERT_FALSE (dtls.waitReadyWrite (_timeout));
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (dtls.bind ({_hostv4, uint16_t (_port + 1)}), 0) << join::lastError.message ();
    if (dtls.connect ({_hostv4, _port}) == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (dtls.waitConnected (_timeout)) << join::lastError.message ();
    if (dtls.handshake () == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (dtls.waitHandshake (_timeout)) << join::lastError.message ();
    ASSERT_TRUE (dtls.waitReadyWrite (_timeout)) << join::lastError.message ();
    if (dtls.shutdown () == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (dtls.waitShutdown (_timeout)) << join::lastError.message ();
    if (dtls.disconnect () == -1)
    {
        ASSERT_EQ (join::lastError, Errc::TemporaryError) << join::lastError.message ();
    }
    ASSERT_TRUE (dtls.waitDisconnected (_timeout)) << join::lastError.message ();
    dtls.close ();
}

/**
 * @brief Test write method.
 */
TEST_F (DtlsSocket, write)
{
    TlsContext ctx (TlsContext::Role::DtlsClient);
    TlsWrapper<Udp::Socket> dtls (ctx, Udp::Socket::Blocking);
    char data[] = {0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (dtls.write (data, sizeof (data)), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (dtls.bind ({_hostv4, uint16_t (_port + 1)}), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.connect ({_hostv4, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.handshake (), 0) << join::lastError.message ();
    ASSERT_TRUE (dtls.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_GT (dtls.write (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (dtls.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_EQ (dtls.shutdown (), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.disconnect (), 0) << join::lastError.message ();
    dtls.close ();
}

TEST_F (DtlsSocket, writeExactly)
{
    TlsContext ctx (TlsContext::Role::DtlsClient);
    TlsWrapper<Udp::Socket> dtls (ctx, Udp::Socket::Blocking);
    char data[] = {0x00, 0x65, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x5B, 0x22, 0x6B, 0x6F, 0x22, 0x5D};

    ASSERT_EQ (dtls.writeExactly (data, sizeof (data)), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);
    ASSERT_EQ (dtls.connect ({_hostv4, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.handshake (), 0) << join::lastError.message ();
    ASSERT_TRUE (dtls.waitReadyWrite (_timeout)) << join::lastError.message ();
    ASSERT_EQ (dtls.writeExactly (data, sizeof (data)), 0) << join::lastError.message ();
    ASSERT_TRUE (dtls.waitReadyRead (_timeout)) << join::lastError.message ();
    ASSERT_EQ (dtls.shutdown (), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.disconnect (), 0) << join::lastError.message ();
    dtls.close ();
}

/**
 * @brief Test setMode method.
 */
TEST_F (DtlsSocket, setMode)
{
    TlsContext ctx (TlsContext::Role::DtlsClient);
    TlsWrapper<Udp::Socket> dtls (ctx, Udp::Socket::NonBlocking);

    ASSERT_EQ (dtls.open (), 0) << join::lastError.message ();

    int flags = ::fcntl (dtls.handle (), F_GETFL, 0);
    ASSERT_TRUE (flags & O_NONBLOCK);

    dtls.setMode (Udp::Socket::Blocking);
    flags = ::fcntl (dtls.handle (), F_GETFL, 0);
    ASSERT_FALSE (flags & O_NONBLOCK);

    dtls.setMode (Udp::Socket::NonBlocking);
    flags = ::fcntl (dtls.handle (), F_GETFL, 0);
    ASSERT_TRUE (flags & O_NONBLOCK);

    dtls.close ();
}

/**
 * @brief Test setOption method.
 */
TEST_F (DtlsSocket, setOption)
{
    TlsContext ctx (TlsContext::Role::DtlsClient);
    TlsWrapper<Udp::Socket> dtls (ctx, Udp::Socket::NonBlocking);

    ASSERT_EQ (dtls.setOption (Udp::Socket::RcvBuffer, 1500), -1);
    ASSERT_EQ (join::lastError, Errc::OperationFailed);

    ASSERT_EQ (dtls.open (), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.setOption (Udp::Socket::NoDelay, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (dtls.setOption (Udp::Socket::KeepAlive, 1), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.setOption (Udp::Socket::KeepIdle, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (dtls.setOption (Udp::Socket::KeepIntvl, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (dtls.setOption (Udp::Socket::KeepCount, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (dtls.setOption (Udp::Socket::SndBuffer, 1500), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.setOption (Udp::Socket::RcvBuffer, 1500), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.setOption (Udp::Socket::TimeStamp, 1), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.setOption (Udp::Socket::ReuseAddr, 1), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.setOption (Udp::Socket::ReusePort, 1), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.setOption (Udp::Socket::Broadcast, 1), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.setOption (Udp::Socket::Ttl, 1), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.setOption (Udp::Socket::MulticastLoop, 1), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.setOption (Udp::Socket::MulticastTtl, 1), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.setOption (Udp::Socket::PathMtuDiscover, 1), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.setOption (Udp::Socket::RcvError, 1), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.setOption (Udp::Socket::AuxData, 1), -1);
    ASSERT_EQ (join::lastError, std::errc::no_protocol_option);
    dtls.close ();

    ASSERT_EQ (dtls.open (Udp::v6 ()), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.setOption (Udp::Socket::NoDelay, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (dtls.setOption (Udp::Socket::KeepAlive, 1), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.setOption (Udp::Socket::KeepIdle, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (dtls.setOption (Udp::Socket::KeepIntvl, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (dtls.setOption (Udp::Socket::KeepCount, 1), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (dtls.setOption (Udp::Socket::SndBuffer, 1500), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.setOption (Udp::Socket::RcvBuffer, 1500), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.setOption (Udp::Socket::TimeStamp, 1), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.setOption (Udp::Socket::ReuseAddr, 1), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.setOption (Udp::Socket::ReusePort, 1), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.setOption (Udp::Socket::Broadcast, 1), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.setOption (Udp::Socket::Ttl, 1), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.setOption (Udp::Socket::MulticastLoop, 1), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.setOption (Udp::Socket::MulticastTtl, 1), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.setOption (Udp::Socket::PathMtuDiscover, 1), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.setOption (Udp::Socket::RcvError, 1), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.setOption (Udp::Socket::AuxData, 1), -1);
    ASSERT_EQ (join::lastError, std::errc::no_protocol_option);
    dtls.close ();
}

/**
 * @brief Test localEndpoint method.
 */
TEST_F (DtlsSocket, localEndpoint)
{
    TlsContext ctx (TlsContext::Role::DtlsClient);
    TlsWrapper<Udp::Socket> dtls (ctx, Udp::Socket::Blocking);

    ASSERT_EQ (dtls.localEndpoint (), Udp::Endpoint{});
    ASSERT_EQ (dtls.bind ({_hostv4, uint16_t (_port + 1)}), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.connect ({_hostv4, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.localEndpoint (), Udp::Endpoint (_hostv4, uint16_t (_port + 1))) << join::lastError.message ();
    dtls.close ();
}

/**
 * @brief Test remoteEndpoint method.
 */
TEST_F (DtlsSocket, remoteEndpoint)
{
    TlsContext ctx (TlsContext::Role::DtlsClient);
    TlsWrapper<Udp::Socket> dtls (ctx, Udp::Socket::Blocking);

    ASSERT_EQ (dtls.remoteEndpoint (), Udp::Endpoint{});
    ASSERT_EQ (dtls.bind ({_hostv4, uint16_t (_port + 1)}), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.connect ({_hostv4, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.remoteEndpoint (), Udp::Endpoint (_hostv4, _port)) << join::lastError.message ();
    dtls.close ();
}

/**
 * @brief Test opened method.
 */
TEST_F (DtlsSocket, opened)
{
    TlsContext ctx (TlsContext::Role::DtlsClient);
    TlsWrapper<Udp::Socket> dtls (ctx, Udp::Socket::Blocking);

    ASSERT_FALSE (dtls.opened ());
    ASSERT_EQ (dtls.open (IpAddress (_hostv4).family ()), 0) << join::lastError.message ();
    ASSERT_TRUE (dtls.opened ());
    ASSERT_EQ (dtls.connect ({_hostv4, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.handshake (), 0) << join::lastError.message ();
    ASSERT_TRUE (dtls.opened ());
    ASSERT_EQ (dtls.shutdown (), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.disconnect (), 0) << join::lastError.message ();
    ASSERT_TRUE (dtls.opened ());
    dtls.close ();
    ASSERT_FALSE (dtls.opened ());
}

/**
 * @brief Test connected method.
 */
TEST_F (DtlsSocket, connected)
{
    TlsContext ctx (TlsContext::Role::DtlsClient);
    TlsWrapper<Udp::Socket> dtls (ctx, Udp::Socket::Blocking);

    ASSERT_FALSE (dtls.connected ());
    ASSERT_EQ (dtls.open (IpAddress (_hostv4).family ()), 0) << join::lastError.message ();
    ASSERT_FALSE (dtls.connected ());
    ASSERT_EQ (dtls.connect ({_hostv4, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.handshake (), 0) << join::lastError.message ();
    ASSERT_TRUE (dtls.connected ());
    ASSERT_EQ (dtls.shutdown (), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.disconnect (), 0) << join::lastError.message ();
    ASSERT_FALSE (dtls.connected ());
    dtls.close ();
    ASSERT_FALSE (dtls.connected ());
}

/**
 * @brief Test encrypted method.
 */
TEST_F (DtlsSocket, encrypted)
{
    TlsContext ctx (TlsContext::Role::DtlsClient);
    TlsWrapper<Udp::Socket> dtls (ctx, Udp::Socket::Blocking);

    ASSERT_FALSE (dtls.encrypted ());
    ASSERT_EQ (dtls.open (IpAddress (_hostv4).family ()), 0) << join::lastError.message ();
    ASSERT_FALSE (dtls.encrypted ());
    ASSERT_EQ (dtls.connect ({_hostv4, _port}), 0) << join::lastError.message ();
    ASSERT_FALSE (dtls.encrypted ());
    ASSERT_EQ (dtls.handshake (), 0) << join::lastError.message ();
    ASSERT_TRUE (dtls.encrypted ());
    ASSERT_EQ (dtls.shutdown (), 0) << join::lastError.message ();
    ASSERT_FALSE (dtls.encrypted ());
    ASSERT_EQ (dtls.disconnect (), 0) << join::lastError.message ();
    ASSERT_FALSE (dtls.encrypted ());
    dtls.close ();
    ASSERT_FALSE (dtls.encrypted ());
}

/**
 * @brief Test family method.
 */
TEST_F (DtlsSocket, family)
{
    TlsContext ctx (TlsContext::Role::DtlsClient);
    TlsWrapper<Udp::Socket> dtls (ctx, Udp::Socket::Blocking);

    ASSERT_EQ (dtls.family (), AF_INET);

    ASSERT_EQ (dtls.bind (IpAddress (AF_INET6)), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.family (), AF_INET6);
    dtls.close ();

    ASSERT_EQ (dtls.bind (IpAddress (AF_INET)), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.family (), AF_INET);
    dtls.close ();
}

/**
 * @brief Test type method.
 */
TEST_F (DtlsSocket, type)
{
    TlsContext ctx (TlsContext::Role::DtlsClient);
    TlsWrapper<Udp::Socket> dtls (ctx, Udp::Socket::NonBlocking);

    ASSERT_EQ (dtls.type (), SOCK_DGRAM);
}

/**
 * @brief Test protocol method.
 */
TEST_F (DtlsSocket, protocol)
{
    TlsContext ctx (TlsContext::Role::DtlsClient);
    TlsWrapper<Udp::Socket> dtls (ctx, Udp::Socket::NonBlocking);

    ASSERT_EQ (dtls.protocol (), IPPROTO_UDP);
}

/**
 * @brief Test handle method.
 */
TEST_F (DtlsSocket, handle)
{
    TlsContext ctx (TlsContext::Role::DtlsClient);
    TlsWrapper<Udp::Socket> dtls (ctx, Udp::Socket::Blocking);

    ASSERT_EQ (dtls.handle (), -1);
    ASSERT_EQ (dtls.open (IpAddress (_hostv4).family ()), 0) << join::lastError.message ();
    ASSERT_GT (dtls.handle (), -1);
    ASSERT_EQ (dtls.connect ({_hostv4, _port}), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.handshake (), 0) << join::lastError.message ();
    ASSERT_GT (dtls.handle (), -1);
    ASSERT_EQ (dtls.shutdown (), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.disconnect (), 0) << join::lastError.message ();
    ASSERT_GT (dtls.handle (), -1);
    dtls.close ();
    ASSERT_EQ (dtls.handle (), -1);
}

/**
 * @brief Test mtu method.
 */
TEST_F (DtlsSocket, mtu)
{
    TlsContext ctx (TlsContext::Role::DtlsClient);
    TlsWrapper<Udp::Socket> dtls (ctx, Udp::Socket::Blocking);

    ASSERT_EQ (dtls.mtu (), -1);
    ASSERT_EQ (dtls.connect ({"127.0.0.1", _port}), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.handshake (), 0) << join::lastError.message ();
    ASSERT_NE (dtls.mtu (), -1) << join::lastError.message ();
    ASSERT_EQ (dtls.shutdown (), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.disconnect (), 0) << join::lastError.message ();
    ASSERT_NE (dtls.mtu (), -1) << join::lastError.message ();
    dtls.close ();
    ASSERT_EQ (dtls.mtu (), -1);

    ASSERT_EQ (dtls.mtu (), -1);
    ASSERT_EQ (dtls.connect ({"::1", _port}), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.handshake (), 0) << join::lastError.message ();
    ASSERT_NE (dtls.mtu (), -1) << join::lastError.message ();
    ASSERT_EQ (dtls.shutdown (), 0) << join::lastError.message ();
    ASSERT_EQ (dtls.disconnect (), 0) << join::lastError.message ();
    ASSERT_NE (dtls.mtu (), -1) << join::lastError.message ();
    dtls.close ();
    ASSERT_EQ (dtls.mtu (), -1);
}

/**
 * @brief Test cert verification.
 */
TEST_F (DtlsSocket, verify)
{
    TlsContext ctx (TlsContext::Role::DtlsClient);
    Udp::Endpoint endpoint{_hostv4, _port};

    ctx.setVerify (false);
    TlsWrapper<Udp::Socket> dtls1 (ctx, Udp::Socket::Blocking);
    ASSERT_EQ (dtls1.connect (endpoint), 0) << join::lastError.message ();
    ASSERT_EQ (dtls1.handshake (), 0) << join::lastError.message ();
    ASSERT_EQ (dtls1.shutdown (), 0) << join::lastError.message ();
    ASSERT_EQ (dtls1.disconnect (), 0) << join::lastError.message ();
    dtls1.close ();

    ctx.setVerify (true, 0);
    TlsWrapper<Udp::Socket> dtls2 (ctx, Udp::Socket::Blocking);
    ASSERT_EQ (dtls2.connect (endpoint), 0) << join::lastError.message ();
    ASSERT_EQ (dtls2.handshake (), -1);
    ASSERT_EQ (dtls2.disconnect (), 0) << join::lastError.message ();
    dtls2.close ();

    ctx.setVerify (true, 1);
    TlsWrapper<Udp::Socket> dtls3 (ctx, Udp::Socket::Blocking);
    ASSERT_EQ (dtls3.connect (endpoint), 0) << join::lastError.message ();
    ASSERT_EQ (dtls3.handshake (), -1);
    ASSERT_EQ (dtls3.disconnect (), 0) << join::lastError.message ();
    dtls3.close ();

    endpoint.hostname ("localhost");

    ctx.setVerify (true, 0);
    TlsWrapper<Udp::Socket> dtls4 (ctx, Udp::Socket::Blocking);
    ASSERT_EQ (dtls4.connect (endpoint), 0) << join::lastError.message ();
    ASSERT_EQ (dtls4.handshake (), -1);
    ASSERT_EQ (dtls4.disconnect (), 0) << join::lastError.message ();
    dtls4.close ();

    ctx.setVerify (true, 1);
    TlsWrapper<Udp::Socket> dtls5 (ctx, Udp::Socket::Blocking);
    ASSERT_EQ (dtls5.connect (endpoint), 0) << join::lastError.message ();
    ASSERT_EQ (dtls5.handshake (), -1);
    ASSERT_EQ (dtls5.disconnect (), 0) << join::lastError.message ();
    dtls5.close ();

    ASSERT_EQ (ctx.setCaFile (_rootcert), 0) << join::lastError.message ();

    ctx.setVerify (true, 0);
    TlsWrapper<Udp::Socket> dtls6 (ctx, Udp::Socket::Blocking);
    ASSERT_EQ (dtls6.connect (endpoint), 0) << join::lastError.message ();
    ASSERT_EQ (dtls6.handshake (), -1);
    ASSERT_EQ (dtls6.disconnect (), 0) << join::lastError.message ();
    dtls6.close ();

    ctx.setVerify (true, 1);
    TlsWrapper<Udp::Socket> dtls7 (ctx, Udp::Socket::Blocking);
    ASSERT_EQ (dtls7.connect (endpoint), 0) << join::lastError.message ();
    ASSERT_EQ (dtls7.handshake (), 0) << join::lastError.message ();
    ASSERT_EQ (dtls7.shutdown (), 0) << join::lastError.message ();
    ASSERT_EQ (dtls7.disconnect (), 0) << join::lastError.message ();
    dtls7.close ();
}

/**
 * @brief Test is lower method.
 */
TEST_F (DtlsSocket, isLower)
{
    TlsContext ctx (TlsContext::Role::DtlsClient);
    TlsWrapper<Udp::Socket> dtls1 (ctx), dtls2 (ctx);

    ASSERT_EQ (dtls1.open (IpAddress (_hostv4).family ()), 0) << join::lastError.message ();
    ASSERT_EQ (dtls2.open (IpAddress (_hostv4).family ()), 0) << join::lastError.message ();
    if (dtls1.handle () < dtls2.handle ())
    {
        ASSERT_TRUE (dtls1 < dtls2);
    }
    else
    {
        ASSERT_TRUE (dtls2 < dtls1);
    }
    dtls1.close ();
    dtls2.close ();
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    join::initializeOpenSSL ();
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
