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

#ifndef __JOIN_MAIL_MESSAGE_HPP__
#define __JOIN_MAIL_MESSAGE_HPP__

// libjoin.
#include <join/version.hpp>
#include <join/error.hpp>

// C++.
#include <iostream>
#include <vector>

namespace join
{
    /**
     * @brief mail sender.
     */
    class MailSender
    {
    public:
        /**
         * @brief create the MailSender instance.
         */
        MailSender () = default;

        /**
         * @brief create the MailSender instance.
         * @param address recipient address.
         */
        MailSender (const std::string& address);

        /**
         * @brief create the MailSender instance.
         * @param address recipient address.
         * @param name recipient name.
         */
        MailSender (const std::string& address, const std::string& name);

        /**
         * @brief create the MailSender instance by copy.
         * @param other request to copy.
         */
        MailSender (const MailSender& other);

        /**
         * @brief assign the MailSender instance by copy.
         * @param other request to copy.
         * @return a reference of the current object.
         */
        MailSender& operator= (const MailSender& other);

        /**
         * @brief create the MailSender instance by move.
         * @param other request to move.
         */
        MailSender (MailSender&& other);

        /**
         * @brief assign the MailSender instance by move.
         * @param other request to move.
         * @return a reference of the current object.
         */
        MailSender& operator= (MailSender&& other);

        /**
         * @brief destroy the MailSender instance.
         */
        virtual ~MailSender () = default;

        /**
         * @brief set address.
         * @param addr address.
         */
        void address (const std::string& addr);

        /**
         * @brief get address.
         * @return address.
         */
        const std::string& address () const;

        /**
         * @brief set real name.
         * @param name real name.
         */
        void realName (const std::string& name);

        /**
         * @brief get real name.
         * @return real name.
         */
        const std::string& realName () const;

        /**
         * @brief check if empty.
         * @return true if if empty, false otherwise.
         */
        bool empty () const;

    protected:
        /// address.
        std::string _address;

        /// name.
        std::string _name;
    };

    /**
     * @brief insert mail sender into stream.
     * @param out output stream.
     * @param sender mail sender.
     * @return a reference to the output stream.
     */
    std::ostream& operator<< (std::ostream& out, const MailSender& sender);

    /**
     * @brief mail recipient.
     */
    class MailRecipient : public MailSender
    {
    public:
        /**
         * @brief recipient type.
         */
        enum Type
        {
            Recipient,              /**< recipient. */
            CCRecipient,            /**< carbon copy. */
            BCCRecipient,           /**< black carbon copy. */
        };

        /**
         * @brief create the MailRecipient instance.
         */
        MailRecipient () = default;

        /**
         * @brief create the MailRecipient instance.
         * @param address recipient address.
         * @param t recipient type.
         */
        MailRecipient (const std::string& address, Type t = Recipient);

        /**
         * @brief create the MailRecipient instance.
         * @param address recipient address.
         * @param name recipient name.
         * @param t recipient type.
         */
        MailRecipient (const std::string& address, const std::string& name, Type t = Recipient);

        /**
         * @brief create the MailRecipient instance by copy.
         * @param other request to copy.
         */
        MailRecipient (const MailRecipient& other);

        /**
         * @brief assign the MailRecipient instance by copy.
         * @param other request to copy.
         * @return a reference of the current object.
         */
        MailRecipient& operator= (const MailRecipient& other);

        /**
         * @brief create the MailRecipient instance by move.
         * @param other request to move.
         */
        MailRecipient (MailRecipient&& other);

        /**
         * @brief assign the MailRecipient instance by move.
         * @param other request to move.
         * @return a reference of the current object.
         */
        MailRecipient& operator= (MailRecipient&& other);

        /**
         * @brief destroy the MailRecipient instance.
         */
        virtual ~MailRecipient () = default;

        /**
         * @brief set recipient type.
         * @param t recipient type.
         */
        void type (Type t);

        /**
         * @brief get recipient type.
         * @return recipient type.
         */
        Type type () const;

    protected:
        /// recipient type.
        Type _type = Recipient;
    };

    /// mail recipient list.
    using MailRecipients = std::vector <MailRecipient>;

    /**
     * @brief mail message.
     */
    class MailMessage
    {
    public:
        /**
         * @brief create the MailMessage instance.
         */
        MailMessage () = default;

        /**
         * @brief create the MailMessage instance by copy.
         * @param other request to copy.
         */
        MailMessage (const MailMessage& other);

        /**
         * @brief assign the MailMessage instance by copy.
         * @param other request to copy.
         * @return a reference of the current object.
         */
        MailMessage& operator= (const MailMessage& other);

        /**
         * @brief create the MailMessage instance by move.
         * @param other request to move.
         */
        MailMessage (MailMessage&& other);

        /**
         * @brief assign the MailMessage instance by move.
         * @param other request to move.
         * @return a reference of the current object.
         */
        MailMessage& operator= (MailMessage&& other);

        /**
         * @brief destroy the MailMessage instance.
         */
        virtual ~MailMessage () = default;

        /**
         * @brief set mail sender.
         * @param from mail sender.
         */
        void sender (const MailSender& from);

        /**
         * @brief get mail sender
         * @return mail sender.
         */
        const MailSender& sender () const;

        /**
         * @brief add mail recipient.
         * @param to mail recipient.
         */
        void addRecipient (const MailRecipient& to);

        /**
         * @brief add mail recipient.
         * @param to mail recipient.
         */
        void addRecipient (MailRecipient&& to);

        /**
         * @brief get mail recipients.
         * @return mail recipients.
         */
        const MailRecipients& recipients () const;

        /**
         * @brief set mail subject.
         * @param subj mail subject.
         */
        void subject (const std::string& subj);

        /**
         * @brief get mail subject.
         * @return mail subject.
         */
        const std::string& subject () const;

        /**
         * @brief set mail content.
         * @param message mail content.
         */
        void content (const std::string& message);

        /**
         * @brief get mail content.
         * @return mail content.
         */
        const std::string& content () const;

        /**
         * @brief write HTTP header to the given output stream.
         * @param out output stream.
         * @return 0 on success, -1 on failure.
         */
        int writeHeaders (std::ostream& out) const;

    protected:
        /// mail sender.
        MailSender _sender;

        /// mail recipients.
        MailRecipients _recipients;

        /// mail sender.
        std::string _subject;

        /// mail content.
        std::string _content;
    };
}

#endif
