/**
 * MIT License
 *
 * Copyright (c) 2023 Mathieu Rabine
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
#include <join/smtpclient.hpp>
#include <join/acceptor.hpp>
#include <join/reactor.hpp>

// Libraries.
#include <gtest/gtest.h>

// C++.
#include <fstream>

using join::Errc;
using join::Resolver;
using join::Reactor;
using join::MailMessage;
using join::Smtp;
using join::Tls;

/**
 * @brief Class used to test the SMTP client.
 */
class SmtpClient : public Tls::Acceptor, public ::testing::Test
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
        rmdir  (_certPath.c_str ());
        unlink (_key.c_str ());
        unlink (_invalidKey.c_str ());
    }

protected:
    /**
     * @brief Sets up the test fixture.
     */
    void SetUp ()
    {
        ASSERT_EQ (this->setCertificate (_certFile, _key), 0) << join::lastError.message ();
        ASSERT_EQ (this->setCipher (join::_defaultCipher), 0) << join::lastError.message ();
    #if OPENSSL_VERSION_NUMBER >= 0x10101000L
        ASSERT_EQ (this->setCipher_1_3 (join::_defaultCipher_1_3), 0) << join::lastError.message ();
    #endif
        ASSERT_EQ (this->create ({Resolver::resolveHost (_host), _port}), 0) << join::lastError.message ();
        ASSERT_EQ (Reactor::instance ()->addHandler (this), 0) << join::lastError.message ();
    }

    /**
     * @brief Tears down the test fixture.
     */
    void TearDown ()
    {
        ASSERT_EQ (Reactor::instance ()->delHandler (this), 0) << join::lastError.message ();
        this->close ();
    }

    /**
     * @brief method called when data are ready to be read on handle.
     */
    virtual void onReceive () override
    {
        Tls::Stream stream = this->acceptStream ();
        if (stream.connected ())
        {
            std::string tmp;
            stream << "220 mail.foo.bar ESMTP Postfix\r\n";
            stream.flush ();
            join::getline (stream, tmp);
            stream << "250-mail.foo.bar\r\n";
            stream << "250-PIPELINING\r\n";
            stream << "250-SIZE 10485760\r\n";
            stream << "250-ETRN\r\n";
            stream << "250-STARTTLS\r\n";
            stream << "250-AUTH PLAIN\r\n";
            stream << "250-AUTH=PLAIN\r\n";
            stream << "250-ENHANCEDSTATUSCODES\r\n";
            stream << "250-8BITMIME\r\n";
            stream << "250-DSN\r\n";
            stream << "250 CHUNKING\r\n";
            stream.flush ();
            join::getline (stream, tmp);
            stream << "220 2.0.0 Ready to start TLS\r\n";
            stream.flush ();
            stream.startEncryption ();
            join::getline (stream, tmp);
            stream << "250-mail.foo.bar\r\n";
            stream << "250-PIPELINING\r\n";
            stream << "250-SIZE 10485760\r\n";
            stream << "250-ETRN\r\n";
            stream << "250-AUTH PLAIN\r\n";
            stream << "250-AUTH=PLAIN\r\n";
            stream << "250-ENHANCEDSTATUSCODES\r\n";
            stream << "250-8BITMIME\r\n";
            stream << "250-DSN\r\n";
            stream << "250 CHUNKING\r\n";
            stream.flush ();
            join::getline (stream, tmp);
            stream << "334 VXNlcm5hbWU6\r\n";
            stream.flush ();
            join::getline (stream, tmp);
            stream << "235 2.7.0 Authentication successful\r\n";
            stream.flush ();
            join::getline (stream, tmp);
            stream << "250 2.1.0 Ok\r\n";
            stream.flush ();
            join::getline (stream, tmp);
            stream << "250 2.1.5 Ok\r\n";
            stream.flush ();
            join::getline (stream, tmp);
            stream << "354 End data with <CR><LF>.<CR><LF>\r\n";
            stream.flush ();
            while (join::getline (stream, tmp) && tmp != ".") {};
            stream << "250 2.0.0 Ok: queued as 1A208D10002C\r\n";
            stream.flush ();
            join::getline (stream, tmp);
            stream << "221 2.0.0 Bye\r\n";
            stream.flush ();
            stream.disconnect ();
            stream.close ();
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

    /// certificate path.
    static const std::string _certPath;

    /// certificate file.
    static const std::string _certFile;

    /// private key.
    static const std::string _key;

    /// invalid private key.
    static const std::string _invalidKey;

    /// user.
    static const std::string _user;

    /// password.
    static const std::string _password;
};

const std::string SmtpClient::_host       = "localhost";
const uint16_t    SmtpClient::_port       = 5000;
const int         SmtpClient::_timeout    = 1000;
const std::string SmtpClient::_rootcert   = "/tmp/tlssocket_test_root.cert";
const std::string SmtpClient::_certPath   = "/tmp/certs";
const std::string SmtpClient::_certFile   = _certPath + "/tlssocket_test.cert";
const std::string SmtpClient::_key        = "/tmp/tlssocket_test.key";
const std::string SmtpClient::_invalidKey = "/tmp/tlssocket_test_invalid.key";
const std::string SmtpClient::_user       = "admin";
const std::string SmtpClient::_password   = "12345";

/**
 * @brief Test move.
 */
TEST_F (SmtpClient, move)
{
    Smtp::Client tmp (_host, _port);
    Smtp::Client client1 (std::move (tmp)); 
    ASSERT_EQ (client1.host (), _host);

    Smtp::Client  client2 ("localhost");
    ASSERT_EQ (client2.host (), "localhost");

    client2 = std::move (client1);
    ASSERT_EQ (client2.host (), _host);
}

/**
 * @brief Test scheme.
 */
TEST_F (SmtpClient, scheme)
{
    Smtp::Client client1 ("localhost", 25);
    ASSERT_EQ (client1.scheme (), "smtp");

    Smtp::Client client2 ("localhost", 465);
    ASSERT_EQ (client2.scheme (), "smtp");
}

/**
 * @brief Test host.
 */
TEST_F (SmtpClient, host)
{
    Smtp::Client client1 ("91.66.32.78", 25);
    ASSERT_EQ (client1.host (), "91.66.32.78");

    Smtp::Client client2 ("localhost", 465);
    ASSERT_EQ (client2.host (), "localhost");
}

/**
 * @brief Test port.
 */
TEST_F (SmtpClient, port)
{
    Smtp::Client client1 ("91.66.32.78", 25);
    ASSERT_EQ (client1.port (), 25);

    Smtp::Client client2 ("91.66.32.78", 465);
    ASSERT_EQ (client2.port (), 465);
}

/**
 * @brief Test authority.
 */
TEST_F (SmtpClient, authority)
{
    ASSERT_EQ (Smtp::Client ("localhost", 25).authority (), "localhost");
    ASSERT_EQ (Smtp::Client ("localhost", 465).authority (), "localhost:465");
    ASSERT_EQ (Smtp::Client ("localhost", 5000).authority (), "localhost:5000");

    ASSERT_EQ (Smtp::Client ("91.66.32.78", 25).authority (), "91.66.32.78");
    ASSERT_EQ (Smtp::Client ("91.66.32.78", 465).authority (), "91.66.32.78:465");
    ASSERT_EQ (Smtp::Client ("91.66.32.78", 5000).authority (), "91.66.32.78:5000");

    ASSERT_EQ (Smtp::Client ("2001:db8:1234:5678::1", 25).authority (), "[2001:db8:1234:5678::1]");
    ASSERT_EQ (Smtp::Client ("2001:db8:1234:5678::1", 465).authority (), "[2001:db8:1234:5678::1]:465");
    ASSERT_EQ (Smtp::Client ("2001:db8:1234:5678::1", 5000).authority (), "[2001:db8:1234:5678::1]:5000");
}

/**
 * @brief Test url.
 */
TEST_F (SmtpClient, url)
{
    ASSERT_EQ (Smtp::Client ("localhost", 25).url (), "smtp://localhost");
    ASSERT_EQ (Smtp::Client ("localhost", 465).url (), "smtp://localhost:465");
    ASSERT_EQ (Smtp::Client ("localhost", 5000).url (), "smtp://localhost:5000");

    ASSERT_EQ (Smtp::Client ("91.66.32.78", 25).url (), "smtp://91.66.32.78");
    ASSERT_EQ (Smtp::Client ("91.66.32.78", 465).url (), "smtp://91.66.32.78:465");
    ASSERT_EQ (Smtp::Client ("91.66.32.78", 5000).url (), "smtp://91.66.32.78:5000");

    ASSERT_EQ (Smtp::Client ("2001:db8:1234:5678::1", 25).url (), "smtp://[2001:db8:1234:5678::1]");
    ASSERT_EQ (Smtp::Client ("2001:db8:1234:5678::1", 465).url (), "smtp://[2001:db8:1234:5678::1]:465");
    ASSERT_EQ (Smtp::Client ("2001:db8:1234:5678::1", 5000).url (), "smtp://[2001:db8:1234:5678::1]:5000");
}

/**
 * @brief Test setCertificate.
 */
TEST_F (SmtpClient, setCertificate)
{
    Smtp::Client client (_host, _port);
    ASSERT_EQ (client.setCertificate ("/invalid/cert/path"), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (client.setCertificate (_certFile), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (client.setCertificate (_certFile, "/invalid/key/path"), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (client.setCertificate (_certFile, _invalidKey), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (client.setCertificate (_certFile, _key), 0) << join::lastError.message ();
}

/**
 * @brief Test setCaPath.
 */
TEST_F (SmtpClient, setCaPath)
{
    Smtp::Client client (_host, _port);
    ASSERT_EQ (client.setCaPath ("/invalid/ca/path"), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (client.setCaPath (_certFile), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (client.setCaPath (_certPath), 0) << join::lastError.message ();
}

/**
 * @brief Test setCaFile.
 */
TEST_F (SmtpClient, setCaFile)
{
    Smtp::Client client (_host, _port);
    ASSERT_EQ (client.setCaFile ("/invalid/ca/file"), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (client.setCaFile (_certPath), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (client.setCaFile (_certFile), 0) << join::lastError.message ();
}

/**
 * @brief Test setCipher.
 */
TEST_F (SmtpClient, setCipher)
{
    Smtp::Client client (_host, _port);
    ASSERT_EQ (client.setCipher ("foo"), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (client.setCipher (join::_defaultCipher), 0) << join::lastError.message ();
}

/**
 * @brief Test setCipher_1_3.
 */
TEST_F (SmtpClient, setCipher_1_3)
{
    Smtp::Client client (_host, _port);
    ASSERT_EQ (client.setCipher_1_3 ("foo"), -1);
    ASSERT_EQ (join::lastError, Errc::InvalidParam);
    ASSERT_EQ (client.setCipher_1_3 (join::_defaultCipher_1_3), 0) << join::lastError.message ();
}

/**
 * @brief Test send.
 */
TEST_F (SmtpClient, send)
{
    MailMessage message;
    message.sender ({"test@foo.com", "tester"});
    message.addRecipient ({"admin@foo.com", "admin"});
    message.subject ("this is a test");
    message.content ("this is a test");

    Smtp::Client client (_host, _port);
    client.credentials (_user, _password);
    client.setVerify (false);
    ASSERT_EQ (client.send (message), 0) << join::lastError.message ();
    client.setVerify (true, 0);
    ASSERT_EQ (client.send (message), -1);
    ASSERT_EQ (client.setCaFile (_rootcert), 0) << join::lastError.message ();
    ASSERT_EQ (client.send (message), -1);
    client.setVerify (true, 1);
    ASSERT_EQ (client.send (message), 0) << join::lastError.message ();
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    join::initializeOpenSSL ();
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
