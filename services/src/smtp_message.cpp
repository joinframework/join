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

// libjoin.
#include <join/smtp_message.hpp>

// C++.
#include <algorithm>
#include <iterator>
#include <iomanip>

using join::SmtpSender;
using join::SmtpRecipient;
using join::SmtpRecipients;
using join::SmtpMessage;

// =========================================================================
//   CLASS     : SmtpSender
//   METHOD    : SmtpSender
// =========================================================================
SmtpSender::SmtpSender (const std::string& address)
: SmtpSender (address, "")
{
}

// =========================================================================
//   CLASS     : SmtpSender
//   METHOD    : SmtpSender
// =========================================================================
SmtpSender::SmtpSender (const std::string& address, const std::string& name)
: _address (address)
, _name (name)
{
}

// =========================================================================
//   CLASS     : SmtpSender
//   METHOD    : SmtpSender
// =========================================================================
SmtpSender::SmtpSender (const SmtpSender& other)
: _address (other._address)
, _name (other._name)
{
}

// =========================================================================
//   CLASS     : SmtpSender
//   METHOD    : operator=
// =========================================================================
SmtpSender& SmtpSender::operator= (const SmtpSender& other)
{
    _address = other._address;
    _name = other._name;
    return *this;
}

// =========================================================================
//   CLASS     : SmtpSender
//   METHOD    : SmtpSender
// =========================================================================
SmtpSender::SmtpSender (SmtpSender&& other)
: _address (std::move (other._address))
, _name (std::move (other._name))
{
}

// =========================================================================
//   CLASS     : SmtpSender
//   METHOD    : operator=
// =========================================================================
SmtpSender& SmtpSender::operator= (SmtpSender&& other)
{
    _address = std::move (other._address);
    _name = std::move (other._name);
    return *this;
}

// =========================================================================
//   CLASS     : SmtpSender
//   METHOD    : address
// =========================================================================
void SmtpSender::address (const std::string& addr)
{
    _address = addr;
}

// =========================================================================
//   CLASS     : SmtpSender
//   METHOD    : address
// =========================================================================
const std::string& SmtpSender::address () const
{
    return _address;
}

// =========================================================================
//   CLASS     : SmtpSender
//   METHOD    : realName
// =========================================================================
void SmtpSender::realName (const std::string& name)
{
    _name = name;
}

// =========================================================================
//   CLASS     : SmtpSender
//   METHOD    : realName
// =========================================================================
const std::string& SmtpSender::realName () const
{
    return _name;
}

// =========================================================================
//   CLASS     : SmtpSender
//   METHOD    : empty
// =========================================================================
bool SmtpSender::empty () const
{
    return _address.empty ();
}

// =========================================================================
//   CLASS     :
//   METHOD    : operator<<
// =========================================================================
std::ostream& join::operator<< (std::ostream& out, const SmtpSender& sender)
{
    out << sender.realName () << "<" << sender.address () << ">";
    return out;
}

// =========================================================================
//   CLASS     : SmtpRecipient
//   METHOD    : SmtpRecipient
// =========================================================================
SmtpRecipient::SmtpRecipient (const std::string& address, Type t)
: SmtpRecipient (address, "", t)
{
}

// =========================================================================
//   CLASS     : SmtpRecipient
//   METHOD    : SmtpRecipient
// =========================================================================
SmtpRecipient::SmtpRecipient (const std::string& address, const std::string& name, Type t)
: SmtpSender (address, name)
, _type (t)
{
}

// =========================================================================
//   CLASS     : SmtpRecipient
//   METHOD    : SmtpRecipient
// =========================================================================
SmtpRecipient::SmtpRecipient (const SmtpRecipient& other)
: SmtpSender (other)
, _type (other._type)
{
}

// =========================================================================
//   CLASS     : SmtpRecipient
//   METHOD    : operator=
// =========================================================================
SmtpRecipient& SmtpRecipient::operator= (const SmtpRecipient& other)
{
    SmtpSender::operator= (other);
    _type = other._type;
    return *this;
}

// =========================================================================
//   CLASS     : SmtpRecipient
//   METHOD    : SmtpRecipient
// =========================================================================
SmtpRecipient::SmtpRecipient (SmtpRecipient&& other)
: SmtpSender (std::move (other))
, _type (other._type)
{
}

// =========================================================================
//   CLASS     : SmtpRecipient
//   METHOD    : operator=
// =========================================================================
SmtpRecipient& SmtpRecipient::operator= (SmtpRecipient&& other)
{
    SmtpSender::operator= (std::move (other));
    _type = other._type;
    return *this;
}

// =========================================================================
//   CLASS     : SmtpRecipient
//   METHOD    : type
// =========================================================================
void SmtpRecipient::type (Type t)
{
    _type = t;
}

// =========================================================================
//   CLASS     : SmtpRecipient
//   METHOD    : type
// =========================================================================
SmtpRecipient::Type SmtpRecipient::type () const
{
    return _type;
}

// =========================================================================
//   CLASS     : SmtpMessage
//   METHOD    : SmtpMessage
// =========================================================================
SmtpMessage::SmtpMessage (const SmtpMessage& other)
: _sender (other._sender)
, _recipients (other._recipients)
, _subject (other._subject)
, _content (other._content)
{
}

// =========================================================================
//   CLASS     : SmtpMessage
//   METHOD    : operator=
// =========================================================================
SmtpMessage& SmtpMessage::operator= (const SmtpMessage& other)
{
    _sender = other._sender;
    _recipients = other._recipients;
    _subject = other._subject;
    _content = other._content;
    return *this;
}

// =========================================================================
//   CLASS     : SmtpMessage
//   METHOD    : SmtpMessage
// =========================================================================
SmtpMessage::SmtpMessage (SmtpMessage&& other)
: _sender (std::move (other._sender))
, _recipients (std::move (other._recipients))
, _subject (std::move (other._subject))
, _content (std::move (other._content))
{
}

// =========================================================================
//   CLASS     : SmtpMessage
//   METHOD    : operator=
// =========================================================================
SmtpMessage& SmtpMessage::operator= (SmtpMessage&& other)
{
    _sender = std::move (other._sender);
    _recipients = std::move (other._recipients);
    _subject = std::move (other._subject);
    _content = std::move (other._content);
    return *this;
}

// =========================================================================
//   CLASS     : SmtpMessage
//   METHOD    : sender
// =========================================================================
void SmtpMessage::sender (const SmtpSender& from)
{
    _sender = from;
}

// =========================================================================
//   CLASS     : SmtpMessage
//   METHOD    : sender
// =========================================================================
const SmtpSender& SmtpMessage::sender () const
{
    return _sender;
}

// =========================================================================
//   CLASS     : SmtpMessage
//   METHOD    : addRecipient
// =========================================================================
void SmtpMessage::addRecipient (const SmtpRecipient& to)
{
    _recipients.push_back (to);
}

// =========================================================================
//   CLASS     : SmtpMessage
//   METHOD    : addRecipient
// =========================================================================
void SmtpMessage::addRecipient (SmtpRecipient&& to)
{
    _recipients.push_back (std::move (to));
}

// =========================================================================
//   CLASS     : SmtpMessage
//   METHOD    : recipients
// =========================================================================
const SmtpRecipients& SmtpMessage::recipients () const
{
    return _recipients;
}

// =========================================================================
//   CLASS     : SmtpMessage
//   METHOD    : subject
// =========================================================================
void SmtpMessage::subject (const std::string& subj)
{
    _subject = subj;
}

// =========================================================================
//   CLASS     : SmtpMessage
//   METHOD    : subject
// =========================================================================
const std::string& SmtpMessage::subject () const
{
    return _subject;
}

// =========================================================================
//   CLASS     : SmtpMessage
//   METHOD    : content
// =========================================================================
void SmtpMessage::content (const std::string& message)
{
    _content = message;
}

// =========================================================================
//   CLASS     : SmtpMessage
//   METHOD    : content
// =========================================================================
const std::string& SmtpMessage::content () const
{
    return _content;
}

// =========================================================================
//   CLASS     : SmtpMessage
//   METHOD    : writeHeaders
// =========================================================================
int SmtpMessage::writeHeaders (std::ostream& out) const
{
    SmtpRecipients to, cc, bcc;
    std::copy_if (_recipients.begin (), _recipients.end (), std::back_inserter (to), [] (auto const& t) {
        return t.type () == SmtpRecipient::Recipient;
    });
    std::copy_if (_recipients.begin (), _recipients.end (), std::back_inserter (cc), [] (auto const& t) {
        return t.type () == SmtpRecipient::CCRecipient;
    });
    std::copy_if (_recipients.begin (), _recipients.end (), std::back_inserter (bcc), [] (auto const& t) {
        return t.type () == SmtpRecipient::BCCRecipient;
    });

    // write date.
    out << "Date: ";
    std::time_t ti = std::time (nullptr);
    out << std::put_time (std::gmtime (&ti), "%a, %d %b %Y %H:%M:%S GMT");
    out << "\r\n";

    // write sender.
    out << "From: ";
    out << _sender;
    out << "\r\n";

    // write recipients.
    if (!to.empty ())
    {
        out << "To: ";
        std::copy (to.begin (), std::prev (to.end ()), std::ostream_iterator<SmtpRecipient> (out, ","));
        out << to.back ();
        out << "\r\n";
    }

    // write carbon copy recipients.
    if (!cc.empty ())
    {
        out << "Cc: ";
        std::copy (cc.begin (), std::prev (cc.end ()), std::ostream_iterator<SmtpRecipient> (out, ","));
        out << cc.back ();
        out << "\r\n";
    }

    // write black carbon copy recipients.
    if (!bcc.empty ())
    {
        out << "Bcc: ";
        std::copy (bcc.begin (), std::prev (bcc.end ()), std::ostream_iterator<SmtpRecipient> (out, ","));
        out << bcc.back ();
        out << "\r\n";
    }

    // write subject.
    out << "Subject: ";
    out << _subject;
    out << "\r\n";

    // write mime version.
    out << "MIME-Version: 1.0";
    out << "\r\n";

    // write content type.
    out << "Content-type: text/plain; charset=iso-8859-1";
    out << "\r\n";

    // write content transfert encoding.
    out << "Content-Transfer-Encoding: 7bit";
    out << "\r\n";

    // write end of headers.
    out << "\r\n";

    // flush data.
    out.flush ();

    return out.fail () ? -1 : 0;
}

// =========================================================================
//   CLASS     : SmtpMessage
//   METHOD    : writeContent
// =========================================================================
int SmtpMessage::writeContent (std::ostream& out) const
{
    // write content.
    out << _content;
    out << "\r\n";

    // end content.
    out << ".";
    out << "\r\n";

    // flush data.
    out.flush ();

    return out.fail () ? -1 : 0;
}
