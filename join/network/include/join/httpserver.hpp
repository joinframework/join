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

#ifndef __JOIN_HTTP_SERVER_HPP__
#define __JOIN_HTTP_SERVER_HPP__

// libjoin.
#include <join/httpmessage.hpp>
#include <join/filesystem.hpp>
#include <join/acceptor.hpp>
#include <join/version.hpp>
#include <join/thread.hpp>
#include <join/cache.hpp>

// C++.
#include <sys/eventfd.h>
#include <thread>
#include <memory>
#include <vector>

// C.
#include <fnmatch.h>

namespace join
{
    /**
     * @brief HTTP content Type.
     */
    enum HttpContentType
    {
        Root,           /**< Content appended to root directory. */
        Alias,          /**< Content replaced by alias. */
        Exec,           /**< Executable content. */
        Redirect,       /**< Redirection. */
    };

    /**
     * @brief basic HTTP content.
     */
    template <class Protocol>
    struct BasicHttpContent
    {
        using Handler = std::function <void (typename Protocol::Worker*)>;
        using Access  = std::function <bool (const std::string&, const std::string&, std::error_code&)>;

        HttpMethod      methods;        /**< allowed methods. */
        HttpContentType type;           /**< content type (root, alias etc...). */
        std::string     directory;      /**< requested resource directory. */
        std::string     name;           /**< requested resource file name. */
        std::string     alias;          /**< mapped file system path. */
        Handler         handler;        /**< mapped content handler. */
        Access          access;         /**< access handler. */
    };

    /**
     * @brief basic HTTP worker.
     */
    template <class Protocol>
    class BasicHttpWorker : public Protocol::Stream
    {
    public:
        using Content = BasicHttpContent <Protocol>;
        using Server  = BasicHttpServer <Protocol>;

        /**
         * @brief create the worker instance.
         * @param server Server instance.
         */
        BasicHttpWorker (Server* server)
        : _server (server),
          _thread ([this] () {work ();})
        {
        }

        /**
         * @brief create instance by copy.
         * @param other object to copy.
         */
        BasicHttpWorker (const BasicHttpWorker& other) = delete;

        /**
         * @brief assign instance by copy.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        BasicHttpWorker& operator= (const BasicHttpWorker& other) = delete;

        /**
         * @brief create instance by move.
         * @param other object to move.
         */
        BasicHttpWorker (BasicHttpWorker&& other) = delete;

        /**
         * @brief assign instance by move.
         * @param other object to move.
         * @return a reference of the current object.
         */
        BasicHttpWorker& operator= (BasicHttpWorker&& other) = delete;

        /**
         * @brief destroy worker thread.
         */
        virtual ~BasicHttpWorker ()
        {
            this->_thread.join ();
        }

        /**
         * @brief send headers.
         */
        void sendHeaders ()
        {
            // restore concrete stream.
            this->clearEncoding ();

            // set missing response headers.
            if (!this->_response.hasHeader ("Date"))
            {
                std::stringstream gmt;
                std::time_t ti = std::time (nullptr);
                gmt << std::put_time (std::gmtime (&ti), "%a, %d %b %Y %H:%M:%S GMT");
                this->_response.header ("Date", gmt.str ());
            }
            if (!this->_response.hasHeader ("Server"))
            {
                this->_response.header ("Server", "join/" JOIN_VERSION);
            }
            if (!this->_response.hasHeader ("Connection"))
            {
                if (this->_max && compareNoCase (this->_request.header ("Connection"), "keep-alive"))
                {
                    std::stringstream keepAlive;
                    keepAlive << "timeout=" << this->_server->keepAliveTimeout ().count () << ", max=" << this->_server->keepAliveMax ();
                    this->_response.header ("Keep-Alive", keepAlive.str ());
                    this->_response.header ("Connection", "Keep-Alive");
                }
                else
                {
                    this->_response.header ("Connection", "close");
                    this->_max = 0;
                }
            }
            if (this->encrypted () && !this->_response.hasHeader ("Strict-Transport-Security"))
            {
                this->_response.header ("Strict-Transport-Security", "max-age=31536000; includeSubDomains; preload");
            }
            if (!this->_response.hasHeader ("Content-Security-Policy"))
            {
                this->_response.header ("Content-Security-Policy", "default-src 'self'; object-src 'none'; script-src 'self'; style-src 'self'; img-src 'self'");
            }
            if (!this->_response.hasHeader ("X-XSS-Protection"))
            {
                this->_response.header ("X-XSS-Protection", "1; mode=block");
            }
            if (!this->_response.hasHeader ("X-Content-Type-Options"))
            {
                this->_response.header ("X-Content-Type-Options", "nosniff");
            }
            if (!this->_response.hasHeader ("X-Frame-Options"))
            {
                this->_response.header ("X-Frame-Options", "SAMEORIGIN");
            }

            // write response headers.
            this->_response.writeHeaders (*this);

            // set encoding.
            if (this->_response.hasHeader ("Transfer-Encoding"))
            {
                this->setEncoding (join::rsplit (this->_response.header ("Transfer-Encoding"), ","));
            }
            if (this->_response.hasHeader ("Content-Encoding"))
            {
                this->setEncoding (join::rsplit (this->_response.header ("Content-Encoding"), ","));
            }
        }

        /**
         * @brief send error message.
         * @param status status.
         * @param reason reason.
         */
        void sendError (const std::string& status, const std::string& reason)
        {
            // set error response.
            this->_response.response (status, reason);

            // stop keepalive.
            this->_response.header ("Connection", "close");
            this->_max = 0;

            // send headers.
            this->sendHeaders ();

            // flush data.
            this->flush ();
        }

        /**
         * @brief send redirect message.
         * @param status status.
         * @param reason reason.
         * @param location location to redirect to.
         */
        void sendRedirect (const std::string& status, const std::string& reason, const std::string& location = {})
        {
            std::string payload;

            // set redirect response.
            this->_response.response (status, reason);

            // set redirect message payload.
            if (!location.empty ())
            {
                payload += "<html>";
                payload += "<head>";
                payload += "<meta http-equiv=\"content-type\" content=\"text/html;charset=utf-8\">";
                payload += "<title>" + status + " " + reason + "</title>";
                payload += "</head>";
                payload += "<body>";
                payload += "<h1>" + status + " " + reason + "</h1>";
                payload += "The document has moved <a href=\"" + location + "\">here</a>";
                payload += "</body>";
                payload += "</html>";
            }

            // set content.
            if (payload.size ())
            {
                this->_response.header ("Content-Length", std::to_string (payload.size ()));
                this->_response.header ("Content-Type", "text/html");
                this->_response.header ("Cache-Control", "no-cache");
            }

            // send headers.
            this->sendHeaders ();

            // send payload.
            if (payload.size ())
            {
                this->write (payload.c_str (), payload.size ());
            }

            // flush data.
            this->flush ();
        }

        /**
         * @brief send a file.
         * @param path path of the file.
         */
        void sendFile (const std::string& path)
        {
            struct stat sbuf;

            // get file.
            void* addr = this->_server->_cache.get (path, sbuf);
            if (addr == nullptr || S_ISDIR (sbuf.st_mode))
            {
                this->sendError ("404", "Not Found");
                return;
            }

            // check modif time.
            std::stringstream modifTime;
            modifTime << std::put_time (std::gmtime (&sbuf.st_ctime), "%a, %d %b %Y %H:%M:%S GMT");
            if (compareNoCase (this->_request.header ("If-Modified-Since"), modifTime.str ()))
            {
                this->sendRedirect ("304", "Not Modified");
                return;
            }

            // set modif time.
            this->_response.header ("Last-Modified", modifTime.str ());

            // set content.
            this->_response.header ("Content-Length", std::to_string (sbuf.st_size));
            this->_response.header ("Content-Type", join::mime (path));
            this->_response.header ("Cache-Control", "no-cache");

            // send headers.
            this->sendHeaders ();

            // check method.
            if (this->_request.method () == HttpMethod::Get)
            {
                // send file.
                this->write (static_cast <char *> (addr), sbuf.st_size);
            }

            // flush data.
            this->flush ();
        }

        /**
         * @brief checks if there is a HTTP request header with the specified name.
         * @param name name of the HTTP request header to search for.
         * @return true of there is such a HTTP request header, false otherwise.
         */
        bool hasHeader (const std::string& name) const
        {
            return this->_request.hasHeader (name);
        }

        /**
         * @brief get HTTP request header by name.
         * @param name header name.
         * @return header value.
         */
        std::string header (const std::string& name) const
        {
            return this->_request.header (name);
        }

        /**
         * @brief get content length.
         * @return content length.
         */
        size_t contentLength () const
        {
            return this->_request.contentLength ();
        }

        /**
         * @brief add header to the HTTP response.
         * @param name header name.
         * @param val header value.
         */
        void header (const std::string& name, const std::string& val)
        {
            return this->_response.header (name, val);
        }

    protected:
        /**
         * @brief worker thread routine.
         */
        void work ()
        {
            fd_set setfd;
            FD_ZERO (&setfd);
            int fdmax = -1;

            FD_SET (this->_server->_event, &setfd);
            fdmax = std::max (fdmax, this->_server->_event);
            FD_SET (this->_server->_acceptor.handle (), &setfd);
            fdmax = std::max (fdmax, this->_server->_acceptor.handle ());

            for (;;)
            {
                {
                    ScopedLock lock (this->_server->_mutex);

                    fd_set fdset = setfd;
                    int nset = ::select (fdmax + 1, &fdset, nullptr, nullptr, nullptr);
                    if (nset > 0)
                    {
                        if (FD_ISSET (this->_server->_event, &fdset))
                        {
                            uint64_t val = 0;
                            [[maybe_unused]] ssize_t bytes = ::read (this->_server->_event, &val, sizeof (uint64_t));
                            return;
                        }

                        if (FD_ISSET (this->_server->_acceptor.handle (), &fdset))
                        {
                            this->_sockbuf.socket () = this->_server->accept ();
                            this->_sockbuf.timeout (this->_server->keepAliveTimeout ().count () * 1000);
                        }
                    }
                }

                this->processRequest ();
            }
        }

        /**
         * @brief process the HTTP request.
         */
        void processRequest ()
        {
            this->_max = this->_server->keepAliveMax ();

            do
            {
                if (this->readRequest () == -1)
                {
                    this->cleanUp ();
                    break;
                }

                this->writeResponse ();
                this->cleanUp ();
            }
            while ((this->_max < 0) || (--this->_max != 0));

            this->endRequest ();
        }

        /**
         * @brief read the HTTP request.
         * @return 0 on success, -1 on failure.
         */
        int readRequest ()
        {
            // restore concrete stream.
            this->clearEncoding ();

            // prepare a standard response.
            this->_response.response ("200", "OK");

            // read request headers.
            if (this->_request.readHeaders (*this) == -1)
            {
                if (join::lastError == HttpErrc::BadRequest)
                {
                    this->sendError ("400", "Bad Request");
                }
                else if (join::lastError == HttpErrc::Unsupported)
                {
                    this->sendError ("405", "Method Not Allowed");
                }
                else if (join::lastError == HttpErrc::HeaderTooLarge)
                {
                    this->sendError ("494", "Request Header Too Large");
                }
                return -1;
            }

            // check host.
            if (this->_request.host ().empty ())
            {
                this->sendError ("400", "Bad Request");
                return -1;
            }

            // set encoding.
            if (this->_request.hasHeader ("Transfer-Encoding"))
            {
                this->setEncoding (join::rsplit (this->_request.header ("Transfer-Encoding"), ","));
            }
            if (this->_request.hasHeader ("Content-Encoding"))
            {
                this->setEncoding (join::rsplit (this->_request.header ("Content-Encoding"), ","));
            }

            return 0;
        }

        /**
         * @brief write the HTTP response.
         */
        void writeResponse ()
        {
            Content* content = this->_server->findContent (this->_request.method (), this->_request.path ());
            if (content == nullptr)
            {
                this->sendError ("404", "Not Found");
                return;
            }

            if (content->access != nullptr)
            {
                if (!this->_request.hasHeader ("Authorization"))
                {
                    this->sendError ("401", "Unauthorized");
                    return;
                }

                std::error_code err;
                if (!content->access (this->_request.auth (), this->_request.credentials (), err))
                {
                    if (err == HttpErrc::Unauthorized)
                    {
                        this->sendError ("401", "Unauthorized");
                    }
                    else if (err == HttpErrc::Forbidden)
                    {
                        this->sendError ("403", "Forbidden");
                    }
                    return;
                }
            }

            std::string alias (content->alias);
            if (!alias.empty ())
            {
                join::replaceAll (alias, "$root", this->_server->baseLocation ());
                join::replaceAll (alias, "$scheme", this->_server->scheme ());
                join::replaceAll (alias, "$host", this->_request.host ());
                join::replaceAll (alias, "$port", std::to_string (this->localEndpoint ().port ()));
                join::replaceAll (alias, "$path", this->_request.path ());
                join::replaceAll (alias, "$query", this->_request.query ());
                join::replaceAll (alias, "$urn", this->_request.urn ());
            }

            if (content->type == HttpContentType::Root)
            {
                this->sendFile (this->_server->baseLocation () + this->_request.path ());
            }
            else if (content->type == HttpContentType::Alias)
            {
                this->sendFile (alias);
            }
            else if (content->type == HttpContentType::Exec)
            {
                if (content->handler == nullptr)
                {
                    this->sendError ("500", "Internal Server Error");
                    return;
                }
                content->handler (this);
            }
            else if (content->type == HttpContentType::Redirect)
            {
                if (this->_request.version () == "HTTP/1.1")
                {
                    this->sendRedirect ("307", "Temporary Redirect", alias);
                }
                else
                {
                    this->sendRedirect ("302", "Found", alias);
                }
            }
        }

        /**
         * @brief clean all.
         */
        void cleanUp ()
        {
            this->_request.clear ();
            this->_response.clear ();
        }

        /**
         * @brief end the HTTP request.
         */
        void endRequest ()
        {
            this->disconnect ();
            this->close ();
        }

        /**
         * @brief set stream encoding.
         * @param encodings encodings applied to the stream.
         */
        void setEncoding (const std::vector <std::string>& encodings)
        {
            for (auto const& encoding : encodings)
            {
                if (encoding.find ("gzip") != std::string::npos)
                {
                    this->_streambuf = new Zstreambuf (this->_streambuf, Zstream::Gzip, this->_wrapped);
                    this->_wrapped = true;
                }
                else if (encoding.find ("deflate") != std::string::npos)
                {
                    this->_streambuf = new Zstreambuf (this->_streambuf, Zstream::Deflate, this->_wrapped);
                    this->_wrapped = true;
                }
                else if (encoding.find ("chunked") != std::string::npos)
                {
                    this->_streambuf = new Chunkstreambuf (this->_streambuf, this->_wrapped);
                    this->_wrapped = true;
                }
            }

            this->set_rdbuf (this->_streambuf);
        }

        /**
         * @brief clear stream encoding.
         */
        void clearEncoding ()
        {
            if (this->_wrapped && this->_streambuf)
            {
                delete this->_streambuf;
                this->_streambuf = nullptr;
            }

            this->_streambuf = &this->_sockbuf;
            this->_wrapped = false;

            this->set_rdbuf (this->_streambuf);
        }

        /// max requests.
        int _max = 0;

        /// HTTP request.
        HttpRequest _request;

        /// HTTP response.
        HttpResponse _response;

        /// HTTP stream buffer.
        std::streambuf* _streambuf = nullptr;

        /// HTTP stream status.
        bool _wrapped = false;

        /// HTTP server.
        Server* _server;

        /// thread.
        Thread _thread;
    };

    /**
     * @brief basic HTTP server.
     */
    template <class Protocol> 
    class BasicHttpServer
    {
    public:
        using Worker   = BasicHttpWorker <Protocol>;
        using Content  = BasicHttpContent <Protocol>;
        using Handler  = typename Content::Handler;
        using Access   = typename Content::Access;
        using Endpoint = typename Protocol::Endpoint;
        using Socket   = typename Protocol::Socket;
        using Acceptor = typename Protocol::Acceptor;

        /**
         * @brief create the HTTP server instance.
         * @param workers number of worker threads.
         */
        BasicHttpServer (size_t workers = std::thread::hardware_concurrency ())
        : _event (eventfd (0, EFD_NONBLOCK | EFD_CLOEXEC | EFD_SEMAPHORE)),
          _nworkers (workers),
          _baseLocation ("/var/www"),
          _keepTimeout (10)
        {
            [[maybe_unused]] int res = chdir (this->_baseLocation.c_str ());
        }

        /**
         * @brief create instance by copy.
         * @param other object to copy.
         */
        BasicHttpServer (const BasicHttpServer& other) = delete;

        /**
         * @brief assign instance by copy.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        BasicHttpServer& operator= (const BasicHttpServer& other) = delete;

        /**
         * @brief create instance by move.
         * @param other object to move.
         */
        BasicHttpServer (BasicHttpServer&& other) = delete;

        /**
         * @brief assign instance by move.
         * @param other object to move.
         * @return a reference of the current object.
         */
        BasicHttpServer& operator= (BasicHttpServer&& other) = delete;

        /**
         * @brief destroy the HTTP server.
         */
        virtual ~BasicHttpServer ()
        {
            this->_acceptor.close ();
            this->_contents.clear ();
            ::close (this->_event);
        }

        /**
         * @brief create server.
         * @param endpoint endpoint to assign to the server.
         * @return 0 on success, -1 on failure.
         */
        int create (const Endpoint& endpoint) noexcept
        {
            if (this->_acceptor.create (endpoint) == -1)
            {
                return -1;
            }

            for (size_t nworkers = 0; nworkers < this->_nworkers; ++nworkers)
            {
                this->_workers.emplace_back (new Worker (this));
            }

            return 0;
        }

        /**
         * @brief close server.
         */
        void close () noexcept
        {
            uint64_t val = this->_nworkers;
            [[maybe_unused]] ssize_t bytes = ::write (this->_event, &val, sizeof (uint64_t));
            this->_workers.clear ();
            this->_acceptor.close ();
        }

        /**
         * @brief accept new connection.
         * @return the accepted client socket object.
         */
        virtual Socket accept () const
        {
            return this->_acceptor.accept ();
        }

        /**
         * @brief set file base location.
         * @param path file base location path.
         */
        void baseLocation (const std::string& path)
        {
            this->_baseLocation = path;

            if (*this->_baseLocation.rbegin () == '/')
            {
                this->_baseLocation.pop_back ();
            }

            [[maybe_unused]] int res = chdir (this->_baseLocation.c_str ());
        }

        /**
         * @brief get file base location.
         * @return file base location path.
         */
        const std::string& baseLocation () const
        {
            return this->_baseLocation;
        }

        /**
         * @brief set HTTP keep alive.
         * @param timeout keep alive timeout (default 10 secs).
         * @param max keep alive max (default 1000).
         */
        void keepAlive (std::chrono::seconds timeout, int max = 1000)
        {
            this->_keepTimeout = timeout;
            this->_keepMax = max;
        }

        /**
         * @brief get HTTP keep alive timeout.
         * @return HTTP keep alive timeout.
         */
        std::chrono::seconds keepAliveTimeout () const
        {
            return this->_keepTimeout;
        }

        /**
         * @brief get HTTP keep alive max.
         * @return HTTP keep alive max.
         */
        int keepAliveMax () const
        {
            return this->_keepMax;
        }

        /**
         * @brief get scheme.
         * @return htpp or https.
         */
        virtual std::string scheme () const
        {
            return "http";
        }

        /**
         * @brief map an URL to filesystem adding URL path to the base location.
         * @param dir directory.
         * @param name file name.
         * @param access access handler.
         * @return a pointer to the content on success, nullptr on failure.
         */
        Content* addDocumentRoot (const std::string& dir, const std::string& name, const Access& access = nullptr)
        {
            Content* newEntry = new Content;
            if (newEntry != nullptr)
            {
                newEntry->methods   = Head | Get;
                newEntry->type      = Root;
                newEntry->directory = dir;
                newEntry->name      = name;
                newEntry->handler   = nullptr;
                newEntry->access    = access;
                this->_contents.emplace_back (newEntry);
            }

            return newEntry;
        }

        /**
         * @brief map an URL to filesystem replacing URL path by the specified path.
         * @param dir directory.
         * @param name file name.
         * @param alias corresponding file system path.
         * @param access access handler.
         * @return a pointer to the content on success, nullptr on failure.
         */
        Content* addAlias (const std::string& dir, const std::string& name, const std::string& alias, const Access& access = nullptr)
        {
            Content* newEntry = new Content;
            if (newEntry != nullptr)
            {
                newEntry->methods   = Head | Get;
                newEntry->type      = Alias;
                newEntry->directory = dir;
                newEntry->name      = name;
                newEntry->alias     = alias;
                newEntry->handler   = nullptr;
                newEntry->access    = access;
                this->_contents.emplace_back (newEntry);
            }

            return newEntry;
        }

        /**
         * @brief map an URL to a callback.
         * @param methods allowed methods.
         * @param dir directory.
         * @param name file name.
         * @param handler content handler.
         * @param access access handler.
         * @return a pointer to the content on success, nullptr on failure.
         */
        Content* addExecute (const HttpMethod methods, const std::string& dir, const std::string& name, const Handler& handler, const Access& access = nullptr)
        {
            Content* newEntry = new Content;
            if (newEntry != nullptr)
            {
                newEntry->methods   = methods;
                newEntry->type      = Exec;
                newEntry->directory = dir;
                newEntry->name      = name;
                newEntry->handler   = handler;
                newEntry->access    = access;
                this->_contents.emplace_back (newEntry);
            }

            return newEntry;
        }

        /**
         * @brief map an URL to a redirection.
         * @param dir directory.
         * @param name file name.
         * @param location location where to do the redirection.
         * @param access access handler.
         * @return a pointer to the content on success, nullptr on failure.
         */
        Content* addRedirect (const std::string& dir, const std::string& name, const std::string& location, const Access& access = nullptr)
        {
            Content* newEntry = new Content;
            if (newEntry != nullptr)
            {
                newEntry->methods   = Head | Get | Put | Post | Delete;
                newEntry->type      = Redirect;
                newEntry->directory = dir;
                newEntry->name      = name;
                newEntry->alias     = location;
                newEntry->handler   = nullptr;
                newEntry->access    = access;
                this->_contents.emplace_back (newEntry);
            }

            return newEntry;
        }

    protected:
        /**
         * @brief find content.
         * @param method method.
         * @param path resource path.
         * @return a pointer to the content on success, nullptr on failure.
         */
        Content* findContent (HttpMethod method, const std::string& path) const
        {
            std::string directory = join::base (path);
            std::string name      = join::filename (path);

            for (auto const& content : this->_contents)
            {
                if (content->methods & method)
                {
                    if (fnmatch (content->directory.c_str (), directory.c_str (), FNM_CASEFOLD) == 0)
                    {
                        if (fnmatch (content->name.c_str (), name.c_str (), FNM_CASEFOLD) == 0)
                        {
                            return content.get ();
                        }
                    }
                }
            }

            return nullptr;
        }

        /// acceptor.
        Acceptor _acceptor;

        /// gracefully stop all workers.
        int _event = -1;

        /// number of workers.
        size_t _nworkers;

        /// workers.
        std::vector <std::unique_ptr <Worker>> _workers;

        /// accept protection mutex.
        Mutex _mutex;

        /// contents.
        std::vector <std::unique_ptr <Content>> _contents;

        /// base location.
        std::string _baseLocation;

        /// keep alive timeout.
        std::chrono::seconds _keepTimeout;

        /// keep alive max.
        int _keepMax = 1000;

        /// file cache.
        Cache _cache;

        /// friendship with worker.
        friend Worker;
    };

    /**
     * @brief basic HTTPS server.
     */
    template <class Protocol> 
    class BasicHttpSecureServer : public BasicHttpServer <Protocol>
    {
    public:
        using Worker   = BasicHttpWorker <Protocol>;
        using Content  = BasicHttpContent <Protocol>;
        using Handler  = typename Content::Handler;
        using Access   = typename Content::Access;
        using Endpoint = typename Protocol::Endpoint;
        using Socket   = typename Protocol::Socket;
        using Acceptor = typename Protocol::Acceptor;

        /**
         * @brief create the HTTPS server instance.
         * @param workers number of worker threads.
         */
        BasicHttpSecureServer (size_t workers = std::thread::hardware_concurrency ())
        : BasicHttpServer <Protocol> (workers)
        {
        }

        /**
         * @brief create instance by copy.
         * @param other object to copy.
         */
        BasicHttpSecureServer (const BasicHttpSecureServer& other) = delete;

        /**
         * @brief assign instance by copy.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        BasicHttpSecureServer& operator= (const BasicHttpSecureServer& other) = delete;

        /**
         * @brief create instance by move.
         * @param other object to move.
         */
        BasicHttpSecureServer (BasicHttpSecureServer&& other) = delete;

        /**
         * @brief assign instance by move.
         * @param other object to move.
         * @return a reference of the current object.
         */
        BasicHttpSecureServer& operator= (BasicHttpSecureServer&& other) = delete;

        /**
         * @brief destroy the HTTPS server.
         */
        virtual ~BasicHttpSecureServer () = default;

        /**
         * @brief accept new connection and fill in the client object with connection parameters.
         * @return the accepted client socket object.
         */
        Socket accept () const override
        {
            return this->_acceptor.acceptEncrypted ();
        }

        /**
         * @brief set the certificate and the private key.
         * @param cert certificate path.
         * @param key private key path.
         * @return 0 on success, -1 on failure.
         */
        int setCertificate (const std::string& cert, const std::string& key = "")
        {
            return this->_acceptor.setCertificate (cert, key);
        }

        /**
         * @brief Set the location of the trusted CA certificate.
         * @param caFile path of the trusted CA certificate file.
         * @return 0 on success, -1 on failure.
         */
        int setCaCertificate (const std::string& caFile)
        {
            return this->_acceptor.setCaCertificate (caFile);
        }

        /**
         * @brief Enable/Disable the verification of the peer certificate.
         * @param verify Enable peer verification if set to true, false otherwise.
         * @param depth The maximum certificate verification depth (default: no limit).
         */
        void setVerify (bool verify, int depth = -1)
        {
            return this->_acceptor.setVerify (verify, depth);
        }

        /**
         * @brief set the cipher list (TLSv1.2 and below).
         * @param cipher the cipher list.
         * @return 0 on success, -1 on failure.
         */
        int setCipher (const std::string& cipher)
        {
            return this->_acceptor.setCipher (cipher);
        }

        /**
         * @brief set the cipher list (TLSv1.3).
         * @param cipher the cipher list.
         * @return 0 on success, -1 on failure.
         */
        int setCipher_1_3 (const std::string &cipher)
        {
            return this->_acceptor.setCipher_1_3 (cipher);
        }

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
        /**
         * @brief set curve list.
         * @param curves curve list.
         * @return 0 on success, -1 on failure.
         */
        int setCurve (const std::string &curves)
        {
            return this->_acceptor.setCurve (curves);
        }
#endif

        /**
         * @brief get scheme.
         * @return htpp or https.
         */
        virtual std::string scheme () const override
        {
            return "https";
        }

        /// friendship with worker.
        friend Worker;
    };
}

#endif
