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

// libjoin.
#include <join/notifier.hpp>
#include <join/proactor.hpp>
#include <join/reactor.hpp>

// Libraries.
#include <gtest/gtest.h>

// C++.
#include <system_error>
#include <string>

using join::lastError;
using join::Errc;
using join::Function;
using join::Notifier;
using join::Reactor;
using join::Proactor;
using join::ReactorThread;
using join::ProactorThread;

using IntNotify = Function<void (int)>;
using TextNotify = Function<void (const std::string&, int)>;

/**
 * @brief notify a notifier from the thread of its event loop.
 * @param executor event loop to notify from.
 * @param notifier notifier to notify.
 * @param value value to hand to the callback.
 */
template <typename Executor>
static void notifyFrom (Executor& executor, Notifier<IntNotify, Executor>& notifier, int value)
{
    Function<void (), 64> fn = [&notifier, value] () {
        notifier.notify (value);
    };

    ASSERT_EQ (executor.invoke (&fn), 0) << lastError.message ();
}

/**
 * @brief Test create method.
 */
TEST (Notifier, create)
{
    int received = 0;

    Notifier<IntNotify, Reactor> reactive (ReactorThread::reactor ());
    int status = reactive.set ([&received] (int value) {
        received = value;
    });
    ASSERT_EQ (status, 0) << lastError.message ();
    notifyFrom (ReactorThread::reactor (), reactive, 1);
    ASSERT_EQ (received, 1);

    Notifier<IntNotify, Proactor> proactive (ProactorThread::proactor ());
    status = proactive.set ([&received] (int value) {
        received = value;
    });
    ASSERT_EQ (status, 0) << lastError.message ();
    notifyFrom (ProactorThread::proactor (), proactive, 2);
    ASSERT_EQ (received, 2);
}

/**
 * @brief Test set method.
 */
TEST (Notifier, set)
{
    Notifier<IntNotify, Reactor> notifier (ReactorThread::reactor ());
    int first = 0, second = 0;

    struct
    {
        int refused = 0;
        std::error_code code;
        int swapped = -1;
    } result;

    int status = notifier.set ([&notifier, &result, &second] (int) {
        result.refused = notifier.set ([] (int) {
        });
        result.code = lastError;
        notifier.unset ();
        result.swapped = notifier.set ([&second] (int value) {
            second = value;
        });
    });
    ASSERT_EQ (status, 0) << lastError.message ();

    status = notifier.set ([&first] (int value) {
        first = value;
    });
    ASSERT_EQ (status, -1);
    ASSERT_EQ (lastError, Errc::InUse) << lastError.message ();

    notifyFrom (ReactorThread::reactor (), notifier, 1);
    ASSERT_EQ (result.refused, -1);
    ASSERT_EQ (result.code, Errc::InUse) << result.code.message ();
    ASSERT_EQ (result.swapped, 0);

    notifyFrom (ReactorThread::reactor (), notifier, 2);
    ASSERT_EQ (second, 2);
    ASSERT_EQ (first, 0);
}

/**
 * @brief Test unset method.
 */
TEST (Notifier, unset)
{
    Notifier<IntNotify, Reactor> notifier (ReactorThread::reactor ());
    int count = 0;

    ASSERT_EQ (notifier.unset (), 0) << lastError.message ();

    int status = notifier.set ([&notifier, &count] (int) {
        notifier.unset ();
        ++count;
    });
    ASSERT_EQ (status, 0) << lastError.message ();

    notifyFrom (ReactorThread::reactor (), notifier, 1);
    notifyFrom (ReactorThread::reactor (), notifier, 2);
    ASSERT_EQ (count, 1);

    status = notifier.set ([&count] (int) {
        ++count;
    });
    ASSERT_EQ (status, 0) << lastError.message ();

    notifyFrom (ReactorThread::reactor (), notifier, 3);
    ASSERT_EQ (count, 2);

    ASSERT_EQ (notifier.unset (), 0) << lastError.message ();

    notifyFrom (ReactorThread::reactor (), notifier, 4);
    ASSERT_EQ (count, 2);
}

/**
 * @brief Test notify method.
 */
TEST (Notifier, notify)
{
    Notifier<TextNotify, Reactor> notifier (ReactorThread::reactor ());
    std::string text;
    int number = 0;

    Function<void (), 64> fn = [&notifier] () {
        notifier.notify ("unheard", 1);
    };
    ASSERT_EQ (ReactorThread::reactor ().invoke (&fn), 0) << lastError.message ();
    ASSERT_TRUE (text.empty ());

    int status = notifier.set ([&text, &number] (const std::string& t, int n) {
        text = t;
        number = n;
    });
    ASSERT_EQ (status, 0) << lastError.message ();

    fn = [&notifier] () {
        notifier.notify ("heard", 2);
    };
    ASSERT_EQ (ReactorThread::reactor ().invoke (&fn), 0) << lastError.message ();
    ASSERT_EQ (text, "heard");
    ASSERT_EQ (number, 2);
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
