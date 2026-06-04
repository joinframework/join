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
#include <join/routemanager.hpp>
#include <join/error.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::lastError;
using join::Route;
using join::RouteManager;
using join::IpAddress;

/**
 * @brief class used to test the route manager API.
 */
class RouteManagerTest : public ::testing::Test
{
public:
    /**
     * @brief set up test case.
     */
    static void SetUpTestCase ()
    {
        [[maybe_unused]] int result;

        result = std::system ("ip link add dummy0 type dummy");
        result = std::system ("ip addr add 192.168.100.1/24 brd 192.168.100.255 dev dummy0");
        result = std::system ("ip -6 addr add 2001:db8::1/64 dev dummy0");
        result = std::system ("ip link set dummy0 up");

        result = std::system ("ip link add veth0 type veth peer name veth1");
        result = std::system ("ip addr add 192.168.200.1/24 brd 192.168.200.255 dev veth0");
        result = std::system ("ip link set veth0 up");
        result = std::system ("ip link set veth1 up");
    }

    /**
     * @brief tear down test case.
     */
    static void TearDownTestCase ()
    {
        [[maybe_unused]] int result;

        result = std::system ("ip link set dummy0 down");
        result = std::system ("ip link del dummy0");

        result = std::system ("ip link set veth0 down");
        result = std::system ("ip link del veth0");
    }
};

/**
 * @brief test the findByIndex method.
 */
TEST_F (RouteManagerTest, findByIndex)
{
    RouteManager mgr;

    auto foo = mgr.findByIndex (50000, "10.0.0.0", 8);
    ASSERT_EQ (foo, nullptr);

    auto routes = mgr.enumerate ();
    ASSERT_GT (routes.size (), 0);

    auto r = mgr.findByIndex (routes.front ()->index (), routes.front ()->dest (), routes.front ()->prefix ());
    ASSERT_NE (r, nullptr);
}

/**
 * @brief test the findByName method.
 */
TEST_F (RouteManagerTest, findByName)
{
    RouteManager mgr;

    auto foo = mgr.findByName ("foo", "10.0.0.0", 8);
    ASSERT_EQ (foo, nullptr);

    auto routes = mgr.enumerate ();
    ASSERT_GT (routes.size (), 0);

    auto r = routes.front ();
    char ifname[IF_NAMESIZE];
    if_indextoname (r->index (), ifname);
    auto found = mgr.findByName (ifname, r->dest (), r->prefix ());
    ASSERT_NE (found, nullptr);
}

/**
 * @brief test the enumerate method.
 */
TEST_F (RouteManagerTest, enumerate)
{
    RouteManager mgr;

    auto routes = mgr.enumerate ();
    ASSERT_GT (routes.size (), 0);

    uint32_t idx = if_nametoindex ("dummy0");
    auto byIface = mgr.enumerate (idx);
    ASSERT_GT (byIface.size (), 0);

    auto byName = mgr.enumerate ("dummy0");
    ASSERT_EQ (byName.size (), byIface.size ());
}

/**
 * @brief test the addRouteListener method.
 */
TEST_F (RouteManagerTest, addRouteListener)
{
    RouteManager mgr;

    bool called = false;
    auto cb = [&] (const auto& /*info*/) {
        called = true;
    };

    EXPECT_EQ (mgr.refresh (), 0) << lastError.message ();
    EXPECT_FALSE (called);

    auto id = mgr.addRouteListener (cb);
    EXPECT_EQ (mgr.refresh (), 0) << lastError.message ();
    EXPECT_TRUE (called);

    mgr.removeRouteListener (id);
    called = false;
    EXPECT_EQ (mgr.refresh (), 0) << lastError.message ();
    EXPECT_FALSE (called);
}

/**
 * @brief test the addRoute method.
 */
TEST_F (RouteManagerTest, addRoute)
{
    RouteManager mgr;

    uint32_t idx = if_nametoindex ("veth0");

    auto r = mgr.findByIndex (idx, "192.168.200.0", 24);
    ASSERT_NE (r, nullptr);
    ASSERT_TRUE (r->isUnicast ());
    ASSERT_TRUE (r->isScopeLink ());
    ASSERT_TRUE (r->isKernel ());

    ASSERT_EQ (mgr.findByIndex (idx, "10.10.0.0", 16), nullptr);
    ASSERT_EQ (mgr.addRoute (idx, "10.10.0.0", 16, "192.168.200.254", 0, true), 0) << lastError.message ();
    r = mgr.findByIndex (idx, "10.10.0.0", 16);
    ASSERT_NE (r, nullptr);
    ASSERT_TRUE (r->isUnicast ());
    ASSERT_TRUE (r->isScopeUniverse ());
    ASSERT_TRUE (r->isStatic ());
    ASSERT_EQ (r->gateway (), IpAddress ("192.168.200.254"));
    ASSERT_EQ (mgr.removeRoute (idx, "10.10.0.0", 16, true), 0) << lastError.message ();
    ASSERT_EQ (mgr.findByIndex (idx, "10.10.0.0", 16), nullptr);
}

/**
 * @brief test the addRoute by name method.
 */
TEST_F (RouteManagerTest, addRouteByName)
{
    RouteManager mgr;

    ASSERT_EQ (mgr.findByName ("veth0", "172.16.0.0", 12), nullptr);
    ASSERT_EQ (mgr.addRoute ("veth0", "172.16.0.0", 12, "192.168.200.254", 0, true), 0) << lastError.message ();
    ASSERT_NE (mgr.findByName ("veth0", "172.16.0.0", 12), nullptr);
    ASSERT_EQ (mgr.removeRoute ("veth0", "172.16.0.0", 12, true), 0) << lastError.message ();
    ASSERT_EQ (mgr.findByName ("veth0", "172.16.0.0", 12), nullptr);
}

/**
 * @brief test the flushRoutes method.
 */
TEST_F (RouteManagerTest, flushRoutes)
{
    RouteManager mgr;

    uint32_t idx = if_nametoindex ("dummy0");

    ASSERT_EQ (mgr.addRoute (idx, "10.20.0.0", 16, "192.168.100.254", 0, true), 0) << lastError.message ();
    ASSERT_EQ (mgr.addRoute (idx, "10.30.0.0", 16, "192.168.100.254", 0, true), 0) << lastError.message ();
    ASSERT_NE (mgr.findByIndex (idx, "10.20.0.0", 16), nullptr);
    ASSERT_NE (mgr.findByIndex (idx, "10.30.0.0", 16), nullptr);

    ASSERT_EQ (mgr.flushRoutes (idx, true), 0) << lastError.message ();

    ASSERT_EQ (mgr.findByIndex (idx, "10.20.0.0", 16), nullptr);
    ASSERT_EQ (mgr.findByIndex (idx, "10.30.0.0", 16), nullptr);
}

/**
 * @brief test the Route::set method.
 */
TEST_F (RouteManagerTest, routeSet)
{
    RouteManager mgr;

    uint32_t idx = if_nametoindex ("veth0");

    ASSERT_EQ (mgr.addRoute (idx, "10.40.0.0", 16, "192.168.200.254", 0, true), 0) << lastError.message ();
    auto r = mgr.findByIndex (idx, "10.40.0.0", 16);
    ASSERT_NE (r, nullptr);
    ASSERT_EQ (r->gateway (), IpAddress ("192.168.200.254"));

    ASSERT_EQ (r->set ("192.168.200.1", 100, true), 0) << lastError.message ();
    r = mgr.findByIndex (idx, "10.40.0.0", 16);
    ASSERT_NE (r, nullptr);
    ASSERT_EQ (r->gateway (), IpAddress ("192.168.200.1"));
    ASSERT_EQ (r->metric (), 100);

    ASSERT_EQ (r->remove (true), 0) << lastError.message ();
    ASSERT_EQ (mgr.findByIndex (idx, "10.40.0.0", 16), nullptr);
}

/**
 * @brief test the Route type methods.
 */
TEST_F (RouteManagerTest, routeType)
{
    RouteManager mgr;

    auto routes = mgr.enumerate ();
    ASSERT_GT (routes.size (), 0);

    for (auto& r : routes)
    {
        // every route is exactly one type
        int count = (r->isUnicast () ? 1 : 0) + (r->isBlackhole () ? 1 : 0) + (r->isUnreachable () ? 1 : 0) +
                    (r->isProhibit () ? 1 : 0) + (r->isLocal () ? 1 : 0);
        ASSERT_EQ (count, 1);
    }
}

/**
 * @brief test the Route scope methods.
 */
TEST_F (RouteManagerTest, routeScope)
{
    RouteManager mgr;

    auto routes = mgr.enumerate ();
    ASSERT_GT (routes.size (), 0);

    for (auto& r : routes)
    {
        // every route has exactly one scope
        int count = (r->isScopeUniverse () ? 1 : 0) + (r->isScopeLink () ? 1 : 0) + (r->isScopeHost () ? 1 : 0);
        ASSERT_GE (count, 1);
    }
}

/**
 * @brief test the compare method.
 */
TEST_F (RouteManagerTest, compare)
{
    RouteManager mgr;
    Route::Ptr nil;

    auto routes = mgr.enumerate ();
    ASSERT_GT (routes.size (), 0);

    auto r = routes.front ();
    ASSERT_TRUE (nil == nil);
    ASSERT_TRUE (r == r);
    ASSERT_FALSE (r == nil);
    ASSERT_FALSE (nil == r);
    ASSERT_FALSE (r < r);
    ASSERT_FALSE (r < nil);
    ASSERT_TRUE (nil < r);
}

/**
 * @brief test the singleton method.
 */
TEST_F (RouteManagerTest, instance)
{
    auto& r1 = RouteManager::instance ();
    auto& r2 = RouteManager::instance ();
    ASSERT_EQ (&r1, &r2);
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
