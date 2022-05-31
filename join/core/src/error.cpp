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

using join::Errc;
using join::ErrorCategory;

/// last error.
thread_local std::error_code join::lastError;

// =========================================================================
//   CLASS     : ErrorCategory
//   METHOD    : name
// =========================================================================
const char* ErrorCategory::name () const noexcept
{
    return "libjoin";
}

// =========================================================================
//   CLASS     : ErrorCategory
//   METHOD    : name
// =========================================================================
std::string ErrorCategory::message (int code) const
{
    switch (static_cast <Errc> (code))
    {
        case Errc::InUse:
            return "already in use";
        case Errc::InvalidParam:
            return "invalid parameters";
        case Errc::ConnectionRefused:
            return "connection refused";
        case Errc::ConnectionClosed:
            return "connection closed";
        case Errc::TimedOut:
            return "timer expired";
        case Errc::PermissionDenied:
            return "operation not permitted";
        case Errc::OutOfMemory:
            return "cannot allocate memory";
        case Errc::OperationFailed:
            return "operation failed";
        case Errc::NotFound:
            return "resource not found";
        case Errc::MessageUnknown:
            return "message unknown";
        case Errc::MessageTooLong:
            return "message too long";
        case Errc::TemporaryError:
            return "temporary error";
        case Errc::UnknownError:
            return "unknown error";
        default:
            return "success";
    }
}

// =========================================================================
//   CLASS     : ErrorCategory
//   METHOD    : equivalent
// =========================================================================
bool ErrorCategory::equivalent (const std::error_code& code, int condition) const noexcept
{
    switch (static_cast <Errc> (condition))
    {
        case Errc::InUse:
            return code == std::errc::already_connected ||
                   code == std::errc::connection_already_in_progress ||
                   code == std::errc::address_in_use;
        case Errc::InvalidParam:
            return code == std::errc::no_such_file_or_directory ||
                   code == std::errc::address_family_not_supported ||
                   code == std::errc::invalid_argument ||
                   code == std::errc::protocol_not_supported ||
                   code == std::errc::not_a_socket ||
                   code == std::errc::bad_address ||
                   code == std::errc::no_protocol_option ||
                   code == std::errc::destination_address_required ||
                   code == std::errc::operation_not_supported;
        case Errc::ConnectionRefused:
            return code == std::errc::connection_refused ||
                   code == std::errc::network_unreachable;
        case Errc::ConnectionClosed:
            return code == std::errc::connection_reset ||
                   code == std::errc::not_connected ||
                   code == std::errc::broken_pipe;
        case Errc::TimedOut:
            return code == std::errc::timed_out;
        case Errc::PermissionDenied:
            return code == std::errc::permission_denied ||
                   code == std::errc::operation_not_permitted;
        case Errc::OutOfMemory:
            return code == std::errc::too_many_files_open ||
                   code == std::errc::too_many_files_open_in_system ||
                   code == std::errc::no_buffer_space ||
                   code == std::errc::not_enough_memory ||
                   code == std::errc::no_lock_available;
        case Errc::OperationFailed:
            return code == std::errc::bad_file_descriptor;
        case Errc::MessageUnknown:
            return code == std::errc::no_message ||
                   code == std::errc::bad_message ||
                   code == std::errc::no_message_available;
        case Errc::MessageTooLong:
            return code == std::errc::message_size;
        case Errc::TemporaryError:
            return code == std::errc::interrupted ||
                   code == std::errc::resource_unavailable_try_again ||
                   code == std::errc::operation_in_progress;
        default:
            return false;
    }
}

// =========================================================================
//   CLASS     :
//   METHOD    : getErrorCategory
// =========================================================================
const std::error_category& join::getErrorCategory ()
{
    static ErrorCategory instance;
    return instance;
}

// =========================================================================
//   CLASS     :
//   METHOD    : make_error_code
// =========================================================================
std::error_code join::make_error_code (join::Errc code)
{
    return std::error_code (static_cast <int> (code), getErrorCategory ());
}

// =========================================================================
//   CLASS     :
//   METHOD    : make_error_condition
// =========================================================================
std::error_condition join::make_error_condition (join::Errc code)
{
    return std::error_condition (static_cast <int> (code), getErrorCategory ());
}
