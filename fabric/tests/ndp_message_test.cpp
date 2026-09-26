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
 * @brief make a byte stream reading the given bytes.
 * @param wire bytes to read.
 */
static std::stringstream streamOf (const std::string& wire)
{
    return std::stringstream (wire);
}

/**
 * @brief Test serialize method with a router solicitation.
 */
TEST (NdpMessage, serializeSolicitation)
{
    NdpMessage message;
    std::stringstream data;

    RouterSolicitation packet;
    ASSERT_EQ (message.serialize (packet, data), 0) << lastError.message ();
    ASSERT_EQ (data.str (), std::string ("\x85\x00\x00\x00\x00\x00\x00\x00", 8));

    data.str ("");
    data.clear ();

    packet.link = "4e:ed:ed:ee:59:db";
    ASSERT_EQ (message.serialize (packet, data), 0) << lastError.message ();
    ASSERT_EQ (data.str (), std::string ("\x85\x00\x00\x00\x00\x00\x00\x00"
                                         "\x01\x01\x4e\xed\xed\xee\x59\xdb",
                                         16));
}

/**
 * @brief Test serialize method with a router advertisement.
 */
TEST (NdpMessage, serializeAdvertisement)
{
    NdpMessage message;
    std::stringstream data;

    RouterAdvertisement bad = sample ();
    bad.prefixes[0].prefix = "192.168.1.0";
    ASSERT_EQ (message.serialize (bad, data), -1);
    ASSERT_EQ (lastError, Errc::InvalidParam) << lastError.message ();

    bad = sample ();
    bad.prefixes[0].length = 129;
    ASSERT_EQ (message.serialize (bad, data), -1);
    ASSERT_EQ (lastError, Errc::InvalidParam) << lastError.message ();

    bad = sample ();
    bad.rdnss[1].servers.push_back ("192.168.1.53");
    ASSERT_EQ (message.serialize (bad, data), -1);
    ASSERT_EQ (lastError, Errc::InvalidParam) << lastError.message ();

    bad = sample ();
    bad.rdnss[1].servers.clear ();
    ASSERT_EQ (message.serialize (bad, data), -1);
    ASSERT_EQ (lastError, Errc::InvalidParam) << lastError.message ();

    bad = sample ();
    bad.rdnss[1].servers.assign (128, IpAddress ("2001:db8::53"));
    ASSERT_EQ (message.serialize (bad, data), -1);
    ASSERT_EQ (lastError, Errc::MessageTooLong) << lastError.message ();

    data.str ("");
    data.clear ();

    RouterAdvertisement minimal;
    ASSERT_EQ (message.serialize (minimal, data), 0) << lastError.message ();
    ASSERT_EQ (data.str (), std::string ("\x86\x00\x00\x00\x00\x00\x00\x00"
                                         "\x00\x00\x00\x00\x00\x00\x00\x00",
                                         16));

    data.str ("");
    data.clear ();

    ASSERT_EQ (message.serialize (sample (), data), 0) << lastError.message ();

    const std::string wire = data.str ();
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

    RouterAdvertisement host = sample ();
    host.prefixes[0].prefix = "2001:db8:1::ffff";

    data.str ("");
    data.clear ();

    ASSERT_EQ (message.serialize (host, data), 0) << lastError.message ();
    ASSERT_EQ (data.str ().substr (48, 16), std::string ("\x20\x01\x0d\xb8\x00\x01\x00\x00"
                                                         "\x00\x00\x00\x00\x00\x00\x00\x00",
                                                         16));
}

/**
 * @brief Test deserialize method with a router solicitation.
 */
TEST (NdpMessage, deserializeSolicitation)
{
    NdpMessage message;
    RouterSolicitation packet;

    std::stringstream data = streamOf (std::string ("\x85\x00\x00\x00\x00\x00", 6));
    ASSERT_EQ (message.deserialize (packet, data), -1);
    ASSERT_EQ (lastError, Errc::MessageTooLong) << lastError.message ();

    data = streamOf (std::string ("\x86\x00\x00\x00\x00\x00\x00\x00", 8));
    ASSERT_EQ (message.deserialize (packet, data), -1);
    ASSERT_EQ (lastError, Errc::MessageUnknown) << lastError.message ();

    data = streamOf (std::string ("\x85\x01\x00\x00\x00\x00\x00\x00", 8));
    ASSERT_EQ (message.deserialize (packet, data), -1);
    ASSERT_EQ (lastError, Errc::MessageUnknown) << lastError.message ();

    data =
        streamOf (std::string ("\x85\x00\x00\x00\x00\x00\x00\x00"
                               "\x01\x00\x4e\xed\xed\xee\x59\xdb",
                               16));
    ASSERT_EQ (message.deserialize (packet, data), -1);
    ASSERT_EQ (lastError, Errc::InvalidParam) << lastError.message ();

    data = streamOf (std::string ("\x85\x00\x00\x00\x00\x00\x00\x00\x01", 9));
    ASSERT_EQ (message.deserialize (packet, data), -1);
    ASSERT_EQ (lastError, Errc::InvalidParam) << lastError.message ();

    data =
        streamOf (std::string ("\x85\x00\x00\x00\x00\x00\x00\x00"
                               "\x01\x02\x4e\xed\xed\xee\x59\xdb",
                               16));
    ASSERT_EQ (message.deserialize (packet, data), -1);
    ASSERT_EQ (lastError, Errc::MessageTooLong) << lastError.message ();

    data = streamOf (std::string ("\x85\x00\x00\x00\x00\x00\x00\x00", 8));
    ASSERT_EQ (message.deserialize (packet, data), 0) << lastError.message ();
    ASSERT_TRUE (packet.link.isWildcard ());

    data =
        streamOf (std::string ("\x85\x00\x00\x00\x00\x00\x00\x00"
                               "\x0e\x01\x00\x00\x00\x00\x00\x00"
                               "\x01\x01\x4e\xed\xed\xee\x59\xdb",
                               24));
    ASSERT_EQ (message.deserialize (packet, data), 0) << lastError.message ();
    ASSERT_EQ (packet.link, MacAddress ("4e:ed:ed:ee:59:db"));
}

/**
 * @brief Test deserialize method with a router advertisement.
 */
TEST (NdpMessage, deserializeAdvertisement)
{
    NdpMessage message;
    RouterAdvertisement packet;

    std::stringstream data = streamOf (std::string ("\x86\x00\x00\x00\x00\x00\x00\x00", 8));
    ASSERT_EQ (message.deserialize (packet, data), -1);
    ASSERT_EQ (lastError, Errc::MessageTooLong) << lastError.message ();

    data =
        streamOf (std::string ("\x85\x00\x00\x00\x00\x00\x00\x00"
                               "\x00\x00\x00\x00\x00\x00\x00\x00",
                               16));
    ASSERT_EQ (message.deserialize (packet, data), -1);
    ASSERT_EQ (lastError, Errc::MessageUnknown) << lastError.message ();

    data =
        streamOf (std::string ("\x86\x01\x00\x00\x00\x00\x00\x00"
                               "\x00\x00\x00\x00\x00\x00\x00\x00",
                               16));
    ASSERT_EQ (message.deserialize (packet, data), -1);
    ASSERT_EQ (lastError, Errc::MessageUnknown) << lastError.message ();

    std::stringstream wire;
    ASSERT_EQ (message.serialize (sample (), wire), 0) << lastError.message ();

    data = streamOf (wire.str ());
    ASSERT_EQ (message.deserialize (packet, data), 0) << lastError.message ();

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
        streamOf (std::string ("\x86\x00\x00\x00\x00\x00\x00\x00"
                               "\x00\x00\x00\x00\x00\x00\x00\x00",
                               16));
    ASSERT_EQ (message.deserialize (packet, data), 0) << lastError.message ();
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

    data = streamOf (ignored);
    ASSERT_EQ (message.deserialize (packet, data), 0) << lastError.message ();
    ASSERT_EQ (packet.mtu, 0u);
    ASSERT_TRUE (packet.prefixes.empty ());
    ASSERT_TRUE (packet.rdnss.empty ());

    data =
        streamOf (std::string ("\x86\x00\x00\x00\x00\x00\x00\x00"
                               "\x00\x00\x00\x00\x00\x00\x00\x00"
                               "\x05\x00\x00\x00\x00\x00\x05\xdc",
                               24));
    ASSERT_EQ (message.deserialize (packet, data), -1);
    ASSERT_EQ (lastError, Errc::InvalidParam) << lastError.message ();
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
