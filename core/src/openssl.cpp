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
#include <openssl/provider.h>
#include <openssl/rand.h>
#include <openssl/err.h>

// C++.
#include <mutex>

const std::string join::defaultCipher     = "EECDH+ECDSA+AESGCM:EECDH+aRSA+AESGCM:EECDH+ECDSA+CHACHA20:EECDH+aRSA+CHACHA20:EECDH+ECDSA+AESCCM:"
                                            "EDH+DSS+AESGCM:EDH+aRSA+CHACHA20:EDH+aRSA+AESCCM:-AESCCM8:EECDH+ECDSA+AESCCM8:EDH+aRSA+AESCCM8";

const std::string join::defaultCipher_1_3 = "TLS_AES_256_GCM_SHA384:TLS_AES_128_GCM_SHA256:TLS_CHACHA20_POLY1305_SHA256:"
                                            "TLS_AES_128_CCM_SHA256:TLS_AES_128_CCM_8_SHA256";

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
const std::string join::defaultCurve      = "prime256v1";
#endif

static std::once_flag flag;

void join::initializeOpenSSL ()
{
    // execute initialization only once.
    std::call_once (flag, [] () {
        // initialize the Crypto library.
        OPENSSL_init_crypto (0, nullptr);

        // initialize the SSL library.
        OPENSSL_init_ssl (0, nullptr);

    #if OPENSSL_VERSION_NUMBER >= 0x30000000L
        OSSL_PROVIDER_load (nullptr, "default");

        // required for MD5, SHA1, SM3 on many systems
        OSSL_PROVIDER_load (nullptr, "legacy");
    #endif
    });
}
