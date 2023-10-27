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

#ifndef __JOIN_SMTP_CLIENT_HPP__
#define __JOIN_SMTP_CLIENT_HPP__

// libjoin.
#include <join/socketstream.hpp>
#include <join/mailmessage.hpp>
#include <join/base64.hpp>

namespace join
{
    /**
     * @brief basic SMTP client.
     */
    template <class Protocol> 
    class BasicSmtpClient : public Protocol::Stream
    {
    public:
        using Endpoint = typename Protocol::Endpoint;

        /**
         * @brief create the basic SMTP client instance.
         * @param host host.
         * @param port port (default: 25).
         */
        BasicSmtpClient (const char* host, uint16_t port = 25)
        : _host (host),
          _port (port)
        {
        }

        /**
         * @brief create the basic SMTP client instance.
         * @param host host.
         * @param port port (default: 25).
         */
        BasicSmtpClient (const std::string& host, uint16_t port = 25)
        : BasicSmtpClient (host.c_str (), port)
        {
        }

        /**
         * @brief create the basic SMTP client instance by copy.
         * @param other request to copy.
         */
        BasicSmtpClient (const BasicSmtpClient& other) = delete;

        /**
         * @brief assign the basic SMTP client instance by copy.
         * @param other request to copy.
         * @return a reference of the current object.
         */
        BasicSmtpClient& operator= (const BasicSmtpClient& other) = delete;

        /**
         * @brief create the basic SMTP client instance by move.
         * @param other request to move.
         */
        BasicSmtpClient (BasicSmtpClient&& other)
        : Protocol::Stream (std::move (other)),
          _host (std::move (other._host)),
          _port (other._port),
          _login (std::move (other._login)),
          _password (std::move (other._password))
        {
        }

        /**
         * @brief assign the basic SMTP client instance by move.
         * @param other request to move.
         * @return a reference of the current object.
         */
        BasicSmtpClient& operator= (BasicSmtpClient&& other)
        {
            Protocol::Stream::operator= (std::move (other));
            this->_host = std::move (other._host);
            this->_port = other._port;
            this->_login = std::move (other._login);
            this->_password = std::move (other._password);
            return *this;
        }

        /**
         * @brief destroy the basic SMTP client instance.
         */
        virtual ~BasicSmtpClient () = default;

        /**
         * @brief get scheme.
         * @return smtp or smtps.
         */
        virtual std::string scheme () const
        {
            return "smtp";
        }

        /**
         * @brief get host.
         * @return host.
         */
        const std::string& host () const
        {
            return this->_host;
        }

        /**
         * @brief get port.
         * @return port.
         */
        uint16_t port () const
        {
            return this->_port;
        }

        /**
         * @brief get authority.
         * @return authority.
         */
        std::string authority () const
        {
            std::string auth;

            if (IpAddress::isIpv6Address (this->host ()))
            {
                auth += "[" + this->host () + "]";
            }
            else
            {
                auth += this->host ();
            }

            if (this->port () != Resolver::resolveService (this->scheme ()))
            {
                auth += ":" + std::to_string (this->port ());
            }

            return auth;
        }

        /**
         * @brief get URL.
         * @return URL.
         */
        std::string url () const
        {
            return this->scheme () + "://" + this->authority ();
        }

        /**
         * @brief set credentials.
         * @param login login.
         * @param password password.
         */
        void credentials (const std::string& login, const std::string& password)
        {
            this->_login = login;
            this->_password = password;
        }

        /**
         * @brief send mail message.
         * @param mail mail message to send.
         * @return 0 on success, -1 on failure.
         */
        int send (const MailMessage& mail)
        {
            this->connect (this->url ());
            if (this->fail ())
            {
                return -1;
            }

            if (this->greeting () == -1)
            {
                return -1;
            }

            std::vector <std::string> replies;

            for (;;)
            {
                replies.clear ();

                if (this->initialize (replies) == -1)
                {
                    return -1;
                }

                if (std::find (replies.begin (), replies.end (), "STARTTLS") != replies.end ())
                {
                    if (this->startTls () == -1)
                    {
                        return -1;
                    }
                    continue;
                }

                break;
            }

            if (std::find (replies.begin (), replies.end (), "AUTH") != replies.end ())
            {
                if (std::find (replies.begin (), replies.end (), "LOGIN") != replies.end ())
                {
                    if (this->loginAuthenticate () == -1)
                    {
                        return -1;
                    }
                }
                else if (std::find (replies.begin (), replies.end (), "PLAIN") != replies.end ())
                {
                    if (this->plainAuthenticate () == -1)
                    {
                        return -1;
                    }
                }
            }

            if (this->sendFrom (mail) == -1)
            {
                return -1;
            }

            if (this->sendTo (mail) == -1)
            {
                return -1;
            }

            if (this->sendData (mail) == -1)
            {
                return -1;
            }

            if (this->quit () == -1)
            {
                return -1;
            }

            return 0;
        }

    protected:
        /**
         * @brief get host name.
         * @return host name.
         */
        std::string hostname () const
        {
            char name[255] = {};
            gethostname (name, sizeof (name));
            return name;
        }

        /**
         * @brief send message.
         * @param message message to send.
         */
        void sendMessage (const std::string& message)
        {
        #ifdef DEBUG
            std::cout << message << std::endl;
        #endif
            this->write (message.c_str (), message.size ());
            this->write ("\r\n", 2);
            this->flush ();
        }

        /**
         * @brief read replies.
         * @param replies replies to read.
         * @return reply code.
         */
        std::string readReplies (std::vector <std::string>* replies = nullptr)
        {
            std::string reply, code;
            for (;;)
            {
                if (!join::getline (*this, reply))
                {
                    return {};
                }
            #ifdef DEBUG
                std::cout << reply << std::endl;
            #endif
                if ((reply.find ("-") != 3) && (reply.find (" ") != 3))
                {
                    //join::lastError = make_error_code (Errc::MessageTooLong);
                    return {};
                }
                if (code.empty ())
                {
                    code = reply.substr (0, 3);
                }
                if (replies)
                {
                    replies->push_back (reply.substr (4));
                }
                if (reply[3] == ' ')
                {
                    break;
                }
            }
            return code;
        }

        /**
         * @brief handle greeting.
         * @return 0 on success, -1 on failure.
         */
        int greeting ()
        {
            if (this->readReplies () != "220")
            {
                return -1;
            }
            return 0;
        }

        /**
         * @brief client initiation.
         * @param replies server replies.
         * @return 0 on success, -1 on failure.
         */
        int initialize (std::vector <std::string>& replies)
        {
            this->sendMessage ("EHLO " + this->hostname ());
            if (this->fail ())
            {
                return -1;
            }
            if (this->readReplies (&replies) != "250")
            {
                return -1;
            }
            return 0;
        }

        /**
         * @brief start encryption.
         * @return 0 on success, -1 on failure.
         */
        int startTls ()
        {
            this->sendMessage ("STARTTLS");
            if (this->fail ())
            {
                return -1;
            }
            if (this->readReplies () != "220")
            {
                return -1;
            }
            this->startEncryption ();
            if (this->fail ())
            {
                return -1;
            }
            return 0;
        }

        /**
         * @brief authenticate using LOGIN.
         * @return 0 on success, -1 on failure.
         */
        int loginAuthenticate ()
        {
            this->sendMessage ("AUTH LOGIN");
            if (this->fail ())
            {
                return -1;
            }
            if (this->readReplies () != "334")
            {
                return -1;
            }
            this->sendMessage (Base64::encode (this->_login));
            if (this->fail ())
            {
                return -1;
            }
            if (this->readReplies () != "334")
            {
                return -1;
            }
            this->sendMessage (Base64::encode (this->_password));
            if (this->fail ())
            {
                return -1;
            }
            if (this->readReplies () != "235")
            {
                return -1;
            }
            return 0;
        }

        /**
         * @brief authenticate using PLAIN.
         * @return 0 on success, -1 on failure.
         */
        int plainAuthenticate ()
        {
            this->sendMessage ("AUTH PLAIN");
            if (this->fail ())
            {
                return -1;
            }
            if (this->readReplies () != "334")
            {
                return -1;
            }
            this->sendMessage (Base64::encode ('\0' + this->_login + '\0' + this->_password));
            if (this->fail ())
            {
                return -1;
            }
            if (this->readReplies () != "235")
            {
                return -1;
            }
            return 0;
        }

        /**
         * @brief .
         * @param message .
         * @return 0 on success, -1 on failure.
         */
        int sendFrom (const MailMessage& message)
        {
            this->sendMessage ("MAIL FROM: <" + message.sender ().address () + ">");
            if (this->fail ())
            {
                return -1;
            }
            if (this->readReplies () != "250")
            {
                return -1;
            }
            return 0;
        }

        /**
         * @brief .
         * @param message .
         * @return 0 on success, -1 on failure.
         */
        int sendTo (const MailMessage& message)
        {
            for (auto const& recipient : message.recipients ())
            {
                this->sendMessage ("RCPT TO: <" + recipient.address () + ">");
                if (this->fail ())
                {
                    return -1;
                }
                if (this->readReplies () != "250")
                {
                    return -1;
                }
            }
            return 0;
        }

        /**
         * @brief .
         * @param .
         * @return 0 on success, -1 on failure.
         */
        int sendData (const MailMessage& message)
        {
            this->sendMessage ("DATA");
            if (this->fail ())
            {
                return -1;
            }
            if (this->readReplies () != "354")
            {
                return -1;
            }
            if (message.writeHeaders (*this) == -1)
            {
                return -1;
            }
            /*if (this->sendContent () == -1)
            {
                return -1;
            }*/
            if (this->readReplies () != "250")
            {
                return -1;
            }
            return 0;
        }

        /**
         * @brief .
         * @param .
         * @return 0 on success, -1 on failure.
         */
        int quit ()
        {
            this->sendMessage ("QUIT");
            if (this->fail ())
            {
                return -1;
            }
            if (this->readReplies () != "221")
            {
                return -1;
            }
            return 0;
        }

        /// SMTP host.
        std::string _host;

        /// SMTP port.
        uint16_t _port;

        /// SMTP login.
        std::string _login;

        /// SMTP password.
        std::string _password;
    };

    /**
     * @brief Basic SMTPS client.
     */
    template <class Protocol> 
    class BasicSmtpSecureClient : public BasicSmtpClient <Protocol>
    {
    public:
        using Endpoint = typename Protocol::Endpoint;

        /**
         * @brief create the basic SMTPS client instance.
         * @param host host.
         * @param port port (default: 465).
         */
        BasicSmtpSecureClient (const char* host, uint16_t port = 465)
        : BasicSmtpClient <Protocol> (host, port)
        {
        }

        /**
         * @brief create the basic SMTPS client instance.
         * @param host host.
         * @param port port (default: 465).
         */
        BasicSmtpSecureClient (const std::string& host, uint16_t port = 465)
        : BasicSmtpSecureClient (host.c_str (), port)
        {
        }

        /**
         * @brief create the basic SMTPS client instance by copy.
         * @param other request to copy.
         */
        BasicSmtpSecureClient (const BasicSmtpSecureClient& other) = delete;

        /**
         * @brief assign the basic SMTPS client instance by copy.
         * @param other request to copy.
         * @return a reference of the current object.
         */
        BasicSmtpSecureClient& operator= (const BasicSmtpSecureClient& other) = delete;

        /**
         * @brief create the basic SMTPS client instance by move.
         * @param other request to move.
         */
        BasicSmtpSecureClient (BasicSmtpSecureClient&& other)
        : BasicSmtpClient <Protocol> (std::move (other))
        {
        }

        /**
         * @brief assign the basic SMTPS client instance by move.
         * @param other request to move.
         * @return a reference of the current object.
         */
        BasicSmtpSecureClient& operator= (BasicSmtpSecureClient&& other)
        {
            BasicSmtpClient <Protocol>::operator= (std::move (other));
            return *this;
        }

        /**
         * @brief destroy the basic SMTPS client instance.
         */
        virtual ~BasicSmtpSecureClient () = default;

        /**
         * @brief get scheme.
         * @return smtp or smtps.
         */
        std::string scheme () const override
        {
            return "smtps";
        }
    };
}

#endif
