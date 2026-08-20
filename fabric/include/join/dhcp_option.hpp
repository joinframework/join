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

#ifndef JOIN_FABRIC_DHCP_OPTION_HPP
#define JOIN_FABRIC_DHCP_OPTION_HPP

// libjoin.
#include <join/mac_address.hpp>
#include <join/ip_address.hpp>
#include <join/variant.hpp>
#include <join/error.hpp>

// C++.
#include <type_traits>
#include <limits>
#include <sstream>
#include <string>
#include <vector>
#include <map>

namespace join
{
    /// list of bytes.
    using ByteList = std::vector<uint8_t>;

    /// list of 16 bits words.
    using WordList = std::vector<uint16_t>;

    /// list of IP addresses.
    using IpList = std::vector<IpAddress>;

    /// value carried by a DHCP option.
    using DhcpOptionValue =
        Variant<uint8_t, uint16_t, uint32_t, int32_t, std::string, IpAddress, MacAddress, ByteList, WordList, IpList>;

    /**
     * @brief DHCP option list.
     */
    class DhcpOption
    {
    public:
        /**
         * @brief option codes.
         */
        enum Code : uint8_t
        {
            Pad = 0,
            SubnetMask = 1,
            TimeOffset = 2,
            Router = 3,
            TimeServer = 4,
            NameServer = 5,
            DomainNameServer = 6,
            LogServer = 7,
            CookieServer = 8,
            LprServer = 9,
            ImpressServer = 10,
            ResourceLocationServer = 11,
            HostName = 12,
            BootFileSize = 13,
            MeritDumpFile = 14,
            DomainName = 15,
            SwapServer = 16,
            RootPath = 17,
            ExtensionsPath = 18,
            IpForwarding = 19,
            NonLocalSourceRouting = 20,
            PolicyFilter = 21,
            MaximumDatagramReassemblySize = 22,
            DefaultIpTimeToLive = 23,
            PathMtuAgingTimeout = 24,
            PathMtuPlateauTable = 25,
            InterfaceMtu = 26,
            AllSubnetsAreLocal = 27,
            BroadcastAddress = 28,
            PerformMaskDiscovery = 29,
            MaskSupplier = 30,
            PerformRouterDiscovery = 31,
            RouterSolicitationAddress = 32,
            StaticRoute = 33,
            TrailerEncapsulation = 34,
            ArpCacheTimeout = 35,
            EthernetEncapsulation = 36,
            TcpDefaultTtl = 37,
            TcpKeepaliveInterval = 38,
            TcpKeepaliveGarbage = 39,
            NisDomain = 40,
            NetworkInformationServer = 41,
            NtpServer = 42,
            VendorSpecificInformation = 43,
            NetbiosNameServer = 44,
            NetbiosDatagramDistributionServer = 45,
            NetbiosNodeType = 46,
            NetbiosScope = 47,
            XWindowFontServer = 48,
            XWindowDisplayManager = 49,
            RequestedIpAddress = 50,
            IpAddressLeaseTime = 51,
            OptionOverload = 52,
            DhcpMessageType = 53,
            ServerIdentifier = 54,
            ParameterRequestList = 55,
            Message = 56,
            MaximumDhcpMessageSize = 57,
            RenewalTimeValue = 58,
            RebindingTimeValue = 59,
            VendorClassIdentifier = 60,
            ClientIdentifier = 61,
            TftpServerName = 66,
            BootFileName = 67,
            MobileIpHomeAgent = 68,
            SmtpServer = 69,
            Pop3Server = 70,
            NntpServer = 71,
            DefaultWwwServer = 72,
            DefaultFingerServer = 73,
            DefaultIrcServer = 74,
            StreetTalkServer = 75,
            StreetTalkDirectoryAssistanceServer = 76,
            End = 255,
        };

        /**
         * @brief type of the value an option carries on the wire.
         */
        enum Type
        {
            Byte,  /**< 8 bits unsigned integer. */
            Word,  /**< 16 bits unsigned integer. */
            Long,  /**< 32 bits unsigned integer. */
            SLong, /**< 32 bits signed integer. */
            Text,  /**< character string. */
            Ip,    /**< IPv4 address. */
            Mac,   /**< hardware address. */
            Bytes, /**< list of 8 bits unsigned integers. */
            Words, /**< list of 16 bits unsigned integers. */
            Ips,   /**< list of IPv4 addresses. */
        };

        /**
         * @brief description of an option.
         */
        struct Info
        {
            uint8_t code;     /**< option code. */
            const char* name; /**< option name. */
            Type type;        /**< type of the value carried. */
            uint32_t min;     /**< lowest accepted value, per element for lists. */
            uint32_t max;     /**< highest accepted value, zero if unconstrained. */
            uint32_t mask;    /**< accepted values as a bitmask, zero if unconstrained. */
        };

        /// option list.
        using Options = std::map<uint8_t, DhcpOptionValue>;

        /// constant iterator from nested container.
        using const_iterator = Options::const_iterator;

        /**
         * @brief create the DhcpOption instance.
         */
        DhcpOption () = default;

        /**
         * @brief create the DhcpOption instance with the elements from range.
         * @param first first element to insert.
         * @param last last element to insert.
         */
        DhcpOption (const_iterator first, const_iterator last)
        : _options (first, last)
        {
        }

        /**
         * @brief create instance by copy.
         * @param other other object to copy.
         */
        DhcpOption (const DhcpOption& other) = default;

        /**
         * @brief assign instance by copy.
         * @param other other object to copy.
         * @return a reference of the current object.
         */
        DhcpOption& operator= (const DhcpOption& other) = default;

        /**
         * @brief create instance by move.
         * @param other other object to move.
         */
        DhcpOption (DhcpOption&& other) = default;

        /**
         * @brief assign instance by move.
         * @param other other object to move.
         * @return a reference of the current object.
         */
        DhcpOption& operator= (DhcpOption&& other) = default;

        /**
         * @brief destroy the DhcpOption instance.
         */
        ~DhcpOption () = default;

        /**
         * @brief get the value identified by code, with range verification.
         * @param code code used to locate the value.
         * @return a reference to the mapped value.
         * @throw std::out_of_range if the option is not in the list.
         */
        const DhcpOptionValue& at (uint8_t code) const
        {
            return _options.at (code);
        }

        /**
         * @brief get the value identified by code if it is present and holds the requested type.
         * @param code code used to locate the value.
         * @return the value address, nullptr if absent or of another type.
         */
        template <typename T>
        const T* getIf (uint8_t code) const
        {
            auto it = _options.find (code);
            return (it == _options.end ()) ? nullptr : it->second.getIf<T> ();
        }

        /**
         * @brief find the element with the given code.
         * @param code code of the element to search for.
         * @return iterator to the element, end () if absent.
         */
        const_iterator find (uint8_t code) const
        {
            return _options.find (code);
        }

        /**
         * @brief check if there is an element identified by code in the list.
         * @param code code of the element to search for.
         * @return true if there is an element identified by code, false otherwise.
         */
        bool contains (uint8_t code) const
        {
            return (find (code) != end ());
        }

        /**
         * @brief get an iterator to the first element of the list.
         * @return iterator to the first element.
         */
        const_iterator begin () const
        {
            return _options.begin ();
        }

        /**
         * @brief get an iterator to the first element of the list.
         * @return iterator to the first element.
         */
        const_iterator cbegin () const
        {
            return _options.cbegin ();
        }

        /**
         * @brief get an iterator to the element following the last element of the list.
         * @return iterator to the element following the last element.
         */
        const_iterator end () const
        {
            return _options.end ();
        }

        /**
         * @brief get an iterator to the element following the last element of the list.
         * @return iterator to the element following the last element.
         */
        const_iterator cend () const
        {
            return _options.cend ();
        }

        /**
         * @brief check if the list has no element.
         * @return true if the list is empty, false otherwise.
         */
        bool empty () const
        {
            return _options.empty ();
        }

        /**
         * @brief get the number of options in the list.
         * @return the number of options in the list.
         */
        size_t size () const
        {
            return _options.size ();
        }

        /**
         * @brief remove the element identified by code.
         * @param code code to compare to.
         * @return number of elements removed.
         */
        size_t erase (uint8_t code)
        {
            return _options.erase (code);
        }

        /**
         * @brief remove all elements from the list.
         */
        void clear ()
        {
            _options.clear ();
        }

        /**
         * @brief add an option to the list, converting the value to the type the option carries.
         * @param code option code.
         * @param value option value.
         * @return true if the option was added to the list, false otherwise.
         */
        template <typename T>
        bool insert (uint8_t code, T&& value)
        {
            DhcpOptionValue converted;

            if (!convert (code, std::forward<T> (value), converted))
            {
                return false;
            }

            if (!_options.emplace (code, std::move (converted)).second)
            {
                lastError = make_error_code (Errc::InUse);
                return false;
            }

            return true;
        }

        /**
         * @brief insert elements from range.
         * @param first first element to insert.
         * @param last last element to insert.
         */
        void insert (const_iterator first, const_iterator last)
        {
            _options.insert (first, last);
        }

        /**
         * @brief exchange the contents of the list with those of other.
         * @param other list to exchange the contents with.
         */
        void swap (DhcpOption& other)
        {
            _options.swap (other._options);
        }

        /**
         * @brief dump the options in string format.
         * @return the options in string format.
         */
        std::string dump () const
        {
            std::stringstream stream;

            for (auto const& option : _options)
            {
                stream << codeName (option.first) << " (" << int (option.first) << "): ";
                write (stream, option.first, option.second);
                stream << "\n";
            }

            return stream.str ();
        }

        /**
         * @brief check that a value can be carried by the given option.
         * @param code option code.
         * @param value option value.
         * @return true if valid, false otherwise.
         */
        template <typename T>
        static bool isValid (uint8_t code, T&& value)
        {
            DhcpOptionValue converted;
            return convert (code, std::forward<T> (value), converted);
        }

        /**
         * @brief get the description of the given option.
         * @param code option code.
         * @return the option description, nullptr if the option is unknown.
         */
        static const Info* describe (uint8_t code) noexcept;

    protected:
        /**
         * @brief get the name of the given option.
         * @param code option code.
         * @return option name.
         */
        static const char* codeName (uint8_t code) noexcept;

        /**
         * @brief write an option value in string format.
         * @param stream stream to write to.
         * @param code option code.
         * @param value option value.
         */
        static void write (std::ostream& stream, uint8_t code, const DhcpOptionValue& value);

        /**
         * @brief check that a value satisfies the constraints of the given option.
         * @param info option description.
         * @param value value to check.
         * @return true if the value is accepted, false otherwise.
         */
        static bool accepts (const Info& info, uint32_t value) noexcept
        {
            if (info.mask)
            {
                return (value < 32) && ((info.mask >> value) & 1u);
            }

            if (info.max)
            {
                return (value >= info.min) && (value <= info.max);
            }

            return true;
        }

        /**
         * @brief check that a signed value can be represented by the target type.
         * @param value value to check.
         * @return true if the value fits, false otherwise.
         */
        template <typename Target, typename Source>
        static typename std::enable_if<std::is_signed<Source>::value, bool>::type fits (Source value) noexcept
        {
            if (value < 0)
            {
                return std::is_signed<Target>::value &&
                       static_cast<int64_t> (value) >= static_cast<int64_t> (std::numeric_limits<Target>::min ());
            }

            return static_cast<uint64_t> (value) <= static_cast<uint64_t> (std::numeric_limits<Target>::max ());
        }

        /**
         * @brief check that an unsigned value can be represented by the target type.
         * @param value value to check.
         * @return true if the value fits, false otherwise.
         */
        template <typename Target, typename Source>
        static typename std::enable_if<!std::is_signed<Source>::value, bool>::type fits (Source value) noexcept
        {
            return static_cast<uint64_t> (value) <= static_cast<uint64_t> (std::numeric_limits<Target>::max ());
        }

        /**
         * @brief store an integer value as the given target type.
         * @param info option description.
         * @param value value to store.
         * @param out converted value.
         * @return true on success, false otherwise.
         */
        template <typename Target, typename Source>
        static bool store (const Info& info, Source value, DhcpOptionValue& out)
        {
            if (!fits<Target> (value) || !accepts (info, static_cast<uint32_t> (value)))
            {
                lastError = make_error_code (Errc::InvalidParam);
                return false;
            }

            out = static_cast<Target> (value);

            return true;
        }

        /**
         * @brief report an invalid option value.
         * @return false.
         */
        static bool invalid () noexcept
        {
            lastError = make_error_code (Errc::InvalidParam);
            return false;
        }

        /**
         * @brief convert an integer to the type the given option carries.
         * @param code option code.
         * @param value value to convert.
         * @param out converted value.
         * @return true on success, false otherwise.
         */
        template <typename T>
        static typename std::enable_if<std::is_integral<typename std::decay<T>::type>::value, bool>::type convert (
            uint8_t code, T value, DhcpOptionValue& out)
        {
            const Info* info = describe (code);
            if (info == nullptr)
            {
                return invalid ();
            }

            switch (info->type)
            {
                case Byte:
                    return store<uint8_t> (*info, value, out);

                case Word:
                    return store<uint16_t> (*info, value, out);

                case Long:
                    return store<uint32_t> (*info, value, out);

                case SLong:
                    return store<int32_t> (*info, value, out);

                default:
                    break;
            }

            return invalid ();
        }

        /**
         * @brief convert a string to the type the given option carries.
         * @param code option code.
         * @param value value to convert.
         * @param out converted value.
         * @return true on success, false otherwise.
         */
        static bool convert (uint8_t code, const std::string& value, DhcpOptionValue& out);

        /**
         * @brief convert a string to the type the given option carries.
         * @param code option code.
         * @param value value to convert.
         * @param out converted value.
         * @return true on success, false otherwise.
         */
        static bool convert (uint8_t code, const char* value, DhcpOptionValue& out)
        {
            return convert (code, std::string (value), out);
        }

        /**
         * @brief store an IP address in the given option.
         * @param code option code.
         * @param value value to store.
         * @param out converted value.
         * @return true on success, false otherwise.
         */
        static bool convert (uint8_t code, const IpAddress& value, DhcpOptionValue& out)
        {
            const Info* info = describe (code);
            if ((info == nullptr) || (info->type != Ip) || (value.family () != AF_INET))
            {
                return invalid ();
            }

            out = value;

            return true;
        }

        /**
         * @brief store a hardware address in the given option.
         * @param code option code.
         * @param value value to store.
         * @param out converted value.
         * @return true on success, false otherwise.
         */
        static bool convert (uint8_t code, const MacAddress& value, DhcpOptionValue& out)
        {
            const Info* info = describe (code);
            if ((info == nullptr) || (info->type != Mac))
            {
                return invalid ();
            }

            out = value;

            return true;
        }

        /**
         * @brief store a list of bytes in the given option.
         * @param code option code.
         * @param value value to store.
         * @param out converted value.
         * @return true on success, false otherwise.
         */
        static bool convert (uint8_t code, const ByteList& value, DhcpOptionValue& out);

        /**
         * @brief store a list of words in the given option.
         * @param code option code.
         * @param value value to store.
         * @param out converted value.
         * @return true on success, false otherwise.
         */
        static bool convert (uint8_t code, const WordList& value, DhcpOptionValue& out);

        /**
         * @brief store a list of IP addresses in the given option.
         * @param code option code.
         * @param value value to store.
         * @param out converted value.
         * @return true on success, false otherwise.
         */
        static bool convert (uint8_t code, const IpList& value, DhcpOptionValue& out);

        /// options.
        Options _options;

        /// friendship with equal operator.
        friend bool operator== (const DhcpOption& lhs, const DhcpOption& rhs);

        /// friendship with not equal operator.
        friend bool operator!= (const DhcpOption& lhs, const DhcpOption& rhs);

        /// friendship with lower operator.
        friend bool operator< (const DhcpOption& lhs, const DhcpOption& rhs);

        /// friendship with greater operator.
        friend bool operator> (const DhcpOption& lhs, const DhcpOption& rhs);

        /// friendship with lower or equal operator.
        friend bool operator<= (const DhcpOption& lhs, const DhcpOption& rhs);

        /// friendship with greater or equal operator.
        friend bool operator>= (const DhcpOption& lhs, const DhcpOption& rhs);
    };

    /**
     * @brief compare if equal.
     * @param lhs options to compare.
     * @param rhs options to compare to.
     * @return true if equal.
     */
    inline bool operator== (const DhcpOption& lhs, const DhcpOption& rhs)
    {
        return lhs._options == rhs._options;
    }

    /**
     * @brief compare if not equal.
     * @param lhs options to compare.
     * @param rhs options to compare to.
     * @return true if not equal.
     */
    inline bool operator!= (const DhcpOption& lhs, const DhcpOption& rhs)
    {
        return lhs._options != rhs._options;
    }

    /**
     * @brief compare if lower.
     * @param lhs options to compare.
     * @param rhs options to compare to.
     * @return true if lower.
     */
    inline bool operator< (const DhcpOption& lhs, const DhcpOption& rhs)
    {
        return lhs._options < rhs._options;
    }

    /**
     * @brief compare if greater.
     * @param lhs options to compare.
     * @param rhs options to compare to.
     * @return true if greater.
     */
    inline bool operator> (const DhcpOption& lhs, const DhcpOption& rhs)
    {
        return lhs._options > rhs._options;
    }

    /**
     * @brief compare if lower or equal.
     * @param lhs options to compare.
     * @param rhs options to compare to.
     * @return true if lower or equal.
     */
    inline bool operator<= (const DhcpOption& lhs, const DhcpOption& rhs)
    {
        return lhs._options <= rhs._options;
    }

    /**
     * @brief compare if greater or equal.
     * @param lhs options to compare.
     * @param rhs options to compare to.
     * @return true if greater or equal.
     */
    inline bool operator>= (const DhcpOption& lhs, const DhcpOption& rhs)
    {
        return lhs._options >= rhs._options;
    }

    /**
     * @brief insert options into stream.
     * @param out output stream.
     * @param options options to insert.
     * @return a reference to the output stream.
     */
    inline std::ostream& operator<< (std::ostream& out, const DhcpOption& options)
    {
        out << options.dump ();
        return out;
    }
}

#endif
