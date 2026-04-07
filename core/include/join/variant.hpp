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

// C.
#include <cstddef>

namespace join
{
    namespace details
    {
        /**
         * @brief check that first alternative is default constructible.
         */
        template <typename... Ts>
        struct is_first_default_constructible
        {
            static constexpr bool value = std::is_default_constructible<find_element_t<0, Ts...>>::value;
        };

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
             * @brief less-than comparison for non-null pointer alternatives.
             * @tparam T type of the alternative to compare.
             * @param a first storage pointer.
             * @param b second storage pointer.
             * @return true if a is lower than b, false otherwise.
             */
            template <typename T>
            static typename std::enable_if<!std::is_null_pointer<T>::value, bool>::type lowerDispatch (const void* a,
                                                                                                       const void* b)
            {
                return *reinterpret_cast<const T*> (a) < *reinterpret_cast<const T*> (b);
            }

            /**
             * @brief less-than comparison for nullptr_t alternatives, always returns false.
             * @tparam T type of the alternative to compare.
             * @return false.
             */
            template <typename T>
            static typename std::enable_if<std::is_null_pointer<T>::value, bool>::type lowerDispatch (const void*,
                                                                                                      const void*)
            {
                return false;
            }

            /**
             * @brief dispatch less-than comparison, handling nullptr_t alternatives.
             * @tparam T type of the alternative to compare.
             * @param a first storage pointer.
             * @param b second storage pointer.
             * @return true if a is lower than b, false otherwise.
             */
            template <typename T>
            static bool lowerImpl (const void* a, const void* b)
            {
                return lowerDispatch<T> (a, b);
            }
        };

        /**
         * @brief helper class representing a variant storage in order to be able to disable default/copy/move
         * constructors/operators.
         */
        template <typename... Ts>
        struct VariantStorage
        {
            /**
             * @brief default constructor.
             */
            constexpr VariantStorage ()
            : VariantStorage (in_place_index_t<0>{})
            {
            }

            /**
             * @brief copy constructor.
             * @param other object to copy.
             */
            constexpr VariantStorage (const VariantStorage& other)
            : _which (other._which)
            {
                VariantHelper<Ts...>::copy (other._which, other.storage (), storage ());
            }

            /**
             * @brief move constructor.
             * @param other object to move.
             */
            constexpr VariantStorage (VariantStorage&& other) noexcept
            : _which (other._which)
            {
                VariantHelper<Ts...>::move (other._which, other.storage (), storage ());
            }

            /**
             * @brief constructs a variant storage with the alternative T specified by the index I.
             * @param args arguments to initialize the contained value with.
             */
            template <std::size_t I, typename... Args>
            constexpr explicit VariantStorage (in_place_index_t<I>, Args&&... args)
            : _which (I)
            {
                new (storage ()) find_element_t<I, Ts...> (std::forward<Args> (args)...);
            }

            /**
             * @brief destroy the VariantStorage instance.
             */
            ~VariantStorage ()
            {
                VariantHelper<Ts...>::destroy (_which, storage ());
            }

            /**
             * @brief copy assignment.
             * @param other object to copy.
             * @return a reference of the current object.
             */
            constexpr VariantStorage& operator= (const VariantStorage& other)
            {
                VariantStorage tmp (other);
                swap (tmp);
                return *this;
            }

            /**
             * @brief move assignment.
             * @param other object to move.
             * @return a reference of the current object.
             */
            constexpr VariantStorage& operator= (VariantStorage&& other) noexcept
            {
                swap (other);
                return *this;
            }

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

            /**
             * @brief swap this storage with other.
             * @param other storage to swap with.
             */
            void swap (VariantStorage& other) noexcept
            {
                alignas (Ts...) unsigned char tmp[std::max ({sizeof (Ts)...})];
                VariantHelper<Ts...>::move (_which, storage (), &tmp);
                VariantHelper<Ts...>::destroy (_which, storage ());
                VariantHelper<Ts...>::move (other._which, other.storage (), storage ());
                VariantHelper<Ts...>::destroy (other._which, other.storage ());
                VariantHelper<Ts...>::move (_which, &tmp, other.storage ());
                VariantHelper<Ts...>::destroy (_which, &tmp);
                std::swap (_which, other._which);
            }

            /// aligned storage.
            alignas (Ts...) unsigned char _data[std::max ({sizeof (Ts)...})];

            /// index of the alternative that is currently held by the variant.
            std::size_t _which = 0;
        };
    }

    /**
     * @brief variant class.
     */
    template <typename... Ts>
    class Variant
    : private details::VariantStorage<Ts...>,
      private EnableDefault<details::is_first_default_constructible<Ts...>::value, Variant<Ts...>>,
      private std::_Enable_copy_move<are_copy_constructible<Ts...>::value, are_copy_assignable<Ts...>::value,
                                     are_move_constructible<Ts...>::value, are_move_assignable<Ts...>::value,
                                     Variant<Ts...>>
    {
    public:
        static_assert (0 < sizeof...(Ts), "Variant must have at least one alternative");
        static_assert (all<!std::is_void<Ts>::value...>::value, "Variant must have no void alternative");
        static_assert (all<!std::is_array<Ts>::value...>::value, "Variant must have no array alternative.");
        static_assert (all<!std::is_reference<Ts>::value...>::value, "Variant must have no reference alternative");

        using Base = details::VariantStorage<Ts...>;
        using DefaultEnabler = EnableDefault<details::is_first_default_constructible<Ts...>::value, Variant<Ts...>>;

        /**
         * @brief default constructor.
         */
        constexpr Variant () = default;

        /**
         * @brief copy constructor.
         * @param other object to copy.
         */
        constexpr Variant (const Variant&) = default;

        /**
         * @brief move constructor.
         * @param other object to move.
         */
        constexpr Variant (Variant&&) = default;

        /**
         * @brief constructs a Variant holding the alternative selected by overload resolution.
         * @param t Value convertible to one of the variant's alternatives
         */
        template <
            typename T, typename Match = match_t<T&&, Ts...>,
            typename = std::enable_if_t<is_unique<Match, Ts...>::value && std::is_constructible<Match, T&&>::value>>
        constexpr Variant (T&& t)
        : Variant (in_place_index_t<find_index<Match, Ts...>::value>{}, std::forward<T> (t))
        {
        }

        /**
         * @brief constructs a Variant with the specified alternative T.
         * @param args arguments to initialize the contained value with.
         */
        template <typename T, typename... Args,
                  typename = std::enable_if_t<is_unique<T, Ts...>::value && std::is_constructible<T, Args&&...>::value>>
        constexpr explicit Variant (in_place_type_t<T>, Args&&... args)
        : Variant (in_place_index_t<find_index<T, Ts...>::value>{}, std::forward<Args> (args)...)
        {
        }

        /**
         * @brief constructs a Variant with the specified alternative T.
         * @param args arguments to initialize the contained value with.
         */
        template <typename T, typename Up, typename... Args,
                  typename = std::enable_if_t<is_unique<T, Ts...>::value &&
                                              std::is_constructible<T, std::initializer_list<Up>&, Args&&...>::value>>
        constexpr explicit Variant (in_place_type_t<T>, std::initializer_list<Up> il, Args&&... args)
        : Variant (in_place_index_t<find_index<T, Ts...>::value>{}, il, std::forward<Args> (args)...)
        {
        }

        /**
         * @brief constructs a variant with the alternative T specified by the index I.
         * @param args arguments to initialize the contained value with.
         */
        template <std::size_t I, typename... Args,
                  typename = std::enable_if_t<std::is_constructible<find_element_t<I, Ts...>, Args&&...>::value>>
        constexpr explicit Variant (in_place_index_t<I>, Args&&... args)
        : Base (in_place_index_t<I>{}, std::forward<Args> (args)...)
        , DefaultEnabler (EnableDefaultTag{})
        {
        }

        /**
         * @brief constructs a variant with the alternative T specified by the index I.
         * @param args arguments to initialize the contained value with.
         */
        template <std::size_t I, typename Up, typename... Args,
                  typename = std::enable_if_t<
                      std::is_constructible<find_element_t<I, Ts...>, std::initializer_list<Up>&, Args&&...>::value>>
        constexpr explicit Variant (in_place_index_t<I>, std::initializer_list<Up> il, Args&&... args)
        : Base (in_place_index_t<I>{}, il, std::forward<Args> (args)...)
        , DefaultEnabler (EnableDefaultTag{})
        {
        }

        /**
         * @brief destroy the Variant instance.
         */
        ~Variant () = default;

        /**
         * @brief copy assignment.
         * @param other object to copy.
         * @return a reference of the current object.
         */
        constexpr Variant& operator= (const Variant& other) = default;

        /**
         * @brief move assignment.
         * @param other object to move.
         * @return a reference of the current object.
         */
        constexpr Variant& operator= (Variant&& other) = default;

        /**
         * @brief assign a Variant holding the alternative selected by overload resolution.
         * @param t Value convertible to one of the variant's alternatives
         * @return a reference of the current object.
         */
        template <typename T, typename Match = match_t<T&&, Ts...>>
        constexpr std::enable_if_t<is_unique<Match, Ts...>::value && std::is_constructible<Match, T&&>::value, Variant&>
        operator= (T&& t)
        {
            set<find_index<Match, Ts...>::value> (std::forward<T> (t));
            return *this;
        }

        /**
         * @brief check that the member type in use is the same than the one specified.
         * @return true if the type is the same, false otherwise.
         */
        template <typename T>
        constexpr std::enable_if_t<is_unique<T, Ts...>::value, bool> is () const
        {
            return (index () == find_index<T, Ts...>::value);
        }

        /**
         * @brief check that the index in use is the same than the one specified.
         * @return true if the index is the same, false otherwise.
         */
        template <std::size_t I>
        constexpr std::enable_if_t<is_index<I, Ts...>::value, bool> is () const
        {
            return (index () == I);
        }

        /**
         * @brief set the variable type of the object identified by type and assign it a value.
         * @param args argument(s) to set.
         */
        template <typename T, typename... Args>
        constexpr std::enable_if_t<is_unique<T, Ts...>::value && std::is_constructible<T, Args&&...>::value, T&> set (
            Args&&... args)
        {
            return set<find_index<T, Ts...>::value> (std::forward<Args> (args)...);
        }

        /**
         * @brief set the variable type of the object identified by type and assign it a value.
         * @param args argument(s) to set.
         */
        template <typename T, typename Up, typename... Args>
        constexpr std::enable_if_t<
            is_unique<T, Ts...>::value && std::is_constructible<T, std::initializer_list<Up>&, Args&&...>::value, T&>
        set (std::initializer_list<Up> il, Args&&... args)
        {
            return set<find_index<T, Ts...>::value> (il, std::forward<Args> (args)...);
        }

        /**
         * @brief set the variable type of the object identified by index and assign it a value.
         * @param args argument(s) to set.
         */
        template <std::size_t I, typename... Args>
        constexpr std::enable_if_t<std::is_constructible<find_element_t<I, Ts...>, Args&&...>::value,
                                   find_element_t<I, Ts...>&>
        set (Args&&... args)
        {
            Variant tmp (in_place_index_t<I>{}, std::forward<Args> (args)...);
            *this = std::move (tmp);
            return get<I> ();
        }

        /**
         * @brief set the variable type of the object identified by index and assign it a value.
         * @param args argument(s) to set.
         */
        template <std::size_t I, class Up, class... Args>
        constexpr std::enable_if_t<
            std::is_constructible<find_element_t<I, Ts...>, std::initializer_list<Up>&, Args&&...>::value,
            find_element_t<I, Ts...>&>
        set (std::initializer_list<Up> il, Args&&... args)
        {
            Variant tmp (in_place_index_t<I>{}, il, std::forward<Args> (args)...);
            *this = std::move (tmp);
            return get<I> ();
        }

        /**
         * @brief get the variable value of the object type identified by type.
         * @return the object value.
         */
        template <typename T>
        constexpr std::enable_if_t<is_unique<T, Ts...>::value, T&> get ()
        {
            if (index () != find_index<T, Ts...>::value)
            {
                throw std::bad_cast ();
            }
            return *reinterpret_cast<T*> (this->storage ());
        }

        /**
         * @brief get the variable value of the object type identified by type.
         * @return the object value.
         */
        template <typename T>
        constexpr std::enable_if_t<is_unique<T, Ts...>::value, const T&> get () const
        {
            if (index () != find_index<T, Ts...>::value)
            {
                throw std::bad_cast ();
            }
            return *reinterpret_cast<const T*> (this->storage ());
        }

        /**
         * @brief get the variable value of the object type identified by index.
         * @return the object value.
         */
        template <std::size_t I>
        constexpr std::enable_if_t<is_index<I, Ts...>::value, find_element_t<I, Ts...>&> get ()
        {
            if (index () != I)
            {
                throw std::bad_cast ();
            }
            return *reinterpret_cast<find_element_t<I, Ts...>*> (this->storage ());
        }

        /**
         * @brief get the variable value of the object type identified by index.
         * @return the object value.
         */
        template <std::size_t I>
        constexpr std::enable_if_t<is_index<I, Ts...>::value, const find_element_t<I, Ts...>&> get () const
        {
            if (index () != I)
            {
                throw std::bad_cast ();
            }
            return *reinterpret_cast<const find_element_t<I, Ts...>*> (this->storage ());
        }

        /**
         * @brief get the variable value address of the object type identified by type.
         * @return the object value address if type is correct, nullptr otherwise.
         */
        template <typename T>
        constexpr std::enable_if_t<is_unique<T, Ts...>::value, T*> getIf ()
        {
            if (index () != find_index<T, Ts...>::value)
            {
                return nullptr;
            }
            return reinterpret_cast<T*> (this->storage ());
        }

        /**
         * @brief get the variable value address of the object type identified by type.
         * @return the object value address if type is correct, nullptr otherwise.
         */
        template <typename T>
        constexpr std::enable_if_t<is_unique<T, Ts...>::value, const T*> getIf () const
        {
            if (index () != find_index<T, Ts...>::value)
            {
                return nullptr;
            }
            return reinterpret_cast<const T*> (this->storage ());
        }

        /**
         * @brief get the variable value address of the object type identified by index.
         * @return the object value address if index is correct, nullptr otherwise.
         */
        template <std::size_t I>
        constexpr std::enable_if_t<is_index<I, Ts...>::value, find_element_t<I, Ts...>*> getIf ()
        {
            if (index () != I)
            {
                return nullptr;
            }
            return reinterpret_cast<find_element_t<I, Ts...>*> (this->storage ());
        }

        /**
         * @brief get the variable value address of the object type identified by index.
         * @return the object value address if index is correct, nullptr otherwise.
         */
        template <std::size_t I>
        constexpr std::enable_if_t<is_index<I, Ts...>::value, const find_element_t<I, Ts...>*> getIf () const
        {
            if (index () != I)
            {
                return nullptr;
            }
            return reinterpret_cast<const find_element_t<I, Ts...>*> (this->storage ());
        }

        /**
         * @brief return the index of the alternative that is currently held by the variant.
         * @return the index of the alternative that is currently held by the variant.
         */
        constexpr std::size_t index () const noexcept
        {
            return this->_which;
        }

        /**
         * @brief swap this variant with another.
         * @param other variant to swap with.
         */
        void swap (Variant& other) noexcept
        {
            Base::swap (other);
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
            return details::VariantHelper<Ts...>::equal (index (), this->storage (), rhs.storage ());
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
            return details::VariantHelper<Ts...>::lower (index (), this->storage (), rhs.storage ());
        }

        // friendship with equal operator.
        template <typename... _Ts>
        friend constexpr bool operator== (const Variant<_Ts...>& lhs, const Variant<_Ts...>& rhs);

        // friendship with lower operator.
        template <typename... _Ts>
        friend constexpr bool operator< (const Variant<_Ts...>& lhs, const Variant<_Ts...>& rhs);
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
     * @brief compare if not equal.
     * @param lhs value to compare.
     * @param rhs value to compare to.
     * @return true if not equal.
     */
    template <typename... Ts>
    constexpr bool operator!= (const Variant<Ts...>& lhs, const Variant<Ts...>& rhs)
    {
        return !(lhs == rhs);
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
