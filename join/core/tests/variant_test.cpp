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

// libjoin.
#include <join/variant.hpp>

// Libraries.
#include <gtest/gtest.h>

// C++.
#include <mutex>

using join::Variant;

/**
 * @brief Test is_first_default_constructible trait.
 */
TEST (Variant, is_first_default_constructible)
{
    auto can = join::details::is_first_default_constructible <int, double, std::string, bool, std::nullptr_t>::value;
    EXPECT_TRUE (can);

    can = join::details::is_first_default_constructible <std::lock_guard <std::mutex>, int, double, std::string, bool, std::nullptr_t>::value;
    EXPECT_FALSE (can);
}

/**
 * @brief Test default construction.
 */
TEST (Variant, defaultConstruction)
{
    Variant <int, double, std::string, bool, std::nullptr_t> var;

    EXPECT_TRUE (var.is <int> ());
    EXPECT_FALSE (var.is <double> ());
    EXPECT_FALSE (var.is <std::string> ());
    EXPECT_FALSE (var.is <bool> ());
    EXPECT_FALSE (var.is <std::nullptr_t> ());
}

/**
 * @brief Test copy construction.
 */
TEST (Variant, copyConstruction)
{
    Variant <int, double, std::string, bool, std::nullptr_t> var;

    var = 6;
    Variant <int, double, std::string, bool, std::nullptr_t> var1 (var);
    ASSERT_TRUE (var1.is <int> ());
    EXPECT_EQ (var1.get <int> (), 6);
    EXPECT_EQ (var1.get <0> (), 6);
    ASSERT_NE (var1.getIf <int> (), nullptr);
    EXPECT_EQ (*var1.getIf <int> (), 6);
    ASSERT_NE (var1.getIf <0> (), nullptr);
    EXPECT_EQ (*var1.getIf <0> (), 6);

    volatile int volatileInt = 8;
    var = volatileInt;
    Variant <int, double, std::string, bool, std::nullptr_t> var2 (var);
    ASSERT_TRUE (var2.is <int> ());
    EXPECT_EQ (var2.get <int> (), 8);
    EXPECT_EQ (var2.get <0> (), 8);
    ASSERT_NE (var2.getIf <int> (), nullptr);
    EXPECT_EQ (*var2.getIf <int> (), 8);
    ASSERT_NE (var2.getIf <0> (), nullptr);
    EXPECT_EQ (*var2.getIf <0> (), 8);

    var = 0.5;
    Variant <int, double, std::string, bool, std::nullptr_t> var3 (var);
    ASSERT_TRUE (var3.is <double> ());
    EXPECT_DOUBLE_EQ (var3.get <double> (), 0.5);
    EXPECT_DOUBLE_EQ (var3.get <1> (), 0.5);
    ASSERT_NE (var3.getIf <double> (), nullptr);
    EXPECT_DOUBLE_EQ (*var3.getIf <double> (), 0.5);
    ASSERT_NE (var3.getIf <1> (), nullptr);
    EXPECT_DOUBLE_EQ (*var3.getIf <1> (), 0.5);

    var = std::string ("foobar1");
    Variant <int, double, std::string, bool, std::nullptr_t> var5 (var);
    ASSERT_TRUE (var5.is <std::string> ());
    EXPECT_STREQ (var5.get <std::string> ().c_str (), "foobar1");
    EXPECT_STREQ (var5.get <2> ().c_str (), "foobar1");
    ASSERT_NE (var5.getIf <std::string> (), nullptr);
    EXPECT_STREQ (var5.getIf <std::string> ()->c_str (), "foobar1");
    ASSERT_NE (var5.getIf <2> (), nullptr);
    EXPECT_STREQ (var5.getIf <2> ()->c_str (), "foobar1");

    const std::string constString ("foobar2");
    var = constString;
    Variant <int, double, std::string, bool, std::nullptr_t> var6 (var);
    ASSERT_TRUE (var6.is <std::string> ());
    EXPECT_STREQ (var6.get <std::string> ().c_str (), "foobar2");
    EXPECT_STREQ (var6.get <2> ().c_str (), "foobar2");
    ASSERT_NE (var6.getIf <std::string> (), nullptr);
    EXPECT_STREQ (var6.getIf <std::string> ()->c_str (), "foobar2");
    ASSERT_NE (var6.getIf <2> (), nullptr);
    EXPECT_STREQ (var6.getIf <2> ()->c_str (), "foobar2");

    var = true;
    Variant <int, double, std::string, bool, std::nullptr_t> var7 (var);
    ASSERT_TRUE (var7.is <bool> ());
    EXPECT_TRUE (var7.get <bool> ());
    EXPECT_TRUE (var7.get <3> ());
    ASSERT_NE (var7.getIf <bool> (), nullptr);
    EXPECT_TRUE (*var7.getIf <bool> ());
    ASSERT_NE (var7.getIf <3> (), nullptr);
    EXPECT_TRUE (*var7.getIf <3> ());

    var = nullptr;
    Variant <int, double, std::string, bool, std::nullptr_t> var8 (var);
    ASSERT_TRUE (var8.is <std::nullptr_t> ());
    EXPECT_EQ (var8.get <std::nullptr_t> (), nullptr);
    EXPECT_EQ (var8.get <4> (), nullptr);
    ASSERT_NE (var8.getIf <std::nullptr_t> (), nullptr);
    EXPECT_EQ (*var8.getIf <std::nullptr_t> (), nullptr);
    ASSERT_NE (var8.getIf <4> (), nullptr);
    EXPECT_EQ (*var8.getIf <4> (), nullptr);
}

/**
 * @brief Test move construction.
 */
TEST (Variant, moveConstruction)
{
    Variant <int, double, std::string, bool, std::nullptr_t> var;

    var = 6;
    Variant <int, double, std::string, bool, std::nullptr_t> var1 (std::move (var));
    ASSERT_TRUE (var1.is <int> ());
    EXPECT_EQ (var1.get <int> (), 6);
    EXPECT_EQ (var1.get <0> (), 6);
    ASSERT_NE (var1.getIf <int> (), nullptr);
    EXPECT_EQ (*var1.getIf <int> (), 6);
    ASSERT_NE (var1.getIf <0> (), nullptr);
    EXPECT_EQ (*var1.getIf <0> (), 6);

    volatile int volatileInt = 8;
    var = volatileInt;
    Variant <int, double, std::string, bool, std::nullptr_t> var2 (std::move (var));
    ASSERT_TRUE (var2.is <int> ());
    EXPECT_EQ (var2.get <int> (), 8);
    EXPECT_EQ (var2.get <0> (), 8);
    ASSERT_NE (var2.getIf <int> (), nullptr);
    EXPECT_EQ (*var2.getIf <int> (), 8);
    ASSERT_NE (var2.getIf <0> (), nullptr);
    EXPECT_EQ (*var2.getIf <0> (), 8);

    var = 0.5;
    Variant <int, double, std::string, bool, std::nullptr_t> var3 (std::move (var));
    ASSERT_TRUE (var3.is <double> ());
    EXPECT_DOUBLE_EQ (var3.get <double> (), 0.5);
    EXPECT_DOUBLE_EQ (var3.get <1> (), 0.5);
    ASSERT_NE (var3.getIf <double> (), nullptr);
    EXPECT_DOUBLE_EQ (*var3.getIf <double> (), 0.5);
    ASSERT_NE (var3.getIf <1> (), nullptr);
    EXPECT_DOUBLE_EQ (*var3.getIf <1> (), 0.5);

    var = std::string ("foobar1");
    Variant <int, double, std::string, bool, std::nullptr_t> var5 (std::move (var));
    ASSERT_TRUE (var5.is <std::string> ());
    EXPECT_STREQ (var5.get <std::string> ().c_str (), "foobar1");
    EXPECT_STREQ (var5.get <2> ().c_str (), "foobar1");
    ASSERT_NE (var5.getIf <std::string> (), nullptr);
    EXPECT_STREQ (var5.getIf <std::string> ()->c_str (), "foobar1");
    ASSERT_NE (var5.getIf <2> (), nullptr);
    EXPECT_STREQ (var5.getIf <2> ()->c_str (), "foobar1");

    const std::string constString ("foobar2");
    var = constString;
    Variant <int, double, std::string, bool, std::nullptr_t> var6 (std::move (var));
    ASSERT_TRUE (var6.is <std::string> ());
    EXPECT_STREQ (var6.get <std::string> ().c_str (), "foobar2");
    EXPECT_STREQ (var6.get <2> ().c_str (), "foobar2");
    ASSERT_NE (var6.getIf <std::string> (), nullptr);
    EXPECT_STREQ (var6.getIf <std::string> ()->c_str (), "foobar2");
    ASSERT_NE (var6.getIf <2> (), nullptr);
    EXPECT_STREQ (var6.getIf <2> ()->c_str (), "foobar2");

    var = true;
    Variant <int, double, std::string, bool, std::nullptr_t> var7 (std::move (var));
    ASSERT_TRUE (var7.is <bool> ());
    EXPECT_TRUE (var7.get <bool> ());
    EXPECT_TRUE (var7.get <3> ());
    ASSERT_NE (var7.getIf <bool> (), nullptr);
    EXPECT_TRUE (*var7.getIf <bool> ());
    ASSERT_NE (var7.getIf <3> (), nullptr);
    EXPECT_TRUE (*var7.getIf <3> ());

    var = nullptr;
    Variant <int, double, std::string, bool, std::nullptr_t> var8 (std::move (var));
    ASSERT_TRUE (var8.is <std::nullptr_t> ());
    EXPECT_EQ (var8.get <std::nullptr_t> (), nullptr);
    EXPECT_EQ (var8.get <4> (), nullptr);
    ASSERT_NE (var8.getIf <std::nullptr_t> (), nullptr);
    EXPECT_EQ (*var8.getIf <std::nullptr_t> (), nullptr);
    ASSERT_NE (var8.getIf <4> (), nullptr);
    EXPECT_EQ (*var8.getIf <4> (), nullptr);
}

/**
 * @brief Test convert construction.
 */
TEST (Variant, convertConstruction)
{
    Variant <int, double, std::string, bool, std::nullptr_t> var (120);
    ASSERT_TRUE (var.is <int> ());
    EXPECT_EQ (var.get <int> (), 120);
    EXPECT_EQ (var.get <0> (), 120);
    ASSERT_NE (var.getIf <int> (), nullptr);
    EXPECT_EQ (*var.getIf <int> (), 120);
    ASSERT_NE (var.getIf <0> (), nullptr);
    EXPECT_EQ (*var.getIf <0> (), 120);

    volatile int volatileInt (4);
    Variant <int, double, std::string, bool, std::nullptr_t> var1 (volatileInt);
    ASSERT_TRUE (var1.is <int> ());
    EXPECT_EQ (var1.get <int> (), 4);
    EXPECT_EQ (var1.get <0> (), 4);
    ASSERT_NE (var1.getIf <int> (), nullptr);
    EXPECT_EQ (*var1.getIf <int> (), 4);
    ASSERT_NE (var1.getIf <0> (), nullptr);
    EXPECT_EQ (*var1.getIf <0> (), 4);

    Variant <int, double, std::string, bool, std::nullptr_t> var2 (0.5);
    ASSERT_TRUE (var2.is <double> ());
    EXPECT_DOUBLE_EQ (var2.get <double> (), 0.5);
    EXPECT_DOUBLE_EQ (var2.get <1> (), 0.5);
    ASSERT_NE (var2.getIf <double> (), nullptr);
    EXPECT_DOUBLE_EQ (*var2.getIf <double> (), 0.5);
    ASSERT_NE (var2.getIf <1> (), nullptr);
    EXPECT_DOUBLE_EQ (*var2.getIf <1> (), 0.5);

    Variant <int, double, std::string, bool, std::nullptr_t> var4 (std::string ("foobar1"));
    ASSERT_TRUE (var4.is <std::string> ());
    EXPECT_STREQ (var4.get <std::string> ().c_str (), "foobar1");
    EXPECT_STREQ (var4.get <2> ().c_str (), "foobar1");
    ASSERT_NE (var4.getIf <std::string> (), nullptr);
    EXPECT_STREQ (var4.getIf <std::string> ()->c_str (), "foobar1");
    ASSERT_NE (var4.getIf <2> (), nullptr);
    EXPECT_STREQ (var4.getIf <2> ()->c_str (), "foobar1");

    const std::string constString ("foobar2");
    Variant <int, double, std::string, bool, std::nullptr_t> var5 (constString);
    ASSERT_TRUE (var5.is <std::string> ());
    EXPECT_STREQ (var5.get <std::string> ().c_str (), "foobar2");
    EXPECT_STREQ (var5.get <2> ().c_str (), "foobar2");
    ASSERT_NE (var5.getIf <std::string> (), nullptr);
    EXPECT_STREQ (var5.getIf <std::string> ()->c_str (), "foobar2");
    ASSERT_NE (var5.getIf <2> (), nullptr);
    EXPECT_STREQ (var5.getIf <2> ()->c_str (), "foobar2");

    Variant <int, double, std::string, bool, std::nullptr_t> var6 (false);
    ASSERT_TRUE (var6.is <bool> ());
    EXPECT_FALSE (var6.get <bool> ());
    EXPECT_FALSE (var6.get <3> ());
    ASSERT_NE (var6.getIf <bool> (), nullptr);
    EXPECT_FALSE (*var6.getIf <bool> ());
    ASSERT_NE (var6.getIf <3> (), nullptr);
    EXPECT_FALSE (*var6.getIf <3> ());

    Variant <int, double, std::string, bool, std::nullptr_t> var7 (nullptr);
    ASSERT_TRUE (var7.is <std::nullptr_t> ());
    EXPECT_EQ (var7.get <std::nullptr_t> (), nullptr);
    EXPECT_EQ (var7.get <4> (), nullptr);
    ASSERT_NE (var7.getIf <std::nullptr_t> (), nullptr);
    EXPECT_EQ (*var7.getIf <std::nullptr_t> (), nullptr);
    ASSERT_NE (var7.getIf <4> (), nullptr);
    EXPECT_EQ (*var7.getIf <4> (), nullptr);
}

/**
 * @brief Test explicit construction.
 */
TEST (Variant, explicitConstruction)
{
    Variant <int, double, std::string, bool, std::nullptr_t, std::vector <int>> var (join::in_place_type_t <int> {}, 120);
    ASSERT_TRUE (var.is <int> ());
    EXPECT_EQ (var.get <int> (), 120);
    EXPECT_EQ (var.get <0> (), 120);
    ASSERT_NE (var.getIf <int> (), nullptr);
    EXPECT_EQ (*var.getIf <int> (), 120);
    ASSERT_NE (var.getIf <0> (), nullptr);
    EXPECT_EQ (*var.getIf <0> (), 120);

    volatile int volatileInt (4);
    Variant <int, double, std::string, bool, std::nullptr_t, std::vector <int>> var1 (join::in_place_type_t <int> {}, volatileInt);
    ASSERT_TRUE (var1.is <int> ());
    EXPECT_EQ (var1.get <int> (), 4);
    EXPECT_EQ (var1.get <0> (), 4);
    ASSERT_NE (var1.getIf <int> (), nullptr);
    EXPECT_EQ (*var1.getIf <int> (), 4);
    ASSERT_NE (var1.getIf <0> (), nullptr);
    EXPECT_EQ (*var1.getIf <0> (), 4);

    Variant <int, double, std::string, bool, std::nullptr_t, std::vector <int>> var2 (join::in_place_type_t <double> {}, 0.5);
    ASSERT_TRUE (var2.is <double> ());
    EXPECT_DOUBLE_EQ (var2.get <double> (), 0.5);
    EXPECT_DOUBLE_EQ (var2.get <1> (), 0.5);
    ASSERT_NE (var2.getIf <double> (), nullptr);
    EXPECT_DOUBLE_EQ (*var2.getIf <double> (), 0.5);
    ASSERT_NE (var2.getIf <1> (), nullptr);
    EXPECT_DOUBLE_EQ (*var2.getIf <1> (), 0.5);

    Variant <int, double, std::string, bool, std::nullptr_t, std::vector <int>> var4 (join::in_place_type_t <std::string> {}, std::string ("foobar1"));
    ASSERT_TRUE (var4.is <std::string> ());
    EXPECT_STREQ (var4.get <std::string> ().c_str (), "foobar1");
    EXPECT_STREQ (var4.get <2> ().c_str (), "foobar1");
    ASSERT_NE (var4.getIf <std::string> (), nullptr);
    EXPECT_STREQ (var4.getIf <std::string> ()->c_str (), "foobar1");
    ASSERT_NE (var4.getIf <2> (), nullptr);
    EXPECT_STREQ (var4.getIf <2> ()->c_str (), "foobar1");

    const std::string constString ("foobar2");
    Variant <int, double, std::string, bool, std::nullptr_t, std::vector <int>> var5 (join::in_place_type_t <std::string> {}, constString);
    ASSERT_TRUE (var5.is <std::string> ());
    EXPECT_STREQ (var5.get <std::string> ().c_str (), "foobar2");
    EXPECT_STREQ (var5.get <2> ().c_str (), "foobar2");
    ASSERT_NE (var5.getIf <std::string> (), nullptr);
    EXPECT_STREQ (var5.getIf <std::string> ()->c_str (), "foobar2");
    ASSERT_NE (var5.getIf <2> (), nullptr);
    EXPECT_STREQ (var5.getIf <2> ()->c_str (), "foobar2");

    Variant <int, double, std::string, bool, std::nullptr_t, std::vector <int>> var6 (join::in_place_type_t <bool> {}, false);
    ASSERT_TRUE (var6.is <bool> ());
    EXPECT_FALSE (var6.get <bool> ());
    EXPECT_FALSE (var6.get <3> ());
    ASSERT_NE (var6.getIf <bool> (), nullptr);
    EXPECT_FALSE (*var6.getIf <bool> ());
    ASSERT_NE (var6.getIf <3> (), nullptr);
    EXPECT_FALSE (*var6.getIf <3> ());

    Variant <int, double, std::string, bool, std::nullptr_t, std::vector <int>> var7 (join::in_place_type_t <std::nullptr_t> {}, nullptr);
    ASSERT_TRUE (var7.is <std::nullptr_t> ());
    EXPECT_EQ (var7.get <std::nullptr_t> (), nullptr);
    EXPECT_EQ (var7.get <4> (), nullptr);
    ASSERT_NE (var7.getIf <std::nullptr_t> (), nullptr);
    EXPECT_EQ (*var7.getIf <std::nullptr_t> (), nullptr);
    ASSERT_NE (var7.getIf <4> (), nullptr);
    EXPECT_EQ (*var7.getIf <4> (), nullptr);

    Variant <int, double, std::string, bool, std::nullptr_t, std::vector <int>> var8 (join::in_place_type_t <std::vector <int>> {}, {0, 1, 2, 3});
    ASSERT_TRUE (var8.is <std::vector <int>> ());
    ASSERT_EQ (var8.get <std::vector <int>> ().size (), 4);
    EXPECT_EQ (var8.get <std::vector <int>> ()[0], 0);
    EXPECT_EQ (var8.get <std::vector <int>> ()[1], 1);
    EXPECT_EQ (var8.get <std::vector <int>> ()[2], 2);
    EXPECT_EQ (var8.get <std::vector <int>> ()[3], 3);
    ASSERT_NE (var8.getIf <std::vector <int>> (), nullptr);
    EXPECT_EQ (var8.getIf <std::vector <int>> ()->size (), 4);
    EXPECT_EQ ((*var8.getIf <std::vector <int>> ())[0], 0);
    EXPECT_EQ ((*var8.getIf <std::vector <int>> ())[1], 1);
    EXPECT_EQ ((*var8.getIf <std::vector <int>> ())[2], 2);
    EXPECT_EQ ((*var8.getIf <std::vector <int>> ())[3], 3);
}

/**
 * @brief Test index construction.
 */
TEST (Variant, indexConstruction)
{
    Variant <int, double, std::string, bool, std::nullptr_t, std::vector <int>> var (join::in_place_index_t <0> {}, 120);
    ASSERT_TRUE (var.is <int> ());
    EXPECT_EQ (var.get <int> (), 120);
    EXPECT_EQ (var.get <0> (), 120);
    ASSERT_NE (var.getIf <int> (), nullptr);
    EXPECT_EQ (*var.getIf <int> (), 120);
    ASSERT_NE (var.getIf <0> (), nullptr);
    EXPECT_EQ (*var.getIf <0> (), 120);

    volatile int volatileInt (4);
    Variant <int, double, std::string, bool, std::nullptr_t, std::vector <int>> var1 (join::in_place_index_t <0> {}, volatileInt);
    ASSERT_TRUE (var1.is <int> ());
    EXPECT_EQ (var1.get <int> (), 4);
    EXPECT_EQ (var1.get <0> (), 4);
    ASSERT_NE (var1.getIf <int> (), nullptr);
    EXPECT_EQ (*var1.getIf <int> (), 4);
    ASSERT_NE (var1.getIf <0> (), nullptr);
    EXPECT_EQ (*var1.getIf <0> (), 4);

    Variant <int, double, std::string, bool, std::nullptr_t, std::vector <int>> var2 (join::in_place_index_t <1> {}, 0.5);
    ASSERT_TRUE (var2.is <double> ());
    EXPECT_DOUBLE_EQ (var2.get <double> (), 0.5);
    EXPECT_DOUBLE_EQ (var2.get <1> (), 0.5);
    ASSERT_NE (var2.getIf <double> (), nullptr);
    EXPECT_DOUBLE_EQ (*var2.getIf <double> (), 0.5);
    ASSERT_NE (var2.getIf <1> (), nullptr);
    EXPECT_DOUBLE_EQ (*var2.getIf <1> (), 0.5);

    Variant <int, double, std::string, bool, std::nullptr_t, std::vector <int>> var4 (join::in_place_index_t <2> {}, std::string ("foobar1"));
    ASSERT_TRUE (var4.is <std::string> ());
    EXPECT_STREQ (var4.get <std::string> ().c_str (), "foobar1");
    EXPECT_STREQ (var4.get <2> ().c_str (), "foobar1");
    ASSERT_NE (var4.getIf <std::string> (), nullptr);
    EXPECT_STREQ (var4.getIf <std::string> ()->c_str (), "foobar1");
    ASSERT_NE (var4.getIf <2> (), nullptr);
    EXPECT_STREQ (var4.getIf <2> ()->c_str (), "foobar1");

    const std::string constString ("foobar2");
    Variant <int, double, std::string, bool, std::nullptr_t, std::vector <int>> var5 (join::in_place_index_t <2> {}, constString);
    ASSERT_TRUE (var5.is <std::string> ());
    EXPECT_STREQ (var5.get <std::string> ().c_str (), "foobar2");
    EXPECT_STREQ (var5.get <2> ().c_str (), "foobar2");
    ASSERT_NE (var5.getIf <std::string> (), nullptr);
    EXPECT_STREQ (var5.getIf <std::string> ()->c_str (), "foobar2");
    ASSERT_NE (var5.getIf <2> (), nullptr);
    EXPECT_STREQ (var5.getIf <2> ()->c_str (), "foobar2");

    Variant <int, double, std::string, bool, std::nullptr_t, std::vector <int>> var6 (join::in_place_index_t <3> {}, false);
    ASSERT_TRUE (var6.is <bool> ());
    EXPECT_FALSE (var6.get <bool> ());
    EXPECT_FALSE (var6.get <3> ());
    ASSERT_NE (var6.getIf <bool> (), nullptr);
    EXPECT_FALSE (*var6.getIf <bool> ());
    ASSERT_NE (var6.getIf <3> (), nullptr);
    EXPECT_FALSE (*var6.getIf <3> ());

    Variant <int, double, std::string, bool, std::nullptr_t, std::vector <int>> var7 (join::in_place_index_t <4> {}, nullptr);
    ASSERT_TRUE (var7.is <std::nullptr_t> ());
    EXPECT_EQ (var7.get <std::nullptr_t> (), nullptr);
    EXPECT_EQ (var7.get <4> (), nullptr);
    ASSERT_NE (var7.getIf <std::nullptr_t> (), nullptr);
    EXPECT_EQ (*var7.getIf <std::nullptr_t> (), nullptr);
    ASSERT_NE (var7.getIf <4> (), nullptr);
    EXPECT_EQ (*var7.getIf <4> (), nullptr);

    Variant <int, double, std::string, bool, std::nullptr_t, std::vector <int>> var8 (join::in_place_index_t <5> {}, {0, 1, 2, 3});
    ASSERT_TRUE (var8.is <std::vector <int>> ());
    ASSERT_EQ (var8.get <std::vector <int>> ().size (), 4);
    EXPECT_EQ (var8.get <std::vector <int>> ()[0], 0);
    EXPECT_EQ (var8.get <std::vector <int>> ()[1], 1);
    EXPECT_EQ (var8.get <std::vector <int>> ()[2], 2);
    EXPECT_EQ (var8.get <std::vector <int>> ()[3], 3);
    ASSERT_NE (var8.getIf <std::vector <int>> (), nullptr);
    EXPECT_EQ (var8.getIf <std::vector <int>> ()->size (), 4);
    EXPECT_EQ ((*var8.getIf <std::vector <int>> ())[0], 0);
    EXPECT_EQ ((*var8.getIf <std::vector <int>> ())[1], 1);
    EXPECT_EQ ((*var8.getIf <std::vector <int>> ())[2], 2);
    EXPECT_EQ ((*var8.getIf <std::vector <int>> ())[3], 3);
}

/**
 * @brief Test copy.
 */
TEST (Variant, copyAssign)
{
    Variant <int, double, std::string, bool, std::nullptr_t> var;

    var = 6;
    Variant <int, double, std::string, bool, std::nullptr_t> var1;
    var1 = var;
    ASSERT_TRUE (var1.is <int> ());
    EXPECT_EQ (var1.get <int> (), 6);
    EXPECT_EQ (var1.get <0> (), 6);
    ASSERT_NE (var1.getIf <int> (), nullptr);
    EXPECT_EQ (*var1.getIf <int> (), 6);
    ASSERT_NE (var1.getIf <0> (), nullptr);
    EXPECT_EQ (*var1.getIf <0> (), 6);

    volatile int volatileInt (4);
    var = volatileInt;
    Variant <int, double, std::string, bool, std::nullptr_t> var2;
    var2 = var;
    ASSERT_TRUE (var2.is <int> ());
    EXPECT_EQ (var2.get <int> (), 4);
    EXPECT_EQ (var2.get <0> (), 4);
    ASSERT_NE (var2.getIf <int> (), nullptr);
    EXPECT_EQ (*var2.getIf <int> (), 4);
    ASSERT_NE (var2.getIf <0> (), nullptr);
    EXPECT_EQ (*var2.getIf <0> (), 4);

    var = 0.5;
    Variant <int, double, std::string, bool, std::nullptr_t> var3;
    var3 = var;
    ASSERT_TRUE (var3.is <double> ());
    EXPECT_DOUBLE_EQ (var3.get <double> (), 0.5);
    EXPECT_DOUBLE_EQ (var3.get <1> (), 0.5);
    ASSERT_NE (var3.getIf <double> (), nullptr);
    EXPECT_DOUBLE_EQ (*var3.getIf <double> (), 0.5);
    ASSERT_NE (var3.getIf <1> (), nullptr);
    EXPECT_DOUBLE_EQ (*var3.getIf <1> (), 0.5);

    var = std::string ("foobar1");
    Variant <int, double, std::string, bool, std::nullptr_t> var5;
    var5 = var;
    ASSERT_TRUE (var5.is <std::string> ());
    EXPECT_STREQ (var5.get <std::string> ().c_str (), "foobar1");
    EXPECT_STREQ (var5.get <2> ().c_str (), "foobar1");
    ASSERT_NE (var5.getIf <std::string> (), nullptr);
    EXPECT_STREQ (var5.getIf <std::string> ()->c_str (), "foobar1");
    ASSERT_NE (var5.getIf <2> (), nullptr);
    EXPECT_STREQ (var5.getIf <2> ()->c_str (), "foobar1");

    const std::string constString ("foobar2");
    var = constString;
    Variant <int, double, std::string, bool, std::nullptr_t> var6;
    var6 = var;
    ASSERT_TRUE (var6.is <std::string> ());
    EXPECT_STREQ (var6.get <std::string> ().c_str (), "foobar2");
    EXPECT_STREQ (var6.get <2> ().c_str (), "foobar2");
    ASSERT_NE (var6.getIf <std::string> (), nullptr);
    EXPECT_STREQ (var6.getIf <std::string> ()->c_str (), "foobar2");
    ASSERT_NE (var6.getIf <2> (), nullptr);
    EXPECT_STREQ (var6.getIf <2> ()->c_str (), "foobar2");

    var = true;
    Variant <int, double, std::string, bool, std::nullptr_t> var7;
    var7 = var;
    ASSERT_TRUE (var7.is <bool> ());
    EXPECT_TRUE (var7.get <bool> ());
    EXPECT_TRUE (var7.get <3> ());
    ASSERT_NE (var7.getIf <bool> (), nullptr);
    EXPECT_TRUE (*var7.getIf <bool> ());
    ASSERT_NE (var7.getIf <3> (), nullptr);
    EXPECT_TRUE (*var7.getIf <3> ());

    var = nullptr;
    Variant <int, double, std::string, bool, std::nullptr_t> var8;
    var8 = var;
    ASSERT_TRUE (var8.is <std::nullptr_t> ());
    EXPECT_EQ (var8.get <std::nullptr_t> (), nullptr);
    EXPECT_EQ (var8.get <4> (), nullptr);
    ASSERT_NE (var8.getIf <std::nullptr_t> (), nullptr);
    EXPECT_EQ (*var8.getIf <std::nullptr_t> (), nullptr);
    ASSERT_NE (var8.getIf <4> (), nullptr);
    EXPECT_EQ (*var8.getIf <4> (), nullptr);
}

/**
 * @brief Test movement.
 */
TEST (Variant, moveAssign)
{
    Variant <int, double, std::string, bool, std::nullptr_t> var;

    var = 6;
    Variant <int, double, std::string, bool, std::nullptr_t> var1;
    var1 = std::move (var);
    ASSERT_TRUE (var1.is <int> ());
    EXPECT_EQ (var1.get <int> (), 6);
    EXPECT_EQ (var1.get <0> (), 6);
    ASSERT_NE (var1.getIf <int> (), nullptr);
    EXPECT_EQ (*var1.getIf <int> (), 6);
    ASSERT_NE (var1.getIf <0> (), nullptr);
    EXPECT_EQ (*var1.getIf <0> (), 6);

    volatile int volatileInt (4);
    var = volatileInt;
    Variant <int, double, std::string, bool, std::nullptr_t> var2;
    var2 = std::move (var);
    ASSERT_TRUE (var2.is <int> ());
    EXPECT_EQ (var2.get <int> (), 4);
    EXPECT_EQ (var2.get <0> (), 4);
    ASSERT_NE (var2.getIf <int> (), nullptr);
    EXPECT_EQ (*var2.getIf <int> (), 4);
    ASSERT_NE (var2.getIf <0> (), nullptr);
    EXPECT_EQ (*var2.getIf <0> (), 4);

    var = 0.5;
    Variant <int, double, std::string, bool, std::nullptr_t> var3;
    var3 = std::move (var);
    ASSERT_TRUE (var3.is <double> ());
    EXPECT_DOUBLE_EQ (var3.get <double> (), 0.5);
    EXPECT_DOUBLE_EQ (var3.get <1> (), 0.5);
    ASSERT_NE (var3.getIf <double> (), nullptr);
    EXPECT_DOUBLE_EQ (*var3.getIf <double> (), 0.5);
    ASSERT_NE (var3.getIf <1> (), nullptr);
    EXPECT_DOUBLE_EQ (*var3.getIf <1> (), 0.5);

    var = std::string ("foobar1");
    Variant <int, double, std::string, bool, std::nullptr_t> var5;
    var5 = std::move (var);
    ASSERT_TRUE (var5.is <std::string> ());
    EXPECT_STREQ (var5.get <std::string> ().c_str (), "foobar1");
    EXPECT_STREQ (var5.get <2> ().c_str (), "foobar1");
    ASSERT_NE (var5.getIf <std::string> (), nullptr);
    EXPECT_STREQ (var5.getIf <std::string> ()->c_str (), "foobar1");
    ASSERT_NE (var5.getIf <2> (), nullptr);
    EXPECT_STREQ (var5.getIf <2> ()->c_str (), "foobar1");

    const std::string constString ("foobar2");
    var = constString;
    Variant <int, double, std::string, bool, std::nullptr_t> var6;
    var6 = std::move (var);
    ASSERT_TRUE (var6.is <std::string> ());
    EXPECT_STREQ (var6.get <std::string> ().c_str (), "foobar2");
    EXPECT_STREQ (var6.get <2> ().c_str (), "foobar2");
    ASSERT_NE (var6.getIf <std::string> (), nullptr);
    EXPECT_STREQ (var6.getIf <std::string> ()->c_str (), "foobar2");
    ASSERT_NE (var6.getIf <2> (), nullptr);
    EXPECT_STREQ (var6.getIf <2> ()->c_str (), "foobar2");

    var = true;
    Variant <int, double, std::string, bool, std::nullptr_t> var7;
    var7 = std::move (var);
    ASSERT_TRUE (var7.is <bool> ());
    EXPECT_TRUE (var7.get <bool> ());
    EXPECT_TRUE (var7.get <3> ());
    ASSERT_NE (var7.getIf <bool> (), nullptr);
    EXPECT_TRUE (*var7.getIf <bool> ());
    ASSERT_NE (var7.getIf <3> (), nullptr);
    EXPECT_TRUE (*var7.getIf <3> ());

    var = nullptr;
    Variant <int, double, std::string, bool, std::nullptr_t> var8;
    var8 = std::move (var);
    ASSERT_TRUE (var8.is <std::nullptr_t> ());
    EXPECT_EQ (var8.get <std::nullptr_t> (), nullptr);
    EXPECT_EQ (var8.get <4> (), nullptr);
    ASSERT_NE (var8.getIf <std::nullptr_t> (), nullptr);
    EXPECT_EQ (*var8.getIf <std::nullptr_t> (), nullptr);
    ASSERT_NE (var8.getIf <4> (), nullptr);
    EXPECT_EQ (*var8.getIf <4> (), nullptr);
}

/**
 * @brief Test overload resolution.
 */
TEST (Variant, convertAssign)
{
    Variant <int, double, std::string, bool, std::nullptr_t> var;

    var = 3;
    ASSERT_TRUE (var.is <int> ());
    EXPECT_EQ (var.get <int> (), 3);
    EXPECT_EQ (var.get <0> (), 3);
    ASSERT_NE (var.getIf <int> (), nullptr);
    EXPECT_EQ (*var.getIf <int> (), 3);
    ASSERT_NE (var.getIf <0> (), nullptr);
    EXPECT_EQ (*var.getIf <0> (), 3);

    volatile int volatileInt (4);
    var = volatileInt;
    ASSERT_TRUE (var.is <int> ());
    EXPECT_EQ (var.get <int> (), 4);
    EXPECT_EQ (var.get <0> (), 4);
    ASSERT_NE (var.getIf <int> (), nullptr);
    EXPECT_EQ (*var.getIf <int> (), 4);
    ASSERT_NE (var.getIf <0> (), nullptr);
    EXPECT_EQ (*var.getIf <0> (), 4);

    var = 0.5;
    ASSERT_TRUE (var.is <double> ());
    EXPECT_DOUBLE_EQ (var.get <double> (), 0.5);
    EXPECT_DOUBLE_EQ (var.get <1> (), 0.5);
    ASSERT_NE (var.getIf <double> (), nullptr);
    EXPECT_DOUBLE_EQ (*var.getIf <double> (), 0.5);
    ASSERT_NE (var.getIf <1> (), nullptr);
    EXPECT_DOUBLE_EQ (*var.getIf <1> (), 0.5);

    var = std::string ("foobar1");
    ASSERT_TRUE (var.is <std::string> ());
    EXPECT_STREQ (var.get <std::string> ().c_str (), "foobar1");
    EXPECT_STREQ (var.get <2> ().c_str (), "foobar1");
    ASSERT_NE (var.getIf <std::string> (), nullptr);
    EXPECT_STREQ (var.getIf <std::string> ()->c_str (), "foobar1");
    ASSERT_NE (var.getIf <2> (), nullptr);
    EXPECT_STREQ (var.getIf <2> ()->c_str (), "foobar1");

    const std::string constString ("foobar2");
    var = constString;
    ASSERT_TRUE (var.is <std::string> ());
    EXPECT_STREQ (var.get <std::string> ().c_str (), "foobar2");
    EXPECT_STREQ (var.get <2> ().c_str (), "foobar2");
    ASSERT_NE (var.getIf <std::string> (), nullptr);
    EXPECT_STREQ (var.getIf <std::string> ()->c_str (), "foobar2");
    ASSERT_NE (var.getIf <2> (), nullptr);
    EXPECT_STREQ (var.getIf <2> ()->c_str (), "foobar2");

    var = false;
    ASSERT_TRUE (var.is <bool> ());
    EXPECT_FALSE (var.get <bool> ());
    EXPECT_FALSE (var.get <3> ());
    ASSERT_NE (var.getIf <bool> (), nullptr);
    EXPECT_FALSE (*var.getIf <bool> ());
    ASSERT_NE (var.getIf <3> (), nullptr);
    EXPECT_FALSE (*var.getIf <3> ());

    var = nullptr;
    ASSERT_TRUE (var.is <std::nullptr_t> ());
    EXPECT_EQ (var.get <std::nullptr_t> (), nullptr);
    EXPECT_EQ (var.get <4> (), nullptr);
    ASSERT_NE (var.getIf <std::nullptr_t> (), nullptr);
    EXPECT_EQ (*var.getIf <std::nullptr_t> (), nullptr);
    ASSERT_NE (var.getIf <4> (), nullptr);
    EXPECT_EQ (*var.getIf <4> (), nullptr);
}

/**
 * @brief Test get method.
 */
TEST (Variant, get)
{
    Variant <int, double, std::string, bool, std::nullptr_t> var;

    var.set <int> (6);
    EXPECT_NO_THROW (var.get <int> ());
    EXPECT_THROW (var.get <double> (), std::bad_cast);
    EXPECT_THROW (var.get <std::string> (), std::bad_cast);
    EXPECT_THROW (var.get <bool> (), std::bad_cast);
    EXPECT_THROW (var.get <std::nullptr_t> (), std::bad_cast);
    EXPECT_NO_THROW (var.get <0> ());
    EXPECT_THROW (var.get <1> (), std::bad_cast);
    EXPECT_THROW (var.get <2> (), std::bad_cast);
    EXPECT_THROW (var.get <3> (), std::bad_cast);
    EXPECT_THROW (var.get <4> (), std::bad_cast);

    volatile int volatileInt (3);
    var.set <int> (volatileInt);
    EXPECT_NO_THROW (var.get <int> ());
    EXPECT_THROW (var.get <double> (), std::bad_cast);
    EXPECT_THROW (var.get <std::string> (), std::bad_cast);
    EXPECT_THROW (var.get <bool> (), std::bad_cast);
    EXPECT_THROW (var.get <std::nullptr_t> (), std::bad_cast);
    EXPECT_NO_THROW (var.get <0> ());
    EXPECT_THROW (var.get <1> (), std::bad_cast);
    EXPECT_THROW (var.get <2> (), std::bad_cast);
    EXPECT_THROW (var.get <3> (), std::bad_cast);
    EXPECT_THROW (var.get <4> (), std::bad_cast);

    var.set <double> (0.5);
    EXPECT_THROW (var.get <int> (), std::bad_cast);
    EXPECT_NO_THROW (var.get <double> ());
    EXPECT_THROW (var.get <std::string> (), std::bad_cast);
    EXPECT_THROW (var.get <bool> (), std::bad_cast);
    EXPECT_THROW (var.get <std::nullptr_t> (), std::bad_cast);
    EXPECT_THROW (var.get <0> (), std::bad_cast);
    EXPECT_NO_THROW (var.get <1> ());
    EXPECT_THROW (var.get <2> (), std::bad_cast);
    EXPECT_THROW (var.get <3> (), std::bad_cast);
    EXPECT_THROW (var.get <4> (), std::bad_cast);

    var.set <std::string> (std::string ("foobar1"));
    EXPECT_THROW (var.get <int> (), std::bad_cast);
    EXPECT_THROW (var.get <double> (), std::bad_cast);
    EXPECT_NO_THROW (var.get <std::string> ());
    EXPECT_THROW (var.get <bool> (), std::bad_cast);
    EXPECT_THROW (var.get <std::nullptr_t> (), std::bad_cast);
    EXPECT_THROW (var.get <0> (), std::bad_cast);
    EXPECT_THROW (var.get <1> (), std::bad_cast);
    EXPECT_NO_THROW (var.get <2> ());
    EXPECT_THROW (var.get <3> (), std::bad_cast);
    EXPECT_THROW (var.get <4> (), std::bad_cast);

    const std::string constString ("foobar2");
    var.set <std::string> (std::string (constString));
    EXPECT_THROW (var.get <int> (), std::bad_cast);
    EXPECT_THROW (var.get <double> (), std::bad_cast);
    EXPECT_NO_THROW (var.get <std::string> ());
    EXPECT_THROW (var.get <bool> (), std::bad_cast);
    EXPECT_THROW (var.get <std::nullptr_t> (), std::bad_cast);
    EXPECT_THROW (var.get <0> (), std::bad_cast);
    EXPECT_THROW (var.get <1> (), std::bad_cast);
    EXPECT_NO_THROW (var.get <2> ());
    EXPECT_THROW (var.get <3> (), std::bad_cast);
    EXPECT_THROW (var.get <4> (), std::bad_cast);

    var.set <bool> (false);
    EXPECT_THROW (var.get <int> (), std::bad_cast);
    EXPECT_THROW (var.get <double> (), std::bad_cast);
    EXPECT_THROW (var.get <std::string> (), std::bad_cast);
    EXPECT_NO_THROW (var.get <bool> ());
    EXPECT_THROW (var.get <std::nullptr_t> (), std::bad_cast);
    EXPECT_THROW (var.get <0> (), std::bad_cast);
    EXPECT_THROW (var.get <1> (), std::bad_cast);
    EXPECT_THROW (var.get <2> (), std::bad_cast);
    EXPECT_NO_THROW (var.get <3> ());
    EXPECT_THROW (var.get <4> (), std::bad_cast);

    var.set <std::nullptr_t> (nullptr);
    EXPECT_THROW (var.get <int> (), std::bad_cast);
    EXPECT_THROW (var.get <double> (), std::bad_cast);
    EXPECT_THROW (var.get <std::string> (), std::bad_cast);
    EXPECT_THROW (var.get <bool> (), std::bad_cast);
    EXPECT_NO_THROW (var.get <std::nullptr_t> ());
    EXPECT_THROW (var.get <0> (), std::bad_cast);
    EXPECT_THROW (var.get <1> (), std::bad_cast);
    EXPECT_THROW (var.get <2> (), std::bad_cast);
    EXPECT_THROW (var.get <3> (), std::bad_cast);
    EXPECT_NO_THROW (var.get <4> ());
}

/**
 * @brief Test getIf method.
 */
TEST (Variant, getIf)
{
    Variant <int, double, std::string, bool, std::nullptr_t> var;

    var.set <int> (6);
    EXPECT_NE (var.getIf <int> (), nullptr);
    EXPECT_EQ (var.getIf <double> (), nullptr);
    EXPECT_EQ (var.getIf <std::string> (), nullptr);
    EXPECT_EQ (var.getIf <bool> (), nullptr);
    EXPECT_EQ (var.getIf <std::nullptr_t> (), nullptr);
    EXPECT_NE (var.getIf <0> (), nullptr);
    EXPECT_EQ (var.getIf <1> (), nullptr);
    EXPECT_EQ (var.getIf <2> (), nullptr);
    EXPECT_EQ (var.getIf <3> (), nullptr);
    EXPECT_EQ (var.getIf <4> (), nullptr);

    volatile int volatileInt (3);
    var.set <int> (volatileInt);
    EXPECT_NE (var.getIf <int> (), nullptr);
    EXPECT_EQ (var.getIf <double> (), nullptr);
    EXPECT_EQ (var.getIf <std::string> (), nullptr);
    EXPECT_EQ (var.getIf <bool> (), nullptr);
    EXPECT_EQ (var.getIf <std::nullptr_t> (), nullptr);
    EXPECT_NE (var.getIf <0> (), nullptr);
    EXPECT_EQ (var.getIf <1> (), nullptr);
    EXPECT_EQ (var.getIf <2> (), nullptr);
    EXPECT_EQ (var.getIf <3> (), nullptr);
    EXPECT_EQ (var.getIf <4> (), nullptr);

    var.set <double> (0.5);
    EXPECT_EQ (var.getIf <int> (), nullptr);
    EXPECT_NE (var.getIf <double> (), nullptr);
    EXPECT_EQ (var.getIf <std::string> (), nullptr);
    EXPECT_EQ (var.getIf <bool> (), nullptr);
    EXPECT_EQ (var.getIf <std::nullptr_t> (), nullptr);
    EXPECT_EQ (var.getIf <0> (), nullptr);
    EXPECT_NE (var.getIf <1> (), nullptr);
    EXPECT_EQ (var.getIf <2> (), nullptr);
    EXPECT_EQ (var.getIf <3> (), nullptr);
    EXPECT_EQ (var.getIf <4> (), nullptr);

    var.set <std::string> (std::string ("foobar1"));
    EXPECT_EQ (var.getIf <int> (), nullptr);
    EXPECT_EQ (var.getIf <double> (), nullptr);
    EXPECT_NE (var.getIf <std::string> (), nullptr);
    EXPECT_EQ (var.getIf <bool> (), nullptr);
    EXPECT_EQ (var.getIf <std::nullptr_t> (), nullptr);
    EXPECT_EQ (var.getIf <0> (), nullptr);
    EXPECT_EQ (var.getIf <1> (), nullptr);
    EXPECT_NE (var.getIf <2> (), nullptr);
    EXPECT_EQ (var.getIf <3> (), nullptr);
    EXPECT_EQ (var.getIf <4> (), nullptr);

    const std::string constString ("foobar2");
    var.set <std::string> (std::string (constString));
    EXPECT_EQ (var.getIf <int> (), nullptr);
    EXPECT_EQ (var.getIf <double> (), nullptr);
    EXPECT_NE (var.getIf <std::string> (), nullptr);
    EXPECT_EQ (var.getIf <bool> (), nullptr);
    EXPECT_EQ (var.getIf <std::nullptr_t> (), nullptr);
    EXPECT_EQ (var.getIf <0> (), nullptr);
    EXPECT_EQ (var.getIf <1> (), nullptr);
    EXPECT_NE (var.getIf <2> (), nullptr);
    EXPECT_EQ (var.getIf <3> (), nullptr);
    EXPECT_EQ (var.getIf <4> (), nullptr);

    var.set <bool> (false);
    EXPECT_EQ (var.getIf <int> (), nullptr);
    EXPECT_EQ (var.getIf <double> (), nullptr);
    EXPECT_EQ (var.getIf <std::string> (), nullptr);
    EXPECT_NE (var.getIf <bool> (), nullptr);
    EXPECT_EQ (var.getIf <std::nullptr_t> (), nullptr);
    EXPECT_EQ (var.getIf <0> (), nullptr);
    EXPECT_EQ (var.getIf <1> (), nullptr);
    EXPECT_EQ (var.getIf <2> (), nullptr);
    EXPECT_NE (var.getIf <3> (), nullptr);
    EXPECT_EQ (var.getIf <4> (), nullptr);

    var.set <std::nullptr_t> (nullptr);
    EXPECT_EQ (var.getIf <int> (), nullptr);
    EXPECT_EQ (var.getIf <double> (), nullptr);
    EXPECT_EQ (var.getIf <std::string> (), nullptr);
    EXPECT_EQ (var.getIf <bool> (), nullptr);
    EXPECT_NE (var.getIf <std::nullptr_t> (), nullptr);
    EXPECT_EQ (var.getIf <0> (), nullptr);
    EXPECT_EQ (var.getIf <1> (), nullptr);
    EXPECT_EQ (var.getIf <2> (), nullptr);
    EXPECT_EQ (var.getIf <3> (), nullptr);
    EXPECT_NE (var.getIf <4> (), nullptr);
}

/**
 * @brief Test set method.
 */
TEST (Variant, set)
{
    Variant <int, double, std::string, bool, std::nullptr_t> var;

    var.set <int> (6);
    ASSERT_TRUE (var.is <int> ());
    EXPECT_EQ (var.get <int> (), 6);

    var.set <0> (8);
    ASSERT_TRUE (var.is <int> ());
    EXPECT_EQ (var.get <int> (), 8);

    volatile int volatileInt (4);
    var.set <int> (volatileInt);
    ASSERT_TRUE (var.is <int> ());
    EXPECT_EQ (var.get <int> (), 4);

    var.set <double> (0.5);
    ASSERT_TRUE (var.is <double> ());
    EXPECT_DOUBLE_EQ (var.get <double> (), 0.5);

    var.set <1> (0.7);
    ASSERT_TRUE (var.is <double> ());
    EXPECT_DOUBLE_EQ (var.get <double> (), 0.7);

    var.set <std::string> (std::string ("foobar1"));
    ASSERT_TRUE (var.is <std::string> ());
    EXPECT_STREQ (var.get <std::string> ().c_str (), "foobar1");

    var.set <2> (std::string ("barfoo1"));
    ASSERT_TRUE (var.is <std::string> ());
    EXPECT_STREQ (var.get <std::string> ().c_str (), "barfoo1");

    const std::string constString ("foobar2");
    var.set <std::string> (constString);
    ASSERT_TRUE (var.is <std::string> ());
    EXPECT_STREQ (var.get <std::string> ().c_str (), "foobar2");

    var.set <bool> (true);
    ASSERT_TRUE (var.is <bool> ());
    EXPECT_TRUE (var.get <bool> ());

    var.set <3> (false);
    ASSERT_TRUE (var.is <bool> ());
    EXPECT_FALSE (var.get <bool> ());

    var.set <std::nullptr_t> (nullptr);
    ASSERT_TRUE (var.is <std::nullptr_t> ());
    EXPECT_EQ (var.get <std::nullptr_t> (), nullptr);

    var.set <4> (nullptr);
    ASSERT_TRUE (var.is <std::nullptr_t> ());
    EXPECT_EQ (var.get <std::nullptr_t> (), nullptr);
}

/**
 * @brief Test is method.
 */
TEST (Variant, is)
{
    Variant <int, double, std::string, bool, std::nullptr_t> var;

    var.set <int> (6);
    EXPECT_TRUE (var.is <int> ());
    EXPECT_FALSE (var.is <double> ());
    EXPECT_FALSE (var.is <std::string> ());
    EXPECT_FALSE (var.is <bool> ());
    EXPECT_FALSE (var.is <std::nullptr_t> ());
    EXPECT_TRUE (var.is <0> ());
    EXPECT_FALSE (var.is <1> ());
    EXPECT_FALSE (var.is <2> ());
    EXPECT_FALSE (var.is <3> ());
    EXPECT_FALSE (var.is <4> ());

    volatile int volatileInt (4);
    var.set <int> (volatileInt);
    EXPECT_TRUE (var.is <int> ());
    EXPECT_FALSE (var.is <double> ());
    EXPECT_FALSE (var.is <std::string> ());
    EXPECT_FALSE (var.is <bool> ());
    EXPECT_FALSE (var.is <std::nullptr_t> ());
    EXPECT_TRUE (var.is <0> ());
    EXPECT_FALSE (var.is <1> ());
    EXPECT_FALSE (var.is <2> ());
    EXPECT_FALSE (var.is <3> ());
    EXPECT_FALSE (var.is <4> ());

    var.set <double> (0.5);
    EXPECT_FALSE (var.is <int> ());
    EXPECT_TRUE (var.is <double> ());
    EXPECT_FALSE (var.is <std::string> ());
    EXPECT_FALSE (var.is <bool> ());
    EXPECT_FALSE (var.is <std::nullptr_t> ());
    EXPECT_FALSE (var.is <0> ());
    EXPECT_TRUE (var.is <1> ());
    EXPECT_FALSE (var.is <2> ());
    EXPECT_FALSE (var.is <3> ());
    EXPECT_FALSE (var.is <4> ());

    var.set <std::string> (std::string ("foobar"));
    EXPECT_FALSE (var.is <int> ());
    EXPECT_FALSE (var.is <double> ());
    EXPECT_TRUE (var.is <std::string> ());
    EXPECT_FALSE (var.is <bool> ());
    EXPECT_FALSE (var.is <std::nullptr_t> ());
    EXPECT_FALSE (var.is <0> ());
    EXPECT_FALSE (var.is <1> ());
    EXPECT_TRUE (var.is <2> ());
    EXPECT_FALSE (var.is <3> ());
    EXPECT_FALSE (var.is <4> ());

    const std::string constString ("foobar2");
    var.set <std::string> (constString);
    EXPECT_FALSE (var.is <int> ());
    EXPECT_FALSE (var.is <double> ());
    EXPECT_TRUE (var.is <std::string> ());
    EXPECT_FALSE (var.is <bool> ());
    EXPECT_FALSE (var.is <std::nullptr_t> ());
    EXPECT_FALSE (var.is <0> ());
    EXPECT_FALSE (var.is <1> ());
    EXPECT_TRUE (var.is <2> ());
    EXPECT_FALSE (var.is <3> ());
    EXPECT_FALSE (var.is <4> ());

    var.set <bool> (true);
    EXPECT_FALSE (var.is <int> ());
    EXPECT_FALSE (var.is <double> ());
    EXPECT_FALSE (var.is <std::string> ());
    EXPECT_TRUE (var.is <bool> ());
    EXPECT_FALSE (var.is <std::nullptr_t> ());
    EXPECT_FALSE (var.is <0> ());
    EXPECT_FALSE (var.is <1> ());
    EXPECT_FALSE (var.is <2> ());
    EXPECT_TRUE (var.is <3> ());
    EXPECT_FALSE (var.is <4> ());

    var.set <std::nullptr_t> (nullptr);
    EXPECT_FALSE (var.is <int> ());
    EXPECT_FALSE (var.is <double> ());
    EXPECT_FALSE (var.is <std::string> ());
    EXPECT_FALSE (var.is <bool> ());
    EXPECT_TRUE (var.is <std::nullptr_t> ());
    EXPECT_FALSE (var.is <0> ());
    EXPECT_FALSE (var.is <1> ());
    EXPECT_FALSE (var.is <2> ());
    EXPECT_FALSE (var.is <3> ());
    EXPECT_TRUE (var.is <4> ());
}

/**
 * @brief Test index method.
 */
TEST (Variant, index)
{
    Variant <int, double, std::string, bool, std::nullptr_t> var;

    var.set <int> (120);
    EXPECT_EQ (var.index(), 0);
    EXPECT_NE (var.index(), 1);
    EXPECT_NE (var.index(), 2);
    EXPECT_NE (var.index(), 3);
    EXPECT_NE (var.index(), 4);

    volatile int volatileInt (4);
    var.set <int> (volatileInt);
    EXPECT_EQ (var.index(), 0);
    EXPECT_NE (var.index(), 1);
    EXPECT_NE (var.index(), 2);
    EXPECT_NE (var.index(), 3);
    EXPECT_NE (var.index(), 4);

    var.set <double> (0.5);
    EXPECT_NE (var.index(), 0);
    EXPECT_EQ (var.index(), 1);
    EXPECT_NE (var.index(), 2);
    EXPECT_NE (var.index(), 3);
    EXPECT_NE (var.index(), 4);

    var.set <std::string> (std::string ("foobar"));
    EXPECT_NE (var.index(), 0);
    EXPECT_NE (var.index(), 1);
    EXPECT_EQ (var.index(), 2);
    EXPECT_NE (var.index(), 3);
    EXPECT_NE (var.index(), 4);

    const std::string constString ("foobar");
    var.set <std::string> (constString);
    EXPECT_NE (var.index(), 0);
    EXPECT_NE (var.index(), 1);
    EXPECT_EQ (var.index(), 2);
    EXPECT_NE (var.index(), 3);
    EXPECT_NE (var.index(), 4);

    var.set <bool> (true);
    EXPECT_NE (var.index(), 0);
    EXPECT_NE (var.index(), 1);
    EXPECT_NE (var.index(), 2);
    EXPECT_EQ (var.index(), 3);
    EXPECT_NE (var.index(), 4);

    var.set <std::nullptr_t> (nullptr);
    EXPECT_NE (var.index(), 0);
    EXPECT_NE (var.index(), 1);
    EXPECT_NE (var.index(), 2);
    EXPECT_NE (var.index(), 3);
    EXPECT_EQ (var.index(), 4);
}

/**
 * @brief Test equal operator.
 */
TEST (Variant, equal)
{
    Variant <int, double, std::string, bool, std::nullptr_t> var1, var2, var3;

    var1.set <int> (3);
    var2.set <int> (3);
    var3.set <int> (4);

    EXPECT_TRUE (var1 == var2);
    EXPECT_FALSE (var1 == var3);

    var1.set <double> (0.5);
    var2.set <double> (0.5);
    var3.set <double> (0.7);

    EXPECT_TRUE (var1 == var2);
    EXPECT_FALSE (var1 == var3);

    var1.set <std::string> ("foo");
    var2.set <std::string> ("foo");
    var3.set <std::string> ("bar");

    EXPECT_TRUE (var1 == var2);
    EXPECT_FALSE (var1 == var3);

    var1.set <bool> (true);
    var2.set <bool> (true);
    var3.set <bool> (false);

    EXPECT_TRUE (var1 == var2);
    EXPECT_FALSE (var1 == var3);

    var1.set <int> (3);
    var2.set <double> (0.5);
    var3.set <std::string> ("foo");

    EXPECT_FALSE (var1 == var2);
    EXPECT_FALSE (var1 == var3);

    var1.set <bool> (true);
    var2.set <std::nullptr_t> (nullptr);
    var3.set <int> (8);

    EXPECT_FALSE (var1 == var2);
    EXPECT_FALSE (var1 == var3);
}

/**
 * @brief Test not_equal operator.
 */
TEST (Variant, not_equal)
{
    Variant <int, double, std::string, bool, std::nullptr_t> var1, var2, var3;

    var1.set <int> (3);
    var2.set <int> (3);
    var3.set <int> (4);

    EXPECT_FALSE (var1 != var2);
    EXPECT_TRUE (var1 != var3);

    var1.set <double> (0.5);
    var2.set <double> (0.5);
    var3.set <double> (0.7);

    EXPECT_FALSE (var1 != var2);
    EXPECT_TRUE (var1 != var3);

    var1.set <std::string> ("foo");
    var2.set <std::string> ("foo");
    var3.set <std::string> ("bar");

    EXPECT_FALSE (var1 != var2);
    EXPECT_TRUE (var1 != var3);

    var1.set <bool> (true);
    var2.set <bool> (true);
    var3.set <bool> (false);

    EXPECT_FALSE (var1 != var2);
    EXPECT_TRUE (var1 != var3);

    var1.set <int> (3);
    var2.set <double> (0.5);
    var3.set <std::string> ("foo");

    EXPECT_TRUE (var1 != var2);
    EXPECT_TRUE (var1 != var3);

    var1.set <bool> (true);
    var2.set <std::nullptr_t> (nullptr);
    var3.set <int> (8);

    EXPECT_TRUE (var1 != var2);
    EXPECT_TRUE (var1 != var3);
}

/**
 * @brief Test less operator.
 */
TEST (Variant, less)
{
    Variant <int, double, std::string, bool, std::nullptr_t> var1, var2, var3;

    var1.set <int> (3);
    var2.set <int> (3);
    var3.set <int> (4);

    EXPECT_FALSE (var1 < var2);
    EXPECT_TRUE (var1 < var3);

    var1.set <double> (0.5);
    var2.set <double> (0.5);
    var3.set <double> (0.7);

    EXPECT_FALSE (var1 < var2);
    EXPECT_TRUE (var1 < var3);

    var1.set <std::string> ("bar");
    var2.set <std::string> ("bar");
    var3.set <std::string> ("foo");

    EXPECT_FALSE (var1 < var2);
    EXPECT_TRUE (var1 < var3);

    var1.set <bool> (false);
    var2.set <bool> (false);
    var3.set <bool> (true);

    EXPECT_FALSE (var1 < var2);
    EXPECT_TRUE (var1 < var3);

    var1.set <std::nullptr_t> (nullptr);
    var2.set <std::nullptr_t> (nullptr);
    var3.set <std::nullptr_t> (nullptr);

    EXPECT_FALSE (var1 < var2);
    EXPECT_FALSE (var1 < var3);

    var1.set <int> (3);
    var2.set <double> (0.5);
    var3.set <std::string> ("foo");

    EXPECT_TRUE (var1 < var2);
    EXPECT_TRUE (var1 < var3);

    var1.set <bool> (true);
    var2.set <std::nullptr_t> (nullptr);
    var3.set <int> (8);

    EXPECT_TRUE (var1 < var2);
    EXPECT_FALSE (var1 < var3);
}

/**
 * @brief Test greater operator.
 */
TEST (Variant, greater)
{
    Variant <int, double, std::string, bool, std::nullptr_t> var1, var2, var3;

    var1.set <int> (3);
    var2.set <int> (3);
    var3.set <int> (4);

    EXPECT_FALSE (var1 > var2);
    EXPECT_TRUE (var3 > var1);

    var1.set <double> (0.5);
    var2.set <double> (0.5);
    var3.set <double> (0.7);

    EXPECT_FALSE (var1 > var2);
    EXPECT_TRUE (var3 > var1);

    var1.set <std::string> ("bar");
    var2.set <std::string> ("bar");
    var3.set <std::string> ("foo");

    EXPECT_FALSE (var1 > var2);
    EXPECT_TRUE (var3 > var1);

    var1.set <bool> (false);
    var2.set <bool> (false);
    var3.set <bool> (true);

    EXPECT_FALSE (var1 > var2);
    EXPECT_TRUE (var3 > var1);

    var1.set <std::nullptr_t> (nullptr);
    var2.set <std::nullptr_t> (nullptr);
    var3.set <std::nullptr_t> (nullptr);

    EXPECT_FALSE (var1 > var2);
    EXPECT_FALSE (var1 > var3);

    var1.set <int> (3);
    var2.set <double> (0.5);
    var3.set <std::string> ("foo");

    EXPECT_FALSE (var1 > var2);
    EXPECT_FALSE (var1 > var3);

    var1.set <bool> (true);
    var2.set <std::nullptr_t> (nullptr);
    var3.set <int> (8);

    EXPECT_FALSE (var1 > var2);
    EXPECT_TRUE (var1 > var3);
}

/**
 * @brief Test less_or_equal operator.
 */
TEST (Variant, less_or_equal)
{
    Variant <int, double, std::string, bool, std::nullptr_t> var1, var2, var3;

    var1.set <int> (3);
    var2.set <int> (3);
    var3.set <int> (4);

    EXPECT_TRUE (var1 <= var2);
    EXPECT_TRUE (var1 <= var3);

    var1.set <double> (0.5);
    var2.set <double> (0.5);
    var3.set <double> (0.7);

    EXPECT_TRUE (var1 <= var2);
    EXPECT_TRUE (var1 <= var3);

    var1.set <std::string> ("bar");
    var2.set <std::string> ("bar");
    var3.set <std::string> ("foo");

    EXPECT_TRUE (var1 <= var2);
    EXPECT_TRUE (var1 <= var3);

    var1.set <bool> (false);
    var2.set <bool> (false);
    var3.set <bool> (true);

    EXPECT_TRUE (var1 <= var2);
    EXPECT_TRUE (var1 <= var3);

    var1.set <std::nullptr_t> (nullptr);
    var2.set <std::nullptr_t> (nullptr);
    var3.set <std::nullptr_t> (nullptr);

    EXPECT_TRUE (var1 <= var2);
    EXPECT_TRUE (var1 <= var3);

    var1.set <int> (3);
    var2.set <double> (0.5);
    var3.set <std::string> ("foo");

    EXPECT_TRUE (var1 <= var2);
    EXPECT_TRUE (var1 <= var3);

    var1.set <bool> (true);
    var2.set <std::nullptr_t> (nullptr);
    var3.set <int> (8);

    EXPECT_TRUE (var1 <= var2);
    EXPECT_FALSE (var1 <= var3);
}

/**
 * @brief Test greater_or_equal operator.
 */
TEST (Variant, greater_or_equal)
{
    Variant <int, double, std::string, bool, std::nullptr_t> var1, var2, var3;

    var1.set <int> (3);
    var2.set <int> (3);
    var3.set <int> (4);

    EXPECT_TRUE (var1 >= var2);
    EXPECT_TRUE (var3 >= var1);

    var1.set <double> (0.5);
    var2.set <double> (0.5);
    var3.set <double> (0.7);

    EXPECT_TRUE (var1 >= var2);
    EXPECT_TRUE (var3 >= var1);

    var1.set <std::string> ("bar");
    var2.set <std::string> ("bar");
    var3.set <std::string> ("foo");

    EXPECT_TRUE (var1 >= var2);
    EXPECT_TRUE (var3 >= var1);

    var1.set <bool> (false);
    var2.set <bool> (false);
    var3.set <bool> (true);

    EXPECT_TRUE (var1 >= var2);
    EXPECT_TRUE (var3 >= var1);

    var1.set <std::nullptr_t> (nullptr);
    var2.set <std::nullptr_t> (nullptr);
    var3.set <std::nullptr_t> (nullptr);

    EXPECT_TRUE (var1 >= var2);
    EXPECT_TRUE (var1 >= var3);

    var1.set <int> (3);
    var2.set <double> (0.5);
    var3.set <std::string> ("foo");

    EXPECT_FALSE (var1 >= var2);
    EXPECT_FALSE (var1 >= var3);

    var1.set <bool> (true);
    var2.set <std::nullptr_t> (nullptr);
    var3.set <int> (8);

    EXPECT_FALSE (var1 >= var2);
    EXPECT_TRUE (var1 >= var3);
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
