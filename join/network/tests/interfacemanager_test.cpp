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
 * @brief test the findByIndex method.
 */
TEST (InterfaceManager, findByIndex)
{
    std::string loopback ("lo");

    auto lo = InterfaceManager::instance ()->findByIndex (if_nametoindex (loopback.c_str ()));
    ASSERT_NE (lo, nullptr);
}

/**
 * @brief test the findByName method.
 */
TEST (InterfaceManager, findByName)
{
    std::string loopback ("lo");

    auto lo = InterfaceManager::instance ()->findByName (loopback);
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

    EXPECT_EQ (InterfaceManager::instance ()->refresh (true), 0) << lastError.message ();
    EXPECT_FALSE (called);

    InterfaceManager::instance ()->addLinkListener (cb);
    EXPECT_EQ (InterfaceManager::instance ()->refresh (true), 0) << lastError.message ();
    EXPECT_TRUE (called);

    InterfaceManager::instance ()->removeLinkListener (cb);
    called = false;
    EXPECT_EQ (InterfaceManager::instance ()->refresh (true), 0) << lastError.message ();
    EXPECT_FALSE (called);
}

/**
 * @brief test the addAddressListener method.
 */
TEST (InterfaceManager, addAddressListener)
{
    bool called = false;
    auto cb = [&] (const auto& /*info*/) { called = true; };

    EXPECT_EQ (InterfaceManager::instance ()->refresh (true), 0) << lastError.message ();
    EXPECT_FALSE (called);

    InterfaceManager::instance ()->addAddressListener (cb);
    EXPECT_EQ (InterfaceManager::instance ()->refresh (true), 0) << lastError.message ();
    EXPECT_TRUE (called);

    InterfaceManager::instance ()->removeAddressListener (cb);
    called = false;
    EXPECT_EQ (InterfaceManager::instance ()->refresh (true), 0) << lastError.message ();
    EXPECT_FALSE (called);
}

/**
 * @brief test the addRouteListener method.
 */
TEST (InterfaceManager, addRouteListener)
{
    bool called = false;
    auto cb = [&] (const auto& /*info*/) { called = true; };

    EXPECT_EQ (InterfaceManager::instance ()->refresh (true), 0) << lastError.message ();
    EXPECT_FALSE (called);

    InterfaceManager::instance ()->addRouteListener (cb);
    EXPECT_EQ (InterfaceManager::instance ()->refresh (true), 0) << lastError.message ();
    EXPECT_TRUE (called);

    InterfaceManager::instance ()->removeRouteListener (cb);
    called = false;
    EXPECT_EQ (InterfaceManager::instance ()->refresh (true), 0) << lastError.message ();
    EXPECT_FALSE (called);
}

/**
 * @brief test the createDummyInterface method.
 */
TEST (InterfaceManager, createDummyInterface)
{
    std::string dummy0 ("dummy0");
    InterfaceManager::instance ()->removeInterface (dummy0, true);

    ASSERT_EQ (InterfaceManager::instance ()->createDummyInterface (dummy0, true), 0) << lastError.message ();
    auto dm = InterfaceManager::instance ()->findByName (dummy0);
    ASSERT_NE (dm, nullptr);
    ASSERT_TRUE (dm->isDummy ());
    ASSERT_EQ (dm->enable (true, true), 0) << lastError.message ();
    ASSERT_EQ (dm->enable (false, true), 0) << lastError.message ();
    EXPECT_EQ (InterfaceManager::instance ()->removeInterface (dummy0, true), 0) << lastError.message ();
}

/**
 * @brief test the createBridgeInterface method.
 */
TEST (InterfaceManager, createBridgeInterface)
{
    std::string bridge0 ("br0");
    InterfaceManager::instance ()->removeInterface (bridge0, true);

    ASSERT_EQ (InterfaceManager::instance ()->createBridgeInterface (bridge0, true), 0) << lastError.message ();
    auto br = InterfaceManager::instance ()->findByName (bridge0);
    ASSERT_NE (br, nullptr);
    ASSERT_TRUE (br->isBridge ());
    ASSERT_EQ (br->enable (true, true), 0) << lastError.message ();
    ASSERT_EQ (br->enable (false, true), 0) << lastError.message ();
    EXPECT_EQ (InterfaceManager::instance ()->removeInterface (bridge0, true), 0) << lastError.message ();
}

/**
 * @brief test the createVlanInterface method.
 */
TEST (InterfaceManager, createVlanInterface)
{
    uint16_t id = 10;
    std::string dummy0 ("dummy0");
    std::string vlan10 = dummy0 + "." + std::to_string (id);
    InterfaceManager::instance ()->removeInterface (vlan10, true);
    InterfaceManager::instance ()->removeInterface (dummy0, true);

    ASSERT_EQ (InterfaceManager::instance ()->createDummyInterface (dummy0, true), 0) << lastError.message ();
    auto dm = InterfaceManager::instance ()->findByName (dummy0);
    ASSERT_NE (dm, nullptr);
    ASSERT_EQ (dm->enable (true, true), 0) << lastError.message ();

    ASSERT_EQ (InterfaceManager::instance ()->createVlanInterface (vlan10, dummy0, id, ETH_P_8021Q, true), 0) << lastError.message ();
    auto vl = InterfaceManager::instance ()->findByName (vlan10);
    ASSERT_TRUE (vl->isVlan ());
    ASSERT_EQ (vl->enable (true, true), 0) << lastError.message ();
    ASSERT_EQ (vl->enable (false, true), 0) << lastError.message ();
    EXPECT_EQ (InterfaceManager::instance ()->removeInterface (vlan10, true), 0) << lastError.message ();

    EXPECT_EQ (InterfaceManager::instance ()->removeInterface (dummy0, true), 0) << lastError.message ();
}

/**
 * @brief test the createVethInterface method.
 */
TEST (InterfaceManager, createVethInterface)
{
    std::string vhost ("veth2"), vpeer ("veth3");
    InterfaceManager::instance ()->removeInterface (vhost, true);

    ASSERT_EQ (InterfaceManager::instance ()->createVethInterface (vhost, vpeer, nullptr, true), 0) << lastError.message ();
    auto vh = InterfaceManager::instance ()->findByName (vhost);
    ASSERT_NE (vh, nullptr);
    ASSERT_TRUE (vh->isVeth ());
    auto vp = InterfaceManager::instance ()->findByName (vpeer);
    ASSERT_NE (vp, nullptr);
    ASSERT_TRUE (vp->isVeth ());
    ASSERT_EQ (vh->enable (true, true), 0) << lastError.message ();
    ASSERT_EQ (vp->enable (true, true), 0) << lastError.message ();
    ASSERT_EQ (vh->enable (false, true), 0) << lastError.message ();
    ASSERT_EQ (vp->enable (false, true), 0) << lastError.message ();
    EXPECT_EQ (InterfaceManager::instance ()->removeInterface (vhost, true), 0) << lastError.message ();
}

/**
 * @brief test the createGreInterface method.
 */
TEST (InterfaceManager, createGreInterface)
{
    std::string dummy0 ("dummy0"), gre4 ("gre4"), gre6 ("gre6");
    InterfaceManager::instance ()->removeInterface (gre4, true);
    InterfaceManager::instance ()->removeInterface (gre6, true);
    InterfaceManager::instance ()->removeInterface (dummy0, true);

    ASSERT_EQ (InterfaceManager::instance ()->createDummyInterface (dummy0, true), 0) << lastError.message ();
    auto dm = InterfaceManager::instance ()->findByName (dummy0);
    ASSERT_NE (dm, nullptr);
    ASSERT_EQ (dm->enable (true, true), 0) << lastError.message ();

    ASSERT_EQ (InterfaceManager::instance ()->createGreInterface (gre4, dummy0, "0.0.0.0", "172.217.22.142", nullptr, nullptr, 64, true), 0) << lastError.message ();
    auto gr = InterfaceManager::instance ()->findByName (gre4);
    ASSERT_NE (gr, nullptr);
    ASSERT_TRUE (gr->isGre ());
    ASSERT_EQ (gr->enable (true, true), 0) << lastError.message ();
    ASSERT_EQ (gr->enable (false, true), 0) << lastError.message ();
    EXPECT_EQ (InterfaceManager::instance ()->removeInterface (gre4, true), 0) << lastError.message ();

    ASSERT_EQ (InterfaceManager::instance ()->createGreInterface (gre6, dummy0, "::", "2a00:1450:4007:811::200e", nullptr, nullptr, 64, true), 0) << lastError.message ();
    gr = InterfaceManager::instance ()->findByName (gre6);
    ASSERT_NE (gr, nullptr);
    ASSERT_TRUE (gr->isGre ());
    ASSERT_EQ (gr->enable (true, true), 0) << lastError.message ();
    ASSERT_EQ (gr->enable (false, true), 0) << lastError.message ();
    EXPECT_EQ (InterfaceManager::instance ()->removeInterface (gre6, true), 0) << lastError.message ();

    EXPECT_EQ (InterfaceManager::instance ()->removeInterface (dummy0, true), 0) << lastError.message ();
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
