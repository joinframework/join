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

#ifndef JOIN_CORE_FUNCTION_HPP
#define JOIN_CORE_FUNCTION_HPP

// C++.
#include <type_traits>
#include <functional>
#include <utility>
#include <new>

// C.
#include <cstddef>

namespace join
{
    /**
     * @brief fixed-capacity move-only allocation-free alternative to std::function.
     */
    template <typename Signature, std::size_t Capacity = 32, std::size_t Alignment = alignof (std::max_align_t)>
    class Function;

    /**
     * @brief fixed-capacity move-only allocation-free alternative to std::function.
     */
    template <typename Return, typename... Args, std::size_t Capacity, std::size_t Alignment>
    class Function<Return (Args...), Capacity, Alignment>
    {
    public:
        /**
         * @brief default constructor.
         */
        Function () noexcept = default;

        /**
         * @brief copy constructor.
         * @param other other object to copy.
         */
        Function (const Function& other) = delete;

        /**
         * @brief copy assignment operator.
         * @param other other object to copy.
         * @return this.
         */
        Function& operator= (const Function& other) = delete;

        /**
         * @brief move constructor.
         * @param other object to move from.
         */
        Function (Function&& other) noexcept
        {
            moveFrom (std::move (other));
        }

        /**
         * @brief move assignment operator.
         * @param other object to move from.
         * @return *this.
         */
        Function& operator= (Function&& other) noexcept
        {
            clear ();
            moveFrom (std::move (other));
            return *this;
        }

        /**
         * @brief construct from nullptr.
         */
        Function (std::nullptr_t) noexcept
        : Function ()
        {
        }

        /**
         * @brief assign nullptr.
         */
        Function& operator= (std::nullptr_t) noexcept
        {
            clear ();
            return *this;
        }

        /**
         * @brief construct with a callable object.
         * @param callable callable object.
         */
        template <
            typename Func, typename DecayedFunc = std::decay_t<Func>,
            typename = std::enable_if_t<!std::is_same<DecayedFunc, Function>::value &&
                                        (std::is_void<Return>::value ||
                                         std::is_convertible<std::result_of_t<DecayedFunc&(Args...)>, Return>::value)>>
        Function (Func&& callable)
        {
            static_assert (sizeof (DecayedFunc) <= Capacity, "Callable size exceeds Function capacity.");
            static_assert (alignof (DecayedFunc) <= Alignment, "Callable alignment exceeds Function alignment.");
            static_assert (std::is_nothrow_move_constructible<DecayedFunc>::value,
                           "Callable must be nothrow move constructible.");

            new (&_storage) DecayedFunc (std::forward<Func> (callable));

            _invoker = [] (void* storage, Args&&... args) -> Return {
                return static_cast<Return> ((*static_cast<DecayedFunc*> (storage)) (std::forward<Args> (args)...));
            };

            _manager = [] (void* dst, void* src) noexcept {
                DecayedFunc* source = static_cast<DecayedFunc*> (src);
                new (dst) DecayedFunc (std::move (*source));
                source->~DecayedFunc ();
            };

            _destructor = [] (void* storage) noexcept {
                static_cast<DecayedFunc*> (storage)->~DecayedFunc ();
            };
        }

        /**
         * @brief destructor.
         */
        ~Function ()
        {
            clear ();
        }

        /**
         * @brief invoke the stored callable.
         * @param args arguments forwarded to the callable.
         * @return result of the invocation.
         * @throw std::bad_function_call if no callable is stored.
         */
        Return operator() (Args... args)
        {
            if (!_invoker)
            {
                throw std::bad_function_call ();
            }
            return _invoker (&_storage, std::forward<Args> (args)...);
        }

        /**
         * @brief checks whether a callable is stored.
         * @return true if a callable is present.
         */
        explicit operator bool () const noexcept
        {
            return _invoker != nullptr;
        }

        /**
         * @brief clear the stored callable.
         */
        void reset () noexcept
        {
            clear ();
        }

        /**
         * @brief swap two Function instances.
         */
        void swap (Function& other) noexcept
        {
            Function tmp (std::move (other));
            other = std::move (*this);
            *this = std::move (tmp);
        }

    private:
        /// invocation function pointer type.
        using InvokerFunc = Return (*) (void*, Args&&...);

        /// relocation function pointer type.
        using ManagerFunc = void (*) (void*, void*);

        /// destructor function pointer type.
        using DestructorFunc = void (*) (void*);

        /**
         * @brief destroy the stored callable and reset all pointers.
         */
        void clear () noexcept
        {
            if (_destructor)
            {
                _destructor (&_storage);
                _invoker = nullptr;
                _manager = nullptr;
                _destructor = nullptr;
            }
        }

        /**
         * @brief move all state from another instance (assumes *this is empty).
         */
        void moveFrom (Function&& other) noexcept
        {
            _invoker = other._invoker;
            _manager = other._manager;
            _destructor = other._destructor;

            if (_manager)
            {
                _manager (&_storage, &other._storage);
                other._invoker = nullptr;
                other._manager = nullptr;
                other._destructor = nullptr;
            }
        }

        /// fixed-capacity aligned storage for the callable target.
        alignas (Alignment) unsigned char _storage[Capacity];

        /// pointer to the static invocation wrapper.
        InvokerFunc _invoker = nullptr;

        /// pointer to the static move-relocate wrapper.
        ManagerFunc _manager = nullptr;

        /// pointer to the static destructor wrapper.
        DestructorFunc _destructor = nullptr;
    };
}

#endif
