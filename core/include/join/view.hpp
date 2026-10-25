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

#ifndef JOIN_CORE_VIEW_HPP
#define JOIN_CORE_VIEW_HPP

// C++.
#include <type_traits>
#include <string_view>
#include <algorithm>
#include <concepts>
#include <compare>
#include <limits>

// C.
#include <cassert>
#include <cstdint>
#include <cstddef>

namespace join
{
    /**
     * @brief non-owning contiguous sequence of elements.
     */
    template <typename Type>
    class __attribute__ ((packed)) View
    {
    public:
        /**
         * @brief create an empty view.
         */
        constexpr View () noexcept = default;

        /**
         * @brief create a view.
         * @param data first element.
         * @param size number of elements, must fit in 32 bits.
         */
        constexpr View (Type* data, size_t size) noexcept
        : _data (data)
        , _size (static_cast<uint32_t> (size))
        {
            assert (size <= std::numeric_limits<uint32_t>::max ());
        }

        /**
         * @brief get the first element.
         * @return pointer to the first element.
         */
        constexpr Type* data () const noexcept
        {
            return _data;
        }

        /**
         * @brief get the number of elements.
         * @return number of elements.
         */
        constexpr size_t size () const noexcept
        {
            return _size;
        }

        /**
         * @brief check if the view is empty.
         * @return true if empty.
         */
        constexpr bool empty () const noexcept
        {
            return _size == 0;
        }

        /**
         * @brief get an iterator to the first element.
         * @return iterator to the first element.
         */
        constexpr Type* begin () const noexcept
        {
            return _data;
        }

        /**
         * @brief get an iterator past the last element.
         * @return iterator past the last element.
         */
        constexpr Type* end () const noexcept
        {
            return _data + _size;
        }

        /**
         * @brief access an element.
         * @param pos element position.
         * @return element.
         */
        constexpr Type& operator[] (size_t pos) const noexcept
        {
            return _data[pos];
        }

    private:
        /// first element.
        Type* _data = nullptr;

        /// number of elements.
        uint32_t _size = 0;
    };

    /**
     * @brief compare views.
     * @param lhs view to compare.
     * @param rhs view to compare to.
     * @return true if equal.
     */
    template <typename Type>
    constexpr bool operator== (const View<Type>& lhs, const View<Type>& rhs)
    {
        return std::equal (lhs.begin (), lhs.end (), rhs.begin (), rhs.end ());
    }

    /**
     * @brief three-way compare views.
     * @param lhs view to compare.
     * @param rhs view to compare to.
     * @return ordering of lhs relative to rhs.
     */
    template <typename Type>
    constexpr std::partial_ordering operator<=> (const View<Type>& lhs, const View<Type>& rhs)
    {
        return std::lexicographical_compare_three_way (lhs.begin (), lhs.end (), rhs.begin (), rhs.end ());
    }

    /**
     * @brief compare character views.
     * @param lhs view to compare.
     * @param rhs view to compare to.
     * @return true if equal.
     */
    template <typename Type>
        requires std::same_as<std::remove_const_t<Type>, char>
    constexpr bool operator== (const View<Type>& lhs, const View<Type>& rhs)
    {
        return std::string_view (lhs.data (), lhs.size ()) == std::string_view (rhs.data (), rhs.size ());
    }

    /**
     * @brief three-way compare character views.
     * @param lhs view to compare.
     * @param rhs view to compare to.
     * @return ordering of lhs relative to rhs.
     */
    template <typename Type>
        requires std::same_as<std::remove_const_t<Type>, char>
    constexpr std::partial_ordering operator<=> (const View<Type>& lhs, const View<Type>& rhs)
    {
        return std::string_view (lhs.data (), lhs.size ()) <=> std::string_view (rhs.data (), rhs.size ());
    }
}

#endif
