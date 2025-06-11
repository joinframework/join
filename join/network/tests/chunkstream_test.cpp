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
#include <join/chunkstream.hpp>
#include <join/utils.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::Errc;
using join::Chunkstream;

/// chunk size.
const std::streamsize chuncksize = 24;

/// sample text.
const std::string decoded = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. "
                            "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. "
                            "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. "
                            "Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";

const std::string encoded = "18\r\nLorem ipsum dolor sit am\r\n18\r\net, consectetur adipisci\r\n18\r\nng elit, sed do eiusmod \r\n"
                            "18\r\ntempor incididunt ut lab\r\n18\r\nore et dolore magna aliq\r\n18\r\nua. Ut enim ad minim ven\r\n"
                            "18\r\niam, quis nostrud exerci\r\n18\r\ntation ullamco laboris n\r\n18\r\nisi ut aliquip ex ea com\r\n"
                            "18\r\nmodo consequat. Duis aut\r\n18\r\ne irure dolor in reprehe\r\n18\r\nnderit in voluptate veli\r\n"
                            "18\r\nt esse cillum dolore eu \r\n18\r\nfugiat nulla pariatur. E\r\n18\r\nxcepteur sint occaecat c\r\n"
                            "18\r\nupidatat non proident, s\r\n18\r\nunt in culpa qui officia\r\n18\r\n deserunt mollit anim id\r\n"
                            "d\r\n est laborum.\r\n0\r\n\r\n";

/**
 * @brief Test encode.
 */
TEST (Chunkstream, encode)
{
    // concrete stream.
    std::stringstream stream;

    // encode.
    Chunkstream tmp (stream, chuncksize);
    Chunkstream chunkstream = std::move (tmp);
    chunkstream.write (decoded.c_str (), decoded.size ());
    chunkstream.flush ();
    ASSERT_TRUE (chunkstream.good ());

    // check result
    ASSERT_EQ (stream.str (), encoded);
}

/**
 * @brief Test decode.
 */
TEST (Chunkstream, decode)
{
    // concrete stream.
    std::stringstream stream (encoded);
    char out[2048];

    // decode.
    Chunkstream chunkstream (stream);
    chunkstream.read (out, sizeof (out));
    ASSERT_EQ (decoded, std::string (out, out + chunkstream.gcount ()));

    // check extension.
    stream.clear (std::ios::goodbit);
    stream.str ("18;ext\r\nLorem ipsum dolor sit am\r\n0\r\n\r\n");

    chunkstream = Chunkstream (stream);
    chunkstream.read (out, sizeof (out));
    ASSERT_EQ ("Lorem ipsum dolor sit am", std::string (out, out + chunkstream.gcount ()));

    // check empty chunk size.
    stream.clear (std::ios::goodbit);
    stream.str ("\r\nThis is an empty chunk size\r\n\r\n0\r\n\r\n");

    chunkstream = Chunkstream (stream, chuncksize);
    chunkstream.read (out, sizeof (out));
    ASSERT_TRUE (chunkstream.fail ());
    ASSERT_EQ (join::lastError, Errc::InvalidParam);

    // check invalid chunk size.
    stream.clear (std::ios::goodbit);
    stream.str ("XX\r\nThis is an invalid chunk size\r\n\r\n0\r\n\r\n");

    chunkstream = Chunkstream (stream, chuncksize);
    chunkstream.read (out, sizeof (out));
    ASSERT_TRUE (chunkstream.fail ());
    ASSERT_EQ (join::lastError, Errc::InvalidParam);

    // check too big chunk size.
    stream.clear (std::ios::goodbit);
    stream.str ("24\r\nThis is a too big chunk size\r\n\r\n0\r\n\r\n");

    chunkstream = Chunkstream (stream, chuncksize);
    chunkstream.read (out, sizeof (out));
    ASSERT_TRUE (chunkstream.fail ());
    ASSERT_EQ (join::lastError, Errc::MessageTooLong);

    // check missing end line.
    stream.clear (std::ios::goodbit);
    stream.str ("12\r\nMissing end line\r\n0\r\n\r\n");

    chunkstream = Chunkstream (stream, chuncksize);
    chunkstream.read (out, sizeof (out));
    ASSERT_TRUE (chunkstream.fail ());
    ASSERT_EQ (join::lastError, Errc::InvalidParam);

    // check invalid data.
    stream.clear (std::ios::goodbit);
    stream.str ("18\r\n\r\n0\r\n\r\n");

    chunkstream = Chunkstream (stream, chuncksize);
    chunkstream.read (out, sizeof (out));
    ASSERT_TRUE (chunkstream.fail ());

    // check invalid data.
    stream.clear (std::ios::goodbit);
    stream.str ("0\r\n");

    chunkstream = Chunkstream (stream, chuncksize);
    chunkstream.read (out, sizeof (out));
    ASSERT_TRUE (chunkstream.fail ());

    // check invalid data.
    stream.clear (std::ios::goodbit);
    stream.str ("18");

    chunkstream = Chunkstream (stream, chuncksize);
    chunkstream.read (out, sizeof (out));
    ASSERT_TRUE (chunkstream.fail ());
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
