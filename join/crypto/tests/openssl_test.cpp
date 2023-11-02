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
#include <join/openssl.hpp>

// Libraries.
#include <gtest/gtest.h>

// C++.
#include <fstream>

/**
 * @brief Class used to test the openssl helper API.
 */
class Openssl : public ::testing::Test
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
    /// root certificate.
    static const std::string _rootcert;

    /// certificate.
    static const std::string _cert;

    /// private key.
    static const std::string _key;
};

const std::string Openssl::_rootcert = "/tmp/tlssocket_test_root.cert";
const std::string Openssl::_cert = "/tmp/tlssocket_test.cert";
const std::string Openssl::_key = "/tmp/tlssocket_test.key";

/**
 * @brief BigNumPtr test.
 */
TEST_F (Openssl, BigNumPtr)
{
    join::BigNumPtr bn (BN_new ());
    ASSERT_NE (bn, nullptr);
    bn.reset ();
    ASSERT_EQ (bn, nullptr);
}

/**
 * @brief EcdsaSigPtr test.
 */
TEST_F (Openssl, EcdsaSigPtr)
{
    join::EcdsaSigPtr sig (ECDSA_SIG_new ());
    ASSERT_NE (sig, nullptr);
    sig.reset ();
    ASSERT_EQ (sig, nullptr);
}

/**
 * @brief EvpPkeyPtr test.
 */
TEST_F (Openssl, EvpPkeyPtr)
{
    join::EvpPkeyPtr evp (EVP_PKEY_new ());
    ASSERT_NE (evp, nullptr);
    evp.reset ();
    ASSERT_EQ (evp, nullptr);
}

/**
 * @brief EvpPkeyCtxPtr test.
 */
TEST_F (Openssl, EvpPkeyCtxPtr)
{
    FILE *fkey = fopen (_key.c_str (), "r");
    ASSERT_NE (fkey, nullptr);
    join::EvpPkeyPtr evp (PEM_read_PrivateKey (fkey, nullptr, 0, nullptr));
    fclose (fkey);
    ASSERT_NE (evp, nullptr);
    join::EvpPkeyCtxPtr evpctx (EVP_PKEY_CTX_new (evp.get (), nullptr));
    ASSERT_NE (evpctx, nullptr);
    evpctx.reset ();
    ASSERT_EQ (evpctx, nullptr);
    evp.reset ();
    ASSERT_EQ (evp, nullptr);
}

/**
 * @brief EvpEncodeCtxPtr test.
 */
TEST_F (Openssl, EvpEncodeCtxPtr)
{
    join::EvpEncodeCtxPtr enc (EVP_ENCODE_CTX_new ());
    ASSERT_NE (enc, nullptr);
    enc.reset ();
    ASSERT_EQ (enc, nullptr);
}

/**
 * @brief EvpMdCtxPtr test.
 */
TEST_F (Openssl, EvpMdCtxPtr)
{
    join::EvpMdCtxPtr ctx (EVP_MD_CTX_new ());
    ASSERT_NE (ctx, nullptr);
    ctx.reset ();
    ASSERT_EQ (ctx, nullptr);
}

/**
 * @brief StackOfX509NamePtr test.
 */
TEST_F (Openssl, StackOfX509NamePtr)
{
    join::StackOfX509NamePtr subject (SSL_load_client_CA_file (_rootcert.c_str ()));
    ASSERT_NE (subject, nullptr);
    subject.reset ();
    ASSERT_EQ (subject, nullptr);
}

/**
 * @brief StackOfGeneralNamePtr test.
 */
TEST_F (Openssl, StackOfGeneralNamePtr)
{
    FILE *file = fopen (_cert.c_str (), "rb");
    ASSERT_NE (file, nullptr);
    X509 *cert = PEM_read_X509 (file, nullptr, nullptr, nullptr);
    fclose (file);
    ASSERT_NE (cert, nullptr);
    join::StackOfGeneralNamePtr altnames (reinterpret_cast <STACK_OF (GENERAL_NAME)*> (X509_get_ext_d2i (cert, NID_subject_alt_name, 0, 0)));
    X509_free (cert);
    ASSERT_NE (altnames, nullptr);
    altnames.reset ();
    ASSERT_EQ (altnames, nullptr);
}

/**
 * @brief SslPtr test.
 */
TEST_F (Openssl, SslPtr)
{
    join::SslCtxPtr ctx (SSL_CTX_new (TLS_method ()));
    ASSERT_NE (ctx, nullptr);
    join::SslPtr ssl (SSL_new (ctx.get ()));
    ASSERT_NE (ssl, nullptr);
    ssl.reset ();
    ASSERT_EQ (ssl, nullptr);
    ctx.reset ();
    ASSERT_EQ (ctx, nullptr);
}

#if OPENSSL_VERSION_NUMBER < 0x30000000L
/**
 * @brief DhKeyPtr test.
 */
TEST_F (Openssl, DhKeyPtr)
{
    join::DhKeyPtr dh (DH_new ());
    ASSERT_NE (dh, nullptr);
    dh.reset ();
    ASSERT_EQ (dh, nullptr);
}

/**
 * @brief EcdhKeyPtr test.
 */
TEST_F (Openssl, EcdhKeyPtr)
{
    join::EcdhKeyPtr ecdh (EC_KEY_new_by_curve_name (NID_X9_62_prime256v1));
    ASSERT_NE (ecdh, nullptr);
    ecdh.reset ();
    ASSERT_EQ (ecdh, nullptr);
}
#endif

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    join::initializeOpenSSL ();
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
