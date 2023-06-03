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
#include <openssl/rand.h>
#include <openssl/err.h>

// C++.
#include <mutex>

std::once_flag flag;
#if OPENSSL_VERSION_NUMBER < 0x10100000L
std::unique_ptr <std::mutex[]> lock;

unsigned long threadIdCallback( void)
{
    return static_cast <unsigned long> (pthread_self ());
}

void threadLockingCallback (int mode, int num, const char *file, int line)
{
    if (mode & CRYPTO_LOCK)
    {
        lock[num].lock ();
    }
    else
    {
        lock[num].unlock ();
    }
}

void threadSetup ()
{
    lock.reset (new std::mutex[CRYPTO_num_locks ()]);
    if (lock == nullptr)
    {
        return;
    }
    CRYPTO_set_id_callback (threadIdCallback);
    CRYPTO_set_locking_callback (threadLockingCallback);
}

void swi::initializeOpenSSL ()
{
    // execute initialization only once.
    std::call_once (flag, [] () {
        // Initialize SSL algorithms.
        OpenSSL_add_all_algorithms ();

        // Initialize the SSL library.
        SSL_library_init ();

        // Initialize the SSL error strings.
        SSL_load_error_strings ();

        // Setup OpenSSL thread support callback.
        threadSetup ();
    });
}

int DH_set0_pqg (DH *dh, BIGNUM *p, BIGNUM *q, BIGNUM *g)
{
   /* If the fields p and g in d are NULL, the corresponding input
    * parameters MUST be non-NULL.  q may remain NULL.
    */
   if ((dh->p == nullptr && p == nullptr) || (dh->g == nullptr && g == nullptr))
       return 0;

   if (p != nullptr)
   {
       BN_free (dh->p);
       dh->p = p;
   }
   if (q != nullptr)
   {
       BN_free (dh->q);
       dh->q = q;
   }
   if (g != nullptr)
   {
       BN_free (dh->g);
       dh->g = g;
   }

   if (q != nullptr)
   {
       dh->length = BN_num_bits (q);
   }

   return 1;
}
#else /* OPENSSL_VERSION_NUMBER < 0x10100000L */

void join::initializeOpenSSL ()
{
    // execute initialization only once.
    std::call_once (flag, [] () {
        // initialize the Crypto library.
        OPENSSL_init_crypto (0, nullptr);

        // initialize the SSL library.
        OPENSSL_init_ssl (0, nullptr);
    });
}
#endif /* OPENSSL_VERSION_NUMBER < 0x10100000L */
