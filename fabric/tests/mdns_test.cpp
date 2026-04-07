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
#include <join/mdns.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::Mdns;
using join::DnsPacket;

/**
 * @brief Class used to test the MDNS API.
 */
class MdnsTest : public Mdns::Peer, public ::testing::Test
{
public:
    MdnsTest ()
    : Mdns::Peer ("eth0", AF_INET)
    {
    }

    /**
     * @brief method called when a DNS query is received.
     * @param packet parsed DNS query received.
     */
    void onQuery (const DnsPacket& packet) override final
    {
        std::cout << std::endl;
        std::cout << "SERVER: " << packet.src << "#" << packet.port << std::endl;

        std::cout << std::endl;
        std::cout << ";; QUESTION SECTION: " << std::endl;
        for (auto const& question : packet.questions)
        {
            std::cout << question.host;
            std::cout << "  " << typeName (question.type);
            std::cout << "  " << className (question.dnsclass);
            std::cout << std::endl;
        }

        std::cout << std::endl;
        std::cout << ";; ANSWER SECTION: " << std::endl;
        for (auto const& answer : packet.answers)
        {
            std::cout << answer.host;
            std::cout << "  " << typeName (answer.type);
            std::cout << "  " << className (answer.dnsclass);
            std::cout << "  " << answer.ttl;
            if (answer.type == RecordType::A)
            {
                std::cout << "  " << answer.addr;
            }
            else if (answer.type == RecordType::NS)
            {
                std::cout << "  " << answer.name;
            }
            else if (answer.type == RecordType::CNAME)
            {
                std::cout << "  " << answer.name;
            }
            else if (answer.type == RecordType::SOA)
            {
                std::cout << "  " << answer.name;
                std::cout << "  " << answer.mail;
                std::cout << "  " << answer.serial;
                std::cout << "  " << answer.refresh;
                std::cout << "  " << answer.retry;
                std::cout << "  " << answer.expire;
                std::cout << "  " << answer.minimum;
            }
            else if (answer.type == RecordType::PTR)
            {
                std::cout << "  " << answer.name;
            }
            else if (answer.type == RecordType::MX)
            {
                std::cout << "  " << answer.mxpref;
                std::cout << "  " << answer.name;
            }
            else if (answer.type == RecordType::AAAA)
            {
                std::cout << "  " << answer.addr;
            }
            std::cout << std::endl;
        }
    }

    /**
     * @brief method called when a DNS annoucement is received.
     * @param packet parsed DNS annoucement received.
     * @param partial partial data received, more data are coming.
     */
    void onAnnouncement (const DnsPacket& packet, [[maybe_unused]] bool partial) override final
    {
        std::cout << std::endl;
        std::cout << "SERVER: " << packet.src << "#" << packet.port << std::endl;

        std::cout << std::endl;
        std::cout << ";; QUESTION SECTION: " << std::endl;
        for (auto const& question : packet.questions)
        {
            std::cout << question.host;
            std::cout << "  " << typeName (question.type);
            std::cout << "  " << className (question.dnsclass);
            std::cout << std::endl;
        }

        std::cout << std::endl;
        std::cout << ";; ANSWER SECTION: " << std::endl;
        for (auto const& answer : packet.answers)
        {
            std::cout << answer.host;
            std::cout << "  " << typeName (answer.type);
            std::cout << "  " << className (answer.dnsclass);
            std::cout << "  " << answer.ttl;
            if (answer.type == RecordType::A)
            {
                std::cout << "  " << answer.addr;
            }
            else if (answer.type == RecordType::NS)
            {
                std::cout << "  " << answer.name;
            }
            else if (answer.type == RecordType::CNAME)
            {
                std::cout << "  " << answer.name;
            }
            else if (answer.type == RecordType::SOA)
            {
                std::cout << "  " << answer.name;
                std::cout << "  " << answer.mail;
                std::cout << "  " << answer.serial;
                std::cout << "  " << answer.refresh;
                std::cout << "  " << answer.retry;
                std::cout << "  " << answer.expire;
                std::cout << "  " << answer.minimum;
            }
            else if (answer.type == RecordType::PTR)
            {
                std::cout << "  " << answer.name;
            }
            else if (answer.type == RecordType::MX)
            {
                std::cout << "  " << answer.mxpref;
                std::cout << "  " << answer.name;
            }
            else if (answer.type == RecordType::AAAA)
            {
                std::cout << "  " << answer.addr;
            }
            std::cout << std::endl;
        }
    }

private:
    ///
};

/**
 * @brief .
 */
TEST_F (MdnsTest, sleep)
{
    browse ("_printer._tcp.local");
    ASSERT_FALSE (resolve ("FR01138NB-2.local", RecordType::A).empty ());
    sleep (500);
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
