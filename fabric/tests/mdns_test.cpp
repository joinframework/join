/**
 * MIT License
 *
 * Copyright (c) 2023 Mathieu Rabine
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
#include <join/nameserver.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::lastError;
using join::Mdns;
using join::IpAddress;
using join::IpAddressList;
using join::AliasList;
using join::DnsPacket;
using join::ResourceRecord;
using join::DnsMessage;

using namespace std::chrono_literals;

/**
 * @brief DNS test class.
 */
class MdnsTest : public ::testing::Test, public Mdns::Peer
{
public:
    MdnsTest ()
    : Mdns::Peer ("enp5s0")
    {
    }

    /**
     * @brief set up the test fixture.
     */
    void SetUp () override
    {
        ASSERT_EQ (this->bind (AF_INET), 0) << lastError.message ();
    }

    /**
     * @brief tear down the test fixture.
     */
    void TearDown () override
    {
        this->close ();
    }

    /**
     * @brief handle a DNS query.
     * @param query the DNS query.
     */
    void onQuery (const DnsPacket& query) override
    {
        this->defaultOnSuccess (query);
    }

    /**
     * @brief handle a mDNS announcement.
     * @param packet the mDNS announcement.
     */
    void onAnnouncement (const DnsPacket& packet) override
    {
        this->defaultOnSuccess (packet);
    }
};

/**
 * @brief test the browse method.
 */
TEST_F (MdnsTest, browse)
{
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
