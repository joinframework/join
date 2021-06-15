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
 
#ifndef __JOIN_ERROR_HPP__
#define __JOIN_ERROR_HPP__

// C++.
#include <system_error>

namespace join
{
    /// last error.
    extern thread_local std::error_code lastError;

    /**
     * @brief Generic error codes.
     */
    enum class Errc
    {
        InUse = 1,              /**< Already in use. */
        InvalidParam,           /**< Invalid parameters were used. */
        ConnectionRefused,      /**< The connection was refused. */
        ConnectionClosed,       /**< The connection was closed by the peer. */
        TimedOut,               /**< The operation timed out. */
        PermissionDenied,       /**< The operation was not permitted.*/
        OutOfMemory,            /**< The operation run out of memory. */
        OperationFailed,        /**< The operation failed. */
        NotFound,               /**< Resource not found. */
        MessageUnknown,         /**< Message unknown. */
        MessageTooLong,         /**< Message too long. */
        TemporaryError,         /**< A temporary error occurred, operation should be performed again later. */
        UnknownError            /**< An unknown error occurred. */
    };

    /**
     * @brief Error category.
     */
    class ErrorCategory : public std::error_category
    {
    public:
        /**
         * @brief Get error category name.
         * @return Error category name.
         */
        virtual const char* name () const noexcept;

        /**
         * @brief Translate error code to human readable error string.
         * @param code error code.
         * @return Human readable error string.
         */
        virtual std::string message (int code) const;

        /**
         * @brief find equivalent from Errc to system error code.
         * @param code System error code.
         * @param condition Errc.
         * @return true if equivalent, false otherwise.
         */
        virtual bool equivalent (const std::error_code& code, int condition) const noexcept;
    };

    /**
     * @brief Get error category.
     * @return the created std::error_category object.
     */
    const std::error_category& getErrorCategory ();

    /**
     * @brief Create an std::error_code object.
     * @param code Error code number.
     * @return The created std::error_code object.
     */
    std::error_code make_error_code (join::Errc code);

    /**
     * @brief Create an std::error_condition object.
     * @param code Error code number.
     * @return The created std::error_condition object.
     */
    std::error_condition make_error_condition (join::Errc code);
}

namespace std
{
    template <> struct is_error_condition_enum <join::Errc> : public true_type {};
}

#endif
