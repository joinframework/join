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
#include <join/dns_message.hpp>

// Libraries.
#include <gtest/gtest.h>

using join::DnsMessage;

/**
 * @brief test serialize / deserialize methods.
 */
TEST (DnsMessage, roundTrip)
{
    join::DnsPacket out;
    out.id = 0x1234;
    out.flags = 0x0100;

    join::QuestionRecord question;
    question.host = "google.com";
    question.type = DnsMessage::A;
    question.dnsclass = DnsMessage::IN;
    out.questions.push_back (question);

    join::ResourceRecord a;
    a.host = "google.com";
    a.type = DnsMessage::A;
    a.dnsclass = DnsMessage::IN;
    a.ttl = 3600;
    a.addr = join::IpAddress ("142.250.179.78");
    out.answers.push_back (a);

    join::ResourceRecord aaaa;
    aaaa.host = "google.com";
    aaaa.type = DnsMessage::AAAA;
    aaaa.dnsclass = DnsMessage::IN;
    aaaa.ttl = 3600;
    aaaa.addr = join::IpAddress ("2a00:1450:4007:80a::200e");
    out.answers.push_back (aaaa);

    join::ResourceRecord ns;
    ns.host = "example.com";
    ns.type = DnsMessage::NS;
    ns.dnsclass = DnsMessage::IN;
    ns.name = "ns1.provider.net";
    out.answers.push_back (ns);

    join::ResourceRecord cname;
    cname.host = "www.example.com";
    cname.type = DnsMessage::CNAME;
    cname.dnsclass = DnsMessage::IN;
    cname.name = "example.com";
    out.answers.push_back (cname);

    join::ResourceRecord ptr;
    ptr.host = "1.0.0.127.in-addr.arpa";
    ptr.type = DnsMessage::PTR;
    ptr.dnsclass = DnsMessage::IN;
    ptr.name = "localhost";
    out.answers.push_back (ptr);

    join::ResourceRecord mx;
    mx.host = "google.com";
    mx.type = DnsMessage::MX;
    mx.dnsclass = DnsMessage::IN;
    mx.mxpref = 10;
    mx.name = "smtp.google.com";
    out.answers.push_back (mx);

    join::ResourceRecord txt;
    txt.host = "google.com";
    txt.type = DnsMessage::TXT;
    txt.dnsclass = DnsMessage::IN;
    txt.txts.push_back ("v=spf1 include:_spf.google.com ~all");
    out.answers.push_back (txt);

    join::ResourceRecord soa;
    soa.host = "google.com";
    soa.type = DnsMessage::SOA;
    soa.dnsclass = DnsMessage::IN;
    soa.name = "ns1.google.com";
    soa.mail = "admin@google.com";
    soa.serial = 2026042001;
    soa.refresh = 7200;
    soa.retry = 3600;
    soa.expire = 1209600;
    soa.minimum = 300;
    out.authorities.push_back (soa);

    join::ResourceRecord srv;
    srv.host = "_sip._udp.example.com";
    srv.type = DnsMessage::SRV;
    srv.dnsclass = DnsMessage::IN;
    srv.priority = 10;
    srv.weight = 60;
    srv.port = 5060;
    srv.name = "sip.example.com";
    out.additionals.push_back (srv);

    DnsMessage codec;
    char data[1024];
    ssize_t size = codec.serialize (out, data, sizeof (data));
    ASSERT_NE (size, -1) << join::lastError.message ();

    join::DnsPacket in;
    ASSERT_EQ (codec.deserialize (in, data, static_cast<size_t> (size)), 0) << join::lastError.message ();

    EXPECT_EQ (in.id, out.id);
    EXPECT_EQ (in.questions.size (), 1);
    EXPECT_EQ (in.questions[0].host, "google.com");

    EXPECT_EQ (in.answers.size (), 7);
    EXPECT_EQ (in.answers[0].addr.toString (), "142.250.179.78");
    EXPECT_EQ (in.answers[1].addr.toString (), "2a00:1450:4007:80a::200e");
    EXPECT_EQ (in.answers[2].name, "ns1.provider.net");
    EXPECT_EQ (in.answers[3].name, "example.com");
    EXPECT_EQ (in.answers[4].name, "localhost");
    EXPECT_EQ (in.answers[5].mxpref, 10);
    EXPECT_EQ (in.answers[5].name, "smtp.google.com");
    EXPECT_EQ (in.answers[6].txts[0], "v=spf1 include:_spf.google.com ~all");

    EXPECT_EQ (in.authorities.size (), 1);
    EXPECT_EQ (in.authorities[0].mail, "admin@google.com");
    EXPECT_EQ (in.authorities[0].serial, 2026042001);

    EXPECT_EQ (in.additionals.size (), 1);
    EXPECT_EQ (in.additionals[0].port, 5060);

    for (size_t length = 0; length < static_cast<size_t> (size); ++length)
    {
        ASSERT_EQ (codec.serialize (out, data, length), -1) << "length " << length;
        ASSERT_EQ (join::lastError, join::Errc::MessageTooLong) << join::lastError.message ();
    }

    ASSERT_EQ (codec.serialize (out, data, sizeof (data)), size);

    for (size_t length = 0; length < static_cast<size_t> (size); ++length)
    {
        join::DnsPacket truncated;
        ASSERT_EQ (codec.deserialize (truncated, data, length), -1) << "length " << length;
    }
}

/**
 * @brief test recursive name decoding error.
 */
TEST (DnsMessage, recursiveError)
{
    DnsMessage codec;
    std::string wire;

    uint16_t header[] = {htons (0x1234), 0, htons (1), 0, 0, 0};
    wire.append ((char*)header, 12);

    uint16_t loop = htons (0xC000 | 12);
    wire.append ((char*)&loop, 2);

    join::DnsPacket packet;
    EXPECT_EQ (codec.deserialize (packet, wire.data (), wire.size ()), -1);

    uint16_t outside = htons (0xC000 | 14);
    wire.replace (12, 2, (char*)&outside, 2);
    EXPECT_EQ (codec.deserialize (packet, wire.data (), wire.size ()), -1);
    EXPECT_EQ (join::lastError, join::Errc::InvalidParam) << join::lastError.message ();
}

/**
 * @brief test unknown record types.
 */
TEST (DnsMessage, unknownRecord)
{
    DnsMessage codec;
    std::string wire;

    uint16_t header[] = {htons (1), 0, 0, htons (1), 0, 0};
    wire.append ((char*)header, 12);

    wire.append ("\x04test\x00", 6);
    uint16_t type = htons (99);
    uint16_t dclass = htons (1);
    uint32_t ttl = htonl (60);
    uint16_t dlen = htons (4);
    wire.append ((char*)&type, 2);
    wire.append ((char*)&dclass, 2);
    wire.append ((char*)&ttl, 4);
    wire.append ((char*)&dlen, 2);
    wire.append ("data", 4);

    join::DnsPacket packet;

    EXPECT_EQ (codec.deserialize (packet, wire.data (), wire.size ()), 0);
    ASSERT_EQ (packet.answers.size (), 1);
    EXPECT_EQ (packet.answers[0].type, 99);
}

/**
 * @brief test decodeError method.
 */
TEST (DnsMessage, decodeError)
{
    EXPECT_FALSE (DnsMessage::decodeError (0));
    EXPECT_EQ (DnsMessage::decodeError (1), join::make_error_code (join::Errc::InvalidParam));
    EXPECT_EQ (DnsMessage::decodeError (2), join::make_error_code (join::Errc::OperationFailed));
    EXPECT_EQ (DnsMessage::decodeError (3), join::make_error_code (join::Errc::NotFound));
    EXPECT_EQ (DnsMessage::decodeError (4), join::make_error_code (join::Errc::InvalidParam));
    EXPECT_EQ (DnsMessage::decodeError (5), join::make_error_code (join::Errc::PermissionDenied));
    EXPECT_EQ (DnsMessage::decodeError (99), join::make_error_code (join::Errc::UnknownError));
}

/**
 * @brief test the typeName method.
 */
TEST (DnsMessage, typeName)
{
    EXPECT_EQ (DnsMessage::typeName (0), "UNKNOWN");
    EXPECT_EQ (DnsMessage::typeName (DnsMessage::A), "A");
    EXPECT_EQ (DnsMessage::typeName (DnsMessage::NS), "NS");
    EXPECT_EQ (DnsMessage::typeName (DnsMessage::CNAME), "CNAME");
    EXPECT_EQ (DnsMessage::typeName (DnsMessage::SOA), "SOA");
    EXPECT_EQ (DnsMessage::typeName (DnsMessage::PTR), "PTR");
    EXPECT_EQ (DnsMessage::typeName (DnsMessage::MX), "MX");
    EXPECT_EQ (DnsMessage::typeName (DnsMessage::TXT), "TXT");
    EXPECT_EQ (DnsMessage::typeName (DnsMessage::AAAA), "AAAA");
    EXPECT_EQ (DnsMessage::typeName (DnsMessage::SRV), "SRV");
    EXPECT_EQ (DnsMessage::typeName (DnsMessage::ANY), "ANY");
}

/**
 * @brief test the className method.
 */
TEST (DnsMessage, className)
{
    EXPECT_EQ (DnsMessage::className (0), "UNKNOWN");
    EXPECT_EQ (DnsMessage::className (DnsMessage::IN), "IN");
}

/**
 * @brief main function.
 */
int main (int argc, char** argv)
{
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
