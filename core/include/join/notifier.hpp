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

#ifndef JOIN_CORE_NOTIFIER_HPP
#define JOIN_CORE_NOTIFIER_HPP

// libjoin.
#include <join/function.hpp>
#include <join/error.hpp>

// C++.
#include <utility>

// C.
#include <cstddef>

namespace join
{
    /**
     * @brief single callback slot, set and unset from any thread, notified from the event loop thread.
     */
    template <typename Callback, typename Executor>
    class Notifier
    {
        static_assert (sizeof (Callback*) == 0, "Notifier callback must be a join::Function returning void.");
    };

    /**
     * @brief single callback slot, set and unset from any thread, notified from the event loop thread.
     */
    template <typename... Args, std::size_t Capacity, std::size_t Alignment, typename Executor>
    class Notifier<Function<void (Args...), Capacity, Alignment>, Executor>
    {
    public:
        /// callback type.
        using Callback = Function<void (Args...), Capacity, Alignment>;

        /**
         * @brief create the Notifier instance.
         */
        Notifier () = delete;

        /**
         * @brief create the instance.
         * @param executor event loop the callback is called from.
         */
        explicit Notifier (Executor& executor) noexcept
        : _executor (executor)
        {
        }

        /**
         * @brief create instance by copy.
         * @param other other object to copy.
         */
        Notifier (const Notifier& other) = delete;

        /**
         * @brief assign instance by copy.
         * @param other other object to copy.
         * @return a reference of the current object.
         */
        Notifier& operator= (const Notifier& other) = delete;

        /**
         * @brief create instance by move.
         * @param other other object to move.
         */
        Notifier (Notifier&& other) = delete;

        /**
         * @brief assign instance by move.
         * @param other other object to move.
         * @return a reference of the current object.
         */
        Notifier& operator= (Notifier&& other) = delete;

        /**
         * @brief destroy the instance, the owner must have stopped notifying.
         */
        ~Notifier () = default;

        /**
         * @brief set the callback, refused if one is already set.
         * @param cb callback, called from the event loop thread, must not throw nor destroy its owner.
         * @return 0 on success, -1 on failure.
         */
        int set (Callback cb) noexcept
        {
            bool busy = false;

            typename Executor::InvokeHandler fn = [this, &cb, &busy] () {
                busy = _calling || _callback;
                if (!busy)
                {
                    _callback = std::move (cb);
                }
            };

            if (_executor.invoke (&fn) == -1)
            {
                return -1;  // LCOV_EXCL_LINE
            }

            if (busy)
            {
                lastError = make_error_code (Errc::InUse);
                return -1;
            }

            return 0;
        }

        /**
         * @brief unset the callback, it is no longer called once this returns.
         * @return 0 on success, -1 on failure.
         */
        int unset () noexcept
        {
            typename Executor::InvokeHandler fn = [this] () {
                _callback = nullptr;
                _calling = false;
            };

            return _executor.invoke (&fn);
        }

        /**
         * @brief call the callback if one is set, to be called from the event loop thread only.
         * @param args callback arguments.
         */
        void notify (Args... args) noexcept
        {
            if (!_callback)
            {
                return;
            }

            Callback callback = std::move (_callback);

            _calling = true;
            callback (std::forward<Args> (args)...);

            if (_calling)
            {
                _callback = std::move (callback);
                _calling = false;
            }
        }

    private:
        /// callback, only accessed from the event loop thread.
        Callback _callback;

        /// set while the callback is being called and still set, only accessed from the event loop thread.
        bool _calling = false;

        /// event loop the callback is called from.
        Executor& _executor;
    };
}

#endif
