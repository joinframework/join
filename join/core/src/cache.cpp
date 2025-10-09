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

// libjoin.
#include <join/cache.hpp>

// C.
#include <cstring>
#include <cerrno>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

using join::Cache;

// =========================================================================
//   CLASS     : Cache
//   METHOD    : ~Cache
// =========================================================================
Cache::~Cache ()
{
    clear ();
}

// =========================================================================
//   CLASS     : Cache
//   METHOD    : get
// =========================================================================
void* Cache::get (const std::string &fileName, struct stat &sbuf)
{
    if ((stat (fileName.c_str (), &sbuf) < 0) || S_ISDIR (sbuf.st_mode))
    {
        return nullptr;
    }

    ScopedLock <Mutex> lock (_mutex);

    auto it = _entries.find (fileName);
    if (it != _entries.end ())
    {
        if (it->second->modifTime.tv_sec  == sbuf.st_ctim.tv_sec &&
            it->second->modifTime.tv_nsec == sbuf.st_ctim.tv_nsec)
        {
            return it->second->addr;
        }

        remove (fileName);
    }

    void* ptr = nullptr;

    int fd = open (fileName.c_str (), O_RDONLY);
    if (fd >= 0)
    {
        std::unique_ptr <CacheEntry> entry = std::make_unique <CacheEntry> ();
        if (entry != nullptr)
        {
            entry->size = sbuf.st_size;
            entry->modifTime = sbuf.st_ctim;
            entry->addr = mmap (0, entry->size, PROT_READ, MAP_PRIVATE, fd, 0);
            if (entry->addr != MAP_FAILED)
            {
                ptr = _entries.emplace (fileName, std::move (entry)).first->second->addr;
            }
        }

        close (fd);
    }

    return ptr;
}

// =========================================================================
//   CLASS     : Cache
//   METHOD    : remove
// =========================================================================
void Cache::remove (const std::string &fileName)
{
    ScopedLock <Mutex> lock (_mutex);

    auto it = _entries.find (fileName);
    if (it != _entries.end ())
    {
        munmap (it->second->addr, it->second->size);
        _entries.erase (it);
    }
}

// =========================================================================
//   CLASS     : Cache
//   METHOD    : clear
// =========================================================================
void Cache::clear ()
{
    ScopedLock <Mutex> lock (_mutex);

    for (auto const& entry : _entries)
    {
        munmap (entry.second->addr, entry.second->size);
    }

    _entries.clear ();
}

// =========================================================================
//   CLASS     : Cache
//   METHOD    : size
// =========================================================================
size_t Cache::size ()
{
    ScopedLock <Mutex> lock (_mutex);

    return _entries.size ();
}
