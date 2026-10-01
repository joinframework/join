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
#include <join/ndp_message.hpp>
#include <join/error.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::lastError;
using join::Errc;
using join::IpAddress;
using join::MacAddress;
using join::NdpPrefix;
using join::NdpRdnss;
using join::NdpMessage;
using join::RouterSolicitation;
using join::RouterAdvertisement;
using join::NeighborSolicitation;
using join::NeighborAdvertisement;

/**
 * @brief build an advertisement exercising every option.
 */
static RouterAdvertisement sample ()
{
    RouterAdvertisement packet;

    packet.hopLimit = 64;
    packet.flags = ND_RA_FLAG_MANAGED | ND_RA_FLAG_OTHER;
    packet.lifetime = 1800;
    packet.reachable = 30000;
    packet.retransmit = 1000;
    packet.link = "4e:ed:ed:ee:59:db";
    packet.mtu = 1500;

    NdpPrefix prefix;
    prefix.prefix = "2001:db8:1::";
    packet.prefixes.push_back (prefix);

    prefix.prefix = "2001:db8:2::";
    prefix.length = 56;
    prefix.flags = ND_OPT_PI_FLAG_ONLINK;
    prefix.valid = 86400;
    prefix.preferred = 14400;
    packet.prefixes.push_back (prefix);

    NdpRdnss rdnss;
    rdnss.lifetime = 3600;
    rdnss.servers = {"2001:db8:1::53", "2001:db8:2::53"};
    packet.rdnss.push_back (rdnss);

    rdnss.lifetime = 600;
    rdnss.servers = {"2001:db8:3::53"};
    packet.rdnss.push_back (rdnss);

    return packet;
}

/**
 * @brief Test serialize method with a router solicitation.
 */
TEST (NdpMessage, serializeRouterSolicitation)
{
    NdpMessage message;
    char data[64];

    RouterSolicitation packet;
    ssize_t size = message.serialize (packet, data, sizeof (data));
    ASSERT_NE (size, -1) << lastError.message ();
    ASSERT_EQ (std::string (data, size), std::string ("\x85\x00\x00\x00\x00\x00\x00\x00", 8));

    packet.link = "4e:ed:ed:ee:59:db";
    size = message.serialize (packet, data, sizeof (data));
    ASSERT_NE (size, -1) << lastError.message ();
    ASSERT_EQ (std::string (data, size), std::string ("\x85\x00\x00\x00\x00\x00\x00\x00"
                                                      "\x01\x01\x4e\xed\xed\xee\x59\xdb",
                                                      16));

    for (size_t length = 0; length < 16; ++length)
    {
        ASSERT_EQ (message.serialize (packet, data, length), -1) << "length " << length;
        ASSERT_EQ (lastError, Errc::MessageTooLong) << lastError.message ();
    }
}

/**
 * @brief Test serialize method with a router advertisement.
 */
TEST (NdpMessage, serializeRouterAdvertisement)
{
    NdpMessage message;
    char data[1024];

    RouterAdvertisement bad = sample ();
    bad.prefixes[0].prefix = "192.168.1.0";
    ASSERT_EQ (message.serialize (bad, data, sizeof (data)), -1);
    ASSERT_EQ (lastError, Errc::InvalidParam) << lastError.message ();

    bad = sample ();
    bad.prefixes[0].length = 129;
    ASSERT_EQ (message.serialize (bad, data, sizeof (data)), -1);
    ASSERT_EQ (lastError, Errc::InvalidParam) << lastError.message ();

    bad = sample ();
    bad.rdnss[1].servers.push_back ("192.168.1.53");
    ASSERT_EQ (message.serialize (bad, data, sizeof (data)), -1);
    ASSERT_EQ (lastError, Errc::InvalidParam) << lastError.message ();

    bad = sample ();
    bad.rdnss[1].servers.clear ();
    ASSERT_EQ (message.serialize (bad, data, sizeof (data)), -1);
    ASSERT_EQ (lastError, Errc::InvalidParam) << lastError.message ();

    bad = sample ();
    bad.rdnss[1].servers.assign (128, IpAddress ("2001:db8::53"));
    ASSERT_EQ (message.serialize (bad, data, sizeof (data)), -1);
    ASSERT_EQ (lastError, Errc::MessageTooLong) << lastError.message ();

    RouterAdvertisement minimal;
    ssize_t size = message.serialize (minimal, data, sizeof (data));
    ASSERT_NE (size, -1) << lastError.message ();
    ASSERT_EQ (std::string (data, size), std::string ("\x86\x00\x00\x00\x00\x00\x00\x00"
                                                      "\x00\x00\x00\x00\x00\x00\x00\x00",
                                                      16));

    size = message.serialize (sample (), data, sizeof (data));
    ASSERT_NE (size, -1) << lastError.message ();

    const std::string wire (data, size);
    ASSERT_EQ (wire.size (), 16u + 8u + 8u + 32u + 32u + 40u + 24u);
    ASSERT_EQ (wire.substr (0, 16), std::string ("\x86\x00\x00\x00\x40\xc0\x07\x08"
                                                 "\x00\x00\x75\x30\x00\x00\x03\xe8",
                                                 16));
    ASSERT_EQ (wire.substr (16, 8), std::string ("\x01\x01\x4e\xed\xed\xee\x59\xdb", 8));
    ASSERT_EQ (wire.substr (24, 8), std::string ("\x05\x01\x00\x00\x00\x00\x05\xdc", 8));
    ASSERT_EQ (wire.substr (32, 16), std::string ("\x03\x04\x40\xc0\x00\x27\x8d\x00"
                                                  "\x00\x09\x3a\x80\x00\x00\x00\x00",
                                                  16));
    ASSERT_EQ (wire.substr (48, 16), std::string ("\x20\x01\x0d\xb8\x00\x01\x00\x00"
                                                  "\x00\x00\x00\x00\x00\x00\x00\x00",
                                                  16));
    ASSERT_EQ (wire.substr (96, 8), std::string ("\x19\x05\x00\x00\x00\x00\x0e\x10", 8));
    ASSERT_EQ (wire.substr (136, 8), std::string ("\x19\x03\x00\x00\x00\x00\x02\x58", 8));

    for (size_t length = 0; length < wire.size (); ++length)
    {
        ASSERT_EQ (message.serialize (sample (), data, length), -1) << "length " << length;
        ASSERT_EQ (lastError, Errc::MessageTooLong) << lastError.message ();
    }

    RouterAdvertisement host = sample ();
    host.prefixes[0].prefix = "2001:db8:1::ffff";

    size = message.serialize (host, data, sizeof (data));
    ASSERT_NE (size, -1) << lastError.message ();
    ASSERT_EQ (std::string (data, size).substr (48, 16), std::string ("\x20\x01\x0d\xb8\x00\x01\x00\x00"
                                                                      "\x00\x00\x00\x00\x00\x00\x00\x00",
                                                                      16));
}

/**
 * @brief Test deserialize method with a router solicitation.
 */
TEST (NdpMessage, deserializeRouterSolicitation)
{
    NdpMessage message;
    RouterSolicitation packet;

    std::string data = (std::string ("\x85\x00\x00\x00\x00\x00", 6));
    ASSERT_EQ (message.deserialize (packet, data.data (), data.size ()), -1);
    ASSERT_EQ (lastError, Errc::MessageTooLong) << lastError.message ();

    data = (std::string ("\x86\x00\x00\x00\x00\x00\x00\x00", 8));
    ASSERT_EQ (message.deserialize (packet, data.data (), data.size ()), -1);
    ASSERT_EQ (lastError, Errc::MessageUnknown) << lastError.message ();

    data = (std::string ("\x85\x01\x00\x00\x00\x00\x00\x00", 8));
    ASSERT_EQ (message.deserialize (packet, data.data (), data.size ()), -1);
    ASSERT_EQ (lastError, Errc::MessageUnknown) << lastError.message ();

    data =
        (std::string ("\x85\x00\x00\x00\x00\x00\x00\x00"
                      "\x01\x00\x4e\xed\xed\xee\x59\xdb",
                      16));
    ASSERT_EQ (message.deserialize (packet, data.data (), data.size ()), -1);
    ASSERT_EQ (lastError, Errc::InvalidParam) << lastError.message ();

    data = (std::string ("\x85\x00\x00\x00\x00\x00\x00\x00\x01", 9));
    ASSERT_EQ (message.deserialize (packet, data.data (), data.size ()), -1);
    ASSERT_EQ (lastError, Errc::InvalidParam) << lastError.message ();

    data =
        (std::string ("\x85\x00\x00\x00\x00\x00\x00\x00"
                      "\x01\x02\x4e\xed\xed\xee\x59\xdb",
                      16));
    ASSERT_EQ (message.deserialize (packet, data.data (), data.size ()), -1);
    ASSERT_EQ (lastError, Errc::MessageTooLong) << lastError.message ();

    data = (std::string ("\x85\x00\x00\x00\x00\x00\x00\x00", 8));
    ASSERT_EQ (message.deserialize (packet, data.data (), data.size ()), 0) << lastError.message ();
    ASSERT_TRUE (packet.link.isWildcard ());

    data =
        (std::string ("\x85\x00\x00\x00\x00\x00\x00\x00"
                      "\x0e\x01\x00\x00\x00\x00\x00\x00"
                      "\x01\x01\x4e\xed\xed\xee\x59\xdb",
                      24));
    ASSERT_EQ (message.deserialize (packet, data.data (), data.size ()), 0) << lastError.message ();
    ASSERT_EQ (packet.link, MacAddress ("4e:ed:ed:ee:59:db"));
}

/**
 * @brief Test deserialize method with a router advertisement.
 */
TEST (NdpMessage, deserializeRouterAdvertisement)
{
    NdpMessage message;
    RouterAdvertisement packet;

    std::string data = (std::string ("\x86\x00\x00\x00\x00\x00\x00\x00", 8));
    ASSERT_EQ (message.deserialize (packet, data.data (), data.size ()), -1);
    ASSERT_EQ (lastError, Errc::MessageTooLong) << lastError.message ();

    data =
        (std::string ("\x85\x00\x00\x00\x00\x00\x00\x00"
                      "\x00\x00\x00\x00\x00\x00\x00\x00",
                      16));
    ASSERT_EQ (message.deserialize (packet, data.data (), data.size ()), -1);
    ASSERT_EQ (lastError, Errc::MessageUnknown) << lastError.message ();

    data =
        (std::string ("\x86\x01\x00\x00\x00\x00\x00\x00"
                      "\x00\x00\x00\x00\x00\x00\x00\x00",
                      16));
    ASSERT_EQ (message.deserialize (packet, data.data (), data.size ()), -1);
    ASSERT_EQ (lastError, Errc::MessageUnknown) << lastError.message ();

    char wire[1024];
    ssize_t size = message.serialize (sample (), wire, sizeof (wire));
    ASSERT_NE (size, -1) << lastError.message ();

    data.assign (wire, size);
    ASSERT_EQ (message.deserialize (packet, data.data (), data.size ()), 0) << lastError.message ();

    const RouterAdvertisement expected = sample ();
    ASSERT_EQ (packet.hopLimit, expected.hopLimit);
    ASSERT_EQ (packet.flags, expected.flags);
    ASSERT_EQ (packet.lifetime, expected.lifetime);
    ASSERT_EQ (packet.reachable, expected.reachable);
    ASSERT_EQ (packet.retransmit, expected.retransmit);
    ASSERT_EQ (packet.link, expected.link);
    ASSERT_EQ (packet.mtu, expected.mtu);
    ASSERT_EQ (packet.prefixes.size (), expected.prefixes.size ());
    for (size_t i = 0; i < packet.prefixes.size (); ++i)
    {
        ASSERT_EQ (packet.prefixes[i].prefix, expected.prefixes[i].prefix);
        ASSERT_EQ (packet.prefixes[i].length, expected.prefixes[i].length);
        ASSERT_EQ (packet.prefixes[i].flags, expected.prefixes[i].flags);
        ASSERT_EQ (packet.prefixes[i].valid, expected.prefixes[i].valid);
        ASSERT_EQ (packet.prefixes[i].preferred, expected.prefixes[i].preferred);
    }
    ASSERT_EQ (packet.rdnss.size (), expected.rdnss.size ());
    for (size_t i = 0; i < packet.rdnss.size (); ++i)
    {
        ASSERT_EQ (packet.rdnss[i].lifetime, expected.rdnss[i].lifetime);
        ASSERT_EQ (packet.rdnss[i].servers, expected.rdnss[i].servers);
    }

    data =
        (std::string ("\x86\x00\x00\x00\x00\x00\x00\x00"
                      "\x00\x00\x00\x00\x00\x00\x00\x00",
                      16));
    ASSERT_EQ (message.deserialize (packet, data.data (), data.size ()), 0) << lastError.message ();
    ASSERT_TRUE (packet.link.isWildcard ());
    ASSERT_EQ (packet.mtu, 0u);
    ASSERT_TRUE (packet.prefixes.empty ());
    ASSERT_TRUE (packet.rdnss.empty ());

    std::string ignored (
        "\x86\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x05\x02\x00\x00\x00\x00\x05\xdc\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x19\x02\x00\x00\x00\x00\x0e\x10\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x1f\x01\x00\x00\x00\x00\x00\x00",
        56);
    std::string prefix ("\x03\x04\x81\xc0\x00\x27\x8d\x00\x00\x09\x3a\x80\x00\x00\x00\x00", 16);
    prefix += std::string (16, '\0');
    ignored += prefix;
    ignored += std::string ("\x03\x01\x40\xc0\x00\x00\x00\x00", 8);

    data = ignored;
    ASSERT_EQ (message.deserialize (packet, data.data (), data.size ()), 0) << lastError.message ();
    ASSERT_EQ (packet.mtu, 0u);
    ASSERT_TRUE (packet.prefixes.empty ());
    ASSERT_TRUE (packet.rdnss.empty ());

    data =
        (std::string ("\x86\x00\x00\x00\x00\x00\x00\x00"
                      "\x00\x00\x00\x00\x00\x00\x00\x00"
                      "\x05\x00\x00\x00\x00\x00\x05\xdc",
                      24));
    ASSERT_EQ (message.deserialize (packet, data.data (), data.size ()), -1);
    ASSERT_EQ (lastError, Errc::InvalidParam) << lastError.message ();
}

/**
 * @brief Test serialize method with a neighbor solicitation.
 */
TEST (NdpMessage, serializeNeighborSolicitation)
{
    NdpMessage message;
    char data[64];

    NeighborSolicitation packet;
    ASSERT_EQ (message.serialize (packet, data, sizeof (data)), -1);
    ASSERT_EQ (lastError, Errc::InvalidParam) << lastError.message ();

    packet.target = "192.168.24.1";
    ASSERT_EQ (message.serialize (packet, data, sizeof (data)), -1);
    ASSERT_EQ (lastError, Errc::InvalidParam) << lastError.message ();

    packet.target = "ff02::1";
    ASSERT_EQ (message.serialize (packet, data, sizeof (data)), -1);
    ASSERT_EQ (lastError, Errc::InvalidParam) << lastError.message ();

    packet.target = "2001:db8::1";
    ssize_t size = message.serialize (packet, data, sizeof (data));
    ASSERT_NE (size, -1) << lastError.message ();
    ASSERT_EQ (std::string (data, size),
               std::string ("\x87\x00\x00\x00\x00\x00\x00\x00"
                            "\x20\x01\x0d\xb8\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01",
                            24));

    packet.link = "4e:ed:ed:ee:59:db";
    size = message.serialize (packet, data, sizeof (data));
    ASSERT_NE (size, -1) << lastError.message ();
    ASSERT_EQ (std::string (data, size), std::string ("\x87\x00\x00\x00\x00\x00\x00\x00"
                                                      "\x20\x01\x0d\xb8\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01"
                                                      "\x01\x01\x4e\xed\xed\xee\x59\xdb",
                                                      32));

    for (size_t length = 0; length < 32; ++length)
    {
        ASSERT_EQ (message.serialize (packet, data, length), -1) << "length " << length;
        ASSERT_EQ (lastError, Errc::MessageTooLong) << lastError.message ();
    }
}

/**
 * @brief Test serialize method with a neighbor advertisement.
 */
TEST (NdpMessage, serializeNeighborAdvertisement)
{
    NdpMessage message;
    char data[64];

    NeighborAdvertisement packet;
    ASSERT_EQ (message.serialize (packet, data, sizeof (data)), -1);
    ASSERT_EQ (lastError, Errc::InvalidParam) << lastError.message ();

    packet.target = "192.168.24.1";
    ASSERT_EQ (message.serialize (packet, data, sizeof (data)), -1);
    ASSERT_EQ (lastError, Errc::InvalidParam) << lastError.message ();

    packet.target = "ff02::1";
    ASSERT_EQ (message.serialize (packet, data, sizeof (data)), -1);
    ASSERT_EQ (lastError, Errc::InvalidParam) << lastError.message ();

    packet.target = "2001:db8::1";
    packet.flags = ND_NA_FLAG_SOLICITED | ND_NA_FLAG_OVERRIDE;
    ssize_t size = message.serialize (packet, data, sizeof (data));
    ASSERT_NE (size, -1) << lastError.message ();
    ASSERT_EQ (std::string (data, size),
               std::string ("\x88\x00\x00\x00\x60\x00\x00\x00"
                            "\x20\x01\x0d\xb8\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01",
                            24));

    packet.link = "4e:ed:ed:ee:59:db";
    size = message.serialize (packet, data, sizeof (data));
    ASSERT_NE (size, -1) << lastError.message ();
    ASSERT_EQ (std::string (data, size), std::string ("\x88\x00\x00\x00\x60\x00\x00\x00"
                                                      "\x20\x01\x0d\xb8\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01"
                                                      "\x02\x01\x4e\xed\xed\xee\x59\xdb",
                                                      32));

    size = message.serialize (packet, data, sizeof (data), "4e:ed:ed:ee:59:dc");
    ASSERT_NE (size, -1) << lastError.message ();
    ASSERT_EQ (std::string (data + 24, size - 24), std::string ("\x02\x01\x4e\xed\xed\xee\x59\xdc", 8));

    for (size_t length = 0; length < 32; ++length)
    {
        ASSERT_EQ (message.serialize (packet, data, length), -1) << "length " << length;
        ASSERT_EQ (lastError, Errc::MessageTooLong) << lastError.message ();
    }
}

/**
 * @brief Test deserialize method with a neighbor solicitation.
 */
TEST (NdpMessage, deserializeNeighborSolicitation)
{
    NdpMessage message;
    NeighborSolicitation packet;

    std::string data = std::string ("\x87\x00\x00\x00\x00\x00\x00\x00", 8);
    ASSERT_EQ (message.deserialize (packet, data.data (), data.size ()), -1);
    ASSERT_EQ (lastError, Errc::MessageTooLong) << lastError.message ();

    data = std::string (
        "\x88\x00\x00\x00\x00\x00\x00\x00"
        "\x20\x01\x0d\xb8\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01",
        24);
    ASSERT_EQ (message.deserialize (packet, data.data (), data.size ()), -1);
    ASSERT_EQ (lastError, Errc::MessageUnknown) << lastError.message ();

    data = std::string (
        "\x87\x01\x00\x00\x00\x00\x00\x00"
        "\x20\x01\x0d\xb8\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01",
        24);
    ASSERT_EQ (message.deserialize (packet, data.data (), data.size ()), -1);
    ASSERT_EQ (lastError, Errc::MessageUnknown) << lastError.message ();

    data = std::string (
        "\x87\x00\x00\x00\x00\x00\x00\x00"
        "\xff\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01",
        24);
    ASSERT_EQ (message.deserialize (packet, data.data (), data.size ()), -1);
    ASSERT_EQ (lastError, Errc::InvalidParam) << lastError.message ();

    data = std::string (
        "\x87\x00\x00\x00\x00\x00\x00\x00"
        "\x20\x01\x0d\xb8\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01"
        "\x01\x00\x4e\xed\xed\xee\x59\xdb",
        32);
    ASSERT_EQ (message.deserialize (packet, data.data (), data.size ()), -1);
    ASSERT_EQ (lastError, Errc::InvalidParam) << lastError.message ();

    data = std::string (
        "\x87\x00\x00\x00\x00\x00\x00\x00"
        "\x20\x01\x0d\xb8\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01",
        24);
    ASSERT_EQ (message.deserialize (packet, data.data (), data.size ()), 0) << lastError.message ();
    ASSERT_EQ (packet.target, IpAddress ("2001:db8::1"));
    ASSERT_TRUE (packet.link.isWildcard ());

    data = std::string (
        "\x87\x00\x00\x00\x00\x00\x00\x00"
        "\x20\x01\x0d\xb8\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01"
        "\x02\x01\x00\x00\x00\x00\x00\x00"
        "\x01\x01\x4e\xed\xed\xee\x59\xdb",
        40);
    ASSERT_EQ (message.deserialize (packet, data.data (), data.size ()), 0) << lastError.message ();
    ASSERT_EQ (packet.target, IpAddress ("2001:db8::1"));
    ASSERT_EQ (packet.link, MacAddress ("4e:ed:ed:ee:59:db"));
}

/**
 * @brief Test deserialize method with a neighbor advertisement.
 */
TEST (NdpMessage, deserializeNeighborAdvertisement)
{
    NdpMessage message;
    NeighborAdvertisement packet;

    std::string data = std::string ("\x88\x00\x00\x00\x00\x00\x00\x00", 8);
    ASSERT_EQ (message.deserialize (packet, data.data (), data.size ()), -1);
    ASSERT_EQ (lastError, Errc::MessageTooLong) << lastError.message ();

    data = std::string (
        "\x87\x00\x00\x00\x00\x00\x00\x00"
        "\x20\x01\x0d\xb8\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01",
        24);
    ASSERT_EQ (message.deserialize (packet, data.data (), data.size ()), -1);
    ASSERT_EQ (lastError, Errc::MessageUnknown) << lastError.message ();

    data = std::string (
        "\x88\x01\x00\x00\x00\x00\x00\x00"
        "\x20\x01\x0d\xb8\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01",
        24);
    ASSERT_EQ (message.deserialize (packet, data.data (), data.size ()), -1);
    ASSERT_EQ (lastError, Errc::MessageUnknown) << lastError.message ();

    data = std::string (
        "\x88\x00\x00\x00\x00\x00\x00\x00"
        "\xff\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01",
        24);
    ASSERT_EQ (message.deserialize (packet, data.data (), data.size ()), -1);
    ASSERT_EQ (lastError, Errc::InvalidParam) << lastError.message ();

    data = std::string (
        "\x88\x00\x00\x00\x00\x00\x00\x00"
        "\x20\x01\x0d\xb8\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01"
        "\x02\x00\x4e\xed\xed\xee\x59\xdb",
        32);
    ASSERT_EQ (message.deserialize (packet, data.data (), data.size ()), -1);
    ASSERT_EQ (lastError, Errc::InvalidParam) << lastError.message ();

    data = std::string (
        "\x88\x00\x00\x00\xe0\x00\x00\x00"
        "\x20\x01\x0d\xb8\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01",
        24);
    ASSERT_EQ (message.deserialize (packet, data.data (), data.size ()), 0) << lastError.message ();
    ASSERT_EQ (packet.flags, ND_NA_FLAG_ROUTER | ND_NA_FLAG_SOLICITED | ND_NA_FLAG_OVERRIDE);
    ASSERT_EQ (packet.target, IpAddress ("2001:db8::1"));
    ASSERT_TRUE (packet.link.isWildcard ());

    data = std::string (
        "\x88\x00\x00\x00\x40\x00\x00\x00"
        "\x20\x01\x0d\xb8\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01"
        "\x01\x01\x00\x00\x00\x00\x00\x00"
        "\x02\x01\x4e\xed\xed\xee\x59\xdb",
        40);
    ASSERT_EQ (message.deserialize (packet, data.data (), data.size ()), 0) << lastError.message ();
    ASSERT_EQ (packet.flags, ND_NA_FLAG_SOLICITED);
    ASSERT_EQ (packet.link, MacAddress ("4e:ed:ed:ee:59:db"));
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
