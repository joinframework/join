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

using join::MailRecipient;

/**
 * @brief Test copy.
 */
TEST (MailRecipient, copy)
{
    MailRecipient recipient1, recipient2 ("foo@bar.com");
    ASSERT_EQ (recipient1.address (), "");
    ASSERT_EQ (recipient2.address (), "foo@bar.com");

    recipient1 = recipient2;
    ASSERT_EQ (recipient1.address (), "foo@bar.com");

    MailRecipient recipient3 (recipient1);
    ASSERT_EQ (recipient3.address (), "foo@bar.com");
}

/**
 * @brief Test move.
 */
TEST (MailRecipient, move)
{
    MailRecipient recipient1, recipient2 ("foo@bar.com");
    ASSERT_EQ (recipient1.address (), "");
    ASSERT_EQ (recipient2.address (), "foo@bar.com");

    recipient1 = std::move (recipient2);
    ASSERT_EQ (recipient1.address (), "foo@bar.com");

    MailRecipient recipient3 (std::move (recipient1));
    ASSERT_EQ (recipient3.address (), "foo@bar.com");
}

/**
 * @brief Test address.
 */
TEST (MailRecipient, address)
{
    MailRecipient recipient;
    ASSERT_EQ (recipient.address (), "");

    recipient.address ("foo@bar.com");
    ASSERT_EQ (recipient.address (), "foo@bar.com");
}

/**
 * @brief Test realName.
 */
TEST (MailRecipient, realName)
{
    MailRecipient recipient;
    ASSERT_EQ (recipient.realName (), "");

    recipient.realName ("foo");
    ASSERT_EQ (recipient.realName (), "foo");
}

/**
 * @brief Test type.
 */
TEST (MailRecipient, type)
{
    MailRecipient recipient;
    ASSERT_EQ (recipient.type (), MailRecipient::Recipient);

    recipient.type (MailRecipient::CCRecipient);
    ASSERT_EQ (recipient.type (), MailRecipient::CCRecipient);
}

/**
 * @brief Test empty.
 */
TEST (MailRecipient, empty)
{
    MailRecipient recipient;
    ASSERT_TRUE (recipient.empty ());

    recipient.address ("foo@bar.com");
    ASSERT_FALSE (recipient.empty ());
}

/**
 * @brief Test serialize.
 */
TEST (MailRecipient, serialize)
{
    std::stringstream stream;
    MailRecipient recipient ("foo@bar.com", "foo");

    ASSERT_NO_THROW (stream << recipient);
    ASSERT_EQ (stream.str (), "foo<foo@bar.com>");
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
