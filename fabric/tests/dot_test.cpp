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
#include <join/resolver.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::lastError;
using join::Dot;
using join::IpAddress;
using join::IpAddressList;
using join::AliasList;
using join::ServerList;
using join::ExchangerList;

using namespace std::chrono_literals;

class DotTest : public ::testing::Test
{
protected:
    void SetUp () override
    {
        _resolver = std::make_unique<Dot::Resolver> ("dns.google");
        ASSERT_NE (_resolver, nullptr);
    }

    void TearDown () override
    {
        if (_resolver->disconnect () == -1)
        {
            ASSERT_EQ (join::lastError, join::Errc::TemporaryError) << join::lastError.message ();
        }
        ASSERT_TRUE (_resolver->waitDisconnected ()) << join::lastError.message ();

        _resolver->close ();
    }

    std::unique_ptr<Dot::Resolver> _resolver;
};

/**
 * @brief test the resolveAllAddress method.
 */
TEST_F (DotTest, resolveAllAddress)
{
    IpAddressList addresses = _resolver->resolveAllAddress ("", AF_INET);
    EXPECT_EQ (addresses.size (), 0);

    addresses = _resolver->resolveAllAddress ("", AF_INET6);
    EXPECT_EQ (addresses.size (), 0);

    addresses = _resolver->resolveAllAddress ("");
    EXPECT_EQ (addresses.size (), 0);

    addresses = _resolver->resolveAllAddress ("localhost", AF_INET);
    EXPECT_EQ (addresses.size (), 0);

    addresses = _resolver->resolveAllAddress ("localhost", AF_INET6);
    EXPECT_EQ (addresses.size (), 0);

    addresses = _resolver->resolveAllAddress ("localhost");
    EXPECT_EQ (addresses.size (), 0);

    addresses = Dot::Resolver ("255.255.255.255").resolveAllAddress ("google.com", AF_INET);
    EXPECT_EQ (addresses.size (), 0);

    addresses = Dot::Resolver ("8.8.8.8", 853).resolveAllAddress ("joinframework.net", AF_INET, 1ms);
    EXPECT_EQ (addresses.size (), 0);

    addresses = Dot::Resolver ("8.8.8.8", 853).resolveAllAddress ("joinframework.net", 1ms);
    EXPECT_EQ (addresses.size (), 0);

    addresses = _resolver->resolveAllAddress ("joinframework.net", AF_INET);
    EXPECT_GT (addresses.size (), 0);

    addresses = _resolver->resolveAllAddress ("google.com", AF_INET);
    EXPECT_GT (addresses.size (), 0);

    addresses = _resolver->resolveAllAddress ("google.com", AF_INET6);
    EXPECT_GT (addresses.size (), 0);

    addresses = _resolver->resolveAllAddress ("google.com");
    EXPECT_GT (addresses.size (), 0);
}

/**
 * @brief test the resolveAddress method.
 */
TEST_F (DotTest, resolveAddress)
{
    IpAddress address = _resolver->resolveAddress ("", AF_INET);
    EXPECT_TRUE (address.isWildcard ());

    address = _resolver->resolveAddress ("", AF_INET6);
    EXPECT_TRUE (address.isWildcard ());

    address = _resolver->resolveAddress ("");
    EXPECT_TRUE (address.isWildcard ());

    address = _resolver->resolveAddress ("localhost", AF_INET);
    EXPECT_TRUE (address.isWildcard ());

    address = _resolver->resolveAddress ("localhost", AF_INET6);
    EXPECT_TRUE (address.isWildcard ());

    address = _resolver->resolveAddress ("localhost");
    EXPECT_TRUE (address.isWildcard ());

    address = Dot::Resolver ("255.255.255.255").resolveAddress ("google.com", AF_INET);
    EXPECT_TRUE (address.isWildcard ());

    address = Dot::Resolver ("8.8.8.8", 853).resolveAddress ("joinframework.net", AF_INET, 1ms);
    EXPECT_TRUE (address.isWildcard ());

    address = Dot::Resolver ("8.8.8.8", 853).resolveAddress ("joinframework.net", 1ms);
    EXPECT_TRUE (address.isWildcard ());

    address = _resolver->resolveAddress ("joinframework.net", AF_INET);
    EXPECT_FALSE (address.isWildcard ());

    address = _resolver->resolveAddress ("google.com", AF_INET);
    EXPECT_TRUE (address.isIpv4Address ());
    EXPECT_FALSE (address.isWildcard ());

    address = _resolver->resolveAddress ("google.com", AF_INET6);
    EXPECT_TRUE (address.isIpv6Address ());
    EXPECT_FALSE (address.isWildcard ());

    address = _resolver->resolveAddress ("google.com");
    EXPECT_FALSE (address.isWildcard ());
}

/**
 * @brief test the resolveAllName method.
 */
TEST_F (DotTest, resolveAllName)
{
    AliasList aliases = _resolver->resolveAllName ("0.0.0.0");
    EXPECT_EQ (aliases.size (), 0);

    aliases = _resolver->resolveAllName ("::");
    EXPECT_EQ (aliases.size (), 0);

    aliases = _resolver->resolveAllName ("192.168.24.32");
    EXPECT_EQ (aliases.size (), 0);

    aliases = Dot::Resolver ("255.255.255.255").resolveAllName ("1.1.1.1");
    EXPECT_EQ (aliases.size (), 0);

    aliases =
        Dot::Resolver ("8.8.8.8", 853).resolveAllName (_resolver->resolveAddress ("joinframework.net", AF_INET), 1ms);
    EXPECT_EQ (aliases.size (), 0);

    aliases = _resolver->resolveAllName ("1.1.1.1");
    EXPECT_GT (aliases.size (), 0);
}

/**
 * @brief test the resolveName method.
 */
TEST_F (DotTest, resolveName)
{
    std::string alias = _resolver->resolveName ("0.0.0.0");
    EXPECT_TRUE (alias.empty ());

    alias = _resolver->resolveName ("::");
    EXPECT_TRUE (alias.empty ());

    alias = _resolver->resolveName ("192.168.24.32");
    EXPECT_TRUE (alias.empty ());

    alias = Dot::Resolver ("255.255.255.255").resolveName ("1.1.1.1");
    EXPECT_TRUE (alias.empty ());

    alias = Dot::Resolver ("8.8.8.8", 853).resolveName (_resolver->resolveAddress ("joinframework.net", AF_INET), 1ms);
    EXPECT_TRUE (alias.empty ());

    alias = _resolver->resolveName ("1.1.1.1");
    EXPECT_FALSE (alias.empty ());
}

/**
 * @brief test the resolveAllNameServer method.
 */
TEST_F (DotTest, resolveAllNameServer)
{
    ServerList servers = _resolver->resolveAllNameServer ("");
    EXPECT_EQ (servers.size (), 0);

    servers = _resolver->resolveAllNameServer ("localhost");
    EXPECT_EQ (servers.size (), 0);

    servers = Dot::Resolver ("255.255.255.255").resolveAllNameServer ("google.com");
    EXPECT_EQ (servers.size (), 0);

    servers = Dot::Resolver ("8.8.8.8", 853).resolveAllNameServer ("joinframework.net", 1ms);
    EXPECT_EQ (servers.size (), 0);

    servers = _resolver->resolveAllNameServer ("google.com");
    EXPECT_GT (servers.size (), 0);

    servers = _resolver->resolveAllNameServer ("joinframework.net");
    EXPECT_GT (servers.size (), 0);
}

/**
 * @brief test the resolveNameServer method.
 */
TEST_F (DotTest, resolveNameServer)
{
    std::string server = _resolver->resolveNameServer ("");
    EXPECT_TRUE (server.empty ());

    server = _resolver->resolveNameServer ("localhost");
    EXPECT_TRUE (server.empty ());

    server = Dot::Resolver ("255.255.255.255").resolveNameServer ("google.com");
    EXPECT_TRUE (server.empty ());

    server = Dot::Resolver ("8.8.8.8", 853).resolveNameServer ("joinframework.net", 1ms);
    EXPECT_TRUE (server.empty ());

    server = _resolver->resolveNameServer ("google.com");
    EXPECT_FALSE (server.empty ());

    server = _resolver->resolveNameServer ("joinframework.net");
    EXPECT_FALSE (server.empty ());
}

/**
 * @brief test the resolveAuthority method.
 */
TEST_F (DotTest, resolveAuthority)
{
    std::string authority = _resolver->resolveAuthority ("");
    EXPECT_TRUE (authority.empty ());

    authority = _resolver->resolveAuthority ("localhost");
    EXPECT_TRUE (authority.empty ());

    authority = Dot::Resolver ("255.255.255.255").resolveAuthority ("google.com");
    EXPECT_TRUE (authority.empty ());

    authority = Dot::Resolver ("8.8.8.8", 853).resolveAuthority ("joinframework.net", 1ms);
    EXPECT_TRUE (authority.empty ());

    authority = _resolver->resolveAuthority ("google.com");
    EXPECT_FALSE (authority.empty ());

    authority = _resolver->resolveAuthority ("joinframework.net");
    EXPECT_FALSE (authority.empty ());
}

/**
 * @brief test the resolveAllMailExchanger method.
 */
TEST_F (DotTest, resolveAllMailExchanger)
{
    ExchangerList exchangers = _resolver->resolveAllMailExchanger ("");
    EXPECT_EQ (exchangers.size (), 0);

    exchangers = _resolver->resolveAllMailExchanger ("localhost");
    EXPECT_EQ (exchangers.size (), 0);

    exchangers = Dot::Resolver ("255.255.255.255").resolveAllMailExchanger ("google.com");
    EXPECT_EQ (exchangers.size (), 0);

    exchangers = Dot::Resolver ("8.8.8.8", 853).resolveAllMailExchanger ("joinframework.net", 1ms);
    EXPECT_EQ (exchangers.size (), 0);

    exchangers = _resolver->resolveAllMailExchanger ("google.com");
    EXPECT_GT (exchangers.size (), 0);
}

/**
 * @brief test the resolveMailExchanger method.
 */
TEST_F (DotTest, resolveMailExchanger)
{
    std::string exchanger = _resolver->resolveMailExchanger ("");
    EXPECT_TRUE (exchanger.empty ());

    exchanger = _resolver->resolveMailExchanger ("localhost");
    EXPECT_TRUE (exchanger.empty ());

    exchanger = Dot::Resolver ("255.255.255.255").resolveMailExchanger ("google.com");
    EXPECT_TRUE (exchanger.empty ());

    exchanger = Dot::Resolver ("8.8.8.8", 853).resolveMailExchanger ("joinframework.net", 1ms);
    EXPECT_TRUE (exchanger.empty ());

    exchanger = _resolver->resolveMailExchanger ("google.com");
    EXPECT_FALSE (exchanger.empty ());
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
