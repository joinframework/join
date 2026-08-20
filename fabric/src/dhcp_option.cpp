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

// C++.
#include <algorithm>
#include <iterator>

using join::IpAddress;
using join::MacAddress;
using join::DhcpOptionValue;
using join::DhcpOption;

// =========================================================================
//   CLASS     : DhcpOption
//   METHOD    : describe
// =========================================================================
const DhcpOption::Info* DhcpOption::describe (uint8_t code) noexcept
{
    static const Info table[] = {
        {SubnetMask, "SubnetMask", Ip, 0, 0, 0},
        {TimeOffset, "TimeOffset", SLong, 0, 0, 0},
        {Router, "Router", Ips, 0, 0, 0},
        {TimeServer, "TimeServer", Ips, 0, 0, 0},
        {NameServer, "NameServer", Ips, 0, 0, 0},
        {DomainNameServer, "DomainNameServer", Ips, 0, 0, 0},
        {LogServer, "LogServer", Ips, 0, 0, 0},
        {CookieServer, "CookieServer", Ips, 0, 0, 0},
        {LprServer, "LprServer", Ips, 0, 0, 0},
        {ImpressServer, "ImpressServer", Ips, 0, 0, 0},
        {ResourceLocationServer, "ResourceLocationServer", Ips, 0, 0, 0},
        {HostName, "HostName", Text, 0, 0, 0},
        {BootFileSize, "BootFileSize", Word, 0, 0, 0},
        {MeritDumpFile, "MeritDumpFile", Text, 0, 0, 0},
        {DomainName, "DomainName", Text, 0, 0, 0},
        {SwapServer, "SwapServer", Ip, 0, 0, 0},
        {RootPath, "RootPath", Text, 0, 0, 0},
        {ExtensionsPath, "ExtensionsPath", Text, 0, 0, 0},
        {IpForwarding, "IpForwarding", Byte, 0, 1, 0},
        {NonLocalSourceRouting, "NonLocalSourceRouting", Byte, 0, 1, 0},
        {MaximumDatagramReassemblySize, "MaximumDatagramReassemblySize", Word, 576, 65535, 0},
        {DefaultIpTimeToLive, "DefaultIpTimeToLive", Byte, 1, 255, 0},
        {PathMtuAgingTimeout, "PathMtuAgingTimeout", Long, 0, 0, 0},
        {PathMtuPlateauTable, "PathMtuPlateauTable", Words, 68, 65535, 0},
        {InterfaceMtu, "InterfaceMtu", Word, 68, 65535, 0},
        {AllSubnetsAreLocal, "AllSubnetsAreLocal", Byte, 0, 1, 0},
        {BroadcastAddress, "BroadcastAddress", Ip, 0, 0, 0},
        {PerformMaskDiscovery, "PerformMaskDiscovery", Byte, 0, 1, 0},
        {MaskSupplier, "MaskSupplier", Byte, 0, 1, 0},
        {PerformRouterDiscovery, "PerformRouterDiscovery", Byte, 0, 1, 0},
        {RouterSolicitationAddress, "RouterSolicitationAddress", Ip, 0, 0, 0},
        {TrailerEncapsulation, "TrailerEncapsulation", Byte, 0, 1, 0},
        {ArpCacheTimeout, "ArpCacheTimeout", Long, 0, 0, 0},
        {EthernetEncapsulation, "EthernetEncapsulation", Byte, 0, 1, 0},
        {TcpDefaultTtl, "TcpDefaultTtl", Byte, 1, 255, 0},
        {TcpKeepaliveInterval, "TcpKeepaliveInterval", Long, 0, 0, 0},
        {TcpKeepaliveGarbage, "TcpKeepaliveGarbage", Byte, 0, 1, 0},
        {NisDomain, "NisDomain", Text, 0, 0, 0},
        {NetworkInformationServer, "NetworkInformationServer", Ips, 0, 0, 0},
        {NtpServer, "NtpServer", Ips, 0, 0, 0},
        {VendorSpecificInformation, "VendorSpecificInformation", Bytes, 0, 0, 0},
        {NetbiosNameServer, "NetbiosNameServer", Ips, 0, 0, 0},
        {NetbiosDatagramDistributionServer, "NetbiosDatagramDistributionServer", Ips, 0, 0, 0},
        {NetbiosNodeType, "NetbiosNodeType", Byte, 0, 0, (1u << 1) | (1u << 2) | (1u << 4) | (1u << 8)},
        {NetbiosScope, "NetbiosScope", Bytes, 0, 0, 0},
        {XWindowFontServer, "XWindowFontServer", Ips, 0, 0, 0},
        {XWindowDisplayManager, "XWindowDisplayManager", Ips, 0, 0, 0},
        {RequestedIpAddress, "RequestedIpAddress", Ip, 0, 0, 0},
        {IpAddressLeaseTime, "IpAddressLeaseTime", Long, 0, 0, 0},
        {OptionOverload, "OptionOverload", Byte, 1, 3, 0},
        {DhcpMessageType, "DhcpMessageType", Byte, 1, 8, 0},
        {ServerIdentifier, "ServerIdentifier", Ip, 0, 0, 0},
        {ParameterRequestList, "ParameterRequestList", Bytes, 0, 0, 0},
        {Message, "Message", Text, 0, 0, 0},
        {MaximumDhcpMessageSize, "MaximumDhcpMessageSize", Word, 576, 65535, 0},
        {RenewalTimeValue, "RenewalTimeValue", Long, 0, 0, 0},
        {RebindingTimeValue, "RebindingTimeValue", Long, 0, 0, 0},
        {VendorClassIdentifier, "VendorClassIdentifier", Text, 0, 0, 0},
        {ClientIdentifier, "ClientIdentifier", Mac, 0, 0, 0},
        {TftpServerName, "TftpServerName", Text, 0, 0, 0},
        {BootFileName, "BootFileName", Text, 0, 0, 0},
        {MobileIpHomeAgent, "MobileIpHomeAgent", Ips, 0, 0, 0},
        {SmtpServer, "SmtpServer", Ips, 0, 0, 0},
        {Pop3Server, "Pop3Server", Ips, 0, 0, 0},
        {NntpServer, "NntpServer", Ips, 0, 0, 0},
        {DefaultWwwServer, "DefaultWwwServer", Ips, 0, 0, 0},
        {DefaultFingerServer, "DefaultFingerServer", Ips, 0, 0, 0},
        {DefaultIrcServer, "DefaultIrcServer", Ips, 0, 0, 0},
        {StreetTalkServer, "StreetTalkServer", Ips, 0, 0, 0},
        {StreetTalkDirectoryAssistanceServer, "StreetTalkDirectoryAssistanceServer", Ips, 0, 0, 0},
    };

    auto it = std::lower_bound (std::begin (table), std::end (table), code, [] (const Info& info, uint8_t value) {
        return info.code < value;
    });

    return ((it != std::end (table)) && (it->code == code)) ? it : nullptr;
}

// =========================================================================
//   CLASS     : DhcpOption
//   METHOD    : codeName
// =========================================================================
const char* DhcpOption::codeName (uint8_t code) noexcept
{
    const Info* info = describe (code);
    return (info != nullptr) ? info->name : "Unknown";  // LCOV_EXCL_LINE
}

// =========================================================================
//   CLASS     : DhcpOption
//   METHOD    : write
// =========================================================================
void DhcpOption::write (std::ostream& stream, [[maybe_unused]] uint8_t code, const DhcpOptionValue& value)
{
    if (auto val = value.getIf<uint8_t> ())
    {
        stream << int (*val);
    }
    else if (auto val = value.getIf<uint16_t> ())
    {
        stream << *val;
    }
    else if (auto val = value.getIf<uint32_t> ())
    {
        stream << *val;
    }
    else if (auto val = value.getIf<int32_t> ())
    {
        stream << *val;
    }
    else if (auto val = value.getIf<std::string> ())
    {
        stream << "\"" << *val << "\"";
    }
    else if (auto val = value.getIf<IpAddress> ())
    {
        stream << "\"" << *val << "\"";
    }
    else if (auto val = value.getIf<MacAddress> ())
    {
        stream << "\"" << *val << "\"";
    }
    else if (auto val = value.getIf<ByteList> ())
    {
        stream << "[";
        for (size_t i = 0; i < val->size (); ++i)
        {
            stream << (i ? "," : "") << int ((*val)[i]);
        }
        stream << "]";
    }
    else if (auto val = value.getIf<WordList> ())
    {
        stream << "[";
        for (size_t i = 0; i < val->size (); ++i)
        {
            stream << (i ? "," : "") << (*val)[i];
        }
        stream << "]";
    }
    else if (auto val = value.getIf<IpList> ())
    {
        stream << "[";
        for (size_t i = 0; i < val->size (); ++i)
        {
            stream << (i ? "," : "") << "\"" << (*val)[i] << "\"";
        }
        stream << "]";
    }
}

// =========================================================================
//   CLASS     : DhcpOption
//   METHOD    : convert
// =========================================================================
bool DhcpOption::convert (uint8_t code, const std::string& value, DhcpOptionValue& out)
{
    const Info* info = describe (code);
    if (info == nullptr)
    {
        return invalid ();
    }

    switch (info->type)
    {
        case Text:
            out = value;
            return true;

        case Ip:
            if (!IpAddress::isIpAddress (value) || (IpAddress (value).family () != AF_INET))
            {
                break;
            }
            out = IpAddress (value);
            return true;

        case Mac:
            if (!MacAddress::isMacAddress (value))
            {
                break;
            }
            out = MacAddress (value);
            return true;

        default:
            break;
    }

    return invalid ();
}

// =========================================================================
//   CLASS     : DhcpOption
//   METHOD    : convert
// =========================================================================
bool DhcpOption::convert (uint8_t code, const ByteList& value, DhcpOptionValue& out)
{
    const Info* info = describe (code);
    if ((info == nullptr) || (info->type != Bytes))
    {
        return invalid ();
    }

    for (auto const& element : value)
    {
        if (!accepts (*info, element))
        {
            return invalid ();  // LCOV_EXCL_LINE
        }
    }

    out = value;

    return true;
}

// =========================================================================
//   CLASS     : DhcpOption
//   METHOD    : convert
// =========================================================================
bool DhcpOption::convert (uint8_t code, const WordList& value, DhcpOptionValue& out)
{
    const Info* info = describe (code);
    if ((info == nullptr) || (info->type != Words))
    {
        return invalid ();
    }

    for (auto const& element : value)
    {
        if (!accepts (*info, element))
        {
            return invalid ();
        }
    }

    out = value;

    return true;
}

// =========================================================================
//   CLASS     : DhcpOption
//   METHOD    : convert
// =========================================================================
bool DhcpOption::convert (uint8_t code, const IpList& value, DhcpOptionValue& out)
{
    const Info* info = describe (code);
    if ((info == nullptr) || (info->type != Ips))
    {
        return invalid ();
    }

    for (auto const& element : value)
    {
        if (element.family () != AF_INET)
        {
            return invalid ();
        }
    }

    out = value;

    return true;
}
