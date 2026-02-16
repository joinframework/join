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

#ifndef __JOIN_FILESYSTEM_HPP__
#define __JOIN_FILESYSTEM_HPP__

// C++.
#include <string>

// C.
#include <sys/stat.h>

namespace join
{
    /**
     * @brief get base path of the specified file.
     * @param filepath path to parse.
     * @return base path of the specified file.
     */
    __inline__ std::string base (const std::string& filepath)
    {
        size_t pos = filepath.rfind ("/");

        if (pos == std::string::npos)
            return {};
        else
            return filepath.substr (0, pos + 1);
    }

    /**
     * @brief get file name of the specified file.
     * @param filepath path to parse.
     * @return The file name of the specified file.
     */
    __inline__ std::string filename (const std::string& filepath)
    {
        size_t pos = filepath.rfind ("/");

        if (pos == std::string::npos)
            return filepath;
        else
            return filepath.substr (pos + 1);
    }

    /**
     * @brief get extension of the file specified.
     * @param filepath path to parse.
     * @return The extension of the specified file.
     */
    __inline__ std::string extension (const std::string& filepath)
    {
        size_t pos = filepath.rfind (".");

        if (pos == std::string::npos)
            return {};
        else
            return filepath.substr (pos + 1);
    }

    /**
     * @brief get mime type of the specified file.
     * @param filepath path to parse.
     * @return mime type of the specified file.
     */
    __inline__ std::string mime (const std::string& filepath)
    {
        std::string mime, suffix (extension (filepath));

        if (suffix == "htm")
            mime = "text/html";
        else if (suffix == "html")
            mime = "text/html";
        else if (suffix == "css")
            mime = "text/css";
        else if (suffix == "less")
            mime = "text/css";
        else if (suffix == "js")
            mime = "application/javascript";
        else if (suffix == "xml")
            mime = "text/xml";
        else if (suffix == "json")
            mime = "application/json";
        else if (suffix == "txt")
            mime = "text/plain";
        else if (suffix == "properties")
            mime = "text/x-java-properties";
        else if (suffix == "jpg")
            mime = "image/jpeg";
        else if (suffix == "jpeg")
            mime = "image/jpeg";
        else if (suffix == "png")
            mime = "image/png";
        else if (suffix == "bmp")
            mime = "image/bmp";
        else if (suffix == "gif")
            mime = "image/gif";
        else if (suffix == "jpe")
            mime = "image/jpg";
        else if (suffix == "xbm")
            mime = "image/xbm";
        else if (suffix == "tiff")
            mime = "image/tiff";
        else if (suffix == "tif")
            mime = "image/tiff";
        else if (suffix == "ico")
            mime = "image/x-icon";
        else if (suffix == "svg")
            mime = "image/svg+xml";
        else if (suffix == "pdf")
            mime = "application/pdf";
        else if (suffix == "mp3")
            mime = "audio/mpeg";
        else if (suffix == "mp4")
            mime = "audio/mp4";
        else if (suffix == "zip")
            mime = "application/zip";
        else if (suffix == "bz2")
            mime = "application/x-bzip";
        else if (suffix == "tbz2")
            mime = "application/x-bzip";
        else if (suffix == "tb2")
            mime = "application/x-bzip";
        else if (suffix == "gz")
            mime = "application/x-gzip";
        else if (suffix == "gzip")
            mime = "application/x-gzip";
        else if (suffix == "tar")
            mime = "application/x-tar";
        else if (suffix == "rar")
            mime = "application/x-rar-compressed";
        else if (suffix == "tpl")
            mime = "application/vnd.groove-tool-template";
        else if (suffix == "woff")
            mime = "application/font-woff";
        else if (suffix == "woff2")
            mime = "application/font-woff2";
        else
            mime = "application/octet-stream";

        return mime;
    }

    /**
     * @brief check if the file specified exists.
     * @param filepath Path to the file to examine.
     * @return true if file exists, false otherwise.
     */
    __inline__ bool exists (const std::string& filepath)
    {
        struct stat status;
        return stat (filepath.c_str (), &status) == 0;
    }
}

#endif
