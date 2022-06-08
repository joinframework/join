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

/**
 * @brief BigNumPtr test.
 */
TEST (Openssl, BigNumPtr)
{
    join::crypto::BigNumPtr bn (BN_new ());
    ASSERT_NE (bn, nullptr);
    bn.reset ();
    ASSERT_EQ (bn, nullptr);
}

/**
 * @brief EcdsaSigPtr test.
 */
TEST (Openssl, EcdsaSigPtr)
{
    join::crypto::EcdsaSigPtr sig (ECDSA_SIG_new ());
    ASSERT_NE (sig, nullptr);
    sig.reset ();
    ASSERT_EQ (sig, nullptr);
}

/**
 * @brief EvpPkeyPtr test.
 */
TEST (Openssl, EvpPkeyPtr)
{
    join::crypto::EvpPkeyPtr evp (EVP_PKEY_new ());
    ASSERT_NE (evp, nullptr);
    evp.reset ();
    ASSERT_EQ (evp, nullptr);
}

/**
 * @brief EvpMdCtxPtr test.
 */
TEST (Openssl, EvpMdCtxPtr)
{
    join::crypto::EvpMdCtxPtr ctx (EVP_MD_CTX_new ());
    ASSERT_NE (ctx, nullptr);
    ctx.reset ();
    ASSERT_EQ (ctx, nullptr);
}

/**
 * @brief EvpMdCtxPtr test.
 */
TEST (Openssl, StackOfX509NamePtr)
{
}

/**
 * @brief StackOfGeneralNamePtr test.
 */
TEST (Openssl, StackOfGeneralNamePtr)
{
}

/**
 * @brief SslPtr test.
 */
TEST (Openssl, SslPtr)
{
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    join::crypto::SslCtxPtr ctx (SSL_CTX_new (SSLv23_method ()), join::crypto::SslCtxDelete ());
#else
    join::crypto::SslCtxPtr ctx (SSL_CTX_new (TLS_method ()), join::crypto::SslCtxDelete ());
#endif
    ASSERT_NE (ctx, nullptr);
    join::crypto::SslPtr ssl (SSL_new (ctx.get ()));
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
TEST (Openssl, DhKeyPtr)
{
    join::crypto::DhKeyPtr dh (DH_new ());
    ASSERT_NE (dh, nullptr);
    dh.reset ();
    ASSERT_EQ (dh, nullptr);
}

/**
 * @brief EcdhKeyPtr test.
 */
TEST (Openssl, EcdhKeyPtr)
{
    join::crypto::EcdhKeyPtr ecdh (EC_KEY_new_by_curve_name (NID_X9_62_prime256v1));
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
    join::crypto::initializeOpenSSL ();
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
