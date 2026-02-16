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

#ifndef __JOIN_REACTORPOOL_HPP__
#define __JOIN_REACTORPOOL_HPP__

// libjoin.
#include <join/reactor.hpp>

// C++
#include <memory>
#include <vector>

namespace join
{
    /**
     * @brief ReactorPool class.
     */
    class ReactorPool
    {
    public:
        /**
         * @brief default constructor.
         */
        explicit ReactorPool ();

        /**
         * @brief destroy instance.
         */
        ~ReactorPool () noexcept = default;

        /**
         * @brief add handler to reactor.
         * @param handler handler pointer.
         * @param sync wait for operation completion if true.
         * @return 0 on success, -1 on failure.
         */
        int addHandler (EventHandler* handler, bool sync = true) noexcept;

        /**
         * @brief delete handler from reactor.
         * @param handler handler pointer.
         * @param sync wait for operation completion if true.
         * @return 0 on success, -1 on failure.
         */
        int delHandler (EventHandler* handler, bool sync = true) noexcept;

        /**
         * @brief create the ReactorPool instance.
         * @return ReactorPool instance pointer.
         */
        static ReactorPool* instance () noexcept;

    private:
        /// pool of reactors.
        std::vector <std::unique_ptr <Reactor>> _reactors;

        /// round-robin counter for load distribution.
        std::atomic <uint64_t> _next {0};
    };
}

#endif
