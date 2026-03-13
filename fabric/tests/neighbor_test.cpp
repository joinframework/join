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

using join::Neighbor;
using join::NeighborManager;
using join::MacAddress;
using join::IpAddress;

/**
 * @brief Class used to test the neighbor API.
 */
class NeighborTest : public ::testing::Test
{
public:
    /**
     * @brief set up test suite.
     */
    static void SetUpTestSuite ()
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
     * @brief tear down test suite.
     */
    static void TearDownTestSuite ()
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
 * @brief test the index method.
 */
TEST_F (NeighborTest, index)
{
    NeighborManager mgr;

    auto ve = mgr.findByName ("veth0", "192.168.100.2");
    ASSERT_NE (ve, nullptr);
    ASSERT_EQ (ve->index (), if_nametoindex ("veth0"));

    ve = mgr.findByName ("veth0", "2001:db8::1236");
    ASSERT_NE (ve, nullptr);
    ASSERT_EQ (ve->index (), if_nametoindex ("veth0"));
}

/**
 * @brief test the ip method.
 */
TEST_F (NeighborTest, ip)
{
    NeighborManager mgr;

    auto ve = mgr.findByName ("veth0", "192.168.100.2");
    ASSERT_NE (ve, nullptr);
    ASSERT_EQ (ve->ip (), "192.168.100.2");

    ve = mgr.findByName ("veth0", "2001:db8::1236");
    ASSERT_NE (ve, nullptr);
    ASSERT_EQ (ve->ip (), "2001:db8::1236%veth0");
}

/**
 * @brief test the mac method.
 */
TEST_F (NeighborTest, mac)
{
    NeighborManager mgr;

    auto ve = mgr.findByName ("veth0", "192.168.100.2");
    ASSERT_NE (ve, nullptr);
    ASSERT_EQ (ve->mac (), "4e:ed:ed:ee:59:dc");

    ve = mgr.findByName ("veth0", "2001:db8::1236");
    ASSERT_NE (ve, nullptr);
    ASSERT_EQ (ve->mac (), "4e:ed:ed:ee:59:dc");
}

/**
 * @brief test the state method.
 */
TEST_F (NeighborTest, state)
{
    NeighborManager mgr;

    auto ve = mgr.findByName ("veth0", "192.168.100.2");
    ASSERT_NE (ve, nullptr);
    ASSERT_NE (ve->state (), 0);

    ve = mgr.findByName ("veth0", "2001:db8::1236");
    ASSERT_NE (ve, nullptr);
    ASSERT_NE (ve->state (), 0);
}

/**
 * @brief test the isReachable method.
 */
TEST_F (NeighborTest, isReachable)
{
    NeighborManager mgr;

    auto ve = mgr.findByName ("veth0", "192.168.100.2");
    ASSERT_NE (ve, nullptr);
    ASSERT_EQ (ve->set ("4e:ed:ed:ee:59:dc", NUD_REACHABLE, true), 0) << join::lastError.message ();
    ASSERT_TRUE (ve->isReachable ());

    ve = mgr.findByName ("veth0", "2001:db8::1236");
    ASSERT_NE (ve, nullptr);
    ASSERT_EQ (ve->set ("4e:ed:ed:ee:59:dc", NUD_REACHABLE, true), 0) << join::lastError.message ();
    ASSERT_TRUE (ve->isReachable ());
}

/**
 * @brief test the isStale method.
 */
TEST_F (NeighborTest, isStale)
{
    NeighborManager mgr;

    auto ve = mgr.findByName ("veth0", "192.168.100.2");
    ASSERT_NE (ve, nullptr);
    ASSERT_EQ (ve->set ("4e:ed:ed:ee:59:dc", NUD_STALE, true), 0) << join::lastError.message ();
    ASSERT_TRUE (ve->isStale ());

    ve = mgr.findByName ("veth0", "2001:db8::1236");
    ASSERT_NE (ve, nullptr);
    ASSERT_EQ (ve->set ("4e:ed:ed:ee:59:dc", NUD_STALE, true), 0) << join::lastError.message ();
    ASSERT_TRUE (ve->isStale ());
}

/**
 * @brief test the isPermanent method.
 */
TEST_F (NeighborTest, isPermanent)
{
    NeighborManager mgr;

    auto ve = mgr.findByName ("veth0", "192.168.100.2");
    ASSERT_NE (ve, nullptr);
    ASSERT_EQ (ve->set ("4e:ed:ed:ee:59:dc", NUD_PERMANENT, true), 0) << join::lastError.message ();
    ASSERT_TRUE (ve->isPermanent ());

    ve = mgr.findByName ("veth0", "2001:db8::1236");
    ASSERT_NE (ve, nullptr);
    ASSERT_EQ (ve->set ("4e:ed:ed:ee:59:dc", NUD_PERMANENT, true), 0) << join::lastError.message ();
    ASSERT_TRUE (ve->isPermanent ());
}

/**
 * @brief test the isIncomplete method.
 */
TEST_F (NeighborTest, isIncomplete)
{
    NeighborManager mgr;

    auto ve = mgr.findByName ("veth0", "192.168.100.2");
    ASSERT_NE (ve, nullptr);
    ASSERT_EQ (ve->set ("4e:ed:ed:ee:59:dc", NUD_INCOMPLETE, true), 0) << join::lastError.message ();
    ASSERT_TRUE (ve->isIncomplete ());

    ve = mgr.findByName ("veth0", "2001:db8::1236");
    ASSERT_NE (ve, nullptr);
    ASSERT_EQ (ve->set ("4e:ed:ed:ee:59:dc", NUD_INCOMPLETE, true), 0) << join::lastError.message ();
    ASSERT_TRUE (ve->isIncomplete ());
}

/**
 * @brief test the isFailed method.
 */
TEST_F (NeighborTest, isFailed)
{
    NeighborManager mgr;

    auto ve = mgr.findByName ("veth0", "192.168.100.2");
    ASSERT_NE (ve, nullptr);
    ASSERT_EQ (ve->set ("4e:ed:ed:ee:59:dc", NUD_FAILED, true), 0) << join::lastError.message ();
    ASSERT_TRUE (ve->isFailed ());

    ve = mgr.findByName ("veth0", "2001:db8::1236");
    ASSERT_NE (ve, nullptr);
    ASSERT_EQ (ve->set ("4e:ed:ed:ee:59:dc", NUD_FAILED, true), 0) << join::lastError.message ();
    ASSERT_TRUE (ve->isFailed ());
}

/**
 * @brief test the remove method.
 */
TEST_F (NeighborTest, remove)
{
    NeighborManager mgr;

    auto ve = mgr.findByName ("veth0", "192.168.100.2");
    ASSERT_NE (ve, nullptr);
    ASSERT_EQ (ve->remove (true), 0) << join::lastError.message ();
    ve = mgr.findByName ("veth0", "192.168.100.2");
    ASSERT_EQ (ve, nullptr);

    ve = mgr.findByName ("veth0", "2001:db8::1236");
    ASSERT_NE (ve, nullptr);
    ASSERT_EQ (ve->remove (true), 0) << join::lastError.message ();
    ve = mgr.findByName ("veth0", "2001:db8::1236");
    ASSERT_EQ (ve, nullptr);
}

/**
 * @brief test the compare method.
 */
TEST_F (NeighborTest, compare)
{
    NeighborManager mgr;
    Neighbor::Ptr nil;

    auto ve = mgr.findByName ("veth0", "2001:db8::1236");
    ASSERT_NE (ve, nullptr);
    ASSERT_TRUE (nil == nil);
    ASSERT_TRUE (ve == ve);
    ASSERT_FALSE (ve == nil);
    ASSERT_FALSE (nil == ve);
    ASSERT_FALSE (ve < ve);
    ASSERT_FALSE (ve < nil);
    ASSERT_TRUE (nil < ve);
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
