/**
 * MIT License
 *
 * Copyright (c) 2026 Mathieu Rabine
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
#include <join/tlscontext.hpp>
#include <join/utils.hpp>

// C.
#include <sys/stat.h>

using join::TlsContext;

// =========================================================================
//   CLASS     : TlsContext
//   METHOD    : TlsContext
// =========================================================================
TlsContext::TlsContext (Role role)
: _role (role)
, _ctx (SSL_CTX_new (method (role)), SSL_CTX_free)
{
    if (!_ctx)
    {
        throw std::runtime_error ("SSL_CTX_new failed");  // LCOV_EXCL_LINE
    }

    // enable the OpenSSL bug workaround options.
    SSL_CTX_set_options (_ctx.get (), SSL_OP_ALL);

    // disallow compression.
    SSL_CTX_set_options (_ctx.get (), SSL_OP_NO_COMPRESSION);

    // disallow usage of SSLv2, SSLv3, TLSv1 and TLSv1.1 which are insecure.
    SSL_CTX_set_options (_ctx.get (), SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1);

    // setup write mode.
    SSL_CTX_set_mode (_ctx.get (), SSL_MODE_ENABLE_PARTIAL_WRITE | SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);

    // automatically renegotiates.
    SSL_CTX_set_mode (_ctx.get (), SSL_MODE_AUTO_RETRY);

    // no verification by default.
    SSL_CTX_set_verify (_ctx.get (), SSL_VERIFY_NONE, nullptr);

    // set default TLSv1.2 and below cipher suites.
    SSL_CTX_set_cipher_list (_ctx.get (), defaultCipher.c_str ());

    // set default TLSv1.3 cipher suites.
    SSL_CTX_set_ciphersuites (_ctx.get (), defaultCipher_1_3.c_str ());

    if (isServer ())
    {
        // choose the cipher according to the server's preferences.
        SSL_CTX_set_options (_ctx.get (), SSL_OP_CIPHER_SERVER_PREFERENCE);

        int sid = randomize<int> ();

        // enable session caching.
        SSL_CTX_set_session_id_context (_ctx.get (), reinterpret_cast<const uint8_t*> (&sid), sizeof (sid));

        // disallow client-side renegotiation.
        SSL_CTX_set_options (_ctx.get (), SSL_OP_NO_RENEGOTIATION);

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
        // use the default built-in Diffie-Hellman parameters.
        SSL_CTX_set_dh_auto (_ctx.get (), 1);

        // Set elliptic curve Diffie-Hellman key.
        SSL_CTX_set1_groups_list (_ctx.get (), defaultCurve.c_str ());
#else
        // Set Diffie-Hellman key.
        DhKeyPtr dh (getDh2236 ());
        if (dh)
        {
            SSL_CTX_set_tmp_dh (_ctx.get (), dh.get ());
        }

        // Set elliptic curve Diffie-Hellman key.
        EcdhKeyPtr ecdh (EC_KEY_new_by_curve_name (NID_X9_62_prime256v1));
        if (ecdh)
        {
            SSL_CTX_set_tmp_ecdh (_ctx.get (), ecdh.get ());
        }
#endif

        // set session cache mode.
        SSL_CTX_set_session_cache_mode (_ctx.get (), SSL_SESS_CACHE_SERVER);
    }
    else
    {
        // set session cache mode.
        SSL_CTX_set_session_cache_mode (_ctx.get (), SSL_SESS_CACHE_CLIENT);
    }
}

// =========================================================================
//   CLASS     : TlsContext
//   METHOD    : setCertificate
// =========================================================================
int TlsContext::setCertificate (const std::string& cert, const std::string& key) noexcept
{
    if (SSL_CTX_use_certificate_file (_ctx.get (), cert.c_str (), SSL_FILETYPE_PEM) == 0)
    {
        lastError = make_error_code (Errc::InvalidParam);
        return -1;
    }

    if (key.size ())
    {
        if (SSL_CTX_use_PrivateKey_file (_ctx.get (), key.c_str (), SSL_FILETYPE_PEM) == 0)
        {
            lastError = make_error_code (Errc::InvalidParam);
            return -1;
        }
    }

    if (SSL_CTX_check_private_key (_ctx.get ()) == 0)
    {
        lastError = make_error_code (Errc::InvalidParam);
        return -1;
    }

    return 0;
}

// =========================================================================
//   CLASS     : TlsContext
//   METHOD    : setCaPath
// =========================================================================
int TlsContext::setCaPath (const std::string& caPath)
{
    struct stat st;
    if (stat (caPath.c_str (), &st) != 0 || !S_ISDIR (st.st_mode) ||
        SSL_CTX_load_verify_locations (_ctx.get (), nullptr, caPath.c_str ()) == 0)
    {
        lastError = make_error_code (Errc::InvalidParam);
        return -1;
    }

    return 0;
}

// =========================================================================
//   CLASS     : TlsContext
//   METHOD    : setCaFile
// =========================================================================
int TlsContext::setCaFile (const std::string& caFile)
{
    struct stat st;
    if (stat (caFile.c_str (), &st) != 0 || !S_ISREG (st.st_mode) ||
        SSL_CTX_load_verify_locations (_ctx.get (), caFile.c_str (), nullptr) == 0)
    {
        lastError = make_error_code (Errc::InvalidParam);
        return -1;
    }

    return 0;
}

// =========================================================================
//   CLASS     : TlsContext
//   METHOD    : setVerify
// =========================================================================
void TlsContext::setVerify (bool verify, int depth) noexcept
{
    _verify = verify;
    _depth = depth;
}

// =========================================================================
//   CLASS     : TlsContext
//   METHOD    : setCipher
// =========================================================================
int TlsContext::setCipher (const std::string& cipher) noexcept
{
    if (SSL_CTX_set_cipher_list (_ctx.get (), cipher.c_str ()) == 0)
    {
        lastError = make_error_code (Errc::InvalidParam);
        return -1;
    }

    return 0;
}

// =========================================================================
//   CLASS     : TlsContext
//   METHOD    : setCipher_1_3
// =========================================================================
int TlsContext::setCipher_1_3 (const std::string& cipher) noexcept
{
    if (SSL_CTX_set_ciphersuites (_ctx.get (), cipher.c_str ()) == 0)
    {
        lastError = make_error_code (Errc::InvalidParam);
        return -1;
    }

    return 0;
}

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
// =========================================================================
//   CLASS     : TlsContext
//   METHOD    : setCurve
// =========================================================================
int TlsContext::setCurve (const std::string& curves)
{
    if (SSL_CTX_set1_groups_list (_ctx.get (), curves.c_str ()) == 0)
    {
        lastError = make_error_code (Errc::InvalidParam);
        return -1;
    }

    return 0;
}
#endif

// =========================================================================
//   CLASS     : TlsContext
//   METHOD    : setAlpnProtocols
// =========================================================================
int TlsContext::setAlpnProtocols (const std::vector<std::string>& protocols)
{
    std::vector<uint8_t> wire;
    wire.reserve (256);

    for (const auto& proto : protocols)
    {
        if (proto.size () > 255)
        {
            lastError = make_error_code (Errc::InvalidParam);
            return -1;
        }

        wire.push_back (static_cast<uint8_t> (proto.size ()));
        wire.insert (wire.end (), proto.begin (), proto.end ());
    }

    if (SSL_CTX_set_alpn_protos (_ctx.get (), wire.data (), static_cast<unsigned int> (wire.size ())) != 0)
    {
        lastError = make_error_code (Errc::InvalidParam);
        return -1;
    }

    return 0;
}

// =========================================================================
//   CLASS     : TlsContext
//   METHOD    : handle
// =========================================================================
SSL_CTX* TlsContext::handle () const noexcept
{
    return _ctx.get ();
}

// =========================================================================
//   CLASS     : TlsContext
//   METHOD    : verify
// =========================================================================
bool TlsContext::verify () const noexcept
{
    return _verify;
}

// =========================================================================
//   CLASS     : TlsContext
//   METHOD    : depth
// =========================================================================
int TlsContext::depth () const noexcept
{
    return _depth;
}

// =========================================================================
//   CLASS     : TlsContext
//   METHOD    : isServer
// =========================================================================
bool TlsContext::isServer () const noexcept
{
    return (_role == Role::TlsServer) || (_role == Role::DtlsServer);
}

// =========================================================================
//   CLASS     : TlsContext
//   METHOD    : method
// =========================================================================
const SSL_METHOD* TlsContext::method (Role role) noexcept
{
    switch (role)
    {
        case Role::TlsServer:
            return TLS_server_method ();
        case Role::DtlsClient:
            return DTLS_client_method ();
        case Role::DtlsServer:
            return DTLS_server_method ();
        default:
            return TLS_client_method ();
    }
}

#if OPENSSL_VERSION_NUMBER < 0x30000000L
// =========================================================================
//   CLASS     : TlsContext
//   METHOD    : getDh2236
// =========================================================================
DH* TlsContext::getDh2236 ()
{
    static unsigned char dhp_2236[] = {
        0x0C, 0xA5, 0x51, 0x2B, 0x8F, 0xF7, 0xA8, 0x74, 0x4D, 0x52, 0xD7, 0xED, 0x97, 0x83, 0xA4, 0xD2, 0x8B, 0xF3,
        0xE7, 0x92, 0xF0, 0x27, 0x1B, 0xA0, 0x80, 0x83, 0x19, 0xDD, 0x02, 0xEF, 0xA3, 0xE6, 0x13, 0x0A, 0x47, 0xE6,
        0xF1, 0x3B, 0xC1, 0x5F, 0x63, 0xC4, 0x03, 0xBA, 0xAC, 0xAA, 0xA3, 0x44, 0xC2, 0x03, 0x6D, 0x62, 0x33, 0xAA,
        0xF9, 0xA2, 0x5A, 0x98, 0xC2, 0xC0, 0x71, 0x6F, 0xB0, 0x93, 0x6A, 0x26, 0x92, 0x90, 0x95, 0xEA, 0xE8, 0x5F,
        0x81, 0x50, 0x57, 0xB3, 0xB7, 0xE6, 0x3A, 0x3A, 0x90, 0x15, 0x01, 0x2F, 0xC7, 0x8F, 0xAA, 0x0C, 0xAE, 0xC0,
        0xFF, 0x3A, 0xA7, 0x26, 0x5C, 0x87, 0xC2, 0x00, 0x68, 0xCA, 0x02, 0x06, 0x50, 0x44, 0xEE, 0x75, 0xE7, 0xFF,
        0x16, 0xD1, 0x0F, 0x64, 0x51, 0x97, 0x52, 0x54, 0x69, 0xF0, 0x31, 0x81, 0x4D, 0xEB, 0xF5, 0xA8, 0xB3, 0x7B,
        0x48, 0x60, 0xBD, 0xC7, 0xC9, 0x6E, 0x97, 0x86, 0x9B, 0xE6, 0x66, 0x4E, 0x1D, 0xE5, 0x6F, 0xBA, 0xC5, 0x3D,
        0xFD, 0x3F, 0x34, 0x69, 0x6F, 0xC0, 0xFA, 0x8D, 0x42, 0x73, 0xA2, 0x49, 0xDE, 0xB6, 0x8D, 0x71, 0x15, 0xFC,
        0xB4, 0x18, 0x31, 0x5A, 0x24, 0xD0, 0x5E, 0xA8, 0xE0, 0xD8, 0x1C, 0xF8, 0x0F, 0x1F, 0x59, 0x22, 0x5A, 0x07,
        0x75, 0x06, 0x98, 0x58, 0xE1, 0xF6, 0xA5, 0x53, 0xFD, 0x66, 0x1E, 0x8F, 0x41, 0x63, 0x61, 0xA1, 0x79, 0x0D,
        0x3B, 0xA7, 0xF4, 0xBD, 0x72, 0xEB, 0xE1, 0xDC, 0xE2, 0xC9, 0x9B, 0x41, 0xF6, 0x33, 0x3F, 0x9F, 0x0C, 0x33,
        0x7B, 0xF2, 0x90, 0x68, 0x28, 0xD3, 0x5A, 0xC1, 0x5C, 0xDE, 0x15, 0x11, 0xF4, 0xDD, 0xCB, 0x09, 0x78, 0x63,
        0x3B, 0xB6, 0xE8, 0xEE, 0x9A, 0x48, 0xE9, 0x79, 0x80, 0x3F, 0x34, 0x8D, 0xB9, 0x24, 0x8D, 0x94, 0x88, 0xA9,
        0x75, 0xA5, 0x19, 0x05, 0x8D, 0x77, 0x20, 0xAF, 0xC2, 0xC9, 0x7B, 0xD2, 0x51, 0xEE, 0x17, 0x22, 0xAC, 0x33,
        0xA8, 0xA6, 0x1B, 0x8B, 0xE3, 0x79, 0xF3, 0xE8, 0x3B, 0x6B};
    static unsigned char dhg_2236[] = {0x02};

    DH* dh = DH_new ();
    if (dh == nullptr)
    {
        return nullptr;
    }

    BIGNUM* p = BN_bin2bn (dhp_2236, sizeof (dhp_2236), nullptr);
    BIGNUM* g = BN_bin2bn (dhg_2236, sizeof (dhg_2236), nullptr);

    if ((p == nullptr) || (g == nullptr) || !DH_set0_pqg (dh, p, nullptr, g))
    {
        DH_free (dh);
        BN_free (p);
        BN_free (g);
        return nullptr;
    }

    return dh;
}
#endif
