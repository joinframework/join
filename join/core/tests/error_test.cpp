/**
 * MIT License
 *
 * Copyright (c) 2021 Mathieu Rabine
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
#include <join/error.hpp>

// Libraries.
#include <gtest/gtest.h>

// C++.
#include <utility>

using join::Errc;
using join::ErrorCategory;

/**
 * @brief Test name.
 */
TEST (ErrorCategory, name)
{
    EXPECT_STREQ (ErrorCategory ().name (), "libjoin");
}

/**
 * @brief Test message.
 */
TEST (ErrorCategory, message)
{
    EXPECT_STREQ (ErrorCategory ().message (static_cast <int> (Errc::InUse)).c_str (), "already in use");
    EXPECT_STREQ (ErrorCategory ().message (static_cast <int> (Errc::InvalidParam)).c_str (), "invalid parameters");
    EXPECT_STREQ (ErrorCategory ().message (static_cast <int> (Errc::ConnectionRefused)).c_str (), "connection refused");
    EXPECT_STREQ (ErrorCategory ().message (static_cast <int> (Errc::ConnectionClosed)).c_str (), "connection closed");
    EXPECT_STREQ (ErrorCategory ().message (static_cast <int> (Errc::TimedOut)).c_str (), "timer expired");
    EXPECT_STREQ (ErrorCategory ().message (static_cast <int> (Errc::PermissionDenied)).c_str (), "operation not permitted");
    EXPECT_STREQ (ErrorCategory ().message (static_cast <int> (Errc::OutOfMemory)).c_str (), "cannot allocate memory");
    EXPECT_STREQ (ErrorCategory ().message (static_cast <int> (Errc::OperationFailed)).c_str (), "operation failed");
    EXPECT_STREQ (ErrorCategory ().message (static_cast <int> (Errc::NotFound)).c_str (), "resource not found");
    EXPECT_STREQ (ErrorCategory ().message (static_cast <int> (Errc::MessageUnknown)).c_str (), "message unknown");
    EXPECT_STREQ (ErrorCategory ().message (static_cast <int> (Errc::MessageTooLong)).c_str (), "message too long");
    EXPECT_STREQ (ErrorCategory ().message (static_cast <int> (Errc::TemporaryError)).c_str (), "temporary error");
    EXPECT_STREQ (ErrorCategory ().message (static_cast <int> (Errc::UnknownError)).c_str (), "unknown error");
}

/**
 * @brief Test default_error_condition.
 */
TEST (ErrorCategory, default_error_condition)
{
    EXPECT_EQ (ErrorCategory ().default_error_condition (1).message(), "already in use");
    EXPECT_EQ (ErrorCategory ().default_error_condition (2).message(), "invalid parameters");
    EXPECT_EQ (ErrorCategory ().default_error_condition (3).message(), "connection refused");
    EXPECT_EQ (ErrorCategory ().default_error_condition (4).message(), "connection closed");
    EXPECT_EQ (ErrorCategory ().default_error_condition (5).message(), "timer expired");
    EXPECT_EQ (ErrorCategory ().default_error_condition (6).message(), "operation not permitted");
    EXPECT_EQ (ErrorCategory ().default_error_condition (7).message(), "cannot allocate memory");
    EXPECT_EQ (ErrorCategory ().default_error_condition (8).message(), "operation failed");
    EXPECT_EQ (ErrorCategory ().default_error_condition (9).message(), "resource not found");
    EXPECT_EQ (ErrorCategory ().default_error_condition (10).message(), "message unknown");
    EXPECT_EQ (ErrorCategory ().default_error_condition (11).message(), "message too long");
    EXPECT_EQ (ErrorCategory ().default_error_condition (12).message(), "temporary error");
    EXPECT_EQ (ErrorCategory ().default_error_condition (13).message(), "unknown error");
}

/**
 * @brief Test equivalent.
 */
TEST (ErrorCategory, equivalent)
{
    std::error_code code;
    code = std::make_error_code (std::errc::already_connected);
    EXPECT_EQ (code, Errc::InUse);
    code = std::make_error_code (std::errc::connection_already_in_progress);
    EXPECT_EQ (code, Errc::InUse);
    code = std::make_error_code (std::errc::address_in_use);
    EXPECT_EQ (code, Errc::InUse);
    code = std::make_error_code (std::errc::no_such_file_or_directory);
    EXPECT_EQ (code, Errc::InvalidParam);
    code = std::make_error_code (std::errc::address_family_not_supported);
    EXPECT_EQ (code, Errc::InvalidParam);
    code = std::make_error_code (std::errc::invalid_argument);
    EXPECT_EQ (code, Errc::InvalidParam);
    code = std::make_error_code (std::errc::protocol_not_supported);
    EXPECT_EQ (code, Errc::InvalidParam);
    code = std::make_error_code (std::errc::bad_file_descriptor);
    EXPECT_EQ (code, Errc::InvalidParam);
    code = std::make_error_code (std::errc::not_a_socket);
    EXPECT_EQ (code, Errc::InvalidParam);
    code = std::make_error_code (std::errc::bad_address);
    EXPECT_EQ (code, Errc::InvalidParam);
    code = std::make_error_code (std::errc::no_protocol_option);
    EXPECT_EQ (code, Errc::InvalidParam);
    code = std::make_error_code (std::errc::destination_address_required);
    EXPECT_EQ (code, Errc::InvalidParam);
    code = std::make_error_code (std::errc::operation_not_supported);
    EXPECT_EQ (code, Errc::InvalidParam);
    code = std::make_error_code (std::errc::connection_refused);
    EXPECT_EQ (code, Errc::ConnectionRefused);
    code = std::make_error_code (std::errc::network_unreachable);
    EXPECT_EQ (code, Errc::ConnectionRefused);
    code = std::make_error_code (std::errc::connection_reset);
    EXPECT_EQ (code, Errc::ConnectionClosed);
    code = std::make_error_code (std::errc::not_connected);
    EXPECT_EQ (code, Errc::ConnectionClosed);
    code = std::make_error_code (std::errc::broken_pipe);
    EXPECT_EQ (code, Errc::ConnectionClosed);
    code = std::make_error_code (std::errc::timed_out);
    EXPECT_EQ (code, Errc::TimedOut);
    code = std::make_error_code (std::errc::permission_denied);
    EXPECT_EQ (code, Errc::PermissionDenied);
    code = std::make_error_code (std::errc::operation_not_permitted);
    EXPECT_EQ (code, Errc::PermissionDenied);
    code = std::make_error_code (std::errc::too_many_files_open);
    EXPECT_EQ (code, Errc::OutOfMemory);
    code = std::make_error_code (std::errc::too_many_files_open_in_system);
    EXPECT_EQ (code, Errc::OutOfMemory);
    code = std::make_error_code (std::errc::no_buffer_space);
    EXPECT_EQ (code, Errc::OutOfMemory);
    code = std::make_error_code (std::errc::not_enough_memory);
    EXPECT_EQ (code, Errc::OutOfMemory);
    code = std::make_error_code (std::errc::no_lock_available);
    EXPECT_EQ (code, Errc::OutOfMemory);
    code = std::make_error_code (std::errc::no_message);
    EXPECT_EQ (code, Errc::MessageUnknown);
    code = std::make_error_code (std::errc::bad_message);
    EXPECT_EQ (code, Errc::MessageUnknown);
    code = std::make_error_code (std::errc::no_message_available);
    EXPECT_EQ (code, Errc::MessageUnknown);
    code = std::make_error_code (std::errc::message_size);
    EXPECT_EQ (code, Errc::MessageTooLong);
    code = std::make_error_code (std::errc::interrupted);
    EXPECT_EQ (code, Errc::TemporaryError);
    code = std::make_error_code (std::errc::resource_unavailable_try_again);
    EXPECT_EQ (code, Errc::TemporaryError);
    code = std::make_error_code (std::errc::operation_in_progress);
    EXPECT_EQ (code, Errc::TemporaryError);
}

/**
 * @brief Test equal.
 */
TEST (ErrorCategory, equal)
{
    std::error_code error1, error2;

    error1 = make_error_code (Errc::NotFound);
    error2 = make_error_code (Errc::NotFound);
    ASSERT_TRUE (error1 == error2);

    error1 = make_error_code (Errc::NotFound);
    error2 = make_error_code (Errc::PermissionDenied);
    ASSERT_FALSE (error1 == error2);

    error1 = make_error_code (Errc::NotFound);
    ASSERT_TRUE (error1 == Errc::NotFound);

    error1 = make_error_code (Errc::NotFound);
    ASSERT_FALSE (error1 == Errc::PermissionDenied);

    error2 = make_error_code (Errc::PermissionDenied);
    ASSERT_TRUE (Errc::PermissionDenied == error2);

    error2 = make_error_code (Errc::PermissionDenied);
    ASSERT_FALSE (Errc::NotFound == error2);
}

/**
 * @brief Test different.
 */
TEST (ErrorCategory, different)
{
    std::error_code error1, error2;

    error1 = make_error_code (Errc::NotFound);
    error2 = make_error_code (Errc::NotFound);
    ASSERT_FALSE (error1 != error2);

    error1 = make_error_code (Errc::NotFound);
    error2 = make_error_code (Errc::PermissionDenied);
    ASSERT_TRUE (error1 != error2);

    error1 = make_error_code (Errc::NotFound);
    ASSERT_FALSE (error1 != Errc::NotFound);

    error1 = make_error_code (Errc::NotFound);
    ASSERT_TRUE (error1 != Errc::PermissionDenied);

    error2 = make_error_code (Errc::PermissionDenied);
    ASSERT_FALSE (Errc::PermissionDenied != error2);

    error2 = make_error_code (Errc::PermissionDenied);
    ASSERT_TRUE (Errc::NotFound != error2);
}

/**
 * @brief Test make_error_code.
 */
TEST (ErrorCategory, make_error_code)
{
    auto code = make_error_code (Errc::InvalidParam);
    EXPECT_EQ (code, Errc::InvalidParam);
    EXPECT_STREQ (code.message ().c_str (), "invalid parameters");
}

/**
 * @brief Test make_error_condition.
 */
TEST (ErrorCategory, make_error_condition)
{
    auto code = make_error_condition (Errc::OutOfMemory);
    EXPECT_EQ (code, Errc::OutOfMemory);
    EXPECT_STREQ (code.message ().c_str (), "cannot allocate memory");
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
