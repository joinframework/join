/**
 * MIT License
 *
 * Copyright (c) 2026 Mathieu Rabine
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

#ifndef JOIN_CORE_TLS_ERROR_HPP
#define JOIN_CORE_TLS_ERROR_HPP

// C++.
#include <system_error>

namespace join
{
    /**
     * @brief TLS error codes.
     */
    enum class TlsErrc
    {
        TlsCloseNotifyAlert = 1, /**< A close notify alert was received. */
        TlsProtocolError         /**< A failure in the TLS library occurred, usually a protocol error. */
    };

    /**
     * @brief TLS error category.
     */
    class TlsCategory : public std::error_category
    {
    public:
        /**
         * @brief get digest error category name.
         * @return digest error category name.
         */
        virtual const char* name () const noexcept;

        /**
         * @brief translate digest error code to human readable error string.
         * @param code error code.
         * @return human readable error string.
         */
        virtual std::string message (int code) const;
    };

    /**
     * @brief get error category.
     * @return the created std::error_category object.
     */
    const std::error_category& getTlsCategory ();

    /**
     * @brief create an std::error_code object.
     * @param code error code number.
     * @return the created std::error_code object.
     */
    std::error_code make_error_code (TlsErrc code);

    /**
     * @brief create an std::error_condition object.
     * @param code error code number.
     * @return the created std::error_condition object.
     */
    std::error_condition make_error_condition (TlsErrc code);
}

namespace std
{
    /// TLS error code specialization.
    template <>
    struct is_error_condition_enum<join::TlsErrc> : public true_type
    {
    };
}

#endif
