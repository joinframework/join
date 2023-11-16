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

// Libraries.
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using join::MailSender;
using join::MailRecipient;
using join::MailRecipients;
using join::MailMessage;

/**
 * @brief Test copy.
 */
TEST (MailMessage, copy)
{
    MailMessage message1, message2;

    message2.sender ({"foo@bar.com"});
    ASSERT_EQ (message1.sender ().address (), "");
    ASSERT_EQ (message2.sender ().address (), "foo@bar.com");

    message1 = message2;
    ASSERT_EQ (message1.sender ().address (), "foo@bar.com");

    MailMessage message3 (message1);
    ASSERT_EQ (message3.sender ().address (), "foo@bar.com");
}

/**
 * @brief Test move.
 */
TEST (MailMessage, move)
{
    MailMessage message1, message2;

    message2.sender ({"foo@bar.com"});
    ASSERT_EQ (message1.sender ().address (), "");
    ASSERT_EQ (message2.sender ().address (), "foo@bar.com");

    message1 = std::move (message2);
    ASSERT_EQ (message1.sender ().address (), "foo@bar.com");

    MailMessage message3 (std::move (message1));
    ASSERT_EQ (message3.sender ().address (), "foo@bar.com");
}

/**
 * @brief Test sender.
 */
TEST (MailMessage, sender)
{
    MailMessage message;
    ASSERT_TRUE (message.sender ().empty ());

    message.sender ({"foo@bar.com"});
    ASSERT_FALSE (message.sender ().empty ());
}

/**
 * @brief Test addRecipient.
 */
TEST (MailMessage, addRecipient)
{
    MailMessage message;
    ASSERT_TRUE (message.recipients ().empty ());

    message.addRecipient ({"foo@bar.com"});
    ASSERT_FALSE (message.recipients ().empty ());
}

/**
 * @brief Test subject.
 */
TEST (MailMessage, subject)
{
    MailMessage message;
    ASSERT_EQ (message.subject (), "");

    message.subject ("test");
    ASSERT_EQ (message.subject (), "test");
}

/**
 * @brief Test content.
 */
TEST (MailMessage, content)
{
    MailMessage message;
    ASSERT_EQ (message.content (), "");

    message.content ("test");
    ASSERT_EQ (message.content (), "test");
}

/**
 * @brief Test writeHeaders.
 */
TEST (MailMessage, writeHeaders)
{
    MailMessage message;
    message.sender ({"foo@bar.com", "foo"});
    message.addRecipient ({"baz@fun.com", "baz", MailRecipient::Recipient});
    message.addRecipient ({"nlo@fre.com", "nlo", MailRecipient::Recipient});
    message.addRecipient ({"bla@zom.com", "bla", MailRecipient::CCRecipient});
    message.addRecipient ({"hbd@qsd.com", "hbd", MailRecipient::CCRecipient});
    message.addRecipient ({"flu@mlo.com", "flu", MailRecipient::BCCRecipient});
    message.addRecipient ({"kjl@try.com", "kjl", MailRecipient::BCCRecipient});
    message.subject ("test");

    std::stringstream ss;
    ASSERT_EQ (message.writeHeaders (ss), 0) << join::lastError.message ();
    ASSERT_THAT (ss.str (), ::testing::MatchesRegex ("^"
                                                     "Date: .* GMT\r\n"
                                                     "From: foo<foo@bar.com>\r\n"
                                                     "To: baz<baz@fun.com>,nlo<nlo@fre.com>\r\n"
                                                     "Cc: bla<bla@zom.com>,hbd<hbd@qsd.com>\r\n"
                                                     "Bcc: flu<flu@mlo.com>,kjl<kjl@try.com>\r\n"
                                                     "Subject: test\r\n"
                                                     "MIME-Version: 1.0\r\n"
                                                     "Content-type: text/plain; charset=iso-8859-1\r\n"
                                                     "Content-Transfer-Encoding: 7bit\r\n"
                                                     "\r\n"
                                                     "$"));
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    testing::InitGoogleMock (&argc, argv);
    return RUN_ALL_TESTS ();
}
