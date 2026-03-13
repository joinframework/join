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
#include <join/neighbormanager.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::lastError;
using join::Neighbor;
using join::NeighborManager;
using join::MacAddress;
using join::IpAddress;

/**
 * @brief Class used to test the neighbor API.
 */
class NeighborManagerTest : public ::testing::Test
{
public:
    /**
     * @brief set up test case.
     */
    static void SetUpTestCase ()
    {
        [[maybe_unused]] int result;

        result = std::system ("ip netns add red");
        result = std::system ("ip link add veth0 type veth peer name eth0 netns red");
        result = std::system ("ip link set veth0 address 4e:ed:ed:ee:59:db");
        result = std::system ("ip addr add 192.168.100.1/24 brd 192.168.100.255 dev veth0");
        result = std::system ("ip -6 addr add 2001:db8::1235/64 dev veth0");
        result = std::system ("ip link set veth0 up arp on multicast on");
        result = std::system ("ip -n red link set eth0 address 4e:ed:ed:ee:59:dc");
        result = std::system ("ip -n red addr add 192.168.100.2/24 brd 192.168.100.255 dev eth0");
        result = std::system ("ip -n red -6 addr add 2001:db8::1236/64 dev eth0");
        result = std::system ("ip -n red link set eth0 up arp on multicast on");
    }

    /**
     * @brief tear down test case.
     */
    static void TearDownTestCase ()
    {
        [[maybe_unused]] int result;

        result = std::system ("ip link set dev veth0 down");
        result = std::system ("ip link del veth0");
        result = std::system ("ip netns del red");
    }

    /**
     * @brief set up test.
     */
    void SetUp () override
    {
        [[maybe_unused]] int result;

        result = std::system ("ip neigh replace 192.168.100.2 lladdr 4e:ed:ed:ee:59:dc dev veth0 nud reachable");
        result = std::system ("ip -6 neigh replace 2001:db8::1236 lladdr 4e:ed:ed:ee:59:dc dev veth0 nud reachable");
    }

    /**
     * @brief set up test.
     */
    void TearDown () override
    {
        [[maybe_unused]] int result;

        result = std::system ("ip neigh flush dev veth0");
        result = std::system ("ip -6 neigh flush dev veth0");
    }
};

/**
 * @brief test the findByIndex method.
 */
TEST_F (NeighborManagerTest, findByIndex)
{
    NeighborManager mgr;

    auto foo = mgr.findByIndex (if_nametoindex ("foo"), "192.168.100.2");
    ASSERT_EQ (foo, nullptr);

    auto ve = mgr.findByIndex (if_nametoindex ("veth0"), "192.168.100.2");
    ASSERT_NE (ve, nullptr);

    foo = mgr.findByIndex (if_nametoindex ("foo"), "2001:db8::1236");
    ASSERT_EQ (foo, nullptr);

    ve = mgr.findByIndex (if_nametoindex ("veth0"), "2001:db8::1236");
    ASSERT_NE (ve, nullptr);
}

/**
 * @brief test the findByName method.
 */
TEST_F (NeighborManagerTest, findByName)
{
    NeighborManager mgr;

    auto foo = mgr.findByName ("foo", "192.168.100.2");
    ASSERT_EQ (foo, nullptr);

    auto ve = mgr.findByName ("veth0", "192.168.100.2");
    ASSERT_NE (ve, nullptr);

    foo = mgr.findByName ("foo", "2001:db8::1236");
    ASSERT_EQ (foo, nullptr);

    ve = mgr.findByName ("veth0", "2001:db8::1236");
    ASSERT_NE (ve, nullptr);
}

/**
 * @brief test the enumerate method.
 */
TEST_F (NeighborManagerTest, enumerate)
{
    NeighborManager mgr;

    auto neighbors = mgr.enumerate ();
    ASSERT_GT (neighbors.size (), 0);

    neighbors = mgr.enumerate (if_nametoindex ("veth0"));
    ASSERT_GT (neighbors.size (), 0);

    neighbors = mgr.enumerate (if_nametoindex ("foo"));
    ASSERT_EQ (neighbors.size (), 0);

    neighbors = mgr.enumerate ("veth0");
    ASSERT_GT (neighbors.size (), 0);

    neighbors = mgr.enumerate ("foo");
    ASSERT_EQ (neighbors.size (), 0);
}

/**
 * @brief test the addNeighborListener method.
 */
TEST_F (NeighborManagerTest, addNeighborListener)
{
    NeighborManager mgr;

    bool called = false;
    auto cb     = [&] (const auto& /*info*/) {
        called = true;
    };

    EXPECT_EQ (mgr.refresh (), 0) << lastError.message ();
    EXPECT_FALSE (called);

    auto id = mgr.addNeighborListener (cb);
    EXPECT_EQ (mgr.refresh (), 0) << lastError.message ();
    EXPECT_TRUE (called);

    mgr.removeNeighborListener (id);
    called = false;
    EXPECT_EQ (mgr.refresh (), 0) << lastError.message ();
    EXPECT_FALSE (called);
}

/**
 * @brief test the addNeighbor method.
 */
TEST_F (NeighborManagerTest, addNeighbor)
{
    NeighborManager mgr;

    ASSERT_EQ (mgr.addNeighbor (if_nametoindex ("veth0"), "192.168.100.3", "4e:ed:ed:ee:59:dd", NUD_PERMANENT, true), 0)
        << lastError.message ();
    auto ve = mgr.findByIndex (if_nametoindex ("veth0"), "192.168.100.3");
    ASSERT_NE (ve, nullptr);
    ASSERT_EQ (mgr.removeNeighbor (if_nametoindex ("veth0"), "192.168.100.3", true), 0) << lastError.message ();
    ve = mgr.findByIndex (if_nametoindex ("veth0"), "192.168.100.3");
    ASSERT_EQ (ve, nullptr);

    ASSERT_EQ (mgr.addNeighbor ("veth0", "2001:db8::1237", "4e:ed:ed:ee:59:dd", NUD_PERMANENT, true), 0)
        << lastError.message ();
    ve = mgr.findByName ("veth0", "2001:db8::1237");
    ASSERT_NE (ve, nullptr);
    ASSERT_EQ (mgr.removeNeighbor ("veth0", "2001:db8::1237", true), 0) << lastError.message ();
    ve = mgr.findByName ("veth0", "2001:db8::1237");
    ASSERT_EQ (ve, nullptr);
}

/**
 * @brief test the flushNeighbors method.
 */
TEST_F (NeighborManagerTest, flushNeighbors)
{
    NeighborManager mgr;

    ASSERT_EQ (mgr.addNeighbor (if_nametoindex ("veth0"), "192.168.100.3", "4e:ed:ed:ee:59:dd", NUD_PERMANENT, true), 0)
        << lastError.message ();
    auto ve = mgr.findByIndex (if_nametoindex ("veth0"), "192.168.100.3");
    ASSERT_NE (ve, nullptr);
    ASSERT_EQ (mgr.flushNeighbors (if_nametoindex ("veth0"), true), 0) << lastError.message ();
    ve = mgr.findByIndex (if_nametoindex ("veth0"), "192.168.100.3");
    ASSERT_EQ (ve, nullptr);

    ASSERT_EQ (mgr.addNeighbor ("veth0", "2001:db8::1237", "4e:ed:ed:ee:59:dd", NUD_PERMANENT, true), 0)
        << lastError.message ();
    ve = mgr.findByName ("veth0", "2001:db8::1237");
    ASSERT_NE (ve, nullptr);
    ASSERT_EQ (mgr.flushNeighbors ("veth0", true), 0) << lastError.message ();
    ve = mgr.findByName ("veth0", "2001:db8::1237");
    ASSERT_EQ (ve, nullptr);
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
