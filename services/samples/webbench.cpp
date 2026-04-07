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
#include <join/httpclient.hpp>
#include <join/filesystem.hpp>
#include <join/thread.hpp>
#include <join/mutex.hpp>
#include <join/cache.hpp>
#include <join/utils.hpp>

// C++.
#include <vector>
#include <regex>

// C.
#include <unistd.h>

struct BenchmarkContext
{
    std::atomic<int> counter{0};    ///< work queue counter
    std::atomic<int> ncomplete{0};  ///< successful requests
    std::atomic<int> nfail{0};      ///< failed requests
    join::Cache fileCache;          ///< file cache
    join::Mutex coutMutex;          ///< output guard
};

// =========================================================================
//   CLASS     :
//   METHOD    : version
// =========================================================================
void version ()
{
    std::cout << "webbench version " << JOIN_VERSION << "\n";
}

// =========================================================================
//   CLASS     :
//   METHOD    : usage
// =========================================================================
void usage ()
{
    std::cout << "Usage\n"
              << "  webbench [options] http[s]://hostname[:port]/path\n"
              << "\n"
              << "Options\n"
              << "  -c level          concurrency level (default: 1)\n"
              << "  -h                show available options\n"
              << "  -H                send HEAD request\n"
              << "  -K                enable keep alive\n"
              << "  -n requests       number of requests to perform (default: 1)\n"
              << "  -P file           file to POST (mime type is deduced from file extension)\n"
              << "  -t                request timeout in seconds\n"
              << "  -U file           file to PUT (mime type is deduced from file extension)\n"
              << "  -v                verbose\n"
              << "  -V                print version\n";
}

// =========================================================================
//   CLASS     :
//   METHOD    : benchmark
// =========================================================================
template <class Client>
void benchmark (const std::string& host, uint16_t port, join::HttpRequest request, const std::string& file, int timeout,
                int max, bool verbose, BenchmarkContext& ctx)
{
    Client client (host, port, false);
    std::string payload;
    void* addr = nullptr;
    struct stat sbuf;

    client.timeout (timeout * 1000);

    if (!file.empty ())
    {
        addr = ctx.fileCache.get (file, sbuf);
        if (addr != nullptr)
        {
            request.header ("Content-Length", std::to_string (sbuf.st_size));
        }
    }

    while (ctx.counter.fetch_add (1, std::memory_order_relaxed) < max)
    {
        join::HttpResponse response;

        // send request headers.
        if (client.send (request) == -1)
        {
            ++ctx.nfail;
            client.disconnect ();
            client.close ();
            continue;
        }

        if (verbose)
        {
            join::ScopedLock<join::Mutex> lock (ctx.coutMutex);
            std::cout << request.methodString () << " " << request.urn () << " " << request.version () << "\n"
                      << request.dumpHeaders ();
        }

        // send payload.
        if (addr != nullptr)
        {
            client.write (static_cast<const char*> (addr), sbuf.st_size);
            client.flush ();
        }

        // read response headers.
        if (client.receive (response) == -1)
        {
            ++ctx.nfail;
            client.disconnect ();
            client.close ();
            continue;
        }

        if (verbose)
        {
            join::ScopedLock<join::Mutex> lock (ctx.coutMutex);
            std::cout << response.version () << " " << response.status () << " " << response.reason () << "\n"
                      << response.dumpHeaders ();
        }

        // read payload.
        const auto len = response.contentLength ();
        if (len > 0)
        {
            payload.resize (len);
            client.read (&payload[0], payload.size ());
        }
        else if (response.header ("Transfer-Encoding").find ("chunked") != std::string::npos)
        {
            payload.resize (4096);
            while (client.read (&payload[0], payload.size ()))
            {
            }
            client.clear ();
        }

        ++ctx.ncomplete;
    }
}

// =========================================================================
//   CLASS     :
//   METHOD    : main
// =========================================================================
int main (int argc, char* argv[])
{
    join::HttpRequest request;
    int tasks = 1, max = 1, timeout = 5;
    bool verbose = false;
    std::string file;

    int opt;
    while ((opt = getopt (argc, argv, "c:hHKn:P:t:U:vV")) != -1)
    {
        switch (opt)
        {
            case 'c':
                tasks = std::stoi (optarg);
                break;
            case 'h':
                usage ();
                return EXIT_SUCCESS;
            case 'H':
                request.method (join::HttpMethod::Head);
                break;
            case 'K':
                request.header ("Connection", "keep-alive");
                break;
            case 'n':
                max = std::stoi (optarg);
                break;
            case 'P':
                file = optarg;
                request.method (join::HttpMethod::Post);
                request.header ("Content-Type", join::mime (file));
                break;
            case 't':
                timeout = std::stoi (optarg);
                break;
            case 'U':
                file = optarg;
                request.method (join::HttpMethod::Put);
                request.header ("Content-Type", join::mime (file));
                break;
            case 'v':
                verbose = true;
                break;
            case 'V':
                version ();
                return EXIT_SUCCESS;
            default:
                usage ();
                return EXIT_FAILURE;
        }
    }

    if (optind != argc - 1)
    {
        std::cerr << "invalid arguments\n\n";
        usage ();
        return EXIT_FAILURE;
    }

    // regular expression inspired by rfc3986 (see https://www.ietf.org/rfc/rfc3986.txt)
    // ex.
    // 0: https://example.com:8080/foo/bar.html?val=1#frag  # URL
    // 1: https                                             # Scheme
    // 2: example.com                                       # Host
    // 3: 8080                                              # Port
    // 4: /foo/bar.html                                     # Path
    // 5: val=1                                             # Query
    // 6: frag                                              # Fragment
    static const std::regex reg (
        R"(^(?:([^:/?#]+)://)?([a-z0-9\-._~%]+|\[[a-f0-9:.]+\])(?::([0-9]+))?([^?#]*)(?:\?([^#]*))?(?:#(.*))?)");
    std::cmatch match;

    if (!std::regex_match (argv[optind], match, reg))
    {
        std::cerr << "invalid arguments\n\n";
        usage ();
        return EXIT_FAILURE;
    }

    const std::string scheme = match[1];
    const std::string host = match[2];
    const uint16_t port =
        match[3].length () ? uint16_t (std::stoi (match[3])) : join::Resolver::resolveService (scheme);
    const std::string path = match[4];

    request.path (path);
    request.header ("Accept-Language", "fr-FR,fr;q=0.8,en-US;q=0.6,en;q=0.4");
    request.header ("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8");
    request.header ("If-Modified-Since", "Sat, 1 Jan 2000 00:00:00 GMT");
    request.header ("User-Agent", "webbench/" JOIN_VERSION);

    std::cout << "\nbenchmarking \"" << host << "\" on port " << port << " ...\n\n";

    BenchmarkContext ctx;

    auto run = [&] () {
        if (scheme == "https")
            ::benchmark<join::Https::Client> (host, port, request, file, timeout, max, verbose, ctx);
        else
            ::benchmark<join::Http::Client> (host, port, request, file, timeout, max, verbose, ctx);
    };

    std::vector<join::Thread> threads;
    threads.reserve (tasks);

    const auto elapsed = join::benchmark ([&] () {
        for (int i = 0; i < tasks; ++i)
            threads.emplace_back (run);

        for (auto& t : threads)
            t.join ();
    });

    const double secs = elapsed.count () / 1000.0;

    std::cout << "\n"
              << "Server Hostname:        " << host << "\n"
              << "Server Port:            " << port << "\n"
              << "\n"
              << "Scheme:                 " << scheme << "\n"
              << "Document Path:          " << request.path () << "\n"
              << "\n"
              << "Concurrency Level:      " << tasks << "\n"
              << "Time taken for tests:   " << secs << " seconds\n"
              << "Completed requests:     " << ctx.ncomplete << "\n"
              << "Failed requests:        " << ctx.nfail << "\n"
              << "Requests per second:    " << max / secs << " [#/sec]\n"
              << "\n";

    return EXIT_SUCCESS;
}
