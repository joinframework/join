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

#ifndef JOIN_DATA_SAX_HPP
#define JOIN_DATA_SAX_HPP

// libjoin.
#include <join/sax_error.hpp>
#include <join/value.hpp>

// C++.
#include <string_view>

// C.
#include <cstdint>
#include <cstddef>

namespace join
{
    /// max number of nested containers.
    constexpr size_t saxMaxDepth = 256;

    /**
     * @brief SAX handler ignoring every event, to derive from and use through the derived type.
     */
    struct SaxHandler
    {
        /**
         * @brief set null value.
         * @return 0.
         */
        int setNull () noexcept
        {
            return 0;
        }

        /**
         * @brief set boolean value.
         * @param value boolean value to set.
         * @return 0.
         */
        int setBool ([[maybe_unused]] bool value) noexcept
        {
            return 0;
        }

        /**
         * @brief set integer value.
         * @param value integer value to set.
         * @return 0.
         */
        int setInt ([[maybe_unused]] int32_t value) noexcept
        {
            return 0;
        }

        /**
         * @brief set unsigned integer value.
         * @param value unsigned integer value to set.
         * @return 0.
         */
        int setUint ([[maybe_unused]] uint32_t value) noexcept
        {
            return 0;
        }

        /**
         * @brief set 64 bits integer value.
         * @param value 64 bits integer value to set.
         * @return 0.
         */
        int setInt64 ([[maybe_unused]] int64_t value) noexcept
        {
            return 0;
        }

        /**
         * @brief set unsigned 64 bits integer value.
         * @param value unsigned 64 bits integer value to set.
         * @return 0.
         */
        int setUint64 ([[maybe_unused]] uint64_t value) noexcept
        {
            return 0;
        }

        /**
         * @brief set real value.
         * @param value real value to set.
         * @return 0.
         */
        int setDouble ([[maybe_unused]] double value) noexcept
        {
            return 0;
        }

        /**
         * @brief set string value.
         * @param value string value to set.
         * @return 0.
         */
        int setString ([[maybe_unused]] std::string_view value) noexcept
        {
            return 0;
        }

        /**
         * @brief start array.
         * @param size array size, absent when called by the JSON reader.
         * @return 0.
         */
        int startArray ([[maybe_unused]] uint32_t size = 0) noexcept
        {
            return 0;
        }

        /**
         * @brief stop array.
         * @return 0.
         */
        int stopArray () noexcept
        {
            return 0;
        }

        /**
         * @brief start object.
         * @param size object size, absent when called by the JSON reader.
         * @return 0.
         */
        int startObject ([[maybe_unused]] uint32_t size = 0) noexcept
        {
            return 0;
        }

        /**
         * @brief get object members in writing order.
         * @param object object to write.
         * @return object members in insertion order.
         */
        const Object& members (const Object& object) const noexcept
        {
            return object;
        }

        /**
         * @brief set null key.
         * @param key object key.
         * @return 0.
         */
        int setKey ([[maybe_unused]] std::nullptr_t key) noexcept
        {
            return 0;
        }

        /**
         * @brief set boolean key.
         * @param key object key.
         * @return 0.
         */
        int setKey ([[maybe_unused]] bool key) noexcept
        {
            return 0;
        }

        /**
         * @brief set integer key.
         * @param key object key.
         * @return 0.
         */
        int setKey ([[maybe_unused]] int32_t key) noexcept
        {
            return 0;
        }

        /**
         * @brief set unsigned integer key.
         * @param key object key.
         * @return 0.
         */
        int setKey ([[maybe_unused]] uint32_t key) noexcept
        {
            return 0;
        }

        /**
         * @brief set 64 bits integer key.
         * @param key object key.
         * @return 0.
         */
        int setKey ([[maybe_unused]] int64_t key) noexcept
        {
            return 0;
        }

        /**
         * @brief set unsigned 64 bits integer key.
         * @param key object key.
         * @return 0.
         */
        int setKey ([[maybe_unused]] uint64_t key) noexcept
        {
            return 0;
        }

        /**
         * @brief set real key.
         * @param key object key.
         * @return 0.
         */
        int setKey ([[maybe_unused]] double key) noexcept
        {
            return 0;
        }

        /**
         * @brief set string key.
         * @param key object key.
         * @return 0.
         */
        int setKey ([[maybe_unused]] std::string_view key) noexcept
        {
            return 0;
        }

        /**
         * @brief set string key.
         * @param key object key.
         * @return 0.
         */
        int setKey ([[maybe_unused]] const char* key) noexcept
        {
            return 0;
        }

        /**
         * @brief stop object.
         * @return 0.
         */
        int stopObject () noexcept
        {
            return 0;
        }

        /**
         * @brief reset the handler state after a failure.
         */
        void reset () noexcept
        {
            // do nothing.
        }
    };
}

#endif
