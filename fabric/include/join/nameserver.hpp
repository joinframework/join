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

#ifndef JOIN_FABRIC_NAMESERVER_HPP
#define JOIN_FABRIC_NAMESERVER_HPP

// libjoin.
#include <join/dnsmessage.hpp>
#include <join/reactor.hpp>

namespace join
{
    /**
     * @brief basic DNS name server over datagram socket.
     */
    template <typename Protocol>
    class BasicDatagramNameServer : public Protocol::Socket
    {
    public:
        using Socket = typename Protocol::Socket;
        using Endpoint = typename Protocol::Endpoint;

        /**
         * @brief construct the name server instance.
         * @param reactor event loop reactor.
         */
        explicit BasicDatagramNameServer (Reactor* reactor = nullptr)
        : _reactor (reactor ? reactor : ReactorThread::reactor ())
        , _buffer (std::make_unique<char[]> (Protocol::maxMsgSize))
        {
        }

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        BasicDatagramNameServer (const BasicDatagramNameServer& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to copy.
         * @return a reference to the current object.
         */
        BasicDatagramNameServer& operator= (const BasicDatagramNameServer& other) = delete;

        /**
         * @brief move constructor.
         * @param other other object to move.
         */
        BasicDatagramNameServer (BasicDatagramNameServer&& other) = delete;

        /**
         * @brief move assignment operator.
         * @param other other object to move.
         * @return a reference to the current object.
         */
        BasicDatagramNameServer& operator= (BasicDatagramNameServer&& other) = delete;

        /**
         * @brief destroy instance.
         */
        virtual ~BasicDatagramNameServer () noexcept = default;

        /**
         * @brief bind the socket to the given endpoint and register with the reactor.
         * @param endpoint endpoint to bind to.
         * @return 0 on success, -1 on failure.
         */
        virtual int bind (const Endpoint& endpoint) noexcept override
        {
            if (Socket::bind (endpoint) == -1)
            {
                return -1;
            }

            _reactor->addHandler (this->handle (), this);

            return 0;
        }

        /**
         * @brief close the socket and unregister from the reactor.
         */
        virtual void close () noexcept override
        {
            _reactor->delHandler (this->handle ());
            Socket::close ();
        }

        /**
         * @brief reply to a DNS query.
         * @param query original query packet.
         * @param answers answer records.
         * @param authorities authority records.
         * @param additionals additional records.
         * @param rcode response code (default: 0 = no error).
         * @return 0 on success, -1 on error.
         */
        int reply (const DnsPacket& query, const std::vector<ResourceRecord>& answers = {},
                   const std::vector<ResourceRecord>& authorities = {},
                   const std::vector<ResourceRecord>& additionals = {}, uint16_t rcode = 0)
        {
            DnsPacket response{};
            response.id = query.id;
            response.flags = (uint16_t (1) << 15) | (query.flags & 0x7800) | (uint16_t (1) << 10) | (rcode & 0x000F);
            response.questions = query.questions;
            response.answers = answers;
            response.authorities = authorities;
            response.additionals = additionals;

            std::stringstream data;
            if (_message.serialize (response, data) == -1)
            {
                lastError = make_error_code (Errc::InvalidParam);
                return -1;
            }

            std::string buffer = data.str ();
            if (buffer.size () > Protocol::maxMsgSize)
            {
                lastError = make_error_code (Errc::MessageTooLong);
                return -1;
            }

            Endpoint to (query.src, query.port);
            if (this->writeTo (buffer.data (), buffer.size (), to) == -1)
            {
                return -1;
            }

            return 0;
        }

        /**
         * @brief method called when a DNS query is received.
         * @param packet parsed DNS query received.
         */
        virtual void onQuery (const DnsPacket& packet) = 0;

    protected:
        /**
         * @brief method called when data are ready to be read on handle.
         * @param fd file descriptor.
         */
        virtual void onReceive ([[maybe_unused]] int fd) override
        {
            Endpoint from;
            int size = this->readFrom (_buffer.get (), Protocol::maxMsgSize, &from);
            if (size >= int (_headerSize))
            {
                std::stringstream data;
                data.rdbuf ()->pubsetbuf (_buffer.get (), size);

                DnsPacket packet;
                _message.deserialize (packet, data);
                packet.src = from.ip ();
                packet.port = from.port ();
                packet.dest = this->localEndpoint ().ip ();

                if ((packet.flags & 0x8000) == 0)
                {
                    this->onQuery (packet);
                }
            }
        }

        /// DNS message header size.
        static constexpr size_t _headerSize = 12;

        /// DNS message codec.
        DnsMessage _message;

        /// event loop reactor.
        Reactor* _reactor;

        /// reception buffer.
        std::unique_ptr<char[]> _buffer;
    };
}

#endif
