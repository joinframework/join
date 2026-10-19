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
#include <bits/enable_special_members.h>
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
     * @brief get element type in a parameter pack according its position.
     */
    template <std::size_t I, typename T, typename... Ts>
    struct find_element
    {
        using type = typename find_element<I - 1, Ts...>::type;
    };

    /**
     * @brief get element type in a parameter pack according its position.
     */
    template <typename T, typename... Ts>
    struct find_element<0, T, Ts...>
    {
        using type = T;
    };

    /**
     * @brief get element type in a parameter pack according its position.
     */
    template <std::size_t I, typename... Ts>
    using find_element_t = typename find_element<I, Ts...>::type;

    /**
     * @brief get element type in a parameter pack according its position.
     */
    template <std::size_t I, typename T>
    struct find_element<I, const T>
    {
        using type = std::add_const_t<find_element_t<I, T>>;
    };

    /**
     * @brief get element type in a parameter pack according its position.
     */
    template <std::size_t I, typename T>
    struct find_element<I, volatile T>
    {
        using type = std::add_volatile_t<find_element_t<I, T>>;
    };

    /**
     * @brief get element type in a parameter pack according its position.
     */
    template <std::size_t I, typename T>
    struct find_element<I, const volatile T>
    {
        using type = std::add_cv_t<find_element_t<I, T>>;
    };

    /**
     * @brief check if a type exists in a parameter pack.
     */
    template <typename T, typename... Ts>
    struct is_alternative : std::bool_constant<(std::is_same_v<T, Ts> || ...)>
    {
    };

    /**
     * @brief check if index exists in a parameter pack.
     */
    template <std::size_t I, typename... Ts>
    struct is_index : std::bool_constant<(I < sizeof...(Ts))>
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

    /**
     * @brief check if an entire sequence is true.
     */
    template <bool... Ts>
    using all = std::bool_constant<(Ts && ...)>;

    /**
     * @brief check if types in a parameter pack are copy constructible.
     */
    template <typename... Ts>
    struct are_copy_constructible : std::bool_constant<(std::is_copy_constructible_v<Ts> && ...)>
    {
    };

    /**
     * @brief check if types in a parameter pack are move constructible.
     */
    template <typename... Ts>
    struct are_move_constructible : std::bool_constant<(std::is_move_constructible_v<Ts> && ...)>
    {
    };

    /**
     * @brief check if types in a parameter pack are copy assignable.
     */
    template <typename... Ts>
    struct are_copy_assignable
    : std::bool_constant<are_copy_constructible<Ts...>::value && are_move_constructible<Ts...>::value &&
                         (std::is_copy_assignable_v<Ts> && ...)>
    {
    };

    /**
     * @brief check if types in a parameter pack are move assignable.
     */
    template <typename... Ts>
    struct are_move_assignable
    : std::bool_constant<are_move_constructible<Ts...>::value && (std::is_move_assignable_v<Ts> && ...)>
    {
    };

    /**
     * @brief check if types in a parameter pack are trivially copyable.
     */
    template <typename... Ts>
    struct are_trivially_copyable : std::bool_constant<(std::is_trivially_copyable_v<Ts> && ...)>
    {
    };

    /**
     * @brief allow construction when default constructor is disabled.
     */
    struct EnableDefaultTag
    {
        explicit constexpr EnableDefaultTag () = default;
    };

    /**
     * @brief enable default constructor.
     */
    template <bool _Switch, typename _Tag = void>
    struct EnableDefault
    {
        constexpr EnableDefault () noexcept = default;
        constexpr EnableDefault (EnableDefault const&) noexcept = default;
        constexpr EnableDefault (EnableDefault&&) noexcept = default;

        constexpr explicit EnableDefault (EnableDefaultTag)
        {
        }

        EnableDefault& operator= (EnableDefault const&) noexcept = default;
        EnableDefault& operator= (EnableDefault&&) noexcept = default;
    };

    /**
     * @brief disable default constructor.
     */
    template <typename _Tag>
    struct EnableDefault<false, _Tag>
    {
        constexpr EnableDefault () noexcept = delete;
        constexpr EnableDefault (EnableDefault const&) noexcept = default;
        constexpr EnableDefault (EnableDefault&&) noexcept = default;

        constexpr explicit EnableDefault (EnableDefaultTag)
        {
        }

        EnableDefault& operator= (EnableDefault const&) noexcept = default;
        EnableDefault& operator= (EnableDefault&&) noexcept = default;
    };
}

#endif
