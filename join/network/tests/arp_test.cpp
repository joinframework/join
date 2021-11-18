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
#include <join/error.hpp>
#include <join/arp.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::IpAddress;
using join::MacAddress;
using join::Arp;

using join::lastError;

/**
 * @brief Class used to test the ARP API.
 */
class ArpProtocol : public ::testing::Test
{
public:
    /**
     * @brief set up test case.
     */
    static void SetUpTestCase()
    {
        [[maybe_unused]] int result;
        result = std::system ("ip link add dummy0 type dummy");
        result = std::system ("ip link set dummy0 address 4e:ed:ed:ee:59:db");
        result = std::system ("ip addr add 192.168.16.100/24 brd 192.168.16.255 dev dummy0");
        result = std::system ("ip link set dummy0 up arp on multicast on");
    }

    /**
     * @brief tear down test case.
     */
    static void TearDownTestCase()
    {
        [[maybe_unused]] int result;
        result = std::system ("ip link set dev dummy0 down");
        result = std::system ("ip link del dummy0");
    }
};

/**
 * @brief Test mac method.
 */
TEST_F (ArpProtocol, mac)
{
    ASSERT_EQ (Arp ("dummy0").mac ("192.168.16.100"), "4e:ed:ed:ee:59:db");
    ASSERT_EQ (Arp::mac ("192.168.16.100", "dummy0"), "4e:ed:ed:ee:59:db") << lastError.message ();
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
