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

#ifndef __JOIN_VARIANT_HPP__
#define __JOIN_VARIANT_HPP__

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"

// libjoin.
#include <join/traits.hpp>

// C++.
#include <algorithm>
#include <typeinfo>

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
            static constexpr bool value = std::is_default_constructible <find_element_t <0, Ts...>>::value;
        };

        /**
         * @brief helper class for variant creation/deletion.
         */
        template <typename... Ts>
        struct VariantHelper
        {
        };

        /**
         * @brief helper class for recursive operations.
         */
        template <typename Last>
        struct VariantHelper <Last>
        {
            /**
             * @brief external routine to destroy the object.
             * @param index object data type index.
             * @param data storage pointer.
             */
            inline static void destroy (std::size_t /*index*/, void* data)
            {
                reinterpret_cast <Last *> (data)->~Last ();
            }

            /**
             * @brief external routine to copy one object to an other.
             * @param oldIndex old object data type index.
             * @param oldData old object storage pointer.
             * @param newData new object storage pointer.
             */
            inline static void copy (std::size_t /*oldIndex*/, const void* oldData, void* newData)
            {
                new (newData) Last (*reinterpret_cast <const Last *> (oldData));
            }

            /**
             * @brief external routine to move one object to an other.
             * @param oldIndex old object data type index.
             * @param oldData old object storage pointer.
             * @param newData new object storage pointer.
             */
            inline static void move (std::size_t /*oldIndex*/, void* oldData, void* newData)
            {
                new (newData) Last (std::move (*reinterpret_cast <Last *> (oldData)));
            }

            /**
             * @brief external routine to compare if one object is equal to an other of same type.
             * @param index object data type index.
             * @param data storage pointer.
             * @param otherData other storage pointer.
             * @return true if equal, false otherwise.
             */
            inline static bool equal (std::size_t /*index*/, const void* data, const void* otherData)
            {
                return *reinterpret_cast <const Last *> (data) == *reinterpret_cast <const Last *> (otherData);
            }

            /**
             * @brief external routine to compare if one object is lower than an other of the same type.
             * @param index object data type index.
             * @param data storage pointer.
             * @param otherData other storage pointer.
             * @return true if lower than, false otherwise.
             */
            inline static bool lower (std::size_t /*index*/, const void* data, const void* otherData)
            {
                return *reinterpret_cast <const Last *> (data) < *reinterpret_cast <const Last *> (otherData);
            }
        };

        /**
         * @brief helper class for recursive operations.
         */
        template <typename First, typename... Ts>
        struct VariantHelper <First, Ts...>
        {
            /**
             * @brief external routine to destroy the object.
             * @param index object data type index.
             * @param data storage pointer.
             */
            inline static void destroy (std::size_t index, void* data)
            {
                if (index == sizeof... (Ts))
                {
                    reinterpret_cast <First *> (data)->~First ();
                }
                else
                {
                    VariantHelper <Ts...>::destroy (index, data);
                }
            }

            /**
             * @brief external routine to copy one object to an other.
             * @param oldIndex old object data type index.
             * @param oldData old object storage pointer.
             * @param newData new object storage pointer.
             */
            inline static void copy (std::size_t oldIndex, const void* oldData, void* newData)
            {
                if (oldIndex == sizeof... (Ts))
                {
                    new (newData) First (*reinterpret_cast <const First *> (oldData));
                }
                else
                {
                    VariantHelper <Ts...>::copy (oldIndex, oldData, newData);
                }
            }

            /**
             * @brief external routine to move one object to an other.
             * @param oldIndex old object data type index.
             * @param oldData old object storage pointer.
             * @param newData new object storage pointer.
             */
            inline static void move (std::size_t oldIndex, void* oldData, void* newData)
            {
                if (oldIndex == sizeof... (Ts))
                {
                    new (newData) First (std::move (*reinterpret_cast <First *> (oldData)));
                }
                else
                {
                    VariantHelper <Ts...>::move (oldIndex, oldData, newData);
                }
            }

            /**
             * @brief external routine to compare if one object is equal to an other of same type.
             * @param index object data type index.
             * @param data storage pointer.
             * @param otherData other storage pointer.
             * @return true if equal, false otherwise.
             */
            inline static bool equal (std::size_t index, const void* data, const void* otherData)
            {
                if (index == sizeof... (Ts))
                {
                    return *reinterpret_cast <const First *> (data) == *reinterpret_cast <const First *> (otherData);
                }
                else
                {
                    return VariantHelper <Ts...>::equal (index, data, otherData);
                }
            }

            /**
             * @brief external routine to compare if one object is lower than an other of the same type.
             * @param index object data type index.
             * @param data storage pointer.
             * @param otherData other storage pointer.
             * @return true if lower than, false otherwise.
             */
            inline static bool lower (std::size_t index, const void* data, const void* otherData)
            {
                if (index == sizeof... (Ts))
                {
                    return *reinterpret_cast <const First *> (data) < *reinterpret_cast <const First *> (otherData);
                }
                else
                {
                    return VariantHelper <Ts...>::lower (index, data, otherData);
                }
            }
        };

        /**
         * @brief helper class representing a variant storage in order to be able to disable default/copy/move constructors/operators.
         */
        template <typename... Ts>
        struct VariantStorage
        {
            /**
             * @brief default constructor.
             */
            constexpr VariantStorage () noexcept
            : VariantStorage (in_place_index_t <0> {})
            {
            }

            /**
             * @brief copy constructor.
             * @param other object to copy.
             */
            constexpr VariantStorage (const VariantStorage& other)
            : _which (other._which)
            {
                VariantHelper <Ts...>::copy (other._which, other.storage (), storage ());
            }

            /**
             * @brief move constructor.
             * @param other object to move.
             */
            constexpr VariantStorage (VariantStorage&& other) noexcept
            : _which (other._which)
            {
                VariantHelper <Ts...>::move (other._which, other.storage (), storage ());
            }

            /**
             * @brief constructs a variant storage with the alternative T specified by the index I.
             * @param args arguments to initialize the contained value with.
             */
            template <std::size_t I, typename... Args>
            constexpr explicit VariantStorage (in_place_index_t <I>, Args&&... args)
            : _which (sizeof... (Ts) - I - 1)
            {
                new (storage ()) find_element_t <I, Ts...> (std::forward <Args> (args)...);
            }

            /**
             * @brief destroy the VariantStorage instance.
             */
            virtual ~VariantStorage ()
            {
                VariantHelper <Ts...>::destroy (_which, storage ());
            }

            /**
             * @brief copy assignment.
             * @param other object to copy.
             * @return a reference of the current object.
             */
            constexpr VariantStorage& operator= (const VariantStorage& other)
            {
                VariantHelper <Ts...>::destroy (_which, storage ());
                VariantHelper <Ts...>::copy (other._which, other.storage (), storage ());
                _which = other._which;
                return *this;
            }

            /**
             * @brief move assignment.
             * @param other object to move.
             * @return a reference of the current object.
             */
            constexpr VariantStorage& operator= (VariantStorage&& other) noexcept
            {
                VariantHelper <Ts...>::destroy (_which, storage ());
                VariantHelper <Ts...>::move (other._which, other.storage (), storage ());
                _which = other._which;
                return *this;
            }

            /**
             * @brief get storage address removing const qualifier.
             * @return storage address.
             */
            constexpr void* storage () const
            {
                return const_cast <void *> (static_cast <const void *> (std::addressof (_data)));
            }

            /// aligned storage.
            typename std::aligned_union <std::max ({sizeof (Ts)...}), Ts...>::type _data;

            /// index of the alternative that is currently held by the variant.
            std::size_t _which = sizeof... (Ts) - 1;
        };
    }

    /**
     * @brief variant class.
     */
    template <typename... Ts>
    class Variant
    : private details::VariantStorage <Ts...>,
      private EnableDefault <
          details::is_first_default_constructible <Ts...>::value,
          Variant <Ts...>>,
      private std::_Enable_copy_move <
          are_copy_constructible <Ts...>::value,
          are_copy_assignable <Ts...>::value,
          are_move_constructible <Ts...>::value,
          are_move_assignable <Ts...>::value,
          Variant <Ts...>>
    {
    public:
        static_assert (0 < sizeof... (Ts),
            "Variant must have at least one alternative");
        static_assert (all <!std::is_void <Ts>::value...>::value,
            "Variant must have no void alternative");
        static_assert (all <!std::is_array <Ts>::value...>::value,
            "Variant must have no array alternative.");
        static_assert (all <!std::is_reference <Ts>::value...>::value,
            "Variant must have no reference alternative");

        using Base = details::VariantStorage <Ts...>;
        using DefaultEnabler = EnableDefault <
            details::is_first_default_constructible <Ts...>::value,
            Variant <Ts...>>;

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
        template <typename T, typename Match = match_t <T&&, Ts...>,
            typename = std::enable_if_t <is_unique <Match, Ts...>::value
                && std::is_constructible <Match, T&&>::value>>
        constexpr Variant (T&& t) noexcept
        : Variant (in_place_index_t <find_index <Match, Ts...>::value> {}, std::forward <T> (t))
        {
        }

        /**
         * @brief constructs a Variant with the specified alternative T.
         * @param args arguments to initialize the contained value with.
         */
        template <typename T, typename... Args,
            typename = std::enable_if_t <is_unique <T, Ts...>::value
                && std::is_constructible <T, Args&&...>::value>>
        constexpr explicit Variant (in_place_type_t <T>, Args&&... args)
        : Variant (in_place_index_t <find_index <T, Ts...>::value> {}, std::forward <Args> (args)...)
        {
        }

        /**
         * @brief constructs a Variant with the specified alternative T.
         * @param args arguments to initialize the contained value with.
         */
        template <typename T, typename Up, typename... Args,
            typename = std::enable_if_t <is_unique <T, Ts...>::value
                && std::is_constructible <T, std::initializer_list <Up>&, Args&&...>::value>>
        constexpr explicit Variant (in_place_type_t <T>, std::initializer_list <Up> il, Args&&... args)
        : Variant (in_place_index_t <find_index <T, Ts...>::value> {}, il, std::forward <Args> (args)...)
        {
        }

        /**
         * @brief constructs a variant with the alternative T specified by the index I.
         * @param args arguments to initialize the contained value with.
         */
        template <std::size_t I, typename... Args,
            typename = std::enable_if_t <std::is_constructible <
                find_element_t <I, Ts...>, Args&&...>::value>>
        constexpr explicit Variant (in_place_index_t <I>, Args&&... args)
        : Base (in_place_index_t <I> {}, std::forward <Args> (args)...),
          DefaultEnabler (EnableDefaultTag {})
        {
        }

        /**
         * @brief constructs a variant with the alternative T specified by the index I.
         * @param args arguments to initialize the contained value with.
         */
        template <std::size_t I, typename Up, typename... Args,
            typename = std::enable_if_t <std::is_constructible <
                find_element_t <I, Ts...>, std::initializer_list <Up>&, Args&&...>::value>>
        constexpr explicit Variant (in_place_index_t <I>, std::initializer_list <Up> il, Args&&... args)
        : Base (in_place_index_t <I> {}, il, std::forward <Args> (args)...),
          DefaultEnabler (EnableDefaultTag {})
        {
        }

        /**
         * @brief destroy the Variant instance.
         */
        virtual ~Variant () = default;

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
        template <typename T, typename Match = match_t <T&&, Ts...>>
        constexpr std::enable_if_t <is_unique <Match, Ts...>::value
            && std::is_constructible <Match, T&&>::value, Variant&>
        operator= (T&& t) noexcept
        {
            set <find_index <Match, Ts...>::value> (std::forward <T> (t));
            return *this;
        }

        /**
         * @brief check that the member type in use is the same than the one specified.
         * @return true if the type is the same, false otherwise.
         */
        template <typename T>
        constexpr std::enable_if_t <is_unique <T, Ts...>::value, bool>
        is () const
        {
            return (index () == find_index <T, Ts...>::value);
        }

        /**
         * @brief check that the index in use is the same than the one specified.
         * @return true if the index is the same, false otherwise.
         */
        template <std::size_t I>
        constexpr std::enable_if_t <is_index <I, Ts...>::value, bool>
        is () const
        {
            return (index () == I);
        }

        /**
         * @brief set the variable type of the object identified by type and assign it a value.
         * @param args argument(s) to set.
         */
        template <typename T, typename... Args>
        constexpr std::enable_if_t <is_unique <T, Ts...>::value
            && std::is_constructible <T, Args&&...>::value, T&>
        set (Args&& ...args)
        {
            return set <find_index <T, Ts...>::value> (std::forward <Args> (args)...);
        }

        /**
         * @brief set the variable type of the object identified by type and assign it a value.
         * @param args argument(s) to set.
         */
        template <typename T, typename Up, typename... Args>
        constexpr std::enable_if_t <is_unique <T, Ts...>::value
            && std::is_constructible <T, std::initializer_list <Up>&, Args&&...>::value, T&>
        set (std::initializer_list <Up> il, Args&&... args)
        {
            return set <find_index <T, Ts...>::value> (il, std::forward <Args> (args)...);
        }

        /**
         * @brief set the variable type of the object identified by index and assign it a value.
         * @param args argument(s) to set.
         */
        template <std::size_t I, typename... Args>
        constexpr std::enable_if_t <std::is_constructible <
            find_element_t <I, Ts...>, Args&&...>::value, find_element_t <I, Ts...>&>
        set (Args&& ...args)
        {
            this->~Variant ();
            new (this) Variant (in_place_index_t <I> {}, std::forward <Args> (args)...);
            return get <I> ();
        }

        /**
         * @brief set the variable type of the object identified by index and assign it a value.
         * @param args argument(s) to set.
         */
        template <std::size_t I, class Up, class... Args>
        constexpr std::enable_if_t <
            std::is_constructible <find_element_t <I, Ts...>,
                std::initializer_list <Up>&, Args&&...>::value, find_element_t <I, Ts...>&>
        set (std::initializer_list <Up> il, Args&&... args)
        {
            this->~Variant ();
            new (this) Variant (in_place_index_t <I> {}, il, std::forward <Args> (args)...);
            return get <I> ();
        }

        /**
         * @brief get the variable value of the object type identified by type.
         * @return the object value.
         */
        template <typename T>
        constexpr std::enable_if_t <is_unique <T, Ts...>::value, T&>
        get () const
        {
            if (index () != find_index <T, Ts...>::value)
            {
                throw std::bad_cast ();
            }
            return *reinterpret_cast <T *> (this->storage ());
        }

        /**
         * @brief get the variable value of the object type identified by index.
         * @return the object value.
         */
        template <std::size_t I>
        constexpr std::enable_if_t <is_index <I, Ts...>::value, find_element_t <I, Ts...>&>
        get () const
        {
            if (index () != I)
            {
                throw std::bad_cast ();
            }
            return *reinterpret_cast <find_element_t <I, Ts...> *> (this->storage ());
        }

        /**
         * @brief get the variable value address of the object type identified by type.
         * @return the object value address if type is correct, nullptr otherwise.
         */
        template <typename T>
        constexpr std::enable_if_t <is_unique <T, Ts...>::value, std::add_pointer_t <T>>
        getIf () const
        {
            if (index () != find_index <T, Ts...>::value)
            {
                return nullptr;
            }
            return reinterpret_cast <T *> (this->storage ());
        }

        /**
         * @brief get the variable value address of the object type identified by index.
         * @return the object value address if index is correct, nullptr otherwise.
         */
        template <std::size_t I>
        constexpr std::enable_if_t <
            is_index <I, Ts...>::value, std::add_pointer_t <find_element_t <I, Ts...>>>
        getIf () const
        {
            if (index () != I)
            {
                return nullptr;
            }
            return reinterpret_cast <find_element_t <I, Ts...> *> (this->storage ());
        }

        /**
         * @brief return the index of the alternative that is currently held by the variant.
         * @return the index of the alternative that is currently held by the variant.
         */
        constexpr std::size_t index () const noexcept
        {
            return sizeof... (Ts) - this->_which - 1;
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
            return details::VariantHelper <Ts...>::equal (this->_which, this->storage (), rhs.storage ());
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
            return details::VariantHelper <Ts...>::lower (this->_which, this->storage (), rhs.storage ());
        }

        // friendship with equal operator.
        template <typename... _Ts>
        friend constexpr bool operator== (const Variant <_Ts...>& lhs, const Variant <_Ts...>& rhs);

        // friendship with lower operator.
        template <typename... _Ts>
        friend constexpr bool operator< (const Variant <_Ts...>& lhs, const Variant <_Ts...>& rhs);
    };

    /**
     * @brief compare if equal.
     * @param lhs value to compare.
     * @param rhs value to compare to.
     * @return true if equal.
     */
    template <typename... Ts>
    constexpr bool operator== (const Variant <Ts...>& lhs, const Variant <Ts...>& rhs)
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
    constexpr bool operator!= (const Variant <Ts...>& lhs, const Variant <Ts...>& rhs)
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
    constexpr bool operator< (const Variant <Ts...>& lhs, const Variant <Ts...>& rhs)
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
    constexpr bool operator> (const Variant <Ts...>& lhs, const Variant <Ts...>& rhs)
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
    constexpr bool operator<= (const Variant <Ts...>& lhs, const Variant <Ts...>& rhs)
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
    constexpr bool operator>= (const Variant <Ts...>& lhs, const Variant <Ts...>& rhs)
    {
        return !(lhs < rhs);
    }
}

#pragma GCC diagnostic pop

#endif
