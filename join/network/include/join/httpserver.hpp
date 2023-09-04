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
#include <join/condition.hpp>
#include <join/acceptor.hpp>
#include <join/version.hpp>
#include <join/thread.hpp>
#include <join/cache.hpp>

// C++.
#include <thread>
#include <memory>
#include <vector>
#include <queue>

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
        Proxy,          /**< Proxy pass. */
        Redirect,       /**< Redirection. */
        Upload,         /**< Upload content. */
    };

    template <class Protocol>
    using BasicAccessHandler  = std::function <bool (BasicWorker <Protocol>*, std::error_code&)>;

    template <class Protocol>
    using BasicContentHandler = std::function <void (BasicWorker <Protocol>*)>;

    /**
     * @brief HTTP content.
     */
    template <class Protocol>
    struct BasicContent
    {
        using ContentHandler = BasicContentHandler <Protocol>;
        using AccessHandler  = BasicAccessHandler  <Protocol>;

        HttpMethod      methods;            /**< allowed methods. */
        HttpContentType type;               /**< content type (root, alias etc...). */
        std::string     directory;          /**< requested resource directory. */
        std::string     name;               /**< requested resource file name. */
        std::string     alias;              /**< mapped file system path. */
        ContentHandler  contentHandler;     /**< mapped content handler. */
        AccessHandler   accessHandler;      /**< access handler. */
    };

    /**
     * @brief HTTP worker thread.
     */
    template <class Protocol>
    class BasicWorker
    {
    public:
        using Content = BasicContent <Protocol>;
        using Server  = BasicHttpServer <Protocol>;
        using Stream  = typename Protocol::Stream;

        /**
         * @brief Create the worker instance.
         * @param server Server instance.
         */
        BasicWorker (Server* server)
        : _server (server),
          _thread ([this] () {work ();})
        {}

        /**
         * @brief create instance by copy.
         * @param other object to copy.
         */
        BasicWorker (const BasicWorker& other) = delete;

        /**
         * @brief assign instance by copy.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        BasicWorker& operator= (const BasicWorker& other) = delete;

        /**
         * @brief create instance by move.
         * @param other object to move.
         */
        BasicWorker (BasicWorker&& other) = delete;

        /**
         * @brief assign instance by move.
         * @param other object to move.
         * @return a reference of the current object.
         */
        BasicWorker& operator= (BasicWorker&& other) = delete;

        /**
         * @brief destroy worker thread.
         */
        virtual ~BasicWorker ()
        {
            this->_thread.join ();
        }

    protected:
        /**
         * @brief worker thread routine.
         */
        void work ()
        {
            for (;;)
            {
                {
                    ScopedLock lock (this->_server->_mutex);

                    this->_server->_condition.wait (lock, [this] () { return !this->_server->_work || !this->_server->_streams.empty (); });
                    if (!this->_server->_work && this->_server->_streams.empty ())
                    {
                        return;
                    }

                    this->_stream = std::move (this->_server->_streams.front ());
                    this->_server->_streams.pop ();
                }

                this->_max = this->_server->keepAliveMax ();

                do
                {
                    if (this->readRequest () == -1)
                    {
                        this->cleanUp ();
                        break;
                    }

                    this->processRequest ();
                    this->cleanUp ();
                }
                while ((this->_max < 0) || (--this->_max != 0));

                this->endRequest ();
            }
        }

        /**
         * @brief read the HTTP request.
         * @return 0 on success, -1 on failure.
         */
        int readRequest ()
        {
            this->_response.response ("200", "ok");
            this->_request.readHeaders (this->_stream);
            if (this->_stream.fail ())
            {
                if (!this->_stream.eof ())
                {
                    this->sendError (std::to_string (join::lastError.value ()), join::lastError.message ());
                }
                return -1;
            }
            if (!this->_request.hasHeader ("Host"))
            {
                this->sendError ("400", "bad request");
                return -1;
            }
            return 0;
        }

        /**
         * @brief process the HTTP request.
         */
        void processRequest ()
        {
            Content* content = this->_server->findContent (this->_request.method (), this->_request.path ());
            if (content == nullptr)
            {
                this->sendError ("404", "not found");
                return;
            }

            if (content->accessHandler != nullptr)
            {
                if (!this->_request.hasHeader ("Authorization"))
                {
                    this->sendError ("401", "unauthorized");
                    return;
                }

                std::error_code error;
                if (!content->accessHandler (this, error))
                {
                    this->sendError (std::to_string (error.value ()), error.message ());
                    return;
                }
            }

            switch (content->type)
            {
                case HttpContentType::Root:
                    this->sendFile (this->_server->baseLocation () + this->_request.path ());
                    break;

                case HttpContentType::Alias:
                    {
                        std::string alias (content->alias);
                        join::replaceAll (alias, "$root", this->_server->baseLocation ());
                        join::replaceAll (alias, "$scheme", this->_server->scheme ());
                        //join::replaceAll (alias, "$host", this->_request.header ("Host"));
                        join::replaceAll (alias, "$port", std::to_string (this->_stream.remoteEndpoint ().port ()));
                        join::replaceAll (alias, "$path", this->_request.path ());
                        join::replaceAll (alias, "$query", this->_request.query ());
                        join::replaceAll (alias, "$urn", this->_request.urn ());
                        this->sendFile (alias);
                    }
                    break;

                case HttpContentType::Exec:
                    if (content->contentHandler == nullptr)
                    {
                        this->sendError ("500", "internal server error");
                        return;
                    }
                    content->contentHandler (this);
                    break;

                case HttpContentType::Proxy:
                    {
                        std::string remote (content->alias);
                        join::replaceAll (remote, "$scheme", this->_server->scheme ());
                        //join::replaceAll (remote, "$host", this->_request.header ("Host"));
                        join::replaceAll (remote, "$port", std::to_string (this->_stream.remoteEndpoint ().port ()));
                        join::replaceAll (remote, "$path", this->_request.path ());
                        join::replaceAll (remote, "$query", this->_request.query ());
                        join::replaceAll (remote, "$urn", this->_request.urn ());
                        //proxyPass (remote);
                    }
                    break;

                case HttpContentType::Redirect:
                    {
                        std::string location (content->alias);
                        join::replaceAll (location, "$scheme", this->_server->scheme ());
                        //join::replaceAll (location, "$host", this->_request.header ("Host"));
                        join::replaceAll (location, "$port", std::to_string (this->_stream.remoteEndpoint ().port ()));
                        join::replaceAll (location, "$path", this->_request.path ());
                        join::replaceAll (location, "$query", this->_request.query ());
                        join::replaceAll (location, "$urn", this->_request.urn ());
                        if (this->_request.version () == "HTTP/1.1")
                        {
                            this->sendRedirect ("307", "temporary redirect", location);
                        }
                        else
                        {
                            this->sendRedirect ("302", "Found", location);
                        }
                    }
                    break;

                case HttpContentType::Upload:
                    if (content->contentHandler == nullptr)
                    {
                        this->sendError ("500", "internal server error");
                        return;
                    }
                    //if (readMultipart ())
                    //{
                    //    content->contentHandler (this);
                    //}
                    break;

                default:
                    break;
            }
        }

        /**
         * @brief clean all.
         */
        void cleanUp ()
        {
            this->_request  = HttpRequest ();
            this->_response = HttpResponse ();
        }

        /**
         * @brief end the HTTP request.
         */
        void endRequest ()
        {
            this->_stream.disconnect ();
            this->_stream.close ();
        }

        /**
         * @brief send headers.
         * @return 0 on success, -1 on failure.
         */
        int sendHeaders ()
        {
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
            if (!this->_response.hasHeader ("Content-Length"))
            {
                this->_response.header ("Transfer-Encoding", "chunked");
            }
            else if (!this->_response.hasHeader ("Cache-Control"))
            {
                this->_response.header ("Cache-Control", "no-cache");
            }
            if (!this->_response.hasHeader ("Connection"))
            {
                if (this->_server->_work && this->_max && compareNoCase (this->_request.header ("Connection"), "keep-alive"))
                {
                    std::stringstream keepAlive;
                    keepAlive << "timeout=" << this->_server->_keepTimeout.count () << ", max=" << this->_server->_keepMax;
                    this->_response.header ("Keep-Alive", keepAlive.str ());
                    this->_response.header ("Connection", "Keep-Alive");
                }
                else
                {
                    this->_response.header ("Connection", "close");
                    this->_max = 0;
                }
            }
            if (this->_stream.encrypted () && !this->_response.hasHeader ("Strict-Transport-Security"))
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
            this->_response.writeHeaders (this->_stream);
            if (this->_stream.fail ())
            {
                return -1;
            }
            this->_stream.flush ();

            //this->writeAccessLog ();

            return 0;
        }

        /**
         * @brief send error message.
         * @param status status.
         * @param reason reason.
         * @return 0 on success, -1 on failure.
         */
        int sendError (const std::string& status, const std::string& reason, const std::string& payload = {})
        {
            this->_response.response (status, reason);

            if (!payload.empty ())
            {
                this->_response.header ("Content-Length", std::to_string (payload.size ()));
            }

            if (this->sendHeaders () == -1)
            {
                return -1;
            }

            if (!payload.empty ())
            {
                this->_stream << payload;
            }

            if (this->_stream.fail ())
            {
                return -1;
            }

            return 0;
        }

        /**
         * @brief send redirect message.
         * @param status status.
         * @param reason reason.
         * @param location location to redirect to.
         * @return 0 on success, -1 on failure.
         */
        int sendRedirect (const std::string& status, const std::string& reason, const std::string& location = {})
        {
            std::string payload;

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

                this->_response.header ("Content-Type", "text/html");
            }

            return sendError (status, reason, payload);
        }

        /**
         * @brief send a file.
         * @param path path of the file.
         */
        void sendFile (std::string path)
        {
            struct stat sbuf;

            if ((stat (path.c_str (), &sbuf) < 0) || S_ISDIR (sbuf.st_mode))
            {
                this->sendError ("404", "not found");
                return;
            }

            this->_response.header ("Content-Type", this->mime (path));
            this->_response.header ("Content-Length", std::to_string (sbuf.st_size));

            std::stringstream modifTime;
            modifTime << std::put_time (std::gmtime (&sbuf.st_ctime), "%a, %d %b %Y %H:%M:%S GMT");
            if (compareNoCase (this->_request.header ("If-Modified-Since"), modifTime.str ()))
            {
                this->sendRedirect ("304", "not modified");
                return;
            }

            this->_response.header ("Last-Modified", modifTime.str ());

            if (this->sendHeaders () == -1)
            {
                return;
            }

            if (this->_request.method () == HttpMethod::Head)
            {
                return;
            }

            void * addr = this->_server->_cache.get (path, &sbuf);
            if (addr == nullptr)
            {
                return;
            }

            this->_stream.write (static_cast <char *> (addr), sbuf.st_size);
        }

        /**
         * @brief returns the extension of the specified file.
         * @param filepath path of the file to parse.
         * @return the extension of the file specified.
         */
        std::string ext (const std::string& filepath) const
        {
            size_t pos = filepath.rfind (".");

            if (pos == std::string::npos)
                return {};
            else
                return filepath.substr (pos + 1);
        }

        /**
         * @brief returns the mime type of the file specified.
         * @param filepath Path of the file to examine.
         * @return The mime type of the file specified.
         */
        std::string mime (const std::string& filepath) const
        {
            std::string mime, suffix (this->ext (filepath)) ;

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

        /// max requests.
        size_t _max = 0;

        /// request.
        HttpRequest _request;

        /// response.
        HttpResponse _response;

        /// stream;
        Stream _stream;

        /// server.
        Server* _server;

        /// thread.
        Thread _thread;
    };

    /**
     * @brief Basic HTTP server.
     */
    template <class Protocol> 
    class BasicHttpServer : public Protocol::Acceptor
    {
    public:
        using Content        = BasicContent <Protocol>;
        using ContentHandler = BasicContentHandler <Protocol>;
        using AccessHandler  = BasicAccessHandler <Protocol>;
        using Worker         = BasicWorker <Protocol>;
        using Endpoint       = typename Protocol::Endpoint;
        using Stream         = typename Protocol::Stream;
        using Acceptor       = typename Protocol::Acceptor;

        /**
         * @brief create the HTTP server instance.
         * @param workers number of worker threads.
         */
        BasicHttpServer (size_t workers = std::thread::hardware_concurrency () + 1)
        : _work (false),
          _nworkers (workers),
          _baseLocation ("/var/www"),
          _uploadLocation ("/tmp/upload"),
          _keepTimeout (10)
        {
            chdir (this->_baseLocation.c_str ());
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
            this->close ();
            this->_contents.clear ();
        }

        /**
         * @brief create server.
         * @param endpoint endpoint to assign to the server.
         * @return 0 on success, -1 on failure.
         */
        virtual int create (const Endpoint& endpoint) noexcept override
        {
            this->_work = true;

            for (size_t nworkers = 0; nworkers < this->_nworkers; ++nworkers)
            {
                this->_workers.emplace_back (new Worker (this));
            }

            if (Acceptor::create (endpoint) == -1)
            {
                this->close ();
                return -1;
            }

            if (Reactor::instance ()->addHandler (this) == -1)
            {
                this->close ();
                return -1;
            }

            return 0;
        }

        /**
         * @brief close server.
         */
        virtual void close () noexcept override
        {
            Reactor::instance ()->delHandler (this);
            Acceptor::close ();
            this->_work = false;
            this->_condition.broadcast ();
            this->_workers.clear ();
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

            chdir (this->_baseLocation.c_str ());
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
         * @brief set upload location.
         * @param path upload location path.
         */
        void uploadLocation (const std::string& path)
        {
            this->_uploadLocation = path;

            if (*this->_uploadLocation.rbegin () == '/')
            {
                this->_uploadLocation.pop_back ();
            }
        }

        /**
         * @brief get upload location.
         * @return upload location path.
         */
        const std::string& uploadLocation () const
        {
            return this->_uploadLocation;
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
         * @param accessHandler access handler.
         * @return a pointer to the content on success, nullptr on failure.
         */
        Content* addDocumentRoot (const std::string& dir, const std::string& name, const AccessHandler& accessHandler = nullptr)
        {
            Content* newEntry = new Content;
            if (newEntry != nullptr)
            {
                newEntry->methods        = Head | Get;
                newEntry->type           = Root;
                newEntry->directory      = dir;
                newEntry->name           = name;
                newEntry->contentHandler = nullptr;
                newEntry->accessHandler  = accessHandler;
                this->_contents.emplace_back (newEntry);
            }

            return newEntry;
        }

        /**
         * @brief map an URL to filesystem replacing URL path by the specified path.
         * @param dir directory.
         * @param name file name.
         * @param alias corresponding file system path.
         * @param accessHandler access handler.
         * @return a pointer to the content on success, nullptr on failure.
         */
        Content* addAlias (const std::string& dir, const std::string& name, const std::string& alias, const AccessHandler& accessHandler = nullptr)
        {
            Content* newEntry = new Content;
            if (newEntry != nullptr)
            {
                newEntry->methods        = Head | Get;
                newEntry->type           = Alias;
                newEntry->directory      = dir;
                newEntry->name           = name;
                newEntry->alias          = alias;
                newEntry->contentHandler = nullptr;
                newEntry->accessHandler  = accessHandler;
                this->_contents.emplace_back (newEntry);
            }

            return newEntry;
        }

        /**
         * @brief map an URL to a callback.
         * @param methods allowed methods.
         * @param dir directory.
         * @param name file name.
         * @param contentHandler content handler.
         * @param accessHandler access handler.
         * @return a pointer to the content on success, nullptr on failure.
         */
        Content* addExecute (const HttpMethod methods, const std::string& dir, const std::string& name, const ContentHandler& contentHandler, const AccessHandler& accessHandler = nullptr)
        {
            Content* newEntry = new Content;
            if (newEntry != nullptr)
            {
                newEntry->methods        = methods;
                newEntry->type           = Exec;
                newEntry->directory      = dir;
                newEntry->name           = name;
                newEntry->contentHandler = contentHandler;
                newEntry->accessHandler  = accessHandler;
                this->_contents.emplace_back (newEntry);
            }

            return newEntry;
        }

        /**
         * @brief map an URL to a remote server
         * @param dir directory.
         * @param name file name.
         * @param remote remote server.
         * @param accessHandler access handler.
         * @return a pointer to the content on success, nullptr on failure.
         */
        Content* addProxyPass (const std::string& dir, const std::string& name, const std::string& remote, const AccessHandler& accessHandler = nullptr)
        {
            Content* newEntry = new Content;
            if (newEntry != nullptr)
            {
                newEntry->methods        = Head | Get | Put | Post | Delete;
                newEntry->type           = Proxy;
                newEntry->directory      = dir;
                newEntry->name           = name;
                newEntry->alias          = remote;
                newEntry->contentHandler = nullptr;
                newEntry->accessHandler  = accessHandler;
                this->_contents.emplace_back (newEntry);
            }

            return newEntry;
        }

        /**
         * @brief map an URL to a redirection.
         * @param dir directory.
         * @param name file name.
         * @param location location where to do the redirection.
         * @param accessHandler access handler.
         * @return a pointer to the content on success, nullptr on failure.
         */
        Content* addRedirect (const std::string& dir, const std::string& name, const std::string& location, const AccessHandler& accessHandler = nullptr)
        {
            Content* newEntry = new Content;
            if (newEntry != nullptr)
            {
                newEntry->methods        = Head | Get | Put | Post | Delete;
                newEntry->type           = Redirect;
                newEntry->directory      = dir;
                newEntry->name           = name;
                newEntry->alias          = location;
                newEntry->contentHandler = nullptr;
                newEntry->accessHandler  = accessHandler;
                this->_contents.emplace_back (newEntry);
            }

            return newEntry;
        }

        /**
         * @brief map an URL allowed to upload file to a callback.
         * @param dir directory.
         * @param name file name.
         * @param contentHandler content handler.
         * @param accessHandler access handler.
         * @return a pointer to the content on success, nullptr on failure.
         */
        Content* addUpload (const std::string& dir, const std::string& name, const ContentHandler& contentHandler, const AccessHandler& accessHandler = nullptr)
        {
            Content* newEntry = new Content;
            if (newEntry != nullptr)
            {
                newEntry->methods        = Put | Post;
                newEntry->type           = Upload;
                newEntry->directory      = dir;
                newEntry->name           = name;
                newEntry->contentHandler = contentHandler;
                newEntry->accessHandler  = accessHandler;
                this->_contents.emplace_back (newEntry);
            }

            return newEntry;
        }

    protected:
        /**
         * @brief method called when data are ready to be read on handle.
         */
        virtual void onReceive () override
        {
            Stream stream = this->acceptStream ();

            if (stream.connected ())
            {
                stream.timeout (keepAliveTimeout ().count () * 1000);

                {
                    ScopedLock lock (this->_mutex);
                    this->_streams.emplace (std::move (stream));
                }

                this->_condition.signal ();
            }
        }

        /**
         * @brief find content.
         * @param method method.
         * @param path resource path.
         * @return a pointer to the content on success, nullptr on failure.
         */
        Content* findContent (HttpMethod method, const std::string& path) const
        {
            std::string directory = this->base (path);
            std::string name      = this->name (path);

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

        /**
         * @brief returns the base path of the specified file.
         * @param filepath path of the file to parse.
         * @return the base path of the file specified.
         */
        std::string base (const std::string& filepath) const
        {
            size_t pos = filepath.rfind ("/");

            if (pos == std::string::npos)
                return {};
            else
                return filepath.substr (0, pos + 1);
        }

        /**
         * @brief returns the file name of the specified file.
         * @param filepath path of the file to parse.
         * @return the file name of the file specified.
         */
        std::string name (const std::string& filepath) const
        {
            size_t pos = filepath.rfind ("/");

            if (pos == std::string::npos)
                return filepath;
            else
                return filepath.substr (pos + 1);
        }

        /// gracefully stop all workers.
        std::atomic <bool> _work;

        /// number of workers.
        size_t _nworkers;

        /// workers.
        std::vector <std::unique_ptr <Worker>> _workers;

        /// condition shared with workers.
        Condition _condition;

        /// condition protection mutex.
        Mutex _mutex;

        /// pending connected streams.
        std::queue <Stream> _streams;

        /// contents.
        std::vector <std::unique_ptr <Content>> _contents;

        /// base location.
        std::string _baseLocation;

        /// upload location.
        std::string _uploadLocation;

        /// keep alive timeout.
        std::chrono::seconds _keepTimeout;

        /// keep alive max.
        int _keepMax = 1000;

        /// file cache.
        Cache _cache;

        /// friendship with worker.
        friend class BasicWorker <Protocol>;
    };

    /**
     * @brief Basic HTTPS server.
     */
    template <class Protocol> 
    class BasicHttpSecureServer : public BasicHttpServer <Protocol>
    {
    public:
        using Content        = BasicContent <Protocol>;
        using ContentHandler = BasicContentHandler <Protocol>;
        using AccessHandler  = BasicAccessHandler <Protocol>;
        using Worker         = BasicWorker <Protocol>;
        using Endpoint       = typename Protocol::Endpoint;
        using Stream         = typename Protocol::Stream;
        using Acceptor       = typename Protocol::Acceptor;

        /**
         * @brief create the HTTPS server instance.
         * @param workers number of worker threads.
         */
        BasicHttpSecureServer (size_t workers = std::thread::hardware_concurrency () + 1)
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
         * @brief get scheme.
         * @return htpp or https.
         */
        virtual std::string scheme () const override
        {
            return "https";
        }

        /// friendship with worker.
        friend class BasicWorker <Protocol>;
    };
}

#endif
