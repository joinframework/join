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
#include <join/filesystem.hpp>

// Libraries.
#include <gtest/gtest.h>

/**
 * @brief Test base method.
 */
TEST (Filesystem, base)
{
    ASSERT_EQ (join::base ("foo"), "");
    ASSERT_EQ (join::base ("foo/bar"), "foo/");
    ASSERT_EQ (join::base ("foo/bar/"), "foo/bar/");
    ASSERT_EQ (join::base ("foo/bar/somefile.txt"), "foo/bar/");
    ASSERT_EQ (join::base ("/foo/bar"), "/foo/");
    ASSERT_EQ (join::base ("/foo/bar/"), "/foo/bar/");
    ASSERT_EQ (join::base ("/foo/bar/somefile.txt"), "/foo/bar/");
}

/**
 * @brief Test filename method.
 */
TEST (Filesystem, filename)
{
    ASSERT_EQ (join::filename ("bar"), "bar");
    ASSERT_EQ (join::filename ("foo/bar"), "bar");
    ASSERT_EQ (join::filename ("foo/bar/"), "");
    ASSERT_EQ (join::filename ("foo/bar/somefile.txt"), "somefile.txt");
    ASSERT_EQ (join::filename ("/foo/bar"), "bar");
    ASSERT_EQ (join::filename ("/foo/bar/"), "");
    ASSERT_EQ (join::filename ("/foo/bar/somefile.txt"), "somefile.txt");
}

/**
 * @brief Test extension method.
 */
TEST (Filesystem, extension)
{
    ASSERT_EQ (join::extension ("foo/bar"), "");
    ASSERT_EQ (join::extension ("foo/bar/"), "");
    ASSERT_EQ (join::extension ("foo/bar/somefile.txt"), "txt");
    ASSERT_EQ (join::extension ("/foo/bar"), "");
    ASSERT_EQ (join::extension ("/foo/bar/"), "");
    ASSERT_EQ (join::extension ("/foo/bar/somefile.txt"), "txt");
}

/**
 * @brief Test mime method.
 */
TEST (Filesystem, mime)
{
    ASSERT_EQ (join::mime ("foo.htm"), "text/html");
    ASSERT_EQ (join::mime ("foo.html"), "text/html");
    ASSERT_EQ (join::mime ("foo.css"), "text/css");
    ASSERT_EQ (join::mime ("foo.less"), "text/css");
    ASSERT_EQ (join::mime ("foo.js"), "application/javascript");
    ASSERT_EQ (join::mime ("foo.xml"), "text/xml");
    ASSERT_EQ (join::mime ("foo.json"), "application/json");
    ASSERT_EQ (join::mime ("foo.txt"), "text/plain");
    ASSERT_EQ (join::mime ("foo.properties"), "text/x-java-properties");
    ASSERT_EQ (join::mime ("foo.jpg"), "image/jpeg");
    ASSERT_EQ (join::mime ("foo.jpeg"), "image/jpeg");
    ASSERT_EQ (join::mime ("foo.png"), "image/png");
    ASSERT_EQ (join::mime ("foo.bmp"), "image/bmp");
    ASSERT_EQ (join::mime ("foo.gif"), "image/gif");
    ASSERT_EQ (join::mime ("foo.jpe"), "image/jpg");
    ASSERT_EQ (join::mime ("foo.xbm"), "image/xbm");
    ASSERT_EQ (join::mime ("foo.tiff"), "image/tiff");
    ASSERT_EQ (join::mime ("foo.tif"), "image/tiff");
    ASSERT_EQ (join::mime ("foo.ico"), "image/x-icon");
    ASSERT_EQ (join::mime ("foo.svg"), "image/svg+xml");
    ASSERT_EQ (join::mime ("foo.pdf"), "application/pdf");
    ASSERT_EQ (join::mime ("foo.mp3"), "audio/mpeg");
    ASSERT_EQ (join::mime ("foo.mp4"), "audio/mp4");
    ASSERT_EQ (join::mime ("foo.zip"), "application/zip");
    ASSERT_EQ (join::mime ("foo.bz2"), "application/x-bzip");
    ASSERT_EQ (join::mime ("foo.tbz2"), "application/x-bzip");
    ASSERT_EQ (join::mime ("foo.tb2"), "application/x-bzip");
    ASSERT_EQ (join::mime ("foo.gz"), "application/x-gzip");
    ASSERT_EQ (join::mime ("foo.gzip"), "application/x-gzip");
    ASSERT_EQ (join::mime ("foo.tar"), "application/x-tar");
    ASSERT_EQ (join::mime ("foo.rar"), "application/x-rar-compressed");
    ASSERT_EQ (join::mime ("foo.tpl"), "application/vnd.groove-tool-template");
    ASSERT_EQ (join::mime ("foo.woff"), "application/font-woff");
    ASSERT_EQ (join::mime ("foo.woff2"), "application/font-woff2");
    ASSERT_EQ (join::mime ("foo.foo"), "application/octet-stream");
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
