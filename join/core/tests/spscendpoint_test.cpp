/**
 * MIT License
 *
 * Copyright (c) 2025 Mathieu Rabine
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
#include <join/shared.hpp>

// Libraries.
#include <gtest/gtest.h>

using namespace std::chrono_literals;

using join::BasicShared;
using join::Spsc;

class SpscEndpoint : public ::testing::Test
{
protected:
    /**
     * @brief sets up the test fixture.
     */
    void SetUp ()
    {
        ASSERT_EQ (BasicShared <Spsc>::unlink (_name + "_AB"), 0) << join::lastError.message ();
        ASSERT_EQ (BasicShared <Spsc>::unlink (_name + "_BA"), 0) << join::lastError.message ();
    }

    static const std::string _name;
    static const uint64_t _elementSize;
    static const uint64_t _capacity;
};

const std::string SpscEndpoint::_name = "test_endpoint";
const uint64_t SpscEndpoint::_elementSize = 64;
const uint64_t SpscEndpoint::_capacity = 4096;

TEST_F (SpscEndpoint, open)
{
    Spsc::Endpoint endpointA (Spsc::Endpoint::Side::A, _name, _elementSize, _capacity), endpointX (Spsc::Endpoint::Side::A, _name, _elementSize, _capacity + 1);
    Spsc::Endpoint endpointB (Spsc::Endpoint::Side::B, _name, _elementSize, _capacity), endpointY (Spsc::Endpoint::Side::B, _name, _elementSize, _capacity + 1);

    ASSERT_EQ (endpointA.open (), 0) << join::lastError.message ();
    ASSERT_TRUE (endpointA.opened ());
    ASSERT_EQ (endpointA.open (), -1);
    ASSERT_EQ (endpointX.open (), -1);
    ASSERT_EQ (endpointA.side (), Spsc::Endpoint::Side::A);
    ASSERT_EQ (endpointA.name (), _name);
    ASSERT_EQ (endpointA.elementSize (), _elementSize);
    ASSERT_EQ (endpointA.capacity (), _capacity);

    ASSERT_EQ (endpointB.open (), 0) << join::lastError.message ();
    ASSERT_TRUE (endpointB.opened ());
    ASSERT_EQ (endpointB.open (), -1);
    ASSERT_EQ (endpointY.open (), -1);
    ASSERT_EQ (endpointB.side (), Spsc::Endpoint::Side::B);
    ASSERT_EQ (endpointB.name (), _name);
    ASSERT_EQ (endpointB.elementSize (), _elementSize);
    ASSERT_EQ (endpointB.capacity (), _capacity);

    endpointA.close ();
    endpointB.close ();

    ASSERT_FALSE (endpointA.opened ());
    ASSERT_FALSE (endpointB.opened ());
}

TEST_F (SpscEndpoint, trySend)
{
    Spsc::Endpoint endpoint (Spsc::Endpoint::Side::A, _name, _elementSize, _capacity);
    char data[_elementSize] = {};

    ASSERT_EQ (endpoint.trySend (data), -1);
    ASSERT_EQ (endpoint.open (), 0) << join::lastError.message ();
    ASSERT_EQ (endpoint.trySend (nullptr), -1);
    ASSERT_FALSE (endpoint.full ());
    ASSERT_EQ (endpoint.available (), _capacity);
    for (uint64_t i = 0; i < _capacity; ++i)
    {
        ASSERT_EQ (endpoint.trySend (data), 0) << join::lastError.message ();
        ASSERT_EQ (endpoint.full (), i == (_capacity - 1));
        ASSERT_EQ (endpoint.available (), (_capacity - 1) - i);
    }
    ASSERT_EQ (endpoint.trySend (data), -1);
    ASSERT_TRUE (endpoint.full ());
    ASSERT_EQ (endpoint.available (), 0);
    endpoint.close ();
}

TEST_F (SpscEndpoint, send)
{
    Spsc::Endpoint endpoint (Spsc::Endpoint::Side::A, _name, _elementSize, _capacity);
    char data[_elementSize] = {};

    ASSERT_EQ (endpoint.send (data), -1);
    ASSERT_EQ (endpoint.open (), 0) << join::lastError.message ();
    ASSERT_EQ (endpoint.send (nullptr), -1);
    ASSERT_FALSE (endpoint.full ());
    ASSERT_EQ (endpoint.available (), _capacity);
    for (uint64_t i = 0; i < _capacity; ++i)
    {
        ASSERT_EQ (endpoint.send (data), 0) << join::lastError.message ();
        ASSERT_EQ (endpoint.full (), i == (_capacity - 1));
        ASSERT_EQ (endpoint.available (), (_capacity - 1) - i);
    }
    endpoint.close ();
}

TEST_F (SpscEndpoint, timedSend)
{
    Spsc::Endpoint endpoint (Spsc::Endpoint::Side::A, _name, _elementSize, _capacity);
    char data[_elementSize] = {};

    ASSERT_EQ (endpoint.timedSend (data, 5ms), -1);
    ASSERT_EQ (endpoint.open (), 0) << join::lastError.message ();
    ASSERT_EQ (endpoint.timedSend (nullptr, 5ms), -1);
    ASSERT_FALSE (endpoint.full ());
    ASSERT_EQ (endpoint.available (), _capacity);
    for (uint64_t i = 0; i < _capacity; ++i)
    {
        ASSERT_EQ (endpoint.timedSend (data, 5ms), 0) << join::lastError.message ();
        ASSERT_EQ (endpoint.full (), i == (_capacity - 1));
        ASSERT_EQ (endpoint.available (), (_capacity - 1) - i);
    }
    ASSERT_EQ (endpoint.timedSend (data, 5ms), -1);
    ASSERT_TRUE (endpoint.full ());
    ASSERT_EQ (endpoint.available (), 0);
    endpoint.close ();
}

TEST_F (SpscEndpoint, tryReceive)
{
    Spsc::Endpoint endpointA (Spsc::Endpoint::Side::A, _name, _elementSize, _capacity);
    Spsc::Endpoint endpointB (Spsc::Endpoint::Side::B, _name, _elementSize, _capacity);
    char data[_elementSize] = {};

    ASSERT_EQ (endpointA.open (), 0) << join::lastError.message ();
    ASSERT_EQ (endpointB.tryReceive (data), -1);
    ASSERT_EQ (endpointB.open (), 0) << join::lastError.message ();
    ASSERT_EQ (endpointB.tryReceive (nullptr), -1);
    ASSERT_TRUE (endpointB.empty ());
    ASSERT_EQ (endpointB.pending (), 0);
    ASSERT_EQ (endpointA.trySend (data), 0) << join::lastError.message ();
    ASSERT_FALSE (endpointB.empty ());
    ASSERT_EQ (endpointB.pending (), 1);
    ASSERT_EQ (endpointB.tryReceive (data), 0) << join::lastError.message ();
    ASSERT_TRUE (endpointB.empty ());
    ASSERT_EQ (endpointB.pending (), 0);
    ASSERT_EQ (endpointB.tryReceive (data), -1);
    endpointB.close ();
    endpointA.close ();
}

TEST_F (SpscEndpoint, receive)
{
    Spsc::Endpoint endpointA (Spsc::Endpoint::Side::A, _name, _elementSize, _capacity);
    Spsc::Endpoint endpointB (Spsc::Endpoint::Side::B, _name, _elementSize, _capacity);
    char data[_elementSize] = {};

    ASSERT_EQ (endpointA.open (), 0) << join::lastError.message ();
    ASSERT_EQ (endpointB.receive (data), -1);
    ASSERT_EQ (endpointB.open (), 0) << join::lastError.message ();
    ASSERT_EQ (endpointB.receive (nullptr), -1);
    ASSERT_TRUE (endpointB.empty ());
    ASSERT_EQ (endpointB.pending (), 0);
    ASSERT_EQ (endpointA.trySend (data), 0) << join::lastError.message ();
    ASSERT_FALSE (endpointB.empty ());
    ASSERT_EQ (endpointB.pending (), 1);
    ASSERT_EQ (endpointB.receive (data), 0) << join::lastError.message ();
    ASSERT_TRUE (endpointB.empty ());
    ASSERT_EQ (endpointB.pending (), 0);
    endpointB.close ();
    endpointA.close ();
}

TEST_F (SpscEndpoint, timedReceive)
{
    Spsc::Endpoint endpointA (Spsc::Endpoint::Side::A, _name, _elementSize, _capacity);
    Spsc::Endpoint endpointB (Spsc::Endpoint::Side::B, _name, _elementSize, _capacity);
    char data[_elementSize] = {};

    ASSERT_EQ (endpointA.open (), 0) << join::lastError.message ();
    ASSERT_EQ (endpointB.timedReceive (data, 5ms), -1);
    ASSERT_EQ (endpointB.open (), 0) << join::lastError.message ();
    ASSERT_EQ (endpointB.timedReceive (nullptr, 5ms), -1);
    ASSERT_TRUE (endpointB.empty ());
    ASSERT_EQ (endpointB.pending (), 0);
    ASSERT_EQ (endpointA.trySend (data), 0) << join::lastError.message ();
    ASSERT_FALSE (endpointB.empty ());
    ASSERT_EQ (endpointB.pending (), 1);
    ASSERT_EQ (endpointB.timedReceive (data, 5ms), 0) << join::lastError.message ();
    ASSERT_TRUE (endpointB.empty ());
    ASSERT_EQ (endpointB.pending (), 0);
    ASSERT_EQ (endpointB.timedReceive (data, 5ms), -1);
    endpointB.close ();
    endpointA.close ();
}

/**
 * @brief main function.
 */
int main (int argc, char **argv)
{
    ::mlockall (MCL_CURRENT | MCL_FUTURE);
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
