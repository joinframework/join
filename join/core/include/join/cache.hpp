/**
 * MIT License
 *
 * Copyright (c) 2022 Mathieu Rabine
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

#ifndef __JOIN_CACHE_HPP__
#define __JOIN_CACHE_HPP__

// libjoin.
#include <join/mutex.hpp>

// C++.
#include <string>
#include <memory>
#include <map>

// C.
#include <sys/stat.h>

namespace join
{
    /**
     * @brief File cache.
     */
    class Cache
    {
    public:
        /**
         * @brief create instance.
         */
        Cache () = default;

        /**
         * @brief create instance by copy.
         * @param other object to copy.
         */
        Cache (const Cache& other) = delete;

        /**
         * @brief assign instance by copy.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        Cache& operator= (const Cache& other) = delete;

        /**
         * @brief create instance by move.
         * @param other object to move.
         */
        Cache (Cache&& other) = delete;

        /**
         * @brief assign instance by move.
         * @param other object to move.
         * @return a reference of the current object.
         */
        Cache& operator= (Cache&& other) = delete;

        /**
         * @brief destroy instance.
         */
        ~Cache ();

        /**
         * @brief get or create the cache entry for the given file.
         * @param fileName path of the file that we want to get cache.
         * @param sbuf buffer returned by a previous call to stat ().
         * @return a pointer to the buffer where the cached file is saved.
         */
        void* get (const std::string &fileName, struct stat *sbuf = nullptr);

        /**
         * @brief remove a cached entry identified by the given file name.
         * @param fileName path of the file that we want to remove cache entry.
         */
        void remove (const std::string &fileName);

        /**
         * @brief clear all cached entries.
         */
        void clear ();

        /**
         * @brief get number of cache entries.
         */
        size_t size ();

    protected:
        /**
         *  @brief cache entry.
         */
        struct CacheEntry
        {
            off_t  size;            /**< file size. */
            time_t modifTime;       /**< file modification date. */
            void   *addr;           /**< file content address. */
        };

        /// cached entries map.
        std::map <std::string, std::unique_ptr <CacheEntry>> _entries;

        /// protection mutex for the cache map.
        Mutex _mutex;
    };
}

#endif
