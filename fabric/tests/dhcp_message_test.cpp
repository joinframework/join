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
#include <join/dhcp_message.hpp>
#include <join/error.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::lastError;
using join::Errc;
using join::IpAddress;
using join::MacAddress;
using join::ByteList;
using join::WordList;
using join::IpList;
using join::DhcpOption;
using join::DhcpPacket;
using join::DhcpMessage;

static constexpr size_t headerSize = DhcpMessage::headerSize;

/// smallest message the codec emits, copied to keep the assertions from odr-using it.
static constexpr size_t minMsgSize = DhcpMessage::minMsgSize;
static constexpr uint32_t magicCookie = DhcpMessage::magicCookie;

/**
 * @brief build a message exercising every option type.
 */
static DhcpPacket sample ()
{
    DhcpPacket packet;

    packet.op = DhcpMessage::BootReply;
    packet.hardware = "50:7b:9d:13:82:df";
    packet.id = 0x12345678;
    packet.secs = 42;
    packet.flags = DhcpMessage::BroadcastFlag;
    packet.client = "192.168.16.110";
    packet.your = "192.168.16.111";
    packet.server = "192.168.16.100";
    packet.gateway = "192.168.16.1";

    packet.options.insert (DhcpOption::DhcpMessageType, static_cast<uint8_t> (DhcpMessage::Ack));
    packet.options.insert (DhcpOption::InterfaceMtu, 1500);
    packet.options.insert (DhcpOption::IpAddressLeaseTime, 86400);
    packet.options.insert (DhcpOption::TimeOffset, -3600);
    packet.options.insert (DhcpOption::DomainName, "foo.local");
    packet.options.insert (DhcpOption::SubnetMask, "255.255.255.0");
    packet.options.insert (DhcpOption::ClientIdentifier, "50:7b:9d:13:82:df");
    packet.options.insert (DhcpOption::ParameterRequestList, ByteList{DhcpOption::SubnetMask, DhcpOption::Router});
    packet.options.insert (DhcpOption::PathMtuPlateauTable, WordList{296, 1006, 1500});
    packet.options.insert (DhcpOption::DomainNameServer, IpList{"192.168.16.249", "192.168.16.250"});

    return packet;
}

/**
 * @brief Test serialize method.
 */
TEST (DhcpMessage, serialize)
{
    DhcpMessage message;
    std::stringstream data;

    DhcpPacket bad = sample ();
    bad.op = 0;
    ASSERT_EQ (message.serialize (bad, data), -1);
    ASSERT_EQ (lastError, Errc::InvalidParam) << lastError.message ();

    DhcpPacket big = sample ();
    big.options.erase (DhcpOption::DomainName);
    ASSERT_TRUE (big.options.insert (DhcpOption::DomainName, std::string (256, 'a')));
    ASSERT_EQ (message.serialize (big, data), -1);
    ASSERT_EQ (lastError, Errc::MessageTooLong) << lastError.message ();

    DhcpPacket bytes = sample ();
    bytes.options.erase (DhcpOption::ParameterRequestList);
    ASSERT_TRUE (bytes.options.insert (DhcpOption::ParameterRequestList, ByteList (256, DhcpOption::SubnetMask)));
    ASSERT_EQ (message.serialize (bytes, data), -1);
    ASSERT_EQ (lastError, Errc::MessageTooLong) << lastError.message ();

    DhcpPacket words = sample ();
    words.options.erase (DhcpOption::PathMtuPlateauTable);
    ASSERT_TRUE (words.options.insert (DhcpOption::PathMtuPlateauTable, WordList (128, 1500)));
    ASSERT_EQ (message.serialize (words, data), -1);
    ASSERT_EQ (lastError, Errc::MessageTooLong) << lastError.message ();

    DhcpPacket addresses = sample ();
    addresses.options.erase (DhcpOption::DomainNameServer);
    ASSERT_TRUE (addresses.options.insert (DhcpOption::DomainNameServer, IpList (64, IpAddress ("192.168.16.249"))));
    ASSERT_EQ (message.serialize (addresses, data), -1);
    ASSERT_EQ (lastError, Errc::MessageTooLong) << lastError.message ();

    data.str ("");
    data.clear ();

    DhcpPacket packet = sample ();
    ASSERT_EQ (message.serialize (packet, data), 0) << lastError.message ();

    const std::string wire = data.str ();
    ASSERT_GT (wire.size (), headerSize);

    ASSERT_EQ (static_cast<uint8_t> (wire[0]), DhcpMessage::BootReply);
    ASSERT_EQ (static_cast<uint8_t> (wire[1]), 1);
    ASSERT_EQ (static_cast<uint8_t> (wire[2]), ETH_ALEN);
    ASSERT_EQ (static_cast<uint8_t> (wire[3]), 0);

    uint32_t cookie = 0;
    ::memcpy (&cookie, wire.data () + headerSize - sizeof (cookie), sizeof (cookie));
    ASSERT_EQ (ntohl (cookie), magicCookie);

    ASSERT_GE (wire.size (), minMsgSize);
    ASSERT_EQ (static_cast<uint8_t> (wire[wire.find_last_not_of ('\0')]), DhcpOption::End);
}

/**
 * @brief Test deserialize method.
 */
TEST (DhcpMessage, deserialize)
{
    DhcpMessage message;

    DhcpPacket packet = sample ();
    std::stringstream data;
    ASSERT_EQ (message.serialize (packet, data), 0) << lastError.message ();

    DhcpPacket decoded;
    ASSERT_EQ (message.deserialize (decoded, data), 0) << lastError.message ();

    ASSERT_EQ (decoded.op, packet.op);
    ASSERT_EQ (decoded.hardware, packet.hardware);
    ASSERT_EQ (decoded.id, packet.id);
    ASSERT_EQ (decoded.secs, packet.secs);
    ASSERT_EQ (decoded.flags, packet.flags);
    ASSERT_EQ (decoded.client, packet.client);
    ASSERT_EQ (decoded.your, packet.your);
    ASSERT_EQ (decoded.server, packet.server);
    ASSERT_EQ (decoded.gateway, packet.gateway);
    ASSERT_EQ (decoded.options, packet.options);

    DhcpPacket ignored;
    for (size_t length : {size_t (2), size_t (6), size_t (20), size_t (35), size_t (100), headerSize - 1})
    {
        std::stringstream truncated (data.str ().substr (0, length));
        ASSERT_EQ (message.deserialize (ignored, truncated), -1) << "length " << length;
        ASSERT_EQ (lastError, Errc::MessageTooLong) << lastError.message ();
    }

    std::string wire = data.str ();
    wire[headerSize - 1] = 0;
    std::stringstream corrupted (wire);
    ASSERT_EQ (message.deserialize (ignored, corrupted), -1);
    ASSERT_EQ (lastError, Errc::InvalidParam) << lastError.message ();

    wire = data.str ();
    wire[0] = 0;
    std::stringstream unknown (wire);
    ASSERT_EQ (message.deserialize (ignored, unknown), -1);
    ASSERT_EQ (lastError, Errc::InvalidParam) << lastError.message ();

    DhcpPacket bare;
    bare.op = DhcpMessage::BootRequest;
    bare.hardware = "50:7b:9d:13:82:df";
    bare.options.insert (DhcpOption::DhcpMessageType, static_cast<uint8_t> (DhcpMessage::Discover));

    std::stringstream head;
    ASSERT_EQ (message.serialize (bare, head), 0) << lastError.message ();

    const std::string bareWire = head.str ();
    std::string prefix = bareWire.substr (0, bareWire.find_last_not_of ('\0'));
    const uint8_t trailer[] = {DhcpOption::Pad,
                               DhcpOption::PolicyFilter,
                               4,
                               1,
                               2,
                               3,
                               4,
                               DhcpOption::InterfaceMtu,
                               1,
                               5,
                               DhcpOption::SubnetMask,
                               4,
                               255,
                               255,
                               255,
                               0,
                               DhcpOption::End};
    prefix.append (reinterpret_cast<const char*> (trailer), sizeof (trailer));

    std::stringstream mixed (prefix);
    DhcpPacket lenient;
    ASSERT_EQ (message.deserialize (lenient, mixed), 0) << lastError.message ();
    ASSERT_TRUE (lenient.options.contains (DhcpOption::DhcpMessageType));
    ASSERT_FALSE (lenient.options.contains (DhcpOption::PolicyFilter));
    ASSERT_FALSE (lenient.options.contains (DhcpOption::InterfaceMtu));
    ASSERT_TRUE (lenient.options.contains (DhcpOption::SubnetMask));
    ASSERT_EQ (*lenient.options.getIf<IpAddress> (DhcpOption::SubnetMask), "255.255.255.0");

    std::string clipped = bareWire.substr (0, bareWire.find_last_not_of ('\0'));
    clipped.push_back (static_cast<char> (DhcpOption::DomainName));
    std::stringstream headless (clipped);
    DhcpPacket short1;
    ASSERT_EQ (message.deserialize (short1, headless), -1);
    ASSERT_EQ (lastError, Errc::MessageTooLong) << lastError.message ();

    clipped.push_back (4);
    clipped.push_back (1);
    std::stringstream starved (clipped);
    DhcpPacket short2;
    ASSERT_EQ (message.deserialize (short2, starved), -1);
    ASSERT_EQ (lastError, Errc::MessageTooLong) << lastError.message ();

    DhcpPacket overloaded;
    overloaded.op = DhcpMessage::BootReply;
    overloaded.hardware = "50:7b:9d:13:82:df";
    overloaded.options.insert (DhcpOption::DhcpMessageType, static_cast<uint8_t> (DhcpMessage::Ack));
    overloaded.options.insert (DhcpOption::OptionOverload,
                               static_cast<uint8_t> (DhcpMessage::FileOverload | DhcpMessage::SnameOverload));

    std::stringstream carrier;
    ASSERT_EQ (message.serialize (overloaded, carrier), 0) << lastError.message ();

    std::string frame = carrier.str ();
    const uint8_t sname[] = {DhcpOption::Router, 4, 192, 168, 16, 254, DhcpOption::End};
    const uint8_t file[] = {DhcpOption::DomainName, 9, 'f', 'o', 'o', '.', 'l', 'o', 'c', 'a', 'l', DhcpOption::End};
    ::memcpy (&frame[44], sname, sizeof (sname));
    ::memcpy (&frame[108], file, sizeof (file));

    std::stringstream spilled (frame);
    DhcpPacket recovered;
    ASSERT_EQ (message.deserialize (recovered, spilled), 0) << lastError.message ();
    ASSERT_EQ (*recovered.options.getIf<IpList> (DhcpOption::Router), IpList{"192.168.16.254"});
    ASSERT_EQ (*recovered.options.getIf<std::string> (DhcpOption::DomainName), "foo.local");

    DhcpPacket reused = sample ();
    std::stringstream again;
    ASSERT_EQ (message.serialize (bare, again), 0) << lastError.message ();
    ASSERT_EQ (message.deserialize (reused, again), 0) << lastError.message ();
    ASSERT_EQ (reused.options.size (), 1);
    ASSERT_TRUE (reused.options.contains (DhcpOption::DhcpMessageType));

    DhcpPacket spoiled;
    spoiled.op = DhcpMessage::BootReply;
    spoiled.hardware = "50:7b:9d:13:82:df";
    spoiled.options.insert (DhcpOption::DhcpMessageType, static_cast<uint8_t> (DhcpMessage::Ack));
    spoiled.options.insert (DhcpOption::OptionOverload, static_cast<uint8_t> (DhcpMessage::FileOverload));

    std::stringstream broken;
    ASSERT_EQ (message.serialize (spoiled, broken), 0) << lastError.message ();

    std::string cut = broken.str ();
    cut[108 + 126] = static_cast<char> (DhcpOption::DomainName);
    cut[108 + 127] = 9;

    std::stringstream clippedFile (cut);
    DhcpPacket lost;
    ASSERT_EQ (message.deserialize (lost, clippedFile), -1);
    ASSERT_EQ (lastError, Errc::MessageTooLong) << lastError.message ();

    spoiled.options.erase (DhcpOption::OptionOverload);
    spoiled.options.insert (DhcpOption::OptionOverload, static_cast<uint8_t> (DhcpMessage::SnameOverload));

    std::stringstream other;
    ASSERT_EQ (message.serialize (spoiled, other), 0) << lastError.message ();

    cut = other.str ();
    cut[44 + 62] = static_cast<char> (DhcpOption::Router);
    cut[44 + 63] = 4;

    std::stringstream clippedSname (cut);
    ASSERT_EQ (message.deserialize (lost, clippedSname), -1);
    ASSERT_EQ (lastError, Errc::MessageTooLong) << lastError.message ();
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
