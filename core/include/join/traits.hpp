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

#ifndef JOIN_CORE_TRAITS_HPP
#define JOIN_CORE_TRAITS_HPP

// C++.
#include <type_traits>
#include <utility>

namespace join
{
    /**
     * @brief overload resolution.
     */
    template <typename... Ts>
    struct overload;

    /**
     * @brief overload resolution.
     */
    template <>
    struct overload<>
    {
        void operator() () const;
    };

    /**
     * @brief overload resolution.
     */
    template <typename First, typename... Ts>
    struct overload<First, Ts...> : overload<Ts...>
    {
        using overload<Ts...>::operator();

        std::type_identity<First> operator() (First) const;
    };

    /**
     * @brief overload resolution.
     */
    template <typename... Ts>
    struct overload<void, Ts...> : overload<Ts...>
    {
        using overload<Ts...>::operator();

        std::type_identity<void> operator() () const;
    };

    /**
     * @brief find a type that match one of the alternatives of a parameter pack.
     */
    template <typename T, typename... Ts>
    using match_t = typename std::invoke_result_t<overload<Ts...>, T>::type;

    /**
     * @brief get element position in a parameter pack according its type.
     */
    template <typename T, typename... Ts>
    struct find_index : std::integral_constant<std::size_t, 0>
    {
    };

    /**
     * @brief get element position in a parameter pack according its type.
     */
    template <typename T, typename First, typename... Ts>
    struct find_index<T, First, Ts...>
    : std::integral_constant<std::size_t, std::is_same_v<T, First> ? 0 : find_index<T, Ts...>::value + 1>
    {
    };

    /**
     * @brief count the number of time a type appears in a parameter pack.
     */
    template <typename T, typename... Ts>
    struct count : std::integral_constant<std::size_t, (std::size_t{0} + ... + std::is_same_v<T, Ts>)>
    {
    };

    /**
     * @brief check if a type only appears one time in a parameter pack.
     */
    template <typename T, typename... Ts>
    struct is_unique : std::bool_constant<count<T, Ts...>::value == 1>
    {
    };
}

#endif
