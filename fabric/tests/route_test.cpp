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
 * @brief class used to test the Route API.
 */
class RouteTest : public ::testing::Test
{
public:
    /**
     * @brief set up test case.
     */
    static void SetUpTestCase ()
    {
        [[maybe_unused]] int result;

        result = std::system ("ip link add veth0 type veth peer name veth1");
        result = std::system ("ip addr add 192.168.100.1/24 brd 192.168.100.255 dev veth0");
        result = std::system ("ip link set veth0 up");
        result = std::system ("ip link set veth1 up");

        result = std::system ("ip link add dummy0 type dummy");
        result = std::system ("ip addr add 192.168.200.1/24 brd 192.168.200.255 dev dummy0");
        result = std::system ("ip -6 addr add 2001:db8::1/64 dev dummy0");
        result = std::system ("ip link set dummy0 up");
    }

    /**
     * @brief tear down test case.
     */
    static void TearDownTestCase ()
    {
        [[maybe_unused]] int result;

        result = std::system ("ip link set veth0 down");
        result = std::system ("ip link del veth0");

        result = std::system ("ip link set dummy0 down");
        result = std::system ("ip link del dummy0");
    }
};

/**
 * @brief test the index method.
 */
TEST_F (RouteTest, index)
{
    RouteManager mgr;

    auto routes = mgr.enumerate ("veth0");
    ASSERT_FALSE (routes.empty ());

    for (auto& r : routes)
    {
        ASSERT_EQ (r->index (), if_nametoindex ("veth0"));
    }
}

/**
 * @brief test the dest method.
 */
TEST_F (RouteTest, dest)
{
    RouteManager mgr;

    uint32_t idx = if_nametoindex ("veth0");
    auto r = mgr.findByIndex (idx, "192.168.100.0", 24);
    ASSERT_NE (r, nullptr);
    ASSERT_EQ (r->dest (), IpAddress ("192.168.100.0"));
}

/**
 * @brief test the prefix method.
 */
TEST_F (RouteTest, prefix)
{
    RouteManager mgr;

    uint32_t idx = if_nametoindex ("veth0");
    auto r = mgr.findByIndex (idx, "192.168.100.0", 24);
    ASSERT_NE (r, nullptr);
    ASSERT_EQ (r->prefix (), 24);
}

/**
 * @brief test the gateway method.
 */
TEST_F (RouteTest, gateway)
{
    RouteManager mgr;

    uint32_t idx = if_nametoindex ("veth0");

    ASSERT_EQ (mgr.addRoute (idx, "10.10.0.0", 16, "192.168.100.254", 0, true), 0) << lastError.message ();
    auto r = mgr.findByIndex (idx, "10.10.0.0", 16);
    ASSERT_NE (r, nullptr);
    ASSERT_EQ (r->gateway (), IpAddress ("192.168.100.254"));
    ASSERT_EQ (mgr.removeRoute (idx, "10.10.0.0", 16, true), 0) << lastError.message ();
}

/**
 * @brief test the metric method.
 */
TEST_F (RouteTest, metric)
{
    RouteManager mgr;

    uint32_t idx = if_nametoindex ("veth0");

    ASSERT_EQ (mgr.addRoute (idx, "10.20.0.0", 16, "192.168.100.254", 100, true), 0) << lastError.message ();
    auto r = mgr.findByIndex (idx, "10.20.0.0", 16);
    ASSERT_NE (r, nullptr);
    ASSERT_EQ (r->metric (), 100);
    ASSERT_EQ (mgr.removeRoute (idx, "10.20.0.0", 16, true), 0) << lastError.message ();
}

/**
 * @brief test the type methods.
 */
TEST_F (RouteTest, type)
{
    RouteManager mgr;

    uint32_t idx = if_nametoindex ("veth0");

    // link-scope route added by kernel when interface comes up — RTN_UNICAST
    auto r = mgr.findByIndex (idx, "192.168.100.0", 24);
    ASSERT_NE (r, nullptr);
    ASSERT_TRUE (r->isUnicast ());
    ASSERT_FALSE (r->isBlackhole ());
    ASSERT_FALSE (r->isUnreachable ());
    ASSERT_FALSE (r->isProhibit ());
    ASSERT_FALSE (r->isLocal ());
}

/**
 * @brief test the scope methods.
 */
TEST_F (RouteTest, scope)
{
    RouteManager mgr;

    uint32_t idx = if_nametoindex ("veth0");

    // link-scope route
    auto link = mgr.findByIndex (idx, "192.168.100.0", 24);
    ASSERT_NE (link, nullptr);
    ASSERT_TRUE (link->isScopeLink ());
    ASSERT_FALSE (link->isScopeUniverse ());
    ASSERT_FALSE (link->isScopeHost ());

    // universe-scope route (via gateway)
    ASSERT_EQ (mgr.addRoute (idx, "10.30.0.0", 16, "192.168.100.254", 0, true), 0) << lastError.message ();
    auto universe = mgr.findByIndex (idx, "10.30.0.0", 16);
    ASSERT_NE (universe, nullptr);
    ASSERT_TRUE (universe->isScopeUniverse ());
    ASSERT_FALSE (universe->isScopeLink ());
    ASSERT_FALSE (universe->isScopeHost ());
    ASSERT_EQ (mgr.removeRoute (idx, "10.30.0.0", 16, true), 0) << lastError.message ();
}

/**
 * @brief test the protocol methods.
 */
TEST_F (RouteTest, protocol)
{
    RouteManager mgr;

    uint32_t idx = if_nametoindex ("veth0");

    // kernel route
    auto kernel = mgr.findByIndex (idx, "192.168.100.0", 24);
    ASSERT_NE (kernel, nullptr);
    ASSERT_TRUE (kernel->isKernel ());
    ASSERT_FALSE (kernel->isStatic ());
    ASSERT_FALSE (kernel->isDhcp ());
    ASSERT_FALSE (kernel->isBoot ());

    // static route
    ASSERT_EQ (mgr.addRoute (idx, "10.40.0.0", 16, "192.168.100.254", 0, true), 0) << lastError.message ();
    auto stat = mgr.findByIndex (idx, "10.40.0.0", 16);
    ASSERT_NE (stat, nullptr);
    ASSERT_TRUE (stat->isStatic ());
    ASSERT_FALSE (stat->isKernel ());
    ASSERT_FALSE (stat->isDhcp ());
    ASSERT_FALSE (stat->isBoot ());
    ASSERT_EQ (mgr.removeRoute (idx, "10.40.0.0", 16, true), 0) << lastError.message ();
}

/**
 * @brief test the set method.
 */
TEST_F (RouteTest, set)
{
    RouteManager mgr;

    uint32_t idx = if_nametoindex ("veth0");

    ASSERT_EQ (mgr.addRoute (idx, "10.50.0.0", 16, "192.168.100.254", 0, true), 0) << lastError.message ();
    auto r = mgr.findByIndex (idx, "10.50.0.0", 16);
    ASSERT_NE (r, nullptr);
    ASSERT_EQ (r->gateway (), IpAddress ("192.168.100.254"));
    ASSERT_EQ (r->metric (), 0);

    ASSERT_EQ (r->set ("192.168.100.1", 50, true), 0) << lastError.message ();
    r = mgr.findByIndex (idx, "10.50.0.0", 16);
    ASSERT_NE (r, nullptr);
    ASSERT_EQ (r->gateway (), IpAddress ("192.168.100.1"));
    ASSERT_EQ (r->metric (), 50);

    ASSERT_EQ (r->remove (true), 0) << lastError.message ();
    ASSERT_EQ (mgr.findByIndex (idx, "10.50.0.0", 16), nullptr);
}

/**
 * @brief test the remove method.
 */
TEST_F (RouteTest, remove)
{
    RouteManager mgr;

    uint32_t idx = if_nametoindex ("dummy0");

    ASSERT_EQ (mgr.addRoute (idx, "10.60.0.0", 16, "192.168.200.254", 0, true), 0) << lastError.message ();
    auto r = mgr.findByIndex (idx, "10.60.0.0", 16);
    ASSERT_NE (r, nullptr);

    ASSERT_EQ (r->remove (true), 0) << lastError.message ();
    ASSERT_EQ (mgr.findByIndex (idx, "10.60.0.0", 16), nullptr);
}

/**
 * @brief test the compare method.
 */
TEST_F (RouteTest, compare)
{
    RouteManager mgr;
    Route::Ptr nil;

    auto routes = mgr.enumerate ();
    ASSERT_FALSE (routes.empty ());

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
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
