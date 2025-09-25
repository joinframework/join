/**
 * MIT License
 *
 * Copyright (c) 2025 Mathieu Rabine
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
#include <join/interfacemanager.hpp>
#include <join/error.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::lastError;
using join::Interface;
using join::InterfaceManager;

/**
 * @brief test the instance method.
 */
TEST (InterfaceManager, instance)
{
    auto mgr = InterfaceManager::instance ();
    ASSERT_NE (mgr, nullptr);
}

/**
 * @brief test the dumpLink method.
 */
TEST (InterfaceManager, dumpLink)
{
    ASSERT_EQ (InterfaceManager::instance ()->dumpLink (true), 0) << lastError.message ();
}

/**
 * @brief test the dumpAddress method.
 */
TEST (InterfaceManager, dumpAddress)
{
    ASSERT_EQ (InterfaceManager::instance ()->dumpAddress (true), 0) << lastError.message ();
}

/**
 * @brief test the dumpRoute method.
 */
TEST (InterfaceManager, dumpRoute)
{
    ASSERT_EQ (InterfaceManager::instance ()->dumpRoute (true), 0) << lastError.message ();
}

/**
 * @brief test the findByIndex method.
 */
TEST (InterfaceManager, findByIndex)
{
    auto lo = InterfaceManager::instance ()->findByIndex (if_nametoindex ("lo"));
    ASSERT_NE (lo, nullptr);
}

/**
 * @brief test the findByName method.
 */
TEST (InterfaceManager, findByName)
{
    auto lo = InterfaceManager::instance ()->findByName ("lo");
    ASSERT_NE (lo, nullptr);
}

/**
 * @brief test the enumerate method.
 */
TEST (InterfaceManager, enumerate)
{
    auto addresses = InterfaceManager::instance ()->enumerate ();
    ASSERT_GT (addresses.size (), 0);
}

/**
 * @brief test the addLinkListener method.
 */
TEST (InterfaceManager, addLinkListener)
{
    bool called = false;
    auto cb = [&] (const auto& /*info*/) { called = true; };

    InterfaceManager::instance ()->addLinkListener (cb);
    EXPECT_EQ (InterfaceManager::instance ()->dumpLink (true), 0) << lastError.message ();
    EXPECT_TRUE (called);
    InterfaceManager::instance ()->removeLinkListener (cb);
}

/**
 * @brief test the addAddressListener method.
 */
TEST (InterfaceManager, addAddressListener)
{
    bool called = false;
    auto cb = [&] (const auto& /*info*/) { called = true; };

    InterfaceManager::instance ()->addAddressListener (cb);
    EXPECT_EQ (InterfaceManager::instance ()->dumpAddress (true), 0) << lastError.message ();
    EXPECT_TRUE (called);
    InterfaceManager::instance ()->removeAddressListener (cb);
}

/**
 * @brief test the addRouteListener method.
 */
TEST (InterfaceManager, addRouteListener)
{
    bool called = false;
    auto cb = [&] (const auto& /*info*/) { called = true; };

    InterfaceManager::instance ()->addRouteListener (cb);
    EXPECT_EQ (InterfaceManager::instance ()->dumpRoute (true), 0) << lastError.message ();
    EXPECT_TRUE (called);
    InterfaceManager::instance ()->removeRouteListener (cb);
}

/**
 * @brief test the createDummyInterface method.
 */
TEST (InterfaceManager, createDummyInterface)
{
    ASSERT_EQ (InterfaceManager::instance ()->createDummyInterface ("dummy0", true), 0) << lastError.message ();
    auto dm = InterfaceManager::instance ()->findByName ("dummy0");
    ASSERT_NE (dm, nullptr);
    EXPECT_EQ (InterfaceManager::instance ()->removeInterface ("dummy0", true), 0) << lastError.message ();
}

/**
 * @brief test the createPointToPointInterface method.
 */
TEST (InterfaceManager, createPointToPointInterface)
{
}

/**
 * @brief test the createBridgeInterface method.
 */
TEST (InterfaceManager, createBridgeInterface)
{
    ASSERT_EQ (InterfaceManager::instance ()->createBridgeInterface ("br0", true), 0) << lastError.message ();
    auto br = InterfaceManager::instance ()->findByName ("br0");
    ASSERT_NE (br, nullptr);
    EXPECT_EQ (InterfaceManager::instance ()->removeInterface ("br0", true), 0) << lastError.message ();
}

/**
 * @brief test the createVlanInterface method.
 */
TEST (InterfaceManager, createVlanInterface)
{
}

/**
 * @brief test the createVethInterface method.
 */
TEST (InterfaceManager, createVethInterface)
{
    // pid_t pid = getpid ();
    // ASSERT_EQ (InterfaceManager::instance ()->createVethInterface ("veth0", "veth1", &pid, true), 0) << lastError.message ();
    // auto ve = InterfaceManager::instance ()->findByName ("veth0");
    // ASSERT_NE (ve, nullptr);
    // EXPECT_EQ (InterfaceManager::instance ()->removeInterface ("veth0", true), 0) << lastError.message ();
}

/**
 * @brief test the createGreInterface method.
 */
TEST (InterfaceManager, createGreInterface)
{
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
