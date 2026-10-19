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

#ifndef JOIN_CORE_VARIANT_HPP
#define JOIN_CORE_VARIANT_HPP

// libjoin.
#include <join/traits.hpp>

// C++.
#include <algorithm>
#include <typeinfo>
#include <tuple>

// C.
#include <cstddef>

namespace join
{
    namespace details
    {
        /**
         * @brief helper class for variant operations using jump tables for O(1) dispatch.
         */
        template <typename... Ts>
        struct VariantHelper
        {
        public:
            /**
             * @brief copy one object to another.
             * @param which internal storage index of the active alternative.
             * @param src source storage pointer.
             * @param dst destination storage pointer.
             */
            static void copy (std::size_t which, const void* src, void* dst)
            {
                static const CopyFunc table[] = {&copyImpl<Ts>...};
                table[which](src, dst);
            }

            /**
             * @brief move one object to another.
             * @param which internal storage index of the active alternative.
             * @param src source storage pointer.
             * @param dst destination storage pointer.
             */
            static void move (std::size_t which, void* src, void* dst)
            {
                static const MoveFunc table[] = {&moveImpl<Ts>...};
                table[which](src, dst);
            }

            /**
             * @brief destroy the currently active object.
             * @param which internal storage index of the active alternative.
             * @param data storage pointer.
             */
            static void destroy (std::size_t which, void* data)
            {
                static const DestroyFunc table[] = {&destroyImpl<Ts>...};
                table[which](data);
            }

            /**
             * @brief compare if two objects of the same alternative are equal.
             * @param which internal storage index of the active alternative.
             * @param a first storage pointer.
             * @param b second storage pointer.
             * @return true if equal, false otherwise.
             */
            static bool equal (std::size_t which, const void* a, const void* b)
            {
                static const EqualFunc table[] = {&equalImpl<Ts>...};
                return table[which](a, b);
            }

            /**
             * @brief compare if one object is lower than another of the same alternative.
             * @param which internal storage index of the active alternative.
             * @param a first storage pointer.
             * @param b second storage pointer.
             * @return true if a is lower than b, false otherwise.
             */
            static bool lower (std::size_t which, const void* a, const void* b)
            {
                static const LowerFunc table[] = {&lowerImpl<Ts>...};
                return table[which](a, b);
            }

        private:
            /// function pointer type for copy operations.
            using CopyFunc = void (*) (const void*, void*);

            /// function pointer type for move operations.
            using MoveFunc = void (*) (void*, void*);

            /// function pointer type for destroy operations.
            using DestroyFunc = void (*) (void*);

            /// function pointer type for equality comparisons.
            using EqualFunc = bool (*) (const void*, const void*);

            /// function pointer type for less-than comparisons.
            using LowerFunc = bool (*) (const void*, const void*);

            /**
             * @brief copy construct a T into dst from src.
             * @tparam T type of the alternative to copy.
             * @param src source storage pointer.
             * @param dst destination storage pointer.
             */
            template <typename T>
            static void copyImpl (const void* src, void* dst)
            {
                new (dst) T (*reinterpret_cast<const T*> (src));
            }

            /**
             * @brief move construct a T into dst from src.
             * @tparam T type of the alternative to move.
             * @param src source storage pointer.
             * @param dst destination storage pointer.
             */
            template <typename T>
            static void moveImpl (void* src, void* dst)
            {
                new (dst) T (std::move (*reinterpret_cast<T*> (src)));
            }

            /**
             * @brief destroy a T in place.
             * @tparam T type of the alternative to destroy.
             * @param data storage pointer.
             */
            template <typename T>
            static void destroyImpl (void* data)
            {
                reinterpret_cast<T*> (data)->~T ();
            }

            /**
             * @brief compare two T objects for equality.
             * @tparam T type of the alternative to compare.
             * @param a first storage pointer.
             * @param b second storage pointer.
             * @return true if equal, false otherwise.
             */
            template <typename T>
            static bool equalImpl (const void* a, const void* b)
            {
                return *reinterpret_cast<const T*> (a) == *reinterpret_cast<const T*> (b);
            }

            /**
             * @brief less-than comparison, always false for nullptr_t.
             * @tparam T type of the alternative to compare.
             * @param a first storage pointer.
             * @param b second storage pointer.
             * @return true if a is lower than b, false otherwise.
             */
            template <typename T>
            static bool lowerImpl (const void* a, const void* b)
            {
                if constexpr (std::is_null_pointer_v<T>)
                {
                    return false;
                }
                else
                {
                    return *reinterpret_cast<const T*> (a) < *reinterpret_cast<const T*> (b);
                }
            }
        };
    }

    /**
     * @brief variant class.
     */
    template <typename... Ts>
    class Variant
    {
        /// all alternatives are trivially copyable.
        static constexpr bool _trivial = (std::is_trivially_copyable_v<Ts> && ...);

        /// first alternative is default constructible.
        static constexpr bool _defaultConstructible =
            std::is_default_constructible_v<std::tuple_element_t<0, std::tuple<Ts...>>>;

        /// all alternatives are copy constructible.
        static constexpr bool _copyConstructible = (std::is_copy_constructible_v<Ts> && ...);

        /// all alternatives are move constructible.
        static constexpr bool _moveConstructible = (std::is_move_constructible_v<Ts> && ...);

        /// all alternatives are copy assignable.
        static constexpr bool _copyAssignable =
            _copyConstructible && _moveConstructible && (std::is_copy_assignable_v<Ts> && ...);

        /// all alternatives are move assignable.
        static constexpr bool _moveAssignable = _moveConstructible && (std::is_move_assignable_v<Ts> && ...);

    public:
        static_assert (0 < sizeof...(Ts), "Variant must have at least one alternative");
        static_assert ((!std::is_void_v<Ts> && ...), "Variant must have no void alternative");
        static_assert ((!std::is_array_v<Ts> && ...), "Variant must have no array alternative.");
        static_assert ((!std::is_reference_v<Ts> && ...), "Variant must have no reference alternative");

        /**
         * @brief default constructor.
         */
        constexpr Variant ()
            requires _defaultConstructible
        : Variant (std::in_place_index<0>)
        {
        }

        /**
         * @brief copy constructor.
         * @param other object to copy.
         */
        constexpr Variant (const Variant& other)
            requires (_trivial && _copyConstructible)
        = default;

        /**
         * @brief copy constructor.
         * @param other object to copy.
         */
        constexpr Variant (const Variant& other)
            requires (!_trivial && _copyConstructible)
        : _which (other._which)
        {
            details::VariantHelper<Ts...>::copy (other._which, other.storage (), storage ());
        }

        /**
         * @brief move constructor.
         * @param other object to move.
         */
        constexpr Variant (Variant&& other)
            requires (_trivial && _moveConstructible)
        = default;

        /**
         * @brief move constructor.
         * @param other object to move.
         */
        constexpr Variant (Variant&& other) noexcept
            requires (!_trivial && _moveConstructible)
        : _which (other._which)
        {
            details::VariantHelper<Ts...>::move (other._which, other.storage (), storage ());
        }

        /**
         * @brief constructs a Variant holding the alternative selected by overload resolution.
         * @param t Value convertible to one of the variant's alternatives
         */
        template <typename T, typename Match = match_t<T&&, Ts...>>
        constexpr Variant (T&& t)
            requires is_unique<Match, Ts...>::value && std::is_constructible_v<Match, T&&>
        : Variant (std::in_place_index<find_index<Match, Ts...>::value>, std::forward<T> (t))
        {
        }

        /**
         * @brief constructs a Variant with the specified alternative T.
         * @param args arguments to initialize the contained value with.
         */
        template <typename T, typename... Args>
        constexpr explicit Variant (std::in_place_type_t<T>, Args&&... args)
            requires is_unique<T, Ts...>::value && std::is_constructible_v<T, Args&&...>
        : Variant (std::in_place_index<find_index<T, Ts...>::value>, std::forward<Args> (args)...)
        {
        }

        /**
         * @brief constructs a Variant with the specified alternative T.
         * @param args arguments to initialize the contained value with.
         */
        template <typename T, typename Up, typename... Args>
        constexpr explicit Variant (std::in_place_type_t<T>, std::initializer_list<Up> il, Args&&... args)
            requires is_unique<T, Ts...>::value && std::is_constructible_v<T, std::initializer_list<Up>&, Args&&...>
        : Variant (std::in_place_index<find_index<T, Ts...>::value>, il, std::forward<Args> (args)...)
        {
        }

        /**
         * @brief constructs a variant with the alternative T specified by the index I.
         * @param args arguments to initialize the contained value with.
         */
        template <std::size_t I, typename... Args>
        constexpr explicit Variant (std::in_place_index_t<I>, Args&&... args)
            requires std::is_constructible_v<std::tuple_element_t<I, std::tuple<Ts...>>, Args&&...>
        : _which (I)
        {
            new (storage ()) std::tuple_element_t<I, std::tuple<Ts...>> (std::forward<Args> (args)...);
        }

        /**
         * @brief constructs a variant with the alternative T specified by the index I.
         * @param args arguments to initialize the contained value with.
         */
        template <std::size_t I, typename Up, typename... Args>
        constexpr explicit Variant (std::in_place_index_t<I>, std::initializer_list<Up> il, Args&&... args)
            requires std::is_constructible_v<std::tuple_element_t<I, std::tuple<Ts...>>, std::initializer_list<Up>&,
                                             Args&&...>
        : _which (I)
        {
            new (storage ()) std::tuple_element_t<I, std::tuple<Ts...>> (il, std::forward<Args> (args)...);
        }

        /**
         * @brief destroy the Variant instance.
         */
        ~Variant ()
            requires _trivial
        = default;

        /**
         * @brief destroy the Variant instance.
         */
        ~Variant ()
            requires (!_trivial)
        {
            details::VariantHelper<Ts...>::destroy (_which, storage ());
        }

        /**
         * @brief copy assignment.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        constexpr Variant& operator= (const Variant& other)
            requires (_trivial && _copyAssignable)
        = default;

        /**
         * @brief copy assignment.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        constexpr Variant& operator= (const Variant& other)
            requires (!_trivial && _copyAssignable)
        {
            Variant tmp (other);
            swap (tmp);
            return *this;
        }

        /**
         * @brief move assignment.
         * @param other object to move.
         * @return a reference of the current object.
         */
        constexpr Variant& operator= (Variant&& other)
            requires (_trivial && _moveAssignable)
        = default;

        /**
         * @brief move assignment.
         * @param other object to move.
         * @return a reference of the current object.
         */
        constexpr Variant& operator= (Variant&& other) noexcept
            requires (!_trivial && _moveAssignable)
        {
            swap (other);
            return *this;
        }

        /**
         * @brief assign a Variant holding the alternative selected by overload resolution.
         * @param t Value convertible to one of the variant's alternatives
         * @return a reference of the current object.
         */
        template <typename T, typename Match = match_t<T&&, Ts...>>
        constexpr Variant& operator= (T&& t)
            requires is_unique<Match, Ts...>::value && std::is_constructible_v<Match, T&&>
        {
            set<find_index<Match, Ts...>::value> (std::forward<T> (t));
            return *this;
        }

        /**
         * @brief check that the member type in use is the same than the one specified.
         * @return true if the type is the same, false otherwise.
         */
        template <typename T>
        constexpr bool is () const
            requires is_unique<T, Ts...>::value
        {
            return (index () == find_index<T, Ts...>::value);
        }

        /**
         * @brief check that the index in use is the same than the one specified.
         * @return true if the index is the same, false otherwise.
         */
        template <std::size_t I>
        constexpr bool is () const
            requires (I < sizeof...(Ts))
        {
            return (index () == I);
        }

        /**
         * @brief set the variable type of the object identified by type and assign it a value.
         * @param args argument(s) to set.
         */
        template <typename T, typename... Args>
        constexpr T& set (Args&&... args)
            requires is_unique<T, Ts...>::value && std::is_constructible_v<T, Args&&...>
        {
            return set<find_index<T, Ts...>::value> (std::forward<Args> (args)...);
        }

        /**
         * @brief set the variable type of the object identified by type and assign it a value.
         * @param args argument(s) to set.
         */
        template <typename T, typename Up, typename... Args>
        constexpr T& set (std::initializer_list<Up> il, Args&&... args)
            requires is_unique<T, Ts...>::value && std::is_constructible_v<T, std::initializer_list<Up>&, Args&&...>
        {
            return set<find_index<T, Ts...>::value> (il, std::forward<Args> (args)...);
        }

        /**
         * @brief set the variable type of the object identified by index and assign it a value.
         * @param args argument(s) to set.
         */
        template <std::size_t I, typename... Args>
        constexpr std::tuple_element_t<I, std::tuple<Ts...>>& set (Args&&... args)
            requires std::is_constructible_v<std::tuple_element_t<I, std::tuple<Ts...>>, Args&&...>
        {
            Variant tmp (std::in_place_index<I>, std::forward<Args> (args)...);
            *this = std::move (tmp);
            return get<I> ();
        }

        /**
         * @brief set the variable type of the object identified by index and assign it a value.
         * @param args argument(s) to set.
         */
        template <std::size_t I, class Up, class... Args>
        constexpr std::tuple_element_t<I, std::tuple<Ts...>>& set (std::initializer_list<Up> il, Args&&... args)
            requires std::is_constructible_v<std::tuple_element_t<I, std::tuple<Ts...>>, std::initializer_list<Up>&,
                                             Args&&...>
        {
            Variant tmp (std::in_place_index<I>, il, std::forward<Args> (args)...);
            *this = std::move (tmp);
            return get<I> ();
        }

        /**
         * @brief get the variable value of the object type identified by type.
         * @return the object value.
         */
        template <typename T>
        constexpr T& get ()
            requires is_unique<T, Ts...>::value
        {
            if (index () != find_index<T, Ts...>::value)
            {
                throw std::bad_cast ();
            }
            return *reinterpret_cast<T*> (storage ());
        }

        /**
         * @brief get the variable value of the object type identified by type.
         * @return the object value.
         */
        template <typename T>
        constexpr const T& get () const
            requires is_unique<T, Ts...>::value
        {
            if (index () != find_index<T, Ts...>::value)
            {
                throw std::bad_cast ();
            }
            return *reinterpret_cast<const T*> (storage ());
        }

        /**
         * @brief get the variable value of the object type identified by index.
         * @return the object value.
         */
        template <std::size_t I>
        constexpr std::tuple_element_t<I, std::tuple<Ts...>>& get ()
            requires (I < sizeof...(Ts))
        {
            if (index () != I)
            {
                throw std::bad_cast ();
            }
            return *reinterpret_cast<std::tuple_element_t<I, std::tuple<Ts...>>*> (storage ());
        }

        /**
         * @brief get the variable value of the object type identified by index.
         * @return the object value.
         */
        template <std::size_t I>
        constexpr const std::tuple_element_t<I, std::tuple<Ts...>>& get () const
            requires (I < sizeof...(Ts))
        {
            if (index () != I)
            {
                throw std::bad_cast ();
            }
            return *reinterpret_cast<const std::tuple_element_t<I, std::tuple<Ts...>>*> (storage ());
        }

        /**
         * @brief get the variable value address of the object type identified by type.
         * @return the object value address if type is correct, nullptr otherwise.
         */
        template <typename T>
        constexpr T* getIf ()
            requires is_unique<T, Ts...>::value
        {
            if (index () != find_index<T, Ts...>::value)
            {
                return nullptr;
            }
            return reinterpret_cast<T*> (storage ());
        }

        /**
         * @brief get the variable value address of the object type identified by type.
         * @return the object value address if type is correct, nullptr otherwise.
         */
        template <typename T>
        constexpr const T* getIf () const
            requires is_unique<T, Ts...>::value
        {
            if (index () != find_index<T, Ts...>::value)
            {
                return nullptr;
            }
            return reinterpret_cast<const T*> (storage ());
        }

        /**
         * @brief get the variable value address of the object type identified by index.
         * @return the object value address if index is correct, nullptr otherwise.
         */
        template <std::size_t I>
        constexpr std::tuple_element_t<I, std::tuple<Ts...>>* getIf ()
            requires (I < sizeof...(Ts))
        {
            if (index () != I)
            {
                return nullptr;
            }
            return reinterpret_cast<std::tuple_element_t<I, std::tuple<Ts...>>*> (storage ());
        }

        /**
         * @brief get the variable value address of the object type identified by index.
         * @return the object value address if index is correct, nullptr otherwise.
         */
        template <std::size_t I>
        constexpr const std::tuple_element_t<I, std::tuple<Ts...>>* getIf () const
            requires (I < sizeof...(Ts))
        {
            if (index () != I)
            {
                return nullptr;
            }
            return reinterpret_cast<const std::tuple_element_t<I, std::tuple<Ts...>>*> (storage ());
        }

        /**
         * @brief return the index of the alternative that is currently held by the variant.
         * @return the index of the alternative that is currently held by the variant.
         */
        constexpr std::size_t index () const noexcept
        {
            return _which;
        }

        /**
         * @brief swap this variant with another.
         * @param other variant to swap with.
         */
        void swap (Variant& other) noexcept
        {
            if constexpr (_trivial)
            {
                std::swap (_data, other._data);
                std::swap (_which, other._which);
            }
            else
            {
                alignas (Ts...) unsigned char tmp[std::max ({sizeof (Ts)...})];
                details::VariantHelper<Ts...>::move (_which, storage (), &tmp);
                details::VariantHelper<Ts...>::destroy (_which, storage ());
                details::VariantHelper<Ts...>::move (other._which, other.storage (), storage ());
                details::VariantHelper<Ts...>::destroy (other._which, other.storage ());
                details::VariantHelper<Ts...>::move (_which, &tmp, other.storage ());
                details::VariantHelper<Ts...>::destroy (_which, &tmp);
                std::swap (_which, other._which);
            }
        }

    protected:
        /**
         * @brief check if equal.
         * @param rhs value to compare to.
         * @return true if equal, false otherwise.
         */
        constexpr bool equal (const Variant& rhs) const
        {
            if (index () != rhs.index ())
            {
                return false;
            }
            return details::VariantHelper<Ts...>::equal (index (), storage (), rhs.storage ());
        }

        /**
         * @brief check if lower than.
         * @param rhs value to compare to.
         * @return true if lower than, false otherwise.
         */
        constexpr bool lower (const Variant& rhs) const
        {
            if (index () < rhs.index ())
            {
                return true;
            }
            if (index () > rhs.index ())
            {
                return false;
            }
            return details::VariantHelper<Ts...>::lower (index (), storage (), rhs.storage ());
        }

        // friendship with equal operator.
        template <typename... _Ts>
        friend constexpr bool operator== (const Variant<_Ts...>& lhs, const Variant<_Ts...>& rhs);

        // friendship with lower operator.
        template <typename... _Ts>
        friend constexpr bool operator< (const Variant<_Ts...>& lhs, const Variant<_Ts...>& rhs);

    private:
        /**
         * @brief get storage address.
         * @return storage address.
         */
        constexpr void* storage ()
        {
            return static_cast<void*> (std::addressof (_data));
        }

        /**
         * @brief get storage address.
         * @return storage address.
         */
        constexpr const void* storage () const
        {
            return static_cast<const void*> (std::addressof (_data));
        }

        /// aligned storage.
        alignas (Ts...) unsigned char _data[std::max ({sizeof (Ts)...})];

        /// index of the alternative that is currently held by the variant.
        std::size_t _which = 0;
    };

    /**
     * @brief compare if equal.
     * @param lhs value to compare.
     * @param rhs value to compare to.
     * @return true if equal.
     */
    template <typename... Ts>
    constexpr bool operator== (const Variant<Ts...>& lhs, const Variant<Ts...>& rhs)
    {
        return lhs.equal (rhs);
    }

    /**
     * @brief compare if lower than.
     * @param lhs value to compare.
     * @param rhs value to compare to.
     * @return true if lower than.
     */
    template <typename... Ts>
    constexpr bool operator< (const Variant<Ts...>& lhs, const Variant<Ts...>& rhs)
    {
        return lhs.lower (rhs);
    }

    /**
     * @brief compare if greater than.
     * @param lhs value to compare.
     * @param rhs value to compare to.
     * @return true if greater than.
     */
    template <typename... Ts>
    constexpr bool operator> (const Variant<Ts...>& lhs, const Variant<Ts...>& rhs)
    {
        return rhs < lhs;
    }

    /**
     * @brief compare if lower or equal than.
     * @param lhs value to compare.
     * @param rhs value to compare to.
     * @return true if lower or equal than.
     */
    template <typename... Ts>
    constexpr bool operator<= (const Variant<Ts...>& lhs, const Variant<Ts...>& rhs)
    {
        return !(rhs < lhs);
    }

    /**
     * @brief compare if greater or equal than.
     * @param lhs value to compare.
     * @param rhs value to compare to.
     * @return true if greater or equal than.
     */
    template <typename... Ts>
    constexpr bool operator>= (const Variant<Ts...>& lhs, const Variant<Ts...>& rhs)
    {
        return !(lhs < rhs);
    }

    /**
     * @brief swap two variants.
     * @param lhs first variant.
     * @param rhs second variant.
     */
    template <typename... Ts>
    void swap (Variant<Ts...>& lhs, Variant<Ts...>& rhs) noexcept
    {
        lhs.swap (rhs);
    }
}

#endif
