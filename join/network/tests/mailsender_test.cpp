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

using join::MailSender;

/**
 * @brief Test copy.
 */
TEST (MailSender, copy)
{
    MailSender sender1, sender2 ("foo@bar.com");
    ASSERT_EQ (sender1.address (), "");
    ASSERT_EQ (sender2.address (), "foo@bar.com");

    sender1 = sender2;
    ASSERT_EQ (sender1.address (), "foo@bar.com");

    MailSender sender3 (sender1);
    ASSERT_EQ (sender3.address (), "foo@bar.com");
}

/**
 * @brief Test move.
 */
TEST (MailSender, move)
{
    MailSender sender1, sender2 ("foo@bar.com");
    ASSERT_EQ (sender1.address (), "");
    ASSERT_EQ (sender2.address (), "foo@bar.com");

    sender1 = std::move (sender2);
    ASSERT_EQ (sender1.address (), "foo@bar.com");

    MailSender sender3 (std::move (sender1));
    ASSERT_EQ (sender3.address (), "foo@bar.com");
}

/**
 * @brief Test address.
 */
TEST (MailSender, address)
{
    MailSender sender;
    ASSERT_EQ (sender.address (), "");

    sender.address ("foo@bar.com");
    ASSERT_EQ (sender.address (), "foo@bar.com");
}

/**
 * @brief Test realName.
 */
TEST (MailSender, realName)
{
    MailSender sender;
    ASSERT_EQ (sender.realName (), "");

    sender.realName ("foo");
    ASSERT_EQ (sender.realName (), "foo");
}

/**
 * @brief Test empty.
 */
TEST (MailSender, empty)
{
    MailSender sender;
    ASSERT_TRUE (sender.empty ());

    sender.address ("foo@bar.com");
    ASSERT_FALSE (sender.empty ());
}

/**
 * @brief Test serialize.
 */
TEST (MailSender, serialize)
{
    std::stringstream stream;
    MailSender sender ("foo@bar.com", "foo");

    ASSERT_NO_THROW (stream << sender);
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
