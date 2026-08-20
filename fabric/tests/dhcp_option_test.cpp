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
#include <join/dhcp_option.hpp>
#include <join/error.hpp>

// Libraries.
#include <gtest/gtest.h>

// C++.
#include <limits>

using join::lastError;
using join::Errc;
using join::IpAddress;
using join::MacAddress;
using join::ByteList;
using join::WordList;
using join::IpList;
using join::DhcpOptionValue;
using join::DhcpOption;

/// a DHCP message type value, the container carries it without interpreting it.
static constexpr uint8_t _request = 3;

/**
 * @brief build a list holding one option of every supported type.
 */
static DhcpOption sample ()
{
    DhcpOption options;

    options.insert (DhcpOption::DhcpMessageType, _request);
    options.insert (DhcpOption::ClientIdentifier, "50:7b:9d:13:82:df");
    options.insert (DhcpOption::ParameterRequestList,
                    ByteList{DhcpOption::SubnetMask, DhcpOption::BroadcastAddress, DhcpOption::DomainNameServer,
                             DhcpOption::InterfaceMtu, DhcpOption::Router});
    options.insert (DhcpOption::MaximumDhcpMessageSize, 576);
    options.insert (DhcpOption::RequestedIpAddress, "192.168.0.247");
    options.insert (DhcpOption::HostName, "foobar");

    return options;
}

/**
 * @brief Test create method.
 */
TEST (DhcpOption, create)
{
    DhcpOption options;
    ASSERT_TRUE (options.empty ());

    DhcpOption filled = sample ();
    ASSERT_EQ (filled.size (), 6);

    DhcpOption range (filled.begin (), filled.end ());
    ASSERT_EQ (range.size (), 6);

    DhcpOption copied (filled);
    ASSERT_EQ (copied, filled);

    DhcpOption assigned;
    assigned = filled;
    ASSERT_EQ (assigned, filled);

    DhcpOption moved (std::move (copied));
    ASSERT_EQ (moved, filled);

    DhcpOption movedTo;
    movedTo = std::move (moved);
    ASSERT_EQ (movedTo, filled);
}

/**
 * @brief Test at method.
 */
TEST (DhcpOption, at)
{
    DhcpOption options = sample ();

    ASSERT_EQ (options.at (DhcpOption::DhcpMessageType).get<uint8_t> (), _request);
    ASSERT_EQ (options.at (DhcpOption::ClientIdentifier).get<MacAddress> (), "50:7b:9d:13:82:df");
    ASSERT_EQ (options.at (DhcpOption::MaximumDhcpMessageSize).get<uint16_t> (), 576);
    ASSERT_EQ (options.at (DhcpOption::RequestedIpAddress).get<IpAddress> (), "192.168.0.247");
    ASSERT_EQ (options.at (DhcpOption::HostName).get<std::string> (), "foobar");

    ASSERT_THROW (options.at (DhcpOption::Router), std::out_of_range);
}

/**
 * @brief Test getIf method.
 */
TEST (DhcpOption, getIf)
{
    DhcpOption options = sample ();

    ASSERT_NE (options.getIf<MacAddress> (DhcpOption::ClientIdentifier), nullptr);
    ASSERT_EQ (*options.getIf<MacAddress> (DhcpOption::ClientIdentifier), "50:7b:9d:13:82:df");

    ASSERT_EQ (options.getIf<IpAddress> (DhcpOption::Router), nullptr);

    ASSERT_EQ (options.getIf<uint32_t> (DhcpOption::MaximumDhcpMessageSize), nullptr);
}

/**
 * @brief Test find method.
 */
TEST (DhcpOption, find)
{
    DhcpOption options = sample ();

    ASSERT_NE (options.find (DhcpOption::DhcpMessageType), options.end ());
    ASSERT_NE (options.find (DhcpOption::HostName), options.end ());
    ASSERT_EQ (options.find (DhcpOption::IpForwarding), options.end ());
    ASSERT_EQ (options.find (DhcpOption::StaticRoute), options.end ());
}

/**
 * @brief Test contains method.
 */
TEST (DhcpOption, contains)
{
    DhcpOption options = sample ();

    ASSERT_TRUE (options.contains (DhcpOption::DhcpMessageType));
    ASSERT_TRUE (options.contains (DhcpOption::ClientIdentifier));
    ASSERT_TRUE (options.contains (DhcpOption::ParameterRequestList));
    ASSERT_TRUE (options.contains (DhcpOption::MaximumDhcpMessageSize));
    ASSERT_TRUE (options.contains (DhcpOption::RequestedIpAddress));
    ASSERT_TRUE (options.contains (DhcpOption::HostName));
    ASSERT_FALSE (options.contains (DhcpOption::IpForwarding));
}

/**
 * @brief Test begin method.
 */
TEST (DhcpOption, begin)
{
    DhcpOption options;
    ASSERT_EQ (options.begin (), options.end ());

    ASSERT_TRUE (options.insert (DhcpOption::DhcpMessageType, _request));
    ASSERT_NE (options.begin (), options.end ());
    ASSERT_EQ (options.begin ()->first, DhcpOption::DhcpMessageType);
}

/**
 * @brief Test cbegin method.
 */
TEST (DhcpOption, cbegin)
{
    DhcpOption options;
    ASSERT_EQ (options.cbegin (), options.cend ());

    ASSERT_TRUE (options.insert (DhcpOption::HostName, "foobar"));
    ASSERT_NE (options.cbegin (), options.cend ());
    ASSERT_EQ (options.cbegin ()->first, DhcpOption::HostName);
}

/**
 * @brief Test end method.
 */
TEST (DhcpOption, end)
{
    DhcpOption options = sample ();

    size_t count = 0;
    for (auto it = options.begin (); it != options.end (); ++it)
    {
        ++count;
    }

    ASSERT_EQ (count, options.size ());
}

/**
 * @brief Test cend method.
 */
TEST (DhcpOption, cend)
{
    DhcpOption options = sample ();

    size_t count = 0;
    for (auto it = options.cbegin (); it != options.cend (); ++it)
    {
        ++count;
    }

    ASSERT_EQ (count, options.size ());
}

/**
 * @brief Test empty method.
 */
TEST (DhcpOption, empty)
{
    DhcpOption options;
    ASSERT_TRUE (options.empty ());

    ASSERT_TRUE (options.insert (DhcpOption::HostName, "foobar"));
    ASSERT_FALSE (options.empty ());
}

/**
 * @brief Test size method.
 */
TEST (DhcpOption, size)
{
    DhcpOption options;
    ASSERT_EQ (options.size (), 0);

    ASSERT_TRUE (options.insert (DhcpOption::HostName, "foobar"));
    ASSERT_TRUE (options.insert (DhcpOption::DomainName, "foo.local"));
    ASSERT_EQ (options.size (), 2);
}

/**
 * @brief Test erase method.
 */
TEST (DhcpOption, erase)
{
    DhcpOption options = sample ();

    ASSERT_EQ (options.erase (DhcpOption::StaticRoute), 0);
    ASSERT_EQ (options.size (), 6);

    ASSERT_EQ (options.erase (DhcpOption::DhcpMessageType), 1);
    ASSERT_EQ (options.erase (DhcpOption::HostName), 1);
    ASSERT_EQ (options.size (), 4);

    ASSERT_FALSE (options.contains (DhcpOption::DhcpMessageType));
    ASSERT_FALSE (options.contains (DhcpOption::HostName));
}

/**
 * @brief Test clear method.
 */
TEST (DhcpOption, clear)
{
    DhcpOption options = sample ();
    ASSERT_EQ (options.size (), 6);

    options.clear ();
    ASSERT_EQ (options.size (), 0);
    ASSERT_TRUE (options.empty ());
}

/**
 * @brief Test insert method.
 */
TEST (DhcpOption, insert)
{
    DhcpOption options;

    ASSERT_TRUE (options.insert (DhcpOption::DhcpMessageType, _request));
    ASSERT_TRUE (options.insert (DhcpOption::InterfaceMtu, 1500));
    ASSERT_TRUE (options.insert (DhcpOption::IpAddressLeaseTime, 86400));
    ASSERT_TRUE (options.insert (DhcpOption::TimeOffset, -3600));
    ASSERT_TRUE (options.insert (DhcpOption::DomainName, std::string ("foo.local")));
    ASSERT_TRUE (options.insert (DhcpOption::SubnetMask, "255.255.255.0"));
    ASSERT_TRUE (options.insert (DhcpOption::BroadcastAddress, IpAddress ("192.168.0.255")));
    ASSERT_TRUE (options.insert (DhcpOption::ClientIdentifier, MacAddress ("50:7b:9d:13:82:df")));
    ASSERT_TRUE (options.insert (DhcpOption::ParameterRequestList, ByteList{DhcpOption::SubnetMask}));
    ASSERT_TRUE (options.insert (DhcpOption::PathMtuPlateauTable, WordList{296, 1006}));
    ASSERT_TRUE (options.insert (DhcpOption::DomainNameServer, IpList{"192.168.0.249"}));

    ASSERT_EQ (options.at (DhcpOption::InterfaceMtu).get<uint16_t> (), 1500);
    ASSERT_EQ (options.at (DhcpOption::IpAddressLeaseTime).get<uint32_t> (), 86400u);
    ASSERT_EQ (options.at (DhcpOption::TimeOffset).get<int32_t> (), -3600);
    ASSERT_EQ (options.at (DhcpOption::SubnetMask).get<IpAddress> (), "255.255.255.0");
    ASSERT_EQ (options.at (DhcpOption::ClientIdentifier).get<MacAddress> (), "50:7b:9d:13:82:df");

    ASSERT_FALSE (options.insert (DhcpOption::InterfaceMtu, 1400));
    ASSERT_EQ (lastError, Errc::InUse) << lastError.message ();

    ASSERT_FALSE (options.insert (DhcpOption::DefaultIpTimeToLive, 0));
    ASSERT_EQ (lastError, Errc::InvalidParam) << lastError.message ();

    DhcpOption other;
    ASSERT_TRUE (other.insert (DhcpOption::InterfaceMtu, 1400));
    ASSERT_TRUE (other.insert (DhcpOption::HostName, "foobar"));

    options.insert (other.begin (), other.end ());
    ASSERT_EQ (options.at (DhcpOption::InterfaceMtu).get<uint16_t> (), 1500);
    ASSERT_EQ (options.at (DhcpOption::HostName).get<std::string> (), "foobar");
}

/**
 * @brief Test swap method.
 */
TEST (DhcpOption, swap)
{
    DhcpOption options = sample (), other;

    ASSERT_TRUE (other.insert (DhcpOption::DomainName, "foo.local"));

    options.swap (other);

    ASSERT_EQ (options.size (), 1);
    ASSERT_TRUE (options.contains (DhcpOption::DomainName));
    ASSERT_EQ (other.size (), 6);
    ASSERT_TRUE (other.contains (DhcpOption::DhcpMessageType));
}

/**
 * @brief Test dump method.
 */
TEST (DhcpOption, dump)
{
    ASSERT_STREQ (sample ().dump ().c_str (),
                  "HostName (12): \"foobar\"\n"
                  "RequestedIpAddress (50): \"192.168.0.247\"\n"
                  "DhcpMessageType (53): 3\n"
                  "ParameterRequestList (55): [1,28,6,26,3]\n"
                  "MaximumDhcpMessageSize (57): 576\n"
                  "ClientIdentifier (61): \"50:7b:9d:13:82:df\"\n");

    DhcpOption types;
    ASSERT_TRUE (types.insert (DhcpOption::IpForwarding, 1));
    ASSERT_TRUE (types.insert (DhcpOption::TimeOffset, -3600));
    ASSERT_TRUE (types.insert (DhcpOption::InterfaceMtu, 1500));
    ASSERT_TRUE (types.insert (DhcpOption::IpAddressLeaseTime, 86400));
    ASSERT_TRUE (types.insert (DhcpOption::DomainName, "foo.local"));
    ASSERT_TRUE (types.insert (DhcpOption::SubnetMask, "255.255.255.0"));
    ASSERT_TRUE (types.insert (DhcpOption::ClientIdentifier, "50:7b:9d:13:82:df"));
    ASSERT_TRUE (types.insert (DhcpOption::ParameterRequestList, ByteList{1, 3}));
    ASSERT_TRUE (types.insert (DhcpOption::PathMtuPlateauTable, WordList{296, 1006}));
    ASSERT_TRUE (types.insert (DhcpOption::DomainNameServer, IpList{"192.168.0.249", "192.168.0.250"}));

    ASSERT_STREQ (types.dump ().c_str (),
                  "SubnetMask (1): \"255.255.255.0\"\n"
                  "TimeOffset (2): -3600\n"
                  "DomainNameServer (6): [\"192.168.0.249\",\"192.168.0.250\"]\n"
                  "DomainName (15): \"foo.local\"\n"
                  "IpForwarding (19): 1\n"
                  "PathMtuPlateauTable (25): [296,1006]\n"
                  "InterfaceMtu (26): 1500\n"
                  "IpAddressLeaseTime (51): 86400\n"
                  "ParameterRequestList (55): [1,3]\n"
                  "ClientIdentifier (61): \"50:7b:9d:13:82:df\"\n");

    std::stringstream stream;
    stream << sample ();
    ASSERT_EQ (stream.str (), sample ().dump ());
}

/**
 * @brief Test isValid method.
 */
TEST (DhcpOption, isValid)
{
    ASSERT_FALSE (DhcpOption::isValid (DhcpOption::PolicyFilter, 1));
    ASSERT_FALSE (DhcpOption::isValid (DhcpOption::StaticRoute, "192.168.0.254"));
    ASSERT_FALSE (DhcpOption::isValid (DhcpOption::StaticRoute, IpAddress ("192.168.0.254")));
    ASSERT_FALSE (DhcpOption::isValid (DhcpOption::StaticRoute, MacAddress ("50:7b:9d:13:82:df")));
    ASSERT_FALSE (DhcpOption::isValid (DhcpOption::StaticRoute, ByteList{1}));
    ASSERT_FALSE (DhcpOption::isValid (DhcpOption::StaticRoute, WordList{296}));
    ASSERT_FALSE (DhcpOption::isValid (DhcpOption::StaticRoute, IpList{"192.168.0.254"}));
    ASSERT_EQ (lastError, Errc::InvalidParam) << lastError.message ();

    ASSERT_TRUE (DhcpOption::isValid (DhcpOption::IpForwarding, 0));
    ASSERT_TRUE (DhcpOption::isValid (DhcpOption::IpForwarding, 1));
    ASSERT_FALSE (DhcpOption::isValid (DhcpOption::IpForwarding, 2));
    ASSERT_FALSE (DhcpOption::isValid (DhcpOption::IpForwarding, std::numeric_limits<uint16_t>::max ()));
    ASSERT_FALSE (DhcpOption::isValid (DhcpOption::DefaultIpTimeToLive, 0));
    ASSERT_TRUE (DhcpOption::isValid (DhcpOption::DefaultIpTimeToLive, std::numeric_limits<uint8_t>::max ()));

    ASSERT_TRUE (DhcpOption::isValid (DhcpOption::NetbiosNodeType, 1));
    ASSERT_TRUE (DhcpOption::isValid (DhcpOption::NetbiosNodeType, 2));
    ASSERT_TRUE (DhcpOption::isValid (DhcpOption::NetbiosNodeType, 4));
    ASSERT_TRUE (DhcpOption::isValid (DhcpOption::NetbiosNodeType, 8));
    ASSERT_FALSE (DhcpOption::isValid (DhcpOption::NetbiosNodeType, 3));
    ASSERT_FALSE (DhcpOption::isValid (DhcpOption::NetbiosNodeType, 0));
    ASSERT_FALSE (DhcpOption::isValid (DhcpOption::NetbiosNodeType, std::numeric_limits<uint8_t>::max ()));

    ASSERT_FALSE (DhcpOption::isValid (DhcpOption::InterfaceMtu, 67));
    ASSERT_TRUE (DhcpOption::isValid (DhcpOption::InterfaceMtu, 68));
    ASSERT_TRUE (DhcpOption::isValid (DhcpOption::InterfaceMtu, std::numeric_limits<uint16_t>::max ()));
    ASSERT_FALSE (DhcpOption::isValid (DhcpOption::InterfaceMtu, std::numeric_limits<int32_t>::max ()));
    ASSERT_FALSE (DhcpOption::isValid (DhcpOption::MaximumDhcpMessageSize, 575));
    ASSERT_TRUE (DhcpOption::isValid (DhcpOption::BootFileSize, 0));

    ASSERT_TRUE (DhcpOption::isValid (DhcpOption::IpAddressLeaseTime, 86400));
    ASSERT_TRUE (DhcpOption::isValid (DhcpOption::IpAddressLeaseTime, std::numeric_limits<int32_t>::max ()));
    ASSERT_FALSE (DhcpOption::isValid (DhcpOption::IpAddressLeaseTime, std::numeric_limits<int32_t>::min ()));
    ASSERT_FALSE (DhcpOption::isValid (DhcpOption::IpAddressLeaseTime, std::numeric_limits<int64_t>::max ()));

    ASSERT_TRUE (DhcpOption::isValid (DhcpOption::TimeOffset, std::numeric_limits<int32_t>::min ()));
    ASSERT_TRUE (DhcpOption::isValid (DhcpOption::TimeOffset, std::numeric_limits<int32_t>::max ()));
    ASSERT_FALSE (DhcpOption::isValid (DhcpOption::TimeOffset, std::numeric_limits<int64_t>::min ()));

    ASSERT_FALSE (DhcpOption::isValid (DhcpOption::SubnetMask, 1));
    ASSERT_FALSE (DhcpOption::isValid (DhcpOption::DomainName, 1));

    ASSERT_TRUE (DhcpOption::isValid (DhcpOption::HostName, "foobar"));
    ASSERT_TRUE (DhcpOption::isValid (DhcpOption::HostName, "192.168.0.254"));
    ASSERT_TRUE (DhcpOption::isValid (DhcpOption::HostName, "50:7b:9d:13:82:df"));

    ASSERT_TRUE (DhcpOption::isValid (DhcpOption::SubnetMask, "255.255.255.0"));
    ASSERT_TRUE (DhcpOption::isValid (DhcpOption::SubnetMask, IpAddress ("255.255.255.0")));
    ASSERT_FALSE (DhcpOption::isValid (DhcpOption::SubnetMask, "foobar"));
    ASSERT_FALSE (DhcpOption::isValid (DhcpOption::SubnetMask, "50:7b:9d:13:82:df"));
    ASSERT_FALSE (DhcpOption::isValid (DhcpOption::SubnetMask, "::1"));
    ASSERT_FALSE (DhcpOption::isValid (DhcpOption::SubnetMask, IpAddress (AF_INET6)));
    ASSERT_FALSE (DhcpOption::isValid (DhcpOption::SubnetMask, MacAddress ("50:7b:9d:13:82:df")));
    ASSERT_FALSE (DhcpOption::isValid (DhcpOption::SubnetMask, IpList{"192.168.0.254"}));

    ASSERT_TRUE (DhcpOption::isValid (DhcpOption::ClientIdentifier, "50:7b:9d:13:82:df"));
    ASSERT_TRUE (DhcpOption::isValid (DhcpOption::ClientIdentifier, MacAddress ("50:7b:9d:13:82:df")));
    ASSERT_FALSE (DhcpOption::isValid (DhcpOption::ClientIdentifier, "192.168.0.254"));
    ASSERT_FALSE (DhcpOption::isValid (DhcpOption::ClientIdentifier, IpAddress ("192.168.0.254")));

    ASSERT_TRUE (DhcpOption::isValid (DhcpOption::ParameterRequestList, ByteList{1, 3, 6}));
    ASSERT_TRUE (DhcpOption::isValid (DhcpOption::ParameterRequestList, ByteList{}));
    ASSERT_FALSE (DhcpOption::isValid (DhcpOption::ParameterRequestList, WordList{296}));
    ASSERT_FALSE (DhcpOption::isValid (DhcpOption::SubnetMask, ByteList{1}));

    ASSERT_TRUE (DhcpOption::isValid (DhcpOption::PathMtuPlateauTable, WordList{68, 296, 1006}));
    ASSERT_FALSE (DhcpOption::isValid (DhcpOption::PathMtuPlateauTable, WordList{67}));
    ASSERT_FALSE (DhcpOption::isValid (DhcpOption::PathMtuPlateauTable, ByteList{68}));

    ASSERT_TRUE (DhcpOption::isValid (DhcpOption::Router, IpList{"192.168.0.254"}));
    ASSERT_FALSE (DhcpOption::isValid (DhcpOption::Router, IpList{IpAddress (AF_INET6)}));
    ASSERT_FALSE (DhcpOption::isValid (DhcpOption::Router, "192.168.0.254"));
    ASSERT_FALSE (DhcpOption::isValid (DhcpOption::Router, IpAddress ("192.168.0.254")));
}

/**
 * @brief Test describe method.
 */
TEST (DhcpOption, describe)
{
    const DhcpOption::Info* info = DhcpOption::describe (DhcpOption::InterfaceMtu);
    ASSERT_NE (info, nullptr);
    ASSERT_EQ (info->code, DhcpOption::InterfaceMtu);
    ASSERT_STREQ (info->name, "InterfaceMtu");
    ASSERT_EQ (info->type, DhcpOption::Word);
    ASSERT_EQ (info->min, 68);

    ASSERT_EQ (DhcpOption::describe (DhcpOption::Pad), nullptr);
    ASSERT_EQ (DhcpOption::describe (DhcpOption::PolicyFilter), nullptr);
    ASSERT_EQ (DhcpOption::describe (DhcpOption::End), nullptr);
}

/**
 * @brief Test equal method.
 */
TEST (DhcpOption, equal)
{
    ASSERT_TRUE (sample () == sample ());
    ASSERT_FALSE (sample () == DhcpOption ());
}

/**
 * @brief Test not_equal method.
 */
TEST (DhcpOption, not_equal)
{
    ASSERT_TRUE (sample () != DhcpOption ());
    ASSERT_FALSE (sample () != sample ());
}

/**
 * @brief Test lower method.
 */
TEST (DhcpOption, lower)
{
    ASSERT_TRUE (DhcpOption () < sample ());
    ASSERT_FALSE (sample () < sample ());
}

/**
 * @brief Test greater method.
 */
TEST (DhcpOption, greater)
{
    ASSERT_TRUE (sample () > DhcpOption ());
    ASSERT_FALSE (sample () > sample ());
}

/**
 * @brief Test lower_or_equal method.
 */
TEST (DhcpOption, lower_or_equal)
{
    ASSERT_TRUE (DhcpOption () <= sample ());
    ASSERT_TRUE (sample () <= sample ());
    ASSERT_FALSE (sample () <= DhcpOption ());
}

/**
 * @brief Test greater_or_equal method.
 */
TEST (DhcpOption, greater_or_equal)
{
    ASSERT_TRUE (sample () >= DhcpOption ());
    ASSERT_TRUE (sample () >= sample ());
    ASSERT_FALSE (DhcpOption () >= sample ());
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
