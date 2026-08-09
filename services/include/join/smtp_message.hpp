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

#ifndef JOIN_SERVICES_SMTP_MESSAGE_HPP
#define JOIN_SERVICES_SMTP_MESSAGE_HPP

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
    class SmtpSender
    {
    public:
        /**
         * @brief create the SmtpSender instance.
         */
        SmtpSender () = default;

        /**
         * @brief create the SmtpSender instance.
         * @param address recipient address.
         */
        SmtpSender (const std::string& address);

        /**
         * @brief create the SmtpSender instance.
         * @param address recipient address.
         * @param name recipient name.
         */
        SmtpSender (const std::string& address, const std::string& name);

        /**
         * @brief create the SmtpSender instance by copy.
         * @param other request to copy.
         */
        SmtpSender (const SmtpSender& other);

        /**
         * @brief assign the SmtpSender instance by copy.
         * @param other request to copy.
         * @return a reference of the current object.
         */
        SmtpSender& operator= (const SmtpSender& other);

        /**
         * @brief create the SmtpSender instance by move.
         * @param other request to move.
         */
        SmtpSender (SmtpSender&& other);

        /**
         * @brief assign the SmtpSender instance by move.
         * @param other request to move.
         * @return a reference of the current object.
         */
        SmtpSender& operator= (SmtpSender&& other);

        /**
         * @brief destroy the SmtpSender instance.
         */
        virtual ~SmtpSender () = default;

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
    std::ostream& operator<< (std::ostream& out, const SmtpSender& sender);

    /**
     * @brief mail recipient.
     */
    class SmtpRecipient : public SmtpSender
    {
    public:
        /**
         * @brief recipient type.
         */
        enum Type
        {
            Recipient,    /**< recipient. */
            CCRecipient,  /**< carbon copy. */
            BCCRecipient, /**< black carbon copy. */
        };

        /**
         * @brief create the SmtpRecipient instance.
         */
        SmtpRecipient () = default;

        /**
         * @brief create the SmtpRecipient instance.
         * @param address recipient address.
         * @param t recipient type.
         */
        SmtpRecipient (const std::string& address, Type t = Recipient);

        /**
         * @brief create the SmtpRecipient instance.
         * @param address recipient address.
         * @param name recipient name.
         * @param t recipient type.
         */
        SmtpRecipient (const std::string& address, const std::string& name, Type t = Recipient);

        /**
         * @brief create the SmtpRecipient instance by copy.
         * @param other request to copy.
         */
        SmtpRecipient (const SmtpRecipient& other);

        /**
         * @brief assign the SmtpRecipient instance by copy.
         * @param other request to copy.
         * @return a reference of the current object.
         */
        SmtpRecipient& operator= (const SmtpRecipient& other);

        /**
         * @brief create the SmtpRecipient instance by move.
         * @param other request to move.
         */
        SmtpRecipient (SmtpRecipient&& other);

        /**
         * @brief assign the SmtpRecipient instance by move.
         * @param other request to move.
         * @return a reference of the current object.
         */
        SmtpRecipient& operator= (SmtpRecipient&& other);

        /**
         * @brief destroy the SmtpRecipient instance.
         */
        virtual ~SmtpRecipient () = default;

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
    using SmtpRecipients = std::vector<SmtpRecipient>;

    /**
     * @brief mail message.
     */
    class SmtpMessage
    {
    public:
        /**
         * @brief create the SmtpMessage instance.
         */
        SmtpMessage () = default;

        /**
         * @brief create the SmtpMessage instance by copy.
         * @param other request to copy.
         */
        SmtpMessage (const SmtpMessage& other);

        /**
         * @brief assign the SmtpMessage instance by copy.
         * @param other request to copy.
         * @return a reference of the current object.
         */
        SmtpMessage& operator= (const SmtpMessage& other);

        /**
         * @brief create the SmtpMessage instance by move.
         * @param other request to move.
         */
        SmtpMessage (SmtpMessage&& other);

        /**
         * @brief assign the SmtpMessage instance by move.
         * @param other request to move.
         * @return a reference of the current object.
         */
        SmtpMessage& operator= (SmtpMessage&& other);

        /**
         * @brief destroy the SmtpMessage instance.
         */
        virtual ~SmtpMessage () = default;

        /**
         * @brief set mail sender.
         * @param from mail sender.
         */
        void sender (const SmtpSender& from);

        /**
         * @brief get mail sender
         * @return mail sender.
         */
        const SmtpSender& sender () const;

        /**
         * @brief add mail recipient.
         * @param to mail recipient.
         */
        void addRecipient (const SmtpRecipient& to);

        /**
         * @brief add mail recipient.
         * @param to mail recipient.
         */
        void addRecipient (SmtpRecipient&& to);

        /**
         * @brief get mail recipients.
         * @return mail recipients.
         */
        const SmtpRecipients& recipients () const;

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
         * @brief write header to the given output stream.
         * @param out output stream.
         * @return 0 on success, -1 on failure.
         */
        int writeHeaders (std::ostream& out) const;

        /**
         * @brief write content to the given output stream.
         * @param out output stream.
         * @return 0 on success, -1 on failure.
         */
        int writeContent (std::ostream& out) const;

    protected:
        /// mail sender.
        SmtpSender _sender;

        /// mail recipients.
        SmtpRecipients _recipients;

        /// mail sender.
        std::string _subject;

        /// mail content.
        std::string _content;
    };
}

#endif
