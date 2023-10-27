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
#include <join/mailmessage.hpp>

// C++.
#include <algorithm>
#include <iterator>
#include <iomanip>

using join::MailSender;
using join::MailRecipient;
using join::MailRecipients;
using join::MailMessage;

// =========================================================================
//   CLASS     : MailSender
//   METHOD    : MailSender
// =========================================================================
MailSender::MailSender (const std::string& address)
: MailSender (address, "")
{
}

// =========================================================================
//   CLASS     : MailSender
//   METHOD    : MailSender
// =========================================================================
MailSender::MailSender (const std::string& address, const std::string& name)
: _address (address),
  _name (name)
{
}

// =========================================================================
//   CLASS     : MailSender
//   METHOD    : MailSender
// =========================================================================
MailSender::MailSender (const MailSender& other)
: _address (other._address),
  _name (other._name)
{
}

// =========================================================================
//   CLASS     : MailSender
//   METHOD    : operator=
// =========================================================================
MailSender& MailSender::operator= (const MailSender& other)
{
    _address = other._address;
    _name = other._name;
    return *this;
}

// =========================================================================
//   CLASS     : MailSender
//   METHOD    : MailSender
// =========================================================================
MailSender::MailSender (MailSender&& other)
: _address (std::move (other._address)),
  _name (std::move (other._name))
{
}

// =========================================================================
//   CLASS     : MailSender
//   METHOD    : operator=
// =========================================================================
MailSender& MailSender::operator= (MailSender&& other)
{
    _address = std::move (other._address);
    _name = std::move (other._name);
    return *this;
}

// =========================================================================
//   CLASS     : MailSender
//   METHOD    : address
// =========================================================================
void MailSender::address (const std::string& addr)
{
    _address = addr;
}

// =========================================================================
//   CLASS     : MailSender
//   METHOD    : address
// =========================================================================
const std::string& MailSender::address () const
{
    return _address;
}

// =========================================================================
//   CLASS     : MailSender
//   METHOD    : realName
// =========================================================================
void MailSender::realName (const std::string& name)
{
    _name = name;
}

// =========================================================================
//   CLASS     : MailSender
//   METHOD    : realName
// =========================================================================
const std::string& MailSender::realName () const
{
    return _name;
}

// =========================================================================
//   CLASS     : MailSender
//   METHOD    : empty
// =========================================================================
bool MailSender::empty () const
{
    return _address.empty ();
}

// =========================================================================
//   CLASS     :
//   METHOD    : operator<<
// =========================================================================
std::ostream& join::operator<< (std::ostream& out, const MailSender& sender)
{
    out << sender.realName () << "<" << sender.address () << ">";
    return out;
}

// =========================================================================
//   CLASS     : MailRecipient
//   METHOD    : MailRecipient
// =========================================================================
MailRecipient::MailRecipient (const std::string& address, Type t)
: MailRecipient (address, "", t)
{
}

// =========================================================================
//   CLASS     : MailRecipient
//   METHOD    : MailRecipient
// =========================================================================
MailRecipient::MailRecipient (const std::string& address, const std::string& name, Type t)
: MailSender (address, name),
  _type (t)
{
}

// =========================================================================
//   CLASS     : MailRecipient
//   METHOD    : MailRecipient
// =========================================================================
MailRecipient::MailRecipient (const MailRecipient& other)
: MailSender (other),
  _type (other._type)
{
}

// =========================================================================
//   CLASS     : MailRecipient
//   METHOD    : operator=
// =========================================================================
MailRecipient& MailRecipient::operator= (const MailRecipient& other)
{
    MailSender::operator= (other);
    _type = other._type;
    return *this;
}

// =========================================================================
//   CLASS     : MailRecipient
//   METHOD    : MailRecipient
// =========================================================================
MailRecipient::MailRecipient (MailRecipient&& other)
: MailSender (std::move (other)),
  _type (other._type)
{
}

// =========================================================================
//   CLASS     : MailRecipient
//   METHOD    : operator=
// =========================================================================
MailRecipient& MailRecipient::operator= (MailRecipient&& other)
{
    MailSender::operator= (std::move (other));
    _type = other._type;
    return *this;
}

// =========================================================================
//   CLASS     : MailRecipient
//   METHOD    : type
// =========================================================================
void MailRecipient::type (Type t)
{
    _type = t;
}

// =========================================================================
//   CLASS     : MailRecipient
//   METHOD    : type
// =========================================================================
MailRecipient::Type MailRecipient::type () const
{
    return _type;
}

// =========================================================================
//   CLASS     : MailMessage
//   METHOD    : MailMessage
// =========================================================================
MailMessage::MailMessage (const MailMessage& other)
: _sender (other._sender),
  _recipients (other._recipients),
  _subject (other._subject),
  _content (other._content)
{
}

// =========================================================================
//   CLASS     : MailMessage
//   METHOD    : operator=
// =========================================================================
MailMessage& MailMessage::operator= (const MailMessage& other)
{
    _sender = other._sender;
    _recipients = other._recipients;
    _subject = other._subject;
    _content = other._content;
    return *this;
}

// =========================================================================
//   CLASS     : MailMessage
//   METHOD    : MailMessage
// =========================================================================
MailMessage::MailMessage (MailMessage&& other)
: _sender (std::move (other._sender)),
  _recipients (std::move (other._recipients)),
  _subject (std::move (other._subject)),
  _content (std::move (other._content))
{
}

// =========================================================================
//   CLASS     : MailMessage
//   METHOD    : operator=
// =========================================================================
MailMessage& MailMessage::operator= (MailMessage&& other)
{
    _sender = std::move (other._sender);
    _recipients = std::move (other._recipients);
    _subject = std::move (other._subject);
    _content = std::move (other._content);
    return *this;
}

// =========================================================================
//   CLASS     : MailMessage
//   METHOD    : sender
// =========================================================================
void MailMessage::sender (const MailSender& from)
{
    _sender = from;
}

// =========================================================================
//   CLASS     : MailMessage
//   METHOD    : sender
// =========================================================================
const MailSender& MailMessage::sender () const
{
    return _sender;
}

// =========================================================================
//   CLASS     : MailMessage
//   METHOD    : addRecipient
// =========================================================================
void MailMessage::addRecipient (const MailRecipient& to)
{
    _recipients.push_back (to);
}

// =========================================================================
//   CLASS     : MailMessage
//   METHOD    : addRecipient
// =========================================================================
void MailMessage::addRecipient (MailRecipient&& to)
{
    _recipients.push_back (std::move (to));
}

// =========================================================================
//   CLASS     : MailMessage
//   METHOD    : recipients
// =========================================================================
const MailRecipients& MailMessage::recipients () const
{
    return _recipients;
}

// =========================================================================
//   CLASS     : MailMessage
//   METHOD    : subject
// =========================================================================
void MailMessage::subject (const std::string& subj)
{
    _subject = subj;
}

// =========================================================================
//   CLASS     : MailMessage
//   METHOD    : subject
// =========================================================================
const std::string& MailMessage::subject () const
{
    return _subject;
}

// =========================================================================
//   CLASS     : MailMessage
//   METHOD    : content
// =========================================================================
void MailMessage::content (const std::string& message)
{
    _content = message;
}

// =========================================================================
//   CLASS     : MailMessage
//   METHOD    : content
// =========================================================================
const std::string& MailMessage::content () const
{
    return _content;
}

// =========================================================================
//   CLASS     : MailMessage
//   METHOD    : writeHeaders
// =========================================================================
int MailMessage::writeHeaders (std::ostream& out) const
{
    MailRecipients to, cc, bcc;
    std::copy_if (_recipients.begin (), _recipients.end(), std::back_inserter (to), [] (auto const& t) { return t.type () == MailRecipient::Recipient; });
    std::copy_if (_recipients.begin (), _recipients.end(), std::back_inserter (cc), [] (auto const& t) { return t.type () == MailRecipient::CCRecipient; });
    std::copy_if (_recipients.begin (), _recipients.end(), std::back_inserter (bcc), [] (auto const& t) { return t.type () == MailRecipient::BCCRecipient; });

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
        std::copy (to.begin (), std::prev (to.end ()), std::ostream_iterator <MailRecipient> (out, ","));
        out << to.back ();
        out << "\r\n";
    }

    // write carbon copy recipients.
    if (!cc.empty ())
    {
        out << "Cc: ";
        std::copy (cc.begin (), std::prev (cc.end ()), std::ostream_iterator <MailRecipient> (out, ","));
        out << cc.back ();
        out << "\r\n";
    }

    // write black carbon copy recipients.
    if (!bcc.empty ())
    {
        out << "Bcc: ";
        std::copy (bcc.begin (), std::prev (bcc.end ()), std::ostream_iterator <MailRecipient> (out, ","));
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

    // write content encoding.
    out << "Content-Transfer-Encoding: 7bit";
    out << "\r\n";

    // write end of headers.
    out << "\r\n";

    return out.fail () ? -1 : 0;
}
