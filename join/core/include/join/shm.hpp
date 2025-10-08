/**
 * MIT License
 *
 * Copyright (c) 2025 Mathieu Rabine
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

#ifndef __JOIN_SHM_HPP__
#define __JOIN_SHM_HPP__

// libjoin.
#include <join/acceptor.hpp>
#include <join/reactor.hpp>
#include <join/error.hpp>

// C++.
#include <string>
#include <array>

// C.
#include <sys/eventfd.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>

namespace join
{
    /**
     * @brief shared memory handler base class.
     */
    template <class ShmPolicy>
    class BasicShm : public EventHandler
    {
    public:
        /**
         * @brief create instance.
         */
        BasicShm (off_t size = 4096) noexcept
        : _size (size)
        {
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicShm (const BasicShm& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to assign.
         * @return assigned object.
         */
        BasicShm& operator= (const BasicShm& other) = delete;

        /**
         * @brief destroy instance.
         */
        virtual ~BasicShm () noexcept
        {
            close ();
        }

        /**
         * @brief open or create the shared memory segment.
         * @param name shared memory object name (must start with '/').
         * @param fdpass if true, eventfd descriptors are exchanged via Unix socket.
         * @return 0 on success, -1 on failure.
         */
        int open (const std::string& name, bool fdpass = false)
        {
            _fd = ::shm_open (name.c_str (), _policy.flag (), 0640);
            if (_fd == -1)
            {
                close ();
                return -1;
            }

            if ((_policy.flag () & O_CREAT) && (::ftruncate (_fd, _size) == -1))
            {
                close ();
                return -1;
            }

            _ptr = ::mmap (nullptr, _size, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, 0);
            if (_ptr == MAP_FAILED)
            {
                close ();
                return -1;
            }

            if ((fdpass) && (_policy.initialize (makePath (name)) == -1))
            {
                close ();
                return -1;
            }

            _name = name;

            return 0;
        }

        /**
         * @brief close the shared memory segment.
         */
        void close () noexcept
        {
            _policy.cleanup ();

            if ((_ptr != nullptr) && (_ptr != MAP_FAILED))
            {
                ::munmap (_ptr, _size);
                _ptr = nullptr;
            }

            if (_fd != -1)
            {
                ::close (_fd);
                _fd = -1;
            }

            if (_policy.flag () & O_CREAT)
            {
                ::shm_unlink (_name.c_str ());
            }

            _name.clear ();
        }

        /**
         * @brief send an event notification to the peer.
         * @return 0 on success, -1 on failure.
         */
        int notify ()
        {
            return _policy.notify ();
        }

        /**
         * @brief wait peer notification event.
         * @return 0 on success, -1 on failure.
         */
        int wait ()
        {
            return _policy.wait ();
        }

        /**
         * @brief get a const pointer to the shared memory.
         * @return pointer to the mapped memory region.
         */
        const void* get () const noexcept
        {
            return _ptr;
        }

        /**
         * @brief get a const pointer to the shared memory.
         * @return pointer to the mapped memory region.
         */
        void* get () noexcept
        {
            return _ptr;
        }

        /**
         * @brief get the size of the shared memory region.
         * @return shared memory size in bytes.
         */
        off_t size () const noexcept
        {
            return _size;
        }

        /**
         * @brief get native handle.
         * @return native handle.
         */
        virtual int handle () const noexcept override
        {
            return _policy.handle ();
        }

    private:
        /**
         * @brief derive unix domain path from shm name.
         * @param name shm name.
         * @return derived unix domain path.
         */
        static std::string makePath (const std::string& name)
        {
            return "/tmp" + name + ".sock";
        }

        /// policy defining behavior (server/client).
        ShmPolicy _policy;

        /// pointer to mapped shared memory.
        void* _ptr = nullptr;

        /// shared memory size.
        off_t _size = 0;

        /// shared memory descriptor.
        int _fd = -1;

        /// shared memory object name.
        std::string _name;
    };

    /**
     * @brief base class for shared memory policies.
     */
    class PolicyBase
    {
    public:
        /**
         * @brief create instance.
         */
        PolicyBase () noexcept = default;

        /**
         * @brief destroy instance.
         */
        virtual ~PolicyBase () noexcept
        {
            cleanup ();
        }

        /**
         * @brief initialize event signaling resources.
         * @param path unix socket path used for file descriptor passing.
         * @return 0 on success, -1 on failure.
         */
        virtual int initialize (const std::string& path) = 0;

        /**
         * @brief release eventfd resources.
         */
        void cleanup () noexcept
         {
            for (auto& fd : _evfds)
            {
                if (fd != -1)
                {
                    ::close (fd);
                    fd = -1;
                }
            }
        }

    protected:
        /// index of server event descriptor.
        static constexpr int _serverIdx = 0;

        /// index of client event descriptor.
        static constexpr int _clientIdx = 1;

        /// event descriptors used for synchronization.
        std::array <int , 2> _evfds = { -1, -1 };
    };

    /**
     * @brief shared memory policy for the server side.
     */
    class ServerPolicy : public PolicyBase
    {
    public:
        /**
         * @brief create instance.
         */
        ServerPolicy () noexcept = default;

        /**
         * @brief destroy instance.
         */
        ~ServerPolicy () noexcept = default;

        /**
         * @brief initialize event signaling resources.
         * @param path unix socket path used for file descriptor passing.
         * @return 0 on success, -1 on failure.
         */
        int initialize (const std::string& path) override
        {
            for (auto& fd : _evfds)
            {
                fd = ::eventfd (0, EFD_SEMAPHORE);
                if (fd == -1)
                {
                    lastError = std::make_error_code (static_cast <std::errc> (errno));
                    cleanup ();
                    return -1;
                }
            }

            UnixStream::Acceptor server;
            if (server.create (path) == -1)
            {
                cleanup ();
                return -1;
            }

            UnixStream::Socket socket = server.accept ();
            if (!socket.connected ())
            {
                cleanup ();
                return -1;
            }

            char dummy = 'X';
            struct iovec iov = {.iov_base = &dummy, .iov_len = sizeof (dummy)};
            char control[CMSG_SPACE (sizeof (int) * _evfds.size ())] = {};

            struct msghdr msg = {};
            msg.msg_iov = &iov;
            msg.msg_iovlen = 1;
            msg.msg_control = control;
            msg.msg_controllen = sizeof (control);

            struct cmsghdr* cmsg = CMSG_FIRSTHDR (&msg);
            cmsg->cmsg_len = CMSG_LEN (sizeof (int) * _evfds.size ());
            cmsg->cmsg_level = SOL_SOCKET;
            cmsg->cmsg_type = SCM_RIGHTS;
            std::memcpy (CMSG_DATA (cmsg), _evfds.data (), sizeof (int) * _evfds.size ());

            if (::sendmsg (socket.handle (), &msg, 0) == -1)
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                cleanup ();
                return -1;
            }

            return 0;
        }

        /**
         * @brief notify the client via eventfd.
         * @return 0 on success, -1 on failure.
         */
        int notify ()
        {
            uint64_t val = 1;

            if (::write (_evfds[_clientIdx], &val, sizeof (val)) != sizeof (val))
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                return -1;
            }

            return 0;
        }

        /**
         * @brief wait client notification via eventfd.
         * @return 0 on success, -1 on failure.
         */
        int wait ()
        {
            uint64_t val;

            if (::read (_evfds[_serverIdx], &val, sizeof(val)) != sizeof(val))
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                return -1;
            }

            return 0;
        }

        /**
         * @brief get open flags for shared memory.
         * @return open flags.
         */
        constexpr int flag () const noexcept
        {
            return O_CREAT | O_RDWR;
        }

        /**
         * @brief get native handle.
         * @return native handle.
         */
        int handle () const noexcept
        {
            return _evfds[_serverIdx];
        }
    };

    /**
     * @brief shared memory policy for the client side.
     */
    class ClientPolicy : public PolicyBase
    {
    public:
        /**
         * @brief create instance.
         */
        ClientPolicy () noexcept = default;

        /**
         * @brief destroy instance.
         */
        ~ClientPolicy () noexcept = default;

        /**
         * @brief initialize event signaling resources.
         * @param path unix socket path used for file descriptor passing.
         * @return 0 on success, -1 on failure.
         */
        int initialize (const std::string& path) override
        {
            UnixStream::Socket socket (UnixStream::Socket::Blocking);
            if (socket.connect (path) == -1)
            {
                cleanup ();
                return -1;
            }

            char dummy;
            struct iovec iov = {.iov_base = &dummy, .iov_len = 1};
            char control[CMSG_SPACE (sizeof (int) * _evfds.size ())] = {};

            struct msghdr msg = {};
            msg.msg_iov = &iov;
            msg.msg_iovlen = 1;
            msg.msg_control = control;
            msg.msg_controllen = sizeof (control);

            if (::recvmsg (socket.handle (), &msg, 0) == -1)
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                cleanup ();
                return -1;
            }

            struct cmsghdr* cmsg = CMSG_FIRSTHDR (&msg);
            if ((cmsg == nullptr) || (cmsg->cmsg_level != SOL_SOCKET) || (cmsg->cmsg_type != SCM_RIGHTS))
            {
                lastError = make_error_code (Errc::MessageUnknown);
                cleanup ();
                return -1;
            }

            int* fds = reinterpret_cast <int*> (CMSG_DATA (cmsg));
            size_t count = (cmsg->cmsg_len - CMSG_LEN (0)) / sizeof (int);
            if (count > _evfds.size ())
            {
                lastError = make_error_code (Errc::MessageTooLong);
                cleanup ();
                return -1;
            }

            ::memcpy (_evfds.data (), fds, count * sizeof (int));

            return 0;
        }

        /**
         * @brief notify the server via eventfd.
         * @return 0 on success, -1 on failure.
         */
        int notify ()
        {
            uint64_t val = 1;

            if (::write (_evfds[_serverIdx], &val, sizeof (val)) != sizeof (val))
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                return -1;
            }

            return 0;
        }

        /**
         * @brief wait server notification via eventfd.
         * @return 0 on success, -1 on failure.
         */
        int wait ()
        {
            uint64_t val;

            if (::read (_evfds[_clientIdx], &val, sizeof(val)) != sizeof(val))
            {
                lastError = std::make_error_code (static_cast <std::errc> (errno));
                return -1;
            }

            return 0;
        }

        /**
         * @brief get open flags for shared memory.
         * @return open flags.
         */
        constexpr int flag () const noexcept
        {
            return O_RDWR;
        }

        /**
         * @brief get native handle.
         * @return native handle.
         */
        int handle () const noexcept
        {
            return _evfds[_clientIdx];
        }
    };

    /**
     * @brief convenience wrapper for server/client shared memory types.
     * 
     * provides aliases for easy access:
     *  - Shm::Server → BasicShm<ServerPolicy>
     *  - Shm::Client → BasicShm<ClientPolicy>
     */
    struct Shm
    {
        using Server = BasicShm <ServerPolicy>;
        using Client = BasicShm <ClientPolicy>;
    };
}

#endif
