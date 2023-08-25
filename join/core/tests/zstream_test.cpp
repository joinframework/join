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
#include <join/zstream.hpp>

// Libraries.
#include <gtest/gtest.h>

// C++.
#include <sstream>

using join::Zstream;

/// sample text.
const std::string sample = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed non risus. Suspendisse lectus tortor, dignissim sit amet, adipiscing "
                           "nec, ultricies sed, dolor. Cras elementum ultrices diam. Maecenas ligula massa, varius a, semper congue, euismod non, mi. Proin "
                           "porttitor, orci nec nonummy molestie, enim est eleifend mi, non fermentum diam nisl sit amet erat. Duis semper. Duis arcu massa, "
                           "scelerisque vitae, consequat in, pretium a, enim. Pellentesque congue. Ut in risus volutpat libero pharetra tempor. Cras vestibulum "
                           "bibendum augue. Praesent egestas leo in pede. Praesent blandit odio eu enim. Pellentesque sed dui ut augue blandit sodales. "
                           "Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae; Aliquam nibh. Mauris ac mauris sed pede "
                           "pellentesque fermentum. Maecenas adipiscing ante non diam sodales hendrerit. "
                           "Ut velit mauris, egestas sed, gravida nec, ornare ut, mi. Aenean ut orci vel massa suscipit pulvinar. Nulla sollicitudin. Fusce varius, "
                           "ligula non tempus aliquam, nunc turpis ullamcorper nibh, in tempus sapien eros vitae ligula. Pellentesque rhoncus nunc et augue. "
                           "Integer id felis. Curabitur aliquet pellentesque diam. Integer quis metus vitae elit lobortis egestas. Lorem ipsum dolor sit amet, "
                           "consectetuer adipiscing elit. Morbi vel erat non mauris convallis vehicula. Nulla et sapien. Integer tortor tellus, aliquam faucibus, "
                           "convallis id, congue eu, quam. Mauris ullamcorper felis vitae erat. Proin feugiat, augue non elementum posuere, metus purus "
                           "iaculis lectus, et tristique ligula justo vitae magna. Aliquam convallis sollicitudin purus. Praesent aliquam, "
                           "enim at fermentum mollis, ligula massa adipiscing nisl, ac euismod nibh nisl eu lectus. "
                           "Fusce vulputate sem at sapien. Vivamus leo. Aliquam euismod libero eu enim. Nulla nec felis sed leo placerat imperdiet. "
                           "Aenean suscipit nulla in justo. Suspendisse cursus rutrum augue. Nulla tincidunt tincidunt mi. Curabitur iaculis, lorem vel rhoncus "
                           "faucibus, felis magna fermentum augue, et ultricies lacus lorem varius purus. Curabitur eu amet.\n";

/**
 * @brief Test deflate data format.
 */
TEST (Zstream, deflate)
{
    // concrete stream.
    std::stringstream stream;

    // compress using the deflate data format.
    stream.clear (std::ios::goodbit);
    Zstream zstream (stream, Zstream::Deflate);
    zstream.write (sample.c_str (), sample.length ());
    zstream.flush ();
    ASSERT_TRUE (zstream.good ());

    // check compression result.
    ASSERT_LT (stream.str ().size (), sample.size ());

    // uncompress using the deflate data format.
    std::string out;
    out.resize (sample.size ());
    zstream.read (&out[0], out.size ());
    ASSERT_TRUE (zstream.good ());

    // check uncompression result.
    ASSERT_EQ (out, sample);

    // test concrete output stream failure.
    stream.clear (std::ios::failbit);
    stream.seekg (0);
    Zstream tmp = std::move (Zstream (stream, Zstream::Deflate));
    out.resize (sample.size ());
    tmp.read (&out[0], out.size ());
    ASSERT_TRUE (tmp.fail ());

    // uncompress using invalid zlib data format.
    stream.clear (std::ios::goodbit);
    stream.seekg (0);
    tmp = Zstream (stream, Zstream::Zlib);
    out.resize (sample.size ());
    tmp.read (&out[0], out.size ());
    ASSERT_TRUE (tmp.fail ());

    // uncompress using invalid gzip data format.
    stream.clear (std::ios::goodbit);
    stream.seekg (0);
    tmp = Zstream (stream, Zstream::Gzip);
    out.resize (sample.size ());
    tmp.read (&out[0], out.size ());
    ASSERT_TRUE (tmp.fail ());
}

/**
 * @brief Test zlib data format.
 */
TEST (Zstream, zlib)
{
    // concrete stream.
    std::stringstream stream;

    // compress using the zlib data format.
    stream.clear (std::ios::goodbit);
    Zstream zstream (stream, Zstream::Zlib);
    zstream.write (sample.c_str (), sample.length ());
    zstream.flush ();
    ASSERT_TRUE (zstream.good ());

    // check compression result.
    ASSERT_LT (stream.str ().size (), sample.size ());

    // uncompress using the zlib data format.
    std::string out;
    out.resize (sample.size ());
    zstream.read (&out[0], out.size ());
    ASSERT_TRUE (zstream.good ());

    // check uncompression result.
    ASSERT_EQ (out, sample);

    // uncompress using invalid deflate data format.
    stream.clear (std::ios::goodbit);
    stream.seekg (0);
    Zstream tmp = std::move (Zstream (stream, Zstream::Deflate));
    out.resize (sample.size ());
    tmp.read (&out[0], out.size ());
    ASSERT_TRUE (tmp.fail ());

    // test concrete output stream failure.
    stream.clear (std::ios::failbit);
    stream.seekg (0);
    tmp = Zstream (stream, Zstream::Zlib);
    out.resize (sample.size ());
    tmp.read (&out[0], out.size ());
    ASSERT_TRUE (tmp.fail ());

    // uncompress using invalid gzip data format.
    stream.clear (std::ios::goodbit);
    stream.seekg (0);
    tmp = Zstream (stream, Zstream::Gzip);
    out.resize (sample.size ());
    tmp.read (&out[0], out.size ());
    ASSERT_TRUE (tmp.fail ());
}

/**
 * @brief Test gzip data format.
 */
TEST (Zstream, gzip)
{
    // concrete stream.
    std::stringstream stream;

    // compress using the gzip data format.
    stream.clear (std::ios::goodbit);
    Zstream zstream (stream, Zstream::Gzip);
    zstream.write (sample.c_str (), sample.length ());
    zstream.flush ();
    ASSERT_TRUE (zstream.good ());

    // check compression result.
    ASSERT_LT (stream.str ().size (), sample.size ());

    // uncompress using the gzip data format.
    std::string out;
    out.resize (sample.size ());
    zstream.read (&out[0], out.size ());
    ASSERT_TRUE (zstream.good ());

    // check uncompression result.
    ASSERT_EQ (out, sample);

    // uncompress using invalid deflate data format.
    stream.clear (std::ios::goodbit);
    stream.seekg (0);
    Zstream tmp = std::move (Zstream (stream, Zstream::Deflate));
    out.resize (sample.size ());
    tmp.read (&out[0], out.size ());
    ASSERT_TRUE (tmp.fail ());

    // uncompress using invalid zlib data format.
    stream.clear (std::ios::goodbit);
    stream.seekg (0);
    tmp = Zstream (stream, Zstream::Zlib);
    out.resize (sample.size ());
    tmp.read (&out[0], out.size ());
    ASSERT_TRUE (tmp.fail ());

    // test concrete output stream failure.
    stream.clear (std::ios::failbit);
    stream.seekg (0);
    tmp = Zstream (stream, Zstream::Gzip);
    out.resize (sample.size ());
    tmp.read (&out[0], out.size ());
    ASSERT_TRUE (tmp.fail ());
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
