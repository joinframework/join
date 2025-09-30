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
using join::IpAddress;

/**
 * @brief Class used to test the interface API.
 */
class InterfaceTest : public ::testing::Test
{
public:
    /**
     * @brief set up test case.
     */
    static void SetUpTestCase ()
    {
        [[maybe_unused]] int result;

        result = std::system ("ip link add dummy0 type dummy");
        result = std::system ("ip link set dummy0 address aa:66:d6:26:e4:59");
        result = std::system ("ip addr add 192.168.35.100/24 brd 192.168.35.255 dev dummy0");
        result = std::system ("ip -6 addr add 2001:db8::1234/64 dev dummy0");
        result = std::system ("ip link set dummy0 up arp on multicast on");

        result = std::system ("ip link add link dummy0 name dummy0.10 type vlan id 10");
        result = std::system ("ip link set dummy0.10 address aa:66:d6:26:e5:59");
        result = std::system ("ip addr add 192.168.10.1/24 brd 192.168.10.255 dev dummy0.10");
        result = std::system ("ip link set dev dummy0.10 up");

        result = std::system ("ip netns add red");
        result = std::system ("ip link add veth0 type veth peer name eth0 netns red");
        result = std::system ("ip link set veth0 address 4e:ed:ed:ee:59:db");
        result = std::system ("ip addr add 192.168.100.1/24 brd 192.168.100.255 dev veth0");
        result = std::system ("ip -6 addr add 2001:db8::1235/64 dev veth0");
        result = std::system ("ip link set veth0 up arp on multicast on");
        result = std::system ("ip -n red link set eth0 address 4e:ed:ed:ee:59:dc");
        result = std::system ("ip -n red addr add 192.168.16.200/24 brd 192.168.16.255 dev eth0");
        result = std::system ("ip -n red link set eth0 up arp on multicast on");

        result = std::system ("brctl addbr br0");
        result = std::system ("ip link set br0 address 4e:ed:ed:ee:59:da");
        result = std::system ("ip addr add 192.168.16.100/24 brd 192.168.16.255 dev br0");
        result = std::system ("ip -6 addr add 2001:db8::1236/64 dev veth0");
        result = std::system ("ip link set br0 up");
    }

    /**
     * @brief tear down test case.
     */
    static void TearDownTestCase ()
    {
        [[maybe_unused]] int result;

        result = std::system ("ip link set dummy0.10 down");
        result = std::system ("ip link del dummy0.10");

        result = std::system ("ip link set dummy0 down");
        result = std::system ("ip link del dummy0");

        result = std::system ("ip link set dev veth0 down");
        result = std::system ("ip link del veth0");
        result = std::system ("ip netns del red");

        result = std::system ("ip link set br0 down");
        result = std::system ("brctl delbr br0");
    }

    /**
     * @brief wait for link local address.
     * @param iface interface.
     * @param timeout timeout.
     * @return true if has link local address, false otherwise.
     */
    static bool waitLinkLocal (const Interface::Ptr& iface, int timeout = 5000)
    {
        auto deadline = std::chrono::steady_clock::now () + std::chrono::milliseconds (timeout);
        while (std::chrono::steady_clock::now () < deadline)
        {
            if (iface->hasLocalAddress ())
            {
                return true;
            }
            std::this_thread::sleep_for (std::chrono::milliseconds (100));
        }
        return false;
    }
};

/**
 * @brief test the index method.
 */
TEST_F (InterfaceTest, index)
{
    auto lo = InterfaceManager::instance ()->findByName ("lo");
    ASSERT_NE (lo, nullptr);
    ASSERT_EQ (lo->index (), if_nametoindex ("lo"));

    auto dm = InterfaceManager::instance ()->findByName ("dummy0");
    ASSERT_NE (dm, nullptr);
    ASSERT_EQ (dm->index (), if_nametoindex ("dummy0"));

    auto vl = InterfaceManager::instance ()->findByName ("dummy0.10");
    ASSERT_NE (vl, nullptr);
    ASSERT_EQ (vl->index (), if_nametoindex ("dummy0.10"));

    auto ve = InterfaceManager::instance ()->findByName ("veth0");
    ASSERT_NE (ve, nullptr);
    ASSERT_EQ (ve->index (), if_nametoindex ("veth0"));

    auto br = InterfaceManager::instance ()->findByName ("br0");
    ASSERT_NE (br, nullptr);
    ASSERT_EQ (br->index (), if_nametoindex ("br0"));
}

/**
 * @brief test the name method.
 */
TEST_F (InterfaceTest, name)
{
    auto lo = InterfaceManager::instance ()->findByName ("lo");
    ASSERT_NE (lo, nullptr);
    ASSERT_EQ (lo->name (), "lo");

    auto dm = InterfaceManager::instance ()->findByName ("dummy0");
    ASSERT_NE (dm, nullptr);
    ASSERT_EQ (dm->name (), "dummy0");

    auto vl = InterfaceManager::instance ()->findByName ("dummy0.10");
    ASSERT_NE (vl, nullptr);
    ASSERT_EQ (vl->name (), "dummy0.10");

    auto ve = InterfaceManager::instance ()->findByName ("veth0");
    ASSERT_NE (ve, nullptr);
    ASSERT_EQ (ve->name (), "veth0");

    auto br = InterfaceManager::instance ()->findByName ("br0");
    ASSERT_NE (br, nullptr);
    ASSERT_EQ (br->name (), "br0");
}

/**
 * @brief test the mtu method.
 */
TEST_F (InterfaceTest, mtu)
{
    auto lo = InterfaceManager::instance ()->findByName ("lo");
    ASSERT_NE (lo, nullptr);
    ASSERT_EQ (lo->mtu (), 65536);

    auto dm = InterfaceManager::instance ()->findByName ("dummy0");
    ASSERT_NE (dm, nullptr);
    ASSERT_EQ (dm->mtu (), 1500);

    auto vl = InterfaceManager::instance ()->findByName ("dummy0.10");
    ASSERT_NE (vl, nullptr);
    ASSERT_EQ (vl->mtu (), 1500);

    auto ve = InterfaceManager::instance ()->findByName ("veth0");
    ASSERT_NE (ve, nullptr);
    ASSERT_EQ (ve->mtu (), 1500);

    auto br = InterfaceManager::instance ()->findByName ("br0");
    ASSERT_NE (br, nullptr);
    ASSERT_EQ (br->mtu (), 1500);
    ASSERT_EQ (br->mtu (2000, true), 0) << lastError.message ();
    ASSERT_EQ (br->mtu (), 2000);
}

/**
 * @brief test the kind method.
 */
TEST_F (InterfaceTest, kind)
{
    auto lo = InterfaceManager::instance ()->findByName ("lo");
    ASSERT_NE (lo, nullptr);
    ASSERT_EQ (lo->kind (), "");

    auto dm = InterfaceManager::instance ()->findByName ("dummy0");
    ASSERT_NE (dm, nullptr);
    ASSERT_EQ (dm->kind (), "dummy");

    auto vl = InterfaceManager::instance ()->findByName ("dummy0.10");
    ASSERT_NE (vl, nullptr);
    ASSERT_EQ (vl->kind (), "vlan");

    auto ve = InterfaceManager::instance ()->findByName ("veth0");
    ASSERT_NE (ve, nullptr);
    ASSERT_EQ (ve->kind (), "veth");

    auto br = InterfaceManager::instance ()->findByName ("br0");
    ASSERT_NE (br, nullptr);
    ASSERT_EQ (br->kind (), "bridge");
}

/**
 * @brief test the mac method.
 */
TEST_F (InterfaceTest, mac)
{
    auto lo = InterfaceManager::instance ()->findByName ("lo");
    ASSERT_NE (lo, nullptr);
    ASSERT_EQ (lo->mac (), "00:00:00:00:00:00");

    auto dm = InterfaceManager::instance ()->findByName ("dummy0");
    ASSERT_NE (dm, nullptr);
    ASSERT_EQ (dm->mac (), "aa:66:d6:26:e4:59");

    auto vl = InterfaceManager::instance ()->findByName ("dummy0.10");
    ASSERT_NE (vl, nullptr);
    ASSERT_EQ (vl->mac (), "aa:66:d6:26:e5:59");

    auto ve = InterfaceManager::instance ()->findByName ("veth0");
    ASSERT_NE (ve, nullptr);
    ASSERT_EQ (ve->mac (), "4e:ed:ed:ee:59:db");

    auto br = InterfaceManager::instance ()->findByName ("br0");
    ASSERT_NE (br, nullptr);
    ASSERT_EQ (br->mac (), "4e:ed:ed:ee:59:da");
    ASSERT_EQ (br->mac ("4e:ed:ed:ee:59:dd", true), 0) << lastError.message ();
    ASSERT_EQ (br->mac (), "4e:ed:ed:ee:59:dd");
}

/**
 * @brief test the addAddress method.
 */
TEST_F (InterfaceTest, addAddress)
{
    auto dm = InterfaceManager::instance ()->findByName ("dummy0");
    ASSERT_NE (dm, nullptr);

    ASSERT_FALSE (dm->hasAddress ("192.168.31.100"));
    ASSERT_EQ (dm->addAddress ({"192.168.31.100", 24, "192.168.31.255"}, true), 0) << lastError.message ();
    ASSERT_TRUE (dm->hasAddress ("192.168.31.100"));
    ASSERT_EQ (dm->removeAddress ({"192.168.31.100", 24, "192.168.31.255"}), 0) << lastError.message ();

    ASSERT_FALSE (dm->hasAddress ("2001:db8:abcd:12::8"));
    ASSERT_EQ (dm->addAddress ({"2001:db8:abcd:12::8", 64, {}}, true), 0) << lastError.message ();
    ASSERT_TRUE (dm->hasAddress ("2001:db8:abcd:12::8"));
    ASSERT_EQ (dm->removeAddress ({"2001:db8:abcd:12::8", 64, {}}), 0) << lastError.message ();

    auto ve = InterfaceManager::instance ()->findByName ("veth0");
    ASSERT_NE (ve, nullptr);

    ASSERT_FALSE (ve->hasAddress ("192.168.200.100"));
    ASSERT_EQ (ve->addAddress ({"192.168.200.100", 24, "192.168.200.255"}, true), 0) << lastError.message ();
    ASSERT_TRUE (ve->hasAddress ("192.168.200.100"));
    ASSERT_EQ (ve->removeAddress ({"192.168.200.100", 24, "192.168.200.255"}), 0) << lastError.message ();

    ASSERT_FALSE (ve->hasAddress ("2001:db8:abcd:12::1"));
    ASSERT_EQ (ve->addAddress ({"2001:db8:abcd:12::1", 64, {}}, true), 0) << lastError.message ();
    ASSERT_TRUE (ve->hasAddress ("2001:db8:abcd:12::1"));
    ASSERT_EQ (ve->removeAddress ({"2001:db8:abcd:12::1", 64, {}}), 0) << lastError.message ();

    auto br = InterfaceManager::instance ()->findByName ("br0");
    ASSERT_NE (br, nullptr);

    ASSERT_FALSE (ve->hasAddress ("192.168.33.100"));
    ASSERT_EQ (ve->addAddress ({"192.168.33.100", 24, "192.168.33.255"}, true), 0) << lastError.message ();
    ASSERT_TRUE (ve->hasAddress ("192.168.33.100"));
    ASSERT_EQ (ve->removeAddress ({"192.168.33.100", 24, "192.168.33.255"}), 0) << lastError.message ();

    ASSERT_FALSE (br->hasAddress ("2001:db8:abcd:12::2"));
    ASSERT_EQ (br->addAddress ({"2001:db8:abcd:12::2", 64, {}}, true), 0) << lastError.message ();
    ASSERT_TRUE (br->hasAddress ("2001:db8:abcd:12::2"));
    ASSERT_EQ (br->removeAddress ({"2001:db8:abcd:12::2", 64, {}}), 0) << lastError.message ();
}

/**
 * @brief test the addressList method.
 */
TEST_F (InterfaceTest, addressList)
{
    auto lo = InterfaceManager::instance ()->findByName ("lo");
    ASSERT_NE (lo, nullptr);
    ASSERT_FALSE (lo->addressList ().empty ());

    auto dm = InterfaceManager::instance ()->findByName ("dummy0");
    ASSERT_NE (dm, nullptr);
    ASSERT_FALSE (dm->addressList ().empty ());

    auto vl = InterfaceManager::instance ()->findByName ("dummy0.10");
    ASSERT_NE (vl, nullptr);
    ASSERT_FALSE (vl->addressList ().empty ());

    auto ve = InterfaceManager::instance ()->findByName ("veth0");
    ASSERT_NE (ve, nullptr);
    ASSERT_FALSE (ve->addressList ().empty ());

    auto br = InterfaceManager::instance ()->findByName ("br0");
    ASSERT_NE (br, nullptr);
    ASSERT_FALSE (br->addressList ().empty ());
}

/**
 * @brief test the addRoute method.
 */
TEST_F (InterfaceTest, addRoute)
{
    auto ve = InterfaceManager::instance ()->findByName ("veth0");
    ASSERT_NE (ve, nullptr);

    ASSERT_FALSE (ve->hasRoute ({"192.168.200.0", 24, "192.168.100.254", 0}));
    ASSERT_EQ (ve->addRoute ({"192.168.200.0", 24, "192.168.100.254", 0}, true), 0) << lastError.message();
    ASSERT_TRUE (ve->hasRoute ({"192.168.200.0", 24, "192.168.100.254", 0}));
    ASSERT_EQ (ve->removeRoute ({"192.168.200.0", 24, "192.168.100.254", 0}), 0) << lastError.message();
}

/**
 * @brief test the routeList method.
 */
TEST_F (InterfaceTest, routeList)
{
    auto lo = InterfaceManager::instance ()->findByName ("lo");
    ASSERT_NE (lo, nullptr);
    ASSERT_FALSE (lo->routeList ().empty ());

    auto dm = InterfaceManager::instance ()->findByName ("dummy0");
    ASSERT_NE (dm, nullptr);
    ASSERT_FALSE (dm->routeList ().empty ());

    auto vl = InterfaceManager::instance ()->findByName ("dummy0.10");
    ASSERT_NE (vl, nullptr);
    ASSERT_FALSE (vl->routeList ().empty ());

    auto ve = InterfaceManager::instance ()->findByName ("veth0");
    ASSERT_NE (ve, nullptr);
    ASSERT_FALSE (ve->routeList ().empty ());

    auto br = InterfaceManager::instance ()->findByName ("br0");
    ASSERT_NE (br, nullptr);
    ASSERT_FALSE (br->routeList ().empty ());
}

/**
 * @brief test the addToBridge method.
 */
TEST_F (InterfaceTest, addToBridge)
{
    auto ve = InterfaceManager::instance ()->findByName ("veth0");
    ASSERT_NE (ve, nullptr);
    ASSERT_EQ (ve->master (), 0);
    ASSERT_EQ (ve->addToBridge ("foo", true), -1);
    ASSERT_EQ (ve->addToBridge ("br0", true), 0) << lastError.message ();
    ASSERT_GT (ve->master (), 0);
    ASSERT_EQ (ve->removeFromBridge (), 0) << lastError.message ();
}

/**
 * @brief test the flags method.
 */
TEST_F (InterfaceTest, flags)
{
    auto lo = InterfaceManager::instance ()->findByName ("lo");
    ASSERT_NE (lo, nullptr);
    ASSERT_GT (lo->flags (), 0);

    auto dm = InterfaceManager::instance ()->findByName ("dummy0");
    ASSERT_NE (dm, nullptr);
    ASSERT_GT (dm->flags (), 0);

    auto vl = InterfaceManager::instance ()->findByName ("dummy0.10");
    ASSERT_NE (vl, nullptr);
    ASSERT_GT (vl->flags (), 0);

    auto ve = InterfaceManager::instance ()->findByName ("veth0");
    ASSERT_NE (ve, nullptr);
    ASSERT_GT (ve->flags (), 0);

    auto br = InterfaceManager::instance ()->findByName ("br0");
    ASSERT_NE (br, nullptr);
    ASSERT_GT (br->flags (), 0);
}

/**
 * @brief test the enable method.
 */
TEST_F (InterfaceTest, enable)
{
    auto lo = InterfaceManager::instance ()->findByName ("lo");
    ASSERT_NE (lo, nullptr);
    ASSERT_TRUE (lo->isEnabled ());
    ASSERT_EQ (lo->enable (false, true), 0) << lastError.message ();
    ASSERT_FALSE (lo->isEnabled ());
    ASSERT_EQ (lo->enable (true), 0) << lastError.message ();

    auto dm = InterfaceManager::instance ()->findByName ("dummy0");
    ASSERT_NE (dm, nullptr);
    ASSERT_TRUE (dm->isEnabled ());
    ASSERT_EQ (dm->enable (false, true), 0) << lastError.message ();
    ASSERT_FALSE (dm->isEnabled ());
    ASSERT_EQ (dm->enable (true), 0) << lastError.message ();

    auto vl = InterfaceManager::instance ()->findByName ("dummy0.10");
    ASSERT_NE (vl, nullptr);
    ASSERT_TRUE (vl->isEnabled ());
    ASSERT_EQ (vl->enable (false, true), 0) << lastError.message ();
    ASSERT_FALSE (vl->isEnabled ());
    ASSERT_EQ (vl->enable (true), 0) << lastError.message ();

    auto ve = InterfaceManager::instance ()->findByName ("veth0");
    ASSERT_NE (ve, nullptr);
    ASSERT_TRUE (ve->isEnabled ());
    ASSERT_EQ (ve->enable (false, true), 0) << lastError.message ();
    ASSERT_FALSE (ve->isEnabled ());
    ASSERT_EQ (ve->enable (true), 0) << lastError.message ();

    auto br = InterfaceManager::instance ()->findByName ("br0");
    ASSERT_NE (br, nullptr);
    ASSERT_TRUE (br->isEnabled ());
    ASSERT_EQ (br->enable (false, true), 0) << lastError.message ();
    ASSERT_FALSE (br->isEnabled ());
    ASSERT_EQ (br->enable (true), 0) << lastError.message ();
}

/**
 * @brief test the isRunning method.
 */
TEST_F (InterfaceTest, isRunning)
{
    auto lo = InterfaceManager::instance ()->findByName ("lo");
    ASSERT_NE (lo, nullptr);
    ASSERT_TRUE (lo->isRunning ());

    auto dm = InterfaceManager::instance ()->findByName ("dummy0");
    ASSERT_NE (dm, nullptr);
    ASSERT_TRUE (waitLinkLocal (dm));
    ASSERT_TRUE (dm->isRunning ());

    auto vl = InterfaceManager::instance ()->findByName ("dummy0.10");
    ASSERT_NE (vl, nullptr);
    ASSERT_TRUE (waitLinkLocal (vl));
    ASSERT_TRUE (vl->isRunning ());

    auto ve = InterfaceManager::instance ()->findByName ("veth0");
    ASSERT_NE (ve, nullptr);
    ASSERT_TRUE (waitLinkLocal (ve));
    ASSERT_TRUE (ve->isRunning ());

    auto br = InterfaceManager::instance ()->findByName ("br0");
    ASSERT_NE (br, nullptr);
    ASSERT_EQ (ve->addToBridge ("br0", true), 0) << lastError.message ();
    ASSERT_TRUE (waitLinkLocal (br));
    ASSERT_TRUE (br->isRunning ());
    ASSERT_EQ (ve->removeFromBridge (true), 0) << lastError.message ();
}

/**
 * @brief test the isLoopback method.
 */
TEST_F (InterfaceTest, isLoopback)
{
    auto lo = InterfaceManager::instance ()->findByName ("lo");
    ASSERT_NE (lo, nullptr);
    ASSERT_TRUE (lo->isLoopback ());

    auto dm = InterfaceManager::instance ()->findByName ("dummy0");
    ASSERT_NE (dm, nullptr);
    ASSERT_FALSE (dm->isLoopback ());

    auto vl = InterfaceManager::instance ()->findByName ("dummy0.10");
    ASSERT_NE (vl, nullptr);
    ASSERT_FALSE (vl->isLoopback ());

    auto ve = InterfaceManager::instance ()->findByName ("veth0");
    ASSERT_NE (ve, nullptr);
    ASSERT_FALSE (ve->isLoopback ());

    auto br = InterfaceManager::instance ()->findByName ("br0");
    ASSERT_NE (br, nullptr);
    ASSERT_FALSE (br->isLoopback ());
}

/**
 * @brief test the isPointToPoint method.
 */
TEST_F (InterfaceTest, isPointToPoint)
{
    auto lo = InterfaceManager::instance ()->findByName ("lo");
    ASSERT_NE (lo, nullptr);
    ASSERT_FALSE (lo->isPointToPoint ());

    auto dm = InterfaceManager::instance ()->findByName ("dummy0");
    ASSERT_NE (dm, nullptr);
    ASSERT_FALSE (dm->isPointToPoint ());

    auto vl = InterfaceManager::instance ()->findByName ("dummy0.10");
    ASSERT_NE (vl, nullptr);
    ASSERT_FALSE (vl->isPointToPoint ());

    auto ve = InterfaceManager::instance ()->findByName ("veth0");
    ASSERT_NE (ve, nullptr);
    ASSERT_FALSE (ve->isPointToPoint ());

    auto br = InterfaceManager::instance ()->findByName ("br0");
    ASSERT_NE (br, nullptr);
    ASSERT_FALSE (br->isPointToPoint ());
}

/**
 * @brief test the isBridge method.
 */
TEST_F (InterfaceTest, isDummy)
{
    auto lo = InterfaceManager::instance ()->findByName ("lo");
    ASSERT_NE (lo, nullptr);
    ASSERT_FALSE (lo->isDummy ());

    auto dm = InterfaceManager::instance ()->findByName ("dummy0");
    ASSERT_NE (dm, nullptr);
    ASSERT_TRUE (dm->isDummy ());

    auto vl = InterfaceManager::instance ()->findByName ("dummy0.10");
    ASSERT_NE (vl, nullptr);
    ASSERT_FALSE (vl->isDummy ());

    auto ve = InterfaceManager::instance ()->findByName ("veth0");
    ASSERT_NE (ve, nullptr);
    ASSERT_FALSE (ve->isDummy ());

    auto br = InterfaceManager::instance ()->findByName ("br0");
    ASSERT_NE (br, nullptr);
    ASSERT_FALSE (br->isDummy ());
}

/**
 * @brief test the isBridge method.
 */
TEST_F (InterfaceTest, isBridge)
{
    auto lo = InterfaceManager::instance ()->findByName ("lo");
    ASSERT_NE (lo, nullptr);
    ASSERT_FALSE (lo->isBridge ());

    auto dm = InterfaceManager::instance ()->findByName ("dummy0");
    ASSERT_NE (dm, nullptr);
    ASSERT_FALSE (dm->isBridge ());

    auto vl = InterfaceManager::instance ()->findByName ("dummy0.10");
    ASSERT_NE (vl, nullptr);
    ASSERT_FALSE (vl->isBridge ());

    auto ve = InterfaceManager::instance ()->findByName ("veth0");
    ASSERT_NE (ve, nullptr);
    ASSERT_FALSE (ve->isBridge ());

    auto br = InterfaceManager::instance ()->findByName ("br0");
    ASSERT_NE (br, nullptr);
    ASSERT_TRUE (br->isBridge ());
}

/**
 * @brief test the isVlan method.
 */
TEST_F (InterfaceTest, isVlan)
{
    auto lo = InterfaceManager::instance ()->findByName ("lo");
    ASSERT_NE (lo, nullptr);
    ASSERT_FALSE (lo->isVlan ());

    auto dm = InterfaceManager::instance ()->findByName ("dummy0");
    ASSERT_NE (dm, nullptr);
    ASSERT_FALSE (dm->isVlan ());

    auto vl = InterfaceManager::instance ()->findByName ("dummy0.10");
    ASSERT_NE (vl, nullptr);
    ASSERT_TRUE (vl->isVlan ());

    auto ve = InterfaceManager::instance ()->findByName ("veth0");
    ASSERT_NE (ve, nullptr);
    ASSERT_FALSE (ve->isVlan ());

    auto br = InterfaceManager::instance ()->findByName ("br0");
    ASSERT_NE (br, nullptr);
    ASSERT_FALSE (br->isVlan ());
}

/**
 * @brief test the isVeth method.
 */
TEST_F (InterfaceTest, isVeth)
{
    auto lo = InterfaceManager::instance ()->findByName ("lo");
    ASSERT_NE (lo, nullptr);
    ASSERT_FALSE (lo->isVeth ());

    auto dm = InterfaceManager::instance ()->findByName ("dummy0");
    ASSERT_NE (dm, nullptr);
    ASSERT_FALSE (dm->isVeth ());

    auto vl = InterfaceManager::instance ()->findByName ("dummy0.10");
    ASSERT_NE (vl, nullptr);
    ASSERT_FALSE (vl->isVeth ());

    auto ve = InterfaceManager::instance ()->findByName ("veth0");
    ASSERT_NE (ve, nullptr);
    ASSERT_TRUE (ve->isVeth ());

    auto br = InterfaceManager::instance ()->findByName ("br0");
    ASSERT_NE (br, nullptr);
    ASSERT_FALSE (br->isVeth ());
}

/**
 * @brief test the isGre method.
 */
TEST_F (InterfaceTest, isGre)
{
    auto lo = InterfaceManager::instance ()->findByName ("lo");
    ASSERT_NE (lo, nullptr);
    ASSERT_FALSE (lo->isGre ());

    auto dm = InterfaceManager::instance ()->findByName ("dummy0");
    ASSERT_NE (dm, nullptr);
    ASSERT_FALSE (dm->isGre ());

    auto vl = InterfaceManager::instance ()->findByName ("dummy0.10");
    ASSERT_NE (vl, nullptr);
    ASSERT_FALSE (vl->isGre ());

    auto ve = InterfaceManager::instance ()->findByName ("veth0");
    ASSERT_NE (ve, nullptr);
    ASSERT_FALSE (ve->isGre ());

    auto br = InterfaceManager::instance ()->findByName ("br0");
    ASSERT_NE (br, nullptr);
    ASSERT_FALSE (br->isGre ());
}

/**
 * @brief test the isTun method.
 */
TEST_F (InterfaceTest, isTun)
{
    auto lo = InterfaceManager::instance ()->findByName ("lo");
    ASSERT_NE (lo, nullptr);
    ASSERT_FALSE (lo->isTun ());

    auto dm = InterfaceManager::instance ()->findByName ("dummy0");
    ASSERT_NE (dm, nullptr);
    ASSERT_FALSE (dm->isTun ());

    auto vl = InterfaceManager::instance ()->findByName ("dummy0.10");
    ASSERT_NE (vl, nullptr);
    ASSERT_FALSE (vl->isTun ());

    auto ve = InterfaceManager::instance ()->findByName ("veth0");
    ASSERT_NE (ve, nullptr);
    ASSERT_FALSE (ve->isTun ());

    auto br = InterfaceManager::instance ()->findByName ("br0");
    ASSERT_NE (br, nullptr);
    ASSERT_FALSE (br->isTun ());
}

/**
 * @brief test the supportsBroadcast method.
 */
TEST_F (InterfaceTest, supportsBroadcast)
{
    auto lo = InterfaceManager::instance ()->findByName ("lo");
    ASSERT_NE (lo, nullptr);
    ASSERT_FALSE (lo->supportsBroadcast ());

    auto dm = InterfaceManager::instance ()->findByName ("dummy0");
    ASSERT_NE (dm, nullptr);
    ASSERT_TRUE (dm->supportsBroadcast ());

    auto vl = InterfaceManager::instance ()->findByName ("dummy0.10");
    ASSERT_NE (vl, nullptr);
    ASSERT_TRUE (vl->supportsBroadcast ());

    auto ve = InterfaceManager::instance ()->findByName ("veth0");
    ASSERT_NE (ve, nullptr);
    ASSERT_TRUE (ve->supportsBroadcast ());

    auto br = InterfaceManager::instance ()->findByName ("br0");
    ASSERT_NE (br, nullptr);
    ASSERT_TRUE (br->supportsBroadcast ());
}

/**
 * @brief test the supportsMulticast method.
 */
TEST_F (InterfaceTest, supportsMulticast)
{
    auto lo = InterfaceManager::instance ()->findByName ("lo");
    ASSERT_NE (lo, nullptr);
    ASSERT_FALSE (lo->supportsMulticast ());

    auto dm = InterfaceManager::instance ()->findByName ("dummy0");
    ASSERT_NE (dm, nullptr);
    ASSERT_TRUE (dm->supportsMulticast ());

    auto vl = InterfaceManager::instance ()->findByName ("dummy0.10");
    ASSERT_NE (vl, nullptr);
    ASSERT_TRUE (vl->supportsMulticast ());

    auto ve = InterfaceManager::instance ()->findByName ("veth0");
    ASSERT_NE (ve, nullptr);
    ASSERT_TRUE (ve->supportsMulticast ());

    auto br = InterfaceManager::instance ()->findByName ("br0");
    ASSERT_NE (br, nullptr);
    ASSERT_TRUE (br->supportsMulticast ());
}

/**
 * @brief test the supportsIpv4 method.
 */
TEST_F (InterfaceTest, supportsIpv4)
{
    auto lo = InterfaceManager::instance ()->findByName ("lo");
    ASSERT_NE (lo, nullptr);
    ASSERT_TRUE (lo->supportsIpv4 ());

    auto dm = InterfaceManager::instance ()->findByName ("dummy0");
    ASSERT_NE (dm, nullptr);
    ASSERT_TRUE (dm->supportsIpv4 ());

    auto vl = InterfaceManager::instance ()->findByName ("dummy0.10");
    ASSERT_NE (vl, nullptr);
    ASSERT_TRUE (vl->supportsIpv4 ());

    auto ve = InterfaceManager::instance ()->findByName ("veth0");
    ASSERT_NE (ve, nullptr);
    ASSERT_TRUE (ve->supportsIpv4 ());

    auto br = InterfaceManager::instance ()->findByName ("br0");
    ASSERT_NE (br, nullptr);
    ASSERT_TRUE (br->supportsIpv4 ());
}

/**
 * @brief test the supportsIpv6 method.
 */
TEST_F (InterfaceTest, supportsIpv6)
{
    auto lo = InterfaceManager::instance ()->findByName ("lo");
    ASSERT_NE (lo, nullptr);
    ASSERT_TRUE (lo->supportsIpv6 ());

    auto dm = InterfaceManager::instance ()->findByName ("dummy0");
    ASSERT_NE (dm, nullptr);
    ASSERT_TRUE (waitLinkLocal (dm));
    ASSERT_TRUE (dm->supportsIpv6 ());

    auto vl = InterfaceManager::instance ()->findByName ("dummy0.10");
    ASSERT_NE (vl, nullptr);
    ASSERT_TRUE (waitLinkLocal (vl));
    ASSERT_TRUE (vl->supportsIpv6 ());

    auto ve = InterfaceManager::instance ()->findByName ("veth0");
    ASSERT_NE (ve, nullptr);
    ASSERT_TRUE (waitLinkLocal (ve));
    ASSERT_TRUE (ve->supportsIpv6 ());

    auto br = InterfaceManager::instance ()->findByName ("br0");
    ASSERT_NE (br, nullptr);
    ASSERT_EQ (ve->addToBridge ("br0", true), 0) << lastError.message ();
    ASSERT_TRUE (waitLinkLocal (br));
    ASSERT_TRUE (br->supportsIpv6 ());
    ASSERT_EQ (ve->removeFromBridge (true), 0) << lastError.message ();
}

/**
 * @brief test the compare method.
 */
TEST_F (InterfaceTest, compare)
{
    Interface::Ptr nil;

    auto lo = InterfaceManager::instance ()->findByName ("lo");
    ASSERT_NE (lo, nullptr);
    ASSERT_TRUE (nil == nil);
    ASSERT_TRUE (lo == lo);
    ASSERT_FALSE (lo == nil);
    ASSERT_FALSE (nil == lo);
    ASSERT_FALSE (lo < lo);
    ASSERT_FALSE (lo < nil);
    ASSERT_TRUE (nil < lo);
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
