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
#include <join/function.hpp>

// Libraries.
#include <gtest/gtest.h>

// C++.
#include <memory>
#include <string>

using join::Function;

static int add (int a, int b)
{
    return a + b;
}

struct Multiplier
{
    int factor;

    int operator() (int val) const
    {
        return val * factor;
    }
};

struct DestructGuard
{
    bool* flag;

    explicit DestructGuard (bool* f) noexcept
    : flag (f)
    {
    }

    DestructGuard (const DestructGuard&) = delete;

    DestructGuard (DestructGuard&& other) noexcept
    : flag (other.flag)
    {
        other.flag = nullptr;
    }

    ~DestructGuard ()
    {
        if (flag)
        {
            *flag = true;
        }
    }

    void operator() () const noexcept
    {
    }
};

/**
 * @brief Test construction.
 */
TEST (Function, construct)
{
    Function<void ()> f1;
    EXPECT_FALSE (f1);
    EXPECT_THROW (f1 (), std::bad_function_call);

    Function<void ()> f2 (nullptr);
    EXPECT_FALSE (f2);
    EXPECT_THROW (f2 (), std::bad_function_call);

    Function<int (int, int)> f3 (add);
    ASSERT_TRUE (f3);
    EXPECT_EQ (f3 (10, 32), 42);

    Function<std::string (const std::string&)> f4 ([] (const std::string& s) {
        return s + "World";
    });
    ASSERT_TRUE (f4);
    EXPECT_EQ (f4 ("Hello "), "Hello World");

    Function<int (int)> f5 (Multiplier{3});
    ASSERT_TRUE (f5);
    EXPECT_EQ (f5 (4), 12);
}

/**
 * @brief Test assignment.
 */
TEST (Function, assign)
{
    int counter = 0;

    Function<void ()> f1 ([&counter] () {
        counter = 1;
    });
    f1 ();
    EXPECT_EQ (counter, 1);

    f1 = [&counter] () {
        counter = 42;
    };
    f1 ();
    EXPECT_EQ (counter, 42);

    Function<int ()> f2 ([] {
        return 42;
    });
    ASSERT_TRUE (f2);

    f2 = nullptr;
    EXPECT_FALSE (f2);
    EXPECT_THROW (f2 (), std::bad_function_call);

    Function<int ()> f3 ([] {
        return 1;
    });
    Function<int ()> f4 ([] {
        return 2;
    });

    f4 = std::move (f3);

    EXPECT_FALSE (f3);
    ASSERT_TRUE (f4);
    EXPECT_EQ (f4 (), 1);
}

/**
 * @brief Test move.
 */
TEST (Function, move)
{
    auto ptr = std::make_unique<int> (100);

    Function<int ()> f1 ([p = std::move (ptr)] () {
        return *p + 50;
    });
    ASSERT_TRUE (f1);

    Function<int ()> f2 (std::move (f1));
    EXPECT_FALSE (f1);
    ASSERT_TRUE (f2);
    EXPECT_EQ (f2 (), 150);
}

/**
 * @brief Test non-trivial move.
 */
TEST (Function, nonTrivialMove)
{
    bool sourceDestroyed = false;

    struct NonTrivial
    {
        bool* destroyed;
        std::unique_ptr<int> ptr;

        NonTrivial (bool* d, int v)
        : destroyed (d)
        , ptr (new int (v))
        {
        }

        NonTrivial (NonTrivial&& other) noexcept
        : destroyed (other.destroyed)
        , ptr (std::move (other.ptr))
        {
            other.destroyed = nullptr;
        }

        ~NonTrivial ()
        {
            if (destroyed)
            {
                *destroyed = true;
            }
        }

        int operator() () const
        {
            return *ptr;
        }
    };

    {
        Function<int ()> f2;

        {
            Function<int ()> f1 (NonTrivial (&sourceDestroyed, 42));
            sourceDestroyed = false;

            f2 = std::move (f1);

            EXPECT_FALSE (f1);
            EXPECT_FALSE (sourceDestroyed);
        }
        EXPECT_FALSE (sourceDestroyed);
        EXPECT_EQ (f2 (), 42);
    }
    EXPECT_TRUE (sourceDestroyed);
}

/**
 * @brief Test forwarding.
 */
TEST (Function, forward)
{
    struct Tracker
    {
        bool lvalue = false;
        bool rvalue = false;

        void operator() (int&)
        {
            lvalue = true;
        }

        void operator() (int&&)
        {
            rvalue = true;
        }
    };

    int x = 0;

    {
        Tracker t;
        Function<void (int&)> f (std::ref (t));
        f (x);
        EXPECT_TRUE (t.lvalue);
        EXPECT_FALSE (t.rvalue);
    }

    {
        Tracker t;
        Function<void (int&&)> f (std::ref (t));
        f (std::move (x));
        EXPECT_TRUE (t.rvalue);
        EXPECT_FALSE (t.lvalue);
    }
}

/**
 * @brief Test destroy.
 */
TEST (Function, destroy)
{
    bool destroyed = false;
    {
        DestructGuard g (&destroyed);
        Function<void ()> f1 (std::move (g));
        destroyed = false;
        EXPECT_FALSE (destroyed);
    }
    EXPECT_TRUE (destroyed);

    destroyed = false;
    DestructGuard v (&destroyed);
    Function<void ()> f2 (std::move (v));
    destroyed = false;
    f2.reset ();
    EXPECT_TRUE (destroyed);
    EXPECT_FALSE (f2);
    EXPECT_THROW (f2 (), std::bad_function_call);

    destroyed = false;
    DestructGuard t (&destroyed);
    Function<void ()> f3 (std::move (t));
    destroyed = false;
    f3 = [] () {
    };
    EXPECT_TRUE (destroyed);
    ASSERT_TRUE (f3);
}

/**
 * @brief Test lambda capture.
 */
TEST (Function, capture)
{
    int x = 10;
    int y = 20;

    Function<int ()> f ([x, &y] () {
        return x + y;
    });

    EXPECT_EQ (f (), 30);
    y = 40;
    EXPECT_EQ (f (), 50);
}

/**
 * @brief Test mutable lambda.
 */
TEST (Function, mutableLambda)
{
    Function<int ()> f ([value = 0] () mutable {
        return ++value;
    });

    EXPECT_EQ (f (), 1);
    EXPECT_EQ (f (), 2);
    EXPECT_EQ (f (), 3);
}

/**
 * @brief Test swap method.
 */
TEST (Function, swap)
{
    Function<int ()> f1 ([] {
        return 1;
    });

    Function<int ()> f2 ([] {
        return 2;
    });

    f1.swap (f2);

    EXPECT_EQ (f1 (), 2);
    EXPECT_EQ (f2 (), 1);

    Function<int ()> f3;

    f2.swap (f3);

    EXPECT_FALSE (f2);
    ASSERT_TRUE (f3);
    EXPECT_EQ (f3 (), 1);
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
